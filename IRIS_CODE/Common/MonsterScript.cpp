#include "StdAfx.h"
#include "MonsterScript.h"

#include "Monster_Common.h"

#include "Token.h"
#include "Parser.h"
#include "Lexer.h"
#include "FileLoader.h"
#include "Tokenizer.h"

#include "Mode.h"

#include "Stage_Common.h"
#include "BaseObject.h"

#ifdef _CLIENT
#include "GameResourceManager.h"
#else
#include "Drop_Common.h"
#endif




cMonsterScript* cMonsterScript::mpMonsterScript = NULL;



cMonsterScript::cMonsterScript(void)
{

	/// 싱글톤
	if( mpMonsterScript )
	{
		assert(NULL);
	}
	else
	{
		mpMonsterScript = this;
	}

	mpMonsterMap = new cPointHashMap( 999 );
	mpMonsterModeAgentMap = new cPointHashMap( 999 );
	mpRegenMonsterMap = new cPointHashMap( 999 );
	mpRegenGroupMap = new cPointHashMap( 999 );
	mpModelNumberMap = new cPointHashMap( 100 );
	mpMonsterTalkMap = new cPointHashMap( 999 );
	mpRegenSwitchGroup = new cPointHashMap( 100 );
	mpMonsterPattern = new cPointHashMap( 100 );
	mpSummonPatternMap = new cPointHashMap( 100 );
	//mpMonsterCourseMoveMap = new cPointHashMap( 999 );
}



bool cMonsterScript::Init()
{

	/// 몬스터 고유 리스트 로드
	if( !LoadMonsterList() )
	{
		assert(NULL);
		return false;
	}


#ifndef _CLIENT

	if( LoadMonsterTalkList() == false )
	{
		assert(NULL);
		return false;
	}

	if( LoadSummonPattern() == false )
	{
		assert(NULL);
		return false;
	}
#endif

	return true;

}


bool cMonsterScript::StageInit( unsigned short mapNumber, unsigned char levelMode )
{
	if( mapNumber == MAP_NONE )
	{
		cPointHashMap* pRegenMonsterMap = new cPointHashMap;
		cPointHashMap* pRegenGroupMap = new cPointHashMap;
		cPointHashMap* pRegenSwitchGroup = new cPointHashMap;

		cPointHashMap* pMonsterPattern = new cPointHashMap;

		for( unsigned short mapNum = MAP_MIN ; mapNum <= MAP_MAX ; ++mapNum )
		{
			/// 맵에 위치한 몬스터 로드
			if( !LoadRegenMonsterlist( mapNum, levelMode, pRegenMonsterMap, pRegenGroupMap, pRegenSwitchGroup ) )
			{
				assert(NULL);
				return false;
			}

			if( LoadMonsterPattern( mapNum, levelMode, pMonsterPattern ) == false )
			{
				assert(NULL);
				return false;
			}
		}

		mpRegenMonsterMap->Insert( 0, pRegenMonsterMap );
		mpRegenGroupMap->Insert( 0, pRegenGroupMap );
		mpRegenSwitchGroup->Insert( 0, pRegenSwitchGroup );

		mpMonsterPattern->Insert( 0, pMonsterPattern );
	}
	else
	{
		cPointHashMap* pRegenMonsterMap = new cPointHashMap;
		cPointHashMap* pRegenGroupMap = new cPointHashMap;
		cPointHashMap* pRegenSwitchGroup = new cPointHashMap;

		cPointHashMap* pMonsterPattern = new cPointHashMap;

		/// 맵에 위치한 몬스터 로드
		if( !LoadRegenMonsterlist( mapNumber, levelMode, pRegenMonsterMap, pRegenGroupMap, pRegenSwitchGroup ) )
		{
			assert(NULL);
			return false;
		}

		if( LoadMonsterPattern( mapNumber, levelMode, pMonsterPattern ) == false )
		{
			assert(NULL);
			return false;
		}

		mpRegenMonsterMap->Insert( levelMode, pRegenMonsterMap );
		mpRegenGroupMap->Insert( levelMode, pRegenGroupMap );
		mpRegenSwitchGroup->Insert( levelMode, pRegenSwitchGroup );

		mpMonsterPattern->Insert( levelMode, pMonsterPattern );
	}

	return true;
}


