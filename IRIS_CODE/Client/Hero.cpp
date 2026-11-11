#include "stdafx.h"
#include "hero.h"

#include "PlayerSceneNode.h"
#include "TransMonSceneNode.h"

#include "InputSystem.h"
#include "ShortcutManager.h"

#include "SceneManager.h"
#include "Application.h"

#include "ObjectManager.h"
#include "GameResourceManager.h"
#include "WorldManager.h"
#include "SkillExecuter.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "StageScript.h"
#include "MonsterScript.h"
#include "GatheringScript.h"

#include "CameraManager.h"
#include "LookAtCamera.h"
#include "CommonNetworkMsgStruct.h"
#include "Protocol.h"
#include "Pathfinder.h"
#include "RangeCheck.h"
#include "StatusScript.h"
#include "TutorialManager.h"

#include "Item_Common.h"
#include "Duel_Common.h"
#include "Gathering_Common.h"
#include "TeleportPortal_Common.h"


#include "CastingBar.h"
#include "HeroInfoWindow.h"
#include "HeroStateWindow.h"
#include "TargetWindow.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "FontAgent.h"

#include "skillwindow.h"
#include "BuffWindow.h"
#include "RevivalWindow.h"

#include "PVPManager.h"
#include "DuelManager.h"
#include "DuelWindow.h"

#include "GameUIManager.h"
#include "UIManager.h"
#include "PartyManager.h"
#include "DamagePrintManager.h"
#include "DramaturgyManager.h"
#include "QuestManager.h"
#include "TitleManager.h"
#include "ItemMixManager.h"
#include "PartyUnionManager.h"
#include "TarotManager.h"

#include "Npc.h"
#include "Monster.h"
#include "MonsterSceneNode.h"
#include "VehicleSceneNode.h"
#include "MapPortalSceneNode.h"
#include "ChatManager.h"

#include "Tarot_Common.h"
#include "ItemManager.h"
#include "QuickManager.h"

#include "uicursor.h"
//
#include "TrailGeometry.h"
#include "UIMsgboxEvent.h"
#include "UIRootNode.h"
#include "NpcScript.h"

#include "DuelMsgWindow.h"
#include "EffectSceneNode.h"
#include "MoveMsgWindow.h"

#ifdef _GMTOOL
/// 치트
#include "CheatManager.h"
#include "GMTool.h"
#endif

cPlayerSceneNodeParam  gTempParam;
cMonsterSceneNodeParam gTempParamMon;

cHero::cHero( unsigned char type )
: cPlayer(type)
{
	mDefaultCameraOffset = 0.0f;
	mCameraOffset = 0.0f;

	/// 길찾기
	mPathFinder = NULL;
	mPathIndex = 0;
	mPathCount = 0;

	::memset(&mHeroInfo, 0, sizeof(sHeroInfo));

	mIsCalcStatus = false;
	mIsUdateHeroInfo = false;

	/// default로 기본공격
	mWaitSkill.skillIdx = 0;
	mWaitSkill.skillStep = 0;

	mWaitWeaponSwitch = false;

	/// 공격중 이동 대기 (Attack상태에서만 유효하다)
	mWaitMoveFlag = eWAITMOVE_NONE;

	mAfterAction.mActionType = eAfterAction_None;

	mFollowPlayerIndex = 0;
	mIsFollowing = false;
	mFollowUpdateTime = 0;

	InitStatusInfoType();

	mMageJobType = false;

	mMoney = 0;
	mDeposit = 0;
	mArmorSets  = ARMOR_SETS_NONE;

	mNpcIdx = 0;
	mNpcFuncType = NPC_FUNC_MAX;

	mPeaceToggleTime = 0;
	mPeaceToggleFlag = true;

	mSwitchWeaponTime = 0;
	mSwitchWeaponFlag = false;

	mMoveTargetEffect = 0;
	mMoveTargetEffectIdx = (unsigned long)-1;

	mJobUsedSkillPoint.Resize( 5 );

	mDamageAfterAnim = ANITYPE_IDLE;

	/// key move
	mIsKeyMove = false;

	mGoCountinue = false;

	mSumKeyMoveDeltaTime = 0;

	mIsFindPosition = false;
	mFindPos = NiPoint2::ZERO;
	mFindPosCnt = 0;

	mMapNum = 0;
	mMapMode = 0;
	mChannelNum = 0;
	mAreaNameIdx = 0;

	mWaitMovePos = NiPoint2::ZERO;

	mCantMove = false;
	mCantSkillPhy = false;
	mCantSkillMag = false;
	mIsSleep = false;
	mIsStun = false;
	mIsAvoidMonLook = false;

	mCheckSendTime = 0;

	mMapPortalSkipTime = 0;

	mAutoAttackFlag = true;

	mPartyRoomIndex = 0;

	mOriMaxMP = 0;

	mRushSkill = 0;
	mRushStep = 0;
	mRushBlink = false;

}

cHero::~cHero()
{
	mMoveTargetEffect = 0;
	mMoveTargetEffectIdx = (unsigned long)-1;

	ClearTabTargetList();

	mJobUsedSkillPoint.Clear();
}

bool cHero::Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, 
				   sPlayerExrInfo* pExrInfo, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	::memcpy( &mPlayerInfo, pBaseInfo, sizeof(sPlayerInfo) );
	::memcpy( &mPlayerExrInfo, pExrInfo, sizeof(sPlayerExrInfo) );
	::memcpy( &mPlayerWearInfo, pWearInfo, sizeof(sPlayerWearInfo) );
	::memcpy( &mPlayerWeaponInfo, pWeaponInfo, sizeof(sPlayerWeaponInfo) );

	gTempParam.mpObject = gTempParamMon.mpObject = this;
	gTempParam.mTranslate = gTempParamMon.mTranslate = sPos;
	gTempParam.mRotate = gTempParamMon.mRotate = sRot;
	gTempParam.mScale = gTempParamMon.mScale = fScale;

	mCantMove = false;
	mCantSkillPhy = false;
	mCantSkillMag = false;
	mIsSleep = false;
	mIsStun = false;

	mGotoX = sPos.x;
	mGotoY = sPos.y;

	///
	mDelay = 0;
	mAngle = 0.0f;
	mAccumAngle = 0.0f;
	mAxis = NiPoint3::UNIT_Z;

	mMoveTargetEffect = 0;
	mMoveTargetEffectIdx = (unsigned long)-1;

	mRushSkill = 0;
	mRushStep = 0;
	mRushBlink = false;
	//	mMoveTargetList.Clear();

	return true;
}

void cHero::SetHeroInfoNet( MSGROOT* pInfo )
{
	assert(pInfo);

	MSG_HEROINFO* pHInfo = (MSG_HEROINFO*)pInfo;

	memcpy( HERO->GetHeroInfo(), &pHInfo->mHeroInfo, sizeof(sHeroInfo) );
	memcpy( HERO->GetHeroExrInfo(), &pHInfo->mExrInfo, sizeof(sPlayerExrInfo) );

	mCantMove = pHInfo->mCanTMove;
	mCantSkillPhy = pHInfo->mCanTSkillPhy;
	mCantSkillMag = pHInfo->mCanTSkillMag;
	mIsSleep = pHInfo->mSleep;
	mIsStun = pHInfo->mStun;
	mPlayerExrInfo.mChgMonsterIdx = pHInfo->mChgMonsterIdx;
	mIsAvoidMonLook = pHInfo->mAvoidMonLook;

	if( pHInfo->mExrInfo.mChgMonsterIdx == 0 )
	{
		const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
		gTempParam.mPathName = strFile;
	}
	else
	{
		/// 외형 몬스터화
		sMonsterScript* pMonsterInfo;
		pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		if( pMonsterInfo == NULL )
		{
			assert(0);
			return;
		}

		float scale = 1.0f;
		if( pMonsterInfo->mMonsterScale == 0.0f )
			scale = pMonsterInfo->mMonsterScale;

		const char* strFile = GAMERESOURCEMAN->GetModelFileName( pMonsterInfo->mKfmIdx );

		cString path;
		path.Format( "./Data/Monster/%s", strFile );

		gTempParamMon.mPathName = path;
		gTempParamMon.mScale = scale;
	}
}


bool cHero::GameIn( unsigned long channelNum )
{
	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		if( WORLDMAN->CalcHeight( &gTempParam.mTranslate.z, gTempParam.mTranslate.x, gTempParam.mTranslate.y ) == false )
			assert(NULL);

		mpObjectSceneNode = CreateSceneNode( &gTempParam );
		if( mpObjectSceneNode == 0 )
		{
			assert(0);
			return false;
		}

		NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
		if( as )
			mpObjectSceneNode->GetNiNode()->AttachChild( as->GetListener() );

		/// head 정보 셋팅
		ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
		SetBaseFace();

		if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
			ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC], true );
		else
			ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE], true );

		/// 코스튬 우선
		for( unsigned int ui=eWEAR_BODY1; ui<eWEAR_HAT; ui++ )
		{
			if( mPlayerWearInfo.CostumeIdx[ui] == 0 )
			{
				if( mPlayerWearInfo.WearIdx[ui] == 0 )
					ChangeBaseWear( ePART_BODY1 + ui );
				else
					EquipWearItem( mPlayerWearInfo.WearIdx[ui] );
			}
			else
			{
				EquipWearItem( mPlayerWearInfo.CostumeIdx[ui] );

				/// 원피스인 경우 배제..
				if( ui == eWEAR_BODY1 )
				{
					cItemDefine* p = ITEMMAN->GetItemDefine( mPlayerWearInfo.CostumeIdx[ui] );
					if( p && p->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
						break;
				}
			}
		}

		/// eWEAR_HAT
		if( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] == 0 )
		{
			if( mPlayerWearInfo.WearIdx[eWEAR_HAT] != 0 )
				EquipWearItem( mPlayerWearInfo.WearIdx[eWEAR_HAT] );
		}
		else
		{
			EquipWearItem( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] );
		}
	
		/// 착용 아이템 설정
		/// 초기화 구문이므로 직접 제어한다..
		EquipWeaponItem( &mPlayerWeaponInfo );

		for( unsigned char i=eCOSTUME_HEAD; i<eCOSTUME_MAX; i++ )
			EquipAccessoryItem( i, mPlayerWearInfo.CostumeAccIdx[i] );
	}
	else
	{
		if( WORLDMAN->CalcHeight( &gTempParamMon.mTranslate.z, gTempParamMon.mTranslate.x, gTempParamMon.mTranslate.y ) == false )
			assert(NULL);

		mpObjectSceneNode = SCENEMAN->CreateTransMonster( gTempParamMon, true );
		if( mpObjectSceneNode == 0 )
		{
			assert(0);
			return false;
		}

		NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
		if( as )
			mpObjectSceneNode->GetNiNode()->AttachChild( as->GetListener() );

		//sMonsterScript* pMonsterInfo;
		//pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		//if( pMonsterInfo == NULL )
		//{
		//	assert(0);
		//	return false;
		//}

		//float monsterSize = 1.0f;
		//if( pMonsterInfo->mMonsterFixSize != 0.0f )
		//	monsterSize = pMonsterInfo->mMonsterFixSize;


		SKILLEXECUTER->SetMonsterSkillInfo( mPlayerExrInfo.mChgMonsterIdx );
	}

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, true );

	ShowNameGauge( true );

	mChannelNum = channelNum;

	SetState( eOBJECT_STATE_IDLE );
	mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;
	mPlayerExrInfo.mStateStop = eSTOP_NONE;

	/// Hero 가지고 다니는 카메라 설정.. 
	CAMERAMAN->ChangeCamera( eCamera_Hero );
	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	if( pCamera == 0 )
	{
		assert(0);
	}

	NiPoint3 sCenter = mpObjectSceneNode->GetObjectCenter();
	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
	mDefaultCameraOffset = sCenter.z - sFoot.z +22.0f;
	mCameraOffset = mDefaultCameraOffset;

	pCamera->SetLookAt( NiPoint3( sFoot.x, sFoot.y, sFoot.z + mCameraOffset ) );

	//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
	//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetWorldScale();

	///
	mDesiredDir = mpObjectSceneNode->GetRotate()*mDesiredDir;
	mDesiredDir.Unitize();

	pCamera->ViewHeroFront( -mDesiredDir );

	/// 길찾기 셋팅
	SetPathFinder( WORLDMAN->GetPathFinder() );

	/// 스탯 계산..
	STATUSCALC->CalcPlayerInit();

	mpObjectSceneNode->UpdateNameCardTextValue();

	/// 스킬창 셋팅
	cSkillWindow* skillWindow = GAMEUI->GetSkillWindow();
	if( skillWindow )
	{
		tArray<unsigned long>* parray = GAMERESOURCEMAN->GetParentJob( mPlayerInfo.Job );
		if( parray )
		{
			assert( parray->IsEmpty() == false );

			if( parray->IsEmpty() == false )
			{
				for( unsigned int i=0;i<parray->GetSize();++i)
					skillWindow->AddJobTab( (*parray)[i] );
			}
		}

		skillWindow->AddJobTab( mPlayerInfo.Job );
	}

	/// 스킬 정보 세팅
	SKILLMAN->UpdateAllSkill();

	// 워록/메이션 기본 공격 스킬이 원거리로 바뀌어서 자동 공격하도록 수정함 [4/16/2010 Jo_Client]
//	mMageJobType = GAMERESOURCEMAN->IsCorrelateMageJob( mPlayerInfo.Job );
	mMageJobType = false;

	mIsCalcStatus = true;
	mIsUdateHeroInfo = true;
	GAMEUI->UpdateSkillPoint();

	/// 
	ITEMMAN->CheckEquipAbility();

	/// 착용 방어구셋 갱신
	ResetArmorSets();

	/// hero의 정보 초기화
	GAMEUI->InitHeroInfo();

	/// 전체 아이템 무게 계산
	ITEMMAN->SetMaxWeightGauge( (float)GetMaxWeight() );

	/// 전체 아이템 쿨타임 세팅 
	ITEMMAN->SetAllItemCool();

	// 제한 체크
	ITEMMAN->CheckItemLimitAll();

	/// 호칭 정보 세팅
	TITLEMAN->Open();

	/// Vehicle Check
	if( mPlayerExrInfo.mVehicleIdx != 0 )
	{
		if( mPlayerExrInfo.mChgMonsterIdx == 0 )
		{
			RideVehicle( mPlayerExrInfo.mVehicleIdx );
		}
		else
		{
			assert(0);
		}
	}

	/// 전장 세력 이펙트
	//PVPMAN->LinkPvpForceEffect( this, mPlayerExrInfo.mForceType );

	SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_LogIn_01.nif", 0 );

	mAfterAction.mActionType = eAfterAction_None;
	mWaitMoveFlag = eWAITMOVE_NONE;

	mWaitSkill.skillIdx = 0;
	mWaitSkill.skillStep = 0;

	mGotoX = GetXPos();
	mGotoY = GetYPos();

	return true;
}

void cHero::ReCreate( char race, char gender )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return;
	}

	if( mpObjectSceneNode->GetTargetAnimation() != 0 )
		return;

	cPlayer::ReCreate( race, gender );

	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();

	NiPoint3 sCenter = mpObjectSceneNode->GetObjectCenter();
	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
	mDefaultCameraOffset = sCenter.z - sFoot.z +22.0f;
	mCameraOffset = mDefaultCameraOffset;

	pCamera->SetLookAt( NiPoint3( sFoot.x, sFoot.y, sFoot.z + mCameraOffset ) );
}

void cHero::SetState( unsigned char state, bool bShowAni )
{
	mIdleTime = 0;

	/// 죽은 경우 무조건 다음 동작으로 대기가 와야 한다.
	if( mPlayerExrInfo.mState == eOBJECT_STATE_DIE )
	{
		if( state != eOBJECT_STATE_IDLE )
		{
			assert(0);
			return;
		}

		cRevivalWindow* win = GAMEUI->GetReVivalWindow();
		if( win && win->IsVisible() )
			return;

		SetReadyUseSkill( true );

		mAfterAction.mActionType = eAfterAction_None;
		mWaitMoveFlag = eWAITMOVE_NONE;

		mWaitSkill.skillIdx = 0;
		mWaitSkill.skillStep = 0;
	}

	if( mPlayerExrInfo.mState == eOBJECT_STATE_MOVE )
	{
		if( state != eOBJECT_STATE_MOVE )
		{
			if( mMoveTargetEffectIdx != (unsigned long)-1 )
			{
				cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mMoveTargetEffectIdx );
				if( node )
				{
					assert( node == mMoveTargetEffect );
					node->Remove();
				}
				mMoveTargetEffect = 0;
				mMoveTargetEffectIdx = (unsigned long)-1;
			}
			mIsKeyMove = false;
		}
	}


	if( state == ePLAYER_STATE_ITEMPICK || state == ePLAYER_STATE_GATHERING )
	{
		CURSOR->SetCursor( eCURSOR_DEFAULT );
	}

	bool isRecoveryUpdate = false;
	if( mPlayerExrInfo.mState ==  ePLAYER_STATE_SITDOWN && state != ePLAYER_STATE_SITDOWN )
	{
		isRecoveryUpdate = true;
		mSitDownFlag = false;

		if( mSitDownEffectIdx != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mSitDownEffectIdx );
			if( node )
			{
				assert( node == mSitDownEffect );
				node->Remove();
			}
			mSitDownEffect = 0;
			mSitDownEffectIdx = (unsigned long)-1;
		}
	}

	if( state != eOBJECT_STATE_STOP )
		mPlayerExrInfo.mStateStop = eSTOP_NONE;

	if( state == eOBJECT_STATE_IDLE )
	{
		SetReadyUseSkill( true );

		mAfterAction.mActionType = eAfterAction_None;
		mWaitMoveFlag = eWAITMOVE_NONE;
		mWaitSkill.skillIdx = 0;
		mWaitSkill.skillStep = 0;

		mIsKeyMove = false;
		mGoCountinue = false;

		mSumKeyMoveDeltaTime = 0;

		if( mIsFindPosition == true )
			DeActivePosition();

	}
	else if( state == eOBJECT_STATE_ATTACK )
	{
		ShowWeapon( true );
		mPlayerExrInfo.mState = (unsigned char)state;
		SetIdleFlag(eIDLE_NORMAL);

		mIsKeyMove = false;
		mGoCountinue = false;

		ClearMoveFlag();

		return;
	}
	else if( state == ePLAYER_STATE_RUSH )
	{
		ShowWeapon( true );
		mPlayerExrInfo.mState = (unsigned char)state;
		SetIdleFlag(eIDLE_NORMAL);

		mIsKeyMove = false;
		mGoCountinue = false;

		ClearMoveFlag();

		ChangeAnimation( ANITYPE_RUN );

		return;
	}
	else if( state == eOBJECT_STATE_DIE )
	{
		SetReadyUseSkill( true );

		/// 죽기전에 상태에 따른 UI 닫기
		GAMEUI->HideUIByDie();

		/// 사망시 보유 효과 제거..
		ClearAllBuff( false );
		ClearAllDeBuff( false );

		mCantMove = false;
		mCantSkillPhy = false;
		mCantSkillMag = false;
		mIsStun = false;
		mIsSleep = false;

		ClearItemMixEffect();
		ClearItemEnhanceEffect();

		mIsKeyMove = false;
		mGoCountinue = false;

		SetJumpFlag( false );

		ClearMoveFlag();
	}
	else if( state == eOBJECT_STATE_PUSHPULL )
	{
		SetReadyUseSkill( true );

		GAMEUI->HideUIByPushPull();

		ClearItemMixEffect();
		ClearItemEnhanceEffect();

		mIsKeyMove = false;
		mGoCountinue = false;

		SetJumpFlag( false );

		ClearMoveFlag();
	}
	else if( state == eOBJECT_STATE_STOP )
	{

		mIsKeyMove = false;
		mGoCountinue = false;

		ClearMoveFlag();
	}

	cPlayer::SetState( state, bShowAni );

	if( isRecoveryUpdate == true )
		GAMEUI->UpdateRecovery();
}

void cHero::SetTargetObject( unsigned char type, unsigned long idx )
{
	if( mTarget.type == type && mTarget.index == idx )
		return;

	if( type == eOBJECTTYPE_NONE )
		ClearTabTargetList();

	/// 기존 타겟
	unsigned char oldType = 0;
	unsigned long oldIndex = 0;
	cBaseObject* obj = GetTargetObject();
	if( obj )
	{
		obj->SetOverFlag( false );
		OBJECTMAN->ResetOverObject();

		oldType = obj->GetObjectType();
		oldIndex = obj->GetObjectID();
	}

	cBaseObject::SetTargetObject( type, idx );

	//
	TUTORIALMAN->UpdateTarget( oldType, oldIndex, type, idx );

	/// 새 타겟
	cTargetWindow* targetWindow = GAMEUI->GetTargetWindow();
	obj = GetTargetObject();
	if( obj )
	{		
		obj->SetOverFlag( true );

		/// 타겟창 세팅
		if( targetWindow )
		{
			targetWindow->ClearBuff();

			/// 타겟의 버프 세팅
			typedef tPointerHashMap<unsigned long, void*> cTargetBuffMap;
			cTargetBuffMap* buffMap = obj->GetBufMap();
			if( buffMap )
			{
				if( buffMap->GetSize() > 0 )
				{
					cTargetBuffMap::cIterator i = buffMap->Begin();
					cTargetBuffMap::cIterator end = buffMap->End();
					for( ; i != end; ++i )
					{
						cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
						if( p )
						{
							/// 효과는 스킬인덱스가 효과인덱스로 쓰임.
							/// 버프/디버프 내부 구별
							targetWindow->AddBuff( p->GetManagedKey(), p->GetInfluenceIdx() );
						}
					}
				}
			}

			/// 타겟의 디버프 세팅
			typedef tPointerHashMap<unsigned long, void*> cTargetDebuffMap;
			cTargetDebuffMap* debuffMap = obj->GetDeBufMap();
			if( debuffMap )
			{
				if( debuffMap->GetSize() > 0 )
				{
					cTargetDebuffMap::cIterator i = debuffMap->Begin();
					cTargetDebuffMap::cIterator end = debuffMap->End();
					for( ; i != end; ++i )
					{
						cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
						if( p )
						{
							/// 효과는 스킬인덱스가 효과인덱스로 쓰임.
							/// 버프/디버프 내부 구별
							targetWindow->AddBuff( p->GetManagedKey(), p->GetInfluenceIdx() );
						}
					}
				}
			}
		}

		if( targetWindow )
			targetWindow->UpdateWindow();

		if( mTarget.type == eOBJECTTYPE_PLAYER )
		{
			PARTYMAN->SetTargetByHero( idx );
			PARTYUNIONMAN->SetTargetByHero( idx );;
		}
		else
		{
			PARTYMAN->ClearTarget();
			PARTYUNIONMAN->ClearTarget();
		}
	}
	else
	{
		if( targetWindow )
		{
			targetWindow->Hide();
			mTarget.index = 0;
			mTarget.type  = eOBJECTTYPE_NONE;
			PARTYMAN->ClearTarget();
			PARTYUNIONMAN->ClearTarget();
		}
	}

	//	SKILLEXECUTER->ResetAttackFlag();
	if( SKILLEXECUTER->IsSkillCharge() == true )
	{
		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
	}
	else
	{
		if( GetState() == eOBJECT_STATE_ATTACK )
		{
			cBaseObject* preTarget = OBJECTMAN->GetObject( oldType, oldIndex );
			if( preTarget && preTarget->GetState() == eOBJECT_STATE_DIE )
			{
				SetState( eOBJECT_STATE_IDLE );
			}
		}
	}
}

void cHero::HPChange( unsigned long objectHP, unsigned long objectMaxHP )
{
	assert(objectHP != 0);
	assert( objectHP <= objectMaxHP );

	mPlayerExrInfo.RestHP = objectHP;
	mPlayerExrInfo.MaxHP = objectMaxHP;

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardGaugeValue();

	/// 
	GAMEUI->UpdateHeroHPMP();
}

void cHero::HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType )
{
	assert( GetState() != eOBJECT_STATE_DIE );

	if( GetState() != eOBJECT_STATE_DIE )
	{
		if( IsRideVehicle() == true )
			AlightVehicle();

		if( pAttacker )
			cPlayer::SetDesiredDir( GetPos(), pAttacker->GetPos() );

		SetState( eOBJECT_STATE_DIE );
		if( attackType == eDAMAGE_CRITICAL )
		{
			ChangeAnimation( ANITYPE_DIE2 );
			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE2 );
		}
		else
		{
			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE1 );
		}
	}

	/// 사망시 보유 효과 제거..
	ClearAllBuff( false );
	ClearAllDeBuff( false );

	mCantMove = false;
	mCantSkillPhy = false;
	mCantSkillMag = false;
	mIsStun = false;
	mIsSleep = false;


	mPlayerExrInfo.RestHP = 0;
	mPlayerExrInfo.RestMP = 0;

	CalcStatus();

	assert( GetHP() <= GetMaxHP() );
	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardGaugeValue();

	/// 
	GAMEUI->UpdateHeroHPMP();
}

void cHero::MPChange( unsigned long objectMP, unsigned long objectMaxMP )
{
	if( GetState() == eOBJECT_STATE_DIE )
	{
		assert(objectMP == 0);
	}

	mPlayerExrInfo.RestMP = objectMP;
	mPlayerExrInfo.MaxMP = objectMaxMP;

	assert( GetMP() <= GetMaxMP() );
	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardGaugeValue();

	///
	GAMEUI->UpdateHeroHPMP();
	QUICKMAN->CheckSkillIcon();
}

void cHero::AfterAttack( cBaseObject* pTarget )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	/// 기본 공격 처리
	if( pTarget == 0 )
		return;

	/// 080508 PKH 정지 상태일때 일반공격 하면 에니메이션이 바껴서 체크
	if( GetState() == eOBJECT_STATE_STOP )
		return;

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	unsigned long attackIdx = chargeInfo.mSkillIdx;
	if( IsReadyUseSkill() == false )
	{
		SKILLEXECUTER->ChargeOff( attackIdx, 0 );
		return;
	}

	/// 공격 가능거리 여부 체크
	NiPoint3 targetPos = pTarget->GetPos();
	//	targetPos.z = 0.0f;
	NiPoint3 heroPos = GetPos();
	//	heroPos.z = 0.0f;

	/// 071127 PKH 
	float range = chargeInfo.mSkillRange;
	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, range );
	cRangeCheck rangecheck( tempRange );
	if( rangecheck.IsRange( heroPos, targetPos ) )
	{
		/// 즉시 공격..
		if( StartAttack( attackIdx, pTarget ) == false )
		{
			/// 공격 상태 유지
			SetState( eOBJECT_STATE_ATTACK );
			SetReadyUseSkill( true );
		}
	}
	else
	{
		mIsKeyMove = false;
		mGoCountinue = false;

		ClearMoveFlag();


		/// 후처리 등록한다.
		mAfterAction.mActionType = eAfterAction_Attack;
		mAfterAction.mTarget.index = pTarget->GetObjectID();
		mAfterAction.mTarget.type = pTarget->GetObjectType();
		mAfterAction.mDestPos = pTarget->GetPos();
		mAfterAction.mRange = range;
		mAfterAction.mExData1 = attackIdx;

		/// 타겟 위치로 이동 등록 ( 위치체크 및 상태 변경은 processAfterAction에 맞긴다. )
		if( ResetActionMovePath( pTarget->GetXPos(), pTarget->GetYPos(), range ) == false )
		{
			mAfterAction.mActionType = eAfterAction_None;
		}
	}

	SKILLEXECUTER->ChargeOff( attackIdx, 0 );
}

