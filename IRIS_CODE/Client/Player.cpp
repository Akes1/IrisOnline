#include "stdafx.h"
#include "player.h"
#include "PlayerSceneNode.h"
#include "EffectSceneNode.h"

#include "SceneManager.h"

#include "Skill_Common.h"
#include "SkillManager.h"
#include "CommunityScript.h"

#include "Gathering_Common.h"
#include "GatheringScript.h"

#include "GameResourceManager.h"

#include "Application.h"
#include "WorldManager.h"

#include "Hero.h"
#include "ObjectManager.h"
#include "ObjectNameCard.h"

#include "RangeCheck.h"
#include "OptionManager.h"

#include "ItemManager.h"
#include "DamagePrintManager.h"
#include "DramaturgyManager.h"
#include "PartyManager.h"
#include "GameUIManager.h"
#include "MonsterScript.h"
#include "PartyUnionManager.h"

#include "DuelManager.h"
#include "DuelWindow.h"
#include "TargetWindow.h"

/// 
#include "MonsterScript.h"
#include "TransMonSceneNode.h"
#include "Vehicle_Common.h"
#include "VehicleScript.h"
#include "VehicleSceneNode.h"
#include "monster.h"
#include "StatusScript.h"

#include "SoundSceneNode.h"


const int EMOTIONREMAIN_TIME = 3000;
const int WINKINTERVAL_TIME = 4000;
const int WINKREMAIN_TIME = 100;


cPlayer::cPlayer( unsigned char type )
: cBaseObject( type )
, mPathFinder(0)
{
	mPlayerWeaponState = eWEAPON_STATE_NONE;

	mReadyTransformInfo.readState = eReady_None;
	mReadyTransformInfo.chgMonIdx = 0;

	for( unsigned int i = 0; i<eCOSTUME_MAX; i++ )
		mCostumeObjectIndex[i] = UINT_MAX;

	for( unsigned int i = 0; i < eHAND_ALL; i++ )
	{
		mWeaponObjectIndex[i] = UINT_MAX;
		mWeaponObjectEffect[i] = UINT_MAX;
	}

	mGatheringToolObj = UINT_MAX;

	mPlayerExrInfo.mMoveSpeed = 350;
	mPlayerExrInfo.mFixedObjectSizePer = 0;
	mPlayerExrInfo.mAttackSpeed = 1.0f;

	mGotoX = 0.0f;
	mGotoY = 0.0f;

	mDesiredDir = -NiPoint3::UNIT_Y;

	mActionMoveRange = 0.0f;

	mSitDownFlag = false;
	mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;

	mShowWeapon = true;

	mDelay = 0;
	mAngle = 0.0f;
	mAccumAngle = 0.0f;
	mAxis = NiPoint3::UNIT_Z;

	mEmotionCommandIdx = 0;
	mEmotionRemainTime = 0;

	mWinkFaceIdx = 0;
	mWinkTime = 0;
	mWinkRemainTime = 0;

	mDuelEffect    = 0;
	mDuelEffectIdx = (unsigned long)-1;

	mSitDownEffect    = 0;
	mSitDownEffectIdx = (unsigned long)-1;

	mItemMixEffect    = 0;
	mItemMixEffectIdx = (unsigned long)-1;
	mItemMixSoundNode = 0;
	mItemMixSoundIdx  = (unsigned long)-1;

	mItemEnhanceEffect    = 0;
	mItemEnhanceEffectIdx = (unsigned long)-1;
	mItemEnhanceSoundNode = 0;
	mItemEnhanceSoundIdx  = (unsigned long)-1;

	mItemGatheringEffect    = 0;
	mItemGatheringEffectIdx = (unsigned long)-1;
 
 	mItemDisjointSoundNode = 0;
	mItemDisjointSoundIdx = (unsigned long)-1;

	mAnimationType = (ePLAYER_ANIMATIONTYPE)UINT_MAX;
	mDamageAfterAnim = ANITYPE_IDLE;

	SetJumpFlag( false );

	mTargetPos = NiPoint3::ZERO;

	mRemoveDropItemIdx = UINT_MAX;

	mFixedObjectSize = 0;
}

cPlayer::~cPlayer()
{
	ClearDuelEffect();
	//ClearSitDownEffect();
	ClearItemMixEffect();
	ClearItemEnhanceEffect();
	ClearItemGatheringEffect();
	ClearItemDisjointSound();

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
	

	RemoveObjectSceneNode();
}

/* ------------------------------------------------------------------
* 함수이름 :	Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, 
				   *						NiPoint3 sPos, NiMatrix3 sRot, float fScale )
				   * 목    적 :	게임 플레이어 객체를 생성한다.
				   * 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayer::Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, 
					 sPlayerExrInfo* pExrInfo, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	::memcpy( &mPlayerInfo, pBaseInfo, sizeof(sPlayerInfo) );
	::memcpy( &mPlayerExrInfo, pExrInfo, sizeof(sPlayerExrInfo) );
	::memcpy( &mPlayerWearInfo, pWearInfo, sizeof(sPlayerWearInfo) );
	::memcpy( &mPlayerWeaponInfo, pWeaponInfo, sizeof(sPlayerWeaponInfo) );

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
		cPlayerSceneNodeParam param;
		param.mpObject = this;
		param.mTranslate = sPos;
		param.mRotate = sRot;
		param.mScale = fScale;

		cString pathName;
		pathName.Format( "./Data/Character/%s", strFile );
		param.mPathName = pathName;

		mpObjectSceneNode = CreateSceneNode( &param );
		if( mpObjectSceneNode == 0 )
		{
			assert(0);
			return false;
		}

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

		/// 게임 진입시 상황에 맞는 최종 동작을 유지하도록 보정.
		switch( mPlayerExrInfo.mState )
		{
		case eOBJECT_STATE_DIE:
			{
				ChangeAnimation( ANITYPE_DIE1 );
				mpObjectSceneNode->GotoLastFrame();
			}
			break;
		case ePLAYER_STATE_SITDOWN:
			{
				ChangeAnimation( ANITYPE_SITDOWN_IDLE );
				ShowWeapon(false);
				mpObjectSceneNode->GotoLastFrame();

				NiTransform trans;
				trans.MakeIdentity();
				mSitDownEffect = SetLinkdEffect( eLINK_BODY, "./Data/Effect/Eff_PC_Rest.nif", &trans, true, true );
				if( mSitDownEffect )
					mSitDownEffectIdx = mSitDownEffect->GetIndexByManger();
			}
			break;
		case eOBJECT_STATE_STOP:
			{
				/// flag별 동작 변경
				switch( mPlayerExrInfo.mStateStop )
				{
				case eSTOP_READYTAROT:
					{
						ActiveTarotBubble( (LPTSTR)GAMERESOURCEMAN->GetGameText( 300 ), COLOR_SKYBLUE );
						ChangeAnimation( ANITYPE_SITDOWN_IDLE );
						mpObjectSceneNode->GotoLastFrame();
					}
					break;
				case eSTOP_OPENTAROT:
					{
						ChangeAnimation( ANITYPE_SITDOWN_IDLE );
						mpObjectSceneNode->GotoLastFrame();
					}
					break;
				case eSTOP_OPENSTALL:
					{
						ChangeAnimation( ANITYPE_SITDOWN_IDLE );
						mpObjectSceneNode->GotoLastFrame();
					}
					break;
				case eSTOP_ENHANCED:
					{
						SetItemEnhanceEffect();
						ChangeAnimation( ANITYPE_ENHANCED );
						mpObjectSceneNode->GotoLastFrame();
					}
					break;
				case eSTOP_ITEMMIX:
					{
						SetItemMixEffect();
						ChangeAnimation( ANITYPE_ENHANCED );
						mpObjectSceneNode->GotoLastFrame();
					}
					break;
				case eSTOP_NONE:
					break;
				default:
					{
						ChangeAnimation( ANITYPE_IDLE );
					}
					break;
				}
			}
			break;
		case eOBJECT_STATE_IDLE:
			{
				/// emoting check
				if( mPlayerExrInfo.mCommunitySkillIdx != 0 )
				{
					sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( mPlayerExrInfo.mCommunitySkillIdx );
					if( pInfo )
					{
						if( pInfo->mType == 1 )
							ChangeAnimation( (ePLAYER_ANIMATIONTYPE)pInfo->mRoleIndex );
					}
				}
			}
			break;
		default:
			ChangeAnimation( ANITYPE_IDLE );
			break;
		}
	}
	else
	{
		/// 외형 몬스터화
		sMonsterScript* pMonsterInfo;
		pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		if( pMonsterInfo == NULL )
		{
			assert(0);
			return false;
		}

		float scale = 1.0f;
		if( pMonsterInfo->mMonsterScale > 0.0f )
		{
			scale = pMonsterInfo->mMonsterScale;
		}
		else
		{
			assert(0);
		}

		const char* strFile = GAMERESOURCEMAN->GetModelFileName( pMonsterInfo->mKfmIdx );
		assert( strFile );

		cMonsterSceneNodeParam param;

		cString path;
		path.Format( "./Data/Monster/%s", strFile );

		param.mpObject = this;
		param.mPathName = path;
		param.mHeadHeight = (float)pMonsterInfo->mNameHeight;
		param.mTranslate = sPos;
		param.mRotate = sRot;
		param.mScale = scale;

		mpObjectSceneNode = SCENEMAN->CreateTransMonster( param, false );
		if( mpObjectSceneNode == 0 )
		{
			assert(0);
			return false;
		}
		/// 
		mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE1 );
	}

	/// 전장 세력 이펙트
	//PVPMAN->LinkPvpForceEffect( this, mPlayerExrInfo.mForceType );

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, true );

//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetWorldScale();

	SetState( mPlayerExrInfo.mState );

	mpObjectSceneNode->UpdateNameCardTextValue();
	mpObjectSceneNode->UpdateNameCardGaugeValue();

	mDesiredDir = sRot*mDesiredDir;
	mDesiredDir.Unitize();

	mGotoX = sPos.x;
	mGotoY = sPos.y;

	mDelay = 0;
	mAngle = 0.0f;
	mAccumAngle = 0.0f;
	mAxis = NiPoint3::UNIT_Z;

	if( PARTYMAN->IsHeroParty( GetObjectID() ) == true )
	{
		PARTYMAN->ClearAllBuff( GetObjectID() );
		PartyIn_Init();
	}
	else if( PARTYUNIONMAN->IsHeroParty( GetObjectID() ) == true )
	{
		PARTYUNIONMAN->ClearAllBuff( GetObjectID() );
		PartyIn_Init();
	}
	else
	{
		PartyOut_Init();
	}

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
	return true;
}

void cPlayer::ShowNameGauge( bool show )
{
	if( mpObjectSceneNode )
		mpObjectSceneNode->ShowNameGauge( show );
}

void cPlayer::PartyIn_Init()
{
	if( mpObjectSceneNode == 0 )
		return;

	mpObjectSceneNode->ShowNameGauge( true );

	/// 파티 세팅시 해야할일들 하기~
	mpObjectSceneNode->UpdateNameCardTextValue();
}

void cPlayer::PartyOut_Init()
{
	if( mpObjectSceneNode == 0 )
		return;

	mpObjectSceneNode->ShowNameGauge( false );

	/// 파티 세팅시 해야할일들 하기~
	mpObjectSceneNode->UpdateNameCardTextValue();
}

/* ------------------------------------------------------------------
* 함수이름 :	ReCreate( char race, char gender )
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
void cPlayer::ReCreate( char race, char gender )
{
	mPlayerInfo.Race = race;
	mPlayerInfo.Gender = gender;

	/// 재생성
	const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );

	cPlayerSceneNodeParam param;
	param.mpObject = this;
	param.mPathName = strFile;
	param.mTranslate = mpObjectSceneNode->GetTranslate();
	param.mRotate = mpObjectSceneNode->GetRotate();
	param.mScale = mpObjectSceneNode->GetScale();
	cPlayerSceneNode* pSceneNode = SCENEMAN->CreatePlayer( param );

	if( !pSceneNode )
	{
		assert(0);
		return;
	}

	/// 등록된 그래픽 데이터 삭제
	RemoveObjectSceneNode();

	/// 새로 등록
	mpObjectSceneNode = pSceneNode;
}

void cPlayer::SetPathFinder( cPathFinder* pathFinder )
{
	mPathFinder = pathFinder;
	//mPathNodeSet.Create(mPathFinder);
}

void cPlayer::IdleUpdate( unsigned long deltaTime )
{
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
				ChangeAnimation( ANITYPE_IDLE2 );
		}
		else
		{
			ChangeAnimation_TransMon( M_ANITYPE_IDLE2 );
		}
	}
}

void cPlayer::MoveUpdate( unsigned long deltaTime )
{
	NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

	/// 이건 캐릭터별로 따로 설정되는 값이다.
	long fSpeed = GetMoveSpeed();
	if( mJumpProcess )
	{
		if( fSpeed > 10 )
			fSpeed = 10;
	}

	/// 속도 * 경과시간 만큼의 거리 계산
	NiPoint3 calcPos;
	calcPos.x = pos.x + (fSpeed*mDesiredDir.x*(deltaTime*0.001f));
	calcPos.y = pos.y + (fSpeed*mDesiredDir.y*(deltaTime*0.001f));
	calcPos.z = 0.0f;

	/// 보정을 위해서 목적지까지의 남은 거리를 계산한다.
	NiPoint2 dir;
	dir.x = mGotoX - pos.x;
	dir.y = mGotoY - pos.y;
	float destlen = dir.Unitize();

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	dir.x = calcPos.x - pos.x;
	dir.y = calcPos.y - pos.y;
	float curlen = dir.Unitize();

	if( destlen <= curlen )
	{
		/// 거의 목적지에 도착했다면.. 목적지로 판단한다.
		/// 클라이언트 평균15프레임으로 벗어 날수있는 오차범위
		calcPos.x = mGotoX;
		calcPos.y = mGotoY;

		SetState(eOBJECT_STATE_IDLE);
	}

	/// 범위 계산 해보자!!
	if( mActionMoveRange != 0.0f )
	{
		float actionMoveRangeFix;
		cBaseObject* pTarget = OBJECTMAN->GetObject( &mTarget );
		NiPoint3 targetPos = mTargetPos;
		if( pTarget )
			targetPos = pTarget->GetPos();
		else
		{
			if( WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y ) == false )
				assert(NULL);
		}

		if( WORLDMAN->CalcHeight( &calcPos.z, calcPos.x, calcPos.y ) == false )
			assert(NULL);

		actionMoveRangeFix = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mActionMoveRange );

		cRangeCheck rangecheck( actionMoveRangeFix );

		if( rangecheck.IsRange( calcPos, targetPos ) )
		{
			/// 좌표 보정
			NiPoint3 p;

			NiPoint3 d = GetPos() - targetPos;
			d.Unitize();

			p.x = targetPos.x + ((actionMoveRangeFix-1.0f)*d.x);
			p.y = targetPos.y + ((actionMoveRangeFix-1.0f)*d.y);

			if( WORLDMAN->CalcHeight( &p.z, p.x, p.y ) )
			{
				SetPos(p);
			}
			else
			{
				assert(0);
			}

			mActionMoveRange = 0.0f;
			SetState( eOBJECT_STATE_IDLE );

			return;
		}
	}

	/// 좌표 셋팅
	if( WORLDMAN->CalcHeight( &calcPos.z, calcPos.x, calcPos.y ) )
	{
		SetPos(calcPos);
	}
	else
	{
		assert(0);
	}
}

void cPlayer::PushPullUpdate( unsigned long deltaTime )
{
	NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

	long speed = SKILL_PUSHPULL_SPEED;

	/// 보정을 위해서 목적지까지의 남은 거리를 계산한다.
	NiPoint2 dir1, dir2;
	dir1.x = mGotoX - pos.x;
	dir1.y = mGotoY - pos.y;
	dir1.Unitize();

	/// 속도 * 경과시간 만큼의 거리 계산
	pos.x += dir1.x * speed * (deltaTime * 0.001f);
	pos.y += dir1.y * speed * (deltaTime * 0.001f);
	pos.z = 0.0f;

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	dir2.x = mGotoX - pos.x;
	dir2.y = mGotoY - pos.y;
	dir2.Unitize();

	if( dir2.Dot(dir1) <= 0.0f )
	{
		/// 거의 목적지에 도착했다면.. 목적지로 판단한다.
		/// 클라이언트 평균15프레임으로 벗어 날수있는 오차범위
		pos.x = mGotoX;
		pos.y = mGotoY;

		SetState(eOBJECT_STATE_IDLE, false);
	}

	/// 좌표 셋팅
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
	{
		SetPos(pos);
	}
	else
	{
		assert(0);
	}
}

bool cPlayer::RushUpdate( unsigned long deltaTime )
{
	NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

	/// 이건 캐릭터별로 따로 설정되는 값이다.
	long fSpeed = SKILL_RUSH_SPEED;

	NiPoint3 targetPos;
	targetPos.x = mGotoX;
	targetPos.y = mGotoY;
	/// 좌표 셋팅
	if( WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y ) == false  )
		assert(NULL);

	SetDesiredDir( GetPos(), targetPos );

	/// 속도 * 경과시간 만큼의 거리 계산
	NiPoint3 calcPos;
	calcPos.x = pos.x + (fSpeed*mDesiredDir.x*(deltaTime*0.001f));
	calcPos.y = pos.y + (fSpeed*mDesiredDir.y*(deltaTime*0.001f));
	calcPos.z = 0.0f;

	/// 보정을 위해서 목적지까지의 남은 거리를 계산한다.
	NiPoint2 dir;
	dir.x = mGotoX - pos.x;
	dir.y = mGotoY - pos.y;
	float destlen = dir.Unitize();

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	dir.x = calcPos.x - pos.x;
	dir.y = calcPos.y - pos.y;
	float curlen = dir.Unitize();

	if( destlen <= curlen )
	{
		/// 거의 목적지에 도착했다면.. 목적지로 판단한다.
		/// 클라이언트 평균15프레임으로 벗어 날수있는 오차범위
		calcPos.x = mGotoX;
		calcPos.y = mGotoY;

		SetState(eOBJECT_STATE_IDLE);

		return false;
	}

	/// 범위 계산 해보자!!
	if( mActionMoveRange != 0.0f )
	{
		float actionMoveRangeFix;
		cBaseObject* pTarget = OBJECTMAN->GetObject( &mTarget );
		NiPoint3 targetPos = mTargetPos;
		if( pTarget )
			targetPos = pTarget->GetPos();
		else
		{
			if( WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y ) == false )
				assert(NULL);
		}

		mGotoX = targetPos.x;
		mGotoY = targetPos.y;

		if( WORLDMAN->CalcHeight( &calcPos.z, calcPos.x, calcPos.y ) == false )
			assert(NULL);

		actionMoveRangeFix = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mActionMoveRange );

		cRangeCheck rangecheck( actionMoveRangeFix );

		if( rangecheck.IsRange( calcPos, targetPos ) )
		{
			/// 좌표 보정
			NiPoint3 p;

			NiPoint3 d = GetPos() - targetPos;
			d.Unitize();

			p.x = targetPos.x + ((actionMoveRangeFix-1.0f)*d.x);
			p.y = targetPos.y + ((actionMoveRangeFix-1.0f)*d.y);

			if( WORLDMAN->CalcHeight( &p.z, p.x, p.y ) )
			{
				SetPos(p);
			}
			else
			{
				assert(0);
			}

			mActionMoveRange = 0.0f;
			SetState( eOBJECT_STATE_IDLE );

			return false;
		}
	}

	/// 좌표 셋팅
	if( WORLDMAN->CalcHeight( &calcPos.z, calcPos.x, calcPos.y ) )
	{
		SetPos(calcPos);
	}
	else
	{
		assert(0);
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	Update( float time )
* 목    적 :	갱신부(이동시에 각종 처리를 한다.)
* 주의사항 :	
* ------------------------------------------------------------------ */
void cPlayer::Update( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpObjectSceneNode  == NULL ) 
		return;

	/// 변신 대기 상태 처리
	if( mReadyTransformInfo.readState == eReady_RestoreMonster )
	{
		if( IsReadyUseSkill() && IsJumping() == false )
		{
			RestoreMonster();
			ChangeAnimation( ANITYPE_IDLE );

			cBaseObject* obj = HERO->GetTargetObject();
			if( obj == this )
			{
				HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
				HERO->SetTargetObject( obj->GetObjectType(), obj->GetObjectID() );
			}
		}
	}
	else if( mReadyTransformInfo.readState == eReady_TransformMonster )
	{
		if( IsReadyUseSkill() && IsJumping() == false )
		{
			TransformMonster( mReadyTransformInfo.chgMonIdx );
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );

			cBaseObject* obj = HERO->GetTargetObject();
			if( obj == this )
			{
				HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
				HERO->SetTargetObject( obj->GetObjectType(), obj->GetObjectID() );
			}
		}
	}

	/// 
	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
		return Update_TransMon( deltaTime, accumTime );

	DesiredDirProcess( deltaTime );

	FaceProcess( deltaTime );

	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_IDLE:
		{
			IdleUpdate(deltaTime);
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			MoveUpdate(deltaTime);
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
}