void cMonsterScript::Release()
{
	mDummyArray.Clear();

	ReleaseSummonPattern();
	ReleaseRegenSwitchGroup();
	ReleaseMonsterTalk();
	ReleaseModelNumber();
	ReleaseRegenGroup();
	ReleaseRegenMonster();
	ReleaseMonsterMode();
	ReleaseMonsterPattern();
	ReleaseMonsterList();

	SAFE_DELETE( mpSummonPatternMap );
	SAFE_DELETE( mpRegenSwitchGroup );
	SAFE_DELETE( mpMonsterTalkMap );
	SAFE_DELETE( mpModelNumberMap );
	SAFE_DELETE( mpRegenGroupMap );
	SAFE_DELETE( mpRegenMonsterMap );
	SAFE_DELETE( mpMonsterModeAgentMap );
	SAFE_DELETE( mpMonsterPattern );
	SAFE_DELETE( mpMonsterMap );

	//mpMonsterCourseMoveMap = new cPointHashMap( 999 );

	//ReleaseCourseMove();
}



bool cMonsterScript::LoadMonsterList()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/MonsterList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MonsterList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	unsigned long nameIdx = 0;
	unsigned long jobnameIdx = 0;

	sMonsterScript* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sMonsterScript;

		//몬스터 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMonsterClassIdx = str.ToInt();

		/// 몬스터 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		nameIdx = str.ToInt();

		/// 몬스터 직업이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		jobnameIdx = str.ToInt();

#ifdef _CLIENT
		/// 몬스터 이름
		_stprintf_s( pList->mName, _T( "%s" ), GAMERESOURCEMAN->GetMonsterName( nameIdx ) ); 

		/// 몬스터 직업명
		if( jobnameIdx > 0 )
		{
			_stprintf_s( pList->mJobName, _T( "%s" ), GAMERESOURCEMAN->GetMonsterName( jobnameIdx ) ); 
		}
		else
		{
			_stprintf( pList->mJobName, _T( "" ) ); 
		}
#else
		/// 몬스터 이름
		_stprintf( pList->mName, _T( "" ) ); 
#endif

		/// kfm파일idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mKfmIdx = str.ToInt();

		/// 이름 높이값
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNameHeight = str.ToInt();

		///
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mKind = (unsigned char)str.ToInt();

		/// 속성
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mType = (unsigned char)str.ToInt();

		/// 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mLevel = (unsigned char)str.ToInt();

		/// HP
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMaxHp = str.ToInt();

		/// MP
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMaxMp = str.ToInt();

		/// exp획득량
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mExp = str.ToInt();

		/// sxp획득량
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mSxp = str.ToInt();

		/// 물리방어
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mDef = (unsigned short)str.ToInt();

		/// 마법방어
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMdef = (unsigned short)str.ToInt();

		/// 명중수치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mHit = str.ToInt();

		/// 회피율
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mAvoid = (unsigned short)str.ToInt();

		/// 시야범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mSeekRange = str.ToInt();

		/// 비중
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mImportance = (unsigned char)str.ToInt();

		/// 크기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMonsterScale = str.ToFloat();

		/// 연출이펙트 크기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mEffectScale = str.ToFloat();

		/// 피해범위 - 보간 사이즈
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mMonsterFixSize = str.ToFloat();

		/// 행동범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFreeMoveRange = str.ToInt();

		/// 행동(정지)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionStopRatio = (unsigned short)str.ToInt();

		/// 행동(걷기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionWalkRatio = (unsigned short)str.ToInt();

		/// 행동(뛰기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionRunRatio = (unsigned short)str.ToInt();

		/// 정지시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionStopTime = str.ToInt();

		/// 이동속도(걷기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionWalkSpd = str.ToInt();

		/// 이동속도(뛰기,도망)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mActionRunSpd = str.ToInt();

		/// 선공여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mIsPrecedeAttack = str.ToInt() == 1 ? true : false;

		/// 변신인식
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mIsChgMonCheck = str.ToInt() == 1 ? true : false;

		/// 추적시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFollowTime = str.ToInt();

		/// 추적거리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFollowRange = str.ToInt();

		/// 도망조건
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mEscapeType = (unsigned char)str.ToInt();

		/// 도망확률
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mEscapePer = (unsigned char)str.ToInt();

		/// 회복여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mIsRecovery = str.ToInt() == 1 ? true : false;

		/// 회복시작시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mRecoveryTime = str.ToInt();

		/// 해쉬에 몬스터 별로 기록
		if( mpMonsterMap->Insert( pList->mMonsterClassIdx, pList ) == false )
		{
			assert(0);
			goto ERR;
		}

		LoadMonsterMode( pList->mMonsterClassIdx );

		// 더미 소환 목록 저장
		if( pList->mKind == 0 )
		{
			mDummyArray.PushBack( pList->mMonsterClassIdx );
		}
	}

	return true;