void cHero::AfterUseSkill( cBaseObject* pTarget )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();

	if( IsTransformMonster() )
	{
		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( GetTransMonsterClassIdx(), (eMONSTERATTACK_TYPE)chargeInfo.mSkillIdx);
		if( pInfo == 0 || pInfo->mBoundType == eBOUNDTYPE_FIELD )
		{
			assert(0);
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}

		if( pInfo->mBoundType == eBOUNDTYPE_SELF_NOTARGET )
		{
			/// 즉시 범위 스킬 시전
			if( IsReadyUseSkill() )
			{
				UseTargetSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, 0 );
			}
			else
			{
				SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
				mWaitSkill.skillIdx = chargeInfo.mSkillIdx;
				mWaitSkill.skillStep = chargeInfo.mSkillStep;
			}

			return;
		}
	}
	else
	{
		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( chargeInfo.mSkillIdx );
		if( pInfo == 0 || pInfo->mBoundType == eBOUNDTYPE_FIELD )
		{
			assert(0);
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}

		if( pInfo->mBoundType == eBOUNDTYPE_SELF_NOTARGET )
		{
			/// 즉시 범위 스킬 시전
			if( IsReadyUseSkill() )
			{
				UseTargetSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, 0 );
			}
			else
			{
				SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
				mWaitSkill.skillIdx = chargeInfo.mSkillIdx;
				mWaitSkill.skillStep = chargeInfo.mSkillStep;
			}

			return;
		}
		else if( pInfo->mBoundType == eBOUNDTYPE_NONE )
		{
			if( pInfo->mShotType == eSHOTTYPE_BLINK || pInfo->mShotType == eSHOTTYPE_BLINK_BACK )
			{
				if( IsReadyUseSkill() )
				{
					bool isForwardBlink = true;

					if( pInfo->mShotType == eSHOTTYPE_BLINK_BACK )
						isForwardBlink = false;

					HERO->UseBlinkSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, isForwardBlink );
				}
				else
				{
					SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
					mWaitSkill.skillIdx = chargeInfo.mSkillIdx;
					mWaitSkill.skillStep = chargeInfo.mSkillStep;
				}

				return;
			}
		}
	}

	if( pTarget == 0 )
	{
		assert(0);
		return;
	}

	NiPoint3 targetPos = pTarget->GetPos();
	NiPoint3 heroPos = GetPos();

	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, chargeInfo.mSkillRange );
	cRangeCheck rangecheck( tempRange );
	if( rangecheck.IsRange( heroPos, targetPos ) )
	{
		/// 시전
		if( IsReadyUseSkill() )
		{
			UseTargetSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, pTarget );
			return;
		}
		else
		{
			mWaitSkill.skillIdx = chargeInfo.mSkillIdx;
			mWaitSkill.skillStep = chargeInfo.mSkillStep;
		}
	}
	else
	{
		if( IsReadyUseSkill() )
		{
			mIsKeyMove = false;
			mGoCountinue = false;

			ClearMoveFlag();

			/// 후처리 등록한다.
			mAfterAction.mActionType = eAfterAction_UseTargetSkill;
			mAfterAction.mTarget.index = pTarget->GetObjectID();
			mAfterAction.mTarget.type = pTarget->GetObjectType();
			mAfterAction.mDestPos = pTarget->GetPos();
			mAfterAction.mRange = chargeInfo.mSkillRange;
			mAfterAction.mExData1 = chargeInfo.mSkillIdx;
			mAfterAction.mExData2 = chargeInfo.mSkillStep;

			/// 타겟 위치로 이동 등록 ( 위치체크 및 상태 변경은 processAfterAction에 맞긴다. )

			if( ResetActionMovePath( pTarget->GetXPos(), pTarget->GetYPos(), chargeInfo.mSkillRange ) == false )
			{
				mAfterAction.mActionType = eAfterAction_None;
			}
		}
		else
		{
			mWaitSkill.skillIdx = chargeInfo.mSkillIdx;
			mWaitSkill.skillStep = chargeInfo.mSkillStep;
		}
	}

	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
}

void cHero::AfterUseSkill( NiPoint3 fieldPos )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();

	if( IsTransformMonster() )
	{
		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( GetTransMonsterClassIdx(), (eMONSTERATTACK_TYPE)chargeInfo.mSkillIdx );
		if( pInfo == 0 || (pInfo->mBoundType != eBOUNDTYPE_FIELD && pInfo->mBoundType != eBOUNDTYPE_AUTOFIELD) )
		{
			assert(0);
			return;
		}

		NiPoint3 heroPos = GetPos();

		cRangeCheck rangecheck( chargeInfo.mSkillRange );
		if( rangecheck.IsRange( heroPos, fieldPos ) )
		{
			/// 시전
			if( IsReadyUseSkill() )
			{
				UseFieldSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, fieldPos );
				return;
			}
		}
	}
	else
	{
		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( chargeInfo.mSkillIdx );
		if( pInfo == 0 || (pInfo->mBoundType != eBOUNDTYPE_FIELD && pInfo->mBoundType != eBOUNDTYPE_AUTOFIELD) )
		{
			assert(0);
			return;
		}

		NiPoint3 heroPos = GetPos();

		cRangeCheck rangecheck( chargeInfo.mSkillRange );
		if( rangecheck.IsRange( heroPos, fieldPos ) )
		{
			/// 시전
			if( IsReadyUseSkill() )
			{
				UseFieldSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, fieldPos );
				return;
			}
		}

	}


	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
}

void cHero::AfterItemPick( unsigned long idx, float destX, float destY )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif

	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	if( IsJumping() == true )
		return;

	if( GetState() == eOBJECT_STATE_STOP )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	/// float range = 20;	20...?
	float range = ITEM_VALID_DISTANCE;

	NiPoint3 targetPos = NiPoint3(destX, destY, 0.0f);

	NiPoint3 heroPos = GetPos();
	heroPos.z = 0.0f;

	cRangeCheck rangecheck( range );
	if( rangecheck.IsRange( heroPos, targetPos ) )
	{
		/// 루팅윈도우 닫기 
		GAMEUI->HideLootingWindow();

		if( ITEMMAN->IsLocked() == true )
			return;

		/// 탈것 해제
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			HERO->AlightVehicle();
		}

		MSG_REQ_ITEM_GET_OPEN msg;
		msg.Category = NM_ITEM;
		msg.Protocol = NM_ITEM_GET_OPEN_REQ;
		msg.idx = idx;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 락 걸기
		ITEMMAN->LockItemTry( ITEM_TRY_PICK );

		/// 바라봐야할 방향 설정
		NiPoint3 charDir = mDesiredDir;
		NiPoint3 viewDir = targetPos - heroPos;
		viewDir.z = 0.0f;
		if( viewDir != NiPoint3::ZERO )
		{
			if( viewDir.SqrLength() >= 0.001f )	
			{
				viewDir.Unitize();

				/// 회전 Matrix 산출
				NiMatrix3 matA, matB, matRot;
				matA.SetCol( 0, charDir );
				matA.SetCol( 1, -NiPoint3::UNIT_Z );
				matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matB.SetCol( 0, viewDir );
				matB.SetCol( 1, -NiPoint3::UNIT_Z );
				matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matRot = matB * matA.Transpose();
				mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
				mDesiredDir = viewDir;
			}
		}

		SetState( ePLAYER_STATE_ITEMPICK );
		return;
	}

	mAfterAction.mActionType = eAfterAction_ItemPick;
	mAfterAction.mExData1 = idx;
	mAfterAction.mDestPos = NiPoint3( destX, destY, 0.0f );
	mAfterAction.mRange = ITEM_VALID_DISTANCE;
	mAfterAction.mTarget.index = 0;
	mAfterAction.mTarget.type = eOBJECTTYPE_NONE;

	if( ResetActionMovePath( destX, destY, mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

void cHero::AfterGatheringPick( unsigned long idx, unsigned long classIdx, float destX, float destY )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif
	if( IsReadyUseSkill() == false )
	{
		/// 스킬의 현재 상태가 캐스팅중이고 이동가능한지 체크한다.
		cSkillObject* p = SKILLEXECUTER->GetCurrentHeroSkillObject();
		if( p && p->GetDramaState() == eDRAMASTATE_CASTING )
		{
			cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pWin )
			{
				if( pWin->GetPercent() < 98.0f )
				{
					/// cast stop
					p->WaitCancel();
					pWin->Clear( eCasting_Skill );
					HERO->SetState( eOBJECT_STATE_IDLE );

					/// 070712 PKH 스킬 요청 메세지 발송
					MSG_REQ_SKILL_CAST_CANCEL Msg;
					Msg.Category = NM_SKILL;
					Msg.Protocol = NM_SKILL_CAST_CANCEL_REQ;
					Msg.mAttacker.index = HERO->GetObjectID();
					Msg.mAttacker.type = HERO->GetObjectType();

					NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
				}
			}

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else if( p && p->GetDramaState() == eDRAMASTATE_ACTIVITY && p->IsChanneling() )
		{
			cCastingBar* pCastingWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pCastingWin )
			{
				pCastingWin->Clear( eCasting_Skill );
			}

			p->WaitCancel();
			HERO->SetState( eOBJECT_STATE_IDLE );

			/// 070712 PKH 스킬 요청 메세지 발송
			MSG_REQ_CHANNELING_SKILL_CANCEL Msg;
			Msg.Category = NM_SKILL;
			Msg.Protocol = NM_SKILL_CHANNELING_CANCEL_REQ;
			Msg.mAttacker.index = HERO->GetObjectID();
			Msg.mAttacker.type = HERO->GetObjectType();

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}	
		else
			return;
	}

	if( NpcSpeechCancel() == false )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	float range = ITEM_VALID_DISTANCE;
	NiPoint3 targetPos = NiPoint3(destX, destY, 0.0f);
	NiPoint3 heroPos = GetPos();
	heroPos.z = 0.0f;

	cRangeCheck rangecheck( range );
	if( rangecheck.IsRange( heroPos, targetPos ) )
	{
		GatheringStart( idx, classIdx, targetPos );
		return;
	}

	mAfterAction.mActionType = eAfterAction_GatheringPick;
	mAfterAction.mExData1 = idx;
	mAfterAction.mExData2 = classIdx;
	mAfterAction.mDestPos = NiPoint3( destX, destY, 0.0f );
	mAfterAction.mRange = range;
	mAfterAction.mTarget.index = 0;
	mAfterAction.mTarget.type = eOBJECTTYPE_NONE;

	if( ResetActionMovePath( destX, destY, mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

void cHero::AfterNpcTalk( unsigned long npcIdx )
{
	AfterNpcTalk( OBJECTMAN->GetNpc(npcIdx) );
}

void cHero::AfterNpcTalk( cNpc* pNpc )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	if( pNpc == 0 )
		return;

	if( DUELMAN->IsDuelEnable() )
		return;

	if( GetState() == eOBJECT_STATE_DIE )
		return;

	if( IsJumping() == true )
		return;

	/// 자동사냥 신고 창이 뜬 도중이라면 무시
	cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
	if( camaelWin && camaelWin->IsVisible() == true )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	float range = NPC_VALID_DISTANCE;
	NiPoint3 targetPos = pNpc->GetPos();
	targetPos.z = 0.0f;

	NiPoint3 heroPos = GetPos();
	heroPos.z = 0.0f;

	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pNpc, range );

	cRangeCheck rangecheck( tempRange );
	if( rangecheck.IsRange( heroPos, targetPos ) )
	{
		MSG_REQ_NPC_OPEN Msg;
		Msg.Category = NM_NPC;
		Msg.Protocol = NM_NPC_OPEN_REQ;
		Msg.mNpcIdx = pNpc->GetObjectID();
		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

		SetState( eOBJECT_STATE_IDLE );

		return;
	}

	mAfterAction.mActionType = eAfterAction_NpcTalk;
	mAfterAction.mTarget.type = eOBJECTTYPE_NPC;
	mAfterAction.mTarget.index = pNpc->GetObjectID();
	mAfterAction.mDestPos = pNpc->GetPos();
	mAfterAction.mRange = NPC_VALID_DISTANCE;

	/// npc위치로 이동 등록 ( 위치체크 및 상태 변경은 processAfterAction에 맞긴다. )
	if( ResetActionMovePath( pNpc->GetXPos(), pNpc->GetYPos(), mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

///
void cHero::AfterUseTarotStore( cPlayer* pPlayer )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	if( pPlayer == 0 )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	mAfterAction.mActionType = eAfterAction_UseTarotStore;
	mAfterAction.mTarget.type = eOBJECTTYPE_PLAYER;
	mAfterAction.mTarget.index = pPlayer->GetObjectID();
	mAfterAction.mDestPos = pPlayer->GetPos();
	mAfterAction.mRange = 100.0f;

	if( ResetActionMovePath( pPlayer->GetXPos(), pPlayer->GetYPos(), mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

void cHero::AfterUseStall( cPlayer* pPlayer )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	if( pPlayer == 0 )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	mAfterAction.mActionType = eAfterAction_UseStall;
	mAfterAction.mTarget.type = eOBJECTTYPE_PLAYER;
	mAfterAction.mTarget.index = pPlayer->GetObjectID();
	mAfterAction.mDestPos = pPlayer->GetPos();
	mAfterAction.mRange = 100.0f;

	if( ResetActionMovePath( pPlayer->GetXPos(), pPlayer->GetYPos(), mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

void cHero::AfterMapChange( cMapPortalSceneNode* portalNode )
{
	if( portalNode == 0 )
		return;

	unsigned long portalIdx = portalNode->GetPortalIdx();
	unsigned long scriptIdx = portalNode->GetPosScriptIdx();
	if( scriptIdx == 0 )
		return;

	sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( scriptIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}

	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();

	NiPoint3 portalPos;
	portalPos.x = portalNode->GetTranslate().x;
	portalPos.y = portalNode->GetTranslate().y; 
	portalPos.z = 0.0f; 

	float range = pInfo->mRange;

	NiPoint3 heroPos = GetPos();
	heroPos.z = 0.0f;

	cRangeCheck rangecheck( range );
	if( rangecheck.IsRange( heroPos, portalPos ) )
	{
		if( pInfo->mFirstRegen == false )
		{
			/// same map / position change
			if( GetMapNum() != pInfo->mMapNumber )
			{
				assert(0);
				return;
			}

			MSG_REQ_TELEPORTPORTAL_USE Msg;
			Msg.Category = NM_TELEPORTPORTAL;
			Msg.Protocol = NM_TELEPORTPORTAL_USE_REQ;
			Msg.mMapChangePosIdx = (unsigned long)scriptIdx;

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			mMapPortalSkipTime = THEAPP->GetWorldAccumTime() + 1000;

			SetStopFlag( eSTOP_TELEPORTPORTAL );
		}
		else
		{
			if( portalIdx != 0 )
			{
				TCHAR str[256] = {0,};
				_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mMapNumber) );

				///
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_USERMAPCHANGE, eSKIN_YESNO,
					str,
					GAMERESOURCEMAN->GetGameText( 59 ),
					(void*)portalIdx );
			}
			else
			{
				if( pInfo->mTargetInfo )
				{
					TCHAR str[256] = {0,};
					_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mTargetInfo->mMapNumber) );

					///
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_MAPCHANGE, eSKIN_YESNO,
						str,
						GAMERESOURCEMAN->GetGameText( 59 ),
						(void*)scriptIdx );
				}
			}
		}
		return;
	}

	mAfterAction.mActionType = eAfterAction_MapChange;
	mAfterAction.mExData1 = portalIdx;
	mAfterAction.mExData2 = scriptIdx;
	mAfterAction.mDestPos = portalPos;
	mAfterAction.mRange = range;
	mAfterAction.mTarget.index = 0;
	mAfterAction.mTarget.type = eOBJECTTYPE_NONE;

	if( ResetActionMovePath( portalPos.x, portalPos.y, mAfterAction.mRange ) == false )
		mAfterAction.mActionType = eAfterAction_None;
}

void cHero::SetStatus1( ePLAYER_STATUS statusBase, float value )
{
	if( statusBase >= ePLAYER_STATUS_LEVEL )
	{
		assert(NULL);
		return;
	}

	switch( statusBase )
	{
	case ePLAYER_STATUS_STR:
		mStatus1.mStr = value; break;
	case ePLAYER_STATUS_DEX:
		mStatus1.mDex = value; break;
	case ePLAYER_STATUS_CON:
		mStatus1.mCon = value; break;
	case ePLAYER_STATUS_INT:
		mStatus1.mInt = value; break;
	case ePLAYER_STATUS_WIS:
		mStatus1.mWis = value; break;
	}
}

float cHero::GetStatusBaseTotal( ePLAYER_STATUS statusBase )
{
	if( statusBase >= ePLAYER_STATUS_LEVEL )
	{
		assert(NULL);
		return 0.0f;
	}

	switch( statusBase )
	{
	case ePLAYER_STATUS_STR:
		return mStatus1.mStr < 0.0f ? 0.0f : mStatus1.mStr;
	case ePLAYER_STATUS_DEX:
		return mStatus1.mDex < 0.0f ? 0.0f : mStatus1.mDex;
	case ePLAYER_STATUS_CON:
		return mStatus1.mCon < 0.0f ? 0.0f : mStatus1.mCon;
	case ePLAYER_STATUS_INT:
		return mStatus1.mInt < 0.0f ? 0.0f : mStatus1.mInt;
	case ePLAYER_STATUS_WIS:
		return mStatus1.mWis < 0.0f ? 0.0f : mStatus1.mWis;
	}
	return 0.0f;
}

float cHero::GetStatusPlusTotal( ePLAYER_STATUS statusBase )
{
	if( statusBase >= ePLAYER_STATUS_LEVEL )
	{
		assert(NULL);
		return 0.0f;
	}

	float total = 0.0f;

	switch( statusBase )
	{
	case ePLAYER_STATUS_STR:
		{
			total = mStatus1Plus.mStr;

			float percent = mStatus1Per.mStr * 0.01f;
			if( percent > 0.0f )
				total += (mStatus1.mStr + mStatus1Plus.mStr) * percent;
		}
		break;
	case ePLAYER_STATUS_DEX:
		{
			total = mStatus1Plus.mDex;

			float percent = mStatus1Per.mDex * 0.01f;
			if( percent > 0.0f )
				total += (mStatus1.mDex + mStatus1Plus.mDex) * percent;
		}
		break;
	case ePLAYER_STATUS_CON:
		{
			total = mStatus1Plus.mCon;

			float percent = mStatus1Per.mCon * 0.01f;
			if( percent > 0.0f )
				total += (mStatus1.mCon + mStatus1Plus.mCon) * percent;
		}
		break;
	case ePLAYER_STATUS_INT:
		{
			total = mStatus1Plus.mInt;

			float percent = mStatus1Per.mInt * 0.01f;
			if( percent > 0.0f )
				total += (mStatus1.mInt + mStatus1Plus.mInt) * percent;
		}
		break;
	case ePLAYER_STATUS_WIS:
		{
			total = mStatus1Plus.mWis;

			float percent = mStatus1Per.mWis * 0.01f;
			if( percent > 0.0f )
				total += (mStatus1.mWis + mStatus1Plus.mWis) * percent;
		}
		break;
	}

	return total < 0.0f ? 0.0f : total;
}


/// 070116 PKH 기본 스텟 추가 수치 등록
void cHero::SetStatus1Plus( ePLAYER_STATUS statusBasePlus, float value )
{
	if( statusBasePlus >= ePLAYER_STATUS_LEVEL )
	{
		assert(NULL);
		return;
	}

	switch( statusBasePlus )
	{
	case ePLAYER_STATUS_STR:
		mStatus1Plus.mStr = value; break;
	case ePLAYER_STATUS_DEX:
		mStatus1Plus.mDex = value; break;
	case ePLAYER_STATUS_CON:
		mStatus1Plus.mCon = value; break;
	case ePLAYER_STATUS_INT:
		mStatus1Plus.mInt = value; break;
	case ePLAYER_STATUS_WIS:
		mStatus1Plus.mWis = value; break;
	}
}

void cHero::SetStatus1Per( ePLAYER_STATUS statusBasePer, float value )
{
	if( statusBasePer >= ePLAYER_STATUS_LEVEL )
	{
		assert(NULL);
		return;
	}

	switch( statusBasePer )
	{
	case ePLAYER_STATUS_STR:
		mStatus1Per.mStr = value; break;
	case ePLAYER_STATUS_DEX:
		mStatus1Per.mDex = value; break;
	case ePLAYER_STATUS_CON:
		mStatus1Per.mCon = value; break;
	case ePLAYER_STATUS_INT:
		mStatus1Per.mInt = value; break;
	case ePLAYER_STATUS_WIS:
		mStatus1Per.mWis = value; break;
	}
}

void cHero::SetStatus2( ePLAYER_STATUS_EXT extStatus, float value )
{
	if( extStatus >= ePLAYER_STATUS_EXT2_MAX )
	{
		assert(NULL);
		return;
	}

	switch( extStatus )
	{
	case ePLAYER_STATUS_EXT_ATK:
		break;	/// 최소 최대 값으로 재정의
	case ePLAYER_STATUS_EXT_RATK:
		break;	/// 최소 최대 값으로 재정의
	case ePLAYER_STATUS_EXT_DEF:
		mStatus2.mPhysicDefense = value; break;
	case ePLAYER_STATUS_EXT_MATK:
		break;	/// 최소 최대 값으로 재정의
	case ePLAYER_STATUS_EXT_MDEF:
		mStatus2.mMagicDefense = value; break;
	case ePLAYER_STATUS_EXT_CRI:
		mStatus2.mPhysicCritical = value; break;
	case ePLAYER_STATUS_EXT_MCRI:
		mStatus2.mMagicCritical = value; break;
		//case ePLAYER_STATUS_EXT_MAXHP:
		//	{
		//		mStatus2.mMaxHP = value;

		//		if( mPlayerExrInfo.RestHP > (unsigned long)mStatus2.mMaxHP )
		//			mPlayerExrInfo.RestHP = (unsigned long)mStatus2.mMaxHP;
		//	}
		//	break;
		//case ePLAYER_STATUS_EXT_MAXMP:
		//	{
		//		mStatus2.mMaxMP = value;

		//		if( mPlayerExrInfo.RestMP > (unsigned long)mStatus2.mMaxMP )
		//			mPlayerExrInfo.RestMP = (unsigned long)mStatus2.mMaxMP;
		//	}
		//	break;
	case ePLAYER_STATUS_EXT_RECOVHP:
		mStatus2.mRecovHP = value; break;
	case ePLAYER_STATUS_EXT_RECOVMP:
		mStatus2.mRecovMP = value; break;
	case ePLAYER_STATUS_EXT_HIT:
		mStatus2.mPhysicAttackRate = value; break;
	case ePLAYER_STATUS_EXT_MHIT:
		mStatus2.mMagicAttackRate = value; break;
	case ePLAYER_STATUS_EXT_AVOID:
		mStatus2.mPhysicAvoid = value; break;
	case ePLAYER_STATUS_EXT_WEIGHT:
		mStatus2.mMaxWeight = value; break;
	case ePLAYER_STATUS_EXT_ASPD:
		mStatus2.mAttackSpeed = (float)value; break;
	case ePLAYER_STATUS_EXT_COOLTIME:
		mStatus2.mCooltime = (float)value; break;
	case ePLAYER_STATUS_EXT_RANGE:
		mStatus2.mInRange = value; break;
	case ePLAYER_STATUS_EXT_RRANGE:
		mStatus2.mOutRange = value; break;
	case ePLAYER_STATUS_EXT_SPD:
		mStatus2.mMoveSpeed = value; break;
	case ePLAYER_STATUS_EXT_HEAL:
		mStatus2.mHeal = value; break;
	case ePLAYER_STATUS_EXT_MAVOID:
		mStatus2.mMagicAvoid = value; break;
	case ePLAYER_STATUS_EXT_CRIDEF:
		mStatus2.mPhyCriDef = value; break;
	case ePLAYER_STATUS_EXT_MCRIDEF:
		mStatus2.mMagCriDef = value; break;
	case ePLAYER_STATUS_EXT_ATKMin:
		mStatus2.mPhysicMinAttack = value; break;
	case ePLAYER_STATUS_EXT_ATKMax:
		mStatus2.mPhysicMaxAttack = value; break;
	case ePLAYER_STATUS_EXT_RATKMin:
		mStatus2.mPhysicMinRangeAttack = value; break;
	case ePLAYER_STATUS_EXT_RATKMax:
		mStatus2.mPhysicMaxRangeAttack = value; break;
	case ePLAYER_STATUS_EXT_MATKMin:
		mStatus2.mMagicMinAttack = value; break;
	case ePLAYER_STATUS_EXT_MATKMax:
		mStatus2.mMagicMaxAttack = value; break;
	}
}


void cHero::SetStatusEtc( ePLAYER_STATUS_EXT_ADD statusAdd, float value  )
{
	if( statusAdd >= ePLAYER_STATUS_EXT_ADD_MAX )
	{
		assert(NULL);
		return;
	}

	switch( statusAdd )
	{
	case ePLAYER_STATUS_EXT_ADD_EXPADDPER:
		mStatusEtc.mExpAddPer = value; break;
	case ePLAYER_STATUS_EXT_ADD_EXPADDPLUS:
		mStatusEtc.mExpAddPlus = value; break;
	case ePLAYER_STATUS_EXT_ADD_SXPADDPER:
		mStatusEtc.mSxpAddPer = value; break;
	case ePLAYER_STATUS_EXT_ADD_SXPADDPLUS:
		mStatusEtc.mSxpAddPlus = value; break;
	}
}


float cHero::CalcStatusSkillRange( float skillRange, eRANGETYPE mRangeType, bool IsNormalAttack )
{
	if( mRangeType == 0 )
		return skillRange;

	/// 스킬 사거리
	float range = skillRange;

	/// 효과 사거리
	float statusRangePlus = 0;
	float statusRangePer = 0;

	sEquipAbility* pEquipAbility = ITEMMAN->GetBasicAbil();
	if( pEquipAbility == 0 )
	{
		assert(0);
	}

	if( mRangeType == eRANGETYPE_CLOSE )
	{
		statusRangePlus = STATUSCALC->GetStatus2Plus( ePLAYER_STATUS_EXT_RANGE );
		statusRangePer = STATUSCALC->GetStatus2Per( ePLAYER_STATUS_EXT_RANGE );

		if( pEquipAbility )
		{
			statusRangePlus = statusRangePlus + pEquipAbility->mOptPlusShortRange;
			statusRangePer = statusRangePer + pEquipAbility->mOptPercentShortRange;
		}
	}
	else if( mRangeType == eRANGETYPE_LONG )
	{
		statusRangePlus = STATUSCALC->GetStatus2Plus( ePLAYER_STATUS_EXT_RRANGE );
		statusRangePer = STATUSCALC->GetStatus2Per( ePLAYER_STATUS_EXT_RRANGE );

		if( pEquipAbility )
		{
			statusRangePlus = statusRangePlus + pEquipAbility->mOptPlusDistanceRange;
			statusRangePer = statusRangePer + pEquipAbility->mOptPercentDistanceRange;
		}
	}
	else
		assert(NULL);

	/// 사거리 계산
	if( pEquipAbility && IsNormalAttack == true )
	{
		range = ( range + pEquipAbility->mAttackRange ) * ( ( 100 + statusRangePer ) / 100 ) + statusRangePlus;
	}
	else
		range = range * ( ( 100 + statusRangePer ) / 100 ) + statusRangePlus;

	return range;
}


void cHero::InitStatusInfoType() 
{
	for( int i = ePLAYER_STATUS_STR ; ePLAYER_STATUS_LEVEL > i ; ++i )
	{
		mStatusBaseInfoType[i] = 0; 
	}
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mStatusExtInfoType[i] = 0; 
	}
}

void cHero::IdleUpdate( unsigned long deltaTime )
{
	if( IsJumping() == true )
	{
		mIdleTime = 0;
		return;
	}

	mIdleTime += deltaTime;
	if( mIdleTime >= 18000 )
	{
		mIdleTime = 0;

		if( mPlayerExrInfo.mChgMonsterIdx == 0 )
		{
			/// idle2 animation
			if( mPlayerExrInfo.mStateIdle == eIDLE_PEACE )
			{
				if( IsRideVehicle() )
					ChangeAnimation( ANITYPE_IDLE2 );
				else
					ChangeAnimation( ANITYPE_PEACE_IDLE2 );
			}
			else
			{
				ChangeAnimation( ANITYPE_IDLE2 );
			}
		}
		else
		{
			ChangeAnimation_TransMon( M_ANITYPE_IDLE2 );
		}
	}

	unsigned int keyFlag = MOVE_NONE;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
		keyFlag |= MOVE_W;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
		keyFlag |= MOVE_S;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
		keyFlag |= MOVE_A;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
		keyFlag |= MOVE_D;

	if( keyFlag != MOVE_NONE )
	{
		UpdateKeyState();
		return;
	}
}

void cHero::MoveUpdate( unsigned long deltaTime )
{
	if( mPathCount == 0 )
		return;

	/// 현재 좌표를 획득
	NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

	/// 이건 캐릭터별로 따로 설정되는 값이다.
	long speed = GetMoveSpeed();
	if( mJumpProcess )
	{
		if( speed > 10 )
			speed = 10;
	}

	/// 보정을 위해서 목적지까지의 남은 거리를 계산한다.
	NiPoint2 dir1, dir2;
	dir1.x = mGotoX - pos.x;
	dir1.y = mGotoY - pos.y;
	dir1.Unitize();

	/// 속도 * 경과시간 만큼의 거리 계산
	pos.x += dir1.x * (speed * (deltaTime * 0.001f));
	pos.y += dir1.y * (speed * (deltaTime * 0.001f));
	pos.z = 0.0f;

	mSumKeyMoveDeltaTime += deltaTime;

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	dir2.x = mGotoX - pos.x;
	dir2.y = mGotoY - pos.y;
	dir2.Unitize();

	bool nextPath = false;
	if( dir2.Dot(dir1) <= 0.0f )
	{
		if( mPathIndex < mPathCount-1 )
		{
			/// 중간경로 끝위치에 허용치를 주는 이유는 이동시 꺾일때 부드럽게 보이기 위해서임
			/// 클라이언트 평균15프레임으로 벗어 날수있는 오차범위 66
			if( deltaTime > 66 )
			{
				pos.x = mGotoX;
				pos.y = mGotoY;
			}

			nextPath = true;
		}
		else
		{
			/// 이동 최종목적지는 항상 지정한점
			pos.x = mGotoX;
			pos.y = mGotoY;

			if( pos.x == mTargetPos.x && pos.y == mTargetPos.y )
			{
				mPathCount = 0;
				SetState( eOBJECT_STATE_IDLE );
			}
			else
			{
				ResetMovePath( NiPoint2(pos.x, pos.y), NiPoint2(mTargetPos.x, mTargetPos.y) );
			}
		}
	}

	if( EnableGateMovePos( NiPoint2(GetXPos(), GetYPos()), pos.x, pos.y ) == false )
	{
		pos.x = GetXPos();
		pos.y = GetYPos();
	}

	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
	{
		SetPos( pos );
	}
	else
	{
		assert(0);
		return;
	}

	if( nextPath == true )
	{
		NiPoint2& nextPos = mPathArray[++mPathIndex];

		if( WORLDMAN->EnableRayCheck( NiPoint2(pos.x, pos.y), nextPos ) == false )
			ResetMovePath( NiPoint2(pos.x, pos.y), NiPoint2(mTargetPos.x, mTargetPos.y) );
		else
			Move( nextPos.x, nextPos.y );
	}

	/// 이동후 처리가 등록 되었다면 처리한다.
	/// 내부에서 거리비교에 따른 좌표 보정을 한다.
	ProcessAfterAction( pos );
}

void cHero::FollowIdleUpdate()
{
	cPlayer* pFollowPlayer = (cPlayer*)OBJECTMAN->GetPlayer( mFollowPlayerIndex );
	if( pFollowPlayer ) 
	{
		NiPoint2 heroPos( GetXPos(), GetYPos() );
		NiPoint2 targetPos( pFollowPlayer->GetXPos(), pFollowPlayer->GetYPos() );
		float len = (heroPos-targetPos).SqrLength();

		if( len >= 160000.0f ) /// 400 * 400
		{
			if( ResetMovePath( heroPos, targetPos ) == false )
			{
				assert(0);
				ReleaseFollowPlayer();		
			}

			mFollowUpdateTime = 0;
		}
		else if( len >= 9000000.0f ) /// 3000 * 3000
		{
			ReleaseFollowPlayer();
			if( GetState() == eOBJECT_STATE_MOVE )
			{
				/// 타겟이 사정거리 이상으로 멀어지면 멈추기.
				SetState( eOBJECT_STATE_IDLE );

				/// stop 패킷 발송
				MSG_REQ_MOVE_STOP Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_FOLLOWMOVE_STOP_REQ;
				Msg.mClientPosX = GetXPos();
				Msg.mClientPosY = GetYPos();
				Msg.mClientTime = mSumKeyMoveDeltaTime;
				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}
		}
	}
	else
	{
		/// 타겟이 사라졌다면 따라가기 취소.
		ReleaseFollowPlayer();		
	}
}

void cHero::FollowMoveUpdate( unsigned long deltaTime )
{
	cPlayer* pFollowPlayer = (cPlayer*)OBJECTMAN->GetPlayer( mFollowPlayerIndex );
	if( pFollowPlayer ) 
	{ 
		NiPoint2 heroPos( GetXPos(), GetYPos() );
		NiPoint2 targetPos( pFollowPlayer->GetXPos(), pFollowPlayer->GetYPos() );
		float len = (heroPos-targetPos).SqrLength();

		if( len <= 40000.0f ) /// 200 * 200
		{
			/// 타겟이 가까워지면 멈추기.
			SetState( eOBJECT_STATE_IDLE );

			/// stop 패킷 발송 (임시용)
			MSG_REQ_MOVE_STOP Msg;
			Msg.Category = NM_PLAYER;
			Msg.Protocol = NM_PLAYER_FOLLOWMOVE_STOP_REQ;
			Msg.mClientPosX = GetXPos();
			Msg.mClientPosY = GetYPos();
			Msg.mClientTime = mSumKeyMoveDeltaTime;
			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			return;
		}
		else if( len >= 9000000.0f ) /// 3000 * 3000
		{
			ReleaseFollowPlayer();
			if( GetState() == eOBJECT_STATE_MOVE )
			{
				/// 타겟이 사정거리 이상으로 멀어지면 멈추기.
				SetState( eOBJECT_STATE_IDLE );

				/// stop 패킷 발송
				MSG_REQ_MOVE_STOP Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_FOLLOWMOVE_STOP_REQ;
				Msg.mClientPosX = GetXPos();
				Msg.mClientPosY = GetYPos();
				Msg.mClientTime = mSumKeyMoveDeltaTime;
				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}
			return;
		}

		///
		mFollowUpdateTime += deltaTime;
		if( mFollowUpdateTime >= 500 )
		{
			if( ResetMovePath( heroPos, targetPos ) == false )
				assert( 0 );

			mFollowUpdateTime = 0;
		}

		///
		MoveUpdate( deltaTime );
	}
	else
	{
		/// 타겟이 사라졌다면 따라가기 취소.
		ReleaseFollowPlayer();		
	}
}

void cHero::KeyMoveUpdate( unsigned long deltaTime )
{
	assert(KEYBOARD);

	cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 );
		return;
	}

	NiPoint3 goDir = cam->GetWorldDirection();
	NiPoint3 rightDir = cam->GetWorldRightDir();

	/// 입력처리
	NiPoint3 dir = NiPoint3::ZERO;
	unsigned int keyFlag = MOVE_NONE;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
	{
		keyFlag |= MOVE_W;
		dir += goDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
	{
		keyFlag |= MOVE_S;
		dir += -goDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
	{
		keyFlag |= MOVE_A;
		dir += -rightDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
	{
		keyFlag |= MOVE_D;
		dir += rightDir;
	}

	/// 지속 모드
	if( mGoCountinue == true )
	{
		if( keyFlag != 0 )
		{
			mGoCountinue = false;
		}
		else
		{
			dir += goDir;
		}
	}

	dir.z = 0.0f;
	dir.Unitize();

	if( keyFlag == 0 && mGoCountinue == false )
	{
		mIsKeyMove = false;
		SetState( eOBJECT_STATE_IDLE );

		/// stop 패킷 발송
		MSG_REQ_MOVE_STOP Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_KEYMOVE_STOP_REQ;
		Msg.mClientPosX = GetXPos();
		Msg.mClientPosY = GetYPos();
		Msg.mClientTime = mSumKeyMoveDeltaTime;
		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
		return;
	}

	if( dir != NiPoint3::ZERO && dir != mDesiredDir )
	{
		SetDesiredDir( dir );

		/// 신규 목적지 설정
		NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();
		pos.z = 0.0f;
		NiPoint3 goPos = pos + mDesiredDir*200.0f;

		WORLDMAN->EnableRayGet( pos, goPos );

		mGotoX = goPos.x;
		mGotoY = goPos.y;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 셋팅된 목적지까지 이동처리
	NiPoint3 curPos = mpObjectSceneNode->GetWorldTranslate();
	if( curPos.x == mGotoX && curPos.y == mGotoY )
		return;
	if( WORLDMAN->EnableRayCheck( NiPoint2(curPos.x, curPos.y), NiPoint2(mGotoX, mGotoY) ) == false )
		return;
	if( EnableGateMovePos( NiPoint2(curPos.x, curPos.y), mGotoX, mGotoY ) == false )
	{
		mGotoX = curPos.x;
		mGotoY = curPos.y;
		return;
	}

	/// 이건 캐릭터별로 따로 설정되는 값이다.
	long speed = GetMoveSpeed();
	if( mJumpProcess )
	{
		if( speed > 10 )
			speed = 10;
	}

	/// 보정을 위해서 목적지까지의 남은 거리를 계산한다.
	/// 속도 * 경과시간 만큼의 거리 계산
	curPos.x += mDesiredDir.x * speed * (deltaTime * 0.001f);
	curPos.y += mDesiredDir.y * speed * (deltaTime * 0.001f);
	curPos.z = 0.0f;

	mSumKeyMoveDeltaTime += deltaTime;

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	NiPoint3 dir1;
	dir1.x = mGotoX - curPos.x;
	dir1.y = mGotoY - curPos.y;
	dir1.z = 0.0f;
	dir1.Unitize();

	if( dir1.Dot(mDesiredDir) <= 0.0f )
	{
		if( deltaTime > 500 )
		{
			curPos.x = mGotoX;
			curPos.y = mGotoY;
		}
		else
		{
			NiPoint3 oldPos = mpObjectSceneNode->GetWorldTranslate();
			if( WORLDMAN->EnableRayCheck( NiPoint2(oldPos.x, oldPos.y), NiPoint2(curPos.x, curPos.y) ) == false )
			{
				curPos.x = mGotoX;
				curPos.y = mGotoY;
			}
		}

		/// 신규 목적지 설정
		NiPoint3 goPos = curPos + mDesiredDir*200.0f;
		WORLDMAN->EnableRayGet( curPos, goPos );
		mGotoX = goPos.x;
		mGotoY = goPos.y;

		if( EnableGateMovePos( NiPoint2(curPos.x, curPos.y), mGotoX, mGotoY ) == false )
		{
			mGotoX = curPos.x;
			mGotoY = curPos.y;
			return;
		}

		/// 신규 목적지 패킷 발송
		MSG_REQ_KEYMOVE Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_KEYMOVE_REQ;
		Msg.mStartX = curPos.x;
		Msg.mStartY = curPos.y;
		Msg.mDestX = mGotoX;
		Msg.mDestY = mGotoY;
		Msg.mClientTime = mSumKeyMoveDeltaTime;
		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

		mSumKeyMoveDeltaTime = 0;
	}

	if( WORLDMAN->CalcHeight( &curPos.z, curPos.x, curPos.y ) )
	{
		SetPos( curPos );
	}
	else
	{
		assert(0);
		return;
	}
	//////////////////////////////////////////////////////////////////////////
}

bool cHero::SetDesiredDir( NiPoint3 dir )
{
	/// 현재 캐릭터의 방향
	NiPoint3 charDir = mDesiredDir;

	if( mDesiredDir == dir )
		return false;

	/// 회전 Matrix 산출
	NiMatrix3 matA, matB, matRot;
	matA.SetCol( 0, charDir );
	matA.SetCol( 1, -NiPoint3::UNIT_Z );
	matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

	matB.SetCol( 0, dir );
	matB.SetCol( 1, -NiPoint3::UNIT_Z );
	matB.SetCol( 2, dir.UnitCross( -NiPoint3::UNIT_Z ) );

	matRot = matB * matA.Transpose();

	mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
	mDesiredDir = dir;

	return true;
}

void cHero::AttackUpdate( unsigned long deltaTime, unsigned long /*accumTime*/ )
{
	/// 스킬 동작 비완료시...
	if( IsReadyUseSkill() == false )
		return;

	/// 무기 교체 대기 - 교체
	if( mWaitWeaponSwitch )
	{
		if( mSwitchWeaponFlag == false )
		{
			if( ITEMMAN->TryToSwitchWeapon() )
				SetWeaponChangeFlag( true );
		}
		mWaitWeaponSwitch = false;

		if( mWaitMoveFlag == eWAITMOVE_NONE )
			return;
	}

	/// 이동 대기인 경우
	switch( mWaitMoveFlag )
	{
	case eWAITMOVE_MOUSE:
		{
			mWaitMoveFlag = eWAITMOVE_NONE;

			if( mAfterAction.mActionType == eAfterAction_None )
			{
				ResetMovePath( NiPoint2(GetXPos(), GetYPos()), mWaitMovePos );
			}
			else
			{
				if( ResetActionMovePath( mWaitMovePos.x, mWaitMovePos.y, mAfterAction.mRange ) == false )
					mAfterAction.mActionType = eAfterAction_None;
			}
		}
		return;
	default:
		{
			/// casting인 경우를 제외하고 처리한다...
			mWaitMoveFlag = eWAITMOVE_NONE;

			unsigned int keyFlag = MOVE_NONE;
			if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
				keyFlag |= MOVE_W;
			if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
				keyFlag |= MOVE_S;
			if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
				keyFlag |= MOVE_A;
			if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
				keyFlag |= MOVE_D;

			if( keyFlag != MOVE_NONE )
			{
				UpdateKeyState();
				return;
			}
		}
		break;
	}

	if( mAutoAttackFlag == false && mWaitSkill.skillIdx == 0 )
	{
		SetState( eOBJECT_STATE_IDLE, false );
		SetIdleFlag( eIDLE_NORMAL );

		if( IsTransformMonster() == true )
		{
			if( mWaitSkill.skillIdx < eMONSTERATTACK_SKILL1 )
				return;
		}
		else
		{
			if( mWaitSkill.skillIdx <NORMAL_ATTACK_SKILL_MAX )
				return;
		}
	}

	if( SKILLEXECUTER->IsSkillCharge() == true )
	{
		if( IsTransformMonster() == true )
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		else
			ChangeAnimation( ANITYPE_IDLE );
		return;
	}

	/// 스킬 사용 대기
	/// 기본 공격 (착용 무기or직업군에 따라서... 자동 비자동으로 구분)
	if( IsTransformMonster() == true )
	{
		if( mWaitSkill.skillIdx < eMONSTERATTACK_SKILL1 )
		{
			if( SKILLMAN->IsUsedMonsterSkill( eMONSTERATTACK_NORMAL1 ) == false )
				return;

			/// 자동공격에 어색하지 않도록 최소한의 딜레이를 주는게 좋을듯..
			if( SKILLEXECUTER->AutoAttack( GetTargetObject() ) == false ) 
				SetState( eOBJECT_STATE_IDLE );
		}
		else
		{
			/// 보유스킬 여부 및 cool time check
			if( SKILLMAN->IsUsedMonsterSkill( mWaitSkill.skillIdx ) == false )
			{
				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
				return;
			}

			SKILLEXECUTER->ExecuteSkill( mWaitSkill.skillIdx, mWaitSkill.skillStep );
			mWaitSkill.skillIdx = 0;
			mWaitSkill.skillStep = 0;
		}
	}
	else
	{
		if( mWaitSkill.skillIdx < NORMAL_ATTACK_SKILL_MAX )
		{
			if( SKILLMAN->IsUsedKeepSkill( NORMAL_ATTACK_SKILL, 0 ) == false )
				return;

			/// 자동공격에 어색하지 않도록 최소한의 딜레이를 주는게 좋을듯..
			if( SKILLEXECUTER->AutoAttack( GetTargetObject() ) == false ) 
				SetState( eOBJECT_STATE_IDLE );
		}
		else
		{
			/// 보유스킬 여부 및 cool time check
			if( SKILLMAN->IsUsedKeepSkill( mWaitSkill.skillIdx, mWaitSkill.skillStep ) == false )
			{
				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
				return;
			}

			SKILLEXECUTER->ExecuteSkill( mWaitSkill.skillIdx, mWaitSkill.skillStep );
			mWaitSkill.skillIdx = 0;
			mWaitSkill.skillStep = 0;
		}
	}
}


void cHero::Update( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpObjectSceneNode  == NULL ) 
		return;

	/// 변신 대기 일때 처리
	if( mReadyTransformInfo.readState == eReady_RestoreMonster )
	{
		if( IsReadyUseSkill() && IsJumping() == false )
		{
			RestoreMonster();
			ChangeAnimation( ANITYPE_IDLE );

			if( mPlayerExrInfo.mState == eOBJECT_STATE_ATTACK )
			{
				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
			}
		}
	}
	else if( mReadyTransformInfo.readState == eReady_TransformMonster )
	{
		if( IsReadyUseSkill() && IsJumping() == false )
		{
			TransformMonster( mReadyTransformInfo.chgMonIdx );
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );

			if( mPlayerExrInfo.mState == eOBJECT_STATE_ATTACK )
			{
				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
			}
		}
	}

	///
	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
		return Update_TransMon( deltaTime, accumTime );

	ProcessDelayFlag( deltaTime );

	DesiredDirProcess( deltaTime );

	FaceProcess( deltaTime );

	/// 정보가 업데이트 될때마다 세팅
	if( mIsCalcStatus )
	{
		NowCalcStatus();
	}
	/// 스킬겨험치나 경험치가 업데이트될때마다 세팅
	else if( mIsUdateHeroInfo )
	{
		GAMEUI->UpdateExpSxp();

		mIsUdateHeroInfo = false;
	}

	/// 상태에 맞는 처리를 한다.
	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_IDLE:
		{
			if( mIsFollowing )
				FollowIdleUpdate();

			IdleUpdate( deltaTime );
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			if( mIsKeyMove == true )
			{
				KeyMoveUpdate( deltaTime );
			}
			else
			{
				if( mIsFollowing )
					FollowMoveUpdate( deltaTime );
				else
					MoveUpdate( deltaTime );
			}

			/// portal 체크
			if( mMapPortalSkipTime <= accumTime )
			{
				cMapPortalSceneNode* portalNode = OBJECTMANAGER->MapPotalPicking();
				if( portalNode )
				{
					unsigned long portalIdx = portalNode->GetPortalIdx();
					unsigned long posScriptIdx = portalNode->GetPosScriptIdx();

					sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( posScriptIdx );
					if( pInfo )
					{
						cRangeCheck check;
						check.SetRadius( pInfo->mRange );
						if( check.IsNotRange( NiPoint2(GetXPos(), GetYPos()), NiPoint2(portalNode->GetTranslate().x, portalNode->GetTranslate().y) ) == false )
						{
							/// 091014 PKH 텔레포트 포탈 체크추가
							if( pInfo->mFirstRegen == false )
							{
								MSG_REQ_TELEPORTPORTAL_USE Msg;
								Msg.Category = NM_TELEPORTPORTAL;
								Msg.Protocol = NM_TELEPORTPORTAL_USE_REQ;
								Msg.mMapChangePosIdx = (unsigned long)posScriptIdx;

								NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

								mMapPortalSkipTime = accumTime + 1000;

								SetStopFlag( eSTOP_TELEPORTPORTAL );
							}
							else
							{
								mIsKeyMove = false;
								mGoCountinue = false;

								ClearMoveFlag();

								SetState( eOBJECT_STATE_IDLE );
								mMapPortalSkipTime = accumTime + 1000;

								if( portalIdx != 0 )
								{
									TCHAR str[256] = {0,};
									_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mMapNumber) );

									///
									UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, eMBEVENT_USERMAPCHANGE, eSKIN_YESNO, str,
										GAMERESOURCEMAN->GetGameText( 59 ), (void*)portalIdx );
								}
								else
								{
									if( pInfo->mTargetInfo )
									{
										TCHAR str[256] = {0,};
										_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mTargetInfo->mMapNumber) );

										///
										UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, eMBEVENT_MAPCHANGE, eSKIN_YESNO, str, 
											GAMERESOURCEMAN->GetGameText( 59 ), (void*)posScriptIdx );
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			AttackUpdate( deltaTime, accumTime );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
			DieUpdate();
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			PushPullUpdate(deltaTime);
		}
		break;
	case ePLAYER_STATE_RUSH:
		{
			RushUpdate(deltaTime);
		}
		break;
	}

	/// 애니메이션 연결에 대한 처리
	Interpret( accumTime );

	if( IsJumping() == true )
	{
		if( mJumpRemainTime <= mpObjectSceneNode->GetScaleAccumTime() )
		{
			SetJumpFlag( false );

			if( GetState() == eOBJECT_STATE_MOVE )
			{
				float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
				float moveSpeed = 1.0f;
				if( baseSpeed != 0.0f )
				{
					moveSpeed = (float)mPlayerExrInfo.mMoveSpeed / (float) baseSpeed;
				}

				if( mpObjectSceneNode )
					mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );
			}
		}
	}


	/// 카메라 처리
	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	if( pCamera )
	{
		const NiPoint3& pos = mpObjectSceneNode->GetTranslate();
		pCamera->SetLookAt( pos.x, pos.y, pos.z + mCameraOffset );

		///
		WORLDMAN->MoveSkyDome( pCamera->GetWorldTranslate() );
	}
}