void cPlayer::DesiredDirProcess( unsigned long delta )
{
	if( mAngle == 0 )
		return;

	float angle = (float)delta/(float)mDelay *mAngle;
	if( mAccumAngle + angle >= mAngle )
	{
		angle = mAngle - mAccumAngle;

		mAngle = 0;
		mAccumAngle = 0;

		if( angle == 0 )
			return;
	}
	else
		mAccumAngle += angle;

	NiMatrix3 mat;
	if( mAxis.z < 0.0f )
		mat.FromEulerAnglesXYZ( 0.0f, 0.0f, -angle );
	else
		mat.FromEulerAnglesXYZ( 0.0f, 0.0f, angle );

	mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*mat );

	mDesiredDir = mat * mDesiredDir;
	mDesiredDir.z = 0.0f;
	mDesiredDir.Unitize();
}

bool cPlayer::SetDesiredDir(NiPoint3 CurrentPos, NiPoint3 TargetPos, unsigned long delay )
{
	if( GetState() == eOBJECT_STATE_DIE )
		return true;

	CurrentPos.z = 0;
	TargetPos.z = 0;

	/// 현재 캐릭터의 방향
	NiPoint3 charDir = mDesiredDir;

	/// 바라봐야할 방향
	NiPoint3 viewDir = TargetPos - CurrentPos;
	if( viewDir.SqrLength() < 0.001f )	
	{
//		assert(0);
		return false;
	}
	viewDir.Unitize();

	if( mDesiredDir == viewDir )
	{
		mDelay = 0;
		mAngle = 0;
		mAccumAngle = 0;

		return true;
	}

	/// 회전 Matrix 산출
	NiMatrix3 matA, matB, matRot;
	matA.SetCol( 0, charDir );
	matA.SetCol( 1, -NiPoint3::UNIT_Z );
	matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

	matB.SetCol( 0, viewDir );
	matB.SetCol( 1, -NiPoint3::UNIT_Z );
	matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

	matRot = matB * matA.Transpose();

	if( delay == 0 )
	{
		mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
		mDesiredDir = viewDir;

		mDelay = 0;
		mAngle = 0;
		mAccumAngle = 0;
	}
	else
	{
		mDelay = delay;
		mAccumAngle = 0;
		matRot.ExtractAngleAndAxis(mAngle, mAxis.x, mAxis.y, mAxis.z);

		if( mAngle > 360.0f )
			mAngle -= 360.0f;
		else if( mAngle < 0.0f )
			mAngle += 360.0f;
	}

	return true;
}


/* ------------------------------------------------------------------
* 함수이름 :	Move( float x, float y )
* 목    적 :	이동을 위해 제공되는 인터페이스
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayer::Move( float x, float y )
{
	/// 현재 캐릭터의 방향
	NiPoint3 charDir = mDesiredDir;

	/// 바라봐야할 방향
	NiPoint3 viewPos = NiPoint3(x, y, 0.0f);
	NiPoint3 curPos = mpObjectSceneNode->GetTranslate();
	curPos.z = 0.0f;

	if(viewPos == curPos) return false;

	NiPoint3 viewDir = viewPos - curPos;

	if( viewDir == NiPoint3::ZERO )
		return false;

	if( viewDir.SqrLength() < 0.001f )	
	{
//		assert(0);
		return false;
	}
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

	SetState( eOBJECT_STATE_MOVE );

	mGotoX = x;
	mGotoY = y;

	return true;
}

void cPlayer::SetActionMove( float tPosX, float tPosY, float range )
{
	mActionMoveRange = range;
	assert( mActionMoveRange > 20.0f );

	mTargetPos.x = tPosX;
	mTargetPos.y = tPosY;
	mTargetPos.z = 0.0f;

	/// 높이값 계산
	if( WORLDMAN )
	{
		if( WORLDMAN->CalcHeight( &mTargetPos.z, mTargetPos.x, mTargetPos.y ) == false )
			assert(NULL);
	}
}


/* ------------------------------------------------------------------
* 함수이름 :	ChangeBaseHair( unsigned int hairIdx )
* 목    적 :	기본 머리카락 정보를 교체한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayer::ChangeBaseHair( unsigned int hairIdx, unsigned int hairColorIdx )
{
	if( IsTransformMonster() == true )
		return true;

	/// 기본 머리 파일명을 가져온다.
	const char* strFileName = NULL;
	strFileName = GAMERESOURCEMAN->GetDefaultHairFileName( hairIdx, mPlayerInfo.Race, mPlayerInfo.Gender );
	if( strFileName == 0 || ::strlen(strFileName) == 0 )
	{
		assert(0);
		return false;
	}

	if( mpObjectSceneNode->ChangeParts( ePART_HAIR, strFileName ) == false )
		return false;

	/// 색상 적용
	if( hairColorIdx != 0 )
	{
		strFileName = GAMERESOURCEMAN->GetHairColorTextureFileName( hairIdx, hairColorIdx, mPlayerInfo.Race, mPlayerInfo.Gender );
		if( strFileName == 0 || ::strlen(strFileName) == 0 )
		{
			assert(0);
			return false;
		}
		mpObjectSceneNode->ChangePartsTexture( ePART_HAIR, strFileName );
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeBaseFace( unsigned int faceIdx )
* 목    적 :	기본 얼굴 정보를 교체한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayer::SetBaseFace()
{
	if( IsTransformMonster() == true )
		return true;

	/// 기본 머리 파일명을 가져온다.
	const char* strFileName = NULL;
	strFileName = GAMERESOURCEMAN->GetDefaultFaceFileName( mPlayerInfo.Race, mPlayerInfo.Gender );
	if( strFileName == 0 || ::strlen(strFileName) == 0 )
	{
		assert(0);
		return false;
	}
	return mpObjectSceneNode->ChangeParts( ePART_FACE, strFileName );
}

bool cPlayer::ChangeFaceTexture( unsigned int faceIdx, bool base )
{
	if( IsTransformMonster() == true )
		return true;

	const char* strFileName = NULL;
	strFileName = GAMERESOURCEMAN->GetFaceTextureFileName( faceIdx, mPlayerInfo.Race, mPlayerInfo.Gender );
	if( strFileName == 0 || ::strlen(strFileName) == 0 )
		return false;
	if( mpObjectSceneNode->ChangePartsTexture( ePART_FACE, strFileName ) == false )
		return false;

	/// wink index setting
	if( base == true )
	{
		mWinkFaceIdx = faceIdx + 1000;
		strFileName = GAMERESOURCEMAN->GetFaceTextureFileName( faceIdx, mPlayerInfo.Race, mPlayerInfo.Gender );
		if( strFileName == 0 || ::strlen(strFileName) == 0 )
		{
			assert(0);
		}
	}
	return true;
}

bool cPlayer::ChangeBaseWear( unsigned int part )
{
	if( IsTransformMonster() == true )
		return true;

	const char* strFileName = 0;
	strFileName = GAMERESOURCEMAN->GetDefaultWearFileName( part, mPlayerInfo.Race, mPlayerInfo.Gender );
	if( strFileName == 0 || ::strlen(strFileName) == 0 )
	{
		assert(0);
		return false;
	}

	return mpObjectSceneNode->ChangeParts( part, strFileName );
}

/* ------------------------------------------------------------------
* 함수이름 :	EquipWearItem( unsigned long itemIdx )
* 목    적 :	아이템으로 취급하는 객체(머리 얼굴 상의 하의 장갑 신발)에 대한 데이터 교체
* 주의사항 :	장착아이템이 없는 경우에 기본 파트는 별도로 함수를 제공한다.
* ------------------------------------------------------------------ */
bool cPlayer::EquipWearItem( unsigned long itemIndex )
{
	if( IsTransformMonster() == true )
		return true;

	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );

	if( itemDefine == 0 )
		return false;

	unsigned int partIndex = ePART_MAX;
	bool updateEar = false;
	switch( itemDefine->GetSubType() )
	{
	case ITEM_WEAR_HEAD:
	case ITEM_WEAR_COS_HEAD:
		{
			if( OPTIONMAN->IsShowHelmet() == true )
			{
				partIndex = ePART_HAIR;
				updateEar = true;
			}
			else
				return true;
		}
		break;
	case ITEM_WEAR_UPPER: 
	case ITEM_WEAR_COS_UPPER:
		partIndex = ePART_BODY1;
		break;
	case ITEM_WEAR_LOWER:  
	case ITEM_WEAR_COS_LOWER:
		partIndex = ePART_BODY2;
		break;
	case ITEM_WEAR_HANDS: 
	case ITEM_WEAR_COS_HANDS:
		partIndex = ePART_HAND;
		break;
	case ITEM_WEAR_FEET: 
	case ITEM_WEAR_COS_FEET:
		partIndex = ePART_FOOT;
		break;
	case ITEM_WEAR_COS_ONEPIECE:
		{
			sItemOnePieceInfo* pInfo = ITEMMAN->GetItemOnePieceInfo( itemIndex );
			if( pInfo == 0 )
			{
				assert(0);
				return false;
			}

			const char* fileName = 0;
			fileName = ITEMMAN->GetItemFileName( pInfo->mUpperIndex, GetRace(), GetGender(), false );
			if( fileName )
				mpObjectSceneNode->ChangeParts( ePART_BODY1, fileName );
			else
				ChangeBaseWear( ePART_BODY1 );

			fileName = ITEMMAN->GetItemFileName( pInfo->mLowerIndex, GetRace(), GetGender(), false );
			if( fileName )
				mpObjectSceneNode->ChangeParts( ePART_BODY2, fileName );
			else
				ChangeBaseWear( ePART_BODY2 );

			fileName = ITEMMAN->GetItemFileName( pInfo->mHandsIndex, GetRace(), GetGender(), false );
			if( fileName )
				mpObjectSceneNode->ChangeParts( ePART_HAND, fileName );
			else
				ChangeBaseWear( ePART_HAND );

			fileName = ITEMMAN->GetItemFileName( pInfo->mFootsIndex, GetRace(), GetGender(), false );
			if( fileName )
				mpObjectSceneNode->ChangeParts( ePART_FOOT, fileName );
			else
				ChangeBaseWear( ePART_FOOT );
		}
		return true;
	default:
		//assert(0);
		return false;
	}

	if( partIndex >= ePART_MAX )
		return false;

	bool check = mpObjectSceneNode->ChangeParts( partIndex, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );

	if( check == true && updateEar == true )
	{
		mpObjectSceneNode->HideEar( itemDefine->GetHideEar() == 1 );
	}

	///// 그래픽 데이터 교체
	return check;
}