ERR:
	delete pList;
	return false;

}


sMonsterScript* cMonsterScript::GetMonsterListInfo( unsigned long monsterIdx )
{
	return (sMonsterScript*)mpMonsterMap->GetAt( monsterIdx );
}

unsigned long cMonsterScript::GetRandomDummy()
{
	unsigned long size = mDummyArray.GetSize();
	unsigned long seed = rand() % size;

	if( seed >= size )
	{
		assert(0);
		seed = rand() % size;
	}
	return mDummyArray[seed];
}

void cMonsterScript::ReleaseMonsterList()
{
	if( mpMonsterMap )
	{
		sMonsterScript* pInfo = NULL;
		cPointHashMap::cIterator i = mpMonsterMap->Begin();
		cPointHashMap::cIterator end = mpMonsterMap->End();

		cPointHashMap::cIterator q;
		cPointHashMap::cIterator qEnd;

		for( ; i != end; ++i )
		{
			pInfo = (sMonsterScript*)((*i).mSecond);
			if( pInfo )
			{
#ifndef _CLIENT
				sDropQType* pDropQ;
				cAryVoid* pAry;

				/// 퀘스트 아이템 목록 제거
				cPointHashMap* pDropMap = pInfo->mpQuestItemDropMap;
				if( pDropMap != NULL )
				{
					q = pDropMap->Begin();
					qEnd = pDropMap->End();
					for( ; q != qEnd; ++q )
					{
						pAry = (cAryVoid*)((*q).mSecond);
						if( pAry == NULL )
							continue;

						for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
						{
							pDropQ = (sDropQType*)(*pAry)[j];
							delete pDropQ;
						}					
						pAry->Clear();
						SAFE_DELETE( pAry );
					}
					pDropMap->Clear();
					SAFE_DELETE( pDropMap );
				}
#endif

				/// 몬스터 정보 제거
				delete pInfo;
				pInfo = NULL;
			}
		}
		mpMonsterMap->Clear();
	}
}