void cHero::ProcessAfterAction( NiPoint3& heroPos )
{
	if( mAfterAction.mActionType == eAfterAction_None )
		return;

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mAfterAction.mTarget );
	NiPoint3 targetPos = mTargetPos;
	if( pTarget )
		targetPos = pTarget->GetPos();
	else
	{
		if( WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y ) == false )
			assert(NULL);
	}

	NiPoint3 heroNotZPos = heroPos;
	heroNotZPos.z = 0.0f;
	NiPoint3 targetNotZPos = targetPos;
	targetNotZPos.z = 0.0f;

	/// 범위 검색이 기본..
	switch( mAfterAction.mActionType )
	{
	case eAfterAction_Attack:
		{	
			if( pTarget == 0 )
			{
				assert(0);
				mAfterAction.mActionType = eAfterAction_None;
				SetState( eOBJECT_STATE_IDLE );
				return;
			}

			bool bAttack = false;
			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );
			/// 상대방이 움직여서 내범위로 들어온 경우를 찾는다.
			NiPoint3 curPos = HERO->GetPos();
			if( rangecheck.IsRange( curPos, targetPos ) )
			{
				/// 이미 상대방이 내범위에 들어온 경우 멈춘다.
				heroPos.x = curPos.x;
				heroPos.y = curPos.y;

				bAttack = true;
			}
			else
			{
				/// 이동후 좌표로 범위 검사를 한다.
				if( rangecheck.IsRange( heroPos, targetPos ) )
				{
					NiPoint3 dir = heroPos - targetPos;
					if( dir.Unitize() != 0 )
					{
						/// heroPos 보정
						heroPos.x = targetPos.x + ((tempRange-1.0f)*dir.x);
						heroPos.y = targetPos.y + ((tempRange-1.0f)*dir.y);
					}

					bAttack = true;
				}
			}

			if( bAttack == true )
			{
				/// 공격 처리
				if( IsReadyUseSkill() )
				{
					/// 후처리 종료
					StopAfterAction();
					if( StartAttack( mAfterAction.mExData1, OBJECTMAN->GetObject( &mAfterAction.mTarget ) ) == false )
					{
						/// 공격 상태 유지
						SetState( eOBJECT_STATE_ATTACK );
						SetReadyUseSkill( true );
					}
				}
				else
				{
					/// 후처리 종료 , 공격 상태 유지
					mAfterAction.mActionType = eAfterAction_None;
					SetState( eOBJECT_STATE_ATTACK );
					SetReadyUseSkill( true );
				}
				return;
			}
		}
		break;
	case eAfterAction_UseTargetSkill:
		{
			if( pTarget == 0 )
			{				
				/// PKH 대상이 사라지는경우 정지하고서버에도 알린다.
				MSG_REQ_MOVE_STOP Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_ACTIONMOVE_STOP_REQ;
				Msg.mClientPosX = GetXPos();
				Msg.mClientPosY = GetYPos();
				Msg.mClientTime = mSumKeyMoveDeltaTime;
				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

				mAfterAction.mActionType = eAfterAction_None;
				SetState( eOBJECT_STATE_IDLE );
				return;
			}

			bool bAttack = false;
			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );

			/// 상대방이 움직여서 내범위로 들어온 경우를 찾는다.
			NiPoint3 curPos = HERO->GetPos();
			if( rangecheck.IsRange( curPos, targetPos ) )
			{
				/// 이미 상대방이 내범위에 들어온 경우 멈춘다.
				heroPos.x = curPos.x;
				heroPos.y = curPos.y;

				bAttack = true;
			}
			else
			{
				/// 이동후 좌표로 범위 검사를 한다.
				if( rangecheck.IsRange( heroPos, targetPos ) )
				{
					NiPoint3 dir = heroPos - targetPos;
					if( dir.Unitize() != 0 )
					{
						/// heroPos 보정
						heroPos.x = targetPos.x + ((tempRange-1.0f)*dir.x);
						heroPos.y = targetPos.y + ((tempRange-1.0f)*dir.y);
					}

					bAttack = true;
				}
			}

			if( bAttack == true )
			{
				/// 후처리 종료
				StopAfterAction();

				/// 공격 시작
				if( IsReadyUseSkill() )
				{
					UseTargetSkill( mAfterAction.mExData1, (unsigned char)mAfterAction.mExData2, OBJECTMAN->GetObject(&mAfterAction.mTarget) );
				}

				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
				return;
			}
		}
		break;
	case eAfterAction_UseFieldSkill:
		{
			bool bAttack = false;
			cRangeCheck rangecheck( mAfterAction.mRange );

			/// 상대방이 움직여서 내범위로 들어온 경우를 찾는다.
			NiPoint3 curPos = HERO->GetPos();
			if( rangecheck.IsRange( curPos, mAfterAction.mDestPos ) )
			{
				/// 이미 상대방이 내범위에 들어온 경우 멈춘다.
				heroPos.x = curPos.x;
				heroPos.y = curPos.y;

				bAttack = true;
			}
			else
			{
				/// 이동후 좌표로 범위 검사를 한다.
				if( rangecheck.IsRange( heroPos, mAfterAction.mDestPos ) )
				{
					NiPoint3 dir = heroPos - mAfterAction.mDestPos;
					if( dir.Unitize() != 0 )
					{
						/// heroPos 보정
						heroPos.x = mAfterAction.mDestPos.x + ((mAfterAction.mRange-1.0f)*dir.x);
						heroPos.y = mAfterAction.mDestPos.y + ((mAfterAction.mRange-1.0f)*dir.y);
					}

					bAttack = true;
				}
			}

			if( bAttack == true )
			{
				/// 후처리 종료
				StopAfterAction();

				/// 공격 시작
				UseFieldSkill( mAfterAction.mExData1, (unsigned char)mAfterAction.mExData2, mAfterAction.mDestPos );

				mWaitSkill.skillIdx = 0;
				mWaitSkill.skillStep = 0;
				return;
			}
		}
		break;
	case eAfterAction_ItemPick:
		{
#ifdef _GMTOOL
			if( CHEATMAN->GetHideMode() )
				return;
#endif

			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, 0, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );

			if( rangecheck.IsRange( heroNotZPos, mAfterAction.mDestPos ) )
			{
				StopAfterAction();

				if( IsJumping() == true )
					return;

				if( IsRideVehicle() == true )
				{
					MSGROOT msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_PLAYER;
					msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					AlightVehicle();
				}

				/// 루팅윈도우 닫기 
				GAMEUI->HideLootingWindow();

				if( ITEMMAN->IsLocked() == true )
					return;

				MSG_REQ_ITEM_GET_OPEN msg;
				msg.Category = NM_ITEM;
				msg.Protocol = NM_ITEM_GET_OPEN_REQ;
				msg.idx = mAfterAction.mExData1;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				SetState( ePLAYER_STATE_ITEMPICK );

				/// 락 걸기
				ITEMMAN->LockItemTry( ITEM_TRY_PICK );
			}

			return;
		}
		break;
	case eAfterAction_GatheringPick:
		{
#ifdef _GMTOOL
			if( CHEATMAN->GetHideMode() )
				return;
#endif

			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, 0, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );
			if( rangecheck.IsRange( heroNotZPos, mAfterAction.mDestPos ) )
			{
				StopAfterAction();
				GatheringStart( mAfterAction.mExData1, mAfterAction.mExData2, mAfterAction.mDestPos );
			}
			return;
		}
		break;
	case eAfterAction_NpcTalk:
		{
			if( pTarget == 0 || pTarget->GetObjectType() != eOBJECTTYPE_NPC )
			{
				assert(0);
				mAfterAction.mActionType = eAfterAction_None;
				SetState( eOBJECT_STATE_IDLE );
				return;
			}

			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );

			if( rangecheck.IsRange( heroNotZPos, targetNotZPos ) )
			{
				StopAfterAction();

				if( IsJumping()  == true )
					return;

				MSG_REQ_NPC_OPEN Msg;
				Msg.Category = NM_NPC;
				Msg.Protocol = NM_NPC_OPEN_REQ;
				Msg.mNpcIdx = pTarget->GetObjectID();
				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}

			return;
		}
		break;
	case eAfterAction_UseTarotStore:
		{
			if( pTarget == 0 || pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				StopAfterAction();
				return;
			}

			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );

			if( rangecheck.IsRange( heroNotZPos, targetNotZPos ) )
			{
				StopAfterAction();

				/// 타롯상점 정보 요청
				MSG_REQ_TAROT_SEEKER_OPEN msg;				// MSG_REQ_TAROT_SEEKER_JOIN msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SEEKER_OPEN_REQ;	// NM_TAROT_SEEKER_JOIN_REQ
				msg.CharacterIdx = pTarget->GetObjectID();
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			return;
		}
		break;
	case eAfterAction_UseStall:
		{
			if( pTarget == 0 || pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				StopAfterAction();
				return;
			}

			if( mAfterAction.mTarget.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				StopAfterAction();
				return;
			}

			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mAfterAction.mRange );

			cRangeCheck ranceCheck( tempRange );

			if( ranceCheck.IsRange( heroNotZPos, targetNotZPos ) )
			{
				StopAfterAction();

				/// 노점상 열기 요청
				ITEMMAN->TryToUserSellJoin( mAfterAction.mTarget );
			}
		}
		break;
	case eAfterAction_MapChange:
		{
			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, 0, mAfterAction.mRange );

			cRangeCheck rangecheck( tempRange );

			if( rangecheck.IsRange( heroNotZPos, mAfterAction.mDestPos ) )
			{
				StopAfterAction();

				sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( (unsigned long)mAfterAction.mExData2 );
				assert(pInfo);
				if( pInfo )
				{
					if( pInfo->mFirstRegen == false )
					{
						if( pInfo->mMapNumber != GetMapNum() )
						{
							assert(0);
							return;
						}

						/// same map / position change
						MSG_REQ_TELEPORTPORTAL_USE Msg;
						Msg.Category = NM_TELEPORTPORTAL;
						Msg.Protocol = NM_TELEPORTPORTAL_USE_REQ;
						Msg.mMapChangePosIdx = (unsigned long)mAfterAction.mExData2;

						NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

						mMapPortalSkipTime = THEAPP->GetWorldAccumTime() + 1000;

						SetStopFlag( eSTOP_TELEPORTPORTAL );
					}
					else
					{
						if( mAfterAction.mExData1 != 0 )
						{
							TCHAR str[256] = {0,};
							_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mMapNumber) );

							///
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_USERMAPCHANGE, eSKIN_YESNO,
								str,
								GAMERESOURCEMAN->GetGameText( 59 ),
								(void*)mAfterAction.mExData1 );
						}
						else
						{
							if( pInfo->mTargetInfo )
							{
								TCHAR str[256] = {0,};
								_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mTargetInfo->mMapNumber) );

								///
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_MAPCHANGE, eSKIN_YESNO,
									str,
									GAMERESOURCEMAN->GetGameText( 59 ),
									(void*)mAfterAction.mExData2 );
							}
						}
					}
				}
			}
			return;
		}
		break;
	default:
		return;
	}

	/*if( mAfterAction.mActionType == eAfterAction_MapChange )
	return;*/

	/// 타겟의 좌표가 변하는 경우 변화량에 의존한 타겟좌표 재설정
	if( pTarget )
	{
		if( mTargetPos != targetNotZPos )
		{
			float range = (heroNotZPos - mTargetPos).Length() * 0.10f;
			if( range < 100.0f )
				range = 100.0f;

			cRangeCheck r( range );
			if( r.IsRange( targetNotZPos, mTargetPos ) == false )
			{
				/// 제한 변화량 이상으로 타겟이 움직였다면..
				/// 객체의 고정 이동좌표를 바꾼다.
				if( ResetActionMovePath( targetNotZPos.x, targetNotZPos.y, mAfterAction.mRange ) == false )
				{
					assert(0);
				}
			}
		}
	}
}