bool cPlayer::EquipAccessoryItem( unsigned char costumePos, unsigned long itemIndex )
{
	if( IsTransformMonster() == true )
		return true;

	if(costumePos >= eCOSTUME_MAX)
	{
		assert(0);
		return false;
	}

	mpObjectSceneNode->UnLinkObject( mCostumeObjectIndex[costumePos] );
	mCostumeObjectIndex[costumePos] = UINT_MAX;


	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
	if( itemDefine == 0 )
		return false;

	unsigned int idx = UINT_MAX;
	switch( itemDefine->GetSubType() )
	{
	case ITEM_ACCESSORY_COS_FACE:
		{
			idx = mpObjectSceneNode->LinkObject( eLINK_HEAD, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
			if( idx == UINT_MAX )
			{
				assert(0);
				return false;
			}
		}
		break;
	case ITEM_ACCESSORY_COS_BACK: 
		{
			idx = mpObjectSceneNode->LinkObject( eLINK_BODY, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
			if( idx == UINT_MAX )
			{
				assert(0);
				return false;
			}
		}
		break;
	default:
		//assert(0);
		return false;
	}

	mpObjectSceneNode->UnLinkObject( mCostumeObjectIndex[costumePos] );
	mCostumeObjectIndex[costumePos] = idx;

	return true;
}


/* ------------------------------------------------------------------
* 함수이름 :	UnEquipWearItem( unsigned int part )
* 목    적 :	상의 하의 장갑 신발 아이템을 제거 했을 경우 기본 아이템으로
*				데이터를 표현하기위한 함수
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayer::UnEquipWearItem( unsigned long itemIndex )
{
	if( IsTransformMonster() == true )
		return true;

	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );

	if( itemDefine == 0 )
		return false;

	unsigned int partIndex = ePART_MAX;

	switch( itemDefine->GetSubType() )
	{
	case ITEM_WEAR_HEAD:
		{
			if( OPTIONMAN->IsShowHelmet() == true )
				mpObjectSceneNode->HideEar( false );
		}
		return true;
	case ITEM_WEAR_UPPER: 
		partIndex = ePART_BODY1;
		break;
	case ITEM_WEAR_LOWER:  
		partIndex = ePART_BODY2;
		break;
	case ITEM_WEAR_HANDS: 
		partIndex = ePART_HAND;
		break;
	case ITEM_WEAR_FEET: 
		partIndex = ePART_FOOT;
		break;
	default:
		//assert(0);
		return false;
	}

	///// 머리와 얼굴은 별도의 해체를 제공하지 않는다.
	if( partIndex <= ePART_FACE || partIndex >= ePART_MAX )
		return false;

	///// 기본 아이템 정보를 가져온다.
	const char* strFileName = NULL;
	strFileName = GAMERESOURCEMAN->GetDefaultWearFileName( partIndex, mPlayerInfo.Race, mPlayerInfo.Gender );
	if( strFileName == 0 || ::strlen(strFileName) == 0 )
	{
		assert(0);
		return false;
	}

	///// 그래픽 데이터 교체
	return mpObjectSceneNode->ChangeParts( partIndex, strFileName );
}

/* ------------------------------------------------------------------
* 함수이름 :	EquipWeaponItem( unsigned long leftItemIndex, unsigned long rightItemIndex )
* 목    적 :	무기 아이템 장착
* 주의사항 :	기존 착용 정보가 있다면 제거한다.
* ------------------------------------------------------------------ */
bool cPlayer::EquipWeaponItem( sPlayerWeaponInfo* info )
//bool cPlayer::EquipWeaponItem( unsigned long lItemIndex, unsigned long rItemIndex )
{
	if( info == 0 )
	{
		assert(0);
		return false;
	}

	if( IsTransformMonster() == true )
		return true;

	//////////////////////////////////////////////////////////////////////////
	/// delete weapon Enhance effect 
	for( unsigned int i=0; i< eHAND_ALL; i++ )
	{
		if( mWeaponObjectEffect[i] == UINT_MAX )
			continue;

		cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mWeaponObjectEffect[i] );
		if( eff )
			eff->Remove();

		mWeaponObjectEffect[i] = UINT_MAX;
	}
	//////////////////////////////////////////////////////////////////////////

	unsigned long lItemIndex = info->WeaponIdx[eHAND_LEFT];
	unsigned long rItemIndex = info->WeaponIdx[eHAND_RIGHT];

	eWEAPON_STATE weaponState = eWEAPON_STATE_NONE;
	cItemDefine* lItemDefine = 0;
	cItemDefine* rItemDefine = 0;

	if( lItemIndex )
		lItemDefine = ITEMMAN->GetItemDefine( lItemIndex );
	if( rItemIndex )
		rItemDefine = ITEMMAN->GetItemDefine( rItemIndex );

	/// 양손 무기를 착용한 경우
	cItemDefine* itemDefine = 0;

	if( lItemIndex == 0 && rItemDefine )
	{
		itemDefine = rItemDefine;
	}
	else if( rItemIndex == 0 && lItemDefine )
	{
		itemDefine = lItemDefine;
	}

	if( itemDefine && itemDefine->GetEquipType() == ITEM_EQUIP_TWO_HANDS )
	{
		switch( itemDefine->GetSubType() )
		{
		case ITEM_WEAPON_DUAL:
		case ITEM_WEAPON_CUTTER:
			{
				unsigned int i = mpObjectSceneNode->LinkObject( eLINK_RHAND, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
				if( i != UINT_MAX )
				{
					mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
					mWeaponObjectIndex[eHAND_RIGHT] = i;
				}
				else
				{
					assert(0);
					return false;
				}

				i = mpObjectSceneNode->LinkObject( eLINK_LHAND, itemDefine->GetFileName1( mPlayerInfo.Race, mPlayerInfo.Gender ) );
				if( i != UINT_MAX )
				{
					mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
					mWeaponObjectIndex[eHAND_LEFT] = i;
				}
				else
				{
					assert(0);
					return false;
				}
			}
			break;
		case ITEM_WEAPON_BLADE:
		case ITEM_WEAPON_GUN:
		case ITEM_WEAPON_STAFF:
			{
				unsigned int i = mpObjectSceneNode->LinkObject( eLINK_RHAND, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
				if( i != UINT_MAX )
				{
					mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
					mWeaponObjectIndex[eHAND_RIGHT] = i;
				}
				else
				{
					assert(0);
					return false;
				}

				mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
				mWeaponObjectIndex[eHAND_LEFT] = UINT_MAX;
			}
			break;
		}

		switch( itemDefine->GetSubType() )
		{
		case ITEM_WEAPON_DUAL:
			weaponState = eWEAPON_STATE_DOUBLESWORD;
			break;
		case ITEM_WEAPON_CUTTER:
			weaponState = eWEAPON_STATE_SHORTSWORD;
			break;
		case ITEM_WEAPON_BLADE:
			weaponState = eWEAPON_STATE_LONGSWORD;
			break;
		case ITEM_WEAPON_GUN:
			weaponState = eWEAPON_STATE_GUN;
			break;
		case ITEM_WEAPON_STAFF:
			weaponState = eWEAPON_STATE_STAFF;
			break;
		}
	}
	if( weaponState )
	{
		ChangeWeaponState( weaponState );
	}
	else
	{
		/// 한손 무기를 착용한 경우
		if( rItemDefine )
		{
			assert( rItemDefine->GetSubType() == ITEM_WEAPON_SWORD );
			unsigned int i = mpObjectSceneNode->LinkObject( eLINK_RHAND, rItemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
			if( i != UINT_MAX )
			{
				mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
				mWeaponObjectIndex[eHAND_RIGHT] = i;
				weaponState = eWEAPON_STATE_SWORD;
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else
		{
			mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
			mWeaponObjectIndex[eHAND_RIGHT] = UINT_MAX;
		}

		/// 방패를 착용한 경우
		if( lItemDefine )
		{
			assert( lItemDefine->GetSubType() == ITEM_WEAPON_SHIELD );
			unsigned int i = mpObjectSceneNode->LinkObject( eLINK_LARM, lItemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );

			if( i != UINT_MAX )
			{
				mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
				mWeaponObjectIndex[eHAND_LEFT] = i;

				if( weaponState == eWEAPON_STATE_SWORD )
					weaponState = eWEAPON_STATE_SWORD_SHEILD;
				else
					weaponState = eWEAPON_STATE_SHEILD;
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else
		{
			mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
			mWeaponObjectIndex[eHAND_LEFT] = UINT_MAX;
		}

		ChangeWeaponState( weaponState );
	}


	//////////////////////////////////////////////////////////////////////////
	/// 강화 이펙트 처리
	cString rEffectName;
	cString lEffectName;
	switch( weaponState )
	{
	case eWEAPON_STATE_SWORD:
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_onehand4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_onehand3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_onehand2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_onehand1.nif";

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), 0, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_SWORD_SHEILD:
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_onehand4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_onehand3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_onehand2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_onehand1.nif";

			if( info->WeaponEnhanced[eHAND_LEFT] > 11 )
				lEffectName = "./data/effect/eff_enhance_shield4.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 8 )
				lEffectName = "./data/effect/eff_enhance_shield3.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 5 )
				lEffectName = "./data/effect/eff_enhance_shield2.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 2 )
				lEffectName = "./data/effect/eff_enhance_shield1.nif";

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), 0, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();

			cEffectSceneNode* lEff = SetLinkdEffect( eLINK_LWEAPON, lEffectName.Cstr(), 0, true, true );
			if( lEff )
				mWeaponObjectEffect[eHAND_LEFT] = lEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_LONGSWORD: 
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_twohand4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_twohand3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_twohand2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_twohand1.nif";

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), 0, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_GUN: 
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_gun4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_gun3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_gun2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_gun1.nif";

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), 0, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_STAFF: 
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_staff4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_staff3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_staff2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_staff1.nif";

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), 0, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_DOUBLESWORD:
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_onehand4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_onehand3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_onehand2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_onehand1.nif";

			lEffectName = rEffectName;

			NiTransform trans;
			trans.MakeIdentity();

			/// 뒤집힌 무기에 대한 처리
			if( itemDefine )
			{
				cString str = itemDefine->GetFileName0( GetRace(), GetGender() );
				if( str.Compare( "weapon3B", 0, 8 ) == 0 )
				{
					NiMatrix3 matX, matY;
					matY.MakeYRotation( NI_PI );
					matX.MakeXRotation( -NI_PI );

					trans.m_Rotate = matY * matX;
				}
			}

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), &trans, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();

			/// 뒤집힌 무기에 대한 처리
			if( itemDefine )
			{
				cString str = itemDefine->GetFileName1( GetRace(), GetGender() );
				if( str.Compare( "weapon3B", 0, 8 ) == 0 )
				{
					NiMatrix3 matX, matY;
					matY.MakeYRotation( NI_PI );
					matX.MakeXRotation( -NI_PI );

					trans.m_Rotate = matY * matX;
				}
			}

			cEffectSceneNode* lEff = SetLinkdEffect( eLINK_LWEAPON, lEffectName.Cstr(), 0, true, true );
			if( lEff )
				mWeaponObjectEffect[eHAND_LEFT] = lEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_SHORTSWORD:
		{
			if( info->WeaponEnhanced[eHAND_RIGHT] > 11 )
				rEffectName = "./data/effect/eff_enhance_dagger4.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 8 )
				rEffectName = "./data/effect/eff_enhance_dagger3.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 5 )
				rEffectName = "./data/effect/eff_enhance_dagger2.nif";
			else if( info->WeaponEnhanced[eHAND_RIGHT] > 2 )
				rEffectName = "./data/effect/eff_enhance_dagger1.nif";

			lEffectName = rEffectName;

			NiTransform trans;
			trans.MakeIdentity();

			/// 뒤집힌 무기에 대한 처리
			if( itemDefine )
			{
				cString str = itemDefine->GetFileName0( GetRace(), GetGender() );
				if( str.Compare( "weapon3B", 0, 8 ) == 0 )
				{
					NiMatrix3 matX, matY;
					matY.MakeYRotation( NI_PI );
					matX.MakeXRotation( -NI_PI );

					trans.m_Rotate = matY * matX;
				}
			}

			cEffectSceneNode* rEff = SetLinkdEffect( eLINK_RWEAPON, rEffectName.Cstr(), &trans, true, true );
			if( rEff )
				mWeaponObjectEffect[eHAND_RIGHT] = rEff->GetIndexByManger();


			/// 뒤집힌 무기에 대한 처리
			if( itemDefine )
			{
				cString str = itemDefine->GetFileName1( GetRace(), GetGender() );
				if( str.Compare( "weapon3B", 0, 8 ) == 0 )
				{
					NiMatrix3 matX, matY;
					matY.MakeYRotation( NI_PI );
					matX.MakeXRotation( -NI_PI );

					trans.m_Rotate = matY * matX;
				}
			}

			cEffectSceneNode* lEff = SetLinkdEffect( eLINK_LWEAPON, lEffectName.Cstr(), &trans, true, true );
			if( lEff )
				mWeaponObjectEffect[eHAND_LEFT] = lEff->GetIndexByManger();
		}
		break;
	case eWEAPON_STATE_SHEILD:
		{
			if( info->WeaponEnhanced[eHAND_LEFT] > 11 )
				lEffectName = "./data/effect/eff_enhance_shield4.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 8 )
				lEffectName = "./data/effect/eff_enhance_shield3.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 5 )
				lEffectName = "./data/effect/eff_enhance_shield2.nif";
			else if( info->WeaponEnhanced[eHAND_LEFT] > 2 )
				lEffectName = "./data/effect/eff_enhance_shield1.nif";


			cEffectSceneNode* lEff = SetLinkdEffect( eLINK_LWEAPON, lEffectName.Cstr(), 0, true, true );
			if( lEff )
				mWeaponObjectEffect[eHAND_LEFT] = lEff->GetIndexByManger();
		}
		break;
	default:
		break;
	}

	ShowWeapon( mShowWeapon );

	return true;
}

void cPlayer::UnEquipWeaponItem( eHAND_TYPE handType )
{
	if( IsTransformMonster() == true )
		return;

	/// 아무것도 장착되어 있지 않다면..
	if( GetWeaponState() == eWEAPON_STATE_NONE )
	{
		/// 아무것도 장착되어 있지 않은데 이 함수를 호출했다면, 다시한번 이함수를 호출부를 확인하세요.
		//assert(0);
		return;
	}

	if( GetWeaponState() == eWEAPON_STATE_SHORTSWORD || handType == eHAND_ALL )
	{
		/// 1. 양손 모두 그래픽 데이터 제거
		for(unsigned int ui=0;ui<eHAND_ALL;ui++)
		{
			mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[ui] );
			mWeaponObjectIndex[ui] = UINT_MAX;
		}

		/// 2. 무기 상태 변경
		ChangeWeaponState(eWEAPON_STATE_NONE);
	}
	else
	{
		/// 1. 그래픽 데이터 제거
		mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[handType] );
		mWeaponObjectIndex[handType] = UINT_MAX;

		/// 2. 무기 상태 변경
		if( handType == eHAND_LEFT )
		{	/// 왼손 제거의 경우
			if( GetWeaponState() >= eWEAPON_STATE_SHEILD ) 
			{
				/// 방패 착용중이면 ( eWEAPON_STATE_NONE 또는 eWEAPON_STATE_SWORD 상태로 변경 )
				ChangeWeaponState( (eWEAPON_STATE)(GetWeaponState() -eWEAPON_STATE_SHEILD) );
			}
			else if( GetWeaponState() == eWEAPON_STATE_DOUBLESWORD )
			{
				ChangeWeaponState(eWEAPON_STATE_SWORD);
			}
		}
		else 
		{	/// 오른손 제거의 경우
			if( GetWeaponState() >= eWEAPON_STATE_SHEILD )
			{
				ChangeWeaponState(eWEAPON_STATE_SHEILD);
			}
			else if( GetWeaponState() == eWEAPON_STATE_DOUBLESWORD )
			{
				ChangeWeaponState(eWEAPON_STATE_DOUBLESWORD);
			}
			else
			{
				ChangeWeaponState(eWEAPON_STATE_NONE);
			}
		}
	}
}

