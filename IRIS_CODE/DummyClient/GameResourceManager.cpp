#include "stdafx.h"
#include "GameResourceManager.h"

cGameFileLexer::cGameFileLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "jobinfo", eTOKEN_JOBINFO );
	BindKeyword( "jobtree", eTOKEN_JOBTREE );

	BindKeyword( "hair", eTOKEN_HAIR );
	BindKeyword( "haircolor", eTOKEN_HAIRCOLOR );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

cGameResourceManager* cGameResourceManager::mpSingleton = 0;

cGameResourceManager::cGameResourceManager()
{
	mpSingleton = this;

}

cGameResourceManager::~cGameResourceManager()
{
	{
		sMonsterScript* pInfo = NULL;
		cPointerHashMap::cIterator i = mMonsterMap.Begin();
		cPointerHashMap::cIterator end = mMonsterMap.End();

		for( ; i != end; ++i )
		{
			pInfo = (sMonsterScript*)((*i).mSecond);
			if( pInfo )
			{
				/// 몬스터 정보 제거
				delete pInfo;
				pInfo = NULL;
			}
		}
		mMonsterMap.Clear();
	}

	cPointerHashMap::cIterator iter;

	if( mPlayerSkillInfoMap.IsEmpty() == false )
	{
		for(iter = mPlayerSkillInfoMap.Begin() ; iter != mPlayerSkillInfoMap.End() ; ++iter)
		{
			delete (sPlayerSkillBaseInfo*)(*iter).mSecond;
		}
		mPlayerSkillInfoMap.Clear();
	}

	if( mStageChangePosMap.IsEmpty() == false )
	{
		for(iter = mStageChangePosMap.Begin(); iter != mStageChangePosMap.End(); ++iter)
		{
			delete (sStageChangePos*)(*iter).mSecond;
		}
		mStageChangePosMap.Clear();
	}

	///
	cPointerHashMap::cIterator i = mMapInfoMap.Begin();
	cPointerHashMap::cIterator end = mMapInfoMap.End();
	i = mMapInfoMap.Begin();
	end = mMapInfoMap.End();
	for( ; i != end; ++i )
	{
		cPointerHashMap* pMap = (cPointerHashMap*)(*i).mSecond;
		if( pMap != NULL )
		{
			cPointerHashMap::cIterator d;

			for( d = pMap->Begin() ; d != pMap->End() ; ++d )
			{
				sMapInfo* p = (sMapInfo*)(*d).mSecond;
				SAFE_DELETE(p);
			}
			pMap->Clear();
			SAFE_DELETE( pMap );
		}		
	}
	mMapInfoMap.Clear();

	mpSingleton = 0;
}

bool cGameResourceManager::Init()
{
	if( LoadMapInfo() == false )
		return false;

	if( StageChangePosScriptLoad() == false )
		return false;

	if( LoadCharacterBaseHairFileInfo() == false )
		return false;

	if( LoadFaceEmotionInfo() == false )
		return false;

	if( LoadMonsterList() == false )
		return false;

	if( PlayerSkillScriptLoad() == false )
		return false;
	if( PlayerSkillStepLoad() == false )
		return false;

	return true;
}


