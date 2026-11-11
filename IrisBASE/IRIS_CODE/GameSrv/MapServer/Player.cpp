#include "gamesrv.h"

#include "stdafx.h"

#include "Player.h"
#include "BaseObject_Common.h"
#include "ObjectManager.h"
#include "Protocol.h"
#include "Skill_Common.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "Status_Common.h"
#include "DamageCalc.h"
#include "StatusCalc_Server.h"
#include "StatusScript.h"
#include "RangeCheck.h"
#include "LevelScript.h"
#include "PartyManager.h"
#include "Drop.h"
#include "AIManager.h"
#include "PVPManager.h"
#include "DeathMatchObject.h"
#include "DuelManager.h"
#include "TutorialScript.h"
#include "NpcScript.h"

#include "ItemManager.h"
#include "Tarot_Common.h"
#include "Tutorial_Common.h"
#include "Monster.h"
#include "Npc.h"
#include "Item.h"
#include "Communityscript.h"
#include "StageScript.h"
#include "MonsterScript.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include "PVP_Common.h"
#include "StatusCalc_Server.h"
#include "Gathering.h"
#include "VehicleScript.h"
#include "Vehicle_Common.h"
#include "VehicleScript.h"
#include "MakeSkill_Common.h"
#include "MakeSkillScript.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GridManager.h"
#include "RandomTable.h"
#include "AppTimer.h"
#include "PartyUnion.h"
#include "PartyUnionManager.h"
#include "Theme_Common.h"
#include "ThemeManager.h"
#include "UserPortal.h"

/// 치트
#include "Protocol_Cheat.h"
#include "Cheat_Common.h"


// 지연시간
#define DELAY_ITEM_AGENT_CHECK	600000	// 판매대행    지연시간 - 10분(MS)
#define DELAY_POST_CHECK		600000	// 우편검사    지연시간 - 10분(MS)
#define DELAY_ITEM_BILL			600		// 아이템 빌링 지연시간 - 10분(SS)


int StallSellItemCompare( const void *arg1, const void *arg2 )
{
	StallSellItem* item1 = (StallSellItem*)arg1;
	StallSellItem* item2 = (StallSellItem*)arg2;

	// 내림차순 정렬
	return (item2->status != StallSellItemNone) - (item1->status != StallSellItemNone);
}

bool InfLeftTimeSort( unsigned long arg1, unsigned long arg2 ) 
{ 
	cInfluenceObject* pLeft = SKILLMANAGER->GetInfluence( arg1 );
	cInfluenceObject* pRight = SKILLMANAGER->GetInfluence( arg2 );

	if( pLeft == NULL )
		return true;

	if( pRight == NULL )
		return false;

	return pLeft->GetChangeMPValue() > pRight->GetChangeMPValue();
} 

bool InfChangeMPMinSort( unsigned long arg1, unsigned long arg2 ) 
{ 
	cInfluenceObject* pLeft = SKILLMANAGER->GetInfluence( arg1 );
	cInfluenceObject* pRight = SKILLMANAGER->GetInfluence( arg2 );

	if( pLeft == NULL )
		return true;

	if( pRight == NULL )
		return false;

	return pLeft->GetInfluenceEndTime() < pRight->GetInfluenceEndTime();
} 

cPlayer::cPlayer() : cBaseObject( eOBJECTTYPE_PLAYER )
{
	//mpSkill = NULL;
	mPlayerWeaponState = eWEAPON_STATE_NONE;
	mNpcIdx = 0;

	mRange = 0.0f;
	mRangeTargetPos = NiPoint2::ZERO;
	mRangeTarget.type = eOBJECTTYPE_NONE;
	mRangeTarget.index = 0;

	mPartyState = ePARTY_NONE;
	mIsGuildAddReq = false;
	mMonsterImportance = 0;

	memset( mStatus2Plus, 0 , sizeof(mStatus2Plus) );
	memset( mStatus2Per, 0 , sizeof(mStatus2Per) );

	mNextRecoveryTime = 0;
	mGameIn = false;

	memset( mFollowPos, 0, sizeof(mFollowPos) );

	mStatus2.mMoveSpeed = 500;
	mStatus2.mAttackSpeed = 1.0f;
	mPlayerExrInfo.mFixedObjectSizePer = 0;

	mIsRequestRejection = eREQREJCT_NONE;

	mThemePartyDb = false;

	mDuelPlayerIdx = 0;
	mIsDuelRequester = false;
	mDuelStraightWin = 0;

	mPlayerExrInfo.mBaseFixedSize = FIXED_OBJECT_SIZE;
	CalcFixedObjectSize( 0 );

	mTitleUniqIndex = 0;

	memset( &mOptionData1, 0, sizeof(mOptionData1) );

	/// 치트
	mCheatHideMode = false;
	mCheatUndeadMode = false;
	mCheatChatDuration = 0;
	mCheatChatAccumTime = 0;
	mCheatStopDuration = 0;
	mCheatSpeedUp = eCHEATSPEEDUP_NONE;

	mJobUsedSpArr.Resize(5);

	for( unsigned int i=0;i<mJobUsedSpArr.GetSize();i++ )
		mJobUsedSpArr[i] = 0;

	mGatheringIdx = 0;
	mVehicleInfIdx = 0;

	mItemMixEndTime = 0;
	mMixRecipeIdx = 0;
	mMixMakeSkill = 0;

	mSkillReqCnt = 0;
	mSkillSuccessCnt = 0;
	mSkillCoolTimeFail = 0;
	mMonSkillReqCnt = 0;
	mMonSkillSuccessCnt = 0;
	mMonSkillCoolTimeFail = 0;
	mGameInTime = NETWORK2->GetAccumTime();

	mIsDbUpdate = false;

	mServerPosTime = 0;

	mReadyGotoPos = NiPoint2::ZERO;

	mJumpEndTime = 0;
	mJumpTime = 0;

	mThemeRoomIdx = 0;
	mThemeUserRoot.pool = NULL;
	mThemeUserRoot.root = NULL;
	mThemeMode = 0;

	mItemEnhanceEndTime = 0;
	mEnhanceSlot1       = USHRT_MAX;
	mEnhanceSlot2       = USHRT_MAX;
    mEnhanceSlot3       = USHRT_MAX;

	memset( mTempNpcTarotCards, 0, sizeof(mTempNpcTarotCards) );

	mTutorialModeIndex = -1;

	mPvPDieEndTime = 0;
	mPvPDieTime = 0;

	mFriendUseDB = false;
	mFriendSend  = false;

    mAutoPlayer         = false;
    mAutoTrial          = false;
    mTrialAnswerCount   = 0;
    mTrialMemberIdx     = 0;
    mTrialAnswerTime    = 0;
    mLastAttackTime     = 0;
    mCaptchaCount       = 0;

	mPcBangNextCheckTime = ULONG_MAX;
	mGMEventNextCheckTime = ULONG_MAX;
	mGMEventDelNextCheckTime = ULONG_MAX;

	mExpRecoverEndTime = TIMER->GetRealTime();

	mInfNoSave = false;
	mCashInfNextSaveTime = 0;
	mIsCashInfSave = false;
	mIsCashInfCycleSave = false;

	mMapNum0Save = false;

	mJumpMoveSpeedDownEndTime = 0;
	mJumpSpeedState = 0;

	mIsMoveMsg = false;
	memset( &mMoveMsg, 0, sizeof(mMoveMsg) );

    memset( mCaptchaName, 0x00, sizeof(mCaptchaName) );

	mChannelPvPGoCid = 0;
	mChannelPvPMoveTime = 0;

	mPvPJoinAryPos = ULONG_MAX;

	mOriMaxHP = 0;
	mOriMaxMP = 0;

	mWaitSaveExp = 0;
}

cPlayer::~cPlayer()
{
	if( mSkillCoolTimeFail > 10 || mMonSkillCoolTimeFail > 10 )
	{
		NETWORK2->PostServerEvent("skill log player[%d][%d][%d,%d,%d][%d,%d,%d]",
			mObject.index, NETWORK2->GetAccumTime()-mGameInTime, mSkillReqCnt, mSkillSuccessCnt, mSkillCoolTimeFail,
			mMonSkillReqCnt, mMonSkillSuccessCnt, mMonSkillCoolTimeFail );
	}

	while ( mItemBillRoot.pool != NULL )
	{
		ITEMBILLPOOL->ReleaseItemBill( &mItemBillRoot, (PerItemBill*)mItemBillRoot.pool );
	}

	while ( mItemGetRoot.pool != NULL )
	{
		ITEMGETPOOL->ReleaseItemGet( &mItemGetRoot, (PerItemGet*)mItemGetRoot.pool );
	}

	while ( mItemCountRoot.pool != NULL )
	{
		ITEMCOUNTPOOL->ReleaseItemCount( &mItemCountRoot, (PerItemCount*)mItemCountRoot.pool );
	}
	mObject.index = 0;
}

void* cPlayer::operator new( size_t n )
{
	if( n != sizeof(cPlayer) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocPlayer();
}

void cPlayer::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cPlayer) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreePlayer( static_cast<cPlayer*>(ptr) );

	return;
}

// Init Method
bool cPlayer::Init(unsigned long connectionIdx, unsigned long playerIdx, unsigned long money, unsigned long deposit)
{
	// sObject & sPlayerInfo 초기화.
	cBaseObject::SetObjectID(playerIdx);
	mPlayerInfo.CharacterIdx = playerIdx;

	// ConnectionIdx 초기화.
	mConnectionIdx = connectionIdx;

	/// 기본정보셋팅 - 초기화
	mPlayerExrInfo.RestHP = 0;
	mPlayerExrInfo.MaxHP =  0;
	mPlayerExrInfo.MaxMP =  0;
	mPlayerInfo.Level = 0;
	mPlayerInfo.Job = ePLAYER_NONEJOB;

	mIsSkillResurrection = false;
	mSkillResurrectionHP = 0;
	mSkillResurrectionMP = 0;

	mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;
	mPlayerExrInfo.mState = eOBJECT_STATE_IDLE;
	mPlayerExrInfo.mStateStop = eSTOP_NONE;

	mMapChange = false;

	mMapTargetPos.mMapNumber = 0;
	mMapTargetPos.mPosX = 0.0f;
	mMapTargetPos.mPosY = 0.0f;
	mMapTargetPos.mRotAngle = 0.0f;

	mThemePartyDb = false;

	/// 소지금액 & 예치금액 설정.
	mMoney   = money;
	mDeposit = deposit;

	memset( mODDITY, 0, sizeof(mODDITY) );

	/// 아이템 관련 변수 초기화.
	mInventoryDatabase = 0;
	memset( mInventory, 0, sizeof(mInventory) );

	mItemActiveWeapon = ItemActiveFront;

	mItemChange = false;
	mItemWeight = 0;

	mXCopyComplete = false;

	memset( &mEquipAbility, 0, sizeof(mEquipAbility) );
	mArmorSets  = ARMOR_SETS_NONE;

	mItemEnhancedSeed = RANDOMTABLE->GetSeed( );
	mItemChangeSeed   = RANDOMTABLE->GetSeed( );

	mItemBillRoot.pool = NULL;
	mItemBillRoot.root = NULL;
	mItemBillStart = false;

	mItemGetRoot.pool = NULL;
	mItemGetRoot.root = NULL;

	mItemCountRoot.pool = NULL;
	mItemCountRoot.root = NULL;

	mCooltimeRoot.pool = NULL;
	mCooltimeRoot.root = NULL;

	/// 아이템줍기 관련 변수 초기화.
	mItemGetOpen = false;
	mItemGetIdx  = 0;

	mItemGetCount = 0;
	memset( mItemGetData, 0, sizeof(mItemGetData) );

	/// 거래 관련 변수 초기화.
	mExchangeStatus = ItemExchangeNone;
	mExchangeItems  = 0;
	mExchangeMoney  = 0;

	/// 노점 관련 변수 초기화.
	mStallSell      = false;

	memset( mStallSellTitle, 0, sizeof(mStallSellTitle) );
	memset( mStallSellItems, 0, sizeof(mStallSellItems) );
	mStallSellItemOffset = 0;

	memset( mStallSellGuests, 0, sizeof(mStallSellGuests) );
	mStallSellGuestOffset = 0;

	memset( &mStallSellOwner, 0, sizeof(mStallSellOwner) );

	/// 판매대행 관련 변수 초기화.
	SetItemAgentCheck( DELAY_ITEM_AGENT_CHECK );

	/// 우편 관련 변수 초기화.
	SetPostCheck( DELAY_POST_CHECK );

	/// 타로카드 관련 변수 초기화.
	memset( mTarotTitle, 0, sizeof(mTarotTitle) );
	mTarotPrice         = 0;
	mTarotUserCount     = 0;
	mTarotItemCount     = 0;
	mTarotCloseReserved = false;

	memset( mTarotCards, 0, sizeof(mTarotCards) );
	memset( mSpread,     0, sizeof(mSpread)     );
	mSpreadOffset = 0;

	mTarotReaderIdx = 0;

	memset( mTarotGuests, 0, sizeof(mTarotGuests) );
	mTarotGuestOffset = 0;

	mTarotResultIndex = 0;
	mTarotResultValue = 0;

	mDuelIdx = 0;

	mPvPDMIdx = 0;
	mIsInDunJoin = false;
	mInDunMapDataNum = 0;

	/// 퀘스트 관련 변수 초기화
	memset( mQuest, 0, sizeof(TB_QUEST_PROGRESS) * MAX_KEEPQUEST );

	///
	TIMESTAMP_STRUCT timestamp;
	timestamp.year		= 1900;
	timestamp.month		= 1;
	timestamp.day		= 1;
	timestamp.hour		= 0;
	timestamp.minute	= 0;
	timestamp.second	= 0;
	timestamp.fraction  = 0;

	for( int i = 0; i < 5; ++i )
	{
		mFortuneThru[i] = timestamp;
	}

	/// 커뮤니티 관련 변수 초기화.
	ClearFriends( );

	mMapNum0Save = false;

	mIsDbUpdate = false;

	// 블록 관련 변수 초기화.
	mBlockApply     = false;
	mBlockValidThru = 0;

	mClientMoveDefTime = 0;
	mClientMovePlusMinus = 0;
	mServerMoveTime = 0;

	mSkillLastAttack = mObject;
	mSkillLastKind = eDAMAGE_MISS;

	mKeyboardMove = false;

	mThemeMode = 0;

	mItemEnhanceEndTime = 0;
	mEnhanceSlot1       = USHRT_MAX;
	mEnhanceSlot2       = USHRT_MAX;
    mEnhanceSlot3       = USHRT_MAX;

	mTutorialModeIndex = -1;

	mPvPDieEndTime = 0;
	mPvPDieTime = 0;

	mFriendUseDB = false;
	mFriendSend  = false;

	mExpRecoverEndTime = TIMER->GetRealTime();

	mInfNoSave = false;
	mCashInfNextSaveTime = NETWORK2->GetAccumTime() + INFLUENCE_CASH_NEXT_SAVETIME;
	mIsCashInfSave = false;
	mIsCashInfCycleSave = false;

	mIsMoveMsg = false;
	memset( &mMoveMsg, 0, sizeof(mMoveMsg) );

	mJumpMoveSpeedDownEndTime = 0;
	mJumpSpeedState = 0;

	mChannelPvPGoCid = 0;
	mChannelPvPMoveTime = 0;

	mPvPJoinAryPos = ULONG_MAX;

	mOriMaxHP = 0;
	mOriMaxMP = 0;

	// 초기화 넣어줌. [3/31/2010 Jo_Client]
	mGuardAry.Clear();
	mChangeMPDamageAry.Clear();
	mDeleteTypeDamagedAry.Clear();

	return true;
}

void cPlayer::Update( unsigned long elapsedTime, unsigned long accumTime )
{
	if( mChannelPvPMoveTime != 0 && mChannelPvPMoveTime < accumTime )
	{
		NETWORK2->GotoInstantDungeon( mConnectionIdx, mChannelPvPGoCid );
		return;
	}

	if( mIsMoveMsg == true )
	{
		mIsMoveMsg = false;

		switch( mMoveMsg.mProtocol )
		{
		case NM_PLAYER_KEYMOVE_REQ:
			{
				if( mMoveMsg.mMsgError == false )
				{
					if( GetCheatHideMode() == false )
					{
						// 다른 플레이어들에게도 알림
						MSG_SYN_MOVE Msg;
						Msg.Category     = NM_PLAYER;
						Msg.Protocol     = NM_PLAYER_MOVE_SYN;
						Msg.characterIdx = mObject.index;
						Msg.destX        = mMoveMsg.mDestPos.x;
						Msg.destY        = mMoveMsg.mDestPos.y;
						Msg.moveSpeed    = GetMoveSpeed();
						NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
					}
				}
				else
				{
					// 070625 PKH 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE            handle = NULL;
					MSG_RES_KEYMOVE* msg    = (MSG_RES_KEYMOVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_KEYMOVE_RES );
					msg->mCharacterPosX = mMoveMsg.mDestPos.x;
					msg->mCharacterPosY = mMoveMsg.mDestPos.y;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_KEYMOVE) );

					if( GetState() == eOBJECT_STATE_MOVE )
					{
						if( GetCheatHideMode() == false )
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_MOVE_STOP Msg;
							Msg.Category = NM_PLAYER;
							Msg.Protocol = NM_PLAYER_MOVE_STOP_SYN;
							Msg.mCharacterIdx = mObject.index;
							Msg.mCharacterPosX = GetXPos();
							Msg.mCharacterPosY = GetYPos();
							NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
						}
					}
				}
			}
			break;
		case NM_PLAYER_MOVE_REQ:
			{
				if( mMoveMsg.mMsgError == false )
				{
					if( GetCheatHideMode() == false )
					{
						// 다른 플레이어들에게도 알림
						MSG_SYN_MOVE Msg;
						Msg.Category     = NM_PLAYER;
						Msg.Protocol     = NM_PLAYER_MOVE_SYN;
						Msg.characterIdx = mObject.index;
						Msg.destX        = mMoveMsg.mDestPos.x;
						Msg.destY        = mMoveMsg.mDestPos.y;
						Msg.moveSpeed    = GetMoveSpeed();
						NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
					}
				}
				else
				{
					// 070625 PKH 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE            handle = NULL;
					MSG_RES_ERR_MOVE* msg    = (MSG_RES_ERR_MOVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_MOVE_RES );
					msg->destX = mMoveMsg.mDestPos.x;
					msg->destY = mMoveMsg.mDestPos.y;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ERR_MOVE) );

					if( GetState() == eOBJECT_STATE_MOVE )
					{
						if( GetCheatHideMode() == false )
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_MOVE_STOP Msg;
							Msg.Category = NM_PLAYER;
							Msg.Protocol = NM_PLAYER_MOVE_STOP_SYN;
							Msg.mCharacterIdx = mObject.index;
							Msg.mCharacterPosX = GetXPos();
							Msg.mCharacterPosY = GetYPos();
							NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
						}
					}
				}
			}
			break;
		case NM_PLAYER_ACTIONMOVE_REQ:
			{
				if( mMoveMsg.mMsgError == false )
				{
					if( GetCheatHideMode() == false )
					{
						// 다른 플레이어들에게도 알림
						MSG_SYN_ACTIONMOVE Msg;
						Msg.Category = NM_PLAYER;
						Msg.Protocol = NM_PLAYER_ACTIONMOVE_SYN;
						Msg.characterIdx = mObject.index;
						Msg.destX = mMoveMsg.mDestPos.x;
						Msg.destY = mMoveMsg.mDestPos.y;
						Msg.targetX = mMoveMsg.mTargetPos.x;
						Msg.targetY = mMoveMsg.mTargetPos.y;
						Msg.range = mMoveMsg.mRange;
						Msg.targetInfo = mMoveMsg.mTargetInfo;
						Msg.moveSpeed = GetMoveSpeed();
						NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
					}
				}
				else
				{
					HANDLE            handle  = NULL;
					MSG_RES_ERR_MOVE* sendMsg = (MSG_RES_ERR_MOVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_ACTIONMOVE_RES );
					sendMsg->destX = mMoveMsg.mDestPos.x;
					sendMsg->destY = mMoveMsg.mDestPos.y;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ERR_MOVE) );

					if( GetState() == eOBJECT_STATE_MOVE )
					{
						if( GetCheatHideMode() == false )
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_MOVE_STOP Msg;
							Msg.Category = NM_PLAYER;
							Msg.Protocol = NM_PLAYER_MOVE_STOP_SYN;
							Msg.mCharacterIdx = mObject.index;
							Msg.mCharacterPosX = GetXPos();
							Msg.mCharacterPosY = GetYPos();
							NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
						}
					}

				}
			}
			break;
		case NM_PLAYER_ACTIONMOVE_STOP_REQ:
			{
				if( mMoveMsg.mMsgError == true )
				{
					// 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE handle = NULL;
					MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_ACTIONMOVE_STOP_RES );
					if ( msg != NULL )
					{
						msg->mCharacterPosX = mObjectPos.x;
						msg->mCharacterPosY = mObjectPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
					}
				}

				if( GetCheatHideMode() == false )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_MOVE_STOP Msg;
					Msg.Category = NM_PLAYER;
					Msg.Protocol = NM_PLAYER_ACTIONMOVE_STOP_SYN;
					Msg.mCharacterIdx = mObject.index;
					Msg.mCharacterPosX = GetXPos();
					Msg.mCharacterPosY = GetYPos();
					NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
				}
			}
			break;
		case NM_PLAYER_KEYMOVE_STOP_REQ:
			{
				if( mMoveMsg.mMsgError == true )
				{
					// 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE handle = NULL;
					MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_KEYMOVE_STOP_RES );
					if ( msg != NULL )
					{
						msg->mCharacterPosX = mObjectPos.x;
						msg->mCharacterPosY = mObjectPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
					}
				}

				if( GetCheatHideMode() == false )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_MOVE_STOP Msg;
					Msg.Category = NM_PLAYER;
					Msg.Protocol = NM_PLAYER_KEYMOVE_STOP_SYN;
					Msg.mCharacterIdx = mObject.index;
					Msg.mCharacterPosX = GetXPos();
					Msg.mCharacterPosY = GetYPos();
					NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
				}
			}
			break;
		case NM_PLAYER_FOLLOWMOVE_STOP_REQ:
			{
				if( mMoveMsg.mMsgError == true )
				{
					// 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE handle = NULL;
					MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FOLLOWMOVE_STOP_RES );
					if ( msg != NULL )
					{
						msg->mCharacterPosX = mObjectPos.x;
						msg->mCharacterPosY = mObjectPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
					}
				}

				if( GetCheatHideMode() == false )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_MOVE_STOP Msg;
					Msg.Category = NM_PLAYER;
					Msg.Protocol = NM_PLAYER_FOLLOWMOVE_STOP_SYN;
					Msg.mCharacterIdx = mObject.index;
					Msg.mCharacterPosX = GetXPos();
					Msg.mCharacterPosY = GetYPos();
					NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
				}
			}
			break;
		case NM_PLAYER_FINDPOSITIONMOVE_STOP_REQ:
			{
				if( mMoveMsg.mMsgError == true )
				{
					// 서버상의 좌표로 클라이언트를 옮기게 함
					HANDLE handle = NULL;
					MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FINDPOSITIONMOVE_STOP_RES );
					if ( msg != NULL )
					{
						msg->mCharacterPosX = mObjectPos.x;
						msg->mCharacterPosY = mObjectPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
					}
				}

				if( GetCheatHideMode() == false )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_MOVE_STOP Msg;
					Msg.Category = NM_PLAYER;
					Msg.Protocol = NM_PLAYER_FINDPOSITIONMOVE_STOP_SYN;
					Msg.mCharacterIdx = mObject.index;
					Msg.mCharacterPosX = GetXPos();
					Msg.mCharacterPosY = GetYPos();
					NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
				}
			}
			break;
		}
	}

	/// 플레이어 이동 처리
	switch( GetState() )
	{
	case eOBJECT_STATE_MOVE:
		{
			/// 이동불가 상태 이상인 경우
			if( IsCantMove() == true )
				return;

			float moveElapsedTime = (float)(elapsedTime) / SECOND_THOUSAND;
			unsigned long applyElapsedTime = elapsedTime;

			if( mKeyboardMove == true )
			{
				long gap = 1;
				if( mClientMoveDefTime > 0 )
				{
					if( mClientMoveDefTime < gap )
						gap = mClientMoveDefTime;
					/// 최소단위를 더해서 증가
					float changeGap = (float)gap * mClientMovePlusMinus / SECOND_THOUSAND;				
					moveElapsedTime = moveElapsedTime + changeGap;
					mClientMoveDefTime -= gap;
				}
				else if( mClientMoveDefTime < 0 )
				{
					gap *= -1;
					if( mClientMoveDefTime > gap )
						gap = mClientMoveDefTime;
					/// 최소단위를 더해서 증가
					float changeGap = (float)gap * mClientMovePlusMinus / SECOND_THOUSAND;				
					moveElapsedTime = moveElapsedTime - changeGap;
					mClientMoveDefTime -= gap;
				}
			}

			/// 플레이어의 이동속도
			NiPoint2 pos = mObjectPos;
			float speed = (float)GetMoveUpdateSpeed( accumTime );

			/// 방향 설정
			NiPoint2 dir1;
			dir1.x = mGotoX - GetXPos();
			dir1.y = mGotoY - GetYPos();
			dir1.Unitize();

			/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
			pos.x += (speed * moveElapsedTime * dir1.x);
			pos.y += (speed * moveElapsedTime * dir1.y);

			/// 목적지 좌표와 예상 이동 위치를 비교
			NiPoint2 dir2;
			dir2.x = mGotoX - pos.x;
			dir2.y = mGotoY - pos.y;
			dir2.Unitize();

			/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달로 판정
			if( dir2.Dot(dir1) <= 0.0f )
			{
				/// 플레이어가 목적지까지 도달 했다고 판정이 난경우
				pos.x = mGotoX;
				pos.y = mGotoY;

				if( mKeyboardMove == false )
					ClearMoveSyncCalc();

				NiPoint2 target( mGotoX, mGotoY );
				float applyRange = (target - pos).Length();
				if( applyRange > 0.0f && speed > 0 )
				{
					applyElapsedTime = (unsigned long)( applyRange * SECOND_THOUSAND / speed );
					if( elapsedTime < applyElapsedTime )
						applyElapsedTime = 0;
				}

				/// 대기 상태로 바꿈
				if( ChangeState( eOBJECT_STATE_IDLE ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
				}
			}
			else
			{
				if( FloatToInt(mRange) != 0 )
				{
					/// 타겟 좌표 설정
					NiPoint3 targetPos( mRangeTargetPos.x, mRangeTargetPos.y, 0.0f );
					NiPoint3 playerGuessPos( pos.x, pos.y, 0.0f );
					AIMANAGER->CalcHeight( mMapNumber, &playerGuessPos.z, playerGuessPos.x, playerGuessPos.y );
					NiPoint3 playerNowPos( GetXPos(), GetYPos(), Height() );

					if( mRangeTarget.type == eOBJECTTYPE_NONE )
					{
						AIMANAGER->CalcHeight( mMapNumber, &targetPos.z, targetPos.x, targetPos.y );

						float rangeFix = OBJECTMANAGER->ObjectSizeRange( this, 0, mRange );

						cRangeCheck range( rangeFix );
						if( range.IsRange( playerGuessPos, targetPos ) )
						{
							NiPoint3 p;

							NiPoint3 d = playerNowPos - targetPos;
							d.Unitize();

							p.x = targetPos.x + (rangeFix-1.0f)*d.x;
							p.y = targetPos.y + (rangeFix-1.0f)*d.y;

							pos.x = p.x;
							pos.y = p.y;

							if( mKeyboardMove == false )
								ClearMoveSyncCalc();

							SetMoveTargetPos( p.x, p.y );
							mRange = 0.0f;

							/// 대기 상태로 바꿈
							if( ChangeState( eOBJECT_STATE_IDLE ) == false )
							{
								assert(NULL);
								NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
							}
						}
					}
					else
					{
						cBaseObject* pTarget = OBJECTMANAGER->GetObject( mRangeTarget.type, mRangeTarget.index );
						if( pTarget != NULL )
						{
							targetPos.x = pTarget->GetXPos();
							targetPos.y = pTarget->GetYPos();
							targetPos.z = pTarget->Height();

							float rangeFix = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mRange );

							cRangeCheck range( rangeFix );
							if( range.IsRange( playerGuessPos, targetPos ) )
							{
								NiPoint3 p;

								NiPoint3 d = playerNowPos - targetPos;
								d.Unitize();

								p.x = targetPos.x + (rangeFix-1.0f)*d.x;
								p.y = targetPos.y + (rangeFix-1.0f)*d.y;

								pos.x = p.x;
								pos.y = p.y;

								if( mKeyboardMove == false )
									ClearMoveSyncCalc();

								SetMoveTargetPos( p.x, p.y );
								mRange = 0.0f;

								/// 대기 상태로 바꿈
								if( ChangeState( eOBJECT_STATE_IDLE ) == false )
								{
									assert(NULL);
									NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
								}
							}
						}
						else
						{
							mRange = 0;

							/// 대기 상태로 바꿈
							if( ChangeState( eOBJECT_STATE_IDLE ) == false )
							{
								assert(NULL);
								NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
							}
						}
					}
				}
			}

			SetPos( pos.x, pos.y );

			if( mReadyGotoPos != NiPoint2::ZERO && AIMANAGER->IsPossible( mMapNumber, mObjectPos, mReadyGotoPos, mObject ) == true )
			{
				mGotoX = mReadyGotoPos.x;
				mGotoY = mReadyGotoPos.y;
				mReadyGotoPos = NiPoint2::ZERO;

				if( mCheatHideMode == false )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_MOVE Msg;
					Msg.Category     = NM_PLAYER;
					Msg.Protocol     = NM_PLAYER_MOVE_SYN;
					Msg.characterIdx = mObject.index;
					Msg.destX        = mGotoX;
					Msg.destY        = mGotoY;
					Msg.moveSpeed    = GetMoveSpeed();
					NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
				}
			}

			mServerMoveTime += applyElapsedTime;
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			/// 플레이어의 이동속도
			NiPoint2 pos = mObjectPos;
			float speed = SKILL_PUSHPULL_SPEED;

			/// 방향 설정
			NiPoint2 dir1;
			dir1.x = mGotoX - GetXPos();
			dir1.y = mGotoY - GetYPos();
			dir1.Unitize();

			/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
			pos.x += (speed * ( (float)elapsedTime / SECOND_THOUSAND ) * dir1.x);
			pos.y += (speed * ( (float)elapsedTime / SECOND_THOUSAND ) * dir1.y);

			/// 목적지 좌표와 예상 이동 위치를 비교
			NiPoint2 dir2;
			dir2.x = mGotoX - pos.x;
			dir2.y = mGotoY - pos.y;
			dir2.Unitize();

			/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달로 판정
			if( dir2.Dot(dir1) <= 0.0f )
			{
				/// 플레이어가 목적지까지 도달 했다고 판정이 난경우
				pos.x = mGotoX;
				pos.y = mGotoY;

				if( mKeyboardMove == false )
					ClearMoveSyncCalc();

				/// 대기 상태로 바꿈
				if( ChangeState( eOBJECT_STATE_IDLE ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
				}
			}

			SetPos( pos.x, pos.y );
		}
		break;	
	case ePLAYER_STATE_RUSH:
		{
			/// 플레이어의 이동속도
			NiPoint2 pos = mObjectPos;
			float speed = SKILL_RUSH_SPEED;

			/// 방향 설정
			NiPoint2 dir1;
			dir1.x = mGotoX - GetXPos();
			dir1.y = mGotoY - GetYPos();
			dir1.Unitize();

			/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
			pos.x += (speed * ( (float)elapsedTime / SECOND_THOUSAND ) * dir1.x);
			pos.y += (speed * ( (float)elapsedTime / SECOND_THOUSAND ) * dir1.y);

			/// 목적지 좌표와 예상 이동 위치를 비교
			NiPoint2 dir2;
			dir2.x = mGotoX - pos.x;
			dir2.y = mGotoY - pos.y;
			dir2.Unitize();

			/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달로 판정
			if( dir2.Dot(dir1) <= 0.0f )
			{
				/// 플레이어가 목적지까지 도달 했다고 판정이 난경우
				pos.x = mGotoX;
				pos.y = mGotoY;

				if( mKeyboardMove == false )
					ClearMoveSyncCalc();

				/// 대기 상태로 바꿈
				if( ChangeState( eOBJECT_STATE_IDLE ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
				}
			}
			else
			{
				if( FloatToInt(mRange) != 0 )
				{
					/// 타겟 좌표 설정
					NiPoint3 targetPos( mRangeTargetPos.x, mRangeTargetPos.y, 0.0f );
					NiPoint3 playerGuessPos( pos.x, pos.y, 0.0f );
					AIMANAGER->CalcHeight( mMapNumber, &playerGuessPos.z, playerGuessPos.x, playerGuessPos.y );
					NiPoint3 playerNowPos( GetXPos(), GetYPos(), Height() );

					if( mRangeTarget.type == eOBJECTTYPE_NONE )
					{
						AIMANAGER->CalcHeight( mMapNumber, &targetPos.z, targetPos.x, targetPos.y );

						float rangeFix = OBJECTMANAGER->ObjectSizeRange( this, 0, mRange );

						cRangeCheck range( rangeFix );
						if( range.IsRange( playerGuessPos, targetPos ) )
						{
							NiPoint3 p;

							NiPoint3 d = playerNowPos - targetPos;
							d.Unitize();

							p.x = targetPos.x + (rangeFix-1.0f)*d.x;
							p.y = targetPos.y + (rangeFix-1.0f)*d.y;

							pos.x = p.x;
							pos.y = p.y;

							if( mKeyboardMove == false )
								ClearMoveSyncCalc();

							SetMoveTargetPos( p.x, p.y );
							mRange = 0.0f;

							/// 대기 상태로 바꿈
							if( ChangeState( eOBJECT_STATE_IDLE ) == false )
							{
								assert(NULL);
								NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
							}
						}
					}
					else
					{
						cBaseObject* pTarget = OBJECTMANAGER->GetObject( mRangeTarget.type, mRangeTarget.index );
						if( pTarget != NULL )
						{
							targetPos.x = pTarget->GetXPos();
							targetPos.y = pTarget->GetYPos();
							targetPos.z = pTarget->Height();
							
							mGotoX = targetPos.x;
							mGotoY = targetPos.y;

							float rangeFix = OBJECTMANAGER->ObjectSizeRange( this, pTarget, mRange );

							cRangeCheck range( rangeFix );
							if( range.IsRange( playerGuessPos, targetPos ) )
							{
								NiPoint3 p;

								NiPoint3 d = playerNowPos - targetPos;
								d.Unitize();

								p.x = targetPos.x + (rangeFix-1.0f)*d.x;
								p.y = targetPos.y + (rangeFix-1.0f)*d.y;

								pos.x = p.x;
								pos.y = p.y;

								if( mKeyboardMove == false )
									ClearMoveSyncCalc();

								SetMoveTargetPos( p.x, p.y );
								mRange = 0.0f;

								/// 대기 상태로 바꿈
								if( ChangeState( eOBJECT_STATE_IDLE ) == false )
								{
									assert(NULL);
									NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
								}
							}
						}
						else
						{
							mRange = 0;

							/// 대기 상태로 바꿈
							if( ChangeState( eOBJECT_STATE_IDLE ) == false )
							{
								assert(NULL);
								NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
							}
						}
					}
				}
			}

			if( IsMovePossible( mObjectPos, pos ) != 0 )
			{
				ChangeState( eOBJECT_STATE_IDLE );
				
				// 다른 플레이어들에게도 알림
				MSG_SYN_MOVE_STOP Msg;
				Msg.Category = NM_PLAYEREXT;
				Msg.Protocol = NM_PLAYEREXT_RUSHMOVESTOP_SYN;
				Msg.mCharacterIdx = mObject.index;
				Msg.mCharacterPosX = GetXPos();
				Msg.mCharacterPosY = GetYPos();
				NETWORK2->QuickSend( this, (char*)&Msg, sizeof(Msg) );
			}
			else
			{
				SetPos( pos.x, pos.y );
			}
		}
		break;	
	case eOBJECT_STATE_ATTACK:
		{
			if( SKILLMANAGER->IsUsingSkill( mObject.type, mObject.index ) == false )
			{
				if( ChangeState( eOBJECT_STATE_IDLE ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_IDLE ) == false", mObject.index );
				}
			}
		}
		break;
	case eOBJECT_STATE_STOP:
		if ( GetStateStop( ) == eSTOP_OPENTAROT )
		{
			if ( IsTarotReaderClose( ) == true )
			{
				if ( IsTarotCloseReserved( ) == true )
				{
					TarotReaderClose( ERROR_TAROT_READER_CLOSE_SUCCESS );
				}
				else if( !(GetTarotItemCount( ) > 0) || IsAddMoney( GetTarotPrice() ) == false )
				{
					TarotReaderClose( ERROR_TAROT_READER_CLOSE_ITEM );
				}
			}
		}
		else if ( GetStateStop() == eSTOP_CHEATSTOP )
		{
			if( mCheatStopDuration < accumTime )
			{
				mCheatStopDuration = 0;
				ChangeState( eOBJECT_STATE_IDLE );
				SetStateStop( eSTOP_NONE );
			}
		}
		else if( GetStateStop() == eSTOP_ENHANCED )
		{
			if( mInventoryDb.enhanced == false && mItemEnhanceEndTime <= accumTime )
			{
				ItemEnhancedEnd();
			}
		}
		else if( GetStateStop() == eSTOP_ITEMMIX )
		{
			/// 조합 시간 체크
			if( mInventoryDb.itemMix == false && mItemMixEndTime <= accumTime )
			{
				ItemMixEnd();
			}
		}
		break;
	}

	/// 자연 회복
	CalcNatureRecovery();

	/// 생활 스킬 종료 체크
	if( mPlayerExrInfo.mCommunitySkillIdx != 0 )
	{
		if( mCommunitySkillEndTime < accumTime || GetState() != eOBJECT_STATE_IDLE )
		{
			mPlayerExrInfo.mCommunitySkillIdx = 0;
			mCommunitySkillEndTime = 0.0f;
		}
	}

	/// 거래 상대방 체크
	if( mExchangeStatus > ItemExchangeAsk )
	{
		cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );
		if ( player != NULL )
		{
			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, player, ITEM_VALID_DISTANCE + SYNC_MOVE_RANGE );
			if ( (mObjectPos - player->GetPos( )).Length( ) > tempRange )
			{
				ExchangeCancel();
			}
		}
	}

	/// 아직 걸리지 않은 pc방 버프를 걸어본다.
	if( mGameIn == true && mPcBangNextCheckTime < accumTime && mReadyPcInfSet.GetSize() != 0 )
	{
		mPcBangNextCheckTime = accumTime + INFLUENE_PCBANG_NEXT_ADDTIME;

		cHashSet::cIterator i = mReadyPcInfSet.Begin();
		while( i != mReadyPcInfSet.End() )
		{
			unsigned long infIdx = (*i++);
			if( SKILLMANAGER->AddInfluence( mObject, mObject, infIdx, 0, true ) == true )
				mReadyPcInfSet.Erase( infIdx );
		}
	}

	/// 아직 삭제하지 않은 GM이벤트 버프를 삭제한다.
	if( mReadyDelGMEventList.GetSize() != 0 && mGameIn == true && mGMEventDelNextCheckTime < accumTime )
	{
		mGMEventDelNextCheckTime = accumTime + INFLUENCE_GMEVENT_NEXT_ADDTIME;

		time_t now;
		time( &now );

		cGMEventList::cIterator pos = mReadyDelGMEventList.Begin();
		while ( pos != mReadyDelGMEventList.End() )
		{
			sGMEventInfo& eventInfo = (*pos);

			/// 시간이 지난것이 있으면 delete 로 빼기.
			if( difftime( eventInfo.validThru, now ) < 1 )
			{
				SKILLMANAGER->DeleteInfluenceClassIdx( mObject, eventInfo.influenceIdx );
				pos = mReadyDelGMEventList.Erase( pos );
				continue;
			}
			++pos;
		}
	}
	
	/// 아직 걸리지 않은 GM이벤트 버프를 걸어본다.
	if( mReadyGMEventList.GetSize() != 0 && mGameIn == true && mGMEventNextCheckTime < accumTime  )
	{
		mGMEventNextCheckTime = accumTime + INFLUENCE_GMEVENT_NEXT_ADDTIME;

		time_t now;
		time( &now );

		cGMEventList::cIterator pos = mReadyGMEventList.Begin();
		while ( pos != mReadyGMEventList.End() )
		{
			sGMEventInfo& eventInfo = (*pos);

			if( difftime( now, eventInfo.startTime ) > 0 && difftime( eventInfo.validThru, now ) > 0 )
			{
				/// 적용될 시간이 있는것이 있다면 적용시키기
				if( SKILLMANAGER->AddInfluence( mObject, mObject, eventInfo.influenceIdx, 0, true ) == true )
				{
					///실재 적용된 것들을 삭제 대기 리스트에 넣는다.
					///이미 삭제 대기 큐에 있는 경우 무시한다.
					cGMEventList::cIterator find = mReadyDelGMEventList.Begin();
					cGMEventList::cIterator  end = mReadyDelGMEventList.End();
					for ( ;find != end ;++find )
					{
						sGMEventInfo& delEventInfo = (*find);
						if( delEventInfo.index == eventInfo.index && delEventInfo.influenceIdx == eventInfo.influenceIdx )
							break;
					}
					if( find == end )
						mReadyDelGMEventList.PushBack( eventInfo );

					mGMEventDelNextCheckTime = 0;

					///실재 적용된 것은 추가 대기 리스트에서 뺀다.
					pos = mReadyGMEventList.Erase( pos );
			
					continue;
				}
			}
			++pos;
		}
	}

	/// 일괄 저장
	if( mBatchSaveTime <= accumTime )
	{
		if( NETWORK2->GetServerType() != _E_ST_ID_PVP_ )
			DBUpdate( false, PLAYER_DBUPDATE_PROCESS );
	}

#ifdef _DEBUG
	/// 테스트코드
	static bool check = false;
	static unsigned long delayTime = 100;
	static bool sync = false;
	if( check == true && mServerPosTime < NETWORK2->GetAccumTime() )
	{
		mServerPosTime += delayTime;

		HANDLE              handle = NULL;
		MSG_SERVERPOS_TEST* resMsg = (MSG_SERVERPOS_TEST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_TEST_SERVERPOS );
		if ( resMsg != NULL )
		{
			resMsg->mCharacterIdx = mObject.index;
			resMsg->mServerPosX = mObjectPos.x;
			resMsg->mServerPosY = mObjectPos.y;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SERVERPOS_TEST) );
		}

		if( sync == true )
		{
			MSG_SERVERPOS_TEST msg;
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_TEST_SERVERPOS;
			msg.mCharacterIdx = mObject.index;
			msg.mServerPosX = mObjectPos.x;
			msg.mServerPosY = mObjectPos.y;
			NETWORK2->QuickSendExcept( this, (char*)&msg, sizeof(MSG_SERVERPOS_TEST) );
		}
	}
#endif

	// 아이템 판매대행 검사.
	if ( mItemAgentCheck == true )
	{
		if ( mItemAgentNextCheck < accumTime )
		{
			HANDLE            handle = NULL;
			ITEM_AGENT_CHECK* check  = (ITEM_AGENT_CHECK*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_CHECK );

			check->characterIdx = mObject.index;

			NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(ITEM_AGENT_CHECK) );
			SetItemAgentCheck( DELAY_ITEM_AGENT_CHECK );
		}
	}

	// 우편 검사.
	if ( mPostCheck == true )
	{
		if ( mPostNextCheck < accumTime )
		{
			HANDLE      handle = NULL;
			POST_CHECK* check  = (POST_CHECK*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_CHECK );

			check->characterIdx = mObject.index;

			NETWORK2->SendSQL( handle, sizeof(POST_CHECK) );
			SetPostCheck( DELAY_POST_CHECK );
		}
	}

	// 아이템 빌.
	if ( mItemBillRoot.pool && mItemBillStart == true )
	{
		PerItemBill* itemBill = (PerItemBill*)mItemBillRoot.pool;
		PerItemBill* nextBill;
		time_t       current;
		double       result;

		long         elapsed;
		long         before;
		long         after;

		time( &current );

		while ( itemBill != NULL && mInventoryDb.itemBill == false )
		{
			nextBill = (PerItemBill*)itemBill->next;
			result   = difftime( itemBill->timeEnd, current );

			if ( (result <= 0) || (itemBill->apply == ItemBillApplyReserved) )
			{
				// 종료됨.
				if ( itemBill->type == ITEM_BILL_TIME )
				{
					elapsed = (long)difftime( current, itemBill->timeBegin );
					before  = itemBill->validTime;
					after   = itemBill->validTime-(before < elapsed ? before : elapsed);

					SaveItemBill( itemBill->idx, elapsed, before, after, itemBill->inventoryIdx );
				}

				TB_INVENTORY* inventory = SelectInventoryIdx( itemBill->inventoryIdx, INVENTORY_EQUIP_BEGIN, INVENTORY_WAREHOUSE_END );

				HANDLE            handle = NULL;
				ITEM_BILL_REMOVE* remove = (ITEM_BILL_REMOVE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_BILL_REMOVE );

				remove->idx = itemBill->idx;

				if ( inventory != NULL )
				{
					remove->inventory = (*inventory);
					RemoveInventory( inventory );
				}

				mInventoryDb.itemBill = NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(ITEM_BILL_REMOVE) );
				ITEMBILLPOOL->ReleaseItemBill( &mItemBillRoot, itemBill );
			}
			else if ( itemBill->type == ITEM_BILL_TIME )
			{
				// 사용중 - 시간제만, 단위 시간만다 LOG 발생.
				elapsed = (long)difftime( current, itemBill->timeBegin );
				if ( elapsed >= DELAY_ITEM_BILL )
				{
					before = itemBill->validTime;
					after  = itemBill->validTime-(before < elapsed ? before : elapsed);

					SaveItemBill( itemBill->idx, elapsed, before, after, itemBill->inventoryIdx );
					itemBill->validTime = after;
					itemBill->timeBegin = current;
				}
			}

			itemBill = nextBill;
		}
	}
	// 아이템 지급.
	if ( mItemGetRoot.pool )
	{
		HANDLE        handle  = NULL;
		ITEM_GET*     itemGet = (ITEM_GET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_GET_QUEST );
		u_long        length  = sizeof(ITEM_GET) - sizeof(itemGet->table);
		TB_INVENTORY* table   = itemGet->table;

		itemGet->characterIdx = mObject.index;

		while ( mItemGetRoot.pool != NULL )
		{
			(*table) = ((PerItemGet*)mItemGetRoot.pool)->inventory;

			itemGet->rowCount++; table++;

			ITEMGETPOOL->ReleaseItemGet( &mItemGetRoot, (PerItemGet*)mItemGetRoot.pool );
		}

		length += (itemGet->rowCount * sizeof(itemGet->table));
		NETWORK2->SendSQL( mConnectionIdx, handle, length );
	}

	// 아이템 변경.
	if ( mItemChange == true )
	{
		// 인벤토리 무게 갱신.
		TB_INVENTORY*   inventory;
		TB_ITEM_DEFINE* itemDefine;

		mItemWeight = 0;

		inventory = SelectInventory( INVENTORY_CHARACTER_BEGIN );
		for ( int i = INVENTORY_CHARACTER_BEGIN; i <= INVENTORY_CHARACTER_END; ++i, ++inventory )
		{
			itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
			if ( itemDefine != NULL )
			{
				mItemWeight = mItemWeight + (itemDefine->weight * inventory->count);
			}
		}

		// 적용완료.
		mItemChange = false;
	}

	if( mXCopyComplete == true )
	{
		// 퀘스트 갱신.
		UpdateDutyItem( );

		mXCopyComplete = false;
	}

	/// 캐시 버프들 주기 저장
	if( mCashInfNextSaveTime < accumTime && mIsCashInfCycleSave == false )
	{
		if( mMapChange == false && mGameIn == true )
		{
			SKILLMANAGER->CycleSaveCashInfluence( this );
			mCashInfNextSaveTime = accumTime + INFLUENCE_CASH_NEXT_SAVETIME;
		}
	}

	else if ( mAutoPlayer == true || (mTrialAnswerTime > 0 && mTrialAnswerTime < accumTime) )
	{
		TrialEnd( true );
	}
}


/// 직업 체인지
void cPlayer::SetJob(ePLAYER_JOB job, unsigned long* skillList, unsigned char skillListCnt )
{ 
	mPlayerInfo.Job = job;
	SetJumpTime();

	/// 전직 관련 퀘스트 트리거 검사
	SendNewQuestList();

	if( GetGuildIndex() > 0 )
	{
		GUILDMAN->RequestUpdateUser( GetGuildIndex(), GetObjectID() );
	}

	HANDLE       handle = NULL;
	MSG_RES_JOB* resMsg = (MSG_RES_JOB*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_JOB_RES );
	if ( resMsg != NULL )
	{
		resMsg->mJob = (unsigned short)mPlayerInfo.Job;
		resMsg->mSkillListCnt = skillListCnt;
		memcpy( resMsg->mSkillClassIdx, skillList, sizeof(unsigned long) * skillListCnt );
		NETWORK2->SendMsgRoot( handle, resMsg->GetMsgLength() );
	}

	for( int i = 0 ; i < skillListCnt ; ++i )
	{
		unsigned char error = SKILLMANAGER->AddPlayerHaveSkill( mObject.index, skillList[i], 0, 0 );
		if( error != SKILL_ADD_ERR_SUCCESS )
		{
			NETWORK2->PostServerEvent("cPlayer::SetJob AddPlayerHaveSkill[%d][%d,%d][%d] == false",
				mObject.index, skillList[i], 0, error );
		}
	}

	MSG_SYN_JOB synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_JOB_SYN;
	synMsg.mPlayerIdx = mObject.index;
	synMsg.mJob = (unsigned short)job;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );

	/// 파티에 전직 싱크 보내기
	PARTYMAN->SendToJobChange( this );
}

int cPlayer::SkillResetReq( unsigned short itemSlotNum )
{
	/*-- 아이템 사용시 검사할 내용들.
	*/
	/// db 사용부 체크
	if ( mInventoryDatabase )
		return ERROR_SKILLRESET_DBERROR;

    if ( IsInventoryBag( itemSlotNum ) == false )
		return ERROR_SKILLRESET_ITEM;

	TB_INVENTORY* inventory = SelectInventory( itemSlotNum );
	if( IsInventory( inventory ) == false )
		return ERROR_SKILLRESET_ITEM;

	if( inventory->apply != InventoryApplyNone )
		return ERROR_SKILLRESET_ITEM;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if( itemDefine == NULL )
		return ERROR_SKILLRESET_ITEM;

    if ( IsInventorySeal( itemDefine, inventory ) == true )
        return ERROR_SKILLRESET_ITEM_LICENSE;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx) )
		return ERROR_SKILLRESET_ITEM;

	/// 스킬 초기화 아이템 인지 확인
	if( !(itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_SKILLRESET) )
		return ERROR_SKILLRESET_ITEM;

	/// 캐릭터 정보db 사용중인지 체크
	if( mIsDbUpdate == true )
		return ERROR_SKILLRESET_DBERROR;

	/// 상태 설정
	if( ChangeState( eOBJECT_STATE_STOP ) == false )
		return ERROR_SKILLRESET_STATE;

	SetStateStop( eSTOP_SKILLRESET );

	/// 아이템 차감
	unsigned long inventoryIdx = inventory->idx;
	if ( inventory->count == 1 )
		RemoveInventory( inventory->number );
	else
		UpdateInventory( inventory, (-1) );

	/// db 저장
	HANDLE                 handle = NULL;
	CHARACTER_SKILL_RESET* reset  = (CHARACTER_SKILL_RESET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_SKILLRESET );
	unsigned long          length = sizeof(CHARACTER_SKILL_RESET) - sizeof(reset->mTable);

	reset->mCharacterIdx = mObject.index;
	reset->mRace = mPlayerInfo.Race;
	reset->mJobStep = (unsigned char)STATUSSCRIPT->GetJobStep( mPlayerInfo.Job );
	reset->mJob = mPlayerInfo.Job;
	reset->mInventoryIdx = inventoryIdx;
	reset->mRetvalue = -1;

	if( NETWORK2->SendSQL( mConnectionIdx, handle, length ) == false )
		return ERROR_SKILLRESET_DBERROR;

	mInventoryDb.skillReset = true;
	mIsDbUpdate = true;

	return ERROR_SKILLRESET_SUCCESS;
}


int cPlayer::ThemeResetReq( unsigned short itemSlotNum, unsigned short themeNum, unsigned char themeMode )
{
	/*-- 아이템 사용시 검사할 내용들.
	*/
	/// db 사용부 체크
	if ( mInventoryDatabase )
		return ERROR_THEMERESET_DBERROR;

	if ( IsInventoryBag( itemSlotNum ) == false )
		return ERROR_THEMERESET_ITEM;

	TB_INVENTORY* inventory = SelectInventory( itemSlotNum );
	if( IsInventory( inventory ) == false )
		return ERROR_THEMERESET_ITEM;

	if( inventory->apply != InventoryApplyNone )
		return ERROR_THEMERESET_ITEM;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if( itemDefine == NULL )
		return ERROR_THEMERESET_ITEM;

	if ( IsInventorySeal( itemDefine, inventory ) == true )
		return ERROR_THEMERESET_ITEM_LICENSE;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx) )
		return ERROR_THEMERESET_ITEM;

	/// 테마 초기화 아이템 인지 확인
	if( !(itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_THEME_RESET) )
		return ERROR_THEMERESET_ITEM;

	PerThemeUser* pTheme = SearchThemeUser( MAKETHEMECID( themeNum, themeMode ) );
	if( pTheme == NULL )
		return ERROR_THEMERESET_READY;

	time_t ltime;
	time( &ltime );
	long result = (long)difftime( pTheme->validThru, ltime );
	if ( result * SECOND < 5 * MINUTE )
		return ERROR_THEMERESET_READY;

	/// 상태 설정
	if( ChangeState( eOBJECT_STATE_STOP ) == false )
		return ERROR_THEMERESET_STATE;

	SetStateStop( eSTOP_THEMERESET );

	/// 아이템 차감
	unsigned long inventoryIdx = inventory->idx;
	if ( inventory->count == 1 )
		RemoveInventory( inventory->number );
	else
		UpdateInventory( inventory, (-1) );

	/// db 저장
	HANDLE                 handle = NULL;
	CHARACTER_THEME_RESET* reset  = (CHARACTER_THEME_RESET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_THEMERESET );
	unsigned long          length = sizeof(CHARACTER_THEME_RESET) - sizeof(reset->mTable);

	reset->mCharacterIdx = mObject.index;
	reset->mThemeIdx = themeNum;
	reset->mThemeMode = themeMode;
	reset->mInventoryIdx = inventoryIdx;
	reset->mSlotNum = itemSlotNum;
	reset->mRetvalue = -1;

	if( NETWORK2->SendSQL( mConnectionIdx, handle, length ) == false )
		return ERROR_THEMERESET_DBERROR;

	mInventoryDb.themeReset = true;

	return ERROR_THEMERESET_SUCCESS;
}


void cPlayer::AddExp( unsigned long addExp, bool addOption, bool expRecover )
{
	if( addExp == 0 )
		return;

	unsigned char level = mPlayerInfo.Level;
	unsigned long totalExp = mHeroInfo.Exp;
	unsigned long calcAddExp = 0;

	//////////////////////////////////////////////////////////////////////////
	/// 추가 경험치 계산
	if( addOption == true )
	{
		/// 추가옵션 경험치 증가치
		float addExpPer = (float)mStatusEtc.mExpAddPer;
		float addExpPlus = (float)mStatusEtc.mExpAddPlus;

		/// 경험치 계산
		calcAddExp = FloatToInt( addExp + ( addExp * addExpPer / 100.0f ) + addExpPlus );		/// 경험치 = 경험치 + %효과 + 효과
	}
	else
		calcAddExp = addExp;

	//////////////////////////////////////////////////////////////////////////
	/// 레벨업 수치와 남은 경험치 계산

	/// 다음 레벨로 넘어가기 위한 필요 경험치를 얻어옴
	sExpTable* pExpTable = LEVELSCRIPT->GetExpTable( level );
	if( pExpTable == NULL )
		return; 

	/// 저장 안된 경험치 기록
	mWaitSaveExp += addExp;
	/// 경험치가 일정량 이상 쌓인상태로 저장이 안됐으면 저장하게 한다
	if( mPlayerInfo.Level > 15 )
	{
		if( mPlayerInfo.Level < 20 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.20f ) )
				mBatchSaveTime = 0;
		}
		else if(  mPlayerInfo.Level < 25 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.15f ) )
				mBatchSaveTime = 0;
		}
		else  if(  mPlayerInfo.Level < 30 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.10f ) )
				mBatchSaveTime = 0;
		}
		else if(  mPlayerInfo.Level < 35 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.05f ) )
				mBatchSaveTime = 0;
		}
		else
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.02f ) )
				mBatchSaveTime = 0;
		}
	}

	sExpTable* pMax = LEVELSCRIPT->GetExpTable( level + 1 );
	if( pMax == NULL )		/// 다음 레벨정보가 없으면 레벨업 안하고 경험치만 증가
	{
		/// 현재 경험치 최대보다 많은경우 최대 -1 설정
		if( pExpTable->mExp <= totalExp + calcAddExp )
			mHeroInfo.Exp = pExpTable->mExp - 1;
		else		
			mHeroInfo.Exp += calcAddExp;	/// 습득한 경험치를 현재경험치에 더함
	}
	else	/// 정상적인 범위내에서의 레벨업 처리
	{
		/// 습득한 경험치를 현재경험치에 더함
		totalExp += calcAddExp;

		/// 현재 레벨 경험치 최대 값을 가져옴
		unsigned long maxExp = pExpTable->mExp;
		/// 레벨업 수치
		unsigned char levelUp = 0;

		/// 경험치 총합이 다음 레벨로 넘어가기 위한 경험치 보다 크면 레벨업
		while( totalExp >= maxExp )
		{
			/// 현재 경험치에서 레벨업할 만큼의 경험치를 감소
			totalExp -= maxExp;

			++levelUp;

			/// 레벨업한 후다음 레벨로 넘어 가기 위한 경험치 최대 값
			pExpTable = LEVELSCRIPT->GetExpTable( level + levelUp );
			if( pExpTable == NULL )
			{
				/// 2단계 레벨업을 하는데 그사이에 만렙이 껴잇는경우는 없겠지?
				NETWORK2->PostServerEvent("cPlayer::AddExp pExpTable == NULL [%d,%d,%d]", mObject.index, level, levelUp );
				return;
			}
			maxExp = pExpTable->mExp;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 적용부
		if( levelUp > 0 )
		{
			unsigned long recoverValue = 0;
			if( expRecover == true )
				recoverValue = calcAddExp;

			/// 레벨정보 저장 & 경험치 적용
			if( LevelUp( levelUp, totalExp, false ) == false )
				return;
		}
		else
			mHeroInfo.Exp = totalExp;		/// 경험치 적용
	}

	if( expRecover == false )
	{
		//////////////////////////////////////////////////////////////////////////
		/// 경험치 메세지 발송
		HANDLE       handle = NULL;
		MSG_RES_EXP* msgExp = (MSG_RES_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EXP_RES );
		if ( msgExp != NULL )
		{
			msgExp->mExp      = calcAddExp;
			msgExp->mTotalExp = mHeroInfo.Exp;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_EXP) );
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		/// 경험치 메세지 발송
		HANDLE       handle = NULL;
		MSG_RES_RECOVER_EXP* msgExp = (MSG_RES_RECOVER_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EXP_RECOVER_RES );
		if ( msgExp != NULL )
		{
			msgExp->mExp      = calcAddExp;
			msgExp->mTotalExp = mHeroInfo.Exp;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_RECOVER_EXP) );
		}
	}
}

void cPlayer::AddPartyExp( unsigned long addExp, bool addOption, bool expRecover, unsigned long addBaseExp, unsigned long addPartyExp )
{
	if( addExp == 0 )
		return;

	unsigned char level = mPlayerInfo.Level;
	unsigned long totalExp = mHeroInfo.Exp;
	unsigned long calcAddExp = 0;

	/// 추가 경험치 계산
	unsigned long addPlusBaseExp = addBaseExp;
	unsigned long addPlusPartyExp = addPartyExp;

	//////////////////////////////////////////////////////////////////////////
	/// 추가 경험치 계산
	if( addOption == true )
	{
		/// 추가옵션 경험치 증가치
		float addExpPer = (float)mStatusEtc.mExpAddPer;
		float addExpPlus = (float)mStatusEtc.mExpAddPlus;
		
		addPlusBaseExp = FloatToInt( addBaseExp + ( addBaseExp * addExpPer / 100.0f ) + addExpPlus );		/// 경험치 = 경험치 + %효과 + 효과
		addPlusPartyExp = FloatToInt( addPartyExp + ( addPartyExp * addExpPer / 100.0f ) );					/// 경험치 = 경험치 + %효과

		calcAddExp = addPlusBaseExp + addPlusPartyExp;
	}
	else
		calcAddExp = addExp;

	//////////////////////////////////////////////////////////////////////////
	/// 레벨업 수치와 남은 경험치 계산

	/// 다음 레벨로 넘어가기 위한 필요 경험치를 얻어옴
	sExpTable* pExpTable = LEVELSCRIPT->GetExpTable( level );
	if( pExpTable == NULL )
		return; 

	/// 저장 안된 경험치 기록
	mWaitSaveExp += addExp;
	/// 경험치가 일정량 이상 쌓인상태로 저장이 안됐으면 저장하게 한다
	if( mPlayerInfo.Level > 15 )
	{
		if( mPlayerInfo.Level < 20 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.20f ) )
				mBatchSaveTime = 0;
		}
		else if(  mPlayerInfo.Level < 25 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.15f ) )
				mBatchSaveTime = 0;
		}
		else  if(  mPlayerInfo.Level < 30 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.10f ) )
				mBatchSaveTime = 0;
		}
		else if(  mPlayerInfo.Level < 35 )
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.05f ) )
				mBatchSaveTime = 0;
		}
		else
		{
			if( mWaitSaveExp > (unsigned long)( pExpTable->mExp * 0.02f ) )
				mBatchSaveTime = 0;
		}
	}

	sExpTable* pMax = LEVELSCRIPT->GetExpTable( level + 1 );
	if( pMax == NULL )		/// 다음 레벨정보가 없으면 레벨업 안하고 경험치만 증가
	{
		/// 현재 경험치 최대보다 많은경우 최대 -1 설정
		if( pExpTable->mExp <= totalExp + calcAddExp )
			mHeroInfo.Exp = pExpTable->mExp - 1;
		else		
			mHeroInfo.Exp += calcAddExp;	/// 습득한 경험치를 현재경험치에 더함
	}
	else	/// 정상적인 범위내에서의 레벨업 처리
	{
		/// 습득한 경험치를 현재경험치에 더함
		totalExp += calcAddExp;

		/// 현재 레벨 경험치 최대 값을 가져옴
		unsigned long maxExp = pExpTable->mExp;
		/// 레벨업 수치
		unsigned char levelUp = 0;

		/// 경험치 총합이 다음 레벨로 넘어가기 위한 경험치 보다 크면 레벨업
		while( totalExp >= maxExp )
		{
			/// 현재 경험치에서 레벨업할 만큼의 경험치를 감소
			totalExp -= maxExp;

			++levelUp;

			/// 레벨업한 후다음 레벨로 넘어 가기 위한 경험치 최대 값
			pExpTable = LEVELSCRIPT->GetExpTable( level + levelUp );
			if( pExpTable == NULL )
			{
				/// 2단계 레벨업을 하는데 그사이에 만렙이 껴잇는경우는 없겠지?
				NETWORK2->PostServerEvent("cPlayer::AddExp pExpTable == NULL [%d,%d,%d]", mObject.index, level, levelUp );
				return;
			}
			maxExp = pExpTable->mExp;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 적용부
		if( levelUp > 0 )
		{
			unsigned long recoverValue = 0;
			if( expRecover == true )
				recoverValue = calcAddExp;

			/// 레벨정보 저장 & 경험치 적용
			if( LevelUp( levelUp, totalExp, false ) == false )
				return;
		}
		else
			mHeroInfo.Exp = totalExp;		/// 경험치 적용
	}

	if( expRecover == false )
	{
		//////////////////////////////////////////////////////////////////////////
		/// 경험치 메세지 발송
		if( addPlusPartyExp < 1 )
		{
			HANDLE       handle = NULL;
			MSG_RES_EXP* msgExp = (MSG_RES_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EXP_RES );
			if ( msgExp != NULL )
			{
				msgExp->mExp      = calcAddExp;
				msgExp->mTotalExp = mHeroInfo.Exp;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_EXP) );
			}
		}
		else
		{
			HANDLE       handle = NULL;
			MSG_RES_PARTY_EXP* msgExp = (MSG_RES_PARTY_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_PARTY_EXP_RES );
			if ( msgExp != NULL )
			{
				msgExp->mExp	  = calcAddExp;
				msgExp->mPartyExp = addPlusPartyExp;
				msgExp->mTotalExp = mHeroInfo.Exp;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PARTY_EXP) );
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		/// 경험치 메세지 발송
		HANDLE       handle = NULL;
		MSG_RES_RECOVER_EXP* msgExp = (MSG_RES_RECOVER_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EXP_RECOVER_RES );
		if ( msgExp != NULL )
		{
			msgExp->mExp      = calcAddExp;
			msgExp->mTotalExp = mHeroInfo.Exp;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_RECOVER_EXP) );
		}
	}
}

bool cPlayer::LevelUp( unsigned char addLevel, unsigned long totalExp, bool isCheat ) 
{ 
	unsigned short sumLevel = mPlayerInfo.Level + addLevel;
	if( LEVELSCRIPT->GetMaxLevel() < sumLevel )
		return false;

	/// 레벨업전 스텟1 저장
	sStatus1 oldStatus1;
	oldStatus1 = mStatus1;

	mPlayerInfo.Level = (unsigned char)sumLevel; 
	mHeroInfo.Exp = totalExp;

	/// 장비 아이템 정보 갱신.
	EquipItems( );

	STATUSCALC->CalcPlayerInit( mObject );

	/// 치트로 인한 레벨업인지 확인
	int updateFlag = PLAYER_DBUPDATE_LEVELUP;
	if( isCheat == true )
		updateFlag = PLAYER_DBUPDATE_LEVELCHEAT;

	/// 무조건 저장
	if( DBUpdate( false, updateFlag ) == false )
		mBatchSaveTime = 0;

	HPHeal( GetMaxHP( ), false, mObject, 0, eTAKEDAMAGETYPE_NONE );
	MPHeal( GetMaxMP( ), false );

	if( GetGuildIndex() > 0 )
		GUILDMAN->RequestUpdateUser( GetGuildIndex(), GetObjectID() );

	/// hero 에게 레벨업 메세지 발송
	HANDLE         handle = NULL;
	MSG_RES_LEVEL* msgExp = (MSG_RES_LEVEL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_LEVEL_RES );
	if ( msgExp != NULL )
	{
		msgExp->mLevel = mPlayerInfo.Level;
		msgExp->mExp = totalExp;
		msgExp->mStr = (unsigned char)(FloatToInt( mStatus1.mStr ) - FloatToInt( oldStatus1.mStr ));
		msgExp->mDex = (unsigned char)(FloatToInt( mStatus1.mDex ) - FloatToInt( oldStatus1.mDex ));
		msgExp->mCon = (unsigned char)(FloatToInt( mStatus1.mCon ) - FloatToInt( oldStatus1.mCon ));
		msgExp->mInt = (unsigned char)(FloatToInt( mStatus1.mInt ) - FloatToInt( oldStatus1.mInt ));
		msgExp->mWis = (unsigned char)(FloatToInt( mStatus1.mWis ) - FloatToInt( oldStatus1.mWis ));
		msgExp->mHP = mPlayerExrInfo.RestHP;
		msgExp->mMaxHP = mPlayerExrInfo.MaxHP;
		msgExp->mMP = mPlayerExrInfo.RestMP;
		msgExp->mMaxMP = mPlayerExrInfo.MaxMP;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_LEVEL) );
	}

	/// 주변 플레이어에게 레벨업 이펙트를 붙이기위한 메세지 발송
	MSG_SYN_LEVEL synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_LEVEL_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mLevel = mPlayerInfo.Level;
	synMsg.mHP = mPlayerExrInfo.RestHP;
	synMsg.mMaxHP = mPlayerExrInfo.MaxHP;
	synMsg.mMP = mPlayerExrInfo.RestMP;
	synMsg.mMaxMP = mPlayerExrInfo.MaxMP;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );

	/// 레벨업 퀘스트 트리거 검사
	SendNewQuestList();
	return true;
}

void cPlayer::AddSxp( unsigned long addSxp, bool addOption )
{
	if( addSxp == 0 )
		return;

	/// 스킬 초기화 중엔 sxp 를 받지 않는다.
	if( mInventoryDb.skillReset == true )
		return;

	if( GetStateStop() == eSTOP_SKILLRESET )
		return;

	unsigned char skillLevel = mHeroInfo.SkillLevel;
	unsigned long totalSxp = mHeroInfo.SkillExp;
	unsigned long calcAddSxp = 0;

	//////////////////////////////////////////////////////////////////////////
	/// 추가 경험치 계산
	if( addOption == true )
	{
		/// 추가옵션 경험치 증가치
		float addSxpPer = (float)mStatusEtc.mSxpAddPer;
		float addSxpPlus = (float)mStatusEtc.mSxpAddPlus;

		calcAddSxp = FloatToInt( addSxp + ( addSxp * addSxpPer / 100.0f ) + addSxpPlus );		/// 스킬경험치2 = 스킬경험치1 + %효과 + 효과
	}
	else
		calcAddSxp = addSxp;

	//////////////////////////////////////////////////////////////////////////
	/// 레벨업 수치와 남은 경험치 계산

	/// 현재레벨 경험치 최대를 가져옴
	sSxpTable* pSxpTable = LEVELSCRIPT->GetSxpTable( skillLevel );
	if( pSxpTable == NULL )	
		return; 

	/// 다음 레벨정보가 없으면 레벨업 안하고 경험치만 증가
	sSxpTable* pMax = LEVELSCRIPT->GetSxpTable( skillLevel + 1 );
	if( pMax == NULL )	
	{
		/// 현재 경험치 최대보다 많은경우 최대 -1 설정후 리턴
		if( pSxpTable->mSxp <= totalSxp + calcAddSxp )
			mHeroInfo.SkillExp = pSxpTable->mSxp - 1;
		else
			mHeroInfo.SkillExp += calcAddSxp;		/// 습득한 경험치를 현재경험치에 더함
	}
	else
	{
		/// 습득한 경험치를 현재경험치에 더함
		totalSxp += calcAddSxp;

		/// 현재 레벨 경험치 최대 값을 가져옴
		unsigned long maxSxp = pSxpTable->mSxp;
		/// 레벨업 수치
		unsigned char levelUp = 0;

		/// 경험치 총합이 다음 레벨로 넘어가기 위한 경험치 보다 크면 레벨업
		while( totalSxp >= maxSxp )
		{
			/// 현재 경험치에서 레벨업할 만큼의 경험치를 감소
			totalSxp -= maxSxp;

			++levelUp;

			/// 다음 레벨로 넘어 가기 위한 경험치 최대 값
			pSxpTable = LEVELSCRIPT->GetSxpTable( skillLevel + levelUp );
			if( pSxpTable == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AddSxp pSxpTable == NULL [%d,%d,%d]", mObject.index, skillLevel, levelUp );
				return;
			}
			maxSxp = pSxpTable->mSxp;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 적용부
		if( levelUp > 0 )
		{
			if( SkillLevelUp( levelUp, totalSxp, false ) == false )
				return;
		}
		else
			mHeroInfo.SkillExp = totalSxp;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 경험치 메세지 발송
	HANDLE       handle = NULL;
	MSG_RES_SXP* msgExp = (MSG_RES_SXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_SXP_RES );
	if ( msgExp != NULL )
	{
		msgExp->mSxp      = calcAddSxp;
		msgExp->mTotalSxp = mHeroInfo.SkillExp;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SXP) );
	}
}

void cPlayer::AddPartySxp( unsigned long addSxp, bool addOption, unsigned long addBaseSxp, unsigned long addPartySxp )
{
	if( addSxp == 0 )
		return;

	/// 스킬 초기화 중엔 sxp 를 받지 않는다.
	if( mInventoryDb.skillReset == true )
		return;

	if( GetStateStop() == eSTOP_SKILLRESET )
		return;

	unsigned char skillLevel = mHeroInfo.SkillLevel;
	unsigned long totalSxp = mHeroInfo.SkillExp;
	unsigned long calcAddSxp = 0;

	/// 추가 경험치 계산
	unsigned long addPlusBaseSxp = addBaseSxp;
	unsigned long addPlusPartySxp = addPartySxp;

	//////////////////////////////////////////////////////////////////////////
	/// 추가 경험치 계산
	if( addOption == true )
	{
		/// 추가옵션 경험치 증가치
		float addSxpPer = (float)mStatusEtc.mSxpAddPer;
		float addSxpPlus = (float)mStatusEtc.mSxpAddPlus;

		addPlusBaseSxp  = FloatToInt( addBaseSxp + ( addBaseSxp * addSxpPer / 100.0f ) + addSxpPlus );		/// 경험치 = 경험치 + %효과 + 효과
		addPlusPartySxp = FloatToInt( addPartySxp + ( addPartySxp * addSxpPer / 100.0f ) );					/// 경험치 = 경험치 + %효과

		calcAddSxp = addPlusBaseSxp + addPlusPartySxp;
	}
	else
		calcAddSxp = addSxp;

	//////////////////////////////////////////////////////////////////////////
	/// 레벨업 수치와 남은 경험치 계산

	/// 현재레벨 경험치 최대를 가져옴
	sSxpTable* pSxpTable = LEVELSCRIPT->GetSxpTable( skillLevel );
	if( pSxpTable == NULL )	
		return; 

	/// 다음 레벨정보가 없으면 레벨업 안하고 경험치만 증가
	sSxpTable* pMax = LEVELSCRIPT->GetSxpTable( skillLevel + 1 );
	if( pMax == NULL )	
	{
		/// 현재 경험치 최대보다 많은경우 최대 -1 설정후 리턴
		if( pSxpTable->mSxp <= totalSxp + calcAddSxp )
			mHeroInfo.SkillExp = pSxpTable->mSxp - 1;
		else
			mHeroInfo.SkillExp += calcAddSxp;		/// 습득한 경험치를 현재경험치에 더함
	}
	else
	{
		/// 습득한 경험치를 현재경험치에 더함
		totalSxp += calcAddSxp;

		/// 현재 레벨 경험치 최대 값을 가져옴
		unsigned long maxSxp = pSxpTable->mSxp;
		/// 레벨업 수치
		unsigned char levelUp = 0;

		/// 경험치 총합이 다음 레벨로 넘어가기 위한 경험치 보다 크면 레벨업
		while( totalSxp >= maxSxp )
		{
			/// 현재 경험치에서 레벨업할 만큼의 경험치를 감소
			totalSxp -= maxSxp;

			++levelUp;

			/// 다음 레벨로 넘어 가기 위한 경험치 최대 값
			pSxpTable = LEVELSCRIPT->GetSxpTable( skillLevel + levelUp );
			if( pSxpTable == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AddSxp pSxpTable == NULL [%d,%d,%d]", mObject.index, skillLevel, levelUp );
				return;
			}
			maxSxp = pSxpTable->mSxp;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 적용부
		if( levelUp > 0 )
		{
			if( SkillLevelUp( levelUp, totalSxp, false ) == false )
				return;
		}
		else
			mHeroInfo.SkillExp = totalSxp;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 경험치 메세지 발송
	if( addPlusPartySxp < 1 )
	{
		HANDLE       handle = NULL;
		MSG_RES_SXP* msgExp = (MSG_RES_SXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_SXP_RES );
		if ( msgExp != NULL )
		{
			msgExp->mSxp      = calcAddSxp;
			msgExp->mTotalSxp = mHeroInfo.SkillExp;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SXP) );
		}
	}
	else
	{
		HANDLE       handle = NULL;
		MSG_RES_PARTY_SXP* msgSxp = (MSG_RES_PARTY_SXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_PARTY_SXP_RES );
		if ( msgSxp != NULL )
		{
			msgSxp->mSxp	  = calcAddSxp;
			msgSxp->mPartySxp = addPlusPartySxp;
			msgSxp->mTotalSxp = mHeroInfo.SkillExp;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PARTY_SXP) );
		}
	}
}

bool cPlayer::SkillLevelUp( unsigned char addLevel, unsigned long totalSxp, bool isCheat )
{
	unsigned short sumLevel = mHeroInfo.SkillLevel + addLevel;
	if( LEVELSCRIPT->GetMaxSkill() < sumLevel )
		return false;

	mHeroInfo.SkillLevel = mHeroInfo.SkillLevel + addLevel; 
	mHeroInfo.SkillPointRemain	+= SKILL_LEVELUP_POINT * addLevel;
	mHeroInfo.SkillPointTotal	+= SKILL_LEVELUP_POINT * addLevel;
	mHeroInfo.SkillExp = totalSxp;

	/// 치트로 인한 레벨업인지 확인
	int updateFlag = PLAYER_DBUPDATE_SKILLUP;
	if( isCheat == true )
		updateFlag = PLAYER_DBUPDATE_SKILLCHEAT;

	/// 무조건 저장
	if( DBUpdate( false, updateFlag ) == false )
		mBatchSaveTime = 0;

	/// 레벨업 메세지 발송
	HANDLE              handle = NULL;
	MSG_RES_SKILLLEVEL* resMsg = (MSG_RES_SKILLLEVEL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_SKILLLEVEL_RES );
	if ( resMsg != NULL )
	{
		resMsg->mSkillLevel = mHeroInfo.SkillLevel;
		resMsg->mSkillExp = totalSxp;
		resMsg->mSkillPoint = mHeroInfo.SkillPointRemain;
		resMsg->mSkillPointTotal = mHeroInfo.SkillPointTotal;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILLLEVEL) );
	}

	MSG_SYN_SKILLLEVEL syncMsg;
	syncMsg.Category = NM_PLAYER;
	syncMsg.Protocol = NM_PLAYER_SKILLLEVEL_SYN;
	syncMsg.mPlayerIdx = mObject.index;
	NETWORK2->QuickSend( this, (char*)&syncMsg, sizeof(MSG_SYN_SKILLLEVEL) );

	return true;
}

void cPlayer::InitHP( unsigned long HP )
{
	mPlayerExrInfo.RestHP = HP;
	mPlayerExrInfo.MaxHP = GetMaxHP();

	if( mPlayerExrInfo.RestHP > GetMaxHP() )
		mPlayerExrInfo.MaxHP = mPlayerExrInfo.RestHP;
}

void cPlayer::InitMP( unsigned long MP )
{
	mPlayerExrInfo.RestMP = MP;
	mPlayerExrInfo.MaxMP = GetMaxMP();

	if( mPlayerExrInfo.RestMP > GetMaxMP() )
		mPlayerExrInfo.MaxMP = mPlayerExrInfo.RestMP;
}

unsigned long cPlayer::HPDamage( sObject /*attacker*/, unsigned long damage, bool* die, bool msgSend, bool /*cri*/ )
{ 
	/// 실제 적용된 데미지
	unsigned long realDamage = 0;
	*die = false;

	if( GetStateDie() == true )
	{
		*die = true;
		return realDamage;
	}

	if( mGameIn == false )
		return 0;

	/// 공격받으면 죽는 경우
	if( mPlayerExrInfo.RestHP <= damage ) 
	{
		/// 치트 무적 여부를 검사
		if( mCheatUndeadMode )
			return 0;

		/// [실제적용된데미지]는 player의 [실제HP값]
		realDamage = mPlayerExrInfo.RestHP;
		/// [실제HP값]은 0
		mPlayerExrInfo.RestHP = 0;
		mPlayerExrInfo.RestMP = 0;

		/// 플레이어의 상태 변경
		if( ChangeState( eOBJECT_STATE_DIE ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("Player[%d] Update ChangeState( eOBJECT_STATE_DIE ) == false", mObject.index );
		}

		*die = true;

		/// 죽은 경우 데미지를 입었을 때 사라지는 버프들은 모두 정리된다.

		// 죽은 위치 기록
		char buffer[260];
		int serverType = NETWORK2->GetServerType();

		sprintf( buffer, "Die Srv:%d Map:%d MapData:%d, X:%.2f Y:%.2f ", serverType, mMapNumber, mInDunMapDataNum, mObjectPos.x, mObjectPos.y );
		PostCharacterEvent( EVENT_CHARACTER_UPDATE, buffer );
	}
	else /// 죽지 않는 경우
	{
		/// 전장 깃발 채집 중인경우 데미지 받으면 취소
		if( GetState() == ePLAYER_STATE_GATHERING && NETWORK2->GetServerType() == _E_ST_ID_PVP_ )
		{
			cGathering* pGather = OBJECTMANAGER->GetGathering( mGatheringIdx );
			if( pGather != NULL )
				pGather->CancelGatherByOddity( this );				
		}

		/// 발동 체크
		SKILLMANAGER->CheckActInfluence( eACTSTATUS_DAMAGE, this, 0 );

		/// 잠들어 있는 상태라면 풀어 준다.
		if( mODDITY[eODDITYTYPE_SLEEP] != 0 )
		{
			cSkillInfluenceSet::cIterator begin = mInfluenceSet.Begin();
			cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

			cInfluenceObject* pInf = NULL;
			unsigned long influenceIdx = 0;
			while( begin != end )
			{
				influenceIdx = (*begin++);
				pInf = SKILLMANAGER->GetInfluence( influenceIdx );
				if( pInf == NULL )
					continue;

				if( pInf->IsSleep() == true )
					SKILLMANAGER->DeleteInfluenceList( influenceIdx );
			}
		}

		/// 데미지를 받으면 풀리는 효과들 삭제하기
		if( mDeleteTypeDamagedAry.GetSize() > 0 )
		{
			cLongAry::cIterator begin = mDeleteTypeDamagedAry.Begin();
			cLongAry::cIterator end = mDeleteTypeDamagedAry.End();

			cInfluenceObject* pInf = NULL;
			unsigned long influenceIdx = 0;
			while( begin != end )
			{
				influenceIdx = (*begin++);
				pInf = SKILLMANAGER->GetInfluence( influenceIdx );
				if( pInf == NULL )
					continue;

				if( pInf->GetDeleteType() == eDELETETYPE_PLAYER_DAMAGED )
					SKILLMANAGER->DeleteInfluenceList( influenceIdx );
			}
		}

		/// 치트 무적 여부를 검사
		if( mCheatUndeadMode )
			return 0;

		/// [실제적용된데미지]는 [파라메타데미지]
		realDamage = damage;
		/// [실제HP값]은 [파라메타데미지]뺀값
		mPlayerExrInfo.RestHP -= damage;	
	}

	if( msgSend == true )
	{
		MSG_SYN_PLAYER_HP synMsg;
		synMsg.Category			= NM_PLAYER;
		synMsg.Protocol			= NM_PLAYER_HP_SYN;
		synMsg.mCharacterIdx	= GetObjectID();
		synMsg.mHp				= mPlayerExrInfo.RestHP;
		synMsg.mMaxHp			= GetMaxHP();
		synMsg.mDie				= GetStateDie();

		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_HP) );
	}

	/// [실제적용된데미지]리턴
	return realDamage;
}

void cPlayer::HPUse( unsigned long useValue, bool msgSend )
{
	if( mPlayerExrInfo.RestHP < useValue )
	{
		assert(NULL);		
		NETWORK2->PostServerEvent("cPlayer::HPUse [%d,%d,%d]", mObject.index, useValue, mPlayerExrInfo.RestHP );
		mPlayerExrInfo.RestHP = 1;
	}
	else
		mPlayerExrInfo.RestHP -= useValue;

	if( msgSend == true )
	{
		MSG_SYN_PLAYER_HP synMsg;
		synMsg.Category			= NM_PLAYER;
		synMsg.Protocol			= NM_PLAYER_HP_SYN;
		synMsg.mCharacterIdx	= GetObjectID();
		synMsg.mHp				= mPlayerExrInfo.RestHP;
		synMsg.mMaxHp			= GetMaxHP();
		synMsg.mDie				= GetStateDie();

		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_HP) );
	}
}

void cPlayer::HPHeal( unsigned long hp, bool msgSend, sObject healer, long distressPoint, eTAKEDAMAGE_TYPE type )
{
	unsigned long maxHP  = GetMaxHP();
	unsigned long restHP = mPlayerExrInfo.RestHP;

	if( GetStateDie() == true )
		return;

	if ( maxHP == restHP )
		return;

	/// 실제 적용되는 힐량 계산
	unsigned long applyHP = hp;
	if( mPlayerExrInfo.RestHP + hp > maxHP )
		applyHP = maxHP - mPlayerExrInfo.RestHP;

	/// 실제 적용된 수치만 큼만 어그로 비중 곱해서 넣준다.
	if( healer.index != 0 && healer.type == eOBJECTTYPE_PLAYER && type != eTAKEDAMAGETYPE_NONE )
	{
		HealAttackToMonster( healer.index, applyHP, distressPoint, type );
	}

	unsigned long currentHp = restHP + hp;

	mPlayerExrInfo.RestHP = (currentHp > maxHP) ? maxHP : currentHp;

	if ( msgSend == true )
		SendHPHeal( );
}

bool cPlayer::SendHPHeal( )
{
	MSG_SYN_PLAYER_HP synMsg;
	synMsg.Category			= NM_PLAYER;
	synMsg.Protocol			= NM_PLAYER_HP_SYN;
	synMsg.mCharacterIdx	= GetObjectID();
	synMsg.mHp				= mPlayerExrInfo.RestHP;
	synMsg.mMaxHp			= GetMaxHP();
	synMsg.mDie				= GetStateDie();

	return NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_HP) );
}

void cPlayer::SendMaxHP( )
{
	MSG_SYN_PLAYER_MAXHP synMsg;
	synMsg.Category			= NM_PLAYER;
	synMsg.Protocol			= NM_PLAYER_MAXHP_SYN;
	synMsg.mCharacterIdx	= GetObjectID();
	synMsg.mMaxHp			= GetMaxHP();

	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_MAXHP) );
}

void cPlayer::MPDamage( unsigned long mp, bool msgSend )
{ 
	/// 발동 체크
	SKILLMANAGER->CheckActInfluence( eACTSTATUS_DAMAGE, this, 0 );

	if( mPlayerExrInfo.RestMP < mp )
	{
		mPlayerExrInfo.RestMP = 0;
	}
	else
	{
		mPlayerExrInfo.RestMP -= mp;
	}	

	/// 데미지를 받으면 풀리는 효과들 삭제하기
	if( mDeleteTypeDamagedAry.GetSize() > 0 )
	{
		cLongAry::cIterator begin = mDeleteTypeDamagedAry.Begin();
		cLongAry::cIterator end = mDeleteTypeDamagedAry.End();

		cInfluenceObject* pInf = NULL;
		unsigned long influenceIdx = 0;
		while( begin != end )
		{
			influenceIdx = (*begin++);
			pInf = SKILLMANAGER->GetInfluence( influenceIdx );
			if( pInf == NULL )
				continue;

			if( pInf->GetDeleteType() == eDELETETYPE_PLAYER_DAMAGED )
				SKILLMANAGER->DeleteInfluenceList( influenceIdx );
		}
	}

	if( msgSend == true )
	{
		MSG_SYN_PLAYER_MP synMsg;
		synMsg.Category			= NM_PLAYER;
		synMsg.Protocol			= NM_PLAYER_MP_SYN;
		synMsg.mCharacterIdx	= GetObjectID();
		synMsg.mMp				= mPlayerExrInfo.RestMP;
		synMsg.mMaxMp			= GetMaxMP();

		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
	}
}

void cPlayer::MPUse( unsigned long useValue, bool msgSend )
{
	if( mPlayerExrInfo.RestMP < useValue )
		mPlayerExrInfo.RestMP = 0;
	else
		mPlayerExrInfo.RestMP -= useValue;

	if( msgSend == true )
	{
		MSG_SYN_PLAYER_MP synMsg;
		synMsg.Category			= NM_PLAYER;
		synMsg.Protocol			= NM_PLAYER_MP_SYN;
		synMsg.mCharacterIdx	= GetObjectID();
		synMsg.mMp				= mPlayerExrInfo.RestMP;
		synMsg.mMaxMp			= GetMaxMP();

		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
	}
}

void cPlayer::MPHeal( unsigned long mp, bool msgSend )
{
	unsigned long maxMP  = GetMaxMP();
	unsigned long restMP = mPlayerExrInfo.RestMP;

	if( maxMP == restMP )
		return;

	unsigned long currentMp = restMP + mp;

	mPlayerExrInfo.RestMP = (currentMp > maxMP) ? maxMP : currentMp;

	if ( msgSend != NULL )
		SendMPHeal( );
}

bool cPlayer::SendMPHeal( )
{
	MSG_SYN_PLAYER_MP synMsg;
	synMsg.Category			= NM_PLAYER;
	synMsg.Protocol			= NM_PLAYER_MP_SYN;
	synMsg.mCharacterIdx	= GetObjectID();
	synMsg.mMp				= mPlayerExrInfo.RestMP;
	synMsg.mMaxMp			= GetMaxMP();

	return NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_MP) );
}

void cPlayer::SendMaxMP( )
{
	MSG_SYN_PLAYER_MAXMP synMsg;
	synMsg.Category			= NM_PLAYER;
	synMsg.Protocol			= NM_PLAYER_MAXMP_SYN;
	synMsg.mCharacterIdx	= GetObjectID();
	synMsg.mMaxMp			= GetMaxMP();

	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_MAXMP) );
}

void cPlayer::SetMoveReq( char protocol, NiPoint2 startPos, NiPoint2 destPos, unsigned long clientTime,
						 float range, sObject targetInfo, NiPoint2 targetPos )
{
	NiPoint2 tempDest = destPos;

	mIsMoveMsg = true;
	memset( &mMoveMsg, 0, sizeof(mMoveMsg) );
	mMoveMsg.mMsgError = true;
	mMoveMsg.mProtocol = protocol;

	try
	{
		if( GetState() == eOBJECT_STATE_PUSHPULL || GetState() == ePLAYER_STATE_RUSH )
			throw false;

		if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
			throw false;

		char errorCode = IsMovePossible( startPos, destPos );
		if( errorCode == 1 )
		{
			/// 이동 불가능 지역이면 클라이언트가 서있던 장소로 이동시킨다.
			if( IsClientMovePossible( startPos, destPos ) == false )
			{
				throw false;
			}
			tempDest = startPos;
		}
		else if( errorCode == 2 )
			throw false;

		throw true;
	}
	catch ( bool isMove )
	{
		if( isMove == true )
		{
			SetMoveTargetPos( destPos.x, destPos.y );
			ChangeState( eOBJECT_STATE_MOVE );
			CalcClientMoveDeff( clientTime );
			SetClientMoveReq( tempDest, startPos, false );

			switch( protocol )
			{
			case NM_PLAYER_KEYMOVE_REQ:
			case NM_PLAYER_MOVE_REQ:
				{
					//if( GetCheatHideMode() == false )
					//{
					//	// 다른 플레이어들에게도 알림
					//	MSG_SYN_MOVE Msg;
					//	Msg.Category     = NM_PLAYER;
					//	Msg.Protocol     = NM_PLAYER_MOVE_SYN;
					//	Msg.characterIdx = mObject.index;
					//	Msg.destX        = tempDest.x;
					//	Msg.destY        = tempDest.y;
					//	Msg.moveSpeed    = GetMoveSpeed();
					//	NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
					//}

					mMoveMsg.mMsgError = false;
					mMoveMsg.mProtocol = protocol;
					mMoveMsg.mStartPos = startPos;
					mMoveMsg.mDestPos = destPos;
					mMoveMsg.mRange = range;
					mMoveMsg.mTargetInfo = targetInfo;
					mMoveMsg.mTargetPos = targetPos;
				}
				break;
			case NM_PLAYER_ACTIONMOVE_REQ:
				{
					SetRangeTarget( targetPos.x, targetPos.y, targetInfo, range );

					//if( GetCheatHideMode() == false )
					//{
					//	// 다른 플레이어들에게도 알림
					//	MSG_SYN_ACTIONMOVE Msg;
					//	Msg.Category = NM_PLAYER;
					//	Msg.Protocol = NM_PLAYER_ACTIONMOVE_SYN;
					//	Msg.characterIdx = mObject.index;
					//	Msg.destX = tempDest.x;
					//	Msg.destY = tempDest.y;
					//	Msg.targetX = targetPos.x;
					//	Msg.targetY = targetPos.y;
					//	Msg.range = range;
					//	Msg.targetInfo = targetInfo;
					//	Msg.moveSpeed = GetMoveSpeed();
					//	NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
					//}

					mMoveMsg.mMsgError = false;
					mMoveMsg.mProtocol = protocol;
					mMoveMsg.mStartPos = startPos;
					mMoveMsg.mDestPos = destPos;
					mMoveMsg.mRange = range;
					mMoveMsg.mTargetInfo = targetInfo;
					mMoveMsg.mTargetPos = targetPos;
				}
				break;
			}
		}
		else
		{
			float posX = GetXPos();
			float posY = GetYPos();
			bool possible = AIMANAGER->IsPossible( GetMapNumber(), GetXPos(), GetYPos(), GetObject() );
			if( possible == false )
			{
				float tempX = posX;
				float tempY = posY;
				/// 랜덤범위 50, 100, 150 3단계로 5번씩 갈수있는 지역 체크해서 가능하면 이동시킨다
				for( int i = 0 ; i < 15 ; ++i )
				{
					/// 랜덤폭 - 0~4=50, 5~9=100, 10~14=150
					unsigned long tempRange = 150;
					if( i <= 4 )
						tempRange = 50;
					else if( 5 <= i && i <= 9 )
						tempRange = 100;
					else
						tempRange = 150;

					tempX = GetXPos() + rand() % ( tempRange * 2 ) - tempRange;
					tempY = GetYPos() + rand() % ( tempRange * 2 ) - tempRange;
					possible = AIMANAGER->IsPossible( GetMapNumber(), tempX, tempY, GetObject() );
					if( possible == true )
					{
						posX = tempX;
						posY = tempY;
						break;
					}
				}
			}

			SetPos( posX, posY );
			ClearMoveSyncCalc();
			SetMoveTargetPos( posX, posY );

			destPos.x = posX;
			destPos.y = posY;

			mMoveMsg.mDestPos = destPos;

			//// 070625 PKH 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
			//HANDLE            handle = NULL;
			//MSG_RES_ERR_MOVE* msg    = (MSG_RES_ERR_MOVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_MOVE_RES );
			//msg->destX = posX;
			//msg->destY = posY;
			//NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ERR_MOVE) );
		}
	}
}

void cPlayer::SetMoveTargetPos( float x, float y )
{
	mRange = 0;
	mReadyGotoPos = NiPoint2::ZERO;

	mGotoX = x; mGotoY = y;

	float viewPosX, viewPosY;
	viewPosX = mObjectPos.x - x;
	viewPosY = mObjectPos.y - y;

	mDirection = atan2( viewPosX, viewPosY );

}

char cPlayer::IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos )
{
	/// 이동불가 상태 이상인 경우
	if( IsCantMove() == true )
		return 2;

	/// 이동 메세지 시간차 고려
	mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
	if( mRangeCheck.IsNotRange( startPos, mObjectPos ) == true )
		return 2;

	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_ID_PVP_:
		{
			if( mPvPDMIdx == 0 )	/// gm은 mPvPDMIdx값이 0일수 있다
				return 0;

			cBaseDeathMatch* pDMObject = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
			if( pDMObject == NULL )
				return 0;

			if( pDMObject->IsMovePossible( mObjectPos, goalPos ) == false )
			{
				if( pDMObject->IsMovePossible( startPos, goalPos ) == false )
					return 2;
				else
					return 1;
			}
		}
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			if( mThemeRoomIdx == 0 )	/// gm은 mPvPDMIdx값이 0일수 있다
				return 0;

			cThemeObject* ptheme = THEMEMANAGER->GetThemeObject( GetThemeRoomIdx() );
			if( ptheme == NULL )
				return 0;

			if( ptheme->IsMovePossible( mObjectPos, goalPos ) == false )
			{
				if( ptheme->IsMovePossible( startPos, goalPos ) == false )
					return 2;
				else
					return 1;
			}
		}
		break;
	}

	/// 직선 코스 이동 가능 인지 확인
	/// 직선 불가능 일때 길찾기 해보고 여러 코스가 나오면 서버와 클라가 안맞을 가능성이 크다.
	if( AIMANAGER->IsPossible( mMapNumber, mObjectPos, goalPos, mObject ) == false)
	{
		return 1;
//		if( AIMANAGER->CalcPathCnt( mMapNumber, startPos, goalPos ) > 1 )
//			return 1;
	}

	return 0;
}

bool cPlayer::IsPushPullPossible( NiPoint2 startPos, NiPoint2 goalPos )
{
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_ID_PVP_:
		{
			if( mPvPDMIdx == 0 )	/// gm은 mPvPDMIdx값이 0일수 있다
				return false;

			cBaseDeathMatch* pDMObject = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
			if( pDMObject == NULL )
				return false;

			if( pDMObject->IsMovePossible( startPos, goalPos ) == false )
				return false;
		}
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			if( mThemeRoomIdx == 0 )
				return false;

			cThemeObject* ptheme = THEMEMANAGER->GetThemeObject( GetThemeRoomIdx() );
			if( ptheme == NULL )
				return false;

			if( ptheme->IsMovePossible( startPos, goalPos ) == false )
				return false;
		}
		break;
	}

	/// 직선 코스 이동 가능 인지 확인
	if( AIMANAGER->IsPossible( mMapNumber, startPos, goalPos, mObject ) == false )
		return false;

	return true;
}

bool cPlayer::IsClientMovePossible( NiPoint2 clientPos, NiPoint2 goalPos )
{
	/// 직선 코스 이동 가능 인지 확인
	/// 직선 불가능 일때 길찾기 해보고 여러 코스가 나오면 서버와 클라가 안맞을 가능성이 크다.
	if( AIMANAGER->IsPossible( mMapNumber, mObjectPos, clientPos, mObject ) == false && 
		AIMANAGER->CalcPathCnt( mMapNumber, mObjectPos, clientPos ) != 1 )
	{
		return false;
	}

	/// 클라이언트가 있던 위치로 이동하지만 player update중 원래 이동 목적지가 갈수 있으면 목적지를 바꾼다.
	mReadyGotoPos = goalPos;

	return true;
}

void cPlayer::SetClientMoveReq( NiPoint2 targetPos ,NiPoint2 clientPos, bool key )
{
	mKeyboardMove = key;

	float clientRange = (targetPos - clientPos).Length();
	float serverRange = (targetPos - GetPos()).Length();

	if( serverRange > clientRange )
		mClientMovePlusMinus = 1;
	else if( serverRange < clientRange )
		mClientMovePlusMinus = -1;
	else
		mClientMoveDefTime = 0;
}

void cPlayer::CalcClientMoveDeff( unsigned long endTime )
{
	long serverTime = mServerMoveTime;
	long clientTime = endTime;

	mServerMoveTime = 0;

	if( serverTime != clientTime )
		mClientMoveDefTime += serverTime - clientTime;
}

void cPlayer::CalcFixedObjectSize( short sizePer )
{
	mPlayerExrInfo.mFixedObjectSizePer = 100 + sizePer;
	if( 100 + sizePer < 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::GetFixedObjectSize() sizePer < -100");
		sizePer = -100;
	}

	float objectSize = mPlayerExrInfo.mBaseFixedSize * ( (float)( 100 + sizePer ) / 100 );
	if( objectSize <= 0.0f )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::GetFixedObjectSize() <= 0");
		objectSize = 1.0f;
	}

	mFixedObjectSize = objectSize;
}

void cPlayer::SetChgMonster( unsigned long infObjectIdx, unsigned long monsterIdx )
{
	/// 몬스터 외형 변경
	sMonsterScript* pMScript = MONSTERSCRIPT->GetMonsterListInfo( monsterIdx );
	if( pMScript == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::SetChgMonsterIdx[%d] pMScript == NULL", monsterIdx );
		return;
	}

	if( pMScript->mMonsterScale <= 0.0f )
	{
		NETWORK2->PostServerEvent("cPlayer::SetChgMonsterIdx MonsterScale ERROR - %d", pMScript->mMonsterScale );
		return;
	}

	mPlayerExrInfo.mBaseFixedSize = pMScript->mMonsterFixSize * pMScript->mMonsterScale;

	memset( mChgMonCoolTime, 0, sizeof(mChgMonCoolTime) );

	mChgMonsterInfIdx = infObjectIdx;
	mPlayerExrInfo.mChgMonsterIdx = monsterIdx;

	MSG_SYN_PLAYER_CHG_MONSTER synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_CHG_MONSTER_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mMonsterIdx = mPlayerExrInfo.mChgMonsterIdx;
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );

	/// 변신 관련 퀘스트체크
	SendEventQuestList( eQUEST_TRANSMON );
}

void cPlayer::EndChgMonster()
{
	/// 몬스터 외형 꺼짐
	mPlayerExrInfo.mBaseFixedSize = FIXED_OBJECT_SIZE;

	mChgMonsterInfIdx = 0;
	mPlayerExrInfo.mChgMonsterIdx = 0;

	MSG_SYN_PLAYER_CHG_MONSTER synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_CHG_MONSTER_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mMonsterIdx = mPlayerExrInfo.mChgMonsterIdx;
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );

	/// 변신 관련 퀘스트체크
	SendEventQuestList( eQUEST_TRANSMON );
}

bool cPlayer::MonsterPrecedeAttackPass()
{
	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
		return false;

	sMonsterScript* pScript = MONSTERSCRIPT->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
	if( pScript == NULL )
		return false;

	if( pScript->mIsChgMonCheck == false )
		return false;

	return true;
}

void cPlayer::SetVehicle( unsigned long infObjectIdx, unsigned long vehicleIdx )
{
	/// 탈것 변경
	sVehicleList* pScript = VEHICLESCRIPT->GetVehicleInfo( vehicleIdx );
	if( pScript == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::SetVehicle[%d,%d] pScript == NULL", mObject.index, vehicleIdx );
		return;
	}

	mPlayerExrInfo.mBaseFixedSize = pScript->mObjectFixSize; 

	mVehicleInfIdx = infObjectIdx;
	mPlayerExrInfo.mVehicleIdx = vehicleIdx; 

	MSG_SYN_PLAYER_VEHICLE synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_VEHICLE_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mVehicleIdx = mPlayerExrInfo.mVehicleIdx;
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
}

void cPlayer::ArightVehicle( bool sendMsg, bool sendHero )
{
	/// 탈것 꺼짐
	mPlayerExrInfo.mBaseFixedSize = FIXED_OBJECT_SIZE;
	mVehicleInfIdx = 0;
	mPlayerExrInfo.mVehicleIdx = 0;

	if( sendMsg == false )
		return;

	MSG_SYN_PLAYER_VEHICLE synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_VEHICLE_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mVehicleIdx = mPlayerExrInfo.mVehicleIdx;

	if( sendHero == true )
		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
	else
		NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
}

void cPlayer::EndVehicleClient( bool sendSync )
{
	/// 프로세스 순서 확인하자!
	if( mVehicleInfIdx != 0 )
		SKILLMANAGER->VehicleDeleteInfluence( mObject, mVehicleInfIdx, sendSync );
}

void cPlayer::EndVehicleItem( unsigned long skillClassIdx )
{
	sPlayerSkillBaseInfo* pSkill = SKILLSCRIPT->GetPlayerSkillInfo( skillClassIdx );
	if( pSkill == NULL )
		return;

	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pSkill->mpSetpInfoArray[0].mInfulenceIdx );
	if( pScript == NULL )
		return;

	if( mPlayerExrInfo.mVehicleIdx == 0 )
		return;

	if( pScript->mStatusPlusIdx1 == eSTATUSPLUS_VEHICLE )
	{
		if( (long)mPlayerExrInfo.mVehicleIdx != pScript->mValue1 )
			return;

		if( mVehicleInfIdx != 0 )
			SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
	}
	else if( pScript->mStatusPlusIdx2 == eSTATUSPLUS_VEHICLE )
	{
		if( (long)mPlayerExrInfo.mVehicleIdx != pScript->mValue2 )
			return;

		if( mVehicleInfIdx != 0 )
			SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
	}
	else if( pScript->mStatusPlusIdx3 == eSTATUSPLUS_VEHICLE )
	{
		if( (long)mPlayerExrInfo.mVehicleIdx != pScript->mValue3 )
			return;

		if( mVehicleInfIdx != 0 )
			SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
	}
}


void cPlayer::SetSkillPushPull( unsigned short type, cBaseObject* pAttacker, unsigned long range )
{
	if( pAttacker == NULL )
		return;

	if( IsChangeState( eOBJECT_STATE_PUSHPULL ) == false )
		return;

	NiPoint2 attackerPos = pAttacker->GetPos();

	if( type == 0 )
		return;

	/// 공격자에서 타겟(현재오브젝트)의 방향산출
	NiPoint2 dir;
	dir.x = mObjectPos.x - attackerPos.x;
	dir.y = mObjectPos.y - attackerPos.y;
	dir.Unitize();

	float fixRange = 0.0f;
	NiPoint2 targetPos = mObjectPos;
	if( type == eSTATUSPLUS_AROUND_PUSH )
	{
		fixRange = OBJECTMANAGER->ObjectSizeRange( this, pAttacker, (float)range );

		if( (mObjectPos - attackerPos).Length() > fixRange )
			return;

		targetPos = attackerPos + dir * fixRange;
	}
	else if( type == eSTATUSPLUS_AROUND_PULL )
	{
		fixRange = OBJECTMANAGER->ObjectSizeRange( this, pAttacker, 100.0f );

		if( (mObjectPos - attackerPos).Length() < fixRange )
			return;
		
		targetPos = attackerPos + dir * fixRange;
	}
	else if( type == eSTATUSPLUS_TARGET_PUSH )
	{
		targetPos = mObjectPos + dir * (float)range;
	}

	NiPoint2 destPos = mObjectPos;

	/// 목적지까지의 방향정보
	NiPoint2 divideDir = targetPos - mObjectPos;
	float moveRange = divideDir.Unitize();

	/// 분할 숫자
	unsigned long divideCnt = 10;

	/// 이동거리를 분할
	float divideRange = moveRange / (float)divideCnt;
	for( unsigned long i = 0 ; i < divideCnt ; ++i )	/// 먼저 선출된 끝점부터 공격자에 가까운 위치로 체크
	{
		/// 목적지에서 분할한 거리만큼 빼가면서 이동가능한지 체크후 가능하면 그위치로 이동한다.
		NiPoint2 pos = mObjectPos + ( divideDir * ( moveRange - ( divideRange * i ) ) );
		if( IsPushPullPossible( mObjectPos, pos ) == true )
		{
			destPos = pos;
			break;
		}
	}

	// 채집 취소
	if( GetState() == ePLAYER_STATE_GATHERING )
	{
		cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
		if( pGathering != NULL )
			pGathering->CancelGatherByOddity( this );
	}
	else if( GetState() == eOBJECT_STATE_STOP )
	{
		// 강화 취소
		if( GetStateStop() == eSTOP_ENHANCED )
		{
			ItemEnhancedForceCancel();
		}
		// 조합 취소
		else if( GetStateStop() == eSTOP_ITEMMIX )
		{
			ItemMixForceCancel();
		}
	}
	else if( GetState() == eOBJECT_STATE_ATTACK )
	{
		SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
	}

	MSG_SYN_SKLL_PUSHPULL sync;
	sync.Category = NM_SKILL;
	sync.Protocol = NM_SKILL_PUSHPULL_SYN;
	sync.mTarget = mObject;
	sync.mPosX = destPos.x;
	sync.mPosY = destPos.y;
	NETWORK2->QuickSend( this, (char*)&sync, sizeof(sync) );

	mGotoX = destPos.x;
	mGotoY = destPos.y;
	ChangeState( eOBJECT_STATE_PUSHPULL );


}


NiPoint2 cPlayer::SetSkillRush( cBaseObject* pTarget, bool blink, unsigned long skillIdx, unsigned char skillStep )
{
	if( pTarget == NULL )
		return GetPos();

	sObject target = pTarget->GetObject();
	SetMoveTargetPos( pTarget->GetXPos(), pTarget->GetYPos() );
	SetRangeTarget( pTarget->GetXPos(), pTarget->GetYPos(), target, 100 );

	mCheckRushSkillIdx = skillIdx;
	mCheckRushSkillStep = skillStep;

	if( blink == true )
	{
		NiPoint2 destPos = mObjectPos;

		/// 목적지까지의 방향정보
		NiPoint2 divideDir = pTarget->GetPos() - mObjectPos;
		divideDir.Unitize();

		float fixRange = OBJECTMANAGER->ObjectSizeRange( pTarget, this, 100 );

		float moveRange = (mObjectPos - pTarget->GetPos()).Length();
		if( moveRange < fixRange )
			return GetPos();
		
		moveRange = moveRange - fixRange;

		/// 분할 숫자
		unsigned long divideCnt = 10;

		/// 이동거리를 분할
		float divideRange = moveRange / (float)divideCnt;
		for( unsigned long i = 0 ; i < divideCnt ; ++i )	/// 먼저 선출된 끝점부터 공격자에 가까운 위치로 체크
		{
			/// 목적지에서 분할한 거리만큼 빼가면서 이동가능한지 체크후 가능하면 그위치로 이동한다.
			NiPoint2 pos = mObjectPos + ( divideDir * ( moveRange - ( divideRange * i ) ) );
			if( IsPushPullPossible( mObjectPos, pos ) == true )
			{
				destPos = pos;
				break;
			}
		}

		if( ChangeState( eOBJECT_STATE_IDLE ) == false )
			return GetPos();
		
		SetPos( destPos.x, destPos.y );
		MoveStop();
	}
	else
	{
		if( ChangeState( ePLAYER_STATE_RUSH ) == false )
			return GetPos();

		return pTarget->GetPos();
	}

	return GetPos();
}


void cPlayer::SetDoorPushPull( sDoorInfo* pInfo )
{
	if( IsChangeState( eOBJECT_STATE_PUSHPULL ) == false )
		return;

	if( STAGESCRIPT->InsidePolygon( pInfo->mRectPos, 4, mObjectPos ) != 0 )
		return;

	/// 공격자 방향
	NiPoint2 dir = NiPoint2( -sinf(mDirection), -cosf(mDirection) );
	dir.Unitize();

	NiPoint2 targetPos = mObjectPos + dir * 350;		/// 예상 최종 목적지
	NiPoint2 destPos = mObjectPos;						/// 시작위치부터 점진적으로 진행되볼 위치

	/// 목적지까지의 방향정보
	float moveRange = (targetPos - mObjectPos).Length();

	/// 분할 숫자
	unsigned long divideCnt = 10;

	/// 이동거리를 분할
	float divideRange = moveRange / (float)divideCnt;
	for( unsigned long i = 0 ; i < divideCnt ; ++i )	/// 먼저 선출된 끝점부터 공격자에 가까운 위치로 체크
	{
		/// 목적지에서 분할한 거리만큼 빼가면서 이동가능한지 체크후 가능하면 그위치로 이동한다.
		NiPoint2 pos = mObjectPos + ( dir * ( moveRange - ( divideRange * i ) ) );
		if( IsPushPullPossible( mObjectPos, pos ) == true )
		{
			destPos = pos;
			break;
		}
	}

	// 채집 취소
	if( GetState() == ePLAYER_STATE_GATHERING )
	{
		cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
		if( pGathering != NULL )
			pGathering->CancelGatherByOddity( this );
	}
	else if( GetState() == eOBJECT_STATE_STOP )
	{
		// 강화 취소
		if( GetStateStop() == eSTOP_ENHANCED )
		{
			ItemEnhancedForceCancel();
		}
		// 조합 취소
		else if( GetStateStop() == eSTOP_ITEMMIX )
		{
			ItemMixForceCancel();
		}
	}
	else if( GetState() == eOBJECT_STATE_ATTACK )
	{
		SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
	}

	MSG_SYN_PLAYEREXT_DOORPUSH sync;
	sync.Category = NM_PLAYEREXT;
	sync.Protocol = NM_PLAYEREXT_DOORPUSH_SYN;
	sync.mTarget = mObject;
	sync.mPosX = destPos.x;
	sync.mPosY = destPos.y;
	NETWORK2->QuickSend( this, (char*)&sync, sizeof(sync) );

	mGotoX = destPos.x;
	mGotoY = destPos.y;
	ChangeState( eOBJECT_STATE_PUSHPULL );
}


void cPlayer::Jump()
{
	/// 점프 중인지 체크 & 점프 불가능 상태인지 체크
	if( mJumpEndTime > NETWORK2->GetAccumTime() || IsChangeState( eOBJECT_STATE_MOVE ) == false )
	{
		NETWORK2->SendMsgRoot( mConnectionIdx, NM_PLAYER, NM_PLAYER_JUMP_RES );
		return;
	}

	if( mPlayerExrInfo.mState == ePLAYER_STATE_SITDOWN )
		ChangeState( eOBJECT_STATE_IDLE );

	MSG_SYN_PLAYER_JUMP msg;
	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_JUMP_SYN;
	msg.mPlayerIdx = mObject.index;
	NETWORK2->QuickSend( this, (char*)&msg, sizeof(msg) );

	unsigned long jumpTime = SYNC_MOVE_TIME;
	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
		jumpTime = mJumpTime;	/// 직업에 맞춰스크립트에서 읽어온 값
	else
		jumpTime = PLAYER_JUMPTIME_CHGMONSTER;	/// 몬스터점프 수치는 고정값	

	if( jumpTime < SYNC_MOVE_TIME )
		jumpTime = 0;
	else
		jumpTime -= SYNC_MOVE_TIME;

	mJumpEndTime = NETWORK2->GetAccumTime() + jumpTime;

	if( GetState() != eOBJECT_STATE_MOVE )
		return;

	mJumpSpeedState = 1;

	/// 전체적으로 가장 많이 분포한 점프 시작 정지모션 시간
	mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 134;
	/// 탈것 인경우
	if( mVehicleInfIdx != 0 )
	{
		mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 167;
	}
	else if( mPlayerExrInfo.mChgMonsterIdx != 0 )
	{
		mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 134;
	}
	else
	{
		/// 종족 별로 시간설정
		switch ( mPlayerInfo.Race )
		{
		case eRACE_HUMAN: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 67; break;
		case eRACE_BEAST: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 134; break;
		case eRACE_ELF: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 134; break;
		}
	}
}

void cPlayer::SetJumpTime()
{
	mJumpTime = (unsigned long)STATUSSCRIPT->GetStatusExtBaseInfo( ePLAYER_STATUS_EXT_JUMP, mPlayerInfo.Job ) ;
}

// SearchThemeUser Method.
PerThemeUser* cPlayer::SearchThemeUser(long cid)
{
	return THEMEUSERPOOL->SearchThemeUser( &mThemeUserRoot, cid );
}

// IsThemeUser Method.
bool cPlayer::IsThemeUser(long cid)
{
	return THEMEUSERPOOL->IsThemeUser( &mThemeUserRoot, cid );
}

// AddThemeUser Method.
void cPlayer::AddThemeUser(THEME_USER* table, long rowCount)
{
	for( int i = 0 ; i < rowCount ; ++i, ++table )
	{
		PerThemeUser* perThemeUser = THEMEUSERPOOL->GetThemeUser( &mThemeUserRoot, MAKETHEMECID( table->themeIdx, table->themeMode ) );
		if ( perThemeUser != NULL )
		{
			perThemeUser->themeIdx  = table->themeIdx;
			perThemeUser->roomIdx   = table->roomIdx;
			perThemeUser->mode      = table->themeMode;						// 모드
			perThemeUser->lastDate  = timestamp2time_t( table->lastDate );	// 마지막 업데이트(일)
			perThemeUser->validThru = timestamp2time_t( table->validThru );	// 유효만기(일)

			cThemeObject* ptheme = THEMEMANAGER->GetThemeObject( table->roomIdx );
			if( ptheme != NULL )
				ptheme->Dependency();
		}
	}

	SendThemeInfo();
}

// SaveThemeUser Method.
bool cPlayer::SaveThemeUser(long cid)
{
	if ( IsThemeUser( cid ) == false )
	{
		NETWORK2->SendMsgRoot( mConnectionIdx, NM_THEME, NM_THEME_DEPENDENCY_SYN );

		HANDLE           handle = NULL;
		THEME_USER_SAVE* save   = (THEME_USER_SAVE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_SAVE );
		save->themeIdx     = HIWORD( cid );
		save->themeMode    = (unsigned char)(LOWORD( cid ));
		save->roomIdx      = mMapNumber;
		save->characterIdx = mObject.index;
		return NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(THEME_USER_SAVE) );
	}
	return false;
}


// SendThemeInfo Method.
void cPlayer::SendThemeInfo( )
{
	if( mGameIn == false )
		return;

	PerThemeUser* perThemeUser = (PerThemeUser*)mThemeUserRoot.pool;

	HANDLE              handle  = NULL;
	MSG_SYN_THEME_LIST* sendMsg = (MSG_SYN_THEME_LIST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_THEME, NM_THEME_HAVE_LIST_SYN );
	if ( sendMsg != NULL )
	{
		sPlayerThemeList* themeList = sendMsg->mThemeList;
		unsigned long&    rowCount  = sendMsg->mCount;

		if ( perThemeUser != NULL )
		{
			time_t ltime;
			long   result;

			time( &ltime );
			while( perThemeUser != NULL )
			{
				result = (long)difftime( perThemeUser->validThru, ltime );
				if ( result > 0 )
				{
					themeList->mMapNumber     = HIWORD( perThemeUser->cid );
					themeList->mThemeMode     = (unsigned char)LOWORD( perThemeUser->cid );
					themeList->mUniqueIdx     = MAKETHEMECID( perThemeUser->themeIdx, perThemeUser->roomIdx );
					themeList->mResetLeftTime = result*SECOND;
					themeList++;
					rowCount++;
				}
				perThemeUser = (PerThemeUser*)perThemeUser->next;
			}
		}
		else
		{
			themeList->mMapNumber     = 0;
			themeList->mThemeMode     = 0;
			themeList->mUniqueIdx     = 0;
			themeList->mResetLeftTime = 0;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
	}

}

void cPlayer::SendThemeExpireTime()
{
	long result = THEMEMANAGER->NextResetTime() - NETWORK2->GetAccumTime();
	if ( result > 0 )
	{
		HANDLE                     handle  = NULL;
		MSG_SYN_THEME_EXPIRE_TIME* sendMsg = (MSG_SYN_THEME_EXPIRE_TIME*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_THEME, NM_THEME_EXPIRE_TIME_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mExpireTime = result;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_THEME_EXPIRE_TIME) );
		}
	}
}


void cPlayer::AddImmuneApplyType( unsigned char InfluenceType, unsigned short applyType )
{ 
	SKILLMANAGER->ClearImmuneApplyType( mObject, InfluenceType, applyType );

	if( InfluenceType == eINFLUENCETYPE_BUF ) 
	{
		mImmuneBuffSet.Insert( applyType );
	}
	else if( InfluenceType == eINFLUENCETYPE_DEBUF ) 
	{
		mImmuneDebuffSet.Insert( applyType ); 
	}
	else
		assert(0);
}


void cPlayer::AddImmuneKind( unsigned long infKind )
{ 
	SKILLMANAGER->ClearImmuneKind( mObject, infKind );
	mImmuneKindSet.Insert( infKind );	
}


bool cPlayer::IsImmuneApplyType( unsigned char InfluenceType, unsigned long applyType )
{
	if( applyType == 0 )
		return false;

	/// 면역 목록에서 찾아봐서 있으면 면역
	if( InfluenceType == eINFLUENCETYPE_BUF )
	{
		cHashSet::cIterator i = mImmuneBuffSet.Find( applyType );
		if( i != mImmuneBuffSet.End() )
			return true;
	}
	else if( InfluenceType == eINFLUENCETYPE_DEBUF )
	{
		cHashSet::cIterator i = mImmuneDebuffSet.Find( applyType );
		if( i != mImmuneDebuffSet.End() )
			return true;
	}
	return false;
}

bool cPlayer::IsImmuneApplyAll( unsigned long applyType )
{
	if( applyType == 0 )
		return false;

	/// 면역 목록에서 찾아봐서 있으면 면역
	cHashSet::cIterator i;
	i = mImmuneBuffSet.Find( applyType );
	if( i != mImmuneBuffSet.End() )
		return true;
	
	i = mImmuneDebuffSet.Find( applyType );
	if( i != mImmuneDebuffSet.End() )
		return true;
	return false;
}


bool cPlayer::IsImmuneKind( unsigned long infKind )
{
	if( infKind == 0 )
		return false;

	/// 면역 목록에서 찾아봐서 있으면 면역
	cHashSet::cIterator b = mImmuneKindSet.Find( infKind );
	cHashSet::cIterator e = mImmuneKindSet.End();

	if( b != e )
		return true;

	return false;
	
}


void cPlayer::SetUserPortal( unsigned long userPortalIdx )
{
	if( mUserPortalIdx == 0 )
	{
		mUserPortalIdx = userPortalIdx;
	}
	else
	{
		/// 이미 생성되 있는경우 기존것을지우고 새로만든다.
		OBJECTMANAGER->InsertDeleteUserPortal( mUserPortalIdx );
		mUserPortalIdx = userPortalIdx;
	}
}

unsigned long cPlayer::CalcChangeMPDamage( unsigned char attributeType, unsigned long damage, bool applyHP )
{
	unsigned long leftDamage = damage;
	unsigned long calcMP = GetMP();

	if( applyHP == true )
	{
		for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mChangeMPDamageAry[i] );
			if( pInf == NULL || pInf->IsDelReady() == true )
				continue;

			/// 속성확인
			if( ( attributeType == eATTRIBUTETYPE_PHYSICAL && ( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_PHY || pInf->GetChangeMPType() == eINFCHANGEMPTYPE_ALL ) ) ||
				( attributeType == eATTRIBUTETYPE_MAGIC && ( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_MAG || pInf->GetChangeMPType() == eINFCHANGEMPTYPE_ALL ) ) )
			{
				/// 효과에서 데미지mp변환 수치 계산
				unsigned long cutMP = (unsigned long)( GetMaxMP() * pInf->GetChangeMPValue() * 0.01f );
				cutMP = GetMaxMP() - cutMP;

				/// 계산된 현재mp가 효과제한값보다 큰경우만 계산
				if( calcMP > cutMP )
				{
					/// 데미지 적용값이 효과 커트라인 넘는지 체크
					if( cutMP + leftDamage < calcMP  )
					{
						/// 넘지 않는경우 데미지 만큼 계산된현재mp에 적용
						calcMP = calcMP - leftDamage;
						leftDamage = 0;
					}
					else
					{
						/// 넘는 경우 효과데미지mp변환 값까지만 적용
						leftDamage = leftDamage - ( calcMP - cutMP );
						calcMP = cutMP;

						/// 해당 효과는 지운다.
						SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
					}
				}
				else
				{
					/// 이미 넘는 경우이므로 효과는 지운다.
					SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
				}
			}
		}
	}
	else	/// mp데미지 이므로 데미지는 전부다 적용하고 허용치 밑으로내려간 효과만 전부 지워버린다.
	{
		calcMP = GetMP() - damage;
		leftDamage = damage;

		for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mChangeMPDamageAry[i] );
			if( pInf == NULL || pInf->IsDelReady() == true )
				continue;

			/// 속성확인
			if( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_NONE )
				continue;

			/// 효과에서 데미지mp변환 수치 계산
			unsigned long cutMP = (unsigned long)( GetMaxMP() * pInf->GetChangeMPValue() * 0.01f );
			cutMP = GetMaxMP() - cutMP;
			
			if( calcMP <= cutMP )
				SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
		}
	}

	return leftDamage;
}


unsigned long cPlayer::CalcGuard( unsigned char attributeType, unsigned long damage )
{
	unsigned long tempDamage = damage;
	for( unsigned long i = 0 ; i < mGuardAry.GetSize() ; ++i )
	{
		cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mGuardAry[i] );
		if( pInf == NULL || pInf->IsDelReady() == true )
			continue;

		if( pInf->CalcLeftGuard( attributeType, &tempDamage ) == false )
			continue;

		if( pInf->GetLeftGuardValue() == 0 || pInf->GetLeftGuardCnt() == 0)
			SKILLMANAGER->DeleteInfluenceList( mGuardAry[i] );
		else
		{
			HANDLE handle = NULL;
			MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE* pMsg = (MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE*)NETWORK2->GetMsgRoot( &handle
																												   ,mConnectionIdx
																												   ,NM_SKILL
																												   ,NM_SKILL_INFLUENCE_GUARDCHANGE_SYN );
			if ( pMsg != NULL )
			{
				pMsg->mInfluenceIdx = pInf->GetUniqueIdx();
				pMsg->mChangeValue = pInf->GetLeftGuardValue();
				pMsg->mChangeCnt = pInf->GetLeftGuardCnt();
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE) );
			}
		}

		if( tempDamage != 0 )
			continue;

		return 0;
	}

	return tempDamage;
}


bool cPlayer::CalcCashExpGuard( unsigned long* pInfIdx, unsigned char* leftCnt )
{
	if( pInfIdx == NULL || leftCnt == NULL )
		return false;

	for( unsigned long i = 0 ; i < mGuardAry.GetSize() ; ++i )
	{
		cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mGuardAry[i] );
		if( pInf == NULL || pInf->IsDelReady() == true )
			continue;

		if( pInf->CalcCashExpGuard() == false )
			continue;

		if( pInf->GetLeftGuardCnt() == 0 )
			SKILLMANAGER->DeleteInfluenceList( mGuardAry[i] );

		*pInfIdx = mGuardAry[i];
		*leftCnt = pInf->GetLeftGuardCnt();
		return true;
	}

	return false;
}


void cPlayer::CalcDirection( float targetX, float targetY )
{
	float viewPosX, viewPosY;
	viewPosX = mObjectPos.x - targetX;
	viewPosY = mObjectPos.y - targetY;

	mDirection = atan2( viewPosX, viewPosY );
}


void cPlayer::SetForceType( unsigned char forceType, bool msgSend )
{
	mPlayerExrInfo.mForceType = forceType;

	if( msgSend == true )
	{
		MSG_SYN_PLAYER_FORCE msg;
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_FORCE_SYN;
		msg.mPlayerIdx = mObject.index;
		msg.mForceType = forceType;
		NETWORK2->QuickSend( this, (char*)&msg, sizeof(msg) );
	}

	PostCharacterEvent(	EVENT_CHARACTER_UPDATE, "SetForceType=%d", forceType );
}

unsigned long cPlayer::WindPointPlus( unsigned long windPoint, bool msgSend )
{
	mHeroInfo.mWindPoint += windPoint;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_WIND;
			pMsg->mValueType = 1;
			pMsg->mValue = mHeroInfo.mWindPoint;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mWindPoint;
}

unsigned long cPlayer::WindFriendlyPlus( unsigned long windFriendly, bool msgSend )
{
	mHeroInfo.mWindFriendly += windFriendly;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_WIND;
			pMsg->mValueType = 2;
			pMsg->mValue = mHeroInfo.mWindFriendly;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mWindFriendly;
}

unsigned long cPlayer::EarthPointPlus( unsigned long earthPoint, bool msgSend )
{
	mHeroInfo.mEarthPoint += earthPoint;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_EARTH;
			pMsg->mValueType = 1;
			pMsg->mValue = mHeroInfo.mEarthPoint;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mEarthPoint;
}

unsigned long cPlayer::EarthFriendlyPlus( unsigned long earthFriendly, bool msgSend )
{
	mHeroInfo.mEarthFriendly += earthFriendly;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_EARTH;
			pMsg->mValueType = 2;
			pMsg->mValue = mHeroInfo.mEarthFriendly;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mEarthFriendly;
}

unsigned long cPlayer::WaterPointPlus( unsigned long waterPoint, bool msgSend )
{
	mHeroInfo.mWaterPoint += waterPoint;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_WATER;
			pMsg->mValueType = 1;
			pMsg->mValue = mHeroInfo.mWaterPoint;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mWaterPoint;
}

unsigned long cPlayer::WaterFriendlyPlus( unsigned long waterFriendly, bool msgSend )
{
	mHeroInfo.mWaterFriendly += waterFriendly;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_WATER;
			pMsg->mValueType = 2;
			pMsg->mValue = mHeroInfo.mWaterFriendly;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mWaterFriendly;
}

unsigned long cPlayer::FirePointPlus( unsigned long firePoint, bool msgSend )
{
	mHeroInfo.mFirePoint += firePoint;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_FIRE;
			pMsg->mValueType = 1;
			pMsg->mValue = mHeroInfo.mFirePoint;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mFirePoint;
}

unsigned long cPlayer::FireFriendlyPlus( unsigned long fireFriendly, bool msgSend )
{
	mHeroInfo.mFireFriendly += fireFriendly;

	if( msgSend == true )
	{
		HANDLE handle = NULL;
		MSG_SYN_PLAYER_FORCEVALUE* pMsg = (MSG_SYN_PLAYER_FORCEVALUE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_FORCEVALUE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mForceType = eFORCETYPE_FIRE;
			pMsg->mValueType = 2;
			pMsg->mValue = mHeroInfo.mFireFriendly;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_FORCEVALUE) );
		}
	}

	return mHeroInfo.mFireFriendly;
}

void cPlayer::ForcePointDBUpdate()
{
	HANDLE handle = NULL;
	FORCE_POINT_UPDATE* pPoint = (FORCE_POINT_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_FORCEPOINT_UPDATE );
	pPoint->mCharacterIdx = mObject.index;
	pPoint->mFirePoint = mHeroInfo.mFirePoint;
	pPoint->mFireFriendly = mHeroInfo.mFireFriendly;
	pPoint->mWaterPoint = mHeroInfo.mWaterPoint;
	pPoint->mWaterFriendly = mHeroInfo.mWaterFriendly;
	pPoint->mWindPoint = mHeroInfo.mWindPoint;
	pPoint->mWindFriendly = mHeroInfo.mWindFriendly;
	pPoint->mEarthPoint = mHeroInfo.mEarthPoint;
	pPoint->mEarthFriendly = mHeroInfo.mEarthFriendly;
	pPoint->mRetValue = -1;

	NETWORK2->SendSQL( handle, sizeof(FORCE_POINT_UPDATE) );
}

void cPlayer::SetStatus1( ePLAYER_STATUS statusBase, float value )
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

/// 070116 PKH 기본 스텟 추가 수치 등록
void cPlayer::SetStatus1Plus( ePLAYER_STATUS statusBasePlus, float value )
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

void cPlayer::SetStatus1Per( ePLAYER_STATUS statusBasePer, float value )
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

float cPlayer::GetStatus2Plus( ePLAYER_STATUS_EXT extStatus )
{
	if( extStatus >= ePLAYER_STATUS_EXT2_MAX )
	{
		assert(NULL);
		return 0.0f;
	}

	return mStatus2Plus[extStatus];
}

float cPlayer::GetStatus2Per( ePLAYER_STATUS_EXT extStatus )
{
	if( extStatus >= ePLAYER_STATUS_EXT2_MAX )
	{
		assert(NULL);
		return 0.0f;
	}

	return mStatus2Per[extStatus];
}

void cPlayer::SetStatus2( ePLAYER_STATUS_EXT extStatus, float value )
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
		mStatus2.mPhysicDefense = value;
		break;

	case ePLAYER_STATUS_EXT_MATK:
		break;	/// 최소 최대 값으로 재정의

	case ePLAYER_STATUS_EXT_MDEF:
		mStatus2.mMagicDefense = value; break;
	case ePLAYER_STATUS_EXT_CRI:
		mStatus2.mPhysicCritical = value; break;
	case ePLAYER_STATUS_EXT_MCRI:
		mStatus2.mMagicCritical = value; break;
	case ePLAYER_STATUS_EXT_MAXHP:
		{
			mPlayerExrInfo.MaxHP = FloatToInt( value );

			if( mPlayerExrInfo.MaxHP == 0 )
				assert(0);
		}
		break;
	case ePLAYER_STATUS_EXT_MAXMP:
		{
			mPlayerExrInfo.MaxMP = FloatToInt( value );

			if( mPlayerExrInfo.MaxMP == 0 )
				assert(0);
		}
		break;
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
		mPlayerExrInfo.mAttackSpeed = value;
		mStatus2.mAttackSpeed = (float)value; break;
	case ePLAYER_STATUS_EXT_COOLTIME:
		mStatus2.mCooltime = (float)value; break;
	case ePLAYER_STATUS_EXT_RANGE:
		mStatus2.mInRange = value; break;
	case ePLAYER_STATUS_EXT_RRANGE:
		mStatus2.mOutRange = value; break;
	case ePLAYER_STATUS_EXT_SPD:
		mPlayerExrInfo.mMoveSpeed = FloatToInt( value );
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

void cPlayer::SetStatusEtc( ePLAYER_STATUS_EXT_ADD statusAdd, float value )
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
	case ePLAYER_STATUS_EXT_ADD_MONEYPER:
		mStatusEtc.mMoneyAddPer = value; break;
	case ePLAYER_STATUS_EXT_ADD_PVPPOINTPER:
		mStatusEtc.mPvPPointAddPer = value; break;
	}
}

//void cPlayer::SetDamageCalc( ePLAYER_STATUS_DAMAGE statusDamage, unsigned long value )
//{
//	if( statusDamage >= ePLAYER_STATUS_DAMAGE_MAX )
//	{
//		assert(NULL);
//		return;
//	}
//
//	switch( statusDamage )
//	{
//	case ePLAYER_STATUS_DAMAGE_INSTANTPERHIT:
//		mDamageCalc.mInstantPerHit = value; break;
//	case ePLAYER_STATUS_DAMAGE_INSTANTPLUSHIT:
//		mDamageCalc.mInstantPlusHit = value; break;
//	case ePLAYER_STATUS_DAMAGE_PHYINSTANTPERCRI:
//		mDamageCalc.mPhyInstantPerCri = value; break;
//	case ePLAYER_STATUS_DAMAGE_PHYINSTANTPLUSCRI:
//		mDamageCalc.mPhyInstantPlusCri = value; break;
//	case ePLAYER_STATUS_DAMAGE_MAGICINSTANTPERCRI:
//		mDamageCalc.mMagicInstantPerCri = value; break;
//	case ePLAYER_STATUS_DAMAGE_MAGICINSTANTPLUSCRI:
//		mDamageCalc.mMagicInstantPlusCri = value; break;
//	case ePLAYER_STATUS_DAMAGE_PHYINSTANTPERATTACK:
//		mDamageCalc.mPhyInstantPerAttack = value; break;
//	case ePLAYER_STATUS_DAMAGE_PHYINSTANTPLUSATTACK:
//		mDamageCalc.mPhyInstantPlusAttack = value; break;
//	case ePLAYER_STATUS_DAMAGE_MAGICINSTANTPERATTACK:
//		mDamageCalc.mMagicInstantPerAttack = value; break;
//	case ePLAYER_STATUS_DAMAGE_MAGICINSTANTPLUSATTACK:
//		mDamageCalc.mMagicInstantPlusAttack = value; break;
//	case ePLAYER_STATUS_DAMAGE_INSTANTPERFINAL:
//		mDamageCalc.mInstantPerFinal = value; break;
//	case ePLAYER_STATUS_DAMAGE_INSTANTPLUSFINAL:
//		mDamageCalc.mInstantPlusFinal = value; break;
//	}
//
//}

unsigned long cPlayer::DamageCalc( sObject target, unsigned long /*skillIdx*/, unsigned long attributeType, unsigned long rangeType,
								  unsigned long skillDamage, unsigned long skillDamagePer,bool isIgnoreDefence, float addTotalDamage, float addPerTotalDamage )
{
	//return DAMAGECALC->PlayerDamageCalc( mObject.index, target, (eATTRIBUTETYPE)attributeType, (eRANGETYPE)rType );

	unsigned long damage = 0;

	//// 오토더미 예외처리
	//if( target.type == eOBJECTTYPE_MONSTER )
	//{
	//	cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
	//	if( pMonster != NULL )
	//	{
	//		if( pMonster->IsAutoDummy() == true )
	//			return 1;
	//	}
	//	else
	//		NETWORK2->PostServerEvent("cPlayer::DamageCalc pMonster == NULL [%d,%d]", target.type, target.index );
	//}

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		unsigned long minAttack = 0;
		unsigned long maxAttack = 0;

		/// 근접 / 원거리 공격력 구분
		if( rangeType == eRANGETYPE_CLOSE )
		{
			/// 플레이어의 공격력 최소/최대값을 읽어옴
			minAttack = FloatToInt( GetStatus2()->mPhysicMinAttack );
			maxAttack = FloatToInt( GetStatus2()->mPhysicMaxAttack );
		}
		else if( rangeType == eRANGETYPE_LONG )
		{
			minAttack = FloatToInt( GetStatus2()->mPhysicMinRangeAttack );
			maxAttack = FloatToInt( GetStatus2()->mPhysicMaxRangeAttack );
		}
		else
		{
			NETWORK2->PostServerEvent("cDamageCalc::PlayerDamageCalc rangeType[%d,%d,%d,%d] == ERROR", rangeType,
				mObject.index, GetRace(), GetRaceGender() );
			return 0;
		}

		/// 변신중이고 무기를 착용하고 있으면 무기정보를 따라 공격력을 결정한다.
		if( mPlayerExrInfo.mChgMonsterIdx != 0 && mPlayerWeaponState != eWEAPON_STATE_NONE )
		{
			/// 근거리
			if( mPlayerWeaponState == eWEAPON_STATE_GUN )
			{
				minAttack = FloatToInt( GetStatus2()->mPhysicMinRangeAttack );
				maxAttack = FloatToInt( GetStatus2()->mPhysicMaxRangeAttack );
			}
			else
			{
				minAttack = FloatToInt( GetStatus2()->mPhysicMinAttack );
				maxAttack = FloatToInt( GetStatus2()->mPhysicMaxAttack );
			}
		}

		/// 공격력 범위안에서 선정
		unsigned long attackerStatusAttack = minAttack;
		if( minAttack < maxAttack )
		{
			attackerStatusAttack = maxAttack - minAttack;
			if( attackerStatusAttack > 0 )
				attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
			attackerStatusAttack = minAttack + attackerStatusAttack;
		}

		/// 타겟의 방어력 산출
		unsigned long targetStatusDef = 0;
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
				return 0;

			targetStatusDef = pMonster->GetPhysicDefense();
			targetLv = pMonster->GetLevel();
		}
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pPlayer == NULL )
				return 0;

			targetStatusDef = FloatToInt( pPlayer->GetStatus2()->mPhysicDefense );
			targetLv = pPlayer->GetLevel();
		}
		else
		{
			NETWORK2->PostServerEvent("cDamageCalc::PlayerDamageCalc eATTRIBUTETYPE_PHYSICAL target.type[%d,%d,%d,%d] ERROR", 
				target.type, mObject.index, GetRace(), GetRaceGender() );
			return 0;
		}

		/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
		unsigned long randTargetDef = 0;
		if( isIgnoreDefence == false )
			randTargetDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

		///// 랜덤 범위 제거 - 테스트용
		//attackerStatusAttack = ( maxAttack + minAttack ) / 2;
		//randTargetDef = targetStatusDef;

		damage = DAMAGECALC->PhyDamage500( eOBJECTTYPE_PLAYER, attackerLv, target.type, targetLv,
			attackerStatusAttack, skillDamage, skillDamagePer, randTargetDef, addTotalDamage, addPerTotalDamage );
	}
	/// 물리 스킬 성공확률 계산 처리
	else if( attributeType == eATTRIBUTETYPE_MAGIC )
	{
		unsigned long minAttack = 0;
		unsigned long maxAttack = 0;

		minAttack = FloatToInt( GetStatus2()->mMagicMinAttack );
		maxAttack = FloatToInt( GetStatus2()->mMagicMaxAttack );

		/// 공격력 범위안에서 선정
		unsigned long attackerStatusAttack = minAttack;
		if( minAttack < maxAttack )
		{
			/// 마법 공격력을 최대값에서 최소값 사이에 랜덤 값으로 설정
			attackerStatusAttack = maxAttack - minAttack;
			if( attackerStatusAttack > 0 )
				attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
			attackerStatusAttack = minAttack + attackerStatusAttack;
		}

		/// 타겟의 방어력 산출
		unsigned long targetStatusDef = 0;
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
				return 0;

			targetStatusDef = pMonster->GetMagicDefense();
			targetLv = pMonster->GetLevel();
		}
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pPlayer == NULL )
				return 0;

			targetStatusDef = FloatToInt( pPlayer->GetStatus2()->mMagicDefense );
			targetLv = pPlayer->GetLevel();
		}
		else
		{
			NETWORK2->PostServerEvent("cDamageCalc::PlayerDamageCalc eATTRIBUTETYPE_PHYSICAL target.type[%d,%d,%d,%d] ERROR", 
				target.type, mObject.index, GetRace(), GetRaceGender() );
			return 0;
		}

		/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
		unsigned long randTargetDef = 0;
		if( isIgnoreDefence == false )
			randTargetDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

		///// 랜덤 범위 제거 - 테스트용
		//attackerStatusAttack = ( maxAttack + minAttack ) / 2;
		//randTargetDef = targetStatusDef;

		damage = DAMAGECALC->MagDamage530( eOBJECTTYPE_PLAYER, attackerLv, target.type, targetLv,
		attackerStatusAttack, skillDamage, skillDamagePer, randTargetDef, addTotalDamage, addPerTotalDamage );
	}
	else
	{
		NETWORK2->PostServerEvent("cDamageCalc::PlayerDamageCalc attributeType[%d,%d,%d,%d] ERROR", attributeType,
			mObject.index, GetRace(), GetRaceGender() );
		return 0;
	}

	return damage;
}

unsigned long cPlayer::DotDamageCalc( unsigned char attackerType, unsigned char attackerLv, unsigned long attackMin, 
									 unsigned long attackMax, unsigned char dotType, unsigned long skillDamage, 
									 unsigned long skillDamagePer, unsigned long totalTime, unsigned long dotTime )
{
	/// 기본정보
	unsigned long damage = 0;
	unsigned char targetLv = GetLevel();

	switch( dotType )
	{
	case eDOTTYPE_PHYSICALCLOSE:
	case eDOTTYPE_PHYSICALRANGE:
		{
			/// 공격력 범위안에서 선정
			unsigned long attackerStatusAttack = attackMin;
			if( attackMin < attackMax )
			{
				attackerStatusAttack = attackMax - attackMin;
				if( attackerStatusAttack > 0 )
					attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
				attackerStatusAttack = attackMin + attackerStatusAttack;
			}

			///  방어력 산출
			unsigned long targetStatusDef = FloatToInt( GetStatus2()->mPhysicDefense );

			/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
			targetStatusDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

			damage = DAMAGECALC->PhyDotDamage600( attackerType, attackerLv, mObject.type, targetLv,
				attackerStatusAttack, skillDamage, skillDamagePer, targetStatusDef, totalTime, dotTime );
		}
		break;
	case eDOTTYPE_MAGIC:
		{
			/// 공격력 범위안에서 선정
			unsigned long attackerStatusAttack = attackMin;
			if( attackMin < attackMax )
			{
				/// 마법 공격력을 최대값에서 최소값 사이에 랜덤 값으로 설정
				attackerStatusAttack = attackMax - attackMin;
				if( attackerStatusAttack > 0 )
					attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
				attackerStatusAttack = attackMin + attackerStatusAttack;
			}

			/// 타겟의 방어력 산출
			targetLv = GetLevel();
			unsigned long targetStatusDef = FloatToInt( GetStatus2()->mMagicDefense );

			/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
			targetStatusDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

			damage = DAMAGECALC->MagDotDamage630( attackerType, attackerLv, mObject.type, targetLv,
				attackerStatusAttack, skillDamage, skillDamagePer, targetStatusDef, totalTime, dotTime );
		}
		break;
	default:
		NETWORK2->PostServerEvent("cDamageCalc::DotDamageCalc dotType[%d,%d,%d,%d] ERROR", dotType,
			mObject.index, GetRace(), GetRaceGender() );
		return 0;
	}

	return damage;
}


unsigned long cPlayer::HealCalc( sObject target, long skillHealPlus, long skillHealPer, bool applyHP )
{
	/// 플레이어 대 플레이어 힐말고 다른 경우가 생기면 어떤 식을 사용할지 정하고 추가하자!!
	if( target.type != eOBJECTTYPE_PLAYER )
	{
		/// 플레이어 이외의 오브젝트가 힐하는 방법이 정의된것은 없다.
		NETWORK2->PostServerEvent("cPlayer::HealCalc target.type ERROR [%d,%d,%d,%d,%d]", target.type, target.index, 
			skillHealPlus, skillHealPer, applyHP );
		return 0;
	}

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::HealCalc pTarget == NULL [%d,%d,%d,%d,%d]", target.type, target.index, 
			skillHealPlus, skillHealPer, applyHP );
		return 0;
	}

	/// 방어자 장비 정보
	sEquipAbility* pItem = pTarget->GetEquipAbility();
	if( pItem == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::HealCalc pItem == NULL [%d,%d,%d,%d,%d]", target.type, target.index, 
			skillHealPlus, skillHealPer, applyHP );
		return 0;
	}

	/// 공격자 정보
	unsigned long randHeal = FloatToInt( GetStatus2()->mHeal );

	/// 회복량을 HEAL_RANDOM_PER 값만큼 랜덤 선택
	randHeal = randHeal * ( HEAL_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

	/// 타겟 버프 정보
	long targetHealPlus = (long)pTarget->GetStatus2Plus( ePLAYER_STATUS_EXT_RECEIVEHEAL );
	long targetHealPer = (long)pTarget->GetStatus2Per( ePLAYER_STATUS_EXT_RECEIVEHEAL );
	/// 타겟 아이템 정보
	long targetItemPlus = pItem->mOptPlusHpReceive;
	long targetItemPer = pItem->mOptPercentHpReceive;

	/// 회복(힐)
	return DAMAGECALC->HealHP1200( randHeal, skillHealPlus, skillHealPer, targetHealPlus, targetHealPer, targetItemPlus, targetItemPer );
}


unsigned long cPlayer::DotHealCalc( sObject target, long infHealPlus, long infHealPer, unsigned long heal, unsigned long dotTotalTime, unsigned long dotTime )
{
	/// 플레이어 대 플레이어 힐말고 다른 경우가 생기면 어떤 식을 사용할지 정하고 추가하자!!
	if( target.type != eOBJECTTYPE_PLAYER )
	{
		/// 플레이어 이외의 오브젝트가 힐하는 방법이 정의된것은 없다.
		NETWORK2->PostServerEvent("cPlayer::DotHealCalc target.type ERROR [%d,%d,%d,%d,%d,%d,%d]", target.type, target.index, infHealPlus,
			infHealPer, heal, dotTotalTime, dotTime );
		return 0;
	}

	/// 회복량을 HEAL_RANDOM_PER 값만큼 랜덤 선택
	unsigned long randHeal = heal * ( HEAL_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

	/// 회복(힐)
	return DAMAGECALC->DotHealHP1230( randHeal, infHealPlus, infHealPer, dotTotalTime, dotTime );
}


unsigned long cPlayer::ConditionOdd( sObject target, long influenceRate, unsigned long skillClassIdx )
{
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::ConditionOdd pTarget == NULL[%d,%d,%d,%d]", target.type, target.index, influenceRate, skillClassIdx );
		return 0;
	}

	/// 플레이어 스크립트 로드
	sPlayerSkillBaseInfo* pPlayerSkill = SKILLSCRIPT->GetPlayerSkillInfo( skillClassIdx );
	if( pPlayerSkill == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::ConditionOdd pPlayerSkill == NULL[%d,%d,%d,%d]", target.type, target.index, influenceRate, skillClassIdx );
		return 0;
	}

	/// 공격자 레벨, 타겟 레벨
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = pTarget->GetLevel();

	unsigned long checkPer = 0;

	if( pPlayerSkill->mAttributeType == eATTRIBUTETYPE_PHYSICAL )
		checkPer = DAMAGECALC->PhyConditionOdd900( influenceRate, attackerLv, targetLv );
	else if( pPlayerSkill->mAttributeType == eATTRIBUTETYPE_MAGIC )
		checkPer = DAMAGECALC->MagConditionOdd930( influenceRate, attackerLv, targetLv );
	else
	{
		NETWORK2->PostServerEvent("cPlayer::ConditionOdd pPlayerSkill->mAttributeType[%d][%d,%d,%d,%d] ERROR",
			pPlayerSkill->mAttributeType, target.type, target.index, influenceRate, skillClassIdx );
	}

	return checkPer;
}


bool cPlayer::AttackSuccess( sObject target, unsigned long attributeType, short accuracyValue, unsigned long instantSuccess, unsigned long instantSuccessPer )
{
	/// return DAMAGECALC->PlayerAttackSuccess( mObject.index, target, (eATTRIBUTETYPE)aType, accuracyValue );

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		/// 공격자 스텟성공수치
		unsigned long attackerStatusRate = FloatToInt( GetStatus2()->mPhysicAttackRate );

		/// 타겟 회피 수치
		unsigned long targetStatusAvoid = 0;

		/// 타겟(몬스터)
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AttackSuccess pMonster[%d] == NULL", target.index );
				return false;
			}

			targetLv = pMonster->GetLevel();
			targetStatusAvoid = pMonster->GetPhysicAvoid();
		}
		/// 타겟(플레이어)
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pTargetPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pTargetPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AttackSuccess pTargetPlayer[%d] == NULL", target.index );
				return false;
			}

			targetLv = pTargetPlayer->GetLevel();
			targetStatusAvoid = FloatToInt( pTargetPlayer->GetStatus2()->mPhysicAvoid );
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayer::AttackSuccess physical target.type[%d] ERROR", target.type );
			return false;
		}

		/// 성공율 퍼센트 수치 환산
		unsigned long resultSuccess = DAMAGECALC->PhyAttackSuccess100( attackerStatusRate, targetStatusAvoid, accuracyValue, 
			eOBJECTTYPE_PLAYER, attackerLv, targetLv, instantSuccess, instantSuccessPer );

		/// 확률체크
		unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100 + 1);
		if( randvalue > resultSuccess ) 
			return false;
	}
	else if( attributeType == eATTRIBUTETYPE_MAGIC )	/// 마법 스킬 성공확률 계산 처리
	{
		/// 공격자 스텟성공수치
		unsigned long attackerStatusRate = FloatToInt( GetStatus2()->mMagicAttackRate );

		/// 타겟 회피 수치
		unsigned long targetStatusAvoid = 0;

		/// 타겟(몬스터)
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AttackSuccess pMonster[%d] == NULL", target.index );
				return false;
			}

			targetStatusAvoid = pMonster->GetMagicAvoid();
			targetLv = pMonster->GetLevel();
		}
		/// 타겟(플레이어)
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pTargetPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pTargetPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::AttackSuccess pTargetPlayer[%d] == NULL", target.index );
				return false;
			}

			targetStatusAvoid = FloatToInt( pTargetPlayer->GetStatus2()->mMagicAvoid );
			targetLv = pTargetPlayer->GetLevel();
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayer::AttackSuccess magic target.type[%d] ERROR", target.type );
			return false;
		}

		/// 성공율 퍼센트 수치 환산
		unsigned long resultSuccess = DAMAGECALC->MagAttackSuccess130( attackerStatusRate, targetStatusAvoid, accuracyValue, 
			eOBJECTTYPE_PLAYER, attackerLv, targetLv, instantSuccess, instantSuccessPer );

		/// 확률체크
		unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100 + 1);
		if( randvalue > resultSuccess ) 
			return false;
	}
	else
	{
		NETWORK2->PostServerEvent("cPlayer::AttackSuccess attributeType[%d] ERROR", attributeType );
		return false;	
	}

	return true;
}

bool cPlayer::CriticalSuccess( sObject target, unsigned long attributeType, unsigned short criticalValue, unsigned long instantCri, unsigned long instantCriPer )
{
	//return DAMAGECALC->PlayerCritical( mObject.index, target, (eATTRIBUTETYPE)attributeType, criticalValue );

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::CriticalSuccess pTarget[%d,%d] == NULL", target.type, target.index );
		return false;
	}

	sEquipAbility* pItem = GetEquipAbility();
	if( pItem == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::CriticalSuccess pItem[%d,%d] == NULL", target.type, target.index );
		return false;
	}

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = pTarget->GetLevel();

	unsigned long criSuccess = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		unsigned long attackerStatusCri = FloatToInt( GetStatus2()->mPhysicCritical );
		long attackerPer = FloatToInt( GetStatus2Per( ePLAYER_STATUS_EXT_CRI ) );
		long attackerItemPer = pItem->mOptPercentPhysicCritical;
		long attackerPlus = FloatToInt( GetStatus2Plus( ePLAYER_STATUS_EXT_CRI ) );
		long attackerItemPlus = pItem->mOptPlusPhysicCritical;

		float targetCriDef = 0.0f;

		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayerTarget = (cPlayer*)pTarget;
			sEquipAbility* pTargetItem = pPlayerTarget->GetEquipAbility();
			if( pTargetItem == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::CriticalSuccess eATTRIBUTETYPE_PHYSICAL pTargetItem[%d,%d] == NULL", target.type, target.index );
				return false;
			}

			targetCriDef = pPlayerTarget->GetStatus2()->mPhyCriDef;
		}
		else if( target.type == eOBJECTTYPE_MONSTER )
		{
			float num201 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 201 );
			float num204 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 204 );

			cMonster* pMonsterTarget = (cMonster*)pTarget;
			float targetCriDefPlus = pMonsterTarget->GetStatusPlus()->mPhysicCriticalDef;
			float targetCriDefPer = pMonsterTarget->GetStatusPer()->mPhysicCriticalDef;
			
			//몬스터의 경우				
			//A =	[공격자] 캐릭터 스탯 물리 크리티컬 수치 * {num201}	
			//D =	A
			//[E] =	D * ((100 +  [방어자]아이템 물리 방호[%]값 + [방어자]버프성 물리 방호 [%]값)  / 100) + ( ([방어자]아이템 물리 방호 수치[+-]값 + [방어자]버프성 물리 방호 수치[+-]값) * {num204} )	
			float valueD = (float)attackerStatusCri * num201;
			targetCriDef = valueD * (( 100.0f + 0.0f + targetCriDefPer ) / 100.0f ) + ( ( 0.0f + targetCriDefPlus ) * num204 );
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayer::CriticalSuccess eATTRIBUTETYPE_PHYSICAL target.type ERROR [%d,%d][%d,%d]", 
				target.type, target.index, attributeType, criticalValue );
			return false;
		}

		criSuccess = DAMAGECALC->PhyCritical200( attackerStatusCri, criticalValue, attackerLv, targetLv,
			attackerPer, attackerItemPer, attackerPlus, attackerItemPlus, 
			targetCriDef, instantCri, instantCriPer );
	}
	else if( attributeType == eATTRIBUTETYPE_MAGIC )
	{
		unsigned long attackerStatusCri = FloatToInt( GetStatus2()->mMagicCritical );
		long attackerPer = FloatToInt( GetStatus2Per( ePLAYER_STATUS_EXT_MCRI ) );
		long attackerItemPer = pItem->mOptPercentMagicCritical;
		long attackerPlus = FloatToInt( GetStatus2Plus( ePLAYER_STATUS_EXT_MCRI ) );
		long attackerItemPlus = pItem->mOptPlusMagicCritical;

		float targetCriDef = 0.0f;

		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayerTarget = (cPlayer*)pTarget;
			sEquipAbility* pTargetItem = pPlayerTarget->GetEquipAbility();
			if( pTargetItem == NULL )
			{
				NETWORK2->PostServerEvent("cPlayer::CriticalSuccess eATTRIBUTETYPE_MAGIC pTargetItem[%d,%d] == NULL", target.type, target.index );
				return false;
			}

			targetCriDef = pPlayerTarget->GetStatus2()->mMagCriDef;
		}
		else if( target.type == eOBJECTTYPE_MONSTER )
		{
			float num231 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 231 );
			float num234 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 234 );

			cMonster* pMonsterTarget = (cMonster*)pTarget;
			float targetCriDefPlus = pMonsterTarget->GetStatusPlus()->mMagicCriticalDef;
			float targetCriDefPer = pMonsterTarget->GetStatusPer()->mMagicCriticalDef;

			//몬스터의 경우				
			//A =	[공격자] 캐릭터 스탯 마법 크리티컬 수치 * {num231}
			//D =	A
			//[E] =	D * ((100 +  [방어자]아이템 마법 방호[%]값 + [방어자]버프성 마법 방호 [%]값)  / 100) + ( ([방어자]아이템 마법 방호 수치[+-]값 + [방어자]버프성 마법 방호 수치[+-]값) * {num234} )		
			float valueD = (float)attackerStatusCri * num231;
			targetCriDef = valueD * (( 100.0f + 0.0f + targetCriDefPer ) / 100.0f ) + ( ( 0.0f + targetCriDefPlus ) * num234 );
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayer::CriticalSuccess eATTRIBUTETYPE_MAGIC target.type ERROR [%d,%d][%d,%d]", 
				target.type, target.index, attributeType, criticalValue );
			return false;
		}

		criSuccess = DAMAGECALC->MagCritical230( attackerStatusCri, criticalValue, attackerLv, targetLv,
			attackerPer, attackerItemPer, attackerPlus, attackerItemPlus, targetCriDef, instantCri, instantCriPer );
	}
	else
	{
		NETWORK2->PostServerEvent("cPlayer::CriticalSuccess attributeType[%d] ERROR", attributeType );
		return false;
	}

	/// 확률체크
	unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100 + 1);
	if( randvalue > criSuccess ) 
		return false;

	return true;
}

//void cPlayer::CalcInstantSkill( unsigned long statusType1, unsigned long value1, unsigned long statusType2, unsigned long value2 )
//{
//	for( int i = ePLAYER_STATUS_DAMAGE_INSTANTPERHIT ; i < ePLAYER_STATUS_DAMAGE_MAX ; ++i )
//		SetDamageCalc( (ePLAYER_STATUS_DAMAGE)i, 0 );
//
//	eSTATUS_PLUS statusType = (eSTATUS_PLUS)statusType1;
//	unsigned long value = value1;
//	unsigned long arrayNum = 0;
//
//	if( statusType != 0 )
//	{
//		if( eSTATUSPLUS_INSTANTPERHIT <= statusType && statusType < eSTATUSPLUS_DAMAGECALC_MAX )
//		{
//			arrayNum = statusType - eSTATUSPLUS_INSTANTPERHIT;
//			SetDamageCalc( (ePLAYER_STATUS_DAMAGE)arrayNum, value );
//		}
//	}
//
//	statusType = (eSTATUS_PLUS)statusType2;
//	value = value2;
//
//	if( statusType != 0 )
//	{
//		/// 타입1, 타입2가 같으면 더해서 넣는다.
//		if( statusType1 == statusType2 )
//		{
//			NETWORK2->PostServerEvent(" Player::CalcInstantSkill [%d,%d] statusType1 == statusType2", statusType1, statusType2 );
//			value = value1 + value2;
//		}	
//
//		if( eSTATUSPLUS_INSTANTPERHIT <= statusType && statusType < eSTATUSPLUS_DAMAGECALC_MAX )
//		{
//			arrayNum = statusType - eSTATUSPLUS_INSTANTPERHIT;
//			SetDamageCalc( (ePLAYER_STATUS_DAMAGE)arrayNum, value );
//		}
//	}
//}

float cPlayer::CalcStatusSkillRange( float skillRange, eRANGETYPE mRangeType, bool IsNormalAttack )
{
	/// 스킬 사거리
	float range = skillRange;

	/// 효과 사거리
	float statusRangePlus = 0;
	float statusRangePer = 0;

	if( mRangeType == eRANGETYPE_CLOSE )
	{
		statusRangePlus = GetStatus2Plus( ePLAYER_STATUS_EXT_RANGE ) + mEquipAbility.mOptPlusShortRange;
		statusRangePer = GetStatus2Per( ePLAYER_STATUS_EXT_RANGE ) + mEquipAbility.mOptPercentShortRange;
	}
	else if( mRangeType == eRANGETYPE_LONG )
	{
		statusRangePlus = GetStatus2Plus( ePLAYER_STATUS_EXT_RRANGE ) + mEquipAbility.mOptPlusDistanceRange;
		statusRangePer = GetStatus2Per( ePLAYER_STATUS_EXT_RRANGE ) + mEquipAbility.mOptPercentDistanceRange;
	}
	else
		assert(NULL);

	/// 사거리 계산
	// 일반 공격
	if( IsNormalAttack == true )
		range = ( range + mEquipAbility.mAttackRange ) * ( ( 100 + statusRangePer ) / 100 ) + statusRangePlus;
	// 스킬 공격
	else
		range = range * ( ( 100 + statusRangePer ) / 100 ) + statusRangePlus;

	return range;
}

unsigned short cPlayer::GetMoveSpeed( )
{
	unsigned short moveSpeed = (unsigned short)FloatToInt( mStatus2.mMoveSpeed );
	if( moveSpeed < 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::GetMoveSpeed( ) <= 0");
		moveSpeed = 1;
	}

	return moveSpeed;
}

unsigned short cPlayer::GetMoveUpdateSpeed( unsigned long accumTime )
{
	unsigned short moveSpeed = GetMoveSpeed();

	/// 점프 시작 모션중
	if( mJumpSpeedState == 1 )
	{
		if( mJumpMoveSpeedDownEndTime > accumTime )
			moveSpeed = 10;
		else
		{
			mJumpMoveSpeedDownEndTime = 0;
			mJumpSpeedState = 2;
		}
	}

	/// 점프해서 공중에 떠서 이동하는 중
	if( mJumpEndTime < accumTime && mJumpSpeedState == 2 )
	{
		/// 전체적으로 가장 많이 분포한 착지 정지 시간
		mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 234;
		/// 탈것 인경우
		if( mVehicleInfIdx != 0 )
		{
			mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 267;
		}
		else if( mPlayerExrInfo.mChgMonsterIdx != 0 )
		{
			mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 234;
		}
		else
		{
			/// 종족 별로 시간설정
			switch ( mPlayerInfo.Race )
			{
			case eRACE_HUMAN: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 200; break;
			case eRACE_BEAST: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 234; break;
			case eRACE_ELF: mJumpMoveSpeedDownEndTime = NETWORK2->GetAccumTime() + 234; break;
			}
		}

		mJumpSpeedState = 3;
	}

	/// 점프 착지 모션중
	if( mJumpSpeedState == 3 )
	{
		if( mJumpMoveSpeedDownEndTime > accumTime )
			moveSpeed = 10;
		else
		{
			mJumpMoveSpeedDownEndTime = 0;
			mJumpSpeedState = 0;
		}
	}

	return moveSpeed;
}


bool cPlayer::IsChangeState( unsigned int newState, eCHANGESTATE_OPTION option )
{
	if( !( newState == eOBJECT_STATE_IDLE || newState == eOBJECT_STATE_MOVE || newState == eOBJECT_STATE_DIE || 
		newState == eOBJECT_STATE_PUSHPULL || newState == ePLAYER_STATE_RUSH ) )
	{
		/// 게임서버를 끄는중엔 통과시킨다.
		if( ( newState == eOBJECT_STATE_STOP && mPlayerExrInfo.mStateStop == eSTOP_GAMEFINISH ) == false )
		{
			unsigned long accumTime = NETWORK2->GetAccumTime();
			if( mJumpEndTime > accumTime )
				return false;
		}
	}

	switch( mPlayerExrInfo.mState )
	{
	case ePLAYER_STATE_SITDOWN:
	case eOBJECT_STATE_IDLE:
	case eOBJECT_STATE_MOVE:
	case ePLAYER_STATE_ITEMPICK:	/// 아이템 집는 중에 모든 상태 가능
		break;
	case eOBJECT_STATE_STOP:
		{
			/// 정지중에 죽는 상태, 대기 상태로만 변경가능
			if( newState == eOBJECT_STATE_DIE || newState == eOBJECT_STATE_IDLE || newState == eOBJECT_STATE_PUSHPULL )
				break;
			return false;
		}
		break;
	case ePLAYER_STATE_GATHERING:
		{
			/// 채집중에 죽는 상태, 대기 상태로만 변경가능
			if( newState == eOBJECT_STATE_DIE || newState == eOBJECT_STATE_IDLE || newState == ePLAYER_STATE_ITEMPICK || newState == eOBJECT_STATE_PUSHPULL )
				break;
			return false;
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			/// 공격중에 죽는 상태, 대기 상태로만 변경가능
			if( newState == eOBJECT_STATE_DIE || newState == eOBJECT_STATE_IDLE || newState == eOBJECT_STATE_PUSHPULL )
				break;
			return false;
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			if( newState == eOBJECT_STATE_IDLE )
				break;

			if( newState == eOBJECT_STATE_DIE )
			{
				//NETWORK2->PostServerEvent("cPlayer::IsChangeState mState == eOBJECT_STATE_PUSHPULL newState == eOBJECT_STATE_DIE");
				break;
			}

			return false;
		}
		break;
	case ePLAYER_STATE_RUSH:
		{
			/// 돌진에 죽는 상태, 대기 상태로만 변경가능
			if( newState == eOBJECT_STATE_DIE || newState == eOBJECT_STATE_IDLE || newState == eOBJECT_STATE_PUSHPULL )
				break;
			if( newState == eOBJECT_STATE_ATTACK && option == eCHANGESTATEOPTION_RUSH )
				break;
			return false;
		}
	case eOBJECT_STATE_DIE:
		{
			/// 죽은 상태에서 살아나는(IDLE) 상태로만 변경가능
			if( newState == eOBJECT_STATE_IDLE && option == eCHANGESTATEOPTION_RESURRECTION )
				break;
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

bool cPlayer::ChangeState( unsigned int newState, eCHANGESTATE_OPTION option )
{
	mVehicleAttack = false;

	/// 변경 가능한지 체크
	if( IsChangeState( newState, option ) == false )
		return false;

	/// 기존 상태에서 다른 상태로 변하는 경우
	switch( mPlayerExrInfo.mState )
	{
	case eOBJECT_STATE_STOP:
		{
			if ( newState == eOBJECT_STATE_DIE || newState == eOBJECT_STATE_PUSHPULL )
			{
				/// 상태에 따른 정리.
				switch ( GetStateStop( ) )
				{
				case eSTOP_READYTAROT:
				case eSTOP_OPENTAROT:
					TarotReaderDie( );
					break;
				case eSTOP_ENTERTAROT:
				case eSTOP_USETAROT:
					TarotSeekerDie( );
					break;
				case eSTOP_OPENSTALL:
					StallSellOpenDie( );
					break;
				case eSTOP_USESTALL:
					StallSellUseDie( );
					break;
				}
			}

			/// 정지상태 세부값을 NONE 으로 바꿈
			SetStateStop( eSTOP_NONE );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
			/// 부활 변수를 초기화
			mIsSkillResurrection = false;
			mSkillResurrectionHP = 0;
			mSkillResurrectionMP = 0;

			mCheckRushSkillIdx = 0;
			mCheckRushSkillStep = 0;

			mNextRecoveryTime = NETWORK2->GetAccumTime() + NATURE_RECOVERY_TIME;
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
			mNextRecoveryTime = NETWORK2->GetAccumTime() + NATURE_RECOVERY_TIME;
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			mJumpMoveSpeedDownEndTime = 0;
			mJumpSpeedState = 0;
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			mCheckRushSkillIdx = 0;
			mCheckRushSkillStep = 0;
		}
		break;
	}

	/// 신규 상태가 이동이 아닌 경우 이동 중이던 목표값 초기화 해준다.
	if( newState != eOBJECT_STATE_MOVE && newState != eOBJECT_STATE_PUSHPULL && newState != ePLAYER_STATE_RUSH )
	{	
		MoveStop();
	}

	/// 신규 상태처리
	switch( newState )
	{
	case eOBJECT_STATE_STOP:
		{
			ExchangeCancel( true );
		}
		break;
	case eOBJECT_STATE_DIE:
		{
			/// 몬스터가 쫓아오는 비중을 클리어 시킨다.
			mMonsterImportance = 0;
			/// npc랑 대화 중이었으면 대화를 초기화한다.
			mNpcIdx = 0;

			mPvPDieTime = NETWORK2->GetAccumTime();
			mPvPDieEndTime = mPvPDieTime + PVP_DM_RESURRECTION_TIME - SYNC_MOVE_TIME;
			
			/// 추적 위치 클리어
			memset( mFollowPos, 0, sizeof(mFollowPos) );

			/// 효과를 제거한다.
			DieDeleteInfluence();	

			/// 타겟으로 잡을 몬스터에게 전달
			DietargetingMonster();

			/// 채집 중이던 목록에서 플레이어 제거
			cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
			if( pGathering != NULL )
				pGathering->EraseGatherPlayer( this );

			mGatheringIdx = 0;

			// 거래 상태일때 풀어주기
			ExchangeCancel( false );
			
			// 신청 관련 취소 처리
			EndRequestRejectionAll();
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			/// npc랑 대화 중이었으면 대화를 초기화한다.
			mNpcIdx = 0;

			/// 채집 중이던 목록에서 플레이어 제거
			cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
			if( pGathering != NULL )
				pGathering->EraseGatherPlayer( this );

			mGatheringIdx = 0;

			// 거래 상태일때 풀어주기
			ExchangeCancel( false );
		}
		break;
	case ePLAYER_STATE_RUSH:
		{
			// 평화 대기 풀림
			mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;

			if( mPlayerExrInfo.mVehicleIdx != 0 )
			{
				NETWORK2->PostServerEvent("cPlayer::ChangeState eOBJECT_STATE_ATTACK mPlayerExrInfo.mVehicleIdx[%d,%d][%d] != 0",
					mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mVehicleIdx );
				SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );

				mVehicleAttack = true;
			}
		}
		break;
	case eOBJECT_STATE_ATTACK:
		{
			// 평화 대기 풀림
			mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;

			if( mPlayerExrInfo.mVehicleIdx != 0 )
			{
				NETWORK2->PostServerEvent("cPlayer::ChangeState eOBJECT_STATE_ATTACK mPlayerExrInfo.mVehicleIdx[%d,%d][%d] != 0",
					mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mVehicleIdx );
				SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );

				mVehicleAttack = true;
			}
		}
		break;
	case ePLAYER_STATE_SITDOWN:
		{
			/// 앉기 시작시간 설정 - 10초 후부터 자연회복 2배
			mNextRecoveryTime = NETWORK2->GetAccumTime() + SITDOWN_RECOVERY_TIME;

			if( mPlayerExrInfo.mVehicleIdx != 0 )
			{
				NETWORK2->PostServerEvent("cPlayer::ChangeState ePLAYER_STATE_SITDOWN mPlayerExrInfo.mVehicleIdx[%d,%d][%d] != 0",
					mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mVehicleIdx );
				SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
			}
		}
		break;
	case ePLAYER_STATE_ITEMPICK:
		{
			if( mPlayerExrInfo.mVehicleIdx != 0 )
			{
				NETWORK2->PostServerEvent("cPlayer::ChangeState ePLAYER_STATE_ITEMPICK mPlayerExrInfo.mVehicleIdx[%d,%d][%d] != 0",
					mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mVehicleIdx );
				SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
			}
		}
		break;
	case ePLAYER_STATE_GATHERING:
		{
			if( mPlayerExrInfo.mVehicleIdx != 0 )
			{
				NETWORK2->PostServerEvent("cPlayer::ChangeState ePLAYER_STATE_GATHERING mPlayerExrInfo.mVehicleIdx[%d,%d][%d] != 0",
					mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mVehicleIdx );
				SKILLMANAGER->DeleteInfluence( mObject, mVehicleInfIdx );
			}
		}
		break;
	}

	/// 상태 변경
	mPlayerExrInfo.mState = (unsigned char)newState;

	return true;
}

void cPlayer::ShutdownChangeState()
{ 
	if( mPlayerExrInfo.mState != eOBJECT_STATE_STOP || mPlayerExrInfo.mStateStop != eSTOP_GAMEFINISH || 
		mIsRequestRejection != eREQREJCT_GAMEFINISH )
	{
		NETWORK2->PostServerEvent("cPlayer::ShutdownChangeState( %d[%d,%d,%d] )", mObject.index, mPlayerExrInfo.mState,
			mPlayerExrInfo.mStateStop, mIsRequestRejection );
	}

	mPlayerExrInfo.mState = eOBJECT_STATE_STOP; 
	mPlayerExrInfo.mStateStop = eSTOP_GAMEFINISH;
	mIsRequestRejection = eREQREJCT_GAMEFINISH; 
}


void cPlayer::Resurrection( unsigned long resurrectionType )
{
	unsigned char errorCode = ERROR_PLAYER_RESURRECTION_SUCCESS;
	unsigned long expDown = 0;

	bool cashUse = false;
	unsigned long infIdx = 0;
	unsigned char cashExpCnt = 0;

	bool addInf = false;
	/// 죽었는지 확인
	if( GetStateDie() == true )
	{
		/// 스킬에 의한 부활여부
		if( resurrectionType == eRESURRECTIONTYPE_SKILL )
		{
			if( SkillResurrection() == false )
				errorCode = ERROR_PLAYER_RESURRECTION_FAIL;
			else
			{
				if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
					expDown = ExpDown( RESURRECTION_THEME_EXP_SKILL_MIN, RESURRECTION_THEME_EXP_SKILL_MAX );
				else
				{
					if( mPlayerInfo.Level >= 10 && mHeroInfo.Exp != 0 )
					{
						if( CalcCashExpGuard( &infIdx, &cashExpCnt ) == true )
						{
							cashUse = true;
							expDown = 0;
						}
						else
							expDown = ExpDown( RESURRECTION_EXP_SKILL_MIN, RESURRECTION_EXP_SKILL_MAX );
					}
				}
			}
		}
		else
		{
			if( ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("Player[%d] Resurrection ChangeState( eOBJECT_STATE_IDLE, true ) == false", mObject.index );
			}

			InitHP( 10 );
			InitMP( 10 );

			/// 경험치 하락
			switch( resurrectionType )
			{
			case eRESURRECTIONTYPE_STAND:		/// 제자리 부활
				{
					if( mPlayerInfo.Level >= 10 )
					{
						if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
						{
							errorCode = ERROR_PLAYER_RESURRECTION_FAIL;
							NETWORK2->PostServerEvent("cPlayer::Resurrection eRESURRECTIONTYPE_STAND _E_ST_ID_THEME_[%d,%d,%d,]",
								mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop );
						}
						else
						{
							if( mPlayerInfo.Level >= 10 && mHeroInfo.Exp != 0 )
							{
								if( CalcCashExpGuard( &infIdx, &cashExpCnt ) == true )
								{
									cashUse = true;
									expDown = 0;
								}
								else
									expDown = ExpDown( RESURRECTION_EXP_STAND_MIN, RESURRECTION_EXP_STAND_MAX );
							}
						}
							
						addInf = true;
					}
				}
				break;
			case eRESURRECTIONTYPE_SAFE:		/// 안전지대 부활
				{
					switch( NETWORK2->GetServerType() )
					{
					case _E_ST_NORMAL_MAP_:
						{
							NiPoint2 pos = STAGESCRIPT->CalcNearTargetMapPos( mMapNumber, mObjectPos.x, mObjectPos.y );
							mObjectPos.x = pos.x;
							mObjectPos.y = pos.y;

							float safeMin = RESURRECTION_EXP_SAFE_MIN;
							float safeMax = RESURRECTION_EXP_SAFE_MAX;

							if( mPlayerInfo.Level >= 10 && mHeroInfo.Exp != 0 )
							{
								if( CalcCashExpGuard( &infIdx, &cashExpCnt ) == true )
								{
									cashUse = true;
									expDown = 0;
								}
								else
									expDown = ExpDown( safeMin, safeMax );
							}
						}
						break;
					case _E_ST_ID_THEME_:
						{
							sTargetPos pos = STAGESCRIPT->GetStageChangePos( THEMEMANAGER->GetStartPosIdx() );
							if( pos.mMapNumber != 0 && pos.mMapNumber == mInDunMapDataNum )
							{
								mObjectPos.x = pos.mPosX;
								mObjectPos.y = pos.mPosY;

								float safeMin = RESURRECTION_THEME_EXP_SAFE_MIN;
								float safeMax = RESURRECTION_THEME_EXP_SAFE_MAX;

								if( mPlayerInfo.Level >= 10 && mHeroInfo.Exp != 0 )
									expDown = ExpDown( safeMin, safeMax );
							}
							else
							{
								NETWORK2->PostServerEvent("cPlayer::Resurrection NETWORK2->GetServerType() == _E_ST_ID_PVP_[%d]", mInDunMapDataNum );
							}
						}
						break;
					case _E_ST_ID_PVP_:
						{
							NETWORK2->PostServerEvent("cPlayer::Resurrection NETWORK2->GetServerType() == _E_ST_ID_PVP_[%d]", mInDunMapDataNum );
						}
						break;
					case _E_ST_ID_TUTORIAL_:
						{
							NETWORK2->PostServerEvent("cPlayer::Resurrection NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_" );
						}
						break;
					}

					ClearMoveSyncCalc();					
				}
				break;
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cPlayer::Resurrection GetStateDie_%d_%d,%d,%d_%d,%d_%d,%f,%f == false", mObject.index, 
			GetState(), GetStateStop(), mIsRequestRejection, mPlayerExrInfo.RestHP, mPlayerExrInfo.RestMP, mMapNumber,
			mObjectPos.x, mObjectPos.y );
		errorCode = ERROR_PLAYER_RESURRECTION_FAIL;
	}

	SetMoveTargetPos( mObjectPos.x, mObjectPos.y );

	if( cashUse == false )
	{
		HANDLE                       handle  = NULL;
		MSG_RES_PLAYER_RESURRECTION* sendMsg = (MSG_RES_PLAYER_RESURRECTION*)NETWORK2->GetMsgRoot( &handle
			,mConnectionIdx
			,NM_PLAYER
			,NM_PLAYER_RESURRECTION_RES );
		if ( sendMsg != NULL )
		{
			sendMsg->ErrorCode = errorCode;
			sendMsg->mHP = mPlayerExrInfo.RestHP;
			sendMsg->mMaxHP = GetMaxHP();
			sendMsg->mMP = mPlayerExrInfo.RestMP;
			sendMsg->mMaxMP = GetMaxMP();
			sendMsg->mExpDown = expDown;
			sendMsg->mPlayerExp = mHeroInfo.Exp;
			sendMsg->mPosX = mObjectPos.x;
			sendMsg->mPosY = mObjectPos.y;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_RESURRECTION) );
		}
	}
	else
	{
		HANDLE                       handle  = NULL;
		MSG_RES_PLAYER_CASHEXP_RESURRECTION* sendMsg = (MSG_RES_PLAYER_CASHEXP_RESURRECTION*)NETWORK2->GetMsgRoot( &handle
			,mConnectionIdx
			,NM_PLAYER
			,NM_PLAYER_RESURRECTION_CASHEXP_RES );
		if ( sendMsg != NULL )
		{
			sendMsg->ErrorCode = errorCode;
			sendMsg->mHP = mPlayerExrInfo.RestHP;
			sendMsg->mMaxHP = GetMaxHP();
			sendMsg->mMP = mPlayerExrInfo.RestMP;
			sendMsg->mMaxMP = GetMaxMP();
			sendMsg->mExpDown = expDown;
			sendMsg->mPlayerExp = mHeroInfo.Exp;
			sendMsg->mPosX = mObjectPos.x;
			sendMsg->mPosY = mObjectPos.y;
			sendMsg->mInfluenceIdx = infIdx;
			sendMsg->mCashExpCnt = cashExpCnt;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_CASHEXP_RESURRECTION) );
		}
	}

	if( errorCode == 0 )
	{
		/// 복구가능한 수치 기록
		mHeroInfo.mLoseExp = expDown;
		mExpRecoverEndTime = TIMER->GetRealPlusTime( 1 * DAY );

		MSG_SYN_PLAYER_RESURRECTION synMsg;
		synMsg.Category  = NM_PLAYER;
		synMsg.Protocol  = NM_PLAYER_RESURRECTION_SYN;
		synMsg.mCharacterIdx = mObject.index;
		synMsg.mHP = mPlayerExrInfo.RestHP;
		synMsg.mMaxHP = GetMaxHP();
		synMsg.mMP = mPlayerExrInfo.RestMP;
		synMsg.mMaxMP = GetMaxMP();
		synMsg.mPosX = mObjectPos.x;
		synMsg.mPosY = mObjectPos.y;   
		NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
	}

	if( addInf == true )
		SKILLMANAGER->AddInfluence( mObject, mObject, RESURRECTION_STAND_INFLUENCE_CLASSIDX, 0, true );		
}

bool cPlayer::SkillResurrection()
{
	bool retBool = mIsSkillResurrection;
	if( mIsSkillResurrection == true )
	{
		InitHP( mSkillResurrectionHP );
		InitMP( mSkillResurrectionMP );
		if( ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("Player[%d] SkillResurrection ChangeState( eOBJECT_STATE_IDLE, true ) == false", mObject.index );
		}

		mSkillResurrectionHP = 0;
		mSkillResurrectionMP = 0;
		mIsSkillResurrection = false;
	}

	return retBool;
}

void cPlayer::PvPResurrection( bool timeCheckPass )
{
	unsigned long accumTime = NETWORK2->GetAccumTime();

	/// 죽었는지 확인
	if( GetStateDie() == false )
		return;

	if( mPvPDMIdx == 0 )
	{
		if( timeCheckPass == true )
		{
			NETWORK2->PostServerEvent("cPlayer::PvPResurrection() hero[%d,%d,%d,%d] mPvPDMIdx == 0", 
				mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mPvPDieTime, mPvPDieEndTime, accumTime );
			return;
		}
	}

	if( mPvPDieEndTime > accumTime && timeCheckPass == true )
	{
		NETWORK2->PostServerEvent("cPlayer::PvPResurrection() hero[%d,%d,%d] mPvPDieEndTime[%d,%d] > accumTime[%d]",
			mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mPvPDieTime, mPvPDieEndTime, accumTime );
	}

	if( ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION ) == false )
		NETWORK2->PostServerEvent("cPlayer::PvPResurrection() hero[%d,%d,%d]->ChangeState() == false", 
		mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop );


	NiPoint2 pos;
	unsigned long posIdx = PVPMANAGER->GetTeamGenPosIdx( (ePVPDM_TEAM_TYPE)mPlayerExrInfo.mPvPDMTeamType );
	if( posIdx )
	{
		if( STAGESCRIPT->CalcTargetMapPos( posIdx, &pos.x, &pos.y ) == true )
		{
			mObjectPos.x = pos.x;
			mObjectPos.y = pos.y;
		}
	}
	else
	{
		assert(0);
	}


	InitHP( GetMaxHP() );
	InitMP( GetMaxMP() );

	HANDLE                       handle  = NULL;
	MSG_RES_PLAYER_RESURRECTION* sendMsg = (MSG_RES_PLAYER_RESURRECTION*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_RESURRECTION_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = 0;	
		sendMsg->mHP = mPlayerExrInfo.RestHP;
		sendMsg->mMaxHP = GetMaxHP();
		sendMsg->mMP = mPlayerExrInfo.RestMP;
		sendMsg->mMaxMP = GetMaxMP();
		sendMsg->mExpDown = 0;
		sendMsg->mPlayerExp = mHeroInfo.Exp;
		sendMsg->mPosX = mObjectPos.x;
		sendMsg->mPosY = mObjectPos.y;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_RESURRECTION) );
	}

	MSG_SYN_PLAYER_RESURRECTION synMsg;
	synMsg.Category  = NM_PLAYER;
	synMsg.Protocol  = NM_PLAYER_RESURRECTION_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mHP = mPlayerExrInfo.RestHP;
	synMsg.mMaxHP = GetMaxHP();
	synMsg.mMP =mPlayerExrInfo.RestMP;
	synMsg.mMaxMP = GetMaxMP();
	synMsg.mPosX = mObjectPos.x;
	synMsg.mPosY = mObjectPos.y;    
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );

	SKILLMANAGER->AddInfluence( mObject, mObject, PVP_DM_NODAMAGE_INFLUENCE, 0, true );
}

int cPlayer::TutorialResurrection( ULONG_PTR socketContext )
{
	if( mTutorialModeIndex == -1 )
	{
		NETWORK2->PostServerEvent("cPlayer::TutorialResurrection modeIndex error(%d, %d, %d)", mObject.index, GetState(), GetStateStop() );
		return ERROR_TUTORIAL_RESURRECTION_FAIL;
	}

	if( GetStateDie() == false )
	{
		NETWORK2->PostServerEvent("cPlayer::TutorialResurrection GetStateDie_%d_%d,%d,%d_%d,%d_%d,%f,%f == false", mObject.index, 
									GetState(), GetStateStop(), mIsRequestRejection, mPlayerExrInfo.RestHP, mPlayerExrInfo.RestMP, mMapNumber,
									mObjectPos.x, mObjectPos.y );
		return ERROR_TUTORIAL_RESURRECTION_FAIL;
	}

	if( ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION ) == false )
	{
		NETWORK2->PostServerEvent("cPlayer::TutorialResurrection() hero[%d,%d,%d]->ChangeState() == false", 
									mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop );
		return ERROR_TUTORIAL_RESURRECTION_FAIL;
	}

	cTutorialMode* mode = TUTORIALSCRIPT->GetTutorialMode( mTutorialModeIndex );
	if( mode == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::TutorialResurrection() mode data error(%d,%d)", mObject.index, mTutorialModeIndex );
		assert(0);
		return ERROR_TUTORIAL_RESURRECTION_FAIL;
	}

	NiPoint2 pos;
	if( STAGESCRIPT->CalcTargetMapPos( mode->mStartPosIndex, &pos.x, &pos.y ) == true )
	{
		mObjectPos.x = pos.x;
		mObjectPos.y = pos.y;
	}
	else
	{
		NETWORK2->PostServerEvent("cPlayer::TutorialResurrection() CalcTargetMapPos(%d,%d,%d)", mObject.index, mTutorialModeIndex, mode->mStartPosIndex );
		assert(0);
		return ERROR_TUTORIAL_RESURRECTION_FAIL;
	}

	InitHP( GetMaxHP() );
	InitMP( GetMaxMP() );

	HANDLE                         handle  = NULL;
	MSG_RES_TUTORIAL_RESURRECTION* sendMsg = (MSG_RES_TUTORIAL_RESURRECTION*)NETWORK2->GetMsgRoot( &handle
																								  ,(PerSocketContext*)socketContext
																								  ,NM_TUTORIAL
																								  ,NM_TUTORIAL_REVIVAL_RES );
	sendMsg->mHP		= mPlayerExrInfo.RestHP;
	sendMsg->mMaxHP		= GetMaxHP();
	sendMsg->mMP		= mPlayerExrInfo.RestMP;
	sendMsg->mMaxMP		= GetMaxMP();
	sendMsg->mPosX		= mObjectPos.x;
	sendMsg->mPosY		= mObjectPos.y;
	sendMsg->ErrorCode	= ERROR_TUTORIAL_RESURRECTION_SUCCESS;	
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TUTORIAL_RESURRECTION) );

	MSG_SYN_TUTORIAL_RESURRECTION synMsg;
	synMsg.Category		 = NM_TUTORIAL;
	synMsg.Protocol		 = NM_TUTORIAL_REVIVAL_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mHP			 = mPlayerExrInfo.RestHP;
	synMsg.mMaxHP		 = GetMaxHP();
	synMsg.mMP			 = mPlayerExrInfo.RestMP;
	synMsg.mMaxMP		 = GetMaxMP();
	synMsg.mPosX		 = mObjectPos.x;
	synMsg.mPosY		 = mObjectPos.y;    
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );

	return ERROR_TUTORIAL_RESURRECTION_SUCCESS;
}

//bool cPlayer::AddInfluence( unsigned long uniqueIdx, unsigned long influenceClassIdx )
bool cPlayer::AddInfluence( cInfluenceObject* pInf )
{
	if( pInf == NULL )
		return false;

	if( mInfluenceSet.Insert( pInf->GetUniqueIdx() ) == false )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::AddSkillInfluence mInfluenceSet.Insert( %d, %d, %d ) == false", 
			mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx() );
		return false;
	}

	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
	if( pScript != NULL )
	{
		switch( pInf->GetChangeMPType() )
		{
		case eINFCHANGEMPTYPE_NONE:
			break;
		case eINFCHANGEMPTYPE_PHY:
		case eINFCHANGEMPTYPE_MAG:
		case eINFCHANGEMPTYPE_ALL:
			{
				mChangeMPDamageAry.PushBack( pInf->GetUniqueIdx() );
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			}
			break;
		}

		switch( pInf->GetGuardType() )
		{
		case eINFGUARDTYPE_NONE:
			break;
		case eINFGUARDTYPE_PHY:
		case eINFGUARDTYPE_MAG:
		case eINFGUARDTYPE_ALL:
		case eINFGUARDTYPE_CASHEXP:
			{
				mGuardAry.PushBack( pInf->GetUniqueIdx() );
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			}
			break;
		default:
			NETWORK2->PostServerEvent("cPlayer::AddSkillInfluence pInf->GetGuardType(%d,%d,%d,%d) == false", 
				mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx(), pInf->GetGuardType() );
			break;
		}

		switch( pInf->GetDeleteType() )
		{
		case eDELETETYPE_PLAYER_DIE:
		case eDELETETYPE_NODELETE_PLAYER_DIE:
			break;
		case eDELETETYPE_PLAYER_DAMAGED:
			{
				mDeleteTypeDamagedAry.PushBack( pInf->GetUniqueIdx() );
			}
			break;
		default:
			NETWORK2->PostServerEvent("cPlayer::AddSkillInfluence pInf->GetDeleteType(%d,%d,%d,%d) == false", 
				mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx(), pInf->GetGuardType() );
			break;
		}

		/// 현재 지속 효과 번호가 3개 밖에 없어서 3번만 체크
		for( int checkCount = 0; checkCount < 3; ++checkCount )
		{
			unsigned short checkStatusPlusIdx = eSTATUSPLUS_NONE;
			switch(checkCount)
			{
			case 0: checkStatusPlusIdx = pScript->mStatusPlusIdx1; break;
			case 1: checkStatusPlusIdx = pScript->mStatusPlusIdx2; break;
			case 2: checkStatusPlusIdx = pScript->mStatusPlusIdx3; break;
			}

			if( checkStatusPlusIdx != eSTATUSPLUS_NONE )
			{
				if( pScript->mType == eINFLUENCETYPE_BUF )
				{
					cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( checkStatusPlusIdx );
					if(  findPos != mBuffmap.End() )
					{
						/// 이미 들어가 있는 경우
						cHashSet& influenceSet = (*findPos).mSecond;
						influenceSet.Insert( pInf->GetUniqueIdx() );
					}
					else
					{
						/// 새로 추가한 경우
						cHashSet influenceSet;
						influenceSet.Insert( pInf->GetUniqueIdx() );
						mBuffmap.Insert( checkStatusPlusIdx, influenceSet );
					}
				}
				else if( pScript->mType == eINFLUENCETYPE_DEBUF )
				{
					cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( checkStatusPlusIdx );
					if(  findPos != mDeBuffmap.End() )
					{
						/// 이미 들어가 있는 경우
						cHashSet& influenceSet = (*findPos).mSecond;
						influenceSet.Insert( pInf->GetUniqueIdx() );
					}
					else
					{
						/// 새로 추가한 경우
						cHashSet influenceSet;
						influenceSet.Insert( pInf->GetUniqueIdx() );
						mDeBuffmap.Insert( checkStatusPlusIdx, influenceSet );
					}
				}
			}
			else
			{
				/// 효과는 순차적으로 집어 넣기 때문에 하나라도 빈 인덱스가 나오면 나머지는 체크할 필요가 없다.
				break;
			}
		}
	}

	return true;
}

void cPlayer::ReleaseInfluenceSet( bool logOut ) 
{ 
	cSkillInfluenceSet::cIterator start = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

	unsigned long infIdx = 0;
	while( start != end )
	{
		infIdx = *start++;
		if( logOut == true )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( infIdx );
			if( pInf != NULL )
			{
				pInf->NoSendDelMsg();
				pInf->NoStatusCalc();
			}
		}
		else
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( infIdx );
			if( pInf != NULL )
			{
				/// 보유중이던 pc방 버프가 꺼지는 경우 주기적으로 다시 등록 시도한다.
				sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
				if( pScript != NULL && pScript->mType == eINFLUENCETYPE_PC )
					mReadyPcInfSet.Insert( pInf->GetInfluenceClassIdx() );
				else if( pScript != NULL && pScript->mType == eINFLUENCETYPE_GMEVENT )
				{
					///GameProcess 에서 정보를 가져와서 이벤트 버프 대기 셋에 넣는다.
					AddAllGMEvent();
				}
			}
		}

		SKILLMANAGER->DeleteInfluenceList( infIdx );
	}
}

void cPlayer::DieDeleteInfluence()
{
	cSkillInfluenceSet::cIterator start = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

	unsigned long infIdx = 0;
	cInfluenceObject* pInfluence = NULL;
	while( start != end )
	{
		infIdx = *start++;
		pInfluence = SKILLMANAGER->GetInfluence( infIdx );
		if( pInfluence == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::ReleaseSkillInfluenceSet pInfluence==NULL[%d]", mObject.index );
			continue;
		}

		if( pInfluence->GetDeleteType() == eDELETETYPE_NODELETE_PLAYER_DIE )
			continue;

		cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( infIdx );
		if( pInf != NULL )
		{
			/// 보유중이던 pc방 버프가 꺼지는 경우 주기적으로 다시 등록 시도한다.
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
			if( pScript != NULL && pScript->mType == eINFLUENCETYPE_PC )
			{
				mReadyPcInfSet.Insert( pInf->GetInfluenceClassIdx() );
			}
			else if( pScript != NULL && pScript->mType == eINFLUENCETYPE_GMEVENT )
			{
				///GameProcess 에서 정보를 가져와서 이벤트 버프 대기 셋에 넣는다.
				AddAllGMEvent();
			}
		}

		pInfluence->NoSendDelMsg();		/// 클라이언트에 발송을 하지 않는다.
		SKILLMANAGER->DeleteInfluenceList( infIdx );
	}

	//mGuardAry.Clear();
	//mChangeMPDamageAry.Clear();
}

void cPlayer::DeleteInfluence( unsigned long influenceIdx )
{
	/// 플레이어에서 효과가 사라질때 다른 항목에 효과가 사라지거나 추가변경이 있는경우 처리
	cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( influenceIdx );
	if( pInf )
	{
		/// 효과가 pvp 깃발 효과이면 효과가 꺼질때 깃발 리젠처리를 한다.
		if( pInf->GetInfluenceClassIdx() == PVP_DM2_FLAG_INFLUENCE && mPvPDMIdx != 0 )
		{
			cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
			if( pDM != NULL && pDM->GetType() == ePVPDM_OBJECTTYPE_2 )
			{
				cDeathMatch2Object* pDM2 = (cDeathMatch2Object*)pDM;
				if( pDM2->DelInfRegenFlag( mObject.index, mObjectPos.x, mObjectPos.y ) == true )
				{
					/// 채집 중이면 취소
					cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
					if( pGathering != NULL )
						pGathering->ReqEraseGatherPlayer( this );
				}
			}			
		}
		
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
		if( pScript != NULL )
		{
			/// 보유중이던 pc방 버프가 꺼지는 경우 주기적으로 다시 등록 시도한다.
			if( pScript->mType == eINFLUENCETYPE_PC )
				mReadyPcInfSet.Insert( pInf->GetInfluenceClassIdx() );
			else if( pScript->mType == eINFLUENCETYPE_CASH && mInfNoSave == false )
				SKILLMANAGER->SavePlayerInfluence( mObject.index, influenceIdx, true );
			else if( pInf->IsDbIdx() == true && mInfNoSave == false )
				SKILLMANAGER->SavePlayerInfluence( mObject.index, influenceIdx, true );
		}
	}

	for( unsigned long i = 0 ; i < mGuardAry.GetSize() ; ++i )
	{
		if( influenceIdx == mGuardAry[i] )
			mGuardAry.PopAt( i );
	}

	for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
	{
		if( influenceIdx == mChangeMPDamageAry[i] )
			mChangeMPDamageAry.PopAt( i );
	}

	for( unsigned long i = 0 ; i < mDeleteTypeDamagedAry.GetSize() ; ++i )
	{
		if( influenceIdx == mDeleteTypeDamagedAry[i] )
			mDeleteTypeDamagedAry.PopAt( i );
	}

	/// 버프/디버프 관리 map 에서 빼기
	if( pInf != NULL )
	{
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
		if( pScript != NULL )
		{
			/// 현재 지속 효과 번호가 3개 밖에 없어서 3번만 체크
			for( int checkCount = 0; checkCount < 3; ++checkCount )
			{
				unsigned short checkStatusPlusIdx = eSTATUSPLUS_NONE;
				switch(checkCount)
				{
				case 0: checkStatusPlusIdx = pScript->mStatusPlusIdx1; break;
				case 1: checkStatusPlusIdx = pScript->mStatusPlusIdx2; break;
				case 2: checkStatusPlusIdx = pScript->mStatusPlusIdx3; break;
				}

				if( checkStatusPlusIdx != eSTATUSPLUS_NONE )
				{
					if( pScript->mType == eINFLUENCETYPE_BUF )
					{
						cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( checkStatusPlusIdx );
						if(  findPos != mBuffmap.End() )
						{
							/// 찾은 경우 해당 set에서 삭제
							cHashSet& influenceSet = (*findPos).mSecond;
							influenceSet.Erase( pInf->GetUniqueIdx() );
						}
						else
						{
							/// 효과 맵에는 있지만 버프 맵에 없는 경우 mInfluenceSet 과 mBuffmap 이 동기화 안맞았음!
							NETWORK2->PostServerEvent("cMonster::DeleteInfluence mBuffmap.Find(%d)==mBuffmap.End (%d,%d,%d)", 
								checkStatusPlusIdx, mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx()  );
						}
					}
					else if( pScript->mType == eINFLUENCETYPE_DEBUF )
					{
						cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( checkStatusPlusIdx );
						if(  findPos != mDeBuffmap.End() )
						{
							/// 찾은 경우 해당 set에서 삭제
							cHashSet& influenceSet = (*findPos).mSecond;
							influenceSet.Erase( pInf->GetUniqueIdx() );
						}
						else
						{
							/// 효과 맵에는 있지만 버프 맵에 없는 경우 mInfluenceSet 과 mBuffmap 이 동기화 안맞았음!
							NETWORK2->PostServerEvent("cMonster::DeleteInfluence mDeBuffmap.Find(%d)==mDeBuffmap.End (%d,%d,%d)", 
								checkStatusPlusIdx, mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx()  );
						}
					}
				}
				else
				{
					/// 효과는 순차적으로 집어 넣기 때문에 하나라도 빈 인덱스가 나오면 나머지는 체크할 필요가 없다.
					break;
				}
			}				
		}
	}

	/// 플레이어가 가지고 있는 목록에서 제거
	if( mInfluenceSet.Erase( influenceIdx ) == false )
		NETWORK2->PostServerEvent("cPlayer::EraseSkillInfluence[%d][%d]", mObject.index, influenceIdx );

	if( pInf != NULL )
	{
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
		if( pScript != NULL )
		{
			if( pScript->mStatusPlusIdx1 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx1 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx2 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx2 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx3 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx3 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx1 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx1 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			else if( pScript->mStatusPlusIdx2 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx2 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			else if( pScript->mStatusPlusIdx3 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx3 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
		}
	}

}

void cPlayer::GameInInfApply()
{
	cInfluenceObject* pInfluenceObject = NULL;

	tHashSet<unsigned long>::cIterator start = mInfluenceSet.Begin();
	tHashSet<unsigned long>::cIterator end = mInfluenceSet.End();

	for( ; start != end ; ++start )
	{
		pInfluenceObject = SKILLMANAGER->GetInfluence( (*start) );
		if ( pInfluenceObject == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::InfluenceProcessStart()");
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInfluenceObject->GetInfluenceClassIdx() );
		if( pScript == NULL )
		{
			SKILLMANAGER->DeleteInfluence( mObject, (*start) );
			NETWORK2->PostServerEvent("cPlayer::InfluenceProcessStart pScript == NULL[%d]", pInfluenceObject->GetInfluenceClassIdx() );
			continue;
		}

		const int colNum = 3;
		long type[colNum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };
		long value[colNum] = { pScript->mValue1, pScript->mValue2, pScript->mValue3 };

		for( int i = 0 ; i < colNum ; ++i )
			SKILLMANAGER->ApplyInfluence( this, this, (*start), pInfluenceObject->GetInfluenceClassIdx(), type[i], value[i], true );
	}
}

void cPlayer::InfluenceProcessStart()
{
	cInfluenceObject* pInfluenceObject = NULL;

	tHashSet<unsigned long>::cIterator start = mInfluenceSet.Begin();
	tHashSet<unsigned long>::cIterator end = mInfluenceSet.End();

	for( ; start != end ; ++start )
	{
		pInfluenceObject = SKILLMANAGER->GetInfluence( (*start) );
		if ( pInfluenceObject == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::InfluenceProcessStart()");
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}
		pInfluenceObject->ProcessStart();

		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInfluenceObject->GetInfluenceClassIdx() );
		if( pScript == NULL )
		{
			SKILLMANAGER->DeleteInfluence( mObject, (*start) );
			NETWORK2->PostServerEvent("cPlayer::InfluenceProcessStart pScript == NULL[%d]", pInfluenceObject->GetInfluenceClassIdx() );
			continue;
		}

		const int colNum = 3;
		long type[colNum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };
		long value[colNum] = { pScript->mValue1, pScript->mValue2, pScript->mValue3 };

		for( int i = 0 ; i < colNum ; ++i )
			SKILLMANAGER->ApplyInfluence( this, this, (*start), pInfluenceObject->GetInfluenceClassIdx(), type[i], value[i], true );
	}
}

void cPlayer::InfluenceProcessStop()
{
	cInfluenceObject* pInfluenceObject = NULL;

	tHashSet<unsigned long>::cIterator start = mInfluenceSet.Begin();
	tHashSet<unsigned long>::cIterator end = mInfluenceSet.End();

	for( ; start != end ; ++start )
	{
		pInfluenceObject = SKILLMANAGER->GetInfluence( (*start) );
		if ( pInfluenceObject == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::InfluenceProcessStart()");
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}
		pInfluenceObject->ProcessStop();
	}
}

void cPlayer::AddActInfluence( unsigned long actIdx )
{
	sActInfluenceScript* pInfo = SKILLSCRIPT->GetActInfluenceInfo( actIdx );
	if( pInfo == 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::AddActInfluence(%d)", actIdx );
		return;
	}

	unsigned long coolTime = pInfo->mCooltime + NETWORK2->GetAccumTime();

	if( mActInfluenceMap.Insert( actIdx, coolTime ) == false )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::AddActInfluence(%d)", actIdx );
	}
}

void cPlayer::ClearActInfluence()
{
	mActInfluenceMap.Clear();
}

bool cPlayer::IsParentEqual( unsigned long parentUniqueIdx )
{
	cSkillInfluenceSet::cIterator beginAura = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator endAura = mInfluenceSet.End();

	cInfluenceObject* pInf = NULL;
	while( beginAura != endAura )
	{
		/// 보유 효과 오브젝트
		pInf = SKILLMANAGER->GetInfluence( *beginAura++ );
		if( !pInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::IsParentEqual pHaveInfluenceObject==NULL Aura" );
			continue;
		}

		if( pInf->GetSwitchType() == eSWITCHTYPE_PARENT && pInf->GetRarentIdx() == parentUniqueIdx )
			return true;
	}

	return false;
}

bool cPlayer::IsInfluenceHave( unsigned long influenceIdx )
{
	/// 현재 보유중인지 체크한다.
	cSkillInfluenceSet::cIterator b = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

	cInfluenceObject* pHaveInfluenceObject = NULL;
	while( b != end )
	{
		/// 보유 효과 오브젝트
		pHaveInfluenceObject = SKILLMANAGER->GetInfluence( *b++ );
		if( !pHaveInfluenceObject )	
		{
			NETWORK2->PostServerEvent("cSkillManager::IsInfluenceHave pHaveInfluenceObject==NULL" );
			continue;
		}

		if( pHaveInfluenceObject->GetInfluenceClassIdx() == influenceIdx )
			return true;
	}

	return false;
}

void cPlayer::SkillResurrectionHP( unsigned long HP )
{
	mIsSkillResurrection = true;
	mSkillResurrectionHP = HP;
}

void cPlayer::SkillResurrectionMP( unsigned long MP )
{
	if( mIsSkillResurrection == true )
	{
		mSkillResurrectionMP = MP;
	}
}

void cPlayer::MonsterInTargeting( tArray<sObject>* targetAry, float range )
{
	if( targetAry == NULL )
		return;

	/// 범위 설정
	mRangeCheck.SetRadius( (float)range );

	/// 나를 타겟으로 하는 몬스터 타겟 해제
	cMonster* pMonster = NULL;

	cTargetingMonsterSet::cIterator start = mTargetingMonsterSet.Begin();
	cTargetingMonsterSet::cIterator end = mTargetingMonsterSet.End();
	while( start != end )
	{
		unsigned long monsterIdx = (*start++);
		if( monsterIdx == 0 )
			continue;

		pMonster = GRIDMANAGER->GetMonster( monsterIdx );
		if( pMonster && pMonster->GetTarget() == this )
		{
			/// 범위 체크
			if( mRangeCheck.IsRange( mObjectPos, pMonster->GetPos() ) == true )
			{
				sObject multiTarget;
				multiTarget.type = pMonster->GetObjectType();
				multiTarget.index = pMonster->GetObjectID();
				//multiTarget.mApplyHP = true;
				targetAry->PushBack( multiTarget );
			}
		}
	}
}


/// 각 직업당 사용한 sp 저장 처리
void cPlayer::UpdateJobUsedSP()
{
	/// 보유스킬 포인터
	typedef tPointerHashMap<unsigned long, void*> cHaveSkillMap;
	cHaveSkillMap* haveSkillMap = (cHaveSkillMap*)SKILLMANAGER->GetPlayerHaveSkill( mObject.index );
	if( haveSkillMap == NULL )
		return;

	for( unsigned int i=0;i<mJobUsedSpArr.GetSize();i++ )
		mJobUsedSpArr[i] = 0;

	cHaveSkillMap::cIterator begin = haveSkillMap->Begin();
	cHaveSkillMap::cIterator end = haveSkillMap->End();
	for( ; begin != end ; ++begin )
	{
		cHaveSkillObject* pHaveSkill = (cHaveSkillObject*)((*begin).mSecond);
		if( pHaveSkill == NULL )
			continue;

		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( pHaveSkill->GetSkillIdx() );
		if( pSkillInfo == 0 )
			continue;
		if( pSkillInfo->mJobStep == 9 )
			continue;
		if( pSkillInfo->mStepCount <= pHaveSkill->GetSkillStep() )
		{
			assert(0);
			continue;
		}

		for( unsigned int i=0;i<=pHaveSkill->GetSkillStep();i++ )
		{
			sPlayerSkillStepInfo* pSkillStep = &pSkillInfo->mpSetpInfoArray[i];
			if( pSkillStep != NULL )
			{
				if( pSkillStep->mPlayerLevel != 0 )
					mJobUsedSpArr[pSkillInfo->mJobStep] = mJobUsedSpArr[pSkillInfo->mJobStep] + pSkillStep->mRequireSP;
			}
		}
	}
}

void cPlayer::StatusCalc()
{ 
	STATUSCALC->CalcPlayerExtensionGlobal( mObject );
}

bool cPlayer::CancelGathering( unsigned long gatheringIdx )
{ 
	if( mGatheringIdx != 0 && mGatheringIdx != gatheringIdx )
		return false;

	mGatheringIdx = 0;
	return true;
}

unsigned char cPlayer::AddMakeSkill( unsigned short itemSlotNum )
{
	/// 아이템 사용시 검사할 내용들.
	if( GetStateDie() == true )
		return ERROR_MAKESKILL_ADD_STATE;

    if ( IsInventoryBag( itemSlotNum ) == false )
		return ERROR_MAKESKILL_ADD_ITEM;

	TB_INVENTORY* inventory = SelectInventory( itemSlotNum );
	if( IsInventory( inventory ) == false )
		return ERROR_MAKESKILL_ADD_ITEM;

	if( inventory->apply != InventoryApplyNone )
		return ERROR_MAKESKILL_ADD_ITEM;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if( itemDefine == NULL )
		return ERROR_MAKESKILL_ADD_ITEM;

	/// 전문 스킬 아이템인지 확인
	if( itemDefine->type != ITEM_MIX_SKILL )
		return ERROR_MAKESKILL_ADD_ITEM;

    switch ( itemDefine->subType )
	{
	case ITEM_MIX_SKILL_L1:
	case ITEM_MIX_SKILL_L2:
	case ITEM_MIX_SKILL_L3:
	case ITEM_MIX_SKILL_L4:
		break;
	default:
		return ERROR_MAKESKILL_ADD_ITEM;
	}

    if ( IsInventorySeal( itemDefine, inventory ) == true )
        return ERROR_MAKESKILL_ADD_LICENSE;


	/// Item Limit Table(TB_ITEM_LIMIT) 적용
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		return ERROR_MAKESKILL_ADD_ITEM;

	/// 아이템 효과 적용 - 스킬과 연결하여 사용한다.
	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	if( itemAbility == NULL )
		return ERROR_MAKESKILL_ADD_ITEM;

	// 전문 스킬 인덱스 확인
	if( !(itemAbility->influence_idx > 0) ) 
		return ERROR_MAKESKILL_ADD_ITEM;

	unsigned char   makeSkill     = (unsigned char)itemAbility->influence_idx;	/// 전문 스킬 인덱스
    unsigned char   makeSkillStep = (unsigned char)itemDefine->subType;         /// 전문 스킬 단계
	sMakeSkill*		pMakeSkill    = NULL;
	unsigned char	addNum        = 0;

    if( mHeroInfo.MakeSkill1.mIndex > 0 || mHeroInfo.MakeSkill2.mIndex > 0 )
    {
        if( mHeroInfo.MakeSkillEnd < MAX_MAKESKILL && IsMakeSkill( makeSkill ) == false )
            return ERROR_MAKESKILL_ADD_NOTEMPTY;
    }

    /// 추가되는 위치
	if( mHeroInfo.MakeSkill1.mIndex == 0 || mHeroInfo.MakeSkill1.mIndex == makeSkill )
	{
		addNum		= 1;
		pMakeSkill	= &mHeroInfo.MakeSkill1;
	}
	else if( mHeroInfo.MakeSkill2.mIndex == 0 || mHeroInfo.MakeSkill2.mIndex == makeSkill )
	{
		addNum		= 2;
		pMakeSkill	= &mHeroInfo.MakeSkill2;
	}
   
    /// 배울 수 있는 직업 초과
	if( addNum == 0 || pMakeSkill == NULL )
		return ERROR_MAKESKILL_ADD_NOTEMPTY;

	/// 단계 체크
    if( pMakeSkill->mIndex > 0 )
	{
		if( pMakeSkill->mStep >= makeSkillStep )
			return ERROR_MAKESKILL_ADD_ALREADY;
		if( (pMakeSkill->mStep + 1) != makeSkillStep )
			return ERROR_MAKESKILL_ADD_STEP;
	}
    else
    {
        /// 처음부터 높은 단계를 배울 수 없게..
        if( makeSkillStep > 0 )
            return ERROR_MAKESKILL_ADD_STEP;
    }

	unsigned long inventoryIdx = inventory->idx;
	/// 아이템 차감
	if ( inventory->count == 1 )
		RemoveInventory( inventory->number );
	else
		UpdateInventory( inventory, (-1) );

	/// db 저장
	HANDLE handle = NULL;
	MAKESKILL_INSERT* pInsert = (MAKESKILL_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MAKESKILL_INSERT );
	pInsert->mCharacterIdx	= mObject.index;
	pInsert->mIsFirstSkill	= addNum == 1 ? true : false;
	pInsert->mMakeSkillIdx	= makeSkill;
    pInsert->mMakeSkillStep = makeSkillStep;
	pInsert->mInventoryIdx	= inventoryIdx;
	pInsert->mRetvalue		= -1;
	return (NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(MAKESKILL_INSERT) ) ) ? ERROR_MAKESKILL_ADD_SUCCESS : ERROR_MAKESKILL_ADD_DBERROR;
}

unsigned char cPlayer::AddRecipe( unsigned short itemSlotNum )
{
	/// 아이템 사용시 검사할 내용들.
	if( GetStateDie() == true )
		return ERROR_MAKESKILL_RECIPE_ADD_STATE;

    if ( IsInventoryBag( itemSlotNum ) == false )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	TB_INVENTORY* inventory = SelectInventory( itemSlotNum );
	if( IsInventory( inventory ) == false )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	if( inventory->apply != InventoryApplyNone )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if( itemDefine == NULL )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

    if ( IsInventorySeal( itemDefine, inventory ) == true )
        return ERROR_MAKESKILL_RECIPE_ADD_LICENSE;


	/// Item Limit Table(TB_ITEM_LIMIT) 적용
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	/// 아이템 효과 적용 - 스킬과 연결하여 사용한다.
	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	if( itemAbility == NULL )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	if( !(itemAbility->influence_idx > 0 && itemDefine->type == ITEM_MIX_SKILL ) )
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;

	/// 퀘스트 의뢰 아이템인지 확인
	if( itemDefine->subType == ITEM_MIX_RECIPE )
	{
		unsigned long recipeIdx = 0;
		recipeIdx = itemAbility->influence_idx;	// 레시피 인덱스	

		/// 레시피 스크립트 확인
		sMakeSkillScript* pRecipeInfo = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
		if( pRecipeInfo == NULL )
			return ERROR_MAKESKILL_RECIPE_ADD_SCRIPT;

        sMakeSkill* pMakeSkill = NULL;    

		/// 보유 전문기술 확인 & 보유 레시피 인지 확인
		if( mHeroInfo.MakeSkill1.mIndex == pRecipeInfo->mMakeSkill )
		{
			cRecipeGroup::cIterator end = mRecipeGroup1.End();
			if( mRecipeGroup1.Find( recipeIdx ) != end )
				return ERROR_MAKESKILL_RECIPE_ADD_ALREADY;

            pMakeSkill = &mHeroInfo.MakeSkill1;
		}
		else if( mHeroInfo.MakeSkill2.mIndex == pRecipeInfo->mMakeSkill )
		{
			cRecipeGroup::cIterator end = mRecipeGroup2.End();
			if( mRecipeGroup2.Find( recipeIdx ) != end )
				return ERROR_MAKESKILL_RECIPE_ADD_ALREADY;

            pMakeSkill = &mHeroInfo.MakeSkill2;
		}
        else
            return ERROR_MAKESKILL_RECIPE_ADD_NOTHAVE;

        /// 단계 체크
        if( !(pMakeSkill && pMakeSkill->mStep >= pRecipeInfo->mMakeStep) )
            return ERROR_MAKESKILL_RECIPE_ADD_STEP;

        /// 최소 숙련도
        if( !(pMakeSkill && pMakeSkill->mExp >= pRecipeInfo->mMinExp) )
            return ERROR_MAKESKILL_RECIPE_ADD_EXP;

		unsigned long inventoryIdx = inventory->idx;
		if ( inventory->count == 1 )
			RemoveInventory( inventory->number );
		else
			UpdateInventory( inventory, (-1) );

		/// db 저장
		HANDLE handle = NULL;
		RECIPE_INSERT* pInsert = (RECIPE_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_RECIPE_INSERT );
		pInsert->mCharacterIdx  = mObject.index;
		pInsert->mMakeSkillIdx  = pRecipeInfo->mMakeSkill;	
		pInsert->mRecipeIdx     = recipeIdx;
		pInsert->mInventoryIdx  = inventoryIdx;
		pInsert->retvalue       = -1;
		return (NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(RECIPE_INSERT) ) ) ? ERROR_MAKESKILL_RECIPE_ADD_SUCCESS : ERROR_MAKESKILL_RECIPE_ADD_DBERROR;

	}
	else if( itemDefine->subType == ITEM_MIX_RECIPEGROUP )
	{
		unsigned long groupIdx = 0;
		groupIdx = itemAbility->influence_idx;	// 레시피 그룹 인덱스

		/// 레시피 스크립트 확인
        sGroupInfo* pGroup = MAKESKILLSCRIPT->GetGroupInfo( groupIdx );
		if( pGroup == NULL )
			return ERROR_MAKESKILL_RECIPE_ADD_SCRIPT;
        
		HANDLE handle = NULL;
		RECIPE_GROUP_INSERT* pMakeSkill = (RECIPE_GROUP_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_RECIPE_GROUP_INSERT );
		pMakeSkill->mRowCnt = 0;

        cAry* pRecipeAry = &pGroup->mRecipeArr;
        for( unsigned long i = 0 ; i < pRecipeAry->GetSize() ; ++i )
        {
            sRecipeGroupInfo* pGroupInfo = (sRecipeGroupInfo*)(*pRecipeAry)[i];
            if( pGroupInfo == NULL )
                continue;

            /// 보유 레시피 인지 확인
			if( mHeroInfo.MakeSkill1.mIndex == pGroupInfo->mMakeSkill )
			{
                if( mHeroInfo.MakeSkill1.mStep < pGroup->mStep )
                {
                    NETWORK2->PostServerEvent( "WARNING - cPlayer::AddRecipe - RECIPEGROUP1 - index(%c), step(%c), exp(%ld) / index(%c), step(%c), exp(%ld)", 
                        mHeroInfo.MakeSkill1.mIndex, mHeroInfo.MakeSkill1.mStep, mHeroInfo.MakeSkill1.mExp,
                        pGroupInfo->mMakeSkill, pGroup->mStep, pGroup->mExp );
                    return ERROR_MAKESKILL_RECIPE_ADD_STEP;
                }

                if( mHeroInfo.MakeSkill1.mExp < pGroup->mExp )
                {
                    NETWORK2->PostServerEvent( "WARNING - cPlayer::AddRecipe - RECIPEGROUP1 - index(%c), step(%c), exp(%ld) / index(%c), step(%c), exp(%ld)", 
                        mHeroInfo.MakeSkill1.mIndex, mHeroInfo.MakeSkill1.mStep, mHeroInfo.MakeSkill1.mExp,
                        pGroupInfo->mMakeSkill, pGroup->mStep, pGroup->mExp );
                    return ERROR_MAKESKILL_RECIPE_ADD_EXP;
                }

				cRecipeGroup::cIterator end = mRecipeGroup1.End();
				if( mRecipeGroup1.Find( pGroupInfo->mRecipeIdx ) == end )
				{
					pMakeSkill->mRecipeGroup[pMakeSkill->mRowCnt].mRecipeIdx = pGroupInfo->mRecipeIdx;
					pMakeSkill->mRecipeGroup[pMakeSkill->mRowCnt].mMakeSkill = pGroupInfo->mMakeSkill;
					++pMakeSkill->mRowCnt;
				}
			}
			else if( mHeroInfo.MakeSkill2.mIndex == pGroupInfo->mMakeSkill )
			{
                if( mHeroInfo.MakeSkill2.mStep < pGroup->mStep )
                {
                    NETWORK2->PostServerEvent( "WARNING - cPlayer::AddRecipe - RECIPEGROUP2 - index(%c), step(%c), exp(%ld) / index(%c), step(%c), exp(%ld)", 
                        mHeroInfo.MakeSkill2.mIndex, mHeroInfo.MakeSkill2.mStep, mHeroInfo.MakeSkill2.mExp,
                        pGroupInfo->mMakeSkill, pGroup->mStep, pGroup->mExp );
                    return ERROR_MAKESKILL_RECIPE_ADD_STEP;
                }

                if( mHeroInfo.MakeSkill2.mExp < pGroup->mExp )
                {
                    NETWORK2->PostServerEvent( "WARNING - cPlayer::AddRecipe - RECIPEGROUP2 - index(%c), step(%c), exp(%ld) / index(%c), step(%c), exp(%ld)", 
                        mHeroInfo.MakeSkill2.mIndex, mHeroInfo.MakeSkill2.mStep, mHeroInfo.MakeSkill2.mExp,
                        pGroupInfo->mMakeSkill, pGroup->mStep, pGroup->mExp );
                    return ERROR_MAKESKILL_RECIPE_ADD_EXP;
                }

				cRecipeGroup::cIterator end = mRecipeGroup2.End();
				if( mRecipeGroup2.Find( pGroupInfo->mRecipeIdx ) == end )
				{
					pMakeSkill->mRecipeGroup[pMakeSkill->mRowCnt].mRecipeIdx = pGroupInfo->mRecipeIdx;
					pMakeSkill->mRecipeGroup[pMakeSkill->mRowCnt].mMakeSkill = pGroupInfo->mMakeSkill;
					++pMakeSkill->mRowCnt;
				}
			}
		}

		unsigned long inventoryIdx = inventory->idx;
		if ( inventory->count == 1 )
			RemoveInventory( inventory->number );
		else
			UpdateInventory( inventory, (-1) );

		/// db 저장
		pMakeSkill->mCharacterIdx   = mObject.index;
		pMakeSkill->mInventoryIdx   = inventoryIdx;
		pMakeSkill->mRetvalue       = -1;
		return (NETWORK2->SendSQL( mConnectionIdx, handle, pMakeSkill->GetMsgLength() ) ) ? ERROR_MAKESKILL_RECIPE_ADD_SUCCESS : ERROR_MAKESKILL_RECIPE_ADD_DBERROR;

	}
	else
		return ERROR_MAKESKILL_RECIPE_ADD_ITEM;
}

void cPlayer::SetMakeSkill( MAKESKILL_INSERT* pInsert )
{
	bool IsfirstSkill           = false;
	unsigned char makeSkill     = 0;
    unsigned char makeSkillStep = 0;

	if( pInsert != NULL )
	{
		IsfirstSkill    = pInsert->mIsFirstSkill;
		makeSkill       = pInsert->mMakeSkillIdx;
        makeSkillStep   = pInsert->mMakeSkillStep;
	}
	else
		NETWORK2->PostServerEvent("cPlayer::SetMakeSkill pInsert == NULL[%d]", mObject.index );

	if( IsfirstSkill == true )
    {
		mHeroInfo.MakeSkill1.mIndex = makeSkill;
        mHeroInfo.MakeSkill1.mStep  = makeSkillStep;
    }
	else
    {
		mHeroInfo.MakeSkill2.mIndex = makeSkill;
        mHeroInfo.MakeSkill2.mStep  = makeSkillStep;
    }

	HANDLE                 handle  = NULL;
	MSG_RES_MAKESKILL_ADD* sendMsg = (MSG_RES_MAKESKILL_ADD*)NETWORK2->GetMsgRoot( &handle
																				  ,mConnectionIdx
																				  ,NM_MAKESKILL
																				  ,NM_MAKESKILL_ADD_RES );
	if( sendMsg != NULL )
	{
		sendMsg->mMakeSkill     = makeSkill;
        sendMsg->mMakeSkillStep = makeSkillStep;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MAKESKILL_ADD) );
	}

	// 퀘스트발생
	SendNewQuestList();
}

unsigned char cPlayer::ReqDelMakeSkill( unsigned char makeSkill )
{
	if( IsMakeSkill( makeSkill ) == false )
		return ERROR_MAKESKILL_DEL_NOTHAVE;

	/// db 저장
	HANDLE handle = NULL;
	MAKESKILL_DELETE* pMakeSkill= (MAKESKILL_DELETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MAKESKILL_DELETE );
	pMakeSkill->mCharacterIdx   = mObject.index;
	pMakeSkill->mMakeSkillIdx   = makeSkill;
	pMakeSkill->mRetvalue       = -1;
	return (NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(MAKESKILL_DELETE) ) ) ? ERROR_MAKESKILL_DEL_SUCCESS : ERROR_MAKESKILL_DEL_DBERROR;
}

void cPlayer::DelMakeSkill( unsigned char makeSkillIdx )
{
	if( mHeroInfo.MakeSkill1.mIndex == makeSkillIdx )
	{
		mHeroInfo.MakeSkill1.mIndex = 0;
        mHeroInfo.MakeSkill1.mStep  = 0;
        mHeroInfo.MakeSkill1.mExp   = 0;
		mRecipeGroup1.Clear();
	}
	else if( mHeroInfo.MakeSkill2.mIndex == makeSkillIdx )
	{
		mHeroInfo.MakeSkill2.mIndex = 0;
        mHeroInfo.MakeSkill2.mStep  = 0;
        mHeroInfo.MakeSkill2.mExp   = 0;
		mRecipeGroup2.Clear();
	}

	HANDLE                    handle  = NULL;
	MSG_RES_MAKESKILL_DELETE* sendMsg = (MSG_RES_MAKESKILL_DELETE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_MAKESKILL, NM_MAKESKILL_DEL_RES );
	if( sendMsg != NULL )
	{
		sendMsg->mMakeSkill = makeSkillIdx;
		sendMsg->ErrorCode  = ERROR_MAKESKILL_DEL_SUCCESS;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MAKESKILL_DELETE) );
	}
}

void cPlayer::SetRecipe( unsigned char makeSkill, unsigned long recipeIdx )
{
	if( makeSkill == mHeroInfo.MakeSkill1.mIndex )
	{
		if( mRecipeGroup1.Insert( recipeIdx, 0 ) == false )
			NETWORK2->PostServerEvent("cPlayer::SetRecipe mRecipeGroup1.Insert( %d,%d,%d ) == false", mObject.index, makeSkill, recipeIdx );
	}
	else if( makeSkill == mHeroInfo.MakeSkill2.mIndex )
	{
		if( mRecipeGroup2.Insert( recipeIdx, 0 ) == false )
			NETWORK2->PostServerEvent("cPlayer::SetRecipe mRecipeGroup2.Insert( %d,%d,%d ) == false", mObject.index, makeSkill, recipeIdx );
	}
	else
		NETWORK2->PostServerEvent("cPlayer::SetRecipe else( %d,%d,%d ) == false", mObject.index, makeSkill, recipeIdx );

	HANDLE                        handle  = NULL;
	MSG_RES_MAKESKILL_RECIPE_ADD* sendMsg = (MSG_RES_MAKESKILL_RECIPE_ADD *)NETWORK2->GetMsgRoot( &handle
																								 ,mConnectionIdx
																								 ,NM_MAKESKILL
																								 ,NM_MAKESKILL_RECIPE_ADD_RES );
	if( sendMsg != NULL )
	{
		sendMsg->mGroupInfo[sendMsg->mCount].mMakeSkill = makeSkill;
		sendMsg->mGroupInfo[sendMsg->mCount].mRecipeIdx = recipeIdx;
		sendMsg->mCount = 1;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MAKESKILL_RECIPE_ADD) );
	}
}

void cPlayer::SetRecipeGroup( sRecipeGroupInfo* groupInfo, unsigned long cnt )
{
	HANDLE                        handle  = NULL;
	MSG_RES_MAKESKILL_RECIPE_ADD* sendMsg = (MSG_RES_MAKESKILL_RECIPE_ADD*)NETWORK2->GetMsgRoot( &handle
																								,mConnectionIdx
																								,NM_MAKESKILL
																								,NM_MAKESKILL_RECIPE_ADD_RES );
	if( sendMsg != NULL )
	{
		for( unsigned long i = 0 ; i < cnt ; ++i )
		{
			if( groupInfo[i].mMakeSkill == mHeroInfo.MakeSkill1.mIndex )
			{
				if( mRecipeGroup1.Insert( groupInfo[i].mRecipeIdx, 0 ) == false )
					NETWORK2->PostServerEvent("cPlayer::SetRecipe mRecipeGroup1.Insert( %d,%d,%d ) == false", mObject.index, groupInfo[i].mMakeSkill, groupInfo[i].mRecipeIdx );
			}
			else if( groupInfo[i].mMakeSkill == mHeroInfo.MakeSkill2.mIndex )
			{
				if( mRecipeGroup2.Insert( groupInfo[i].mRecipeIdx, 0 ) == false )
					NETWORK2->PostServerEvent("cPlayer::SetRecipe mRecipeGroup2.Insert( %d,%d,%d ) == false", mObject.index, groupInfo[i].mMakeSkill, groupInfo[i].mRecipeIdx );
			}
			else
			{
				NETWORK2->PostServerEvent("cPlayer::SetRecipe else( %d,%d,%d ) == false", mObject.index, groupInfo[i].mMakeSkill, groupInfo[i].mRecipeIdx );
				continue;
			}

			sendMsg->mGroupInfo[sendMsg->mCount].mMakeSkill = groupInfo[i].mMakeSkill;
			sendMsg->mGroupInfo[sendMsg->mCount].mRecipeIdx = groupInfo[i].mRecipeIdx;
			++sendMsg->mCount;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
	}
}

bool cPlayer::InitMakeSkill( MAKESKILL_SELECT* pMakeSkill )
{
	TB_MAKESKILL* pTable = NULL;
	for( long i = 0 ; i < pMakeSkill->mRowCount ; ++i )
	{
		pTable = &pMakeSkill->mTable[i];
		if( pTable == NULL )
		{
			NETWORK2->PostServerEvent("cPlayer::InitMakeSkill pTable == NULL[%d]", mObject.index );
			continue;
		}

		if( pTable->mMakeSkill != 0 && pTable->mMakeSkill == mHeroInfo.MakeSkill1.mIndex )		/// 제조스킬 비교
		{
			unsigned long endTime = ( pTable->mCoolTime * SECOND ) + NETWORK2->GetAccumTime();
			if( mRecipeGroup1.Insert( pTable->mRecipeIdx, endTime ) == false )
			{
				NETWORK2->PostServerEvent("cPlayer::InitMakeSkill mRecipeGroup1[%d][%d].Insert[%d]",
					mObject.index, mHeroInfo.MakeSkill1.mIndex, pTable->mRecipeIdx );
				return false;
			}
		}
		else if( pTable->mMakeSkill != 0 && pTable->mMakeSkill == mHeroInfo.MakeSkill2.mIndex )
		{
			unsigned long endTime = ( pTable->mCoolTime * SECOND ) + NETWORK2->GetAccumTime();
			if( mRecipeGroup2.Insert( pTable->mRecipeIdx, endTime ) == false )
			{
				NETWORK2->PostServerEvent("cPlayer::InitMakeSkill mRecipeGroup2[%d][%d].Insert[%d]",
					mObject.index, mHeroInfo.MakeSkill2.mIndex, pTable->mRecipeIdx );
				return false;
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayer::InitMakeSkill pTable->mMakeSkill[%d][%d] != ERROR", 
				mObject.index, pTable->mMakeSkill );
			continue;
		}
	}

	SendMsgRecipeList();
	return true;
}

void cPlayer::SendMsgRecipeList()
{
	HANDLE handle = NULL;

	cRecipeGroup::cIterator start	= mRecipeGroup1.Begin();
	cRecipeGroup::cIterator end		= mRecipeGroup1.End();
	MSG_RES_MAKESKILL_LIST* sendMsg = (MSG_RES_MAKESKILL_LIST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_MAKESKILL, NM_MAKESKILL_LIST1_RES );
	if( sendMsg != NULL )
	{
		unsigned short i = 0;
		for( ; start != end ; ++start, ++i )
		{
			sendMsg->mRecipeInfo[i].mRecipeIdx = (*start).mFirst;
			unsigned long endCoolTime	= (*start).mSecond;
			unsigned long accumTime		= NETWORK2->GetAccumTime();
			if( endCoolTime > accumTime )
				sendMsg->mRecipeInfo[i].mCoolTime = endCoolTime - accumTime;
			else
				sendMsg->mRecipeInfo[i].mCoolTime = 0;
		}

		sendMsg->mCount = i;
		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}

	start	= mRecipeGroup2.Begin();
	end		= mRecipeGroup2.End();
	sendMsg = (MSG_RES_MAKESKILL_LIST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_MAKESKILL, NM_MAKESKILL_LIST2_RES );
	if( sendMsg != NULL )
	{
		unsigned short i = 0;
		for( ; start != end ; ++start, ++i )
		{
			sendMsg->mRecipeInfo[i].mRecipeIdx = (*start).mFirst;
			unsigned long endCoolTime	= (*start).mSecond;
			unsigned long accumTime		= NETWORK2->GetAccumTime();
			if( endCoolTime > accumTime )
				sendMsg->mRecipeInfo[i].mCoolTime = endCoolTime - accumTime;
			else
				sendMsg->mRecipeInfo[i].mCoolTime = 0;
		}

		sendMsg->mCount = i;
		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
}

bool cPlayer::IsMakeSkill( unsigned char makeSkillIdx )
{
	if( makeSkillIdx == 0 )
		return false;

	if( mHeroInfo.MakeSkill1.mIndex == makeSkillIdx )
		return true;

	if( mHeroInfo.MakeSkill2.mIndex == makeSkillIdx )
		return true;

	return false;
}

int cPlayer::ItemMixReq( unsigned char makeSkill, unsigned long mixRecipeIdx, unsigned short makeCnt )
{
	/// db 사용부 체크
    if ( mInventoryDatabase )
        return ERROR_MAKESKILL_START_DBERROR;

    /// 조합 불가능 체크
    if( mPlayerExrInfo.mIsDuelIdx )
        return ERROR_MAKESKILL_START_STATE;

    /// 플레이어 상태 체크 ( 스턴, 슬립 )
    if( GetStateOddity( eODDITYTYPE_SLEEP ) != 0 )
        return ERROR_MAKESKILL_START_STATE;

    if( GetStateOddity( eODDITYTYPE_STUN ) != 0 )
        return ERROR_MAKESKILL_START_STATE;

    if( (int)mStatus2.mMaxWeight <= mItemWeight )
        return ERROR_MAKESKILL_START_STATE;

    /// 스크립트 체크
	sMakeSkillScript* pScript = MAKESKILLSCRIPT->GetMakeSkill( mixRecipeIdx );
	if( pScript == NULL )
	{
		NETWORK2->PostServerEvent( "cPlayer::ItemMixStart pScript[%d][%d] == NULL", mObject.index, mixRecipeIdx );
		return ERROR_MAKESKILL_START_SCRIPT;
	}

	/// 보유 여부 체크
	if( makeSkill == mHeroInfo.MakeSkill1.mIndex )
	{
        /// 단계
        if( mHeroInfo.MakeSkill1.mStep < pScript->mMakeStep )
            return ERROR_MAKESKILL_START_STEP;

        /// 숙련도
        if( mHeroInfo.MakeSkill1.mExp < pScript->mMinExp )
            return ERROR_MAKESKILL_START_EXP;

		/// 쿨타임 체크
		cRecipeGroup::cIterator find = mRecipeGroup1.Find( mixRecipeIdx );
		cRecipeGroup::cIterator end = mRecipeGroup1.End();
		if( find == end || (*find).mSecond > NETWORK2->GetAccumTime() )
			return ERROR_MAKESKILL_START_COOLTIME;
	}
	else if( makeSkill == mHeroInfo.MakeSkill2.mIndex )
	{
        if( mHeroInfo.MakeSkill2.mStep < pScript->mMakeStep )
            return ERROR_MAKESKILL_START_STEP;

        if( mHeroInfo.MakeSkill2.mExp < pScript->mMinExp )
            return ERROR_MAKESKILL_START_EXP;

		cRecipeGroup::cIterator find = mRecipeGroup2.Find( mixRecipeIdx );
		cRecipeGroup::cIterator end = mRecipeGroup2.End();
		if( find == end || (*find).mSecond > NETWORK2->GetAccumTime() )
			return ERROR_MAKESKILL_START_COOLTIME;
	}
	else
		return ERROR_MAKESKILL_START_HAVESKILL;

	/// 상태 변경
	if( ChangeState( eOBJECT_STATE_STOP ) == false )
		return ERROR_MAKESKILL_START_STATE;

	SetStateStop( eSTOP_ITEMMIX );

	/// 조합 정보 기록
	mMixRecipeIdx   = mixRecipeIdx;
	mMixMakeSkill   = makeSkill;
	mMixCnt         = makeCnt;
	mItemMixEndTime = NETWORK2->GetAccumTime() + pScript->mMakeTime;

	return ERROR_MAKESKILL_START_SUCCESS;
}

int cPlayer::ItemMix( sMakeSkillScript* script )
{
    if( script == NULL )
        return ERROR_MAKESKILL_END_ITEM;

	if( mItemMixEndTime > NETWORK2->GetAccumTime() + 1000 )
		return ERROR_MAKESKILL_END_TIME;

	/// 상태 체크
	if( GetState() != eOBJECT_STATE_STOP )
		return ERROR_MAKESKILL_END_STATE;

	if( GetStateStop() != eSTOP_ITEMMIX )
		return ERROR_MAKESKILL_END_STATE;

	tArray<void*>* array = script->mpMaterial;
	if ( array == NULL )
		return ERROR_MAKESKILL_END_ITEM;

	// 인벤토리 재료 검사.
	for ( unsigned long i = 0; i < array->GetSize( ); ++i )
	{
		sMaterialScript* material  = (sMaterialScript*)(*array)[i];
		PerItemCount*    itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, material->mItemIdx );

		if ( itemCount != NULL )
		{
			if ( itemCount->bag < material->mItemCnt )
				return ERROR_MAKESKILL_END_ITEM;
		}
		else
			return ERROR_MAKESKILL_END_ITEM;
	}

	// 인벤토리 영역 검사 & 조합 확률 처리.
	unsigned long*  makeItem;	// 캐스팅 포인터 - 조합결과 아이템
	unsigned char*  makeCount;	// "             - 조합결과 수량
	unsigned short* makeRate;   // "             - 조합     확률

	int             sumRate;	// 합계 확률.
	int             addRate;	// 누적 확률.
	int             rndRate;	// 램덤 확률.
	long            mixItem;	// 결과.
	short           mixCount;   // 결과.

	makeItem  = script->mMakeItemIdx;
	makeCount = script->mMakeCnt;
	makeRate  = script->mMakeRate;
	sumRate   = 0;

	for ( int i = 0; (i < MAKESKILL_MAKE_ITEM_CNT) && ((*makeItem) > 0) && ((*makeCount) > 0); i++, makeItem++, makeCount++, makeRate++ )
	{
		if ( CheckInventory( (*makeItem), (*makeCount) ) == false )
			return ERROR_MAKESKILL_END_ITEM;

		sumRate += (*makeRate);
	}

	makeItem  = script->mMakeItemIdx;
	makeCount = script->mMakeCnt;
	makeRate  = script->mMakeRate;

	addRate  = 0;
	unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
	rndRate  = randValue % sumRate;
	mixItem  = 0;
	mixCount = 0;

	for ( int i = 0; (i < MAKESKILL_MAKE_ITEM_CNT) && ((*makeItem) > 0) && ((*makeCount) > 0); i++, makeItem++, makeCount++, makeRate++ )
	{
		addRate += (*makeRate);

		if ( rndRate < addRate )
		{
			mixItem  = (*makeItem);
			mixCount = (*makeCount);
			break;
		}
	}

	if ( !( mixItem > 0 && mixCount > 0 ) )
		return ERROR_MAKESKILL_END_ITEM;

    TB_ITEM_DEFINE* mixItemDefine  = ITEMMANAGER->GetItemDefineByIndex( mixItem );
    if( mixItemDefine == NULL )
        return ERROR_MAKESKILL_END_ITEM;


    /// 남은 보유갯수 만큼만 만든다. - IsMaxInventory
    if ( mixItemDefine->maxInventory > 0 )
    {
        long sumCount = GetItemSumCount( mixItemDefine->index );
        if( mixItemDefine->maxInventory < (sumCount + mixCount) )
        {
            mixCount = (short)(mixItemDefine->maxInventory - sumCount);
            if( mixCount <= 0 )
                return ERROR_MAKESKILL_END_MAX_ITEM;
        }
    }

	HANDLE    handle   = NULL;
	ITEM_MIX* itemMix  = (ITEM_MIX*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_MIX );
	long      length   = sizeof(ITEM_MIX) - sizeof(itemMix->table);

	TB_INVENTORY* table    = itemMix->table;
	long&         rowCount = itemMix->rowCount;

	itemMix->characterIdx = mObject.index;

	// 재료 아이템 차감.
	for ( unsigned long i = 0; i < array->GetSize( ); ++i )
	{
		sMaterialScript* material    = (sMaterialScript*)(*array)[i];
		TB_INVENTORY*    tempInv     = SelectInventory( INVENTORY_BAG_BEGIN );
		short            remainCount = material->mItemCnt;
		short            shortCount;

		for ( short k = INVENTORY_BAG_BEGIN; (k <= mHeroInfo.BagEnd) && (remainCount > 0); k++, tempInv++ )
		{
			if ( (long)material->mItemIdx == tempInv->itemDefineIndex )
			{
				shortCount  = UpdateInventory( tempInv, (-remainCount) );
				remainCount = remainCount + shortCount;

				(*table) = (*tempInv);
				table->count = shortCount;

				table++; rowCount++;

				if ( tempInv->count == 0 )
					RemoveInventory( tempInv );
			}
		}
	}

	// 조합 아이템 생성.
	{
		short           remainCount = mixCount;

		TB_INVENTORY*   tempInv     = NULL;
		short           shortCount  = 0;
		short           number      = 0;

		// 아이템 조합 보상. 겹치기 검사 및 처리.
		if ( FindFirstInventory( mixItemDefine, &number, remainCount, &shortCount, true ) == true )
		{
			do {
				tempInv     = SelectInventory( number );
				shortCount  = UpdateInventory( tempInv, remainCount );
				remainCount = remainCount - shortCount;

				(*table) = (*tempInv);
				table->count = (+shortCount);

				table++; rowCount++;
			} while ( FindNextInventory( mixItemDefine, &(++number), remainCount, &shortCount, true ) == true );
		}
		// 아이템 조합 보상. 생성 검사 및 등록.
		if ( FindFirstInventory( mixItemDefine, &number, remainCount, &shortCount, false ) == true )
		{
			do {
				CreateInventory( mixItemDefine, number, remainCount, &(shortCount=0) );
				remainCount = remainCount - shortCount;

				table->idx             = 0;
				table->itemDefineIdx   = mixItemDefine->idx;
				table->itemDefineIndex = mixItemDefine->index;
				table->number          = number;
				table->count           = (+shortCount);

				table++; rowCount++;
			} while ( FindNextInventory( mixItemDefine, &(++number), remainCount, &shortCount, false ) == true );
		}
	}

	length += sizeof(TB_INVENTORY) * itemMix->rowCount;
	mInventoryDb.itemMix = NETWORK2->SendSQL( mConnectionIdx, handle, length );
	return ERROR_MAKESKILL_END_SUCCESS;
}

void cPlayer::ItemMixEnd()
{
	sMakeSkillScript* pScript = MAKESKILLSCRIPT->GetMakeSkill( mMixRecipeIdx );
	if( pScript != NULL )
	{
		/// 실제 아이템을 조합함 - db처리요청
		int errorCode = ItemMix( pScript );

		if( errorCode != ERROR_MAKESKILL_END_SUCCESS )
		{
			mMixCnt = 0;		/// 종료 설정
		}
		else	/// 다음 조합 시간 설정
		{
			unsigned long oldExp = 0;	// 기존 exp값
			unsigned long mixExp = 0;
			
			mItemMixEndTime = NETWORK2->GetAccumTime() + pScript->mMakeTime;
			--mMixCnt;			/// 조합 갯수 감소

			/// 쿨타임이 있으면 조합 중단
			if( pScript->mNextMakeTime != 0 )
				mMixCnt = 0;

			/// 쿨타임 변경
			if( mMixMakeSkill == mHeroInfo.MakeSkill1.mIndex )
			{
				oldExp = mHeroInfo.MakeSkill1.mExp;

				MakeSkillExpPlus( &mHeroInfo.MakeSkill1, pScript );
				mixExp = mHeroInfo.MakeSkill1.mExp;

				cRecipeGroup::cIterator find = mRecipeGroup1.Find( mMixRecipeIdx );
				cRecipeGroup::cIterator end  = mRecipeGroup1.End();
				if( find != end )
					(*find).mSecond = NETWORK2->GetAccumTime() + pScript->mNextMakeTime;
			}
			else if( mMixMakeSkill == mHeroInfo.MakeSkill2.mIndex )
			{
				oldExp = mHeroInfo.MakeSkill2.mExp;

				MakeSkillExpPlus( &mHeroInfo.MakeSkill2, pScript );
				mixExp = mHeroInfo.MakeSkill2.mExp;

				cRecipeGroup::cIterator find = mRecipeGroup2.Find( mMixRecipeIdx );
				cRecipeGroup::cIterator end  = mRecipeGroup2.End();
				if( find != end )
					(*find).mSecond = NETWORK2->GetAccumTime() + pScript->mNextMakeTime;
			}

			HANDLE                         handle  = NULL;
			MSG_SYN_MAKESKILL_ITEMMIX_EXP* sendMsg = (MSG_SYN_MAKESKILL_ITEMMIX_EXP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_MAKESKILL, NM_MAKESKILL_ITEMMIX_EXP_SYN );
			if ( sendMsg != NULL )
			{
				sendMsg->mMakeSkill      = mMixMakeSkill;
				sendMsg->mMakeSkillExp   = mixExp;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_EXP) );
			}

			// 조합 경험치가 올라간 경우 퀘스트 체크
			if( mixExp > oldExp )
			{
				SendNewQuestList();
			}
		}

		/// 더 진행 내역이 없으면 종료 발송
		if ( mMixCnt == 0 )
		{
			mItemMixEndTime = 0;
			mMixMakeSkill   = 0;
			mMixRecipeIdx   = 0;

			MSG_SYN_MAKESKILL_ITEMMIX_END synMsg;
			synMsg.Category         = NM_MAKESKILL;
			synMsg.Protocol         = NM_MAKESKILL_ITEMMIX_END_SYN;
			synMsg.mCharacterIdx    = mObject.index;
			synMsg.ErrorCode        = errorCode;
			NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_END) );

			/// 상태 풀어줌
			if ( GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_ITEMMIX )
			{
				ChangeState( eOBJECT_STATE_IDLE );
				SetStateStop( eSTOP_NONE );
			}
		}
		else
		{
			MSG_SYN_MAKESKILL_ITEMMIX_MIDDLEEND synMsg;
			synMsg.Category      = NM_MAKESKILL;
			synMsg.Protocol      = NM_MAKESKILL_ITEMMIX_MIDDLEEND_SYN;
			synMsg.mCharacterIdx = mObject.index;
			NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_MIDDLEEND) );
		}
	}
}

void cPlayer::ItemMixCancel()
{
	if( GetStateStop( ) != eSTOP_ITEMMIX )
		return;

	/// 조합 설정 끔
	mItemMixEndTime = 0;
	mMixRecipeIdx   = 0;
	mMixMakeSkill   = 0;

	/// 플레이어 상태를 풀어줌
	if ( GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_ITEMMIX )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );
	}

	/// hero외의 플레이어에게 취소 확인 메세지 발송
	MSG_SYN_MAKESKILL_ITEMMIX_CANCEL synMsg;
	synMsg.Category      = NM_MAKESKILL;
	synMsg.Protocol      = NM_MAKESKILL_ITEMMIX_CANCEL_SYN;
	synMsg.mCharacterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_CANCEL) );

	/// db 처리중인 내용이 있으면 클라이언트에 알림
	int retcode = ( mInventoryDb.itemMix == true ) ? ERROR_MAKESKILL_CANCEL_DBWORK : ERROR_MAKESKILL_CANCEL_SUCCESS;
	NETWORK2->SendMsgError( mConnectionIdx, NM_MAKESKILL, NM_MAKESKILL_ITEMMIX_CANCEL_RES, retcode );
}

void cPlayer::ItemMixForceCancel()
{
	/// 조합 진행중인지 체크
	if( GetStateStop( ) != eSTOP_ITEMMIX )
		return;

	/// 조합 설정 끔
	mItemMixEndTime = 0;
	mMixRecipeIdx   = 0;
	mMixMakeSkill   = 0;

	/// 플레이어 상태를 풀어줌
	if ( GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_ITEMMIX )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );
	}

	/// hero 에게 발송
	MSG_SYN_MAKESKILL_ITEMMIX_CANCELFORCE msg;
	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_ITEMMIX_CANCELFORCE_SYN;
	NETWORK2->QuickSend( this, (char*)&msg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_CANCELFORCE) );

	/// hero외의 플레이어에게 취소 확인 메세지 발송
	MSG_SYN_MAKESKILL_ITEMMIX_CANCEL synMsg;
	synMsg.Category      = NM_MAKESKILL;
	synMsg.Protocol      = NM_MAKESKILL_ITEMMIX_CANCEL_SYN;
	synMsg.mCharacterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_CANCEL) );
}

bool cPlayer::SaveRecipeCoolTime( RECIPE_COOLTIME* pRecipeCoolTime )
{
	pRecipeCoolTime->mCharacterIdx = mObject.index;
	pRecipeCoolTime->mRetvalue = -1;
	unsigned long row = 0;

	if( mHeroInfo.MakeSkill1.mIndex != 0 )
	{
		cRecipeGroup::cIterator begin = mRecipeGroup1.Begin();
		cRecipeGroup::cIterator end = mRecipeGroup1.End();

		for(  ; begin != end ; ++begin )
		{
			TB_RECIPE_COOLTIME* pCoolTime =	&pRecipeCoolTime->mTable[row];

			if( pCoolTime != NULL && (*begin).mSecond > NETWORK2->GetAccumTime() )
			{
				pCoolTime->mMakeSkill = mHeroInfo.MakeSkill1.mIndex;
				pCoolTime->mRecipeIdx = (*begin).mFirst;
				pCoolTime->mLeftCoolTime = (*begin).mSecond - NETWORK2->GetAccumTime();
				++row;
			}
		}
	}

	if( mHeroInfo.MakeSkill2.mIndex != 0 )
	{
		cRecipeGroup::cIterator begin = mRecipeGroup2.Begin();
		cRecipeGroup::cIterator end = mRecipeGroup2.End();

		for(  ; begin != end ; ++begin )
		{
			TB_RECIPE_COOLTIME* pCoolTime =	&pRecipeCoolTime->mTable[row];

			if( pCoolTime != NULL && (*begin).mSecond > NETWORK2->GetAccumTime() )
			{
				pCoolTime->mMakeSkill = mHeroInfo.MakeSkill2.mIndex;
				pCoolTime->mRecipeIdx = (*begin).mFirst;
				pCoolTime->mLeftCoolTime = (*begin).mSecond - NETWORK2->GetAccumTime();
				++row;
			}
		}
	}

	pRecipeCoolTime->mRowCount = row;
	return true;

}

void cPlayer::MakeSkillExpPlus( sMakeSkill* pMakeSkill, sMakeSkillScript* Recipe )
{
    if( !pMakeSkill )
        return;

    if( pMakeSkill->mExp >= Recipe->mMaxDefaultExp && pMakeSkill->mExp < Recipe->mMaxExp )
        ++pMakeSkill->mExp;
    else if( pMakeSkill->mExp >= Recipe->mMinDefaultExp && pMakeSkill->mExp < Recipe->mMaxDefaultExp)
        pMakeSkill->mExp += 2;
    else if( pMakeSkill->mExp >= Recipe->mMinExp && pMakeSkill->mExp < Recipe->mMinDefaultExp)
        pMakeSkill->mExp += 3;

    /// 최대 목표치 체크
    unsigned long maxStepExp = 0;
    switch( pMakeSkill->mStep )
    {
    case ITEM_MIX_SKILL_L1: maxStepExp = MIXSKILL_EXP1; break;
    case ITEM_MIX_SKILL_L2: maxStepExp = MIXSKILL_EXP2; break;
    case ITEM_MIX_SKILL_L3: maxStepExp = MIXSKILL_EXP3; break;
    case ITEM_MIX_SKILL_L4: maxStepExp = MIXSKILL_EXP4; break;
    default: return;
    }

    if( maxStepExp != 0 && pMakeSkill->mExp > maxStepExp )
        pMakeSkill->mExp = maxStepExp;
}

bool cPlayer::SkillPointMinus( unsigned short minusSP )
{
	if( mHeroInfo.SkillPointRemain - minusSP < 0 )
	{
		return false;
	}

	mHeroInfo.SkillPointRemain = mHeroInfo.SkillPointRemain - minusSP;
	return true;
}

void cPlayer::SkillPointMinusRestore( unsigned short minusSP )
{
	mHeroInfo.SkillPointRemain = mHeroInfo.SkillPointRemain + minusSP;
}

bool cPlayer::SkillPointPlus( unsigned short plusSP )
{
	if( mHeroInfo.SkillPointRemain + plusSP > USHRT_MAX )
	{
		return false;
	}

	mHeroInfo.SkillPointRemain = mHeroInfo.SkillPointRemain + plusSP;
	mHeroInfo.SkillPointTotal = mHeroInfo.SkillPointTotal + plusSP;

	// 캐릭터 이벤트.
	PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - PointPlus" );

	SendSkillPoint();
	return true;
}

void cPlayer::SetSkillReset( TB_CHARACTER_SKILL* pSkill, long cnt )
{
	mHeroInfo.SkillPointRemain = mHeroInfo.SkillPointTotal;

	/// 스킬 초기화
	SKILLMANAGER->ReleasePlayerHaveSkill( mObject.index );

	/// 자신이 사용한 오라효과 모두 제거
	SKILLMANAGER->SwitchParentAllOff( mObject.index );

	/// 기본 공격 등록
	SKILLMANAGER->AddSkillReset( mObject.index, NORMAL_ATTACK_SKILL, SKILL_START_STEP );

	HANDLE                     handle  = NULL;
	MSG_RES_PLAYER_SKILLRESET* sendMsg = (MSG_RES_PLAYER_SKILLRESET*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_SKILLRESET_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_SKILLRESET_SUCCESS;
		sendMsg->mRowCount = cnt;
		sendMsg->mSkillPoint = mHeroInfo.SkillPointRemain;
		for( long i = 0 ; i < cnt ; ++i )
		{
			/// 기본 보유 스킬 등록
			SKILLMANAGER->AddSkillReset( mObject.index, pSkill[i].mSkillIdx, pSkill[i].mSkillStep );
			sendMsg->mTable[i].mSkillIdx = pSkill[i].mSkillIdx;
			sendMsg->mTable[i].mSkillStep = pSkill[i].mSkillStep;
		}
		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
	}
}

void cPlayer::SendSkillPoint()
{
	HANDLE             handle  = NULL;
	MSG_RES_PLAYER_SP* sendMsg = (MSG_RES_PLAYER_SP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_SP_RES );
	if ( sendMsg != NULL )
	{
		unsigned long length = sizeof(MSG_RES_PLAYER_SP) - sizeof(sendMsg->usedSP);

		sendMsg->mSP		 = mHeroInfo.SkillPointRemain;
		sendMsg->mTotalSP	 = mHeroInfo.SkillPointTotal;

		unsigned short* list = sendMsg->usedSP;
		for( unsigned long i = 0; i < mJobUsedSpArr.GetSize(); ++i, ++list )
		{
			*list = mJobUsedSpArr[i];
			sendMsg->rowCount++;
		}

		length += (sendMsg->rowCount * sizeof(sendMsg->usedSP));
		NETWORK2->SendMsgRoot( handle, length );
	}
}


bool cPlayer::MapChange( unsigned long posIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
		return false;

    if( IsAutoTrial() == true )
        return false;

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return false;

	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, NULL, 0 );

	// 이동 좌표를 얻어옴
	sTargetPos targetPos;
	memset( &targetPos, 0 , sizeof(sTargetPos) );
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_ID_PVP_:
		targetPos = STAGESCRIPT->GetStageChangePos( mObjectPos.x, mObjectPos.y, NETWORK2->GetInDunMapNum(), tempRange, posIdx );
		break;
	case _E_ST_ID_THEME_:
		{
			sStageChangePos* pPos = STAGESCRIPT->GetPosScriptInfo( posIdx );
			if( pPos != NULL && pPos->mFirstRegen == true )
			{
				targetPos = STAGESCRIPT->GetStageChangePos( pPos->mTargetPosIdx );
			}
			else
			{
				NETWORK2->PostServerEvent("cPlayer::MapChange _E_ST_ID_THEME_[%d,%d,%d]", mObject.index, posIdx, NETWORK2->GetInDunMapNum() ); 
				targetPos = STAGESCRIPT->GetStageChangePos( CHARACTER_CREATE_POS_IDX );
			}
		}
		break;
	case _E_ST_NORMAL_MAP_:
		targetPos = STAGESCRIPT->GetStageChangePos( mObjectPos.x, mObjectPos.y, mMapNumber, tempRange, posIdx );
		break;
	case _E_ST_ID_TUTORIAL_:
		{
			NETWORK2->PostServerEvent("cPlayer::MapChange _E_ST_ID_TUTORIAL_[%d,%d,%d]", mObject.index, posIdx, NETWORK2->GetInDunMapNum() ); 			
			return false;
		}
		break;
	}

	/// 이동 좌표 오류
	if( targetPos.mMapNumber == 0 )
		return false;

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return false;

	/// 맵이동 목적지가 현재 맵과 같다면 텔레포트를 시킨다.
	
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_NORMAL_MAP_:
		{
			mMapChange = true;
		}
		break;
	case _E_ST_ID_PVP_:
		{
			return false;
		}
		break;
	case _E_ST_ID_THEME_:
		{
			mIsInDunJoin = false;
			mMapChange = false;
			mMapNumber = targetPos.mMapNumber;
			mGotoX = mObjectPos.x = targetPos.mPosX;
			mGotoY = mObjectPos.y = targetPos.mPosY;
			mDirection = targetPos.mRotAngle;

			CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );
		}
		break;
	case _E_ST_ID_TUTORIAL_:
		{
			/// 위에서 체크햇음!
			return false;
		}
		break;
	}

	/// 이동 지역 설정
	SetMapTargetPos( targetPos );

	InfluenceProcessStop();

	SetStateStop( eSTOP_MAPCHANGE );
	return true;
}

bool cPlayer::CheatMapChange( unsigned short mapNumber, float x, float y )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
		return false;

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return false;

	/// 이동 좌표 오류
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_NORMAL_MAP_:
		{
			if( mapNumber < MAP_MIN || MAP_MAX < mapNumber )
				return false;
		}
		break;
	case _E_ST_ID_PVP_:
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			if( mapNumber >= INDUN_MAP_MAX )
				return false;
		}
	}
	
	/// 이동 위치 체크
	float mapMaxSize = AIMANAGER->GetPathMaxSideSize( mapNumber );
	if( x < 0 || x > mapMaxSize || y < 0 || y > mapMaxSize )
		return false;

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return false;

	// 이동 좌표를 설정
	sTargetPos targetPos;
	targetPos.mMapNumber = mapNumber;
	targetPos.mPosX = x;
	targetPos.mPosY = y;
	targetPos.mRotAngle = 0;

	/// 좌표값이 0( 입력없음)이면 첫번째 맵이동 포인트로 이동
	if( FloatToInt( x ) == 0 && FloatToInt( y ) == 0 )
	{
		unsigned short mapDataNum = mapNumber;
		switch( NETWORK2->GetServerType() )
		{
		case _E_ST_ID_PVP_:
		case _E_ST_ID_THEME_:
		case _E_ST_ID_TUTORIAL_:
			{
				mapDataNum = NETWORK2->GetInDunMapNum();	
			}
			break;
		}

		tPointerArray<void*>* ary = (tPointerArray<void*>*)STAGESCRIPT->GetMapChangeDestArr( mapDataNum );
		if( ary->GetSize() == 0 )
			return false;

		sStageChangePos* pScript = (sStageChangePos*)((*ary)[0]);
		if( pScript == NULL )
			return false;

		targetPos.mPosX = pScript->mPosX;
		targetPos.mPosY = pScript->mPosY;
	}

	/// 이동 지역 설정
	SetMapTargetPos( targetPos );

	InfluenceProcessStop();

	mMapChange = true;
	return true;

}

bool cPlayer::InDunMapChange( unsigned short mapNumber, unsigned short mapDataNumber, float x, float y )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
		return false;

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return false;

	/// 이동 좌표 오류
	if( mapNumber > INDUN_MAP_MAX )
		return false;

	/// 이동 위치 체크
	float mapMaxSize = AIMANAGER->GetPathMaxSideSize( mapDataNumber );
	if( x < 0 || x > mapMaxSize || y < 0 || y > mapMaxSize )
		return false;

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return false;

	// 이동 좌표를 설정
	sTargetPos targetPos;
	targetPos.mMapNumber = mapNumber;
	targetPos.mPosX = x;
	targetPos.mPosY = y;
	targetPos.mRotAngle = 0;

	/// 이동 지역 설정
	SetMapTargetPos( targetPos );

	InfluenceProcessStop();

	mPvPDMIdx = mapNumber;

	mIsInDunJoin = true;
	mInDunMapDataNum = mapDataNumber;

	mMapChange = true;
	return true;
}

bool cPlayer::QuestMapChange( unsigned long posIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d,%d] mMapChange == true", mObject.index, posIdx);
		return false;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d,%d] IsRequestRejection == true", mObject.index, GetRequestRejection());
		return false;
	}

	/// NPC 대화상태일때 강제로 풀기
	if( GetState() == eOBJECT_STATE_STOP && GetStateStop() == eSTOP_NPCSPEECH )
	{
		/// 대화중인 npc인덱스를 제거
		SetNpcIdx( 0 );

		if( ChangeState( eOBJECT_STATE_IDLE ) == true )
		{
			SetStateStop( eSTOP_NONE );
			NETWORK2->SendMsgRoot( mConnectionIdx, NM_NPC, NM_NPC_CLOSEFORCE_RES );
		}
	}

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d] IsChangeState( eOBJECT_STATE_MOVE ) == false", mObject.index );
		return false;
	}

	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( posIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d] scriptPos == NULL", mObject.index );
		return false;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d] scriptPos.mMapNumber == 0", mObject.index );
		return false;
	}

	sTargetPos targetPos;
	::memset( &targetPos, 0, sizeof(targetPos) );

	targetPos.mMapNumber = scriptPos->mMapNumber;
	targetPos.mRotAngle = scriptPos->mRotAngle;

	bool result = false;
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
		if( scriptPos->mRange > MAP_RANGE_CHECK_RING )
		{
			for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
			{
				/// 최대 범위에서 20%씩 줄여가면서 체크
				int range = (int)(scriptPos->mRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
				if( range <= 0 ) 
					continue;

				targetPos.mPosX = scriptPos->mPosX + ( rand() % range ) - ( rand() % range );
				targetPos.mPosY = scriptPos->mPosY + ( rand() % range ) - ( rand() % range );

				if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == false )
					continue;

				result = true;
				break;
			}
		}
	}

	if( result == false )
	{
		/// 위에 랜덤 범위 좌표중 모두다 못가는 경우 스크립트상 정해진 고정위치로 이동시킨다.
		targetPos.mPosX = scriptPos->mPosX;
		targetPos.mPosY = scriptPos->mPosY;
	}

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		// 고정좌표로 한번 더 검사
		if( result == false )
		{
			if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == false )
			{
				NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d] return false", mObject.index );
				return false;
			}
		}

		// 
		mMapChange = true;

		/// 이동 지역 설정
		SetMapTargetPos( targetPos );

		InfluenceProcessStop();

		//
		SetStateStop( eSTOP_MAPCHANGE );

		/// 맵이동시 hp 10인경우 로그남김
		if( GetHP() == 10 )
		{
			NETWORK2->PostServerEvent("cPlayer::QuestMapChange hero->GetHP(%d,%d,%d,%d %d,%d,%.2f,%.2f) == 10", 
												mObject.index, GetMaxHP(), GetMP(), GetMaxMP(),
												NETWORK2->GetServerType(), mMapNumber, mObjectPos.x, mObjectPos.y );
		}
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
	{
		if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
			return false;

		mIsInDunJoin = false;
		mMapChange = false;
		mMapNumber = targetPos.mMapNumber;
		mGotoX = mObjectPos.x = targetPos.mPosX;
		mGotoY = mObjectPos.y = targetPos.mPosY;
		mDirection = targetPos.mRotAngle;

		CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
	{
		NETWORK2->PostServerEvent("cPlayer::QuestMapChange[%d] NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_", mObject.index );
		return false;
	}
	else
		return false;

	return true;
}

// 튜토리얼을 종료하고 기본 맵으로 돌아간다
bool cPlayer::MapChangeByTutorial( unsigned long posIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::MapChangeByTutorial[%d,%d] mMapChange == true", mObject.index, posIdx );
		return false;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
	{
		NETWORK2->PostServerEvent("cPlayer::MapChangeByTutorial[%d,%d] IsRequestRejection == true", mObject.index, GetRequestRejection());
		return false;
	}

	/// NPC 대화상태일때 강제로 풀기
	if( GetState() == eOBJECT_STATE_STOP && GetStateStop() == eSTOP_NPCSPEECH )
	{
		/// 대화중인 npc인덱스를 제거
		SetNpcIdx( 0 );

		if( ChangeState( eOBJECT_STATE_IDLE ) == true )
		{
			SetStateStop( eSTOP_NONE );
			NETWORK2->SendMsgRoot( mConnectionIdx, NM_NPC, NM_NPC_CLOSEFORCE_RES );
		}
	}
	
	/// 맵이동 가능인지
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
	{
		NETWORK2->PostServerEvent("cPlayer::MapChangeByTutorial[%d] IsChangeState( eOBJECT_STATE_MOVE ) == false", mObject.index );
		return false;
	}

	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( posIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::MapChangeByTutorial[%d] scriptPos == NULL", mObject.index );
		return false;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cPlayer::MapChangeByTutorial[%d] scriptPos.mMapNumber == 0", mObject.index );
		return false;
	}

	sTargetPos targetPos;
	::memset( &targetPos, 0, sizeof(targetPos) );

	targetPos.mMapNumber = scriptPos->mMapNumber;
	targetPos.mRotAngle = scriptPos->mRotAngle;
	targetPos.mPosX = scriptPos->mPosX;
	targetPos.mPosY = scriptPos->mPosY;

	if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
		return false;

	InfluenceProcessStop();

	//
	SetStateStop( eSTOP_MAPCHANGE );

	mIsInDunJoin = false;
	mMapChange = false;
	mMapNumber = targetPos.mMapNumber;
	mGotoX = mObjectPos.x = targetPos.mPosX;
	mGotoY = mObjectPos.y = targetPos.mPosY;
	mDirection = targetPos.mRotAngle;

	ReleaseInfluenceSet( true );

	/// 이동 지역 설정
	SetMapTargetPos( targetPos );

	CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );

	return true;

}

bool cPlayer::GMMapChangeByTutorial( unsigned short mapNum, float x, float y )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::GMMapChangeByTutorial[%d,%d] mMapChange == true", mObject.index );
		return false;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
	{
		NETWORK2->PostServerEvent("cPlayer::GMMapChangeByTutorial[%d,%d] IsRequestRejection == true", mObject.index, GetRequestRejection());
		return false;
	}

	/// NPC 대화상태일때 강제로 풀기
	if( GetState() == eOBJECT_STATE_STOP && GetStateStop() == eSTOP_NPCSPEECH )
	{
		/// 대화중인 npc인덱스를 제거
		SetNpcIdx( 0 );

		if( ChangeState( eOBJECT_STATE_IDLE ) == true )
		{
			SetStateStop( eSTOP_NONE );
			NETWORK2->SendMsgRoot( mConnectionIdx, NM_NPC, NM_NPC_CLOSEFORCE_RES );
		}
	}

	/// 맵이동 가능인지
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
	{
		NETWORK2->PostServerEvent("cPlayer::GMMapChangeByTutorial[%d] IsChangeState( eOBJECT_STATE_MOVE ) == false", mObject.index );
		return false;
	}

	sTargetPos targetPos;
	::memset( &targetPos, 0, sizeof(targetPos) );

	targetPos.mMapNumber = mapNum;
	targetPos.mRotAngle = 0;
	targetPos.mPosX = x;
	targetPos.mPosY = y;

	if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
		return false;

	InfluenceProcessStop();

	//
	SetStateStop( eSTOP_MAPCHANGE );

	mIsInDunJoin = false;
	mMapChange = false;
	mMapNumber = targetPos.mMapNumber;
	mGotoX = mObjectPos.x = targetPos.mPosX;
	mGotoY = mObjectPos.y = targetPos.mPosY;
	mDirection = targetPos.mRotAngle;

	ReleaseInfluenceSet( true );

	/// 이동 지역 설정
	SetMapTargetPos( targetPos );

	CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );

	return true;
}

bool cPlayer::IsItemMapChange( unsigned long posIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::IsItemMapChange[%d,%d] mMapChange == true", mObject.index, posIdx);
		return false;
	}

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
	{
		NETWORK2->PostServerEvent("cPlayer::IsItemMapChange[%d] IsChangeState( eOBJECT_STATE_MOVE ) == false", mObject.index );
		return false;
	}

    if( IsAutoTrial() == true )
    {
		NETWORK2->PostServerEvent("cPlayer::IsItemMapChange[%d] IsAutoTrial() == true", mObject.index );
        return false;
    }

	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( posIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::IsItemMapChange[%d] scriptPos == NULL", mObject.index );
		return false;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cPlayer::IsItemMapChange[%d] scriptPos.mMapNumber == 0", mObject.index );
		return false;
	}

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		if( AIMANAGER->IsPossible( scriptPos->mMapNumber, scriptPos->mPosX, scriptPos->mPosY, mObject ) == true )
			return true;
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
		return true;
	else if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
		return false;
	else
		return false;

	return false;
}

bool cPlayer::ItemMapChange( unsigned long posIdx, unsigned long useItemIdx )
{	
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d,%d] mMapChange == true", mObject.index, posIdx);
		return false;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return false;

	/// 맵이동 가능인지 현재 상태 체크
	if( GetState() != eOBJECT_STATE_ATTACK )
	{
		if( GetState() != eOBJECT_STATE_DIE )
			NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] ChangeState( eOBJECT_STATE_MOVE ) == false[%d,%d,%d]",
			mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mIsRequestRejection, mObject.index );
		return false;
	}

    if( IsAutoTrial() == true )
    {
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d,%d] IsAutoTrial() == true", mObject.index, posIdx);
        return false;
    }

	sTargetPos targetPos;
	memset( &targetPos, 0 , sizeof(sTargetPos) );
	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( posIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] scriptPos == NULL", mObject.index );
		return false;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] scriptPos.mMapNumber == 0", mObject.index );
		return false;
	}

	targetPos.mMapNumber = scriptPos->mMapNumber;
	targetPos.mRotAngle = scriptPos->mRotAngle;

	bool isPos = false;
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
		if( scriptPos->mRange > MAP_RANGE_CHECK_RING )
		{
			for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
			{
				/// 최대 범위에서 20%씩 줄여가면서 체크
				int range = (int)(scriptPos->mRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
				if( range <= 0 ) 
					continue;

				targetPos.mPosX = scriptPos->mPosX + ( rand() % range ) - ( rand() % range );
				targetPos.mPosY = scriptPos->mPosY + ( rand() % range ) - ( rand() % range );

				if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == false )
					continue;

				isPos = true;
				break;
			}
		}
	}

	if( isPos == false )
	{
		/// 위에 랜덤 범위 좌표중 모두다 못가는 경우 스크립트상 정해진 고정위치로 이동시킨다.
		targetPos.mPosX = scriptPos->mPosX;
		targetPos.mPosY = scriptPos->mPosY;
	}


	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == true )
		{
			if( ItemUseMapChange( useItemIdx ) == false )
				return false;

			mMapChange = true;

			/// 이동 지역 설정
			SetMapTargetPos( targetPos );

			InfluenceProcessStop();

			return true;
		}
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
	{
		if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
			return false;

		if( ItemUseMapChange( useItemIdx ) == false )
			return false;

		mIsInDunJoin = false;
		mMapChange = false;
		mMapNumber = targetPos.mMapNumber;
		mGotoX = mObjectPos.x = targetPos.mPosX;
		mGotoY = mObjectPos.y = targetPos.mPosY;
		mDirection = targetPos.mRotAngle;

		CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );

		return true;
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] NETWORK2->GetServerType(%d) == _E_ST_ID_TUTORIAL_", mObject.index, NETWORK2->GetServerType() );
		return false;
	}
	else
		return false;

	/// 랜덤이 포함안된 고정 위치로도 이동 못하면 실패 리턴
	return false;
}

char cPlayer::UserPortalMapChange( unsigned long userPortalIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] mMapChange == true", mObject.index );
		return ERROR_RES_MAPCHANGE_STATE;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return ERROR_RES_MAPCHANGE_STATE;

	/// 맵이동 가능인지 현재 상태 체크
	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return ERROR_RES_MAPCHANGE_STATE;

	cUserPortal* pPortal = GRIDMANAGER->GetUserPortal( userPortalIdx );
	if( pPortal == NULL )
		return ERROR_RES_MAPCHANGE_NOTPORTAL;

	if( pPortal->IsMapChange( mObject.index ) == false )
		return ERROR_RES_MAPCHANGE_NOTPARTY;

	sTargetPos targetPos;
	memset( &targetPos, 0 , sizeof(sTargetPos) );
	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( pPortal->GetMapChangePosIdx() );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] scriptPos == NULL", mObject.index );
		return ERROR_RES_MAPCHANGE_SCRIPT;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] scriptPos.mMapNumber == 0", mObject.index );
		return ERROR_RES_MAPCHANGE_SCRIPT;
	}

	float objectSize = OBJECTMANAGER->ObjectSizeRange( this, NULL, 0 );
	mRangeCheck.SetRadius( scriptPos->mRange + objectSize + SYNC_MOVE_RANGE );
	if( mRangeCheck.IsNotRange( pPortal->GetPos(), GetPos() ) == true )
		return ERROR_RES_MAPCHANGE_POSITION;

	targetPos.mMapNumber = scriptPos->mMapNumber;
	targetPos.mRotAngle = scriptPos->mRotAngle;

	bool isPos = false;
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
		if( scriptPos->mRange > MAP_RANGE_CHECK_RING )
		{
			for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
			{
				/// 최대 범위에서 20%씩 줄여가면서 체크
				int range = (int)(scriptPos->mRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
				if( range <= 0 ) 
					continue;

				targetPos.mPosX = scriptPos->mPosX + ( rand() % range ) - ( rand() % range );
				targetPos.mPosY = scriptPos->mPosY + ( rand() % range ) - ( rand() % range );

				if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == false )
					continue;

				isPos = true;
				break;
			}
		}
	}

	if( isPos == false )
	{
		/// 위에 랜덤 범위 좌표중 모두다 못가는 경우 스크립트상 정해진 고정위치로 이동시킨다.
		targetPos.mPosX = scriptPos->mPosX;
		targetPos.mPosY = scriptPos->mPosY;
	}

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		if( AIMANAGER->IsPossible( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, mObject ) == true )
		{
			mMapChange = true;

			/// 이동 지역 설정
			SetMapTargetPos( targetPos );

			InfluenceProcessStop();

			return ERROR_RES_MAPCHANGE_SUCCESS;
		}
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
	{
		if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
			return ERROR_RES_MAPCHANGE_STATE;

		mIsInDunJoin = false;
		mMapChange = false;
		mMapNumber = targetPos.mMapNumber;
		mGotoX = mObjectPos.x = targetPos.mPosX;
		mGotoY = mObjectPos.y = targetPos.mPosY;
		mDirection = targetPos.mRotAngle;

		CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );

		return ERROR_RES_MAPCHANGE_SUCCESS;
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange[%d] NETWORK2->GetServerType(%d) == _E_ST_ID_THEME_", mObject.index, NETWORK2->GetServerType() );
		return ERROR_RES_MAPCHANGE_SCRIPT;
	}
	else
		return ERROR_RES_MAPCHANGE_STATE;

	/// 랜덤이 포함안된 고정 위치로도 이동 못하면 실패 리턴
	return ERROR_RES_MAPCHANGE_STATE;
}

bool cPlayer::CashItemMapChange( unsigned short mapNumber, float x, float y, unsigned long useItemIdx )
{
	/// 맵이동중 맵이동이 올수없음
	if( mMapChange == true )
	{
		NETWORK2->PostServerEvent("cPlayer::CashItemMapChange mMapChange[%d,%d,%f,%f] == true", mObject.index, mapNumber, x, y);
		return false;
	}

	/// 2차상태 체크
	if( IsRequestRejection() == true )
		return false;

	/// 맵이동 가능인지 현재 상태 체크
	if( GetState() != eOBJECT_STATE_ATTACK )
	{
		if( GetState() != eOBJECT_STATE_DIE )
			NETWORK2->PostServerEvent("cPlayer::CashItemMapChange[%d] GetState() != eOBJECT_STATE_ATTACK[%d,%d,%d]",
			mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mIsRequestRejection, mObject.index );
		return false;
	}

	if( IsAutoTrial() == true )
	{
		NETWORK2->PostServerEvent("cPlayer::ItemMapChange IsAutoTrial()[%d,%d,%f,%f] == true", mObject.index, mapNumber, x, y);
		return false;
	}

	if( mapNumber < MAP_MIN || MAP_MAX < mapNumber )
	{
		NETWORK2->PostServerEvent("cPlayer::CashItemMapChange MAP_MIN > mapNumber || mapNumber < MAP_MAX[%d,%d,%f,%f]", mObject.index, mapNumber, x, y );
		return false;
	}

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		if( AIMANAGER->IsPossible( mapNumber, x, y, mObject ) == true )
		{
			mMapChange = true;

			/// 이동 지역 설정
			mMapTargetPos.mMapNumber = mapNumber;
			mMapTargetPos.mPosX = x;
			mMapTargetPos.mPosY = y;
			mMapTargetPos.mRotAngle = 0;

			InfluenceProcessStop();

			ItemUseCashMapChange( useItemIdx );

			return true;
		}
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
	{
		if( NETWORK2->ReturnGameServer( GetConnectionIdx() ) == false )
			return false;

		ItemUseCashMapChange( useItemIdx );

		mIsInDunJoin = false;
		mMapChange = false;
		mMapNumber = mapNumber;
		mGotoX = mObjectPos.x = x;
		mGotoY = mObjectPos.y = y;
		mDirection = 0;

		CalcThemeReturnGameSrv( mMapNumber, mObjectPos.x, mObjectPos.y, false );

		return true;
	}
	else if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
	{
		NETWORK2->PostServerEvent("cPlayer::CashItemMapChange[%d] NETWORK2->GetServerType(%d) == _E_ST_ID_TUTORIAL_", mObject.index, NETWORK2->GetServerType() );
		return false;
	}
	else
		return false;

	return false;
}

void cPlayer::MoveStopReq( MSG_REQ_MOVE_STOP* pmsg, char reqProtocol, char resProtocol )
{
	if( pmsg == NULL )
	{
		mIsMoveMsg = false;

		// 서버상의 좌표로 클라이언트를 옮기게 함
		HANDLE handle = NULL;
		MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, resProtocol );
		if ( msg != NULL )
		{
			msg->mCharacterPosX = mObjectPos.x;
			msg->mCharacterPosY = mObjectPos.y;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
		}
		return;
	}

	NiPoint2 clientPos( pmsg->mClientPosX, pmsg->mClientPosY );

	MoveStop();
	if( GetState() == eOBJECT_STATE_MOVE )
		ChangeState( eOBJECT_STATE_IDLE );

	CalcClientMoveDeff( pmsg->mClientTime );

	mIsMoveMsg = true;
	mMoveMsg.mMsgError = false;
	mMoveMsg.mProtocol = reqProtocol;

	/// 오차 허용치 확인
	cRangeCheck rangeCheck( SYNC_MOVE_RANGE );
	if( rangeCheck.IsNotRange( clientPos, mObjectPos ) == true )
	{
		mMoveMsg.mMsgError = true;
		//// 서버상의 좌표로 클라이언트를 옮기게 함
		//HANDLE handle = NULL;
		//MSG_RES_MOVE_STOP* msg = (MSG_RES_MOVE_STOP*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, resProtocol );
		//if ( msg != NULL )
		//{
		//	msg->mCharacterPosX = mObjectPos.x;
		//	msg->mCharacterPosY = mObjectPos.y;
		//	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_MOVE_STOP) );
		//}
	}

	//if( GetCheatHideMode() == false )
	//{
	//	// 다른 플레이어들에게도 알림
	//	MSG_SYN_MOVE_STOP Msg;
	//	Msg.Category = NM_PLAYER;
	//	Msg.Protocol = synProtocol;
	//	Msg.mCharacterIdx = mObject.index;
	//	Msg.mCharacterPosX = GetXPos();
	//	Msg.mCharacterPosY = GetYPos();
	//	NETWORK2->QuickSendExcept( this, (char*)&Msg, sizeof(Msg) );
	//}
}

void cPlayer::SendMoveSpeed()
{
	MSG_SYN_MOVESPEED Msg;
	Msg.Category     = NM_PLAYER;
	Msg.Protocol     = NM_PLAYER_MOVESPEED_SYN;
	Msg.mCharacterIdx = mObject.index;
	Msg.moveSpeed    = GetMoveSpeed();
	NETWORK2->QuickSend( this, (char*)&Msg, sizeof(Msg) );
}

void  cPlayer::SendAttackSpeed()
{
	MSG_SYN_ATTACKSPEED Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_ATTACKSPEED_SYN;
	Msg.mCharacterIdx = mObject.index;
	Msg.mAttackSpeed = mStatus2.mAttackSpeed;
	NETWORK2->QuickSend( this, (char*)&Msg, sizeof(Msg) );
}

void cPlayer::SendPlayerSize()
{
	MSG_SYN_PLAYER_SIZE synMsg;
	synMsg.Category = NM_PLAYER;
	synMsg.Protocol = NM_PLAYER_SCALE_SYN;
	synMsg.mCharacterIdx = mObject.index;
	synMsg.mScale = GetFixedObjectSizePer();
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
}

//void cPlayer::SendSynStatus()
//{
//	HANDLE                handle  = NULL;
//	MSG_SYN_PLAYER_STATUS* sendMsg = (MSG_SYN_PLAYER_STATUS*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx );
//	if ( sendMsg != NULL )
//	{
//		sendMsg->Category = NM_PLAYER;
//		sendMsg->Protocol = NM_PLAYER_STATUS_RES;
//		sendMsg->mCharacterIdx = mObject.index;
//		sendMsg->mScale = mPlayerExrInfo.mFixedObjectSizePer;
//		sendMsg->mMoveSpeed = GetMoveSpeed();
//		sendMsg->mAttackSpeed = mStatus2.mAttackSpeed;
//		sendMsg->mMonsterIdx = mPlayerExrInfo.mChgMonsterIdx;
//		sendMsg->mIsSleep = mIsSleep;
//		sendMsg->mIsStun = mIsStun;
//		sendMsg->mVehicleIdx = mPlayerExrInfo.mVehicleIdx;
//		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_STATUS) );
//	}
//
//	if( sendMsg->mMonsterIdx != 0 )
//		SendCoolTimeChgMon();
//}

void cPlayer::SetGameIn()
{ 
	mGameIn = true; 
	mNextRecoveryTime = NETWORK2->GetAccumTime(); 

	/// 일괄 저장 시간 설정
	mBatchSaveTime = NETWORK2->GetAccumTime() + BATCH_SAVE_TIME;

	/// 061208 PKH 이동시 목적지 좌표
	mRangeTargetPos.x = mGotoX = mObjectPos.x;
	mRangeTargetPos.y = mGotoY = mObjectPos.y;

	mRange = 0;

	mNpcIdx = 0;

	mIsSkillResurrection = false;
	mSkillResurrectionHP = 0;
	mSkillResurrectionMP = 0;

	mNextRecoveryTime = NETWORK2->GetAccumTime() + NATURE_RECOVERY_TIME;

	mCommunitySkillEndTime = 0;

	mServerMoveTime = 0;
	mClientMoveDefTime = 0;
	mClientMovePlusMinus = 0;

	mKeyboardMove = false;

	mJumpEndTime = 0;

	mPvPDieEndTime = 0;
	mPvPDieTime = 0;

	mMapNum0Save = false;

	mIsMoveMsg = false;
	memset( &mMoveMsg, 0, sizeof(mMoveMsg) );

	mJumpMoveSpeedDownEndTime = 0;
	mJumpSpeedState = 0;

}

bool cPlayer::IsGameFinish()
{
	if( GetRequestRejection() != eREQREJCT_GAMEFINISH )
		return false;

	if( GetState( ) != eOBJECT_STATE_STOP )
		return false;

	if( GetStateStop( ) != eSTOP_GAMEFINISH )
		return false;

	return true;
}

void cPlayer::CalcNatureRecovery()
{
	/// 게임 밖이면 자연회복 안함
	if( mGameIn == false )
		return;

	/// 죽었으면 회복 안함
	if( GetStateDie() == true )
		return;

	assert( mPlayerExrInfo.RestHP );

	/// 스텟 계산 전이면 자연회복 안함
	if( GetMaxHP() == 0 )
	{
		NETWORK2->PostServerEvent("0 divide 9");
		return;
	}

	if( mNextRecoveryTime > NETWORK2->GetAccumTime() )
		return;

	/// 앉았는지 체크
	float rate = 1.0f;
	if( mPlayerExrInfo.mState == ePLAYER_STATE_SITDOWN )
	{
		/// 회복
		rate = STATUSSCRIPT->GetDamageCalcNumericalInfo( 10 );
		mNextRecoveryTime = mNextRecoveryTime + SITDOWN_RECOVERY_TIME;
	}
	else
	{
		/// 회복
		mNextRecoveryTime = mNextRecoveryTime + NATURE_RECOVERY_TIME;
	}

	unsigned long recovHP = FloatToInt( mStatus2.mRecovHP * rate );
	unsigned long recovMP = FloatToInt( mStatus2.mRecovMP * rate );

	if( recovHP != 0 )
		HPHeal( recovHP, true, mObject, 0, eTAKEDAMAGETYPE_NONE );

	if( recovMP != 0 )
		MPHeal( recovMP, true );
}

void cPlayer::ApplyInitRest()
{
	if( mInitRestHP != 0 && mInitRestMP != 0 )
	{
		if( mInitRestHP > mPlayerExrInfo.RestHP )
		{
			unsigned long hp = mInitRestHP - mPlayerExrInfo.RestHP;
			HPHeal( hp, true, mObject, 0, eTAKEDAMAGETYPE_NONE );
		}

		if( mInitRestMP > mPlayerExrInfo.RestMP )
		{
			unsigned long mp = mInitRestMP - mPlayerExrInfo.RestMP;
			MPHeal( mp, true );
		}

		mInitRestHP = 0;
		mInitRestMP = 0;
	}
}

void cPlayer::SetInitRestHP()
{
	mInitRestHP = mPlayerExrInfo.RestHP;
}

void cPlayer::SetInitRestMP()
{
	mInitRestMP = mPlayerExrInfo.RestMP;
}

void cPlayer::SetCommunitySkill( unsigned long communitySkillIdx )
{
	/// 생활 스킬 스크립트에서 읽어온 사용 시간 반영
	mPlayerExrInfo.mCommunitySkillIdx = communitySkillIdx;
	float skillTime = COMMUNITYSCRIPT->GetEmotingMaintainTime( communitySkillIdx, mPlayerInfo.Race, mPlayerInfo.Gender );

	mCommunitySkillEndTime = NETWORK2->GetAccumTime() + skillTime;
}

unsigned char cPlayer::SelectFollowPos( unsigned char followPos )
{
	/// 요청한자가 이미 중심 위치면 리턴
	if( followPos == FOLLOW_POS_CENTER )
		return followPos;

	/// 현재 추적위치를 가지고 있는 몬스터가 (중심자리)요청을 한경우
	if( mFollowPos[FOLLOW_POS_CENTER] == false )
	{
		/// 자신이 사용중이던 자리 반환
		if( followPos != FOLLOW_POS_MAX )
			mFollowPos[followPos] = false;

		/// 센터 설정후 리턴
		mFollowPos[FOLLOW_POS_CENTER] = true;
		return FOLLOW_POS_CENTER;            
	}

	/// 기존 자리를 배당 받은 요청자는 센터만 체크하고 리턴
	if( followPos != FOLLOW_POS_MAX )
		return followPos;

	/// 위치가 지정되지 않은 몬스터들의 위치 선정
	///unsigned char selectPos[FOLLOW_POS_MAX] = { 4, 0, 8, 6, 2, 7, 1, 3, 5 };
	unsigned char selectPos[FOLLOW_POS_MAX] = { 4, 0, 2, 8, 6, 1, 5, 7, 3 };

	/// 전체 자리 검사
	for( unsigned char i = 0 ; i < FOLLOW_POS_MAX ; ++i )
	{
		/// 순서에 맞게 위치 빈자리 확인
		if( mFollowPos[selectPos[i]] == false )
		{
			/// 빈자리 설정
			mFollowPos[selectPos[i]] = true;
			return selectPos[i];
		}
	}

	/// 빈자리가 하나도 없는 경우 센터 자리 설정
	return FOLLOW_POS_MAX;
}

void cPlayer::UnSelectFollowPos( unsigned char followPos )
{
	/// 사용 플레그 끔
	if( followPos != FOLLOW_POS_MAX )
		mFollowPos[followPos] = false;
}

void cPlayer::DuelEnd()
{
	if( mDuelPlayerIdx == 0 )
		return;

	if( mIsDuelRequester == true )
		DUELMANAGER->DuelObjectEnd( mObject.index, mDuelPlayerIdx, mDuelIdx, true );
	else
		DUELMANAGER->DuelObjectEnd( mDuelPlayerIdx, mObject.index, mDuelIdx, false );
}

void cPlayer::DuelEndDebuffClear( unsigned long enemyIdx )
{
	cSkillInfluenceSet::cConstIterator start = mInfluenceSet.Begin();
	cSkillInfluenceSet::cConstIterator end = mInfluenceSet.End();

	unsigned long influenceIdx;
	cInfluenceObject* pInfluence;
	sInfluenceScript* pScript;
	while( start != end )
	{
		influenceIdx = *start++;
		pInfluence = SKILLMANAGER->GetInfluence( influenceIdx );
		if( pInfluence == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::DuelEndDebuffClear pInfluence == NULL");
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		pScript = SKILLSCRIPT->GetInfluenceInfo( pInfluence->GetInfluenceClassIdx() );
		if( pScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::DuelEndDebuffClear pScript == NULL");
			continue;
		}

		/// 디버프만 제거
		if( pScript->mType != eINFLUENCETYPE_DEBUF )
			continue;

		/// 결투 상대편이 걸어준 디버프만 제거
		if( pInfluence->GetAttacker().index != enemyIdx )
			continue;

		/// 스킬 메니저에서 객체 삭제
		SKILLMANAGER->DeleteInfluenceList( influenceIdx );
	}
}

void cPlayer::CalcPvPReturnGameSrv( unsigned short mapNumber, float beforePosX, float beforePosY )
{
	mGameSrvReturnMapNum = mapNumber;
	mGameSrvReturnPos.x = beforePosX;
	mGameSrvReturnPos.y = beforePosY;
	
	NiPoint2 calcPos = STAGESCRIPT->CalcNearTargetMapPos( mapNumber, beforePosX, beforePosY );
	if( calcPos != NiPoint2::ZERO )
		mGameSrvReturnPos = calcPos;
}

void cPlayer::ChannelMovePvPWait( long pvpGoCid, unsigned long waitTime )
{
	mChannelPvPGoCid = pvpGoCid;
	mChannelPvPMoveTime = NETWORK2->GetAccumTime() + waitTime;
}

bool cPlayer::CheckPvPJoin()
{
	/// pvp 진입 불가 효과를 가진 사람 체크

	cSkillInfluenceSet::cIterator infBegin = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator infEnd = mInfluenceSet.End();

	while( infBegin != infEnd )
	{
		cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( (*infBegin++) );
		if( pInf != NULL && pInf->GetInfluenceClassIdx() == PVP_DM_CANTPVP_INFLUENCE )
			return false;
	}

	return true;
}

void cPlayer::PvPOut()
{
	mIsInDunJoin = false;
	mPvPDMIdx = 0;

	/// 무적 효과 제거
	SKILLMANAGER->DeleteInfluenceClassIdx( mObject, PVP_DM_NODAMAGE_INFLUENCE );

	/// 수장 효과 제거
	SKILLMANAGER->DeleteInfluenceClassIdx( mObject, PVP_DM_LEADER_INFLUENCE );
	SKILLMANAGER->DeleteInfluenceClassIdx( mObject, PVP_DM_LEADER_INFLUENCE2 );
	SKILLMANAGER->DeleteInfluenceClassIdx( mObject, PVP_DM2_FLAG_INFLUENCE );

	HPHeal( GetMaxHP(), true, mObject, 0, eTAKEDAMAGETYPE_NONE );
	MPHeal( GetMaxMP(), true );
}

void cPlayer::AutoPvPOut()
{
	PvPOut();
	SetStateStop( eSTOP_MAPCHANGE );

	/// 마을이동 알림
	NETWORK2->SendMsgRoot( mConnectionIdx, NM_PVP, NM_PVP_DM_END_SYN );
}

bool cPlayer::SendPvPTeamChat( char* msg, int length )
{
	cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
	if( pDM != NULL )
	{
		pDM->TeamChatQuickSend( this, (ePVPDM_TEAM_TYPE)mPlayerExrInfo.mPvPDMTeamType, msg, length );
		return true;
	}

	return false;
}

void cPlayer::PvPEndDebuffClear()
{
	cSkillInfluenceSet::cConstIterator start = mInfluenceSet.Begin();
	cSkillInfluenceSet::cConstIterator end = mInfluenceSet.End();

	unsigned long influenceIdx;
	cInfluenceObject* pInfluence;
	sInfluenceScript* pScript;
	while( start != end )
	{
		influenceIdx = *start++;
		pInfluence = SKILLMANAGER->GetInfluence( influenceIdx );
		if( pInfluence == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::DuelEndDebuffClear pInfluence == NULL");
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		pScript = SKILLSCRIPT->GetInfluenceInfo( pInfluence->GetInfluenceClassIdx() );
		if( pScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::DuelEndDebuffClear pScript == NULL");
			continue;
		}

		/// 디버프만 제거
		if( pScript->mType != eINFLUENCETYPE_DEBUF )
			continue;

		/// 스킬 메니저에서 객체 삭제
		SKILLMANAGER->DeleteInfluenceList( influenceIdx );
	}
}

void cPlayer::CalcThemeReturnGameSrv( unsigned short beforeMapNumber, float beforePosX, float beforePosY, bool isAuto )
{
	mGameSrvReturnMapNum = beforeMapNumber;
	mGameSrvReturnPos.x = beforePosX;
	mGameSrvReturnPos.y = beforePosY;

	if( isAuto == true )
	{
		cThemeObject* ptheme = THEMEMANAGER->GetThemeObject( mThemeRoomIdx );
		if( ptheme != NULL )
		{
			sStageChangePos* pCalcPos = ptheme->GetThemeOutPos();
			if( pCalcPos != NULL && pCalcPos->mTargetInfo != NULL )
			{
				mGameSrvReturnMapNum = pCalcPos->mTargetInfo->mMapNumber;
				mGameSrvReturnPos.x = pCalcPos->mTargetInfo->mPosX;
				mGameSrvReturnPos.y = pCalcPos->mTargetInfo->mPosY;
			}
		}
	}
}

void cPlayer::AddTakeDamage( sObject attacker, unsigned long damage, long /*distressPoint*/, eTAKEDAMAGE_TYPE /*type*/ )
{
	if( attacker.type == eOBJECTTYPE_PLAYER )
	{
		cPlayer* pAttacker = GRIDMANAGER->GetPlayer( attacker.index );
		if( pAttacker == NULL )
			return;

		if( GetStateDie() == true && mPvPDMIdx != 0 && damage !=0 )
		{
			cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
			if( pDM != NULL )
			{
				pDM->ScoreUpdate( attacker.index, (ePVPDM_TEAM_TYPE)pAttacker->GetPvPDMTeam(), 
					mObject.index, (ePVPDM_TEAM_TYPE)mPlayerExrInfo.mPvPDMTeamType );
			}
		}
	}
}

bool cPlayer::IsAttack( unsigned long targetIdx )
{
	if( mObject.index == targetIdx )
		return false;

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( targetIdx );
	if( pTarget == NULL )
		return false;

	/// 결투
	if( mDuelIdx != 0 )
	{
		cDuelObject* pDuelObject = DUELMANAGER->GetDuel( mDuelIdx );
		if( pDuelObject == NULL )
			return false;

		if( pDuelObject->GetDuelProcess() != eDUELPROCESS_FIGHT ) 
			return false;

		if( !( mObject.index == pDuelObject->GetAttackerIdx() || mObject.index == pDuelObject->GetTargetIdx() ) )
			return false;
		if( !( targetIdx == pDuelObject->GetAttackerIdx() || targetIdx == pDuelObject->GetTargetIdx() ) )
			return false;

		return true;
	}

	/// PvP1
	if( mPvPDMIdx != 0 )
	{
		cBaseDeathMatch* pPvPDM = PVPMANAGER->GetPvPDMObject( mPvPDMIdx );
		if( pPvPDM == NULL )
			return false;

		if( pPvPDM->IsFight() == false )
			return false;

		if( mPvPDMIdx == pTarget->GetPvPDMIdx() && mPlayerExrInfo.mPvPDMTeamType != pTarget->GetPvPDMTeam() )
			return true;
	}		

	return false;
}

bool cPlayer::IsRequestRejection()
{ 
	if( mIsRequestRejection != eREQREJCT_NONE )
		return true;

	return false; 
}

void cPlayer::StartRequestRejection( eREQUEST_REJECTION IsRequestRejection )
{
	if( mIsRequestRejection != eREQREJCT_NONE )
	{
		NETWORK2->PostServerEvent("cPlayer::StartRequestRejection[%d,%d,%d] member[%d] input[%d]", 
			mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mObject.index, mIsRequestRejection, IsRequestRejection);
	}

	mIsRequestRejection = IsRequestRejection;
}

void cPlayer::EndRequestRejection( eREQUEST_REJECTION IsRequestRejection )
{
	if( mIsRequestRejection != IsRequestRejection )
	{
		NETWORK2->PostServerEvent("cPlayer::EndRequestRejection[%d,%d,%d] member[%d] input[%d]", 
			mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, mObject.index, mIsRequestRejection, IsRequestRejection );
	}

	/// 종료 취소 요청이 들어온경우 기본 상태로 변경
	if( mPlayerExrInfo.mState == eOBJECT_STATE_STOP && mPlayerExrInfo.mStateStop == eSTOP_GAMEFINISH && eREQREJCT_GAMEFINISH == IsRequestRejection )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );
	}

	mIsRequestRejection = eREQREJCT_NONE;
}

// 맵체인지, 게임종료, 죽었을시
void cPlayer::EndRequestRejectionAll()
{
	switch( mIsRequestRejection )
	{
	case eREQREJCT_NONE:
		return;
	case eREQREJCT_GAMEFINISH:
		break;
	case eREQREJCT_DUELWAIT:	/// 결투 요청 대기 상태
		{
			cPlayer* pRequestor = GRIDMANAGER->GetPlayer( mDuelPlayerIdx );	// 요청자
			if( pRequestor != NULL )
				DUELMANAGER->DuelAllReady( false, this );
		}
		break;
	case eREQREJCT_DUELPLAY:
		{
			DuelEnd();	// 결투 종료 처리
		}
		break;
	case eREQREJCT_EXCHANGE:	/// 거래 상태
		{
			cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );	// 요청자
			if ( player != NULL )
			{
				player->ExchangeReplySyn( TYPE_ITEM_EXCHANGE_REP_REJECT, ERROR_ITEM_EXCHANGE_REP_ERROR );
			}

			if ( mExchangeStatus == ItemExchangeAsk )
				mExchangeStatus = ItemExchangeNone;
		}
		break;
	case eREQREJCT_PARTY:		/// 파티 상태
		{
			PARTYMAN->ClearRequest( this );
		}
		break;
	case eREQREJCT_GUILD:		/// 길드 상태
		{
			GUILDMAN->ClearRequest( this );
		}
		break;
	case eREQREJCT_THEME:
		{
			THEMEMANAGER->ThemeDBErr( mObject.index, ERROR_THEME_READY_STATE );
		}
		break;
	default:
		return;
	}

	mIsRequestRejection = eREQREJCT_NONE;
}


unsigned long cPlayer::ExpDown( float minimum, float maximum )
{
	if( mPlayerInfo.Level < 10 )
		return 0;

	/// 현재 캐릭터 lv에서 다음 lv까지 얻어야 하는 총 경험치
	sExpTable* pExpTable = LEVELSCRIPT->GetExpTable( mPlayerInfo.Level );
	if( pExpTable == NULL )
	{
		NETWORK2->PostServerEvent("cPlayer::ExpDown pExpTable[%d][%f,%f][%d] == NULL", mObject.index, minimum, maximum, mPlayerInfo.Level );
		return 0;
	}

	/// 랜덤값 추출
	unsigned long randvalue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );

	/// 랜덤 감소 확률 선택 - 소수점 2자리 살림
	unsigned long downGap = (unsigned long)(( maximum * 100 ) - ( minimum * 100 ));

	/// 최소 최대 구간 사이의 랜덤값
	unsigned long downGapExp = (unsigned long)( pExpTable->mExp * ( ( randvalue % downGap ) * 0.01f ) * 0.01f );
	/// 최소 경험치 하락분
	unsigned long downMinExp = (unsigned long)( pExpTable->mExp * minimum * 0.01f );

	/// 하락할 경험치량
	unsigned long minusExp = downMinExp + downGapExp;

	/// 경험치 감소 가능 한 만큼 만 감소
	if( mHeroInfo.Exp < minusExp )
	{
		minusExp = mHeroInfo.Exp;		/// 실제 감소량 기록
		mHeroInfo.Exp = 0;				/// 경험치 감소
		return minusExp;				/// 감소량 리턴
	}
	else
	{	
		mHeroInfo.Exp = mHeroInfo.Exp - minusExp;	/// 경험치 감소
		return minusExp;							/// 감소량 리턴
	}
}

bool cPlayer::DBUpdate( bool updateComplete, int updateFlag )
{
	if ( updateComplete == false && mIsDbUpdate == true )
		return false;

	/// 다음 저장시간 설정
	mBatchSaveTime = NETWORK2->GetAccumTime() + BATCH_SAVE_TIME;
	/// 쌓여있는 경험치량 제거
	mWaitSaveExp = 0;

	HANDLE            handle          = NULL;
	CHARACTER_UPDATE* characterUpdate = (CHARACTER_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_UPDATE );

	characterUpdate->cid				= mConnectionIdx;
	characterUpdate->idx				= mPlayerInfo.CharacterIdx;
	characterUpdate->hp					= mPlayerExrInfo.RestHP;
	characterUpdate->mp					= mPlayerExrInfo.RestMP;
	characterUpdate->tarotPoint			= mHeroInfo.TarotPoint;
	characterUpdate->titleIndex			= mPlayerExrInfo.mTitleIndex;
	characterUpdate->guildIndex			= mPlayerExrInfo.mGuildIndex;
	characterUpdate->guildPosition		= mPlayerExrInfo.mGuildPosition;
	characterUpdate->level				= mPlayerInfo.Level;
	characterUpdate->exp				= mHeroInfo.Exp;
	characterUpdate->skillLevel			= mHeroInfo.SkillLevel;
	characterUpdate->skillExp			= mHeroInfo.SkillExp;
	characterUpdate->skillPointRemain	= mHeroInfo.SkillPointRemain;
	characterUpdate->skillPointTotal	= mHeroInfo.SkillPointTotal;
    characterUpdate->makeSkill1_Exp     = mHeroInfo.MakeSkill1.mExp;
    characterUpdate->makeSkill2_Exp     = mHeroInfo.MakeSkill2.mExp;

	if ( mIsInDunJoin == true )
	{
		if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
		{
			// 튜토리얼
			characterUpdate->mapNumber = mInDunMapDataNum;
			characterUpdate->xPos      = mObjectPos.x;
			characterUpdate->yPos      = mObjectPos.y;
		}
		else
		{
			// 일반 인던
			characterUpdate->mapNumber = mGameSrvReturnMapNum;
			characterUpdate->xPos      = mGameSrvReturnPos.x;
			characterUpdate->yPos      = mGameSrvReturnPos.y;
		}
	}
	else
	{
		// 일반맵
		characterUpdate->mapNumber = mMapNumber;
		characterUpdate->xPos      = mObjectPos.x;
		characterUpdate->yPos      = mObjectPos.y;
	}

	characterUpdate->retvalue = -1;
	characterUpdate->flag     = updateFlag;
	characterUpdate->expRecover = mHeroInfo.mLoseExp;
	characterUpdate->expRecoverendDate = mExpRecoverEndTime;

	NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(CHARACTER_UPDATE), updateComplete ? COMMON_DB_CHARACTER_UPDATE : 0 );
	return (mIsDbUpdate = true);
}

bool cPlayer::IsChgMonCoolTimeEnd( eMONSTERATTACK_TYPE type )
{
	unsigned long check = NETWORK2->GetAccumTime();

	if( mChgMonCoolTime[type] > check )
	{
		++mMonSkillCoolTimeFail;
		return false;
	}

	return true;
}

void cPlayer::UpdateChgMonCoolTime( eMONSTERATTACK_TYPE type )
{
	if( type >= eMONSTERATTACK_MAX )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::UpdateCoolTime type[%d][%d] >= eMONSTERATTACK_MAX", mObject.index, type );
		return;
	}

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::UpdateCoolTime mPlayerExrInfo.mChgMonsterIdx[%d][%d] != 0", mObject.index, mPlayerExrInfo.mChgMonsterIdx );
		return;
	}

	sMonsterSkillScript* pScript = SKILLSCRIPT->GetMonsterSkillInfo( mPlayerExrInfo.mChgMonsterIdx, type );
	if( pScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayer::UpdateCoolTime pScript[%d][%d,%d] == NULL", mObject.index, mPlayerExrInfo.mChgMonsterIdx, type );
		return;
	}

	/// 스텟에서 계산된 공격속도 ( 아이템, 버프 포함 )
	float attackSpeed = GetStatus2()->mAttackSpeed;

	if( attackSpeed <= 0.0f )
		attackSpeed = 0.1f;

	if( attackSpeed >= 2.0f )
		attackSpeed = 1.9f;

	unsigned long fixCoolTime = pScript->mCoolTime;

	/// 일반공격은 공속반영
	if( type < eMONSTERATTACK_SKILL1 )
		fixCoolTime = (unsigned long)( pScript->mCoolTime * ( 2.0f - attackSpeed ) );

	/// 허용 오차 적용
	if( fixCoolTime > SYNC_NORMALATTACK )
		fixCoolTime = fixCoolTime - SYNC_NORMALATTACK;
	else
		fixCoolTime = 0;

	mChgMonCoolTime[type] = NETWORK2->GetAccumTime() + fixCoolTime;
}

void cPlayer::SendCoolTimeChgMon()
{
	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
		return;

	unsigned long accumTime = NETWORK2->GetAccumTime();
	unsigned long skill[eMONSTERATTACK_MAX - eMONSTERATTACK_SKILL1] = {0,};

	for( int i = 0 ; i < eMONSTERATTACK_MAX - eMONSTERATTACK_SKILL1 ; ++i )
	{
		if( mChgMonCoolTime[ eMONSTERATTACK_SKILL1 + i ] > accumTime )
			skill[i] = mChgMonCoolTime[eMONSTERATTACK_SKILL1 + i] - accumTime;
		else
			skill[i] = 0;

		/// 보유 스킬 체크
		if( SKILLSCRIPT->GetMonsterSkillInfo( mPlayerExrInfo.mChgMonsterIdx, 
			eMONSTERATTACK_TYPE( eMONSTERATTACK_SKILL1 + i ) ) == NULL )
			skill[i] = ULONG_MAX;
	}

	HANDLE                         handle  = NULL;
	MSG_RES_SKILL_CHGMON_COOLTIME* sendMsg = (MSG_RES_SKILL_CHGMON_COOLTIME*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_SKILL, NM_SKILL_CHGMON_SKILLCOOLTIME );
	if ( sendMsg != NULL )
	{
		sendMsg->mSkill1CoolTime = skill[eMONSTERATTACK_SKILL1 - eMONSTERATTACK_SKILL1];
		sendMsg->mSkill2CoolTime = skill[eMONSTERATTACK_SKILL2 - eMONSTERATTACK_SKILL1];
		sendMsg->mSkill3CoolTime = skill[eMONSTERATTACK_SKILL3 - eMONSTERATTACK_SKILL1];
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_CHGMON_COOLTIME) );
	}
}

void cPlayer::SetDuelIdx( unsigned long duelIdx )
{
	mDuelIdx = duelIdx;
	mPlayerExrInfo.mIsDuelIdx = duelIdx == 0 ? false : true;
}

void cPlayer::SetStateOddity( long* pOddity )
{
	for( int i = 0 ; i < eODDITYTYPE_MAX ; ++i )
	{
		if( mODDITY[i] != pOddity[i] )
		{			
			mODDITY[i] = pOddity[i];
		
			/// 적용/해제 메세지 발송처리
			if( pOddity[i] != 0 )
			{
				/// 상태이상 적용부 클라이언트에 알림
				HANDLE handle  = NULL;
				switch( i )
				{
				case eODDITYTYPE_CANTMOVE:
					{
						MoveStop();

						MSG_SYN_PLAYER_CANTMOVE synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_CANTMOVE_SYN;
						synMsg.mIsCantMove = true;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
					}
					break;
				case eODDITYTYPE_CANTSKILL_PHY:
					{
						MSG_SYN_PLAYER_CANTSKILL* sendMsg = (MSG_SYN_PLAYER_CANTSKILL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_CANTSKILL_SYN );
						if ( sendMsg != NULL )
						{
							sendMsg->mAttributeType = eATTRIBUTETYPE_PHYSICAL;
							sendMsg->mIsCantSkill = true;  
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_CANTSKILL) );
						}
						
						if( GetState() == eOBJECT_STATE_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STOPSKILL_PHY ) == false*/ )
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_PHYSICAL );
					}
					break;
				case eODDITYTYPE_CANTSKILL_MAG:
					{
						MSG_SYN_PLAYER_CANTSKILL* sendMsg = (MSG_SYN_PLAYER_CANTSKILL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_CANTSKILL_SYN );
						if ( sendMsg != NULL )
						{
							sendMsg->mAttributeType = eATTRIBUTETYPE_MAGIC;
							sendMsg->mIsCantSkill = true;  
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_CANTSKILL) );
						}

						if( GetState() == eOBJECT_STATE_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STOPSKILL_MAG ) == false*/ )
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_MAGIC );
					}
					break;
				case eODDITYTYPE_SLEEP:
					{
						MoveStop();

						MSG_SYN_PLAYER_SLEEP synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_SLEEP_SYN;
						synMsg.mIsSleep = true;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );

						// 채집 취소
						if( GetState() == ePLAYER_STATE_GATHERING )
						{
							cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
							if( pGathering != NULL )
								pGathering->CancelGatherByOddity( this );
						}
						else if( GetState() == eOBJECT_STATE_STOP )
						{
							// 강화 취소
							if( GetStateStop() == eSTOP_ENHANCED )
							{
								ItemEnhancedForceCancel();
							}
							// 조합 취소
							else if( GetStateStop() == eSTOP_ITEMMIX )
							{
								ItemMixForceCancel();
							}
						}
						else if( GetState() == eOBJECT_STATE_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_SLEEP ) == false*/ )
						{
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
						}
					}
					break;
				case eODDITYTYPE_STUN:
					{
						MoveStop();

						MSG_SYN_PLAYER_STUN synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_STUN_SYN;
						synMsg.mIsStun = true;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );

						// 채집 취소
						if( GetState() == ePLAYER_STATE_GATHERING )
						{
							cGathering* pGathering = OBJECTMANAGER->GetGathering( mGatheringIdx );
							if( pGathering != NULL )
								pGathering->CancelGatherByOddity( this );
						}
						else if( GetState() == eOBJECT_STATE_STOP )
						{
							// 강화 취소
							if( GetStateStop() == eSTOP_ENHANCED )
							{
								ItemEnhancedForceCancel();
							}
							// 조합 취소
							else if( GetStateStop() == eSTOP_ITEMMIX )
							{
								ItemMixForceCancel();
							}
						}
						else if( GetState() == eOBJECT_STATE_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STUN ) == false*/ )
						{
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
						}
					}
					break;
				case eODDITYTYPE_AVOID_MON_LOOK:
					{
						MSG_SYN_PLAYEREXT_AVOID_MON_LOOK* synMsg = (MSG_SYN_PLAYEREXT_AVOID_MON_LOOK*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYEREXT, NM_PLAYEREXT_AVOID_MON_LOOK_SYN );
						if ( synMsg != NULL )
						{
							synMsg->mIsAvoidMonLookOn = true;
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYEREXT_AVOID_MON_LOOK) );
						}
					}
					break;
				}
			}
			else
			{
				/// 상태이상 해제부 클라이언트에 알림
				HANDLE handle  = NULL;
				switch( i )
				{
				case eODDITYTYPE_CANTMOVE:
					{
						MSG_SYN_PLAYER_CANTMOVE synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_CANTMOVE_SYN;
						synMsg.mIsCantMove = false;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
					}
					break;
				case eODDITYTYPE_CANTSKILL_PHY:
					{
						MSG_SYN_PLAYER_CANTSKILL* sendMsg = (MSG_SYN_PLAYER_CANTSKILL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_CANTSKILL_SYN );
						if ( sendMsg != NULL )
						{
							sendMsg->mAttributeType = eATTRIBUTETYPE_PHYSICAL;
							sendMsg->mIsCantSkill = false;  
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_CANTSKILL) );
						}
					}
					break;
				case eODDITYTYPE_CANTSKILL_MAG:
					{
						MSG_SYN_PLAYER_CANTSKILL* sendMsg = (MSG_SYN_PLAYER_CANTSKILL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_CANTSKILL_SYN );
						if ( sendMsg != NULL )
						{
							sendMsg->mAttributeType = eATTRIBUTETYPE_MAGIC;
							sendMsg->mIsCantSkill = false;  
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_CANTSKILL) );
						}
					}
					break;
				case eODDITYTYPE_SLEEP:
					{
						MSG_SYN_PLAYER_SLEEP synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_SLEEP_SYN;
						synMsg.mIsSleep = false;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
					}
					break;
				case eODDITYTYPE_STUN:
					{
						MSG_SYN_PLAYER_STUN synMsg;
						synMsg.Category = NM_PLAYER;
						synMsg.Protocol = NM_PLAYER_STUN_SYN;
						synMsg.mIsStun = false;  
						synMsg.mCharaterIdx = mObject.index;
						synMsg.mCharacterPos.x = mObjectPos.x;
						synMsg.mCharacterPos.y = mObjectPos.y;

						NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
					}
					break;
				case eODDITYTYPE_AVOID_MON_LOOK:
					{
						MSG_SYN_PLAYEREXT_AVOID_MON_LOOK* synMsg = (MSG_SYN_PLAYEREXT_AVOID_MON_LOOK*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYEREXT, NM_PLAYEREXT_AVOID_MON_LOOK_SYN );
						if ( synMsg != NULL )
						{
							synMsg->mIsAvoidMonLookOn = false;
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYEREXT_AVOID_MON_LOOK) );
						}

					}
					break;
				}
			}
		}
	}
}

bool cPlayer::IsCantSkill( unsigned long attributeType )
{ 
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
		return mODDITY[eODDITYTYPE_CANTSKILL_PHY] != 0 || mODDITY[eODDITYTYPE_SLEEP] != 0 || mODDITY[eODDITYTYPE_STUN] != 0; 
	else
		return mODDITY[eODDITYTYPE_CANTSKILL_MAG] != 0 || mODDITY[eODDITYTYPE_SLEEP] != 0 || mODDITY[eODDITYTYPE_STUN] != 0; 
}

// IsAddMoney Method.
bool cPlayer::IsAddMoney(long money)
{
	if ( money > 0 )
	{
		unsigned long newMoney = mMoney + money;

        // 최대 금액보다 커질겨우 또는 오버플로 일 경우
		return ( newMoney > MAX_HAVE_MONEY || newMoney < mMoney ) ? false : true;
	}
	else
		return ( (unsigned long)labs(money) <= mMoney ) ? true : false;
}

// AddMoney Method - #include <LIMITS.H> ULONG_MAX=[4,294,967,295].
long cPlayer::AddMoney(sObject object, long money, bool save)
{
	if ( money > 0 )
	{
		unsigned long maxMoney = MAX_HAVE_MONEY;
		unsigned long newMoney = mMoney + money;

		// 최대 금액보다 커질겨우 또는 오버플로 일 경우
		if ( newMoney > maxMoney || newMoney < mMoney )
			money = maxMoney - mMoney;

		// 변경된 금액 저장(DB).
		if ( save == true )
			SaveMoney( object, mMoney, mMoney+money, money );

		mMoney += money;

		// 변경된 금액 발송.
		SendMoney( object, money );
		return money;
	}
	else if ( (unsigned long)labs(money) <= mMoney )
	{
		// 변경된 금액 저장(DB).
		if ( save == true )
			SaveMoney( object, mMoney, mMoney+money, money );

		mMoney += money;

		// 변경된 금액 발송.
		SendMoney( object, money );
		return money;
	}
	else
		return 0;
}

// SaveMoney Method - 변경할 금액만 반영.
bool cPlayer::SaveMoney(sObject object, unsigned long befor, unsigned long after, long money)
{
	HANDLE           handle         = NULL;
	CHARACTER_MONEY* characterMoney = (CHARACTER_MONEY*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_MONEY );
	if ( characterMoney != NULL )
	{
		characterMoney->idx         = mObject.index;
		characterMoney->money       = (__int64)money;
		characterMoney->objectType  = object.type;
		characterMoney->objectIndex = object.index;
		characterMoney->befor       = befor;
		characterMoney->after       = after;

		switch ( object.type )
		{
		case eOBJECTTYPE_MONSTER:
			{
				cMonster* monster = OBJECTMANAGER->GetMonster( object.index );
				if ( monster != NULL )
					characterMoney->objectRaceGender = monster->GetRaceGender( );
			}
			break;
		case eOBJECTTYPE_NPC:
			{
				cNpc* npc = OBJECTMANAGER->GetNpc( object.index );
				if ( npc != NULL )
					characterMoney->objectRaceGender = npc->GetRaceGender( );
			}
			break;
		}

		return NETWORK2->SendSQL( handle, sizeof(CHARACTER_MONEY) );
	}
	return false;
}

// SendMoney Method.
bool cPlayer::SendMoney(sObject object, long addMoney)
{
	HANDLE                handle  = NULL;
	MSG_RES_PLAYER_MONEY* sendMsg = (MSG_RES_PLAYER_MONEY*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_MONEY_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mMoney      = mMoney;
		sendMsg->mObjectType = object.type;
		sendMsg->mAddMoney   = addMoney;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_MONEY) );
	}
	return false;
}

// AddDeposit Method - #include <LIMITS.H> ULONG_MAX=[4,294,967,295].
long cPlayer::AddDeposit(sObject object, long deposit)
{
	if ( deposit > 0 )
	{
		unsigned long maxDeposit = MAX_HAVE_MONEY;
		unsigned long newDeposit = mDeposit + deposit;

		// 최대 금액보다 커질겨우 또는 오버플로 일 경우
		if ( newDeposit > maxDeposit || newDeposit < mDeposit )
			deposit = maxDeposit - mDeposit;

		// 변경된 금액 저장(DB).
		SaveDeposit( object, mDeposit, mDeposit+deposit, deposit );

		mDeposit += deposit;

		// 변경된 금액 발송.
		SendDeposit( object, deposit );
		return deposit;
	}
	else if ( (unsigned long)labs(deposit) <= mDeposit )
	{
		// 변경된 금액 저장(DB).
		SaveDeposit( object, mDeposit, mDeposit+deposit, deposit );

		mDeposit += deposit;

		// 변경된 금액 발송.
		SendDeposit( object, deposit );
		return deposit;
	}
	else
		return 0;
}

// SaveDeposit Method - 변경할 금액만 반영.
bool cPlayer::SaveDeposit(sObject object, unsigned long befor, unsigned long after, long deposit)
{
	HANDLE             handle           = NULL;
	CHARACTER_DEPOSIT* characterDeposit = (CHARACTER_DEPOSIT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_DEPOSIT );
	if ( characterDeposit != NULL )
	{
		characterDeposit->idx         = mObject.index;
		characterDeposit->money       = (__int64)deposit;
		characterDeposit->objectType  = object.type;
		characterDeposit->objectIndex = object.index;
		characterDeposit->befor       = befor;
		characterDeposit->after       = after;

		if ( object.type == eOBJECTTYPE_NPC )
		{
			cNpc* npc = OBJECTMANAGER->GetNpc( object.index );
			if ( npc != NULL )
			{
				characterDeposit->objectRaceGender = npc->GetRaceGender( );
			}
		}

		return NETWORK2->SendSQL( handle, sizeof(CHARACTER_DEPOSIT) );
	}
	return false;
}

// SendDeposit Method.
bool cPlayer::SendDeposit(sObject object, long addDeposit)
{
	HANDLE                  handle  = NULL;
	MSG_RES_PLAYER_DEPOSIT* sendMsg = (MSG_RES_PLAYER_DEPOSIT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_DEPOSIT_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mDeposit    = mDeposit;
		sendMsg->mObjectType = object.type;
		sendMsg->mAddDeposit = addDeposit;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_DEPOSIT) );
	}
	return false;
}

// NpcDeposit Method.
void cPlayer::NpcDeposit(MSG_REQ_NPC_DEPOSIT* msg)
{
	// npc 인증
    if ( GetStateStop() == eSTOP_NPCSPEECH )
    {
        unsigned long retValue = OBJECTMANAGER->IsNpcTalk( msg->npcIdx, this, eNPCTALK_INVENTORY, 0 );
        if ( retValue != ERROR_NPC_OPEN_SUCCESS )
        {
            NETWORK2->PostServerEvent( "player[%d,%d,%d] return[%d]cPlayer::NpcDeposit retValue != ERROR_NPC_OPEN_SUCCESS", 
                                        mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, retValue );
            throw ERROR_NPC_DEPOSIT_NPC;
        }
    }
    else
    {
        if ( GetStateStop() != eSTOP_SUMMON_PAPER )
        {
            NETWORK2->PostServerEvent( "ERROR player[%d,%d,%d] cPlayer::NpcDeposit GetStateStop != eSTOP_SUMMON_PAPER", 
                                        mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop );
            throw ERROR_NPC_DEPOSIT_FAIL;
        }
    }

	if ( msg->money > GetMoney( ) || msg->money > MAX_TRADE_MONEY )
		throw ERROR_NPC_DEPOSIT_TRADE_MONEY;

	sObject object  = { eOBJECTTYPE_NPC, msg->npcIdx };
	long    money   = -(long)msg->money;
	long    deposit = +(long)msg->money;
	long    result;

    // 최대 보유금액
    if( GetDeposit() + deposit > MAX_HAVE_MONEY )
        throw ERROR_NPC_DEPOSIT_HAVE_MONEY;

    result = AddMoney( object, money );
	if ( result != money )
		NETWORK2->PostServerEvent( "WARNING - NM_NPC_DEPOSIT_REQ - CharacterIdx(%u) Money(%u)", mObject.index, GetMoney( ) );

	result = AddDeposit( object, deposit );
	if ( result != deposit )
		NETWORK2->PostServerEvent( "WARNING - NM_NPC_DEPOSIT_REQ - CharacterIdx(%u) Deposit(%u) AddMoney(%u)", mObject.index, GetDeposit( ), deposit );

	throw ERROR_NPC_DEPOSIT_SUCCESS;
}

// NpcWithdraw Method.
void cPlayer::NpcWithdraw(MSG_REQ_NPC_WITHDRAW* msg)
{
	// npc 인증
    if ( GetStateStop() == eSTOP_NPCSPEECH )
    {
        unsigned long retValue = OBJECTMANAGER->IsNpcTalk( msg->npcIdx, this, eNPCTALK_INVENTORY, 0 );
        if ( retValue != ERROR_NPC_OPEN_SUCCESS )
        {
            NETWORK2->PostServerEvent( "player[%d,%d,%d] return[%d]cPlayer::NpcWithdraw retValue != ERROR_NPC_OPEN_SUCCESS", 
                                        mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop, retValue );
            throw ERROR_NPC_WITHDRAW_NPC;
        }
    }
    else
    {
        if ( GetStateStop() != eSTOP_SUMMON_PAPER )
        {
            NETWORK2->PostServerEvent( "ERROR player[%d,%d,%d] cPlayer::NpcWithdraw GetStateStop != eSTOP_SUMMON_PAPER", 
                                        mObject.index, mPlayerExrInfo.mState, mPlayerExrInfo.mStateStop );
            throw ERROR_NPC_WITHDRAW_FAIL;
        }
    }

	if ( msg->money > GetDeposit() || msg->money > MAX_TRADE_MONEY )
		throw ERROR_NPC_WITHDRAW_TRADE_MONEY;

	sObject object   = { eOBJECTTYPE_NPC, msg->npcIdx };
	long    money    = +(long)msg->money;
	long    withdraw = -(long)msg->money;
	long    result;

    if( IsAddMoney( money ) == false )
		throw ERROR_NPC_WITHDRAW_HAVE_MONEY;

	result = AddMoney( object, money );
	if ( result != money )
		NETWORK2->PostServerEvent( "WARNING - NM_NPC_WITHDRAW_REQ - CharacterIdx(%u) Money(%u)", mObject.index, GetMoney( ) );

	result = AddDeposit( object, withdraw );
	if ( result != withdraw )
		NETWORK2->PostServerEvent( "WARNING - NM_NPC_WITHDRAW_REQ - CharacterIdx(%u) Deposit(%u)", mObject.index, GetDeposit( ) );

	throw ERROR_NPC_WITHDRAW_SUCCESS;
}

// AddTarotPoint Method
long cPlayer::AddTarotPoint(long point)
{
	unsigned long& point_t = mHeroInfo.TarotPoint;
	if ( point > 0 )
	{
		unsigned long maxPoint = ULONG_MAX - 1UL;
		unsigned long newPoint = point_t + point;

		// 최대 금액보다 커질겨우 또는 오버플로 일 경우
		if ( newPoint > maxPoint || newPoint < point_t )
		{
			point = maxPoint - point_t;
		}

		// 변경된 금액 저장(DB).
		SaveTarotPoint( point );

		point_t += point;

		// 변경된 금액 발송.
		SendTarotPoint( point );
		return point;
	}
	else if ( (unsigned long)labs(point) <= point_t )
	{
		// 변경된 금액 저장(DB).
		SaveTarotPoint( point );

		point_t += point;

		// 변경된 금액 발송.
		SendTarotPoint( point );
		return point;
	}
	else
		return 0;
}

// SaveTarotPoint Method
bool cPlayer::SaveTarotPoint(long point)
{
	HANDLE                 handle     = NULL;
	CHARACTER_TAROT_POINT* tarotPoint = (CHARACTER_TAROT_POINT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_TAROT_POINT );
	if ( point != NULL )
	{
		tarotPoint->idx   = mObject.index;
		tarotPoint->point = point;
		return NETWORK2->SendSQL( handle, sizeof(CHARACTER_TAROT_POINT) );
	}
	return false;
}

// SendTarotPoint Method
bool cPlayer::SendTarotPoint(long point)
{
	HANDLE                      handle  = NULL;
	MSG_RES_PLAYER_TAROT_POINT* sendMsg = (MSG_RES_PLAYER_TAROT_POINT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_TAROT_POINT_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mPoint    = mHeroInfo.TarotPoint;
		sendMsg->mAddPoint = point;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TAROT_POINT) );
	}
	return false;
}

unsigned long cPlayer::GetPvPPoint( unsigned char forceType )
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

// AddPvPPoint Method
long cPlayer::AddPvPPoint( long point, unsigned char forceType )
{
	unsigned long* point_t = NULL;
	switch( forceType )
	{
	case eFORCETYPE_FIRE:
		point_t = &mHeroInfo.mFirePoint;
		break;
	case eFORCETYPE_WATER:
		point_t = &mHeroInfo.mWaterPoint;
		break;
	case eFORCETYPE_WIND:
		point_t = &mHeroInfo.mWindPoint;
		break;
	case eFORCETYPE_EARTH:
		point_t = &mHeroInfo.mEarthPoint;
		break;
	default:
		return 0;
	}

	if ( point > 0 )
	{
		unsigned long maxPoint = ULONG_MAX - 1UL;
		unsigned long newPoint = *point_t + point;

		// 최대 금액보다 커질겨우 또는 오버플로 일 경우
		if ( newPoint > maxPoint || newPoint < *point_t )
		{
			point = maxPoint - *point_t;
		}

		// 변경된 금액 저장(DB).
		//SaveTarotPoint( point );

		*point_t += point;

		/// point 동시 처리 경우가 생기면 +- 합산 저장으로 바꿔야함
		ForcePointDBUpdate();

		// 변경된 금액 발송.
		SendPvPPoint( point, forceType );
		return point;
	}
	else if ( (unsigned long)labs(point) <= *point_t )
	{
		// 변경된 금액 저장(DB).
		//SaveTarotPoint( point );

		*point_t += point;

		/// point 동시 처리 경우가 생기면 +- 합산 저장으로 바꿔야함
		ForcePointDBUpdate();

		// 변경된 금액 발송.
		SendPvPPoint( point, forceType );
		return point;
	}
	else
		return 0;
}

// SendPvPPoint Method
bool cPlayer::SendPvPPoint( long point, unsigned char forceType )
{
	HANDLE                    handle  = NULL;
	MSG_RES_PLAYER_PVP_POINT* sendMsg = (MSG_RES_PLAYER_PVP_POINT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_PVP_POINT_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mForceType = forceType;
		sendMsg->mPoint    = GetPvPPoint( forceType );
		sendMsg->mAddPoint = point;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_PVP_POINT) );
	}
	return false;
}

/// InventoryMove Method
bool cPlayer::IsInventoryMove(unsigned short number1, unsigned short number2, short* except, int& depth)
{
	TB_INVENTORY* inventory1 = SelectInventory( number1 );
	TB_INVENTORY* inventory2 = SelectInventory( number2 );

	if ( IsInventory( inventory1 ) == false )
	{
		return false;
	}
	else if ( IsInventory( inventory2 ) && depth == 0 )
	{
		if ( IsInventoryMove( number2, number1, except, (--depth) ) == false )
			return false;
	}

    // 타로카드 체크
    if ( IsInventoryCollect( number2 ) == true )
    {
	    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
	    if ( itemDefine == NULL )
		    return false;

        if ( itemDefine->type == ITEM_CARD )
        {
            if ( !(itemDefine->subType == ITEM_CARD_TAROT || 
                   itemDefine->subType == ITEM_CARD_TAROT_SILVER ||
                   itemDefine->subType == ITEM_CARD_TAROT_GOLD ||
                   itemDefine->subType == ITEM_CARD_TAROT_DIA) )
            {
                return false;
            }

            TB_ITEM_TAROT* itemTarot = ITEMMANAGER->GetItemTarot( inventory1->itemDefineIdx );
            if ( itemTarot == NULL )
                return false;

            long tarotBegin = INVENTORY_TAROT_NORMAL_BEGIN;
            switch ( itemDefine->subType )
            {
            case ITEM_CARD_TAROT:        tarotBegin = INVENTORY_TAROT_NORMAL_BEGIN;   break;
            case ITEM_CARD_TAROT_SILVER: tarotBegin = INVENTORY_TAROT_SILVER_BEGIN;   break;
            case ITEM_CARD_TAROT_GOLD:   tarotBegin = INVENTORY_TAROT_GOLD_BEGIN;     break;
            case ITEM_CARD_TAROT_DIA:    tarotBegin = INVENTORY_TAROT_DIA_BEGIN;      break;
            }

            if ( (itemTarot->number + tarotBegin) != number2 )
                return false;
        }
        else
            return false;
    }

	if ( IsInventoryEquip( number2 ) == false )
		return true;

	/*-- 아이템 착용 가능 확인
	*/
	TB_ITEM_DEFINE* tbItemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
	if ( tbItemDefine == NULL )
		return false;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory1->itemDefineIdx ) )
		return false;

	/*-- 미인증 아이템 --*/
	if ( IsInventorySeal( inventory1 ) == true )
		return false;

	switch ( tbItemDefine->type )
	{
	case ITEM_WEAPON:
		switch ( tbItemDefine->subType )
		{
		case ITEM_WEAPON_SWORD:
			switch ( number2 )
			{
			case INVENTORY_HAND_RIGHT1:
				{
					TB_INVENTORY* left1 = SelectInventory( INVENTORY_HAND_LEFT1 );
					if ( IsInventory( left1 ) )
					{
						if ( ITEMMANAGER->GetItemDefineSubType( left1->itemDefineIdx ) != ITEM_WEAPON_SHIELD )
							(*except) = INVENTORY_HAND_LEFT1;
					}
				} return true;
			case INVENTORY_HAND_RIGHT2:
				{
					TB_INVENTORY* left2 = SelectInventory( INVENTORY_HAND_LEFT2 );
					if ( IsInventory( left2 ) )
					{
						if ( ITEMMANAGER->GetItemDefineSubType( left2->itemDefineIdx ) != ITEM_WEAPON_SHIELD )
							(*except) = INVENTORY_HAND_LEFT2;
					}
				} return true;
			}
			break;		// tbItemDefine->subType

		case ITEM_WEAPON_BLADE:
		case ITEM_WEAPON_DUAL:
		case ITEM_WEAPON_CUTTER:
		case ITEM_WEAPON_GUN:
		case ITEM_WEAPON_STAFF:
			switch ( number2 )
			{
			case INVENTORY_HAND_LEFT1:
				if ( IsInventory( SelectInventory( INVENTORY_HAND_RIGHT1 ) ) )
					(*except) = INVENTORY_HAND_RIGHT1;
				return true;
			case INVENTORY_HAND_RIGHT1:
				if ( IsInventory( SelectInventory( INVENTORY_HAND_LEFT1 ) ) )
					(*except) = INVENTORY_HAND_LEFT1;
				return true;
			case INVENTORY_HAND_LEFT2:
				if ( IsInventory( SelectInventory( INVENTORY_HAND_RIGHT2 ) ) )
					(*except) = INVENTORY_HAND_RIGHT2;
				return true;
			case INVENTORY_HAND_RIGHT2:
				if ( IsInventory( SelectInventory( INVENTORY_HAND_LEFT2 ) ) )
					(*except) = INVENTORY_HAND_LEFT2;
				return true;
			}
			break;		// tbItemDefine->subType

		case ITEM_WEAPON_SHIELD:
			switch ( number2 )
			{
			case INVENTORY_HAND_LEFT1:
				{
					TB_INVENTORY* right1 = SelectInventory( INVENTORY_HAND_RIGHT1 );
					if ( IsInventory( right1 ) )
					{
						if ( ITEMMANAGER->GetItemDefineSubType( right1->itemDefineIdx ) != ITEM_WEAPON_SWORD )
							(*except) = INVENTORY_HAND_RIGHT1;
					}
				} return true;
			case INVENTORY_HAND_LEFT2:
				{
					TB_INVENTORY* right2 = SelectInventory( INVENTORY_HAND_RIGHT2 );
					if ( IsInventory( right2 ) )
					{
						if ( ITEMMANAGER->GetItemDefineSubType( right2->itemDefineIdx ) != ITEM_WEAPON_SWORD )
							(*except) = INVENTORY_HAND_RIGHT2;
					}
				} return true;
			}
			break;		// tbItemDefine->subType
		}
		return false;	// switch ( tbItemDefine->type )

	case ITEM_WEAR:
		switch ( tbItemDefine->subType )
		{
		case ITEM_WEAR_HEAD:
			return (number2 == INVENTORY_WEAR_HAT);
		case ITEM_WEAR_UPPER:
			return (number2 == INVENTORY_WEAR_BODY1);
		case ITEM_WEAR_LOWER:
			return (number2 == INVENTORY_WEAR_BODY2);
		case ITEM_WEAR_HANDS:
			return (number2 == INVENTORY_WEAR_HAND);
		case ITEM_WEAR_FEET:
			return (number2 == INVENTORY_WEAR_FOOT);
		case ITEM_WEAR_ONEPIECE:
			switch ( number2 )
			{
			case INVENTORY_WEAR_BODY1:
				if ( IsInventory( SelectInventory( INVENTORY_WEAR_BODY2 ) ) )
					(*except) = INVENTORY_WEAR_BODY2;
				return true;
			case INVENTORY_WEAR_BODY2:
				if ( IsInventory( SelectInventory( INVENTORY_WEAR_BODY1 ) ) )
					(*except) = INVENTORY_WEAR_BODY1;
				return true;
			}
			break;
        case ITEM_WEAR_COS_ONEPIECE:
			if ( number2 == INVENTORY_COSTUME_BODY1 )
			{
				if ( IsInventory( SelectInventory( INVENTORY_COSTUME_BODY2 ) ) )
				{
					(*except) = INVENTORY_COSTUME_BODY2;
					except++;
				}
				if ( IsInventory( SelectInventory( INVENTORY_COSTUME_HAND ) ) )
				{
					(*except) = INVENTORY_COSTUME_HAND;
					except++;
				}
				if ( IsInventory( SelectInventory( INVENTORY_COSTUME_FOOT ) ) )
				{
					(*except) = INVENTORY_COSTUME_FOOT;
					except++;
				}
				return true;
			}
            return false;
        case ITEM_WEAR_COS_HEAD:
            return (number2 == INVENTORY_COSTUME_HAT);
        case ITEM_WEAR_COS_UPPER:
            return (number2 == INVENTORY_COSTUME_BODY1);
        case ITEM_WEAR_COS_LOWER:
			if ( number2 == INVENTORY_COSTUME_BODY2 )
			{
				TB_INVENTORY* upper = SelectInventory( INVENTORY_COSTUME_BODY1 );
				if ( IsInventory( upper ) )
				{
					TB_ITEM_DEFINE* tempDefine = ITEMMANAGER->GetItemDefine( upper->itemDefineIdx );
					if ( tempDefine->subType == ITEM_WEAR_COS_ONEPIECE )
						(*except) = INVENTORY_COSTUME_BODY1;
				}
				return true;
			} return false;
        case ITEM_WEAR_COS_HANDS:
            if ( number2 == INVENTORY_COSTUME_HAND )
			{
				TB_INVENTORY* upper = SelectInventory( INVENTORY_COSTUME_BODY1 );
				if ( IsInventory( upper ) )
				{
					TB_ITEM_DEFINE* tempDefine = ITEMMANAGER->GetItemDefine( upper->itemDefineIdx );
					if ( tempDefine->subType == ITEM_WEAR_COS_ONEPIECE )                                                   
						(*except) = INVENTORY_COSTUME_BODY1;
				}
				return true;
			} else return false;
        case ITEM_WEAR_COS_FEET:
            if ( number2 == INVENTORY_COSTUME_FOOT )
			{
				TB_INVENTORY* upper = SelectInventory( INVENTORY_COSTUME_BODY1 );
				if ( IsInventory( upper ) )
				{
					TB_ITEM_DEFINE* tempDefine = ITEMMANAGER->GetItemDefine( upper->itemDefineIdx );
					if ( tempDefine->subType == ITEM_WEAR_COS_ONEPIECE )
						(*except) = INVENTORY_COSTUME_BODY1;
				}
				return true;
			} else return false;
		}
		return false;	// switch ( tbItemDefine->type )

	case ITEM_ACCESSORY:
		switch ( tbItemDefine->subType )
		{
		case ITEM_ACCESSORY_EARRING:
			return (number2 == INVENTORY_WEAR_EARRING);
		case ITEM_ACCESSORY_NECKLACE:
			return (number2 == INVENTORY_WEAR_NECKLACE);
		case ITEM_ACCESSORY_BROOCH:
			return (number2 == INVENTORY_WEAR_BROOCH);
		case ITEM_ACCESSORY_BRACELET:
			return (number2 == INVENTORY_WEAR_BRACELET);
		case ITEM_ACCESSORY_RING:
			return (number2 == INVENTORY_WEAR_RING);
		case ITEM_ACCESSORY_COS_FACE:
			return (number2 == INVENTORY_COSTUME_FACE);
		case ITEM_ACCESSORY_COS_BACK:
			return (number2 == INVENTORY_COSTUME_BACK);
		}
		return false;	// switch ( tbItemDefine->type )

	case ITEM_CARD:
		if ( tbItemDefine->subType == ITEM_CARD_EQUIP_C )
		{
			switch ( number2 )
			{
			case INVENTORY_WEAR_CARD1:
			case INVENTORY_WEAR_CARD2:
			case INVENTORY_WEAR_CARD3:
				return true;
			}
		}
		return false;	// switch ( tbItemDefine->type )

	default:
		return false;	// switch ( tbItemDefine->type )
	}
}

/// IsInventoryMerge Method
bool cPlayer::IsInventoryMerge(TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory1, TB_INVENTORY* inventory2)
{
	if ( IsInventory( inventory1 ) && IsInventory( inventory2 ) )
	{
		if ( inventory1->itemDefineIdx == inventory2->itemDefineIdx )
		{
			if ( itemDefine == NULL )
				return false;
			else if ( inventory1->cash > 0 || inventory2->cash > 0 )
				return false;
			else
				return (itemDefine->capacity > 1 && itemDefine->licenseType != 1) ? true : false;
		}
	}

	return false;
}

/// IsInventorySeal Method
bool cPlayer::IsInventorySeal( TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory )
{
    if( IsInventory( inventory ) )
    {
        if ( itemDefine )
            return ( itemDefine->licenseType == 1 && inventory->seal == 1);
    }

    return false;
}

/// IsInventorySeal Method
bool cPlayer::IsInventorySeal(TB_INVENTORY* inventory)
{
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if ( itemDefine != NULL )
	{
		return IsInventorySeal( itemDefine, inventory );
	}
	return false;
}

/// IsInventoryMegaphone Method
bool cPlayer::IsInventoryMegaphone(long inventoryIdx)
{
	TB_INVENTORY* inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( unsigned short i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd; i++, inventory++ )
	{
		if ( IsInventory( inventory ) == true )
		{
			if ( inventory->idx == inventoryIdx )
			{
				TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
				if ( itemDefine != NULL && inventory->seal == 0 )
				{
					if ( (itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_SHOUT) &&
						 (COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, inventory->itemDefineIndex ) == false) )
					{
						COOLTIMEPOOL->ApplyCooltime( &mCooltimeRoot, inventory );
						return true;
					}
				}
				break;
			}
		}
	}
	return false;
}

/// SetInventorySize Method
bool cPlayer::SetInvenLimit( TB_ITEM_DEFINE* itemDefine, CHARACTER_INVEN_SIZE* invenSize )
{
    if( itemDefine->subType == ITEM_INVENTORY_BAG )
    {
        switch( mHeroInfo.BagEnd )
        {
        case INVENTORY_BAG0_END: invenSize->bag = INVENTORY_BAG1_END; break;
        case INVENTORY_BAG1_END: invenSize->bag = INVENTORY_BAG2_END; break;
        case INVENTORY_BAG2_END: invenSize->bag = INVENTORY_BAG3_END; break;
        default: return false; break;
        }
    }
    else if( itemDefine->subType == ITEM_INVENTORY_CARD )
    {
        switch( mHeroInfo.WearEnd )
        {
		case INVENTORY_WEAR_RING : invenSize->wear = INVENTORY_WEAR_CARD1; break;
        case INVENTORY_WEAR_CARD1: invenSize->wear = INVENTORY_WEAR_CARD2; break;
        case INVENTORY_WEAR_CARD2: invenSize->wear = INVENTORY_WEAR_CARD3; break;
        default: return false; break;
        }
    }
    else if( itemDefine->subType == ITEM_INVENTORY_WAREHOUSE )
    {
        switch( mHeroInfo.WareHouseEnd )
        {
        case INVENTORY_WAREHOUSE0_END: invenSize->wareHouse = INVENTORY_WAREHOUSE1_END; break;
        case INVENTORY_WAREHOUSE1_END: invenSize->wareHouse = INVENTORY_WAREHOUSE2_END; break;
        case INVENTORY_WAREHOUSE2_END: invenSize->wareHouse = INVENTORY_WAREHOUSE3_END; break;
        default: return false; break;
        }
    }
    else if( itemDefine->subType == ITEM_INVENTORY_MAKESKILL )
    {
        invenSize->makeSkill += 1;
        if( invenSize->makeSkill > MAX_MAKESKILL )
            invenSize->makeSkill = MAX_MAKESKILL;
    }

    return true;
}

// GetEmptyBagNumber Method
short cPlayer::GetEmptyBagNumber(short number)
{
	TB_INVENTORY* inventory = SelectInventory( number );
    for ( short i = number; i <= mHeroInfo.BagEnd; i++, inventory++ )
	{
		if ( !IsInventory( inventory ) )
			return i;
	}
	return INVENTORY_BAG_NONE;
}

// GetEmptyQuestNumber Method
short cPlayer::GetEmptyQuestNumber(short number)
{
	TB_INVENTORY* inventory = SelectInventory( number );
	for ( short i = number; i <= INVENTORY_QUEST_END; i++, inventory++ )
	{
		if ( !IsInventory( inventory ) )
			return i;
	}
	return INVENTORY_QUEST_NONE;
}

/// InventorySwap Method
void cPlayer::InventorySwap(unsigned short number1, unsigned short number2)
{
	TB_INVENTORY* inventory1 = SelectInventory( number1 );
	TB_INVENTORY* inventory2 = SelectInventory( number2 );

	XCopyInventory( inventory1, inventory2, XCOPY_INVENTORY_SWAP );
}

// InventoryMove Method
void cPlayer::InventoryMove(unsigned short number1, unsigned short number2)
{
	TB_INVENTORY* inventory1 = SelectInventory( number1 );
	TB_INVENTORY* inventory2 = SelectInventory( number2 );

	inventory1->number = number2;
	XCopyInventory( inventory2, inventory1, XCOPY_INVENTORY_ALL );

	inventory1->number = number1;
	if ( IsInventoryCash( number1 ) == true )
	{
		inventory1->count += -(inventory1->count);
	}
	RemoveInventory( inventory1 );
}

// InventoryExcept Method
void cPlayer::InventoryExcept(unsigned short number1, unsigned short number2)
{
	TB_INVENTORY* inventory1 = SelectInventory( number1 );
	TB_INVENTORY* inventory2 = SelectInventory( number2 );

	inventory1->number = number2;
	XCopyInventory( inventory2, inventory1, XCOPY_INVENTORY_ALL );

	inventory1->number = number1;
	RemoveInventory( inventory1 );
}

// IsItemLimit Method.
bool cPlayer::IsItemLimit(long itemDefineIdx)
{
	TB_ITEM_LIMIT* itemLimit = ITEMMANAGER->GetItemLimit( itemDefineIdx );
	if ( itemLimit != NULL )
	{
		// 종족; TB_ITEM_LIMIT::charRace 와 sPlayerInfo::Race 는 정의는 다르나 같은 값을 사용.
		if ( itemLimit->charRace != ITEM_RACE_ALL )
		{
			if ( itemLimit->charRace != mPlayerInfo.Race )
				return false;
		}
		// 성별; TB_ITEM_LIMIT::charGender 와 sPlayerInfo::Gender 는 정의는 다르나 같은 값을 사용.
		if ( itemLimit->charGender != ITEM_GENDER_ALL )
		{
			if ( itemLimit->charGender != mPlayerInfo.Gender )
				return false;
		}
		// 직업; TB_ITEM_LIMIT::charJob 와 sPlayerInfo::Job 은 정의는 다르나 같은 값을 사용.
		if ( itemLimit->charJob1 != ITEM_JOB_ALL && itemLimit->charJob2 != ITEM_JOB_ALL )
		{
			switch ( mPlayerInfo.Job )
			{
			// 파이터(전사형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_CRUSADER:
				if ( ePLAYER_CRUSADER == itemLimit->charJob1 || ePLAYER_CRUSADER == itemLimit->charJob2 )
					break;
			case ePLAYER_KNIGHT:
				if ( ePLAYER_KNIGHT == itemLimit->charJob1 || ePLAYER_KNIGHT == itemLimit->charJob2 )
					break;
				if ( ePLAYER_FIGHTER == itemLimit->charJob1 || ePLAYER_FIGHTER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_TEMPLAR:
				if ( ePLAYER_TEMPLAR == itemLimit->charJob1 || ePLAYER_TEMPLAR == itemLimit->charJob2 )
					break;
			case ePLAYER_GLADIATOR:
				if ( ePLAYER_GLADIATOR == itemLimit->charJob1 || ePLAYER_GLADIATOR == itemLimit->charJob2 )
					break;
				if ( ePLAYER_FIGHTER == itemLimit->charJob1 || ePLAYER_FIGHTER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_SOULBLADER:
				if ( ePLAYER_SOULBLADER == itemLimit->charJob1 || ePLAYER_SOULBLADER == itemLimit->charJob2 )
					break;
			case ePLAYER_MERCENARY:
				if ( ePLAYER_MERCENARY == itemLimit->charJob1 || ePLAYER_MERCENARY == itemLimit->charJob2 )
					break;
				if ( ePLAYER_FIGHTER == itemLimit->charJob1 || ePLAYER_FIGHTER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_FIGHTER:
				if ( ePLAYER_FIGHTER == itemLimit->charJob1 || ePLAYER_FIGHTER == itemLimit->charJob2 )
					break;
				return false;

			// 메이지(마법형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_WIZARD:
				if ( ePLAYER_WIZARD == itemLimit->charJob1 || ePLAYER_WIZARD == itemLimit->charJob2 )
					break;
			case ePLAYER_MAGICIAN:
				if ( ePLAYER_MAGICIAN == itemLimit->charJob1 || ePLAYER_MAGICIAN == itemLimit->charJob2 )
					break;
				if ( ePLAYER_MAGE == itemLimit->charJob1 || ePLAYER_MAGE == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_HIGHPRIEST:
				if ( ePLAYER_HIGHPRIEST == itemLimit->charJob1 || ePLAYER_HIGHPRIEST == itemLimit->charJob2 )
                    break;
			case ePLAYER_PRIEST:
				if ( ePLAYER_PRIEST == itemLimit->charJob1 || ePLAYER_PRIEST == itemLimit->charJob2 )
					break;
				if ( ePLAYER_MAGE == itemLimit->charJob1 || ePLAYER_MAGE == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_MAGE:
				if ( ePLAYER_MAGE == itemLimit->charJob1 || ePLAYER_MAGE == itemLimit->charJob2 )
					break;
				return false;

			// 원더러(민첩형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_ASSASSIN:
				if ( ePLAYER_ASSASSIN == itemLimit->charJob1 || ePLAYER_ASSASSIN == itemLimit->charJob2 )
					break;
			case ePLAYER_WINDWALKER:
				if ( ePLAYER_WINDWALKER == itemLimit->charJob1 || ePLAYER_WINDWALKER == itemLimit->charJob2 )
					break;
				if ( ePLAYER_WANDERER == itemLimit->charJob1 || ePLAYER_WANDERER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_SNIPER:
				if ( ePLAYER_SNIPER == itemLimit->charJob1 || ePLAYER_SNIPER == itemLimit->charJob2 )
					break;
			case ePLAYER_SCOUT:
				if ( ePLAYER_SCOUT == itemLimit->charJob1 || ePLAYER_SCOUT == itemLimit->charJob2 )
					break;
				if ( ePLAYER_WANDERER == itemLimit->charJob1 || ePLAYER_WANDERER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_WANDERER:
				if ( ePLAYER_WANDERER == itemLimit->charJob1 || ePLAYER_WANDERER == itemLimit->charJob2 )
					break;
				return false;

			// 워록(마법형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_SORCERER:
				if ( ePLAYER_SORCERER == itemLimit->charJob1 || ePLAYER_SORCERER == itemLimit->charJob2 )
					break;
			case ePLAYER_SHAMAN:
				if ( ePLAYER_SHAMAN == itemLimit->charJob1 || ePLAYER_SHAMAN == itemLimit->charJob2 )
					break;
				if ( ePLAYER_WARLOCK == itemLimit->charJob1 || ePLAYER_WARLOCK == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_MENTALIST:
				if ( ePLAYER_MENTALIST == itemLimit->charJob1 || ePLAYER_MENTALIST == itemLimit->charJob2 )
					break;
			case ePLAYER_SAGE:
				if ( ePLAYER_SAGE == itemLimit->charJob1 || ePLAYER_SAGE == itemLimit->charJob2 )
					break;
				if ( ePLAYER_WARLOCK == itemLimit->charJob1 || ePLAYER_WARLOCK == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_WARLOCK:
				if ( ePLAYER_WARLOCK == itemLimit->charJob1 || ePLAYER_WARLOCK == itemLimit->charJob2 )
					break;
				return false;

			// 암즈맨(전사형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_CHAMPION:
				if ( ePLAYER_CHAMPION == itemLimit->charJob1 || ePLAYER_CHAMPION == itemLimit->charJob2 )
					break;
			case ePLAYER_VETERAN:
				if ( ePLAYER_VETERAN == itemLimit->charJob1 || ePLAYER_VETERAN == itemLimit->charJob2 )
					break;
				if ( ePLAYER_ARMSMAN == itemLimit->charJob1 || ePLAYER_ARMSMAN == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_SLAYER:
				if ( ePLAYER_SLAYER == itemLimit->charJob1 || ePLAYER_SLAYER == itemLimit->charJob2 )
					break;
			case ePLAYER_BERSERK:
				if ( ePLAYER_BERSERK == itemLimit->charJob1 || ePLAYER_BERSERK == itemLimit->charJob2 )
					break;
				if ( ePLAYER_ARMSMAN == itemLimit->charJob1 || ePLAYER_ARMSMAN == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_HIGHLANDER:
				if ( ePLAYER_HIGHLANDER == itemLimit->charJob1 || ePLAYER_HIGHLANDER == itemLimit->charJob2 )
					break;
			case ePLAYER_WARRIOR:
				if ( ePLAYER_WARRIOR == itemLimit->charJob1 || ePLAYER_WARRIOR == itemLimit->charJob2 )
					break;
				if ( ePLAYER_ARMSMAN == itemLimit->charJob1 || ePLAYER_ARMSMAN == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_ARMSMAN:
				if ( ePLAYER_ARMSMAN == itemLimit->charJob1 || ePLAYER_ARMSMAN == itemLimit->charJob2 )
					break;
				return false;

			// 레인저(민첩형) 전직 클래스 - 하위 직업군 포함.
			case ePLAYER_AVENGER:
				if ( ePLAYER_AVENGER == itemLimit->charJob1 || ePLAYER_AVENGER == itemLimit->charJob2 )
					break;
			case ePLAYER_SHADOWWALKER:
				if ( ePLAYER_SHADOWWALKER == itemLimit->charJob1 || ePLAYER_SHADOWWALKER == itemLimit->charJob2 )
					break;
				if ( ePLAYER_RANGER == itemLimit->charJob1 || ePLAYER_RANGER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_SHARPSHOOTER:
				if ( ePLAYER_SHARPSHOOTER == itemLimit->charJob1 || ePLAYER_SHARPSHOOTER == itemLimit->charJob2 )
					break;
			case ePLAYER_HUNTER:
				if ( ePLAYER_HUNTER == itemLimit->charJob1 || ePLAYER_HUNTER == itemLimit->charJob2 )
					break;
				if ( ePLAYER_RANGER == itemLimit->charJob1 || ePLAYER_RANGER == itemLimit->charJob2 )
					break;
				return false;
			case ePLAYER_RANGER:
				if ( ePLAYER_RANGER == itemLimit->charJob1 || ePLAYER_RANGER == itemLimit->charJob2 )
					break;
				return false;
			}
		}
		// 레벨; 이하일 경우만 제어를 한다.
		if ( mPlayerInfo.Level < itemLimit->charLevelMin || mPlayerInfo.Level > itemLimit->charLevelMax )
			return false;

		// 서버분류; 0 공용 / 1 전장전용 / 2 튜토리얼전용 / 3 필드전용
		switch ( itemLimit->serverType )
		{
		case ITEM_LIMIT_COMMON: // 공용
			break;
		case ITEM_LIMIT_PVP: // 전장 전용
			if ( NETWORK2->GetServerType( ) == _E_ST_ID_PVP_ )
				break;
			return false;
		case ITEM_LIMIT_TUTORIAL: // 튜토리얼 전용
			if ( NETWORK2->GetServerType( ) == _E_ST_ID_TUTORIAL_ )
				break;
			return false;
        case ITEM_LIMIT_FIELD:
			if ( NETWORK2->GetServerType( ) == _E_ST_NORMAL_MAP_ )
				break;
			return false;
		default: // 알수없는 정의
			return false;
		}
		// 세력; 세력전용 아이템일경우만 사용가능.
		if ( (itemLimit->forceType != eFORCETYPE_NONE) && (itemLimit->forceType != mPlayerExrInfo.mForceType) )
			return false;

		// 보조직업 && 숙련도; 보조직업전용 아이템일경우만 사용가능.
		if ( itemLimit->makeSkill != eMAKESKILL_NONE )
        {
		    if ( itemLimit->makeSkill   == mHeroInfo.MakeSkill1.mIndex )
            {
                if ( itemLimit->makeSkillExp > mHeroInfo.MakeSkill1.mExp )
                    return false;
            }
            else if ( itemLimit->makeSkill   == mHeroInfo.MakeSkill2.mIndex )
            {
                if ( itemLimit->makeSkillExp > mHeroInfo.MakeSkill2.mExp )
                    return false;
            }
            else
                return false;
        }
	}
	// 성공; 사용가능.
	return true;
}

// IsItemExchange Method.
bool cPlayer::IsItemExchange( TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory )
{
    if( itemDefine == NULL )
        return false;

    /// 미인증 아이템
    if( itemDefine->licenseType == 1 )
    {

        /// 인증 받은 아이템은 거래 할 수 없음.
        if( inventory->seal == 0 )
            return false;

        if( itemDefine->exchange == 2 )
            return true;
    }
    /// 일반 아이템
    else
    {
        if( itemDefine->exchange == 0 )
            return true;
    }

    return false;
}

// IsMaxInventory Method.
bool cPlayer::IsMaxInventory( TB_ITEM_DEFINE* itemDefine, long count )
{
    if( itemDefine == NULL )
        return false;

    if( itemDefine->maxInventory == 0 )
        return false;

    long sumCount = GetItemSumCount( itemDefine->index ) + count;
    if( itemDefine->maxInventory < sumCount )
        return true;

    return false;
}

// IsMaxInventory Method.
bool cPlayer::IsMaxInventory( sQuestItem* items, unsigned int rowCount )
{
    for( unsigned int i = 0; i < rowCount, i < QUEST_ITEM_MAX; i++, items++ )
    {
        if( items )
        {
            TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );
            return IsMaxInventory( itemDefine, items->count );
        }
    }
    return false;
}

// GetItemSumCount Method.
long cPlayer::GetItemSumCount( long itemDefineIndex )
{
	PerItemCount* itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemDefineIndex );
	if( itemCount != NULL )
		return itemCount->sum;

    return 0;
}

// GetItemCount Method.
short cPlayer::GetItemCount( long itemDefineIndex )
{
	PerItemCount* itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemDefineIndex );
	if ( itemCount != NULL )
	{
		return (short)itemCount->bag;
	}
	return 0;
}

// IsItem Method.
bool cPlayer::IsItem(long itemDefineIndex)
{
	PerItemCount* itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemDefineIndex );
	if ( itemCount != NULL )
	{
		return (itemCount->bag > 0) ? true : false;
	}
	return false;
}

// IsItem Method.
bool cPlayer::IsItemIdx(long inventoryidx)
{
	TB_INVENTORY* inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( int i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd; i++, inventory++ )
	{
		if ( inventory->idx == inventoryidx )
			return true;
	}
	return false;
}

// IsItemUse Method.
bool cPlayer::IsItemUse(long itemDefineIndex, unsigned short count)
{
	// 쿨타임 검사.
	if ( COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, itemDefineIndex ) == true )
		return false;

	PerItemCount* itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemDefineIndex );
	if ( itemCount != NULL )
	{
		return (itemCount->bag >= count) ? true : false;
	}
	return false;
}

// ItemUse Method.
bool cPlayer::ItemUse(long itemDefineIndex, unsigned short count)
{
	if ( IsItemUse( itemDefineIndex, count ) == true )
	{
		TB_INVENTORY* inventory   = SelectInventory( INVENTORY_BAG_BEGIN );
		short         remainCount = count;

		HANDLE        handle   = NULL;
		ITEM_USE*     itemUse  = (ITEM_USE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_USE );
		long&         rowCount = itemUse->rowCount;
		TB_INVENTORY* table    = itemUse->table;
		long          length   = sizeof(ITEM_USE) - sizeof(itemUse->table);

		itemUse->characterIdx = mObject.index;

        for ( int i = INVENTORY_BAG_BEGIN; (i <= mHeroInfo.BagEnd) && (remainCount > 0); i++, inventory++ )
		{
			if ( IsInventory( inventory ) == true )
			{
				if ( itemDefineIndex == inventory->itemDefineIndex )
				{
					short shortCount = (remainCount < inventory->count) ? remainCount : inventory->count;

					(*table) = (*inventory);
					table->count = shortCount;

					table++; rowCount++;

					// 쿨타임 적용.
					COOLTIMEPOOL->ApplyCooltime( &mCooltimeRoot, inventory );

					// 가상처리; 차감을 완성 해 놓는다.
					UpdateInventory( inventory, (-shortCount) );

					// 가상처리; 사용된 아이템 정보를 전송한다.
					SendItemUse( inventory );

					// 가상처리; 사용이 완료된 인벤토리 삭제.
					if ( inventory->count == 0 )
						RemoveInventory( inventory );

					// 남은갯수 정산.
					remainCount = remainCount - shortCount;
				}
			}
		}

		length += sizeof(itemUse->table) * rowCount;
		return NETWORK2->SendSQL( handle, length );
	}
	else
		return false;
}

// IsItemUseSkill Method.
long cPlayer::IsItemMapChange(short number, long itemDefineIndex, bool isCash, long itemAbilityInfluence)
{
	//if ( !IsInventoryRange( number ) )
	//	return 0;

	//if( IsAutoTrial() == true )
	//	return 0;

	//TB_INVENTORY* inventory = SelectInventory( number );
	//if ( IsInventory( inventory ) == true )
	//{
	//	if ( inventory->itemDefineIndex == itemDefineIndex )
	//	{
	//		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	//		if ( itemDefine != NULL )
	//		{
	//			if ( itemDefine->type == ITEM_MATERIAL && itemDefine->subType == ITEM_MATERIAL_MAP )
	//			{
	//				if ( !IsItemLimit( inventory->itemDefineIdx ) )
	//					return 0;

	//				// 미인증 아이템
	//				if ( IsInventorySeal( itemDefine, inventory ) == true )
	//					return 0;

	//				TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	//				if ( itemAbility != NULL )
	//				{
	//					if ( itemAbility->influence_idx == itemAbilityInfluence )
	//					{
	//						// 쿨타임 검사.
	//						if ( COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, itemDefineIndex ) != true )
	//							return inventory->idx;
	//						else
	//							return 0;
	//					}
	//				}
	//				else
	//					NETWORK2->PostServerEvent( "ERROR - cPlayer::IsItemUseMapChange:TB_ITEM_ABILITY::IDX(=%d)", itemDefine->idxAbility );
	//			}
	//		}
	//		else
	//			NETWORK2->PostServerEvent( "ERROR - cPlayer::IsItemUseMapChange:TB_ITEM_DEFINE::IDX(=%d)", inventory->itemDefineIdx );
	//	}
	//}
	//return 0;

    if ( !IsInventoryRange( number ) )
        return 0;

    if( IsAutoTrial() == true )
        return 0;

	TB_INVENTORY* inventory = SelectInventory( number );
	if ( IsInventory( inventory ) == false )
		return 0;

	if ( inventory->itemDefineIndex != itemDefineIndex )
		return 0;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if ( itemDefine == NULL )
		return 0;

	if( isCash == true )
	{
		/// 캐시템 타입 확인
		if( itemDefine->type != ITEM_ETC1 || itemDefine->subType != ITEM_ETC1_FREE_MOVE )
			return 0;
	}
	else
	{
		/// 맵이동 일반 아이템 타입 확인
		if( itemDefine->type != ITEM_MATERIAL || itemDefine->subType != ITEM_MATERIAL_MAP )
			return 0;
	}

    if ( !IsItemLimit( inventory->itemDefineIdx ) )
        return 0;

    // 미인증 아이템
    if ( IsInventorySeal( itemDefine, inventory ) == true )
        return 0;

	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	if ( itemAbility == NULL )
		return 0;
	
	/// 효과 번호 확인
	if ( itemAbility->influence_idx != itemAbilityInfluence )
		return 0;

	// 쿨타임 검사.
	if ( COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, itemDefineIndex ) != true )
		return inventory->idx;
	else
		return 0;
}

// IsItemUseSkill Method.
long cPlayer::IsItemVehicle(short number, long itemDefineIndex, long itemAbilityInfluence)
{
    if ( !IsInventoryRange( number ) )
        return 0;

	TB_INVENTORY* inventory = SelectInventory( number );
	if ( IsInventory( inventory ) == true )
	{
		if ( inventory->itemDefineIndex == itemDefineIndex )
		{
			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
			if ( itemDefine != NULL )
			{
				if ( itemDefine->type == ITEM_MATERIAL && itemDefine->subType == ITEM_MATERIAL_VEHICLE )
				{
                    if ( !IsItemLimit( inventory->itemDefineIdx ) )
                        return 0;

                    // 미인증 아이템
                    if ( IsInventorySeal( itemDefine, inventory ) == true )
                        return 0;

					TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
					if ( itemAbility != NULL )
					{
						if ( itemAbility->influence_idx == itemAbilityInfluence )
						{
							// 쿨타임 검사.
							if ( COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, itemDefineIndex ) != true )
								return inventory->itemDefineIndex;
							else
								return 0;
						}
					}
					else
						NETWORK2->PostServerEvent( "ERROR - cPlayer::IsItemUseMapChange:TB_ITEM_ABILITY::IDX(=%d)", itemDefine->idxAbility );
				}
			}
			else
				NETWORK2->PostServerEvent( "ERROR - cPlayer::IsItemUseMapChange:TB_ITEM_DEFINE::IDX(=%d)", inventory->itemDefineIdx );
		}
	}
	return 0;
}


// ItemUseMapChange Method.
bool cPlayer::ItemUseMapChange(long inventroyIdx)
{
	TB_INVENTORY* inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	bool          apply     = false;

	HANDLE        handle   = NULL;
	ITEM_USE*     itemUse  = (ITEM_USE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_USE );
	long&         rowCount = itemUse->rowCount;
	TB_INVENTORY* table    = itemUse->table;
	long          length   = sizeof(ITEM_USE)-sizeof(itemUse->table);

	itemUse->characterIdx = mObject.index;

    for ( int i = INVENTORY_BAG_BEGIN; (i <= mHeroInfo.BagEnd) && (apply == false); i++, inventory++ )
	{
		if ( inventory->idx == inventroyIdx )
		{
			(*table) = (*inventory);
			table->count = 1;
			table++; rowCount++;

			// 쿨타임 적용.
			COOLTIMEPOOL->ApplyCooltime( &mCooltimeRoot, inventory );

			// 가상처리; 차감을 완성 해 놓는다.
			UpdateInventory( inventory, (-1) );

			// 가상처리; 사용된 아이템 정보를 전송한다.
			SendItemUse( inventory );

			// 가상처리; 사용이 완료된 인벤토리 삭제.
			if ( inventory->count == 0 )
				RemoveInventory( inventory );

			// 적용완료.
			apply = true;
		}
	}

	length += sizeof(itemUse->table) * rowCount;
	if ( apply == true )
	{
		return NETWORK2->SendSQL( handle, length );
	}
	else
	{
		NETWORK2->ReleaseSQL( handle, length );
		return false;
	}
}

void cPlayer::ItemUseCashMapChange( long inventoryIdx )
{
	TB_INVENTORY* inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	bool          apply     = false;

	for ( int i = INVENTORY_BAG_BEGIN; (i <= mHeroInfo.BagEnd) && (apply == false); i++, inventory++ )
	{
		if ( inventory->idx == inventoryIdx )
		{
			// 쿨타임 적용.
			COOLTIMEPOOL->ApplyCooltime( &mCooltimeRoot, inventory );
			return;
		}
	}
}

// SendItemUse Method.
bool cPlayer::SendItemUse(TB_INVENTORY* inventory)
{
	HANDLE            handle  = NULL;
	MSG_RES_ITEM_USE* sendMsg = (MSG_RES_ITEM_USE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_USE_RES );
	if ( sendMsg != NULL )
	{
		Inventory2sInventory( &sendMsg->inventory, inventory );
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_USE) );
	}
	return false;
}

// IsItemReward Method - 분해될 아이템은 비어있는 인벤토리로 간주한다.
unsigned int cPlayer::IsItemReward(long inventoryIdx, eDisjointItem* items, int maxCount)
{
	TB_ITEM_DEFINE* itemDefine  = NULL;
	TB_INVENTORY*   inventory   = NULL;
	short           inventoryCount;

	int             emptyCount  = 0;
	int             applyCount  = 0;

	// 남은 인벤토리 수량 검토 - 분해될 아이템은 남은 인벤토리로 처리.
	inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( int i0 = INVENTORY_BAG_BEGIN; i0 <= mHeroInfo.BagEnd && emptyCount < maxCount; i0++, inventory++ )
	{
		if ( (inventory->idx == 0) || (inventory->idx == inventoryIdx) )
			emptyCount++;
	}

	// 남은 인벤토리 수량이 보상받을 아이템 수량보다 적은 경우, 겹치기 가능 아이템 검토.
	if ( emptyCount < maxCount )
	{
		for ( int i0 = 0; i0 < maxCount; i0++, items++ )
		{
			itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemDefineIndex );

			if ( itemDefine == NULL )
			{
				return applyCount;
			}
			else if ( (!items->inventoryCount) || (itemDefine->capacity < items->inventoryCount) )
			{
				return applyCount;
			}
			else if ( (emptyCount + applyCount) < maxCount )
			{
				inventory      = SelectInventory( INVENTORY_BAG_BEGIN );
				inventoryCount = 0;

				for ( int i1 = INVENTORY_BAG_BEGIN; i1 <= mHeroInfo.BagEnd && inventoryCount < items->inventoryCount; i1++, inventory++ )
				{
					// 분해될 아이템은 무시
					if ( inventory->idx == inventoryIdx )
						continue;

					// 남은 수량 누적.
					if ( inventory->itemDefineIndex == (long)items->itemDefineIndex )
						inventoryCount += (itemDefine->capacity - inventory->count);
				}

				// 겹치기 확인후 적용행 수 증가.
				if ( inventoryCount >= items->inventoryCount )
				{
					applyCount++;
				}
				// 겹치기 실패시 남은 인벤토리 차감후 적용행 수 증가.
				else if ( emptyCount > 0 )
				{
					emptyCount--;
					applyCount++;
				}
			}
			else
				applyCount++;
		}
	}
	// 남은 인벤토리 수량이 보상받을 아이템 수량보다 큰 경우, 아이템 정의 테이블만 검토.
	else
	{
		for ( int i0 = 0; i0 < maxCount; i0++, items++ )
		{
			itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemDefineIndex );

			if ( itemDefine == NULL )
			{
				return applyCount;
			}
			else if ( (!items->inventoryCount) || (itemDefine->capacity < items->inventoryCount) )
			{
				return applyCount;
			}
			else
				applyCount++;
		}
	}

	return applyCount;
}

// IsItemReward Method.
unsigned int cPlayer::IsItemReward(sQuestItem* items, int maxCount, short slotBegin, short slotEnd )
{
	TB_ITEM_DEFINE* itemDefine  = NULL;
	TB_INVENTORY*   inventory   = NULL;
	unsigned int    inventoryCount;

	int             emptyCount  = 0;
	int             applyCount  = 0;

	// 남은 인벤토리 수량 검토.
	inventory = SelectInventory( slotBegin );
	for ( int i0 = slotBegin; i0 <= slotEnd && emptyCount < maxCount; i0++, inventory++ )
	{
		if ( !inventory->idx )
			emptyCount++;
	}

	// 남은 인벤토리 수량이 보상받을 아이템 수량보다 적은 경우, 겹치기 가능 아이템 검토.
	if ( emptyCount < maxCount )
	{
		for ( int i0 = 0; i0 < maxCount; i0++, items++ )
		{
			itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );

			if ( itemDefine == NULL )
			{
				return applyCount;
			}
			else if ( (!items->count) || (itemDefine->capacity < (short)items->count) )
			{
				return applyCount;
			}
			else if ( (emptyCount + applyCount) < maxCount )
			{
				inventory      = SelectInventory( slotBegin );
				inventoryCount = 0;

				for ( int i1 = slotBegin; i1 <= slotEnd && inventoryCount < items->count; i1++, inventory++ )
				{
					if ( inventory->itemDefineIndex == (long)items->itemIndex )
					{
						inventoryCount += (itemDefine->capacity - inventory->count);
					}
				}

				// 겹치기 확인후 적용행 수 증가.
				if ( inventoryCount >= items->count )
				{
					applyCount++;
				}
				// 겹치기 실패시 남은 인벤토리 차감후 적용행 수 증가.
				else if ( emptyCount > 0 )
				{
					emptyCount--;
					applyCount++;
				}
			}
			else
				applyCount++;
		}
	}
	// 남은 인벤토리 수량이 보상받을 아이템 수량보다 큰 경우, 아이템 정의 테이블만 검토.
	else
	{
		for ( int i0 = 0; i0 < maxCount; i0++, items++ )
		{
			itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );

			if ( itemDefine == NULL )
			{
				return applyCount;
			}
			else if ( (!items->count) || (itemDefine->capacity < (short)items->count) )
			{
				return applyCount;
			}
			else
				applyCount++;
		}
	}

	return applyCount;


	//// 남은 인벤토리 수량 검토.
	//inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	//for ( int i0 = INVENTORY_BAG_BEGIN; i0 <= mHeroInfo.BagEnd && emptyCount < maxCount; i0++, inventory++ )
	//{
	//	if ( !inventory->idx )
	//		emptyCount++;
	//}

	//// 남은 인벤토리 수량이 보상받을 아이템 수량보다 적은 경우, 겹치기 가능 아이템 검토.
	//if ( emptyCount < maxCount )
	//{
	//	for ( int i0 = 0; i0 < maxCount; i0++, items++ )
	//	{
	//		itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );

	//		if ( itemDefine == NULL )
	//		{
	//			return applyCount;
	//		}
	//		else if ( (!items->count) || (itemDefine->capacity < (short)items->count) )
	//		{
	//			return applyCount;
	//		}
	//		else if ( (emptyCount + applyCount) < maxCount )
	//		{
	//			inventory      = SelectInventory( INVENTORY_BAG_BEGIN );
	//			inventoryCount = 0;

	//			for ( int i1 = INVENTORY_BAG_BEGIN; i1 <= mHeroInfo.BagEnd && inventoryCount < items->count; i1++, inventory++ )
	//			{
	//				if ( inventory->itemDefineIndex == (long)items->itemIndex )
	//				{
	//					inventoryCount += (itemDefine->capacity - inventory->count);
	//				}
	//			}

	//			// 겹치기 확인후 적용행 수 증가.
	//			if ( inventoryCount >= items->count )
	//			{
	//				applyCount++;
	//			}
	//			// 겹치기 실패시 남은 인벤토리 차감후 적용행 수 증가.
	//			else if ( emptyCount > 0 )
	//			{
	//				emptyCount--;
	//				applyCount++;
	//			}
	//		}
	//		else
	//			applyCount++;
	//	}
	//}
	//// 남은 인벤토리 수량이 보상받을 아이템 수량보다 큰 경우, 아이템 정의 테이블만 검토.
	//else
	//{
	//	for ( int i0 = 0; i0 < maxCount; i0++, items++ )
	//	{
	//		itemDefine = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );

	//		if ( itemDefine == NULL )
	//		{
	//			return applyCount;
	//		}
	//		else if ( (!items->count) || (itemDefine->capacity < (short)items->count) )
	//		{
	//			return applyCount;
	//		}
	//		else
	//			applyCount++;
	//	}
	//}

	//return applyCount;
}

// IsItemTake Method.
unsigned int cPlayer::IsItemTake(sQuestItem* items, int maxCount)
{
	TB_INVENTORY* inventory  = NULL;
	int           remainCount;

	int           applyCount = 0;

	for ( int i0 = 0; i0 < maxCount; i0++, items++ )
	{
		inventory   = SelectInventory( INVENTORY_BAG0_BEGIN );
		remainCount = items->count;
		for ( int i1 = INVENTORY_BAG0_BEGIN; i1 <= INVENTORY_QUEST_END; i1++, inventory++ )
		{
			if ( inventory->itemDefineIndex == (long)items->itemIndex )
			{
				remainCount -= inventory->count;
				if ( !(remainCount > 0) )
				{
					applyCount++;
					break;
				}
			}
		}
	}

	return applyCount;
}

/// 해당 퀘스트 아이템 검사 
unsigned int cPlayer::GetQuestItemCount( long itemDefineIndex, int maxCount )
{
	TB_INVENTORY* inventory  = SelectInventory( INVENTORY_QUEST_BEGIN );
	int           totalCount = 0;

	for ( int i = INVENTORY_QUEST_BEGIN; i <= INVENTORY_QUEST_END; i++, inventory++ )
	{
		if ( inventory->idx > 0 &&
			inventory->count > 0 &&
			inventory->itemDefineIndex == itemDefineIndex )
		{
			totalCount += inventory->count;
			if ( totalCount >= maxCount )
				return maxCount;
		}
	}

	return totalCount;
}

// EquipItems Method - 착용 아이템.
void cPlayer::EquipItems( )
{
	/// 발동 효과 초기화
	ClearActInfluence();

	short leftHand;
	short rightHand;

	// 활성상태 검사.
	if ( mItemActiveWeapon == ItemActiveFront )
	{
		leftHand  = INVENTORY_HAND_LEFT2;
		rightHand = INVENTORY_HAND_RIGHT2;
	}
	else
	{
		leftHand  = INVENTORY_HAND_LEFT1;
		rightHand = INVENTORY_HAND_RIGHT1;
	}

	// 캐릭터의 무기 상태 정보 갱신.
	{
		int           weaponState = eWEAPON_STATE_NONE;
		TB_INVENTORY* inventory;

		inventory = SelectInventory( leftHand );
		for ( short i = leftHand; i <= rightHand; i++, inventory++ )
		{
			if ( IsInventory( inventory ) && IsItemLimit( inventory->itemDefineIdx ) )
			{
				if ( ITEMMANAGER->GetItemDefineType( inventory->itemDefineIdx ) == ITEM_WEAPON )
				{
					switch ( ITEMMANAGER->GetItemDefineSubType( inventory->itemDefineIdx ) )
					{
					case ITEM_WEAPON_SWORD:  weaponState += eWEAPON_STATE_SWORD;       break;
					case ITEM_WEAPON_BLADE:  weaponState += eWEAPON_STATE_LONGSWORD;   break;
					case ITEM_WEAPON_DUAL:   weaponState += eWEAPON_STATE_DOUBLESWORD; break;
					case ITEM_WEAPON_CUTTER: weaponState += eWEAPON_STATE_SHORTSWORD;  break;
					case ITEM_WEAPON_GUN:    weaponState += eWEAPON_STATE_GUN;         break;
					case ITEM_WEAPON_STAFF:  weaponState += eWEAPON_STATE_STAFF;       break;
					case ITEM_WEAPON_SHIELD: weaponState += eWEAPON_STATE_SHEILD;      break;
					}
				}
			}
		}

		mPlayerWeaponState = (eWEAPON_STATE)weaponState;
	}

	// 캐릭터의 착용 아이템 정보 갱신.
	{
		// 방어구
		unsigned long wearHat   = SelectInventory( INVENTORY_WEAR_HAT   )->itemDefineIndex;
		unsigned long wearBody1 = SelectInventory( INVENTORY_WEAR_BODY1 )->itemDefineIndex;
		unsigned long wearBody2 = SelectInventory( INVENTORY_WEAR_BODY2 )->itemDefineIndex;
		unsigned long wearHand  = SelectInventory( INVENTORY_WEAR_HAND  )->itemDefineIndex;
		unsigned long wearFoot  = SelectInventory( INVENTORY_WEAR_FOOT  )->itemDefineIndex;

		SetWearInfoWithSendMsg( eWEAR_HAT,   wearHat   ); // 모자
		SetWearInfoWithSendMsg( eWEAR_BODY1, wearBody1 ); // 바디1 - 상의
		SetWearInfoWithSendMsg( eWEAR_BODY2, wearBody2 ); // 바디2 - 하의
		SetWearInfoWithSendMsg( eWEAR_HAND,  wearHand  ); // 장갑
		SetWearInfoWithSendMsg( eWEAR_FOOT,  wearFoot  ); // 신발

		// 액세서리
		mPlayerWearInfo.Earring  = SelectInventory( INVENTORY_WEAR_EARRING  )->itemDefineIndex;
		mPlayerWearInfo.Necklace = SelectInventory( INVENTORY_WEAR_NECKLACE )->itemDefineIndex;
		mPlayerWearInfo.Brooch   = SelectInventory( INVENTORY_WEAR_BROOCH   )->itemDefineIndex;
		mPlayerWearInfo.Bracelet = SelectInventory( INVENTORY_WEAR_BRACELET )->itemDefineIndex;
		mPlayerWearInfo.Ring     = SelectInventory( INVENTORY_WEAR_RING     )->itemDefineIndex;

		// 무기
		SetWeaponInfoWithSendMsg( SelectInventory( leftHand  ), SelectInventory( rightHand ) );

        // 코스튬 아이템
		unsigned long cosBody1 = SelectInventory( INVENTORY_COSTUME_BODY1 )->itemDefineIndex;
        unsigned long cosBody2 = SelectInventory( INVENTORY_COSTUME_BODY2 )->itemDefineIndex;
		unsigned long cosHand  = SelectInventory( INVENTORY_COSTUME_HAND  )->itemDefineIndex;
		unsigned long cosFoot  = SelectInventory( INVENTORY_COSTUME_FOOT  )->itemDefineIndex;
        unsigned long cosHat   = SelectInventory( INVENTORY_COSTUME_HAT   )->itemDefineIndex;
        unsigned long cosFace  = SelectInventory( INVENTORY_COSTUME_FACE  )->itemDefineIndex;
        unsigned long cosBack  = SelectInventory( INVENTORY_COSTUME_BACK  )->itemDefineIndex;

		SetCosWearInfoWithSendMsg( eWEAR_BODY1, cosBody1 );	// 코스튬 바디1 - 상의
		SetCosWearInfoWithSendMsg( eWEAR_BODY2, cosBody2 );	// 코스튬 바디2 - 하의
		SetCosWearInfoWithSendMsg( eWEAR_HAND,  cosHand  );	// 코스튬 장갑
		SetCosWearInfoWithSendMsg( eWEAR_FOOT,  cosFoot  );	// 코스튬 신발
		SetCosWearInfoWithSendMsg( eWEAR_HAT,   cosHat   );	// 코스튬 모자

        SetCosAccInfoWithSendMsg( eCOSTUME_HEAD, cosFace );	// 코스튬 장식 - 얼굴
        SetCosAccInfoWithSendMsg( eCOSTUME_BACK, cosBack );	// 코스튬 장식 - 몸통
	}

	// 캐릭터의 방어 아이템 세트 갱신.
	ResetArmorSets( );

	// 캐릭터의 착용 아이템 속성 갱신.
	{
		TB_INVENTORY*     inventory;
		TB_ITEM_DEFINE*   itemDefine;
		TB_ITEM_ABILITY*  itemAbility;
		TB_ITEM_ENHANCED* itemEnhanced;

		memset( &mEquipAbility, 0, sizeof(mEquipAbility) );

		tHashSet<unsigned long> ItemSet;

		// 아이템 누적 속성 - 착용 방어구 & 액세서리 & 카드.
		inventory = SelectInventory( INVENTORY_WEAR_BEGIN );
        for ( int i = INVENTORY_WEAR_BEGIN; i <= mHeroInfo.WearEnd; i++, inventory++ )
		{
			if ( IsInventory( inventory ) && IsItemLimit( inventory->itemDefineIdx ) && !IsInventorySeal( inventory ) )
			{
				itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );

				/// set 정보 입력
				if( itemDefine->setIndex != 0 )
					ItemSet.NoneUniqueInsert( itemDefine->setIndex );

				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( itemAbility != NULL )
				{
					AddEquipAbility( itemAbility );

					if( itemAbility->active_index != 0 )
						AddActInfluence( itemAbility->active_index );
				}
				if ( !(inventory->enhanced < MIN_ITEM_ENHANCED || inventory->enhanced > MAX_ITEM_ENHANCED) )
				{
					itemEnhanced = ITEMMANAGER->GetItemEnhanced( inventory->itemDefineIdx );
					if ( itemEnhanced != NULL )
					{
						AddEquipEnhanced( itemEnhanced, inventory->enhanced );
					}
				}
				AddEquipCards( inventory );
			}
		}

		// 아이템 누적 속성 - 코스튬 아이템
		inventory = SelectInventory( INVENTORY_COSTUME_HAT );
        for ( int i = INVENTORY_COSTUME_HAT; i <= INVENTORY_COSTUME_BACK; i++, inventory++ )
		{
			if ( IsInventory( inventory ) && IsItemLimit( inventory->itemDefineIdx ) && !IsInventorySeal( inventory ) )
			{
				itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );

				/// set 정보 입력
				if( itemDefine->setIndex != 0 )
					ItemSet.NoneUniqueInsert( itemDefine->setIndex );

				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( itemAbility != NULL )
				{
					AddEquipAbility( itemAbility );

					if( itemAbility->active_index != 0 )
						AddActInfluence( itemAbility->active_index );
				}
				AddEquipCards( inventory );
			}
		}

		// 아이템 누적 속성 - 착용 무기.
		inventory = SelectInventory( leftHand );
		for ( int i = leftHand; i <= rightHand; i++, inventory++ )
		{
			if ( IsInventory( inventory ) && IsItemLimit( inventory->itemDefineIdx ) && !IsInventorySeal( inventory ) )
			{
				itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );

				/// set 정보 입력
				if( itemDefine->setIndex != 0 )
					ItemSet.NoneUniqueInsert( itemDefine->setIndex );

				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( itemAbility != NULL )
				{
					AddEquipAbility( itemAbility );

					if( itemAbility->active_index != 0 )
						AddActInfluence( itemAbility->active_index );
				}
				if ( !(inventory->enhanced < MIN_ITEM_ENHANCED || inventory->enhanced > MAX_ITEM_ENHANCED) )
				{
					itemEnhanced = ITEMMANAGER->GetItemEnhanced( inventory->itemDefineIdx );
					if ( itemEnhanced != NULL )
					{
						AddEquipEnhanced( itemEnhanced, inventory->enhanced );
					}
				}
				AddEquipCards( inventory );
			}
		}

		// SET 아이템의 OPT 값 추가.
		/// 세트 정보 갱신
		tHashSet<unsigned long>::cIterator b = ItemSet.Begin();
		tHashSet<unsigned long>::cIterator end = ItemSet.End();
		for(; b != end; )
		{
			/// 세트 인덱스와 갯수
			unsigned long idx = (unsigned long)(*b);
			unsigned int checkCnt = ItemSet.GetCount(idx);

			cItemSetInfo* pInfo = ITEMMANAGER->GetItemSetInfo( idx );
			if( pInfo )
			{
				/// 갯수에 따른 설정
				for( unsigned int c=1; c<=checkCnt; ++c )
				{
					sSetAblityOption* opt = pInfo->GetItemOption(c);
					if( opt )
					{
						if( opt->mOptType[0] != 0 )
							AddEquipAbility( opt->mOptType[0], opt->mOptValue[0] );

						if( opt->mOptType[1] != 0 )
							AddEquipAbility( opt->mOptType[1], opt->mOptValue[1] );

						/// 발동 적용
						if( opt->mActIdx != 0 )
							AddActInfluence( opt->mActIdx );
					}
				}
			}

			/// reset
			ItemSet.Erase(idx);

			b = ItemSet.Begin();
			end = ItemSet.End();
		}
	}
}

// CalcItemWeight Method.
void cPlayer::CalcItemWeight( )
{
	TB_INVENTORY*   inventory;
	TB_ITEM_DEFINE* itemDefine;

	mItemWeight = 0;

	inventory = SelectInventory( INVENTORY_CHARACTER_BEGIN );
	for ( int i = INVENTORY_CHARACTER_BEGIN; i <= INVENTORY_CHARACTER_END; ++i, ++inventory )
	{
		itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
		if ( itemDefine != NULL )
		{
			mItemWeight = mItemWeight + (itemDefine->weight * inventory->count);
		}
	}

	/// 퀘스트 갱신
	UpdateDutyItem();
}

void cPlayer::AddEquipAbility(short opt, short value)
{
#pragma warning( push )
#pragma warning( disable: 4244 )

	switch ( opt )
	{
	case ITEM_OPT_STR:
		mEquipAbility.mOptStr += value;		// 힘
		break;
	case ITEM_OPT_DEX:
		mEquipAbility.mOptDex += value;		// 민첩
		break;
	case ITEM_OPT_CON:
		mEquipAbility.mOptCon += value;		// 체력
		break;
	case ITEM_OPT_INT:
		mEquipAbility.mOptInt += value;		// 지능
		break;
	case ITEM_OPT_WIS:
		mEquipAbility.mOptWis += value;		// 지혜
		break;
	case ITEM_OPT_ALL:	// 모두(힘:민첩:체력:지능:지혜)
		mEquipAbility.mOptStr += value;
		mEquipAbility.mOptDex += value;
		mEquipAbility.mOptCon += value;
		mEquipAbility.mOptInt += value;
		mEquipAbility.mOptWis += value;
		break;

		// Option - Plus
	case ITEM_OPT_PLUS_PHYSIC_ATTACK:
		mEquipAbility.mOptPlusPhysicAttack += value;			// 물리공격력
		break;
	case ITEM_OPT_PLUS_MAGIC_ATTACK:
		mEquipAbility.mOptPlusMagicAttack += value;				// 마법공격력
		break;
	case ITEM_OPT_PLUS_PHYSIC_DEFENSE:
		mEquipAbility.mOptPlusPhysicDefense += value;			// 물리방어력
		break;
	case ITEM_OPT_PLUS_MAGIC_DEFENSE:
		mEquipAbility.mOptPlusMagicDefense += value;			// 마법방어력
		break;
	case ITEM_OPT_PLUS_PHYSIC_ATTACK_RATE:
		mEquipAbility.mOptPlusPhysicAttackRate += value;		// 물리명중률
		break;
	case ITEM_OPT_PLUS_MAGIC_ATTACK_RATE:
		mEquipAbility.mOptPlusMagicAttackRate += value;				// 마법명중률
		break;
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL:
		mEquipAbility.mOptPlusPhysicCritical += value;			// 물리크리티컬
		break;
	case ITEM_OPT_PLUS_MAGIC_CRITICAL:
		mEquipAbility.mOptPlusMagicCritical += value;			// 마법크리티컬
		break;
	case ITEM_OPT_PLUS_DOD:
		mEquipAbility.mOptPlusDod += value;						// 회피
		break;
	case ITEM_OPT_PLUS_WEI:
		mEquipAbility.mOptPlusWei += value;						// 무게
		break;
	case ITEM_OPT_PLUS_MAX_HP:
		mEquipAbility.mOptPlusMaxHp += value;					// 최대량 HP
		break;
	case ITEM_OPT_PLUS_MAX_MP:
		mEquipAbility.mOptPlusMaxMp += value;					// 최대량 MP
		break;
	case ITEM_OPT_PLUS_MOV:
		mEquipAbility.mOptPlusMov += value;						// 이동
		break;
	case ITEM_OPT_PLUS_HP:
		mEquipAbility.mOptPlusHp += value;						// HP 회복
		break;
	case ITEM_OPT_PLUS_MP:
		mEquipAbility.mOptPlusMp += value;						// MP 회복
		break;
	case ITEM_OPT_PLUS_ATTACK_SPEED:
		mEquipAbility.mOptPlusAttackSpeed += value;				// 일반공격속도
		break;
	case ITEM_OPT_PLUS_SKILL_COOLTIME_SPEED:
		mEquipAbility.mOptPlusSkillCooltimeSpeed += value;		// 스킬쿨타임속도
		break;
	case ITEM_OPT_PLUS_SHORT_RANGE:
		mEquipAbility.mOptPlusShortRange += value;				// 근거리사정거리
		break;
	case ITEM_OPT_PLUS_DISTANCE_RANGE:
		mEquipAbility.mOptPlusDistanceRange += value;			// 원거리사정거리
		break;
	case ITEM_OPT_PLUS_HEAL:
		mEquipAbility.mOptPlusHeal += value;					// 힐 회복량
		break;
	case ITEM_OPT_PLUS_HP_RECOVERTY:
		mEquipAbility.mOptPlusHpReceive += value;				// HP 회복시 증가량
		break;
	case ITEM_OPT_PLUS_ALL_ATTACK:
		mEquipAbility.mOptPlusPhysicAttack += value;			// 물리공격력
		mEquipAbility.mOptPlusMagicAttack += value;				// 마법공격력
		break;
	case ITEM_OPT_PLUS_ALL_DEFENSE:
		mEquipAbility.mOptPlusPhysicDefense += value;			// 물리방어력
		mEquipAbility.mOptPlusMagicDefense += value;			// 마법방어력
		break;
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL_DEC:
		mEquipAbility.mOptPlusPhysicCriticalDec += value;		// 물리크리티컬 감소
		break;
	case ITEM_OPT_PLUS_MAGIC_CRITICAL_DEC:
		mEquipAbility.mOptPlusMagicCriticalDec += value;		// 마법크리티컬 감소
		break;
	case ITEM_OPT_PLUS_RESISTANCE:
		mEquipAbility.mOptPlusResistance += value;				// 저항력
		break;

		// Option - Percent
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK:
		mEquipAbility.mOptPercentPhysicAttack += value;			// 물리공격력
		break;
	case ITEM_OPT_PERCENT_MAGIC_ATTACK:
		mEquipAbility.mOptPercentMagicAttack += value;			// 마법공격력
		break;
	case ITEM_OPT_PERCENT_PHYSIC_DEFENSE:
		mEquipAbility.mOptPercentPhysicDefense += value;		// 물리방어력
		break;
	case ITEM_OPT_PERCENT_MAGIC_DEFENSE:
		mEquipAbility.mOptPercentMagicDefense += value;			// 마법방어력
		break;
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK_RATE:
		mEquipAbility.mOptPercentPhysicAttackRate += value;		// 물리명중률
		break;
	case ITEM_OPT_PERCENT_MAGIC_ATTACK_RATE:
		mEquipAbility.mOptPercentMagicAttackRate += value;		// 마법명중률
		break;
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL:
		mEquipAbility.mOptPercentPhysicCritical += value;		// 물리크리티컬
		break;
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL:
		mEquipAbility.mOptPercentMagicCritical += value;		// 마법크리티컬
		break;
	case ITEM_OPT_PERCENT_DOD:
		mEquipAbility.mOptPercentDod += value;					// 회피
		break;
	case ITEM_OPT_PERCENT_WEI:
		mEquipAbility.mOptPercentWei += value;					// 무게
		break;
	case ITEM_OPT_PERCENT_MAX_HP:
		mEquipAbility.mOptPercentMaxHp += value;				// 최대량 HP
		break;
	case ITEM_OPT_PERCENT_MAX_MP:
		mEquipAbility.mOptPercentMaxMp += value;				// 최대량 MP
		break;
	case ITEM_OPT_PERCENT_MOV:
		mEquipAbility.mOptPercentMov += value;					// 이동
		break;
	case ITEM_OPT_PERCENT_HP:
		mEquipAbility.mOptPercentHp, value;						// HP 회복
		break;
	case ITEM_OPT_PERCENT_MP:
		mEquipAbility.mOptPercentMp += value;					// MP 회복
		break;
	case ITEM_OPT_PERCENT_ATTACK_SPEED:
		mEquipAbility.mOptPercentAttackSpeed += value;			// 일반공격속도
		break;
	case ITEM_OPT_PERCENT_SKILL_COOLTIME_SPEED:
		mEquipAbility.mOptPercentSkillCooltimeSpeed += value;	// 스킬쿨타임속도
		break;
	case ITEM_OPT_PERCENT_SHORT_RANGE:
		mEquipAbility.mOptPercentShortRange += value;			// 근거리사정거리
		break;
	case ITEM_OPT_PERCENT_DISTANCE_RANGE:
		mEquipAbility.mOptPercentDistanceRange += value;		// 원거리사정거리
		break;
	case ITEM_OPT_PERCENT_HEAL:
		mEquipAbility.mOptPercentHeal += value;					// 힐 회복량
		break;
	case ITEM_OPT_PERCENT_HP_RECOVERTY:
		mEquipAbility.mOptPercentHpReceive += value;			// HP 회복시 증가량
		break;
	case ITEM_OPT_PERCENT_ALL_ATTACK:
		mEquipAbility.mOptPercentPhysicAttack += value;			// 물리공격력
		mEquipAbility.mOptPercentMagicAttack += value;			// 마법공격력
		break;
	case ITEM_OPT_PERCENT_ALL_DEFENSE:
		mEquipAbility.mOptPercentPhysicDefense += value;		// 물리방어력
		mEquipAbility.mOptPercentMagicDefense += value;			// 마법방어력
		break;
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL_DEC:
		mEquipAbility.mOptPercentPhysicCriticalDec += value;	// 물리크리티컬 감소
		break;
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL_DEC:
		mEquipAbility.mOptPercentMagicCriticalDec += value;		// 마법크리티컬 감소
		break;
	case ITEM_OPT_PERCENT_RESISTANCE:
		mEquipAbility.mOptPercentResistance += value;			// 저항력
		break;

		// Option - Plus Etc
	case ITEM_OPT_PLUS_DRAIN_HP:
		mEquipAbility.mOptPlusDrainHp += value;					// HP 흡수
		break;
	case ITEM_OPT_PLUS_DRAIN_MP:
		mEquipAbility.mOptPlusDrainMp += value;					// MP 흡수
		break;
	case ITEM_OPT_PLUS_DAMAGE_DEC:
		mEquipAbility.mOptPlusDamageDec += value;				// 데미지 흡수/감소
		break;
	case ITEM_OPT_PLUS_DAMAGE_REF:
		mEquipAbility.mOptPlusDamageRef += value;				// 데미지 반사
		break;

		// Option - Percent Etc
	case ITEM_OPT_PERCENT_DRAIN_HP:
		mEquipAbility.mOptPercentDrainHp += value;				// HP 흡수
		break;
	case ITEM_OPT_PERCENT_DRAIN_MP:
		mEquipAbility.mOptPercentDrainMp += value;				// MP 흡수
		break;
	case ITEM_OPT_PERCENT_DAMAGE_DEC:
		mEquipAbility.mOptPercentDamageDec += value;			// 데미지 흡수/감소
		break;
	case ITEM_OPT_PERCENT_DAMAGE_REF:
		mEquipAbility.mOptPercentDamageRef += value;			// 데미지 반사
		break;
	}
#pragma warning( pop )
}

// AddEquipAbility Method.
void cPlayer::AddEquipAbility(TB_ITEM_ABILITY* itemAbility)
{
	// 기본 값 적용.
	mEquipAbility.mPhysicDefense    = mEquipAbility.mPhysicDefense    + itemAbility->p_def;
	mEquipAbility.mMagicDefense     = mEquipAbility.mMagicDefense     + itemAbility->m_def;
	mEquipAbility.mAttackRange      = mEquipAbility.mAttackRange      + itemAbility->atk_range;
	mEquipAbility.mAttackSpeed      = mEquipAbility.mAttackSpeed      + itemAbility->atk_spd;
    mEquipAbility.mHealValue        = mEquipAbility.mHealValue        + itemAbility->heal;

	mEquipAbility.mPhysicMinAttack  = mEquipAbility.mPhysicMinAttack  + itemAbility->min_p_atk;
	mEquipAbility.mPhysicMaxAttack  = mEquipAbility.mPhysicMaxAttack  + itemAbility->max_p_atk;
	mEquipAbility.mMagicMinAttack   = mEquipAbility.mMagicMinAttack   + itemAbility->min_m_atk;
	mEquipAbility.mMagicMaxAttack   = mEquipAbility.mMagicMaxAttack   + itemAbility->max_m_atk;

	// 옵션 값 적용.
	AddEquipAbility( itemAbility->opt01_index, itemAbility->opt01_value );
	AddEquipAbility( itemAbility->opt02_index, itemAbility->opt02_value );
	AddEquipAbility( itemAbility->opt03_index, itemAbility->opt03_value );
	AddEquipAbility( itemAbility->opt04_index, itemAbility->opt04_value );
	AddEquipAbility( itemAbility->opt05_index, itemAbility->opt05_value );
}

// AddEquipEnhanced Method - 강화단계는 MIN_ITEM_ENHANCED 에서 MAX_ITEM_ENHANCED 까지.
void cPlayer::AddEquipEnhanced(TB_ITEM_ENHANCED* itemEnhanced, BYTE enhanced)
{
	if ( MIN_ITEM_ENHANCED <= enhanced && enhanced <= MAX_ITEM_ENHANCED )
	{
		for ( int i= 0; i < itemEnhanced->row; i++ )
		{
			short value = itemEnhanced->value[ i ].value[ enhanced-MIN_ITEM_ENHANCED ];

			switch ( itemEnhanced->value[ i ].type )
			{
			case ITEM_ENHANCED_PHYSIC_ATTACK:
				mEquipAbility.mPhysicMinAttack = mEquipAbility.mPhysicMinAttack + value;
				mEquipAbility.mPhysicMaxAttack = mEquipAbility.mPhysicMaxAttack + value;
				break;
			case ITEM_ENHANCED_MAGIC_ATTACK:
				mEquipAbility.mMagicMinAttack = mEquipAbility.mMagicMinAttack + value;
				mEquipAbility.mMagicMaxAttack = mEquipAbility.mMagicMaxAttack + value;
				break;
			case ITEM_ENHANCED_PHYSIC_DEFENSE:
				mEquipAbility.mPhysicDefense = mEquipAbility.mPhysicDefense + value;
				break;
			case ITEM_ENHANCED_MAGIC_DEFENSE:
				mEquipAbility.mMagicDefense = mEquipAbility.mMagicDefense + value;
				break;
			case ITEM_ENHANCED_MAGIC_HEAL:
				mEquipAbility.mOptPlusHeal = mEquipAbility.mOptPlusHeal + value;
				break;
			}
		}
	}
}

// AddEquipCards Method.
void cPlayer::AddEquipCards(TB_INVENTORY* inventory)
{
	TB_ITEM_DEFINE*  itemDefine;
	TB_ITEM_ABILITY* itemAbility;

	itemDefine = ITEMMANAGER->GetItemDefineByIndex( inventory->cardSlot1 );
	if ( itemDefine != NULL )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
		if ( itemAbility != NULL )
		{
			AddEquipAbility( itemAbility );
		}
	}
	itemDefine = ITEMMANAGER->GetItemDefineByIndex( inventory->cardSlot2 );
	if ( itemDefine != NULL )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
		if ( itemAbility != NULL )
		{
			AddEquipAbility( itemAbility );
		}
	}
	itemDefine = ITEMMANAGER->GetItemDefineByIndex( inventory->cardSlot3 );
	if ( itemDefine != NULL )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
		if ( itemAbility != NULL )
		{
			AddEquipAbility( itemAbility );
		}
	}
	itemDefine = ITEMMANAGER->GetItemDefineByIndex( inventory->cardSlot4 );
	if ( itemDefine != NULL )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
		if ( itemAbility != NULL )
		{
			AddEquipAbility( itemAbility );
		}
	}
	itemDefine = ITEMMANAGER->GetItemDefineByIndex( inventory->cardSlot5 );
	if ( itemDefine != NULL )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
		if ( itemAbility != NULL )
		{
			AddEquipAbility( itemAbility );
		}
	}
}

// ResetArmorSets Method.
void cPlayer::ResetArmorSets( )
{
	TB_ITEM_DEFINE*  itemDefine;
	TB_ITEM_ABILITY* itemAbility;
	TB_INVENTORY*    inventory;

	// CLEAR - ARMOR SETS
	mArmorSets = ARMOR_SETS_NONE;

	// ARMOR SETS - HAT
	inventory = SelectInventory( INVENTORY_WEAR_HAT );
	if ( IsItemLimit( inventory->itemDefineIdx ) )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
		if ( itemAbility != NULL )
		{
			switch ( itemAbility->def_class )
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

	// ARMOR SETS - UPPER(BODY1)
	inventory = SelectInventory( INVENTORY_WEAR_BODY1 );
	if ( IsItemLimit( inventory->itemDefineIdx ) )
	{
		itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
		if ( itemDefine != NULL )
		{
			if ( itemDefine->subType == ITEM_WEAR_ONEPIECE )
			{
				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				switch ( itemAbility->def_class )
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
				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( itemAbility != NULL )
				{
					switch ( itemAbility->def_class )
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
	}

	// ARMOR SETS - LOWER(BODY2)
	inventory = SelectInventory( INVENTORY_WEAR_BODY2 );
	if ( IsItemLimit( inventory->itemDefineIdx ) )
	{
		itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
		if ( itemDefine != NULL )
		{
			if ( itemDefine->subType == ITEM_WEAR_ONEPIECE )
			{
				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				switch ( itemAbility->def_class )
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
				itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( itemAbility != NULL )
				{
					switch ( itemAbility->def_class )
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
	}

	// ARMOR SETS - HANDS(HAND)
	inventory = SelectInventory( INVENTORY_WEAR_HAND );
	if ( IsItemLimit( inventory->itemDefineIdx ) )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
		if ( itemAbility != NULL )
		{
			switch ( itemAbility->def_class )
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

	// ARMOR SETS - FEET(HAND)
	inventory = SelectInventory( INVENTORY_WEAR_FOOT );
	if ( IsItemLimit( inventory->itemDefineIdx ) )
	{
		itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
		if ( itemAbility != NULL )
		{
			switch ( itemAbility->def_class )
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

// AddCooltime Method.
void cPlayer::AddCooltime(TB_INVENTORY_COOLTIME* table)
{
	PerCooltime* perCooltime = COOLTIMEPOOL->GetCooltime( &mCooltimeRoot, table->itemDefineIndex );

	if ( perCooltime != NULL )
	{
		time_t    ltime;
		struct tm validThru;

		time( &ltime );
		validThru = *localtime( &ltime );
		validThru.tm_sec += (table->cooltime/1000);

		perCooltime->cooltime1 = table->cooltime1;
		perCooltime->cooltime2 = table->cooltime2;
		perCooltime->validThru = mktime( &validThru );
	}
}

// AddItemBill Method.
PerItemBill* cPlayer::AddItemBill(TB_ITEM_BILL* table)
{
	PerItemBill* itemBill = ITEMBILLPOOL->GetItemBill( &mItemBillRoot, table->inventoryIdx );
	struct tm    result;

	if ( itemBill != NULL )
	{
		itemBill->idx          = table->idx;
		itemBill->type         = table->type;
		itemBill->validDate    = table->validDate;
		itemBill->validTime    = table->validTime;
		itemBill->apply        = table->apply;

		time( &itemBill->timeBegin );
		itemBill->timeEnd = itemBill->timeBegin;

		result = *localtime( &itemBill->timeBegin );

		switch ( itemBill->type )
		{
		case ITEM_BILL_DATE:
			result.tm_sec += itemBill->validDate;
			itemBill->timeEnd = mktime( &result );
			break;
		case ITEM_BILL_TIME:
			result.tm_sec += itemBill->validTime;
			itemBill->timeEnd = mktime( &result );
			break;
		}
	}

	return itemBill;
}

// SaveItemBill Method.
bool cPlayer::SaveItemBill(long itemBillIdx, long elapsed, long before, long after, long inventoryIdx)
{
	HANDLE            handle = NULL;
	ITEM_BILL_UPDATE* update = (ITEM_BILL_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_BILL_UPDATE );

	update->characterIdx = mObject.index;

	update->idx          = itemBillIdx;
	update->elapsed      = elapsed;
	update->inventoryIdx = inventoryIdx;
	update->before       = before;
	update->after        = after;

	return NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(ITEM_BILL_UPDATE) );
}

// CheckInventory Method.
bool cPlayer::UpdateItemBill( )
{
	if ( mItemBillRoot.pool )
	{
		PerItemBill* itemBill = (PerItemBill*)mItemBillRoot.pool;
		PerItemBill* nextBill;
		time_t       current;

		long         elapsed;
		long         before;
		long         after;

		time( &current );

		while ( itemBill != NULL )
		{
			nextBill = (PerItemBill*)itemBill->next;

			if ( itemBill->type == ITEM_BILL_TIME )
			{
				elapsed = (long)difftime( current, itemBill->timeBegin );
				before  = itemBill->validTime;
				after   = itemBill->validTime-elapsed;

				SaveItemBill( itemBill->idx, elapsed, before, after, itemBill->inventoryIdx );
			}

			ITEMBILLPOOL->ReleaseItemBill( &mItemBillRoot, itemBill );
			itemBill = nextBill;
		}
	}
	return false;
}

// StartItemBill Method.
void cPlayer::StartItemBill( )
{
	mItemBillStart = true;

	if ( mItemBillRoot.pool != NULL )
	{
		PerItemBill*       itemBillPool = (PerItemBill*)mItemBillRoot.pool;
		struct tm          result;

		HANDLE             handle       = NULL;
		MSG_SYN_ITEM_BILL* sendSyn      = (MSG_SYN_ITEM_BILL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_BILL_SYN );

		if ( sendSyn != NULL )
		{
			long&      rowCount = sendSyn->rowCount;
			sItemBill* itemBill = sendSyn->itemBill;
			long       length   = sizeof(MSG_SYN_ITEM_BILL)-sizeof(sendSyn->itemBill);

			rowCount = 0;
			while ( itemBillPool != NULL )
			{
				// 시간제만 정산.
				if ( itemBill->type == ITEM_BILL_TIME )
				{
					time( &itemBillPool->timeBegin );
					itemBillPool->timeEnd = itemBillPool->timeBegin;

					result = *localtime( &itemBillPool->timeBegin );

					result.tm_sec += itemBillPool->validTime;
					itemBillPool->timeEnd = mktime( &result );
				}

				itemBill->idx          = itemBillPool->idx;
				itemBill->type         = itemBillPool->type;
				itemBill->validDate    = itemBillPool->timeEnd;
				itemBill->validTime    = itemBillPool->validTime;
				itemBill->inventoryIdx = itemBillPool->inventoryIdx;

				rowCount++;
				itemBill++;

				itemBillPool = (PerItemBill*)itemBillPool->next;
			}

			length += (rowCount * sizeof(sendSyn->itemBill));
			NETWORK2->SendMsgRoot( handle, length );
		}
	}
}

// StopItemBill Method.
void cPlayer::StopItemBill( )
{
	PerItemBill* itemBillPool = (PerItemBill*)mItemBillRoot.pool;
	time_t       current;

	long         elapsed;
	long         before;
	long         after;

	time( &current );

	while ( itemBillPool != NULL )
	{
		// 시간제만 정산.
		if ( itemBillPool->type == ITEM_BILL_TIME )
		{
			elapsed = (long)difftime( current, itemBillPool->timeBegin );
			before  = itemBillPool->validTime;
			after   = itemBillPool->validTime-(before < elapsed ? before : elapsed);

			SaveItemBill( itemBillPool->idx, elapsed, before, after, itemBillPool->inventoryIdx );
			itemBillPool->validTime = after;
			itemBillPool->timeBegin = current;
		}

		itemBillPool = (PerItemBill*)itemBillPool->next;
	}

	mItemBillStart = false;
}

// CheckInventory Method.
bool cPlayer::CheckInventory(long itemDefineIndex, int itemCount)
{
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );

	if ( itemDefine != NULL )
	{
		TB_INVENTORY* inventory   = SelectInventory( INVENTORY_BAG_BEGIN );
		int           remainCount = itemCount;

        for ( int i = INVENTORY_BAG_BEGIN; (i <= mHeroInfo.BagEnd) && (remainCount > 0); i++, inventory++ )
		{
			if ( IsInventory( inventory ) == false )
			{
				remainCount -= itemDefine->capacity;
			}
			else if ( inventory->itemDefineIndex == itemDefineIndex )
			{
				remainCount -= (itemDefine->capacity - inventory->count);
			}
		}

		return (remainCount > 0) ? false : true;
	}

	return false;
}

// XCopyInventory Method.
TB_INVENTORY* cPlayer::XCopyInventory(TB_INVENTORY* destination, TB_INVENTORY* source, XCOPY_INVENTORY_OPT opt)
{
	size_t       length = sizeof(TB_INVENTORY);
	TB_INVENTORY tempInv;
	short        tempNum;

	switch ( opt )
	{
	case XCOPY_INVENTORY_ALL:
		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, source->itemDefineIndex, source->number, (source->count - destination->count) );
		memcpy( destination, source, length );
		break;
	case XCOPY_INVENTORY_COUNT:
		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, source->itemDefineIndex, source->number, (source->count - destination->count) );
		destination->count = source->count;
		break;
	case XCOPY_INVENTORY_SWAP:
		tempInv = (*destination);

		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, source->itemDefineIndex, destination->number, source->count );
			tempNum = destination->number;
			memcpy( destination, source, length );
			destination->number = tempNum;
		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, source->itemDefineIndex, source->number, (-source->count) );

		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, tempInv.itemDefineIndex, source->number, tempInv.count );
			tempNum = source->number;
			memcpy( source, &tempInv, length );
			source->number = tempNum;
		ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, tempInv.itemDefineIndex, tempInv.number, (-tempInv.count) );
		break;
	}
	mItemChange = true;
	mXCopyComplete = true;
	return destination;
}

// XCopyInventory Method.
TB_INVENTORY* cPlayer::XCopyInventory(TB_INVENTORY* source, XCOPY_INVENTORY_OPT opt)
{
	TB_INVENTORY* destination = SelectInventory( source->number );
	if ( destination != NULL )
	{
		return XCopyInventory( destination, source, opt );
	}
	return NULL;
}

TB_INVENTORY* cPlayer::SelectInventoryIdx(long idx, short begin, short end)
{
	TB_INVENTORY* inventory = SelectInventory( begin );
    for ( short i = begin; i <= end; i++, inventory++ )
	{
		if ( inventory->idx == idx )
			return inventory;
	}
	return NULL;
}

// FindFirstInventory Method(number의 값은 자동 초기화됨).
bool cPlayer::FindFirstInventory(TB_ITEM_DEFINE* itemDefine, short* number, short remainCount, short* pcvValue, bool overlap)
{
	if ( itemDefine->type != ITEM_QUEST )
	{
		(*number)=INVENTORY_BAG_BEGIN;
		return FindNextInventory( itemDefine, number, remainCount, pcvValue, overlap );
	}
	else
	{
		(*number)=INVENTORY_QUEST_BEGIN;
		return FindNextInventory( itemDefine, number, remainCount, pcvValue, overlap );
	}
}
// FindNextInventory Method(number의 값은 수동 증가 해야됨).
bool cPlayer::FindNextInventory(TB_ITEM_DEFINE* itemDefine, short* number, short remainCount, short* pcvValue, bool overlap)
{
	if ( !(remainCount > 0) )
		return false;

	if ( overlap == true )
	{
		short begin = (*number);
        short end   = (short)((itemDefine->type != ITEM_QUEST) ? mHeroInfo.BagEnd : INVENTORY_QUEST_END);

		TB_INVENTORY* inventory = SelectInventory( begin );
		for ( short i = begin; i <= end; i++, inventory++ )
		{
			if ( IsInventory( inventory ) == true && 
                 inventory->itemDefineIndex == itemDefine->index && 
                 inventory->count < itemDefine->capacity && 
                 inventory->cash == 0
               )
			{
				if ( (inventory->count + remainCount) > itemDefine->capacity )
					(*pcvValue) = itemDefine->capacity - inventory->count;
				else
					(*pcvValue) = remainCount;

				(*number) = i;
				return true;
			}
		}
	}
	else
	{
		if ( itemDefine->type != ITEM_QUEST )
		{
			(*number) = GetEmptyBagNumber( (*number) );
			if ( (*number) != INVENTORY_BAG_NONE )
			{
				(*pcvValue) = remainCount;
				return true;
			}
		}
		else
		{
			(*number) = GetEmptyQuestNumber( (*number) );
			if ( (*number) != INVENTORY_QUEST_NONE )
			{
				(*pcvValue) = remainCount;
				return true;
			}
		}
	}
	return false;
}

// CreateInventory Method.
TB_INVENTORY* cPlayer::CreateInventory(TB_ITEM_DEFINE* itemDefine, short number, short count, short* pcbValue)
{
	TB_INVENTORY* inventory = SelectInventory( number );

	if ( inventory != NULL )
	{
		if ( IsInventory( inventory ) == false )
		{
			short         shortCount = (count > itemDefine->capacity) ? itemDefine->capacity : count;
			ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, itemDefine->index, number, shortCount );

            inventory->idx             = LONG_MAX;
			inventory->itemDefineIdx   = itemDefine->idx;
			inventory->itemDefineIndex = itemDefine->index;
			inventory->number          = number;
			inventory->count           = shortCount;
			inventory->enhanced        = 0;
            inventory->seal            = (itemDefine->licenseType == 1) ? 1 : 0;
            inventory->cash            = 0;
            inventory->apply           = InventoryApplyNone;

			(*pcbValue) = shortCount;
			mItemChange = true;
			return inventory;
		}
	}

	return NULL;
}
TB_INVENTORY* cPlayer::CreateInventory(long itemDefineIndex, short number, short count, short* pcbValue)
{
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
	return ( itemDefine != NULL ) ? CreateInventory( itemDefine, number, count, pcbValue ) : NULL;
}

// UpdateInventory Method.
short cPlayer::UpdateInventory(TB_INVENTORY* inventory, short count)
{
	short retvalue = 0;
	if ( inventory != NULL )
	{
		if ( inventory->count > 0 )
		{
			PerItemCount* itemCount = NULL;
			if ( count > 0 )
			{
				TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
				if ( itemDefine != NULL )
				{
					if ( (inventory->count + count) > itemDefine->capacity )
						retvalue = itemDefine->capacity - inventory->count;
					else
						retvalue = count;

					itemCount =ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, inventory->itemDefineIndex, inventory->number, retvalue );
					inventory->count = inventory->count + retvalue;
				}
			}
			else
			{
				if ( inventory->count <= abs( count ) )
					retvalue = (-inventory->count);
				else
					retvalue = count;

				itemCount =ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, inventory->itemDefineIndex, inventory->number, retvalue );
				inventory->count = inventory->count + retvalue;
			}
			mItemChange = true;
		}
	}
	return retvalue;
}

// UpdateInventory Method.
short cPlayer::UpdateInventory(short number, short count)
{
	return UpdateInventory( SelectInventory( number ), count );
}

// RemoveInventory Method.
bool cPlayer::RemoveInventory(TB_INVENTORY* inventory)
{
	if ( inventory != NULL )
	{
		int inventoryBagCount = 0;

		if ( inventory->count > 0 )
		{
			PerItemCount* itemCount = ITEMCOUNTPOOL->UpdateItemCount( &mItemCountRoot, inventory->itemDefineIndex, inventory->number, (-inventory->count) );
			if ( itemCount != NULL )
			{
				inventoryBagCount = itemCount->bag;
			}
		}

		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
		if ( itemDefine != NULL )
		{
			if ( itemDefine->type == ITEM_MATERIAL && itemDefine->subType == ITEM_MATERIAL_VEHICLE )
			{
				TB_ITEM_ABILITY* ability = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
				if ( ability != NULL && !(inventoryBagCount > 0) )
				{
					EndVehicleItem( ability->influence_idx );
				}
			}
		}

		memset( inventory, 0, sizeof(TB_INVENTORY) );
		mItemChange = true;
		return true;
	}
	return false;
}

// RemoveInventory Method.
bool cPlayer::RemoveInventory(short number)
{
	return RemoveInventory( SelectInventory( number ) );
}

// ItemInsInventory Method.
void cPlayer::ItemInsInventory(ULONG_PTR socketContext, MSG_REQ_CHEAT_MAKE* recvMsg)
{
	if ( mInventoryDatabase )
		throw ERROR_CHEAT_MAKE_NOTYET;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( recvMsg->mItemIndex );

	short remainCount = recvMsg->mCount;
	short shortCount;
	short number;

	if ( itemDefine!= NULL && recvMsg->mCount > 0 )
	{
		// 치트 아이템. 생성 검사 및 등록.
		if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == false )
			throw ERROR_CHEAT_MAKE_FAIL;

		HANDLE            handle          = NULL;
		INVENTORY_INSERT* inventoryInsert = (INVENTORY_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_INSERT );
		TB_INVENTORY*     table           = inventoryInsert->table;
		long&             rowCount        = inventoryInsert->rowCount;
		long              length          = sizeof(INVENTORY_INSERT) - sizeof(inventoryInsert->table);

		inventoryInsert->characterIdx = mObject.index;

		// 치트 아이템. 생성 검사 및 등록.
		do {
			CreateInventory( itemDefine, number, remainCount, &shortCount );
			remainCount = remainCount - shortCount;

			table->idx = 0;
			table->itemDefineIdx   = itemDefine->idx;
			table->itemDefineIndex = itemDefine->index;
			table->number          = number;
			table->count           = (+shortCount);
			table->enhanced        = recvMsg->mEnhance;
			table->cardSlot1       = recvMsg->mCardSlot[0];
			table->cardSlot2       = recvMsg->mCardSlot[1];
			table->cardSlot3       = recvMsg->mCardSlot[2];
			table->cardSlot4       = recvMsg->mCardSlot[3];
			table->cardSlot5       = recvMsg->mCardSlot[4];
            table->seal            = ( itemDefine->licenseType == 1 ) ? 1 : 0;

			table++; rowCount++;
		} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );

		length += (sizeof(inventoryInsert->table) * inventoryInsert->rowCount);
		mInventoryDb.insert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
		if ( mInventoryDb.insert == false )
		{
			NETWORK2->ReleaseSQL( handle, length );
			throw ERROR_CHEAT_MAKE_FAIL;
		}
	}
	else
		throw ERROR_CHEAT_MAKE_FAIL;
}

// ItemInsInventory Method.
void cPlayer::ItemInsInventory(ULONG_PTR socketContext, MSG_REQ_CHEAT_MAKE_ITEMS* recvMsg)
{
	if ( mInventoryDatabase )
		throw ERROR_CHEAT_MAKE_NOTYET;
	
	if( recvMsg->mCount < 1 )
		throw ERROR_CHEAT_MAKE_FAIL;

	sCheat_Make_Item_Info* pItemInfo = recvMsg->mItemInfo;

	HANDLE            handle          = NULL;
	INVENTORY_INSERT* inventoryInsert = (INVENTORY_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_INSERT );
	TB_INVENTORY*     table           = inventoryInsert->table;
	long&             rowCount        = inventoryInsert->rowCount;
	long              length          = sizeof(INVENTORY_INSERT) - sizeof(inventoryInsert->table);

	for( unsigned int cnt = 0; cnt < recvMsg->mCount; ++cnt, pItemInfo++ )
	{

		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( pItemInfo->mItemIndex );

		short remainCount = pItemInfo->mCount;
		short shortCount;
		short number;
		
		if ( itemDefine!= NULL && pItemInfo->mCount > 0 )
		{
			// 치트 아이템. 생성 검사 및 등록.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == false )
			{
				// 만들 수 있는 갯수 만큼 생성하기.
				if( cnt != 0 )
				{
					mInventoryDb.insert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
					if ( mInventoryDb.insert == false )
					{
						NETWORK2->ReleaseSQL( handle, length );
						throw ERROR_CHEAT_MAKE_FAIL;
					}
					return;
				}

				NETWORK2->ReleaseSQL( handle, length );
				throw ERROR_CHEAT_MAKE_FAIL;
			}

			inventoryInsert->characterIdx = mObject.index;

			// 치트 아이템. 생성 검사 및 등록.
			do {
				CreateInventory( itemDefine, number, remainCount, &shortCount );
				remainCount = remainCount - shortCount;

				table->idx = 0;
				table->itemDefineIdx   = itemDefine->idx;
				table->itemDefineIndex = itemDefine->index;
				table->number          = number;
				table->count           = (+shortCount);
				table->enhanced        = pItemInfo->mEnhance;
				table->cardSlot1       = pItemInfo->mCardSlot[0];
				table->cardSlot2       = pItemInfo->mCardSlot[1];
				table->cardSlot3       = pItemInfo->mCardSlot[2];
				table->cardSlot4       = pItemInfo->mCardSlot[3];
				table->cardSlot5       = pItemInfo->mCardSlot[4];
                table->seal            = ( itemDefine->licenseType == 1 ) ? 1 : 0;

				table++; rowCount++;
			} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );

			length = (sizeof(inventoryInsert->table) * inventoryInsert->rowCount) + (sizeof(INVENTORY_INSERT) - sizeof(inventoryInsert->table));
		}
		else
		{
			// 만들 수 있는 갯수 만큼 생성하기.
			if( cnt != 0 )
			{
				mInventoryDb.insert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
				if ( mInventoryDb.insert == false )
				{
					NETWORK2->ReleaseSQL( handle, length );
					throw ERROR_CHEAT_MAKE_FAIL;
				}
				return;
			}

			NETWORK2->ReleaseSQL( handle, length );
			throw ERROR_CHEAT_MAKE_FAIL;
		}
	}

	mInventoryDb.insert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.insert == false )
	{
		NETWORK2->ReleaseSQL( handle, length );
		throw ERROR_CHEAT_MAKE_FAIL;
	}
}

// ItemMovInventory Method.
void cPlayer::ItemMovInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_MOV_INVENTORY* msg)
{
	if ( mInventoryDatabase )
		throw ERROR_ITEM_MOV_INVENTORY_NOTYET;

	if ( GetStateDie( ) )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	if ( msg->number1 == msg->number2 )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	TB_INVENTORY*   inventory1 = SelectInventory( msg->number1 );
	TB_INVENTORY*   inventory2 = SelectInventory( msg->number2 );
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );

    if ( IsInventoryCash( msg->number1 ) && IsInventoryRange( msg->number2 ) )
    {
        if ( !IsInventoryBag( msg->number2 ) || IsInventory( inventory2 ) )
            throw ERROR_ITEM_MOV_INVENTORY_FAIL;

        if ( IsMaxInventory( itemDefine, inventory1->count ) == true )
            throw ERROR_ITEM_MOV_INVENTORY_MAX_ITEM;
    }
    else if ( !IsInventoryRange( msg->number1 ) || !IsInventoryRange( msg->number2 ) )
        throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	if ( IsInventoryEquip( msg->number1 ) && IsInventoryEquip( msg->number2 ) )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	if ( IsInventoryQuest( msg->number1 ) != IsInventoryQuest( msg->number2 ) )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	if ( IsInventoryWarehouse( msg->number1 ) != IsInventoryWarehouse( msg->number2 ) )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

    /// 타로카드의 경우
	if ( IsInventoryCollect( msg->number2 ) )
    {
        if ( !IsInventoryBag( msg->number1 ) )
            throw ERROR_ITEM_MOV_INVENTORY_FAIL;

        if ( GetStateStop() == eSTOP_SELFTAROT || GetStateStop() == eSTOP_OPENTAROT )
            throw ERROR_ITEM_MOV_INVENTORY_STATE;
    }
	else if ( IsInventoryCollect( msg->number1 ) )
    {
        if ( !IsInventoryBag( msg->number2 ) )
		    throw ERROR_ITEM_MOV_INVENTORY_FAIL;

        if ( GetStateStop() == eSTOP_SELFTAROT || GetStateStop() == eSTOP_OPENTAROT )
            throw ERROR_ITEM_MOV_INVENTORY_STATE;
    }


	if ( inventory1->apply != InventoryApplyNone || inventory2->apply != InventoryApplyNone )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	/*-- 인벤토리 이동중, 병합 처리.
	*/
	if ( IsInventoryMerge( itemDefine, inventory1, inventory2 ) == true )
	{
        mInventoryDb.merge = NETWORK2->InventoryMerge( (PerSocketContext*)socketContext
                                                        ,inventory1->idx
                                                        ,inventory2->idx
									                    ,itemDefine->capacity );
        throw ERROR_ITEM_MOV_INVENTORY_MERGE;
	}


	/*-- 인벤토리 이동 처리.
	*/
	int   depth = 0;
	short except[3];
	short number[3];

	memset( except, 0, sizeof(except) );
	memset( number, 0, sizeof(number) );
	(*except) = msg->number2;

	if ( IsInventoryMove( msg->number1, msg->number2, except, depth ) == false )
		throw ERROR_ITEM_MOV_INVENTORY_FAIL;

	// 예외처리 검사
	if ( (*except) != msg->number2 )
	{
		short  empty   = INVENTORY_BAG_BEGIN;
		short* pExcept = except;
		short* pNumber = number;

		for ( int i = 0; i < 3 && (*pExcept) != 0; i++, pExcept++, pNumber++ )
		{
			empty = GetEmptyBagNumber( empty );

			if ( empty == INVENTORY_BAG_NONE )
				throw ERROR_ITEM_MOV_INVENTORY_FAIL;

			(*pNumber) = empty;
			empty++;
		}
	}


	/*-- 'depth' 가 0 일경우 move, 아닐경우 swap 처리 한다.
	     'swap' 과 'move' 는 처리 순서가 서로 다르므로 주의해야 한다.
	*/
	if ( depth != 0 )
	{
		// Except DB & SRV
		if ( (*except) != msg->number2 )
		{
			long   idx[3];
			short* pExcept = except;
			short* pNumber = number;

			memset( idx, 0, sizeof(idx) );
			for ( int i = 0; i < 3 && (*pExcept) != 0; i++, pExcept++, pNumber++ )
			{
				idx[ i ] = SelectInventory( (*pExcept) )->idx;
				InventoryExcept( (*pExcept), (*pNumber) );
			}

			mInventoryDb.except = NETWORK2->InventoryExcept( (PerSocketContext*)socketContext, idx, (short*)number );
		}
		// Swap DB & SRV
		mInventoryDb.swap = NETWORK2->InventorySwap( (PerSocketContext*)socketContext
													,inventory1->idx
													,inventory1->number
													,inventory2->idx
													,inventory2->number );
		if ( mInventoryDb.swap )
		{
			InventorySwap( msg->number1, msg->number2 );
		}
		throw ERROR_ITEM_MOV_INVENTORY_SUCCESS;
	}
	else
	{
		// Move DB & SRV
		mInventoryDb.move = NETWORK2->InventoryMove( (PerSocketContext*)socketContext
													,inventory1->idx
													,msg->number2
													,IsInventoryCash( msg->number1 ) );
		if ( mInventoryDb.move )
		{
			InventoryMove( msg->number1, msg->number2 );
		}
		// Except DB & SRV
		if ( (*except) != msg->number2 )
		{
			long   idx[3];
			short* pExcept = except;
			short* pNumber = number;

			memset( idx, 0, sizeof(idx) );
			for ( int i = 0; i < 3 && (*pExcept) != 0; i++, pExcept++, pNumber++ )
			{
				idx[ i ] = SelectInventory( (*pExcept) )->idx;
				InventoryExcept( (*pExcept), (*pNumber) );
			}

			mInventoryDb.except = NETWORK2->InventoryExcept( (PerSocketContext*)socketContext, idx, (short*)number );
		}
		throw ERROR_ITEM_MOV_INVENTORY_SUCCESS;
	}
}

// ItemUseInventory Method.
void cPlayer::ItemUseInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_USE_INVENTORY* msg)
{
	/*-- 아이템 사용시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_USE_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

    if ( IsInventoryBag( msg->number ) == false )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

	/*-- Item Define Table(TB_ITEM_DEFINE) 적용.
	*/
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	unsigned int    getState;

	if ( itemDefine == NULL )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

    if ( IsInventorySeal( itemDefine, inventory ) == true )
        throw ERROR_ITEM_USE_INVENTORY_LICENSE;

	if ( !( (itemDefine->type == ITEM_ETC1) ||	
		    (itemDefine->type == ITEM_CARD && itemDefine->subType == ITEM_CARD_COMMON ) ||
		    (itemDefine->type == ITEM_MONSTER) || 
            (itemDefine->type == ITEM_INVENTORY) 
       ) )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;
 
	/// 원본
	//if ( itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_QUEST )
	//{
	//	throw ERROR_ITEM_USE_INVENTORY_FAIL;
	//}
	if ( itemDefine->type == ITEM_ETC1 )
	{
		switch( itemDefine->subType )
		{
		case ITEM_ETC1_QUEST:
			{
				throw ERROR_ITEM_USE_INVENTORY_FAIL;
			}
			break;
		case ITEM_ETC1_EXP_RESTORE:
			{
				if( mHeroInfo.mLoseExp == 0 )
					throw ERROR_ITEM_USE_EXP_RECOVER;

				if( TIMER->DiffRealTime( mExpRecoverEndTime ) < 0 )
					throw ERROR_ITEM_USE_EXP_RECOVER;
			}
			break;
		}
	}
	else if ( itemDefine->type == ITEM_CARD && itemDefine->subType == ITEM_CARD_COMMON )
	{
		getState = GetState( );
		if ( !( getState == eOBJECT_STATE_IDLE || getState == ePLAYER_STATE_ITEMPICK ) )
			throw ERROR_ITEM_USE_INVENTORY_FAIL;
	}
	else if ( itemDefine->type == ITEM_MONSTER )
	{
		getState = GetState( );
		if ( !( getState == eOBJECT_STATE_IDLE || getState == ePLAYER_STATE_ITEMPICK ) )
			throw ERROR_ITEM_USE_INVENTORY_FAIL;
	}

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

	/*-- 사용할 효과 적용. --*/

	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	cTitleDefine*    titleDefine = TITLEMAN->GetTitleDefine( itemDefine->titleIndex );

	/*-- Item Ability 효과 적용가능 검사
	*/
	if ( itemAbility != NULL )
	{
		int apply = ERROR_ITEM_USE_INVENTORY_SUCCESS;
		if ( itemAbility->influence_idx > 0 )
		{
			// 우선 검사 조건.
			if ( itemAbility->target_type == ITEM_TARGET_MYSELF )
			{
				apply = SKILLMANAGER->ItemCheckAddInf( this, this, itemAbility->influence_idx );
			}
			else if ( itemAbility->target_type == ITEM_TARGET_MONSTER )
			{
				if ( msg->targetType == eOBJECTTYPE_MONSTER )
				{
					cMonster* monster = OBJECTMANAGER->GetMonster( msg->targetIndex );
					if ( monster != NULL )
					{
						NiPoint3 monsterPos( monster->GetXPos( ), monster->GetYPos( ), monster->Height( ) );
						NiPoint3 heroPos( mObjectPos.x, mObjectPos.y, Height() );
						if ( mMapNumber == monster->GetMapNumber( ) && (short)(heroPos - monsterPos).Length( ) < itemAbility->use_range )
						{
							apply = SKILLMANAGER->ItemCheckAddInf( this, monster, itemAbility->influence_idx );
						}
					}
				}
			}
			// 차선 검사 조건.
			else
			{
				if ( itemAbility->target_type & ITEM_TARGET_MYSELF )
				{
					if ( msg->targetType == eOBJECTTYPE_HERO || msg->targetIndex == mObject.index )
					{
						apply = SKILLMANAGER->ItemCheckAddInf( this, this, itemAbility->influence_idx );
					}
				}
				if ( itemAbility->target_type & ITEM_TARGET_PLAYER )
				{
					if ( msg->targetType == eOBJECTTYPE_PLAYER )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
						if ( player != NULL )
						{
							NiPoint3 playerPos( player->GetXPos( ), player->GetYPos( ), player->Height( ) );
							NiPoint3 heroPos( mObjectPos.x, mObjectPos.y, Height() );
							if ( mMapNumber == player->GetMapNumber( ) && (short)(heroPos - playerPos).Length( ) < itemAbility->use_range )
							{
								apply = SKILLMANAGER->ItemCheckAddInf( this, player, itemAbility->influence_idx );
							}
						}
					}
				}
				if ( itemAbility->target_type & ITEM_TARGET_PARTY )
				{
					// 파티 검사
					cParty* party = PARTYMAN->GetParty( mPlayerExrInfo.mPartyIndex );
					if ( party != NULL )
					{
						if ( party->IsUser( msg->targetIndex ) == true )
						{
							cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
							if ( player != NULL )
							{
								NiPoint3 playerPos( player->GetXPos( ), player->GetYPos( ), player->Height( ) );
								NiPoint3 heroPos( mObjectPos.x, mObjectPos.y, Height() );
								if ( mMapNumber == player->GetMapNumber( ) && (short)(heroPos - playerPos).Length( ) < itemAbility->use_range )
								{
									apply = SKILLMANAGER->ItemCheckAddInf( this, player, itemAbility->influence_idx );
								}
							}
						}
					}
					else
					{
						// 파티 연합 검사
						cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( mPlayerExrInfo.mPartyUnionIndex );
						if( partyUnion != NULL )
						{
							if ( partyUnion->IsUser( GetObjectID(), msg->targetIndex ) == true )
							{
								cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
								if ( player != NULL )
								{
									NiPoint3 playerPos( player->GetXPos( ), player->GetYPos( ), player->Height( ) );
									NiPoint3 heroPos( mObjectPos.x, mObjectPos.y, Height() );
									if ( mMapNumber == player->GetMapNumber( ) && (short)(heroPos - playerPos).Length( ) < itemAbility->use_range )
									{
										apply = SKILLMANAGER->ItemCheckAddInf( this, player, itemAbility->influence_idx );
									}
								}
							}
						}
					}
				}
			}
		}

		// 스킬의 효과 적용여부 확인.
		if ( apply != ERROR_ITEM_USE_INVENTORY_SUCCESS )
		{
			throw apply;
		}
	}
	/* Title Table 효과 적용가능 검사
	*/
	else if ( titleDefine != NULL )
	{
		cHaveTitleSet::cIterator title = mHaveTitleSet.Find( itemDefine->titleIndex );
		if ( title != mHaveTitleSet.End( ) )
			throw ERROR_ITEM_USE_INVENTORY_TITLE;
	}
    /* 효과 없는 아이템 기능 적용
    */
	else if ( itemDefine->type == ITEM_INVENTORY )
    {
        if( (itemDefine->subType == ITEM_INVENTORY_BAG       && mHeroInfo.BagEnd        == INVENTORY_BAG_END        ) ||
            (itemDefine->subType == ITEM_INVENTORY_CARD      && mHeroInfo.WearEnd       == INVENTORY_WEAR_END       ) ||
            (itemDefine->subType == ITEM_INVENTORY_WAREHOUSE && mHeroInfo.WareHouseEnd  == INVENTORY_WAREHOUSE_END  ) ||
            (itemDefine->subType == ITEM_INVENTORY_MAKESKILL && mHeroInfo.MakeSkillEnd  == MAX_MAKESKILL ) )
            throw ERROR_ITEM_USE_INVENTORY_SLOT;

        HANDLE                handle    = NULL;
        CHARACTER_INVEN_SIZE* invenSize = (CHARACTER_INVEN_SIZE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_INVEN_SIZE );
        invenSize->idx                  = mObject.index;
        invenSize->bag                  = mHeroInfo.BagEnd;
        invenSize->wear                 = mHeroInfo.WearEnd;
        invenSize->wareHouse            = mHeroInfo.WareHouseEnd;
        invenSize->makeSkill            = mHeroInfo.MakeSkillEnd;

        if( SetInvenLimit( itemDefine, invenSize ) == false )
        {
            NETWORK2->PostServerEvent( "ERROR INVENTORY_USE - (itemDefine->type == ITEM_INVENTORY), ItemIndex(=%d)", inventory->itemDefineIndex );
            throw ERROR_ITEM_USE_INVENTORY_FAIL;
        }

        NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(CHARACTER_INVEN_SIZE) );
    }
	else if( itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_EXP_RESTORE )
	{
	}
	else 
		throw ERROR_ITEM_USE_INVENTORY_FAIL;

	/*-- Item Ability Table - 쿨타임 검사 및 적용.
	*/
	if ( COOLTIMEPOOL->IsCooltime( &mCooltimeRoot, inventory->itemDefineIndex ) == true )
		throw ERROR_ITEM_USE_INVENTORY_COOLTIME;
	else
		COOLTIMEPOOL->ApplyCooltime( &mCooltimeRoot, inventory );

	/*-- Item Ability Table - 효과 적용.
	*/
	if ( itemAbility != NULL )
	{
		bool apply = false;
		if( itemAbility->influence_idx == 0 )
		{
			apply = true;
		}
		else if ( itemAbility->influence_idx > 0 )
		{
			// 우선 검사 조건.
			if ( itemAbility->target_type == ITEM_TARGET_MYSELF )
			{
				apply = SKILLMANAGER->AddInfluence( mObject, mObject, itemAbility->influence_idx, 0, true );
			}
			else if ( itemAbility->target_type == ITEM_TARGET_MONSTER )
			{
				cMonster* monster = OBJECTMANAGER->GetMonster( msg->targetIndex );
				if ( monster != NULL )
					apply = SKILLMANAGER->AddInfluence( mObject, monster->GetObject( ), itemAbility->influence_idx, 0, true );
			}
			// 차선 검사 조건.
			else
			{
				if ( msg->targetType == eOBJECTTYPE_HERO || msg->targetIndex == mObject.index )
				{
					apply = SKILLMANAGER->AddInfluence( mObject, mObject, itemAbility->influence_idx, 0, true );
				}
				if ( itemAbility->target_type & ITEM_TARGET_PLAYER )
				{
					if ( msg->targetType == eOBJECTTYPE_PLAYER )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
						if ( player != NULL )
							apply = SKILLMANAGER->AddInfluence( mObject, player->GetObject( ), itemAbility->influence_idx, 0, true );
					}
				}
				if ( itemAbility->target_type & ITEM_TARGET_PARTY )
				{
					// 파티 검사
					cParty* party = PARTYMAN->GetParty( GetPartyIndex( ) );
					if ( party != NULL )
					{
						if ( party->IsUser( msg->targetIndex ) == true )
						{
							cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
							if ( player != NULL )
								apply = SKILLMANAGER->AddInfluence( mObject, player->GetObject( ), itemAbility->influence_idx, 0, true );
						}
					}
					else
					{
						// 파티 연합 검사
						cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( GetPartyUnionIndex() );
						if( partyUnion != NULL )
						{
							if ( partyUnion->IsUser( GetObjectID(), msg->targetIndex ) == true )
							{
								cPlayer* player = OBJECTMANAGER->GetPlayer( msg->targetIndex );
								if ( player != NULL )
									apply = SKILLMANAGER->AddInfluence( mObject, player->GetObject( ), itemAbility->influence_idx, 0, true );
							}
						}
					}
				}
			}
		}
		if ( apply == false )
		{
			NETWORK2->PostServerEvent( "ERROR INVENTORY_USE, ItemIndex(=%d)", inventory->itemDefineIndex );
			throw ERROR_ITEM_USE_INVENTORY_FAIL;
		}
	}
	else if ( titleDefine != NULL )
	{
		HANDLE        handle      = NULL;
		TITLE_INSERT* titleInsert = (TITLE_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_TITLE_INSERT );

		titleInsert->characterIdx = mObject.index;
		titleInsert->titleIdx     = itemDefine->titleIndex;

		NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(TITLE_INSERT) );
	}

	if ( itemDefine->type == ITEM_ETC1 )
	{
		switch( itemDefine->subType )
		{
			/// 경험치 복구 수치 적용
		case ITEM_ETC1_EXP_RESTORE:
			{
				AddExp( mHeroInfo.mLoseExp, false, true );
				mHeroInfo.mLoseExp = 0;		

				if( DBUpdate( false, PLAYER_DBUPDATE_EXPRECOVER ) == false )
					mBatchSaveTime = 0;
			}
			break;
		case ITEM_ETC1_AUTODETECT:
			{
				// 랜덤 몬스터 소환
				if( AIMANAGER->AutoDetectSummonMonRegen( mMapNumber, mObjectPos.x, mObjectPos.y ) == false )
					throw ERROR_ITEM_USE_INVENTORY_FAIL;
			}
			break;
		}
	}

	// 아이템 사용 선처리.
	UpdateInventory( inventory, (-1) );

	// DATABASE - 인벤토리내 아이템 사용 요청.
	HANDLE         handle       = NULL;
	INVENTORY_USE* inventoryUse = (INVENTORY_USE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_USE );
	TB_INVENTORY*  table        = inventoryUse->table;

	(*table) = (*inventory);

	mInventoryDb.use = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(INVENTORY_USE) );
	if ( mInventoryDb.use == false ) throw ERROR_ITEM_USE_INVENTORY_FAIL;
}

// ItemDelInventory Method.
void cPlayer::ItemDelInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_DEL_INVENTORY* msg)
{
	/*-- 아이템 삭제시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_DEL_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_DEL_INVENTORY_FAIL;

    if ( IsInventoryRange(msg->number) == false )
		throw ERROR_ITEM_DEL_INVENTORY_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_DEL_INVENTORY_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_DEL_INVENTORY_FAIL;

	// DATABASE - 인벤토리내 아이템 삭제 요청.
	HANDLE            handle          = NULL;
	INVENTORY_REMOVE* inventoryRemove = (INVENTORY_REMOVE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_REMOVE );

	inventoryRemove->idx    = inventory->idx;
	inventoryRemove->number = inventory->number;

	mInventoryDb.remove = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(INVENTORY_REMOVE) );
	if ( mInventoryDb.remove == false ) throw ERROR_ITEM_DEL_INVENTORY_FAIL;
}

// ItemSwiInventory Method.
void cPlayer::ItemSwiInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_SWI_INVENTORY* /*msg*/)
{
	/*-- 아이템 삭제시 검사할 내용들.
	*/
	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_SWI_INVENTORY_FAIL;

	if ( GetState( ) == eOBJECT_STATE_ATTACK )
		throw ERROR_ITEM_SWI_INVENTORY_FAIL;

	int activeWeapon  = mItemActiveWeapon ^ ItemActiveSwitch;
	mItemActiveWeapon = (eItemActiveWeapon)activeWeapon;

	HANDLE                      handle  = NULL;
	MSG_RES_ITEM_SWI_INVENTORY* sendMsg = (MSG_RES_ITEM_SWI_INVENTORY*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_SWI_INVENTORY_RES );
		sendMsg->ErrorCode    = ERROR_ITEM_SWI_INVENTORY_SUCCESS;
		sendMsg->activeWeapon = mItemActiveWeapon;
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_SWI_INVENTORY) );
}

// ItemDivideInventory Method.
void cPlayer::ItemDivideInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_DIVIDE_INVENTORY* msg)
{
	/*-- 아이템 분활시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_DIVIDE_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( msg->number1 == msg->number2 )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( !IsInventoryRange( msg->number1 ) || !IsInventoryRange( msg->number2 ) )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( IsInventoryEquip( msg->number1 ) || IsInventoryEquip( msg->number2 ) )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( IsInventoryQuest( msg->number1 ) || IsInventoryQuest( msg->number2 ) )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( IsInventoryWarehouse( msg->number1 ) != IsInventoryWarehouse( msg->number2 ) )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	TB_INVENTORY* inventory1 = SelectInventory( msg->number1 );
	TB_INVENTORY* inventory2 = SelectInventory( msg->number2 );

	if ( IsInventory( inventory1 ) == false || IsInventory( inventory2 ) == true )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( inventory1->cash > 0 )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if ( inventory1->apply != InventoryApplyNone )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	if  ( (msg->divide < 1) || (msg->divide >= inventory1->count) )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
	if ( itemDefine == NULL )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;

    if ( itemDefine->licenseType == 1 )
		throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;


	// DATABASE - 인벤토리내 아이템 분활 요청.
	HANDLE            handle          = NULL;
	INVENTORY_DIVIDE* inventoryDivide = (INVENTORY_DIVIDE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_DIVIDE );

	// 인벤토리 분활.
	UpdateInventory( inventory1, (-msg->divide) );

	// 인벤토리 생성.
	short shortCount=0;
	CreateInventory( itemDefine, msg->number2, msg->divide, &shortCount );

	// DB 값 입력.
	inventoryDivide->characterIdx = mObject.index;
	inventoryDivide->table[ 0 ] = (*inventory1);	// 인벤토리 정보 복사.
	inventoryDivide->table[ 1 ] = (*inventory2);	// 인벤토리 정보 복사.
	inventoryDivide->table[ 1 ].idx = 0;			// 임시 생성번호 삭제.

	mInventoryDb.divide = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(INVENTORY_DIVIDE) );
	if ( mInventoryDb.divide == false ) throw ERROR_ITEM_DIVIDE_INVENTORY_FAIL;
}

// ItemMov2Inventory Method.
void cPlayer::ItemMov2Inventory(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_MOV2_INVENTORY* msg)
{
	/*-- 아이템 이동시(창고 & 인벤) 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_MOV2_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	if ( !IsInventoryRange( msg->number1 ) || !IsInventoryRange( msg->number2 ) )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	if ( IsInventoryEquip( msg->number1 ) || IsInventoryEquip( msg->number2 ) )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	if ( !IsInventoryWarehouse( msg->number1 ) && !IsInventoryWarehouse( msg->number2 ) )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	TB_INVENTORY* inventory1 = SelectInventory( msg->number1 );
	TB_INVENTORY* inventory2 = SelectInventory( msg->number2 );

	if ( inventory1->apply != InventoryApplyNone || inventory2->apply != InventoryApplyNone )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	if ( msg->count > inventory1->count )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	if ( msg->count == 0 )
		msg->count = inventory1->count;

    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
    if ( itemDefine == NULL )
        throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

    /*-- 인벤토리 이동중, 병합 처리.
    */
    if ( IsInventoryMerge( itemDefine, inventory1, inventory2 ) == true )
    {
        mInventoryDb.merge = NETWORK2->InventoryMerge( (PerSocketContext*)socketContext
                                                        ,inventory1->idx
                                                        ,inventory2->idx
									                    ,itemDefine->capacity );
        throw ERROR_ITEM_MOV2_INVENTORY_MERGE;
    }

	/*-- 인벤토리 이동 처리.
	*/
	int   depth = 0;
	short except[3];

	memset( except, 0, sizeof(except) );
	(*except) = msg->number2;

	if ( IsInventoryMove( msg->number1, msg->number2, except, depth ) == false )
		throw ERROR_ITEM_MOV2_INVENTORY_FAIL;

	/*-- 'depth' 가 0 일경우 move, 아닐경우 swap 처리 한다.
	     'swap' 과 'move' 는 처리 순서가 서로 다르므로 주의해야 한다.
	*/
	if ( depth != 0 )
	{
		// Swap DB & SRV
		mInventoryDb.swap = NETWORK2->InventorySwap( (PerSocketContext*)socketContext
													,inventory1->idx
													,inventory1->number
													,inventory2->idx
													,inventory2->number );
		if ( mInventoryDb.swap )
		{
			InventorySwap( msg->number1, msg->number2 );
		}
		throw ERROR_ITEM_MOV2_INVENTORY_SUCCESS;
	}
    else
    {
        // DATABASE - 인벤토리내 아이템 이동(창고 & 인벤) 요청.
        HANDLE           handle         = NULL;
        INVENTORY_MOVE2* inventoryMove2 = (INVENTORY_MOVE2*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_MOVE2 );
        unsigned long    length         = sizeof(INVENTORY_MOVE2) - sizeof(inventoryMove2->table);

        inventoryMove2->characterIdx = mObject.index;

        if ( msg->count == inventory1->count )
        {
            InventoryMove( msg->number1, msg->number2 );

            inventoryMove2->rowCount = 1;
            inventoryMove2->table[ 0 ] = (*inventory2);
        }
        else
        {
            // 인벤토리 분활.
            UpdateInventory( inventory1, (-msg->count) );

            // 인벤토리 생성.
            short shortCount=0;
            CreateInventory( itemDefine, msg->number2, msg->count, &shortCount );

            // DB 값 입력.
            inventoryMove2->rowCount = 2;
            inventoryMove2->table[ 0 ] = (*inventory1);	// 인벤토리 정보 복사.
            inventoryMove2->table[ 1 ] = (*inventory2);	// 인벤토리 정보 복사.
            inventoryMove2->table[ 1 ].idx = 0;			// 임시 생성번호 삭제.
        }

        // Move DB & SRV
        length += (inventoryMove2->rowCount * sizeof(inventoryMove2->table));
        mInventoryDb.move2 = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
        if ( mInventoryDb.move2 == false ) throw ERROR_ITEM_MOV2_INVENTORY_FAIL;
	}
}

// ItemDisjointInventory Method.
void cPlayer::ItemDisjointInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_DISJOINT_INVENTORY* msg)
{
	/*-- 아이템 분해시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_DISJOINT_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

    if ( IsInventoryBag( msg->number ) == false )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );
	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

    if( (int)mStatus2.mMaxWeight <= mItemWeight )
        throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

    TB_ITEM_DEFINE* disItemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if ( IsInventorySeal( disItemDefine, inventory ) == true )
        throw ERROR_ITEM_DISJOINT_INVENTORY_LICENSE;

    TB_ITEM_DISJOINT* itemDisjoint = ITEMMANAGER->GetItemDisjoint( inventory->itemDefineIdx );
	if ( itemDisjoint == NULL )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

	// 비용확인.
	if ( IsAddMoney( -(itemDisjoint->price) ) == false )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

	// 보상아이템 목록.
	eDisjointItem disjointItems[ MAX_DISJOINT_ITEM ];
	unsigned int  disjointItemsCount = 0;

	if ( itemDisjoint->itemDefineIndex1 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex1;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount1;
		disjointItemsCount++;
	}
	if ( itemDisjoint->itemDefineIndex2 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex2;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount2;
		disjointItemsCount++;
	}
	if ( itemDisjoint->itemDefineIndex3 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex3;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount3;
		disjointItemsCount++;
	}
	if ( itemDisjoint->itemDefineIndex4 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex4;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount4;
		disjointItemsCount++;
	}
	if ( itemDisjoint->itemDefineIndex5 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex5;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount5;
		disjointItemsCount++;
	}
	if ( itemDisjoint->itemDefineIndex6 > 0 )
	{
		disjointItems[ disjointItemsCount ].itemDefineIndex = itemDisjoint->itemDefineIndex6;
		disjointItems[ disjointItemsCount ].inventoryCount  = itemDisjoint->inventoryCount6;
		disjointItemsCount++;
	}

    long itemDefineIndex = 0;
    long inventoryCount  = 0;
    for( unsigned int i = 0; i < disjointItemsCount; i++ )
    {
        itemDefineIndex = disjointItems[i].itemDefineIndex;
        inventoryCount  = (long)disjointItems[i].inventoryCount;

        TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
        if( IsMaxInventory( itemDefine, inventoryCount ) == true ) 
            throw ERROR_ITEM_DISJOINT_INVENTORY_MAX_ITEM;
    }

	// 보상아이템 검사.
	if ( IsItemReward( inventory->idx, disjointItems, disjointItemsCount ) < disjointItemsCount )
		throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;

	HANDLE              handle            = NULL;
	INVENTORY_DISJOINT* inventoryDisjoint = (INVENTORY_DISJOINT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_DISJOINT );
	unsigned long       length            = sizeof(INVENTORY_DISJOINT) - sizeof(inventoryDisjoint->table);
	TB_INVENTORY*       table             = inventoryDisjoint->table;

    inventoryDisjoint->rowCount = 0;
    long&              rowCount = inventoryDisjoint->rowCount;

	/*-- 캐릭터 고유번호
	*/
	inventoryDisjoint->characterIdx = mObject.index;

	/*-- 분해 비용
	*/
	inventoryDisjoint->characterMoney = -(itemDisjoint->price);

	/*-- 분해 아이템
	*/
	(*table) = (*inventory);
	table->count = (-inventory->count);
	table++; rowCount++;

	// 아이템 선 삭제 - 보상 아이템 공간 확보
	RemoveInventory( inventory );

	/*-- 보상 아이템
	*/
	eDisjointItem* items = disjointItems;

	for ( unsigned int i0 = 0; i0 < disjointItemsCount; i0++, items++ )
	{
		TB_ITEM_DEFINE* itemDefine  = ITEMMANAGER->GetItemDefineByIndex( items->itemDefineIndex );
		u_short         remainCount = (u_short)items->inventoryCount;

		if ( itemDefine != NULL && remainCount > 0 )
		{
			TB_INVENTORY* inventory;
			short         shortCount;
			short         number;

			// 아이템 분해 보상. 겹치기 검사 및 처리.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
			{
				do {
					inventory   = SelectInventory( number );
					shortCount  = UpdateInventory( inventory, remainCount );
					remainCount = remainCount - shortCount;

					// INVENTORY_IDX, ITEM_DEFINE_INDEX, INVENTORY_NUMBER, INVENTORY_COUNT...
					(*table) = (*inventory);
					table->count = shortCount;
					table++; rowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
			}
			// 아이템 분해 보상. 생성 검사 및 등록.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
			{
				do {
					CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
					remainCount = remainCount - shortCount;

					// INVENTORY_IDX, ITEM_DEFINE_INDEX, INVENTORY_NUMBER, INVENTORY_COUNT...
					table->idx             = 0;
					table->itemDefineIdx   = itemDefine->idx;
					table->itemDefineIndex = itemDefine->index;
					table->number          = number;
					table->count           = shortCount;
					table++; rowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
			}
		}
	}

	length += (inventoryDisjoint->rowCount * sizeof(inventoryDisjoint->table));
	mInventoryDb.disjoint = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.disjoint == false ) throw ERROR_ITEM_DISJOINT_INVENTORY_FAIL;
}

// ItemPutCardInventory Method.
void cPlayer::ItemPutCardInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_PUT_CARD_INVENTORY* msg)
{
	/*-- 아이템 카드장착시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

    if ( !IsInventoryBag( msg->number1 ) || !IsInventoryBag( msg->number2 ) )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	TB_INVENTORY* inventory1 = SelectInventory( msg->number1 );
	TB_INVENTORY* inventory2 = SelectInventory( msg->number2 );

	if ( IsInventory( inventory1 ) == false || IsInventory( inventory2 ) == false )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	if ( inventory1->apply != InventoryApplyNone || inventory2->apply != InventoryApplyNone )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	if ( msg->cardSlot < MIN_CARD_SLOT || msg->cardSlot > MAX_CARD_SLOT )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	// 장착슬롯 검사.
    TB_ITEM_DEFINE* itemDefine1 = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
    if ( IsInventorySeal( itemDefine1, inventory1 ) == true )
        throw ERROR_ITEM_PUT_CARD_INVENTORY_LICENSE;

	TB_ITEM_CARD_SLOT* itemCardSlot = ITEMMANAGER->GetItemCardSlot( inventory1->itemDefineIdx );
	if ( itemCardSlot == NULL )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	// 장착카드 검사.
	TB_ITEM_DEFINE* itemDefine2 = ITEMMANAGER->GetItemDefine( inventory2->itemDefineIdx );
	if ( itemDefine2 == NULL )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

	if ( itemDefine2->type != ITEM_CARD )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;

    if ( IsInventorySeal( itemDefine2, inventory2 ) == true )
        throw ERROR_ITEM_PUT_CARD_INVENTORY_LICENSE;

    if ( !IsItemLimit( inventory2->itemDefineIdx ) )
        throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;


	switch ( msg->cardSlot )
	{
	case ITEM_CARD_SLOT_1:
		if ( !(ItemSlotType2Mtr( itemCardSlot->slot1 ) & ItemCardType2Mtr( itemDefine2->subType )) )
			throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
		break;
	case ITEM_CARD_SLOT_2:
		if ( !(ItemSlotType2Mtr( itemCardSlot->slot2 ) & ItemCardType2Mtr( itemDefine2->subType )) )
			throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
		break;
	case ITEM_CARD_SLOT_3:
		if ( !(ItemSlotType2Mtr( itemCardSlot->slot3 ) & ItemCardType2Mtr( itemDefine2->subType )) )
			throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
		break;
	case ITEM_CARD_SLOT_4:
		if ( !(ItemSlotType2Mtr( itemCardSlot->slot4 ) & ItemCardType2Mtr( itemDefine2->subType )) )
			throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
		break;
	case ITEM_CARD_SLOT_5:
		if ( !(ItemSlotType2Mtr( itemCardSlot->slot5 ) & ItemCardType2Mtr( itemDefine2->subType )) )
			throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
		break;
	default:
		throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
	}

	HANDLE              handle           = NULL;
	INVENTORY_PUT_CARD* inventoryPutCard = (INVENTORY_PUT_CARD*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_PUT_CARD );

	inventoryPutCard->idx1            = inventory1->idx;
	inventoryPutCard->cardSlot        = msg->cardSlot;
	inventoryPutCard->idx2            = inventory2->idx;
	inventoryPutCard->itemDefineIndex = inventory2->itemDefineIndex;

	mInventoryDb.putCard = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(INVENTORY_PUT_CARD) );
	if ( mInventoryDb.putCard == false ) throw ERROR_ITEM_PUT_CARD_INVENTORY_FAIL;
}

// ItemChangeInventory Method.
void cPlayer::ItemChangeInventory(ULONG_PTR socketContext, MSG_REQ_ITEM_CHANGE_INVENTORY* msg)
{
	/*-- 아이템 체인지 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_PUT_CARD_INVENTORY_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

    if ( IsInventoryBag( msg->number ) == false )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );
	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

    TB_ITEM_DEFINE* itemBox = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if ( itemBox == NULL )
        throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

    if ( IsInventorySeal( itemBox, inventory ) == true )
        throw ERROR_ITEM_CHANGE_INVENTORY_LICENSE;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

    if( (int)mStatus2.mMaxWeight <= mItemWeight )
        throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

	TB_ITEM_CHANGE* itemChange = ITEMMANAGER->GetItemChange( inventory->itemDefineIdx );
	if ( itemChange == NULL )
		throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;

	long money       = itemChange->maxMoney - itemChange->minMoney;
	long randomValue = (long)(RANDOMTABLE->Get( mItemChangeSeed ) * 1000000);

	if ( money > 0 )
		money = itemChange->minMoney + (rand( ) % money) + 1;
	else
		money = itemChange->minMoney;

    // 최대 보유금액.
    if ( IsAddMoney( money ) == false )
        throw ERROR_ITEM_CHANGE_INVENTORY_HAVE_MONEY;

	// 체인지 확률 적용.
	long*  itemChangeIndex = itemChange->itemDefineIndex;
	short* itemChangeCount = itemChange->itemCount;
	long*  changeRate      = itemChange->changeRate;
	int    itemChangeItems = 0;

	int    itemChnageData[ MAX_ITEM_CHANGE_DATA ];
	int*   itemChangePtr = itemChnageData;

	for ( int i = 0; i < itemChange->row; i++ )
	{
		if ( (*itemChangeIndex) != 0 && (*itemChangeCount) != 0 && randomValue <= (*changeRate) )
		{
			(*itemChangePtr) = i;
			itemChangePtr++;
			itemChangeItems++;
		}

		itemChangeIndex++;
		itemChangeCount++;
		changeRate++;
	}

	// 램덤하게 뒤썩기.
	for ( int i = 0, s, t; i < itemChangeItems; i++ )
	{
		s = (int)(rand( ) % itemChangeItems);

		t                   = itemChnageData[ i ];
		itemChnageData[ i ] = itemChnageData[ s ];
		itemChnageData[ s ] = t;
	}
	// 최대수량 보정.
	if ( itemChangeItems > itemChange->rate )
	{
		itemChangeItems = itemChange->rate;
	}

	// 인벤토리 최대 수량 검사.
    long itemDefineIndex = 0;
    long inventoryCount  = 0;
    for( long i = 0; i < itemChangeItems; i++ )
    {
		itemDefineIndex =       *(itemChange->itemDefineIndex + itemChnageData[ i ]);
		inventoryCount  = (long)*(itemChange->itemCount       + itemChnageData[ i ]);

        TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
        if( IsMaxInventory( itemDefine, inventoryCount ) == true )
            throw ERROR_ITEM_CHANGE_INVENTORY_MAX_ITEM;
    }


	HANDLE            handle          = NULL;
	INVENTORY_CHANGE* inventoryChange = (INVENTORY_CHANGE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_CHANGE );
	TB_INVENTORY*     table           = inventoryChange->table;
	unsigned long     length          = sizeof(INVENTORY_CHANGE) - sizeof(inventoryChange->table);

    inventoryChange->rowCount         = 0;
    long&            rowCount         = inventoryChange->rowCount;

	/*-- 캐릭터 고유번호 --*/
	inventoryChange->characterIdx = mObject.index;

	/*-- 획득 비용 -- */
	inventoryChange->characterMoney = money;

	/*-- 체인지 아이템 --*/
	(*table) = (*inventory);

	table->count     = -1;
	inventory->count = inventory->count -1;

	table++; rowCount++;

	if ( inventory->count <= 0 )
	{
		RemoveInventory( inventory );
	}

	/*-- 보상 아이템
	*/
	for ( long i0 = 0; i0 < itemChangeItems; i0++ )
	{
		long  itemDefineIndex = *(itemChange->itemDefineIndex + itemChnageData[ i0 ]);
		short remainCount     = *(itemChange->itemCount       + itemChnageData[ i0 ]);

		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );

		if ( itemDefine != NULL && remainCount > 0 )
		{
			TB_INVENTORY* inventory;
			short         shortCount;
			short         number;

			// 아이템 변환 보상. 겹치기 검사 및 처리.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
			{
				do {
					inventory   = SelectInventory( number );
					shortCount  = UpdateInventory( inventory, remainCount );
					remainCount = remainCount - shortCount;

					// INVENTORY_IDX, ITEM_DEFINE_INDEX, INVENTORY_NUMBER, INVENTORY_COUNT...
					(*table) = (*inventory);
					table->count = shortCount;

					table++; rowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
			}
			// 아이템 변환 보상. 생성 검사 및 등록.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
			{
				do {
					inventory   = CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
					remainCount = remainCount - shortCount;

					// INVENTORY_IDX, ITEM_DEFINE_INDEX, INVENTORY_NUMBER, INVENTORY_COUNT...
                    (*table)     = (*inventory);
					table->idx   = 0;
                    table->count = shortCount;

					table++; rowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
			}
		}
	}

	length += (sizeof(inventoryChange->table) * rowCount);
	mInventoryDb.change = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.change == false ) throw ERROR_ITEM_CHANGE_INVENTORY_FAIL;
}

// ItemCashInventory Method.
void cPlayer::ItemCashInventory( ULONG_PTR socketContext )
{
	if( mInventoryDatabase )
		throw ERROR_ITEM_CASH_LIST_FAIL;

	HANDLE            handle          = NULL;
	INVENTORY_SELECT* inventorySelect = (INVENTORY_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_CASH_SELECT );
	unsigned long     length          = sizeof(INVENTORY_SELECT) - sizeof(inventorySelect->table);

	inventorySelect->characterIdx   = mObject.index;

	mInventoryDb.cash = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if( mInventoryDb.cash == false ) throw ERROR_ITEM_CASH_LIST_FAIL;
}

// ItemSell Method.
void cPlayer::ItemSell(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_SELL* msg)
{
	/*-- 아이템 판매시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_SELL_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_SELL_FAIL;

    if ( IsInventoryBag( msg->inventoryNumber ) == false )
		throw ERROR_ITEM_SELL_FAIL;

	// 인벤토리 내용 확인.
	TB_INVENTORY* inventory = SelectInventory( msg->inventoryNumber );

	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_SELL_FAIL;

	if ( msg->count < 1 || msg->count > inventory->count )
		throw ERROR_ITEM_SELL_FAIL;

	// 아이템 정의 확인.
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	long            sellType   = ItemSellNone;
	long            sellPrice  = 0;

	if ( itemDefine == NULL )
		throw ERROR_ITEM_SELL_FAIL;

    // 미인증 아이템은 상점거래가 무조건 불가능
    if( itemDefine->licenseType == 1 )
        throw ERROR_ITEM_SELL_EXCHANGE;
    
	if ( !(itemDefine->sellPrice > 0) )
		throw ERROR_ITEM_SELL_FAIL;

	switch ( itemDefine->sellType )
	{
	case ItemSellNone:
		throw ERROR_ITEM_SELL_FAIL;
	case ItemSellMoney:
	case ItemSellPvp:
	case ItemSellTarot:
		sellPrice = itemDefine->sellPrice * msg->count;
        if( IsAddMoney( sellPrice ) == false )
            throw ERROR_ITEM_SELL_HAVE_MONEY;
		break;
	default:
		sellType = itemDefine->sellType;
		break;
	}

	// 아이템 - 아이템 습득검사(Ex: 코인).
	if ( sellType != ItemSellNone )
	{
		if ( CheckInventory( sellType, (itemDefine->sellPrice * msg->count) ) == false )
			throw ERROR_ITEM_SELL_FAIL;	// 판매가능 수량 오류일경우.
	}

	// DATABASE - 인벤토리내 아이템 판매 요청.
	HANDLE        handle   = NULL;
	ITEM_SELL*    itemSell = (ITEM_SELL*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SELL );
	long&         rowCount = itemSell->rowCount;
	TB_INVENTORY* table    = itemSell->table;
	long          length   = sizeof(ITEM_SELL) - sizeof(itemSell->table);

	itemSell->characterIdx = mObject.index;
	itemSell->npcIdx       = msg->npcIdx;
	itemSell->sellType     = itemDefine->sellType;
	itemSell->sellPrice    = (+sellPrice);
	rowCount = 1;

	(*table) = (*inventory);
	table->count = (-msg->count);
	table++;

	if ( inventory->count == msg->count )
		RemoveInventory( inventory );

	// 판매보상 아이템 - 습득처리.
	if ( sellType != ItemSellNone )
	{
		TB_ITEM_DEFINE* tempDfn     = ITEMMANAGER->GetItemDefine( sellType );
		short           remainCount = (short)(itemDefine->sellPrice * msg->count);
		short           shortCount;
		short           number;

		if ( tempDfn != NULL )
		{
			// 아이템 판매보상 아이템1. 겹치기 검사 및 처리.
			if ( FindFirstInventory( tempDfn, &number, remainCount, &shortCount, true ) == true )
			{
				TB_INVENTORY* tempInv;
				do {
					tempInv     = SelectInventory( number );
					shortCount  = UpdateInventory( tempInv, remainCount );
					remainCount = remainCount - shortCount;

					(*table) = (*tempInv);
					table->count = (+shortCount);

					table++; rowCount++;
				} while ( FindNextInventory( tempDfn, &(++number), remainCount, &shortCount, true ) == true );
			}
			// 아이템 판매보상 아이템2. 생성 검사 및 등록.
			if ( FindFirstInventory( tempDfn, &number, remainCount, &shortCount, false ) == true )
			{
				do {
					CreateInventory( tempDfn, number, remainCount, &(shortCount=0) );
					remainCount = remainCount - shortCount;

					table->idx             = 0;
					table->itemDefineIdx   = tempDfn->idx;
					table->itemDefineIndex = tempDfn->index;
					table->count           = (+shortCount);
					table->number          = number;
					table->seal            = tempDfn->licenseType;

					table++; rowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
			}
		}
	}

	length += (rowCount * sizeof(itemSell->table));
	mInventoryDb.sell = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.sell == false ) throw ERROR_ITEM_SELL_FAIL;
}

// ItemBuy Method.
void cPlayer::ItemBuy( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_BUY* msg)
{
	/*-- 아이템 구입시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_BUY_NOTYET;

	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
        throw ERROR_ITEM_BUY_FAIL;

    // 희망하는 구매수량 검사.
    if ( !(msg->count > 0) )
        throw ERROR_ITEM_BUY_FAIL;

    cNpc* pNpc = OBJECTMANAGER->GetNpc( msg->npcIdx );
    if( !pNpc )
        throw ERROR_QUEST_ADD_FAIL;

    unsigned int npcId = pNpc->GetRaceGender();

    /// npc에 해당하는 item 확인
    cItemNpcStore* pItemStore = ITEMSCRIPT->GetItemNpcStore( npcId );
    if( pItemStore == NULL )
        throw ERROR_ITEM_BUY_NPC;

    if( pItemStore->IsItemIdx( msg->tabIdx, msg->itemDefineIndex ) == false )
        throw ERROR_ITEM_BUY_NPC;

    /// npc 상점 tab 정보
    sNpcItemData* itemStore     = NULL;
    unsigned long priceRate     = 0;            /// 판매 금액/세력포인트 비율
    unsigned char storeForce    = 0;            /// 0:테르, 1/2/3/4:세력

    itemStore   = pItemStore->GetNpcItemData( msg->tabIdx );
    if( itemStore == NULL )
        throw ERROR_ITEM_BUY_NPC;

    priceRate   = itemStore->mRate;
    storeForce  = itemStore->mForce;


    // 아이템 정의 테이블 검사.
    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( msg->itemDefineIndex );
    long            buyType    = ItemBuyNone;
    long            buyPrice   = 0;
	bool            payItem    = false;

	PerItemCount*   itemCount  = NULL;

	if ( itemDefine == NULL )
		throw ERROR_ITEM_BUY_FAIL;

	if ( !(itemDefine->buyPrice > 0) )
		throw ERROR_ITEM_BUY_MONEY;

    if( IsMaxInventory( itemDefine, msg->count ) == true )
        throw ERROR_ITEM_BUY_MAX_ITEM;
    
    buyPrice = (long)( itemDefine->buyPrice * ( priceRate * 0.01 ) ) * msg->count;

	switch ( itemDefine->buyType )
	{
	case ItemBuyNone:
		throw ERROR_ITEM_BUY_FAIL;
	case ItemBuyMoney:
		if ( (ULONG)buyPrice > mMoney )
			throw ERROR_ITEM_BUY_MONEY;
		break;
	case ItemBuyPvp:
		if ( (ULONG)buyPrice > GetPvPPoint( storeForce ) )
			throw ERROR_ITEM_BUY_PVP_POINT;
		break;
	case ItemBuyTarot:
		if ( (ULONG)buyPrice > mMoney )
			throw ERROR_ITEM_BUY_TAROT_POINT;
		break;
	default:
		buyType   = itemDefine->buyType;
		itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, buyType );
		if ( itemCount != NULL )
		{
			if ( itemCount->bag < buyPrice )
				throw ERROR_ITEM_BUY_FAIL;
		}
		else
			throw ERROR_ITEM_BUY_FAIL;
		payItem = true;
		break;
	}

	// 구입가능 검사.
	if ( CheckInventory( msg->itemDefineIndex, msg->count ) == false )
		throw ERROR_ITEM_BUY_FAIL;

	// DATABASE - 아이템 구매 요청.
	HANDLE        handle      = NULL;
	ITEM_BUY*     itemBuy     = (ITEM_BUY*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_BUY );
	TB_INVENTORY* table       = itemBuy->table;
	u_long        length      = sizeof(ITEM_BUY) - sizeof(itemBuy->table);
	BYTE&         rowCount    = (BYTE&)itemBuy->rowCount;
	u_short       remainCount = msg->count;

	itemBuy->characterIdx = mObject.index;
	itemBuy->npcIdx       = msg->npcIdx;
	itemBuy->buyType      = itemDefine->buyType;
	itemBuy->buyPrice     = (-buyPrice);
	itemBuy->buyStore     = storeForce;

	TB_INVENTORY* inventory;
	short         number;
	short         shortCount;

	// 아이템 구매. 겹치기 검사 및 처리.
	if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
	{
		do {
			inventory   = SelectInventory( number );
			shortCount  = UpdateInventory( inventory, remainCount );
			remainCount = remainCount - shortCount;

			(*table) = (*inventory);
			table->count = (+shortCount);

			table++; rowCount++;
		} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
	}
	// 아이템 구매. 생성 검사 및 등록.
	if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
	{
		do {
			CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
			remainCount = remainCount - shortCount;

			table->idx             = 0;
			table->itemDefineIdx   = itemDefine->idx;
			table->itemDefineIndex = itemDefine->index;
			table->count           = (+shortCount);
			table->number          = number;
			table->seal            = itemDefine->licenseType;

			table++; rowCount++;
		} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
	}

	if ( payItem == true )
	{
		TB_INVENTORY* tempInv;
		short         remainCount;

		tempInv     = SelectInventory( INVENTORY_BAG_BEGIN );
		remainCount = (short)(itemDefine->buyPrice * msg->count);

		// 아이템 차감(Ex: 코인).
		for ( short i = INVENTORY_BAG_BEGIN; (i <= mHeroInfo.BagEnd) && (remainCount > 0); i++, tempInv++ )
		{
			if ( buyType == tempInv->itemDefineIndex )
			{
				short shortCount;

				shortCount  = UpdateInventory( tempInv, (-remainCount) );
				remainCount = remainCount + shortCount;

				(*table) = (*tempInv);
				table->count = shortCount;

				table++; rowCount++;
			}
		}
	}

	if ( itemBuy->rowCount > 0 )
	{
		length += (itemBuy->rowCount * sizeof(itemBuy->table));
		mInventoryDb.buy = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
		if ( mInventoryDb.buy == false ) throw ERROR_ITEM_BUY_FAIL;
	}
	else
	{
		NETWORK2->ReleaseSQL( handle, length );
		throw ERROR_ITEM_BUY_FAIL;
	}
}

// ItemCollect Method
void cPlayer::ItemCollect(ULONG_PTR socketContext, MSG_REQ_ITEM_COLLECT* msg)
{
	/*-- 아이템 수집시 검사할 내용들.
	*/
	if ( mInventoryDatabase )
		throw ERROR_ITEM_COLLECT_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_COLLECT_FAIL;

    if ( GetStateStop() == eSTOP_SELFTAROT || GetStateStop() == eSTOP_OPENTAROT )
        throw ERROR_ITEM_COLLECT_STATE;

    if ( IsInventoryBag( msg->number1 ) == false )
		throw ERROR_ITEM_COLLECT_FAIL;

    if ( IsInventoryCollect( msg->number2 ) == false )
		throw ERROR_ITEM_COLLECT_FAIL;

	// 인벤토리 내용 확인.
	TB_INVENTORY* inventory1 = SelectInventory( msg->number1 );
    TB_INVENTORY* inventory2 = SelectInventory( msg->number2 );
	if ( IsInventory( inventory1 ) == false )
		throw ERROR_ITEM_COLLECT_FAIL;

    // 아이템 테이블 확인.
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
	if ( itemDefine == NULL )
		throw ERROR_ITEM_COLLECT_FAIL;

    if ( IsInventorySeal( itemDefine, inventory1 ) == true )
        throw ERROR_ITEM_COLLECT_LICENSE;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory1->itemDefineIdx ) )
		throw ERROR_ITEM_COLLECT_FAIL;

    if ( itemDefine->type == ITEM_SPREAD )
	{
        switch ( itemDefine->subType )
        {
        case ITEM_SPREAD_NORMAL:
            if ( msg->number2 < INVENTORY_SPREAD_NORMAL_BEGIN || msg->number2 > INVENTORY_SPREAD_NORMAL_END )
                throw ERROR_ITEM_COLLECT_FAIL;
            break;
        case ITEM_SPREAD_SILVER:
            if ( msg->number2 < INVENTORY_SPREAD_SILVER_BEGIN || msg->number2 > INVENTORY_SPREAD_SILVER_END )
                throw ERROR_ITEM_COLLECT_FAIL;
            break;
        case ITEM_SPREAD_GOLD:
            if ( msg->number2 < INVENTORY_SPREAD_GOLD_BEGIN || msg->number2 > INVENTORY_SPREAD_GOLD_END )
                throw ERROR_ITEM_COLLECT_FAIL;
            break;
        case ITEM_SPREAD_DIA:
            if ( msg->number2 < INVENTORY_SPREAD_DIA_BEGIN || msg->number2 > INVENTORY_SPREAD_DIA_END )
                throw ERROR_ITEM_COLLECT_FAIL;
            break;
        default : 
            throw ERROR_ITEM_COLLECT_FAIL;
            break;
        }
	}
	else
		throw ERROR_ITEM_COLLECT_FAIL;

	HANDLE        handle      = NULL;
	ITEM_COLLECT* itemCollect = (ITEM_COLLECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_COLLECT );
	itemCollect->characterIdx = mObject.index;

    // itemCollect->table : [0:기존카드/1:등록카드/2:삭제카드]
    if ( IsInventory( inventory2 ) == true )
    {
        // 기존 등록된 카드가 있으면 삭제
		itemCollect->table[ 2 ] = (*inventory2);
        RemoveInventory( inventory2 );
    }

    if ( inventory1->count == 1 )
    {
		itemCollect->table[ 0 ] = (*inventory1);
		itemCollect->table[ 0 ].count = 0;

		itemCollect->table[ 1 ] = (*inventory1);
		itemCollect->table[ 1 ].number = msg->number2;

        InventoryMove( msg->number1, msg->number2 );
    }
    else
    {
        short         shortCount = 0;
        TB_INVENTORY* inventory3 = CreateInventory( itemDefine, msg->number2, 1, &shortCount );

        inventory3->idx = 0;
        UpdateInventory( inventory1, -1 );

		itemCollect->table[ 0 ] = (*inventory1);
		itemCollect->table[ 1 ] = (*inventory3);
    }

	mInventoryDb.collect = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_COLLECT) );
	if ( mInventoryDb.collect == false )
		throw ERROR_ITEM_COLLECT_FAIL;
}

// ItemCollectChange Method
void cPlayer::ItemCollectChange(ULONG_PTR socketContext, MSG_REQ_COLLECT_CARD_CHANGE* msg)
{
	if ( mInventoryDatabase )
		throw ERROR_ITEM_COLLECT_CARD_CHANGE_NOTYET;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL;

    if ( GetStateStop() == eSTOP_SELFTAROT || GetStateStop() == eSTOP_OPENTAROT )
        throw ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL;

    if( (int)mStatus2.mMaxWeight <= mItemWeight )
        throw ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL;


    short   invenBegin      = INVENTORY_TAROT_NORMAL_BEGIN;      // 인벤토리 대역
    short   invenEnd        = INVENTORY_TAROT_NORMAL_END;        // 인벤토리 대역
    long    changeItem      = -1;                                // 체인지 아이템

    // 등급에 따른 인벤토리 대역
    switch ( msg->tarotClass )
    {
        case eTAROT_CLASS_NORMAL:
            invenBegin  = INVENTORY_TAROT_NORMAL_BEGIN;
            invenEnd    = INVENTORY_TAROT_NORMAL_END;
            changeItem  = TAROT_CHANGE_ITEM_SILVER;
            break;
        case eTAROT_CLASS_SILVER:
            invenBegin  = INVENTORY_TAROT_SILVER_BEGIN;
            invenEnd    = INVENTORY_TAROT_SILVER_END;
            changeItem  = TAROT_CHANGE_ITEM_GOLD;
            break;
        case eTAROT_CLASS_GOLD:
            invenBegin  = INVENTORY_TAROT_GOLD_BEGIN;
            invenEnd    = INVENTORY_TAROT_GOLD_END;
            changeItem  = TAROT_CHANGE_ITEM_DIA;
            break;
        default : 
            throw ERROR_ITEM_COLLECT_CARD_CHANGE_CLASS; 
            break;
    }

    // 타로카드 22장 체크
    int tarotCount = 0;
    for ( short tarotNumber = invenBegin; tarotNumber <= invenEnd; tarotNumber++ )
	{
        TB_INVENTORY* inventory = SelectInventory( tarotNumber );
		if ( IsInventory( inventory ) )
        {
            tarotCount++;
        }
        inventory++;
	}

    if ( tarotCount != MAX_TAROT_CARDS )
        throw ERROR_ITEM_COLLECT_CARD_CHANGE_COUNT; 


    // 체인지 아이템 생성
	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( changeItem );
	if ( itemDefine == NULL )
		throw ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL;

    if ( IsMaxInventory( itemDefine, 1 ) == true )
        throw ERROR_ITEM_COLLECT_CARD_CHANGE_MAX_ITEM;


    HANDLE               handle      = NULL;
    ITEM_COLLECT_CHANGE* itemCollect = (ITEM_COLLECT_CHANGE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE );
    TB_INVENTORY*        table       = itemCollect->table;
    u_long               length      = sizeof(ITEM_COLLECT_CHANGE) - sizeof(itemCollect->table);

    itemCollect->rowCount   = 0;
    long&        rowCount   = itemCollect->rowCount;
    

    // 체인지 아이템 생성
    TB_INVENTORY*   inventory   = NULL;
    long            itemCount   = 0;
    short           remainCount = 1;
    short           shortCount  = 0;
    short           itemNumber  = 0;

    // 아이템 줍기. 겹치기 검사 및 처리.
    if ( FindFirstInventory( itemDefine, &itemNumber, remainCount, &shortCount, true ) == true )
    {
        do {
            inventory   = SelectInventory( itemNumber );
            shortCount  = UpdateInventory( inventory, remainCount );
            remainCount = remainCount - shortCount;

            itemCollect->changeItem = (*inventory);
            itemCount++;
        } while ( FindNextInventory( itemDefine, &(++itemNumber), remainCount, &shortCount, true ) == true );
    }
    // 아이템 줍기. 생성 검사 및 등록.
    if ( FindFirstInventory( itemDefine, &itemNumber, remainCount, &shortCount, false ) == true )
    {
        do {
            CreateInventory( itemDefine, itemNumber, remainCount, &(shortCount=0) );
            remainCount = remainCount - shortCount;

            itemCollect->changeItem.idx             = 0;
            itemCollect->changeItem.itemDefineIdx   = itemDefine->idx;
            itemCollect->changeItem.itemDefineIndex = itemDefine->index;
            itemCollect->changeItem.number          = itemNumber;
            itemCollect->changeItem.count           = shortCount;
            itemCollect->changeItem.seal            = itemDefine->licenseType;

            itemCount++;
        } while ( FindNextInventory( itemDefine, &(++itemNumber), remainCount, &shortCount, false ) == true );
    }

    if ( itemCount != 1 )
        throw ERROR_ITEM_COLLECT_CARD_CHANGE_FULL;


    // 22장의 타로카드 사용
    for ( short tarotNumber = invenBegin; tarotNumber <= invenEnd; tarotNumber++ )
	{
        TB_INVENTORY* inventory = SelectInventory( tarotNumber );
		if ( IsInventory( inventory ) )
        {
            UpdateInventory( inventory, (-1) );
            (*table) = (*inventory);

            table++;
            rowCount++;
        }
	}

    itemCollect->characterIdx   = mObject.index;
    itemCollect->begin          = invenBegin;
    itemCollect->end            = invenEnd;

	length += (itemCollect->rowCount * sizeof(itemCollect->table));
    mInventoryDb.collect = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
    if ( mInventoryDb.collect == false ) throw ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL;

    throw ERROR_ITEM_COLLECT_CARD_CHANGE_SUCCESS;
}

// ItemObtEvent Method
void cPlayer::ItemObtEvent(ULONG_PTR socketContext, BYTE cbtUser, BYTE obtUser, BYTE fourGamer)
{
	if ( mInventoryDatabase )
		throw ERROR_NPC_OBT_EVENT_FAIL;

	if ( GetStateDie( ) == true )
		throw ERROR_NPC_OBT_EVENT_FAIL;
	
	// 이미 모두 지급 받음
	if ( cbtUser == 2 && obtUser == 2 && fourGamer == 2 )
		throw ERROR_NPC_OBT_EVENT_COMPLETE;

	// 이벤트 대상이 아님
	if ( cbtUser != 1 && obtUser != 1 && fourGamer != 1 )
		throw ERROR_NPC_OBT_EVENT_NO_USER;

	HANDLE			handle		= NULL;
	ITEM_OBT_EVENT* itemEvent	= (ITEM_OBT_EVENT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_OBT_EVENT );
	long			length		= sizeof(ITEM_OBT_EVENT) - sizeof(itemEvent->table);

	itemEvent->characterIdx		= mObject.index;
	itemEvent->cbtUser			= cbtUser;
	itemEvent->obtUser			= obtUser;
	itemEvent->fourGamer		= fourGamer;

	mInventoryDb.cash = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.cash == false )
		throw ERROR_NPC_OBT_EVENT_FAIL;
}

// IsItemEnhanced Method.
int cPlayer::IsItemEnhanced( unsigned short equip, unsigned short card, unsigned short safeCard, int* rateType )
{
	/*-- 아이템 강화시 검사할 내용들.
	*/
	// 강화 장비 검사.
	TB_ITEM_DEFINE* itemDefine1 = NULL;
	TB_INVENTORY*   inventory1  = NULL;
	
	inventory1 = SelectInventory( equip );
	if ( IsInventory( inventory1 ) == false )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

	if ( inventory1->apply != InventoryApplyNone )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;
	
	itemDefine1 = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
	if ( itemDefine1 == NULL )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;
	
	if ( !(itemDefine1->type != ITEM_WEAPON || itemDefine1->type != ITEM_WEAR) )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;
	
    if ( IsInventorySeal( itemDefine1, inventory1 ) == true )
        return ERROR_ITEM_ENHANCED_CHECK_LICENSE;

	if ( !(inventory1->enhanced < MAX_ITEM_ENHANCED) )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;


    // 장비 itemLimit
    TB_ITEM_LIMIT* equipLimit = ITEMMANAGER->GetItemLimit( inventory1->itemDefineIdx );
    if ( equipLimit == NULL )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

	// 강화 카드 검사.
	TB_ITEM_DEFINE* itemDefine2 = NULL;
	TB_INVENTORY*   inventory2  = NULL;

	inventory2 = SelectInventory( card );
	if ( IsInventory( inventory2 ) == false )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

	if ( inventory2->apply != InventoryApplyNone )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

	itemDefine2 = ITEMMANAGER->GetItemDefine( inventory2->itemDefineIdx );
	if ( itemDefine2 == NULL )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

	if ( itemDefine2->type != ITEM_CARD )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;

    // 장비 레벨 체크
    switch ( itemDefine2->subType )
    {
    case ITEM_CARD_ENHANCE_WEAPON_LV1:
    case ITEM_CARD_ENHANCE_WEAPON_LV2:
    case ITEM_CARD_ENHANCE_WEAR_LV1:
    case ITEM_CARD_ENHANCE_WEAR_LV2:
        {
            if ( !(equipLimit->charLevelMin > 0 && equipLimit->charLevelMin <= 50) )
                return ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_CARD;
        }
        break;
    case ITEM_CARD_ENHANCE_WEAPON_LV1_75:
    case ITEM_CARD_ENHANCE_WEAPON_LV2_75:
    case ITEM_CARD_ENHANCE_WEAR_LV1_75:
    case ITEM_CARD_ENHANCE_WEAR_LV2_75:
        {
            if ( !(equipLimit->charLevelMin > 50 && equipLimit->charLevelMin <= 75) )
                return ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_CARD;
        }
        break;
    default:
        return ERROR_ITEM_ENHANCED_CHECK_FAIL;
    }

    if ( IsInventorySeal( itemDefine2, inventory2 ) == true )
        return ERROR_ITEM_ENHANCED_CHECK_LICENSE;

	if ( !IsItemLimit( inventory2->itemDefineIdx ) )
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;


	// 강화 보호 카드 검사.
    if ( safeCard > 0 )
    {
        TB_ITEM_DEFINE* itemDefine3 = NULL;
        TB_INVENTORY*   inventory3  = NULL;

        inventory3 = SelectInventory( safeCard );
        if ( IsInventory( inventory3 ) == false )
            return ERROR_ITEM_ENHANCED_CHECK_FAIL;

        if ( inventory3->apply != InventoryApplyNone )
            return ERROR_ITEM_ENHANCED_CHECK_FAIL;

        itemDefine3 = ITEMMANAGER->GetItemDefine( inventory3->itemDefineIdx );
        if ( itemDefine3 == NULL )
            return ERROR_ITEM_ENHANCED_CHECK_FAIL;

        /// 장비 레벨 체크
        if ( itemDefine3->type == ITEM_MATERIAL )
        {
            if ( itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DOWN_30    || 
                 itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DESTROY_30 )
            {
                if ( !(equipLimit->charLevelMin > 0 && equipLimit->charLevelMin <= 30) )
                    return ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_SAFE;
            }
            else if ( itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DOWN_60    || 
                      itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DESTROY_60 )
            {
                if ( !(equipLimit->charLevelMin > 30 && equipLimit->charLevelMin <= 60) )
                    return ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_SAFE;
            }
            else 
                return ERROR_ITEM_ENHANCED_CHECK_FAIL;
        }
        else
        {
            return ERROR_ITEM_ENHANCED_CHECK_FAIL;
        }

        if ( IsInventorySeal( itemDefine3, inventory3 ) == true )
            return ERROR_ITEM_ENHANCED_CHECK_LICENSE;

        if ( !IsItemLimit( inventory3->itemDefineIdx ) )
            return ERROR_ITEM_ENHANCED_CHECK_FAIL;
    }


	// 강화장비 & 강화카드 조합검사.
    int rate = 0;
	switch ( itemDefine1->type )
	{
	case ITEM_WEAPON:
		switch ( itemDefine2->subType )
		{
		case ITEM_CARD_ENHANCE_WEAPON_LV1:
        case ITEM_CARD_ENHANCE_WEAPON_LV1_75:
            rate = 0;
			break;
		case ITEM_CARD_ENHANCE_WEAPON_LV2:
        case ITEM_CARD_ENHANCE_WEAPON_LV2_75:
            rate = 1;
			break;
		default:
			return ERROR_ITEM_ENHANCED_CHECK_FAIL;
		}
		break;
	case ITEM_WEAR:
		switch ( itemDefine2->subType )
		{
		case ITEM_CARD_ENHANCE_WEAR_LV1:
        case ITEM_CARD_ENHANCE_WEAR_LV1_75:
            rate = 0;
			break;
		case ITEM_CARD_ENHANCE_WEAR_LV2:
        case ITEM_CARD_ENHANCE_WEAR_LV2_75:
            rate = 1;
			break;
		default:
			return ERROR_ITEM_ENHANCED_CHECK_FAIL;
		}
		break;
	default:
		return ERROR_ITEM_ENHANCED_CHECK_FAIL;
	}

    if ( rateType )
        *rateType = rate;

    return 0;
}

// ItemEnhancedStart Method.
void cPlayer::ItemEnhancedStart( MSG_REQ_ITEM_ENHANCED_START* msg )
{
	/// db 사용부 체크
	if ( mInventoryDatabase )
		throw ERROR_ITEM_ENHANCED_START_FAIL;

	if ( IsInventoryBag( msg->number1 ) == false )
		throw ERROR_ITEM_ENHANCED_START_FAIL;

	if ( IsInventoryBag( msg->number2 ) == false )
		throw ERROR_ITEM_ENHANCED_START_FAIL;

    if ( msg->number3 > 0 )
    {
	    if ( IsInventoryBag( msg->number3 ) == false )
		    throw ERROR_ITEM_ENHANCED_START_FAIL;
    }

    /// 아이템 강화 검사
    int retval = IsItemEnhanced( msg->number1, msg->number2, msg->number3, 0 );
    if ( retval != ERROR_ITEM_ENHANCED_CHECK_SUCCESS )
        throw retval;

    /// 강화 불가능 체크
    if( mPlayerExrInfo.mIsDuelIdx )
        throw ERROR_ITEM_ENHANCED_START_STATE;

	/// 상태 변경
	if( ChangeState( eOBJECT_STATE_STOP ) == false )
		throw ERROR_ITEM_ENHANCED_START_FAIL;

	SetStateStop( eSTOP_ENHANCED );

	// 완료 시간 저장
	mItemEnhanceEndTime = NETWORK2->GetAccumTime() + ENHANCE_DELAYTIME;
	mEnhanceSlot1       = msg->number1;
	mEnhanceSlot2       = msg->number2;
    mEnhanceSlot3       = msg->number3;

	/// 성공시 모두에게 알림
	MSG_SYN_ITEM_ENHANCED_START sync;
	sync.Category = NM_ITEM;
	sync.Protocol = NM_ITEM_ENHANCED_START_SYN;
	sync.characterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&sync, sizeof( MSG_SYN_ITEM_ENHANCED_START ) );
	
	throw ERROR_ITEM_ENHANCED_START_SUCCESS;
}

void cPlayer::ItemEnhancedEnd()
{
	try
	{
		if( mEnhanceSlot1 == USHRT_MAX || mEnhanceSlot2 == USHRT_MAX || mEnhanceSlot3 == USHRT_MAX )
		{
			NETWORK2->PostServerEvent( "ERROR ItemEnhancedEnd - CHARACTER_IDX(=%d) RETVALUE(=%d,%d,%d,%d)", 
                mObject.index, GetStateStop(), mEnhanceSlot1, mEnhanceSlot2, mEnhanceSlot3 );

			throw ERROR_ITEM_ENHANCED_END_FAIL;
		}

        if ( GetStateDie( ) == true )
            throw ERROR_ITEM_ENHANCED_END_FAIL;

        /// 아이템 강화 검사
        int rateType = 0;
        int retval = IsItemEnhanced( mEnhanceSlot1, mEnhanceSlot2, mEnhanceSlot3, &rateType );
        if ( retval != ERROR_ITEM_ENHANCED_CHECK_SUCCESS )
            throw retval;

        TB_INVENTORY* inventory1 = NULL;
        TB_INVENTORY* inventory2 = NULL;
        TB_INVENTORY* inventory3 = NULL;

		inventory1 = SelectInventory( mEnhanceSlot1 );
		inventory2 = SelectInventory( mEnhanceSlot2 );
		inventory3 = SelectInventory( mEnhanceSlot3 );


		/*-- 강화확률 적용 방법.
		0                                  1 = 0 ~ 1 사이의 확률 (최대 1/1000000)
		| SUCCESS | DOWN | DESTRUCT | FAIL | = 성공 < 다운 < 파괴 < 변화없음 순으로 적용.
		*/
		TB_ITEM_ENHANCED_RATE* itemEnhancedRate = ITEMMANAGER->GetItemEnhancedRate( inventory1->itemDefineIdx );
		if ( itemEnhancedRate == NULL )
			throw ERROR_ITEM_ENHANCED_END_FAIL;

		long success  = *(itemEnhancedRate->value[ rateType ].success  + inventory1->enhanced);
		long down     = *(itemEnhancedRate->value[ rateType ].down     + inventory1->enhanced) + success;
		long destruct = *(itemEnhancedRate->value[ rateType ].destruct + inventory1->enhanced) + down;

		// 아이템 강화확률 - 고유확률 적용.
		long randomValue = (long)(RANDOMTABLE->Get( mItemEnhancedSeed ) * 1000000);

		// DATABASE - 인벤토리내 아이템 이동(창고 & 인벤) 요청.
		HANDLE              handle   = NULL;
		INVENTORY_ENHANCED* enhanced = (INVENTORY_ENHANCED*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_ENHANCED );

        char result = RESULT_INVENTORY_ENHANCED_SUCCESS;
		if ( randomValue <= success )
		{
			result = RESULT_INVENTORY_ENHANCED_SUCCESS;
		}
		else if ( randomValue <= down && inventory1->enhanced > 0 )
		{
			result = RESULT_INVENTORY_ENHANCED_DOWN;
		}
		else if ( randomValue <= destruct )
		{
			result = RESULT_INVENTORY_ENHANCED_DESTROY;
		}
		else
		{
			result = RESULT_INVENTORY_ENHANCED_KEEP;
		}

        if ( result != RESULT_INVENTORY_ENHANCED_SUCCESS )
        {
            if ( IsInventory( inventory3 ) == true )
            {
                TB_ITEM_DEFINE* itemDefine3 = ITEMMANAGER->GetItemDefine( inventory3->itemDefineIdx );
                if ( itemDefine3 != NULL )
                {
                    if ( itemDefine3->type == ITEM_MATERIAL )
                    {
                        if ( result == RESULT_INVENTORY_ENHANCED_DOWN ) 
                        {
                            if ( itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DOWN_30 ||
                                 itemDefine3->subType == ITEM_MATERIAL_ENHANCED_DOWN_60 )
                            {
                                result = RESULT_INVENTORY_ENHANCED_KEEP;
                            }
                        }
                        else
                            result = RESULT_INVENTORY_ENHANCED_KEEP;
                    }
                }
            }
        }

        enhanced->result = result;
        enhanced->table[ 0 ] = (*inventory1);
		enhanced->table[ 1 ] = (*inventory2);
        if ( mEnhanceSlot3 > 0 )
        {
		    enhanced->table[ 2 ] = (*inventory3);
        }

		mInventoryDb.enhanced = NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(INVENTORY_ENHANCED) );
		if ( mInventoryDb.enhanced == false )
			throw ERROR_ITEM_ENHANCED_END_FAIL;

		mItemEnhanceEndTime = 0;
		mEnhanceSlot1       = USHRT_MAX;
		mEnhanceSlot2       = USHRT_MAX;
        mEnhanceSlot3       = USHRT_MAX;
	}
	catch ( int error )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );

		NETWORK2->SendMsgError( mConnectionIdx, NM_ITEM, NM_ITEM_ENHANCED_END_SYN, error );

		// 다른 유저들에게 결과전송
		MSG_SYN_ITEM_ENHANCED_OTHEREND sync;
		sync.Category  = NM_ITEM;
		sync.Protocol  = NM_ITEM_ENHANCED_OTHEREND_SYN;
		sync.ErrorCode = ERROR_ITEM_ENHANCED_SYN_ENDFAIL;
		sync.characterIdx = mObject.index;
		NETWORK2->QuickSendExcept( this, (char*)&sync, sizeof(MSG_SYN_ITEM_ENHANCED_OTHEREND) );
	}
}

// ItemEnhancedCancel Method.
void cPlayer::ItemEnhancedCancel( )
{
	if( GetStateStop() == eSTOP_ENHANCED )
	{
		/// 상태 풀어줌
		if ( GetState() == eOBJECT_STATE_STOP && GetStateStop() == eSTOP_ENHANCED )
		{
			ChangeState( eOBJECT_STATE_IDLE );
			SetStateStop( eSTOP_NONE );
		}

		mItemEnhanceEndTime = 0;
		mEnhanceSlot1       = USHRT_MAX;
		mEnhanceSlot2       = USHRT_MAX;
		mEnhanceSlot3       = USHRT_MAX;

		/// hero외의 플레이어에게 취소 확인 메세지 발송
		MSG_SYN_ITEM_ENHANCED_CANCEL sync;
		sync.Category = NM_ITEM;
		sync.Protocol = NM_ITEM_ENHANCED_CANCEL_SYN;
		sync.characterIdx = mObject.index;
		NETWORK2->QuickSendExcept( this, (char*)&sync, sizeof( MSG_SYN_ITEM_ENHANCED_CANCEL ) );

		/// db 처리중인 내용이 있으면 클라이언트에 알림
		int retcode = ( mInventoryDb.enhanced == true ) ? ERROR_ITEM_ENHANCED_CANCEL_DBWORK : ERROR_ITEM_ENHANCED_CANCEL_SUCCESS;
		NETWORK2->SendMsgError( mConnectionIdx, NM_ITEM, NM_ITEM_ENHANCED_CANCEL_RES, retcode );
	}
	else
	{
		NETWORK2->PostServerEvent( "ERROR ItemEnhancedCancel - CHARACTER_IDX(=%d) RETVALUE(=%d)", mObject.index, GetStateStop()	);
	}
}

// 강제 취소( 상태이상등 )
void cPlayer::ItemEnhancedForceCancel()
{
	/// 상태 풀어줌
	if ( GetState() == eOBJECT_STATE_STOP && GetStateStop() == eSTOP_ENHANCED )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );
	}

	mItemEnhanceEndTime = 0;
	mEnhanceSlot1       = USHRT_MAX;
	mEnhanceSlot2       = USHRT_MAX;
	mEnhanceSlot3       = USHRT_MAX;

	/// hero 에게 발송
	MSG_SYN_ITEM_ENHANCED_CANCELFORCE msg;
	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_ENHANCED_CANCELFORCE_SYN;
	NETWORK2->QuickSend( this, (char*)&msg, sizeof( MSG_SYN_ITEM_ENHANCED_CANCELFORCE ) );

	/// hero외의 플레이어에게 취소 확인 메세지 발송
	MSG_SYN_ITEM_ENHANCED_CANCEL sync;
	sync.Category = NM_ITEM;
	sync.Protocol = NM_ITEM_ENHANCED_CANCEL_SYN;
	sync.characterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&sync, sizeof( MSG_SYN_ITEM_ENHANCED_CANCEL ) );
}

// ItemLicense Method.
void cPlayer::ItemLicense( ULONG_PTR socketContext, MSG_REQ_ITEM_LICENSE* msg )
{
    if( !IsInventoryRange( msg->number ) )
        throw ERROR_ITEM_LICENSE_FAIL;

    if( !IsInventoryBag( msg->number ) )
        throw ERROR_ITEM_LICENSE_FAIL;

    TB_INVENTORY* inventory = SelectInventory( msg->number );
    if( inventory == NULL )
        throw ERROR_ITEM_LICENSE_FAIL;

	if ( inventory->seal == 0 )
		throw ERROR_ITEM_LICENSE_ALREADY;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_LICENSE_FAIL;

    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if( itemDefine == NULL )
        throw ERROR_ITEM_LICENSE_FAIL;

    // 미인증 아이템
    if ( itemDefine->licenseType != 1 )
		throw ERROR_ITEM_LICENSE_TYPE;

	// 인증설정
	inventory->seal = 0;

	HANDLE          handle        = NULL;
	INVENTORY_SEAL* inventorySeal = (INVENTORY_SEAL*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_SEAL );

	inventorySeal->characterIdx = mObject.index;
	inventorySeal->inventory = (*inventory);

	if ( itemDefine->licenseClass == 1 || itemDefine->licenseClass == 2 )
	{
		inventorySeal->itemBill.type         = itemDefine->licenseClass;
		inventorySeal->itemBill.validDate    = itemDefine->licenseValidThru;
		inventorySeal->itemBill.validTime    = itemDefine->licenseValidThru;
		inventorySeal->itemBill.inventoryIdx = inventory->idx;
	}

	mInventoryDb.seal = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(INVENTORY_SEAL) );
}

// ItemSummonPost Method.
void cPlayer::ItemSummonPost( ULONG_PTR socketContext, MSG_REQ_TRADE_POST_SUMMON_OPEN* msg )
{
	if ( mInventoryDatabase )
		throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    // 플레이어 상태 체크
	if ( IsMapChange() == true )
		throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    if ( GetStateDie() || GetStateStop() == eSTOP_SUMMON_PAPER )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    // 인벤토리 체크
    if( !IsInventoryRange( msg->number ) )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    if( !IsInventoryBag( msg->number ) )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    TB_INVENTORY* inventory = SelectInventory( msg->number );
    if( inventory == NULL )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    // 아이템 체크
    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if( itemDefine == NULL )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    if ( itemDefine->type != ITEM_INVENTORY )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

	if ( IsInventorySeal( itemDefine, inventory ) == true )
		throw ERROR_TRADE_POST_SUMMON_OPEN_LICENSE;


    if ( IsChangeState( eOBJECT_STATE_STOP ) == false )
        throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

    if ( itemDefine->subType == ITEM_INVENTORY_POST_SUMMON )
    {
        HANDLE              handle     = NULL;
        ITEM_SUMMON_POST*   summonPost = (ITEM_SUMMON_POST*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SUMMON_POST );
        long                length     = sizeof(ITEM_SUMMON_POST) - sizeof(summonPost->post);

        // 소환 주문서
        UpdateInventory( inventory, -1 );
        summonPost->inventory = (*inventory);

        // 우편
        summonPost->characterIdx  = mObject.index;
        summonPost->startPage     = 1;

        mInventoryDb.use = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
        if ( mInventoryDb.use == false ) throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;

        throw ERROR_TRADE_POST_SUMMON_OPEN_SUCCESS;
    }
    else if ( itemDefine->subType == ITEM_INVENTORY_USEFUL_PAPER )
    {
        if ( PostOpen( (ULONG_PTR)socketContext ) == true )
        {
			ChangeState( eOBJECT_STATE_STOP );
            SetStateStop( eSTOP_SUMMON_PAPER );
            throw ERROR_TRADE_POST_SUMMON_OPEN_USEFUL_SUCCESS;
        }
    }

    throw ERROR_TRADE_POST_SUMMON_OPEN_FAIL;
}

// ItemSummonAgent Method.
void cPlayer::ItemSummonAgent( ULONG_PTR socketContext, MSG_REQ_TRADE_AGENT_SUMMON_OPEN* msg )
{
	if ( mInventoryDatabase )
		throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    // 플레이어 상태 체크
    if ( IsMapChange() == true )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    if ( GetStateDie() || GetStateStop() == eSTOP_SUMMON_PAPER )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    if( !IsInventoryRange( msg->number ) )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    if( !IsInventoryBag( msg->number ) )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    TB_INVENTORY* inventory = SelectInventory( msg->number );
    if( inventory == NULL )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if( itemDefine == NULL )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

    if ( itemDefine->type != ITEM_INVENTORY )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

	if ( IsInventorySeal( itemDefine, inventory ) == true )
		throw ERROR_TRADE_AGENT_SUMMON_OPEN_LICENSE;

    // 플레이어 상태 체크
    if( IsChangeState( eOBJECT_STATE_STOP ) == false )
        throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;


    // 소모용 소환 주문서
    if ( itemDefine->subType == ITEM_INVENTORY_AGENT_SUMMON )
    {
        HANDLE              handle      = NULL;
        ITEM_SUMMON_AGENT*  summonAgent = (ITEM_SUMMON_AGENT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SUMMON_AGENT );
        long                length      = sizeof(ITEM_SUMMON_AGENT) - sizeof(summonAgent->agent);

        // 소환 주문서
        UpdateInventory( inventory, -1 );
        summonAgent->inventory = (*inventory);

        // 판매 대행
        summonAgent->characterIdx  = mObject.index;
        summonAgent->startPage     = 1;

        mInventoryDb.use = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
        if ( mInventoryDb.use == false ) throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

        throw ERROR_TRADE_AGENT_SUMMON_OPEN_SUCCESS;
    }
    // 기간제 편의기능 주문서
    else if ( itemDefine->subType == ITEM_INVENTORY_USEFUL_PAPER )
    {
        HANDLE                   handle = NULL;
        ITEM_AGENT_SELECT_OWNER* select = (ITEM_AGENT_SELECT_OWNER*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT_OWNER );
        long                     length = sizeof(ITEM_AGENT_SELECT_OWNER) - sizeof(select->table);

        select->characterIdx = mObject.index;
        select->startPage    = 1;

        if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length ) == false )
            throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;

		ChangeState( eOBJECT_STATE_STOP );
        SetStateStop( eSTOP_SUMMON_PAPER );

        throw ERROR_TRADE_AGENT_SUMMON_OPEN_USEFUL_SUCCESS;
    }

    throw ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL;
 }

// ItemSummonWareHouse Method.
void cPlayer::ItemSummonWareHouse( ULONG_PTR socketContext, MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN* msg )
{
	if ( mInventoryDatabase )
		throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    // 플레이어 상태 체크
  	if ( IsMapChange() == true )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    if ( GetStateDie() || GetStateStop() == eSTOP_SUMMON_PAPER )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    // 인벤토리 체크
    if( !IsInventoryRange( msg->number ) )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    if( !IsInventoryBag( msg->number ) )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    TB_INVENTORY* inventory = SelectInventory( msg->number );
    if( inventory == NULL )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

	if ( inventory->apply != InventoryApplyNone )
		throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    // 아이템 체크
    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if( itemDefine == NULL )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    if ( itemDefine->type != ITEM_INVENTORY )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    if ( IsInventorySeal( itemDefine, inventory ) == true )
		throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_LICENSE;


    if ( IsChangeState( eOBJECT_STATE_STOP ) == false )
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

    // 소모용 소환 주문서
    if ( itemDefine->subType == ITEM_INVENTORY_WAREHOUSE_SUMMON )
    {
        HANDLE                  handle     = NULL;
        ITEM_SUMMON_WAREHOUSE*  summonWare = (ITEM_SUMMON_WAREHOUSE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SUMMON_WAREHOUSE );
        summonWare->characterIdx           = mObject.index;

        UpdateInventory( inventory, -1 );
        summonWare->inventory = (*inventory);

        mInventoryDb.use = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_SUMMON_WAREHOUSE) );
        if ( mInventoryDb.use == false ) throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;

        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_SUCCESS;
    }
    // 기간제 편의기능 주문서
    else if ( itemDefine->subType == ITEM_INVENTORY_USEFUL_PAPER )
    {
		ChangeState( eOBJECT_STATE_STOP );
        SetStateStop( eSTOP_SUMMON_PAPER );
        throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_USEFUL_SUCCESS;
    }

    throw ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL;
}

void cPlayer::ItemNpcEvent( ULONG_PTR socketContext, unsigned long eventIndex )
{
	if ( mInventoryDatabase )
		throw ERROR_NPC_EVENT_ITEM_GIVE_FAIL;

	if ( GetStateDie( ) == true )
		throw ERROR_NPC_EVENT_ITEM_GIVE_FAIL;

	if( !(eventIndex > NPC_EVENT_NONE && eventIndex < NPC_EVENT_MAX) )
		throw ERROR_NPC_EVENT_ITEM_GIVE_FAIL;

	HANDLE			handle		= NULL;
	ITEM_NPC_EVENT* itemEvent	= (ITEM_NPC_EVENT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_NPC_EVENT );
	long			length		= sizeof(ITEM_NPC_EVENT) - sizeof(itemEvent->table);

	itemEvent->characterIdx		= mObject.index;
	itemEvent->userIdx			= mConnectionIdx;
	itemEvent->eventType		= eventIndex;
	itemEvent->retvalue			= 0;

	mInventoryDb.cash = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.cash == false )
		throw ERROR_NPC_EVENT_ITEM_GIVE_FAIL;
}

// SendInventory Method.
bool cPlayer::SendInventory(ULONG_PTR socketContext)
{
	HANDLE             handle     = NULL;
	MSG_RES_INVENTORY* sendMsg    = (MSG_RES_INVENTORY*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_INVENTORY_RES );
	sInventory*        sinventory = sendMsg->Inventory;

	TB_INVENTORY*      inventory  = SelectInventory( );
	unsigned long      length     = sizeof(MSG_RES_INVENTORY) - sizeof(sendMsg->Inventory);
	int                index      = MIN_INVENTORY;

	sendMsg->activeWeapon = mItemActiveWeapon;
	sendMsg->RowCount     = 0;

    while ( index <= MAX_VALID_INVENTORY )
	{
		if ( IsInventory( inventory ) )
		{
			Inventory2sInventory( sinventory, inventory );
			sinventory++;

			sendMsg->RowCount++;
		}
		inventory++;
		index++;
	}

	length += (sendMsg->RowCount * sizeof(sendMsg->Inventory));
	return NETWORK2->SendMsgRoot( handle, length );
}

bool cPlayer::SendInventoryCash(ULONG_PTR socketContext)
{
	HANDLE             handle     = NULL;
	MSG_RES_ITEM_CASH_LIST* sendMsg    = (MSG_RES_ITEM_CASH_LIST*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_CASH_LIST_RES );
	sCashInventory*         sinventory = sendMsg->Inventory;

	TB_INVENTORY*           inventory  = SelectInventory( INVENTORY_CASH_ITEM_BEGIN );
	int                     index      = INVENTORY_CASH_ITEM_BEGIN;

	sendMsg->RowCount   = 0;
    sendMsg->ErrorCode  = ERROR_ITEM_CASH_LIST_SUCCESS;

    while ( index <= INVENTORY_CASH_ITEM_END )
	{
		if ( IsInventory( inventory ) )
		{
            sinventory->idx       = inventory->idx;
            sinventory->ItemIndex = inventory->itemDefineIndex;
            sinventory->number    = inventory->number;
            sinventory->count     = inventory->count;
            sinventory->seal      = inventory->seal;
            sinventory->cash      = inventory->cash;
            sinventory++;

			sendMsg->RowCount++;
		}
		inventory++;
		index++;
	}

	return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
}

// SendInventoryCooltime Method.
bool cPlayer::SendInventoryCooltime(ULONG_PTR socketContext)
{
	HANDLE                      handle  = NULL;
	MSG_RES_INVENTORY_COOLTIME* sendMsg = (MSG_RES_INVENTORY_COOLTIME*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_INVENTORY_COOLTIME_RES );
	unsigned long               length  = sizeof(MSG_RES_INVENTORY_COOLTIME) - sizeof(sendMsg->InventoryCooltime);

	PerCooltime*        perCooltime       = (PerCooltime*)mCooltimeRoot.pool;
	sInventoryCooltime* inventoryCooltime = sendMsg->InventoryCooltime;
	long                remainCooltime;
	time_t              ltime;

	time( &ltime );

	while ( perCooltime != NULL )
	{
		remainCooltime = (long)difftime( perCooltime->validThru, ltime );
		if ( remainCooltime > 0 )
		{
			inventoryCooltime->ItemIndex = perCooltime->index;
			inventoryCooltime->cooltime2 = perCooltime->cooltime2;
			inventoryCooltime->cooltime  = remainCooltime*1000;
			inventoryCooltime->cooltime1 = perCooltime->cooltime1;
			inventoryCooltime++;

			sendMsg->RowCount++;
		}
		perCooltime = (PerCooltime*)perCooltime->next;
	}

	length += (sendMsg->RowCount * sizeof(sendMsg->InventoryCooltime));
	return NETWORK2->SendMsgRoot( handle, length );
}

void cPlayer::SetInventoryEquip( sInventory* sInventory, unsigned long* count )
{
    TB_INVENTORY* inventory = NULL;

    for ( short number = INVENTORY_EQUIP_BEGIN; number <=  INVENTORY_EQUIP_END; number++ )
	{
        inventory = SelectInventory( number );
		if ( IsInventory( inventory ) )
		{
            Inventory2sInventory( sInventory, inventory );

            sInventory++;
            (*count)++;
		}
        inventory++;
	}
}

// SetWearInfoWithSendMsg Method - Player의 착용정보(방어구) 설정과 관련 메시지 처리.
bool cPlayer::SetWearInfoWithSendMsg(eWEAR_TYPE type, unsigned long itemDefineIndex)
{
	if ( type < eWEAR_MAX && mPlayerWearInfo.WearIdx[ type ] != itemDefineIndex )
	{
		// 착용정보 백업.
		mPlayerWearInfo.WearIdx[ type ] = itemDefineIndex;

		// 착용정보 보내기-RES.
		HANDLE                     handle = NULL;
		MSG_RES_PLAYER_EQUIP_WEAR* msgRes = (MSG_RES_PLAYER_EQUIP_WEAR*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EQUIP_WEAR_RES );
		if ( msgRes != NULL )
		{
			msgRes->mType     = (unsigned char)type;
			msgRes->mWearInfo = itemDefineIndex;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_EQUIP_WEAR) );
		}

		// 착용정보 보내기-SYN.
		MSG_SYN_PLAYER_EQUIP_WEAR msgSyn;
		memset( &msgSyn, 0, sizeof(MSG_SYN_PLAYER_EQUIP_WEAR) );

		msgSyn.Category      = NM_PLAYER;
		msgSyn.Protocol      = NM_PLAYER_EQUIP_WEAR_SYN;
		msgSyn.mCharacterIdx = mObject.index;			// mPlayerInfo.CharacterIdx 와 같음.
		msgSyn.mType         = (unsigned char)type;
		msgSyn.mWearInfo     = itemDefineIndex;
		NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
		return true;
	}
	return false;
}

// SetCosWearInfoWithSendMsg Method - Player의 코스튬 착용정보(방어구) 설정과 관련 메시지 처리.
bool cPlayer::SetCosWearInfoWithSendMsg( eWEAR_TYPE type, unsigned long itemDefineIndex )
{
    if ( type < eWEAR_MAX )
    {
		unsigned long costumeIndex = mPlayerWearInfo.CostumeIdx[ type ];

		if ( costumeIndex != itemDefineIndex )
		{
			// 착용정보 백업.
			mPlayerWearInfo.CostumeIdx[ type ] = itemDefineIndex;

			// 착용정보.
			TB_ITEM_DEFINE* itemDefine = NULL;
			BYTE            current    = 0;
			BYTE            changed    = 0;

			itemDefine = ITEMMANAGER->GetItemDefineByIndex( costumeIndex );
			if ( itemDefine != NULL )
			{
				current = itemDefine->subType;
			}

			itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
			if ( itemDefine != NULL )
			{
				changed = itemDefine->subType;
			}

			if ( current == ITEM_WEAR_COS_ONEPIECE && changed == ITEM_WEAR_COS_ONEPIECE )
			{
				SendCosWearInfo( eWEAR_ONEPIECE, itemDefineIndex );
			}
			else if ( current == ITEM_WEAR_COS_ONEPIECE )
			{
				SendCosWearInfo( eWEAR_ONEPIECE, 0 );

				if ( changed == ITEM_WEAR_COS_UPPER )
					SendCosWearInfo( type, itemDefineIndex );
			}
			else if ( changed == ITEM_WEAR_COS_ONEPIECE )
			{
				SendCosWearInfo( eWEAR_ONEPIECE, itemDefineIndex );

				mPlayerWearInfo.CostumeIdx[ eWEAR_BODY2 ] = 0;
				mPlayerWearInfo.CostumeIdx[ eWEAR_HAND  ] = 0;
				mPlayerWearInfo.CostumeIdx[ eWEAR_FOOT  ] = 0;
			}
			else
			{
				SendCosWearInfo( type, itemDefineIndex );
			}
			return true;
		}
	}

	return false;
}

// SendCosWearInfo Method
void cPlayer::SendCosWearInfo(eWEAR_TYPE type, unsigned long itemDefineIndex)
{
	// 착용정보 보내기-RES.
	HANDLE                     handle = NULL;
	MSG_RES_PLAYER_COSTUME_WEAR* msgRes = (MSG_RES_PLAYER_COSTUME_WEAR*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_COSTUME_WEAR_RES );
	if ( msgRes != NULL )
	{
		msgRes->mType       = (unsigned char)type;
		msgRes->mWearInfo   = itemDefineIndex;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_COSTUME_WEAR) );
	}

	// 착용정보 보내기-SYN.
	MSG_SYN_PLAYER_COSTUME_WEAR msgSyn;
	memset( &msgSyn, 0, sizeof(MSG_SYN_PLAYER_COSTUME_WEAR) );

	msgSyn.Category      = NM_PLAYER;
	msgSyn.Protocol      = NM_PLAYER_COSTUME_WEAR_SYN;
	msgSyn.mCharacterIdx = mObject.index;			// mPlayerInfo.CharacterIdx 와 같음.
	msgSyn.mType         = (unsigned char)type;
	msgSyn.mWearInfo     = itemDefineIndex;
	NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
}

// SetCosAccInfoWithSendMsg Method - Player의 코스튬 착용정보(악세사리) 설정과 관련 메시지 처리.
bool cPlayer::SetCosAccInfoWithSendMsg( eCOSTUME_LINK type, unsigned long itemDefineIndex )
{
   if ( type < eCOSTUME_MAX && mPlayerWearInfo.CostumeAccIdx[ type ] != itemDefineIndex )
    {
        mPlayerWearInfo.CostumeAccIdx[ type ] = itemDefineIndex;

        HANDLE                      handle = NULL;
        MSG_RES_PLAYER_COSTUME_ACC* msgRes = (MSG_RES_PLAYER_COSTUME_ACC*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_COSTUME_ACC_RES );
        if ( msgRes != NULL )
        {
            msgRes->mType     = (unsigned char)type;
            msgRes->mWearInfo = itemDefineIndex;
            NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_COSTUME_ACC) );
        }

        // 착용정보 보내기-SYN.
        MSG_SYN_PLAYER_COSTUME_ACC msgSyn;
        memset( &msgSyn, 0, sizeof(MSG_SYN_PLAYER_COSTUME_ACC) );

        msgSyn.Category      = NM_PLAYER;
        msgSyn.Protocol      = NM_PLAYER_COSTUME_ACC_SYN;
        msgSyn.mCharacterIdx = mObject.index;
        msgSyn.mType         = (unsigned char)type;
        msgSyn.mWearInfo     = itemDefineIndex;
        NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
        return true;
    }
    return false;
}

// SetWeaponInfoWithSendMsg Method - Player의 착용정보(무기) 설정과 관련 메시지 처리.
void cPlayer::SetWeaponInfoWithSendMsg(TB_INVENTORY* left, TB_INVENTORY* right)
{
	// 착용정보 백업.
	mPlayerWeaponInfo.WeaponIdx[ eHAND_LEFT  ] = left->itemDefineIndex;
	mPlayerWeaponInfo.WeaponIdx[ eHAND_RIGHT ] = right->itemDefineIndex;

	mPlayerWeaponInfo.WeaponEnhanced[ eHAND_LEFT  ] = left->enhanced;
	mPlayerWeaponInfo.WeaponEnhanced[ eHAND_RIGHT ] = right->enhanced;

	// 착용정보 보내기-RES.
	HANDLE                       handle = NULL;
	MSG_RES_PLAYER_EQUIP_WEAPON* msgRes = (MSG_RES_PLAYER_EQUIP_WEAPON*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_EQUIP_WEAPON_RES );
	if ( msgRes != NULL )
	{
		msgRes->mWeaponState = (unsigned char)mPlayerWeaponState;
		msgRes->mWeaponInfo  = mPlayerWeaponInfo;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_EQUIP_WEAPON) );
	}

	// 착용정보 보내기-SYN.
	MSG_SYN_PLAYER_EQUIP_WEAPON msgSyn;
	memset( &msgSyn, 0, sizeof(MSG_SYN_PLAYER_EQUIP_WEAPON) );
	msgSyn.Category      = NM_PLAYER;
	msgSyn.Protocol      = NM_PLAYER_EQUIP_WEAPON_SYN;
	msgSyn.mCharacterIdx = mObject.index;
	msgSyn.mWeaponState  = (unsigned char)mPlayerWeaponState;
	msgSyn.mWeaponInfo   = mPlayerWeaponInfo;
	NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
}

// CbItemBillDel Method.
bool cPlayer::CbItemBillDel(ULONG_PTR socketContext, ITEM_BILL_REMOVE* remove, u_long requestResult)
{
	if ( mInventoryDb.itemBill == true )
		mInventoryDb.itemBill = false;
	else
		return false;

	if ( requestResult != 0 ) throw requestResult;
	if ( remove->retvalue != 0 ) throw remove->retvalue;

	HANDLE                 handle   = NULL;
	MSG_SYN_ITEM_BILL_DEL* sendSyn  = (MSG_SYN_ITEM_BILL_DEL*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_BILL_DEL_SYN );

	sendSyn->idx = remove->idx;

	return NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_BILL_DEL) );
}

// CbInventoryInsert Method.
void cPlayer::CbInventoryInsert(ULONG_PTR socketContext, INVENTORY_INSERT* inventoryInsert, u_long requestResult)
{
	if ( mInventoryDb.insert )
		mInventoryDb.insert = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		HANDLE              handle  = NULL;
		MSG_RES_CHEAT_MAKE* sendMsg = (MSG_RES_CHEAT_MAKE*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_CHEAT, NM_CHEAT_MAKE_RES );

		TB_INVENTORY* table      = inventoryInsert->table;
		sInventory*   sinventory = sendMsg->inventory;

		sendMsg->ErrorCode = ERROR_CHEAT_MAKE_SUCCESS;

		for ( long i = 0; i < inventoryInsert->rowCount; i++, table++ )
		{
			if ( table->idx != 0 )
			{
				XCopyInventory( table );
				Inventory2sInventory( sinventory, table );

				sinventory++;
				sendMsg->rowCount++;

				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, table, "INSERT-CHEAT" );
			}
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	else
		throw requestResult;
}

// CbInventoryRemove Method.
void cPlayer::CbInventoryRemove(ULONG_PTR socketContext, INVENTORY_REMOVE* inventoryRemove, u_long requestResult)
{
	if ( mInventoryDb.remove )
		mInventoryDb.remove = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* inventory = SelectInventory( inventoryRemove->number );

		if ( inventoryRemove->idx == inventory->idx )
		{
			// 아이템 빌 삭제.
			if ( inventory->idx > 0 )
			{
				PerItemBill* itemBill = ITEMBILLPOOL->SearchItemBill( &mItemBillRoot, inventory->idx );
				if ( itemBill != NULL )
				{
					itemBill->apply = ItemBillApplyReserved;
				}
			}
			// 로그 전송.
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, inventory, "REMOVE" );
			// 삭제 완료.
			RemoveInventory( inventory );
			// 결과 전송.
			NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_DEL_INVENTORY_RES, ERROR_ITEM_DEL_INVENTORY_SUCCESS );
		}
		else
			throw inventory;
	}
	else
		throw requestResult;
}

// CbInventoryUse Method.
void cPlayer::CbInventoryUse(ULONG_PTR socketContext, INVENTORY_USE* inventoryUse, u_long requestResult)
{
	if ( mInventoryDb.use )
		mInventoryDb.use = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table     = inventoryUse->table;
		TB_INVENTORY* inventory = SelectInventory( table->number );

		if ( table->idx != inventory->idx )
		{
			throw table;
		}
		else if ( table->apply == InventoryApplyRemoved )
		{
			// 삭제 완료.
			RemoveInventory( inventory );
			// 로그 전송.
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "USE" );
		}
		else
		{
			// 갱신 완료.
			XCopyInventory( inventory, table, XCOPY_INVENTORY_COUNT );
			// 로그 전송.
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "USE" );
		}

		// 결과 전송.
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_USE_INVENTORY_RES, ERROR_ITEM_USE_INVENTORY_SUCCESS );
	}
	else
		throw requestResult;
}

// CbInventorySwap Method.
void cPlayer::CbInventorySwap(ULONG_PTR /*socketContext*/, INVENTORY_SWAP* /*inventorySwap*/, u_long requestResult)
{
	if ( mInventoryDb.swap )
		mInventoryDb.swap = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 )
		throw requestResult;
}

// CbInventoryMove Method.
void cPlayer::CbInventoryMove(ULONG_PTR /*socketContext*/, INVENTORY_MOVE* inventoryMove, u_long requestResult)
{
	if ( mInventoryDb.move )
		mInventoryDb.move = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* inventory = SelectInventory( inventoryMove->number );

		if ( inventoryMove->cashBox )
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, inventory, "CASH BOX" );

		if ( inventory->idx != inventoryMove->idx || inventory->number != inventoryMove->number )
			throw inventory;
	}
	else
		throw requestResult;
}

// CbInventoryExcept Method.
void cPlayer::CbInventoryExcept(ULONG_PTR /*socketContext*/, INVENTORY_EXCEPT* inventoryExcept, u_long requestResult)
{
	if ( mInventoryDb.except )
		mInventoryDb.except = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* inventory;
		for ( long i = 0; i <  inventoryExcept->rowCount; i++ )
		{
			inventory = SelectInventory( inventoryExcept->table[ i ].number );
			if ( inventory->idx != inventoryExcept->table[ i ].idx )
				throw inventory;
		}
	}
	else
		throw requestResult;
}

// CbInventoryMerge Method.
void cPlayer::CbInventoryMerge(ULONG_PTR socketContext, INVENTORY_MERGE* inventoryMerge, u_long requestResult)
{
	if ( mInventoryDb.merge )
		mInventoryDb.merge = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table     = inventoryMerge->table;
		TB_INVENTORY* inventory = NULL;

		for ( long i = 0; i < inventoryMerge->rowCount; i++, table++ )
		{
			inventory = SelectInventory( table->number );

			if ( inventory->idx == table->idx )
			{
				if ( table->apply == InventoryApplyRemoved )
				{
					// 삭제 완료.
					RemoveInventory( inventory );
					// 로그 전송.
					NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "MERGE" );
				}
				else
				{
					// 갱신 완료.
					XCopyInventory( inventory, table, XCOPY_INVENTORY_COUNT );
					// 로그 전송.
					NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "MERGE" );
				}
			}
			else
				throw table;
		}
		// 결과 전송.
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_MERGE_INVENTORY_RES, ERROR_ITEM_MERGE_INVENTORY_SUCCESS );
	}
	else
		throw requestResult;
}

// CbInventoryDivide Method.
void cPlayer::CbInventoryDivide(ULONG_PTR socketContext, INVENTORY_DIVIDE* inventoryDivide, u_long requestResult)
{
	if ( mInventoryDb.divide )
		mInventoryDb.divide = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table = inventoryDivide->table;
		TB_INVENTORY* inventory;

		for ( int i = 0; i < ROW_INVENTORY_DIVIDE; i++, table++ )
		{
			inventory = SelectInventory( table->number );

			// 로그 전송.
			NETWORK2->PostInventoryEvent( inventory->idx == table->idx ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
										 ,mObject.index
										 ,table
										 ,"DIVIDE" );
			// 갱신 완료.
			XCopyInventory( table );
		}
		// 결과 전송.
		HANDLE                         handle  = NULL;
		MSG_RES_ITEM_DIVIDE_INVENTORY* sendMsg = (MSG_RES_ITEM_DIVIDE_INVENTORY*)NETWORK2->GetMsgRoot( &handle
																									  ,(PerSocketContext*)socketContext
																									  ,NM_ITEM
																									  ,NM_ITEM_DIVIDE_INVENTORY_RES );
		sInventory* sinventory = sendMsg->Inventory;

		sendMsg->ErrorCode = ERROR_ITEM_DIVIDE_INVENTORY_SUCCESS;
		sendMsg->RowCount  = ROW_INVENTORY_DIVIDE;

		table = inventoryDivide->table;
		for ( int i = 0; i < ROW_INVENTORY_DIVIDE; i++, table++, sinventory++ )
		{
			Inventory2sInventory( sinventory, table );
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	else
		throw requestResult;
}

// CbInventoryMove2 Method.
void cPlayer::CbInventoryMove2(ULONG_PTR socketContext, INVENTORY_MOVE2* inventoryMove2, u_long requestResult)
{
	if ( mInventoryDb.move2 )
		mInventoryDb.move2 = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table;
		TB_INVENTORY* inventory;

		table = inventoryMove2->table;
		for ( long i = 0; i < inventoryMove2->rowCount; i++, table++ )
		{
			inventory = SelectInventory( table->number );

			// 로그 전송.
			NETWORK2->PostInventoryEvent( (inventory->idx == table->idx) ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
										 ,mObject.index
										 ,table
										 ,"MOVE2" );
			// 갱신.
			XCopyInventory( inventory, table );
		}

		// 결과 전송.
		HANDLE                           handle  = NULL;
		MSG_RES_NPC_ITEM_MOV2_INVENTORY* sendMsg = (MSG_RES_NPC_ITEM_MOV2_INVENTORY*)NETWORK2->GetMsgRoot( &handle
																										  ,(PerSocketContext*)socketContext
																										  ,NM_NPC
																										  ,NM_NPC_ITEM_MOV2_INVENTORY_RES );
		sInventory* sinventory = sendMsg->Inventory;

		sendMsg->ErrorCode = ERROR_ITEM_MOV2_INVENTORY_SUCCESS;
		sendMsg->RowCount  = inventoryMove2->rowCount;

		table = inventoryMove2->table;
		for ( long i = 0; i < inventoryMove2->rowCount; i++, table++, sinventory++ )
		{
			Inventory2sInventory( sinventory, table );
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	else
		throw requestResult;
}

// CbInventoryEnhanced Method.
void cPlayer::CbInventoryEnhanced(ULONG_PTR socketContext, INVENTORY_ENHANCED* inventoryEnhanced, u_long requestResult)
{
	if ( mInventoryDb.enhanced )
		mInventoryDb.enhanced = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table     = NULL;
        TB_INVENTORY* card      = NULL;
        TB_INVENTORY* safeCard  = NULL;

		// 강화장비
		table = &inventoryEnhanced->table[ 0 ];
		switch ( inventoryEnhanced->result )
		{
		case RESULT_INVENTORY_ENHANCED_SUCCESS:
            {
                XCopyInventory( table );
                NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "ENHANCED SUCCESS" );

                if ( table->enhanced > 8 )
                {
                    MSG_SYN_CHAT_SYSTEM sync;
                    memset( &sync, 0, sizeof(sync) );

                    sync.Category = NM_CHAT;
                    sync.Protocol = NM_CHAT_ENHANCED_SYN;
                    Inventory2sInventory( &sync.inventory, table );

                    wcscpy( sync.characterName, mPlayerInfo.strName );

                    cSender* sender = g_gameSrv->GetSender( );
                    sender->PostEnhancedSync( NETWORK2->GetCID( ), sync.characterName, (char*)&sync.inventory, sizeof(sync.inventory) );

                    OBJECTMANAGER->SendNotice( (char*)&sync, sizeof(MSG_SYN_CHAT_SYSTEM) );
                }
            }
			break;
		case RESULT_INVENTORY_ENHANCED_DOWN:
			XCopyInventory( table );
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "ENHANCED DOWN" );
			break;
		case RESULT_INVENTORY_ENHANCED_DESTROY:
			if ( table->apply == InventoryApplyRemoved )
			{
				RemoveInventory( table->number );
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "ENHANCED DESTRUCT" );

                if ( table->enhanced > 8 )
                {
                    MSG_SYN_CHAT_SYSTEM sync;
                    memset( &sync, 0, sizeof(sync) );

                    sync.Category = NM_CHAT;
                    sync.Protocol = NM_CHAT_ENHANCED_SYN;
                    Inventory2sInventory( &sync.inventory, table );

                    cSender* sender = g_gameSrv->GetSender( );
                    sender->PostEnhancedSync( NETWORK2->GetCID( ), NULL, (char*)&sync.inventory, sizeof(sync.inventory) );

                    OBJECTMANAGER->SendNotice( (char*)&sync, sizeof(MSG_SYN_CHAT_SYSTEM) );
                }
			}
			break;
		case RESULT_INVENTORY_ENHANCED_KEEP:
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "ENHANCED FAIL" );
			break;
		}

		// 강화카드
		card = &inventoryEnhanced->table[ 1 ];
		if ( card->apply == InventoryApplyRemoved )
		{
			RemoveInventory( card->number );
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, card, "USED ENHANCED" );
		}
		else
		{
			XCopyInventory( card );
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, card, "USED ENHANCED" );
		}

		// 강화보호카드
        safeCard = &inventoryEnhanced->table[ 2 ];
        if ( safeCard->idx > 0 )
        {
            if ( safeCard->apply == InventoryApplyRemoved )
            {
                RemoveInventory( safeCard->number );
                NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, safeCard, "USED ENHANCED" );
            }
            else
            {
                XCopyInventory( safeCard );
                NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, safeCard, "USED ENHANCED" );
            }
        }

		// 결과 전송
		// HERO에 결과전송
		HANDLE                     handle  = NULL;
		MSG_SYN_ITEM_ENHANCED_END* sendMsg = (MSG_SYN_ITEM_ENHANCED_END*)NETWORK2->GetMsgRoot( &handle
																							  ,(PerSocketContext*)socketContext
																							  ,NM_ITEM
																							  ,NM_ITEM_ENHANCED_END_SYN );
		sInventory*   sinventory = sendMsg->Inventory;
		TB_INVENTORY* inventory  = inventoryEnhanced->table;

		sendMsg->ErrorCode  = ERROR_ITEM_ENHANCED_END_SUCCESS;
        sendMsg->RowCount   = 0;
		for ( int i = 0; i < 3; i++, sinventory++, inventory++ )
        {
            if ( inventory->idx > 0 )
            {
			    Inventory2sInventory( sinventory, inventory );
                sendMsg->RowCount++;
            }
        }

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );


        ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );

		// 다른 유저들에게 결과전송
		MSG_SYN_ITEM_ENHANCED_OTHEREND sync;
			sync.Category  = NM_ITEM;
			sync.Protocol  = NM_ITEM_ENHANCED_OTHEREND_SYN;
			sync.ErrorCode = inventoryEnhanced->result;
			sync.characterIdx = mObject.index;
		NETWORK2->QuickSendExcept( this, (char*)&sync, sizeof(MSG_SYN_ITEM_ENHANCED_OTHEREND) );

		// 튜토리얼 퀘스트 체크
		if( IsTutorialMode() == true )
		{
			UpdateDutyTutorial( eDUTY_ENHANCE );
		}
	}
	else
		throw requestResult;
}

// CbInventoryDisjoint Method.
void cPlayer::CbInventoryDisjoint(ULONG_PTR socketContext, INVENTORY_DISJOINT* inventoryDisjoint, u_long requestResult)
{
	if ( mInventoryDb.disjoint )
		mInventoryDb.disjoint = false;	// DB; 사용종료
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( inventoryDisjoint->retvalue != 0 ) throw inventoryDisjoint->retvalue;

	// 비용 처리.
	if ( inventoryDisjoint->characterMoney != 0 )
	{
		unsigned long befor  = GetMoney( );
		long          result = AddMoney( GetObject( ), inventoryDisjoint->characterMoney, false );

		NETWORK2->PostMoneyEvent( inventoryDisjoint->characterIdx, befor, GetMoney( ), "INVENTORY_DISJOINT::Money(=%d)", inventoryDisjoint->characterMoney );

		if ( result != inventoryDisjoint->characterMoney )
			NETWORK2->PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_DISJOINT AddMoney(=%d) != ResultMoney(=%d).", inventoryDisjoint->characterMoney, result );
	}
	// 결과 전송.
	HANDLE                           handle     = NULL;
	MSG_RES_ITEM_DISJOINT_INVENTORY* sendMsg    = (MSG_RES_ITEM_DISJOINT_INVENTORY*)NETWORK2->GetMsgRoot( &handle
																										 ,(PerSocketContext*)socketContext
																										 ,NM_ITEM
																										 ,NM_ITEM_DISJOINT_INVENTORY_RES );
	sInventory* sinventory = sendMsg->Inventory;

	TB_INVENTORY* table      = inventoryDisjoint->table;
	TB_INVENTORY* inventory  = NULL;

	sendMsg->ErrorCode = ERROR_ITEM_DISJOINT_INVENTORY_SUCCESS;
    sendMsg->RowCount  = 0;

	for ( long i = 0; i < inventoryDisjoint->rowCount; i++, table++ )
	{
		if ( table->apply == InventoryApplyRemoved )
		{
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "INVENTORY_DISJOINT" );
		}
		else
		{
			inventory = SelectInventory( table->number );

			NETWORK2->PostInventoryEvent( (inventory->idx == table->idx) ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
										 ,mObject.index
										 ,table
										 ,"INVENTORY_DISJOINT" );

			XCopyInventory( inventory, table );
			Inventory2sInventory( sinventory, inventory );
			sinventory++;

			sendMsg->RowCount++;
		}
	}

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
}

// CbInventoryPutCard Method.
void cPlayer::CbInventoryPutCard(ULONG_PTR socketContext, INVENTORY_PUT_CARD* inventoryPutCard, u_long requestResult)
{
	if ( mInventoryDb.putCard )
		mInventoryDb.putCard = false;    // DB; 사용종료
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table = inventoryPutCard->table;
		for ( int i = 0; i < 2; i++, table++ )
		{
			if ( table->apply == 0 )
			{
				// 장착 아이템
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "PUT_CARD" );
				XCopyInventory( table );

				HANDLE                           handle  = NULL;
				MSG_RES_ITEM_PUT_CARD_INVENTORY* sendMsg = (MSG_RES_ITEM_PUT_CARD_INVENTORY*)NETWORK2->GetMsgRoot( &handle
																												   ,(PerSocketContext*)socketContext
																												   ,NM_ITEM
																												   ,NM_ITEM_PUT_CARD_INVENTORY_RES );

				sendMsg->ErrorCode = ERROR_ITEM_PUT_CARD_INVENTORY_SUCCESS;
				Inventory2sInventory( &sendMsg->Inventory, table );

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_PUT_CARD_INVENTORY) );
			}
			else
			{
				// 카드 아이템
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "PUT_CARD" );
				RemoveInventory( table->number );
			}
		}

		// 튜토리얼 퀘스트 체크
		if( IsTutorialMode() == true )
		{
			UpdateDutyTutorial( eDUTY_PUTCARD );
		}
	}
	else
		throw requestResult;
}

// CbInventoryChange Method.
void cPlayer::CbInventoryChange(ULONG_PTR socketContext, INVENTORY_CHANGE* inventoryChange, u_long requestResult)
{
	if ( mInventoryDb.change )
		mInventoryDb.change = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( inventoryChange->retvalue != 0 ) throw inventoryChange->retvalue;

	// 비용 처리.
	if ( inventoryChange->characterMoney != 0 )
	{
		unsigned long befor  = GetMoney( );
		long          result = AddMoney( GetObject( ), inventoryChange->characterMoney, false );

		NETWORK2->PostMoneyEvent( inventoryChange->characterIdx, befor, GetMoney( ), "INVENTORY_CHANGE::Money(=%d)", inventoryChange->characterMoney );

		if ( result != inventoryChange->characterMoney )
			NETWORK2->PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_CHANGE AddMoney(=%d) != ResultMoney(=%d).", inventoryChange->characterMoney, result );
	}

	// 결과 전송.
	HANDLE                         handle  = NULL;
	MSG_RES_ITEM_CHANGE_INVENTORY* sendMsg = (MSG_RES_ITEM_CHANGE_INVENTORY*)NETWORK2->GetMsgRoot( &handle
																								  ,(PerSocketContext*)socketContext
																								  ,NM_ITEM
																								  ,NM_ITEM_CHANGE_INVENTORY_RES );
	sInventory* sinventory = sendMsg->Inventory;

	TB_INVENTORY* table = inventoryChange->table;
	TB_INVENTORY* inventory;

	sendMsg->ErrorCode = ERROR_ITEM_CHANGE_INVENTORY_SUCCESS;
    sendMsg->RowCount  = 0;

	for ( long i = 0; i < inventoryChange->rowCount; i++, table++ )
	{
		if ( table->apply == InventoryApplyRemoved )
		{
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "INVENTORY_CHANGE" );
		}
		else
		{
			inventory = SelectInventory( table->number );

			NETWORK2->PostInventoryEvent( inventory->idx == table->idx ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
										 ,mObject.index
										 ,table
										 ,"INVENTORY_CHANGE" );

			XCopyInventory( table );
		}

		Inventory2sInventory( sinventory, table );
		sinventory++;

		sendMsg->RowCount++;
	}

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

	/// 유니크 아이템 습득 SYNC
	TB_INVENTORY* syncTable = inventoryChange->table;
	TB_INVENTORY* syncInven = NULL;

	sInventory inven;
	memset( &inven, 0, sizeof(inven) );

	for( long i = 0; i < inventoryChange->rowCount; i++, syncTable++ )
	{
		syncInven = SelectInventory( syncTable->number );
		if( IsInventory( syncInven ) )
		{
			TB_ITEM_DEFINE* define = ITEMMANAGER->GetItemDefine( syncInven->itemDefineIdx );
			if( define == NULL )
				continue;

			// 유니크
			if( define->rareLevel >= eTOOLTIP_UNIQUE && define->rareLevel <= eTOOLTIP_MAX )
			{
				Inventory2sInventory( &inven, syncInven );

				cSender* sender = g_gameSrv->GetSender( );
				sender->PostUniqueItemSync( NETWORK2->GetCID( ), GetName(), (char*)&inven, sizeof(inven) );
			}
		}
	}
}

// CbInventorySeal Method.
void cPlayer::CbInventorySeal(ULONG_PTR socketContext, INVENTORY_SEAL* inventorySeal, u_long requestResult)
{
	HANDLE handle = NULL;

	if ( mInventoryDb.seal )
		mInventoryDb.seal = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( inventorySeal->retvalue != 0 ) throw inventorySeal->retvalue;

	// 인벤토리 인증 완료
	TB_INVENTORY* inventory = &inventorySeal->inventory;

	MSG_RES_ITEM_LICENSE* sendMsg = (MSG_RES_ITEM_LICENSE*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_LICENSE_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode  = ERROR_ITEM_LICENSE_SUCCESS;
		sendMsg->number     = inventory->number;
		sendMsg->seal       = inventory->seal;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_LICENSE) );
	}

    NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, inventory, "SEAL-INVENTORY" );

    // 기간(시간) 완료.
	TB_ITEM_BILL* table = &inventorySeal->itemBill;

	if ( table->idx != 0 )
	{
		PerItemBill* itemBillPool = AddItemBill( table );
		if ( itemBillPool != NULL )
		{
			MSG_SYN_ITEM_BILL_ADD* sendSyn  = (MSG_SYN_ITEM_BILL_ADD*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_BILL_ADD_SYN );
			sItemBill&             itemBill = sendSyn->itemBill;

			itemBill.idx          = itemBillPool->idx;
			itemBill.type         = itemBillPool->type;
			itemBill.validDate    = itemBillPool->timeEnd;
			itemBill.validTime    = itemBillPool->validTime;
			itemBill.inventoryIdx = itemBillPool->inventoryIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_BILL_ADD) );
		}
	}
}

// CbInventoryCashSelect Method.
void cPlayer::CbInventoryCashSelect( ULONG_PTR socketContext, INVENTORY_SELECT* inventorySelect, u_long requestResult )
{
	if ( mInventoryDb.cash ) 
		mInventoryDb.cash = false;
	else
		return;

	if ( requestResult == 0 ) 
	{
		TB_INVENTORY* inventory = SelectInventory( INVENTORY_CASH_ITEM_BEGIN );
		TB_INVENTORY* table     = &inventorySelect->table[0];

		int rowCount = 0;
		for ( short number = INVENTORY_CASH_ITEM_BEGIN; number < INVENTORY_CASH_ITEM_END; number++, inventory++ )
		{
			if ( rowCount < inventorySelect->rowCount )
			{
				memcpy( inventory, table, sizeof(TB_INVENTORY) );
				inventory->number = number;

				table++;
				rowCount++;
			}
			else if ( IsInventory( inventory ) == true )
			{
				memset( inventory, 0, sizeof(TB_INVENTORY) );
			}
		}

		SendInventoryCash( socketContext );
	}
	else
		throw requestResult;

}

// CbItemSell Method.
void cPlayer::CbItemSell(ULONG_PTR socketContext, ITEM_SELL* itemSell, u_long requestResult)
{
	if ( mInventoryDb.sell )
		mInventoryDb.sell = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table = itemSell->table;
		TB_INVENTORY* inventory;

		HANDLE                handle  = NULL;
		MSG_RES_NPC_ITEM_BUY* sendMsg = (MSG_RES_NPC_ITEM_BUY*)NETWORK2->GetMsgRoot( &handle
																					,(PerSocketContext*)socketContext
																					,NM_NPC
																					,NM_NPC_ITEM_SELL_RES );
		sInventory* sinventory = sendMsg->inventory;

		sendMsg->ErrorCode = ERROR_ITEM_SELL_SUCCESS;
		sendMsg->rowCount = 0;

		for ( long i = 0; i < itemSell->rowCount; i++, table++, sinventory++ )
		{
			inventory = SelectInventory( table->number );

			if ( table->apply == InventoryApplyRemoved )
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "SELL-INVENTORY" );
				RemoveInventory( inventory );
			}
			else
			{
				NETWORK2->PostInventoryEvent( inventory->idx == table->idx ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
											 ,mObject.index
											 ,table
											 ,"SELL-REWARD" );
				XCopyInventory( table );
			}

			Inventory2sInventory( sinventory, table );
			sendMsg->rowCount++;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

		if ( itemSell->sellPrice > 0 )
		{
			switch ( itemSell->sellType )
			{
			case ItemSellMoney:
				{
					sObject object = { eOBJECTTYPE_NPC, itemSell->npcIdx };
					long    result = AddMoney( object, itemSell->sellPrice );

					if ( result != itemSell->sellPrice )
						NETWORK2->PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_SELL - CharacterIdx(%u) Money(%u)", mObject.index, GetMoney( ) );
				}
				break;
			case ItemSellPvp:
				//Skip
				//AddPvPPoint( itemSell->sellPrice, eFORCETYPE_FIRE );
				//AddPvPPoint( itemSell->sellPrice, eFORCETYPE_WATER );
				//AddPvPPoint( itemSell->sellPrice, eFORCETYPE_WIND );
				//AddPvPPoint( itemSell->sellPrice, eFORCETYPE_EARTH );
				break;
			case ItemBuyTarot:
				AddTarotPoint( itemSell->sellPrice );
				break;
			}
		}
	}
	else
		throw requestResult;
}

// CbItemBuy Method.
void cPlayer::CbItemBuy(ULONG_PTR socketContext, ITEM_BUY* itemBuy, u_long requestResult)
{
	if ( mInventoryDb.buy )
		mInventoryDb.buy = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* table   = itemBuy->table;
		TB_INVENTORY* inventory;

		HANDLE                handle  = NULL;
		MSG_RES_NPC_ITEM_BUY* sendMsg = (MSG_RES_NPC_ITEM_BUY*)NETWORK2->GetMsgRoot( &handle
																					,(PerSocketContext*)socketContext
																					,NM_NPC
																					,NM_NPC_ITEM_BUY_RES );
		sInventory* sinventory = sendMsg->inventory;

		sendMsg->ErrorCode = ERROR_ITEM_BUY_SUCCESS;
		sendMsg->rowCount  = 0;

		for ( long i = 0; i < itemBuy->rowCount; i++, table++, sinventory++ )
		{
			inventory = SelectInventory( table->number );

			if ( table->apply == InventoryApplyRemoved )
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "BUY-INVENTORY" );
				RemoveInventory( inventory );
			}
			else
			{
				NETWORK2->PostInventoryEvent( inventory->idx == table->idx ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
											 ,mObject.index
											 ,table
											 ,"BUY-REWARD" );
				XCopyInventory( table );
			}

			Inventory2sInventory( sinventory, table );
			sendMsg->rowCount++;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

		if ( itemBuy->buyPrice != 0 )
		{
			switch ( itemBuy->buyType )
			{
			case ItemBuyMoney:
				{
					sObject object = { eOBJECTTYPE_NPC, itemBuy->npcIdx };
					long    result = AddMoney( object, itemBuy->buyPrice );

					if ( result != itemBuy->buyPrice )
						NETWORK2->PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_BUY - CharacterIdx(%u) Money(%u)", mObject.index, GetMoney( ) );
				}
				break;
			case ItemBuyPvp:
				AddPvPPoint( itemBuy->buyPrice, itemBuy->buyStore );
				break;
			case ItemBuyTarot:
				AddTarotPoint( itemBuy->buyPrice );
				break;
			}
		}
	}
	else
		throw requestResult;
}

// CbItemCollect Method.
void cPlayer::CbItemCollect(ULONG_PTR socketContext, ITEM_COLLECT* itemCollect, u_long requestResult)
{
	if ( mInventoryDb.collect )
		mInventoryDb.collect = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( itemCollect->retvalue != 0 ) throw itemCollect->retvalue;

    TB_INVENTORY* table = itemCollect->table;

    // 결과 전송.
    HANDLE                handle  = NULL;
    MSG_RES_ITEM_COLLECT* sendMsg = (MSG_RES_ITEM_COLLECT*)NETWORK2->GetMsgRoot( &handle
                                                                                ,(PerSocketContext*)socketContext
                                                                                ,NM_ITEM
                                                                                ,NM_ITEM_COLLECT_RES );
    sendMsg->ErrorCode = ERROR_ITEM_COLLECT_SUCCESS;
    sendMsg->RowCount  = 2;
    Inventory2sInventory( &sendMsg->Inventory[0], &table[0] );   // 인벤토리
    Inventory2sInventory( &sendMsg->Inventory[1], &table[1] );
    NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

    if ( table[2].apply == InventoryApplyRemoved )
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, &table[2], "ITEM COLLECT" );
    }
    if ( table[0].idx != table[1].idx )
    {
		TB_INVENTORY* inventory = SelectInventory( table[1].number );
		inventory->idx = table[1].idx;

        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, &table[0], "ITEM COLLECT" );
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, &table[1], "ITEM COLLECT" );
    }
}

// CbItemCollectChange Method.
void cPlayer::CbItemCollectChange(ULONG_PTR socketContext, ITEM_COLLECT_CHANGE* itemCollect, u_long requestResult)
{
	if ( mInventoryDb.collect )
		mInventoryDb.collect = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( itemCollect->retvalue != 0 ) throw itemCollect->retvalue;


    TB_INVENTORY* changeItem = &itemCollect->changeItem;
    TB_INVENTORY* tarotCard  = itemCollect->table;
    TB_INVENTORY* inventory  = NULL;


    // 결과 전송.
    HANDLE                       handle  = NULL;
    MSG_RES_COLLECT_CARD_CHANGE* sendMsg = (MSG_RES_COLLECT_CARD_CHANGE*)NETWORK2->GetMsgRoot( &handle
                                                                                              ,(PerSocketContext*)socketContext
                                                                                              ,NM_ITEM
                                                                                              ,NM_ITEM_COLLECT_CARD_CHANGE_RES );
	sInventory* sinventory  = sendMsg->Inventory;
    sendMsg->ErrorCode      = ERROR_ITEM_COLLECT_CARD_CHANGE_SUCCESS;

    // 체인지 아이템
    inventory = SelectInventory( changeItem->number );
    NETWORK2->PostInventoryEvent( (inventory->idx != changeItem->idx) ? EVENT_INVENTORY_CREATE : EVENT_INVENTORY_UPDATE, 
                                  mObject.index, 
                                  changeItem, 
                                  "COLLECT-CHANGE" );

    XCopyInventory( changeItem );
    Inventory2sInventory( sinventory, changeItem );
    sinventory++;   sendMsg->RowCount++;


    // 타로카드 22장
    for ( long i = 0; i < itemCollect->rowCount; i++, sinventory++, tarotCard++ )
    {
        Inventory2sInventory( sinventory, tarotCard );
        sendMsg->RowCount++;

        inventory = SelectInventory( tarotCard->number );
        if ( tarotCard->apply == InventoryApplyRemoved )
        {
            NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, tarotCard, "COLLECT-CHANGE" );
            RemoveInventory( inventory );
        }
        else
        {
            NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, tarotCard, "COLLECT-CHANGE" );
        }
    }

    NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
}

// CbItemGet Method.
void cPlayer::CbItemGet(ULONG_PTR socketContext, ITEM_GET* itemGet, u_long requestResult)
{
	if ( mInventoryDb.get )
		mInventoryDb.get = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;

	HANDLE            handle  = NULL;
	MSG_RES_ITEM_GET* sendMsg = (MSG_RES_ITEM_GET*)NETWORK2->GetMsgRoot( &handle
																		,(PerSocketContext*)socketContext
																		,NM_ITEM
																		,NM_ITEM_GET_RES );
	BYTE&       rowCount   = sendMsg->RowCount;
	sInventory* sinventory = sendMsg->Inventory;
	int         error      = ERROR_ITEM_GET_SUCCESS;

	TB_INVENTORY* table = itemGet->table;
	TB_INVENTORY* inventory;

	for ( long i = 0; i < itemGet->rowCount; i++, table++ )
	{
		inventory = SelectInventory( table->number );

		if ( inventory->apply == InventoryApplyGetAuto )
			error = ERROR_ITEM_GET_AUTO;

		NETWORK2->PostInventoryEvent( (inventory->idx == table->idx) ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, mObject.index, table, "GET" );

		XCopyInventory( inventory, table );
		Inventory2sInventory( sinventory, table );
		rowCount++; sinventory++;
	}

	sendMsg->ErrorCode = error;
	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

	/// 파티에 send
	cParty* party = PARTYMAN->GetParty( mPlayerExrInfo.mPartyIndex );
	if ( party != NULL )
	{
		unsigned long* userArr    = party->GetUserArr( );
		unsigned int   partyCount = party->GetCount( );
		cPlayer*       partyUser;

		for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
		{
			if ( (*userArr) != mObject.index )
			{
				partyUser = OBJECTMANAGER->GetPlayer( (*userArr) );
				if ( partyUser != NULL )
				{
					/// 획득 아이템 데이터 담기
					MSG_SYN_PARTY_ITEMGET* partySend = (MSG_SYN_PARTY_ITEMGET*)NETWORK2->GetMsgRoot( &handle
																									,partyUser->GetConnectionIdx( )
																									,NM_PARTY
																									,NM_PARTY_ITEMGET_SYN );
					if ( partySend != NULL )
					{
						unsigned long  length    = sizeof(MSG_SYN_PARTY_ITEMGET) - sizeof(partySend->ItemIndex);

						short&         rowCount  = partySend->RowCount;
						unsigned long* itemIndex = partySend->ItemIndex;

						partySend->UserIndex = mObject.index;
						rowCount = 0;

						table = itemGet->table;
						for ( long i = 0; i < itemGet->rowCount; ++i, ++table )
						{
							(*itemIndex) = table->itemDefineIndex;
							rowCount++; itemIndex++;
						}

						length += (rowCount * sizeof(partySend->ItemIndex));
						NETWORK2->SendMsgRoot( handle, length );
					}
				}
			}
		}
	}

	// 유니크 습득 SYNC
	TB_INVENTORY* syncTable = itemGet->table;
	TB_INVENTORY* syncInven = NULL;

	sInventory inven;
	memset( &inven, 0, sizeof(inven) );

	for( long i = 0; i < itemGet->rowCount; i++, syncTable++ )
	{
		syncInven = SelectInventory( syncTable->number );
		if( IsInventory( syncInven ) )
		{
			TB_ITEM_DEFINE* define = ITEMMANAGER->GetItemDefine( syncInven->itemDefineIdx );
			if( define == NULL )
				continue;

			// 유니크
			if( define->rareLevel >= eTOOLTIP_UNIQUE && define->rareLevel <= eTOOLTIP_MAX )
			{
				Inventory2sInventory( &inven, syncInven );

				cSender* sender = g_gameSrv->GetSender( );
				sender->PostUniqueItemSync( NETWORK2->GetCID( ), GetName(), (char*)&inven, sizeof(inven) );
			}
		}
	}
}

// CbItemGetQuest Method.
void cPlayer::CbItemGetQuest(ULONG_PTR socketContext, ITEM_GET* itemGet, u_long requestResult)
{
	if ( requestResult == 0 )
	{
		HANDLE                  handle  = NULL;
		MSG_RES_ITEM_GET_QUEST* sendMsg = (MSG_RES_ITEM_GET_QUEST*)NETWORK2->GetMsgRoot( &handle
																						,(PerSocketContext*)socketContext
																						,NM_ITEM
																						,NM_ITEM_GET_QUEST_RES );
		BYTE&       rowCount   = sendMsg->RowCount;
		sInventory* sinventory = sendMsg->Inventory;

		TB_INVENTORY* table = itemGet->table;
		TB_INVENTORY* inventory;

		for ( long i = 0; i < itemGet->rowCount; i++, table++ )
		{
			inventory = SelectInventory( table->number );

			NETWORK2->PostInventoryEvent( (inventory->idx == table->idx) ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, mObject.index, table, "GET_QUEST" );

			XCopyInventory( inventory, table );
			Inventory2sInventory( sinventory, table );
			rowCount++; sinventory++;
		}

		sendMsg->ErrorCode = ERROR_ITEM_GET_QUEST_SUCCESS;
		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	else
		throw requestResult;
}

// CbItemPartyGive Method.
void cPlayer::CbItemPartyGive(ULONG_PTR socketContext, ITEM_PARTY_GIVE* itemPartyGive, u_long requestResult)
{
	if ( mInventoryDb.get )
		mInventoryDb.get = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;

    // 파티장에 결과 전송
    cPlayer* pGivePlayer = OBJECTMANAGER->GetPlayer( itemPartyGive->giveCharacterIdx );
    if ( pGivePlayer )
    {
        NETWORK2->SendMsgError( pGivePlayer->GetConnectionIdx(), NM_ITEM, NM_ITEM_PARTY_GIVE_RES, 0 );
    }

    // 아이템 지급 전송
    HANDLE                   handle  = NULL;
    MSG_SYN_ITEM_PARTY_GIVE* sendMsg = (MSG_SYN_ITEM_PARTY_GIVE*)NETWORK2->GetMsgRoot( &handle
                                                                                       ,(PerSocketContext*)socketContext
                                                                                       ,NM_ITEM
                                                                                       ,NM_ITEM_PARTY_GIVE_SYN );
	sInventory*     sinventory  = sendMsg->inventory;
	TB_INVENTORY*   table       = itemPartyGive->table;
	TB_INVENTORY*   inventory;

	for ( long i = 0; i < itemPartyGive->rowCount; i++, table++ )
	{
		inventory = SelectInventory( table->number );

		NETWORK2->PostInventoryEvent( (inventory->idx == table->idx) ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, 
                                      mObject.index, 
                                      table, 
                                      "PARTY-GIVE" );

		XCopyInventory( inventory, table );
		Inventory2sInventory( sinventory, table );
		sendMsg->rowCount++; sinventory++;
	}

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

	/// 파티에 send
	cParty* party = PARTYMAN->GetParty( mPlayerExrInfo.mPartyIndex );
	if ( party != NULL )
	{
		unsigned long* userArr    = party->GetUserArr( );
		unsigned int   partyCount = party->GetCount( );
		cPlayer*       partyUser;

		for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
		{
			if ( (*userArr) != mObject.index )
			{
				partyUser = OBJECTMANAGER->GetPlayer( (*userArr) );
				if ( partyUser != NULL )
				{
					/// 획득 아이템 데이터 담기
					MSG_SYN_PARTY_ITEMGET* partySend = (MSG_SYN_PARTY_ITEMGET*)NETWORK2->GetMsgRoot( &handle
																									,partyUser->GetConnectionIdx( )
																									,NM_PARTY
																									,NM_PARTY_ITEMGET_SYN );
					if ( partySend != NULL )
					{
						unsigned long  length = sizeof(MSG_SYN_PARTY_ITEMGET) - sizeof(partySend->ItemIndex);

						unsigned long* itemIndex = partySend->ItemIndex;
						partySend->UserIndex     = mObject.index;

						table = itemPartyGive->table;
						for ( long i = 0; i < itemPartyGive->rowCount; ++i, ++table )
						{
							(*itemIndex) = table->itemDefineIndex;
							itemIndex++;

                            partySend->RowCount++;
						}

						length += (partySend->RowCount * sizeof(partySend->ItemIndex));
						NETWORK2->SendMsgRoot( handle, length );
					}
				}
			}
		} // for
	}

	// 유니크 아이템 습득 SYNC
	TB_INVENTORY* syncTable = itemPartyGive->table;
	TB_INVENTORY* syncInven = NULL;

	sInventory inven;
	memset( &inven, 0, sizeof(inven) );

	for( long i = 0; i < itemPartyGive->rowCount; i++, syncTable++ )
	{
		syncInven = SelectInventory( syncTable->number );
		if( IsInventory( syncInven ) )
		{
			TB_ITEM_DEFINE* define = ITEMMANAGER->GetItemDefine( syncInven->itemDefineIdx );
			if( define == NULL )
				continue;

			// 유니크
			if( define->rareLevel >= eTOOLTIP_UNIQUE && define->rareLevel <= eTOOLTIP_MAX )
			{
				Inventory2sInventory( &inven, syncInven );

				cSender* sender = g_gameSrv->GetSender( );
				sender->PostUniqueItemSync( NETWORK2->GetCID( ), GetName(), (char*)&inven, sizeof(inven) );
			}
		}
	}
}

// CbItemUse Method.
void cPlayer::CbItemUse(ULONG_PTR socketContext, ITEM_USE* itemUse, u_long requestResult)
{
	if ( requestResult == 0 )
	{
		UNREFERENCED_PARAMETER(socketContext);
		UNREFERENCED_PARAMETER(itemUse);
	}
	else
		throw requestResult;
}

// CbItemSummonPost Method.
void cPlayer::CbItemSummonPost(ULONG_PTR socketContext, ITEM_SUMMON_POST* summonPost, u_long requestResult)
{
    if ( mInventoryDb.use )
		mInventoryDb.use = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 )   throw requestResult;


    TB_INVENTORY* inventory  = &(summonPost->inventory);
    if ( inventory->apply == InventoryApplyRemoved )
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, inventory, "SUMMON POST" );
    }
    else
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, inventory, "SUMMON POST" );
    }

    // 상태 변경
	ChangeState( eOBJECT_STATE_STOP );
    SetStateStop( eSTOP_SUMMON_PAPER );

    // 결과 전송.
    HANDLE                           handle  = NULL;
	MSG_RES_TRADE_POST_SUMMON_OPEN*  sendMsg = (MSG_RES_TRADE_POST_SUMMON_OPEN*)NETWORK2->GetMsgRoot( &handle
																									 ,(PerSocketContext*)socketContext
																									 ,NM_TRADE
																									 ,NM_TRADE_POST_SUMMON_OPEN_RES );
	sendMsg->ErrorCode = ERROR_TRADE_POST_SUMMON_OPEN_SUCCESS;
	Inventory2sInventory( &sendMsg->inventory, inventory );
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TRADE_POST_SUMMON_OPEN) );

    // 우편 리스트 전송
    SendPostSelect( socketContext, 
                    summonPost->startPage, 
                    summonPost->endPage, 
                    summonPost->noneRead, 
                    summonPost->post,
                    summonPost->postCount );


    // 다른 플레이어들에게도 알림
    MSG_SYN_TRADE_POST_SUMMON_OPEN synMsg;
    synMsg.Category     = NM_TRADE;
    synMsg.Protocol     = NM_TRADE_POST_SUMMON_OPEN_SYN;
    synMsg.characterIdx = mObject.index;
    NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
}

// CbItemSummonAgent Method.
void cPlayer::CbItemSummonAgent(ULONG_PTR socketContext, ITEM_SUMMON_AGENT* summonAgent, u_long requestResult)
{
    if ( mInventoryDb.use )
		mInventoryDb.use = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 )   throw requestResult;


    TB_INVENTORY* inventory  = &(summonAgent->inventory);
    if ( inventory->apply == InventoryApplyRemoved )
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, inventory, "SUMMON AGENT" );
    }
    else
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, inventory, "SUMMON AGENT" );
    }

    // 상태 변경
	ChangeState( eOBJECT_STATE_STOP );
    SetStateStop( eSTOP_SUMMON_PAPER );


    // 결과 전송.
    HANDLE                            handle  = NULL;
    MSG_RES_TRADE_AGENT_SUMMON_OPEN*  sendMsg = (MSG_RES_TRADE_AGENT_SUMMON_OPEN*)NETWORK2->GetMsgRoot( &handle
                                                                                                       ,(PerSocketContext*)socketContext
                                                                                                       ,NM_TRADE
                                                                                                       ,NM_TRADE_AGENT_SUMMON_OPEN_RES );
	sendMsg->ErrorCode = ERROR_TRADE_AGENT_SUMMON_OPEN_SUCCESS;

	// 소환 주문서
	Inventory2sInventory( &sendMsg->inventory, inventory );

	// 판매 대행
	ITEM_AGENT_RESULT_OWNER*    result      = summonAgent->agent;
	NPC_ITEM_AGENT_OPEN_RESULT* sendResult  = sendMsg->Agent;

	sendMsg->StartPage = summonAgent->startPage;
	sendMsg->EndPage   = summonAgent->endPage;
	sendMsg->RowCount  = summonAgent->rowCount;

	for ( long i = 0; i < summonAgent->rowCount; i++, result++, sendResult++ )
	{
		sendResult->Idx       = result->idx;
		sendResult->ValidThru = result->validThru;
		sendResult->Price     = result->price;
		Inventory2sInventory( &sendResult->Inventory, &result->table );
	}
	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );


    // 다른 플레이어들에게도 알림
    MSG_SYN_TRADE_AGENT_SUMMON_OPEN synMsg;
    synMsg.Category     = NM_TRADE;
    synMsg.Protocol     = NM_TRADE_AGENT_SUMMON_OPEN_SYN;
    synMsg.characterIdx = mObject.index;
    NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
}

// CbItemSummonWareHouse Method.
void cPlayer::CbItemSummonWareHouse(ULONG_PTR socketContext, ITEM_SUMMON_WAREHOUSE* summonWare, u_long requestResult)
{
   if ( mInventoryDb.use )
		mInventoryDb.use = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 )   throw requestResult;


    TB_INVENTORY* inventory  = &(summonWare->inventory);
    if ( inventory->apply == InventoryApplyRemoved )
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, inventory, "SUMMON WAREHOUSE" );
    }
    else
    {
        NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, inventory, "SUMMON WAREHOUSE" );
    }


	// 상태 변경
	ChangeState( eOBJECT_STATE_STOP );
	SetStateStop( eSTOP_SUMMON_PAPER );

	// 결과 전송.
    HANDLE                               handle  = NULL;
    MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN*  sendMsg = (MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN*)NETWORK2->GetMsgRoot( &handle
                                                                                                             ,(PerSocketContext*)socketContext
                                                                                                             ,NM_ITEM
                                                                                                             ,NM_ITEM_WAREHOUSE_SUMMON_OPEN_RES );

	sendMsg->ErrorCode = ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_SUCCESS;
	Inventory2sInventory( &sendMsg->inventory, inventory );
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN) );


	// 다른 플레이어들에게도 알림
	MSG_SYN_ITEM_WAREHOUSE_SUMMON_OPEN synMsg;
	synMsg.Category     = NM_ITEM;
	synMsg.Protocol     = NM_ITEM_WAREHOUSE_SUMMON_OPEN_SYN;
	synMsg.characterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
}



// CbItemObtEvent Method.
void cPlayer::CbItemObtEvent(ULONG_PTR socketContext, ITEM_OBT_EVENT* itemEvent, u_long requestResult)
{
	if ( mInventoryDb.cash )
		mInventoryDb.cash = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;

	// 로그 저장
	for ( int i = 0; i < itemEvent->rowCount; i++ )
	{
		TB_INVENTORY* table = &(itemEvent->table[i]);
		if ( table )
		{
			if ( table->apply == InventoryApplyRemoved )
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "OBT EVENT" );
			}
			else
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, table, "OBT EVENT" );
			}
		}
	}

    // 결과 전송.
    HANDLE                 handle  = NULL;
    MSG_RES_NPC_OBT_EVENT* sendMsg = (MSG_RES_NPC_OBT_EVENT*)NETWORK2->GetMsgRoot( &handle
                                                                                  ,(PerSocketContext*)socketContext
                                                                                  ,NM_NPC
                                                                                  ,NM_NPC_OBT_EVENT_RES );

	if ( itemEvent->retvalue == 0 )
	{
		sendMsg->ErrorCode = ERROR_NPC_OBT_EVENT_SUCCESS;

		// 이벤트 완료 설정
		HANDLE						handle     = NULL;
		MEMBER_OBT_EVENT_COMPLETE*  eventComplete = (MEMBER_OBT_EVENT_COMPLETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_OBT_EVENT_COMPLETE );

		eventComplete->memberIdx    = mConnectionIdx;
		eventComplete->cbtUser		= itemEvent->cbtUser;
		eventComplete->obtUser		= itemEvent->obtUser;
		eventComplete->fourGamer	= itemEvent->fourGamer;
		eventComplete->retvalue     = 0;

		if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(MEMBER_OBT_EVENT_COMPLETE), COMMON_DB_MEMBER_OBT_EVENT_COMPLETE ) == false )
		{
			NETWORK2->PostServerEvent("ERROR - COMMON_DB_MEMBER_OBT_EVENT_COMPLETE SendSQL FAIL [member:%d][cbtUser:%d,obtUser:%d,fourGamer:%d]", 
									   mConnectionIdx, itemEvent->cbtUser, itemEvent->obtUser, itemEvent->fourGamer);
		}
	}
	else
	{
		sendMsg->ErrorCode = ERROR_NPC_OBT_EVENT_FAIL;
	}
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_NPC_OBT_EVENT) );
}

void cPlayer::CbItemNpcEvent( ULONG_PTR socketContext, ITEM_NPC_EVENT* npcEvent, u_long requestResult )
{
	if ( mInventoryDb.cash )
		mInventoryDb.cash = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 ) throw requestResult;

	// 로그 저장
	for ( int i = 0; i < npcEvent->rowCount; i++ )
	{
		TB_INVENTORY* table = &(npcEvent->table[i]);
		if ( table )
		{
			if ( table->apply == InventoryApplyRemoved )
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "NPC EVENT" );
			}
			else
			{
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, table, "NPC EVENT" );
			}
		}
	}

    // 결과 전송.
    HANDLE						 handle  = NULL;
    MSG_RES_NPC_EVENT_ITEM_GIVE* sendMsg = (MSG_RES_NPC_EVENT_ITEM_GIVE*)NETWORK2->GetMsgRoot( &handle
																							,(PerSocketContext*)socketContext
																							,NM_NPC
																							,NM_NPC_EVENT_ITEM_GIVE_RES );

	sendMsg->ErrorCode = npcEvent->retvalue;
	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_NPC_EVENT_ITEM_GIVE) );
}

// CbStallSellGet Method.
void cPlayer::CbStallSellGet(ULONG_PTR socketContext, STALL_SELL_GET* stallSellGet, u_long requestResult)
{
	if ( mInventoryDb.stallSellGet )
		mInventoryDb.stallSellGet = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* inventory;

		unsigned long befor  = GetMoney( );
		long          result = AddMoney( stallSellGet->fObject, stallSellGet->tCharacterMoney, false );
		unsigned long after  = GetMoney( );

		// 로그 전송.
		NETWORK2->PostMoneyEvent( GetObjectID( )
								 ,befor
								 ,after
								 ,"STALL_SELL_GET(BUY)::Character(=%d):Money(=%d)"
								 ,stallSellGet->fObject.index
								 ,stallSellGet->tCharacterMoney );
		NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE
									 ,mObject.index
									 ,&stallSellGet->tInventory
									 ,"STALL SELL - BUY CHARACTER_IDX=%d INVENTORY_IDX=%d"
									 ,stallSellGet->fObject.index
									 ,stallSellGet->fInventory.idx );

		// 오류 검사.
		if ( result != stallSellGet->tCharacterMoney )
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_STALL_SELL_GET AddMoney(=%d) != ResultMoney(=%d).", stallSellGet->tCharacterMoney, result );

		// 결과 전송.
		HANDLE                       handle  = NULL;
		MSG_RES_ITEM_STALL_SELL_GET* sendMsg = (MSG_RES_ITEM_STALL_SELL_GET*)NETWORK2->GetMsgRoot( &handle
																								  ,(PerSocketContext*)socketContext
																								  ,NM_ITEM
																								  ,NM_ITEM_STALL_SELL_GET_RES );
		sendMsg->ErrorCode = ERROR_ITEM_STALL_SELL_GET_SUCCESS;

		inventory = XCopyInventory( &stallSellGet->tInventory );
		Inventory2sInventory( &sendMsg->Inventory, inventory );

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_STALL_SELL_GET) );
	}
	else
		throw requestResult;
}

// CbCharacterSkillreset Method.
void cPlayer::CbCharacterSkillreset(ULONG_PTR socketContext, CHARACTER_SKILL_RESET* characterSkillReset, u_long requestResult)
{
	if ( mInventoryDb.skillReset )
	{
		mInventoryDb.skillReset = false;	// DB; 사용종료.
		mIsDbUpdate = false;
	}
	else
		return;

	if ( requestResult == 0 )
	{
		ChangeState( eOBJECT_STATE_IDLE );
		SetStateStop( eSTOP_NONE );

		if ( characterSkillReset->mRetvalue == ERROR_SKILLRESET_SUCCESS )
		{
			/// 성공처리 & 메세지 발송
			SetSkillReset( characterSkillReset->mTable, characterSkillReset->mRowCount );

			/// 아이템 정보 갱신
			if ( characterSkillReset->mItem.idx != 0 )
			{
				HANDLE                   handle  = NULL;
				MSG_RES_ITEM_SKILLRESET* sendMsg = (MSG_RES_ITEM_SKILLRESET*)NETWORK2->GetMsgRoot( &handle
																								  ,(PerSocketContext*)socketContext
																								  ,NM_ITEM
																								  ,NM_ITEM_SKILLRESET_RES );
				sInventory* sinventory = &sendMsg->inventory;

				TB_INVENTORY* table = &characterSkillReset->mItem;
				TB_INVENTORY* inventory;

				if ( table->apply == InventoryApplyRemoved )
				{
					NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "SKILLRESET DELETE BY ITEM" );
					table->count = 0;	// 클라이언트 삭제 요청를 위해 0으로 설정.
				}
				else
				{
					inventory = SelectInventory( table->number );

					if ( inventory->idx == table->idx )
						NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "SKILLRESET DELETE BY ITEM" );
					else
						NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, table, "SKILLRESET DELETE BY ITEM" );

					inventory = XCopyInventory( inventory, table );
				}

				Inventory2sInventory( sinventory, table );
				sinventory++;

				NETWORK2->SendMsgRoot( handle, sizeof( MSG_RES_ITEM_SKILLRESET ) );
			}			

			NETWORK2->PostSkillEvent( mObject.index, true, 0, 0 );
		}
		else
		{
			/// 처리 실패 메세지 발송
			NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_PLAYER, NM_PLAYER_SKILLRESET_RES, characterSkillReset->mRetvalue );
			NETWORK2->PostServerEvent(" SQL_GAME_PROCESS_CHARACTER_SKILLRESET [%d]", characterSkillReset->mRetvalue );
		}

		// 캐릭터 이벤트.
		PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - Reset" );
	}
	else
		throw requestResult;
}

// CbCharacterThemeReset Method.
void cPlayer::CbCharacterThemeReset(ULONG_PTR socketContext, CHARACTER_THEME_RESET* characterThemeReset, u_long requestResult)
{
	if ( mInventoryDb.themeReset )
		mInventoryDb.themeReset = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult != 0 )
		throw requestResult;

	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	/// db결과 체크
	switch( characterThemeReset->mRetvalue )
	{
	case ERROR_SKILLRESET_SUCCESS:
		break;
	case 1:
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_THEMERESET_RES, ERROR_THEMERESET_READY );
		break;
	default:
		/// 처리 실패 메세지 발송
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_ITEM, NM_ITEM_THEMERESET_RES, ERROR_THEMERESET_DBERROR );
		NETWORK2->PostServerEvent("cPlayer::CbCharacterThemeReset [%d]", characterThemeReset->mRetvalue );
		return;
	}

	// 기존 테마 정보 삭제
	PerThemeUser* perThemeUser = (PerThemeUser*)mThemeUserRoot.pool;
	while( perThemeUser != NULL )
	{
		PerThemeUser* pDel = perThemeUser;
		perThemeUser = (PerThemeUser*)perThemeUser->next;
		THEMEUSERPOOL->ReleaseThemeUser( &mThemeUserRoot, pDel );
	}

	/// 테마 정보 기록
	THEME_USER*	pTable = characterThemeReset->mTable;
	AddThemeUser( pTable, characterThemeReset->mRowCount );
	
	/// 메세지
	HANDLE handle  = NULL;
	MSG_RES_ITEM_THEMERESET* sendMsg = (MSG_RES_ITEM_THEMERESET*)NETWORK2->GetMsgRoot( &handle ,(PerSocketContext*)socketContext
		,NM_ITEM, NM_ITEM_THEMERESET_RES );
	sInventory* sinventory = &sendMsg->mInventory;
	sendMsg->mThemeNum = characterThemeReset->mThemeIdx;
	sendMsg->mThemeMode = characterThemeReset->mThemeMode;

	/// 아이템 정보 갱신
	if ( characterThemeReset->mItem.idx != 0 )
	{
		TB_INVENTORY* table = &characterThemeReset->mItem;
		TB_INVENTORY* inventory;

		if ( table->apply == InventoryApplyRemoved )
		{
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE, mObject.index, table, "THEMERESET DELETE BY ITEM" );
			table->count = 0;	// 클라이언트 삭제 요청를 위해 0으로 설정.
		}
		else
		{
			inventory = SelectInventory( table->number );

			if ( inventory->idx == table->idx )
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, table, "THEMERESET DELETE BY ITEM" );
			else
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE, mObject.index, table, "THEMERESET DELETE BY ITEM" );

			inventory = XCopyInventory( inventory, table );
		}

		Inventory2sInventory( sinventory, table );
		sinventory++;

	}
	else
	{
		sinventory->number = characterThemeReset->mSlotNum;

		NETWORK2->PostServerEvent(" SQL_GAME_PROCESS_CHARACTER_SKILLRESET characterThemeReset->mItem.idx == 0[%d]", characterThemeReset->mRetvalue );
	}

	/// 메세지 발송
	NETWORK2->SendMsgRoot( handle, sizeof( MSG_RES_ITEM_THEMERESET ) );
}

void cPlayer::CbCharacterGMThemeReset( CHARACTER_GM_THEME_RESET* characterThemeReset )
{
	// 기존 테마 정보 삭제
	PerThemeUser* perThemeUser = (PerThemeUser*)mThemeUserRoot.pool;
	while( perThemeUser != NULL )
	{
		PerThemeUser* pDel = perThemeUser;
		perThemeUser = (PerThemeUser*)perThemeUser->next;
		THEMEUSERPOOL->ReleaseThemeUser( &mThemeUserRoot, pDel );
	}

	/// 테마 정보 기록
	THEME_USER*	pTable = characterThemeReset->mTable;
	AddThemeUser( pTable, characterThemeReset->mRowCount );
	
	/// 메세지
	HANDLE handle  = NULL;
	MSG_SYN_CHEAT_THEMERESET* sendMsg = (MSG_SYN_CHEAT_THEMERESET*)NETWORK2->GetMsgRoot( &handle ,mConnectionIdx, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_SYN );
	if( handle )
	{
		sendMsg->mThemeNum = characterThemeReset->mThemeIdx;
		sendMsg->mThemeMode = characterThemeReset->mThemeMode;

		/// 메세지 발송
		NETWORK2->SendMsgRoot( handle, sizeof( MSG_SYN_CHEAT_THEMERESET ) );
	}
}


// CbInventoryReward Method.
void cPlayer::CbInventoryReward( )
{
	mInventoryDb.reward = false; // DB; 사용종료.
}

// CbInventoryTake Method.
void cPlayer::CbInventoryTake( )
{
	mInventoryDb.take = false; // DB; 사용종료.
}

bool cPlayer::CbInventoryItemMix( )
{
	mInventoryDb.itemMix = false;
	return false;
}

// ItemGetOpen Method.
void cPlayer::ItemGetOpen(MSG_REQ_ITEM_GET_OPEN* msg)
{
	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_GET_OPEN_FAIL;

	if ( IsChangeState( ePLAYER_STATE_ITEMPICK ) == false )
		throw ERROR_ITEM_GET_OPEN_FAIL;

	if ( mItemGetOpen == true )
	{
		cItem* item = OBJECTMANAGER->GetItem( mItemGetIdx );
		if ( item != NULL )
		{
			if ( item->IsOpen( this ) )
				item->Close( );
		}
		ItemGetClose( );
	}

	cItem* item = OBJECTMANAGER->GetItem( msg->idx );

	if ( item == NULL )
		throw ERROR_ITEM_GET_OPEN_FAIL;

	item->Open( this );

	mItemGetOpen = true;
	mItemGetIdx  = item->GetObjectID( );

	if ( ChangeState( ePLAYER_STATE_ITEMPICK ) == false )
		throw ERROR_ITEM_GET_OPEN_FAIL;

	SendItemGetOpen( item );
}

// ItemGetClose Method.
void cPlayer::ItemGetClose(MSG_REQ_ITEM_GET_CLOSE* /*msg*/)
{
	if ( mItemGetOpen == false )
		throw ERROR_ITEM_GET_CLOSE_FAIL;

	cItem* item = OBJECTMANAGER->GetItem( mItemGetIdx );

	if ( item == NULL )
		throw ERROR_ITEM_GET_CLOSE_FAIL;

	if ( !item->IsOpen( this ) )
		throw ERROR_ITEM_GET_CLOSE_FAIL;

	if ( !item->Close( ) )
		throw ERROR_ITEM_GET_CLOSE_FAIL;

	ItemGetClose( );
}
void cPlayer::ItemGetClose( )
{
	mItemGetIdx  = 0;
	mItemGetOpen = false;

	if ( GetState( ) == ePLAYER_STATE_ITEMPICK )
		ChangeState( eOBJECT_STATE_IDLE );

	SendItemGetClose( );
}

// ItemGet Method.
void cPlayer::ItemGet(MSG_REQ_ITEM_GET* msg)
{ 
	if ( mInventoryDatabase )
		throw ERROR_ITEM_GET_FAIL;

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_GET_FAIL;

	if ( mItemGetOpen == false )
		throw ERROR_ITEM_GET_FAIL;

	cItem* item = OBJECTMANAGER->GetItem( mItemGetIdx );

	if ( item == NULL )
		throw ERROR_ITEM_GET_FAIL;

	if ( !item->IsOpen( this ) )
		throw ERROR_ITEM_GET_FAIL;

	BYTE       rowCount = 0;
	sItemData* itemData = item->GetItemData( rowCount );
	if ( rowCount > 0 && itemData != NULL )
	{
		HANDLE    handle  = NULL;
		ITEM_GET* itemGet = (ITEM_GET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_GET );
		u_long    length  = sizeof(ITEM_GET) - sizeof(itemGet->table);

		BYTE&      count    = (BYTE&)itemGet->rowCount;

		itemGet->characterIdx = mObject.index;
		for ( int i = 0; (i < MAX_ITEMS) && (rowCount > 0); i++, itemData++ )
		{
			if ( itemData->itemDefineIndex > 0 && itemData->count > 0 )
			{
				bool getBegin = false;
				bool getEnd   = false;
				if ( msg->itemIndexOrInd == ITEM_GET_ALL )
				{
					getBegin = true;
				}
				else if ( msg->itemIndexOrInd == itemData->itemDefineIndex )
				{
					getBegin = true;
					getEnd   = true;
				}
				if ( getBegin == true )
				{
					TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemData->itemDefineIndex );
					TB_INVENTORY*   inventory;
					short           remainCount = itemData->count;
					short           shortCount;
					short           number;

                    if( IsMaxInventory( itemDefine, remainCount ) == true )
                        continue;

					// 아이템 줍기. 겹치기 검사 및 처리.
					if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
					{
						do {
							inventory   = SelectInventory( number );
							shortCount  = UpdateInventory( inventory, remainCount );
							remainCount = remainCount - shortCount;

							itemGet->table[ count ].idx             = inventory->idx;
							itemGet->table[ count ].itemDefineIdx   = itemDefine->idx;
							itemGet->table[ count ].itemDefineIndex = itemDefine->index;
							itemGet->table[ count ].number          = number;
							itemGet->table[ count ].count           = shortCount;
							count++;

							item->PopItem( itemDefine->index, shortCount );	// 아이템 차감.
						} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
					}
					// 아이템 줍기. 생성 검사 및 등록.
					if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
					{
						do {
							CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
							remainCount = remainCount - shortCount;

							itemGet->table[ count ].idx             = 0;
							itemGet->table[ count ].itemDefineIdx   = itemDefine->idx;
							itemGet->table[ count ].itemDefineIndex = itemDefine->index;
							itemGet->table[ count ].number          = number;
							itemGet->table[ count ].count           = shortCount;
							itemGet->table[ count ].seal            = itemDefine->licenseType;
							count++;

							// 아이템 차감.
							item->PopItem( itemDefine->index, shortCount );
						} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
					}
					rowCount--;
				}
				if ( getEnd == true )
					break; // for ( ... )
			}
		}
		if ( itemGet->rowCount > 0 )
		{
			length += (itemGet->rowCount * sizeof(itemGet->table));
			mInventoryDb.get = NETWORK2->SendSQL( mConnectionIdx, handle, length );
			if ( mInventoryDb.get == false )
			{
				NETWORK2->ReleaseSQL( handle, length );
				throw ERROR_ITEM_GET_FAIL;
			}
			// 습득후 남은 아이템 동기화.
			SendItemGetLeave( item );
		}
		else
		{
			NETWORK2->ReleaseSQL( handle, length );
			throw ERROR_ITEM_GET_FAIL;
		}
	}
	else
		throw ERROR_ITEM_GET_FAIL;
}

// ItemPartyGive Method.
int cPlayer::ItemPartyGive(MSG_REQ_ITEM_PARTY_GIVE* msg)
{ 
	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_PARTY_GIVE_FAIL;

	if ( mItemGetOpen == false )
		throw ERROR_ITEM_PARTY_GIVE_FAIL;

    // 파티장 검사
    cParty* party = PARTYMAN->GetParty( mPlayerExrInfo.mPartyIndex );
    if ( party == NULL )
		throw ERROR_ITEM_PARTY_GIVE_FAIL;

    if ( party->GetDivideType( ) != PARTY_DIVIDE_LEADER )
        throw ERROR_ITEM_PARTY_GIVE_TYPE;

    // 지급할 캐릭터
    cPlayer* pTarget = OBJECTMANAGER->GetPlayer( msg->characterIdx );
    if ( pTarget == NULL )
        throw ERROR_ITEM_PARTY_GIVE_TARGET;

    if ( pTarget->GetStateDie() == true )
		throw ERROR_ITEM_PARTY_GIVE_FAIL;

    if ( pTarget->InventoryDatabase() )
        throw ERROR_ITEM_PARTY_GIVE_FAIL;

    if ( GetMapNumber() != pTarget->GetMapNumber() )
        throw ERROR_ITEM_PARTY_GIVE_MAP;

    if ( party->IsUser( msg->characterIdx ) == false )
        throw ERROR_ITEM_PARTY_GIVE_PARTY;


    // 아이템
	cItem* item = OBJECTMANAGER->GetItem( mItemGetIdx );
	if ( item == NULL )
		throw ERROR_ITEM_PARTY_GIVE_TYPE;

	if ( !item->IsOpen( this ) )
		throw ERROR_ITEM_PARTY_GIVE_OWNER;

    if ( item->IsParty( pTarget ) == false )
        throw ERROR_ITEM_PARTY_GIVE_PARTY;

    sItemData* itemData = item->GetItemData( msg->itemDefineIndex );
    if ( itemData == NULL )
        throw ERROR_ITEM_PARTY_GIVE_FAIL;

    if ( !(itemData->count > 0) )
        throw ERROR_ITEM_PARTY_GIVE_FAIL;

    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemData->itemDefineIndex );
    if ( itemDefine == NULL )
        throw ERROR_ITEM_PARTY_GIVE_FAIL;

    // 지급
    int ret = pTarget->ItemPartyTake( itemDefine, itemData->count, mPlayerInfo.CharacterIdx );
    if ( ret != 0 )
        throw ret;

    item->PopItem( itemDefine->index, itemData->count );	// 아이템 차감.

    // 습득후 남은 아이템 동기화.
	SendItemGetLeave( item );

    throw ERROR_ITEM_PARTY_GIVE_SUCCESS;
}

// ItemPartyTake Method.
int cPlayer::ItemPartyTake( TB_ITEM_DEFINE* takeItem, short count, unsigned long giveCharacterIdx )
{ 
    if( IsMaxInventory( takeItem, count ) == true )
        return ERROR_ITEM_PARTY_GIVE_MAX_ITEM;

    float weight = (float)( mItemWeight + (takeItem->weight * count) );
    if( mStatus2.mMaxWeight <= weight )
        return ERROR_ITEM_PARTY_GIVE_WEIGHT;

    short           remainCount = count;
    TB_INVENTORY*   inventory   = NULL;
    short           shortCount  = 0;
    short           rowCount    = 0;
    short           number      = 0;

    HANDLE           handle         = NULL;
    ITEM_PARTY_GIVE* itemPartyGive  = (ITEM_PARTY_GIVE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_GET_PARTY_GIVE );
    u_long           length         = sizeof(ITEM_PARTY_GIVE) - sizeof(itemPartyGive->table);

    itemPartyGive->giveCharacterIdx = giveCharacterIdx;
    itemPartyGive->takeCharacterIdx = mPlayerInfo.CharacterIdx;

    // 아이템 줍기. 겹치기 검사 및 처리.
    if ( FindFirstInventory( takeItem, &number, remainCount, &shortCount, true ) == true )
    {
        do {
            inventory   = SelectInventory( number );
            shortCount  = UpdateInventory( inventory, remainCount );
            remainCount = remainCount - shortCount;

            itemPartyGive->table[ rowCount ].idx             = inventory->idx;
            itemPartyGive->table[ rowCount ].itemDefineIdx   = takeItem->idx;
            itemPartyGive->table[ rowCount ].itemDefineIndex = takeItem->index;
            itemPartyGive->table[ rowCount ].number          = number;
            itemPartyGive->table[ rowCount ].count           = shortCount;
            rowCount++;

        } while ( FindNextInventory( takeItem, &(++number), remainCount, &shortCount, true ) == true );
    }
    // 아이템 줍기. 생성 검사 및 등록.
    if ( FindFirstInventory( takeItem, &number, remainCount, &shortCount, false ) == true )
    {
        do {
            CreateInventory( takeItem, number, remainCount, &(shortCount=0) );
            remainCount = remainCount - shortCount;

            itemPartyGive->table[ rowCount ].idx             = 0;
            itemPartyGive->table[ rowCount ].itemDefineIdx   = takeItem->idx;
            itemPartyGive->table[ rowCount ].itemDefineIndex = takeItem->index;
            itemPartyGive->table[ rowCount ].number          = number;
            itemPartyGive->table[ rowCount ].count           = shortCount;
            itemPartyGive->table[ rowCount ].seal            = takeItem->licenseType;
            rowCount++;

        } while ( FindNextInventory( takeItem, &(++number), remainCount, &shortCount, false ) == true );
    }

    if ( !(rowCount > 0) )
        return ERROR_ITEM_PARTY_GIVE_INVEN_FULL;

    itemPartyGive->rowCount = rowCount;

	length += (itemPartyGive->rowCount * sizeof(itemPartyGive->table));
    mInventoryDb.get = NETWORK2->SendSQL( mConnectionIdx, handle, length );
    if ( mInventoryDb.get == false ) return ERROR_ITEM_PARTY_GIVE_FAIL;

    return ERROR_ITEM_PARTY_GIVE_SUCCESS;
}

// ItemGetAutoPush Method.
unsigned short cPlayer::ItemGetAutoPush(unsigned long itemDefineIndex, unsigned short count)
{
	if ( mItemGetCount < MAX_ITEMS )
	{
		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
		if ( itemDefine != NULL )
		{
			if ( count > itemDefine->capacity )
				count = itemDefine->capacity;

			mItemGetData[ mItemGetCount ].itemDefineIndex = itemDefineIndex;
			mItemGetData[ mItemGetCount ].count           = count;
			mItemGetCount +=1;
			return count;
		}
	}
	return 0;
}

// ItemGetAutoEnd Method.
bool cPlayer::ItemGetAutoEnd(bool apply)
{
	sItemData* itemGetData  = mItemGetData;
	int        itemGetCount = mItemGetCount;

	// 자동습득 수량 정리.
	mItemGetCount = 0;

	if ( apply == true )
	{
		if ( mItemGetOpen == true )
		{
			cItem* item = OBJECTMANAGER->GetItem( mItemGetIdx );
			if ( item != NULL )
			{
				if ( item->IsOpen( this ) )
					item->Close( );
			}
			ItemGetClose( );
		}

		unsigned itemIdx = ITEMMANAGER->MakeItemIdx( );
		cItem*   item    = OBJECTMANAGER->AddItem( itemIdx, this, true );

		if ( item != NULL )
		{
			item->PushOwner( mObject.index );

			for ( int i = 0; i < itemGetCount; ++i, ++itemGetData )
			{
				unsigned short remainCount = itemGetData->count;
				unsigned short shortCount  = 0;

				while ( remainCount > 0 )
				{
					shortCount  = item->PushItem( itemGetData->itemDefineIndex, remainCount );
					remainCount = ( shortCount > 0 ) ? (remainCount - shortCount) : 0;
				}
			}

			item->Open( this );

			mItemGetOpen = true;
			mItemGetIdx  = itemIdx;

			if ( ChangeState( ePLAYER_STATE_ITEMPICK ) == false )
			{
				OBJECTMANAGER->RemoveItem( itemIdx );
				return false;
			}

			return SendItemGatheringOpen( item );
		}
	}
	return false;
}

// ItemGetQuest Method.
bool cPlayer::ItemGetQuest(unsigned long itemDefineIndex, unsigned short count)
{
	TB_ITEM_DEFINE* itemDefine  = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
	short           remainCount = count;

	if ( itemDefine != NULL && count > 0 )
	{
		TB_INVENTORY* inventory;
		short         shortCount;
		short         number;

		// 아이템 퀘스트 자동습득. 겹치기 검사 및 처리.
		if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
		{
			do {
				inventory   = SelectInventory( number );
				shortCount  = UpdateInventory( inventory, remainCount );
				remainCount = remainCount - shortCount;

				PerItemGet* itemGet = ITEMGETPOOL->SearchItemGet( &mItemGetRoot, number );

				if ( itemGet == NULL )
				{
					itemGet = ITEMGETPOOL->GetItemGet( &mItemGetRoot, number );
					itemGet->inventory = (*inventory);
					itemGet->inventory.count = 0;
				}

				itemGet->inventory.count = itemGet->inventory.count + shortCount;

				// 겹치기 오류.
				if ( remainCount > 0 )
				{
					NETWORK2->PostServerEvent( "Error - cPlayer::ItemGetQuest(=%d, =%d, =%d )", itemDefineIndex, remainCount, shortCount );
					remainCount = 0;
				}
			} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
		}
		// 아이템 퀘스트 자동습득. 생성 검사 및 등록.
		if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
		{
			do {
				CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
				remainCount = remainCount - shortCount;

				PerItemGet* itemGet = ITEMGETPOOL->SearchItemGet( &mItemGetRoot, number );

				if ( itemGet == NULL )
				{
					itemGet = ITEMGETPOOL->GetItemGet( &mItemGetRoot, number );

					itemGet->inventory.idx             = 0;
					itemGet->inventory.itemDefineIdx   = itemDefine->idx;
					itemGet->inventory.itemDefineIndex = itemDefine->index;
					itemGet->inventory.number          = number;
					itemGet->inventory.seal            = itemDefine->licenseType;
				}

				itemGet->inventory.count = itemGet->inventory.count + shortCount;

				// 생성 오류.
				if ( remainCount > 0 )
				{
					NETWORK2->PostServerEvent( "Error - cPlayer::ItemGetQuest(=%d, =%d, =%d )", itemDefineIndex, remainCount, shortCount );
					remainCount = 0;
				}
			} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
		}

		return true;
	}
	return false;
}

// SendItemGetOpen Method.
bool cPlayer::SendItemGetOpen(cBaseObject* baseObject)
{
	HANDLE                 handle   = NULL;
	MSG_RES_ITEM_GET_OPEN* sendMsg  = (MSG_RES_ITEM_GET_OPEN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_GET_OPEN_RES );
	bool                   retvalue = false;
	if ( sendMsg != NULL )
	{
		cItem*     item     = (cItem*)baseObject;
		sItemData* itemData = sendMsg->itemData;

		sendMsg->ErrorCode      = ERROR_ITEM_GET_OPEN_SUCCESS;
		sendMsg->rowCount       = item->GetItemData( itemData );
        sendMsg->openItemIdx    = mItemGetIdx;

		retvalue = NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}

	MSG_SYN_ITEM_GET_OPEN sendSyn;

	sendSyn.Category     = NM_ITEM;
	sendSyn.Protocol     = NM_ITEM_GET_OPEN_SYN;
	sendSyn.characterIdx = mObject.index;

	NETWORK2->QuickSendExcept( this, (char*)&sendSyn, sizeof(MSG_SYN_ITEM_GET_OPEN) );
	return retvalue;
}

// SendItemGetClose Method.
bool cPlayer::SendItemGetClose( )
{
	bool retvalue = NETWORK2->SendMsgError( mConnectionIdx, NM_ITEM, NM_ITEM_GET_CLOSE_RES, ERROR_ITEM_GET_CLOSE_SUCCESS );

	MSG_SYN_ITEM_GET_CLOSE sendSyn;

	sendSyn.Category     = NM_ITEM;
	sendSyn.Protocol     = NM_ITEM_GET_CLOSE_SYN;
	sendSyn.characterIdx = mObject.index;

	NETWORK2->QuickSendExcept( this, (char*)&sendSyn, sizeof(MSG_SYN_ITEM_GET_CLOSE) );
	return retvalue;
}

// SendItemGetLeave Method.
bool cPlayer::SendItemGetLeave(cBaseObject* baseObject)
{
	HANDLE                  handle   = NULL;
	MSG_RES_ITEM_GET_LEAVE* sendMsg  = (MSG_RES_ITEM_GET_LEAVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_GET_LEAVE_RES );
	if ( sendMsg != NULL )
	{
		cItem*     item     = (cItem*)baseObject;
		sItemData* itemData = sendMsg->itemData;

		sendMsg->rowCount = item->GetItemData( itemData );

		return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	return false;
}

// SendItemGatheringOpen Method.
bool cPlayer::SendItemGatheringOpen(cBaseObject* baseObject)
{
	HANDLE                       handle   = NULL;
	MSG_RES_ITEM_GATHERING_OPEN* sendMsg  = (MSG_RES_ITEM_GATHERING_OPEN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_GATHERING_OPEN_RES );
	if ( sendMsg != NULL )
	{
		cItem*     item     = (cItem*)baseObject;
		sItemData* itemData = sendMsg->itemData;

		sendMsg->ErrorCode = ERROR_ITEM_GET_OPEN_SUCCESS;
		sendMsg->rowCount  = item->GetItemData( itemData );

		return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	return false;
}

// ExchangeAsk Method.
void cPlayer::ExchangeAsk(MSG_REQ_ITEM_EXCHANGE_ASK* msg)
{
	if ( GetCheatHideMode() == true )
		throw ERROR_ITEM_EXCHANGE_ASK_ERROR;					// 캐릭터 상태오류 - hero 가 hide on.

	// 이미 거래중 확인.
	if ( mExchangeStatus != ItemExchangeNone )
		throw ERROR_ITEM_EXCHANGE_ASK_ERROR;

	if ( msg->mTarget.type != eOBJECTTYPE_PLAYER )
		throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		throw ERROR_ITEM_EXCHANGE_ASK_ERROR;

	cPlayer* player = GRIDMANAGER->GetPlayer( msg->mTarget.index );
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

	sGameOption* toOption1 = GetOptionData();
	if( toOption1->option1.rejectionTrade == true )
		throw ERROR_ITEM_EXCHANGE_ASK_OPTIONREFUSE;

	// 맵 위치 검사.
	if ( mMapNumber != player->GetMapNumber( ) )
		throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

	// 거리 검사.
	float tempRange = OBJECTMANAGER->ObjectSizeRange( this, player, ITEM_VALID_DISTANCE + SYNC_MOVE_RANGE );
	if ( (mObjectPos - player->GetPos( )).Length( ) > tempRange )
		throw ERROR_ITEM_EXCHANGE_ASK_DISTANCE;

	// 거래 응답 요구.
	player->ExchangeAsk( mObject );

	// 거래중 처리와 대상 저장.
	mExchangeStatus = ItemExchangeAsk;
	mExchangeTarget = player->GetObject( );
	mExchangeItems  = 0;
	mExchangeMoney  = 0;

	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_ASK_RES, ERROR_ITEM_EXCHANGE_ASK_SUCCESS );
}
void cPlayer::ExchangeAsk(sObject object)
{
	if ( IsRequestRejection( ) == true )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETERROR;					// 캐릭터 상태오류.

	if ( GetStateDie( ) == true )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETERROR;					// 캐릭터 상태오류.

	if( IsChangeState( eOBJECT_STATE_MOVE ) == false )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETERROR;

	if ( GetCheatHideMode() == true )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETERROR;					// 캐릭터 상태오류.

	sGameOption* toOption1 = GetOptionData();
	if( toOption1->option1.rejectionTrade == true )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETOPTIONREFUSE;

	// 이미 거래중 확인.
	if ( mExchangeStatus != ItemExchangeNone )
		throw ERROR_ITEM_EXCHANGE_ASK_TARGETERROR;

	StartRequestRejection( eREQREJCT_EXCHANGE );	// 캐릭터 상태변경.

	// 거래중 처리와 대상 저장.
	mExchangeStatus = ItemExchangeAsk;
	mExchangeTarget = object;
	mExchangeItems  = 0;
	mExchangeMoney  = 0;

	HANDLE handle = NULL;
	MSG_RES_ITEM_EXCHANGE_ASK* sendMsg = (MSG_RES_ITEM_EXCHANGE_ASK*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_EXCHANGE_ASK_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode    = ERROR_ITEM_EXCHANGE_ASK_REPLAY;
		sendMsg->characterIdx = object.index;

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_EXCHANGE_ASK) );
	}
}

// ExchangeReply Method - 승락이 아닐경우, 모두 거절처리.
void cPlayer::ExchangeReply(MSG_REQ_ITEM_EXCHANGE_REP* msg)
{
	// 내 상태 체크
	if( GetRequestRejection() != eREQREJCT_EXCHANGE )
		throw ERROR_ITEM_EXCHANGE_REP_ERROR;				// 캐릭터 상태 오류.

	EndRequestRejection( eREQREJCT_EXCHANGE );				// 캐릭터 상태 변경.

	// 신청자가 없으면 자신의 상태 푼다
	cPlayer* player = OBJECTMANAGER->GetPlayer( mExchangeTarget.index );
	if( player == NULL )
	{
		if ( mExchangeStatus == ItemExchangeAsk )
			mExchangeStatus = ItemExchangeNone;

		throw ERROR_ITEM_EXCHANGE_REP_NOTEXIST;
	}
	else
	{
		// 존재하되, 맵이동중이면 자신과 상대방의 상태를 푼다
		if( player->IsMapChange() == true )
		{
			player->ExchangeReplySyn( TYPE_ITEM_EXCHANGE_REP_REJECT, ERROR_ITEM_EXCHANGE_REP_TARGETERROR );

			if ( mExchangeStatus == ItemExchangeAsk )
				mExchangeStatus = ItemExchangeNone;

			throw ERROR_ITEM_EXCHANGE_REP_NOTEXIST;
		}
	}

	//
	if( msg->type == TYPE_ITEM_EXCHANGE_REP_ACCEPT )
	{
		// 거래 상태 체크
		try
		{
			if ( GetCheatHideMode() == true )
				throw ERROR_ITEM_EXCHANGE_REP_ERROR;				// 캐릭터 상태오류 - hero 가 hide on

			if ( GetStateDie( ) == true )
				throw ERROR_ITEM_EXCHANGE_REP_ERROR;				// 캐릭터 상태오류.

			if( GetState() == eOBJECT_STATE_STOP )
				throw ERROR_ITEM_EXCHANGE_REP_FAIL;

			sGameOption* myOption1 = GetOptionData();
			if( myOption1->option1.rejectionTrade == true )
				throw ERROR_ITEM_EXCHANGE_REP_OPTIONREFUSE;			// 내가 거부상태임
			
			// 이미 거래중 확인.
			if ( mExchangeStatus != ItemExchangeAsk )
				throw ERROR_ITEM_EXCHANGE_REP_ERROR;

			// 거래 신청자 상태 체크
			if( player->GetState() == eOBJECT_STATE_STOP || player->GetStateDie() )
				throw ERROR_ITEM_EXCHANGE_REP_TARGETERROR;
		
			sGameOption* toOption1 = player->GetOptionData();
			if( toOption1->option1.rejectionTrade == true )
				throw ERROR_ITEM_EXCHANGE_REP_TARGETOPTIONREFUSE;

			if ( mMapNumber != player->GetMapNumber( ) )
				throw ERROR_ITEM_EXCHANGE_REP_DISTANCE;

			// 거리 검사.
			float tempRange = OBJECTMANAGER->ObjectSizeRange( this, player, ITEM_VALID_DISTANCE + SYNC_MOVE_RANGE );
			if ( (mObjectPos - player->GetPos( )).Length( ) > tempRange )
				throw ERROR_ITEM_EXCHANGE_REP_DISTANCE;
		}
		catch ( int error )
		{
			// 오류가 나면, 내 상태와 상대방상태를 모두 풀어준다
			player = OBJECTMANAGER->GetPlayer( mExchangeTarget.index );	// 요청자
			if ( player != NULL )
			{
				player->ExchangeReplySyn( TYPE_ITEM_EXCHANGE_REP_REJECT, error );
			}

			if ( mExchangeStatus == ItemExchangeAsk )
				mExchangeStatus = ItemExchangeNone;

			throw error;
		}
		
		player->ExchangeReplySyn( msg->type, ERROR_ITEM_EXCHANGE_REP_SUCCESS );
		ExchangeReply( msg->type );
	}
	else if( msg->type == TYPE_ITEM_EXCHANGE_REP_REJECTTIME )
	{
		player->ExchangeReplySyn( msg->type, ERROR_ITEM_EXCHANGE_REP_TIME );
		ExchangeReply( msg->type );
	}
	else 
	{
		player->ExchangeReplySyn( msg->type, ERROR_ITEM_EXCHANGE_REP_TARGETREFUSE );
		ExchangeReply( msg->type );
	}
}

// 자기자신에게 보냄
void cPlayer::ExchangeReply( BYTE type )
{
	if ( type == TYPE_ITEM_EXCHANGE_REP_ACCEPT )
	{
		mExchangeStatus = ItemExchangeBegin;
		SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_REP_RES, ERROR_ITEM_EXCHANGE_REP_SUCCESS );
	}
	else if( type == TYPE_ITEM_EXCHANGE_REP_REJECTTIME )
	{
		mExchangeStatus = ItemExchangeNone;
		SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_REP_RES, ERROR_ITEM_EXCHANGE_REP_TIME );
	}
	else
	{
		mExchangeStatus = ItemExchangeNone;
		SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_REP_RES, ERROR_ITEM_EXCHANGE_REP_REFUSE );		
	}
}

// 거래요청자에게 보냄
void cPlayer::ExchangeReplySyn( BYTE type, int error )
{
	if ( type == TYPE_ITEM_EXCHANGE_REP_ACCEPT )
	{
		mExchangeStatus = ItemExchangeBegin;
	}
	else
	{
		mExchangeStatus = ItemExchangeNone;
	}

	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_REP_SYN, error );
}

// ExchangeAdd Method.
void cPlayer::ExchangeAdd(MSG_REQ_ITEM_EXCHANGE_ADD* msg)
{
	if ( mExchangeStatus != ItemExchangeBegin )
		throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );

	// 심각한 오류이므로, 반드시 오류처리 해야됨.
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

	switch ( msg->type )
	{
	case TYPE_ITEM_EXCHANGE_ADD_ITEM:
		{
			if ( !(mExchangeItems < MAX_ITEM_EXCHANGE) )
				throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

            if( IsInventoryBag( msg->number ) == false )
                throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

			TB_INVENTORY* inventory = SelectInventory( msg->number );

			if ( IsInventory( inventory ) == false )
				throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

			if ( inventory->apply != InventoryApplyNone )
				throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );

			if ( itemDefine == NULL )
				throw ERROR_ITEM_EXCHANGE_ADD_FAIL;

            if ( IsItemExchange( itemDefine, inventory ) == false )
				throw ERROR_ITEM_EXCHANGE_ADD_EXCHANGE;

			inventory->apply = InventoryApplyExchange;
			mExchangeItems++;

			player->ExchangeAdd( inventory );

			SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_ADD_RES, ERROR_ITEM_EXCHANGE_ADD_SUCCESS );
		}
		break;
	case TYPE_ITEM_EXCHANGE_ADD_MONEY:
		{
			if ( msg->money == 0 || msg->money > mMoney || msg->money > MAX_TRADE_MONEY )
				throw ERROR_ITEM_EXCHANGE_ADD_TRADE_MONEY;

			mExchangeMoney = msg->money;

			player->ExchangeAdd( mExchangeMoney );

			SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_ADD_RES, ERROR_ITEM_EXCHANGE_ADD_SUCCESS );
		}
		break;
	default:
		throw ERROR_ITEM_EXCHANGE_ADD_FAIL;
	}
}
void cPlayer::ExchangeAdd(TB_INVENTORY* inventory)
{
	HANDLE                     handle  = NULL;
	MSG_SYN_ITEM_EXCHANGE_ADD* sendMsg = (MSG_SYN_ITEM_EXCHANGE_ADD*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_EXCHANGE_ADD_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->type = TYPE_ITEM_EXCHANGE_ADD_ITEM;
		Inventory2sInventory( &sendMsg->inventory, inventory );

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_EXCHANGE_ADD) );
	}
}
void cPlayer::ExchangeAdd(unsigned long money)
{
	HANDLE                     handle  = NULL;
	MSG_SYN_ITEM_EXCHANGE_ADD* sendMsg = (MSG_SYN_ITEM_EXCHANGE_ADD*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_EXCHANGE_ADD_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->type  = TYPE_ITEM_EXCHANGE_ADD_MONEY;
		sendMsg->money = money;

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_EXCHANGE_ADD) );
	}
}

// ExchangeDel Method.
void cPlayer::ExchangeDel(MSG_REQ_ITEM_EXCHANGE_DEL* msg)
{
	if ( mExchangeStatus != ItemExchangeBegin )
		throw ERROR_ITEM_EXCHANGE_DEL_FAIL;

	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );

	// 심각한 오류이므로, 반드시 오류처리 해야됨.
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_DEL_FAIL;

	if ( !(mExchangeItems > 0) )
		throw ERROR_ITEM_EXCHANGE_DEL_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_EXCHANGE_DEL_FAIL;

	if ( inventory->apply != InventoryApplyExchange )
		throw ERROR_ITEM_EXCHANGE_DEL_FAIL;

	inventory->apply = InventoryApplyNone;
	mExchangeItems--;

	player->ExchangeDel( inventory );

	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_DEL_RES, ERROR_ITEM_EXCHANGE_DEL_SUCCESS );
}
void cPlayer::ExchangeDel(TB_INVENTORY* inventory)
{
	HANDLE                     handle  = NULL;
	MSG_SYN_ITEM_EXCHANGE_DEL* sendMsg = (MSG_SYN_ITEM_EXCHANGE_DEL*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_EXCHANGE_DEL_SYN );
	if ( sendMsg != NULL )
	{
		Inventory2sInventory( &sendMsg->inventory, inventory );
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_EXCHANGE_DEL) );
	}
}

// ExchangeOk Method.
void cPlayer::ExchangeOk(MSG_REQ_ITEM_EXCHANGE_OK* /*msg*/)
{
	if ( mExchangeStatus != ItemExchangeBegin )
		throw ERROR_ITEM_EXCHANGE_OK_FAIL;

	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_OK_FAIL;

	mExchangeStatus = ItemExchangeOk;

	player->SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_OK_RES, ERROR_ITEM_EXCHANGE_OK_PLAYER );

	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_OK_RES, ERROR_ITEM_EXCHANGE_OK_SUCCESS );
}

// ExchangeRetry Method.
void cPlayer::ExchangeRetry(MSG_REQ_ITEM_EXCHANGE_RETRY* /*msg*/)
{
	if ( mExchangeStatus != ItemExchangeOk )
		throw ERROR_ITEM_EXCHANGE_RETRY_FAIL;

	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_RETRY_FAIL;

	mExchangeStatus = ItemExchangeBegin;

	player->ExchangeRetry(  );
	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_RETRY_RES, ERROR_ITEM_EXCHANGE_RETRY_SUCCESS );
}

// ExchangeRetry Method.
void cPlayer::ExchangeRetry( )
{
	// 상대방이 상태가 변경되면 같이 변경 처리
	mExchangeStatus = ItemExchangeBegin;

	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_RETRY_RES, ERROR_ITEM_EXCHANGE_RETRY_PLAYER );
}

// ExchangeCancel Method.
void cPlayer::ExchangeCancel(MSG_REQ_ITEM_EXCHANGE_CANCEL* /*msg*/)
{
	if ( mExchangeStatus == ItemExchangeNone )
		throw ERROR_ITEM_EXCHANGE_CANCEL_FAIL;
	
	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );
	if ( player != NULL )
	{
		player->SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, ERROR_ITEM_EXCHANGE_CANCEL_PLAYER );
		player->ExchangeClear( );
	}

	ExchangeClear( );
	SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, ERROR_ITEM_EXCHANGE_CANCEL_SUCCESS );
}

void cPlayer::ExchangeCancel( bool sendme )
{
	if( mExchangeStatus > ItemExchangeAsk )
	{
		cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );

		if ( player != NULL )
		{
			player->SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, ERROR_ITEM_EXCHANGE_CANCEL_PLAYER );
			player->ExchangeClear( );
		}

		ExchangeClear( );

		// 자기자신에게 보낸다
		if( sendme == true )
		{
			SendMsgResExchange( NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, ERROR_ITEM_EXCHANGE_CANCEL_SUCCESS );
		}
	}
}

void cPlayer::ExchangeClear( )
{
	// 거래 상  태 초기화.
	mExchangeStatus = ItemExchangeNone;

	// 거래 아이템 초기화.
	TB_INVENTORY* inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( int i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd && mExchangeItems > 0; i++, inventory++ )
	{
		if ( inventory->apply == InventoryApplyExchange )
		{
			inventory->apply = InventoryApplyNone;
			mExchangeItems--;
		}
	}

	// 거래 금  액 초기화.
	mExchangeMoney = 0;

	mExchangeTarget.index = 0;
	mExchangeTarget.type = eOBJECTTYPE_NONE;
}

// ExchangeEnd Method.
void cPlayer::ExchangeEnd(MSG_REQ_ITEM_EXCHANGE_END* /*msg*/)
{
	if ( mInventoryDatabase )
		throw ERROR_ITEM_EXCHANGE_END_FAIL;

	if ( mExchangeStatus != ItemExchangeOk )
		throw ERROR_ITEM_EXCHANGE_END_FAIL;

	cPlayer* player = GRIDMANAGER->GetPlayer( mExchangeTarget.index );
	if ( player == NULL )
		throw ERROR_ITEM_EXCHANGE_END_FAIL;

	if ( player->InventoryDatabase( ) )
		throw ERROR_ITEM_EXCHANGE_END_FAIL;

	if ( player->GetExchangeStatus( ) < ItemExchangeOk )
		throw ERROR_ITEM_EXCHANGE_END_FAIL;

    sHeroInfo* pTargetInfo = player->GetHeroInfo();
    if( pTargetInfo == NULL )
        throw ERROR_ITEM_EXCHANGE_END_FAIL;
	
	// 상대방의 보유 금액이 한도 체크
    unsigned long resultMoney = player->GetMoney() - player->GetExchangeMoney() + GetExchangeMoney();
	if( resultMoney > MAX_HAVE_MONEY) 
		throw ERROR_ITEM_EXCHANGE_END_THATMONEY_FAIL;

	// 나의 보유 금액이 한도 체크
    resultMoney = GetMoney() - GetExchangeMoney() + player->GetExchangeMoney();
	if( resultMoney > MAX_HAVE_MONEY )
		throw ERROR_ITEM_EXCHANGE_END_MYMONEY_FAIL;


	// 교환 수량검사.
	TB_INVENTORY* inventory;
	int           exchangeCount;
	int           remainCount;

	inventory     = SelectInventory( INVENTORY_BAG_BEGIN );
	exchangeCount = 0;
	remainCount   = 0;
	for ( int i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd; i++, inventory++ )
	{
		if ( IsInventory( inventory ) == true )
		{
			if ( inventory->apply == InventoryApplyExchange )
				exchangeCount++;
		}
		else
			remainCount++;
	}

	inventory = player->SelectInventory( INVENTORY_BAG_BEGIN );
    for ( int i = INVENTORY_BAG_BEGIN; i <= pTargetInfo->BagEnd; i++, inventory++ )
	{
		if ( IsInventory( inventory ) == true )
		{
			if ( inventory->apply == InventoryApplyExchange )
				remainCount--;
		}
		else
			exchangeCount--;
	}

	if ( exchangeCount > 0 || remainCount < 0 )
		throw ERROR_ITEM_EXCHANGE_END_INVENTORY;

    // (내가 올린 아이템)상대방의 최대 보유갯수
	inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( int i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd; i++, inventory++ )
	{
		if ( inventory->apply == InventoryApplyExchange )
        {
            TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
            if( player->IsMaxInventory( itemDefine, inventory->count ) == true )
                throw ERROR_ITEM_EXCHANGE_END_THAT_MAX_ITEM;
        }
	}

    // (상대방이 올린 아이템)나의 최대 보유갯수
	inventory = player->SelectInventory( INVENTORY_BAG_BEGIN );
	for ( int i = INVENTORY_BAG_BEGIN; i <= pTargetInfo->BagEnd; i++, inventory++ )
	{
		if ( inventory->apply == InventoryApplyExchange )
        {
            TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
            if( IsMaxInventory( itemDefine, inventory->count ) == true )
                throw ERROR_ITEM_EXCHANGE_END_MY_MAX_ITEM;
        }
	}

	mExchangeStatus = ItemExchangeEnd;

	if ( player->GetExchangeStatus( ) != ItemExchangeEnd )
		throw ERROR_ITEM_EXCHANGE_END_WAIT;

	// Database 처리요청.
	HANDLE         handle       = NULL;
	ITEM_EXCHANGE* itemExchange = (ITEM_EXCHANGE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_EXCHANGE );
	TB_INVENTORY*  table;

	// 적용 행 수
	itemExchange->rowCount = 2;

	// Player1 - 인벤토리 이동(캐릭터 고유번호).
	itemExchange->table[ 0 ].characterIdx   = mObject.index;
	itemExchange->table[ 0 ].characterMoney = player->GetExchangeMoney( ) - GetExchangeMoney( );
	itemExchange->table[ 0 ].characterFrom  = player->GetObjectID( );
	itemExchange->table[ 0 ].characterBefor = mMoney;
	itemExchange->table[ 0 ].characterAfter = mMoney + itemExchange->table[ 0 ].characterMoney;

	inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	table     = itemExchange->table[ 1 ].table;
	for ( int i = INVENTORY_BAG_BEGIN; i <= mHeroInfo.BagEnd && itemExchange->table[ 1 ].rowCount < MAX_ITEM_EXCHANGE; i++, inventory++ )
	{
		if ( inventory->apply == InventoryApplyExchange )
		{
			(*table) = (*inventory);
			RemoveInventory( inventory );

			itemExchange->table[ 1 ].rowCount++;
			table++;
		}
	}

	// Player2 - 인벤토리 이동(캐릭터 고유번호).
	itemExchange->table[ 1 ].characterIdx   = player->GetObjectID( );
	itemExchange->table[ 1 ].characterMoney = GetExchangeMoney( ) - player->GetExchangeMoney( );
	itemExchange->table[ 1 ].characterFrom  = mObject.index;
	itemExchange->table[ 1 ].characterBefor = player->GetMoney( );
	itemExchange->table[ 1 ].characterAfter = player->GetMoney( ) + itemExchange->table[ 1 ].characterMoney;

	inventory = player->SelectInventory( INVENTORY_BAG_BEGIN );
	table     = itemExchange->table[ 0 ].table;
	for ( int i = INVENTORY_BAG_BEGIN; i <= pTargetInfo->BagEnd && itemExchange->table[ 0 ].rowCount < MAX_ITEM_EXCHANGE; i++, inventory++ )
	{
		if ( inventory->apply == InventoryApplyExchange )
		{
			(*table) = (*inventory);
			player->RemoveInventory( inventory );

			itemExchange->table[ 0 ].rowCount++;
			table++;
		}
	}

	// Player1 - 인벤토리 번호정리(슬롯).
	table     = itemExchange->table[ 0 ].table;
	inventory = SelectInventory( INVENTORY_BAG_BEGIN );
	for ( short i = INVENTORY_BAG_BEGIN, rowCount = 0; i <= mHeroInfo.BagEnd && rowCount < itemExchange->table[ 0 ].rowCount; i++, inventory++ )
	{
		if ( IsInventory( inventory ) == false )
		{
			table->number = i;
			XCopyInventory( table );

			table++; rowCount++;
		}
	}

	if ( GetExchangeMoney( ) > 0 )
	{
		long exchangeMoney = -GetExchangeMoney( );
		long result = AddMoney( player->GetObject( ), exchangeMoney, false );
		if ( result != exchangeMoney )
		{
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_ITEM_EXCHANGE From(=%d) To(=%d) AddMoney(=%d) != ResultMoney(=%d).", GetObjectID( ), GetObjectID( ), exchangeMoney, result );
		}
	}
	if ( player->GetExchangeMoney( ) > 0 )
	{
		long exchangeMoney = +player->GetExchangeMoney( );
		long result = AddMoney( player->GetObject( ), exchangeMoney, false );
		if ( result != exchangeMoney )
		{
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_ITEM_EXCHANGE From(=%d) To(=%d) AddMoney(=%d) != ResultMoney(=%d).", player->GetObjectID( ), GetObjectID( ), exchangeMoney, result );
		}
	}

	// Player2 - 인벤토리 번호정리(슬롯).
	table     = itemExchange->table[ 1 ].table;
	inventory = player->SelectInventory( INVENTORY_BAG_BEGIN );
	for ( short i = INVENTORY_BAG_BEGIN, rowCount = 0; i <= pTargetInfo->BagEnd && rowCount < itemExchange->table[ 1 ].rowCount; i++, inventory++ )
	{
		if ( IsInventory( inventory ) == false )
		{
			table->number = i;
			player->XCopyInventory( table );

			table++; rowCount++;
		}
	}
	if ( player->GetExchangeMoney( ) > 0 )
	{
		long exchangeMoney = -player->GetExchangeMoney( );
		long result = player->AddMoney( GetObject( ), exchangeMoney, false );
		if ( result != exchangeMoney )
		{
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_ITEM_EXCHANGE From(=%d) To(=%d) AddMoney(=%d) != ResultMoney(=%d).", player->GetObjectID( ), player->GetObjectID( ), exchangeMoney, result );
		}
	}
	if ( GetExchangeMoney( ) > 0 )
	{
		long exchangeMoney = +GetExchangeMoney( );
		long result = player->AddMoney( GetObject( ), exchangeMoney, false );
		if ( result != exchangeMoney )
		{
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_ITEM_EXCHANGE From(=%d) To(=%d) AddMoney(=%d) != ResultMoney(=%d).", GetObjectID( ), player->GetObjectID( ), exchangeMoney, result );
		}
	}

	if ( NETWORK2->SendSQL( handle, sizeof(ITEM_EXCHANGE) ) == false )
	{
		NETWORK2->CloseCID( player->GetConnectionIdx( ) );		
		NETWORK2->CloseCID( mConnectionIdx );
		throw ERROR_ITEM_EXCHANGE_END_FAIL;
	}

	// Begin DB
	player->ExchangeEnd( );
	ExchangeEnd( );
}
void cPlayer::CbExchangeEnd(TB_INVENTORY* table, unsigned long rowCount, u_long requestResult)
{
	if ( mInventoryDb.exchange )
		mInventoryDb.exchange = false;	// DB; 사용종료.
	else
		return;

	if ( requestResult == 0 )
	{
		TB_INVENTORY* inventory;

		SendMsgResExchangeEnd( table, rowCount );

		while ( rowCount > 0 )
		{
			inventory = SelectInventory( table->number );

			if ( inventory->idx == table->idx )
			{
				table->apply = InventoryApplyNone;
				XCopyInventory( inventory, table );
			}
			else
				throw table;

			table++;
			rowCount--;
		}

		// 거래종료 & 초기화.
		mExchangeStatus = ItemExchangeNone;
		mExchangeItems  = 0;
		mExchangeMoney  = 0;

		mExchangeTarget.index = 0;
		mExchangeTarget.type = eOBJECTTYPE_NONE;
	}
	else
		throw requestResult;
}

// SendMsgResExchange Method.
bool cPlayer::SendMsgResExchange(char category, char protocol, int error)
{
	return NETWORK2->SendMsgError( mConnectionIdx, category, protocol, error );
}

// SendMsgResExchangeEnd Method.
bool cPlayer::SendMsgResExchangeEnd(TB_INVENTORY* table, unsigned long rowCount)
{
	HANDLE                     handle  = NULL;
	MSG_RES_ITEM_EXCHANGE_END* sendMsg = (MSG_RES_ITEM_EXCHANGE_END*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_ITEM, NM_ITEM_EXCHANGE_END_RES );
	if ( sendMsg != NULL )
	{
		sInventory* sinventory = sendMsg->Inventory;

		// 거래아이템
		while ( sendMsg->RowCount < rowCount )
		{
			Inventory2sInventory( sinventory, table );
			sinventory++; table++;
			sendMsg->RowCount++;
		}

		sendMsg->ErrorCode = ERROR_ITEM_EXCHANGE_END_SUCCESS;
		return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	return false;
}

// StallSellOpen Method.
void cPlayer::StallSellOpen(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_OPEN* msg)
{
	// 1-1. 캐릭터 상태 검사.
	if ( IsChangeState( eOBJECT_STATE_STOP ) == false )
		throw ERROR_ITEM_STALL_SELL_OPEN_FAIL;			// 캐릭터 상태 오류 - 변경불가.

	if ( GetStateStop( ) != eSTOP_NONE )
		throw ERROR_ITEM_STALL_SELL_OPEN_FAIL;			// 캐릭터 상태 오류.

	if ( IsRequestRejection( ) == true )
		throw ERROR_ITEM_STALL_SELL_OPEN_FAIL;			// 캐릭터 상태오류.

	// 1-2. 노점 상태 검사.
	if ( mStallSell == true )
		throw ERROR_ITEM_STALL_SELL_OPEN_EXIST;			// 노  점 상태 오류 - 이미 오픈중.

	// 2-1. 캐릭터 상태 변경.
	ChangeState( eOBJECT_STATE_STOP );					// 캐릭터 상태 - 정지.
	SetStateStop( eSTOP_OPENSTALL );					// 정지중 상태 - 노점오픈.

	// 2-2. 노점 판매자 열기.
	mStallSell = true;									// 노  점 상태 - 오픈.

	// 2-3. 노점명 등록.
	msg->title[ STALL_TITLE_LEN ] = 0;
	wcscpy( mStallSellTitle, msg->title );

	// 3-1. 노점열기 성공.
	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_OPEN_RES, ERROR_ITEM_STALL_SELL_OPEN_SUCCESS );

	// 3-2. 노점열기 동기화.
	MSG_SYN_ITEM_STALL_SELL_OPEN sendMsg;
	memset( &sendMsg, 0, sizeof(MSG_SYN_ITEM_STALL_SELL_OPEN) );

	sendMsg.Category     = NM_ITEM;
	sendMsg.Protocol     = NM_ITEM_STALL_SELL_OPEN_SYN;
	sendMsg.characterIdx = mObject.index;
	wcscpy( sendMsg.title, mStallSellTitle );

	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_ITEM_STALL_SELL_OPEN) );
}

// StallSellClose Method.
void cPlayer::StallSellClose(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_CLOSE* /*msg*/)
{
	// 1-1. 캐릭터 상태 검사.
	if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_ITEM_STALL_SELL_CLOSE_FAIL;			// 캐릭터 상태 오류 - 변경불가.

	if ( GetStateStop( ) != eSTOP_OPENSTALL )
		throw ERROR_ITEM_STALL_SELL_CLOSE_FAIL;			// 캐릭터 상태 오류.

	// 1-2. 노점 상태 검사.
	if ( mStallSell != true )
		throw ERROR_ITEM_STALL_SELL_CLOSE_FAIL;

	// 2-1. 캐릭터 상태 변경.
	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	// 2-2. 노점 판매자 닫기.
	ReleaseStallSellAllGuests( );						// 게스트(GUEST)닫기.
	ClearStallSellItem( );								// 노  점 아이템 정리.

	// 3-1. 노점닫기 성공.
	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_CLOSE_RES, ERROR_ITEM_STALL_SELL_CLOSE_SUCCESS );

	// 3-2. 노점닫기 동기화.
	MSG_SYN_ITEM_STALL_SELL_CLOSE sendMsg;
	memset( &sendMsg, 0, sizeof(MSG_SYN_ITEM_STALL_SELL_CLOSE) );

	sendMsg.Category     = NM_ITEM;
	sendMsg.Protocol     = NM_ITEM_STALL_SELL_CLOSE_SYN;
	sendMsg.characterIdx = mObject.index;

	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_ITEM_STALL_SELL_CLOSE) );
}

// StallSellOpenDie Method.
void cPlayer::StallSellOpenDie( )
{
	ReleaseStallSellAllGuests( );   // 게스트 정리.
	ClearStallSellItem( );          // 아이템 정리.
}

// StallSellOpenShutdown Method.
void cPlayer::StallSellOpenShutdown(bool /*shutdown*/)
{
	ReleaseStallSellAllGuests( );   // 게스트 정리.
	ClearStallSellItem( );		    // 아이템 정리.

	ChangeState( eOBJECT_STATE_STOP );
	SetStateStop( eSTOP_GAMEFINISH );
}

// StallSellUseDie Method.
void cPlayer::StallSellUseDie( )
{
	cPlayer* player = GRIDMANAGER->GetPlayer( mStallSellOwner.index );
	if ( player != NULL )
		player->ReleaseStallSellGuest( mObject.index );
}

// StallSellUseShutdown Method.
void cPlayer::StallSellUseShutdown(bool /*shutdown*/)
{
	cPlayer* player = GRIDMANAGER->GetPlayer( mStallSellOwner.index );

	if ( player != NULL )
		player->ReleaseStallSellGuest( mObject.index );

	ChangeState( eOBJECT_STATE_STOP );
	SetStateStop( eSTOP_GAMEFINISH );
}

// GetStallSellItem Method - 생성.
StallSellItem* cPlayer::GetStallSellItem(TB_INVENTORY* inventory, long price)
{
	if ( !(mStallSellItemOffset < MAX_STALL_ITEM) )
		return NULL;

	StallSellItem* stallSellItem = mStallSellItems;
	for ( int i = 0; i < MAX_STALL_ITEM; i++, stallSellItem++ )
	{
		if ( stallSellItem->status == StallSellItemNone )
		{
			stallSellItem->status    = StallSellItemStart;	// 노점 아이템 판매시작.
			stallSellItem->inventory = inventory;			// 인벤토리 포인터 참조.
			stallSellItem->price     = price;				// 노점 아이템가격 설정.

			mStallSellItemOffset++;
			return stallSellItem;
		}
	}

	return NULL;
}

// GetStallSellItem Method - 검색.
StallSellItem* cPlayer::GetStallSellItem(TB_INVENTORY* inventory)
{
	StallSellItem* stallSellItem = mStallSellItems;
	for ( int i = 0; i < MAX_STALL_ITEM; i++, stallSellItem++ )
	{
		if ( stallSellItem->inventory == inventory )
			return stallSellItem;
	}
	return NULL;
}

// ReleaseStallSellItem Method - 해제.
bool cPlayer::ReleaseStallSellItem(StallSellItem* stallSellItem)
{
	if ( !(mStallSellItemOffset > 0) )
		return false;

	stallSellItem->status    = StallSellItemNone;	// 노점 아이템 판매중지.
	stallSellItem->inventory = NULL;				// 인벤토리 포인터 초기.
	stallSellItem->price     = 0;					// 노점 아이템 가격초기.

	mStallSellItemOffset--;

	// 노점 아이템 정렬 - 등록 기준으로 내림차순.
	qsort( (void*)mStallSellItems, MAX_STALL_ITEM, sizeof(StallSellItem), StallSellItemCompare );
	return true;
}

// ClearStallSellItem Method.
void cPlayer::ClearStallSellItem( )
{
	StallSellItem* stallSellItem = mStallSellItems;
	for ( int i = 0; i < MAX_STALL_ITEM && mStallSellItemOffset > 0; i++, stallSellItem++ )
	{
		if ( stallSellItem->status != StallSellItemNone )
		{
			stallSellItem->inventory->apply = InventoryApplyNone;

			stallSellItem->status    = StallSellItemNone;
			stallSellItem->inventory = NULL;
			stallSellItem->price     = 0;

			mStallSellItemOffset--;
		}
	}
	mStallSell = false;	// 노점종료.
}

// GetStallSellGuest Method
bool cPlayer::GetStallSellGuest(unsigned long characterIdx)
{
	if ( !(mStallSellGuestOffset < MAX_STALL_GUEST) )
		return false;

	unsigned long* stallSellGuest = mStallSellGuests;
	for ( int i = 0; i < MAX_STALL_GUEST; i++, stallSellGuest++ )
	{
		if ( (*stallSellGuest) == 0 )
		{
			(*stallSellGuest) = characterIdx;
			mStallSellGuestOffset++;
			return true;
		}
	}

	return false;
}

// ReleaseStallSellGuest Method
bool cPlayer::ReleaseStallSellGuest(unsigned long characterIdx)
{
	unsigned long* stallSellGuest = mStallSellGuests;
	for ( int i = 0; i < MAX_STALL_GUEST && mStallSellGuestOffset > 0; i++, stallSellGuest++ )
	{
		if ( (*stallSellGuest) == characterIdx )
		{
			(*stallSellGuest) = 0;
			mStallSellGuestOffset--;
			return true;
		}
	}
	return false;
}

// ReleaseStallSellAllGuests Method - 게스트(GUEST) 닫기.
void cPlayer::ReleaseStallSellAllGuests( )
{
	unsigned long* stallSellGuest = mStallSellGuests;
	for ( int i = 0; i < MAX_STALL_GUEST && mStallSellGuestOffset > 0; i++, stallSellGuest++ )
	{
		if ( (*stallSellGuest) != 0 )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( (*stallSellGuest) );

			if ( player != NULL )
			{
				if ( player->GetState( ) == eOBJECT_STATE_STOP && player->GetStateStop( ) == eSTOP_USESTALL )
				{
					player->ChangeState( eOBJECT_STATE_IDLE );
					player->SetStateStop( eSTOP_NONE );

					player->StallSellLeave( ERROR_ITEM_STALL_SELL_LEAVE_CLOSED );
				}
			}

			(*stallSellGuest) = 0;
			mStallSellGuestOffset--;
		}
	}
}

// StallSellRename Method.
void cPlayer::StallSellRename(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_RENAME* msg)
{
	// 이미 오픈중 검사
	if ( mStallSell == false )
		throw ERROR_ITEM_STALL_SELL_RENAME_FAIL;

	// 노점명 변경.
	msg->title[ STALL_TITLE_LEN ] = 0;
	wcscpy( mStallSellTitle, msg->title );

	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_RENAME_RES, ERROR_ITEM_STALL_SELL_RENAME_SUCCESS );

	MSG_SYN_ITEM_STALL_SELL_RENAME sendMsg;
	memset( &sendMsg, 0, sizeof(MSG_SYN_ITEM_STALL_SELL_RENAME) );

	sendMsg.Category     = NM_ITEM;
	sendMsg.Protocol     = NM_ITEM_STALL_SELL_RENAME_SYN;
	sendMsg.characterIdx = mObject.index;
	wcscpy( sendMsg.title, mStallSellTitle );

	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_ITEM_STALL_SELL_RENAME) );
}

// StallSellAdd Method - 목록에 아이템 등록후 DB 동기화 요청 - 데이터 처리후 메시지 전송.
void cPlayer::StallSellAdd(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_ADD* msg)
{
	// 노점 상태검사.
	if ( mStallSell == false )
		throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

    if ( IsInventoryBag( msg->number ) == false )
        throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

	StallSellItem* stallSellItem = NULL;
	TB_INVENTORY*  inventory     = SelectInventory( msg->number );

    if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

    TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx ) ;
    if ( IsItemExchange( itemDefine, inventory ) == false )
        throw ERROR_ITEM_STALL_SELL_ADD_EXCHANGE;

    __int64 totalPrice = (__int64)msg->price * (__int64)inventory->count;
    if( totalPrice > MAX_TRADE_MONEY )
		throw ERROR_ITEM_STALL_SELL_ADD_TRADE_MONEY;

	// 데이터 처리.
	if ( inventory->apply == InventoryApplyNone )
	{
		stallSellItem = GetStallSellItem( inventory, msg->price );	// 아이템 등록.

		if ( stallSellItem == NULL )
			throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

		inventory->apply = InventoryApplyStallSell;					// 인벤토리 APPLY 상태변경.
	}
	else if ( inventory->apply == InventoryApplyStallSell )
	{
		stallSellItem = GetStallSellItem( inventory );				// 아이템 갱신.

		if ( stallSellItem == NULL )
			throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

		if ( stallSellItem->status != StallSellItemPause )
			throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

		stallSellItem->status = StallSellItemStart;
		stallSellItem->price  = msg->price;
	}
	else
		throw ERROR_ITEM_STALL_SELL_ADD_FAIL;

	// 메시지 전송.
	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_ADD_RES, ERROR_ITEM_STALL_SELL_ADD_SUCCESS );
	SendMsgSynStallSell( NM_ITEM, NM_ITEM_STALL_SELL_ADD_SYN, stallSellItem->inventory, stallSellItem->price );
}

// StallSellDel Method - 목록중 아이템 삭제후 DB동기화 요청 - 메시지 전송후 데이터 처리.
void cPlayer::StallSellDel(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_DEL* msg)
{
	// 노점 상태검사.
	if ( mStallSell == false )
		throw ERROR_ITEM_STALL_SELL_DEL_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false || inventory->apply != InventoryApplyStallSell )
		throw ERROR_ITEM_STALL_SELL_DEL_FAIL;

	// 노점 아이템 검색.
	StallSellItem* stallSellItem = GetStallSellItem( inventory );

	if ( stallSellItem == NULL )
		throw ERROR_ITEM_STALL_SELL_DEL_FAIL;

	if ( stallSellItem->status == StallSellItemNone )
		throw ERROR_ITEM_STALL_SELL_DEL_FAIL;

	// 메시지 전송.
	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_DEL_RES, ERROR_ITEM_STALL_SELL_DEL_SUCCESS );
	SendMsgSynStallSell( NM_ITEM, NM_ITEM_STALL_SELL_DEL_SYN, stallSellItem->inventory, stallSellItem->price );

	// 데이터 처리.
	inventory->apply = InventoryApplyNone;			// 인벤토리 APPLY 상태변경.

	// 노점 아이템 삭제
	ReleaseStallSellItem( stallSellItem );
}

// StallSellMod Method - 상태만 변경.
void cPlayer::StallSellMod(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_MOD* msg)
{
	// 노점 상태검사.
	if ( mStallSell == false )
		throw ERROR_ITEM_STALL_SELL_MOD_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false || inventory->apply != InventoryApplyStallSell )
		throw ERROR_ITEM_STALL_SELL_MOD_FAIL;

	if ( !(mStallSellItemOffset > 0) )
		throw ERROR_ITEM_STALL_SELL_MOD_FAIL;

	// 노점 아이템 검색.
	StallSellItem* stallSellItem = GetStallSellItem( inventory );

	if ( stallSellItem == NULL )
		throw ERROR_ITEM_STALL_SELL_MOD_FAIL;

	if ( stallSellItem->status == StallSellItemNone )
		throw ERROR_ITEM_STALL_SELL_MOD_FAIL;

	stallSellItem->status = StallSellItemPause;	// 노점 아이템 판매 일시중지.

	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_MOD_RES, ERROR_ITEM_STALL_SELL_MOD_SUCCESS );
	SendMsgSynStallSell( NM_ITEM, NM_ITEM_STALL_SELL_MOD_SYN, stallSellItem->inventory, stallSellItem->price );
}

// StallSellJoin Method.
void cPlayer::StallSellJoin(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_JOIN* msg)
{
	if ( IsChangeState( eOBJECT_STATE_STOP ) == false )
		throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;			// 캐릭터 상태 오류 - 변경불가.

	if ( GetStateStop( ) != eSTOP_NONE )
		throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;			// 캐릭터 상태 오류.

	cPlayer* player = GRIDMANAGER->GetPlayer( msg->mTarget.index );

	if ( player == NULL )
		throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;

	player->StallSellJoin( this );

	mStallSellOwner = player->GetObject( );

	ChangeState( eOBJECT_STATE_STOP );
	SetStateStop( eSTOP_USESTALL );
}
void cPlayer::StallSellJoin(cPlayer* player)
{
	// 노점 상태검사.
	if ( mStallSell == false )
		throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;

	if ( GetStallSellGuest( player->GetObjectID( ) ) == false )
		throw ERROR_ITEM_STALL_SELL_JOIN_OVER;

	// 노점 아이템목록 전송.
	HANDLE                        handle  = NULL;
	MSG_RES_ITEM_STALL_SELL_JOIN* sendMsg = (MSG_RES_ITEM_STALL_SELL_JOIN*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_ITEM, NM_ITEM_STALL_SELL_JOIN_RES );
	if ( sendMsg != NULL )
	{
		StallSellItem*  stallSellItem = mStallSellItems;
		sStallSellData* stallSellData = sendMsg->stallSellData;

		sendMsg->ErrorCode = ERROR_ITEM_STALL_SELL_JOIN_SUCCESS;	// 성공.
		wcscpy( sendMsg->title, mStallSellTitle );					// 타이틀 복사.

		for ( unsigned long i = 0; i < mStallSellItemOffset; i++, stallSellData++, stallSellItem++ )
		{
			stallSellData->status = stallSellItem->status;
			Inventory2sInventory( &stallSellData->inventory, stallSellItem->inventory );
			stallSellData->price  = stallSellItem->price;
			sendMsg->rowCount++;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
}

// StallSellLeave Method.
void cPlayer::StallSellLeave(ULONG_PTR /*socketContext*/, MSG_REQ_ITEM_STALL_SELL_LEAVE* /*msg*/)
{
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_USESTALL )
		throw ERROR_ITEM_STALL_SELL_LEAVE_FAIL;			// 캐릭터 상태 오류.

	if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_ITEM_STALL_SELL_LEAVE_FAIL;			// 캐릭터 상태 오류 - 변경불가.

	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	cPlayer* player = GRIDMANAGER->GetPlayer( mStallSellOwner.index );

	if ( player == NULL )
		throw ERROR_ITEM_STALL_SELL_LEAVE_FAIL;

	if ( player->StallSellLeave( this ) == false )
		throw ERROR_ITEM_STALL_SELL_LEAVE_FAIL;

	StallSellLeave( ERROR_ITEM_STALL_SELL_LEAVE_SUCCESS );
}
bool cPlayer::StallSellLeave(cPlayer* player)
{
	return ( mStallSell == true ) ? ReleaseStallSellGuest( player->GetObjectID( ) ) : false;
}
void cPlayer::StallSellLeave(int error)
{
	SendMsgResStallSell( NM_ITEM, NM_ITEM_STALL_SELL_LEAVE_RES, error );
}

// StallSellGet Method.
void cPlayer::StallSellGet(ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_GET* msg)
{
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_USESTALL )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;			// 캐릭터 상태 오류.

	cPlayer* player = OBJECTMANAGER->GetPlayer( mStallSellOwner.index );

	if ( player == NULL )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	if ( player->IsStallSellOpen( ) == false )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

    if ( player->IsInventoryBag( msg->number ) == false )
        throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	TB_INVENTORY* inventory = player->SelectInventory( msg->number );

	if ( IsInventory( inventory ) == false )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	if ( inventory->apply != InventoryApplyStallSell )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	if ( msg->count > inventory->count )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	StallSellItem* stallSellItem = player->GetStallSellItem( inventory );

	if ( stallSellItem == NULL )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	if ( stallSellItem->status != StallSellItemStart )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	if ( mMoney < (stallSellItem->price * msg->count) )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	short inventoryNumber = GetEmptyBagNumber( );

	if ( inventoryNumber == INVENTORY_BAG_NONE )
		throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
    if( IsMaxInventory( itemDefine, msg->count ) == true )
        throw ERROR_ITEM_STALL_SELL_LEAVE_MAX_ITEM;

    // 보유가능 금액
	long totalPrice = (long)(stallSellItem->price * msg->count);
    if( player->IsAddMoney( totalPrice ) == false )
        throw ERROR_ITEM_STALL_SELL_LEAVE_HAVE_MONEY;


	// 구매시작 - 구매자 후 응답.
	HANDLE          handle       = NULL;
	STALL_SELL_GET* stallSellGet = (STALL_SELL_GET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_STALL_SELL_GET );

	stallSellGet->fObject           = player->GetObject( );
	stallSellGet->fCharacterMoney   = (+totalPrice);
	stallSellGet->fInventory        = *inventory;

	stallSellGet->tObject           = GetObject( );
	stallSellGet->tCharacterMoney   = (-totalPrice);
	stallSellGet->tInventory        = *inventory;

	stallSellGet->tInventory.idx    = 0;
	stallSellGet->tInventory.number = inventoryNumber;
	stallSellGet->tInventory.count  = msg->count;
	stallSellGet->tInventory.apply  = InventoryApplyNone;

	mInventoryDb.stallSellGet = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(STALL_SELL_GET) );
	if ( mInventoryDb.stallSellGet == false ) throw ERROR_ITEM_STALL_SELL_GET_FAIL;

	// 구매시작 - 판매자 선 응답.
	{
		// 판매자 선 응답 - 판매금액.
		unsigned long befor  = player->GetMoney( );
		long          result = player->AddMoney( GetObject( ), totalPrice, false );
		unsigned long after  = player->GetMoney( );

		NETWORK2->PostMoneyEvent( player->GetObjectID( ), befor, after, "STALL_SELL_GET(SELL)::Character(=%d):Money(=%d)", GetObjectID( ), totalPrice );

		if ( result != totalPrice )
			NETWORK2->PostServerEvent( "Error - SQL_GAME_PROCESS_STALL_SELL_GET CharacterIdx(=%d), AddMoney(=%d) != ResultMoney(=%d).", player->GetObjectID( ), totalPrice, result );

		// 판매자 선 응답 - 아이템.
		player->UpdateInventory( inventory, (-msg->count) );

		HANDLE                       handle  = NULL;
		MSG_RES_ITEM_STALL_SELL_COM* sendMsg = (MSG_RES_ITEM_STALL_SELL_COM*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_ITEM, NM_ITEM_STALL_SELL_COM_RES );
		if ( sendMsg != NULL )
		{
			wcscpy( sendMsg->characterName, GetName() );
			Inventory2sInventory( &sendMsg->inventory, inventory );
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_ITEM_STALL_SELL_COM) );
		}

		if ( stallSellItem->inventory->count > 0 )
		{
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE
										 ,player->GetObjectID( )
										 ,stallSellItem->inventory
										 ,"STALL SELL - SELL CHARACTER_IDX=%d"
										 ,mObject.index );

			// 판매후 갱신.
			stallSellItem->status = StallSellItemStart;
			player->SendMsgSynStallSell( NM_ITEM, NM_ITEM_STALL_SELL_ADD_SYN, stallSellItem->inventory, stallSellItem->price );
		}
		else
		{
			NETWORK2->PostInventoryEvent( EVENT_INVENTORY_DELETE
										 ,player->GetObjectID( )
										 ,stallSellItem->inventory
										 ,"STALL SELL - SELL CHARACTER_IDX=%d"
										 ,mObject.index );

			// 판매후 삭제.
			player->SendMsgSynStallSell( NM_ITEM, NM_ITEM_STALL_SELL_DEL_SYN, stallSellItem->inventory, stallSellItem->price );

			player->RemoveInventory( stallSellItem->inventory->number );
			player->ReleaseStallSellItem( stallSellItem );
		}
	}
}

// SendMsgResStallSell Method.
bool cPlayer::SendMsgResStallSell(char category, char protocol, int error)
{
	return NETWORK2->SendMsgError( mConnectionIdx, category, protocol, error );
}

// SendMsgSynStallSell Method.
void cPlayer::SendMsgSynStallSell(char category, char protocol, TB_INVENTORY* inventory, long price)
{
	unsigned long* stallSellGuest = mStallSellGuests;

	for ( int i = 0; i < MAX_STALL_GUEST; i++, stallSellGuest++ )
	{
		if ( (*stallSellGuest) != 0 )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( (*stallSellGuest) );
			if ( player != NULL )
			{
				HANDLE                       handle = NULL;
				MSG_SYN_ITEM_STALL_SELL_DAT* synMsg = (MSG_SYN_ITEM_STALL_SELL_DAT*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), category, protocol );
				if ( synMsg != NULL )
				{
					Inventory2sInventory( &synMsg->inventory, inventory );
					synMsg->price = price;

					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_STALL_SELL_DAT) );
				}
			}
			else
			{
				ReleaseStallSellGuest( (*stallSellGuest) );
			}
		}
	}
}

// SetItemAgentCheck Method.
void cPlayer::SetItemAgentCheck(long nextCheck)
{
	mItemAgentCheck     = (nextCheck > 0) ? true : false;
	mItemAgentNextCheck = NETWORK2->GetAccumTime( ) + nextCheck;
}

// ItemAgentSearchOwner Method.
bool cPlayer::ItemAgentPageOpen( ULONG_PTR socketContext, long startPage )
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_OPEN_PAGE_FAIL;

	if ( startPage <= 0 )
		startPage= 1;

	HANDLE                   handle = NULL;
	ITEM_AGENT_SELECT_OWNER* select = (ITEM_AGENT_SELECT_OWNER*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT_OWNER );
	long                     length = sizeof(ITEM_AGENT_SELECT_OWNER) - sizeof(select->table);

	select->characterIdx = mObject.index;
	select->startPage    = startPage;

	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}

// ItemAgentAdd Method.
void cPlayer::ItemAgentAdd(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_ADD* msg)
{
	/*-- 검사할 내용들. --*/
	if ( mInventoryDatabase )
		throw ERROR_NPC_ITEM_AGENT_ADD_NOTYET;

	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

	// 인벤토리 검사.
    if( IsInventoryBag( msg->Number ) == false )
        throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

	TB_INVENTORY* inventory = SelectInventory( msg->Number );
	if ( IsInventory( inventory ) == false )
		throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

    __int64 totalPrice = (__int64)msg->Price * (__int64)msg->Count;
    if( totalPrice > MAX_TRADE_MONEY )
        throw ERROR_NPC_ITEM_AGENT_ADD_TRADE_MONEY;

	// 등록수수료 검사.
	switch ( msg->ValidThru )
	{
	case NPC_ITEM_AGENT_TIME1:
	case NPC_ITEM_AGENT_TIME2:
	case NPC_ITEM_AGENT_TIME3:
		break;
	default:
		throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;
	}
	u_long commission = (u_long)(msg->ValidThru / NPC_ITEM_AGENT_TIME1 * ITEM_AGENT_NPC_FEE);
	if ( commission > mMoney )
		throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if ( itemDefine == NULL )
		throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

    if ( IsItemExchange( itemDefine, inventory ) == false )
		throw ERROR_NPC_ITEM_AGENT_ADD_EXCHANGE;

	if ( msg->StartPage <= 0 )
		msg->StartPage = 1;

	HANDLE             handle = NULL;
	ITEM_AGENT_INSERT* insert = (ITEM_AGENT_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_INSERT );
	long               length = sizeof(ITEM_AGENT_INSERT) - sizeof(insert->table);

	insert->inventory = (*inventory);
	insert->validThru = msg->ValidThru;
	insert->price     = msg->Price;

	insert->characterIdx = mObject.index;
	insert->startPage    = msg->StartPage;

	mInventoryDb.agentInsert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.agentInsert == false ) throw ERROR_NPC_ITEM_AGENT_ADD_FAIL;

	// 등록전 삭제.
	RemoveInventory( inventory );
}

// ItemAgentDel Method.
void cPlayer::ItemAgentDel(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_DEL* msg)
{
	/*-- 검사할 내용들. --*/
	if ( mInventoryDatabase )
		throw ERROR_NPC_ITEM_AGENT_DEL_NOTYET;

	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_DEL_FAIL;

	short inventoryNumber = GetEmptyBagNumber( );
	if ( inventoryNumber == INVENTORY_BAG_NONE )
		throw ERROR_NPC_ITEM_AGENT_DEL_FAIL;

	if ( msg->StartPage <= 0 )
		msg->StartPage = 1;

	HANDLE             handle = NULL;
	ITEM_AGENT_DELETE* del    = (ITEM_AGENT_DELETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_DELETE );
	long               length = sizeof(ITEM_AGENT_DELETE) - sizeof(del->table);

	del->idx             = msg->Idx;
	del->characterIdx    = mObject.index;
	del->inventoryNumber = inventoryNumber;

	del->startPage       = msg->StartPage;

	mInventoryDb.agentDelete = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.agentDelete == false ) throw ERROR_NPC_ITEM_AGENT_DEL_FAIL;
}

// ItemAgentGet Method.
void cPlayer::ItemAgentGet(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_GET* msg)
{
	/*-- 검사할 내용들. --*/

	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_DEL_FAIL;

	if ( msg->startPage <= 0 )
		msg->startPage = 1;
	msg->itemDefineName[ 50 ] = 0;

	HANDLE             handle = NULL;
	ITEM_AGENT_UPDATE* update = (ITEM_AGENT_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_UPDATE );
	unsigned long      length = sizeof(ITEM_AGENT_UPDATE) - sizeof(update->table);

	update->characterIdx = mObject.index;
	update->idx          = msg->Idx;
	update->count        = msg->Count;

	update->startPage               = msg->startPage;
	update->sort                    = msg->sortData;
	wcscpy_s( update->itemDefineName, 50, msg->itemDefineName );
	update->itemDefineSearchType    = msg->itemDefineSearchType;
	update->itemDefineSearchSubType = msg->itemDefineSearchSubType;
	update->itemDefineRareLevel     = msg->itemDefineRareLevel;
	update->itemLimitLevelB         = msg->itemLimitCharLevelB;
	update->itemLimitLevelE         = msg->itemLimitCharLevelE;
	update->inventoryEnhanced       = msg->inventoryEnhanced;

	mInventoryDb.agentUpdate = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}

// CbItemAgentCheck Method.
void cPlayer::CbItemAgentCheck(ITEM_AGENT_CHECK* check)
{
	if ( check->retvalue == 0 )
	{
		// 아이템 판매대행 취소.
		if ( check->rowCount > 0 )
		{
			NETWORK2->SendMsgRoot( mConnectionIdx, NM_CHAT ,NM_CHAT_POST_RECEIVED_SYN );
		}
		// 아이템 판매대행 검사 설정.
		if ( check->nextCheck > DELAY_ITEM_AGENT_CHECK )
		{
			SetItemAgentCheck( check->nextCheck );
		}
		else if ( check->nextCheck > 0 )
		{
			SetItemAgentCheck( DELAY_ITEM_AGENT_CHECK );
		}
		else
			SetItemAgentCheck( 0 );
	}
	else
	{
		// 오류일경우, 10분후 재시도.
		SetItemAgentCheck( DELAY_ITEM_AGENT_CHECK );
	}
}

// CbItemAgentAdd Method.
void cPlayer::CbItemAgentAdd(ULONG_PTR socketContext, ITEM_AGENT_INSERT* insert, u_long requestResult)
{
	if ( mInventoryDb.agentInsert )
		mInventoryDb.agentInsert = false;
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( insert->retvalue != 0 ) throw insert->retvalue;

	ITEM_AGENT_RESULT_OWNER*    table   = insert->table;
	HANDLE                      handle  = NULL;
	MSG_RES_NPC_ITEM_AGENT_ADD* sendMsg = (MSG_RES_NPC_ITEM_AGENT_ADD*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_NPC, NM_NPC_ITEM_AGENT_ADD_RES );
	NPC_ITEM_AGENT_OPEN_RESULT* result  = sendMsg->Result;

	sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_ADD_SUCCESS;
	sendMsg->StartPage = insert->startPage;
	sendMsg->EndPage   = insert->endPage;
	sendMsg->RowCount  = insert->rowCount;

	for ( long i = 0; i < insert->rowCount; i++, table++, result++ )
	{
		result->Idx = table->idx;
		result->ValidThru = table->validThru;
		result->Price = table->price;
		Inventory2sInventory( &result->Inventory, &table->table );
	}

	// 등록수수료 차감.
	long commission = (long)(insert->validThru / NPC_ITEM_AGENT_TIME1 * ITEM_AGENT_NPC_FEE);
	AddMoney( mObject, -commission );

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
	NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, &insert->inventory, "ITEM AGENT - ADD" );

	// 다음 검사시간 설정.
	mItemAgentCheck     = true;
	mItemAgentNextCheck = insert->nextCheck;

	// 아이템 판매대행 검사 설정.
	if ( insert->nextCheck > 0 )
	{
		SetItemAgentCheck( insert->nextCheck );
	}
}

// CbItemAgentDel Method.
void cPlayer::CbItemAgentDel(ULONG_PTR socketContext, ITEM_AGENT_DELETE* del, u_long requestResult)
{
	if ( mInventoryDb.agentDelete )
		mInventoryDb.agentDelete = false;
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( del->retvalue != 0 ) throw del->retvalue;

	HANDLE                      handle  = NULL;
	MSG_RES_NPC_ITEM_AGENT_DEL* sendMsg = (MSG_RES_NPC_ITEM_AGENT_DEL*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_NPC, NM_NPC_ITEM_AGENT_DEL_RES );
	NPC_ITEM_AGENT_OPEN_RESULT* result  = sendMsg->Result;
	ITEM_AGENT_RESULT_OWNER*    table   = del->table;

	sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_DEL_SUCCESS;
	Inventory2sInventory( &sendMsg->Inventory, &del->result );

	sendMsg->StartPage = del->startPage;
	sendMsg->EndPage   = del->endPage;
	sendMsg->RowCount  = del->rowCount;

	for ( long i = 0; i < del->rowCount; i++, table++, result++ )
	{
		result->Idx = table->idx;
		result->ValidThru = table->validThru;
		result->Price = table->price;
		Inventory2sInventory( &result->Inventory, &table->table );
	}

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );

	XCopyInventory( &del->result );

	NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE, mObject.index, &del->result, "ITEM AGENT - DEL" );
}

// CbItemAgentGet Method.
void cPlayer::CbItemAgentGet(ULONG_PTR socketContext, ITEM_AGENT_UPDATE* update, u_long requestResult)
{
	if ( mInventoryDb.agentUpdate )
		mInventoryDb.agentUpdate = false;
	else
		return;

	if ( requestResult != 0 ) throw requestResult;
	if ( update->retvalue != 0 ) throw update->retvalue;

	HANDLE                      handle  = NULL;
	MSG_RES_NPC_ITEM_AGENT_GET* sendMsg = (MSG_RES_NPC_ITEM_AGENT_GET*)NETWORK2->GetMsgRoot( &handle
																							,(PerSocketContext*)socketContext
																							,NM_NPC
																							,NM_NPC_ITEM_AGENT_GET_RES );
	NPC_ITEM_AGENT_SEARCH_RESULT* result = sendMsg->Result;

	sendMsg->ErrorCode = update->retvalue;

	AddMoney( mObject, (-update->price) );

	sendMsg->StartPage = update->startPage;
	sendMsg->EndPage   = update->endPage;
	sendMsg->RowCount  = update->rowCount;

	ITEM_AGENT_RESULT* table = update->table;
	for ( long i = 0; i < update->rowCount; i++, table++, result++ )
	{
		result->Idx = table->idx;
		wcscpy_s( result->CharacterName, table->characterName );
		result->ValidThru = table->validThru;
		result->Price = table->price;
		Inventory2sInventory( &result->Inventory, &table->table );
	}

	/*-- 로그 --*/
	NETWORK2->PostInventoryEvent( update->sciLog.apply ? EVENT_INVENTORY_DELETE : EVENT_INVENTORY_UPDATE
								 ,update->sccIdx
								 ,&update->sciLog
								 ,"ITEM AGENT - GET (BUY::%d)", update->bciLog.idx );	// 판매완료(SELL COMPLETE ITEM)
	NETWORK2->PostInventoryEvent( EVENT_INVENTORY_CREATE
								 ,mObject.index
								 ,&update->bciLog
								 ,"ITEM AGENT - GET (SELL::%d)", update->sciLog.idx );	// 구매완료(BUY COMPLETE ITEM)

	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

	/*-- 우편동기화 --*/
	NETWORK2->SendMsgRoot( (PerSocketContext*)socketContext, NM_CHAT, NM_CHAT_POST_RECEIVED_SYN );

	cPlayer* player = GRIDMANAGER->GetPlayer( update->sccIdx );
	if ( player != NULL )
	{
		NETWORK2->SendMsgRoot( player->GetConnectionIdx ( ), NM_CHAT, NM_CHAT_POST_RECEIVED_SYN );
	}
	else
	{
		NETWORK2->PostPostReceived( update->sccIdx );
	}
}

//// ItemAgentSearch Method.
//bool cPlayer::ItemAgentSearch(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH* msg)
//{
//	// 캐릭터 상태 검사.
//	if ( GetStateDie( ) == true )
//		throw ERROR_NPC_ITEM_AGENT_SEARCH_FAIL;
//
//	if ( msg->startPage <= 0 )
//		msg->startPage = 1;
//
//	HANDLE              handle = NULL;
//	ITEM_AGENT_SELECT* select = (ITEM_AGENT_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT );
//	long               length = sizeof(ITEM_AGENT_SELECT) - sizeof(select->table);
//
//	select->startPage           = msg->startPage;
//	select->sort                = msg->sortData;
//	select->itemDefineRareLevel = msg->itemDefineRareLevel;
//	select->itemLimitLevelB     = msg->itemLimitCharLevelB;
//	select->itemLimitLevelE     = msg->itemLimitCharLevelE;
//	select->inventoryEnhanced   = msg->inventoryEnhanced;
//
//	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
//}

// ItemAgentSearchName Method.
//bool cPlayer::ItemAgentSearchName(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_NAME* msg)
//{
//	// 캐릭터 상태 검사.
//	if ( GetStateDie( ) == true )
//		throw ERROR_NPC_ITEM_AGENT_SEARCH_FAIL;
//
//	if ( msg->startPage <= 0 )
//		msg->startPage = 1;
//	msg->itemDefineName[ 50 ] = 0;
//
//	HANDLE                  handle = NULL;
//	ITEM_AGENT_SELECT_NAME* select = (ITEM_AGENT_SELECT_NAME*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT_NAME );
//	long                    length = sizeof(ITEM_AGENT_SELECT_NAME) - sizeof(select->table);
//
//	select->startPage           = msg->startPage;
//	select->sort                = msg->sortData;
//	wcscpy_s( select->itemDefineName, 50, msg->itemDefineName );
//	select->itemDefineRareLevel = msg->itemDefineRareLevel;
//	select->itemLimitLevelB     = msg->itemLimitCharLevelB;
//	select->itemLimitLevelE     = msg->itemLimitCharLevelE;
//	select->inventoryEnhanced   = msg->inventoryEnhanced;
//
//	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
//}

// ItemAgentSearchType Method.
bool cPlayer::ItemAgentSearchType(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_TYPE* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_SEARCH_FAIL;

	if ( msg->startPage <= 0 )
		msg->startPage = 1;

	HANDLE                  handle = NULL;
	ITEM_AGENT_SELECT_TYPE* select = (ITEM_AGENT_SELECT_TYPE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT_TYPE );
	long                    length = sizeof(ITEM_AGENT_SELECT_TYPE) - sizeof(select->table);

	select->startPage               = msg->startPage;
	select->sort                    = msg->sortData;
	select->itemDefineSearchType    = msg->itemDefineSearchType;
	select->itemDefineSearchSubType = msg->itemDefineSearchSubType;
	select->itemDefineRareLevel     = msg->itemDefineRareLevel;
	select->itemLimitLevelB         = msg->itemLimitCharLevelB;
	select->itemLimitLevelE         = msg->itemLimitCharLevelE;
	select->inventoryEnhanced       = msg->inventoryEnhanced;

	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}

// ItemAgentSearchAll Method.
bool cPlayer::ItemAgentSearchAll(ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_ALL* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_NPC_ITEM_AGENT_SEARCH_FAIL;

	if ( msg->startPage <= 0 )
		msg->startPage = 1;
	msg->itemDefineName[ 50 ] = 0;

	HANDLE                 handle = NULL;
	ITEM_AGENT_SELECT_ALL* select = (ITEM_AGENT_SELECT_ALL*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_AGENT_SELECT_ALL );
	long                   length = sizeof(ITEM_AGENT_SELECT_ALL) - sizeof(select->table);

	select->startPage               = msg->startPage;
	select->sort                    = msg->sortData;
	wcscpy_s( select->itemDefineName, 50, msg->itemDefineName );
	select->itemDefineSearchType    = msg->itemDefineSearchType;
	select->itemDefineSearchSubType = msg->itemDefineSearchSubType;
	select->itemDefineRareLevel     = msg->itemDefineRareLevel;
	select->itemLimitLevelB         = msg->itemLimitCharLevelB;
	select->itemLimitLevelE         = msg->itemLimitCharLevelE;
	select->inventoryEnhanced       = msg->inventoryEnhanced;

	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}

// SetPostCheck Method.
void cPlayer::SetPostCheck(long nextCheck)
{
	mPostCheck     = (nextCheck > 0) ? true : false;
	mPostNextCheck = NETWORK2->GetAccumTime( ) + nextCheck;
}

// PostOpen Method.
bool cPlayer::PostOpen(ULONG_PTR socketContext, long startPage)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw false;

	if ( startPage <= 0 )
		startPage = 1;

	HANDLE       handle = NULL;
	POST_SELECT* select = (POST_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_SELECT );
	long         length = sizeof(POST_SELECT) - sizeof(select->table);

	select->characterIdx = mObject.index;
	select->startPage    = startPage;

	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}
bool cPlayer::PostOpen(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_OPEN* msg)
{
	if ( PostOpen( socketContext, msg->startPage ) == false )
		throw ERROR_CHAT_POST_OPEN_FAIL;
	return true;
}

// PostClose Method.
void cPlayer::PostClose(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_CLOSE* /*msg*/)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) != true )
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_CHAT, NM_CHAT_POST_CLOSE_RES, ERROR_CHAT_POST_CLOSE_SUCCESS );
	else
		throw ERROR_CHAT_POST_CLOSE_FAIL;
}

// PostRead Method.
void cPlayer::PostRead(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_READ* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_CHAT_POST_READ_FAIL;

	HANDLE              handle = NULL;
	POST_UPDATE_STATUS* status = (POST_UPDATE_STATUS*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_UPDATE_STATUS );

	status->characterIdx = mObject.index;
	status->idx          = msg->idx;
	status->status       = POST_STATUS_READ;

	NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(POST_UPDATE_STATUS) );
}

// PostWrite Method.
void cPlayer::PostWrite(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_WRITE* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_CHAT_POST_WRITE_FAIL;

	if ( mInventoryDatabase )
		throw ERROR_CHAT_POST_WRITE_FAIL;

	msg->name[ MAX_POST_NAME ]=0;
	msg->title[ MAX_POST_TITLE ]=0;
	msg->message[ MAX_POST_MESSAGE ]=0;

	if ( wcsicmp( msg->name, mPlayerInfo.strName ) == 0 )
		throw ERROR_CHAT_POST_WRITE_FAIL;

	TB_INVENTORY*   inventory1 = NULL;
	TB_INVENTORY*   inventory2 = NULL;
	TB_INVENTORY*   inventory3 = NULL;
	TB_ITEM_DEFINE* itemDefine = NULL;

	switch ( msg->rowCount )
	{
	case 3:
        if ( IsInventoryBag( msg->inventoryNumber3 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		if ( msg->inventoryNumber3 == msg->inventoryNumber2 )
			throw ERROR_CHAT_POST_WRITE_FAIL;
		if ( msg->inventoryNumber3 == msg->inventoryNumber1 )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		inventory3 = SelectInventory( msg->inventoryNumber3 );
		if ( IsInventory( inventory3 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		itemDefine = ITEMMANAGER->GetItemDefine( inventory3->itemDefineIdx );
        if ( itemDefine == NULL )
		    throw ERROR_CHAT_POST_WRITE_FAIL;

        if ( IsItemExchange( itemDefine, inventory3 ) == false )
			throw ERROR_CHAT_POST_WRITE_EXCHANGE;	
	case 2:
        if ( IsInventoryBag( msg->inventoryNumber2 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		if ( msg->inventoryNumber2 == msg->inventoryNumber1 )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		inventory2 = SelectInventory( msg->inventoryNumber2 );
		if ( IsInventory( inventory2 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		itemDefine = ITEMMANAGER->GetItemDefine( inventory2->itemDefineIdx );
		if ( itemDefine == NULL )
			throw ERROR_CHAT_POST_WRITE_FAIL;

        if ( IsItemExchange( itemDefine, inventory2 ) == false )
			throw ERROR_CHAT_POST_WRITE_EXCHANGE;	
	case 1:
        if ( IsInventoryBag( msg->inventoryNumber1 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		inventory1 = SelectInventory( msg->inventoryNumber1 );
		if ( IsInventory( inventory1 ) == false )
			throw ERROR_CHAT_POST_WRITE_FAIL;

		itemDefine = ITEMMANAGER->GetItemDefine( inventory1->itemDefineIdx );
		if ( itemDefine == NULL )
			throw ERROR_CHAT_POST_WRITE_FAIL;

        if ( IsItemExchange( itemDefine, inventory1 ) == false )
			throw ERROR_CHAT_POST_WRITE_EXCHANGE;	
		break;
	case 0:
		break;
	default:
		throw ERROR_CHAT_POST_WRITE_FAIL;
	}

    // 최대 거래 가능금액
    if( msg->money > MAX_TRADE_MONEY )
        throw ERROR_CHAT_POST_WRITE_TRADE_MONEY;

	long serviceCharge = MAIL_DEFAULT_CHARGE;				// 기본 수수료
	serviceCharge += (msg->rowCount * MAIL_ITEM_CHARGE);	// 첨부 아이템 수수료

	if ( msg->money < 0 || (unsigned long)(msg->money + serviceCharge) > mMoney )
		throw ERROR_CHAT_POST_WRITE_FAIL;

	HANDLE       handle = NULL;
	POST_INSERT* insert = (POST_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_INSERT );

	insert->toIdx         = 0;
	wcscpy( insert->toName, msg->name );

	insert->validThru     = 14;
	insert->type          = 0;										// 0우편
	insert->fromIdx       = mObject.index;
	wcscpy( insert->fromName, mPlayerInfo.strName );

	wcscpy( insert->contentTitle, msg->title );
	wcscpy( insert->contentMessage, msg->message );

	if ( inventory1 != NULL ) insert->inventory1 = (*inventory1);
	if ( inventory2 != NULL ) insert->inventory2 = (*inventory2);
	if ( inventory3 != NULL ) insert->inventory3 = (*inventory3);
	insert->money          = msg->money;
	insert->serviceCharage = serviceCharge;

	mInventoryDb.postInsert = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(POST_INSERT) );
	if ( mInventoryDb.postInsert == 0 )
		throw ERROR_CHAT_POST_WRITE_FAIL;
}

// PostDownload Method.
void cPlayer::PostDownload(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_DOWNLOAD* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_CHAT_POST_DOWNLOAD_FAIL;

	if ( mInventoryDatabase )
		throw ERROR_CHAT_POST_DOWNLOAD_FAIL;

	if ( msg->inventoryNumber1 != 0 )
	{
		if ( (msg->inventoryNumber2 != 0 && msg->inventoryNumber1 == msg->inventoryNumber2) ||
			 (msg->inventoryNumber3 != 0 && msg->inventoryNumber1 == msg->inventoryNumber3) )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}
	if ( msg->inventoryNumber2 != 0 )
	{
		if ( (msg->inventoryNumber1 != 0 && msg->inventoryNumber2 == msg->inventoryNumber1) ||
			 (msg->inventoryNumber3 != 0 && msg->inventoryNumber2 == msg->inventoryNumber3) )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}
	if ( msg->inventoryNumber3 != 0 )
	{
		if ( (msg->inventoryNumber1 != 0 && msg->inventoryNumber3 == msg->inventoryNumber1) ||
			 (msg->inventoryNumber2 != 0 && msg->inventoryNumber3 == msg->inventoryNumber2) )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}

	HANDLE       handle = NULL;
	POST_UPDATE* update = (POST_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_UPDATE );
	long         length = sizeof(POST_UPDATE)-sizeof(update->table);

	TB_INVENTORY* inventory;

	update->characterIdx = mObject.index;
	update->idx = msg->idx;

	if ( msg->inventoryNumber1 != 0 )
	{
        if ( IsInventoryBag( msg->inventoryNumber1 ) == false )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;

		inventory = SelectInventory( msg->inventoryNumber1 );
		if ( IsInventory( inventory ) != true )
			update->inventoryNumber1 = msg->inventoryNumber1;
		else
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}
	if ( msg->inventoryNumber2 != 0 )
	{
        if ( IsInventoryBag( msg->inventoryNumber2 ) == false )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;

		inventory = SelectInventory( msg->inventoryNumber2 );
		if ( IsInventory( inventory ) != true )
			update->inventoryNumber2 = msg->inventoryNumber2;
		else
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}
	if ( msg->inventoryNumber3 != 0 )
	{
        if ( IsInventoryBag( msg->inventoryNumber3 ) == false )
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;

		inventory = SelectInventory( msg->inventoryNumber3 );
		if ( IsInventory( inventory ) != true )
			update->inventoryNumber3 = msg->inventoryNumber3;
		else
			throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
	}

	update->money = (long)msg->money;

	mInventoryDb.postUpdate = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	if ( mInventoryDb.postUpdate == 0 )
		throw ERROR_CHAT_POST_DOWNLOAD_FAIL;
}

// PostDelete Method.
void cPlayer::PostDelete(ULONG_PTR socketContext, MSG_REQ_CHAT_POST_DELETE* msg)
{
	// 캐릭터 상태 검사.
	if ( GetStateDie( ) == true )
		throw ERROR_CHAT_POST_DELETE_FAIL;

	if ( msg->rowCount > MAX_POST_DELETE )
		throw ERROR_CHAT_POST_DELETE_FAIL;

	if ( msg->startPage <= 0 )
		msg->startPage = 1;

	HANDLE       handle = NULL;
	POST_DELETE* del    = (POST_DELETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_POST_DELETE );
	long         length = sizeof(POST_DELETE)-sizeof(del->table);

	del->characterIdx = mObject.index;

	for ( BYTE i = 0; i < msg->rowCount; i++ )
	{
		del->idx[i] = msg->idx[i];
	}

	del->startPage = msg->startPage;

	NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
}

// SendPostSelect Method.
void cPlayer::SendPostSelect( ULONG_PTR socketContext, long startPage, long endPage, long noneRead, POST_RESULT* postResult, long postCount )
{
    // 우편 리스트 전송
    HANDLE                  handle  = NULL;
    MSG_RES_CHAT_POST_OPEN* sendMsg = (MSG_RES_CHAT_POST_OPEN*)NETWORK2->GetMsgRoot( &handle, 
                                                                                    (PerSocketContext*)socketContext, 
                                                                                    NM_CHAT, 
                                                                                    NM_CHAT_POST_OPEN_RES );
    POST_OPEN_RESULT* sendPost  = sendMsg->Result;

    sendMsg->ErrorCode = ERROR_CHAT_POST_OPEN_SUCCESS;
    sendMsg->startPage = startPage;
    sendMsg->endPage   = endPage;
    sendMsg->noneRead  = noneRead;
    sendMsg->RowCount  = postCount;

    for ( long i = 0; i < postCount; i++, postResult++, sendPost++ )
    {
        sendPost->idx = postResult->idx;

        sendPost->registerDate.wYear   = postResult->registerDate.year;
        sendPost->registerDate.wMonth  = postResult->registerDate.month;
        sendPost->registerDate.wDay    = postResult->registerDate.day;
        sendPost->registerDate.wHour   = postResult->registerDate.hour;
        sendPost->registerDate.wMinute = postResult->registerDate.minute;
        sendPost->registerDate.wSecond = postResult->registerDate.second;

        sendPost->validThru = postResult->validThru;
        sendPost->type      = postResult->type;
        sendPost->status    = postResult->status;

        wcscpy( sendPost->fromName, postResult->fromName );

        switch ( sendPost->type )
        {
        case POST_TYPE_NORMAL:
            wcscpy( sendPost->content.title,    postResult->contenTtitle    );
            wcscpy( sendPost->content.message,  postResult->contentMessage  );
            break;
        case POST_TYPE_AGENT_CANCEL:
            break;
        case POST_TYPE_AGENT_SALE:
            sendPost->receipt.price             = postResult->receiptPrice;
            sendPost->receipt.salesCharges      = postResult->receiptSalesCharges;
            sendPost->receipt.inventory.count   = postResult->receiptInventoryCount;
            Inventory2sInventory( &sendPost->receipt.inventory, &postResult->receiptInventory );
            break;
        case POST_TYPE_AGENT_PURCHASE:
            Inventory2sInventory( &sendPost->receipt.inventory, &postResult->receiptInventory );
            sendPost->receipt.inventory.count = postResult->receiptInventoryCount;
            break;
        }

        Inventory2sInventory( &sendPost->inventory1, &postResult->inventory1 );
        Inventory2sInventory( &sendPost->inventory2, &postResult->inventory2 );
        Inventory2sInventory( &sendPost->inventory3, &postResult->inventory3 );
        sendPost->money = postResult->money;
    }

    NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
}

// CbPostCheck Method.
void cPlayer::CbPostCheck(POST_CHECK* check)
{
	if ( check->rowCount > 0 )
	{
		NETWORK2->SendMsgRoot( mConnectionIdx, NM_CHAT, NM_CHAT_POST_RECEIVED_SYN );
	}
}

// CbPostRead Method.
void cPlayer::CbPostRead(POST_UPDATE_STATUS* status)
{
	HANDLE                  handle  = NULL;
	MSG_RES_CHAT_POST_READ* sendMsg = (MSG_RES_CHAT_POST_READ*)NETWORK2->GetMsgRoot( &handle
																					,mConnectionIdx
																					,NM_CHAT
																					,NM_CHAT_POST_READ_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_CHAT_POST_READ_SUCCESS;
		sendMsg->noneRead  = status->noneRead;

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_CHAT_POST_READ) );
	}
}

// CbPostWrite Method.
void cPlayer::CbPostWrite(ULONG_PTR socketContext, POST_INSERT* insert)
{
	if ( mInventoryDb.postInsert )
	{
		mInventoryDb.postInsert = false; // DB 사용종료.
		NETWORK2->SendMsgError( (PerSocketContext*)socketContext, NM_CHAT, NM_CHAT_POST_WRITE_RES, insert->retvalue );

		if ( insert->retvalue == 0 )
		{
			if ( IsInventory( &insert->inventory1 ) )
			{
				RemoveInventory( insert->inventory1.number );
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE
											 ,mObject.index
											 ,&insert->inventory1
											 ,"Post Write - Items attached (TB_CHARACTER::IDX=%d)"
											 ,insert->toIdx );
			}
			if ( IsInventory( &insert->inventory2 ) )
			{
				RemoveInventory( insert->inventory2.number );
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE
											 ,mObject.index
											 ,&insert->inventory2
											 ,"Post Write - Items attached (TB_CHARACTER::IDX=%d)"
											 ,insert->toIdx );
			}
			if ( IsInventory( &insert->inventory3 ) )
			{
				RemoveInventory( insert->inventory3.number );
				NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE
											 ,mObject.index
											 ,&insert->inventory3
											 ,"Post Write - Items attached (TB_CHARACTER::IDX=%d)"
											 , insert->toIdx );
			}
			if ( insert->money > 0 )
			{
				sObject object  = { eOBJECTTYPE_PLAYER, insert->toIdx };
				AddMoney( object, (-insert->money) );
			}
			if ( insert->serviceCharage > 0 )
			{
				AddMoney( mObject, (-insert->serviceCharage) );
			}
		}

		/*-- 우편동기화 --*/
		cPlayer* player = GRIDMANAGER->GetPlayer( insert->toIdx );
		if ( player != NULL )
		{
			NETWORK2->SendMsgRoot( player->GetConnectionIdx( ), NM_CHAT, NM_CHAT_POST_RECEIVED_SYN );
		}
		else
		{
			NETWORK2->PostPostReceived( insert->toIdx );
		}
	}
}

// CbPostDownload Method.
void cPlayer::CbPostDownload(ULONG_PTR socketContext, POST_UPDATE* update)
{
	if ( mInventoryDb.postUpdate )
	{
		mInventoryDb.postUpdate = false; // DB 사용종료.

		HANDLE                      handle  = NULL;
		MSG_RES_CHAT_POST_DOWNLOAD* sendMsg = (MSG_RES_CHAT_POST_DOWNLOAD*)NETWORK2->GetMsgRoot( &handle
																								,(PerSocketContext*)socketContext
																								,NM_CHAT
																								,NM_CHAT_POST_DOWNLOAD_RES );
		TB_INVENTORY* table      = update->table;
		sInventory*   sinventory = sendMsg->inventory;

		sendMsg->ErrorCode = update->retvalue;
		if ( sendMsg->ErrorCode == ERROR_CHAT_POST_DOWNLOAD_SUCCESS )
		{
			for ( long i = 0; i < update->rowCount; i++, table++, sinventory++ )
			{
				if ( ITEMMANAGER->GetItemDefine( table->itemDefineIdx ) != NULL )
				{
					XCopyInventory( table );
					Inventory2sInventory( sinventory, table );
					NETWORK2->PostInventoryEvent( EVENT_INVENTORY_UPDATE
												 ,mObject.index
												 ,table
												 ,"Post Download (TB_POST::IDX=%d)"
												 ,update->idx );

					sendMsg->rowCount++;
				}
				else
				{
					NETWORK2->PostServerEvent( "Error - cPlayer::CbPostDownload - TB_POST::IDX(%d).", update->idx );
				}
			}

			sendMsg->money = (update->money != 0);

			if ( update->money != 0 )
			{
				unsigned long befor = GetMoney( );
				AddMoney( mObject, update->money, false );
				NETWORK2->PostMoneyEvent( update->characterIdx
										 ,befor
										 ,GetMoney( )
										 ,"Post Download (TB_POST::IDX=%d, MONEY=%d)"
										 ,update->idx
										 ,update->money );
			}
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );

		// 우편 수령로그
		long inventoryIdx1=0;
		long inventoryIdx2=0;
		long inventoryIdx3=0;
		switch ( update->rowCount )
		{
		case 3:
			inventoryIdx3 = update->table[ 2 ].idx;
		case 2:
			inventoryIdx2 = update->table[ 1 ].idx;
		case 1:
			inventoryIdx1 = update->table[ 0 ].idx;
			break;
		}
		NETWORK2->PostPostEvent( update->idx, update->characterIdx, inventoryIdx1, inventoryIdx2, inventoryIdx3, update->money );
	}
}

// IsTarotReaderClose Method.
bool cPlayer::IsTarotReaderClose( )
{
	unsigned long* tarotGuest       = mTarotGuests;
	int            tarotGuestOffset = mTarotGuestOffset;

	for ( int i = 0; i < MAX_TAROT_GUEST && tarotGuestOffset > 0; i++, tarotGuest++ )
	{
		if ( (*tarotGuest) > 0 )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( (*tarotGuest) );

			if ( player != NULL )
			{
				if ( player->GetStateStop( ) == eSTOP_USETAROT )
					return false;
			}

			tarotGuestOffset--;
		}
	}
	return true;
}

// TarotReaderReady Method.
void cPlayer::TarotReaderReady(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_READER_READY* /*msg*/)
{
	/// 2차상태 체크
	if ( IsRequestRejection() == true )
		throw ERROR_TAROT_READER_READY_FAIL;			// 캐릭터 상태2오류.

	if ( IsChangeState( eOBJECT_STATE_STOP ) == false )
		throw ERROR_TAROT_READER_READY_FAIL;			// 캐릭터 상태 오류 - 상태변경 불가.

	if ( GetStateStop( ) != eSTOP_NONE )
		throw ERROR_TAROT_READER_READY_FAIL;			// 캐릭터 상태 오류.

	/*-- 캐릭터 상태 변경.
	*/
	if ( ChangeState( eOBJECT_STATE_STOP ) == false )
		throw ERROR_TAROT_READER_READY_FAIL;			// 캐릭터 상태 오류 - 변경불가.

	SetStateStop( eSTOP_READYTAROT );					// 캐릭터 정지상태 중 타로준비.

	/*-- 타로 개설정보 보내기 [RES/SYN] 메시지 발송.
	*/
	SendMsgResTarot( NM_TAROT, NM_TAROT_READER_READY_RES, ERROR_TAROT_READER_READY_SUCCESS );

	MSG_SYN_TAROT_READER_READY msgSyn;
	memset( &msgSyn, 0, sizeof(MSG_SYN_TAROT_READER_READY) );

	msgSyn.Category      = NM_TAROT;
	msgSyn.Protocol      = NM_TAROT_READER_READY_SYN;
	msgSyn.mCharacterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
}

// TarotReaderOpen Method.
void cPlayer::TarotReaderOpen(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_READER_OPEN* msg)
{
	if ( GetState( ) != eOBJECT_STATE_STOP )
		throw ERROR_TAROT_READER_OPEN_FAIL;				// 캐릭터 상태 오류.

	if ( GetStateStop( ) != eSTOP_READYTAROT )
		throw ERROR_TAROT_READER_OPEN_FAIL;				// 캐릭터 상태 오류.

	/*-- 타로카드 검사 - 22장(1벌)을 모두 가지고 있는지 확인과 고유번호 검사.
	*/
	TB_INVENTORY*  tarotCard   = SelectInventory( INVENTORY_TAROT_NORMAL_BEGIN );
	TB_ITEM_TAROT* itemTarot   = NULL;
	long           tarotNumber = 0;

	for ( int i = INVENTORY_TAROT_NORMAL_BEGIN; i <= INVENTORY_TAROT_NORMAL_END; i++, tarotCard++ )
	{
		itemTarot = ITEMMANAGER->GetItemTarot( tarotCard->itemDefineIdx );
		if ( itemTarot != NULL )
		{
			if ( itemTarot->number != tarotNumber )		// 타로카드 고유번호 검사.
				throw ERROR_TAROT_READER_OPEN_CARD;		// 타로카드 고유번호 오류.
		}
		else
			throw ERROR_TAROT_READER_OPEN_CARD;			// 타로카드 오류.

		tarotNumber++;
	}

	/*-- 스프레드 검사.
	*/
	TB_INVENTORY* spread      = SelectInventory( INVENTORY_SPREAD_NORMAL_BEGIN );
	long          spreadCount = 0;

	for ( long i = INVENTORY_SPREAD_NORMAL_BEGIN; i <= INVENTORY_SPREAD_NORMAL_END; i++, spread++ )
	{
		if ( ITEMMANAGER->IsItemDefineType( spread->itemDefineIdx, ITEM_SPREAD ) )
			spreadCount++;
	}

	if ( !(spreadCount > 0) )
		throw ERROR_TAROT_READER_OPEN_SPREAD;			// 스프레드 오류.

	// 재료 아이템 수량 확인.
	mTarotItemCount = GetItemCount( TAROT_MATERIAL_ITEM );
	if ( !(mTarotItemCount > 0) )
		throw ERROR_TAROT_READER_OPEN_ITEM;				// 재료아이템 수량 오류.

    if ( msg->Price > MAX_TRADE_MONEY )
        throw ERROR_TAROT_READER_OPEN_TRADE_MONEY;

    if ( IsAddMoney( (long)msg->Price ) == false )
        throw ERROR_TAROT_READER_OPEN_HAVE_MONEY;


	// 타로상점 개설.
	SetStateStop( eSTOP_OPENTAROT );					// 캐릭터 정지상태 중 타로개설.

	// 타로상점 개설 정보 백업.
	wcscpy( mTarotTitle, msg->strTitle );				// 개설명
	mTarotPrice         = msg->Price;					// 가격
	mTarotUserCount     = 0;							// 사용 캐릭터 수 - 초기화.
	mTarotCloseReserved = false;						// 닫기예약 - 초기화.

	/*-- 타로 개설정보 보내기 [RES/SYN] 메시지 발송.
	*/
	HANDLE                     handle = NULL;
	MSG_RES_TAROT_READER_OPEN* msgRes = (MSG_RES_TAROT_READER_OPEN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_READER_OPEN_RES );
	if ( msgRes != NULL )
	{
		msgRes->ErrorCode = ERROR_TAROT_READER_OPEN_SUCCESS;
		wcscpy( msgRes->strTitle, mTarotTitle );
		msgRes->Price       = mTarotPrice;
		msgRes->UserCount   = mTarotUserCount;
		msgRes->Point       = (unsigned short)mHeroInfo.TarotPoint;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_READER_OPEN) );
	}

	MSG_SYN_TAROT_READER_OPEN msgSyn;
	memset( &msgSyn, 0, sizeof(MSG_SYN_TAROT_READER_OPEN) );

	msgSyn.Category      = NM_TAROT;
	msgSyn.Protocol      = NM_TAROT_READER_OPEN_SYN;
	msgSyn.mCharacterIdx = mObject.index;
	wcscpy( msgSyn.mTitle, mTarotTitle );
	NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
}

// TarotReaderClose Method
void cPlayer::TarotReaderClose(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_READER_CLOSE* /*msg*/)
{
	eSTOPFLAG stopFlag = (eSTOPFLAG)GetStateStop( );
	if ( !(stopFlag == eSTOP_READYTAROT || stopFlag == eSTOP_OPENTAROT) )
		throw ERROR_TAROT_READER_CLOSE_FAIL;			// 캐릭터 상태 오류.

	if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_TAROT_READER_CLOSE_FAIL;			// 캐릭터 상태 오류 - 상태변경 불가.

	if ( IsTarotCloseReserved( ) == true )
	{
		mTarotCloseReserved = false;
		throw ERROR_TAROT_READER_CLOSE_RESERVED_OFF;	// 타로상점 닫기 예약해지.
	}
	if ( IsTarotReaderClose( ) == false )
	{
		mTarotCloseReserved = true;
		throw ERROR_TAROT_READER_CLOSE_RESERVED_ON;		// 타로상점 닫기 예약.
	}

	TarotReaderClose( ERROR_TAROT_READER_CLOSE_SUCCESS );
}
void cPlayer::TarotReaderClose(int error)
{
	// 상태 변경
	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );							// 캐릭터 정지 상태 - 세부설정 없음.

	// 모든 게스트 종료.
	ReleaseTarotAllGuests( );

	/*-- 타로종료 정보 보내기 [RES/SYN] 메시지 발송.
	*/
	SendMsgResTarot( NM_TAROT, NM_TAROT_READER_CLOSE_RES, error );

	MSG_SYN_TAROT_READER_CLOSE msgSyn;
	memset( &msgSyn, 0, sizeof(MSG_SYN_TAROT_READER_CLOSE) );

	msgSyn.Category = NM_TAROT;
	msgSyn.Protocol = NM_TAROT_READER_CLOSE_SYN;
	msgSyn.mCharacterIdx = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&msgSyn, sizeof(msgSyn) );
}

// GetTarotGuest Method.
bool cPlayer::GetTarotGuest(unsigned long characterIdx)
{
	if ( mTarotGuestOffset < MAX_TAROT_GUEST )
	{
		unsigned long* tarotGuest = mTarotGuests;
		for ( int i = 0; i < MAX_STALL_GUEST; i++, tarotGuest++ )
		{
			if ( (*tarotGuest) == 0 )
			{
				(*tarotGuest) = characterIdx;
				mTarotGuestOffset++;
				return true;
			}
		}
	}
	return false;
}

// ReleaseTarotGuest Method.
bool cPlayer::ReleaseTarotGuest(unsigned long characterIdx)
{
	if ( mTarotGuestOffset > 0 )
	{
		unsigned long* tarotGuest = mTarotGuests;
		for ( int i = 0; i < MAX_TAROT_GUEST; i++, tarotGuest++ )
		{
			if ( (*tarotGuest) == characterIdx )
			{
				(*tarotGuest) = 0;
				mTarotGuestOffset--;
				return true;
			}
		}
	}
	return false;
}

// ReleaseTarotAllGuests Method.
void cPlayer::ReleaseTarotAllGuests( )
{
	unsigned long* tarotGuest = mTarotGuests;

	for ( int i = 0; i < MAX_TAROT_GUEST && mTarotGuestOffset > 0; i++, tarotGuest++ )
	{
		if ( (*tarotGuest) > 0 )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( (*tarotGuest) );

			if ( player != NULL )
			{
				player->ChangeState( eOBJECT_STATE_IDLE );
				player->SetStateStop( eSTOP_NONE );
				player->SendMsgResTarot( NM_TAROT, NM_TAROT_SEEKER_CLOSE_RES, ERROR_TAROT_SEEKER_CLOSE_READER );
			}

			(*tarotGuest) = 0;
			--mTarotGuestOffset;
		}
	}
}

// GetTarotCards Method - 타로카드 인벤토리 복사.
bool cPlayer::GetTarotCards( TB_INVENTORY* inventory[], short begin )
{
	TB_INVENTORY*  tarotCard   = SelectInventory( begin );
	TB_ITEM_TAROT* itemTarot   = NULL;

	for ( int i = 0; i < MAX_TAROT_CARDS; i++, tarotCard++ )
	{
		itemTarot = ITEMMANAGER->GetItemTarot( tarotCard->itemDefineIdx );
		if ( itemTarot != NULL )
		{
			if ( itemTarot->number == i )	// 타로카드 고유번호 검사.
			{
				inventory[ i ] = tarotCard;
			}
			else
				return false;
		}
		else
			return false;
	}

	return true;
}

// GetSpreads Method - 스프레드 인벤토리 복사.
void cPlayer::GetSpreads(TB_INVENTORY* inventory[], short begin, int& offset)
{
	TB_INVENTORY* spread = SelectInventory( begin );

	for ( long i = 0; i <= MAX_SPREAD; i++, spread++ )
	{
		if ( ITEMMANAGER->IsItemDefineType( spread->itemDefineIdx, ITEM_SPREAD ) )
		{
			inventory[ offset ] = spread;
			offset++;
		}
	}
}

// UseTarotCard Method.
void cPlayer::UseTarotCard(sObject object, long price, long itemDefineIndex)
{
	AddMoney( object, price );
	ItemUse( itemDefineIndex );

	mTarotItemCount = GetItemCount( TAROT_MATERIAL_ITEM );
}

// TarotSeekerOpen Method - 순서에 주의!.
void cPlayer::TarotSeekerOpen(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_SEEKER_OPEN* msg)
{
	// 2차상태 체크
	if ( IsRequestRejection() == true )
		throw ERROR_TAROT_SEEKER_OPEN_FAIL;			// 캐릭터 상태2오류.

	// 상태검사.
	if ( IsChangeState( eOBJECT_STATE_STOP ) == false || GetStateStop( ) != eSTOP_NONE )
		throw ERROR_TAROT_SEEKER_OPEN_FAIL;			// 캐릭터 상태오류.

	// 타로상점 - 리더검색.
	cPlayer* reader = OBJECTMANAGER->GetPlayer( msg->CharacterIdx );

	if ( reader == NULL )
		throw ERROR_TAROT_SEEKER_OPEN_READER;		// 타로상점 오류 - 캐릭터 검색 오류.

	if ( reader->GetStateStop( ) != eSTOP_OPENTAROT && reader->IsTarotCloseReserved( ) == false )
		throw ERROR_TAROT_SEEKER_OPEN_READER;		// 타로상점 오류 - 타로상점 개설 오류.

    /// 보유금액 비교해서 더이상 안받기
    __int64 totalPrice = (__int64)reader->GetMoney() + (__int64)reader->GetTarotPrice();
    if ( totalPrice > MAX_HAVE_MONEY ) 
        throw ERROR_TAROT_SEEKER_OPEN_HAVE_MONEY;

	/// 점집 닫기 예약 상태 검사.
	if( reader->IsTarotCloseReserved( ) == true )
		throw ERROR_TAROT_SEEKER_OPEN_RESERVED;	// 타로상점 닫기 예약해지.

	// 타로상점 - 게스트 등록.
	if ( reader->GetTarotGuest( mObject.index ) == false )
		throw ERROR_TAROT_SEEKER_OPEN_OVER;			// 타로상점 오류 - 최대 게스트 오버.

	if ( reader->GetTarotCards( mTarotCards, (short)INVENTORY_TAROT_NORMAL_BEGIN ) == false )
		throw ERROR_TAROT_SEEKER_OPEN_FAIL;			// 타로상점 오류 - 타로카드 복사 실패.

	reader->GetSpreads( mSpread, (short)INVENTORY_SPREAD_NORMAL_BEGIN, (mSpreadOffset=0) );
	if ( !(mSpreadOffset > 0) )
		throw ERROR_TAROT_SEEKER_OPEN_FAIL;			// 타로상점 오류 - 스프레드 복사 실패.

	// 타로상점, 리더의 캐릭터 고유번호 복사.
	mTarotReaderIdx = reader->GetObjectID( );		// 타로상점 백업 - 리더의 캐릭터 고유번호.

	// 상태변경.
	ChangeState( eOBJECT_STATE_STOP );				// 캐릭터 상태변경.
	SetStateStop( eSTOP_ENTERTAROT );				// 캐릭터 상태변경 - 세부항복.

	// 타로 개설정보 발송.
	SendTarotStoreOpen( reader->GetName( ), reader->GetTarotTitle( ), reader->GetTarotPrice( ) );
}

// TarotSeekerClose Method - 순서에 주의!.
void cPlayer::TarotSeekerClose(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_SEEKER_CLOSE* /*msg*/)
{
	// 상태검사.
	eSTOPFLAG stopFlag = (eSTOPFLAG)GetStateStop( );

	if ( !(stopFlag == eSTOP_ENTERTAROT || stopFlag == eSTOP_USETAROT) )
		throw ERROR_TAROT_SEEKER_CLOSE_FAIL;		// 캐릭터 상태오류.

	if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_TAROT_SEEKER_CLOSE_FAIL;		// 캐릭터 상태오류 - 상태변경 불가.

	// 타로상점 - 리더처리.
	cPlayer* reader = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );

	if ( reader != NULL )
	{
		if ( reader->GetStateStop( ) == eSTOP_OPENTAROT )
		{
			reader->ReleaseTarotGuest( mObject.index );	// 타로 게스트 종료.

			if ( stopFlag == eSTOP_USETAROT )
				reader->SendTarotReaderLeave( mPlayerInfo.strName );
		}
	}

	// 타로상점 백업 - 클리어.
	mTarotReaderIdx = 0;

	// 상태변경.
	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	// 타로상점 참여종료.
	SendMsgResTarot( NM_TAROT, NM_TAROT_SEEKER_CLOSE_RES, ERROR_TAROT_SEEKER_CLOSE_SUCCESS );
}

// SendTarotStoreJoin Method.
void cPlayer::TarotSeekerJoin(ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_SEEKER_JOIN* /*msg*/)
{
	// 상태검사.
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_ENTERTAROT )
		throw ERROR_TAROT_SEEKER_JOIN_FAIL;		// 캐릭터 상태오류.

	// 리더검색
	cPlayer* reader = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );

	if ( reader == NULL )
		throw ERROR_TAROT_SEEKER_JOIN_READER;		// 리더검색 오류 - 자동으로 타로상점 닫기 추가 해야됨.

	if ( reader->GetStateStop( ) != eSTOP_OPENTAROT )
		throw ERROR_TAROT_SEEKER_JOIN_READER;		// 리더검색 오류 - 리더아님.

	if ( !(reader->GetItemCount( TAROT_MATERIAL_ITEM ) > 0) )
		throw ERROR_TAROT_SEEKER_JOIN_ITEM;			// 리더검색 오류 - 재료부족.

	if ( mMoney < reader->GetTarotPrice( ) )
		throw ERROR_TAROT_SEEKER_JOIN_MONEY;		// 소지금액 부족.

    __int64 totalPrice = (__int64)reader->GetMoney() + (__int64)reader->GetTarotPrice();
    if ( totalPrice > MAX_HAVE_MONEY ) 
        throw ERROR_TAROT_SEEKER_JOIN_HAVE_MONEY;


	// 타로상점 가격 얻어오기.
	long tarotPrice = (long)reader->GetTarotPrice( );

	// 타로상점 이용금액 -.
	AddMoney( reader->GetObject( ), -tarotPrice );

	// 타로상점 이용금액 + & 아이템 사용.
	reader->UseTarotCard( mObject, +tarotPrice, TAROT_MATERIAL_ITEM );

	// 상태변경.
	SetStateStop( eSTOP_USETAROT );

	// 참여 씨커네임 전송.
	reader->SendTarotReaderJoin( mPlayerInfo.strName );

	// 참여 정보 전송.
	SendTarotStoreJoin( reader->GetTarotTitle( ), reader->GetTarotPrice( ) );
}

// TarotSeekerResult Method
void cPlayer::TarotSeekerResult(ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_RESULT* msg)
{
	// 상태검사.
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_USETAROT )
		throw ERROR_TAROT_SEEKER_RESULT_FAIL;		// 캐릭터 상태오류.

	// 리더검색
	cPlayer* reader = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );

	if ( reader == NULL )
		throw ERROR_TAROT_SEEKER_RESULT_READER;		// 리더검색 오류.

	if ( reader->GetStateStop( ) != eSTOP_OPENTAROT )
		throw ERROR_TAROT_SEEKER_RESULT_READER;		// 리더검색 오류 - 리더아님.

	// 스프레드 인덱스 검색.
	long spreadIndex = 0;
	bool exists      = false;

	// Spread 인덱스 확인.
	for ( long i = 0; i < mSpreadOffset; i++ )
	{
		if ( mSpread[ i ]->itemDefineIndex == (long)msg->SpreadIndex )
		{
			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( mSpread[ i ]->itemDefineIdx );
			if ( itemDefine != NULL )
			{
				spreadIndex = itemDefine->spreadIndex;
				exists      = true;
			}
		}
	}

	if ( exists == false )
		throw ERROR_TAROT_SEEKER_RESULT_FAIL;		// 스프레드 인덱스 오류.

	// Database 처리 요청.
	HANDLE             handle          = NULL;
	ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SPREAD_VALUE );

	itemSpreadValue->itemDefineIndex = msg->SpreadIndex;
	itemSpreadValue->spreadIndex     = spreadIndex;

	for ( long i = 0; i < MIN_TAROT_CARDS; i++ )
	{
		TB_ITEM_TAROT* tbItemTarot   = ITEMMANAGER->GetItemTarot( mTarotCards[ i ]->itemDefineIdx );
		int            dir           = (rand( ) % 2);

		itemSpreadValue->values[ i ].itemDefineIndex = mTarotCards[ i ]->itemDefineIndex;
		itemSpreadValue->values[ i ].dir             = (BYTE)(dir + 1);
		itemSpreadValue->values[ i ].tarot           = (dir == 0) ? tbItemTarot->strValue : tbItemTarot->invValue;
	}

	if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_SPREAD_VALUE) ) == false )
		throw ERROR_TAROT_SEEKER_RESULT_FAIL;		// SQL 오류.

	mTarotResultIndex = msg->SpreadIndex;
}

// TarotSeekerAccept Method
void cPlayer::TarotSeekerAccept(ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_ACCEPT* /*msg*/)
{
	// 상태검사.
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_USETAROT )
		throw ERROR_TAROT_SEEKER_ACCEPT_FAIL;		// 캐릭터 상태오류.

	else if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_TAROT_SEEKER_ACCEPT_FAIL;		// 캐릭터 상태오류 - 상태변경 불가.

	// 상태변경.
	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	// 리더검색.
	cPlayer* reader = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );

	if ( reader == NULL )
		throw ERROR_TAROT_SEEKER_ACCEPT_READER;		// 리더검색 오류.

	if ( reader->GetStateStop( ) != eSTOP_OPENTAROT )
		throw ERROR_TAROT_SEEKER_ACCEPT_READER;		// 리더검색 오류 - 리더아님.

	// 타로상점 - 게스트 종료.
	reader->ReleaseTarotGuest( mObject.index );
	reader->SendTarotReaderLeave( mPlayerInfo.strName );

	SendMsgResTarot( NM_TAROT, NM_TAROT_SEEKER_CLOSE_RES, ERROR_TAROT_SEEKER_CLOSE_SUCCESS );

	// Database 처리요청
	HANDLE             handle          = NULL;
	ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_TAROT_RESULT );

	itemTarotResult->itemDefineIndex = mTarotResultIndex;
	itemTarotResult->value           = mTarotResultValue;

	if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_TAROT_RESULT) ) == false )
		throw ERROR_TAROT_SEEKER_ACCEPT_FAIL;		// SQL 오류.
}

void cPlayer::TarotTutorialSeekerJoin()
{
	if( GetMapDataNumber() != MAP_TUTORIAL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_JOIN_FAIL;

	// 상태검사.
	if ( !(GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_NPCSPEECH) )
		throw ERROR_TAROT_TUTORIAL_SEEKER_JOIN_FAIL;		// 캐릭터 상태오류.
	
	cNpc* npc = OBJECTMANAGER->GetNpc( GetNpcIdx() );
	if( npc == NULL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_JOIN_FAIL;		// 캐릭터 상태오류.

	sNpcFuncData* funcData = NPCSCRIPT->GetNpcFuncData( npc->GetRaceGender() );
	if( funcData == NULL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_JOIN_FAIL;		// 캐릭터 상태오류.

	for ( int i = 0; i < MAX_TAROT_CARDS; i++ )
	{
		mTempNpcTarotCards[i] = funcData->mTarotIndex[i];
	}

	// 선택 카드 전송
	HANDLE								handle  = NULL;
	MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN* sendMsg = (MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_JOIN_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_TAROT_TUTORIAL_SEEKER_JOIN_SUCCESS;

		/*-- 카드섞이(shuffle).
		*/
		unsigned long temp;
		int			  s;
		for ( int i = 0; i < MAX_TAROT_CARDS; i++ )
		{
			s = (int)(rand( ) % MAX_TAROT_CARDS);	// 0 ~ 21의 램덤 수를 생성한다.
			
			temp					= mTempNpcTarotCards[ i ];	// 교환을 위해 임시 저장.
			mTempNpcTarotCards[ i ] = mTempNpcTarotCards[ s ];	// 교환 시작.
			mTempNpcTarotCards[ s ] = temp;				// 교환 완료.
		}	
		
		// 카드선택.
		for ( int i = 0; i < MIN_TAROT_CARDS; i++ )
		{
			sendMsg->TarotCardIndex[ i ] = mTempNpcTarotCards[ i ];
		}

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN) );
	}
}

void cPlayer::TarotTutorialSeekerResult( ULONG_PTR socketContext, MSG_REQ_TAROT_TUTORIAL_SEEKER_RESULT* msg )
{
	if( GetMapDataNumber() != MAP_TUTORIAL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL;

	if ( !(GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_NPCSPEECH) )
		throw ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL;		// 캐릭터 상태오류.

	cNpc* npc = OBJECTMANAGER->GetNpc( GetNpcIdx() );
	if( npc == NULL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL;		// 캐릭터 상태오류.

	sNpcFuncData* funcData = NPCSCRIPT->GetNpcFuncData( npc->GetRaceGender() );
	if( funcData == NULL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL;		// 캐릭터 상태오류.

	// 스프레드 인덱스 검색.
	long spreadIndex = 0;
	bool exists      = false;

	// Spread 인덱스 확인.
	for ( unsigned int i = 0; i < funcData->mSpreadCount; i++ )
	{
		// 동일한 아이템 인덱스이면
		if ( funcData->mSpreadIndex[i] == msg->SpreadIndex )
		{
			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( funcData->mSpreadIndex[i] );
			if ( itemDefine != NULL )
			{
				spreadIndex = itemDefine->spreadIndex;
				exists      = true;
			}
		}
	}

	if ( exists == false )
		throw ERROR_TAROT_SEEKER_RESULT_FAIL;		// 스프레드 인덱스 오류.

	// Database 처리 요청.
	HANDLE             handle          = NULL;
	ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_TUTORIALSPREAD_VALUE );

	// 두개의 인덱스는 다른 종류이다!! (정의인덱스와 db인덱스)
	itemSpreadValue->itemDefineIndex = msg->SpreadIndex;
	itemSpreadValue->spreadIndex     = spreadIndex;

	for ( long i = 0; i < MIN_TAROT_CARDS; i++ )
	{
		TB_ITEM_DEFINE* itemDefine   = ITEMMANAGER->GetItemDefineByIndex( mTempNpcTarotCards[ i ] );
		TB_ITEM_TAROT* tbItemTarot   = ITEMMANAGER->GetItemTarot( itemDefine->idx );

		if( tbItemTarot )
		{
			int            dir           = (rand( ) % 2);

			itemSpreadValue->values[ i ].itemDefineIndex = mTempNpcTarotCards[ i ];
			itemSpreadValue->values[ i ].dir             = (BYTE)(dir + 1);
			itemSpreadValue->values[ i ].tarot           = (dir == 0) ? tbItemTarot->strValue : tbItemTarot->invValue;
		}
		else
		{
			NETWORK2->PostServerEvent( "Error(%d,%d): TarotTutorialSeekerResult", mTempNpcTarotCards[ i ], itemDefine->idx );
		}
	}

	if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_SPREAD_VALUE) ) == false )
		throw ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL;		// SQL 오류.

	mTarotResultIndex = msg->SpreadIndex;
}

void cPlayer::TarotTutorialSeekerAccept( ULONG_PTR socketContext )
{
	if( GetMapDataNumber() != MAP_TUTORIAL )
		throw ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_FAIL;

	if ( !(GetState( ) == eOBJECT_STATE_STOP && GetStateStop( ) == eSTOP_NPCSPEECH) )
		throw ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_FAIL;		// 캐릭터 상태오류.

	if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
		throw ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_FAIL;		// 캐릭터 상태오류 - 상태변경 불가.

	// 상태변경.
	ChangeState( eOBJECT_STATE_IDLE );
	SetStateStop( eSTOP_NONE );

	SendMsgResTarot( NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_CLOSE_RES, ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_SUCCESS );

	// Database 처리요청
	HANDLE             handle          = NULL;
	ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_TUTORIALTAROT_RESULT );

	itemTarotResult->itemDefineIndex = mTarotResultIndex;
	itemTarotResult->value           = mTarotResultValue;

	if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_TAROT_RESULT) ) == false )
		throw ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_FAIL;		// SQL 오류.
}

int cPlayer::TarotSelfJoin( ULONG_PTR /*socketContext*/, MSG_REQ_TAROT_SELF_JOIN* /*msg*/ )
{
    if ( IsRequestRejection() == true )
        throw ERROR_TAROT_SELF_JOIN_FAIL;

    if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_SELFTAROT )
		throw ERROR_TAROT_SELF_JOIN_FAIL;

    // 점괘개선 적용시 수정해야 함....
    unsigned char tarotClass = eTAROT_CLASS_NORMAL;
    //if ( msg->tarotClass > eTAROT_CLASS_MAX )
    if ( tarotClass > eTAROT_CLASS_MAX )
        throw ERROR_TAROT_SELF_JOIN_FAIL;


    // 등급에 따른 재료아이템 및 인벤토리
    long    material    = TAROT_MATERIAL_ITEM;
    short   tarotBegin  = INVENTORY_TAROT_NORMAL_BEGIN;
    short   spreadBegin = INVENTORY_SPREAD_NORMAL_BEGIN;

    //switch ( msg->tarotClass )
    switch ( tarotClass )
    {
    case eTAROT_CLASS_NORMAL:
        material    = TAROT_MATERIAL_ITEM;
        tarotBegin  = INVENTORY_TAROT_NORMAL_BEGIN;
        spreadBegin = INVENTORY_SPREAD_NORMAL_BEGIN;
        break;
    case eTAROT_CLASS_SILVER:
        material    = TAROT_MATERIAL_ITEM_SILVER;
        tarotBegin  = INVENTORY_TAROT_SILVER_BEGIN;
        spreadBegin = INVENTORY_SPREAD_SILVER_BEGIN;
        break;
    case eTAROT_CLASS_GOLD:
        material    = TAROT_MATERIAL_ITEM_GOLD;
        tarotBegin  = INVENTORY_TAROT_GOLD_BEGIN;
        spreadBegin = INVENTORY_SPREAD_GOLD_BEGIN;
        break;
    case eTAROT_CLASS_DIA:
        material    = TAROT_MATERIAL_ITEM_DIA;
        tarotBegin  = INVENTORY_TAROT_DIA_BEGIN;
        spreadBegin = INVENTORY_SPREAD_DIA_BEGIN;
        break;
    default:
        throw ERROR_TAROT_SELF_JOIN_CLASS;
        break;
    }

	if ( !(GetItemCount( material ) > 0) )
		throw ERROR_TAROT_SELF_JOIN_METARIAL;

    if ( GetTarotCards( mTarotCards, tarotBegin ) == false )
		throw ERROR_TAROT_SELF_JOIN_FAIL;

	GetSpreads( mSpread, spreadBegin, (mSpreadOffset=0) );
	if ( !(mSpreadOffset > 0) )
		throw ERROR_TAROT_SELF_JOIN_SPREAD;


	HANDLE                   handle  = NULL;
	MSG_RES_TAROT_SELF_JOIN* sendMsg = (MSG_RES_TAROT_SELF_JOIN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_SELF_JOIN_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_TAROT_SELF_JOIN_SUCCESS;

        // 재료소모
        ItemUse( material );
        mTarotItemCount = GetItemCount( material );

		/*-- 카드섞이(shuffle).
		*/
		TB_INVENTORY* temp;
		int           s;
		for ( int i = 0; i < MAX_TAROT_CARDS; i++ )
		{
			s = (int)(rand( ) % MAX_TAROT_CARDS);	// 0 ~ 21의 램덤 수를 생성한다.

			temp             = mTarotCards[ i ];	// 교환을 위해 임시 저장.
			mTarotCards[ i ] = mTarotCards[ s ];	// 교환 시작.
			mTarotCards[ s ] = temp;				// 교환 완료.
		}

		// 카드선택.
		for ( int i = 0; i < MIN_TAROT_CARDS; i++ )
		{
			sendMsg->TarotCardIndex[ i ] = mTarotCards[ i ]->itemDefineIndex;
		}

        NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_SELF_JOIN) );

        throw ERROR_TAROT_SELF_JOIN_SUCCESS;
    }

    throw ERROR_TAROT_SELF_JOIN_FAIL;
}

int cPlayer::TarotSelfResult( ULONG_PTR socketContext, MSG_REQ_TAROT_SELF_RESULT* msg )
{
    // 상태검사
	if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_SELFTAROT )
		throw ERROR_TAROT_SELF_RESULT_FAIL;

	// 스프레드 인덱스 검색.
	long spreadIndex = 0;
	bool exists      = false;

	// Spread 인덱스 확인.
	for ( long i = 0; i < mSpreadOffset; i++ )
	{
		if ( mSpread[ i ]->itemDefineIndex == (long)msg->SpreadIndex )
		{
			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( mSpread[ i ]->itemDefineIdx );
			if ( itemDefine != NULL )
			{
				spreadIndex = itemDefine->spreadIndex;
				exists      = true;
			}
		}
	}

    // 스프레드 인덱스 오류.
	if ( exists == false )
		throw ERROR_TAROT_SELF_RESULT_FAIL;


	// Database 처리 요청.
	HANDLE             handle          = NULL;
	ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SELF_SPREAD_VALUE );

	itemSpreadValue->itemDefineIndex = msg->SpreadIndex;
	itemSpreadValue->spreadIndex     = spreadIndex;

	for ( long i = 0; i < MIN_TAROT_CARDS; i++ )
	{
		TB_ITEM_TAROT* tbItemTarot   = ITEMMANAGER->GetItemTarot( mTarotCards[ i ]->itemDefineIdx );
		int            dir           = (rand( ) % 2);

		itemSpreadValue->values[ i ].itemDefineIndex = mTarotCards[ i ]->itemDefineIndex;
		itemSpreadValue->values[ i ].dir             = (BYTE)(dir + 1);
		itemSpreadValue->values[ i ].tarot           = (dir == 0) ? tbItemTarot->strValue : tbItemTarot->invValue;
	}

	if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_SPREAD_VALUE) ) == true )
    {
    	mTarotResultIndex = msg->SpreadIndex;
        throw ERROR_TAROT_SELF_RESULT_SUCCESS;
    }

	throw ERROR_TAROT_SELF_RESULT_FAIL;
}

int cPlayer::TarotSelfAccept( ULONG_PTR socketContext )
{
    // 상태검사
    if ( GetState( ) != eOBJECT_STATE_STOP || GetStateStop( ) != eSTOP_SELFTAROT )
        throw ERROR_TAROT_SELF_ACCEPT_FAIL;

    if ( IsChangeState( eOBJECT_STATE_IDLE ) == false )
        throw ERROR_TAROT_SELF_ACCEPT_FAIL;

    // 상태변경.
    ChangeState( eOBJECT_STATE_IDLE );
    SetStateStop( eSTOP_NONE );

    // Database 처리요청
    HANDLE             handle          = NULL;
    ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_SELF_TAROT_RESULT );

    itemTarotResult->itemDefineIndex = mTarotResultIndex;
    itemTarotResult->value           = mTarotResultValue;

     if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(ITEM_TAROT_RESULT) ) == true )
        throw ERROR_TAROT_SELF_ACCEPT_SUCCESS;

     throw ERROR_TAROT_SELF_ACCEPT_FAIL;
}

// TarotReaderDie Method
void cPlayer::TarotReaderDie( )
{
	ReleaseTarotAllGuests( );
}

// TarotSeekerDie Method
void cPlayer::TarotSeekerDie( )
{
	cPlayer* player = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );
	if ( player != NULL )
		player->ReleaseTarotGuest( mObject.index );
}

// TarotReaderShutdown Method
void cPlayer::TarotReaderShutdown(bool shutdown)
{
	if ( mTarotCloseReserved == false )
		mTarotCloseReserved = true;

	if ( IsTarotReaderClose( ) == true || shutdown == true )
	{
		ReleaseTarotAllGuests( );

		ChangeState( eOBJECT_STATE_STOP );
		SetStateStop( eSTOP_GAMEFINISH );
	}
}

// TarotSeekerShutdown Method
void cPlayer::TarotSeekerShutdown(bool /*shutdown*/)
{
	cPlayer* player = OBJECTMANAGER->GetPlayer( mTarotReaderIdx );

	if ( player != NULL )
		player->ReleaseTarotGuest( mObject.index );

	ChangeState( eOBJECT_STATE_STOP );
	SetStateStop( eSTOP_GAMEFINISH );
}

// SendMsgResTarot Method
bool cPlayer::SendMsgResTarot(char category, char protocol, int error)
{
	return NETWORK2->SendMsgError( mConnectionIdx, category, protocol, error );
}

// SendTarotStoreOpen Method.
bool cPlayer::SendTarotStoreOpen(wchar_t* readerName, wchar_t* tarotTitle, unsigned long price)
{
	HANDLE                     handle  = NULL;
	MSG_RES_TAROT_SEEKER_OPEN* sendMsg = (MSG_RES_TAROT_SEEKER_OPEN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_SEEKER_OPEN_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_TAROT_SEEKER_OPEN_SUCCESS;

		// 타로카드 최대 22장.
		unsigned long* tarotCardIndex = sendMsg->TarotCardIndex;

		for ( long i = 0; i < MAX_TAROT_CARDS; i++, tarotCardIndex++ )
		{
			(*tarotCardIndex) = mTarotCards[ i ]->itemDefineIndex;
		}

		// 스프레드 최대 5장.
		unsigned long* spreadIndex = sendMsg->SpreadIndex;

		for ( long i = 0; i < mSpreadOffset; i++, spreadIndex++ )
		{
			(*spreadIndex) = mSpread[ i ]->itemDefineIndex;
		}

		wcscpy( sendMsg->Name, readerName );
		wcscpy( sendMsg->Title, tarotTitle );
		sendMsg->Price = price;

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_SEEKER_OPEN) );
	}
	return false;
}

// SendTarotStoreOpen Method.
bool cPlayer::SendTarotReaderJoin(wchar_t* seekerName)
{
	HANDLE                     handle  = NULL;
	MSG_RES_TAROT_READER_JOIN* sendMsg = (MSG_RES_TAROT_READER_JOIN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_READER_JOIN_RES );
	if ( sendMsg != NULL )
	{
		wcscpy( sendMsg->seekerName, seekerName );
		sendMsg->userCount   = (++mTarotUserCount);
		sendMsg->point       = (unsigned short)(++mHeroInfo.TarotPoint);
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_READER_JOIN) );
	}
	return false;
}

// SendTarotReaderLeave Method.
bool cPlayer::SendTarotReaderLeave(wchar_t* seekerName)
{
	HANDLE                      handle  = NULL;
	MSG_RES_TAROT_READER_LEAVE* sendMsg = (MSG_RES_TAROT_READER_LEAVE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_READER_LEAVE_RES );
	if ( sendMsg != NULL )
	{
		wcscpy( sendMsg->seekerName, seekerName );
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_READER_LEAVE) );
	}
	return false;
}

// SendTarotStoreJoin Method.
bool cPlayer::SendTarotStoreJoin(wchar_t* tarotTitle, unsigned long price)
{
	HANDLE                     handle  = NULL;
	MSG_RES_TAROT_SEEKER_JOIN* sendMsg = (MSG_RES_TAROT_SEEKER_JOIN*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TAROT, NM_TAROT_SEEKER_JOIN_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = ERROR_TAROT_SEEKER_JOIN_SUCCESS;

		/*-- 카드섞이(shuffle).
		*/
		TB_INVENTORY* temp;
		int           s;
		for ( int i = 0; i < MAX_TAROT_CARDS; i++ )
		{
			s = (int)(rand( ) % MAX_TAROT_CARDS);	// 0 ~ 21의 램덤 수를 생성한다.

			temp             = mTarotCards[ i ];	// 교환을 위해 임시 저장.
			mTarotCards[ i ] = mTarotCards[ s ];	// 교환 시작.
			mTarotCards[ s ] = temp;				// 교환 완료.
		}

		// 카드선택.
		for ( int i = 0; i < MIN_TAROT_CARDS; i++ )
		{
			sendMsg->TarotCardIndex[ i ] = mTarotCards[ i ]->itemDefineIndex;
		}

		wcscpy( sendMsg->strTitle, tarotTitle );
		sendMsg->Price = price;

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_TAROT_SEEKER_JOIN) );
	}

	return false;
}

// ClearFriends Method.
void cPlayer::ClearFriends( )
{
	memset( mFriends, 0, sizeof(mFriends) );
}

// SelectFriend Method.
TB_FRIEND* cPlayer::SelectFriend(short offset)
{
	return (offset < MAX_FRIEND) ? (mFriends+offset) : NULL;
}

bool cPlayer::UpdateFriend( )
{
	TB_FRIEND* friends = SelectFriend( );
	cPlayer*   player  = NULL;
	bool       retcode = false;

	for ( long i = 0; i < MAX_FRIEND && friends->idx > 0; i++, friends++ )
	{
		if ( friends->block == 0 )
		{
			if ( friends->channelNum == 0 )
			{
				// 게임중 친구확인 - 신규접속 검토.
				player = OBJECTMANAGER->GetPlayer( friends->characterIdx );
				if ( player != NULL )
				{
					friends->level  = player->GetLevel( );
					friends->job    = (unsigned long)player->GetJob( );
					friends->mapNum = player->GetMapDataNumber( );
					friends->channelNum = NETWORK2->GetChannelNum();
				}
				
				retcode = true;
			}
			else
			{
				// 게임중 친구확인 - 변경내용 검토.
				player = OBJECTMANAGER->GetPlayer( friends->characterIdx );
				if ( player != NULL )
				{
					if ( friends->level != player->GetLevel( ) )
					{
						friends->level = player->GetLevel( );
						retcode = true;
					}
					if ( friends->job != (unsigned long)player->GetJob( ) )
					{
						friends->job = (unsigned long)player->GetJob( );
						retcode = true;
					}
					if ( friends->mapNum != player->GetMapDataNumber( ) )
					{
						friends->mapNum = player->GetMapDataNumber( );
						retcode = true;
					}
				}
				else
				{
					retcode = true;
				}
			}
		}
	}
	return retcode;
}

// SendFriendList Method.
void cPlayer::SendFriendList(ULONG_PTR socketContext)
{
	HANDLE               handle  = NULL;
	MSG_RES_FRIEND_LIST* sendMsg = (MSG_RES_FRIEND_LIST*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_CHAT, NM_CHAT_FRIEND_LIST_RES );
	unsigned long        length  = sizeof(MSG_RES_FRIEND_LIST) - sizeof(sendMsg->table);

	TB_FRIEND*           friends = SelectFriend( );

	sendMsg->RowCount = 0;
	for ( long i = 0; i < 50 && friends->idx > 0; i++, friends++ )
	{
		sendMsg->table[ i ].Idx          = friends->idx;
		sendMsg->table[ i ].CharacterIdx = friends->characterIdx;
		wcscpy( sendMsg->table[ i ].CharacterName, friends->characterName );
		sendMsg->table[ i ].Race         = friends->race;
		sendMsg->table[ i ].Gender       = friends->gender;
		sendMsg->table[ i ].Status       = friends->status;
		sendMsg->table[ i ].Level        = friends->level;
		sendMsg->table[ i ].Job          = (unsigned short)friends->job;
		sendMsg->table[ i ].MapNum       = (short)friends->mapNum;
		sendMsg->table[ i ].ChannelNum   = friends->channelNum;
		sendMsg->table[ i ].Block		 = friends->block;

		sendMsg->RowCount++;
	}

	length += (sendMsg->RowCount * sizeof(sendMsg->table));
	NETWORK2->SendMsgRoot( handle, length );
}

// 접속 관련 정보 알림
void cPlayer::SendFriendConnect( BYTE connect )
{
	// 자신의 친구목록에게만 전송
	TB_FRIEND*		friends = SelectFriend();
	for( long i = 0; i < 50 && friends->idx > 0; ++i, ++friends )
	{
		// 차단하지 않은 유저에게만 전송
		if( friends->block == 0 )
		{
			HANDLE	 handle = NULL;
			cPlayer* player = OBJECTMANAGER->GetPlayer( friends->characterIdx );
			if( player != NULL )
			{
				MSG_SYN_FRIEND_CONNECT*	sendMsg = (MSG_SYN_FRIEND_CONNECT*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), NM_CHAT, NM_CHAT_FRIEND_CONNECT_SYN )	;
				if( sendMsg != NULL )
				{
					sendMsg->characterIndex = mObject.index;
					sendMsg->connect		= connect;
					sendMsg->channelNum		= (connect == 1) ? NETWORK2->GetChannelNum() : 0;
					sendMsg->mapNum			= GetMapDataNumber();
					sendMsg->level			= GetLevel();
					sendMsg->job			= GetJob();
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_FRIEND_CONNECT) );
				}
			}
		}
	}
}

void cPlayer::SyncFriendConnect( long characterIdx, char level, unsigned long job, unsigned short mapnum, BYTE connect, DWORD channelNum )
{
	TB_FRIEND*		friends = SelectFriend();
	for( long i = 0; i < 50 && friends->idx > 0; ++i, ++friends )
	{
		if( friends->characterIdx == characterIdx )
		{
			friends->channelNum = ( connect == 1 ) ? channelNum : 0;
			friends->level		= level;
			friends->job		= job;
			friends->mapNum		= mapnum;

			//
			HANDLE	 handle = NULL;
			MSG_SYN_FRIEND_CONNECT*	sendMsg = (MSG_SYN_FRIEND_CONNECT*)NETWORK2->GetMsgRoot( &handle, GetConnectionIdx(), NM_CHAT, NM_CHAT_FRIEND_CONNECT_SYN );
			if( sendMsg != NULL )
			{
				sendMsg->characterIndex = characterIdx;
				sendMsg->connect		= connect;
				sendMsg->channelNum		= friends->channelNum;
				sendMsg->mapNum			= friends->mapNum;
				sendMsg->level			= friends->level;
				sendMsg->job			= friends->job;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_FRIEND_CONNECT) );
			}
			break;
		}
	}
}

void cPlayer::PostFriendConnect( BYTE connect )
{
	// 채널 동기화
	cSender* sender = g_gameSrv->GetSender();

	MSG_SYN_POSTFRIEND_CONNECT sync;
	memset( &sync, 0, sizeof(MSG_SYN_POSTFRIEND_CONNECT) );

	sync.Category		= NM_CHAT;
	sync.Protocol		= NM_FRIENDPOST_CONNECT_SYN;
	sync.characterIdx	= mObject.index;
	sync.connect		= connect;
	sync.job			= GetJob();
	sync.level			= GetLevel();
	sync.mapNum			= GetMapDataNumber();

	long*		list = sync.friendlist;
	TB_FRIEND*	friends = SelectFriend();
	for( long i = 0; i < 50 && friends->idx > 0; ++i, ++friends, ++list )
	{
		// 차단하지 않은 유저만 전송
		if( friends->block == 0 )
		{
			*list = friends->characterIdx;
		}
	}

	sender->PostFriendSync( NETWORK2->GetCID(), (char*)&sync, sizeof(MSG_SYN_POSTFRIEND_CONNECT) );
}

// TrialOpenReq Method
int cPlayer::TrialOpenReq( ULONG_PTR socketContext, BYTE slevel, cPlayer* pTarget )
{
    if ( pTarget == NULL )
        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

    if( GetLevel() < 10 )
        throw ERROR_PLAYER_TRIAL_OPEN_LEVEL;

    //마을, 테마, 전장, 튜토리얼에서 사용 불가능
    if ( !(mMapNumber > MAP_MIN && mMapNumber <= MAP_MAX) )
        throw ERROR_PLAYER_TRIAL_OPEN_MAP;

    // GM제외 체크 사항
    if ( slevel != 1 )
    {
        // 신고 재사용 시간
        if ( mTrialTime > 0 )
        {
            time_t current;
            time( &current );
            if ( difftime( mTrialTime, current ) > 0 )
                throw ERROR_PLAYER_TRIAL_OPEN_TIME;
        }
    }

    // 상태
    if( pTarget->GetState() == eOBJECT_STATE_DIE || pTarget->GetState() == eOBJECT_STATE_STOP )
        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

    // 맵이동
    if( pTarget->IsMapChange() == true )
        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

    // 결투 중
    if( pTarget->GetDuelIdx() != 0 || GetDuelIdx() != 0 )
        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;


    // 신고 거부 시간
    if ( pTarget->GetTrialRejectTime() > 0 )
    {
        time_t current;
        time( &current );
        if ( difftime( pTarget->GetTrialRejectTime(), current ) > 0 )
            throw ERROR_PLAYER_TRIAL_OPEN_TRIAL_REJECT;
    }

    // 상대 또는 내가 신고당한 상태이면 신고 불가능
    if( pTarget->IsAutoTrial() == true || IsAutoTrial() == true )
        throw ERROR_PLAYER_TRIAL_OPEN_TRIAL;

    // 몬스터 데미지 입힌지 10초 이상이면 신고 불가능
    unsigned long diffTime = NETWORK2->GetAccumTime() - pTarget->GetLastAttackTime();
    if( diffTime > 10 * SECOND )
        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;


    // 재사용 신고 체크
    HANDLE        handle     = NULL;
    MEMBER_TRIAL* trialCheck = (MEMBER_TRIAL*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_TRIAL_CHECK );
    if ( trialCheck != NULL )
    {
        sPlayerInfo* pTargetInfo = pTarget->GetPlayerInfo();

        trialCheck->memberIdx        = mConnectionIdx;
        trialCheck->autoCharacterIdx = (pTargetInfo->CharacterIdx > 0) ? pTargetInfo->CharacterIdx : 0; 
        trialCheck->retvalue         = 0;

        if ( NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(MEMBER_TRIAL), COMMON_DB_MEMBER_TRIAL ) == false )
            throw ERROR_PLAYER_TRIAL_OPEN_FAIL;
    }

    // 신고 상태 설정
    pTarget->IsAutoTrial( true );
    throw ERROR_PLAYER_TRIAL_OPEN_SUCCESS;
}

// SendTrialReqResult Method
void cPlayer::SendTrialReqResult( ULONG_PTR socketContext, int error, cPlayer* pTarget )
{
    HANDLE  handle = NULL;
    MSG_RES_PLAYER_TRIAL_OPEN* sendMsg = (MSG_RES_PLAYER_TRIAL_OPEN*)NETWORK2->GetMsgRoot( &handle, 
                                                                                           (PerSocketContext*)socketContext, 
                                                                                           NM_PLAYER, 
                                                                                           NM_PLAYER_TRIAL_OPEN_RES );
    if ( sendMsg == NULL )
        return;

    /// 신고자 재사용 남은 시간
    if ( error == ERROR_PLAYER_TRIAL_OPEN_TIME )
    {
        time_t current;
        time( &current );
        sendMsg->remainTime = (long)difftime( mTrialTime, current ) * 1000;
    }
    /// 신고 거부 남은 시간
    else if ( error == ERROR_PLAYER_TRIAL_OPEN_TRIAL_REJECT )
    {
        if ( pTarget )
        {
            time_t current;
            time( &current );
            sendMsg->remainTime = (long)difftime( pTarget->GetTrialRejectTime(), current ) * 1000;
        }
        else
        {
            NETWORK2->PostServerEvent( "Error(%d): cPlayer::SendTrialReqResult() - pTarget == NULL", error );
        }
    }

    // 신고 실패이면, 심판 상태를 변경해야 한다.
    if ( error != ERROR_PLAYER_TRIAL_OPEN_SUCCESS )
    {
        if ( pTarget && pTarget->GetTrialMemberIdx() == 0 )
            pTarget->IsAutoTrial( false );
    }

    sendMsg->ErrorCode  = error;
    NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TRIAL_OPEN) );
}

// TrialAnswer Method
int cPlayer::TrialAnswer( ULONG_PTR /*socketContext*/, wchar_t* answer )
{
    if( IsAutoTrial() == false )
        return ERROR_PLAYER_TRIAL_ANSWER_FAIL;

    if( wcslen(answer) != MAX_TRIAL_ANSWER_LEN )
        return ERROR_PLAYER_TRIAL_ANSWER_TEXTLEN;

    /// 심판 시간 초과
    if( mTrialAnswerTime < NETWORK2->GetAccumTime() )
        return ERROR_PLAYER_TRIAL_ANSWER_TIME_OVER;

    /// 정답
    if( wcscmp( answer, mCaptchaName ) == 0 )
    {
        return ERROR_PLAYER_TRIAL_ANSWER_SUCCESS;
    }
    /// 오답
    else
    {
        mTrialAnswerCount--;

        /// 3번 모두 실패 - 오토 판정
        if( mTrialAnswerCount == 0 )
            mAutoPlayer = true;

        return ERROR_PLAYER_TRIAL_ANSWER_WRONG;
    }
}

// TrialStart Method
void cPlayer::TrialStart( unsigned long trialPlayerIdx, wchar_t* answer )
{
    // 재사용 시간 3분 15초(클라이언트와 텀을 10초 여유 적용)
    mTrialAnswerTime    = NETWORK2->GetAccumTime() + TRIAL_ANSWER_TIME + (10 * SECOND);
    mTrialAnswerCount   = MAX_TRIAL_ANSWER_COUNT;
    mCaptchaCount       = MAX_TRIAL_CAPTCHA_COUNT;
    mTrialMemberIdx     = trialPlayerIdx;

    ::wcsncpy( mCaptchaName, answer, MAX_TRIAL_ANSWER_LEN );
    IsAutoTrial( true );
}

// TrialEnd Method
void cPlayer::TrialEnd( bool bAuto )
{
    int result = ERROR_PLAYER_TRIAL_RESULT_AUTO;

    // 오토 유저 판정
    if ( bAuto == true )
    {
        // 로그인 제한 설정(DB)
		AutoLimit( );
        result = ERROR_PLAYER_TRIAL_RESULT_AUTO;
    }
    // 오토 유저 아님
    else
    {
        result = ERROR_PLAYER_TRIAL_RESULT_NOT_AUTO;

        HANDLE          handle = NULL;
        TRIAL_REJECT*   trialReject  = (TRIAL_REJECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_TRIAL_REJECT_UPDATE );
        trialReject->memberIdx  = mConnectionIdx;
        trialReject->rejectTime = 300;
        trialReject->retvalue   = 0;

        NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(CAPTCHA_SELECT), COMMON_DB_MEMBER_TRIAL );
    }
   
	HANDLE          handle  = NULL;
	MSG_SYN_PLAYER_TRIAL_RESULT* sendMsg = (MSG_SYN_PLAYER_TRIAL_RESULT*)NETWORK2->GetMsgRoot( &handle, mTrialMemberIdx, NM_PLAYER, NM_PLAYER_TRIAL_RESULT_SYN );
    if ( sendMsg != NULL )
    {
        ::wcsncpy( sendMsg->name, GetName( ), MAX_NAME_BUFFER_SIZE );
        sendMsg->ErrorCode = result;

        NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_TRIAL_RESULT) );
    }

    // 오토 유저 아님
    mAutoPlayer         = false;
    mTrialAnswerTime    = 0;
    mTrialAnswerCount   = 0;
    mTrialMemberIdx     = 0;
    mCaptchaCount       = 0;

    memset( mCaptchaName, 0x00, sizeof(mCaptchaName) );

    /// 심판 상태 풀어줌
    IsAutoTrial( false );
}

void cPlayer::AutoLimit()
{
	// 로그인 제한 설정(DB)
	HANDLE          handle      = NULL;
	LOGIN_LIMIT*    loginLimit  = (LOGIN_LIMIT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_LOGIN_LIMIT );
	loginLimit->memberIdx = mConnectionIdx;
	loginLimit->count     = 1;                  // 제재 카운트 1씩 증가
	loginLimit->retvalue  = 0;

	NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(LOGIN_LIMIT) );
	//NETWORK2->SendSQL( mConnectionIdx, handle, sizeof(LOGIN_LIMIT), COMMON_DB_MEMBER_LOGIN_LIMIT );
}

// SendPlayerInfo Method - [cGameProcess/CallbackComplete/SocketManager]에서 호출.
bool cPlayer::SendPlayerInfo(char category, char protocol, ULONG_PTR perSocketContext, unsigned char inDunType )
{
	HANDLE          handle  = NULL;
	MSG_PLAYERINFO* sendMsg = (MSG_PLAYERINFO*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)perSocketContext, category, protocol );

	sendMsg->ErrorCode  = 0;
	sendMsg->PlayerInfo = mPlayerInfo;
	sendMsg->WeaponInfo = mPlayerWeaponInfo;
	sendMsg->WearInfo   = mPlayerWearInfo;
	sendMsg->Exrinfo    = mPlayerExrInfo;
	sendMsg->MapNum     = GetMapDataNumber();
	sendMsg->MapMode	= mThemeMode;
	sendMsg->XPos       = mObjectPos.x;
	sendMsg->YPos       = mObjectPos.y;
	sendMsg->RotAngle	= mMapTargetPos.mRotAngle;
	sendMsg->IndunType  = inDunType;

	return NETWORK2->SendMsgRoot( handle, sizeof(MSG_PLAYERINFO) );
}

// SendSightIn Method - [cGridManager/SendPlayerGameIn/SendSightInPlayer]에서 호출.
bool cPlayer::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	/// 클라이언트 전송 수치 계산
	mPlayerExrInfo.mMoveSpeed = GetMoveSpeed();
	mPlayerExrInfo.mAttackSpeed = GetStatus2()->mAttackSpeed;

	/// 장비관련 스텟 계산전이라 db에 기록됐던 값이 더 클수있다 
	if( mPlayerExrInfo.MaxHP < mInitRestHP )
		mPlayerExrInfo.MaxHP = mInitRestHP;

	if( mPlayerExrInfo.MaxMP < mInitRestMP )
		mPlayerExrInfo.MaxMP = mInitRestMP;

	HANDLE          handle  = NULL;
	MSG_PLAYERINFO* sendMsg = (MSG_PLAYERINFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode  = 0;
		sendMsg->PlayerInfo = mPlayerInfo;
		sendMsg->WeaponInfo = mPlayerWeaponInfo;
		sendMsg->WearInfo   = mPlayerWearInfo;
		sendMsg->Exrinfo    = mPlayerExrInfo;
		sendMsg->MapNum     = GetMapDataNumber();
		sendMsg->MapMode	= mThemeMode;
		sendMsg->XPos       = mObjectPos.x;
		sendMsg->YPos       = mObjectPos.y;
		sendMsg->RotAngle	= mDirection;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_PLAYERINFO) );
	}

	SKILLMANAGER->SendAddSynMsgInfluenceToPlayer( connectionIdx, mPlayerInfo.CharacterIdx );

	if ( mPlayerExrInfo.mState == eOBJECT_STATE_MOVE )
	{
		HANDLE        handle  = NULL;
		MSG_SYN_MOVE* sendMsg = (MSG_SYN_MOVE*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_PLAYER, NM_PLAYER_MOVE_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->characterIdx = mObject.index;
			sendMsg->destX        = mGotoX;
			sendMsg->destY        = mGotoY;
			sendMsg->moveSpeed    = GetMoveSpeed();
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_MOVE) );
		}
	}
	else if ( mPlayerExrInfo.mState == eOBJECT_STATE_PUSHPULL )
	{
		HANDLE        handle  = NULL;
		MSG_SYN_SKLL_PUSHPULL* sendMsg = (MSG_SYN_SKLL_PUSHPULL*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_SKILL, NM_SKILL_PUSHPULL_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mTarget = mObject;
			sendMsg->mPosX = mGotoX;
			sendMsg->mPosY = mGotoY;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKLL_PUSHPULL) );
		}
	}

	if ( mPlayerExrInfo.mStateStop == eSTOP_OPENTAROT )
	{
		HANDLE                     handle  = NULL;
		MSG_SYN_TAROT_READER_OPEN* sendMsg = (MSG_SYN_TAROT_READER_OPEN*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_TAROT, NM_TAROT_READER_OPEN_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mCharacterIdx = mObject.index;
			wcscpy( sendMsg->mTitle, mTarotTitle );
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TAROT_READER_OPEN) );
		}
	}
	if ( mPlayerExrInfo.mStateStop == eSTOP_OPENSTALL )
	{
		HANDLE                        handle  = NULL;
		MSG_SYN_ITEM_STALL_SELL_OPEN* sendMsg = (MSG_SYN_ITEM_STALL_SELL_OPEN*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_ITEM, NM_ITEM_STALL_SELL_OPEN_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->characterIdx = mObject.index;
			wcscpy( sendMsg->title, mStallSellTitle );
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_ITEM_STALL_SELL_OPEN) );
		}
	}
	return false;
}

// SendSightOut Method - [cGridManager::SendPlayerGameOut/SendSightOutPlayer]에서 호출.
bool cPlayer::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE            handle  = NULL;
	MSG_CHARACTERIDX* sendMsg = (MSG_CHARACTERIDX*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mCharacterIdx = mObject.index;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_CHARACTERIDX) );
	}

	return false;
}

bool cPlayer::SendHeroInfo()
{
	HANDLE        handle  = NULL;
	MSG_HEROINFO* sendMsg = (MSG_HEROINFO*)NETWORK2->NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_HEROINFO_RES );
	if ( sendMsg != NULL )
	{
		/// 장비관련 스텟 계산전이라 db에 기록됐던 값이 더 클수있다 
		if( mPlayerExrInfo.MaxHP < mInitRestHP )
			mPlayerExrInfo.MaxHP = mInitRestHP;

		if( mPlayerExrInfo.MaxMP < mInitRestMP )
			mPlayerExrInfo.MaxMP = mInitRestMP;

		sendMsg->mHeroInfo      = mHeroInfo;
		sendMsg->mExrInfo       = mPlayerExrInfo;
		sendMsg->mCanTMove      = GetStateOddity( eODDITYTYPE_CANTMOVE ) != 0;
		sendMsg->mCanTSkillPhy  = GetStateOddity( eODDITYTYPE_CANTSKILL_PHY ) != 0;
		sendMsg->mCanTSkillMag  = GetStateOddity( eODDITYTYPE_CANTSKILL_MAG ) != 0;
		sendMsg->mSleep         = GetStateOddity( eODDITYTYPE_SLEEP ) != 0;
		sendMsg->mStun          = GetStateOddity( eODDITYTYPE_STUN ) != 0;
		sendMsg->mAvoidMonLook  = GetStateOddity( eODDITYTYPE_AVOID_MON_LOOK ) != 0;
		sendMsg->mChgMonsterIdx = mPlayerExrInfo.mChgMonsterIdx;

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_HEROINFO) );
	}
	return false;
}

/// 배열인덱스, DB 고유인덱스
int cPlayer::IsKeepQuest( int arrIdx, unsigned long questIdx )
{
	if( arrIdx < 0 || arrIdx >= MAX_KEEPQUEST )
		return -1;

	if( mQuest[arrIdx].questIdx == questIdx )
		return mQuest[arrIdx].idx;
	return -1;
}

int cPlayer::IsKeepQuest( unsigned long questIdx )
{
	for( unsigned int i = 0; i < MAX_KEEPQUEST; ++i )
	{
		if( mQuest[i].idx == 0 )
			return -1;

		if( mQuest[i].questIdx == questIdx )
			return i;
	}
	return -1;
}

/// 해당 퀘스트와 동일한 그룹의 퀘스트를 보유중인지 검사
int cPlayer::IsKeepGroupQuest( unsigned long questIdx )
{
	for( unsigned int i = 0; i < MAX_KEEPQUEST; ++i )
	{
		if( mQuest[i].idx == 0 )
			return -1;

		if( mQuest[i].questIdx == questIdx )
			return i;

		/// 동일한 그룹의 퀘스트 보유중이면 리턴
		cQuestDefine* keepDefine = QUESTMAN->GetQuestDefine( mQuest[i].questIdx );
		cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
		if( keepDefine && define )
		{
			if( define->mGroup > 0 && define->mGroup == keepDefine->mGroup )
				return i;
		}
	}
	return -1;
}

/// 퀘스트 리스트 빈자리 찾기
int cPlayer::FindEmptyQuest()
{
	for( unsigned int i = 0; i < MAX_KEEPQUEST; ++i )
	{
		if( mQuest[i].idx == 0 && mQuest[i].questIdx == 0 )
			return i;
	}
	return -1;
}

///
bool cPlayer::IsCompleteQuest( unsigned long questIdx, bool checkGroup )
{
	cCompleteQuestSet::cIterator i = mCompleteQuestSet.Find( questIdx );
	if( i != mCompleteQuestSet.End() )
		return true;

	if( checkGroup )
	{
		/// 동일한 그룹의 퀘스트를 완료하였는지 검사
		cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
		if( define )
		{
			cCompleteGroupQuestSet::cIterator i = mCompleteGroupQuestSet.Find( define->mGroup );
			if( i != mCompleteGroupQuestSet.End() )
				return true;
		}
	}
	return false;
}

//
bool cPlayer::AddCompleteQuest( unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		//NETWORK2->PostServerEvent( "Error(%d): QUEST AddValidQuest() - failed to search questIdx", questIdx );
		return false;
	}

	/// 반복퀘도 있으므로 에러체크하지 않는다
	if( mCompleteQuestSet.Insert( questIdx ) == false )
		return false;

	/// 완료 그룹정보도 저장
	if( define->mGroup > 0 )
		mCompleteGroupQuestSet.Insert( define->mGroup );
	return true;
}

///
bool cPlayer::AddValidQuest( long questIdx, TIMESTAMP_STRUCT valid )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		//NETWORK2->PostServerEvent( "Error(%d): QUEST AddValidQuest() - failed to search questIdx", questIdx );
		return false;
	}

	if( define->mEventType == eQUEST_TIMEEVENT && valid.year != 0 )
	{
		if( mValidQuestMap.Insert( questIdx, valid ) == false )
			return false;
		
		if( define->mGroup > 0 )
			mValidGroupQuestSet.Insert( define->mGroup );
	}
	return true;
}

bool cPlayer::IsValidEndQuest( long questIdx )
{
	cValidQuestMap::cIterator i = mValidQuestMap.Find( questIdx );
	if( i != mValidQuestMap.End() )
	{
		/// 만기일목록에 존재하면, 시간이 되었는지 검사
		TIMESTAMP_STRUCT valid = (*i).mSecond;

		long diff = TIMER->DiffRealTime( valid );
		if( diff > 0 )
			return false;

		/// 만기일이 완료된 정보는 삭제 한다
		mValidQuestMap.Erase( questIdx );

		cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
		if( define && define->mGroup > 0 )
			mValidGroupQuestSet.Erase( define->mGroup );
	}
	return true;
}

void cPlayer::DeleteQuest( int arrIdx )
{
	if( arrIdx < 0 || arrIdx >= MAX_KEEPQUEST )
	{
		NETWORK2->PostServerEvent( "Error(%d,%d): QUEST DeleteQuest() ", mObject.index, arrIdx );
		return;
	}

	TB_QUEST_PROGRESS* delQuest = mQuest + arrIdx;
	memset( delQuest, 0, sizeof(TB_QUEST_PROGRESS) );

	/// 이동시키기
	TB_QUEST_PROGRESS* quest1 = mQuest + arrIdx;
	TB_QUEST_PROGRESS* quest2 = 0;
	for( int i = arrIdx; i < MAX_KEEPQUEST - 1; ++i, quest1++ )
	{
		quest2 = quest1 + 1;
		*quest1 = *quest2;
	}

	TB_QUEST_PROGRESS* lastQuest = mQuest + MAX_KEEPQUEST - 1;
	memset( lastQuest, 0, sizeof(TB_QUEST_PROGRESS) );
}

/// 퀘스트 목록 발송 
bool cPlayer::SendQuest( ULONG_PTR socketContext )
{
	HANDLE				 handle = NULL;
	MSG_RES_QUEST_LIST*	 sendMsg = (MSG_RES_QUEST_LIST*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_QUEST, NM_QUEST_LIST_RES );

	sQuestList*        questlist = sendMsg->table;
	unsigned long      length    = sizeof(MSG_RES_QUEST_LIST) - sizeof(sendMsg->table);
	TB_QUEST_PROGRESS* data      = mQuest;

	sendMsg->rowCount = 0;
	for( int i = 0; i < MAX_KEEPQUEST; ++i )
	{
		if( data->idx == 0 )
			break;

		questlist->arrIdx		= i;
		questlist->questIdx		= data->questIdx;
		questlist->check		= data->check;
		questlist->status		= data->status;
		questlist->restTime		= 0;

		/// 남은시간 재계산
		cQuestDefine* define = QUESTMAN->GetQuestDefine( data->questIdx );
		if( define && define->mQuestLimit && define->mQuestLimit->mTimeType != eTIME_NONE )
		{
			long accTime = NETWORK2->GetAccumTime();
			long restTime = data->restTime - (accTime - data->startTime);
			questlist->restTime = ( restTime > 0 ) ? restTime : 0;
		}

		questlist++;
		sendMsg->rowCount++;
		data++;
	}

	length += (sendMsg->rowCount * sizeof(sendMsg->table));
	return NETWORK2->SendMsgRoot( handle, length );
}

/// DB에 퀘스트 진행정보 저장
bool cPlayer::SaveQuestProgress(ULONG_PTR /*socketContext*/)
{
	HANDLE        handle      = NULL;
	QUEST_UPDATE* questUpdate = (QUEST_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_UPDATE );

	if ( questUpdate != NULL )
	{
		TB_QUEST_PROGRESS* table	   = questUpdate->table;
		int                length      = sizeof(QUEST_UPDATE) - sizeof(questUpdate->table);

		questUpdate->characterIdx = GetObjectID();

		TB_QUEST_PROGRESS* quest = mQuest;
		for( long i = 0; i < MAX_KEEPQUEST; i++, quest++ )
		{
			if( quest->idx > 0 )
			{
				/// 진행 상태인것만 저장한다
				if( quest->status != eQUEST_PLAYING )
					continue;

				cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
				if( define )
				{
					(*table) = (*quest);

					/// 시간 제한퀘스트 - 게임시간 일 경우 시간 검사
					/// 게임시간 - 남은시간 저장하기
					cQuestLimit* limit = define->mQuestLimit;
					if( limit && limit->mTimeType == eTIME_GAME )
					{
						long accTime = NETWORK2->GetAccumTime();
						long restTime = table->restTime - (accTime - table->startTime);
						table->restTime = ( restTime > 0 ) ? restTime : 0;
					}

					table++;
					questUpdate->rowCount++;
				}
			}
			else
				break;
		}
		length += (questUpdate->rowCount * sizeof(questUpdate->table));
		return NETWORK2->SendSQL( handle, length );
	}
	return false;
}

int cPlayer::IsRegistQuest( eQUESTADD_TYPE type, unsigned long questIdx, unsigned long npcIdx )
{
	/// 퀘스트번호 유효성 검사
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
		return ERROR_QUEST_ADD_FAIL;

	/// NPC에 의한 퀘스트일경우, NPC 검사
	if( type == eQUESTADD_NPC )
	{
		cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIdx );
		if( !pNpc )
			return ERROR_QUEST_ADD_FAIL;

		unsigned int npcId = pNpc->GetRaceGender();

		/// 실제로 npc가 소유한 퀘스트인지 검사
		if( !QUESTMAN->IsHaveQuest( questIdx, npcId ) )
			return ERROR_QUEST_ADD_NPC;
	}

	/// 완전 완료된 퀘스트인지 검사 (반복퀘 제외)
	if( define->mRepeatType != eQUEST_REPEAT )
	{
		/// 해당퀘와 그룹퀘가 완료되었는지 검사 (반복퀘 제외)
		if( IsCompleteQuest( questIdx ) )
			return ERROR_QUEST_ADD_COMPLETE;
	}

	/// 해당 퀘와 그룹퀘가 만기일이 지났는지 검사
	if( define->mEventType == eQUEST_TIMEEVENT )
	{
		if( IsValidEndQuest( questIdx ) == false )
			return ERROR_QUEST_ADD_TIME;
	}

	/// 퀘스트 목록 갯수 검사
	if( FindEmptyQuest() == -1 )
		return ERROR_QUEST_ADD_MAX;

	/// 가지고 있는 퀘스트인지 검사.
	if( IsKeepGroupQuest( questIdx ) != -1 )
		return ERROR_QUEST_ADD_EXIST;

	/// 지불할 금액이 있을시 금액 검사
	cQuestLimit* limit = define->mQuestLimit;
	if( limit )
	{
		if( limit->mTakeMoney > 0 )
		{
			if( GetMoney() < limit->mTakeMoney )
				return ERROR_QUEST_ADD_MONEY;
		}

		if( limit->mTakeFire > 0 )
		{
			if( mHeroInfo.mFirePoint < limit->mTakeFire )
				return ERROR_QUEST_ADD_POINT;
		}

		if( limit->mTakeWater > 0 )
		{
			if( mHeroInfo.mWaterPoint < limit->mTakeWater )
				return ERROR_QUEST_ADD_POINT;
		}

		if( limit->mTakeWind > 0 )
		{
			if( mHeroInfo.mWindPoint < limit->mTakeWind )
				return ERROR_QUEST_ADD_POINT;
		}

		if( limit->mTakeEarth > 0 )
		{
			if( mHeroInfo.mEarthPoint < limit->mTakeEarth )
				return ERROR_QUEST_ADD_POINT;
		}
	}

	/// 트리거 검사( 조건 검사 )
	if( !TRIGGERMAN->CheckTrigger( GetObjectID(), questIdx ) == eCHECK_TRIGGER_SUCCESS )
		return ERROR_QUEST_ADD_TRIGGER;

	return ERROR_QUEST_ADD_SUCCESS;
}

// 현재 맵에서 발생가능한 신규퀘스트 리스트만 보낸다
bool cPlayer::SendNewQuestList()
{
	if( GetGameIn() == false )
		return false;

	/// 해당맵의 있는 npc로 재 세팅
	typedef tPointerHashMap<unsigned long, cNpc*> cNpcMap;
	typedef tHashSet<unsigned long> cQuestSet;
	cQuestSet nextLevelQuestList;

	HANDLE					handle  = NULL;
	MSG_SYN_NEWQUESTLIST*	sendMsg = (MSG_SYN_NEWQUESTLIST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_NEWLIST_SYN );
	if( sendMsg != NULL )
	{
		unsigned long	length    = sizeof(MSG_SYN_NEWQUESTLIST) - sizeof(sendMsg->questIdxList);
		unsigned long*	questlist = sendMsg->questIdxList;
		sendMsg->rowCount		  = 0;

		cNpcMap* npcMap = (cNpcMap*)OBJECTMANAGER->GetNpcMap();
		if( npcMap != NULL )
		{
			/// npc 목록중 동일한 맵에 있는 npc만 처리
			cNpcMap::cIterator i = npcMap->Begin();
			cNpcMap::cIterator end = npcMap->End();
			for( ; i != end; ++i )
			{
				cNpc* pNpc = (cNpc*)(i->mSecond);
				if( pNpc == NULL )
				{
					NETWORK2->PostServerEvent( "Error(%d): QUEST SendNewQuestList() - not exist npc", GetMapNumber() );
					continue;
				}

				if( pNpc->GetMapNumber() != mMapNumber )
					continue;

				sNpcQuestList* list = QUESTMAN->GetNpcQuestMap( pNpc->GetRaceGender() );
				if( list != NULL )
				{
					cQuestSet::cIterator s = list->mQuestSet.Begin();
					cQuestSet::cIterator send = list->mQuestSet.End();
					for( ; s != send; ++s )
					{
						unsigned long questIdx = (unsigned long)(*s);
						cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
						if( !define )
						{
							NETWORK2->PostServerEvent( "Error(%d): QUEST SendNewQuestList() - failed to search quest define", questIdx );
							assert(0);
							continue;
						}

						/// 반복퀘스트가 아닌경우
						if( define->mRepeatType != eQUEST_REPEAT )
						{
							/// 해당퀘와 그룹퀘가 완료되었으면 skip
							if( IsCompleteQuest( questIdx ) )
								continue;
						}

						/// 해당 퀘와 그룹퀘가 만기일이 지났는지 검사
						if( define->mEventType == eQUEST_TIMEEVENT )
						{
							if( IsValidEndQuest( questIdx ) == false )
								continue;
						}

						/// 3. player 가 보유중인 퀘스트거나 동일한 그룹의 퀘스트 보유중이면 skip
						if( IsKeepGroupQuest( questIdx ) != -1 )
							continue;

						/// 트리거 검사 후 가능하면 신규 
						eCheckTriggerResult triggerRet = eCHECK_TRIGGER_ERROR;
						triggerRet = TRIGGERMAN->CheckTrigger( mObject.index, questIdx );
						if( triggerRet == eCHECK_TRIGGER_SUCCESS )
						{
							/// 보낼내용
							(*questlist) = questIdx;
							questlist++;
							sendMsg->rowCount++;
						}
						else if( triggerRet == eCHECK_TRIGGER_NEXT_LEVEL_QUEST )
						{
							nextLevelQuestList.Insert( questIdx );
						}
					}
				}
			}

			if( sendMsg->rowCount > 0 )
				length += (sendMsg->rowCount * sizeof(sendMsg->questIdxList));

			NETWORK2->SendMsgRoot( handle, length );

			/// 현재 레벨 + NEXT_LEVEL_QUEST_GAP 랩 때 할 수 있는 퀘스트 보내주기
			if( nextLevelQuestList.GetSize() > 0 )
			{
				HANDLE								handle2  = NULL;
				MSG_SYN_NEXT_LEVEL_NEWQUESTLIST*	sendMsg2 = (MSG_SYN_NEXT_LEVEL_NEWQUESTLIST*)NETWORK2->GetMsgRoot( &handle2, mConnectionIdx, NM_QUEST, NM_QUEST_NEXT_LEVEL_NEWLIST_SYN );
				if( sendMsg2 != NULL )
				{
					unsigned long	len		  = sizeof(MSG_SYN_NEXT_LEVEL_NEWQUESTLIST) - sizeof(sendMsg2->questIdxList);
					unsigned long*	questlist = sendMsg2->questIdxList;
					sendMsg2->rowCount		  = 0;

					cQuestSet::cIterator pos  = nextLevelQuestList.Begin();
					cQuestSet::cIterator end  = nextLevelQuestList.End();
					for ( ; pos != end; ++pos )
					{
						(*questlist) = (*pos);
						questlist++;
						sendMsg2->rowCount++;
					}

					len += (sendMsg2->rowCount * sizeof(sendMsg2->questIdxList));
					NETWORK2->SendMsgRoot( handle2, len );
				}
			}
		}
	}
	return true;
}

// 퀘스트의 일정 상황에만 검사
// 퀘스트 기준으로 NPC 상태 체크
bool cPlayer::SendEventQuestList( short eventtype )
{
	typedef tPointerHashMap<unsigned long, cNpc*> cNpcMap;
	typedef tHashSet<unsigned long> cQuestSet;
	typedef tArray<unsigned long> cArray;
	cQuestSet nextLevelQuestList;

	cArray* typeArr = QUESTMAN->GetQuestTypeArr( eventtype );
	
	if( typeArr->GetSize() == 0 )
		return true;

	HANDLE					handle  = NULL;
	MSG_SYN_NEWQUESTLIST*	sendMsg = (MSG_SYN_NEWQUESTLIST*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_NEWLISTEVENT_SYN );
	if( sendMsg != NULL )
	{
		unsigned long	length    = sizeof(MSG_SYN_NEWQUESTLIST) - sizeof(sendMsg->questIdxList);
		unsigned long*	questlist = sendMsg->questIdxList;
		sendMsg->rowCount		  = 0;

		/// 일정 상황에서만 발생하는 퀘스트
		cArray::cIterator i = typeArr->Begin();
		cArray::cIterator end = typeArr->End();
		for( ; i != end; ++i )
		{
			unsigned long questIndex = (unsigned long)(*i);
			cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST SendEventQuestList() - failed to search questIdx", questIndex );
				assert(0);
				continue;
			}

			cNpcMap* npcMap = (cNpcMap*)OBJECTMANAGER->GetNpcMap();
			if( npcMap != NULL )
			{
				/// npc 목록중 동일한 맵에 있는 npc만 처리
				cNpcMap::cIterator ni = npcMap->Begin();
				cNpcMap::cIterator nend = npcMap->End();
				for( ; ni != nend; ++ni )
				{
					cNpc* pNpc = (cNpc*)(ni->mSecond);
					if( pNpc == NULL )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST SendEventQuestList() - not exist npc", GetMapNumber() );
						continue;
					}

					// 동일맵 검사
					if( pNpc->GetMapNumber() != mMapNumber )
						continue;

					// 이벤트 퀘를 발생하는 NPC인지 검사
					if( pNpc->GetRaceGender() != define->mGiveNpcIndex )
						continue;

					/// 반복퀘스트가 아닌경우
					if( define->mRepeatType != eQUEST_REPEAT )
					{
						/// 해당퀘와 그룹퀘가 완료되었으면 skip
						if( IsCompleteQuest( questIndex ) )
							continue;
					}

					/// 해당 퀘와 그룹퀘가 만기일이 지났는지 검사
					if( define->mEventType == eQUEST_TIMEEVENT )
					{
						if( IsValidEndQuest( questIndex ) == false )
							continue;
					}

					/// player 가 보유중인 퀘스트거나 동일한 그룹의 퀘스트 보유중이면 skip
					if( IsKeepGroupQuest( questIndex ) != -1 )
						continue;

					/// 트리거 검사 후 가능하면 신규 
					eCheckTriggerResult triggerRet = eCHECK_TRIGGER_ERROR;
					triggerRet = TRIGGERMAN->CheckTrigger( mObject.index, questIndex );
					if( triggerRet == eCHECK_TRIGGER_SUCCESS )
					{
						/// 보낼내용
						(*questlist) = questIndex;
						questlist++;
						sendMsg->rowCount++;
					}
					else if( triggerRet == eCHECK_TRIGGER_NEXT_LEVEL_QUEST )
					{
						nextLevelQuestList.Insert( questIndex );
					}
				}
			}
		}

		if( sendMsg->rowCount > 0 )
			length += (sendMsg->rowCount * sizeof(sendMsg->questIdxList));

		NETWORK2->SendMsgRoot( handle, length );

		/// 현재 레벨 + NEXT_LEVEL_QUEST_GAP 랩 때 할 수 있는 퀘스트 보내주기
		if( nextLevelQuestList.GetSize() > 0 )
		{
			HANDLE								handle2  = NULL;
			MSG_SYN_NEXT_LEVEL_NEWQUESTLIST*	sendMsg2 = (MSG_SYN_NEXT_LEVEL_NEWQUESTLIST*)NETWORK2->GetMsgRoot( &handle2, mConnectionIdx, NM_QUEST, NM_QUEST_NEXT_LEVEL_NEWLISTEVENT_SYN );
			if( sendMsg2 != NULL )
			{
				unsigned long	len		  = sizeof(MSG_SYN_NEXT_LEVEL_NEWQUESTLIST) - sizeof(sendMsg2->questIdxList);
				unsigned long*	questlist = sendMsg2->questIdxList;
				sendMsg2->rowCount		  = 0;

				cQuestSet::cIterator pos  = nextLevelQuestList.Begin();
				cQuestSet::cIterator end  = nextLevelQuestList.End();
				for ( ; pos != end; ++pos )
				{
					(*questlist) = (*pos);
					questlist++;
					sendMsg2->rowCount++;
				}

				len += (sendMsg2->rowCount * sizeof(sendMsg2->questIdxList));
				NETWORK2->SendMsgRoot( handle2, len );
			}
		}
	}
	return true;
}

void cPlayer::UpdateDutyItem()
{
	bool updateNpcStatus = false;
	bool addSend = false;

	HANDLE                handle  = NULL;
	MSG_RES_QUEST_UPDATE* sendMsg = (MSG_RES_QUEST_UPDATE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_UPDATE_SYN );
	if( sendMsg != NULL )
	{
		sQuestList*			  questlist = sendMsg->table;
		unsigned long		  length	= sizeof(MSG_RES_QUEST_UPDATE) - sizeof(sendMsg->table);
		sendMsg->rowCount				= 0;

		TB_QUEST_PROGRESS* quest = mQuest;
		for( unsigned int arrIdx = 0; arrIdx < MAX_KEEPQUEST; ++arrIdx, ++quest )
		{
			if( quest->idx == 0 )
				break;

			cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyItem() - failed to search questIdx", quest->questIdx );
				assert(0);
				break;
			}

			if( quest->status != eQUEST_PLAYING )
				continue;

			cQuestDuty* duty = define->mQuestDuty;
			if( !duty )
				continue;

			/// 수집 아이템 갯수 갱신
			for( int i = 0; i < MAX_DUTY; ++i )
			{
				if( duty->mDuty[i].dutyType == eDUTY_NONE )
					break;

				/// 아이템 수집 의무 찾기
				if( duty->mDuty[i].dutyType == eDUTY_COLLECT )
				{
					PerItemCount* itemCount = ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, duty->mDuty[i].targetIdx );
					unsigned int count = ( itemCount != NULL ) ? itemCount->bag + itemCount->equip : 0;

					// max 검사
					if( count > duty->mDuty[i].count )
						count = duty->mDuty[i].count;

					__int64 end = 0xFF  << ( i * 8 );
					__int64 cnt = count << ( i * 8 );
					__int64 mask = (0xFFFFFFFFFFFFFFFF ^ end);

					__int64 update = ( quest->check & mask ) | cnt;

					if( quest->check != update ) 
					{
						quest->check = update;

						/// 완료가능시 상태 갱신 요청
						if ( quest->check == define->mComplete )
							updateNpcStatus = true;

						addSend = true;
					}
				}
			}

			/// 클라이언트 쏘기
			if( addSend == true )
			{
				questlist->arrIdx		= arrIdx;
				questlist->questIdx		= quest->questIdx;
				questlist->check		= quest->check;
				questlist->status		= quest->status;

				questlist++;
				sendMsg->rowCount++;

				addSend = false;
			}
		}

		/// 쏘기 
		if( sendMsg->rowCount > 0 )
		{
			length += (sendMsg->rowCount * sizeof(sendMsg->table));
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
			NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, length );

		/// 의무조건 충족에 따른 npc 상태 갱신 요청
		if( updateNpcStatus == true )
		{
			SendNewQuestList();
		}
	}
}

/// 퀘스트 진행상황 갱신 ( playerIdx : 몬스터를 잡은 사람 )
void cPlayer::UpdateDutyHunt( unsigned int classIdx, unsigned long playerIdx )
{
	bool				  checkStatus = false;

	/// 테마 서버 이벤트 발생
	if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
		SaveThemeUser( MAKETHEMECID( NETWORK2->GetInDunMapNum(), mThemeMode ) );
				
	HANDLE                handle  = NULL;
	MSG_RES_QUEST_UPDATE* sendMsg = (MSG_RES_QUEST_UPDATE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_UPDATE_SYN );
	if( sendMsg != NULL )
	{
		sQuestList*			  questlist = sendMsg->table;
		unsigned long		  length  = sizeof(MSG_RES_QUEST_UPDATE) - sizeof(sendMsg->table);

		sendMsg->rowCount = 0;
		TB_QUEST_PROGRESS* quest = mQuest;
		for( unsigned int arrIdx = 0; arrIdx < MAX_KEEPQUEST; arrIdx++, quest++ )
		{
			if( quest->idx == 0 )
				break;

			cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDuty() - failed to search questIdx", quest->questIdx );
				assert(0);
				break;
			}

			if( quest->status != eQUEST_PLAYING )
				continue;

			/// 해당 의무조건이 있는지 찾기
			cQuestDuty* duty = define->mQuestDuty;
			if( !duty )
			{
				NETWORK2->PostServerEvent( "Error(%d): not exist quest duty", quest->questIdx );
				continue;
			}

			// 사냥 의무 조건 검사
			int number = -1;
			for( unsigned int i = 0; i < MAX_DUTY; ++i, ++number )
			{
				if( duty->mDuty[i].dutyType == eDUTY_HUNT )
				{
					if( duty->mDuty[i].targetIdx == classIdx )
					{
						number = i;
						break;
					}
				}
			}

			if( number == -1 )
				continue;

			/// 퀘스트 아이템 착용 검사
			if( IsEquipQuestItem( quest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( mObject.index, playerIdx, quest->questIdx ) == false )
				continue;

			/// 퀘스트 완료이면 skip
			if( quest->check == define->mComplete )
				continue;

			__int64 end = 0xFF;
			for( int i = 0; i < number; ++i )
				end = end << 8;

			/// 부분완료인경우 skip
			/// 현재 관련된 내용이 완료인지 검사해서 맞으면, 완료된 퀘므로 카운트 하지 않는다
			if( (quest->check & end) == (define->mComplete & end) )
				continue;

			/// 진행상황 갱신
			__int64 cnt = 0x01;
			for( int i = 0; i < number; ++i )
				cnt = cnt << 8;

			quest->check += cnt;

			/// 완료가능시 상태 갱신 요청
			if( quest->check == define->mComplete )
			{
				checkStatus = true;
			}

			/// 클라이언트 쏘기
			questlist->arrIdx		= arrIdx;
			questlist->questIdx		= quest->questIdx;
			questlist->check		= quest->check;
			questlist->status		= quest->status;

			questlist++;
			sendMsg->rowCount++;
		}

		/// 쏘기 
		if( sendMsg->rowCount > 0 )
		{
			length += (sendMsg->rowCount * sizeof(sendMsg->table));
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
			NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, length );

		/// 의무조건 충족에 따른 npc 상태 갱신 요청
		if( checkStatus == true )
		{
			SendNewQuestList();
		}
	}
}

// 튜토리얼맵에서만 사용하는 퀘스트 조건
void cPlayer::UpdateDutyActiveSkill( unsigned int classIdx, unsigned long skillIndex )
{
	bool checkStatus = false;

	HANDLE                handle  = NULL;
	MSG_RES_QUEST_UPDATE* sendMsg = (MSG_RES_QUEST_UPDATE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_UPDATE_SYN );
	if( sendMsg != NULL )
	{
		sQuestList*			  questlist = sendMsg->table;
		unsigned long		  length  = sizeof(MSG_RES_QUEST_UPDATE) - sizeof(sendMsg->table);

		sendMsg->rowCount = 0;
		TB_QUEST_PROGRESS* quest = mQuest;
		for( unsigned int arrIdx = 0; arrIdx < MAX_KEEPQUEST; arrIdx++, quest++ )
		{
			if( quest->idx == 0 )
				break;

			cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyActiveSkill() - failed to search questIdx", quest->questIdx );
				assert(0);
				break;
			}

			if( quest->status != eQUEST_PLAYING )
				continue;

			/// 해당 의무조건이 있는지 찾기
			cQuestDuty* duty = define->mQuestDuty;
			if( !duty )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyActiveSkill() - not exist quest duty", quest->questIdx );
				continue;
			}

			// 스킬발동 의무 조건 검사
			int number = -1;
			for( unsigned int i = 0; i < MAX_DUTY; ++i, ++number )
			{
				if( duty->mDuty[i].dutyType == eDUTY_ACTIVESKILL && duty->mDuty[i].targetIdx == classIdx )
				{
					// 동일스킬이거나, 일반공격 스킬중에 하나이면 성공
					if( duty->mDuty[i].skillIndex == skillIndex ||
						duty->mDuty[i].skillIndex == NORMAL_ATTACK_SKILL && skillIndex < NORMAL_ATTACK_SKILL_MAX )
					{
						number = i;
						break;
					}
				}
			}

			if( number == -1 )
				continue;

			/// 퀘스트 아이템 착용 검사
			if( IsEquipQuestItem( quest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( mObject.index, mObject.index, quest->questIdx ) == false )
				continue;

			/// 퀘스트 완료이면 skip
			if( quest->check == define->mComplete )
				continue;

			__int64 end = 0xFF;
			for( int i = 0; i < number; ++i )
				end = end << 8;

			/// 부분완료인경우 skip
			/// 현재 관련된 내용이 완료인지 검사해서 맞으면, 완료된 퀘므로 카운트 하지 않는다
			if( (quest->check & end) == (define->mComplete & end) )
				continue;

			/// 진행상황 갱신
			__int64 cnt = 0x01;
			for( int i = 0; i < number; ++i )
				cnt = cnt << 8;

			quest->check += cnt;

			/// 완료가능시 상태 갱신 요청
			if( quest->check == define->mComplete )
			{
				checkStatus = true;
			}

			/// 클라이언트 쏘기
			questlist->arrIdx		= arrIdx;
			questlist->questIdx		= quest->questIdx;
			questlist->check		= quest->check;
			questlist->status		= quest->status;

			questlist++;
			sendMsg->rowCount++;
		}

		/// 쏘기 
		if( sendMsg->rowCount > 0 )
		{
			length += (sendMsg->rowCount * sizeof(sendMsg->table));
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
			NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, length );

		/// 의무조건 충족에 따른 npc 상태 갱신 요청
		if( checkStatus == true )
		{
			SendNewQuestList();
		}
	}
}

//ePVPDM_OBJECTTYPE
void cPlayer::UpdateDutyPVP( unsigned char pvptype, bool win )
{
	bool checkStatus = false;

	HANDLE                handle  = NULL;
	MSG_RES_QUEST_UPDATE* sendMsg = (MSG_RES_QUEST_UPDATE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_UPDATE_SYN );
	if( sendMsg != NULL )
	{
		sQuestList*			  questlist = sendMsg->table;
		unsigned long		  length  = sizeof(MSG_RES_QUEST_UPDATE) - sizeof(sendMsg->table);

		sendMsg->rowCount = 0;
		TB_QUEST_PROGRESS* quest = mQuest;
		for( unsigned int arrIdx = 0; arrIdx < MAX_KEEPQUEST; arrIdx++, quest++ )
		{
			if( quest->idx == 0 )
				break;

			cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyPVP() - failed to search questIdx", quest->questIdx );
				assert(0);
				break;
			}

			if( quest->status != eQUEST_PLAYING )
				continue;

			/// 해당 의무조건이 있는지 찾기
			cQuestDuty* duty = define->mQuestDuty;
			if( !duty )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyPVP() - not exist quest duty", quest->questIdx );
				continue;
			}

			/// pvp 의무조건 검사
			int number = -1;
			for( unsigned int i = 0; i < MAX_DUTY; ++i, ++number )
			{
				// 동일 전장일때
				if( duty->mDuty[i].targetIdx == pvptype )
				{
					if( duty->mDuty[i].dutyType == eDUTY_PVPJOIN )
					{
						// 참여
						number = i;
						break;
					}
					else if( duty->mDuty[i].dutyType == eDUTY_PVPWIN && win == true )
					{
						// 승리
						number = i;
						break;
					}
					else if( duty->mDuty[i].dutyType == eDUTY_PVPLOSE && win == false )
					{
						// 패배
						number = i;
						break;
					}
				}
			}

			if( number == -1 )
				continue;

			/// 퀘스트 아이템 착용 검사
			if( IsEquipQuestItem( quest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( mObject.index, mObject.index, quest->questIdx ) == false )
				continue;

			/// 퀘스트 완료이면 skip
			if( quest->check == define->mComplete )
				continue;

			__int64 end = 0xFF;
			for( int i = 0; i < number; ++i )
				end = end << 8;

			/// 부분완료인경우 skip
			/// 현재 관련된 내용이 완료인지 검사해서 맞으면, 완료된 퀘므로 카운트 하지 않는다
			if( (quest->check & end) == (define->mComplete & end) )
				continue;

			/// 진행상황 갱신
			__int64 cnt = 0x01;
			for( int i = 0; i < number; ++i )
				cnt = cnt << 8;

			quest->check += cnt;

			/// 완료가능시 상태 갱신 요청
			if( quest->check == define->mComplete )
			{
				checkStatus = true;
			}

			/// 클라이언트 쏘기
			questlist->arrIdx		= arrIdx;
			questlist->questIdx		= quest->questIdx;
			questlist->check		= quest->check;
			questlist->status		= quest->status;

			questlist++;
			sendMsg->rowCount++;
		}

		/// 쏘기 
		if( sendMsg->rowCount > 0 )
		{
			length += (sendMsg->rowCount * sizeof(sendMsg->table));
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
			NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, length );

		/// 의무조건 충족에 따른 npc 상태 갱신 요청
		if( checkStatus == true )
		{
			SendNewQuestList();
		}
	}
}

void cPlayer::UpdateDutyTutorial( eDutyType dutyType )
{
	bool checkStatus = false;

	HANDLE                handle  = NULL;
	MSG_RES_QUEST_UPDATE* sendMsg = (MSG_RES_QUEST_UPDATE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_QUEST, NM_QUEST_UPDATE_SYN );
	if( sendMsg != NULL )
	{
		sQuestList*			  questlist = sendMsg->table;
		unsigned long		  length  = sizeof(MSG_RES_QUEST_UPDATE) - sizeof(sendMsg->table);

		sendMsg->rowCount = 0;
		TB_QUEST_PROGRESS* quest = mQuest;
		for( unsigned int arrIdx = 0; arrIdx < MAX_KEEPQUEST; arrIdx++, quest++ )
		{
			if( quest->idx == 0 )
				break;

			cQuestDefine* define = QUESTMAN->GetQuestDefine( quest->questIdx );
			if( !define )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyActiveSkill() - failed to search questIdx", quest->questIdx );
				assert(0);
				break;
			}

			if( quest->status != eQUEST_PLAYING )
				continue;

			/// 해당 의무조건이 있는지 찾기
			cQuestDuty* duty = define->mQuestDuty;
			if( !duty )
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST UpdateDutyActiveSkill() - not exist quest duty", quest->questIdx );
				continue;
			}

			// 스킬발동 의무 조건 검사
			int number = -1;
			for( unsigned int i = 0; i < MAX_DUTY; ++i, ++number )
			{
				if( duty->mDuty[i].dutyType == dutyType )
				{
					number = i;
					break;
				}
			}

			if( number == -1 )
				continue;

			/// 퀘스트 아이템 착용 검사
			if( IsEquipQuestItem( quest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( mObject.index, mObject.index, quest->questIdx ) == false )
				continue;

			/// 퀘스트 완료이면 skip
			if( quest->check == define->mComplete )
				continue;

			__int64 end = 0xFF;
			for( int i = 0; i < number; ++i )
				end = end << 8;

			/// 부분완료인경우 skip
			/// 현재 관련된 내용이 완료인지 검사해서 맞으면, 완료된 퀘므로 카운트 하지 않는다
			if( (quest->check & end) == (define->mComplete & end) )
				continue;

			/// 진행상황 갱신
			__int64 cnt = 0x01;
			for( int i = 0; i < number; ++i )
				cnt = cnt << 8;

			quest->check += cnt;

			/// 완료가능시 상태 갱신 요청
			if( quest->check == define->mComplete )
			{
				checkStatus = true;
			}

			/// 클라이언트 쏘기
			questlist->arrIdx		= arrIdx;
			questlist->questIdx		= quest->questIdx;
			questlist->check		= quest->check;
			questlist->status		= quest->status;

			questlist++;
			sendMsg->rowCount++;
		}

		/// 쏘기 
		if( sendMsg->rowCount > 0 )
		{
			length += (sendMsg->rowCount * sizeof(sendMsg->table));
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
			NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, length );

		/// 의무조건 충족에 따른 npc 상태 갱신 요청
		if( checkStatus == true )
		{
			SendNewQuestList();
		}
	}
}

/// 퀘스트진행시 아이템착용 여부 검사
bool cPlayer::IsEquipQuestItem( unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsEquipQuestItem() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	cQuestLimit* limit = define->mQuestLimit;
	if( limit != NULL )
	{
		long* itemIndex = (long*)limit->mLimitEquipItem;
		for( int i = 0; i < QUEST_ITEM_MAX && (*itemIndex) != 0; ++i, ++itemIndex )
		{
			TB_INVENTORY*   inventory  = 0;
			TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( (*itemIndex) );

			if( itemDefine != NULL )
			{
				switch( itemDefine->type )
				{
				case ITEM_WEAPON:
					switch( itemDefine->subType )
					{
					case ITEM_WEAPON_SWORD: 	// 한손검
					case ITEM_WEAPON_BLADE:		// 양손검
					case ITEM_WEAPON_DUAL:		// 이도류
					case ITEM_WEAPON_CUTTER:	// 단  검
					case ITEM_WEAPON_GUN:		// 총
					case ITEM_WEAPON_STAFF:		// 지팡이
						inventory = SelectInventory( (short)((mItemActiveWeapon == ItemActiveFront) ? INVENTORY_HAND_RIGHT2 : INVENTORY_HAND_RIGHT1) );
						break;
					case ITEM_WEAPON_SHIELD:	// 방패
						inventory = SelectInventory( (short)((mItemActiveWeapon == ItemActiveFront) ? INVENTORY_HAND_LEFT2 : INVENTORY_HAND_LEFT1) );
						break;
					default: 
						NETWORK2->PostServerEvent( "Error(%d): QUEST IsEquipQuestItem() - error equip type", (*itemIndex) );
						assert(0);	
						break;
					}
					break;
				case ITEM_WEAR:
					switch( itemDefine->subType )
					{
					case ITEM_WEAR_HEAD:	    inventory = SelectInventory( INVENTORY_WEAR_HAT      ); break;	// 모자
					case ITEM_WEAR_UPPER:	    inventory = SelectInventory( INVENTORY_WEAR_BODY1    ); break;	// 상의
					case ITEM_WEAR_LOWER:	    inventory = SelectInventory( INVENTORY_WEAR_BODY2    ); break;	// 하의
					case ITEM_WEAR_HANDS:	    inventory = SelectInventory( INVENTORY_WEAR_HAND     ); break;	// 손
					case ITEM_WEAR_FEET:	    inventory = SelectInventory( INVENTORY_WEAR_FOOT     ); break;	// 발
					case ITEM_WEAR_ONEPIECE:    inventory = SelectInventory( INVENTORY_WEAR_BODY1    ); break;	// 원피스
					case ITEM_WEAR_COS_HEAD:	inventory = SelectInventory( INVENTORY_COSTUME_HAT   ); break;	// 코스튬 모자
					case ITEM_WEAR_COS_UPPER:	inventory = SelectInventory( INVENTORY_COSTUME_BODY1 ); break;	// 코스튬 상의
					case ITEM_WEAR_COS_LOWER:	inventory = SelectInventory( INVENTORY_COSTUME_BODY2 ); break;	// 코스튬 하의
					case ITEM_WEAR_COS_HANDS:	inventory = SelectInventory( INVENTORY_COSTUME_HAND  ); break;	// 코스튬 손
					case ITEM_WEAR_COS_FEET:	inventory = SelectInventory( INVENTORY_COSTUME_FOOT  ); break;	// 코스튬 발
					case ITEM_WEAR_COS_ONEPIECE:inventory = SelectInventory( INVENTORY_COSTUME_BODY1 ); break;	// 코스튬 원피스
					default: 
						NETWORK2->PostServerEvent( "Error(%d): QUEST IsEquipQuestItem() - error equip type", (*itemIndex) );
						assert(0);	
						break;
					}
					break;
				case ITEM_ACCESSORY:
					switch( itemDefine->subType )
					{
					case ITEM_ACCESSORY_EARRING:	inventory = SelectInventory( INVENTORY_WEAR_EARRING  ); break;	// 귀걸이
					case ITEM_ACCESSORY_NECKLACE:	inventory = SelectInventory( INVENTORY_WEAR_NECKLACE ); break;	// 목걸이
					case ITEM_ACCESSORY_BROOCH:		inventory = SelectInventory( INVENTORY_WEAR_BROOCH   ); break;	// 브로치
					case ITEM_ACCESSORY_BRACELET:	inventory = SelectInventory( INVENTORY_WEAR_BRACELET ); break;	// 팔찌
					case ITEM_ACCESSORY_RING:		inventory = SelectInventory( INVENTORY_WEAR_RING     ); break;	// 반지
					case ITEM_ACCESSORY_COS_FACE:	inventory = SelectInventory( INVENTORY_COSTUME_FACE  ); break;	// 코스튬 장식 머리
					case ITEM_ACCESSORY_COS_BACK:	inventory = SelectInventory( INVENTORY_COSTUME_BACK  ); break;	// 코스튬 장식 몸통
					default: 
						NETWORK2->PostServerEvent( "Error(%d): QUEST IsEquipQuestItem() - error equip type", (*itemIndex) );
						assert(0);
						break;
					}
					break;
				case ITEM_CARD:
					if( itemDefine->subType == ITEM_CARD_EQUIP_C )
					{
						for( short i = INVENTORY_WEAR_CARD1; i <= INVENTORY_WEAR_CARD3; ++i )
						{
							inventory = SelectInventory(i);
							if( inventory && inventory->itemDefineIndex == (*itemIndex) )
								break;
						}
					}
					break;
				default: 
					NETWORK2->PostServerEvent( "Error(%d): QUEST IsEquipQuestItem() - error equip type", (*itemIndex) );
					assert(0);
					break;
				}

				// 동일한지 검사
				if( inventory && inventory->itemDefineIndex != (*itemIndex) )
					return false;
			}
		}
	}	
	return true;
}

/// 의뢰아이템 사용 가능 여부 체크
int cPlayer::IsUseQuestItem( unsigned int number, unsigned long& questIdx)
{
	/*-- 아이템 사용시 검사할 내용들.
	*/
	if( GetStateDie() == true )
		return ERROR_QUEST_ADD_ITEMFAIL;

    if ( IsInventoryBag( (unsigned short)number ) == false )
		return ERROR_QUEST_ADD_ITEMFAIL;

	TB_INVENTORY* inventory = SelectInventory( (short)number );
	if( IsInventory( inventory ) == false )
		return ERROR_QUEST_ADD_ITEMFAIL;

	if( inventory->apply != InventoryApplyNone )
		return ERROR_QUEST_ADD_ITEMFAIL;

	TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
	if( itemDefine == NULL )
		return ERROR_QUEST_ADD_ITEMFAIL;

    if ( IsInventorySeal( itemDefine, inventory ) == true )
        return ERROR_QUEST_ADD_LICENSE;

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		return ERROR_QUEST_ADD_ITEMFAIL;

	/*unsigned int getState = GetState( );
	if ( !( getState == eOBJECT_STATE_IDLE || getState == ePLAYER_STATE_ITEMPICK ) )
	return ERROR_QUEST_ADDBYITEM_ITEMFAIL;*/

	/*-- Item Limit Table(TB_ITEM_LIMIT) 적용 --*/
	if ( !IsItemLimit( inventory->itemDefineIdx ) )
		return ERROR_QUEST_ADD_ITEMFAIL;

	/*-- 아이템 효과 적용 - 스킬과 연결하여 사용한다.
	*/
	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	if( itemAbility == NULL )
		return ERROR_QUEST_ADD_ITEMFAIL;

	/// 퀘스트 의뢰 아이템인지 확인
	if( !(itemAbility->influence_idx > 0 && (itemDefine->type == ITEM_ETC1 && itemDefine->subType == ITEM_ETC1_QUEST)) )
		return ERROR_QUEST_ADD_ITEMFAIL;

	questIdx = itemAbility->influence_idx;	// 퀘스트 인덱스
	return ERROR_QUEST_ADD_SUCCESS;
}

/// 채집 관련 완료 검사
bool cPlayer::IsCompleteGathering( unsigned long questIndex )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsCompleteGathering() - failed to search questIndex", questIndex );
		assert(0);
		return true;
	}

	cQuestDuty* duty = define->mQuestDuty;
	if( !duty )
	{
		NETWORK2->PostServerEvent( "Error: QUEST IsCompleteGathering() - failed to search duty" );
		assert(0);
		return true;
	}

	int arrIdx = IsKeepQuest( questIndex );
	if( arrIdx == -1 )
		return true;

	/// 퀘스트 자체가 완료되었으면 리턴
	if( mQuest[arrIdx].check >= define->mComplete )
		return true;

	/// 채집 관련부분 완료되었는지 검사
	__int64 mask = 0x00;
	for( unsigned int i = 0; i < MAX_DUTY; ++i )
	{
		eDutyType type = duty->mDuty[i].dutyType;

		/// 의무가 없는게 나오면 뒤에도 없으므로 끝
		if( type == eDUTY_NONE )
			break;

		if( type == eDUTY_COLLECT )
		{
			__int64 bit = 0xFF << ( i * 8 );
			mask = mask | bit;
		}
	}

	/// 모든 채집 조건을 완료하였으면
	if( (mQuest[arrIdx].check & mask) >= (define->mComplete & mask) )
		return true;
	return false;
}

/// 기본 보상 처리
bool cPlayer::SendDefaultReward( unsigned long questIdx, unsigned long* skillList, unsigned char skillListCnt )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SendDefaultReward() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	/// 기본 보상 없는 퀘스트도 있다
	cQuestReward* reward = define->mDefaultReward;
	if( !reward )
		return true;

	if( reward->mSkillPoint > 0 )
		SkillPointPlus( (unsigned short)reward->mSkillPoint );

	if( reward->mExp > 0 )
		AddExp( reward->mExp, false, false );

	if( reward->mSxp > 0 )
		AddSxp( reward->mSxp, false );

	/// 보상 버프 
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long buffIdx = reward->mRewardBuff[i];
		SendRewardBuff( buffIdx );
	}

	/// 전직 보상
	if( reward->mChangeJobIndex > 0 )
	{
		SetJob( (ePLAYER_JOB)reward->mChangeJobIndex, skillList, skillListCnt );

		NETWORK2->PostReservedEvent( EVENT_RESERVED_CHANGEJOB, mObject.index, mConnectionIdx );		
	}

	/// 세력 세팅
	if( reward->mForceType > eFORCETYPE_NONE && reward->mForceType < eFORCETYPE_MAX )
	{
		SetForceType( reward->mForceType );
	}

	/// 호칭 보상
	if( reward->mGiveTitleIndex > 0  )
	{
		/// 타이틀은 기본 보상으로 주어지기 때문에 따로 쿼리문을 적용시키지 않았다!
		if( AddHaveTitle( reward->mGiveTitleIndex ) == true )
			SendHaveTitle( reward->mGiveTitleIndex );
	}

	/// 맵 체인지
	if( reward->mMapChangeIndex > 0 )
	{
		if( NETWORK2->GetServerType() != _E_ST_ID_TUTORIAL_ )
		{
			if( QuestMapChange( reward->mMapChangeIndex ) == false )
			{
				NETWORK2->PostServerEvent( "Error(%d,%d): SendDefaultReward MapChange", questIdx, reward->mMapChangeIndex );
				assert(0);
			}
		}
		else
		{
			// 로그
			NETWORK2->PostReservedEvent( EVENT_RESERVED_TUTORIAL, mObject.index, mConnectionIdx );

			// 맵이동 가능 상태인지 체크
			if( MapChangeByTutorial( reward->mMapChangeIndex ) == false )
			{
				NETWORK2->PostServerEvent( "Error(%d,%d): SendDefaultReward MapChangeByTutorial MapChange", questIdx, reward->mMapChangeIndex );
				assert(0);
			}
		}
	}
	return true;
}

/// 보상 버프 추가
bool cPlayer::SendRewardBuff( unsigned long buffIdx )
{
	if( buffIdx > 0 )
	{
		sInfluenceScript* info = SKILLSCRIPT->GetInfluenceInfo( buffIdx );
		if( info )
		{
			/// 오라타입
			if( info->mContinuanceTime == -1 )
			{
				int retvalue = SKILLMANAGER->AddInfSwitch( mObject, mObject, buffIdx, 0, true );
				if( retvalue == 0 )
				{
					NETWORK2->PostServerEvent( "Error(%d): SendRewardBuff mpSkillManager->AddInfSwitch", buffIdx );
					assert(0);
				}
			}
			/// 그외 타입
			else
			{
				bool retvalue = SKILLMANAGER->AddInfluence( mObject, mObject, buffIdx, 0, true );
				if( retvalue == false )
				{
					NETWORK2->PostServerEvent( "Error(%d): SendRewardBuff mpSkillManager->AddInfluence", buffIdx );
					assert(0);
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent( "Error(%d): SendRewardBuff mpSkillManager->GetInfluenceInfo", buffIdx );
			assert(0);
		}
	}
	return true;
}

/// 퀘스트 수락에 의한 제약 조건 걸기
bool cPlayer::SendLimitPenalty( unsigned long questIdx, bool gameIn )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SendLimitPenalty() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	cQuestLimit* limit = define->mQuestLimit;
	if( !limit )
		return true;

	/// 제약 버프
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long buffIdx = limit->mLimitBuff[i];
		if( buffIdx > 0 )
		{
			sInfluenceScript* info =  SKILLSCRIPT->GetInfluenceInfo( buffIdx );
			if( info )
			{
				/// 오라타입
				if( info->mContinuanceTime == -1 )
				{
					int retvalue = SKILLMANAGER->AddInfSwitch( mObject, mObject, buffIdx, 0, true );
					if( retvalue == 0 )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->AddInfSwitch", buffIdx );
						assert(0);
						return false;
					}
				}
				/// 그외 타입
				else
				{
					bool retvalue = SKILLMANAGER->AddInfluence( mObject, mObject, buffIdx, 0, true );
					if( retvalue == false )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->AddInfluence", buffIdx );
						assert(0);
						return false;
					}
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->GetInfluenceInfo", buffIdx );
				assert(0);
			}
		}
	}

	/// 제약 디버프
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long debuffIdx = limit->mLimitDeBuff[i];
		if( debuffIdx > 0 )
		{
			sInfluenceScript* info =  SKILLSCRIPT->GetInfluenceInfo( debuffIdx );
			if( info )
			{
				/// 오라타입
				if( info->mContinuanceTime == -1 )
				{
					int retvalue = SKILLMANAGER->AddInfSwitch( mObject, mObject, debuffIdx, 0, true );
					if( retvalue == 0 )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->AddInfSwitch debuff", debuffIdx );
						assert(0);
						return false;
					}
				}
				/// 그외 타입
				else
				{
					bool retvalue = SKILLMANAGER->AddInfluence( mObject, mObject, debuffIdx, 0, true );
					if( retvalue == false )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->AddInfluence debuff", debuffIdx );
						assert(0);
						return false;
					}
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST LIMIT PENALTY mpSkillManager->GetInfluenceInfo debuff", debuffIdx );
				assert(0);
				return false;
			}
		}
	}

	/// gamein 시에는 적용하지 않는다
	if( gameIn == false )
	{
		/// 맵 체인지
		if( limit->mMapChangeIndex > 0 )
		{
			if( QuestMapChange( limit->mMapChangeIndex ) == false )
			{
				NETWORK2->PostServerEvent( "Error(%d,%d): QUEST LIMIT PENALTY MapChange", questIdx, limit->mMapChangeIndex );
				return false;
			}
		}

		/// 몬스터 소환
		if( limit->mMonsterClassIndex > 0 && limit->mMonsterCount > 0 )
		{
			for( unsigned int i = 0; i < limit->mMonsterCount; ++i )
			{
				unsigned long monsterIdx = AIMANAGER->QuestSummonMonRegen( mMapNumber, GetXPos(), GetYPos(), limit->mMonsterClassIndex, limit->mMonsterTime * 1000 );
				if( monsterIdx > 0 )
				{
					if( QUESTMAN->AddQuestMonster( mObject.index, questIdx, monsterIdx ) == false )
					{
						NETWORK2->PostServerEvent( "Error(%d,%d,%d): QUEST LIMIT PENALTY AddQuestMonster", questIdx, limit->mMonsterClassIndex, monsterIdx );
						return false;
					}
				}
			}
		}
	}
	return true;
}

/// 실패로 인한 패널티 주기
bool cPlayer::SendFailPenalty( unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SendFailPenalty() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	cQuestFail* fail = define->mQuestFail;
	if( !fail )
		return true;

	/// 실패버프 
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long buffIdx = fail->mFailBuff[i];
		if( buffIdx > 0 )
		{
			sInfluenceScript* info =  SKILLSCRIPT->GetInfluenceInfo( buffIdx );
			if( info )
			{
				/// 오라타입
				if( info->mContinuanceTime == -1 )
				{
					int retvalue = SKILLMANAGER->AddInfSwitch( mObject, mObject, buffIdx, 0, true );
					if( retvalue == 0 )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->AddInfSwitch", buffIdx );
						assert(0);
					}
				}
				/// 그외 타입
				else
				{
					bool retvalue = SKILLMANAGER->AddInfluence( mObject, mObject, buffIdx, 0, true );
					if( retvalue == false )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->AddInfluence", buffIdx );
						assert(0);
					}
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->GetInfluenceInfo", buffIdx );
				assert(0);
			}
		}
	}

	/// 실패 디버프
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long debuffIdx = fail->mFailDeBuff[i];
		if( debuffIdx > 0 )
		{
			sInfluenceScript* info =  SKILLSCRIPT->GetInfluenceInfo( debuffIdx );
			if( info )
			{
				/// 오라타입
				if( info->mContinuanceTime == -1 )
				{
					int retvalue = SKILLMANAGER->AddInfSwitch( mObject, mObject, debuffIdx, 0, true );
					if( retvalue == 0 )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->AddInfSwitch debuff", debuffIdx );
						assert(0);
					}
				}
				/// 그외 타입
				else
				{
					bool retvalue = SKILLMANAGER->AddInfluence( mObject, mObject, debuffIdx, 0, true );
					if( retvalue == false )
					{
						NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->AddInfluence debuff", debuffIdx );
						assert(0);
					}
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error(%d): QUEST FAIL PENALTY mpSkillManager->GetInfluenceInfo debuff", debuffIdx );
				assert(0);
			}
		}
	}
	return true;
}

bool cPlayer::SendEndPenalty( unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SendEndPenalty() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	cQuestEnd* end = define->mQuestEnd;
	if( !end )
		return true;

	/// 버프, 디버프 해제
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long buffIdx = end->mEndBuff[i];
		if( buffIdx > 0 )
		{
			SKILLMANAGER->InfluenceOff( mObject.index, buffIdx );
		}
	}
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long debuffIdx = end->mEndDeBuff[i];
		if( debuffIdx > 0 )
		{
			SKILLMANAGER->InfluenceOff( mObject.index, debuffIdx );
		}
	}

	/// 몬스터 소환정보 삭제
	cQuestLimit* limit = define->mQuestLimit;
	if( limit && limit->mMonsterClassIndex > 0 && limit->mMonsterCount > 0 )
	{
		QUESTMAN->DeleteQuestMonster( mObject.index, questIdx );
	}
	return true;
}

/// SaveQuestInsert Method.
int cPlayer::SaveQuestInsert( ULONG_PTR socketContext, unsigned long characterIdx, unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestInsert() - failed to search questIdx", questIdx );
		assert(0);
		return ERROR_QUEST_ADD_FAIL;
	}

	/// 퀘스트시작시 제공할 아이템을 받을수 있는지 체크
	unsigned long total = 0;
	cQuestLimit* questIimit = define->mQuestLimit;
	if( questIimit )
	{
		unsigned int bagCnt = 0;
		unsigned int questCnt = 0;

		sQuestItem bagItem[QUEST_ITEM_MAX];
		sQuestItem questItem[QUEST_ITEM_MAX];

		memset( bagItem, 0, sizeof(bagItem) );
		memset( questItem, 0, sizeof(questItem) );

		sQuestItem* item = questIimit->mGiveItem;
		for( int i = 0; i < QUEST_ITEM_MAX; ++i, ++item )
		{
			if( item->itemIndex != 0 && item->count > 0 )
			{
				TB_ITEM_DEFINE* p = ITEMMANAGER->GetItemDefineByIndex( item->itemIndex );
				if( p )
				{
					if( p->type	== ITEM_QUEST )
					{
						questItem[questCnt].itemIndex = item->itemIndex;
						questItem[questCnt].count	  = item->count;
						questCnt++;
					}
					else
					{
						bagItem[questCnt].itemIndex = item->itemIndex;
						bagItem[questCnt].count		= item->count;
						bagCnt++;	
					}

					//
					total++;
				}
			}
		}

		/// 가방 검사
		if( IsItemReward( bagItem, bagCnt, INVENTORY_BAG_BEGIN, mHeroInfo.BagEnd ) < bagCnt )
			return ERROR_QUEST_ADD_ITEM;

		/// 퀘스트 탭 검사
		if( IsItemReward( questItem, questCnt, INVENTORY_QUEST_BEGIN, INVENTORY_QUEST_END ) < questCnt )
			return ERROR_QUEST_ADD_ITEM;

        /// 가방 검사 - 최대 보유갯수
        if( IsMaxInventory( bagItem, bagCnt ) == true )
            return ERROR_QUEST_ADD_MAX_ITEM;

        /// 퀘스트 탭 검사 - 최대 보유갯수
        if( IsMaxInventory( questItem, questCnt ) == true )
            return ERROR_QUEST_ADD_MAX_ITEM;
	}

	/// 퀘스트 삽입시 처리
	__int64 check = 0;
	cQuestDuty* duty = define->mQuestDuty;
	if( duty )
	{
		for( int i = 0; i < MAX_DUTY; ++i )
		{
			if( duty->mDuty[i].dutyType == eDUTY_MEET )
			{
				/// 갱신 
				__int64 end = 0xFF;
				for( int e = 0; e < i; ++e )
					end = end << 8;

				/// 진행상황 갱신
				__int64 cnt = 0x01;
				for( int c = 0; c < i; ++c )
					cnt = cnt << 8;

				check += cnt;
				break;
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestInsert() - failed to search duty", questIdx );
		return ERROR_QUEST_ADD_FAIL;
	}

	///
	HANDLE			handle      = NULL;
	QUEST_INSERT*	questInsert = (QUEST_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_INSERT );
	long&           rowCount    = questInsert->rowCount;
	TB_INVENTORY*   table       = questInsert->inventory;
	unsigned long   length      = sizeof(QUEST_INSERT) - sizeof(questInsert->inventory);

	questInsert->characterIdx = characterIdx;
	questInsert->questIdx = questIdx;
	questInsert->timeType = eTIME_NONE;
	questInsert->restTime = 0;
	questInsert->takemoney  = ( questIimit != NULL ) ? -((long)questIimit->mTakeMoney) : 0;
	questInsert->firepoint  = ( questIimit != NULL ) ? -((long)questIimit->mTakeFire) : 0;
	questInsert->waterpoint = ( questIimit != NULL ) ? -((long)questIimit->mTakeWater) : 0;
	questInsert->windpoint  = ( questIimit != NULL ) ? -((long)questIimit->mTakeWind) : 0;
	questInsert->earthpoint = ( questIimit != NULL ) ? -((long)questIimit->mTakeEarth) : 0;
	questInsert->tutorialindex = 0;

	if( define->mType == eQUEST_TYPE_TUTORIAL )
	{
		questInsert->tutorialindex = ( questIimit != NULL ) ? questIimit->mTutorialModeIndex : 0;
	}

	questInsert->check = check;

	/// 남은시간 세팅
	if( questIimit && questIimit->mTimeType != eTIME_NONE )
	{
		questInsert->timeType = questIimit->mTimeType;
		questInsert->restTime = questIimit->mTime * 1000;
	}

	/*-- 제공 아이템
	*/
	if( questIimit->mGiveItem )
	{
		sQuestItem* items = questIimit->mGiveItem;

		for ( unsigned int i0 = 0; i0 < total; i0++, items++ )
		{
			TB_ITEM_DEFINE* itemDefine  = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );
			short           remainCount = (short)items->count;

			TB_INVENTORY*   inventory;
			short           shortCount;
			short           number;

			if ( itemDefine != NULL && remainCount > 0 )
			{
				// 아이템 퀘스트 제공 아이템1. 겹치기 검사 및 처리.
				if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
				{
					do {
						inventory   = SelectInventory( number );
						shortCount  = UpdateInventory( inventory, remainCount );
						remainCount = remainCount - shortCount;

						(*table) = (*inventory);
						table->count = shortCount;

						table++; rowCount++;
					} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
				}

				// 아이템 퀘스트 제공 아이템2. 생성 검사 및 등록.
				if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
				{
					do {
						inventory = CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
						remainCount = remainCount - shortCount;

						(*table)     = (*inventory);
						table->idx   = 0;
						table->count = shortCount;

						table++; rowCount++;
					} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
				}
			}
		}
	}

	length += (sizeof(questInsert->inventory) * rowCount);
	mInventoryDb.reward = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return ( mInventoryDb.reward ) ? ERROR_QUEST_ADD_SUCCESS : ERROR_QUEST_ADD_FAIL;
}

// SaveQuestInsertByItem - 의뢰아이템으로 퀘스트 발급
int cPlayer::SaveQuestInsertByItem( ULONG_PTR socketContext, unsigned long characterIdx, unsigned long questIndex, unsigned short number )
{
	/// 1. 퀘스트 검사
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestInsertByItem() - failed to search questIdx", questIndex );
		assert(0);
		return ERROR_QUEST_ADD_FAIL;
	}

	TB_INVENTORY* inventory = SelectInventory( number );
	if ( IsInventory( inventory ) == false )
		return ERROR_QUEST_ADD_ITEMFAIL;

	/// 2. 아이템 제공 체크 - 퀘스트시작시 제공할 아이템을 받을수 있는지 체크
	unsigned long total = 0;
	cQuestLimit* questIimit = define->mQuestLimit;
	if( questIimit )
	{
		unsigned int bagCnt = 0;
		unsigned int questCnt = 0;

		sQuestItem bagItem[QUEST_ITEM_MAX];
		sQuestItem questItem[QUEST_ITEM_MAX];

		memset( bagItem, 0, sizeof(bagItem) );
		memset( questItem, 0, sizeof(questItem) );

		sQuestItem* item = questIimit->mGiveItem;
		for( int i = 0; i < QUEST_ITEM_MAX; ++i, ++item )
		{
			if( item->itemIndex != 0 && item->count > 0 )
			{
				TB_ITEM_DEFINE* p = ITEMMANAGER->GetItemDefineByIndex( item->itemIndex );
				if( p )
				{
					if( p->type	== ITEM_QUEST )
					{
						questItem[questCnt].itemIndex = item->itemIndex;
						questItem[questCnt].count	  = item->count;
						questCnt++;
					}
					else
					{
						bagItem[questCnt].itemIndex = item->itemIndex;
						bagItem[questCnt].count		= item->count;
						bagCnt++;	
					}

					//
					total++;
				}
			}
		}

		/// 가방 검사
		if( IsItemReward( bagItem, bagCnt, INVENTORY_BAG_BEGIN, mHeroInfo.BagEnd ) < bagCnt )
			return ERROR_QUEST_ADD_ITEMFAIL;

		/// 퀘스트 탭 검사
		if( IsItemReward( questItem, questCnt, INVENTORY_QUEST_BEGIN, INVENTORY_QUEST_END ) < questCnt )
			return ERROR_QUEST_ADD_ITEMFAIL;

        /// 가방 검사 - 최대 보유갯수
        if( IsMaxInventory( bagItem, bagCnt ) == true )
            return ERROR_QUEST_ADD_MAX_ITEM;

        /// 퀘스트 탭 검사 - 최대 보유갯수
        if( IsMaxInventory( questItem, questCnt ) == true )
            return ERROR_QUEST_ADD_MAX_ITEM;
	}


	/// 퀘스트 삽입시 처리
	__int64 check = 0;
	cQuestDuty* duty = define->mQuestDuty;
	if( duty )
	{
		for( int i = 0; i < MAX_DUTY; ++i )
		{
			if( duty->mDuty[i].dutyType == eDUTY_MEET )
			{
				/// 갱신 
				__int64 end = 0xFF;
				for( int e = 0; e < i; ++e )
					end = end << 8;

				/// 진행상황 갱신
				__int64 cnt = 0x01;
				for( int c = 0; c < i; ++c )
					cnt = cnt << 8;

				check += cnt;
				break;
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestInsertByItem() - failed to search duty", questIndex );
		return ERROR_QUEST_ADD_FAIL;
	}

	/// 3. sql 호출
	HANDLE              handle		= NULL;
	QUEST_INSERT*		questInsert = (QUEST_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_INSERTBYITEM );
	long&               rowCount    = questInsert->rowCount;
	TB_INVENTORY*       table       = questInsert->inventory;
	unsigned long       length      = sizeof(QUEST_INSERT) - sizeof(questInsert->inventory);
	short               shortCount;

	questInsert->characterIdx = characterIdx;
	questInsert->questIdx = questIndex;
	questInsert->check    = check;
	questInsert->timeType = eTIME_NONE;
	questInsert->restTime = 0;
	questInsert->takemoney  = ( questIimit != NULL ) ? -((long)questIimit->mTakeMoney) : 0;
	questInsert->firepoint  = ( questIimit != NULL ) ? -((long)questIimit->mTakeFire) : 0;
	questInsert->waterpoint = ( questIimit != NULL ) ? -((long)questIimit->mTakeWater) : 0;
	questInsert->windpoint  = ( questIimit != NULL ) ? -((long)questIimit->mTakeWind) : 0;
	questInsert->earthpoint = ( questIimit != NULL ) ? -((long)questIimit->mTakeEarth) : 0;
	questInsert->tutorialindex = 0;

	if( define->mType == eQUEST_TYPE_TUTORIAL )
	{
		questInsert->tutorialindex = ( questIimit != NULL ) ? questIimit->mTutorialModeIndex : 0;
	}
	
	/// 사용할 아이템
	shortCount = UpdateInventory( inventory, (-1) );
	table->idx             = inventory->idx;
	table->itemDefineIdx   = inventory->itemDefineIdx;
	table->itemDefineIndex = inventory->itemDefineIndex;
	table->number          = inventory->number;
	table->count           = shortCount;

	table++; rowCount++;

	/// 갯수가 0개이면 완전 삭제므로, 서버정보 미리 삭제
	if( inventory->count == 0 )
		RemoveInventory( inventory );

	/*-- 제공 아이템
	*/
	if( questIimit->mGiveItem )
	{
		sQuestItem* items = questIimit->mGiveItem;

		for ( unsigned int i0 = 0; i0 < total; i0++, items++ )
		{
			TB_ITEM_DEFINE* itemDefine  = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );
			short           remainCount = (short)items->count;

			if ( itemDefine != NULL && remainCount > 0 )
			{
				TB_INVENTORY* inventory;
				short         shortCount;
				short         number;

				// 아이템 퀘스트 제공 아이템1. 겹치기 검사 및 처리.
				if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
				{
					do {
						inventory   = SelectInventory( number );
						shortCount  = UpdateInventory( inventory, remainCount );
						remainCount = remainCount - shortCount;

						(*table) = (*inventory);
						table->count = shortCount;

						table++; rowCount++;
					} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
				}
				// 아이템 퀘스트 제공 아이템2. 생성 검사 및 등록.
				if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
				{
					do {
						inventory = CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
						remainCount = remainCount - shortCount;

						(*table) = (*inventory);
						table->idx		= 0;
						table->count	= shortCount;

						table++; rowCount++;
					} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
				}
			}
		}
	}

	length += (sizeof(questInsert->inventory) * rowCount);
	mInventoryDb.reward = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return ( mInventoryDb.reward ) ? ERROR_QUEST_ADD_SUCCESS : ERROR_QUEST_ADD_FAIL;
}

// SaveQuestDelete - 퀘스트 자체를 포기 시킨다
bool cPlayer::SaveQuestDelete( ULONG_PTR socketContext, MSG_REQ_QUEST_DEL* msg, unsigned long dbIdx )
{
	/// 퀘스트 유효성 검사
	cQuestDefine* define = QUESTMAN->GetQuestDefine( msg->questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestDelete() - failed to search questIdx", msg->questIndex );
		assert(0);
		return false;
	}

	if( msg->arrayIdx < 0 || msg->arrayIdx >= MAX_KEEPQUEST )
		return false;

	/// 이미 진행완료된 퀘스트인지 검사
	if( mQuest[msg->arrayIdx].status != eQUEST_PLAYING )
		return false;

	/// 회수해야할 아이템 목록 만들기 - 퀘스트 포기로인한 회수
	// count = 0 : 한 슬롯의 해당 아이템 전체 삭제
	// count = x : 전체 범위중 x 개만큼 삭제

	// 퀘스트아이템은 분리가 안되고, capacity 가 획득갯수보다 크다는 전제하에, 
	// count 를 0 으로 세팅해서 슬롯 자체 삭제를 하도록 한다 
	sQuestItem	takeList[MAX_DUTY+QUEST_ITEM_MAX];
	memset( takeList, 0, sizeof(takeList) );

	unsigned int takeCnt = 0;
	cQuestDuty* duty = define->mQuestDuty;
	if( duty != NULL )
	{
		for ( int i = 0; i < MAX_DUTY; ++i )
		{
			if( duty->mDuty[i].dutyType == eDUTY_COLLECT )
			{
				// 일반 아이템인경우 회수하지 않는다
				TB_ITEM_DEFINE* define		= ITEMMANAGER->GetItemDefineByIndex( duty->mDuty[i].targetIdx );
				PerItemCount*   itemCount	= ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, duty->mDuty[i].targetIdx );
				if( define && define->type == ITEM_QUEST && itemCount != NULL )
				{
					takeList[takeCnt].itemIndex = duty->mDuty[i].targetIdx;
					takeList[takeCnt].count		= itemCount->bag;		// 퀘스트아이템으로 전체 삭제
					takeCnt++;	
				}
			}
		}
	}

	/// 제공한 아이템 회수 - 있는만큼 회수
	if( define->mQuestEnd ) 
	{
		cQuestEnd* questEnd = define->mQuestEnd;

		for( int i = 0; i < QUEST_ITEM_MAX; ++i )
		{
			unsigned long itemIndex		= questEnd->mTakeItem[i].itemIndex;
			unsigned long count			= questEnd->mTakeItem[i].count;
			PerItemCount* itemCount		= ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemIndex );
			if( itemIndex > 0 && count > 0 && itemCount != NULL )
			{
				// 아이템이 존재하면 있는 갯수만큼 회수
				takeList[takeCnt].itemIndex = itemIndex;
				takeList[takeCnt].count		= ( (long)count > (itemCount->bag+itemCount->equip) ) ? (itemCount->bag+itemCount->equip) : count;
				takeCnt++;
			}
		}
	}

	///
	HANDLE         handle      = NULL;
	QUEST_DELETE*  questDelete = (QUEST_DELETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_DELETE );
	long&          rowCount    = questDelete->rowCount;
	TB_INVENTORY*  table       = questDelete->table;
	unsigned long  length	   = sizeof(QUEST_DELETE) - sizeof(questDelete->table);

	// 퀘스트 포기
	questDelete->idx          = dbIdx;
	questDelete->arrIdx       = msg->arrayIdx;
	questDelete->characterIdx = mObject.index;

	// 회수 아이템
	sQuestItem*   items = takeList;
	TB_INVENTORY* tempInv;
	short         remainCount;
	bool          endLoop;

	for ( unsigned int i0 = 0; i0 < takeCnt; i0++, items++ )
	{
		tempInv     = SelectInventory( INVENTORY_BAG0_BEGIN );
		remainCount = (short)items->count;
		endLoop     = false;

		for ( int i1 = INVENTORY_BAG0_BEGIN; (i1 <= INVENTORY_QUEST_END) && (endLoop == false); i1++, tempInv++ )
		{
			if ( tempInv->itemDefineIndex == (long)items->itemIndex )
			{
				short shortCount;

				table->idx             = tempInv->idx;
				table->itemDefineIdx   = tempInv->itemDefineIdx;
				table->itemDefineIndex = tempInv->itemDefineIndex;
				table->number          = tempInv->number;
				table->seal			   = tempInv->seal;
				if ( remainCount > 0 )
				{
					shortCount  = UpdateInventory( tempInv, (-remainCount) );
					remainCount = remainCount + shortCount;
					if ( !(remainCount > 0) )
						endLoop = true;
				}
				else
				{
					shortCount = UpdateInventory( tempInv, (-tempInv->count) );
					endLoop = true;
				}
				table->count = shortCount;

				table++;
				rowCount++;
			}
		}
	}

	length += (sizeof(questDelete->table) * rowCount);
	mInventoryDb.take = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return mInventoryDb.take;
}

bool cPlayer::SaveQuestDeleteAuto( ULONG_PTR socketContext, MSG_REQ_QUEST_DELAUTO* msg, unsigned long dbIdx )
{
	/// 퀘스트 유효성 검사
	cQuestDefine* define = QUESTMAN->GetQuestDefine( msg->questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestDelete() - failed to search questIdx", msg->questIndex );
		assert(0);
		return false;
	}

	if( msg->arrayIdx < 0 || msg->arrayIdx >= MAX_KEEPQUEST )
		return false;

	/// 이미 진행완료된 퀘스트인지 검사
	if( mQuest[msg->arrayIdx].status != eQUEST_PLAYING )
		return false;

	/// 수집 아이템 회수
	sQuestItem	takeList[MAX_DUTY+QUEST_ITEM_MAX];
	memset( takeList, 0, sizeof(takeList) );

	unsigned int takeCnt = 0;
	cQuestDuty* duty = define->mQuestDuty;
	if( duty != NULL )
	{
		for ( int i = 0; i < MAX_DUTY; ++i )
		{
			if( duty->mDuty[i].dutyType == eDUTY_COLLECT )
			{
				// 일반 아이템인경우 회수하지 않는다
				TB_ITEM_DEFINE* define		= ITEMMANAGER->GetItemDefineByIndex( duty->mDuty[i].targetIdx );
				PerItemCount*   itemCount	= ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, duty->mDuty[i].targetIdx );
				if( define && define->type == ITEM_QUEST && itemCount != NULL )
				{
					takeList[takeCnt].itemIndex = duty->mDuty[i].targetIdx;
					takeList[takeCnt].count		= itemCount->bag;		// 퀘스트아이템으로 전체 삭제
					takeCnt++;
				}
			}
		}
	}

	/// 제공한 아이템 회수 - 있는만큼 회수
	cQuestEnd* questEnd = define->mQuestEnd;
	if( questEnd )
	{
		for( int i = 0; i < QUEST_ITEM_MAX; ++i )
		{
			unsigned long itemIndex		= questEnd->mTakeItem[i].itemIndex;
			unsigned long count			= questEnd->mTakeItem[i].count;
			PerItemCount* itemCount		= ITEMCOUNTPOOL->SearchItemCount( &mItemCountRoot, itemIndex );
			if( itemIndex > 0 && count > 0 && itemCount != NULL )
			{
				// 아이템이 존재하면 있는 갯수만큼 회수
				takeList[takeCnt].itemIndex = itemIndex;
				takeList[takeCnt].count		= ( (long)count > (itemCount->bag+itemCount->equip) ) ? (itemCount->bag+itemCount->equip) : count;
				takeCnt++;
			}
		}
	}


	///
	HANDLE         handle      = NULL;
	QUEST_DELETE*  questDelete = (QUEST_DELETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_DELETEAUTO );
	long&          rowCount    = questDelete->rowCount;
	TB_INVENTORY*  table       = questDelete->table;
	unsigned long  length	   = sizeof(QUEST_DELETE) - sizeof(questDelete->table);

	// 퀘스트 포기
	questDelete->idx          = dbIdx;
	questDelete->arrIdx       = msg->arrayIdx;
	questDelete->characterIdx = mObject.index;

	/*-- 회수 아이템
	*/
	sQuestItem*   items = takeList;
	TB_INVENTORY* tempInv;
	short         remainCount;
	bool          endLoop;

	for ( unsigned int i0 = 0; i0 < takeCnt; i0++, items++ )
	{
		tempInv     = SelectInventory( INVENTORY_BAG0_BEGIN );
		remainCount = (short)items->count;
		endLoop     = false;

		for ( int i1 = INVENTORY_BAG0_BEGIN; (i1 <= INVENTORY_QUEST_END) && (endLoop == false); i1++, tempInv++ )
		{
			if ( tempInv->itemDefineIndex == (long)items->itemIndex )
			{
				short shortCount;

				table->idx             = tempInv->idx;
				table->itemDefineIdx   = tempInv->itemDefineIdx;
				table->itemDefineIndex = tempInv->itemDefineIndex;
				table->number          = tempInv->number;
				table->seal			   = tempInv->seal;
				if ( remainCount > 0 )
				{
					shortCount = UpdateInventory( tempInv, (-remainCount) );
					remainCount = remainCount + shortCount;
					if ( !(remainCount > 0) )
						endLoop = true;
				}
				else
				{
					shortCount = UpdateInventory( tempInv, (-tempInv->count) );
					endLoop = true;
				}
				table->count = shortCount;

				table++;
				rowCount++;
			}
		}
	}

	length += (sizeof(questDelete->table) * rowCount);
	mInventoryDb.take = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return mInventoryDb.take;
}


/// SaveQuestComplete Method.
int cPlayer::SaveQuestComplete( ULONG_PTR socketContext, MSG_REQ_NPC_QUEST_COMPLETE* msg, unsigned long dbIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( msg->questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestComplete() - failed to search questIdx", msg->questIndex );
		assert(0);
		return ERROR_QUEST_COMPLETE_FAIL;
	}

	if( msg->arrayIdx < 0 || msg->arrayIdx >= MAX_KEEPQUEST )
		return ERROR_QUEST_COMPLETE_FAIL;

	/// 시간 제한퀘스트일 경우 시간 검사
	cQuestLimit* limit = define->mQuestLimit;
	if( limit && limit->mTimeType != eTIME_NONE )
	{
		long startTime = mQuest[msg->arrayIdx].startTime;
		long restTime = mQuest[msg->arrayIdx].restTime;
		long accTime = NETWORK2->GetAccumTime();

		/// 지난시간 > 남은시간 : 시간오버
		if( accTime - startTime > restTime )
			return ERROR_QUEST_COMPLETE_TIME;
	}

	/// 이미 진행완료된 퀘스트인지 검사
	if( mQuest[msg->arrayIdx].status != eQUEST_PLAYING )
	{
		if( mQuest[msg->arrayIdx].status == eQUEST_COMPLETE )
			return ERROR_QUEST_COMPLETE_ALREADY;
		else
			return ERROR_QUEST_COMPLETE_FAIL;
	}

	/// 수집 의무 아이템 회수 
	sQuestItem   takeList[MAX_DUTY+QUEST_ITEM_MAX];
	memset( takeList, 0, sizeof(takeList) );

	unsigned int takeCnt = 0;
	cQuestDuty*  duty    = define->mQuestDuty;
	if ( duty != NULL )
	{
		for ( int i = 0; i < MAX_DUTY; ++i )
		{
			if ( duty->mDuty[i].dutyType == eDUTY_COLLECT )
			{
				takeList[takeCnt].itemIndex = duty->mDuty[i].targetIdx;
				takeList[takeCnt].count     = duty->mDuty[i].count;
				takeCnt++;
			}
		}
	}

	/// 제공한 아이템 회수
	cQuestEnd* questEnd = define->mQuestEnd;
	if( questEnd )
	{
		for( int i = 0; i < QUEST_ITEM_MAX; ++i )
		{
			unsigned long itemIndex = questEnd->mTakeItem[i].itemIndex;
			unsigned long count = questEnd->mTakeItem[i].count;
			if( itemIndex > 0 && count > 0 )
			{
				takeList[takeCnt].itemIndex = itemIndex;
				takeList[takeCnt].count = count;
				takeCnt++;
			}
		}
	}

	// 검사 - 회수 아이템.
	if ( IsItemTake( takeList, takeCnt ) < takeCnt )
		return ERROR_QUEST_COMPLETE_TAKE;

	// DATABASE - 퀘스트 완료 처리.
	HANDLE			 handle        = NULL;
	QUEST_COMPLETE*  questComplete = (QUEST_COMPLETE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_COMPLETE );
	long&            rowCount      = questComplete->rowCount;
	TB_INVENTORY*    table         = questComplete->table;
	unsigned long	 length        = sizeof(QUEST_COMPLETE) - sizeof(questComplete->table);

	/*-- 퀘스트 완료
	*/
	questComplete->idx          = dbIdx;
	questComplete->arrIdx       = msg->arrayIdx;
	questComplete->characterIdx = mObject.index;
	questComplete->repeatType	= define->mRepeatType;
	questComplete->check		= mQuest[msg->arrayIdx].check;
	questComplete->tutorialIndex = 0;

	// 튜토리얼 변경인덱스
	if( define->mType == eQUEST_TYPE_TUTORIAL )
	{
		questComplete->tutorialIndex = ( questEnd ) ? questEnd->mTutorialModeIndex : 0;
	}

	/*-- 회수 아이템
	*/
	sQuestItem*   items = takeList;
	TB_INVENTORY* tempInv;
	int           remainCount;
	bool          endLoop;

	for ( unsigned int i0 = 0; i0 < takeCnt; i0++, items++ )
	{
		tempInv     = SelectInventory( INVENTORY_BAG0_BEGIN );
		remainCount = items->count;
		endLoop     = false;

		for ( int i1 = INVENTORY_BAG0_BEGIN; (i1 <= INVENTORY_QUEST_END) && (endLoop == false); i1++, tempInv++ )
		{
			if ( tempInv->itemDefineIndex == (long)items->itemIndex )
			{
				short shortCount;

				table->idx             = tempInv->idx;
				table->itemDefineIdx   = tempInv->itemDefineIdx;
				table->itemDefineIndex = tempInv->itemDefineIndex;
				table->number          = tempInv->number;
				table->seal			   = tempInv->seal;
				if ( remainCount > 0 )
				{
					shortCount = UpdateInventory( tempInv, (short)(-remainCount) );
					remainCount = remainCount + shortCount;
					if ( !(remainCount > 0) )
						endLoop = true;
				}
				else
				{
					shortCount = UpdateInventory( tempInv, (-tempInv->count) );
					endLoop = true;
				}
				table->count = shortCount;

				table++;
				rowCount++;
			}
		}
	}

	length += (sizeof(questComplete->table) * rowCount);
	mInventoryDb.reward = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return ( mInventoryDb.reward ) ? ERROR_QUEST_COMPLETE_SUCCESS : ERROR_QUEST_COMPLETE_FAIL; 
}

/// SaveQuestReward Method.
int cPlayer::SaveQuestReward( ULONG_PTR socketContext, MSG_REQ_NPC_QUEST_REWARD* msg, unsigned long dbIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( msg->questIndex );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST SaveQuestReward() - failed to search questIdx", msg->questIndex );
		assert(0);
		return ERROR_QUEST_REWARD_FAIL;
	}

	if( msg->arrayIdx < 0 || msg->arrayIdx >= MAX_KEEPQUEST )
		return ERROR_QUEST_REWARD_FAIL;

	/// 진행완료된 퀘스트인지 검사
	if( mQuest[msg->arrayIdx].status != eQUEST_COMPLETE )
		return ERROR_QUEST_REWARD_COMPLETE;

	/// 만기일 계산
	/// 시간 지정 퀘스트인 경우,
	TIMESTAMP_STRUCT validTime;
	memset( &validTime, 0, sizeof(validTime) );

	if( define->mEventType == eQUEST_TIMEEVENT )
	{
		validTime = TRIGGERMAN->GetValidTime( msg->questIndex );
	}

	/// 제공 아이템 총 목록
	sQuestItem itemList[QUEST_ITEM_MAX];
	unsigned int listCnt = 0;

	// 기본보상 아이템 목록
	cQuestReward* defaultReward = define->mDefaultReward;
	if ( defaultReward )
	{
		sQuestItem* item = defaultReward->mRewardItem;
		for( int i = 0; i < QUEST_ITEM_MAX; ++i, ++item )
		{
			if( item->itemIndex != 0 && item->count > 0 )
			{
				itemList[ listCnt ].itemIndex = item->itemIndex;
				itemList[ listCnt ].count = item->count;
				listCnt++;
			}
		}
	}

	// 선택 보상 처리
	if( define->mSelectReward && define->mSelectCount > 0 )
	{
		/// 검사 - 선택 갯수 검사
		if( define->mSelectCount != msg->rowCount )
			return ERROR_QUEST_REWARD_COUNT;

		for ( unsigned int i = 0; i < msg->rowCount; ++i )
		{
			sRewardData& data = msg->table[i];
			if( data.type != eREWARD_ITEM )
				continue;

			/// 선택보상 찾기
			int count = QUESTMAN->IsSelectRewardItem( msg->questIndex, data.index );
			if( count == 0 )
				continue;

			itemList[ listCnt ].itemIndex = data.index;
			itemList[ listCnt ].count = count;
			listCnt++;
		}
	}

	//
	if ( !(msg->rowCount < QUEST_BUFF_MAX) )
		return ERROR_QUEST_REWARD_FAIL;

	// 검사 - 보상 금  액 .
	if( defaultReward )
	{
		if ( IsAddMoney( defaultReward->mMoney ) == false )
			return ERROR_QUEST_REWARD_MONEY;
	}
	
	// 검사 - 보상 아이템.
	if ( IsItemReward( itemList, listCnt, INVENTORY_BAG_BEGIN, mHeroInfo.BagEnd ) < listCnt )
		return ERROR_QUEST_REWARD_ITEM;

    // 검사 - 최대 보유갯수 제한
    if( IsMaxInventory( itemList, listCnt ) == true )
        return ERROR_QUEST_REWARD_MAX_ITEM;


	// DATABASE - 퀘스트 보상 처리.
	HANDLE         handle       = NULL;
	QUEST_REWARD*  questReward  = (QUEST_REWARD*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_QUEST_REWARD );
	long&          itemRowCount = questReward->itemRowCount;
	TB_INVENTORY*  table        = questReward->table;
	unsigned long  length       = sizeof(QUEST_REWARD) - sizeof(questReward->skillClassIdx);

	/*-- 퀘스트 완료 --*/
	questReward->idx          = dbIdx;
	questReward->arrIdx       = msg->arrayIdx;
	questReward->characterIdx = mObject.index;
	questReward->thru		  = validTime;

	/*-- 보상 금  액 --*/
	questReward->characterMoney = ( defaultReward ) ? defaultReward->mMoney : 0;

	/*-- 보상 전  직 ( 기본 보상만 가능 ) --*/
	questReward->jobIdx = ( defaultReward ) ? defaultReward->mChangeJobIndex : 0;
	questReward->race = mPlayerInfo.Race;
	if( defaultReward && defaultReward->mChangeJobIndex != 0 )
		questReward->jobStep = STATUSSCRIPT->GetJobStep( defaultReward->mChangeJobIndex );
	else
		questReward->jobStep = 0;
	
	/*-- 보상 스  킬 ( 기본 보상만 가능 ) --*/
	questReward->skillPoint = ( defaultReward ) ? defaultReward->mSkillPoint : 0;

	/*-- 보상 세  력 ( 기본 보상만 가능 ) --*/
	questReward->forceType = ( defaultReward ) ? defaultReward->mForceType : 0;

	/*-- 보상 호  칭 ( 기본 보상만 가능 ) --*/
	questReward->titleIdx = ( defaultReward ) ? defaultReward->mGiveTitleIndex : 0;

	/*-- 선택 보  상 (버 프) --*/
	int buffCnt = 0;
	for( unsigned int i = 0; i < msg->rowCount; ++i )
	{
		sRewardData& data = msg->table[i];
		if( data.type != eREWARD_SKILL )
			continue;

		bool ret = QUESTMAN->IsSelectRewardBuff( msg->questIndex, data.index );
		if( ret == true )
		{
			questReward->bufIndex[ buffCnt ] = data.index;
			buffCnt++;
		}
	}
	questReward->bufRowCount = buffCnt;

	/*-- 보상 아이템 --*/
	sQuestItem* items = itemList;

	for ( unsigned int i0 = 0; i0 < listCnt; i0++, items++ )
	{
		TB_ITEM_DEFINE* itemDefine  = ITEMMANAGER->GetItemDefineByIndex( items->itemIndex );
		short           remainCount = (short)items->count;

		TB_INVENTORY*   tempInv;
		short           shortCount;
		short           number;

		if ( itemDefine != NULL && remainCount > 0 )
		{
			// 아이템 퀘스트 보상. 겹치기 검사 및 처리.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, true ) == true )
			{
				do {
					tempInv     = SelectInventory( number );
					shortCount  = UpdateInventory( tempInv, remainCount );
					remainCount = remainCount - shortCount;

					(*table)	 = (*tempInv);
					table->count = shortCount;

					table++; itemRowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, true ) == true );
			}
			// 아이템 퀘스트 보상. 생성 검사 및 등록.
			if ( FindFirstInventory( itemDefine, &number, remainCount, &shortCount, false ) == true )
			{
				do {
					tempInv = CreateInventory( itemDefine, number, remainCount, &(shortCount=0) );
					remainCount = remainCount - shortCount;

					(*table) = (*tempInv);
					table->idx		= 0;
					table->count    = shortCount;

					table++; itemRowCount++;
				} while ( FindNextInventory( itemDefine, &(++number), remainCount, &shortCount, false ) == true );
			}
		}
	}

	mInventoryDb.reward = NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, length );
	return ( mInventoryDb.reward ) ? ERROR_QUEST_REWARD_SUCCESS : ERROR_QUEST_REWARD_FAIL; 
}

// 퀘스트 관련 드랍해야할 아이템 갯수 획득
unsigned int cPlayer::GetDropItemCount( unsigned int arrIdx, unsigned long itemIdx )
{
	if( arrIdx < 0 || arrIdx >= MAX_KEEPQUEST )
		return 0;

	if( mQuest[arrIdx].idx == 0 )
		return 0;

	cQuestDefine* define = QUESTMAN->GetQuestDefine( mQuest[arrIdx].questIdx );
	if ( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST GetDropItemCount() - failed to search questIdx", mQuest[arrIdx].questIdx );
		assert(0);
		return 0;
	}

	/// 해당 퀘스트의 의무중, 찾는 아이템이 있는지 찾아서, 있으면 필요 갯수를 리턴한다
	cQuestDuty* duty = define->mQuestDuty;
	if ( duty == NULL )
		return 0;

	/// 동일한 아이템을 찾아 갯수 세팅
	unsigned int count = 0;
	int          num   = 0;
	for( int i = 0; i < MAX_DUTY; ++i )
	{
		if( duty->mDuty[i].dutyType == eDUTY_COLLECT && 
			duty->mDuty[i].targetIdx == itemIdx )
		{
			count = duty->mDuty[i].count;
			num = i;
			break;
		}
	}

	/// 필요갯수가 0 이상이면, 실제 가지고있는 갯수와 비교한다
	if( count > 0 )
	{
		TB_QUEST_PROGRESS* quest = mQuest + arrIdx;

		/// 이미 완료된 퀘스트이면 필요갯수 0
		if( quest->check == define->mComplete )
			return 0;

		__int64 end = 0xFF;
		for( int e = 0; e < num; ++e )
			end = end << 8;

		/// 해당 의무가 완료이면 필요갯수 0
		if( (quest->check & end) == (define->mComplete & end) )
			return 0;

		/// 미완료이면 필요갯수 비교 검사
		__int64 itemCount = (define->mComplete & end) - (quest->check & end);
		for( int c = 0; c < num; ++c )
			itemCount = itemCount >> 8;

		count = (unsigned int)itemCount;
	}
	return count;
}

bool cPlayer::SetCheatHideMode( bool mode )
{
	if( mode == mCheatHideMode )
		return false;

	if ( mode )
	{
		GRIDMANAGER->SetHidePlayer( this, mode );
		mCheatHideMode = mode;
	}
	else
	{
		mCheatHideMode = mode;
		GRIDMANAGER->SetHidePlayer( this, mode );
	}

	return true;
}

void cPlayer::CheatRecover()
{
	InitHP( GetMaxHP() );
	InitMP( GetMaxMP() );

	//if( ChangeState( eOBJECT_STATE_IDLE, true ) == false )
	//{
	//	assert(NULL);
	//	NETWORK2->PostServerEvent("Player[%d] CheatRebirth ChangeState( eOBJECT_STATE_IDLE, true ) == false", mObject.index );
	//}

	//mSkillResurrectionHP = 0;
	//mSkillResurrectionMP = 0;
	//mIsSkillResurrection = false;
}

void cPlayer::CheatRebirth()
{
	InitHP( GetMaxHP() );
	InitMP( GetMaxMP() );

	if( ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION ) == false )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("Player[%d] CheatRebirth ChangeState( eOBJECT_STATE_IDLE, true ) == false", mObject.index );
	}

	mSkillResurrectionHP = 0;
	mSkillResurrectionMP = 0;
	mIsSkillResurrection = false;
}

void cPlayer::CheatStop( unsigned long duration )
{
	int error = ERROR_CHEAT_STOP_FAIL;

	/// 정지 상태 변경
	if( ChangeState( eOBJECT_STATE_STOP ) == true )
	{

		SetStateStop( eSTOP_CHEATSTOP );

		error = ERROR_CHEAT_STOP_SUCCESS;
		mCheatStopDuration = NETWORK2->GetAccumTime() + duration;
		SetCheatUndeadMode( true );
	}

	/// 정지 메세지 발송
	NETWORK2->SendMsgError( mConnectionIdx, NM_CHEAT, NM_CHEAT_STOP_RES, error );

	/// 성공시 싱크 발송
	if( error == ERROR_CHEAT_STOP_SUCCESS )
	{
		/// 동기
		MSG_SYN_CHEAT_STOP syn;
		syn.Category = NM_CHEAT;
		syn.Protocol = NM_CHEAT_STOP_SYN;
		syn.mObjIndex = mObject.index;
		NETWORK2->QuickSend( this, (char*)&syn, sizeof(syn) );
	}
}

void cPlayer::CheatCancelStop()
{
	int error = ERROR_CHEAT_STOP_FAIL;

	if( GetStateStop() == eSTOP_CHEATSTOP )
	{
		/// 정지 상태 해제
		if( ChangeState( eOBJECT_STATE_IDLE ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cPlayer::CheatCancelStop[%d][%d,%d] State ERROR", mObject.index, GetState(), GetStateStop() );
		}

		SetStateStop( eSTOP_NONE );

		error = ERROR_CHEAT_STOP_SUCCESS;
		mCheatStopDuration = 0;
		SetCheatUndeadMode( false );
	}
	else
		return;
	//error = ERROR_CHEAT_STOP_NOT;

	/// 정지 해제 메세지 발송
	NETWORK2->SendMsgError( mConnectionIdx, NM_CHEAT, NM_CHEAT_CANCEL_STOP_RES, error );

	/// 성공시 싱크 발송
	if( error == ERROR_CHEAT_STOP_SUCCESS )
	{
		/// 동기
		MSG_SYN_CHEAT_CANCEL_STOP syn;
		syn.Category = NM_CHEAT;
		syn.Protocol = NM_CHEAT_CANCEL_STOP_SYN;
		syn.mObjIndex = mObject.index;
		NETWORK2->QuickSend( this, (char*)&syn, sizeof(syn) );
	}
}

void cPlayer::SetCheatSpeedUp( unsigned long speedUp )
{
	if( speedUp >= eCHEATSPEEDUP_MAX )
		return;

	mCheatSpeedUp = (eCHEAT_SPEEDUP)speedUp;

	STATUSCALC->CalcPlayerExtensionGlobal( mObject );
}

void cPlayer::CalcCheatSpeedUp()
{
	switch( mCheatSpeedUp )
	{
	case eCHEATSPEEDUP_50UP:	mStatus2.mMoveSpeed = mStatus2.mMoveSpeed * 1.5f; break;
	case eCHEATSPEEDUP_100UP:	mStatus2.mMoveSpeed = mStatus2.mMoveSpeed * 2.0f; break;
	case eCHEATSPEEDUP_150UP:	mStatus2.mMoveSpeed = mStatus2.mMoveSpeed * 2.5f; break;
	case eCHEATSPEEDUP_200UP:	mStatus2.mMoveSpeed = mStatus2.mMoveSpeed * 3.0f; break;
	case eCHEATSPEEDUP_250UP:	mStatus2.mMoveSpeed = mStatus2.mMoveSpeed * 3.5f; break;
	}
}

/// 호칭관리
void cPlayer::ClearHaveTitleList()
{
	mHaveTitleSet.Clear();
}

bool cPlayer::AddHaveTitle( unsigned long titleIdx )
{
	cTitleDefine* define = TITLEMAN->GetTitleDefine( titleIdx );
	if( !define )
	{
		assert(0);
		return false;
	}

	if( mHaveTitleSet.Insert( titleIdx ) == false )
	{
		assert(0);
		return false;
	}
	return true;
}

bool cPlayer::IsHaveTitle( unsigned long titleIdx )
{
	return mHaveTitleSet.Find( titleIdx ) != mHaveTitleSet.End();
}


/// 클라이언트에 보유호칭 목록 send
bool cPlayer::SendHaveTitleList( ULONG_PTR socketContext )
{
	HANDLE						handle    = NULL;
	MSG_RES_PLAYER_TITLELIST*	sendMsg   = (MSG_RES_PLAYER_TITLELIST*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_PLAYER, NM_PLAYER_TITLE_LIST_RES );
	unsigned long				length    = sizeof(MSG_RES_PLAYER_TITLELIST) - sizeof(sendMsg->mTitleIndex);
	unsigned long*				titleList = sendMsg->mTitleIndex;

	cHaveTitleSet::cIterator i = mHaveTitleSet.Begin();
	cHaveTitleSet::cIterator end = mHaveTitleSet.End();
	for( ; i != end; ++i, ++titleList )
	{
		unsigned long idx = (unsigned long)(*i);
		*titleList = idx;

		sendMsg->rowCount++;
	}

	length += (sendMsg->rowCount * sizeof(sendMsg->mTitleIndex));
	return NETWORK2->SendMsgRoot( handle, length );
}

/// 해당 호칭 쏘기
void cPlayer::SendHaveTitle( unsigned long titleIdx )
{
	HANDLE                   handle  = NULL;
	MSG_RES_PLAYER_TITLEADD* sendMsg = (MSG_RES_PLAYER_TITLEADD*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_TITLE_ADD_RES );
	if ( sendMsg !=NULL )
	{
		sendMsg->mTitleIndex = titleIdx;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TITLEADD) );
	}
}

bool cPlayer::InitTitle()
{
    if( mPlayerExrInfo.mTitleIndex == 0 )
        return true;

	cTitleDefine* define = TITLEMAN->GetTitleDefine( mPlayerExrInfo.mTitleIndex );
	if ( !define )
	{
		NETWORK2->PostServerEvent("ERROR InitTitle(%d,%d)", mObject.index, mPlayerExrInfo.mTitleIndex );
		assert(0);
		return false;
	}

	if ( define->mInfluenceIndex > 0 )
	{
		unsigned long uniqueIdx = SKILLMANAGER->AddInfSwitch( mObject, mObject, define->mInfluenceIndex, 0, false );
		if ( uniqueIdx != 0 )
			mTitleUniqIndex = uniqueIdx;
		else
			return false;
	}
	return true;
}

/// 현재 호칭으로 설정
bool cPlayer::ChangeTitle( unsigned long titleIdx )
{
	if ( mPlayerExrInfo.mTitleIndex == titleIdx )
		return true;

	/// 효과적용
	if ( titleIdx > 0 )
	{
		cTitleDefine* define = TITLEMAN->GetTitleDefine( titleIdx );
		if ( !define )
		{
			NETWORK2->PostServerEvent("ERROR ChangeTitle(%d,%d)", mObject.index, titleIdx );
			assert(0);
			return false;
		}

		unsigned long uniqueIdx = SKILLMANAGER->AddInfSwitch( mObject, mObject, define->mInfluenceIndex, 0, true );
		if ( uniqueIdx == 0 )
			return false;

		mTitleUniqIndex = uniqueIdx;
	}
	else
	{
		SKILLMANAGER->DeleteInfluence( mObject, mTitleUniqIndex );
		mTitleUniqIndex = 0;
	}

	/// 설정
	mPlayerExrInfo.mTitleIndex = titleIdx;

	/// 다른 유저에게 쏘기
	MSG_SYN_PLAYER_TITLE_SET sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );
	sendMsg.Category = NM_PLAYER;
	sendMsg.Protocol = NM_PLAYER_TITLE_SET_SYN;
	sendMsg.mCharacterIndex = GetObjectID();
	sendMsg.mTitleIndex     = titleIdx;
	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_PLAYER_TITLE_SET) );

	SendEventQuestList( eQUEST_SETTITLE );
	return true;
}

// 튜토리얼 모드 변경 보내기
void cPlayer::SetTutorialMode( int modeIndex, bool send )
{
	// 튜토리얼 맵이 아니면 처리하지 않는다
	if( NETWORK2->GetInDunMapNum() != MAP_TUTORIAL )
		return;

	mTutorialModeIndex = modeIndex;

	if( send == true )
	{
		HANDLE		                 handle  = NULL;
		MSG_SYN_TUTORIAL_CHANGEMODE* sendMsg = (MSG_SYN_TUTORIAL_CHANGEMODE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_TUTORIAL, NM_TUTORIAL_CHANGEMODE_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mModeIndex = modeIndex;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TUTORIAL_CHANGEMODE) );
		}
	}
}

void cPlayer::SetPartyIndex( unsigned long idx )
{ 
	mPlayerExrInfo.mPartyIndex = idx;

	MSG_SYN_PLAYER_PARTY_SET sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );
	sendMsg.Category = NM_PLAYER;
	sendMsg.Protocol = NM_PLAYER_PARTY_SET_SYN;
	sendMsg.mCharacterIndex = GetObjectID();
	sendMsg.mPartyIndex     = mPlayerExrInfo.mPartyIndex;
	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_PLAYER_PARTY_SET) );
}

void cPlayer::GetPartyUserData(sPartyUserData* data)
{
	data->userIndex = mObject.index;
	wcscpy( data->name, mPlayerInfo.strName );
	data->job		= (unsigned short)mPlayerInfo.Job;
	data->level		= mPlayerInfo.Level;
	data->mapNum	= GetMapDataNumber();
	data->roomNum	= GetRoomDataNumber();
	data->channelNum = NETWORK2->GetChannelNum();
	data->posX		= mObjectPos.x;
	data->posY		= mObjectPos.y;
	data->hp		= mPlayerExrInfo.RestHP;
	data->maxhp		= mPlayerExrInfo.MaxHP;
	data->mp		= mPlayerExrInfo.RestMP;
	data->maxmp		= mPlayerExrInfo.MaxMP;
}

void cPlayer::SendPartyDiceStart( )
{
	NETWORK2->SendMsgRoot( mConnectionIdx, NM_PARTY, NM_PARTY_DICE_START_SYN );
}

void cPlayer::SendPartyAddoneSyn(sPartyUserData* userData)
{
	HANDLE                handle  = NULL;
	MSG_SYN_PARTY_ADDONE* sendMsg = (MSG_SYN_PARTY_ADDONE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_ADDONE_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->mTable = (*userData);
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_ADDONE) );
	}
}

void cPlayer::SendPartyOutSyn(unsigned long playerIdx)
{
	HANDLE             handle  = NULL;
	MSG_SYN_PARTY_OUT* sendMsg = (MSG_SYN_PARTY_OUT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_OUT_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->UserIndex = playerIdx;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_OUT) );
	}
}

void cPlayer::SendPartyThrowSyn(unsigned long playerIdx)
{
	HANDLE               handle  = NULL;
	MSG_SYN_PARTY_THROW* sendMsg = (MSG_SYN_PARTY_THROW*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_THROW_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->UserIndex = playerIdx;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_THROW) );
	}
}

void cPlayer::SendPartyLeaderSyn(unsigned long leaderIdx)
{
	HANDLE                      handle  = NULL;
	MSG_SYN_PARTY_CHANGELEADER*	sendMsg = (MSG_SYN_PARTY_CHANGELEADER*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_CHANGELEADER_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->UserIndex = leaderIdx;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_CHANGELEADER) );
	}
}

void cPlayer::SendPartyDivideType(char type)
{
	HANDLE                     handle  = NULL;
	MSG_SYN_PARTY_CHANGEDIVDE* sendMsg = (MSG_SYN_PARTY_CHANGEDIVDE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_CHANGEDIVIDE_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->type = type;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_CHANGEDIVDE) );
	}
}

void cPlayer::SendPartyDataSyn(unsigned long connectionIdx)
{
	HANDLE              handle  = NULL;
	MSG_SYN_PARTY_DATA* sendMsg = (MSG_SYN_PARTY_DATA*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_PARTY, NM_PARTY_DATA_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->userIndex = mObject.index;
		sendMsg->level     = mPlayerInfo.Level;
		sendMsg->maxHP     = mPlayerExrInfo.MaxHP;
		sendMsg->hp        = mPlayerExrInfo.RestHP;
		sendMsg->maxMP     = mPlayerExrInfo.MaxMP;
		sendMsg->mp        = mPlayerExrInfo.RestMP;
		sendMsg->posX      = mObjectPos.x;
		sendMsg->posY      = mObjectPos.y;
		sendMsg->RowCount  = 0;

		// 버프리스트 담기
		cSkillInfluenceSet* influenceSet = GetInfluenceSet( );

		if ( influenceSet && influenceSet->GetSize( ) > 0  )
		{
			cSkillInfluenceSet::cIterator begin = influenceSet->Begin( );
			cSkillInfluenceSet::cIterator end   = influenceSet->End( );

			short&            rowCount = sendMsg->RowCount;
			sInfluenceClient* table    = sendMsg->mTable;
			cInfluenceObject* object   = 0;

			for ( ; begin != end; ++begin, ++table )
			{
				object = SKILLMANAGER->GetInfluence( (unsigned long)(*begin) );
				if ( object != NULL )
				{
					table->mInfluenceClassIdx = object->GetInfluenceClassIdx();
					table->mUniqueIdx         = object->GetUniqueIdx();
					table->mRestTime          = 0;
					rowCount++;
				}
				else
					NETWORK2->PostServerEvent("cPartyManager::SendUpdateParty[%d, %d]", mObject.index, (unsigned long)(*begin) );
			}
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
}

void cPlayer::SendPartyDataLevelSyn(unsigned long connectionIdx)
{
	HANDLE                   handle  = NULL;
	MSG_SYN_PARTYLEVEL_DATA* sendMsg = (MSG_SYN_PARTYLEVEL_DATA*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_PARTY, NM_PARTY_DATA_LEVEL_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->userIndex = mObject.index;
		sendMsg->level     = mPlayerInfo.Level;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYLEVEL_DATA) );
	}
}

void cPlayer::SendPartyDataLevelSyn(unsigned long playerIdx, unsigned char level)
{
	HANDLE                   handle  = NULL;
	MSG_SYN_PARTYLEVEL_DATA* sendMsg = (MSG_SYN_PARTYLEVEL_DATA*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_DATA_LEVEL_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->userIndex = playerIdx;
		sendMsg->level     = level;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYLEVEL_DATA) );
	}
}

void cPlayer::SendPartyMapchangeSyn(unsigned long connectionIdx)
{
	HANDLE                   handle  = NULL;
	MSG_SYN_PARTY_MAPCHANGE* sendMsg = (MSG_SYN_PARTY_MAPCHANGE*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_PARTY, NM_PARTY_MAPCHANGE_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->UserIndex = mObject.index;
		sendMsg->MapNum    = GetMapDataNumber();
		sendMsg->RoomNum   = GetRoomDataNumber();
		sendMsg->ChannelNum = NETWORK2->GetChannelNum();

		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_MAPCHANGE) );
	}
}

void cPlayer::SendPartyMapchangeSyn(unsigned long playerIdx, short map, short room, DWORD channelnum )
{
	HANDLE                   handle  = NULL;
	MSG_SYN_PARTY_MAPCHANGE* sendMsg = (MSG_SYN_PARTY_MAPCHANGE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PARTY, NM_PARTY_MAPCHANGE_SYN );
	if ( sendMsg != NULL )
	{
		sendMsg->UserIndex = playerIdx;
		sendMsg->MapNum    = map;
		sendMsg->RoomNum   = room;
		sendMsg->ChannelNum = channelnum;
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_MAPCHANGE) );
	}
}

void cPlayer::SendPartyDeleteSyn( )
{
	NETWORK2->SendMsgRoot( mConnectionIdx, NM_PARTY, NM_PARTY_DELETE_SYN );
}

void cPlayer::SetPartyUnionIndex( unsigned long idx )
{
	mPlayerExrInfo.mPartyUnionIndex = idx;

	MSG_SYN_PLAYER_PARTYUNION_SET sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );
	sendMsg.Category = NM_PLAYER;
	sendMsg.Protocol = NM_PLAYER_PARTYUNION_SET_SYN;
	sendMsg.mCharacterIndex = GetObjectID();
	sendMsg.mPartyUnionIndex     = mPlayerExrInfo.mPartyUnionIndex;
	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_PLAYER_PARTYUNION_SET) );
}

bool cPlayer::ChangeGuild( unsigned long guildIndex, char position )
{
	mPlayerExrInfo.mGuildIndex = guildIndex;
	mPlayerExrInfo.mGuildPosition = position;

	struct tm initTm;
	memset( &initTm, 0, sizeof(initTm) );
	initTm.tm_year = 1900;
	initTm.tm_mon  = 1;
	initTm.tm_mday = 1;

	if( guildIndex == 0 )
	{
		memset( &mPlayerExrInfo.mGuildName, 0, sizeof(mPlayerExrInfo.mGuildName) );
		
		//
		mPlayerExrInfo.mGuildMarkDate = initTm;
		mPlayerExrInfo.mUseGuildMark  = false;
	}
	else
	{
		// 길드명설정
		wchar_t* name = GUILDMAN->GetGuildName( guildIndex );
		wcsncpy( mPlayerExrInfo.mGuildName, name, MAX_GUILDNAME_BUFFER_SIZE ); 

		// 길드마크 설정
		GUILDMAN->GetGuildMarkDate( guildIndex, mPlayerExrInfo.mGuildMarkDate, mPlayerExrInfo.mUseGuildMark );
	}

	MSG_SYN_PLAYER_GUILD_SET sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );
	sendMsg.Category = NM_PLAYER;
	sendMsg.Protocol = NM_PLAYER_GUILD_SET_SYN;
	sendMsg.mCharacterIndex = GetObjectID();
	sendMsg.mGuildIndex     = guildIndex;
	sendMsg.mPosition       = position;
	sendMsg.mMarkRegistDate = mPlayerExrInfo.mGuildMarkDate;
	sendMsg.mUseGuildMark	= mPlayerExrInfo.mUseGuildMark;

	int len = wcslen( mPlayerExrInfo.mGuildName );
	if( len > 0 )
		wcsncpy( sendMsg.mName, mPlayerExrInfo.mGuildName, MAX_GUILDNAME_BUFFER_SIZE ); 

	NETWORK2->QuickSendExcept( this, (char*)&sendMsg, sizeof(MSG_SYN_PLAYER_GUILD_SET) );
	return true;
}

void cPlayer::ChangeGuildMarkRegistDate()
{
	if( mPlayerExrInfo.mGuildIndex == 0 )
	{
		NETWORK2->PostServerEvent("ERROR ChangeGuildMarkRegistDate(%d), guildIndex == 0", mObject.index );
		return;
	}

	// 길드마크 설정
	GUILDMAN->GetGuildMarkDate( mPlayerExrInfo.mGuildIndex, mPlayerExrInfo.mGuildMarkDate, mPlayerExrInfo.mUseGuildMark );

	MSG_SYN_PLAYER_GUILDMARK_CHANGE sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );
	sendMsg.Category = NM_PLAYER;
	sendMsg.Protocol = NM_PLAYER_GUILDMARK_CHANGE_SYN;
	sendMsg.mCharacterIndex = GetObjectID();
	sendMsg.mMarkRegistDate = mPlayerExrInfo.mGuildMarkDate;
	sendMsg.mUseGuildMark	= mPlayerExrInfo.mUseGuildMark;

	NETWORK2->QuickSend( this, (char*)&sendMsg, sizeof(MSG_SYN_PLAYER_GUILDMARK_CHANGE) );
}

void cPlayer::ChangeGuildPosition( char position )
{
	mPlayerExrInfo.mGuildPosition = position;
}

void cPlayer::SendGuildConnectSyn( unsigned long characterIdx, BYTE connect, DWORD channelNum )
{
	HANDLE			handle = NULL;
	MSG_SYN_GUILDUSER_CONNECT* sendMsg = (MSG_SYN_GUILDUSER_CONNECT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_GUILD, NM_GUILDUSER_CONNECT_SYN );
	if( sendMsg != NULL )
	{
		sendMsg->characterIndex = characterIdx;
		sendMsg->connect		= connect;
		sendMsg->channelNum		= channelNum;
		NETWORK2->SendMsgRoot ( handle, sizeof(MSG_SYN_GUILDUSER_CONNECT) );
	}
}

void cPlayer::GetGuildUserData( sGuildConnect* data )
{
	data->characterIdx = mObject.index;
	data->job		= (unsigned short)mPlayerInfo.Job;
	data->level		= mPlayerInfo.Level;
}

unsigned short cPlayer::GetJobUsedSkillPoint( unsigned char jobstep )
{
	if( jobstep > mJobUsedSpArr.GetSize() )
	{
		assert(0);
		return 0;
	}
	return mJobUsedSpArr[jobstep];
}

void cPlayer::AddTargetingMonster( unsigned long monsterIdx )
{
	mTargetingMonsterSet.Insert( monsterIdx );
}

void cPlayer::DelTargetingMonster( unsigned long monsterIdx )
{
	mTargetingMonsterSet.Erase( monsterIdx );
}

void cPlayer::ClearTargetingMonster()
{
	cTargetingMonsterSet::cIterator start = mTargetingMonsterSet.Begin();
	cTargetingMonsterSet::cIterator end = mTargetingMonsterSet.End();

	while( start != end )
	{
		unsigned long monsterIdx = (*start++);
		if ( monsterIdx == 0 )
			continue;

		cMonster* mon = GRIDMANAGER->GetMonster( monsterIdx ); //OBJECTMANAGER->GetMonster( monsterIdx );
		if ( mon != NULL )
			mon->DelTakeDamagePlayer( GetObjectID() );
	}
	mTargetingMonsterSet.Clear();
}

void cPlayer::DietargetingMonster()
{
	cTargetingMonsterSet::cIterator start = mTargetingMonsterSet.Begin();
	cTargetingMonsterSet::cIterator end = mTargetingMonsterSet.End();

	while( start != end )
	{
		unsigned long monsterIdx = (*start++);
		if ( monsterIdx == 0 )
			continue;

		cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
		if ( mon != NULL )
			mon->ClearDistressPointPlayer( GetObjectID() );
	}
}

void cPlayer::GetTargetingMonster( tHashSet<unsigned long>& monsterSet )
{
	cTargetingMonsterSet::cIterator start = mTargetingMonsterSet.Begin();
	cTargetingMonsterSet::cIterator end = mTargetingMonsterSet.End();

	while( start != end )
	{
		unsigned long monsterIdx = (*start++);
		if ( monsterIdx == 0 )
			continue;

		cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
		if ( mon != NULL )
		{
			monsterSet.Insert( monsterIdx );
		}
	}
}

void cPlayer::HealAttackToMonster( unsigned long healerIdx, unsigned long heal, long distressPoint, eTAKEDAMAGE_TYPE type )
{
	cTargetingMonsterSet::cIterator start = mTargetingMonsterSet.Begin();
	cTargetingMonsterSet::cIterator end = mTargetingMonsterSet.End();

	sObject healer = { eOBJECTTYPE_PLAYER, healerIdx };

	while( start != end )
	{
		unsigned long monsterIdx = (*start++);
		if ( monsterIdx == 0 )
			continue;

		cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
		if ( mon != NULL )
			mon->AddTakeDamage( healer, heal, distressPoint, type );
	}
}

/// DB에 저장
bool cPlayer::SaveFortuneData(ULONG_PTR socketContext)
{
	HANDLE          handle        = NULL;
	FORTUNE_UPDATE* fortuneUpdate = (FORTUNE_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_FORTUNE_UPDATE );
	if ( fortuneUpdate != NULL )
	{
		TIMESTAMP_STRUCT* validThru = fortuneUpdate->thru;
		TIMESTAMP_STRUCT* thru      = mFortuneThru;

		/// 정보 담기
		for ( int i = 0; i < 5; ++i, ++validThru, ++thru )
			(*validThru) = (*thru);

		fortuneUpdate->characterIdx = mObject.index;
		fortuneUpdate->color        = mPlayerExrInfo.mTodayColor;
		::wcsncpy( fortuneUpdate->word, mPlayerExrInfo.mTodayWord, MAX_TODAYWORD_BUFFER_SIZE ); 
	}
	return NETWORK2->SendSQL( (PerSocketContext*)socketContext, handle, sizeof(FORTUNE_UPDATE));
}

bool cPlayer::SendInitTodayWord()
{
	if ( wcslen( mPlayerExrInfo.mTodayWord ) > 0 )
	{
		HANDLE                           handle  = NULL;
		MSG_RES_PLAYER_TODAYWORD_CHANGE* sendMsg = (MSG_RES_PLAYER_TODAYWORD_CHANGE*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx, NM_PLAYER, NM_PLAYER_TODAYWORD_CHANGE_RES );
		if ( sendMsg != NULL )
		{
			sendMsg->mWordColor = mPlayerExrInfo.mTodayColor;
			sendMsg->ErrorCode  = ERROR_PLAYER_TODAYWORD_CHANGE_INIT;
			::wcsncpy( sendMsg->mWord, mPlayerExrInfo.mTodayWord, MAX_TODAYWORD_BUFFER_SIZE ); 
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TODAYWORD_CHANGE) );
		}
	}
	return true;
}

/// 오늘의 한마디 변경
bool cPlayer::ChangeTodayWord( ULONG_PTR socketContext, wchar_t* word, unsigned long color )
{	
	mPlayerExrInfo.mTodayColor = color;

	if ( word == NULL )
		memset( &mPlayerExrInfo.mTodayWord, 0, sizeof(mPlayerExrInfo.mTodayWord) );
	else
		wcsncpy( mPlayerExrInfo.mTodayWord, word, MAX_TODAYWORD_BUFFER_SIZE ); 

	/// HERO에게 쏘기
	HANDLE                           handle  = NULL;
	MSG_RES_PLAYER_TODAYWORD_CHANGE* sendMsg = (MSG_RES_PLAYER_TODAYWORD_CHANGE*)NETWORK2->GetMsgRoot( &handle
																									  ,(PerSocketContext*)socketContext
																									  ,NM_PLAYER
																									  ,NM_PLAYER_TODAYWORD_CHANGE_RES  );

	sendMsg->ErrorCode = ERROR_PLAYER_TODAYWORD_CHANGE_SUCCESS;
	sendMsg->mWordColor = mPlayerExrInfo.mTodayColor;
	wcsncpy( sendMsg->mWord, mPlayerExrInfo.mTodayWord, MAX_TODAYWORD_BUFFER_SIZE ); 

	NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TODAYWORD_CHANGE) );

	MSG_SYN_PLAYER_TODAYWORD_CHANGE synMsg;
	memset( &synMsg, 0, sizeof(synMsg) );
	synMsg.Category        = NM_PLAYER;
	synMsg.Protocol        = NM_PLAYER_TODAYWORD_CHANGE_SYN;
	synMsg.mWordColor      = mPlayerExrInfo.mTodayColor;
	synMsg.mCharacterIndex = mObject.index;
	if ( word != NULL ) wcsncpy( synMsg.mWord, word, MAX_TODAYWORD_BUFFER_SIZE );
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_PLAYER_TODAYWORD_CHANGE) );
	return true;
}

// Pg_Game_Start Method.
void cPlayer::Pg_Game_Start(char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain)
{
	User_Status;
	Bill_Method;
	Bill_Expire;
	Bill_Remain;

	/// pc방 효과목록을 가지고와서 효과를 걸어보고 불가능하면 주기적으로 재시도한다.
	cLongAry* pPcAry = SKILLSCRIPT->PcBangInfAry();
	if( pPcAry != NULL && pPcAry->GetSize() != 0 )
	{
		mPcBangNextCheckTime = 0;

		for( unsigned long i = 0 ; i < pPcAry->GetSize() ; ++i )
			mReadyPcInfSet.Insert( (*pPcAry)[i] );
	}
}

// Pg_User_Alert Method.
void cPlayer::Pg_User_Alert(long Packet_Result, char* Bill_Method, long Bill_Remain)
{
	Bill_Method;
	Bill_Remain;

	if( Packet_Result != 1 && Packet_Result != -3 )
	{
		/// 자신에게 걸려잇는 버프를 제거한다.
		cLongAry* pPcAry = SKILLSCRIPT->PcBangInfAry();
		if( pPcAry != NULL && pPcAry->GetSize() != 0 )
		{
			mPcBangNextCheckTime = ULONG_MAX;

			for( unsigned long i = 0 ; i < pPcAry->GetSize() ; ++i )
				SKILLMANAGER->DeleteInfluenceClassIdx( mObject, (*pPcAry)[i] );
		}
	}
}

// Pg_Billing_Auth Method.
void cPlayer::Pg_Billing_Auth(char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain)
{
	User_Status;
	Bill_Method;
	Bill_Expire;
	Bill_Remain;

}

// Pg_Client_Move Method.
void cPlayer::Pg_Client_Move(char* User_Status)
{
	User_Status;
}

// Pg_Close Method.
void cPlayer::Pg_Close( )
{
	/// 자신에게 걸려잇는 버프를 제거한다.
	cLongAry* pPcAry = SKILLSCRIPT->PcBangInfAry();
	if( pPcAry != NULL && pPcAry->GetSize() != 0 )
	{
		mPcBangNextCheckTime = ULONG_MAX;

		for( unsigned long i = 0 ; i < pPcAry->GetSize() ; ++i )
			SKILLMANAGER->DeleteInfluenceClassIdx( mObject, (*pPcAry)[i] );
	}
}

// Pg_Game_Start Method.
void cPlayer::BP_Start()
{
	/// pc방 효과목록을 가지고와서 효과를 걸어보고 불가능하면 주기적으로 재시도한다.
	cLongAry* pPcAry = SKILLSCRIPT->PcBangInfAry();
	if( pPcAry != NULL && pPcAry->GetSize() != 0 )
	{
		mPcBangNextCheckTime = 0;

		for( unsigned long i = 0 ; i < pPcAry->GetSize() ; ++i )
			mReadyPcInfSet.Insert( (*pPcAry)[i] );
	}
}

void cPlayer::BP_End()
{
	/// 자신에게 걸려잇는 버프를 제거한다.
	cLongAry* pPcAry = SKILLSCRIPT->PcBangInfAry();
	if( pPcAry != NULL && pPcAry->GetSize() != 0 )
	{
		mPcBangNextCheckTime = ULONG_MAX;

		for( unsigned long i = 0 ; i < pPcAry->GetSize() ; ++i )
			SKILLMANAGER->DeleteInfluenceClassIdx( mObject, (*pPcAry)[i] );
	}
}

void cPlayer::BP_ItemGive()
{	
	/// HERO에게 쏘기
	HANDLE handle  = NULL;
	MSGROOT* sendMsg = (MSGROOT*)NETWORK2->GetMsgRoot( &handle, mConnectionIdx ,NM_PLAYER ,NM_PLAYER_PCITEM_GIVE_SYN );
	if( sendMsg != NULL )
		NETWORK2->SendMsgRoot( handle, sizeof(MSGROOT) );
}

// IsBlock Method.
bool cPlayer::IsBlock( )
{
	if ( mBlockApply == true )
	{
		time_t ltime;
		time( &ltime );

		if ( difftime( mBlockValidThru, ltime ) <= 0 )
			mBlockApply = false;
	}
	return mBlockApply;
}

// SetBlock Method.
bool cPlayer::SetBlock(bool apply, unsigned long second)
{
	mBlockApply = apply;
	if ( mBlockApply == true )
	{
		struct tm validThru;
		time_t    ltime;
		time_t    result;

		time( &ltime );

		validThru = *localtime( &ltime );
		validThru.tm_sec += second;

		result = mktime( &validThru );
		if ( result != (time_t)-1 )
		{
			mBlockValidThru = result;
			return true;
		}
		return false;
	}
	return true;
}

// PostCharacterEvent Method.
bool cPlayer::PostCharacterEvent(char category, LPCTSTR format, ...)
{
	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;

	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) + 1;
	msgBuf       = malloc( bufferLength );

	vsprintf( (char*)msgBuf, format, args );

	va_end( args );

	bool retvalue = false;
	if ( msgBuf != NULL )
	{
		retvalue = NETWORK2->PostCharacterEvent( category
			,mConnectionIdx
			,mObject.index
			,mPlayerInfo.Level
			,mHeroInfo.Exp
			,mHeroInfo.SkillLevel
			,mHeroInfo.SkillExp
			,mHeroInfo.SkillPointTotal
			,(char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

void cPlayer::AddAllGMEvent( )
{
	if( NETWORK2 )
	{
		mReadyGMEventList.Clear();

		time_t now;
		time( &now );

		cGMEventList::cIterator i = mReadyGMEventList.Begin();

		cGMEventList* eventArr = NETWORK2->GetGMEvent();
		if( eventArr && eventArr->IsEmpty() == false )
		{
			mGMEventNextCheckTime = 0;
			cGMEventList::cIterator pos = eventArr->Begin();
			cGMEventList::cIterator end = eventArr->End();
			for( ; pos != end; ++pos )
			{
				sGMEventInfo& eventInfo = (*pos);
				/// 끝나는 시간이 지나지 않는것들만 추가한다.
				if( difftime( eventInfo.validThru, now ) > 0 )
					mReadyGMEventList.PushBack( eventInfo );
			}
		}
	}
}

void cPlayer::AddGMEvent( sGMEventInfo eventInfo )
{
	if( NETWORK2 )
	{
		time_t now;
		time( &now );

		mGMEventNextCheckTime = 0;
		/// 끝나는 시간이 지나지 않는것들만 추가한다.
		if(  difftime( eventInfo.validThru, now ) > 0 )
			mReadyGMEventList.PushBack( eventInfo );
	}
}

void cPlayer::DelGMEvent( unsigned long DBIndex, unsigned long influenceIdx )
{
	if( NETWORK2 )
	{
		///삭제할 효과가 추가 대기 리스트에 있다면 삭제한다.
		cGMEventList::cIterator pos = mReadyGMEventList.Begin();
		while( pos != mReadyGMEventList.End() )
		{
			sGMEventInfo& eventInfo = (*pos);
			if( eventInfo.index == DBIndex && eventInfo.influenceIdx == influenceIdx )
			{
				pos = mReadyGMEventList.Erase( pos );
				continue;
			}
			++pos;
		}

		mGMEventDelNextCheckTime = 0;
		
		///삭제할 효과가 이미 삭제 대기 리스트에 있다면 정보를 수정한다.
		pos = mReadyDelGMEventList.Begin();
		while( pos != mReadyDelGMEventList.End() )
		{
			sGMEventInfo& eventInfo = (*pos);
			if( eventInfo.index == DBIndex && eventInfo.influenceIdx == influenceIdx )
			{
				eventInfo.validThru = 0;
				break;
			}
			++pos;
		}
	}
}

bool cPlayer::IsHaveApplyBuffType( unsigned short applyValueType )
{
	cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( applyValueType );
	if(  findPos != mBuffmap.End() )
	{
		/// 이미 들어가 있는 경우
		cHashSet& influenceSet = (*findPos).mSecond;
		if( influenceSet.IsEmpty() == false )
		{	
			/// 해당 스킬이 유효한지 검사는 생략 중
			return true;
		}
	}

	return false;
}

bool cPlayer::IsHaveApplyDeBuffType( unsigned short applyValueType )
{
	cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( applyValueType );
	if(  findPos != mDeBuffmap.End() )
	{
		/// 이미 들어가 있는 경우
		cHashSet& influenceSet = (*findPos).mSecond;
		if( influenceSet.IsEmpty() == false )
		{	
			/// 해당 스킬이 유효한지 검사는 생략 중
			return true;
		}
	}

	return false;
}

unsigned long cPlayer::CalcUseMPExt( unsigned long originUseMP )
{	
	/// 소모 체크
	float useMPPlus = mStatus2Plus[ePLAYER_STATUS_EXT_USE_MP];
	float useMPPer  = mStatus2Per[ePLAYER_STATUS_EXT_USE_MP];

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

unsigned long cPlayer::CalcCastingExt( unsigned long originCastingTime )
{	
	/// 순간 스킬 체크
	if( originCastingTime == 0 )
		return 0;

	/// 소모 체크
	float useCastingPlus = mStatus2Plus[ePLAYER_STATUS_EXT_CASTING];
	float useCastingPer  = mStatus2Per[ePLAYER_STATUS_EXT_CASTING];

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