bool cHero::Move( float x, float y )
{
	if( x < 0 || y < 0 )
	{
		assert(0);
		return false;
	}

	/// 게이트 검사
	float goX = x;
	float goY = y;
	if( EnableGateMovePos( NiPoint2(GetPos().x, GetPos().y), goX, goY ) == false )
	{
		SetState( eOBJECT_STATE_IDLE );
		return false;
	}

	/// 이동.회전보간 - 도착지점 셋팅
	if( !cPlayer::Move( goX, goY ) )
	{
		return false;
	}

	/// 패킷날리기
	MSG_REQ_MOVE Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_MOVE_REQ;
	Msg.startX = GetXPos();
	Msg.startY = GetYPos();
	Msg.destX = goX;
	Msg.destY = goY;
	Msg.mClientTime = mSumKeyMoveDeltaTime;
	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	mSumKeyMoveDeltaTime = 0;
	return true;
}

bool cHero::ActionMove( float x, float y, float range )
{
	if(x < 0 || y < 0)
	{
		assert(0);
		return false;
	}

	///
	float goX = x;
	float goY = y;
	if( EnableGateMovePos( NiPoint2(GetPos().x, GetPos().y), goX, goY ) == false )
	{
		SetState( eOBJECT_STATE_IDLE );
		return false;
	}

	/// 이동.회전보간 - 도착지점 셋팅
	if( !cPlayer::Move( x, y ) )
	{
		return false;
	}

	/// 패킷날리기
	MSG_REQ_ACTIONMOVE Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_ACTIONMOVE_REQ;
	Msg.startX = GetXPos();
	Msg.startY = GetYPos();
	Msg.destX = goX;
	Msg.destY = goY;
	Msg.targetX = mTargetPos.x;
	Msg.targetY = mTargetPos.y;
	Msg.range = range;
	Msg.targetInfo = mAfterAction.mTarget;
	Msg.mClientTime = mSumKeyMoveDeltaTime;

	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	mSumKeyMoveDeltaTime = 0;

	return true;
}

/// 새로운 path를 생성하고 첫번째 path로 이동..
bool cHero::ResetMovePath( NiPoint2 start, NiPoint2 goal )
{
	if( IsReadyUseSkill() == false )
	{
		/// 스킬의 현재 상태가 캐스팅중이고 이동가능한지 체크한다.
		cSkillObject* p = SKILLEXECUTER->GetCurrentHeroSkillObject();
		if( p && p->GetDramaState() == eDRAMASTATE_CASTING )
		{
			cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pWin )
			{
				if( pWin->GetPercent() < 98.0f )
				{
					/// cast stop
					p->WaitCancel();
					pWin->Clear( eCasting_Skill );
					HERO->SetState( eOBJECT_STATE_IDLE );

					/// 070712 PKH 스킬 요청 메세지 발송
					MSG_REQ_SKILL_CAST_CANCEL Msg;
					Msg.Category = NM_SKILL;
					Msg.Protocol = NM_SKILL_CAST_CANCEL_REQ;
					Msg.mAttacker.index = HERO->GetObjectID();
					Msg.mAttacker.type = HERO->GetObjectType();

					NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
				}
			}

			// 취소 처리
			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else if( p && p->GetDramaState() == eDRAMASTATE_ACTIVITY && p->IsChanneling() )
		{
			cCastingBar* pCastingWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pCastingWin )
			{
				pCastingWin->Clear( eCasting_Skill );
			}

			p->WaitCancel();
			HERO->SetState( eOBJECT_STATE_IDLE );

			/// 070712 PKH 스킬 요청 메세지 발송
			MSG_REQ_CHANNELING_SKILL_CANCEL Msg;
			Msg.Category = NM_SKILL;
			Msg.Protocol = NM_SKILL_CHANNELING_CANCEL_REQ;
			Msg.mAttacker.index = HERO->GetObjectID();
			Msg.mAttacker.type = HERO->GetObjectType();

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else
		{
			mWaitMovePos.x = goal.x;
			mWaitMovePos.y = goal.y;
			mWaitMoveFlag = eWAITMOVE_MOUSE;

			return false;
		}
	}

	if( GatheringCancel() == false )
		return false;

	if( ItemMixCancel() == false )
		return false;

	if( ItemEnhanceCancel() == false )
		return false;

	if( NpcSpeechCancel() == false )
		return false;

	/// 이동 불가
	if( IsCantMove() == true )
		return false;

	/// 거리 제한
	if( mIsFindPosition == false )
	{
		if( (start - goal).Length() > (float)MAX_PATH_DIST )
			return false;
	}

	///
	mAfterAction.mActionType = eAfterAction_None;

	/// 길을 찾아보세~
	if( mPathFinder->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, start, goal ) == false )
		return false;

	///
	mPathIndex = 0;
	mTargetPos.x = goal.x;
	mTargetPos.y = goal.y;
	mTargetPos.z = 0.0f;

	float z = 0.0f;
	if( WORLDMAN->CalcHeight( &z, mTargetPos.x, mTargetPos.y ) )
	{
		if( mMoveTargetEffectIdx != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mMoveTargetEffectIdx );
			if( node )
			{
				assert( node == mMoveTargetEffect );
				node->Remove();
			}
			mMoveTargetEffect = 0;
			mMoveTargetEffectIdx = (unsigned long)-1;
		}

		if( mIsFollowing == false )
		{
			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = mTargetPos;
			trans.m_Translate.z = z;
			trans.m_fScale = 1.0f;

			mMoveTargetEffect = SCENEMAN->CreateSelfEffect( "./Data/Effect/Eff_PC_Target_03.nif", false, trans, true );
			if( mMoveTargetEffect )
				mMoveTargetEffectIdx = mMoveTargetEffect->GetIndexByManger();
		}
	}
	else
	{
		assert(0);
	}

	/// 이동
	Move( mPathArray[0].x, mPathArray[0].y );
	return true;
}

bool cHero::ResetActionMovePath( float destX, float destY, float range )
{
	if( GatheringCancel() == false )
		return false;

	if( NpcSpeechCancel() == false )
		return false;

	/// 이동 불가
	if( IsCantMove() == true )
	{
		return false;
	}

	if( range == 0.0f )
	{
		assert(0);
		return ResetMovePath( NiPoint2(GetXPos(), GetYPos()), NiPoint2(destX, destY) );
	}

	if( IsReadyUseSkill() == false )
	{
		mWaitMovePos.x = destX;
		mWaitMovePos.y = destY;
		mWaitMoveFlag = eWAITMOVE_MOUSE;

		return true;
	}

	/// 거리 제한
	NiPoint2 start( GetXPos(), GetYPos() );
	NiPoint2 goal( destX, destY );

	if( (start - goal).Length() > 30000.0f )
		return false;

	/// 길을 찾아보세~
	if( mPathFinder->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, start, goal ) == false )
	{
		return false;
	}

	///
	mPathIndex = 0;
	mTargetPos.x = destX;
	mTargetPos.y = destY;
	mTargetPos.z = 0.0f;

	if( mMoveTargetEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mMoveTargetEffectIdx );
		if( node )
		{
			assert( node == mMoveTargetEffect );
			node->Remove();
		}
		mMoveTargetEffect = 0;
		mMoveTargetEffectIdx = (unsigned long)-1;
	}

	/// 이동
	return ActionMove( mPathArray[0].x, mPathArray[0].y, range );
}

void cHero::StopAfterAction()
{
	mAfterAction.mActionType = eAfterAction_None;
	SetState( eOBJECT_STATE_IDLE );

	mWaitSkill.skillIdx = 0;
	mWaitSkill.skillStep = 0;
}

bool cHero::StartAttack( unsigned long attackIdx, cBaseObject* pTarget )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return false;
#endif
	if( IsJumping() == true )
		return false;

	if( IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		AlightVehicle();
	}

	/// 공격이 시작되는 시점( 자동 등록 제거 )
	AutoActionClear();

	if( GetState() == eOBJECT_STATE_DIE )
		return false;

	if( IsWeaponChanging() == true )
		return false;

	if( IsReadyUseSkill() == false )
		return false;

	if( pTarget == 0 )
	{
		assert(0);
		return false;
	}

	/// 바라봐야할 방향 설정
	NiPoint3 charDir = mDesiredDir;
	NiPoint3 viewDir = pTarget->GetPos() - GetPos();
	viewDir.z = 0.0f;
	if( viewDir != NiPoint3::ZERO )
	{
		if( viewDir.SqrLength() >= 0.001f )	
		{
			viewDir.Unitize();

			/// 회전 Matrix 산출
			NiMatrix3 matA, matB, matRot;
			matA.SetCol( 0, charDir );
			matA.SetCol( 1, -NiPoint3::UNIT_Z );
			matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matB.SetCol( 0, viewDir );
			matB.SetCol( 1, -NiPoint3::UNIT_Z );
			matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matRot = matB * matA.Transpose();
			mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
			mDesiredDir = viewDir;
		}
	}

	mAutoAttackFlag = true;

	if( IsTransformMonster() )
		return StartAttack_Mon( attackIdx, pTarget );

	/// 쿨타임 체크
	if( SKILLMAN->IsUsedKeepSkill( NORMAL_ATTACK_SKILL, 0 ) == false )
	{
		return false;
	}

	/// 기본 스킬 생성..
	unsigned long key = SKILLMAN->CreateHeroSkillObject( attackIdx, 0, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		/// 상태 설정
		SetState( eOBJECT_STATE_ATTACK );
		SetReadyUseSkill( false );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_USED Msg;
		Msg.Category = NM_SKILL;
		Msg.Protocol = NM_SKILL_USED_REQ;
		Msg.mTarget.index = pTarget->GetObjectID();
		Msg.mTarget.type = pTarget->GetObjectType();

		Msg.mSkillClassIndex = attackIdx;
		Msg.mSkillStep = 0;

		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}

	return true;
}

void cHero::UseTargetSkill( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif
	if( IsJumping() == true )
		return;

	if( IsTransformMonster() )
	{
		UseTargetSkill_TransMon( skillIdx, pTarget );
		return;
	}
	else
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}
	}

	AutoActionClear();
	SKILLEXECUTER->ChargeOff( skillIdx, skillStep );

	if( IsWeaponChanging() == true )
		return;

	/// 적용 타겟 검증
	if( SKILLEXECUTER->IsPossibleUseSkillTarget( skillIdx, pTarget ) == false )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	if( pTarget )
	{
		/// 바라봐야할 방향 설정
		NiPoint3 charDir = mDesiredDir;
		NiPoint3 viewDir = pTarget->GetPos() - GetPos();
		viewDir.z = 0.0f;
		if( viewDir != NiPoint3::ZERO )
		{
			if( viewDir.SqrLength() >= 0.001f )	
			{
				viewDir.Unitize();

				/// 회전 Matrix 산출
				NiMatrix3 matA, matB, matRot;
				matA.SetCol( 0, charDir );
				matA.SetCol( 1, -NiPoint3::UNIT_Z );
				matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matB.SetCol( 0, viewDir );
				matB.SetCol( 1, -NiPoint3::UNIT_Z );
				matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matRot = matB * matA.Transpose();
				mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
				mDesiredDir = viewDir;
			}
		}
	}

	unsigned long key = SKILLMAN->CreateHeroSkillObject( skillIdx, skillStep, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		mAutoAttackFlag = true;

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pInfo )
		{
			if( mMageJobType == true )
			{
				mAutoAttackFlag = false;
			}
			else 
			{
				if( GetState() != eOBJECT_STATE_ATTACK )
				{
					// 발생후 자동공격 하지 않게 한다
					if( pInfo->mType == eSKILLTYPE_PASSIVE || 
						pInfo->mType == eSKILLTYPE_HEAL || 
						pInfo->mType == eSKILLTYPE_TOTEM )
					{
						mAutoAttackFlag = false;
					}
					else if( pInfo->mType == eSKILLTYPE_NONACTIVE )
					{
						if( pInfo->mApplyType != eAPPLYTYPE_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
						{
							mAutoAttackFlag = false;
						}
					}
				}
			}
		}

		if( pInfo->mType == eSKILLTYPE_NONSTOP )
		{
			mAutoAttackFlag = false;
		}
		else
		{
			/// 상태 설정
			SetState( eOBJECT_STATE_ATTACK );
			SetReadyUseSkill( false );
		}


		if( mShowWeapon == false )
			ShowWeapon( true );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_USED Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		if( SKILLSCRIPT->IsCastSkill( skillIdx, 0 ) == true )
			Msg.Protocol = NM_SKILL_CAST_USED_REQ;
		else
		{
			if( pInfo->mType == eSKILLTYPE_NONSTOP )
			{
				if( pInfo->mShotType == eSHOTTYPE_RUSHRUN )
					Msg.Protocol = NM_SKILL_RUSH_REQ;
				else if ( pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
					Msg.Protocol = NM_SKILL_RUSHBLINK_REQ;
				else
					Msg.Protocol = NM_SKILL_NONSTOP_REQ;
			}
			else
				Msg.Protocol = NM_SKILL_USED_REQ;
		}

		Msg.mSkillClassIndex = skillIdx;
		Msg.mSkillStep = skillStep;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();


		if( pTarget )
		{
			Msg.mTarget.index = pTarget->GetObjectID();
			Msg.mTarget.type = pTarget->GetObjectType();
		}
		else
		{
			Msg.mTarget.index = 0;
			Msg.mTarget.type = eOBJECTTYPE_NONE;
		}

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::UseRushTargetSkill()
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif

	unsigned long skillIdx = mRushSkill;
	if( mRushSkill == 0 )
		return;

	unsigned char skillStep = mRushStep;

	mRushSkill = 0;
	mRushStep = 0;

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTarget );
	if( pTarget == NULL )
		return;

	if( IsJumping() == true )
		return;

	if( IsTransformMonster() )
	{
		return;
	}
	else
	{
		if( IsRideVehicle() == true )
		{
			return;
		}
	}

	AutoActionClear();
	SKILLEXECUTER->ChargeOff( skillIdx, skillStep );

	if( IsWeaponChanging() == true )
		return;

	/// 적용 타겟 검증
	if( SKILLEXECUTER->IsPossibleUseSkillTarget( skillIdx, pTarget ) == false )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsRushUseSkill( skillIdx, skillStep ) == false )
		return;

	if( pTarget )
	{
		/// 바라봐야할 방향 설정
		NiPoint3 charDir = mDesiredDir;
		NiPoint3 viewDir = pTarget->GetPos() - GetPos();
		viewDir.z = 0.0f;
		if( viewDir != NiPoint3::ZERO )
		{
			if( viewDir.SqrLength() >= 0.001f )	
			{
				viewDir.Unitize();

				/// 회전 Matrix 산출
				NiMatrix3 matA, matB, matRot;
				matA.SetCol( 0, charDir );
				matA.SetCol( 1, -NiPoint3::UNIT_Z );
				matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matB.SetCol( 0, viewDir );
				matB.SetCol( 1, -NiPoint3::UNIT_Z );
				matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matRot = matB * matA.Transpose();
				mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
				mDesiredDir = viewDir;
			}
		}
	}

	unsigned long key = SKILLMAN->CreateHeroSkillObject( skillIdx, skillStep, this, pTarget );
	if( key )
	{
		mAutoAttackFlag = true;

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pInfo )
		{
			if( mMageJobType == true )
			{
				mAutoAttackFlag = false;
			}
			else 
			{
				if( GetState() != eOBJECT_STATE_ATTACK )
				{
					// 발생후 자동공격 하지 않게 한다
					if( pInfo->mType == eSKILLTYPE_PASSIVE || 
						pInfo->mType == eSKILLTYPE_HEAL || 
						pInfo->mType == eSKILLTYPE_TOTEM )
					{
						mAutoAttackFlag = false;
					}
					else if( pInfo->mType == eSKILLTYPE_NONACTIVE )
					{
						if( pInfo->mApplyType != eAPPLYTYPE_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
						{
							mAutoAttackFlag = false;
						}
					}
				}
			}
		}

		if( pInfo->mType == eSKILLTYPE_NONSTOP )
		{
			mAutoAttackFlag = false;
		}
		else
		{
			/// 상태 설정
			SetState( eOBJECT_STATE_ATTACK );
			SetReadyUseSkill( false );
		}


		if( mShowWeapon == false )
			ShowWeapon( true );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_USED Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		if( SKILLSCRIPT->IsCastSkill( skillIdx, 0 ) == true )
		{
			return;
			//Msg.Protocol = NM_SKILL_CAST_USED_REQ;
		}
		else
		{
			if( pInfo->mType == eSKILLTYPE_NONSTOP )
			{
				return;
				//if( pInfo->mShotType == eSHOTTYPE_RUSH )
				//	return;
				//else
				//	Msg.Protocol = NM_SKILL_NONSTOP_REQ;
			}
			else
				Msg.Protocol = NM_SKILL_RUSHAFTER_REQ;
		}

		Msg.mSkillClassIndex = skillIdx;
		Msg.mSkillStep = skillStep;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		if( pTarget )
		{
			Msg.mTarget.index = pTarget->GetObjectID();
			Msg.mTarget.type = pTarget->GetObjectType();
		}
		else
		{
			Msg.mTarget.index = 0;
			Msg.mTarget.type = eOBJECTTYPE_NONE;
		}

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}

	return;
}

void cHero::UseBlinkSkill( unsigned long skillIdx, unsigned char skillStep, bool isForward )
{
	if( GatheringCancel() == false )
		return;

	if( NpcSpeechCancel() == false )
		return;

	if( IsReadyUseSkill() == false )
	{
		return;
	}

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 || pInfo->mBoundType == eBOUNDTYPE_FIELD )
	{
		assert(0);
		return;
	}

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif
	if( IsJumping() == true )
		return;

	if( IsTransformMonster() )
	{
		return;
	}
	else
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}
	}

	AutoActionClear();
	SKILLEXECUTER->ChargeOff( skillIdx, skillStep );

	if( IsWeaponChanging() == true )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == false )
		return;

	if( pInfo->mStepCount <= skillStep )
	{
		assert(0);
		return;
	}

	float blinkDist = pInfo->mpSetpInfoArray[skillStep].mTargetDist;
	unsigned long key = SKILLMAN->CreateHeroBlinkSkillObject( skillIdx, skillStep, this );
	if( key )
	{
		/// 상태 설정
		SetState( eOBJECT_STATE_IDLE, false );
		SetReadyUseSkill( false );

		NiPoint3 heroPos = GetPos();
		heroPos.z = 0.0f;

		NiPoint3 blinkPos = heroPos;
		if( isForward )
			blinkPos = heroPos + mDesiredDir*blinkDist;
		else
			blinkPos = heroPos - mDesiredDir*blinkDist;

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_BLINK Msg;
		Msg.Category = NM_SKILL;
		Msg.Protocol = NM_SKILL_BLINK_REQ;

		Msg.mSkillClassIndex = skillIdx;
		Msg.mSkillStep = skillStep;
		Msg.mManagedKey = key;

		Msg.mTargetPosX = blinkPos.x;
		Msg.mTargetPosY = blinkPos.y;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::UseFieldSkill( unsigned long skillIdx, unsigned char skillStep, NiPoint3 fieldPos )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
		return;
#endif
	if( IsJumping() == true )
		return;

	if( IsTransformMonster() )
	{
		UseFieldSkill_TransMon( skillIdx, fieldPos );
		return;
	}
	else
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}
	}

	AutoActionClear();
	SKILLEXECUTER->ChargeOff( skillIdx, skillStep );

	if( IsWeaponChanging() == true )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	/// 바라봐야할 방향 설정
	NiPoint3 charDir = mDesiredDir;
	NiPoint3 viewDir = fieldPos - GetPos();
	viewDir.z = 0.0f;
	if( viewDir != NiPoint3::ZERO )
	{
		if( viewDir.SqrLength() >= 0.001f )	
		{
			viewDir.Unitize();

			/// 회전 Matrix 산출
			NiMatrix3 matA, matB, matRot;
			matA.SetCol( 0, charDir );
			matA.SetCol( 1, -NiPoint3::UNIT_Z );
			matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matB.SetCol( 0, viewDir );
			matB.SetCol( 1, -NiPoint3::UNIT_Z );
			matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matRot = matB * matA.Transpose();
			mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
			mDesiredDir = viewDir;
		}
	}

	unsigned long key = SKILLMAN->CreateHeroSkillObject( skillIdx, skillStep, this, fieldPos, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		mAutoAttackFlag = true;

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pInfo )
		{
			if( mMageJobType == true )
			{
				mAutoAttackFlag = false;
			}
			else 
			{
				if( GetState() != eOBJECT_STATE_ATTACK )
				{
					// 발생후 자동공격 하지 않게 한다
					if( pInfo->mType == eSKILLTYPE_PASSIVE || 
						pInfo->mType == eSKILLTYPE_HEAL || 
						pInfo->mType == eSKILLTYPE_TOTEM )
					{
						mAutoAttackFlag = false;
					}
					else if( pInfo->mType == eSKILLTYPE_NONACTIVE )
					{
						if( pInfo->mApplyType != eAPPLYTYPE_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
							pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
							mAutoAttackFlag = false;
					}
				}
			}
		}

		/// 상태 설정
		SetState( eOBJECT_STATE_ATTACK );
		SetReadyUseSkill( false );

		if( mShowWeapon == false )
			ShowWeapon( true );

		/// 070712 PKH 스킬 요청 메세지 발송

		MSG_REQ_POS_USED Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		if( SKILLSCRIPT->IsCastSkill( skillIdx, 0 ) == true )
			Msg.Protocol = NM_SKILL_POS_CAST_USED_REQ;
		else
			Msg.Protocol = NM_SKILL_POS_USED_REQ;

		Msg.mSkillClassIndex = skillIdx;
		Msg.mSkillStep = skillStep;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		Msg.mTargetPosX = fieldPos.x;
		Msg.mTargetPosY = fieldPos.y;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::UseMapChangeSkill( unsigned long skillIdx, unsigned short slotIdx )
{
	if( DUELMAN->IsDuelEnable() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(809) );
		return;
	}

	if( PVPMAN->IsEnableBattle() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(842) );
		return;
	}

	/// 자동사냥 신고 창이 뜬 도중이라면 무시
	cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
	if( camaelWin && camaelWin->IsVisible() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11817 ) );
		return;
	}

	if( IsJumping() == true )
		return;
	
	if( IsTransformMonster() == false )
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}
	}

	AutoActionClear();
	if( IsReadyUseSkill() == false )
		return;

	SetWaitMoveFlag( eWAITMOVE_NONE );

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );

	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		assert(0);
		return;
	}

	/// 상태 검사
	if( !(GetState() == eOBJECT_STATE_IDLE || GetState() == eOBJECT_STATE_MOVE) )
		return;

	if( IsWeaponChanging() == true )
		return;

	/// 스킬 정보를 획득한다.
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[0];
	if( pStepInfo == 0 )
	{
		assert(0);
		return;
	}

	/// 사용 가능 여부 체크
	if( pInfo->mUseItem )
	{
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
			return;
	}

	if( GetLevel() < pStepInfo->mPlayerLevel )
		return;

	/// 스킬 발동
	unsigned long key = SKILLMAN->CreateHeroMapChangeSkillObject( skillIdx, this );
	if( key )
	{
		SetState( eOBJECT_STATE_ATTACK );

		/// 상태 설정
		SetReadyUseSkill( false );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_ITEMMAPCHANGE Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		Msg.Protocol = NM_SKILL_ITEMMAPCHANGE_REQ;
		Msg.mSkillClassIndex = skillIdx;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		Msg.mTarget.index = HERO->GetObjectID();
		Msg.mTarget.type = eOBJECTTYPE_HERO;
		Msg.mSlotNumber = slotIdx;
		Msg.mUseItemIndex = pInfo->mUseItem;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

		ITEMMAN->LockItemTry( ITEM_TRY_MAPCHANGE, slotIdx );
	}
}

