#include "gamesrv.h"

#include "StdAfx.h"
#include "SkillScript.h"

#include "FileLoader.h"
#include "Tokenizer.h"

#include "StatusScript.h"
#include "Parser.h"
#include "Monster_common.h"
#include "Monsterscript.h"

#ifdef _CLIENT
#include "DramaturgyManager.h"
#include "GameResourceManager.h"
#endif


cSkillScript* cSkillScript::mpSkillScript = NULL;


cSkillTreeLexer::cSkillTreeLexer( const char* buffer, unsigned int size )
: cLexer( buffer, size )
{
	BindKeyword( "skillTree", eTOKEN_SKILLTREE );
	BindKeyword( "imageFile", eTOKEN_IMAGEFILE );
	BindKeyword( "jobIdx", eTOKEN_JOBIDX );
	BindKeyword( "skillpos", eTOKEN_SKILLPOS );
}


cSkillScript::cSkillScript(void)
{

	/// 싱글톤
	if( mpSkillScript )
	{
#ifndef _CLIENT
		Verbose->WriteLog( "cSkillScript new error");
#endif
	}
	else
	{
		mpSkillScript = this;
	}

}

bool cSkillScript::Init()
{
	Release();

	/// 스킬 스크립트 로드
	if( !PlayerSkillScriptLoad() )
	{
		assert(NULL);
		return false;
	}

	/// 스킬 스크립트 로드
	if( !MonsterSkillScriptLoad() )
	{
		assert(NULL);
		return false;
	}

	if( !PlayerSkillStepLoad() )
	{
		assert(NULL);
		return false;
	}

	if( !InfluenceLoad() )
	{
		assert(NULL);
		return false;
	}

	if( !ActInfluenctLoad() )
	{
		assert(NULL);
		return false;
	}

	/// 스킬 스크립트 로드
	if( !PlayerSkillJobScriptLoad() )
	{
		assert(NULL);
		return false;
	}

	PlayerFreeSkillList();

	if( LoadSkillTree("./Script/Resource/Path_SkillTree.txt") == false )
	{
		assert(0);
		return false;
	}

	/// 스킬효과 중첩불가 정보 로드
	if( !InfOverlapLoad() )
	{
		assert(NULL);
		return false;
	}

	return true;

}

void cSkillScript::Release()
{
	cSkillHashMap::cIterator iter;

	cPHashMap::cIterator ob = mOverLapMap.Begin();
	cPHashMap::cIterator oe = mOverLapMap.End();
	for( ; ob != oe; ++ob )
	{
		delete (cHashSet*)(ob->mSecond);
	}
	mOverLapMap.Clear();

	cSkillTreeMap::cIterator k = mSkillTreeMap.Begin();
	cSkillTreeMap::cIterator kend = mSkillTreeMap.End();
	for( ; k != kend; ++k )
	{
		delete (cSkillTree*)(k->mSecond);
	}
	mSkillTreeMap.Clear();

	for(iter = mPlayerSkillInfoMap.Begin() ; iter != mPlayerSkillInfoMap.End() ; ++iter)
	{
		delete (sPlayerSkillBaseInfo*)(*iter).mSecond;
	}
	mPlayerSkillInfoMap.Clear();

	/// 몬스터 기본스킬 정보 삭제
	for( iter = mMonsterBaseSkillMap.Begin(); iter != mMonsterBaseSkillMap.End(); ++iter )
	{
		delete (cArray*)iter->mSecond;
	}
	mMonsterBaseSkillMap.Clear();

	cPHashMap::cIterator bb = mBridgeSkillMap.Begin();
	cPHashMap::cIterator be = mBridgeSkillMap.End();
	for( ; bb != be; ++bb )
	{
		delete (cHashMap*)(bb->mSecond);
	}
	mBridgeSkillMap.Clear();

	/// 전체스킬 정보 삭제
	for( iter = mMonsterSkillTotalMap.Begin(); iter != mMonsterSkillTotalMap.End(); ++iter )
	{
		delete (sMonsterSkillScript*)iter->mSecond;
	}
	mMonsterSkillTotalMap.Clear();

	for( iter = mInfluenceMap.Begin() ; iter != mInfluenceMap.End() ; ++iter )
	{
		delete (sInfluenceScript*)(*iter).mSecond;
	}
	mInfluenceMap.Clear();

	for( iter = mActInfluenceMap.Begin() ; iter != mActInfluenceMap.End() ; ++iter )
	{
		delete (sActInfluenceScript*)(*iter).mSecond;
	}
	mActInfluenceMap.Clear();

	for( iter = mSkillJobMap.Begin() ; iter != mSkillJobMap.End() ; ++iter )
	{
		delete (tArray<unsigned short>*)(*iter).mSecond;
	}
	mSkillJobMap.Clear();

	for( iter = mFreeSkillMap.Begin() ; iter != mFreeSkillMap.End() ; ++iter )
	{
		delete (tArray<unsigned long>*)(*iter).mSecond;
	}
	mFreeSkillMap.Clear();
}