/* ------------------------------------------------------------------
* 함수이름 :	(( float time )
* 목    적 :	애니메이션 관계 설정
* 주의사항 :	
* ------------------------------------------------------------------ */
void cPlayer::Interpret( unsigned long time )
{
	if( IsTransformMonster() == true )
	{
		return Interpret_TransMon( time );
	}

	if( mpObjectSceneNode->GetType() == cSceneNode::eVEHICLE )
	{
		return InterpretVehicle( time );
	}

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

	InterpretEvent( id, checkTime, pActor );

	if( endTime < checkTime )
	{
		/// 공격이 종료 되었으면... 기본 동작으로 변경
		if( id >= ANITYPE_IDLE && id < ANITYPE_IDLE2 )
		{
			return;
		}
		else if( id >= ANITYPE_IDLE2 && id < ANITYPE_RUN )
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
				{
					SetState( eOBJECT_STATE_IDLE );
				}
				break;
			default:
				ChangeAnimation( ANITYPE_IDLE );
				break;
			}

			ShowWeapon( true );
		}
		else if( id >= ANITYPE_ATTACK1 && id < ANITYPE_CASTING1 )
		{
			if( IsReadyUseSkill() == true )
				SetState( eOBJECT_STATE_IDLE );
			else
				ChangeAnimation( ANITYPE_IDLE );
		}
		else if( (id >= ANITYPE_COMMON_SKILL_BEGIN && id < ANITYPE_COMMON_SKILL_END) ||
	 			 (id >= ANITYPE_SKILL_BEGIN && id < ANITYPE_SKILL_END) )
		{
			if( IsReadyUseSkill() == true )
				SetState( eOBJECT_STATE_IDLE );
			else
				ChangeAnimation( ANITYPE_IDLE );
		}
		else if( mPlayerExrInfo.mState == ePLAYER_STATE_ITEMPICK )
		{
			SetRemoveDropItem( UINT_MAX );
			SetState( eOBJECT_STATE_IDLE );
			//			ChangeAnimation( ANITYPE_IDLE );
			ShowWeapon( true );
		}
		else if( id == ANITYPE_PEACE_IDLE2 )
		{
			ChangeAnimation( ANITYPE_IDLE );
		}
		else if( id >= ANITYPE_DAMAGE1 && id < ANITYPE_DIE1 )
		{
			ChangeAnimation( ANITYPE_IDLE );
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
		else if( id >= ANITYPE_DAMAGE1 && id < ANITYPE_DIE1 )
		{
			if( ANITYPE_CASTING1 <= mDamageAfterAnim && mDamageAfterAnim < ANITYPE_DAMAGE1 )
			{
				ChangeAnimation( (ePLAYER_ANIMATIONTYPE)mDamageAfterAnim );
			}
			else
				SetState( eOBJECT_STATE_IDLE );

			ShowWeapon( true );
		}
		else if( id >= ANITYPE_EMOTE_START && id < ANITYPE_VEHICLE_IDLE )
		{
			SetState( eOBJECT_STATE_IDLE );
			ShowWeapon( true );
		}
		else if( id >= ANITYPE_ENHANCEDSUCCESS && id <= ANITYPE_ENHANCEDFAIL )
		{
			SetState( eOBJECT_STATE_IDLE );
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
	}
}

void cPlayer::InterpretVehicle( unsigned long time )
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

void cPlayer::InterpretEvent( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

	unsigned long soundIdx = 0;
	if( id == ANITYPE_GATHERING1 || id == ANITYPE_GATHERING2 )
	{
		if( mRevisionEventTime[eANIEVENT_GATHER_MINING] > 0.0f && mRevisionEventTime[eANIEVENT_GATHER_MINING] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
			soundIdx = 2037;

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
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
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

	if( soundIdx )
		SetStepSound( soundIdx );
}

void cPlayer::InterpretEvent_TransMon( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

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

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "FootStep_Right" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_RUN_RIGHT] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_RUN_RIGHT] > 0.0f && mRevisionEventTime[eANIEVENT_RUN_RIGHT] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;

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
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
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
}

void cPlayer::InterpretEvent_Vehicle( unsigned long id, float checkTime, cActorManagerForPartition* pActor )
{
	if( pActor == 0 )
		return;

	if( id == V_ANITYPE_JUMP )
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
			mJumpProcess = false;

			float eventTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "Jump_End" );
			if( eventTime != cActorManagerForPartition::INVALID_TIME )
				mRevisionEventTime[eANIEVENT_JUMP_END] = eventTime;
		}

		if( mRevisionEventTime[eANIEVENT_JUMP_END] > 0.0f && mRevisionEventTime[eANIEVENT_JUMP_END] <= checkTime )
		{
			mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
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
}


void cPlayer::ChangeWeaponState( eWEAPON_STATE state )
{
	if( IsTransformMonster() == true )
		return;

	/// 종족, 성별에 따른 무기 Scale조정
	float fScale = 1.00f;
	if( mPlayerInfo.Race == eRACE_HUMAN )
	{
		fScale = (mPlayerInfo.Gender == eGENDER_MALE)? 1.03f:1.06f;
	}
	else if( mPlayerInfo.Race == eRACE_ELF )
	{
		fScale = (mPlayerInfo.Gender == eGENDER_MALE)? 1.15f:1.09f;
	}
	else if( mPlayerInfo.Race == eRACE_BEAST )
	{
		fScale = (mPlayerInfo.Gender == eGENDER_MALE)? 1.12f:1.00f;
	}

	NiAVObject* pobj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
	if( pobj )
		pobj->SetScale( fScale );

	pobj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
	if( pobj )
		pobj->SetScale( fScale );

	mPlayerWeaponState = state;

	/// 현재 자신의 상태를 재확인 한다.
	if( ANITYPE_COMMON >= mAnimationType )
	{
		/// 공통 동작은 제외한다.
		if( mPlayerExrInfo.mState != eOBJECT_STATE_DIE )
			SetState( mPlayerExrInfo.mState );
	}

	/// 현재 무기가 보여지는 상태에 맞게 표시한다.
	ShowWeapon( mShowWeapon );

	mpObjectSceneNode->UpdateLinkEffect();
}

void cPlayer::ChangeAnimation( ePLAYER_ANIMATIONTYPE type )
{
	if( IsTransformMonster() == true )
	{
		//assert(0);
		return;
	}

	mIdleTime = 0;
	
	unsigned int aniIdx;
	if( ANITYPE_COMMON < type )
	{
		/// 공통 처리 애니
		aniIdx = type;
	}
	else
	{
		if( type == ANITYPE_DAMAGE1 || type == ANITYPE_DAMAGE2 )
			UpdateAnimationSpeed( 1.0f );

		if( mPlayerExrInfo.mStateIdle != eIDLE_PEACE )
		{
			/// 무기별 처리 애니
			eWEAPON_STATE state = GetWeaponState();
			switch( state )
			{
			case eWEAPON_STATE_SHEILD:
				aniIdx = type + eWEAPON_STATE_NONE;
				break;
			case eWEAPON_STATE_SWORD_SHEILD:
				aniIdx = type + eWEAPON_STATE_SWORD;
				break;
			default : 
				aniIdx = type + state; 
				break;
			}
		}
		else
		{
			aniIdx = type;
		}
	}

	mAnimationType = type;
	mpObjectSceneNode->SetTargetAnimation( aniIdx );

	if( type >= ANITYPE_EMOTE_START ||
		type == ANITYPE_DUELWIN ||
		type == ANITYPE_DUELLOSE )
	{
		ShowWeapon( false );
	}

	/// 이벤트 타임 초기화
	if( aniIdx >= ANITYPE_RUN && aniIdx < ANITYPE_ATTACK1 )
	{
		mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
		mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
		mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
	}
	else if( aniIdx == ANITYPE_GATHERING1 || aniIdx == ANITYPE_GATHERING2 )
	{
		mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
		mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
		mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
		mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;

	}
	else if( aniIdx >=ANITYPE_JUMP && aniIdx < ANITYPE_JUMP_END )
	{
		mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
		mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
		mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
	}
}

void cPlayer::UpdateSkillAnimation( unsigned int aniIdx, bool checkWeapon )
{
	SetJumpFlag( false );

	if( IsTransformMonster() == true )
	{
		cBaseObject::UpdateSkillAnimation( aniIdx );
		return;
	}

	unsigned int idx = aniIdx;

	if( (aniIdx >= ANITYPE_ATTACK1 && aniIdx <= ANITYPE_DAMAGE2)  ||
		(aniIdx >= ANITYPE_COMMON_SKILL_BEGIN && aniIdx < ANITYPE_COMMON_SKILL_END) ||
		(aniIdx >= ANITYPE_SKILL_BEGIN && aniIdx < ANITYPE_SKILL_END) )
	{
		unsigned int type = aniIdx - aniIdx%10;

		eWEAPON_STATE state = GetWeaponState();
		switch( state )
		{
		case eWEAPON_STATE_NONE:
		case eWEAPON_STATE_SHEILD:
			idx = type + eWEAPON_STATE_SWORD;
			break;
		case eWEAPON_STATE_SWORD_SHEILD:
			idx = type + eWEAPON_STATE_SWORD;
			break;
		default : 
			idx = type + state; 
			break;
		}
	}

	if( checkWeapon == true )
		cBaseObject::UpdateSkillAnimation( idx );
	else
		cBaseObject::UpdateSkillAnimation( aniIdx );
}

float cPlayer::GetFixedObjectSize()
{ 
	if( mFixedObjectSize <= 0.0f )
	{
		assert(NULL);
		return 1.0f;
	}

	return mFixedObjectSize;
}

void cPlayer::SetFixedObjectSizeScale( unsigned short scalePer, bool init )
{
	mPlayerExrInfo.mFixedObjectSizePer = scalePer;

	if( IsTransformMonster() == false )
	{
		if( IsRideVehicle() == true )
		{
			sVehicleList* pInfo = VEHICLESCRIPT->GetVehicleInfo( mPlayerExrInfo.mVehicleIdx );
			if( pInfo == 0 )
			{
				assert(0);
				return;
			}
			mFixedObjectSize = pInfo->mObjectFixSize * (float)scalePer * 0.01f;
		}
		else
		{
			mFixedObjectSize = mPlayerExrInfo.mBaseFixedSize * (float)scalePer * 0.01f;
		}

		assert(mpObjectSceneNode);
		mpObjectSceneNode->SetTargetScale( 1.0f * (float)scalePer * 0.01f, init );
	}
	else
	{
		sMonsterScript* pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		if( pMonsterInfo == NULL )
		{
			assert(0);
			return ;
		}

		float scale = 1.0f;
		if( pMonsterInfo->mMonsterScale > 0.0f )
		{
			scale = pMonsterInfo->mMonsterScale;
		}
		else
		{
			assert(0);
		}

		mFixedObjectSize = ( pMonsterInfo->mMonsterFixSize * scale ) * (float)scalePer * 0.01f;
		mpObjectSceneNode->SetTargetScale( scale * (float)scalePer * 0.01f, init );
	}
}

bool cPlayer::TransformMonster( unsigned long monsterClassIdx )
{
	mReadyTransformInfo.readState = eReady_None;
	mReadyTransformInfo.chgMonIdx = 0;

	if( monsterClassIdx == 0 )
		return false;

	if( IsRideVehicle() == true )
	{
		assert(0);
		return false;
	}

	mPlayerExrInfo.mChgMonsterIdx = monsterClassIdx;

	/// 외형 몬스터화
	sMonsterScript* pMonsterInfo;
	pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
	if( pMonsterInfo == NULL )
	{
		assert(0);
		return false;
	}

	const char* strFile = GAMERESOURCEMAN->GetModelFileName( pMonsterInfo->mKfmIdx );
	assert( strFile );

	cString path;
	path.Format( "./Data/Monster/%s", strFile );

	cMonsterSceneNodeParam param;
	param.mpObject = this;
	param.mPathName = path;
	param.mHeadHeight = (float)pMonsterInfo->mNameHeight;
	param.mTranslate = GetPos();
	param.mRotate = mpObjectSceneNode->GetRotate();
	param.mScale = 1.0f;

	/// buff/debuff effect set
	{
		cInfluenceMap::cIterator i = mBuffMap.Begin();
		cInfluenceMap::cIterator end = mBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}

		i = mDeBuffMap.Begin();
		end = mDeBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}
	}

	/// 기존 객체 제거
	if( mpObjectSceneNode )
	{
		SCENEMAN->DestroyDynamic( mpObjectSceneNode );
		mpObjectSceneNode = 0;
	}


	/// 신규 객체 생성
	bool isHero = ( HERO->GetObjectID() == mPlayerInfo.CharacterIdx ) ? true : false;
	
	cTransMonSceneNode* pSceneNode = SCENEMAN->CreateTransMonster( param, isHero );
	if( pSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	mpObjectSceneNode = pSceneNode;
	mpObjectSceneNode->SetTargetAnimation( 0 );

	//PVPMAN->LinkPvpForceEffect( this, mPlayerExrInfo.mForceType );

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, false );