void cHero::UseCashMapChangeSkill( unsigned long skillIdx, unsigned short slotIdx, unsigned short mapIndex, float mapX, float mapY )
{

	if( DUELMAN->IsDuelEnable() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(809) );
		return;
	}

	if( PVPMAN->IsEnableBattle() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(842) );
		return;
	}

	if( IsJumping() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(12207) );
		return;
	}

	/// 자동사냥 신고 창이 뜬 도중이라면 무시
	cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
	if( camaelWin && camaelWin->IsVisible() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11817 ) );
		return;
	}

	if( IsTransformMonster() == false )
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}
	}

	AutoActionClear();
	if( IsReadyUseSkill() == false )
		return;

	SetWaitMoveFlag( eWAITMOVE_NONE );

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
	
	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		assert(0);
		return;
	}

	/// 상태 검사
	if( !(GetState() == eOBJECT_STATE_IDLE || GetState() == eOBJECT_STATE_MOVE) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12200 ) );
		return;
	}

	if( IsWeaponChanging() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12207 ) );
		return;
	}

	/// 스킬 정보를 획득한다.
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[0];
	if( pStepInfo == 0 )
	{
		assert(0);
		return;
	}

	/// 사용 가능 여부 체크
	if( pInfo->mUseItem )
	{
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12204 ) );
			return;
		}
	}

	cItem* item = ITEMMAN->GetItem( slotIdx );

	/// 스킬 발동
	unsigned long key = SKILLMAN->CreateHeroMapChangeSkillObject( skillIdx , this );
	if( key )
	{
		SetState( eOBJECT_STATE_ATTACK );

		/// 상태 설정
		SetReadyUseSkill( false );

		MSG_REQ_SKILL_CASHMAPCHANGE msg;
		msg.Category = NM_SKILL;
		msg.Protocol = NM_SKILL_CASHMAPCHANGE_REQ;
		msg.mMapNumber = mapIndex;
		msg.mPosX = mapX;
		msg.mPosY = mapY;

		msg.mSkillClassIndex = skillIdx ;
		msg.mManagedKey = key;
		msg.mClientPosX = HERO->GetXPos();
		msg.mClientPosY = HERO->GetYPos();

		msg.mTarget.index = HERO->GetObjectID();
		msg.mTarget.type = eOBJECTTYPE_HERO;
		msg.mSlotNumber = slotIdx;
		msg.mUseItemIndex = item->GetIndex();

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		ITEMMAN->LockItemTry( ITEM_TRY_MAPCHANGE, slotIdx );
	}
}

void cHero::UseVehicleSkill( unsigned long skillIdx, unsigned short slotIdx )
{
	AutoActionClear();

	if( IsTransformMonster() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 276 ) );
		return;
	}
	if( IsReadyUseSkill() == false )
		return;
	if( IsJumping() == true )
		return;

	SetWaitMoveFlag( eWAITMOVE_NONE );

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );

	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		assert(0);
		return;
	}

	/// 상태 검사
	if( !(GetState() == eOBJECT_STATE_IDLE || GetState() == eOBJECT_STATE_MOVE) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 242 ) );
		return;
	}

	//	if( IsWeaponChanging() == true )
	//		return;

	/// 스킬 정보를 획득한다.
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[0];
	if( pStepInfo == 0 )
	{
		assert(0);
		return;
	}

	if( IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		AlightVehicle();

		///
		if( IsBuffHave( pStepInfo->mInfulenceIdx ) == true )
			return;
	}

	/// 사용 가능 여부 체크
	if( pInfo->mUseItem )
	{
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
			return;
	}

	if( GetLevel() < pStepInfo->mPlayerLevel )
		return;

	/// 스킬 발동
	unsigned long key = SKILLMAN->CreateHeroVehicleSkillObject( skillIdx, this );
	if( key )
	{
		SetState( eOBJECT_STATE_ATTACK );

		/// 상태 설정
		SetReadyUseSkill( false );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_VEHICLE Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		Msg.Protocol = NM_SKILL_ITEMVEHICLE_REQ;
		Msg.mSkillClassIndex = skillIdx;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		Msg.mTarget.index = HERO->GetObjectID();
		Msg.mTarget.type = eOBJECTTYPE_HERO;
		Msg.mSlotNumber = slotIdx;
		Msg.mUseItemIndex = pInfo->mUseItem;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

		ITEMMAN->LockItemTry( ITEM_TRY_VEHICLE, slotIdx );
	}
}


void cHero::AutoActionClear()
{
	mWaitMoveFlag = eWAITMOVE_NONE;

	mWaitSkill.skillIdx = 0;
	mWaitSkill.skillStep = 0;

	mAfterAction.mActionType = eAfterAction_None;
}


bool cHero::IsOpenNpcTalk()
{
	if( mNpcIdx == 0 )
		return false;

	cNpc* pNpc = (cNpc*)OBJECTMAN->GetNpc( mNpcIdx );
	if( !pNpc )
		return false;
	
	// npc 타입에 따라 분류
	sNPCList* npcInfo = NPCSCRIPT->GetNPCList( pNpc->GetClassId() );
	if( !npcInfo )
	{
		assert(0);
		return false;
	}

	return (npcInfo->mType == eNPCTYPE_NORMAL);
}

void cHero::OpenNpcTalk( unsigned long npcIndex, bool setUI )
{
	if( mNpcIdx != 0 )
	{
		assert(0);
	}

	// 정보 세팅
	mNpcIdx = npcIndex;
	SetStopFlag( eSTOP_NPCSPEECH );

	// UI 세팅
	if( setUI == true )
	{
		GAMEUI->ShowNpcBackWindow();
	}
}

void cHero::CloseNpcTalk( bool sendMsg )
{
	if( mNpcIdx == 0 )
		return;

	// 초기화
	cNpc* npc = OBJECTMAN->GetNpc( mNpcIdx );
	if( npc )
		npc->EndSpeech();

	mNpcFuncType = NPC_FUNC_MAX;
	mNpcIdx = 0;

	// 
	if( sendMsg == true )
	{
		if( HERO->GetState() != eOBJECT_STATE_DIE )
		{
			SetStopFlag( eSTOP_NONE );

			MSGROOT reqMsg;
			reqMsg.Category = NM_NPC;
			reqMsg.Protocol = NM_NPC_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&reqMsg, sizeof(reqMsg) );
		}
	}

	// UI 세팅
	GAMEUI->HideNpcBackWindow();
	UIMAN->CloseByNpcHide();

	UIMAN->GetRootNode()->ShowAllChild();
}


void cHero::CloseBySummonItem(  char nCategory, char nProtocal )
{
	if( GetStopFlag()		== eSTOP_SUMMON_PAPER	&& 
		GetNpcFunc()		== NPC_FUNC_MAX			&&
		GetState()			!= eOBJECT_STATE_DIE	)
	{
		SetStopFlag( eSTOP_NONE );

		MSGROOT reqMsg;
		reqMsg.Category = nCategory;
		reqMsg.Protocol = nProtocal;
		
		NETWORK->SendNetworkMsg( (char*)&reqMsg, sizeof(reqMsg) );
	}
}


/// 애니메이션 관계 처리
void cHero::Interpret( unsigned long time )
{
	if( mpObjectSceneNode->GetType() == cSceneNode::eVEHICLE )
	{
		return InterpretVehicle( time );
	}

	cActorManagerForPartition*	pActor = mpObjectSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpObjectSceneNode->GetTargetAnimation();
	float checkTime = (float)mpObjectSceneNode->GetScaleAccumTime() * 0.001f;

	float endTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;

	if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		endTime = mpObjectSceneNode->GetTargetAnimationScaleEndTime();
	}

	InterpretEvent( id, checkTime, pActor );

	if( endTime > 0 && endTime <= checkTime )
	{
		if( id >= ANITYPE_IDLE && id < ANITYPE_IDLE2 )
		{
			return;
		}
		else if( id >= ANITYPE_IDLE2 && id < ANITYPE_RUN )
		{
			ChangeAnimation( ANITYPE_IDLE );
		}
		else if( id >= ANITYPE_VEHICLE_IDLE2 && id < ANITYPE_VEHICLE_RUN )
		{
			ChangeAnimation( ANITYPE_IDLE );
		}
		else if( id>= ANITYPE_RUN && id < ANITYPE_ATTACK1 )
		{
			return;
		}
		else if( id >= ANITYPE_JUMP && id < ANITYPE_JUMP_END )
		{
			SetJumpFlag( false );

			if( IsGoContinue() == false )
			{
				/// key state check
				unsigned int keyFlag = MOVE_NONE;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
					keyFlag |= MOVE_W;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
					keyFlag |= MOVE_S;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
					keyFlag |= MOVE_A;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
					keyFlag |= MOVE_D;

				if( keyFlag != 0 )
					UpdateKeyState();
			}

			switch( GetState() )
			{
			case eOBJECT_STATE_IDLE:
			case eOBJECT_STATE_STOP:
				{
					if( mPlayerExrInfo.mStateIdle == eIDLE_PEACE )
					{
						if( IsRideVehicle() )
							ChangeAnimation( ANITYPE_IDLE );
						else
							ChangeAnimation( ANITYPE_PEACE_IDLE );
					}
					else
						ChangeAnimation( ANITYPE_IDLE );
				}
				break;
			case eOBJECT_STATE_MOVE:
				SetState( eOBJECT_STATE_MOVE );
				break;
			case eOBJECT_STATE_DIE:
				ChangeAnimation( ANITYPE_DIE1 );
				break;
			case ePLAYER_STATE_SITDOWN:
				ChangeAnimation( ANITYPE_SITDOWN_IDLE );
				break;
			default:
				ChangeAnimation( ANITYPE_IDLE );
				break;
			}
			ShowWeapon( true );
		}
		else if( id >= ANITYPE_ATTACK1 && id < ANITYPE_CASTING1 )
		{
			SetReadyUseSkill( true );
			ChangeAnimation( ANITYPE_IDLE );
		}
		else if( (id >= ANITYPE_COMMON_SKILL_BEGIN && id < ANITYPE_COMMON_SKILL_END) ||
			(id >= ANITYPE_SKILL_BEGIN && id < ANITYPE_SKILL_END) )
		{
			if( mMageJobType )
			{
				/// switch weapon check
				if( mWaitWeaponSwitch )
				{
					if( HERO->IsWeaponChanging() == false )
					{
						if( ITEMMAN->TryToSwitchWeapon() )
							HERO->SetWeaponChangeFlag( true );
					}
					mWaitWeaponSwitch = false;
				}

				SetReadyUseSkill( true );

				if( mWaitSkill.skillIdx > NORMAL_ATTACK_SKILL_MAX )
					ChangeAnimation( ANITYPE_IDLE );
				else
					SetState( eOBJECT_STATE_IDLE );
			}
			else
			{
				SetReadyUseSkill( true );
				ChangeAnimation( ANITYPE_IDLE );
			}
		}
		else if( mPlayerExrInfo.mState == ePLAYER_STATE_ITEMPICK )
		{
			SetRemoveDropItem( UINT_MAX );
			SetState( eOBJECT_STATE_IDLE );
			ShowWeapon( true );
		}
		else if( id == ANITYPE_PEACE_IDLE2 )
		{
			ChangeAnimation( ANITYPE_PEACE_IDLE );
		}
		else if( id >= ANITYPE_DAMAGE1 && id < ANITYPE_DIE1 )
		{
			if( !(ANITYPE_CASTING1 <= mDamageAfterAnim && mDamageAfterAnim < ANITYPE_DAMAGE1) )
			{
				SetReadyUseSkill( true );
				ChangeAnimation( ANITYPE_IDLE );
			}
			else
			{
				ChangeAnimation( (ePLAYER_ANIMATIONTYPE)mDamageAfterAnim );
			}
			ShowWeapon( true );
		}
		else if( id == ANITYPE_STANDUP )
		{
			SetState( eOBJECT_STATE_IDLE );
		}
		else if( id == ANITYPE_SITDOWN )
		{
			ChangeAnimation( ANITYPE_SITDOWN_IDLE );
			mSitDownFlag = false;
		}
		else if( id >= ANITYPE_EMOTE_START && id < ANITYPE_VEHICLE_IDLE )
		{
			SetState( eOBJECT_STATE_IDLE );
			ShowWeapon( true );
		}
		else if( id >= ANITYPE_ENHANCEDSUCCESS && id <= ANITYPE_ENHANCEDFAIL )
		{
			ChangeAnimation( ANITYPE_IDLE );
			ShowWeapon( true );
		}
		else if( id == ANITYPE_DUELWIN || id == ANITYPE_DUELLOSE )
		{
			SetState( eOBJECT_STATE_IDLE );
			ShowWeapon( true );
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		return;
	}
}


void cHero::InterpretVehicle( unsigned long time )
{
	cActorManagerForPartition*	pActor = mpObjectSceneNode->GetVehicleActorManager();
	if( !pActor ) return;

	unsigned int id = mpObjectSceneNode->GetVehicleTargetAnimation();

	float checkTime = (float)mpObjectSceneNode->GetScaleAccumTime() * 0.001f;

	float endTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;

	if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		endTime = mpObjectSceneNode->GetTargetAnimationScaleEndTime();
	}

	InterpretEvent_Vehicle( id, checkTime, pActor );

	if( endTime > 0 && endTime <= checkTime )
	{
		if( V_ANITYPE_IDLE2 == id )
		{
			mpObjectSceneNode->SetTargetAnimation( ANITYPE_IDLE );
		}
		else if( V_ANITYPE_JUMP == id )
		{
			SetJumpFlag( false );

			if( IsGoContinue() == false )
			{
				/// key state check
				unsigned int keyFlag = MOVE_NONE;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
					keyFlag |= MOVE_W;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
					keyFlag |= MOVE_S;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
					keyFlag |= MOVE_A;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
					keyFlag |= MOVE_D;

				if( keyFlag != 0 )
					UpdateKeyState();
			}

			switch( GetState() )
			{
			case eOBJECT_STATE_IDLE:
			case eOBJECT_STATE_STOP:
				ChangeAnimation( ANITYPE_IDLE );
				break;
			case eOBJECT_STATE_MOVE:
				ChangeAnimation( ANITYPE_RUN );
				break;
			case eOBJECT_STATE_DIE:
				ChangeAnimation( ANITYPE_DIE1 );
				break;
			case ePLAYER_STATE_SITDOWN:
				ChangeAnimation( ANITYPE_SITDOWN_IDLE );
				break;
			default:
				ChangeAnimation( ANITYPE_IDLE );
				break;
			}
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
	}
}


void cHero::InterpretEvent( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

	unsigned int soundIdx = 0;
	if( id >= ANITYPE_RUN && id < ANITYPE_ATTACK1 )
	{
		/// 오른발 소리 체크
		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] == 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
			}
			else
			{
				///
				if( eventTime2 > eventTime1 )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
				else
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_RUN_LEFT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_RIGHT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime;
		}
	}
	else if( id == ANITYPE_GATHERING1 || id == ANITYPE_GATHERING2 )
	{
		if( mRevisionEventTime[eANIEVENT_GATHER_MINING] > 0.0f && mRevisionEventTime[eANIEVENT_GATHER_MINING] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
			soundIdx = 2037;

			/// effect 
			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = NiPoint3( 0.0f, -100.0f, 30.0f );
			SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_collection_01.nif", &trans, false, true );

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Sound1" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
			{
				mRevisionEventTime[eANIEVENT_GATHER_MINING] = eventTime;
			}
		}
	}
	else if( id >=ANITYPE_JUMP && id < ANITYPE_JUMP_END )
	{
		mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
		mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
		mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;

		/// 
		if( mRevisionEventTime[eANIEVENT_JUMP_START] == 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime1;
			}
			else
			{
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime1;
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_START] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_START] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
			soundIdx = 2301;
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
			soundIdx = 2401;
			mJumpProcess = true;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime;
		}
	}
	else
	{
		memset( mRevisionEventTime, 0, sizeof(float)*eANIEVENT_MAX );
	}

	/// 이벤트 처리
	if( soundIdx )
		SetStepSound( soundIdx );
}

void cHero::InterpretEvent_Vehicle( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

	unsigned long soundIdx = 0;
	if( id == V_ANITYPE_RUN )
	{
		/// 오른발 소리 체크
		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] == 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
			}
			else
			{
				///
				if( eventTime2 > eventTime1 )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
				else
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_RUN_LEFT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_RIGHT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime;
		}
	}
	else if( id == V_ANITYPE_JUMP )
	{
		/// 
		if( mRevisionEventTime[eANIEVENT_JUMP_START] == 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime1;
			}
			else
			{
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime1;
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_START] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_START] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
			soundIdx = 2301;
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
			soundIdx = 2401;
			mJumpProcess = true;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime;
		}
	}
	else
	{
		memset( mRevisionEventTime, 0, sizeof(float)*eANIEVENT_MAX );
	}

	/// 이벤트 처리
	if( soundIdx )
		SetStepSound( soundIdx );
}


NiPoint3 cHero::GetDirection() const
{
	return mDesiredDir;//mpObjectSceneNode->GetWorldRotate() * -NiPoint3::UNIT_Y;
}

const NiMatrix3& cHero::GetRotate() const
{
	return mpObjectSceneNode->GetWorldRotate();
}

void cHero::AttachListener()
{
	//	((NiNode*)mpObjectSceneNode->GetNiObject())->AttachChild( SOUNDSYS->GetListener() );
}

cDynamicSceneNode* cHero::CreateSceneNode( cPlayerSceneNodeParam* param )
{
	return SCENEMAN->CreateHero( *param );
}

void cHero::AddBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	cPlayer::AddBuff( uniqueIdx, p );

	// 버프윈도우 / 캐쉬버프윈도우
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( p->GetInfluenceIdx() );
	if( pInfo )
	{
		bool cash = (pInfo->mType == eINFLUENCETYPE_CASH || pInfo->mType == eINFLUENCETYPE_PC || pInfo->mType == eINFLUENCETYPE_GMEVENT);

		if( cash == false )
		{
			/// buff 윈도우에 등록
			cBuffWindow* pWin = GAMEUI->GetBuffWindow();
			if( pWin )
				pWin->AddIcon( uniqueIdx, p->GetInfluenceIdx() );
		}
		else
		{
			// cash 버프 윈도우 등록
			cBuffWindow* cashBuff = GAMEUI->GetCashBuffWindow();
			if( cashBuff )
				cashBuff->AddIcon( uniqueIdx, p->GetInfluenceIdx() );
		}
	}
	
	if( DUELMAN->IsDuelEnable() )
	{
		cDuelWindow* win = GAMEUI->GetDuelWindow();
		if( win )
			win->AddBuff( false, uniqueIdx, p->GetInfluenceIdx() );
	}

}

void cHero::DelBuff( unsigned long uniqueIdx )
{
	cInfluenceObject* p = (cInfluenceObject*)mBuffMap.GetAt( uniqueIdx );
	if( p )
	{
		cBuffWindow* pWin = GAMEUI->GetBuffWindow();
		if( pWin )
		{
			pWin->DelIcon( uniqueIdx );
		}

		cBuffWindow* pCashWin = GAMEUI->GetCashBuffWindow();
		if( pCashWin )
		{
			pCashWin->DelIcon( uniqueIdx );
		}

		if( DUELMAN->IsDuelEnable() )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->DelBuff( false, uniqueIdx );
		}
	}
	cPlayer::DelBuff( uniqueIdx );
}

void cHero::AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	cPlayer::AddDeBuff( uniqueIdx, p );

	/// debuff 윈도우에 등록
	cBuffWindow* pWin = GAMEUI->GetDeBuffWindow();
	if( pWin )
	{
		pWin->AddIcon( uniqueIdx, p->GetInfluenceIdx() );
	}

	if( DUELMAN->IsDuelEnable() )
	{
		cDuelWindow* win = GAMEUI->GetDuelWindow();
		if( win )
			win->AddDeBuff( false, uniqueIdx, p->GetInfluenceIdx() );
	}
}

void cHero::DelDeBuff( unsigned long uniqueIdx )
{
	cInfluenceObject* p = (cInfluenceObject*)mDeBuffMap.GetAt( uniqueIdx );
	if( p )
	{
		cBuffWindow* pWin = GAMEUI->GetDeBuffWindow();
		if( pWin )
		{
			pWin->DelIcon( uniqueIdx );
		}

		if( DUELMAN->IsDuelEnable() )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->DelDeBuff( false, uniqueIdx );
		}
	}

	cPlayer::DelDeBuff( uniqueIdx );
}

void cHero::ClearAllBuff( bool delDieInf )
{
	cInfluenceMap::cIterator i = mBuffMap.Begin();
	cInfluenceMap::cIterator end = mBuffMap.End();

	cBuffWindow* pWin = GAMEUI->GetBuffWindow();
	cBuffWindow* pCashWin = GAMEUI->GetCashBuffWindow();
		
	while( i != end )
	{
		unsigned long idx = (*i).mFirst;
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		++i;

		if( p )
		{
			if( delDieInf == true ) 
			{
				if( pWin )
					pWin->DelIcon( p->GetManagedKey() );

				if( pCashWin )
					pCashWin->DelIcon( p->GetManagedKey() );

				SKILLMAN->DeleteInfluenceObject( p );
				mBuffMap.Erase( idx );
			}
			else
			{
				/// 090302 PKH 죽었을때 지워지지 않는 효과 확인후 삭제
				if( p->GetDeleteType() != eDELETETYPE_NODELETE_PLAYER_DIE )
				{
					if( pWin )
						pWin->DelIcon( p->GetManagedKey() );

					SKILLMAN->DeleteInfluenceObject( p );
					mBuffMap.Erase( idx );
				}
			}
		}
	}
}

void cHero::ClearAllDeBuff( bool delDieInf )
{
	cInfluenceMap::cIterator i = mDeBuffMap.Begin();
	cInfluenceMap::cIterator end = mDeBuffMap.End();

	cBuffWindow* pWin = GAMEUI->GetDeBuffWindow();

	while( i != end )
	{
		unsigned long idx = (*i).mFirst;
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		++i;

		if( p )
		{
			if( delDieInf == true ) 
			{
				if( pWin )
					pWin->DelIcon( p->GetManagedKey() );

				SKILLMAN->DeleteInfluenceObject( p );
				mBuffMap.Erase( idx );
			}
			else
			{
				/// 090302 PKH 죽었을때 지워지지 않는 효과 확인후 삭제
				if( p->GetDeleteType() != eDELETETYPE_NODELETE_PLAYER_DIE )
				{
					if( pWin )
						pWin->DelIcon( p->GetManagedKey() );

					SKILLMAN->DeleteInfluenceObject( p );
					mBuffMap.Erase( idx );
				}
			}
		}
	}
}


void cHero::DuelRequest( unsigned long playerIdx )
{
	cBaseObject* pTarget = NULL;

	try
	{
		if( playerIdx == 0 )
			pTarget = HERO->GetTargetObject();
		else
			pTarget = OBJECTMAN->GetPlayer( playerIdx );

		if( pTarget == NULL )
			throw 1;

		/// 타겟은 플레이어
		if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
			throw 1;

		if( PVPMAN->IsEnableBattle() == true )
			throw 1;
	}
	catch ( ... )
	{
		cStringT Message;
		Message.Format( GAMERESOURCEMAN->GetGameText( 808 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );

		return;
	}

#ifdef _GMTOOL
	if ( CHEATMAN->GetHideMode() == true )
	{
		cStringT Message;
		Message.Format( GAMERESOURCEMAN->GetGameText( 805 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );

		return;
	}
#endif

	/// 자동사냥 신고 창이 뜬 도중이라면 무시
	cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
	if( camaelWin && camaelWin->IsVisible() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11816 ) );
		return;
	}


	MSG_REQ_DUEL_ADD msg;
	msg.Category = NM_DUEL;
	msg.Protocol = NM_DUEL_ADD_REQ;
	msg.mTargetIdx = pTarget->GetObjectID();

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 신청
	cStringT Message;
	Message.Format( GAMERESOURCEMAN->GetGameText( 800 ), pTarget->GetName() );
	CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
}