bool cMonsterScript::LoadRegenMonsterlist( unsigned short mapnum, unsigned char levelMode, 
										  tPointerHashMap<unsigned long, void*>* pRegenMonsterMap,
										  tPointerHashMap<unsigned long, void*>* pRegenGroupMap,
										  tPointerHashMap<unsigned long, void*>* pRegenSwitchGroup )
{
	if( pRegenMonsterMap == NULL )
		return false;
	if( pRegenGroupMap == NULL )
		return false;
	if( pRegenSwitchGroup == NULL )
		return false;

	cFileLoader loader;
	cString pathName;
	pathName.Format( "./Script/Resource/MonsterRegen%d_%d.txt", mapnum, levelMode );
	unsigned long insertIdx = 1;

	if( loader.Open( pathName, true ) == false )
	{
		/// 파일을 읽지 못하면 정상리턴
		return true;
	}

	if( loader.GetSize() == 0 )
		return true;

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	unsigned long groupNumber = 0;
	sRegenGroupScript* pGroupList = NULL;
	sRegenScript* pList = NULL;

	cHashSet* pMondelNumberHashSet = (cHashSet*)mpModelNumberMap->GetAt( mapnum );
	if( pMondelNumberHashSet == NULL )
	{
		pMondelNumberHashSet = new cHashSet;
		/// 맵에 해당하는 모델 정보 해쉬셋을 기록
		if( mpModelNumberMap->Insert( mapnum, pMondelNumberHashSet ) == false )
		{
			assert(NULL);
			goto ERR;
		}
	}

	float direction = 0;

	while( tok.IsEnd() == false )
	{
		/// 괄호상단 첫줄 그룹번호
		tok.GetNext( &str );
		groupNumber = (unsigned short)str.ToInt();

		/// 그룹 고유번호가 맵넘버 자리까지 침범하는것을 방지
		if( MAPNUMBER_UNIT <= groupNumber )
		{
			assert(NULL);
			goto ERR;
		}

		/// 맵별로 구별을 하기위한 그룹 넘버
		groupNumber = ( mapnum * MAPNUMBER_UNIT ) + groupNumber;

		/// 그룹 번호 안의 세부항목 시작
		tok.GetNext( &str );
		if( str != '{' )
		{
			assert( 0 && "wrong script" );
			goto ERR;
		}

		/// 그룹 설정 조건
		pGroupList = new sRegenGroupScript;

		pGroupList->mGroupIdx = groupNumber;
		/// 서버시작시 리젠되는 그룹인지 여부
		tok.GetNext( &str );
		pGroupList->mServerOnRegen = str.ToInt() == 0 ? false : true;	
/*
		/// 그룹 단위 리젠 조건 만족시 리젠될 그룹번호
		tok.GetNext( &str );
		unsigned long regenGroupIdx = str.ToInt();
		if( regenGroupIdx == 0 )
			pGroupList->mRegenGroupIdx = regenGroupIdx;
		else
			pGroupList->mRegenGroupIdx = ( mapnum * MAPNUMBER_UNIT ) + regenGroupIdx;		
*/
		/// 리젠 그룹 몬스터가 일정 퍼센티지 이하 인경우 리젠
		tok.GetNext( &str );
		pGroupList->mRegenPer = (unsigned char)str.ToInt();
		tok.GetNext( &str );
		pGroupList->mRegenRange = str.ToInt();
		tok.GetNext( &str );
		pGroupList->mRegenTime = str.ToInt();
		/// 맵 그룹번호
		tok.GetNext( &str );
		pGroupList->mMapGroupIdx = str.ToInt();
		/// 채널 공유 체크
		tok.GetNext( &str );
		pGroupList->mChannelCheck = str.ToInt() == 1 ? true : false;

		if( pGroupList->mChannelCheck == true )
		{
			/// 채널간 리젠시간이 너무 짧다.
			if( pGroupList->mRegenTime <= SWITCH_RANDTIME + SWITCH_CHANNEL_CHECKTIME )
				goto ERR;
		}

		/// 그룹 단위 리젠 조건 만족시 리젠될 그룹번호
		tok.GetNext( &str );
		unsigned long regenGroupCount = str.ToInt();
		if( regenGroupCount != 0 )
		{
			for( unsigned int i=0; i<regenGroupCount; ++i )
			{
				tok.GetNext( &str );
				unsigned long regenGroupIdx = str.ToInt();

				pGroupList->mRegenGroupAry.PushBack( ( mapnum * MAPNUMBER_UNIT ) + regenGroupIdx );
			}
		}

		/// 선택적인 그룹 리젠 정보 저장
		if( pGroupList->mMapGroupIdx != 0 )
		{
			cArray* pSwitchGroup = (cArray*)pRegenSwitchGroup->GetAt( pGroupList->mMapGroupIdx );
			if( pSwitchGroup == NULL )
			{
				pSwitchGroup = new cArray;
				if( pRegenSwitchGroup->Insert( pGroupList->mMapGroupIdx, pSwitchGroup ) == false )
					goto ERR;
			}

			pSwitchGroup->PushBack( groupNumber );
		}

		while( tok.IsEnd() == false )
		{
			tok.GetNext( &str );
			if( str == "}" )
				break;

			/// 개개의 몬스터 리젠 설정 조건
			pList = new sRegenScript;

			/// 몬스터 종류 번호
			pList->mMonsterClassIdx = str.ToInt();
			tok.GetNext( &str );
			pList->mRegenX = str.ToFloat();
			tok.GetNext( &str );
			pList->mRegenY = str.ToFloat();
			tok.GetNext( &str );

			direction = str.ToFloat();

			if( direction <= 1000.0f )
				direction = S_ToRadian( direction );

			pList->mDirection = direction;

			tok.GetNext( &str );
			pList->mRegenTime = str.ToInt();
			tok.GetNext( &str );
			pList->mRegenLifeTime = str.ToInt();
			tok.GetNext( &str );
			pList->mFamilyNum = str.ToInt();
			tok.GetNext( &str );
			pList->mPatternIdx = str.ToInt();

			pList->mMapNumber = mapnum;
			pList->mLevelMode = levelMode;
			pList->mGroupNumber = groupNumber;

			/// 몬스터 고유번호가 맵넘버 자리까지 침범하는것을 방지
			if( MAPNUMBER_UNIT <= insertIdx )
			{
				assert(NULL);
				goto ERR;
			}

			pList->mRegenIdx = ( mapnum * MAPNUMBER_UNIT ) + insertIdx;

			/// 몬스터 인덱스만 보고 어느맵인지 구별을 쉽게 하기위해 맵번호를 몬스터 고유번호에 포함
			if( pRegenMonsterMap->Insert( ( mapnum * MAPNUMBER_UNIT ) + insertIdx, pList ) == false )
			{
				assert(NULL);
				goto ERR;
			}

			/// 그룹 정보에 몬스터 고유번호만 기록
			pGroupList->mMonsterAry.PushBack( ( mapnum * MAPNUMBER_UNIT ) + insertIdx );

			/// 맵정보 제외된 고유번호 증가
			++insertIdx;

			/// 클라이언트에 loadstage시점에 먼저 데이타를 읽어와야 할 몬스터 목록 기록
			if( pMondelNumberHashSet->GetCount( pList->mMonsterClassIdx ) == 0 )
			{
				pMondelNumberHashSet->Insert( pList->mMonsterClassIdx );
			}
		}

		/// 그룹에 해당하는 몬스터 갯수를 확인
		if( pGroupList->mMonsterAry.GetSize() == 0 )
		{
			assert(NULL);
			goto ERR;
		}

		/// 그룹 정보 추가
		if( pRegenGroupMap->Insert( groupNumber, pGroupList ) == false )
		{
			assert(NULL);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pList;
	delete pGroupList;
	return false;
}



sRegenScript* cMonsterScript::GetRegenMonsterInfo( unsigned char modeNum, unsigned long monsterIdx )
{
	if( mpRegenMonsterMap == NULL )
		return NULL;

	cPointHashMap* pModeMap = (cPointHashMap*)mpRegenMonsterMap->GetAt( modeNum );
	if( pModeMap == NULL )
		return NULL;

	return (sRegenScript*)pModeMap->GetAt( monsterIdx );
}



sRegenGroupScript* cMonsterScript::GetRegenGroupInfo( unsigned char modeNum, unsigned long groupIdx )
{
	if( mpRegenGroupMap == NULL )
		return NULL;

	cPointHashMap* pModeMap = (cPointHashMap*)mpRegenGroupMap->GetAt( modeNum );
	if( pModeMap == NULL )
		return NULL;

	return (sRegenGroupScript*)pModeMap->GetAt( groupIdx );
}



void cMonsterScript::ReleaseRegenMonster()
{
	if( mpRegenMonsterMap )
	{
		cPointHashMap::cIterator mI = mpRegenMonsterMap->Begin();
		cPointHashMap::cIterator mE = mpRegenMonsterMap->End();

		for( ; mI != mE ; ++mI )
		{
			cPointHashMap* pRegenMonsterMap = (cPointHashMap*)(*mI).mSecond;
			if( pRegenMonsterMap != NULL )
			{
				cPointHashMap::cIterator i = pRegenMonsterMap->Begin();
				cPointHashMap::cIterator end = pRegenMonsterMap->End();
				for( ; i != end; ++i )
				{
					sRegenScript* pInfo = (sRegenScript*)((*i).mSecond);
					if( pInfo )
					{
						delete pInfo;
						pInfo = NULL;
					}
				}
				pRegenMonsterMap->Clear();
				SAFE_DELETE( pRegenMonsterMap );
			}
		}
		mpRegenMonsterMap->Clear();
		SAFE_DELETE( mpRegenMonsterMap );
	}
}



void cMonsterScript::ReleaseRegenGroup()
{
	if( mpRegenGroupMap )
	{
		cPointHashMap::cIterator mI = mpRegenGroupMap->Begin();
		cPointHashMap::cIterator mE = mpRegenGroupMap->End();

		for( ; mI != mE ; ++mI )
		{
			cPointHashMap* pRegenGroupMap = (cPointHashMap*)(*mI).mSecond;
			if( pRegenGroupMap != NULL )
			{
				sRegenGroupScript* pInfo = NULL;
				cPointHashMap::cIterator i = pRegenGroupMap->Begin();
				cPointHashMap::cIterator end = pRegenGroupMap->End();
				for( ; i != end; ++i )
				{
					pInfo = (sRegenGroupScript*)(*i).mSecond;
					if( pInfo )
					{
						SAFE_DELETE(pInfo);
					}
				}
				pRegenGroupMap->Clear();
				SAFE_DELETE( pRegenGroupMap );
			}
		}
		mpRegenGroupMap->Clear();
		SAFE_DELETE( mpRegenGroupMap );
	}
}



void cMonsterScript::ReleaseModelNumber()
{
	if( mpModelNumberMap )
	{
		cHashSet* pInfo = NULL;
		cPointHashMap::cIterator i = mpModelNumberMap->Begin();
		cPointHashMap::cIterator end = mpModelNumberMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (cHashSet*)(*i).mSecond;
			if( pInfo )
			{
				SAFE_DELETE(pInfo);
			}
		}
		mpModelNumberMap->Clear();
	}
}



bool cMonsterScript::LoadMonsterTalkList()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/MonsterSpeech.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MonsterSpeech.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMonsterTalk* pList = NULL;
	sMonsterTalk** pArray = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sMonsterTalk;

		//몬스터 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned long monsterClassIdx = str.ToInt();

		/// 상황
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		eMONSTER_TALK_STATE talk = (eMONSTER_TALK_STATE)str.ToInt();

		if( talk >= eMONSTERTALK_MAX )				{ goto ERR; }

		/// 표시 설정
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mViewType = (unsigned char)str.ToInt();

		/// 대사 표시 확률
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mViewPercent = (unsigned char)str.ToInt();

		/// 대사 표시 체크 시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mCheckTime = str.ToInt();

		/// 대사 인덱스
		for( unsigned long i = 0 ; i < MONSTER_TALK_SIZE ; ++i )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pList->mTalkIdx[i] = str.ToInt();
		}

		pArray = (sMonsterTalk**)mpMonsterTalkMap->GetAt( monsterClassIdx );
		if( pArray == NULL )
		{
			sMonsterTalk** arry = new sMonsterTalk*[eMONSTERTALK_MAX];
			memset( arry, 0, sizeof(sMonsterTalk*) * eMONSTERTALK_MAX );

			pArray = arry;

			/// 해쉬에 몬스터 별로 기록
			if( mpMonsterTalkMap->Insert( monsterClassIdx, pArray ) == false )
			{
				assert(NULL);
				goto ERR;
			}
		}

		if( pArray[talk] != NULL )
		{
			assert(NULL);
			goto ERR;
		}

		pArray[talk] = pList;
	}

	return true;