//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetWorldScale();

	SetState( mPlayerExrInfo.mState );

	for( unsigned int i = 0; i<eCOSTUME_MAX; i++ )
		mCostumeObjectIndex[i] = UINT_MAX;

	for( unsigned int i = 0; i < eHAND_ALL; i++ )
	{
		mWeaponObjectIndex[i] = UINT_MAX;

		if( mWeaponObjectEffect[i] == UINT_MAX )
			continue;

		cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mWeaponObjectEffect[i] );

		if( eff )
			eff->Remove();
		mWeaponObjectEffect[i] = UINT_MAX;
	}

	mIdleTime = 0;

	mpObjectSceneNode->UpdateNameCardTextValue();
	mpObjectSceneNode->UpdateNameCardGaugeValue();

	if( PARTYMAN->IsHeroParty( GetObjectID() ) == true ||
		PARTYUNIONMAN->IsHeroParty( GetObjectID() ) == true )
		PartyIn_Init();
	else
		PartyOut_Init();

	///변신 이펙트 & 사운드 설정
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 0.0f );
	SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_change_01.nif", &trans, false, true );
	SetLinkdSound( 2029 );
	
	return true;
}

void cPlayer::RestoreMonster()
{
	mReadyTransformInfo.readState = eReady_None;
	mReadyTransformInfo.chgMonIdx = 0;

	if( IsJumping() == true )
		SetJumpFlag( false );

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		assert(0);
		return;
	}

	if( IsRideVehicle() == true )
	{
		assert(0);
		return;
	}

	mPlayerExrInfo.mChgMonsterIdx = 0; 


	/// 외형 복구
	const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
	cPlayerSceneNodeParam param;
	param.mpObject = this;
	param.mPathName = strFile;

	param.mTranslate = mpObjectSceneNode->GetTranslate();
	param.mRotate = mpObjectSceneNode->GetRotate();
	//param.mScale = mpObjectSceneNode->GetScale();

	//param.mTranslate = mpObjectSceneNode->GetWorldTranslate();
	//param.mRotate = mpObjectSceneNode->GetWorldRotate();
	//param.mScale = (float)mPlayerExrInfo.mFixedObjectSizePer * 0.01f;
	param.mScale = 1.0f;

	/// buff/debuff effect set
	{
		cInfluenceMap::cIterator i = mBuffMap.Begin();
		cInfluenceMap::cIterator end = mBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}

		i = mDeBuffMap.Begin();
		end = mDeBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}
	}

	/// 기존 객체 제거
	if( mpObjectSceneNode )
	{
		SCENEMAN->DestroyDynamic( mpObjectSceneNode );
		mpObjectSceneNode = 0;
	}

	/// 신규 객체 생성
	cDynamicSceneNode* node = CreateSceneNode( &param );
	if( node == 0 )
	{
		assert(0);
		return;
	}
	mpObjectSceneNode = node;
	mpObjectSceneNode->SetTargetAnimation( 0 );

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

	/// 게임 진입시 상황에 맞는 최종 동작을 유지하도록 보정.
	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_DIE:
		{
			ChangeAnimation( ANITYPE_DIE1 );
			mpObjectSceneNode->GotoLastFrame();
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
			ChangeAnimation( ANITYPE_SITDOWN_IDLE );
			mpObjectSceneNode->GotoLastFrame();
		}
		break;
	case eOBJECT_STATE_STOP:
		{
			/// flag별 동작 변경
			switch( mPlayerExrInfo.mStateStop )
			{
			case eSTOP_READYTAROT:
				{
					ActiveTarotBubble( (LPTSTR)GAMERESOURCEMAN->GetGameText( 300 ), COLOR_SKYBLUE );
					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_OPENTAROT:
				{
					ActiveTarotBubble( (LPTSTR)mTarotTitle.Cstr(), COLOR_SKYBLUE );
					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_OPENSTALL:
				{
					ActiveUserSellBubble( (LPTSTR)mSellTitle.Cstr(), COLOR_GREEN );

					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_ENHANCED:
				{
					SetItemEnhanceEffect();
					ChangeAnimation( ANITYPE_ENHANCED );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_ITEMMIX:
				{
					SetItemMixEffect();
					ChangeAnimation( ANITYPE_ENHANCED );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_ENTERTAROT:
			case eSTOP_USETAROT:
			case eSTOP_NPCSPEECH:
			case eSTOP_CHEATSTOP:
			case eSTOP_USESTALL:
				{
					ChangeAnimation( ANITYPE_IDLE );
				}
				break;
			}
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			ChangeAnimation( ANITYPE_RUN );
		}
		break;
	case eOBJECT_STATE_IDLE:
		{
			/// emoting check
			if( mPlayerExrInfo.mCommunitySkillIdx != 0 )
			{
				sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( mPlayerExrInfo.mCommunitySkillIdx );
				if( pInfo )
				{
					if( pInfo->mType == 1 )
						ChangeAnimation( (ePLAYER_ANIMATIONTYPE)pInfo->mRoleIndex );
				}
			}

			ChangeAnimation( ANITYPE_IDLE );
		}
		break;
	default:
		ChangeAnimation( ANITYPE_IDLE );
		break;
	}

	//PVPMAN->LinkPvpForceEffect( this, mPlayerExrInfo.mForceType );

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, false );

//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetScale();

	mIdleTime = 0;

	mpObjectSceneNode->UpdateNameCardTextValue();
	mpObjectSceneNode->UpdateNameCardGaugeValue();

	if( PARTYMAN->IsHeroParty( GetObjectID() ) == true || 
		PARTYUNIONMAN->IsHeroParty( GetObjectID() ) == true )
		PartyIn_Init();
	else
		PartyOut_Init();

	/// 변신 해제 이펙트 & 사운드 출력
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 0.0f );
	this->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_change_02.nif", &trans, false, true );
	this->SetLinkdSound( 2030 );

}

void cPlayer::SetRemoveDropItem( unsigned int idx ) 
{ 
	//if( mRemoveDropItemIdx !=  UINT_MAX )
	//	ITEMMANAGER->Remove( mRemoveDropItemIdx );

	mRemoveDropItemIdx = idx; 
}

void cPlayer::SetState( unsigned char state , bool bShowAni )
{
	mIdleTime = 0;

	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		SetState_TransMon( state, bShowAni );
		return;
	}

	/// 죽은 경우 무조건 다음 동작으로 대기가 와야 한다.
	if( mPlayerExrInfo.mState == eOBJECT_STATE_DIE )
	{
		if( state != eOBJECT_STATE_IDLE )
		{
//			assert(0);
			return;
		}
	}

	if( mPlayerExrInfo.mState == ePLAYER_STATE_GATHERING && state != ePLAYER_STATE_GATHERING )
	{
		if( mGatheringToolObj != UINT_MAX )
		{
			if( mpObjectSceneNode )
			{
				mpObjectSceneNode->UnLinkObject( mGatheringToolObj );
				mGatheringToolObj = UINT_MAX;
			}
			else
			{
				assert(0);
			}
		}

		ClearItemGatheringEffect();
	}

	if( mPlayerExrInfo.mState == ePLAYER_STATE_ITEMPICK &&
		state != ePLAYER_STATE_ITEMPICK )
		ShowWeapon( true );

	if( mPlayerExrInfo.mState == ePLAYER_STATE_GATHERING && 
		state != ePLAYER_STATE_GATHERING )
		ShowWeapon( true );

	if( state != ePLAYER_STATE_SITDOWN )
	{
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

	/// 기본 속도 보정
	if( state == eOBJECT_STATE_MOVE )
	{
		float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
		float moveSpeed = 1.0f;
		if( baseSpeed != 0.0f )
		{
			moveSpeed = (float)mPlayerExrInfo.mMoveSpeed / (float) baseSpeed;
		}

		if( mpObjectSceneNode && IsJumping() == false )
			mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );
	}
	else
	{
		if( mpObjectSceneNode )
			mpObjectSceneNode->UpdateAniScaleFactor( 1.0f );
	}

	/// 그래픽적인 추가 처리가 필요한 경우...
	switch( state )
	{
	case eOBJECT_STATE_IDLE:
		{
			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
			{
				ShowWeapon( true );
				if( bShowAni && mJumping == false )
					ChangeAnimation( ANITYPE_IDLE );
			}
			else
			{
				ShowWeapon( false );
				if( bShowAni && mJumping == false )
				{
					if( IsRideVehicle() == true )
						ChangeAnimation( ANITYPE_IDLE );
					else
						ChangeAnimation( ANITYPE_PEACE_IDLE );
				}
			}
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
				ShowWeapon( true );
			else
				ShowWeapon( false );

			if( bShowAni && mJumping == false )
			{
				ChangeAnimation( ANITYPE_RUN );
			}
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			SetReadyUseSkill( false );
			SetIdleFlag(eIDLE_NORMAL);
			ShowWeapon( true );

			/// 
			if( mpObjectSceneNode )
				mpObjectSceneNode->UpdateAniScaleFactor( mPlayerExrInfo.mAttackSpeed );//1.0f );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
//			if( mpObjectSceneNode )
//				mpObjectSceneNode->StopMorpher();

			ChangeAnimation( ANITYPE_DIE1 );

			/// 해당 유저가 죽으면 초기화
			DeActiveTarotBubble();
			DeActiveUserSellBubble();

			ClearItemMixEffect();
			ClearItemEnhanceEffect();
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
		}
		break;
	case ePLAYER_STATE_ITEMPICK:
		{
			if( IsJumping() == false )
				ChangeAnimation( ANITYPE_ITEMPICK );

			ShowWeapon( false );
		}
		break;
	case ePLAYER_STATE_GATHERING:
		{
			ShowWeapon( false );
		}
		break;
	case eOBJECT_STATE_STOP:
		{
			/// flag별 동작 변경
			switch( mPlayerExrInfo.mStateStop )
			{
			case eSTOP_READYTAROT:
			case eSTOP_OPENTAROT:
			case eSTOP_OPENSTALL:
				{
					ShowWeapon( false );
					if( bShowAni )
					{
						if( mAnimationType != ANITYPE_STANDUP && mAnimationType != ANITYPE_SITDOWN_IDLE )
							ChangeAnimation( ANITYPE_SITDOWN );
						else
							ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					}
				}
				break;
			case eSTOP_ENHANCED:
				{
					ShowWeapon( false );
					if( bShowAni )
						ChangeAnimation( ANITYPE_ENHANCED );
				}
				break;
			case eSTOP_ITEMMIX:
				{
					ShowWeapon( false );
					if( bShowAni )
						ChangeAnimation( ANITYPE_ENHANCED );
				}
				break;
			case eSTOP_SUMMON_PAPER:
				{
					if( IsTransformMonster() )
						ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
					else
						ChangeAnimation( ANITYPE_IDLE );
				}
				break;
			case eSTOP_NONE:
				break;
			default:
				ChangeAnimation( ANITYPE_IDLE );
				break;
			}
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			if( bShowAni )
				ChangeAnimation( ANITYPE_DAMAGE1 );

			/// 해당 유저가 죽으면 초기화
			DeActiveTarotBubble();
			DeActiveUserSellBubble();

			ClearItemMixEffect();
			ClearItemEnhanceEffect();
		}
		break;
	case ePLAYER_STATE_RUSH:
		{
			float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
			float moveSpeed = 1.0f;
			if( baseSpeed != 0.0f )
			{
				moveSpeed = (float)SKILL_RUSH_SPEED / (float) baseSpeed;
			}

			if( mpObjectSceneNode && IsJumping() == false )
				mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );

			ChangeAnimation( ANITYPE_RUN );

			SetReadyUseSkill( false );
			SetIdleFlag(eIDLE_NORMAL);
			ShowWeapon( true );
		}
		break;
	}

	mPlayerExrInfo.mState = state;

}

void cPlayer::ResetState()
{
	SetState( eOBJECT_STATE_IDLE );
}

void cPlayer::Rebirth()
{
	SetState( eOBJECT_STATE_IDLE );
	//TCHAR msg[256] = {0,};
	//::_stprintf( msg, _T("부활후정지 - %f\n"), TIMER->GetCurrentTime() );
	//OutputDebugString(msg);

//	if( mpObjectSceneNode )
//		mpObjectSceneNode->StartMorpher();

	/// 부활에 따른 hp,mp 초기화
	mPlayerExrInfo.RestHP = mPlayerExrInfo.MaxHP / 2;

	mPlayerExrInfo.RestMP = mPlayerExrInfo.MaxMP / 2;

	mpObjectSceneNode->UpdateNameCardGaugeValue();
}

void cPlayer::SitDown()
{
	ShowWeapon( false );
	SetState( ePLAYER_STATE_SITDOWN );

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		ChangeAnimation( ANITYPE_SITDOWN );
		mSitDownFlag = true;
	}

	/// effect 출력
	eSTOPFLAG flag = GetStopFlag();
	if( flag != eSTOP_READYTAROT && flag != eSTOP_OPENTAROT && flag != eSTOP_OPENSTALL )
	{
		/// effect 
		assert(mDuelEffect==0);

		NiTransform trans;
		trans.MakeIdentity();
		mSitDownEffect = SetLinkdEffect( eLINK_BODY, "./Data/Effect/Eff_PC_Rest.nif", &trans, true, true );
		if( mSitDownEffect )
			mSitDownEffectIdx = mSitDownEffect->GetIndexByManger();
	}
	else
	{
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
}

void cPlayer::StandUp()
{
//	ShowWeapon( true );

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		ChangeAnimation( ANITYPE_STANDUP );
		mSitDownFlag = true;
	}
	else
	{
		SetState( eOBJECT_STATE_IDLE );
	}

	/// effect 제거
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

void cPlayer::SetIdleFlag( unsigned char flag )
{
	if( mPlayerExrInfo.mStateIdle == flag )
		return;

	mPlayerExrInfo.mStateIdle = flag;

	if( mPlayerExrInfo.mChgMonsterIdx !=0 )
		return;

	if( flag == eIDLE_NORMAL )
	{
		if( mPlayerExrInfo.mState == eOBJECT_STATE_IDLE && mJumping == false )
		{
			ChangeAnimation( ANITYPE_IDLE );
		}

		if( mPlayerExrInfo.mState != ePLAYER_STATE_SITDOWN )
			ShowWeapon( true );
	}
	else
	{
		if( mPlayerExrInfo.mState == eOBJECT_STATE_IDLE  && mJumping == false )
		{
			if( IsRideVehicle() )
				ChangeAnimation( ANITYPE_IDLE );
			else
				ChangeAnimation( ANITYPE_PEACE_IDLE );
		}

		ShowWeapon( false );
	}
}

void cPlayer::SetStopFlag( unsigned char flag )
{
	if( flag == eSTOP_NONE )
	{
		/// 기존 플래그에 대한 처리
		if( mPlayerExrInfo.mStateStop == eSTOP_ENHANCED )
		{
			ClearItemEnhanceEffect();
		}
		else if( mPlayerExrInfo.mStateStop ==eSTOP_ITEMMIX )
		{
			ClearItemMixEffect();
		}

		SetState( eOBJECT_STATE_IDLE );

		mPlayerExrInfo.mStateStop = flag;
	}
	else
	{
		/// 신규 플래그에 대한 처리
		if( flag == eSTOP_READYTAROT )
		{
			ActiveTarotBubble( (LPTSTR)GAMERESOURCEMAN->GetGameText( 300 ), COLOR_SKYBLUE );
		}
		else if( flag == eSTOP_ITEMMIX )
		{
			SetItemMixEffect();
		}
		else if( flag == eSTOP_ENHANCED )
		{
			SetItemEnhanceEffect();
		}

		mPlayerExrInfo.mStateStop = flag;

		SetState( eOBJECT_STATE_STOP );
	}
}

void cPlayer::ActiveChatBubble( LPTSTR msg, unsigned long color )
{
	mpObjectSceneNode->ActiveChatBubble( msg, color );
}