void cHero::DuelGiveUp()
{
	/// 결투 포기 메세지 발송
	MSGROOT msg;
	msg.Category = NM_DUEL;
	msg.Protocol = NM_DUEL_GIVEUP_RES;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cHero::ViewMsgDuelReq( unsigned long duelReqPlayerIdx )
{ 
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( duelReqPlayerIdx );
	if( pPlayer == NULL )
		return;

	cDuelMsgWindow* pDuelMsgWin = GAMEUI->GetDuelMsgWindow();
	if( pDuelMsgWin == NULL )
		return;

	/// 결투 요청자 설정
	pDuelMsgWin->SetTargetIdx( duelReqPlayerIdx );

	/// 결투 요청창 열림
	pDuelMsgWin->ShowModal();
}

void cHero::DuelNoStartMsg( int errorCode )
{
	cStringT Message;

	switch( errorCode )
	{
	case ERR_DUEL_ACCEPT_REQ_ERROR:		/// 요청자 상태 불가
		Message.Format( GAMERESOURCEMAN->GetGameText( 803 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		break;
	case ERR_DUEL_ACCEPT_RES_REJECTION:	/// 대상자 거절
		Message.Format( GAMERESOURCEMAN->GetGameText( 804 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		break;
	case ERR_DUEL_ACCEPT_RES_ERROR:		/// 대상자 상태 불가
		Message.Format( GAMERESOURCEMAN->GetGameText( 806 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		break;
	}					

	///// 결투 신청창 닫힘
	cDuelMsgWindow* pWin = GAMEUI->GetDuelMsgWindow();
	if( pWin != NULL )
		pWin->Hide();
}

void cHero::BlinkBuffIcon( unsigned long uniqueIdx, unsigned long influenceIdx )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( pInfo )
	{
		bool cash = (pInfo->mType == eINFLUENCETYPE_CASH || pInfo->mType == eINFLUENCETYPE_PC || pInfo->mType == eINFLUENCETYPE_GMEVENT );

		if( cash == false )
		{
			cBuffWindow* pWin = GAMEUI->GetBuffWindow();
			if( pWin )
				pWin->BlinkIcon( uniqueIdx, influenceIdx );
		}
		else
		{
			cBuffWindow* pWin = GAMEUI->GetCashBuffWindow();
			if( pWin )
				pWin->BlinkIcon( uniqueIdx, influenceIdx );
		}
	}
}

void cHero::BlinkDeBuffIcon( unsigned long uniqueIdx, unsigned long influenceIdx )
{
	cBuffWindow* pWin = GAMEUI->GetDeBuffWindow();
	if( pWin )
		pWin->BlinkIcon( uniqueIdx, influenceIdx );
}

unsigned long cHero::GetPVPPoint( unsigned char forceType )
{ 
	switch( forceType )
	{
	case eFORCETYPE_FIRE:
		return mHeroInfo.mFirePoint;
	case eFORCETYPE_WATER:
		return mHeroInfo.mWaterPoint;
	case eFORCETYPE_WIND:
		return mHeroInfo.mWindPoint;
	case eFORCETYPE_EARTH:
		return mHeroInfo.mEarthPoint;
	default:
		return 0;
	}
}

void cHero::SetPVPPoint( unsigned long point, unsigned char forceType )
{ 
	switch( forceType )
	{
	case eFORCETYPE_FIRE:
		mHeroInfo.mFirePoint = point;
		break;
	case eFORCETYPE_WATER:
		mHeroInfo.mWaterPoint = point;
		break;
	case eFORCETYPE_WIND:
		mHeroInfo.mWindPoint = point;
		break;
	case eFORCETYPE_EARTH:
		mHeroInfo.mEarthPoint = point;
		break;
	}
}


void cHero::ToggleIdleFlag()
{
	if( mPeaceToggleFlag == false )
		return;
	if( IsReadyUseSkill() == false )
		return;
	if( GetState() == eOBJECT_STATE_DIE ||
		GetState() == eOBJECT_STATE_ATTACK ||
		GetState() == eOBJECT_STATE_STOP ||
		GetState() == eOBJECT_STATE_MOVE )
		return;

	eIDLEFLAG flag = (mPlayerExrInfo.mStateIdle == eIDLE_NORMAL)? eIDLE_PEACE:eIDLE_NORMAL;
	cPlayer::SetIdleFlag( (unsigned char)flag );

	/// Network Msg;
	MSG_REQ_PLAYER_STATE_IDLE Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_STATE_IDLE_REQ;
	Msg.mPlayerStateIdle = (unsigned char)flag;
	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	mPeaceToggleFlag = false;
}

void cHero::ToggleSitDown()
{
	if( IsPossibleSitDown() == false )
		return;

	if( GatheringCancel() == false )
		return;

	/// 앉기/서기 전환
	if( GetState() == ePLAYER_STATE_SITDOWN )
	{
		StandUp();

		/// Network Msg
		MSG_REQ_PLAYER_STATE_SITDWON Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
		Msg.mSitDown = false;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
	else
	{
		if( IsRideVehicle() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			AlightVehicle();
		}

		SitDown();

		/// Network Msg
		MSG_REQ_PLAYER_STATE_SITDWON Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
		Msg.mSitDown = true;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::SetWeaponChangeFlag( bool val ) 
{
	if( mSwitchWeaponFlag == true && val == false )
	{
		/// 기본공격을 위한 이동인지 체크한다.
		if( mAfterAction.mActionType == eAfterAction_Attack )
		{
			/// 기본공격을 위한 이동중에 장비교체시 재장착한다.
			SKILLEXECUTER->ReCharge();
		
			sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
			mAfterAction.mExData1 = chargeInfo.mSkillIdx;
			mAfterAction.mExData2 = chargeInfo.mSkillStep;
			mAfterAction.mRange = chargeInfo.mSkillRange;

			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
		}
	}

	mSwitchWeaponFlag = val;

	if( val == true )
		mSwitchWeaponTime = 600;
}


void cHero::EquipWeaponInfo( eWEAPON_STATE state, sPlayerWeaponInfo* pInfo )
{
	SetWeaponChangeFlag( false );

	cPlayer::EquipWeaponInfo( state, pInfo );
}

bool cHero::EquipWeaponItem( sPlayerWeaponInfo* info )
{
	SetWeaponChangeFlag( false );

	return cPlayer::EquipWeaponItem( info );
}

void cHero::ChangeWeaponState( eWEAPON_STATE state )
{
	cPlayer::ChangeWeaponState( state );

	/// 기본공격을 위한 이동인지 체크한다.
	if( mAfterAction.mActionType == eAfterAction_Attack )
	{
		/// 기본공격을 위한 이동중에 장비교체시 재장착한다.
		SKILLEXECUTER->ReCharge();

		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		mAfterAction.mExData1 = chargeInfo.mSkillIdx;
		mAfterAction.mExData2 = chargeInfo.mSkillStep;
		mAfterAction.mRange = chargeInfo.mSkillRange;

		SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
	}
}

// ResetArmorSets Method.
void cHero::ResetArmorSets()
{
	// CLEAR - ARMOR SETS
	mArmorSets = ARMOR_SETS_NONE;

	//// ARMOR SETS - HAT
	cItem* hat = ITEMMAN->GetItem( INVENTORY_WEAR_HAT );
	if( hat )
	{
		cItemAbility* hatAbil = ITEMMAN->GetItemAbility( hat->GetIndex() );
		if( hatAbil )
		{
			switch( hatAbil->GetDefenseType() )
			{
			case ITEM_MATERIAL_CLOTH:		/// 천
				mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
				break;
			case ITEM_MATERIAL_LEATHER:		/// 가죽
				mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
				break;
			case ITEM_MATERIAL_METAL:		/// 금속
				mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
				break;
			}
		}
	}

	//// ARMOR SETS - UPPER(BODY1)
	cItem* uppper = ITEMMAN->GetItem( INVENTORY_WEAR_BODY1 );
	if( uppper )
	{
		cItemDefine* upperDefine = ITEMMAN->GetItemDefine( uppper->GetIndex() );
		cItemAbility* uppperAbil = ITEMMAN->GetItemAbility( uppper->GetIndex() );

		if( uppperAbil )
		{
			if( upperDefine && upperDefine->GetSubType() == ITEM_WEAR_ONEPIECE )
			{
				switch ( uppperAbil->GetDefenseType() )
				{
				case 1:
					mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
					break; // 천
				case 2:
					mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
					break; // 가죽
				case 3:
					mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
					break; // 금속
				}
			}
			else
			{
				switch ( uppperAbil->GetDefenseType() )
				{
				case 1:
					mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
					break; // 천
				case 2:
					mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
					break; // 가죽
				case 3:
					mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
					break; // 금속
				}
			}
		}
	}

	//// ARMOR SETS - LOWER(BODY2)
	cItem* lower = ITEMMAN->GetItem( INVENTORY_WEAR_BODY2 );
	if( lower )
	{
		cItemDefine* lowerDefine = ITEMMAN->GetItemDefine( lower->GetIndex() );
		cItemAbility* lowerAbil = ITEMMAN->GetItemAbility( lower->GetIndex() );

		if( lowerAbil )
		{
			if( lowerDefine && lowerDefine->GetSubType() == ITEM_WEAR_ONEPIECE )
			{
				switch ( lowerAbil->GetDefenseType() )
				{
				case 1:
					mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
					break; // 천
				case 2:
					mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
					break; // 가죽
				case 3:
					mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
					break; // 금속
				}
			}
			else
			{
				switch ( lowerAbil->GetDefenseType() )
				{
				case 1:
					mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
					break; // 천
				case 2:
					mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
					break; // 가죽
				case 3:
					mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
					break; // 금속
				}
			}
		}
	}


	//// ARMOR SETS - HANDS(HAND)
	cItem* hand = ITEMMAN->GetItem( INVENTORY_WEAR_HAND );
	if( hand )
	{
		cItemAbility* handAbil = ITEMMAN->GetItemAbility( hand->GetIndex() );
		if( handAbil )
		{
			switch( handAbil->GetDefenseType() )
			{
			case ITEM_MATERIAL_CLOTH:		/// 천
				mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
				break;
			case ITEM_MATERIAL_LEATHER:		/// 가죽
				mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
				break;
			case ITEM_MATERIAL_METAL:		/// 금속
				mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
				break;
			}
		}
	}

	//// ARMOR SETS - FEET(HAND)
	cItem* feet = ITEMMAN->GetItem( INVENTORY_WEAR_FOOT );
	if( feet )
	{
		cItemAbility* feetAbil = ITEMMAN->GetItemAbility( feet->GetIndex() );
		if( feetAbil )
		{
			switch( feetAbil->GetDefenseType() )
			{
			case ITEM_MATERIAL_CLOTH:		/// 천
				mArmorSets = mArmorSets | ARMOR_SETS_ROBES;
				break;
			case ITEM_MATERIAL_LEATHER:		/// 가죽
				mArmorSets = mArmorSets | ARMOR_SETS_LIGHT;
				break;
			case ITEM_MATERIAL_METAL:		/// 금속
				mArmorSets = mArmorSets | ARMOR_SETS_HEAVY;
				break;
			}
		}
	}
}

void cHero::SetAreaNameIdx( unsigned long idx )
{
	mAreaNameIdx = idx;
}

void cHero::ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker )
{
	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		return ApplyDamageDrama_TransMon( damage, damageType, pAttacker );
	}

	cBaseObject::ApplyDamageDrama( damage, damageType, pAttacker );

	if( GetState() == eOBJECT_STATE_DIE )
		return;

	if( damageType < eDAMAGEPRINT_HEAL_OUR )
		CAMERAMAN->Shake();

	mDamageAfterAnim = ANITYPE_IDLE;

	unsigned int aniIdx = mpObjectSceneNode->GetTargetAnimation();
	if( (aniIdx >= ANITYPE_COMMON_SKILL_BEGIN && aniIdx < ANITYPE_COMMON_SKILL_END) ||
		(aniIdx >= ANITYPE_SKILL_BEGIN && aniIdx < ANITYPE_SKILL_END) || 
		(aniIdx >= ANITYPE_CASTING1    && aniIdx < ANITYPE_DAMAGE1) ) 	///캐스팅 동작 중 데미지 표현 금지
	{
		return;
	}


	switch( mAnimationType )
	{
	case ANITYPE_SITDOWN:
	case ANITYPE_SITDOWN_IDLE:
	case ANITYPE_STANDUP:
	case ANITYPE_RUN:
	case ANITYPE_ITEMPICK:
	case ANITYPE_DAMAGE1:
//	case ANITYPE_DAMAGE2:
	case ANITYPE_GATHERING1:
	case ANITYPE_GATHERING2:
	case ANITYPE_ENHANCED:
		return;
	}

	if( aniIdx >= ANITYPE_CASTING1 && aniIdx < ANITYPE_DAMAGE1 )
	{
		mDamageAfterAnim = (aniIdx/10)*10;
	}

	if( aniIdx >= ANITYPE_ATTACK1 && aniIdx < ANITYPE_CASTING1 )
	{
		if( GetTargetObject() != pAttacker )
			return;

		unsigned long time = mpObjectSceneNode->GetTargetAnimationEndTime() - 200;
		if( THEAPP->GetWorldAccumTime() < time )
			return;
	}

	/// damage동작 적용
	if( damageType < eDAMAGEPRINT_HEAL_OUR )
	{
		mIdleTime = 0;

		if( GetState() != eOBJECT_STATE_ATTACK )
		{
			/// change dir
			if( aniIdx >= ANITYPE_IDLE && aniIdx <= ANITYPE_IDLE2 )
			{
				if( pAttacker )
					cPlayer::SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 200 );
			}
			if( mAnimationType == ANITYPE_PEACE_IDLE || mAnimationType == ANITYPE_PEACE_IDLE2 )
			{
				if( pAttacker )
					cPlayer::SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 200 );
			}
		}

		if( IsRideVehicle() == false && mJumping == false )
		{
			if( damageType == eDAMAGEPRINT_CRITICAL_ENEMY || damageType == eDAMAGEPRINT_CRITICAL_OUR || damageType == eDAMAGEPRINT_MP_CRITICAL )
				ChangeAnimation( ANITYPE_DAMAGE2 );
			else
				ChangeAnimation( ANITYPE_DAMAGE1 );
		}
	}
}

const cSphere& cHero::GetBoundSphere() const
{
	assert( mpObjectSceneNode );
	return mpObjectSceneNode->GetBoundSphere();
}

const NiPoint3& cHero::GetCenter() const
{
	assert( mpObjectSceneNode );
	return mpObjectSceneNode->GetCenter();
}

void cHero::SetFixedObjectSizeScale( unsigned short size, bool init )
{
	cPlayer::SetFixedObjectSizeScale( size, init );
	mCameraOffset = mDefaultCameraOffset * (float)size * 0.01f;
}

void cHero::ActiveQuestText( LPCTSTR msg, unsigned long color )
{
	mpObjectSceneNode->ActiveQuestText( msg, color );
}

void cHero::ActivePosition( float x, float y )
{
	if( x == GetXPos() && y == GetYPos() )
		return;

	mIsFindPosition = true;
	mFindPos.x = x;
	mFindPos.y = y;
	mFindPosCnt = 0;
	bool ret = ResetMovePath( NiPoint2(GetXPos(), GetYPos()), NiPoint2(x, y) );

	if( ret == false || mPathCount == 0 || mIsFindPosition == false )
	{
		StopFindPos();
		return;
	}

	ReleaseFollowPlayer();

	cStringT str;
	str.Format( _T("%d, %d"), (int)(x*0.01f), (int)(y*0.01f) );

	cMoveMsgWindow* moveMsgWin = GAMEUI->GetMoveMsgWindow();
	if( moveMsgWin )
		moveMsgWin->AddMoveMsg( (LPCTSTR)str.Cstr(), COLOR_GREEN, GAMERESOURCEMAN->GetGameText( 160 ), COLOR_WHITE );
	else
		assert(0);

	cMiniMapWindow* map = GAMEUI->GetMinimapWindow();
	if( map )
	{
		map->AddFindPosition( x, y );
	}

	cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
	if( mini )
	{
		mini->AddFindPosition( x, y );
	}
}

bool cHero::ReActivePosition()
{
	if( mFindPosCnt > 2 || mFindPos == NiPoint2::ZERO )
	{
		DeActivePosition();
		return false;
	}

	if( mFindPos.x == GetXPos() && mFindPos.y == GetYPos() )
	{
		DeActivePosition();
		return false;
	}

	bool ret = ResetMovePath( NiPoint2(GetXPos(), GetYPos()), NiPoint2(mFindPos.x, mFindPos.y) );

	if( ret == false || mPathCount == 0 )
	{
		DeActivePosition();
		StopFindPos();

		return false;
	}

	++mFindPosCnt;
	mIsFindPosition = true;

	return true;

}

void cHero::DeActivePosition()
{
	GAMEUI->HideMoveMsgWindow();

	cMiniMapWindow* map = GAMEUI->GetMinimapWindow();
	if( map )
	{
		map->DeleteFindPsotion();
	}

	cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
	if( mini )
	{
		mini->DeleteFindPsotion();
	}

	mIsFindPosition = false;
}

void cHero::SetStaticFaceReq( unsigned long faceIdx )
{
	MSG_REQ_PLAYER_STATICFACE msg;
	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_STATICFACE_REQ;
	msg.mStaticFaceIdx = faceIdx;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cHero::ProcessDelayFlag( unsigned long deltaTime )
{
	/// 평화(Z) 버튼 delay설정
	if( mPeaceToggleFlag == false )
	{
		if( mPeaceToggleTime > 1000 )
		{
			mPeaceToggleFlag = true;
			mPeaceToggleTime = 0;
		}
		else
		{
			mPeaceToggleTime += deltaTime;
		}
	}

	if( mSwitchWeaponTime )
	{
		if( mSwitchWeaponTime <= deltaTime )
			mSwitchWeaponTime = 0;
		else
			mSwitchWeaponTime -= deltaTime;
	}
}

bool cHero::IsPossibleSitDown()
{
	if( mIsSleep == true || mIsStun == true )
		return false;

	if( IsWeaponChanging() )
		return false;
	if( IsReadyUseSkill() == false )
		return false;

	if( GetState() == eOBJECT_STATE_DIE ||
		GetState() == eOBJECT_STATE_STOP ||
		GetState() == eOBJECT_STATE_PUSHPULL ||
		GetState() == ePLAYER_STATE_RUSH )
		return false;

	if( mJumping == true )
		return false;

	if( mSitDownFlag == false )
		return true;

	return false;
}

void cHero::SetPartyIndex( unsigned long partyIndex )
{
	cPlayer::SetPartyIndex( partyIndex );

	if( partyIndex > 0 )
		PartyIn_Init();
	else
		PartyOut_Init();
}

void cHero::SetGuild( unsigned long guildIndex, unsigned char guildposition, LPCTSTR guildName, tm markRegistDate, bool useMark )
{
	cPlayer::SetGuild( guildIndex, guildposition, guildName, markRegistDate, useMark );

	cHeroInfoWindow* win = GAMEUI->GetHeroInfoWindow();
	if( win )
		win->SetGuild( guildName );
}

void cHero::CheckTabTargetObject()
{
	/// camera Dir
	NiCamera* cam = CAMERAMAN->GetCurrentNi();
	NiPoint3 camDir = cam->GetWorldDirection();
	camDir.z = 0.0f;
	camDir.Unitize();

	/// Hero Position
	NiPoint3 heroPos = GetPos();
	heroPos.z = 0.0f;

	cBaseObject* newTarget = 0;
	unsigned int nearCount = 0;
	unsigned int heroTargetCnt = 0;
	const tArray<unsigned long>* vArr = SCENEMAN->GetVisibleDynamicSceneArray();
	if( vArr && vArr->IsEmpty() == false )
	{
		for( unsigned int i=0; i < vArr->GetSize(); ++i )
		{
			cDynamicSceneNode* n = SCENEMAN->GetDynamicSceneNode( (*vArr)[i] );
			if( n == 0 )
				continue;
			cBaseObject* obj = n->GetGameObject();
			if( obj == 0 )
				continue;
			if( obj == this )
				continue;
			if( obj->IsDie() == true )
				continue;

			/// 타입 검사
			bool heroTarget = false;
			if( obj->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( DUELMAN->IsDuelEnemy( (cPlayer*)obj ) == false && PVPMAN->IsEnableAttack( (cPlayer*)obj ) == false )
					continue;
			}
			else if( obj->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				if( obj->GetTargetObject() == this )
					heroTarget = true;
			}
			else
				continue;

			/// 거리 체크
			NiPoint3 targetPos = obj->GetPos();
			targetPos.z = 0.0f;

			NiPoint3 calcDir = targetPos - heroPos;
			calcDir.z = 0.0f;

			float len = calcDir.Unitize();
			if( len > 2000 )
				break;
			if( len > 700 )
			{
				if( nearCount >=5 )
					break;
				if( heroTargetCnt > 0 )
					break;
			}
			else
			{
				if( heroTarget == true )
					heroTargetCnt++;
			}

			/// 위치 체크
			float check = camDir.Dot(calcDir);
			if( check >= 0.0f )
			{
				if( len > 700 && check < 0.3f )
					continue;
			}
			else
			{
				/// hero뒤쪽 객체 거리 체크
				if( len > 300 )
					continue;
			}

			/// 최접 객체수 증가
			if( len <= 700 )
				nearCount++;

			/// 이미 목록에 들어 있다면 검사 제외
			bool bFind = false;
			NiTListIterator kIter = mTabTargetList.GetHeadPos();
			while (kIter)
			{
				sObject* data = mTabTargetList.GetNext(kIter);
				if( obj->GetObjectID() == data->index && data->type == obj->GetObjectType() )
				{
					bFind = true;
					break;
				}
			}
			if( bFind == true )
				continue;

			newTarget = obj;
			break;
		}
	}

	/// 목록중 조건에서 벗어난 객체 제거
	NiTListIterator kIter = mTabTargetList.GetHeadPos();
	while (kIter)
	{
		NiTListIterator kOldIter = kIter;
		sObject* data = mTabTargetList.GetNext(kIter);

		cBaseObject* o = OBJECTMAN->GetObject( data->type, data->index );
		if( o == 0 || o->IsDie() == true )
		{
			sObject* rp = mTabTargetList.RemovePos(kOldIter);
			SAFE_DELETE(rp);
			continue;
		}

		//// 타입 검사
		if( o->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			if( DUELMAN->IsDuelEnemy( (cPlayer*)o ) == false &&
				PVPMAN->IsEnableAttack( (cPlayer*)o ) == false )
			{
				sObject* rp = mTabTargetList.RemovePos(kOldIter);
				SAFE_DELETE(rp);
				continue;
			}
		}

		/// 거리 / 방향 체크
		NiPoint3 targetPos = o->GetPos();
		NiPoint3 calcDir = targetPos - heroPos;
		calcDir.z = 0.0f;

		float len = calcDir.Unitize();
		if( len > 2000 )
		{
			sObject* rp = mTabTargetList.RemovePos(kOldIter);
			SAFE_DELETE(rp);
			continue;
		}

		if( heroTargetCnt > 0 && len > 700 )
		{
			sObject* rp = mTabTargetList.RemovePos(kOldIter);
			SAFE_DELETE(rp);
			continue;
		}

		float check = camDir.Dot(calcDir);
		if( check >= 0.0f )
		{
			if( len > 700 && check < 0.3f )
			{
				sObject* rp = mTabTargetList.RemovePos(kOldIter);
				SAFE_DELETE(rp);
				continue;
			}
		}
		else
		{
			if( len > 300 )
			{
				sObject* rp = mTabTargetList.RemovePos(kOldIter);
				SAFE_DELETE(rp);
				continue;
			}
		}
	}


	/// 신규 타겟이 없다면, 목록에서 선정한다.
	if( newTarget == 0 )
	{
		if( mTabTargetList.IsEmpty() == false )
		{
			sObject* obj = mTabTargetList.GetHead();
			if( obj )
			{
				newTarget = OBJECTMAN->GetObject( obj->type, obj->index );
				mTabTargetList.RemoveHead();
			}
		}
	}

	///
	if( newTarget )
	{
		SetTargetObject( newTarget->GetObjectType(), newTarget->GetObjectID() );

		sObject* target = new sObject;
		target->index = newTarget->GetObjectID();
		target->type = newTarget->GetObjectType();
		mTabTargetList.AddTail( target );

		while( mTabTargetList.GetSize() > 5 )
		{
			sObject* rp = mTabTargetList.RemoveHead();
			SAFE_DELETE(rp);
		}
	}
}

void cHero::ClearTabTargetList()
{
	NiTListIterator kIter = mTabTargetList.GetHeadPos();
	while (kIter)
	{
		sObject* data = mTabTargetList.Get(kIter);
		SAFE_DELETE(data);

		kIter = mTabTargetList.GetNextPos(kIter);
	}
	mTabTargetList.RemoveAll();
}

void cHero::SetAlphaBlended( float alpha )
{
	cDynamicSceneNode* n = GetSceneNode();
	if( n == 0 )
		return;

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() )
	{
		n->SetAlphaBlended( 0.25f );
		return;
	}
#endif
	{
		n->SetAlphaBlended( alpha );
	}
}

bool cHero::TransformMonster( unsigned long monsterClassIdx )
{
	/// 일어나는중 or 앉는중 변신했을때 보정
	if( mAnimationType == ANITYPE_STANDUP )
		SetState(eOBJECT_STATE_IDLE);
	else if( mAnimationType == ANITYPE_SITDOWN )
		mSitDownFlag = false;

	if( SKILLEXECUTER->IsSkillCharge() == true )
	{
		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
	}

	if( cPlayer::TransformMonster( monsterClassIdx ) == false )
		return false;

	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as )
		mpObjectSceneNode->GetNiNode()->AttachChild( as->GetListener() );

	ShowNameGauge( true );

	/// 몬스터 스킬 세팅
	SKILLMAN->ClearCoolTimeMon();
	QUICKMAN->ChangeMonster( monsterClassIdx );
	QUICKMAN->CheckSkillIcon();

	/// 
	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	assert(pCamera);

	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
	NiPoint3 sCenter = mpObjectSceneNode->GetObjectCenter();
	mDefaultCameraOffset = sCenter.z - sFoot.z + 22.0f;
	mCameraOffset = mDefaultCameraOffset;

	pCamera->SetLookAt( NiPoint3( sFoot.x, sFoot.y, sFoot.z + mCameraOffset ) );

	cBaseObject* obj = GetTargetObject();
	if( obj && obj == this )
	{
		SetTargetObject( eOBJECTTYPE_NONE,0 );
		SetTargetObject( obj->GetObjectType(), obj->GetObjectID() );
	}

	SKILLEXECUTER->SetMonsterSkillInfo( monsterClassIdx );

	//
	OBJECTMANAGER->UpdateMonsterNameColor();
	return true;
}

void cHero::RestoreMonster()
{
	if( SKILLEXECUTER->IsSkillCharge() == true )
	{
		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
	}

	cPlayer::RestoreMonster();

	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as )
		mpObjectSceneNode->GetNiNode()->AttachChild( as->GetListener() );

	ShowNameGauge( true );

	/// 몬스터 스킬 세팅
	SKILLMAN->ClearCoolTimeMon();
	QUICKMAN->ChangeMonster( 0 );
	QUICKMAN->CheckSkillIcon();

	/// 
	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	assert(pCamera);

	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
	NiPoint3 sCenter = mpObjectSceneNode->GetObjectCenter();
	mDefaultCameraOffset = sCenter.z - sFoot.z + 22.0f;
	mCameraOffset = mDefaultCameraOffset;

	pCamera->SetLookAt( NiPoint3( sFoot.x, sFoot.y, sFoot.z + mCameraOffset ) );

	cBaseObject* obj = GetTargetObject();
	if( obj && obj == this )
	{
		SetTargetObject( eOBJECTTYPE_NONE,0 );
		SetTargetObject( obj->GetObjectType(), obj->GetObjectID() );
	}

	//
	OBJECTMANAGER->UpdateMonsterNameColor();
}


///
void cHero::Update_TransMon( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpObjectSceneNode == NULL ) return;

	ProcessDelayFlag( deltaTime );
	DesiredDirProcess( deltaTime );

	/// 정보가 업데이트 될때마다 세팅
	if( mIsCalcStatus )
	{
		STATUSCALC->CalcPlayerExtensionGlobal();

		mpObjectSceneNode->UpdateNameCardGaugeValue();
		GAMEUI->UpdateHeroInfo();

		mIsCalcStatus  = false;
	}
	/// 스킬겨험치나 경험치가 업데이트될때마다 세팅
	else if( mIsUdateHeroInfo )
	{
		GAMEUI->UpdateExpSxp();

		mIsUdateHeroInfo = false;
	}

	/// 상태에 맞는 처리를 한다.
	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_IDLE:
		{
			if( mIsFollowing )
				FollowIdleUpdate();

			IdleUpdate( deltaTime );
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			if( mIsKeyMove == true )
			{
				KeyMoveUpdate( deltaTime );
			}
			else
			{
				if( mIsFollowing )
					FollowMoveUpdate( deltaTime );
				else
					MoveUpdate( deltaTime );
			}

			/// portal 체크
			if( mMapPortalSkipTime <= accumTime )
			{
				cMapPortalSceneNode* portalNode = OBJECTMANAGER->MapPotalPicking();
				if( portalNode )
				{
					unsigned long portalIdx = portalNode->GetPortalIdx();
					unsigned long posScriptIdx = portalNode->GetPosScriptIdx();

					sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( posScriptIdx );
					if( pInfo )
					{
						cRangeCheck check;
						check.SetRadius( pInfo->mRange );
						if( check.IsNotRange( NiPoint2(GetXPos(), GetYPos()), NiPoint2(portalNode->GetTranslate().x, portalNode->GetTranslate().y) ) == false )
						{
							//if( portalIdx == 0 && pInfo->mTargetInfo )
							//{
							//	mIsKeyMove = false;
							//	mGoCountinue = false;

							//	ClearMoveFlag();

							//	SetState( eOBJECT_STATE_IDLE );

							//	TCHAR str[256] = {0,};
							//	_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mTargetInfo->mMapNumber) );

							//	///
							//	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, eMBEVENT_MAPCHANGE, eSKIN_YESNO, str,
							//		GAMERESOURCEMAN->GetGameText( 59 ), (void*)posScriptIdx );

							//	mMapPortalSkipTime = accumTime + 1000;
							//}

							/// 091014 PKH 텔레포트 포탈 체크추가
							if( pInfo->mFirstRegen == false )
							{
								MSG_REQ_TELEPORTPORTAL_USE Msg;
								Msg.Category = NM_TELEPORTPORTAL;
								Msg.Protocol = NM_TELEPORTPORTAL_USE_REQ;
								Msg.mMapChangePosIdx = (unsigned long)posScriptIdx;

								NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

								mMapPortalSkipTime = accumTime + 1000;

								SetStopFlag( eSTOP_TELEPORTPORTAL );
							}
							else
							{
								mIsKeyMove = false;
								mGoCountinue = false;

								ClearMoveFlag();

								SetState( eOBJECT_STATE_IDLE );
								mMapPortalSkipTime = accumTime + 1000;

								if( portalIdx != 0 )
								{
									TCHAR str[256] = {0,};
									_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mMapNumber) );

									///
									UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, eMBEVENT_USERMAPCHANGE, eSKIN_YESNO, str,
										GAMERESOURCEMAN->GetGameText( 59 ), (void*)portalIdx );
								}
								else
								{
									if( pInfo->mTargetInfo )
									{
										TCHAR str[256] = {0,};
										_stprintf_s(str,GAMERESOURCEMAN->GetGameText( 60 ), GAMERESOURCEMAN->GetMapName(pInfo->mTargetInfo->mMapNumber) );

										///
										UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, eMBEVENT_MAPCHANGE, eSKIN_YESNO, str, 
											GAMERESOURCEMAN->GetGameText( 59 ), (void*)posScriptIdx );
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			AttackUpdate( deltaTime, accumTime );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
			DieUpdate();
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			PushPullUpdate( deltaTime );
		}
		break;
	}

	/// 애니메이션 연결에 대한 처리
	Interpret_TransMon( accumTime );

	if( IsJumping() == true )
	{
		if( mJumpRemainTime <= mpObjectSceneNode->GetScaleAccumTime() )
			SetJumpFlag( false );

		if( GetState() == eOBJECT_STATE_MOVE )
		{
			float baseSpeed = 0.0f;
			sMonsterScript* script = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
			if( script )
				baseSpeed = (float)script->mActionRunSpd;

			float moveSpeed = 1.0f;
			if( baseSpeed != 0.0f )
			{
				moveSpeed = (float)mPlayerExrInfo.mMoveSpeed / (float) baseSpeed;
			}

			if( mpObjectSceneNode )
				mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );
		}
	}

	/// 카메라 처리
	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	if( pCamera )
	{
		const NiPoint3& pos = mpObjectSceneNode->GetTranslate();
		pCamera->SetLookAt( pos.x, pos.y, pos.z + mCameraOffset );

		///
		WORLDMAN->MoveSkyDome( pCamera->GetWorldTranslate() );
	}
}

void cHero::Interpret_TransMon( unsigned long time )
{
	cActorManagerForPartition*	pActor = mpObjectSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpObjectSceneNode->GetTargetAnimation();
	float checkTime = (float)mpObjectSceneNode->GetScaleAccumTime() * 0.001f;

	/// 현재 애니메이션 종료 시점 획득
	float endTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;

	if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		endTime = mpObjectSceneNode->GetTargetAnimationScaleEndTime();
	}

	/// 애니/사운드 데이터 생성후 추가
	InterpretEvent_TransMon( id, checkTime, pActor );

	if( endTime > 0 && endTime <= checkTime )
	{
		if( M_ANITYPE_IDLE2 == id )
		{
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( M_ANITYPE_ATTACK1 <= id && id <= M_ANITYPE_ATTACK3 )
		{
			SetReadyUseSkill( true );
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( M_ANITYPE_SKILL1 <= id && id <= M_ANITYPE_SKILL2 )
		{
			SetReadyUseSkill( true );
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( id == M_ANITYPE_DAMAGE1 || id == M_ANITYPE_DAMAGE2 )
		{
			/// 공격받는 애니메이션이 끝나면 기본 애니메이션으로 바꾼다.
			if( mDamageAfterAnim == M_ANITYPE_CASTING )
				UpdateSkillAnimation( M_ANITYPE_CASTING );
			else
			{
				ChangeAnimation_TransMon( mDamageAfterAnim );
			}

			//				SetState( eOBJECT_STATE_IDLE );

			SetReadyUseSkill( true );
		}
		else if( id == M_ANITYPE_JUMP )
		{
			SetJumpFlag( false );

			if( IsGoContinue() == false )
			{
				/// key state check
				unsigned int keyFlag = MOVE_NONE;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
					keyFlag |= MOVE_W;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
					keyFlag |= MOVE_S;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
					keyFlag |= MOVE_A;
				if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
					keyFlag |= MOVE_D;

				if( keyFlag != 0 )
					UpdateKeyState();
			}

			switch( GetState() )
			{
			case eOBJECT_STATE_MOVE:
				SetState( eOBJECT_STATE_MOVE );
				break;
			case eOBJECT_STATE_DIE:
				ChangeAnimation_TransMon( M_ANITYPE_DIE1 );
				break;
			default:
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
				break;
			}

			ShowWeapon( true );
		}
		else
		{
			NiSequenceData* seq = pActor->GetSequenceData( id );
			if( seq )
			{
				if( seq->GetCycleType() != NiTimeController::LOOP )
					SetState( eOBJECT_STATE_IDLE );
			}
			else
			{
				assert(0);
				SetState( eOBJECT_STATE_IDLE );
			}
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		return;
	}
}

void cHero::InterpretEvent_TransMon( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

	unsigned long soundIdx = 0;
	if( id == M_ANITYPE_WORK || id == M_ANITYPE_RUN )
	{
		/// 오른발 소리 체크
		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] == 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
			}
			else
			{
				///
				if( eventTime2 > eventTime1 )
					mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime1;
				else
					mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_RUN_LEFT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_LEFT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
//			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_RIGHT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
//			soundIdx = 2004;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Left" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_LEFT] = eventTime;
		}
	}
	else if( id == M_ANITYPE_JUMP )
	{
		/// 
		if( mRevisionEventTime[eANIEVENT_JUMP_START] == 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] == 0.0f)
		{
			float eventTime1 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			float eventTime2 = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );

			if( eventTime1 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime2 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime2;
			}
			else if( eventTime2 == cActorManagerForPartition::INVALID_TIME )
			{
				if( eventTime1 != cActorManagerForPartition::INVALID_TIME )
					mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime1;
			}
			else
			{
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime1;
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime2;
			}
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_START] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_START] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
//			soundIdx = 2301;
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
//			soundIdx = 2401;
			mJumpProcess = true;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_Start" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_START] = eventTime;
		}
	}
	else
	{
		memset( mRevisionEventTime, 0, sizeof(float)*eANIEVENT_MAX );
	}

	/// 이벤트 처리
	if( soundIdx )
		SetStepSound( soundIdx );
}