void cSkillScript::PlayerFreeSkillList()
{
	cSkillHashMap::cIterator i = mPlayerSkillInfoMap.Begin();
	cSkillHashMap::cIterator iend = mPlayerSkillInfoMap.End();

	/// 지급용 스킬
	for( ; i!=iend; i++ )
	{
		sPlayerSkillBaseInfo* info = (sPlayerSkillBaseInfo*)(*i).mSecond;

		unsigned long idx = (unsigned long)(*i).mFirst;
		if( idx <= 100 )
			continue;

		if( info )
		{
			if( info->mStepCount == 0 )
			{
				assert(0);
				continue;
			}

			if( info->mpSetpInfoArray[0].mRequireSP == 0 )
			{
				tArray<unsigned long>* p = (tArray<unsigned long>*)mFreeSkillMap.GetAt(info->mJobStep);
				if( p == 0 )
				{
					p = new tArray<unsigned long>;
					mFreeSkillMap.Insert( info->mJobStep, p );
				}
				p->PushBack( idx );
			}
		}
	}
	///
}


bool cSkillScript::PlayerSkillScriptLoad()
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

		/// 타겟이 걸린 효과( skilllist_effect에서 사용하는 효과 번호 기입 : 0 사용안함 )
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyValueType = static_cast<unsigned short>(str.ToInt());

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