void cPlayer::ActiveTarotBubble( LPTSTR title, unsigned long color )
{
	mTarotTitle.Format(_T("%s"), title );

	mpObjectSceneNode->ActiveTarotBubble( title, color );
}

void cPlayer::ActiveUserSellBubble( LPTSTR title, unsigned long color )
{
	mSellTitle.Format(_T("%s"), title );

	mpObjectSceneNode->ActiveUserSellBubble( title, color );

	ShowWeapon(false);
}

void cPlayer::DeActiveTarotBubble()
{
	mTarotTitle.Clear();

	mpObjectSceneNode->DeActiveTarotBubble();
}

void cPlayer::DeActiveUserSellBubble()
{
	mSellTitle.Clear();

	mpObjectSceneNode->DeActiveUserSellBubble();
}

cDynamicSceneNode* cPlayer::CreateSceneNode( cPlayerSceneNodeParam* param )
{
	return SCENEMAN->CreatePlayer( *param );
}


void cPlayer::HPChange( unsigned long objectHP, unsigned long objectMaxHP )
{
	assert(objectHP != 0);
	assert( objectHP <= objectMaxHP );

	mPlayerExrInfo.RestHP = objectHP;
	mPlayerExrInfo.MaxHP = objectMaxHP;

	mpObjectSceneNode->UpdateNameCardGaugeValue();
}

void cPlayer::HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType )
{
//	assert( damage->mObjectHP == 0 );
	assert( GetState() != eOBJECT_STATE_DIE );

	if( GetState() != eOBJECT_STATE_DIE )
	{
		if( IsRideVehicle() == true )
			AlightVehicle();

		if( pAttacker )
			SetDesiredDir( GetPos(), pAttacker->GetPos() );

		SetState( eOBJECT_STATE_DIE );
		if( attackType == eDAMAGE_CRITICAL )
		{
			if( IsTransformMonster() == true )
				ChangeAnimation_TransMon( M_ANITYPE_DIE2 );
			else
				ChangeAnimation( ANITYPE_DIE2 );

			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE2 );
		}
		else
		{
			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE1 );
		}

		/// 사망시 보유 효과 제거..
		ClearAllBuff( false );
		ClearAllDeBuff( false );
	}

	/// 죽는 경우 mp도 0으로 만들어 버린다.
	mPlayerExrInfo.RestHP = 0;
	mPlayerExrInfo.RestMP = 0;
	mPlayerExrInfo.MaxHP = objectMaxHP;

	mpObjectSceneNode->UpdateNameCardGaugeValue();
}

void cPlayer::MPChange( unsigned long objectMP, unsigned long objectMaxMP )
{
	if( GetState() == eOBJECT_STATE_DIE )
	{
		assert( objectMP==0 );
	}

	mPlayerExrInfo.RestMP = objectMP;
	mPlayerExrInfo.MaxMP = objectMaxMP;

	mpObjectSceneNode->UpdateNameCardGaugeValue();
}

void cPlayer::PlayerAttack( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pTarget )
{
	if( GetState() == eOBJECT_STATE_DIE )
		return;

	/// 바라봐야할 방향 설정
	if( pTarget )
	{
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

	SKILLMAN->CreateSkillObject( uIdx, skillIdx, 0, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::PlayerUseSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget )
{
	/// 바라봐야할 방향 설정
	if( pTarget )
	{
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

	SKILLMAN->CreateSkillObject( uIdx, skillIdx, skillStep, this, pTarget, mPlayerExrInfo.mAttackSpeed );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::PlayerUseSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, NiPoint3 fieldPos )
{
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

	NiPoint3 pos = fieldPos;
	if( WORLDMAN->CalcHeight(&pos.z, pos.x, pos.y) == false )
	{
		assert(0);
		return;
	}

	SKILLMAN->CreateSkillObject( uIdx, skillIdx, skillStep, this, pos, mPlayerExrInfo.mAttackSpeed );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::PlayerMapChangeSkill( unsigned long uIdx, unsigned long skillIdx )
{
	SKILLMAN->CreateMapChangeSkillObject( uIdx, skillIdx, this );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::PlayerVehicleSkill( unsigned long uIdx, unsigned long skillIdx )
{
	SKILLMAN->CreateVehicleSkillObject( uIdx, skillIdx, this );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::PlayerBlinkSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, NiPoint3 blinkPos )
{
	SKILLMAN->CreateBlinkSkillObject( uIdx, skillIdx, skillStep, this, blinkPos );
	SetState( eOBJECT_STATE_ATTACK );
}

void cPlayer::EquipWearInfo( eWEAR_TYPE type, unsigned long itemIdx )
{
	if( type > eWEAR_MAX )
	{
		assert(0);
		return;
	}

	/// 원피스인 경우 배제..
	if( type == eWEAR_HAT )
	{
		if( mPlayerWearInfo.CostumeIdx[type] == 0 )
		{
			if( itemIdx == 0 )
			{
				ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
				mpObjectSceneNode->HideEar( false );
			}
			else
			{
				EquipWearItem( itemIdx );
			}
		}
	}
	else
	{
		if( mPlayerWearInfo.CostumeIdx[eWEAR_BODY1] != 0 )
		{
			/// 원피스 체크
			cItemDefine* p = ITEMMAN->GetItemDefine( mPlayerWearInfo.CostumeIdx[eWEAR_BODY1] );
			if( p )
			{
				if( p->GetSubType() != ITEM_WEAR_COS_ONEPIECE )
				{
					if( mPlayerWearInfo.CostumeIdx[type] == 0 )
					{
						if( itemIdx == 0 )
							ChangeBaseWear( ePART_BODY1 + type );
						else
							EquipWearItem( itemIdx );
					}
				}
			}
			else
				assert(0);
		}
		else
		{
			if( mPlayerWearInfo.CostumeIdx[type] == 0 )
			{
				if( itemIdx == 0 )
					ChangeBaseWear( ePART_BODY1 + type );
				else
					EquipWearItem( itemIdx );
			}
		}
	}

	mPlayerWearInfo.WearIdx[type] = itemIdx;
}

void cPlayer::EquipCostumeInfo( unsigned char type, unsigned long itemIdx )
{
	if( type == eWEAR_ONEPIECE )
	{
		mPlayerWearInfo.CostumeIdx[eWEAR_BODY1] = itemIdx;
		mPlayerWearInfo.CostumeIdx[eWEAR_BODY2] = 0;
		mPlayerWearInfo.CostumeIdx[eWEAR_HAND] = 0;
		mPlayerWearInfo.CostumeIdx[eWEAR_FOOT] = 0;

		if( itemIdx != 0 )
		{
			EquipWearItem( itemIdx );
		}
		else
		{
			for( unsigned int i=eWEAR_BODY1; i<=eWEAR_FOOT; i++ )
			{
				if( mPlayerWearInfo.WearIdx[i] != 0 )
					EquipWearItem( mPlayerWearInfo.WearIdx[i] );
				else
					ChangeBaseWear( ePART_BODY1+i );
			}
		}
	}
	else
	{
		if( type >= eWEAR_MAX )
		{
			assert(0);
			return;
		}

		mPlayerWearInfo.CostumeIdx[type] = itemIdx;
		if( itemIdx != 0 )
		{
			EquipWearItem( itemIdx );
		}
		else
		{
			if( type == eWEAR_HAT )
			{
				ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
				mpObjectSceneNode->HideEar( false );

				if( mPlayerWearInfo.WearIdx[type] != 0 )
					EquipWearItem( mPlayerWearInfo.WearIdx[type] );
			}
			else
			{
				if( mPlayerWearInfo.WearIdx[type] != 0 )
					EquipWearItem( mPlayerWearInfo.WearIdx[type] );
				else
					ChangeBaseWear( ePART_BODY1 + type );
			}
		}
	}
}

void cPlayer::EquipCostumeAcceInfo( unsigned char type, unsigned long itemIdx )
{
	if( type >= eCOSTUME_MAX )
	{
		assert(0);
		return;
	}

	EquipAccessoryItem( type, itemIdx );
	mPlayerWearInfo.CostumeAccIdx[type] = itemIdx;
}

void cPlayer::EquipWeaponInfo( eWEAPON_STATE state, sPlayerWeaponInfo* pInfo )
{
	bool changeWeapon = false;
	for( unsigned int i=0; i<eHAND_ALL; i++ )
	{
		if( pInfo->WeaponIdx[i] != mPlayerWeaponInfo.WeaponIdx[i] ||
			pInfo->WeaponEnhanced[i] != mPlayerWeaponInfo.WeaponEnhanced[i] )
		{
			changeWeapon = true;
			break;
		}
	}

	if( changeWeapon == true )
	{
		if( EquipWeaponItem( pInfo ) == false )
		{
			assert(0);
			return;
		}
	}
	
	::memcpy( &mPlayerWeaponInfo, pInfo, sizeof(sPlayerWeaponInfo) );
	mPlayerWeaponState = state;

	if( GetWeaponState() != state )
	{
		assert(0);
		return;
	}
}

bool cPlayer::GetDummyPos( unsigned int dummyId, NiPoint3& pos )
{
	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		if( dummyId == eLINK_RWEAPON || dummyId == eLINK_LWEAPON )
			return false;
		return cBaseObject::GetDummyPos( dummyId, pos );
	}

	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	if( dummyId == eLINK_RWEAPON )
	{
		NiAVObject* pObj = mpObjectSceneNode->GetWeaponDummy( mWeaponObjectIndex[eHAND_RIGHT], false );
		if( pObj == 0 )
			return false;

		pos = pObj->GetWorldTransform().m_Translate;
		return true;
	}
	else if( dummyId == eLINK_LWEAPON )
	{
		NiAVObject* pObj = mpObjectSceneNode->GetWeaponDummy( mWeaponObjectIndex[eHAND_LEFT], true );
		if( pObj == 0 )
			return false;

		pos = pObj->GetWorldTransform().m_Translate;
		return true;
	}

	return cBaseObject::GetDummyPos( dummyId, pos );
}

void cPlayer::ShowWeapon( bool show )
{
	if( IsRideVehicle() == true )
	{
		/// 오른손
		NiAVObject* pObj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
		if( pObj )
			cSceneNode::SetAppCulled( pObj, true );

		/// 왼손
		pObj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
		if( pObj )
			cSceneNode::SetAppCulled( pObj, true );
	}
	else if( IsTransformMonster() == false )
	{
		if( show == true && mPlayerExrInfo.mStateIdle == eIDLE_PEACE )
			return;

		/// 오른손
		NiAVObject* pObj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_RIGHT] );
		if( pObj )
			cSceneNode::SetAppCulled( pObj, !show );

		/// 왼손
		pObj = mpObjectSceneNode->GetLinkObject( mWeaponObjectIndex[eHAND_LEFT] );
		if( pObj )
			cSceneNode::SetAppCulled( pObj, !show );
	}

	mShowWeapon = show;
}

void cPlayer::ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker )
{
	cBaseObject::ApplyDamageDrama( damage, damageType, pAttacker );

	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		return ApplyDamageDrama_TransMon( damage, damageType, pAttacker );
	}


	if( GetState() == eOBJECT_STATE_DIE )
		return;

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
	case ANITYPE_DUELWIN:
	case ANITYPE_DUELLOSE:
	case ANITYPE_EMOTE_START:
		return;
	}

	if( aniIdx >= ANITYPE_CASTING1 && aniIdx < ANITYPE_DAMAGE1 )
		mDamageAfterAnim = mAnimationType;

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

		/// change dir
		if( GetState() != eOBJECT_STATE_ATTACK )
		{
			if( aniIdx >= ANITYPE_IDLE && aniIdx <= ANITYPE_IDLE2 )
			{
				if( pAttacker )
					SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 200 );
			}
			if( mAnimationType == ANITYPE_PEACE_IDLE || mAnimationType == ANITYPE_PEACE_IDLE2 )
			{
				if( pAttacker )
					SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 200 );
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

void cPlayer::FaceProcess( unsigned long delta )
{
	if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		assert(0);
		return;
	}

	/// emotion enable check
	if( mEmotionRemainTime != 0 )
	{
		if( mEmotionRemainTime <= delta )
		{
			mEmotionRemainTime = 0;
			if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
				ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] );
			else
				ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE] );
		}
		else
			mEmotionRemainTime -= delta;
	}
	else
	{
		if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] == 0 )
		{
			/// eye check
			if( mWinkRemainTime != 0 )
			{
				/// wink end check
				if( mWinkRemainTime <= delta )
				{
					mWinkRemainTime = 0;
					if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
						ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] );
					else
						ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE] );
				}
				else
					mWinkRemainTime -= delta;
			}
			else
			{
				/// next wink check
				mWinkTime += delta;
				if( mWinkTime >= WINKINTERVAL_TIME )
				{
					mWinkTime = 0;
					mWinkRemainTime = WINKREMAIN_TIME;

					ChangeFaceTexture( mWinkFaceIdx );
				}
			}
		}
	}
}

void cPlayer::SetEmotionFace( unsigned long faceIdx )
{
	mWinkTime = 0;
	mWinkRemainTime = 0;

	mEmotionRemainTime = EMOTIONREMAIN_TIME;

	ChangeFaceTexture( faceIdx );
}

void cPlayer::SetStaticFace( unsigned long faceIdx )
{
	mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] = faceIdx;

	///
	if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] );
	else
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE] );
}

void cPlayer::SetDuelEffect()
{
/*
	assert( mDuelEffect==0 );

	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 10.0f );

	mDuelEffect = SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_Target_01.nif", &trans, true, true );
*/
}

void cPlayer::ClearDuelEffect()
{
	if( mDuelEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mDuelEffectIdx );
		if( node )
		{
			assert( node == mDuelEffect );
			node->Remove();
		}
		mDuelEffect = 0;
		mDuelEffectIdx = (unsigned long)-1;
	}
}

void cPlayer::SetItemMixEffect()
{
	if( IsTransformMonster() == true )
		return;

	if( mItemMixEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemMixEffectIdx );
		if( node )
		{
			assert( node == mItemMixEffect );
			node->Remove();
		}
		mItemMixEffect = 0;
		mItemMixEffectIdx = (unsigned long)-1;
	}

	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, -40.0f, 0.0f );
	trans.m_fScale = 0.5f;

 	mItemMixEffect = SetLinkdEffect( eLINK_BODY, "./Data/Effect/eff_pc_mix_01.nif", &trans, true, true );

	if( mItemMixEffect )
		mItemMixEffectIdx = mItemMixEffect->GetIndexByManger();

	/// 자기 자신만 사운드 재생
	if( GetObjectID() == HERO->GetObjectID() )
	{
		if( mItemMixSoundIdx != (unsigned long)-1 )
		{
			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( mItemMixSoundIdx );
			if( node )
			{
				assert( node == mItemMixSoundNode );
				node->Remove();
			}
			mItemMixSoundNode = 0;
			mItemMixSoundIdx = (unsigned long)-1;
		}

		mItemMixSoundNode = SetLinkdSound( 2020 , true );

		if( mItemMixSoundNode )
			mItemMixSoundIdx = mItemMixSoundNode->GetIndexByManger();
	}
}

void cPlayer::ClearItemMixEffect()
{
	if( mItemMixEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemMixEffectIdx );
		if( node )
		{
			assert( node == mItemMixEffect );
			node->Remove();
		}
		mItemMixEffect = 0;
		mItemMixEffectIdx = (unsigned long)-1;
	}
	
	if( mItemMixSoundIdx != (unsigned long)-1 )
	{
		cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( mItemMixSoundIdx );
		if( node )
		{
			assert( node == mItemMixSoundNode );
			node->Remove();
		}
		mItemMixSoundNode = 0;
		mItemMixSoundIdx = (unsigned long)-1;
	}
}