void cHero::ApplyDamageDrama_TransMon( int damage, unsigned int damageType, cBaseObject* pAttacker )
{
	cBaseObject::ApplyDamageDrama( damage, damageType, pAttacker );

	if( GetState() == eOBJECT_STATE_DIE )
		return;

	if( GetState() == eOBJECT_STATE_STOP )
	{
		switch( mPlayerExrInfo.mStateStop )
		{
		case eSTOP_READYTAROT:
		case eSTOP_OPENTAROT:
		case eSTOP_OPENSTALL:
			return;
		}
	}

	if( damageType < eDAMAGEPRINT_HEAL_OUR )
		CAMERAMAN->Shake();

	unsigned int ani = mpObjectSceneNode->GetTargetAnimation();

	mDamageAfterAnim = M_ANITYPE_IDLE1;
	//	mDamageAfterAnim = M_ANITYPE_IDLE1;

	switch( ani )
	{
	case M_ANITYPE_WORK:
	case M_ANITYPE_RUN:
	case M_ANITYPE_SKILL1:
	case M_ANITYPE_SKILL2:
	case M_ANITYPE_DAMAGE1:
//	case M_ANITYPE_DAMAGE2:
	case M_ANITYPE_DIE1:
	case M_ANITYPE_DIE2:
	case M_ANITYPE_JUMP:
		return;
	}

	if( ani >= M_ANITYPE_MODEANI )
		return;

	if( M_ANITYPE_ATTACK1 <= ani && ani <= M_ANITYPE_ATTACK3 )
	{
		if( GetTargetObject() != pAttacker )
			return;

		unsigned long time = mpObjectSceneNode->GetTargetAnimationEndTime() - 200;
		if( THEAPP->GetWorldAccumTime() < time )
			return;
	}

	if( ani == M_ANITYPE_CASTING )
		mDamageAfterAnim = M_ANITYPE_CASTING;

	/// damage동작 적용
	if( damageType < eDAMAGEPRINT_HEAL_OUR )
	{
		mIdleTime = 0;

		if( GetState() != eOBJECT_STATE_ATTACK )
		{
			if( mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE1 || 
				mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE2 )
			{
				if( pAttacker )
					cPlayer::SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 10 );
			}
		}

		if( damageType == eDAMAGEPRINT_CRITICAL_ENEMY || damageType == eDAMAGEPRINT_CRITICAL_OUR || damageType == eDAMAGEPRINT_MP_CRITICAL )
		{
			UpdateAnimationSpeed(1.0f);
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DAMAGE2 );
		}
		else
		{
			UpdateAnimationSpeed(1.0f);
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DAMAGE1 );
		}
	}
}

bool cHero::StartAttack_Mon( unsigned long attackIdx, cBaseObject* pTarget )
{
	/// 기본 스킬 생성..
	unsigned long key =  SKILLMAN->CreateHeroSkillObject( attackIdx, 0, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		/// 상태 설정
		SetState( eOBJECT_STATE_ATTACK );
		SetReadyUseSkill( false );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_USED Msg;
		Msg.Category = NM_SKILL;
		Msg.Protocol = NM_SKILL_USED_MON_REQ;//NM_SKILL_USED_REQ;
		Msg.mTarget.index = pTarget->GetObjectID();
		Msg.mTarget.type = pTarget->GetObjectType();

		Msg.mSkillClassIndex = attackIdx;
		Msg.mSkillStep = 0;

		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}

	return true;
}

void cHero::UseTargetSkill_TransMon( unsigned long skillIdx, cBaseObject* pTarget )
{
	AutoActionClear();

	SKILLEXECUTER->ChargeOff( skillIdx, 0 );

	if( IsReadyUseSkill() == false )
	{
		SetState( eOBJECT_STATE_ATTACK );
		return;
	}

	if( IsWeaponChanging() == true )
		return;


	/// 적용 타겟 검증
	if( SKILLEXECUTER->IsPossibleUseSkillTarget( skillIdx, pTarget ) == false )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsPossibleUseSkill( skillIdx, 0 ) == false )
		return;

	if( pTarget )
	{
		/// 바라봐야할 방향 설정
		NiPoint3 charDir = mDesiredDir;
		NiPoint3 viewDir = pTarget->GetPos() - GetPos();
		viewDir.z = 0.0f;
		if( viewDir != NiPoint3::ZERO )
		{
			if( viewDir.SqrLength() >= 0.001f )	
			{
				viewDir.Unitize();

				/// 회전 Matrix 산출
				NiMatrix3 matA, matB, matRot;
				matA.SetCol( 0, charDir );
				matA.SetCol( 1, -NiPoint3::UNIT_Z );
				matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matB.SetCol( 0, viewDir );
				matB.SetCol( 1, -NiPoint3::UNIT_Z );
				matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matRot = matB * matA.Transpose();
				mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
				mDesiredDir = viewDir;
			}
		}
	}

	unsigned long key = SKILLMAN->CreateHeroSkillObject( skillIdx, 0, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		mAutoAttackFlag = true;

		unsigned long monsterClassIdx = GetTransMonsterClassIdx();
		if( monsterClassIdx == 0 )
			return ;
		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
		if( pInfo == 0 )
			return;

		// 발생후 공격하지 않아야하는것만 걸러낸다
		if( pInfo->mType == eMONSTERSKILLTYPE_HEAL || 
			pInfo->mType == eMONSTERSKILLTYPE_TOTEM )
		{
			mAutoAttackFlag = false;
			//			SetState( eOBJECT_STATE_IDLE, false );
		}
		else if( pInfo->mType == eMONSTERSKILLTYPE_NONACTIVE )
		{
			if( pInfo->mApplyType != eAPPLYTYPE_ENEMY &&
				pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
				pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
				mAutoAttackFlag = false;
		}

		/// 상태 설정
		if( pInfo->mType == eMONSTERSKILLTYPE_NONSTOP )
		{
			mAutoAttackFlag = false;
		}
		else
		{
			SetState( eOBJECT_STATE_ATTACK );
			SetReadyUseSkill( false );
		}

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_SKILL_USED Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		if( pInfo->mShotType == eMONSTERSHOTTYPE_CASTING )		/// 090213 PKH 
			Msg.Protocol = NM_SKILL_CAST_USED_MON_REQ;
		else
		{
			if( pInfo->mType == eSKILLTYPE_NONSTOP )
				Msg.Protocol = NM_SKILL_NONSTOP_MON_REQ;
			else
				Msg.Protocol = NM_SKILL_USED_MON_REQ;
		}

		Msg.mSkillClassIndex = skillIdx;
		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		if( pTarget )
		{
			Msg.mTarget.index = pTarget->GetObjectID();
			Msg.mTarget.type = pTarget->GetObjectType();
		}
		else
		{
			Msg.mTarget.index = 0;
			Msg.mTarget.type = eOBJECTTYPE_NONE;
		}

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::UseFieldSkill_TransMon( unsigned long skillIdx, NiPoint3 fieldPos )
{
	AutoActionClear();
	SKILLEXECUTER->ChargeOff( skillIdx, 0 );

	if( IsWeaponChanging() == true )
		return;

	/// 발동 검증
	if( SKILLEXECUTER->IsPossibleUseSkill( skillIdx, 0 ) == false )
		return;

	/// 바라봐야할 방향 설정
	NiPoint3 charDir = mDesiredDir;
	NiPoint3 viewDir = fieldPos - GetPos();
	viewDir.z = 0.0f;
	if( viewDir != NiPoint3::ZERO )
	{
		if( viewDir.SqrLength() >= 0.001f )	
		{
			viewDir.Unitize();

			/// 회전 Matrix 산출
			NiMatrix3 matA, matB, matRot;
			matA.SetCol( 0, charDir );
			matA.SetCol( 1, -NiPoint3::UNIT_Z );
			matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matB.SetCol( 0, viewDir );
			matB.SetCol( 1, -NiPoint3::UNIT_Z );
			matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

			matRot = matB * matA.Transpose();
			mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
			mDesiredDir = viewDir;
		}
	}

	unsigned long key = SKILLMAN->CreateHeroSkillObject( skillIdx, 0, this, fieldPos, mPlayerExrInfo.mAttackSpeed );
	if( key )
	{
		mAutoAttackFlag = true;

		unsigned long monsterClassIdx = GetTransMonsterClassIdx();
		if( monsterClassIdx == 0 )
			return ;

		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
		if( pInfo == 0 )
			return;

		// 발생후 자동공격 하지 않게 한다
		if( pInfo->mType == eMONSTERSKILLTYPE_HEAL  || 
			pInfo->mType == eSKILLTYPE_TOTEM )
		{
			mAutoAttackFlag = false;
		}
		else if( pInfo->mType == eMONSTERSKILLTYPE_NONACTIVE  )
		{
			if( pInfo->mApplyType != eAPPLYTYPE_ENEMY &&
				pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
				pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
				mAutoAttackFlag = false;
		}

		/// 상태 설정
		SetState( eOBJECT_STATE_ATTACK );
		SetReadyUseSkill( false );

		/// 070712 PKH 스킬 요청 메세지 발송
		MSG_REQ_POS_USED Msg;
		Msg.Category = NM_SKILL;

		/// casting이 있는 경우
		if( SKILLSCRIPT->IsCastSkill( skillIdx, 0 ) == true )
			Msg.Protocol = NM_SKILL_POS_CAST_USED_MON_REQ;
		else
			Msg.Protocol = NM_SKILL_POS_USED_MON_REQ;

		Msg.mSkillClassIndex = skillIdx;
		Msg.mSkillStep = 0;

		Msg.mManagedKey = key;
		Msg.mClientPosX = HERO->GetXPos();
		Msg.mClientPosY = HERO->GetYPos();

		Msg.mTargetPosX = fieldPos.x;
		Msg.mTargetPosY = fieldPos.y;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cHero::SetForceType( unsigned char forceType )
{
	cPlayer::SetForceType( forceType );

	/// 세력 UI 세팅
	cHeroInfoWindow* info = GAMEUI->GetHeroInfoWindow();
	if( info )
		info->SetForceType( forceType );
}

void cHero::SetWindPoint( unsigned long windPoint )
{ 
	mHeroInfo.mWindPoint = windPoint; 
}

void cHero::SetWindFriendly( unsigned long windFriendly )
{ 
	mHeroInfo.mWindFriendly = windFriendly; 
}

void cHero::SetEarthPoint( unsigned long earthPoint )
{ 
	mHeroInfo.mEarthPoint = earthPoint; 
}

void cHero::SetEarthFriendly( unsigned long earthFriendly )
{ 
	mHeroInfo.mEarthFriendly = earthFriendly; 
}

void cHero::SetWaterPoint( unsigned long waterPoint )
{ 
	mHeroInfo.mWaterPoint = waterPoint; 
}

void cHero::SetWaterFriendly( unsigned long waterFriendly )
{ 
	mHeroInfo.mWaterFriendly = waterFriendly; 
}

void cHero::SetFirePoint( unsigned long firePoint )
{ 
	mHeroInfo.mFirePoint = firePoint; 
}

void cHero::SetFireFriendly( unsigned long fireFriendly )
{ 
	mHeroInfo.mFireFriendly = fireFriendly; 
}

void cHero::SetFieldEffRadius( float r )
{
	if( mpObjectSceneNode == 0 )
		return;

	mpObjectSceneNode->SetFieldEffRadius( r );
}

void cHero::UpdateFieldTargetEffect( NiPoint3 center )
{
	if( mpObjectSceneNode == 0 )
		return;

	mpObjectSceneNode->UpdateFieldTargetEffect( center );
}

void cHero::SetLevel( unsigned char level )
{
	cPlayer::SetLevel( level);
	
	ITEMMAN->CheckEquipAbility();

	STATUSCALC->CalcPlayerInit();

	mpObjectSceneNode->UpdateNameCardGaugeValue();

	GAMEUI->UpdateHeroInfo();
	GAMEUI->UpdateExpSxp();
	GAMEUI->UpdateSkillPoint();
}

void cHero::NowCalcStatus()
{
	STATUSCALC->CalcPlayerExtensionGlobal();

	mpObjectSceneNode->UpdateNameCardGaugeValue();
	GAMEUI->UpdateHeroInfo();

	/// STATUSCALC 에서 효과 중에 스킬 MP소모 증감 효과가 있어서 스킬창을 업데이트 시킨다.
	QUICKMAN->CheckSkillIcon();

	mIsCalcStatus  = false;
}

/// 전직
void cHero::SetJob( ePLAYER_JOB job )
{
	/// 전직세팅
	cPlayer::SetJob( job );

	/// UI 업데이트
	GAMEUI->UpdateHeroInfo();

	/// 스킬창 셋팅
	cSkillWindow* skillWindow = GAMEUI->GetSkillWindow();
	if( skillWindow )
	{
		skillWindow->AddJobTab( mPlayerInfo.Job );
		skillWindow->SetCurrent( 0 );
	}

	/// 스킬 정보 세팅
	SKILLMAN->UpdateAllSkill();
}

void cHero::SetJobUsedSkillPoint( unsigned int count, unsigned short* values )
{
	if( count == 0 || values == 0 )
		return;

	if( count > mJobUsedSkillPoint.GetSize() )
	{
		assert(0);
		return;
	}

	for( unsigned int i=0; i<count; i++ )
		mJobUsedSkillPoint[i] = values[i];
}

unsigned short	cHero::GetJobUsedSkillPoint( unsigned int jobLevel )
{
	if( jobLevel > mJobUsedSkillPoint.GetSize() )
	{
		assert(0);
		return 0;
	}
	return mJobUsedSkillPoint[jobLevel];
}

void cHero::InitJobUsedSkillPoint()
{
	for( unsigned int i=0; i<mJobUsedSkillPoint.GetSize(); i++ )
		mJobUsedSkillPoint[i] = 0;
}

bool cHero::GatheringStart( unsigned long gaderIdx, unsigned long gatherClassIdx, NiPoint3 targetPos )
{
	if( GetState() == ePLAYER_STATE_GATHERING )
		return false;

	if( IsJumping() == true )
		return false;

	if( IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		AlightVehicle();
	}

	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( gatherClassIdx );
	if( info == 0 )
	{
		assert(0);
		return false;
	}

	/// check
	if( info->mNeedBuffIdx != 0 )
	{
		if( IsBuffHave( info->mNeedBuffIdx ) == false )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 601 ) );
			return false;
		}
	}

	if( GetLevel() < info->mNeedLevel )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 602 ) );
		return false;
	}

	if( info->mNeedItemIdx )
	{
		if( ITEMMAN->FindSlotIndex( info->mNeedItemIdx ) == 0 )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 603 ) );
			return false;
		}
	}

	if( info->mNeedQuestIdx )
	{
		if( QUESTMAN->IsCompleteGathering( info->mNeedQuestIdx ) == true )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 606 ) );
			return false;
		}
	}

	if( info->mNeedMakeIdx )
	{
		if( CheckHeroMakeSkill( info->mNeedMakeIdx ) == false )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 604 ) );
			return false;
		}
	}

	/// casting start
	cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
	if( pWin )
		pWin->SetTime( gatherClassIdx, eCasting_Gather, info->mGatheringTime );

	/// 서버에 msg전송
	MSG_REQ_GATHERING_START msg;
	msg.Category = NM_GATHERING;
	msg.Protocol = NM_GATHERING_START_REQ;
	msg.mObjectIdx = gaderIdx;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// type에 따라 애니 설정 및 추가 링크
	if( IsTransformMonster() == false )
	{
		if( info->mType == eGATHERINGTYPE_COMMON )
		{
			ChangeAnimation( ANITYPE_GATHERING1 );
			SetItemGatheringEffect( eGATHERINGTYPE_COMMON );
		}
		else if( info->mType == eGATHERINGTYPE_MINING )
		{
			ChangeAnimation( ANITYPE_GATHERING2 );
			SetItemGatheringEffect( eGATHERINGTYPE_MINING );

			if( mGatheringToolObj != UINT_MAX )
			{
				mpObjectSceneNode->UnLinkObject( mGatheringToolObj );
				mGatheringToolObj = UINT_MAX;
			}

			/// 
			mGatheringToolObj = mpObjectSceneNode->LinkObject( eLINK_RHAND, "g_fool_001.nif" );
		}
		else
		{
			ChangeAnimation( ANITYPE_GATHERING1 );
			SetItemGatheringEffect( eGATHERINGTYPE_COMMON );
		}
	}

	SetState( ePLAYER_STATE_GATHERING );

	cPlayer::SetDesiredDir( GetPos(), targetPos );

	return true;
}

void cHero::GatheringEnd()
{
	cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
	if( pWin )
		pWin->Clear( eCasting_Gather );

	SetState( eOBJECT_STATE_IDLE );
	ClearItemGatheringEffect();

	if( mGatheringToolObj != UINT_MAX )
	{
		mpObjectSceneNode->UnLinkObject( mGatheringToolObj );
		mGatheringToolObj = UINT_MAX;
	}
}

bool cHero::ItemMixCancel()
{
	if( GetStopFlag() == eSTOP_ITEMMIX )
	{
		return ITEMMIXMAN->ItemMixCancel( false );
	}
	return true;
}

bool cHero::ItemEnhanceCancel()
{
	if( GetStopFlag() == eSTOP_ENHANCED )
	{
		return ITEMMAN->TryToEnhancedCancel( false );
	}
	return true;
}

bool cHero::GatheringCancel()
{
	if( GetState() == ePLAYER_STATE_GATHERING )
	{
		ClearItemGatheringEffect();

		cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
		if( pWin )
		{
			if( pWin->GetPercent() < 98.0f )
			{
				pWin->Clear( eCasting_Gather );
				HERO->SetState( eOBJECT_STATE_IDLE );

				MSGROOT Msg;
				Msg.Category = NM_GATHERING;
				Msg.Protocol = NM_GATHERING_CANCEL_REQ;

				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 605 ) );
				return false;
			}
		}
		else 
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 605 ) );
			return false;
		}

		if( mGatheringToolObj != UINT_MAX )
		{
			mpObjectSceneNode->UnLinkObject( mGatheringToolObj );
			mGatheringToolObj = UINT_MAX;
		}
	}

	return true;
}

// 상황에 따른 npc 대화 상태 취소
bool cHero::NpcSpeechCancel()
{
	switch( mNpcFuncType )
	{
	case NPC_FUNC_MAX:
		return true;
	case NPC_FUNC_STORE:
	case NPC_FUNC_WAREHOUSE:
	case NPC_FUNC_ITEMDISJOINT:
	case NPC_FUNC_GUILD:
	case NPC_FUNC_SKILLPREVIEW:
	case NPC_FUNC_FORTUNETAROT:
	case NPC_FUNC_TAROT:
	case NPC_FUNC_DEAL:
	case NPC_FUNC_THEME:
	case NPC_FUNC_BOARD:
	case NPC_FUNC_TUTORIALBOOK:
	case NPC_FUNC_EVENT:
		break;
	case NPC_FUNC_MAIL:
		{
			// 우편상태
			cUIWindow* mail = (cUIWindow*)GAMEUI->GetMailWindow();
			if( mail )
				mail->Hide();
			return true;
		}
		break;
	}
	return false;
}


bool cHero::IsCantSkill( unsigned long skillIdx )
{
	if( mIsSleep == true || mIsStun == true )
		return true;

	if( IsTransformMonster() )
	{
		sMonsterSkillScript* pScript = SKILLSCRIPT->GetMonsterSkillInfo( mPlayerExrInfo.mChgMonsterIdx, (eMONSTERATTACK_TYPE)skillIdx );
		if( pScript == NULL )
		{
			assert(NULL);
			return true;	/// 스킬 사용 불가
		}

		/// 일반 공격은 사용 가능
		if( skillIdx < eMONSTERATTACK_SKILL1 )
			return false;

		/// 물리 속성에 물리스킬 사용 불가인지 확인
		if( pScript->mAttributeType == eATTRIBUTETYPE_PHYSICAL && mCantSkillPhy == true )
			return true;
		else if( pScript->mAttributeType == eATTRIBUTETYPE_MAGIC && mCantSkillMag == true )
			return true;
	}
	else
	{
		sPlayerSkillBaseInfo* pScript = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pScript == NULL )
		{
			assert(NULL);
			return true;	/// 스킬 사용 불가
		}

		/// 일반 공격은 사용 가능
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
			return false;

		if( pScript->mAttributeType == eATTRIBUTETYPE_PHYSICAL && mCantSkillPhy == true )
			return true;
		else if( pScript->mAttributeType == eATTRIBUTETYPE_MAGIC && mCantSkillMag == true )
			return true;
	}

	return false;
}