ERR:
	delete pList;
	return false;
}


sMonsterTalk* cMonsterScript::GetMonsterTalkInfo( unsigned long monsterClassIdx, unsigned char state )
{
	if( state >= eMONSTERTALK_MAX )
	{
		assert(NULL);
		return NULL;
	}

	sMonsterTalk** pArray = (sMonsterTalk**)mpMonsterTalkMap->GetAt( monsterClassIdx );
	if( pArray == NULL )
		return NULL;

	return pArray[state];		
}


tPointerHashMap<unsigned long, void*>* cMonsterScript::GetSwitchGroup( unsigned char modeNum )
{ 
	if( mpRegenSwitchGroup == NULL )
		return NULL;

	return (tPointerHashMap<unsigned long, void*>*)mpRegenSwitchGroup->GetAt( modeNum );
}

void cMonsterScript::ReleaseMonsterTalk()
{
	if( mpMonsterTalkMap )
	{
		/// mMonsterDramaturgyInfoMap Clear
		cPointHashMap::cIterator i = mpMonsterTalkMap->Begin();
		cPointHashMap::cIterator end = mpMonsterTalkMap->End();
		for( ; i != end; ++i )
		{
			sMonsterTalk** pInfo = (sMonsterTalk**)((*i).mSecond);
			for( unsigned int j = 0 ; j < eMONSTERTALK_MAX ; ++j )
				SAFE_DELETE( pInfo[j] );

			SAFE_DELETE_ARRAY( pInfo );
		}
		mpMonsterTalkMap->Clear();
	}
}