void cPlayer::SetItemEnhanceEffect()
{
	if( IsTransformMonster() == true )
		return;

	if( mItemEnhanceEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemEnhanceEffectIdx );
		if( node )
		{
			assert( node == mItemEnhanceEffect );
			node->Remove();
		}

		mItemEnhanceEffect = 0;
		mItemEnhanceEffectIdx = (unsigned long)-1;
	}

	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, -40.0f, 0.0f );

	mItemEnhanceEffect = SetLinkdEffect( eLINK_BODY, "./Data/Effect/eff_pc_enchant_01.nif", &trans, true, true );
	
	if( mItemEnhanceEffect )
		mItemEnhanceEffectIdx = mItemEnhanceEffect->GetIndexByManger();

	/// 자기 자신만 사운드 재생
	if( GetObjectID() == HERO->GetObjectID() )
	{
		if( mItemEnhanceSoundIdx != (unsigned long)-1 )
		{
			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( mItemEnhanceSoundIdx );
			if( node )
			{
				assert( node == mItemEnhanceSoundNode );
				node->Remove();
			}

			mItemEnhanceSoundNode = 0;
			mItemEnhanceSoundIdx = (unsigned long)-1;
		}

		mItemEnhanceSoundNode = SetLinkdSound( 2022, true );
		if( mItemEnhanceSoundNode )
			mItemEnhanceSoundIdx = mItemEnhanceSoundNode->GetIndexByManger();
	}
}

void cPlayer::ClearItemEnhanceEffect()
{
	if( mItemEnhanceEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemEnhanceEffectIdx );
		if( node )
		{
			assert( node == mItemEnhanceEffect );
			node->Remove();
		}

		mItemEnhanceEffect = 0;
		mItemEnhanceEffectIdx = (unsigned long)-1;
	}

	if( mItemEnhanceSoundIdx != (unsigned long)-1 )
	{
		cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( mItemEnhanceSoundIdx );
		if( node )
		{
			assert( node == mItemEnhanceSoundNode );
			node->Remove();
		}

		mItemEnhanceSoundNode = 0;
		mItemEnhanceSoundIdx = (unsigned long)-1;
	}

}

void cPlayer::SetItemGatheringEffect( eGATHERING_TYPE gatheringType )
{
	if( mItemGatheringEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemGatheringEffectIdx );
		if( node )
		{
			assert( node == mItemGatheringEffect );
			node->Remove();
		}

		mItemGatheringEffect = 0;
		mItemGatheringEffectIdx= (unsigned long)-1;
	}

	if( IsTransformMonster() == true )
		return;
	if( gatheringType != eGATHERINGTYPE_COMMON )
		return;

	NiTransform trans;
	trans.MakeIdentity();

	trans.m_Translate = NiPoint3( 0.0f, -40.0f, 40.0f );
	trans.m_fScale = 0.5f;
	mItemGatheringEffect = SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_collection_02.nif", &trans, true, true );

	if( mItemGatheringEffect )
		mItemGatheringEffectIdx = mItemGatheringEffect->GetIndexByManger();
}

void cPlayer::ClearItemGatheringEffect()
{
	if( mItemGatheringEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mItemGatheringEffectIdx );
		if( node )
		{
			assert( node == mItemGatheringEffect );
			node->Remove();
		}
		
		mItemGatheringEffect = 0;
		mItemGatheringEffectIdx= (unsigned long)-1;
	}
}

void cPlayer::SetItemDisjointSound()
{
	mItemDisjointSoundNode = SetLinkdSound( 2018 , true );
	if( mItemDisjointSoundNode )
		mItemDisjointSoundIdx = mItemDisjointSoundNode->GetIndexByManger();

}

void cPlayer::ClearItemDisjointSound()
{
	if( mItemDisjointSoundIdx != (unsigned long)-1 )
	{
		cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( mItemDisjointSoundIdx );
		if( node )
		{
			assert( node == mItemDisjointSoundNode );
			node->Remove();
		}

		mItemDisjointSoundNode = 0;
		mItemDisjointSoundIdx = (unsigned long)-1;
	}
}

void cPlayer::AddBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	cBaseObject::AddBuff( uniqueIdx, p );

	if( DUELMAN->IsDuelEnable() )
	{
		if( DUELMAN->IsDuelEnemy( this ) )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->AddBuff( true, uniqueIdx, p->GetInfluenceIdx() );
		}
	}

	if( GetPartyIndex() > 0 && HERO->GetPartyIndex() == GetPartyIndex() )
	{
		PARTYMAN->AddBuff( GetObjectID(), uniqueIdx, p->GetInfluenceIdx() );
	}

	if( GetPartyUnionIndex() > 0 && HERO->GetPartyUnionIndex() == GetPartyUnionIndex() )
	{
		PARTYUNIONMAN->AddBuff( GetObjectID(), uniqueIdx, p->GetInfluenceIdx() );
	}
}

void cPlayer::DelBuff( unsigned long uniqueIdx )
{
	cBaseObject::DelBuff( uniqueIdx );

	if( DUELMAN->IsDuelEnable() )
	{
		if( DUELMAN->IsDuelEnemy( this ) )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->DelBuff( true, uniqueIdx );
		}
	}

	if( GetPartyIndex() > 0 && HERO->GetPartyIndex() == GetPartyIndex() )
	{
		PARTYMAN->DeleteBuff( GetObjectID(), uniqueIdx );
	}

	if( GetPartyUnionIndex() > 0 && HERO->GetPartyUnionIndex() == GetPartyUnionIndex() )
	{
		PARTYUNIONMAN->DeleteBuff( GetObjectID(), uniqueIdx );
	}
}

void cPlayer::AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	cBaseObject::AddDeBuff( uniqueIdx, p );

	if( DUELMAN->IsDuelEnable() )
	{
		if( DUELMAN->IsDuelEnemy( this ) )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->AddDeBuff( true, uniqueIdx, p->GetInfluenceIdx() );
		}
	}

	if( GetPartyIndex() > 0 && HERO->GetPartyIndex() == GetPartyIndex() )
	{
		PARTYMAN->AddBuff( GetObjectID(), uniqueIdx, p->GetInfluenceIdx() );
	}

	if( GetPartyUnionIndex() > 0 && HERO->GetPartyUnionIndex() == GetPartyUnionIndex() )
	{
		PARTYUNIONMAN->AddBuff( GetObjectID(), uniqueIdx, p->GetInfluenceIdx() );
	}
}

void cPlayer::DelDeBuff( unsigned long uniqueIdx )
{
	cBaseObject::DelDeBuff( uniqueIdx );

	if( DUELMAN->IsDuelEnable() )
	{
		if( DUELMAN->IsDuelEnemy( this ) )
		{
			cDuelWindow* win = GAMEUI->GetDuelWindow();
			if( win )
				win->DelDeBuff( true, uniqueIdx );
		}
	}

	if( HERO->GetPartyIndex() == GetPartyIndex() )
	{
		PARTYMAN->DeleteDeBuff( GetObjectID(), uniqueIdx );
	}

	if( GetPartyUnionIndex() > 0 && HERO->GetPartyUnionIndex() == GetPartyUnionIndex() )
	{
		PARTYUNIONMAN->DeleteDeBuff( GetObjectID(), uniqueIdx );
	}
}

/// monster trans
void cPlayer::Update_TransMon( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpObjectSceneNode  == NULL ) return;

	DesiredDirProcess( deltaTime );

	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_IDLE:
		{
			IdleUpdate(deltaTime);
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			MoveUpdate(deltaTime);
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

}