void cHero::RideVehicle( unsigned long vehicleIdx )
{
	cPlayer::RideVehicle( vehicleIdx );

	ShowNameGauge( true );
}

void cHero::AlightVehicle()
{
	if( mJumping == true )
		return;

	cPlayer::AlightVehicle();

	ShowNameGauge( true );
}

void cHero::PreviewTodayWord( LPCTSTR word, unsigned long color )
{
	if( mpObjectSceneNode )
	{
		mpObjectSceneNode->PreviewTodayWord( (LPTSTR)word, color );
	}
}

void cHero::PreviewGuildMark( NiTexture* tex )
{
	if( mpObjectSceneNode )
	{
		mpObjectSceneNode->PreviewGuildMark( tex );
	}
}

void cHero::UpdateGuildMark()
{
	if( mpObjectSceneNode )
	{
		mpObjectSceneNode->UpdateGuildMark();
	}
}

bool cHero::CheckHeroMakeSkill( unsigned long makeSkillIdx )
{
	if( makeSkillIdx == 0 )
		return true;

	if( makeSkillIdx == mHeroInfo.MakeSkill1.mIndex )
		return true;

	if( makeSkillIdx == mHeroInfo.MakeSkill2.mIndex )
		return true;

	return false;
}

// 보조직업 익히기
void cHero::SetMixSkillFirst( unsigned char skill1, unsigned char step )
{
	/// 기존 직업에서 단계 업 인지 체크
	if( mHeroInfo.MakeSkill1.mIndex == skill1 && mHeroInfo.MakeSkill1.mStep < step )
	{
		mHeroInfo.MakeSkill1.mIndex = skill1;
		mHeroInfo.MakeSkill1.mStep = step;
	}
	else
	{
		mHeroInfo.MakeSkill1.mIndex = skill1;
		mHeroInfo.MakeSkill1.mStep = step;
		mHeroInfo.MakeSkill1.mExp = 0;
	}
}

// 보조직업 익히기
void cHero::SetMixSkillSecond( unsigned char skill2, unsigned char step )
{
	/// 기존 직업에서 단계 업 인지 체크
	if( mHeroInfo.MakeSkill2.mIndex == skill2 && mHeroInfo.MakeSkill2.mStep < step )
	{
		mHeroInfo.MakeSkill2.mIndex = skill2;
		mHeroInfo.MakeSkill2.mStep = step;
	}
	else
	{
		mHeroInfo.MakeSkill2.mIndex = skill2;
		mHeroInfo.MakeSkill2.mStep = step;
		mHeroInfo.MakeSkill2.mExp = 0;
	}
}

// 숙련도 갱신
void cHero::UpdateMixSkillExp( unsigned char skillIndex, unsigned long exp )
{
	if( mHeroInfo.MakeSkill1.mIndex == skillIndex )
	{
		mHeroInfo.MakeSkill1.mExp = exp;
	}
	else if( mHeroInfo.MakeSkill2.mIndex == skillIndex )
	{
		mHeroInfo.MakeSkill2.mExp = exp;
	}
	else
	{
		assert(0);
	}
}

bool cHero::CheckGatheringEnable( unsigned long gaderClassIdx, unsigned char teamType )
{
	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( gaderClassIdx );
	if( info == 0 )
		return false;

	if( info->mNeedBuffIdx != 0 )
	{
		if( IsBuffHave( info->mNeedBuffIdx ) == false )
		{
			return false;
		}
	}

	if( GetLevel() < info->mNeedLevel )
	{
		return false;
	}

	if( info->mNeedItemIdx )
	{
		if( ITEMMAN->FindSlotIndex( info->mNeedItemIdx ) == 0 )
		{
			return false;
		}
	}

	if( info->mNeedQuestIdx )
	{
		if( QUESTMAN->IsCompleteGathering( info->mNeedQuestIdx ) == true )
			return false;
	}

	if( info->mNeedMakeIdx )
	{
		if( HERO->CheckHeroMakeSkill( info->mNeedMakeIdx ) == false )
		{
			return false;
		}
	}

	if( PVPMAN && PVPMAN->IsEnableBattle() )
	{
		if( teamType != ePVPDM_TEAMTYPE_MAX && teamType != mPlayerExrInfo.mPvPDMTeamType )
			return false;
	}
	return true;
}

void cHero::SetCantSkill( bool cantSkill, unsigned char attributeType )	
{ 
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
		mCantSkillPhy = cantSkill; 
	else
		mCantSkillMag = cantSkill;

	QUICKMAN->CheckSkillIcon();
}

void cHero::SetSleep( bool sleep )	
{ 
	mIsSleep = sleep; 

	QUICKMAN->CheckSkillIcon();
}

void cHero::SetStun( bool stun )	
{ 
	mIsStun = stun; 

	QUICKMAN->CheckSkillIcon();
}

void cHero::SetAvoidMonLook( bool avoidMonLook )
{
	mIsAvoidMonLook = avoidMonLook;

	OBJECTMANAGER->UpdateMonsterNameColor();
}

void cHero::SitDown()
{
	cPlayer::SitDown();

	GAMEUI->UpdateRecovery();

	mIsKeyMove = false;
	mGoCountinue = false;

	ClearMoveFlag();
}

void cHero::StandUp()
{
	cPlayer::StandUp();

	GAMEUI->UpdateRecovery();
}

void cHero::GoCountinueMove() 
{ 
	if( mIsKeyMove == false )
		mGoCountinue = true; 
}

void cHero::StopCountinueMove() 
{ 
	mIsKeyMove = false; 
	mGoCountinue = false; 
}

bool cHero::Jump()
{
	/// key move 상태와 동일하게 처리
	if( IsEnableKeyMove() == false )
		return false;

	if( cPlayer::Jump() == false )
		return false;

	MSGROOT Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_JUMP_REQ;

	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	return true;
}

bool cHero::IsEnableKeyMove()
{
	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		unsigned int stopflag = HERO->GetStopFlag();
		if( !(stopflag == eSTOP_ENHANCED || stopflag == eSTOP_ITEMMIX || stopflag == eSTOP_NPCSPEECH) )
			return false;

		// npc 대화상태일때 검사
		if( stopflag == eSTOP_NPCSPEECH && IsOpenNpcTalk() == true )
			return false;
	}

	if( mJumping == true )
		return false;

	if( IsReadyUseSkill() == false )
	{
		/// 스킬의 현재 상태가 캐스팅중이고 이동가능한지 체크한다.
		cSkillObject* p = SKILLEXECUTER->GetCurrentHeroSkillObject();
		if( p && p->GetDramaState() == eDRAMASTATE_CASTING )
		{
			cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pWin )
			{
				if( pWin->GetPercent() < 98.0f )
				{
					/// cast stop
					p->WaitCancel();
					pWin->Clear( eCasting_Skill );
					HERO->SetState( eOBJECT_STATE_IDLE );

					/// 070712 PKH 스킬 요청 메세지 발송
					MSG_REQ_SKILL_CAST_CANCEL Msg;
					Msg.Category = NM_SKILL;
					Msg.Protocol = NM_SKILL_CAST_CANCEL_REQ;
					Msg.mAttacker.index = HERO->GetObjectID();
					Msg.mAttacker.type = HERO->GetObjectType();

					NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
				}
			}

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else if( p && p->GetDramaState() == eDRAMASTATE_ACTIVITY && p->IsChanneling() )
		{
			cCastingBar* pCastingWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pCastingWin )
			{
				pCastingWin->Clear( eCasting_Skill );
			}

			p->WaitCancel();
			HERO->SetState( eOBJECT_STATE_IDLE );

			/// 070712 PKH 스킬 요청 메세지 발송
			MSG_REQ_CHANNELING_SKILL_CANCEL Msg;
			Msg.Category = NM_SKILL;
			Msg.Protocol = NM_SKILL_CHANNELING_CANCEL_REQ;
			Msg.mAttacker.index = HERO->GetObjectID();
			Msg.mAttacker.type = HERO->GetObjectType();

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else
		{
			return false;
		}
	}

	if( GatheringCancel() == false )
		return false;

	if( ItemMixCancel() == false )
		return false;

	if( ItemEnhanceCancel() == false )
		return false;

	if( NpcSpeechCancel() == false )
		return false;

	/// 이동 불가
	if( IsCantMove() == true )
		return false;

	return true;
}

void cHero::CheckSendMoveMsg( unsigned long delta, bool force )
{
	if( mIsKeyMove == false )
	{
		mCheckSendTime = 0;
		return;
	}

	bool sendKeymove = false;
	if( force == true )
	{
		sendKeymove = true;
	}
	else
	{
		/// (이동허용치-키보드다음목적지최대)*초단위변환 / ( 이동속도 * 허용치10% )
		unsigned long checkTime = (unsigned long)((300-200)*1000 / ( GetMoveSpeed() * 1.1f ));
		if( checkTime < 100 )
			checkTime = 100;

		mCheckSendTime += delta;
		if( mCheckSendTime >= checkTime )
			sendKeymove = true;
	}

	if( sendKeymove == true )
	{
		mCheckSendTime = 0;
		/// send key move msg

		cCamera* cam = CAMERAMAN->GetCurrent();
		if( cam == 0 )
		{
			assert( 0 );
			return;
		}

		NiPoint3 goDir = cam->GetWorldDirection();
		NiPoint3 rightDir = cam->GetWorldRightDir();

		NiPoint3 dir = NiPoint3::ZERO;
		unsigned int keyFlag = MOVE_NONE;
		if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
		{
			keyFlag |= MOVE_W;
			dir += goDir;
		}
		if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
		{
			keyFlag |= MOVE_S;
			dir += -goDir;
		}
		if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
		{
			keyFlag |= MOVE_A;
			dir += -rightDir;
		}
		if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
		{
			keyFlag |= MOVE_D;
			dir += rightDir;
		}

		/// 지속 모드
		if( keyFlag == 0 && mGoCountinue == true )
		{
			dir += goDir;
		}
		dir.z = 0.0f;
		dir.Unitize();

		if( dir != NiPoint3::ZERO && dir != mDesiredDir )
			SetDesiredDir( dir );

		/// 신규 목적지 설정
		NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();
		pos.z = 0.0f;

		NiPoint3 goPos = pos + mDesiredDir*200.0f;
		WORLDMAN->EnableRayGet( pos, goPos );
		mGotoX = goPos.x;
		mGotoY = goPos.y;

		if( EnableGateMovePos( NiPoint2(pos.x, pos.y), mGotoX, mGotoY ) == false )
		{
			mGotoX = pos.x;
			mGotoY = pos.y;
			return;
		}

		/// 신규 목적지 패킷 발송
		MSG_REQ_KEYMOVE Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_KEYMOVE_REQ;
		Msg.mStartX = GetXPos();
		Msg.mStartY = GetYPos();
		Msg.mDestX = mGotoX;
		Msg.mDestY = mGotoY;
		Msg.mClientTime = mSumKeyMoveDeltaTime;
		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

		mSumKeyMoveDeltaTime = 0;
	}
}


void cHero::UpdateKeyState()
{
	if( CAMERAMAN->GetCurrentIdx() != 0 )
		return;

	cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 );
		return;
	}

	NiPoint3 goDir = cam->GetWorldDirection();
	NiPoint3 rightDir = cam->GetWorldRightDir();

	/// 입력처리
	NiPoint3 dir = NiPoint3::ZERO;
	unsigned int keyFlag = MOVE_NONE;
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_UP ) == true )
	{
		keyFlag |= MOVE_W;
		dir += goDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_DOWN ) == true )
	{
		keyFlag |= MOVE_S;
		dir += -goDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_LEFT ) == true )
	{
		keyFlag |= MOVE_A;
		dir += -rightDir;
	}
	if( KEYBOARD->GetKeyPressed( KEY_MOVE_RIGHT ) == true )
	{
		keyFlag |= MOVE_D;
		dir += rightDir;
	}

	/// 지속 모드
	if( mGoCountinue == true )
	{
		if( keyFlag != 0 )
		{
			mGoCountinue = false;
		}
		else
		{
			dir += goDir;
		}
	}
	dir.z = 0.0f;
	dir.Unitize();

	if( keyFlag == 0 && mGoCountinue == false )
	{
		if( mIsKeyMove == true )
		{
			mIsKeyMove = false;
			SetState( eOBJECT_STATE_IDLE );

			/// stop 패킷 발송
			MSG_REQ_MOVE_STOP Msg;
			Msg.Category = NM_PLAYER;
			Msg.Protocol = NM_PLAYER_KEYMOVE_STOP_REQ;
			Msg.mClientPosX = GetXPos();
			Msg.mClientPosY = GetYPos();
			Msg.mClientTime = mSumKeyMoveDeltaTime;
			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
		}
	}
	else
	{
		if( mIsKeyMove == false )
		{
			if( IsEnableKeyMove() == false )
			{
				if( IsReadyUseSkill() == false )
					mWaitMoveFlag = eWAITMOVE_KEY;

				return;
			}

			///
			if( mMoveTargetEffectIdx != (unsigned long)-1 )
			{
				cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mMoveTargetEffectIdx );
				if( node )
				{
					assert( node == mMoveTargetEffect );
					node->Remove();
				}
				mMoveTargetEffect = 0;
				mMoveTargetEffectIdx = (unsigned long)-1;
			}

			///
			mAfterAction.mActionType = eAfterAction_None;
			mWaitMoveFlag = eWAITMOVE_NONE;

			mWaitSkill.skillIdx = 0;
			mWaitSkill.skillStep = 0;

			ClearMoveFlag();
		}

		if( dir != NiPoint3::ZERO && dir != mDesiredDir )
			SetDesiredDir( dir );

		/// 신규 목적지 설정
		NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();
		pos.z = 0.0f;

		NiPoint3 goPos = pos + mDesiredDir*200.0f;
		WORLDMAN->EnableRayGet( pos, goPos );
		mGotoX = goPos.x;
		mGotoY = goPos.y;

		if( EnableGateMovePos( NiPoint2(pos.x, pos.y), mGotoX, mGotoY ) == false )
		{
			mGotoX = pos.x;
			mGotoY = pos.y;
		}
		else
		{
			/// 신규 목적지 패킷 발송
			MSG_REQ_KEYMOVE Msg;
			Msg.Category = NM_PLAYER;
			Msg.Protocol = NM_PLAYER_KEYMOVE_REQ;
			Msg.mStartX = GetXPos();
			Msg.mStartY = GetYPos();
			Msg.mDestX = mGotoX;
			Msg.mDestY = mGotoY;
			Msg.mClientTime = mSumKeyMoveDeltaTime;
			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			mSumKeyMoveDeltaTime = 0;
		}

		mIsKeyMove = true;
		SetState( eOBJECT_STATE_MOVE );
	}
}

bool cHero::RequestFollowPlayer( unsigned long idx )
{ 
	cBaseObject* pFollowPlayer = (cBaseObject*)OBJECTMAN->GetPlayer( idx );

	/// 플레이어만 체크
	if( !pFollowPlayer || pFollowPlayer->GetObjectType() != eOBJECTTYPE_PLAYER )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7000 ) );
		return false;
	}

	if( IsReadyUseSkill() == false )
	{
		/// 스킬의 현재 상태가 캐스팅중이고 이동가능한지 체크한다.
		cSkillObject* p = SKILLEXECUTER->GetCurrentHeroSkillObject();
		if( p && p->GetDramaState() == eDRAMASTATE_CASTING )
		{
			cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pWin )
			{
				if( pWin->GetPercent() < 98.0f )
				{
					/// cast stop
					p->WaitCancel();
					pWin->Clear( eCasting_Skill );
					HERO->SetState( eOBJECT_STATE_IDLE );

					/// 070712 PKH 스킬 요청 메세지 발송
					MSG_REQ_SKILL_CAST_CANCEL Msg;
					Msg.Category = NM_SKILL;
					Msg.Protocol = NM_SKILL_CAST_CANCEL_REQ;
					Msg.mAttacker.index = HERO->GetObjectID();
					Msg.mAttacker.type = HERO->GetObjectType();

					NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
				}
			}

			// 취소 처리
			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}
		else if( p && p->GetDramaState() == eDRAMASTATE_ACTIVITY && p->IsChanneling() )
		{
			cCastingBar* pCastingWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pCastingWin )
			{
				pCastingWin->Clear( eCasting_Skill );
			}

			p->WaitCancel();
			HERO->SetState( eOBJECT_STATE_IDLE );

			/// 070712 PKH 스킬 요청 메세지 발송
			MSG_REQ_CHANNELING_SKILL_CANCEL Msg;
			Msg.Category = NM_SKILL;
			Msg.Protocol = NM_SKILL_CHANNELING_CANCEL_REQ;
			Msg.mAttacker.index = HERO->GetObjectID();
			Msg.mAttacker.type = HERO->GetObjectType();

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			if( p->GetSpecialType() == eSpecial_MapChange )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
			}
			else if( p->GetSpecialType() == eSpecial_Vehicle )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}

			/// 캐스팅 취소 패킷을 쏜다..
			SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
		}

		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7005 ) );
		return false;
	}

	if( GatheringCancel() == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7005 ) );
		return false;
	}
	if( ItemMixCancel() == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7005 ) );
		return false;
	}
	if( ItemEnhanceCancel() == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7005 ) );
		return false;
	}
	/// 이동 불가
	if( IsCantMove() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7005 ) );
		return false;
	}

	NiPoint3 heroNotZPos = GetPos();
	heroNotZPos.z = 0.0f;
	NiPoint3 targetNotZPos = pFollowPlayer->GetPos();
	targetNotZPos.z = 0.0f;	

	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, pFollowPlayer, 1000.0f );
	cRangeCheck rangecheck( tempRange );

	/// 거리 체크
	if( rangecheck.IsRange( heroNotZPos, targetNotZPos ) == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7001 ) );
		return false;
	}

	/// 적인지 판단한다.
	if( IsEnemy( (cPlayer*)pFollowPlayer ) == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7000 ) );
		return false;
	}

	NiPoint2 heroPos( GetXPos(), GetYPos() );
	NiPoint2 targetPos( pFollowPlayer->GetXPos(), pFollowPlayer->GetYPos() );

	mIsFollowing = true;
	mFollowPlayerIndex = idx; 

	/// 따라가기 메시지 출력.
	cStringT msg;
	msg.Format( GAMERESOURCEMAN->GetGameText( 7003 ), pFollowPlayer->GetName() );
	CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );

	cMoveMsgWindow* moveMsgWin = GAMEUI->GetMoveMsgWindow();
	if( moveMsgWin )
		moveMsgWin->AddMoveMsg( pFollowPlayer->GetName(), COLOR_GREEN, GAMERESOURCEMAN->GetGameText( 7004 ), COLOR_WHITE );
	else
		assert(0);

	return true;
}

void cHero::ReleaseFollowPlayer()
{ 
	mFollowPlayerIndex = 0; 
	mIsFollowing = false; 

	GAMEUI->HideMoveMsgWindow();
}

void cHero::StopFindPos()
{
	SetState( eOBJECT_STATE_IDLE );

	MSG_REQ_MOVE_STOP msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_FINDPOSITIONMOVE_STOP_REQ;
	msg.mClientPosX = GetXPos();
	msg.mClientPosY = GetYPos();
	msg.mClientTime = mSumKeyMoveDeltaTime;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cHero::ClearMoveFlag()
{
	///
	if( mIsFollowing == true )
	{
		mFollowPlayerIndex = 0; 
		mIsFollowing = false; 
		GAMEUI->HideMoveMsgWindow();
	}

	///
	if( mIsFindPosition == true )
		DeActivePosition();
}

// 맵이동으로 인한 취소 요청
void cHero::CancelByMapchange()
{
	/// 노점, 타롯, NPC상점 거래
	eSTOPFLAG flag = GetStopFlag();
	switch( flag )
	{
	case eSTOP_OPENTAROT:		/// 개설자 오픈 상태
	case eSTOP_READYTAROT:
		{
			MSG_REQ_TAROT_READER_CLOSE msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			TAROTMAN->ChangePage( ePAGE_NONE );
		}
		break;
	case eSTOP_SELFTAROT:
		{
			/// 취소
			MSGROOT msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_SELF_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			TAROTMAN->ChangePage( ePAGE_NONE );
		}
		break;
	case eSTOP_USETAROT:		/// 참여자 타롯 사용중
		{
			MSG_REQ_TAROT_SEEKER_CLOSE msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_SEEKER_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			TAROTMAN->ChangePage( ePAGE_NONE );
		}
		break;
	case eSTOP_OPENSTALL:		/// 노점 오픈 상태
		{
			MSG_REQ_ITEM_STALL_SELL_CLOSE msg;
			::memset( &msg, 0, sizeof(msg) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_STALL_SELL_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eSTOP_USESTALL:		/// 노점 이용 상태
		{
			MSG_REQ_ITEM_STALL_SELL_LEAVE msg;
			::memset( &msg, 0, sizeof(msg) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_STALL_SELL_LEAVE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eSTOP_NPCSPEECH:		/// NPC대화중
		{
			CloseNpcTalk();
		}
		break;
	case eSTOP_ITEMMIX:			/// 조합중
		{
			ITEMMIXMAN->ItemMixCancel();
		}
		break;
	case eSTOP_ENHANCED:
		{
			ITEMMAN->TryToEnhancedCancel();
		}
		break;
	}

	if( PVPMAN->GetPVPState() == ePVPState_Count )
	{
		// 입장 대기 취소요청
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_READYCANCEL_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
}

bool cHero::EnableGateMovePos( NiPoint2 startPos, float& x, float& y )
{
	NiPoint2 goalPos = NiPoint2( x, y );
	sDoorInfo* pInfo = SCENEMAN->DisableDoorMove( startPos, goalPos );
	if( pInfo == 0 )
		return true;

	NiPoint2 dir = goalPos - startPos;
	unsigned int len = (unsigned int)dir.Unitize();
	for( ; len > 0; len/=2 )
	{
		NiPoint2 goPos = startPos + dir*(float)len;
		if( STAGESCRIPT->CrashDoor( pInfo->mIndex, startPos, goPos ) == 0 )
		{
			x = goPos.x;
			y = goPos.y;
			return true;
		}
	}
	return false;
}

void cHero::SetMapPortalSkipTime( unsigned long time )
{
	mMapPortalSkipTime = THEAPP->GetWorldAccumTime() + time;
}

bool cHero::IsEnemy( cPlayer* player )
{
	if( player == 0 )
		return false;

	bool enemy = false;
	if( DUELMAN && DUELMAN->IsDuelEnable() == true )
	{
		if( DUELMAN->IsDuelEnemy( player ) == true )
			enemy = true;
	}
	else if( PVPMAN && PVPMAN->IsEnableBattle() == true )
	{
		if( PVPMAN->IsMyTeam( player ) == false )
			enemy = true;
	}
	return enemy;
}

void cHero::SetPos_ResetCam( NiPoint3 pos )
{
	SetPos( pos );

	cLookAtCamera* pCamera = (cLookAtCamera*)CAMERAMAN->GetCurrent();
	if( pCamera == 0 )
	{
		assert(0);
		return;
	}
	pCamera->SetFixedPos();
}

void cHero::SetTitle( unsigned long titleIdx )
{
	cPlayer::SetTitle( titleIdx );
	
	cHeroInfoWindow* pHeroinfoWin = GAMEUI->GetHeroInfoWindow();
	if( pHeroinfoWin )
	{
		cTitleDefine* define = TITLEMAN->GetTitleDefine( titleIdx );
		if( define )
		{
			cStringT name = define->mName;
			name.TrimLeft(_T("["));
			name.TrimRight(_T("]"));
			pHeroinfoWin->SetTitle( name.Cstr() );
		}
	}
}

void cHero::RequestLookPlayer( unsigned long playerIndex )
{
	if( playerIndex == 0 )
	{
		// 타겟 검사
		cBaseObject* pTarget = GetTargetObject();
		if( pTarget == 0 )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
			return;
		}

		// 타겟이 Player가 아닌 다른 Object인 경우 취소
		if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
			return;
		}

		playerIndex = pTarget->GetObjectID();
	}

	MSG_REQ_PLAYER_LOOK_OTHER_EQUIP msg;
	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_LOOK_OTHER_EQUIP_REQ;
	msg.characterIdx = playerIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}
void cHero::SetSkillRush( float posX, float posY, sObject target, unsigned long nextSkill, unsigned char nextStep, bool blink )
{
	if( blink )
	{
		mGotoX = posX;
		mGotoY = posY;

		mActionMoveRange = 0;

		mTargetPos.x = posX;
		mTargetPos.y = posY;
		mTargetPos.z = 0.0f;

		/// 높이값 계산
		if( WORLDMAN )
		{
			if( WORLDMAN->CalcHeight( &mTargetPos.z, posX, posY ) == false )
				assert(NULL);
		}

		SetTargetObject( target.type, target.index );

	
		HERO->SetWorldPos( mTargetPos );
		HERO->SetPos_ResetCam( mTargetPos );

		if( !(HERO->GetState() == eOBJECT_STATE_DIE || HERO->GetState() == eOBJECT_STATE_STOP) )
			HERO->SetState( eOBJECT_STATE_IDLE );

		mRushSkill = nextSkill;
		mRushStep = nextStep;
		mRushBlink = blink;
		UseRushTargetSkill();		
	}
	else
	{
		cPlayer::SetSkillRush( posX, posY, target );
		mRushSkill = nextSkill;
		mRushStep = nextStep;
		mRushBlink = blink;
	}
}

bool cHero::RushUpdate( unsigned long deltaTime )
{
	if( cPlayer::RushUpdate( deltaTime ) == false )
	{
		mpObjectSceneNode->UpdateAniScaleFactor( 1.0 );		
		UseRushTargetSkill();		
	}

	return true;
}

unsigned long cHero::CalcUseMPExt( unsigned long originUseMP )
{	
	/// 소모 체크
	float useMPPlus = STATUSCALC->GetStatus2Plus( ePLAYER_STATUS_EXT_USE_MP );
	float useMPPer  = STATUSCALC->GetStatus2Per( ePLAYER_STATUS_EXT_USE_MP );

	float totalUseMP = (float)originUseMP;

	if( useMPPer != 0.0f )
	{
		totalUseMP += ( (totalUseMP * useMPPer) / 100.0f );
	}

	totalUseMP += useMPPlus;

	if( totalUseMP < 0.0f )
		totalUseMP = 0.0f;

	return (unsigned long)totalUseMP;
}

unsigned long cHero::CalcCastingExt( unsigned long originCastingTime )
{	
	/// 순간 스킬 체크
	if( originCastingTime == 0 )
		return 0;

	/// 소모 체크
	float useCastingPlus = STATUSCALC->GetStatus2Plus( ePLAYER_STATUS_EXT_CASTING );
	float useCastingPer  = STATUSCALC->GetStatus2Per( ePLAYER_STATUS_EXT_CASTING );

	float totalCastingTime = (float)originCastingTime;

	if( useCastingPer != 0.0f )
	{
		totalCastingTime += ( (totalCastingTime * useCastingPer) / 100.0f );
	}

	totalCastingTime += useCastingPlus;

	if( totalCastingTime < 1.0f )
		totalCastingTime = 1.0f;

	return (unsigned long)totalCastingTime;
}