bool cSkillScript::PlayerSkillStepLoad()
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

		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		stepArray[step].mActiveIdx = str.ToInt();

		if( stepArray[step].mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH && stepArray[step].mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;

		if( stepArray[step].mApplyValueType1 == eSTATUSPLUS_AROUND_PULL && stepArray[step].mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;

		/// 범위 스킬 적용 인원 증가 효과가 있으면서 범위 스킬이 아닌경우 에러
		sPlayerSkillBaseInfo* pSkillInfo = GetPlayerSkillInfo( currentSkillIdx );
		if( pSkillInfo == 0 )
		{
			assert(0);
			return false;
		}

		if( stepArray[step].mApplyValueType1 == eSTATUSPLUS_ADD_APPLYCNT && pSkillInfo->mBoundType == eBOUNDTYPE_NONE )
			return false;
		if( stepArray[step].mApplyValueType2 == eSTATUSPLUS_ADD_APPLYCNT && pSkillInfo->mBoundType == eBOUNDTYPE_NONE )
			return false;

		if( pSkillInfo->mType == eSKILLTYPE_NONSTOP )
		{
			if( pSkillInfo->mShotType == eSHOTTYPE_RUSHRUN || pSkillInfo->mShotType == eSHOTTYPE_RUSHBLINK )
			{
				cHashMap* pStepMap = (cHashMap*)mBridgeSkillMap.GetAt( skillIdx );
				if( pStepMap == NULL )
				{
					pStepMap = new cHashMap;
					if( mBridgeSkillMap.Insert( skillIdx, pStepMap ) == false )
						return false;
				}

				pStepMap->Insert( step, stepArray[step].mInfulenceIdx );
			}
		}

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



bool cSkillScript::InfluenceLoad()
{
	/// 기존 데이터 제거
	cSkillHashMap::cIterator iter;
	for( iter = mInfluenceMap.Begin() ; iter != mInfluenceMap.End() ; ++iter )
	{
		delete (sInfluenceScript*)(*iter).mSecond;
	}
	mInfluenceMap.Clear();

	/// 로드..
	cFileLoader loader;
	cString pathName = "./Script/resource/SkillList_Effect.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SkillList_Effect.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 효과Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long influenceIdx = str.ToInt();

		///연출파일명
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		cString dramaturgyFile = str;

		/// 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long nameidx = str.ToInt();

		/// 효과아이콘Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long iconIdx = str.ToInt();

		/// 툴팁효과Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long toolTipIdx = str.ToInt();

		/// 속성
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char type = static_cast<unsigned char>(str.ToInt());

		/// 도트 속성
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char dotType = (unsigned char)str.ToInt();

		/// 지속도트량
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		long dotValue = (long)str.ToInt();

		/// 분류
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long typeDetail = str.ToInt();

		/// 죽는경우 효과 제거
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char deleteType = (unsigned char)str.ToInt();

		/// 효력 그룹
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long groupIdx = str.ToInt();

		/// 효력
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long groupOrder = str.ToInt();

		/// 중복불가그룹
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long notOverlapGroupIdx = str.ToInt();

		/// 중복불가그룹 스크립트idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long overlapScriptIdx = str.ToInt();

		/// 시간 속성
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char timeType = (unsigned char)str.ToInt();

		/// 지속(채널링)시간
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long continuanceTime = str.ToInt();

		/// 발동확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char activityPer = static_cast<unsigned char>(str.ToInt());

		/// 지속효과번호1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short statusPlusIdx1 = static_cast<unsigned short>(str.ToInt());

		/// 시간1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long time1 = str.ToInt();

		/// 효과값1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		long value1 = str.ToInt();

		/// 지속효과번호2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short statusPlusIdx2 = static_cast<unsigned short>(str.ToInt());

		/// 시간2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long time2 = str.ToInt();

		/// 효과값2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		long value2 = str.ToInt();

		/// 지속효과번호3
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short statusPlusIdx3 = static_cast<unsigned short>(str.ToInt());

		/// 효과값3
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		long value3 = str.ToInt();

		sInfluenceScript* pInfluence = new sInfluenceScript;

		pInfluence->mInfluenceIdx = influenceIdx;
		pInfluence->mDramaturgyFile = dramaturgyFile;
		pInfluence->mNameidx = nameidx;
		pInfluence->mIconIdx = iconIdx;
		pInfluence->mToolTipIdx = toolTipIdx;
		pInfluence->mType = type;
		pInfluence->mDotType = dotType;
		pInfluence->mDotValue = dotValue;
		pInfluence->mTypeDetail = typeDetail;
		pInfluence->mDeleteType = deleteType;			/// 삭제 조건
		pInfluence->mGroupIdx = groupIdx;
		pInfluence->mGroupOrder = groupOrder;
		pInfluence->mNotOverlapGroupIdx = notOverlapGroupIdx;
		pInfluence->mOverlapScriptIdx = overlapScriptIdx;
		pInfluence->mTimeType = timeType;
		pInfluence->mContinuanceTime = continuanceTime;
		pInfluence->mActivityPer = activityPer;
		pInfluence->mStatusPlusIdx1 = statusPlusIdx1;
		pInfluence->mTime1 = time1;
		pInfluence->mValue1 = value1;
		pInfluence->mStatusPlusIdx2 = statusPlusIdx2;
		pInfluence->mTime2 = time2;
		pInfluence->mValue2 = value2;
		pInfluence->mStatusPlusIdx3 = statusPlusIdx3;
		pInfluence->mValue3 = value3;

		if( dotType == eDOTTYPE_NONE )
		{
			switch( statusPlusIdx1 )
			{
			case eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS:
			case eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS:
			case eSTATUSPLUS_DOT_HPCHANGE_CALC_PER:
			case eSTATUSPLUS_DOT_MPCHANGE_CALC_PER:
				{
					if( value1 < 0 )
					{
						assert(NULL);
						return false;
					}
				}
			}
			switch( statusPlusIdx2 )
			{
			case eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS:
			case eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS:
			case eSTATUSPLUS_DOT_HPCHANGE_CALC_PER:
			case eSTATUSPLUS_DOT_MPCHANGE_CALC_PER:
				{
					if( value2 < 0 )
					{
						assert(NULL);
						return false;
					}
				}
			}
		}

		if( groupOrder == 0 && notOverlapGroupIdx == 0 && continuanceTime > 0 )
		{
			assert(NULL);
			return false;
		}				

		if( !mInfluenceMap.Insert( influenceIdx, pInfluence ) )
		{
			assert(0);
			return false;
		}

		if( continuanceTime > 0 && groupIdx == 0 && notOverlapGroupIdx == 0 )
		{
			assert(0);
		}

		if( type == eINFLUENCETYPE_PC )
			mPcBangIdxAry.PushBack( influenceIdx );

		if( type == eINFLUENCETYPE_GMEVENT )
			mGMEventIdxAry.PushBack( influenceIdx );

		// 이런 코드 만들지 않도록 공유 가능한 부분만 Common에 넣으세요
#ifdef _CLIENT
		/// 연출 파일에 대한 로드
		DRAMATURGYMAN->LoadInfluenceDrama( pInfluence->mInfluenceIdx, pInfluence->mDramaturgyFile, false );
#endif
	}

	cSkillHashMap::cIterator i = mInfluenceMap.Begin();
	cSkillHashMap::cIterator e = mInfluenceMap.End();
	for( ; i != e ; ++i )
	{
		sInfluenceScript* pScript = (sInfluenceScript*)(*i).mSecond;
		if( pScript == NULL )
			return false;

		if( pScript->mType != eINFLUENCETYPE_PC && pScript->mType != eINFLUENCETYPE_GMEVENT )
			continue;

		/// pc방용 버프 & GM이벤트 버프를 스킬사용부에 추가한것이 있는지 체크
		cSkillHashMap::cIterator i3 = mPlayerSkillInfoMap.Begin();
		cSkillHashMap::cIterator e3 = mPlayerSkillInfoMap.End();
		for( ; i3 != e3 ; ++i3 )
		{
			sPlayerSkillBaseInfo* pSkill = (sPlayerSkillBaseInfo*)(*i3).mSecond;
			if( pSkill == NULL )
				return false;

			for( unsigned char j = 0 ; j < pSkill->mStepCount ; ++j )
			{
				sPlayerSkillStepInfo* pStep = &pSkill->mpSetpInfoArray[j];
				if( pStep == NULL )
					return false;

				if( pStep->mInfulenceIdx == (*i).mFirst )
					return false;
			}

		}

		/// pc방 버프끼리중첩되는것이 있는지 확인
		cSkillHashMap::cIterator i2 = mInfluenceMap.Begin();
		cSkillHashMap::cIterator e2 = mInfluenceMap.End();
		for( ; i2 != e2 ; ++i2 )
		{
			sInfluenceScript* pScript2 = (sInfluenceScript*)(*i2).mSecond;
			if( pScript2 == NULL )
				return false;

			if( pScript2->mType != eINFLUENCETYPE_PC )
				continue;

			if( pScript->mInfluenceIdx == pScript2->mInfluenceIdx )
				continue;

			if( pScript->mNotOverlapGroupIdx == pScript2->mNotOverlapGroupIdx )
				return false;
		}
	}

	return true;
}

bool cSkillScript::ActInfluenctLoad()
{
	/// 기존 데이터 제거
	cSkillHashMap::cIterator iter;
	for( iter = mActInfluenceMap.Begin() ; iter != mActInfluenceMap.End() ; ++iter )
	{
		delete (sActInfluenceScript*)(*iter).mSecond;
	}
	mActInfluenceMap.Clear();

	/// 로드..
	cFileLoader loader;
	cString pathName = "./script/resource/skilllist_active.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load skilllist_acteffect.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 발동Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long Idx = str.ToInt();

		/// 발동 툴팁
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long tipIdx = str.ToInt();

		///연출파일명
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		cString dramaturgyFile = str;

		/// 발동 상황
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		eACTSTATUS activityState = (eACTSTATUS)str.ToInt();

		/// 발동 확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long activityPer = str.ToInt();

		/// 재사용 대기 시간
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long coolTime = str.ToInt();

		/// 범위 방식
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		eBOUNDTYPE boundType = (eBOUNDTYPE)str.ToInt();
		if( boundType != eBOUNDTYPE_NONE && boundType != eBOUNDTYPE_SELF_TARGET && boundType != eBOUNDTYPE_TARGET )
		{
			assert(0);
		}

		/// 적용 대상
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		eAPPLYTYPE applyType = (eAPPLYTYPE)str.ToInt();
		if( applyType != eAPPLYTYPE_SELF && applyType != eAPPLYTYPE_ENEMY && applyType != eAPPLYTYPE_PARTY )
		{
			assert(0);
		}

		/// 범위 값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float boundDist = str.ToFloat();

		/// 참조Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long infIdx = str.ToInt();

		sActInfluenceScript* pInfo = new sActInfluenceScript;
		pInfo->mActivityIdx		= Idx;
		pInfo->mTooltipIdx		= tipIdx;
		pInfo->mDramaturgyFile	= dramaturgyFile;
		pInfo->mActivityState	= activityState;
		pInfo->mActivityPer		= activityPer;
		pInfo->mCooltime		= coolTime;
		pInfo->mApplyType		= applyType;
		pInfo->mBoundType		= boundType;
		pInfo->mBoundDist		= boundDist;
		pInfo->mInfIdx			= infIdx;

		if( !mActInfluenceMap.Insert( Idx, pInfo ) )
		{
			assert(0);
			return false;
		}

		// 이런 코드 만들지 않도록 공유 가능한 부분만 Common에 넣으세요
#ifdef _CLIENT
		/// 연출 파일에 대한 로드
		DRAMATURGYMAN->LoadActInfluenceDrama( pInfo->mActivityIdx, pInfo->mDramaturgyFile, false );
#endif
	}

	return true;
}