bool cGameResourceManager::LoadMapInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/MapData.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapData.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMapInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sMapInfo;

		// Map number
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapNum = str.ToInt();

		if( pInfo->mapNum > 999 )	{ goto ERR; }

		cPointerHashMap* pModeMap = (cPointerHashMap*)mMapInfoMap.GetAt( pInfo->mapNum );
		if( pModeMap == NULL )
		{
			pModeMap = new cPointerHashMap;
			mMapInfoMap.Insert( pInfo->mapNum, pModeMap );
		}

		/// Map 폴더번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->folderIdx = str.ToInt();

		/// 한개의 맵타입에 모드별로 다른 맵데이터를 읽어오는경우가 있다면 
		/// 관련부분을 다처리 한후에 이부분을 빼야한다.
		/// ※ 모드가 나뉘는것은 리소스를 재활용하고 리젠정보만 바꾸기 위함이다
		/// 현재는 모드별 다른맵 설정된경우는 오타일 가능성이 높다
		if( pModeMap->GetSize() != 0 )
		{
			cPointerHashMap::cIterator b = pModeMap->Begin();
			sMapInfo* pMapInfo = (sMapInfo*)(*b).mSecond;
			if( pMapInfo == NULL )
				goto ERR;

			if( pMapInfo->folderIdx != pInfo->folderIdx )
				goto ERR;			
		}

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 난이도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapMode = (unsigned char)str.ToInt();

		/// 최소레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->minLevel = (unsigned char)str.ToInt();
		/// 최대레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxLevel = (unsigned char)str.ToInt();
		/// 최대인원
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxPlayer = (unsigned char)str.ToInt();
		/// 필요아이템
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->itemIdx = str.ToInt();

		if( pModeMap->Insert( pInfo->mapMode, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pInfo;
	return false;
}

bool cGameResourceManager::LoadCharacterBaseHairFileInfo()
{
	mHairCnt = 0;
	mHairColorCnt = 0;

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/CharacterHairFile.txt", true ) == false )
	{
		return false;
	}

	cToken token;
	cGameFileLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/CharacterHairFile.txt" );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_HAIR:
			{
				if( LoadHair(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_HAIRCOLOR:
			{
				mHairColorCnt = parser.ParseInt();
				if( LoadHairColor(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}
bool cGameResourceManager::LoadHair( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				token.ToInt();
				for( unsigned int i=0; i<count; ++i)
					parser.ParseString();

				mHairCnt++;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cGameResourceManager::LoadHairColor( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				// 키 생성
				token.ToInt();
				parser.ParseInt();

				for( unsigned int i=0; i<count; ++i)
					parser.ParseString();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cGameResourceManager::LoadFaceEmotionInfo()
{
	mFaceCnt = 0;

	cFileLoader loader;
	cString pathName = "./Script/Resource/EmotionList.txt";
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapData.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	while( tokenizer.IsEnd() == false )
	{
		/// 
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		str.ToInt();

		/// type
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned char type = (unsigned char)str.ToInt();

		/// texture file name
		for( unsigned int i=0; i<count; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		}

		/// filter text index
		for( unsigned int i=0; i<10; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			str.ToInt();
		}

		if( type == 0 )
			mFaceCnt++;
	}

	return true;

ERR:
	return false;
}

bool cGameResourceManager::StageChangePosScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/MapChangePos.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapChangePos.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();
	sStageChangePos* pChangePos = 0;

	while( tokenizer.IsEnd() == false )
	{
		pChangePos = new sStageChangePos;

		/// 고유번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosIdx = str.ToInt();

		///
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mFileName = str;

		/// 맵번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mMapNumber = (unsigned char)str.ToInt();

		/// 좌표X
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosX = str.ToFloat();

		/// 좌표Y
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosY = str.ToFloat();

		/// 범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mRange = str.ToFloat();

		/// 목적지맵idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mTargetPosIdx = str.ToInt();

		/// 회전 각도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mRotAngle = S_ToRadian( str.ToFloat() );

		/// 자동생성 유무
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mFirstRegen = str.ToInt() == 1;

		/// 포탈명 출력 높이값
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mNameHeight = str.ToFloat();

		/// 보관소 저장
		if( mStageChangePosMap.Insert( pChangePos->mPosIdx, pChangePos ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	begin = mStageChangePosMap.Begin();
	while( begin != mStageChangePosMap.End() )
	{
		pChangePos = (sStageChangePos*)((*begin).mSecond);
		++begin;

		if( pChangePos == 0 )
		{
			assert(0);
			goto ERR;
		}

		pChangePos->mTargetInfo = (sStageChangePos*)mStageChangePosMap.GetAt( pChangePos->mTargetPosIdx );

		if( pChangePos->mTargetInfo != NULL )
		{
			/// 텔레포트포탈로 생성되는데 맵번호가틀리면 이동좌표가 틀어짐
			if( pChangePos->mFirstRegen == false )
			{
				if( pChangePos->mMapNumber != pChangePos->mTargetInfo->mMapNumber )
				{
					assert(0);
					goto ERR;
				}
			}
		}
	}

	return true;

ERR:
	delete pChangePos;

	return false;
}

bool cGameResourceManager::LoadMonsterList()
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

		/// 몬스터 이름
		_stprintf( pList->mName, _T( "" ) ); 

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
		if( mMonsterMap.Insert( pList->mMonsterClassIdx, pList ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pList;
	return false;
}


bool cGameResourceManager::PlayerSkillScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/SkillList_PC.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SkillList_PC.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sPlayerSkillBaseInfo* pSkillInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pSkillInfo = new sPlayerSkillBaseInfo;

		/// 스킬 종류Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mSkillIdx = str.ToInt();

		/// 스킬 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mNameIdx = str.ToInt();

		/// 스킬 아이콘Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mIconIdx = str.ToInt();

		/// 습득,사용 종족
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mRace = static_cast<unsigned char>(str.ToInt());

		/// 직업 단계
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mJobStep = static_cast<unsigned char>(str.ToInt());

		/// 스킬습득조건 - 총사용 SP양
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mBeforeReqSP = str.ToInt();

		/// 스킬사용조건 - 필요장비
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mUseEquipment = static_cast<unsigned char>(str.ToInt());

		/// 스킬사용조건 - 캐릭터상태
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mUseState = static_cast<unsigned char>(str.ToInt());

		/// 스킬사용조건 - 캐릭터상태
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mUseItem = str.ToInt();

		/// 분류 - 스킬 사용 형태
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mType = static_cast<eSKILLTYPE>(str.ToInt());

		/// 스킬유형 - 속성
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mAttributeType = static_cast<eATTRIBUTETYPE>(str.ToInt());

		/// 시전유형
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mShotType = static_cast<eSHOTTYPE>(str.ToInt());

		/// 범위방식 - 스킬 사용시 기준의 범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mBoundType = static_cast<eBOUNDTYPE>(str.ToInt());

		/// 효과적용타겟 - 범위안에서 효과를 적용받는 대상
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyType = static_cast<eAPPLYTYPE>(str.ToInt());

		/// 스킬유형 - 사정거리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mRangeType = static_cast<eRANGETYPE>(str.ToInt());

		/// 글로벌쿨타임
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mIsGlobalCoolTime = str.ToInt() == 0 ? false : true;

		/// 그룹쿨타임
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mGroupCoolTimeIdx = str.ToInt();

		/// 보관소 저장
		if( mPlayerSkillInfoMap.Insert( pSkillInfo->mSkillIdx, pSkillInfo ) == false )
		{
			assert(0);
			goto ERR;
		}

	}
	return true;

ERR:
	delete pSkillInfo;
	return false;
}

bool cGameResourceManager::PlayerSkillStepLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/SkillList_Sub.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SkillList_Sub.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	unsigned long skillIdx = ULONG_MAX;
	unsigned char step;

	sPlayerSkillStepInfo	stepArray[50] = {0,};
	unsigned char			stepCount = 0;
	unsigned long			currentSkillIdx = ULONG_MAX;

	while( tokenizer.IsEnd() == false )
	{
		/// 스킬 종류Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		skillIdx = str.ToInt();

		if( currentSkillIdx != ULONG_MAX && (currentSkillIdx != skillIdx) )
		{
			/// 지금까지 쌓인 정보를 셋팅한다.
			assert(stepCount);
			assert(stepCount<50);

			sPlayerSkillBaseInfo* pSkillInfo = GetPlayerSkillInfo( currentSkillIdx );
			if( pSkillInfo == 0 )
			{
				assert(0);
				return false;
			}

			/// 읽어들인 정보를 셋팅한다.
			pSkillInfo->mpSetpInfoArray = new sPlayerSkillStepInfo[stepCount];
			::memcpy( pSkillInfo->mpSetpInfoArray, stepArray, sizeof(sPlayerSkillStepInfo)*stepCount);
			pSkillInfo->mStepCount = stepCount;

			/// 정보 초기화
			stepCount = 0;
		}
		currentSkillIdx = skillIdx;
		stepCount++;

		/// 스킬 단계
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		step = static_cast<unsigned char>(str.ToInt());

		/// 툴팁Index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mToolTipIdx = str.ToInt();

		/// SP필요값	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mRequireSP = static_cast<unsigned short>(str.ToInt());

		/// 직업 Index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mJobType = static_cast<unsigned long>(str.ToInt());

		/// 캐릭터 필요Lv	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mPlayerLevel = static_cast<unsigned char>(str.ToInt());

		/// 스킬습득조건1 - 필요스킬idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mLearnSkillIdx1 = str.ToInt();

		/// 스킬습득조건1 - 스킬필요단계
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mLearnSkillStep1 = static_cast<unsigned char>(str.ToInt());

		/// 스킬습득조건2 - 필요스킬idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mLearnSkillIdx2 = str.ToInt();

		/// 스킬습득조건2 - 스킬필요단계
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mLearnSkillStep2 = static_cast<unsigned char>(str.ToInt());

		/// 소모금액
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseMoney = str.ToInt();

		/// 상태수치값	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseStateValue = static_cast<unsigned short>(str.ToInt());

		/// 소모MP값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseMP = str.ToInt();

		/// 소모MP값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseMPPer = str.ToInt();

		/// 소모HP값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseHP = str.ToInt();

		/// 소모아이템개수	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mUseItemCount = static_cast<unsigned char>(str.ToInt());

		/// 사거리
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mTargetDist = static_cast<float>(str.ToInt());

		/// 캐스팅시간
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mCastingTime = str.ToInt();

		/// 쿨타임	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mCoolTime = str.ToInt();

		/// 범위거리1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mBoundDist1 = static_cast<unsigned short>(str.ToInt());

		/// 범위거리2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mBoundDist2 = static_cast<unsigned short>(str.ToInt());

		/// 적용인원
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mApplyCnt = static_cast<unsigned char>(str.ToInt());

		/// 고유명중률	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mAccuracyValue = static_cast<short>(str.ToInt());

		/// 고유크리티컬	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mCriticalValue = static_cast<unsigned short>(str.ToInt());

		/// 고유 어그로 수치
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mDistressValue = str.ToInt();

		/// 적용값 type1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mApplyValueType1 = static_cast<unsigned short>(str.ToInt());

		/// 적용값1	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mApplyValue1 = str.ToInt();

		/// 적용값 type2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mApplyValueType2 = static_cast<unsigned short>(str.ToInt());

		/// 적용값2	
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mApplyValue2 = str.ToInt();

		/// 효과 Index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mInfulenceIdx = str.ToInt();

//		if( stepArray[step].mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH && stepArray[step].mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
//			return false;

//		if( stepArray[step].mApplyValueType1 == eSTATUSPLUS_AROUND_PULL && stepArray[step].mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
//			return false;

		if( tokenizer.IsEnd() )
		{
			/// 지금까지 쌓인 정보를 셋팅한다.
			assert(stepCount);
			assert(stepCount<50);

			sPlayerSkillBaseInfo* pSkillInfo = GetPlayerSkillInfo( currentSkillIdx );
			if( pSkillInfo == 0 )
			{
				assert(0);
				return false;
			}

			/// 읽어들인 정보를 셋팅한다.
			pSkillInfo->mpSetpInfoArray = new sPlayerSkillStepInfo[stepCount];
			::memcpy( pSkillInfo->mpSetpInfoArray, stepArray, sizeof(sPlayerSkillStepInfo)*stepCount);
			pSkillInfo->mStepCount = stepCount;

			/// 정보 초기화
			stepCount = 0;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
unsigned int cGameResourceManager::GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode )
{
	cPointerHashMap* pModeMap = (cPointerHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pModeMap == NULL )
		return 0;

	sMapInfo* pMapData = (sMapInfo*)pModeMap->GetAt( mapMode );
	if( pMapData == NULL )
		return 0;

	return pMapData->folderIdx;
}

sStageChangePos* cGameResourceManager::GetPosScriptInfo( unsigned long idx )
{
	return (sStageChangePos*)mStageChangePosMap.GetAt( idx );
}

tPointerArray<void*>* cGameResourceManager::GetMapChangeStarterArr( unsigned short mapNum )
{
	mReturnArray.Clear();

	if( mStageChangePosMap.IsEmpty() )
		return &mReturnArray;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();

	while( begin != mStageChangePosMap.End() )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*begin++).mSecond;
		if( pChangePos == 0 )
		{
			assert(0);
			continue;
		}

		if( pChangePos->mMapNumber != mapNum )
			continue;

		/// 검사 기준에서 목적지 제외
		if( pChangePos->mTargetInfo == 0 )
			continue;

		mReturnArray.PushBack( pChangePos );
	}

	return &mReturnArray;
}

///
tPointerArray<void*>* cGameResourceManager::GetMapChangeDestArr( unsigned short mapNum )
{
	mReturnArray.Clear();

	if( mStageChangePosMap.IsEmpty() )
		return &mReturnArray;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();

	while( begin != mStageChangePosMap.End() )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*begin++).mSecond;
		if( pChangePos == 0 )
		{
			assert(0);
			continue;
		}

		if( pChangePos->mMapNumber != mapNum )
			continue;

		/// 검사 기준에서 출발지 제외
		if( pChangePos->mTargetInfo != 0 )
			continue;

		mReturnArray.PushBack( pChangePos );
	}

	return &mReturnArray;
}

unsigned short cGameResourceManager::GetStageChangeMap( unsigned long posIdx )
{
	/// 스크립트상의 시작좌표
	sStageChangePos* pChangePos = (sStageChangePos*)mStageChangePosMap.GetAt( posIdx );
	if( pChangePos == NULL )
	{
		assert(NULL);
		return 0;
	}

	if( pChangePos->mFirstRegen == false )
		return 0;

	return pChangePos->mMapNumber;
}

sMonsterScript* cGameResourceManager::GetMonsterListInfo( unsigned long monsterIdx )
{
	return (sMonsterScript*)mMonsterMap.GetAt( monsterIdx );
}

sPlayerSkillBaseInfo* cGameResourceManager::GetPlayerSkillInfo( unsigned long skillNumber )
{
	sPlayerSkillBaseInfo* pSkillInfo = NULL;

	cPointerHashMap::cIterator start = mPlayerSkillInfoMap.Find( skillNumber );
	cPointerHashMap::cIterator end = mPlayerSkillInfoMap.End();

	if( start == end )	
	{ 
		return NULL; 
	}

	pSkillInfo = (sPlayerSkillBaseInfo*)((*start).mSecond);

	if( pSkillInfo == NULL )
	{
		assert(NULL);
		return NULL;
	}

	return pSkillInfo;
}