void cMonsterScript::LoadMonsterMode( unsigned long monsterIdx )
{
	cModeAgent* modeAgent = new cModeAgent;
	if( modeAgent == 0 )
	{
		assert(0);
		return;
	}

	if( modeAgent->LoadFile( monsterIdx ) == false )
	{
		delete modeAgent;
		return;
	}

	if( mpMonsterModeAgentMap->Insert( monsterIdx, modeAgent ) == false )
	{
		assert(0);
		delete modeAgent;
		return;
	}
}

void cMonsterScript::ReleaseMonsterMode()
{
	if( mpMonsterModeAgentMap )
	{
		/// 
		cPointHashMap::cIterator i = mpMonsterModeAgentMap->Begin();
		cPointHashMap::cIterator end = mpMonsterModeAgentMap->End();
		for( ; i != end; ++i )
		{
			cModeAgent* agent = (cModeAgent*)((*i).mSecond);
			SAFE_DELETE( agent );
		}
		mpMonsterModeAgentMap->Clear();
	}
}

cModeAgent* cMonsterScript::GetMonsterModeAgent( unsigned long monsterIdx )
{
	return (cModeAgent*)mpMonsterModeAgentMap->GetAt( monsterIdx );
}


sMonsterPattern* cMonsterScript::GetMonsterPattern( unsigned char levelMode, unsigned long patternIdx )
{
	if( mpMonsterPattern == NULL )
		return NULL;

	cPointHashMap* pModeMap = (cPointHashMap*)mpMonsterPattern->GetAt( levelMode );
	if( pModeMap == NULL )
		return NULL;
	
	return (sMonsterPattern*)pModeMap->GetAt( patternIdx );
}