bool cSkillScript::PlayerSkillJobScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/SkillList_Class.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SkillList_Class.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	tArray<unsigned long>	*pJobArry;

	while( tokenizer.IsEnd() == false )
	{
		/// 직업Idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long jobIdx = str.ToInt();

		/// 직업코드1
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_JOB jobCode1 = static_cast<ePLAYER_JOB>(str.ToInt());

		/// 직업코드2
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_JOB jobCode2 = static_cast<ePLAYER_JOB>(str.ToInt());

		/// 직업코드3
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_JOB jobCode3 = static_cast<ePLAYER_JOB>(str.ToInt());

		pJobArry = new tArray<unsigned long>;

		/// 배열에 값을 넣음
		if( jobCode1 != 0 )	pJobArry->PushBack( static_cast<unsigned long>(jobCode1) );
		if( jobCode2 != 0 )	pJobArry->PushBack( static_cast<unsigned long>(jobCode2) );
		if( jobCode3 != 0 )	pJobArry->PushBack( static_cast<unsigned long>(jobCode3) );

		if( !mSkillJobMap.Insert( jobIdx, pJobArry ) )
		{
			return false;
		}


	}

	return true;
}



bool cSkillScript::MonsterSkillScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/SkillList_Mon.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SkillList_Mon.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMonsterSkillScript* pSkillInfo;

	while( tokenizer.IsEnd() == false )
	{
		pSkillInfo = new sMonsterSkillScript;

		/// 몬스터Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mMonsterIdx = str.ToInt();

		/// 공격방식
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mAttackType = (eMONSTERATTACK_TYPE)str.ToInt();

		/// Drama file
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mDramaFile = str;

		/// 툴팁Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mToolTipIdx = str.ToInt();

		/// 스킬 분류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mType = (eMONSTERSKILLTYPE)str.ToInt();

		/// 속성유형
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mAttributeType = (eATTRIBUTETYPE)str.ToInt();

		/// 거리유형
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mRangeType = (eRANGETYPE)str.ToInt();

		/// 공격력(소)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mMinAttackValue = str.ToInt();

		/// 공격력(대)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mMaxAttackValue = str.ToInt();

		/// 고유명중률
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mAccuracyValue = (short)str.ToInt();

		/// 고유크리티컬
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mCriticalValue = (unsigned short)str.ToInt();

		/// 사용조건(HP)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mUseHPCheck = (eMONSTERHPCHECK)str.ToInt();

		/// 사용조건(소모MP)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mUseMP = str.ToInt();

		/// 사거리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mTargetDist = (float)str.ToInt();

		/// 공격속도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mAttackSpd = str.ToInt();

		/// 쿨타임
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mCoolTime = str.ToInt();

		/// 사용확률
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mSkillUsePer = (unsigned char)str.ToInt();

		/// 시전유형
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mShotType = (eMONSTERSHOTTYPE)str.ToInt();

		/// 범위방식
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mBoundType = (eBOUNDTYPE)str.ToInt();

		/// 범위거리1
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mBoundDist1 = (unsigned short)str.ToInt();

		/// 범위거리2
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mBoundDist2 = (unsigned short)str.ToInt();

		/// 적용인원
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyCnt = (unsigned char)str.ToInt();

		/// 효과적용타겟
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyType = (eAPPLYTYPE)str.ToInt();

		if( pSkillInfo->mApplyType != eAPPLYTYPE_SELF  && 
			pSkillInfo->mApplyType != eAPPLYTYPE_ENEMY && 
			pSkillInfo->mApplyType != eAPPLYTYPE_BUDDY &&
			pSkillInfo->mApplyType != eAPPLYTYPE_FAR   &&
			pSkillInfo->mApplyType != eAPPLYTYPE_RANDOM      &&		
			pSkillInfo->mApplyType != eAPPLYTYPE_DISTRESS2   &&
			pSkillInfo->mApplyType != eAPPLYTYPE_DISTRESS3   &&	
			pSkillInfo->mApplyType != eAPPLYTYPE_DISTRESSMIN && 	
			pSkillInfo->mApplyType != eAPPLYTYPE_HPMAX &&	
			pSkillInfo->mApplyType != eAPPLYTYPE_HPMIN )
			goto ERR;

		/// 캐스팅시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mCastingTime = str.ToInt();

		/// 효과번호1
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyValueType1 = (unsigned short)str.ToInt();

		if( pSkillInfo->mApplyValueType1 != 0 &&
			( pSkillInfo->mApplyValueType1 < eSTATUSPLUS_INSTANTPERHIT ||
			pSkillInfo->mApplyValueType1 > eSTATUSPLUS_DAMAGECALC_MAX ) )
			goto ERR;

		/// 효과값1
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyValue1 = str.ToInt();

		/// 효과번호2
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyValueType2 = (unsigned short)str.ToInt();

		if( pSkillInfo->mApplyValueType2 != 0 &&
			( pSkillInfo->mApplyValueType2 < eSTATUSPLUS_INSTANTPERHIT ||
			pSkillInfo->mApplyValueType2 >= eSTATUSPLUS_DAMAGECALC_MAX ))
			goto ERR;

		/// 효과값2
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mApplyValue2 = str.ToInt();

		if( pSkillInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH && pSkillInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			goto ERR;

		if( pSkillInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL && pSkillInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			goto ERR;

		/// 범위 스킬 적용 인원 증가 효과가 있으면서 범위 스킬이 아닌경우 에러
		if( pSkillInfo->mApplyValueType1 == eSTATUSPLUS_ADD_APPLYCNT && pSkillInfo->mBoundType == eBOUNDTYPE_NONE )
			goto ERR;
		if( pSkillInfo->mApplyValueType2 == eSTATUSPLUS_ADD_APPLYCNT && pSkillInfo->mBoundType == eBOUNDTYPE_NONE )
			goto ERR;

		/// 효과Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mInfulenceIdx = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pSkillInfo->mActiveIdx = str.ToInt();

		/// 몬스터 스킬 인덱스 생성 - 몬스터클래스인덱스 * 100 + 공격타입
		unsigned long monSkillIdx = pSkillInfo->mMonsterIdx * 100 + pSkillInfo->mAttackType;

		/// 전체 스킬 리스트에 담는다.
		if( mMonsterSkillTotalMap.Insert( monSkillIdx, pSkillInfo ) == false )
		{
			assert(0);
			return false;
		}

		
		if( pSkillInfo->mAttackType < eMONSTERATTACK_MAX )
		{
			/// 기본 스킬 5개에대한 정보를 담는다.
			cArray* pMonsterSkillAry = (cArray*)mMonsterBaseSkillMap.GetAt( pSkillInfo->mMonsterIdx );
			if( pMonsterSkillAry == NULL )
			{
				pMonsterSkillAry = new cArray;
				if( mMonsterBaseSkillMap.Insert( pSkillInfo->mMonsterIdx, pMonsterSkillAry ) == false)
				{
					assert(NULL);
					return false;
				}
			}
			pMonsterSkillAry->PushBack( pSkillInfo );
		}

		/// drama 정보 생성부

#ifndef _CLIENT
		if( MONSTERSCRIPT != NULL )
		{
			sMonsterScript* pMon = MONSTERSCRIPT->GetMonsterListInfo( pSkillInfo->mMonsterIdx );
			if( pMon != NULL )
			{
				if( FloatToInt( pMon->mSkillMinRange ) == 0 )
				{
					pMon->mSkillMinRange = pSkillInfo->mTargetDist;
					pMon->mSkillMinRangeType = (unsigned char)pSkillInfo->mRangeType;
				}
				else
				{
					if( pMon->mSkillMinRange > pSkillInfo->mTargetDist )
					{
						pMon->mSkillMinRange = pSkillInfo->mTargetDist;
						pMon->mSkillMinRangeType = (unsigned char)pSkillInfo->mRangeType;
					}
				}
			}
			else
				assert(NULL);
		}
#endif
	}

	return true;

ERR:
	delete pSkillInfo;
	return false;
}



sPlayerSkillBaseInfo* cSkillScript::GetPlayerSkillInfo( unsigned long skillNumber )
{
	sPlayerSkillBaseInfo* pSkillInfo = NULL;

	cSkillHashMap::cIterator start = mPlayerSkillInfoMap.Find( skillNumber );
	cSkillHashMap::cIterator end = mPlayerSkillInfoMap.End();

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

sMonsterSkillScript* cSkillScript::GetMonsterSkillInfo( unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType )
{
	unsigned long skillIdx = monsterClassIdx*100 + attackType;

	return (sMonsterSkillScript*)mMonsterSkillTotalMap.GetAt( skillIdx );
}

cSkillHashMap* cSkillScript::GetMonsterSkillTotalMap()
{
	return &mMonsterSkillTotalMap;
}

void* cSkillScript::GetMonsterBaseSkillArr( unsigned long monsterClassIdx )
{
	return mMonsterBaseSkillMap.GetAt( monsterClassIdx );
}


tArray<unsigned long>* cSkillScript::GetPlayerSkillJob( unsigned long jobIdx )
{
	return (tArray<unsigned long>*)mSkillJobMap.GetAt( jobIdx );
}



sInfluenceScript* cSkillScript::GetInfluenceInfo( unsigned long influenceClassIdx )
{	
	return (sInfluenceScript*)mInfluenceMap.GetAt( influenceClassIdx );
}


sActInfluenceScript* cSkillScript::GetActInfluenceInfo( unsigned long actIdx )
{
	return (sActInfluenceScript*)mActInfluenceMap.GetAt( actIdx );
}


bool cSkillScript::IsPlayerSkillJob( unsigned long skillJobIdx, unsigned long playerJob )
{
	tArray<unsigned long>* pSkillJobArray = GetPlayerSkillJob( skillJobIdx );
	if( pSkillJobArray == 0 )
		return false;

	for( unsigned int i = 0 ; i < pSkillJobArray->GetSize() ; ++i )
	{
		if( (*pSkillJobArray)[i] == playerJob )
			return true;
	}

#ifdef _CLIENT
	tArray<unsigned long>* playerJobParent = GAMERESOURCEMAN->GetParentJob( playerJob );
#else
	tArray<unsigned long>* playerJobParent = STATUSSCRIPT->GetParentJob( playerJob );
#endif

	if( playerJobParent )
	{
		/// 모든 상위 직업 검사
		for( unsigned int j = 0 ; j < playerJobParent->GetSize() ; ++j )
		{
			for( unsigned int i = 0 ; i < pSkillJobArray->GetSize() ; ++i )
			{
				if( (*pSkillJobArray)[i] == (*playerJobParent)[j] )
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool cSkillScript::IsCastSkill( unsigned long skillIdx, unsigned char /*skillStep*/ )
{
	sPlayerSkillBaseInfo* pInfo = GetPlayerSkillInfo( skillIdx );
	assert(pInfo);
	if( pInfo )
	{
		if( pInfo->mShotType == eSHOTTYPE_CASTING )
			return true;
	}

	return false;
}

bool cSkillScript::LoadSkillTree( const cString& pathName )
{
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
		return false;

	/// 렉서에 밥주고 파싱
	cToken token;
	cSkillTreeLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case cSkillTreeLexer::eTOKEN_SKILLTREE:
			{
				unsigned long jobIdx = (unsigned long)parser.ParseInt();
				cSkillTree* s = new cSkillTree;

				if( LoadTree( parser, s) == false )
				{
					delete s;
					return false;
				}
				if( mSkillTreeMap.Insert(jobIdx, s) == false )
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

bool cSkillScript::LoadTree( cParser& parser, cSkillTree* pTree )
{
	if( pTree == 0 )
		return false;

	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	cSkillTree::sTreeIcon buffer[100] = {0,};
	unsigned int iconCount = 0;
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
		case cSkillTreeLexer::eTOKEN_IMAGEFILE:
			{
				pTree->mImageName = parser.ParseString();
			}
			break;
		case cSkillTreeLexer::eTOKEN_SKILLPOS:
			{
				buffer[iconCount].skillIdx = parser.ParseInt();

				buffer[iconCount].uiPosX = parser.ParseInt();
				buffer[iconCount].uiPosY = parser.ParseInt();

				iconCount++;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	/// 임시로 담아놓은 icon들을 담는다.
	if( iconCount )
	{
		pTree->mIconCount = iconCount;
		pTree->mIconList = new cSkillTree::sTreeIcon[iconCount];
		::memcpy( pTree->mIconList, buffer, sizeof(cSkillTree::sTreeIcon)*iconCount );
	}
	return true;
}

bool cSkillScript::InfOverlapLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/skilllist_overlap.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load skilllist_overlap.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sPlayerSkillBaseInfo* pSkillInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		/// 중복불가고유idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned long overlapScriptIdx = str.ToInt();

		cHashSet* pSet = (cHashSet*)mOverLapMap.GetAt( overlapScriptIdx );
		if( pSet == NULL )
		{
			pSet = new cHashSet;
			if( mOverLapMap.Insert( overlapScriptIdx, pSet ) == false )
				return false;
		}

		/// 스킬 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned long notOverlapGroupIdx = str.ToInt();

		pSet->Insert( notOverlapGroupIdx );
		
	}
	return true;

ERR:
	delete pSkillInfo;
	return false;
}

bool cSkillScript::IsInfOverLap( unsigned long overlapScriptIdx, unsigned long notOverLapIdx )
{
	cHashSet* pSet = (cHashSet*)mOverLapMap.GetAt( overlapScriptIdx );
	if( pSet == NULL )
		return false;

	cHashSet::cIterator f = pSet->Find( notOverLapIdx );
	cHashSet::cIterator e = pSet->End();

	if( f == e )
		return false;

	return true;
}


unsigned long cSkillScript::GetBridgeSkillMap( unsigned long skillIdx, unsigned long skillStep )
{
	cHashMap* pStepMap = (cHashMap*)mBridgeSkillMap.GetAt( skillIdx );
	if( pStepMap == NULL )
		return 0;

	unsigned long nextBridge = 0;
	
	pStepMap->GetAt( &nextBridge, skillStep );
	return nextBridge;
}