void cPlayer::SetState_TransMon( unsigned char state , bool bShowAni )
{
	/// 죽은 경우 무조건 다음 동작으로 대기가 와야 한다.
	if( mPlayerExrInfo.mState == eOBJECT_STATE_DIE )
	{
		if( state != eOBJECT_STATE_IDLE )
			return;
	}

	if( mPlayerExrInfo.mState == ePLAYER_STATE_ITEMPICK &&
		state != ePLAYER_STATE_ITEMPICK )
		ShowWeapon( true );

	if( state != ePLAYER_STATE_SITDOWN )
	{
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

	if( mPlayerExrInfo.mState == ePLAYER_STATE_GATHERING && state != ePLAYER_STATE_GATHERING )
		ClearItemGatheringEffect();

	if( state != eOBJECT_STATE_STOP )
		mPlayerExrInfo.mStateStop = eSTOP_NONE;

	/// 기본 속도 보정
//	if( mpObjectSceneNode )
//		mpObjectSceneNode->UpdateAniScaleFactor( 1.0f );

	/// 기본 속도 보정
	if( state == eOBJECT_STATE_MOVE )
	{
		float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
		if( IsTransformMonster() == true )
		{
			sMonsterScript* script = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
			if( script )
				baseSpeed = (float)script->mActionRunSpd;
		}

		float moveSpeed = 1.0f;
		if( baseSpeed != 0.0f )
		{
			moveSpeed = (float)mPlayerExrInfo.mMoveSpeed / (float) baseSpeed;
		}

		if( mpObjectSceneNode )
			mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );
	}
	else
	{
		if( mpObjectSceneNode )
			mpObjectSceneNode->UpdateAniScaleFactor( 1.0f );
	}

	/// 그래픽적인 추가 처리가 필요한 경우...
	switch( state )
	{
	case eOBJECT_STATE_IDLE:
		{
			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
				ShowWeapon( true );
			else
				ShowWeapon( false );

			if( bShowAni && mJumping == false )
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
				ShowWeapon( true );
			else
				ShowWeapon( false );

			if( bShowAni && mJumping == false )
				ChangeAnimation_TransMon( M_ANITYPE_RUN );
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			SetReadyUseSkill( false );

			SetIdleFlag(eIDLE_NORMAL);

			/// 
			if( mpObjectSceneNode )
				mpObjectSceneNode->UpdateAniScaleFactor( mPlayerExrInfo.mAttackSpeed );//1.0f );
			//			SetState( eOBJECT_STATE_IDLE );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
//			if( mpObjectSceneNode )
//				mpObjectSceneNode->StopMorpher();

			ChangeAnimation_TransMon( M_ANITYPE_DIE1 );
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
			if( bShowAni )
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		break;
	case ePLAYER_STATE_ITEMPICK:
		{
			ShowWeapon( false );
			SetState( eOBJECT_STATE_IDLE );
		}
		break;
	case ePLAYER_STATE_GATHERING:
		{
			if( bShowAni )
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		break;
	case eOBJECT_STATE_STOP:
		{
			/// flag별 동작 변경
			switch( mPlayerExrInfo.mStateStop )
			{
			case eSTOP_READYTAROT:
			case eSTOP_OPENTAROT:
			case eSTOP_OPENSTALL:
				{
				}
				break;
			}
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			if( bShowAni )
				ChangeAnimation_TransMon( M_ANITYPE_DAMAGE1 );
		}
		break;
	}

	mPlayerExrInfo.mState = state;
}

void cPlayer::Interpret_TransMon( unsigned long time )
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

	InterpretEvent_TransMon( id, checkTime, pActor );

	if( endTime > 0 && endTime <= checkTime )
	{
		if( M_ANITYPE_IDLE2 == id )
		{
			ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( M_ANITYPE_ATTACK1 <= id && id <= M_ANITYPE_ATTACK3 )
		{
			if( IsReadyUseSkill() == true )
				SetState( eOBJECT_STATE_IDLE );
			else
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( M_ANITYPE_SKILL1 <= id && id <= M_ANITYPE_SKILL2 )
		{
			if( IsReadyUseSkill() == true )
				SetState( eOBJECT_STATE_IDLE );
			else
				ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
		}
		else if( id == M_ANITYPE_DAMAGE1 || id == M_ANITYPE_DAMAGE2 )
		{
			/// 공격받는 애니메이션이 끝나면 기본 애니메이션으로 바꾼다.
//			if( mDamageAfterAnim == M_ANITYPE_CASTING )
//				UpdateSkillAnimation( M_ANITYPE_CASTING );
//			else
				SetState( eOBJECT_STATE_IDLE );
		}
		else if( id == M_ANITYPE_JUMP )
		{
			SetJumpFlag( false );

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
	}
}

void cPlayer::ChangeAnimation_TransMon( unsigned int type )
{
	if( IsTransformMonster() == false )
		return;

	mpObjectSceneNode->SetTargetAnimation( type );

	/// 이벤트 타임 초기화
	if( type == M_ANITYPE_WORK || type == M_ANITYPE_RUN )
	{
		mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
		mRevisionEventTime[eANIEVENT_JUMP_START] = 0.0f;
		mRevisionEventTime[eANIEVENT_JUMP_END] = 0.0f;
	}
	else if( type == M_ANITYPE_JUMP )
	{
		mRevisionEventTime[eANIEVENT_RUN_LEFT] = 0.0f;
		mRevisionEventTime[eANIEVENT_RUN_RIGHT] = 0.0f;
		mRevisionEventTime[eANIEVENT_GATHER_MINING] = 0.0f;
	}
	else if( type == M_ANITYPE_DAMAGE1 || type == M_ANITYPE_DAMAGE2 )
	{
		UpdateAnimationSpeed( 1.0f );
	}
}

void cPlayer::ApplyDamageDrama_TransMon( int damage, unsigned int damageType, cBaseObject* pAttacker )
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

		if( mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE1 || 
			mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE2 )
		{
			if( pAttacker )
				SetDesiredDir( GetPos(), pAttacker->GetPos() ); //, 10 );
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
	return;
}

cEffectSceneNode* cPlayer::SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop, bool bFollow ) 
{
	if( ::strlen(nif) == 0 )
		return 0;

	unsigned int linkPos = num;
	if( IsTransformMonster() == true )
	{
		if( linkPos >= eLINK_RWEAPON )
			return 0;

		sMonsterScript* pInfo = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		if( pInfo && trans )
			trans->m_fScale *= pInfo->mEffectScale;
	}
	return mpObjectSceneNode->LinkEffect( linkPos, nif, trans, bLoop, bFollow ); 
}

cEffectSceneNode* cPlayer::SetLinkdDamage( unsigned int num, const char* nif, NiTransform* trans, bool IsMiss, bool zFalse )
{
	if( mpObjectSceneNode && mpObjectSceneNode->IsViewNode() == false )
		return 0;

	if( ::strlen(nif) == 0 )
		return 0;

	unsigned int linkPos = num;
	if( IsTransformMonster() == true )
	{
		if( linkPos >= eLINK_RWEAPON )
			return 0;
	}
	return mpObjectSceneNode->LinkDamage( linkPos, nif, trans, IsMiss, zFalse ); 
}

void cPlayer::SetTitle( unsigned long titleIdx )
{
	if( mPlayerExrInfo.mTitleIndex != titleIdx )
		mPlayerExrInfo.mTitleIndex = titleIdx;

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

void cPlayer::ClearGuild()
{
	mPlayerExrInfo.mGuildIndex		= 0;
	mPlayerExrInfo.mGuildPosition	= 0;
	mPlayerExrInfo.mUseGuildMark	= false;

	memset( &mPlayerExrInfo.mGuildMarkDate, 0, sizeof(mPlayerExrInfo.mGuildMarkDate) );
	memset( mPlayerExrInfo.mGuildName, 0, sizeof(mPlayerExrInfo.mGuildName) );

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

void cPlayer::SetGuild( unsigned long guildIndex, unsigned char guildposition, LPCTSTR guildName, tm markRegistDate, bool useMark )
{
	mPlayerExrInfo.mGuildIndex		= guildIndex;
	mPlayerExrInfo.mGuildPosition	= guildposition;
	mPlayerExrInfo.mGuildMarkDate	= markRegistDate;
	mPlayerExrInfo.mUseGuildMark	= useMark;

	Sstrncpy( mPlayerExrInfo.mGuildName, MAX_GUILDNAME_BUFFER_SIZE, guildName, MAX_GUILDNAME_SIZE );

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

void cPlayer::SetGuildPosition( unsigned char guildPosition )
{
	mPlayerExrInfo.mGuildPosition	= guildPosition;
}

void cPlayer::ChangeGuildMarkRegistDate( tm markRegistDate, bool useMark )
{
	mPlayerExrInfo.mGuildMarkDate = markRegistDate;
	mPlayerExrInfo.mUseGuildMark  = useMark;

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateGuildMark();
}

/// 파티세팅 OR 파티탈퇴 세팅
void cPlayer::SetPartyIndex( unsigned long partyIndex )
{
	if( mPlayerExrInfo.mPartyIndex != partyIndex )
	{
		mPlayerExrInfo.mPartyIndex = partyIndex;

		// 타겟창 세팅
		cTargetWindow* win = GAMEUI->GetTargetWindow();
		if( win )
		{
			win->UpdateParty();
		}
	}
}

//
void cPlayer::SetPartyUnionIndex( unsigned long unionIndex )
{
	if( mPlayerExrInfo.mPartyUnionIndex != unionIndex )
		mPlayerExrInfo.mPartyUnionIndex = unionIndex;
}

void cPlayer::SetTodayWord( LPCTSTR word, unsigned long color )
{
	mPlayerExrInfo.mTodayColor		= color;
	Sstrncpy( mPlayerExrInfo.mTodayWord, MAX_TODAYWORD_BUFFER_SIZE, word, MAX_TODAYWORD_SIZE );

	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

bool cPlayer::GatheringStart( unsigned long gaderIdx, unsigned long gatherClassIdx, NiPoint3 targetPos )
{
	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( gatherClassIdx );
	if( info == 0 )
		return false;


	/// 채집 이펙트 위치 설정.
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 0.0f );

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

	SetDesiredDir( GetPos(), targetPos );

	return true;
}

void cPlayer::GatheringEnd()
{
	SetState( eOBJECT_STATE_IDLE );
	ClearItemGatheringEffect();

	if( mGatheringToolObj != UINT_MAX )
	{
		mpObjectSceneNode->UnLinkObject( mGatheringToolObj );
		mGatheringToolObj = UINT_MAX;
	}
}

void cPlayer::RideVehicle( unsigned long vehicleIdx )
{
	if( IsJumping() )
	{
		SetJumpFlag( false );
	}

	if( IsTransformMonster() == true )
	{
		assert(0);
		return;
	}

	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return;
	}

	mpObjectSceneNode->ClearEffect();
	mpObjectSceneNode->ClearSound();

	for( unsigned int i = 0; i < eHAND_ALL; i++ )
	{
		mpObjectSceneNode->UnLinkObject( mWeaponObjectIndex[i] );
		mWeaponObjectIndex[i] = UINT_MAX;

		if( mWeaponObjectEffect[i] == UINT_MAX )
			continue;

		cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mWeaponObjectEffect[i] );
		if( eff )
			eff->Remove();
		mWeaponObjectEffect[i] = UINT_MAX;
	}


	/// buff/debuff effect set
	{
		cInfluenceMap::cIterator i = mBuffMap.Begin();
		cInfluenceMap::cIterator end = mBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}

		i = mDeBuffMap.Begin();
		end = mDeBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}
	}

	cVehicleSceneNodeParam param;
	param.mTranslate = GetPos();
	param.mRotate = GetModelRot();
	param.mScale = mpObjectSceneNode->GetScale();
	param.mVehicleIndex = vehicleIdx;
	param.mPlayerSceneNode = (cPlayerSceneNode*)mpObjectSceneNode;

	cVehicleSceneNode* node = SCENEMAN->CreateVehicle( param );
	if( node == 0 )
	{
		assert(0);
		return;
	}
	mpObjectSceneNode->ResetHead2();

	mpObjectSceneNode = node;
	mPlayerExrInfo.mVehicleIdx = vehicleIdx;

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, true );
	SetState( eOBJECT_STATE_IDLE );

	ShowWeapon( false );

	for( unsigned int i=0;i<eANIEVENT_MAX;i++ )
		mRevisionEventTime[i] = 0.0f;
}

void cPlayer::AlightVehicle()
{
	if( IsTransformMonster() == true )
	{
		assert(0);
		return;
	}

	if( IsRideVehicle() == false )
	{
		return;
	}

	if( mpObjectSceneNode == 0 || mpObjectSceneNode->GetType() != cSceneNode::eVEHICLE )
	{
		assert(0);
		return;
	}

	cPlayerSceneNode* playerNode = ((cVehicleSceneNode*)mpObjectSceneNode)->GetPlayerNode();
	if( playerNode == 0 )
	{
		assert(0);

		/// 외형 복구
		const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
		cPlayerSceneNodeParam param;
		param.mpObject = this;
		param.mPathName = strFile;

		param.mTranslate = mpObjectSceneNode->GetTranslate();
		param.mRotate = mpObjectSceneNode->GetRotate();
		param.mScale = (float)mPlayerExrInfo.mFixedObjectSizePer * 0.01f;

		///
		if( SCENEMAN->DestroyDynamic( mpObjectSceneNode ) == false )
		{
			assert(0);
		}

		/// 기존 객체 제거
		for( unsigned int i = 0; i < eHAND_ALL; i++ )
			mWeaponObjectIndex[i] = UINT_MAX;

		/// 신규 객체 생성
		cDynamicSceneNode* node = CreateSceneNode( &param );
		if( node == 0 )
		{
			assert(0);
			return;
		}
		mpObjectSceneNode = node;
		mpObjectSceneNode->SetTargetAnimation( 0 );

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
	}
	else
	{
		mpObjectSceneNode->ClearEffect();
		mpObjectSceneNode->ClearSound();

		///
		if( SCENEMAN->DestroyDynamic( mpObjectSceneNode ) == false )
		{
			assert(0);
		}

		mpObjectSceneNode = playerNode;

		EquipWeaponItem( &mPlayerWeaponInfo );
	}

	/// buff/debuff effect set
	{
		cInfluenceMap::cIterator i = mBuffMap.Begin();
		cInfluenceMap::cIterator end = mBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}

		i = mDeBuffMap.Begin();
		end = mDeBuffMap.End();
		for( ; i != end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
			p->Init_TargetUpdate();
		}
	}

	/// 게임 진입시 상황에 맞는 최종 동작을 유지하도록 보정.
	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_DIE:
		{
			ChangeAnimation( ANITYPE_DIE1 );

			mpObjectSceneNode->GotoLastFrame();
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
			ChangeAnimation( ANITYPE_SITDOWN_IDLE );
			mpObjectSceneNode->GotoLastFrame();
		}
		break;
	case eOBJECT_STATE_STOP:
		{
			/// flag별 동작 변경
			switch( mPlayerExrInfo.mStateStop )
			{
			case eSTOP_READYTAROT:
				{
					ActiveTarotBubble( (LPTSTR)GAMERESOURCEMAN->GetGameText( 300 ), COLOR_SKYBLUE );
					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_OPENTAROT:
				{
					ActiveTarotBubble( (LPTSTR)mTarotTitle.Cstr(), COLOR_SKYBLUE );
					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_OPENSTALL:
				{
					ActiveUserSellBubble( (LPTSTR)mSellTitle.Cstr(), COLOR_GREEN );

					ChangeAnimation( ANITYPE_SITDOWN_IDLE );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_ENHANCED:
				{
					ChangeAnimation( ANITYPE_ENHANCED );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_ITEMMIX:
				{
					ChangeAnimation( ANITYPE_ENHANCED );
					mpObjectSceneNode->GotoLastFrame();
				}
				break;
			case eSTOP_NONE:
				break;
			default:
				{
					if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
						ChangeAnimation( ANITYPE_IDLE );
					else
						ChangeAnimation( ANITYPE_PEACE_IDLE );
				}
				break;
			}
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			ChangeAnimation( ANITYPE_RUN );
		}
		break;
	case eOBJECT_STATE_IDLE:
		{
			/// emoting check
			if( mPlayerExrInfo.mCommunitySkillIdx != 0 )
			{
				sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( mPlayerExrInfo.mCommunitySkillIdx );
				if( pInfo )
				{
					if( pInfo->mType == 1 )
						ChangeAnimation( (ePLAYER_ANIMATIONTYPE)pInfo->mRoleIndex );
				}
			}

			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
				ChangeAnimation( ANITYPE_IDLE );
			else
				ChangeAnimation( ANITYPE_PEACE_IDLE );
		}
		break;
	default:
		{
			if( mPlayerExrInfo.mStateIdle == eIDLE_NORMAL )
				ChangeAnimation( ANITYPE_IDLE );
			else
				ChangeAnimation( ANITYPE_PEACE_IDLE );
		}
		break;
	}

	//PVPMAN->LinkPvpForceEffect( this, mPlayerExrInfo.mForceType );

	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, false );

//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetScale();

	mIdleTime = 0;

	mpObjectSceneNode->UpdateNameCardTextValue();
	mpObjectSceneNode->UpdateNameCardGaugeValue();

	mpObjectSceneNode->ResetHead2();

	if( PARTYMAN->IsHeroParty( GetObjectID() ) == true ||
		PARTYUNIONMAN->IsHeroParty( GetObjectID() ) == true )
		PartyIn_Init();
	else
		PartyOut_Init();

	mPlayerExrInfo.mVehicleIdx = 0;

	ShowWeapon( true );


	for( unsigned int i=0;i<eANIEVENT_MAX;i++ )
		mRevisionEventTime[i] = 0.0f;
}

bool cPlayer::IsRideVehicle()
{
	if( mPlayerExrInfo.mVehicleIdx == 0 )
		return false;

//	if( mpObjectSceneNode && mpObjectSceneNode->GetType() == cSceneNode::eVEHICLE )

	return true;
}

void cPlayer::UpdateHelmetState( bool show )
{
	if( IsTransformMonster() == true )
		return;

	unsigned long itemIdx = mPlayerWearInfo.WearIdx[eWEAR_HAT];
	if( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] != 0 )
		itemIdx = mPlayerWearInfo.CostumeIdx[eWEAR_HAT];

	if( show == true )
	{
		cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIdx );
		if( itemDefine )
		{
			if( itemDefine->GetSubType() != ITEM_WEAR_HEAD && itemDefine->GetSubType() != ITEM_WEAR_COS_HEAD)
			{
				assert(0);
				return;
			}

			mpObjectSceneNode->ChangeParts( ePART_HAIR, itemDefine->GetFileName0( mPlayerInfo.Race, mPlayerInfo.Gender ) );
			mpObjectSceneNode->HideEar( itemDefine->GetHideEar() == 1 );
		}
	}
	else
	{
		ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
		mpObjectSceneNode->HideEar( false );
	}
}

void cPlayer::RemoveObjectSceneNode()
{
	if( mpObjectSceneNode == 0 )
	{
		return;
	}

	if( mpObjectSceneNode->GetType() != cSceneNode::eVEHICLE )
	{
		/// 등록된 그래픽 객체 삭제
		SCENEMAN->DestroyDynamic( mpObjectSceneNode );
		mpObjectSceneNode = NULL;
	}
	else
	{
		cPlayerSceneNode* playerNode = ((cVehicleSceneNode*)mpObjectSceneNode)->GetPlayerNode();

		SCENEMAN->DestroyDynamic( mpObjectSceneNode );
		mpObjectSceneNode = NULL;

		if( playerNode )
		{
			SCENEMAN->DestroyDynamic( playerNode );
		}
	}
}

bool cPlayer::Jump()
{
	//if( IsTransformMonster() == true )
	//	return false ;

	switch( GetState() )
	{
	case eOBJECT_STATE_IDLE:
	case eOBJECT_STATE_MOVE:
		break;
	case ePLAYER_STATE_SITDOWN:
		SetState( eOBJECT_STATE_IDLE, false );
		break;
	default:
		return false;
	}

	if( IsTransformMonster() == true )
		ChangeAnimation_TransMon( M_ANITYPE_JUMP );
	else
		ChangeAnimation( ANITYPE_JUMP );

	SetJumpFlag( true );

	mpObjectSceneNode->UpdateAniScaleFactor( 1.0f );

	return true;
}

void cPlayer::UpdateNameCardTextValue()
{
	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

void cPlayer::SetJumpFlag( bool flag )
{
	mJumping = flag;

	if( mJumping == true )
	{
		mJumpProcess = true;
		mJumpRemainTime = (unsigned long)(mpObjectSceneNode->GetTargetAnimationScaleEndTime()*1000) + 2000;
	}
	else
	{
		mJumpProcess = false;
		mJumpRemainTime = 0;

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

void cPlayer::SetMoveSpeed( long moveSpeed )
{
	mPlayerExrInfo.mMoveSpeed = moveSpeed;

	if( GetState() == eOBJECT_STATE_MOVE )
	{
		float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
		if( IsTransformMonster() == true )
		{
			sMonsterScript* script = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
			if( script )
				baseSpeed = (float)script->mActionRunSpd;
		}

		float moveSpeed = 1.0f;
		if( baseSpeed != 0.0f )
			moveSpeed = (float)mPlayerExrInfo.mMoveSpeed / (float) baseSpeed;

		if( mpObjectSceneNode && IsJumping() == false )
			mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );
	}
}

void cPlayer::ReadyTransform( unsigned char readyState, unsigned long chgMonIdx )
{
	mReadyTransformInfo.readState = readyState;
	mReadyTransformInfo.chgMonIdx = chgMonIdx;
}

void cPlayer::SetSkillPushPull( float posX, float posY )
{
	mGotoX = posX;
	mGotoY = posY;

	NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

	SetState( eOBJECT_STATE_PUSHPULL );
}

void cPlayer::SetSkillRush( float posX, float posY, sObject target )
{
	mGotoX = posX;
	mGotoY = posY;

	mActionMoveRange = 100;

	mTargetPos.x = posX;
	mTargetPos.y = posY;
	mTargetPos.z = 0.0f;

	/// 높이값 계산
	if( WORLDMAN )
	{
		if( WORLDMAN->CalcHeight( &mTargetPos.z, mTargetPos.x, mTargetPos.y ) == false )
			assert(NULL);
	}

	SetDesiredDir( GetPos(), mTargetPos );

	SetTargetObject( target.type, target.index );

	float baseSpeed = STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_SPD, GetJob() );
	float moveSpeed = 1.0f;
	if( baseSpeed != 0.0f )
	{
		moveSpeed = SKILL_RUSH_SPEED / (float) baseSpeed;
	}

	if( mpObjectSceneNode && IsJumping() == false )
		mpObjectSceneNode->UpdateAniScaleFactor( moveSpeed );

	SetState( ePLAYER_STATE_RUSH );
}