void cMonsterScript::ReleaseRegenSwitchGroup()
{
	if( mpRegenSwitchGroup )
	{
		cPointHashMap::cIterator mI = mpRegenSwitchGroup->Begin();
		cPointHashMap::cIterator mE = mpRegenSwitchGroup->End();

		for( ; mI != mE ; ++mI )
		{
			cPointHashMap* pRegenSwitchGroup = (cPointHashMap*)(*mI).mSecond;
			if( pRegenSwitchGroup != NULL )
			{
				/// mpRegenSwitchGroup Clear
				cPointHashMap::cIterator i = pRegenSwitchGroup->Begin();
				cPointHashMap::cIterator end = pRegenSwitchGroup->End();
				for( ; i != end; ++i )
				{
					cArray* pInfo = (cArray*)((*i).mSecond);
					SAFE_DELETE( pInfo );
				}
				pRegenSwitchGroup->Clear();
				SAFE_DELETE( pRegenSwitchGroup );
			}
		}
		mpRegenSwitchGroup->Clear();
		SAFE_DELETE( mpRegenSwitchGroup );
	}
}


bool cMonsterScript::LoadMonsterPattern( unsigned short mapnum, unsigned char levelMode,
										tPointerHashMap<unsigned long, void*>* pMonsterPattern )
{
	cFileLoader loader;
	cString pathName;
	pathName.Format( "./script/resource/monsterpattern%d_%d.txt", mapnum, levelMode );

	if( loader.Open( pathName, true ) == false )
	{
		return true;
	}

	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	sMonsterPattern* pPattern = NULL;
	sMonsterPatternPos* pPos = NULL;

	while( tok.IsEnd() == false )
	{
		pPattern = new sMonsterPattern;

		/// 패턴idx
		tok.GetNext( &str );
		pPattern->mPatternIdx = str.ToInt();

		/// 패턴설정
		tok.GetNext( &str );
		pPattern->mType = str.ToInt() == 1 ? true : false;

		/// 위치 정보 로드
		tok.GetNext( &str );
		if( str != '{' )
		{
			assert( 0 && "wrong script" );
			goto ERR;
		}

		cAryVoid* pPosAry = &pPattern->mPosAry;

		tok.GetNext( &str );
		while( tok.IsEnd() == false )
		{
			pPos = new sMonsterPatternPos;

			/// 좌표x
			pPos->mPosX = str.ToFloat();

			/// 좌표y
			tok.GetNext( &str );
			pPos->mPosY = str.ToFloat();

			/// 패턴범위
			tok.GetNext( &str );
			pPos->mRange = str.ToInt();

			/// 경유지설정
			tok.GetNext( &str );
			pPos->mRelaxPoint = str.ToInt() == 1 ? true : false;

			/// 대기시간
			tok.GetNext( &str );
			pPos->mWaitTime = str.ToInt();

			pPosAry->PushBack( pPos );

			tok.GetNext( &str );
			if( str == "}" )
				break;
		}

		pMonsterPattern->Insert( pPattern->mPatternIdx, pPattern );

	}

	return true;

ERR:
	delete pPattern;
	delete pPos;
	return false;

}


void cMonsterScript::ReleaseMonsterPattern()
{

	if( mpMonsterPattern )
	{
		cPointHashMap::cIterator mI = mpMonsterPattern->Begin();
		cPointHashMap::cIterator mE = mpMonsterPattern->End();

		for( ; mI != mE ; ++mI )
		{
			cPointHashMap* pMonsterPattern = (cPointHashMap*)(*mI).mSecond;
			if( pMonsterPattern != NULL )
			{
				cPointHashMap::cIterator i = pMonsterPattern->Begin();
				cPointHashMap::cIterator end = pMonsterPattern->End();
				for( ; i != end ; ++i )
				{
					sMonsterPattern* pPattern = (sMonsterPattern*)i->mSecond;
					cAryVoid* pAry = &pPattern->mPosAry;
					for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
					{
						sMonsterPatternPos* pPos = (sMonsterPatternPos*)(*pAry)[j];
						SAFE_DELETE( pPos );			
					}
					pAry->Clear();
					pAry = NULL;

					SAFE_DELETE( pPattern );
				}	
				pMonsterPattern->Clear();
				SAFE_DELETE( pMonsterPattern );
			}
		}
		mpMonsterPattern->Clear();
		SAFE_DELETE( mpMonsterPattern );
	}
}


bool cMonsterScript::LoadSummonPattern()
{
	cFileLoader loader;
	cString pathName;
	pathName.Format( "./script/resource/genpattern.txt" );

	if( loader.Open( pathName, true ) == false )
	{
		return true;
	}

	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	sSummonPatternScript* pPattern = NULL;

	while( tok.IsEnd() == false )
	{
		/// 패턴idx
		tok.GetNext( &str );
		unsigned long patternIdx = str.ToInt();

		cAryVoid* pAry = (cAryVoid*)mpSummonPatternMap->GetAt( patternIdx );
		if( pAry == NULL )
		{
			pAry = new cAryVoid;
			mpSummonPatternMap->Insert( patternIdx, pAry );
		}

		pPattern = new sSummonPatternScript;

		/// 방향
		tok.GetNext( &str );
		pPattern->mDir = S_ToRadian( str.ToFloat() );

		/// 지연시간
		tok.GetNext( &str );
		pPattern->mDelayTime = str.ToInt();

		pAry->PushBack( pPattern );
	}

	return true;
}


void cMonsterScript::ReleaseSummonPattern()
{
	cPointHashMap::cIterator i = mpSummonPatternMap->Begin();
	cPointHashMap::cIterator e = mpSummonPatternMap->End();

	for( ; i != e ; ++i )
	{
		cAryVoid* pAry = (cAryVoid*)(*i).mSecond;
		if( pAry != NULL )
		{
			for( unsigned long b = 0 ; b < pAry->GetSize() ; ++b )
			{
				sSummonPatternScript* pPattern = (sSummonPatternScript*)(*pAry)[b];
				SAFE_DELETE( pPattern );
			}
			SAFE_DELETE( pAry );
		}
	}	
	mpSummonPatternMap->Clear();
	SAFE_DELETE( mpSummonPatternMap );
}
