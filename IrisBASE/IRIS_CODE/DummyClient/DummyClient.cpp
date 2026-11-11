#include "StdAfx.h"
#include "DummyClient.h"

#include "AsyncSelect.h"

#include "GameResourceManager.h"
#include "Application.h"
#include "WorldManager.h"

#include "DummyManager.h"
#include "ObjectManager.h"
#include "Object.h"
#include "ItemManager.h"
#include "SkillManager.h"

#include "protocol_category.h"
#include "protocol.h"

#include "Monster_Common.h"
#include "Npc_Common.h"
#include "Skill_Common.h"
#include "TeleportPortal_Common.h"
#include "UserPortal_Common.h"

cDummyClient::cDummyClient()
{
	mAsyncSelect = 0;

	mStage = eStage_Disconnet;

	mCurState = eState_Idle;
	mStateTime = 0;

	mDesiredDir = -NiPoint3::UNIT_Y;

	mPlayerWeaponState = eWEAPON_STATE_NONE;

	mFixedObjectSize = 0.0f;

	///
	mpObjectManager = 0;
	mpSkillManager = 0;

	mCurServer = 1;
	mCurChannel = 0;

	mLoadFlag.loadInfo = 0;

	mGotoX = 0.0f;
	mGotoY = 0.0f;

	mPathIndex = 0;
	mPathCount = 0;

	mCurActionInfo.type = eAction_None;

	mMoney = 0;
	mDeposit = 0;

	mCantMove = false;
	mCantSkillPhy = false;
	mCantSkillMag = false;
	mIsSleep = false;
	mIsStun = false;

	::memset( mItemArray, 0, sizeof(sItem)*LEN_INVENTORY );
}

cDummyClient::~cDummyClient()
{
	SAFE_DELETE(mpSkillManager);
	SAFE_DELETE(mpObjectManager);

	SAFE_DELETE(mAsyncSelect);
}

bool cDummyClient::Init( HWND wnd, unsigned int msgIdx, char* id, char* pwd, char* spwd, long serverIdx, long channelIdx )
{
	mId = id;
	mPwd = pwd;
	mSec = spwd;

	mCurServer = serverIdx;
	mCurChannel = channelIdx;

	mAsyncSelect = new cAsyncSelect( wnd, msgIdx, this );
	if( mAsyncSelect->Connect( DUMMYMAN->GetIP(), DUMMYMAN->GetPort() ) == false )
	{
		SAFE_DELETE( mAsyncSelect );
		return false;
	}

	mpSkillManager = new cSkillManager;

	mpObjectManager = new cObjectManager(this);

	return true;
}

void cDummyClient::ProcessManager( unsigned long deltaTime, unsigned long accumTime )
{
	if( mAsyncSelect )
		mAsyncSelect->Process();

	if( mpSkillManager )
		mpSkillManager->Process( deltaTime, accumTime );

	if( mpObjectManager )
		mpObjectManager->Process( deltaTime, accumTime );
}

void cDummyClient::Process( unsigned long deltaTime, unsigned long accumTime )
{
	deltaTime;
	accumTime;

	///
	switch( mStage )
	{
	case eStage_ReLogin:
		{
			mReLogintime += deltaTime;
			if( mReLogintime >= 5*MINUTE )
			{
				mReLogintime = 0;
				ChangeStage( eStage_Login );
			}
		}
		break;
	case eStage_Load:
		{
			if( mLoadFlag.flag.heroBaseInfoOn == false )
				break;
			if( mLoadFlag.flag.itemCoolTimeInfoOn == false )
				break;
			if( mLoadFlag.flag.shortcutInfoOn == false )
				break;
			if( mLoadFlag.flag.titleInfoOn == false )
				break;
			if( mLoadFlag.flag.questInfoOn == false )
				break;
			if( mLoadFlag.flag.optionInfoOn == false )
				break;
			if( mLoadFlag.flag.makeSkillInfoOn == false )
				break;
			if( mLoadFlag.flag.InfluenceInfoOn == false )
				break;
			if( mLoadFlag.flag.guardInfoOn == false )
				break;

			ChangeStage( eStage_GameIn );
		}
		break;
	case eStage_MapChange:
		{
			if( mLoadFlag.flag.heroBaseInfoOn == false )
				break;
			if( mLoadFlag.flag.itemCoolTimeInfoOn == false )
				break;
			if( mLoadFlag.flag.shortcutInfoOn == false )
				break;
			if( mLoadFlag.flag.titleInfoOn == false )
				break;
			if( mLoadFlag.flag.questInfoOn == false )
				break;
			if( mLoadFlag.flag.optionInfoOn == false )
				break;
			if( mLoadFlag.flag.makeSkillInfoOn == false )
				break;
			if( mLoadFlag.flag.InfluenceInfoOn == false )
				break;
			if( mLoadFlag.flag.guardInfoOn == false )
				break;

			ChangeStage( eStage_GameIn );
		}
		break;
	case eStage_GameProcess:
		{
			switch( mCurState )
			{
			case eState_Idle:
				{
					if( mStateTime <= accumTime )
						ChangeState();
				}
				break;
			case eState_SitDown:
				{
					if( mPlayerExrInfo.RestHP >= mPlayerExrInfo.MaxHP )
					{
						if( mStateTime <= accumTime )
							ChangeState();
					}
				}
				break;
			case eState_Die:
				{
					if( mStateTime <= accumTime )
					{
						MSG_REQ_PLAYER_RESURRECTION msg;
						msg.Category = NM_PLAYER;
						msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
						msg.mFlag = eRESURRECTIONTYPE_SAFE;
						mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						SetState( eState_Stop );
					}
				}
				break;
			case eState_Move:
				{
					MoveProcess( deltaTime );
					ProcessMoveAction();

					if( mCurState != eState_Stop )
					{
						if( mGotoX == mTrans.m_Translate.x && mGotoY == mTrans.m_Translate.y )
							ChangeState();
					}
				}
				break;
			case eState_Attack:
				{
					cMonster* monster = mpObjectManager->GetMonster( mTarget.index );
					if( monster == 0 )
					{
						SetState( eState_Idle, SECOND );
					}
					else
					{
						if( monster->GetState() == eOBJECT_STATE_DIE )
						{
							SetState( eState_Idle, SECOND );
						}
						else
						{
							if( mStateTime <= accumTime )
							{
								ChangeState();
							}
						}
					}
				}
				break;
			case eState_ItemPick:
				break;
			case eState_Max:
				ChangeState();
				break;
			}
		}
		break;
	}
}

void cDummyClient::ProcessMoveAction()
{
	if( mCurActionInfo.type == eAction_None )
		return;

	NiPoint3 heroPos = mTrans.m_Translate;
	heroPos.z = 0.0f;

	switch( mCurActionInfo.type )
	{
	case eAction_Attack:
		{
			cMonster* monster = mpObjectManager->GetMonster( mTarget.index );
			if( monster == 0 )
			{
				mCurActionInfo.type = eAction_None;
				ChangeState();
				return;
			}

			NiPoint3 targetPos = monster->GetPos();

			bool bAttack = false;
			float tempRange = monster->GetFixedObjectSize() + GetFixedObjectSize() + mCurActionInfo.range;

			cRangeCheck rangecheck( tempRange );
			/// 상대방이 움직여서 내범위로 들어온 경우를 찾는다.
			NiPoint3 curPos = mTrans.m_Translate;
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
				mCurState = eState_Attack;
				mStateTime = THEAPP->GetWorldAccumTime() + 1300;

				///
				MSG_REQ_SKILL_USED Msg;
				Msg.Category = NM_SKILL;
				Msg.Protocol = NM_SKILL_USED_REQ;
				Msg.mTarget = mTarget;

				Msg.mSkillClassIndex = mCurActionInfo.exData1;
				Msg.mSkillStep = 0;

				Msg.mManagedKey = 0;
				Msg.mClientPosX = mTrans.m_Translate.x;
				Msg.mClientPosY = mTrans.m_Translate.y;

				mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

				mCurActionInfo.type = eAction_None;
				return;
			}
			else
			{
				if( monster->GetHP() < monster->GetMaxHP()*0.95f )
				{
					mCurActionInfo.type = eAction_None;

					mTarget.type = eOBJECTTYPE_NONE;
					mTarget.index = 0;

					ChangeState();

					return;
				}
			}
		}
		break;
	case eAction_ItemPick:
		{
			float tempRange = GetFixedObjectSize() + mCurActionInfo.range;
			NiPoint3 curPos = mTrans.m_Translate;

			cRangeCheck rangecheck( tempRange );
			if( rangecheck.IsRange( curPos, mCurActionInfo.destPos ) )
			{
				mCurActionInfo.type = eAction_None;

				MSG_REQ_ITEM_GET_OPEN msg;
				msg.Category = NM_ITEM;
				msg.Protocol = NM_ITEM_GET_OPEN_REQ;
				msg.idx = mCurActionInfo.exData1;

				mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				SetState( eState_ItemPick );
			}

			return;
		}
		break;
	case eAction_MapChange:
		{
			float tempRange = GetFixedObjectSize() + mCurActionInfo.range;
			cRangeCheck rangecheck( tempRange );
			if( rangecheck.IsRange( heroPos , mCurActionInfo.destPos ) )
			{
				mCurActionInfo.type = eAction_None;

				sStageChangePos* pInfo = GAMERESOURCEMAN->GetPosScriptInfo( (unsigned long)mCurActionInfo.exData2 );
				if( pInfo )
				{
					if( pInfo->mFirstRegen == false )
					{
						if( pInfo->mMapNumber != mCurMapIdx )
							return;

						/// same map / position change
						MSG_REQ_TELEPORTPORTAL_USE Msg;
						Msg.Category = NM_TELEPORTPORTAL;
						Msg.Protocol = NM_TELEPORTPORTAL_USE_REQ;
						Msg.mMapChangePosIdx = (unsigned long)mCurActionInfo.exData2;

						mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

						SetState( eState_Stop );
					}
					else
					{
						if( mCurActionInfo.exData1 != 0 )
						{
							MSG_REQ_USERPORTAL_MAPCHANGE Msg;
							Msg.Category = NM_USERPORTAL;
							Msg.Protocol = NM_USERPORTAL_ENTER_REQ;
							Msg.mUserPortalObjectIdx = (unsigned long)mCurActionInfo.exData1;

							mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

							SetState( eState_Stop );
						}
						else
						{
							if( pInfo->mTargetInfo )
							{
								MSG_REQ_PLAYER_MAPCHANGE Msg;
								Msg.Category = NM_PLAYER;
								Msg.Protocol = NM_PLAYER_MAPCHANGE_REQ;
								Msg.mMapChangePosIndex = (unsigned long)mCurActionInfo.exData2;

								mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

								SetState( eState_Stop );
							}
						}
					}
				}
			}
		}
		break;
	}
}

void cDummyClient::SetState( eSTATE state, unsigned long time )
{
	if( mCurState == eState_Die )
	{
		if( state != eState_Idle && state != eState_Stop )
			return;
	}

	if( state == eState_Die )
	{
		if( time == 0 )
			mStateTime = THEAPP->GetWorldAccumTime() + 10*SECOND;
		else
			mStateTime = THEAPP->GetWorldAccumTime() + time;

		mCantMove = false;
		mCantSkillPhy = false;
		mCantSkillMag = false;
		mIsStun = false;
		mIsSleep = false;
	}
	else if( state == eState_Idle )
	{
		if( time == 0 )
			mStateTime = THEAPP->GetWorldAccumTime() + 5*SECOND;
		else
			mStateTime = THEAPP->GetWorldAccumTime() + time;
	}
	else if( state == eState_ItemPick )
	{
		if( time == 0 )
			mStateTime = THEAPP->GetWorldAccumTime() + 2*SECOND;
		else
			mStateTime = THEAPP->GetWorldAccumTime() + time;
	}

	mCurState = state;
	mCurActionInfo.type = eAction_None;
}

void cDummyClient::ChangeState()
{
	if( mCurState == eState_Die )
		return;

	::srand( ::rand()%500000 );

	eSTATE oldState = mCurState;
	mCurActionInfo.type = eAction_None;

	mCurState = eState_Max;

	cMonster* nearMon = 0;

	/// 드랍 아이템 검사
	cDropItem* nearItem = mpObjectManager->GetNearDropItem( mPlayerInfo.CharacterIdx, mTrans.m_Translate );
	if( nearItem )
	{
		NiPoint3 itemPos = nearItem->GetPos();
		if( ITEM_VALID_DISTANCE < (mTrans.m_Translate - nearItem->GetPos()).Length() )
		{
			mCurState = eState_Move;
			mCurActionInfo.type = eAction_ItemPick;
			mCurActionInfo.range = ITEM_VALID_DISTANCE;
			mCurActionInfo.exData1 = nearItem->GetObjectID();
		}
		else
		{
			if( oldState == eState_SitDown )
			{
				MSG_REQ_PLAYER_STATE_SITDWON Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
				Msg.mSitDown = false;
				mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}

			mCurState = eState_ItemPick;
			mStateTime = THEAPP->GetWorldAccumTime() + 2000;

			/// msg
			MSG_REQ_ITEM_GET_OPEN msg;
			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_GET_OPEN_REQ;
			msg.idx = nearItem->GetObjectID();

			mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
	}
	else
	{
		/// 몬스터 공격
		nearMon = mpObjectManager->GetMonster( mTarget.index );
		if( nearMon == 0 || nearMon->GetState() == eOBJECT_STATE_DIE )
		{
			if( mPlayerExrInfo.RestHP < mPlayerExrInfo.MaxHP/3 )
				mCurState = eState_SitDown;
			else
			{
				nearMon = mpObjectManager->GetNearMonster( mPlayerInfo.Level, mTrans.m_Translate );
			}
		}

		if( nearMon )
		{
			/// 공격
			if( IsCantSkill( NORMAL_ATTACK_SKILL ) == false )
			{
				/// 타겟 설정
				mTarget.type = nearMon->GetType();
				mTarget.index = nearMon->GetObjectID();

				/// 공격 인덱스 설정
				unsigned int state = (unsigned int)mPlayerWeaponState;
				if( state > eWEAPON_STATE_MAX )
					state -= eWEAPON_STATE_SHEILD;

				unsigned int flag = ::rand()%3;
				unsigned long skillIdx = flag*10 + state + 1;

				sPlayerSkillBaseInfo* pInfo = GAMERESOURCEMAN->GetPlayerSkillInfo( skillIdx );
				if( pInfo )
				{
					float tempRange = GetFixedObjectSize() + nearMon->GetFixedObjectSize() + pInfo->mpSetpInfoArray[0].mTargetDist;
					cRangeCheck rangecheck( tempRange );
					if( rangecheck.IsRange( mTrans.m_Translate, nearMon->GetPos() ) )
					{
						if( oldState == eState_SitDown )
						{
							MSG_REQ_PLAYER_STATE_SITDWON Msg;
							Msg.Category = NM_PLAYER;
							Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
							Msg.mSitDown = false;
							mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
						}

						mCurState = eState_Attack;
						mStateTime = THEAPP->GetWorldAccumTime() + 1300;

						///
						MSG_REQ_SKILL_USED Msg;
						Msg.Category = NM_SKILL;
						Msg.Protocol = NM_SKILL_USED_REQ;
						Msg.mTarget = mTarget;

						Msg.mSkillClassIndex = skillIdx;
						Msg.mSkillStep = 0;

						Msg.mManagedKey = 0;
						Msg.mClientPosX = mTrans.m_Translate.x;
						Msg.mClientPosY = mTrans.m_Translate.y;

						mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
					}
					else
					{
						mCurState = eState_Move;
						mCurActionInfo.type = eAction_Attack;
						mCurActionInfo.target = mTarget;
						mCurActionInfo.range = pInfo->mpSetpInfoArray[0].mTargetDist;
						mCurActionInfo.exData1 = skillIdx;
					}
				}
			}
		}
	}


	if( mCurState == eState_Max )
	{
		if( IsCantMove() == true )
		{
			mCurState = eState_Idle;
		}
		else
		{
			unsigned int val = ::rand()%100000;

			if( val < 1000 )
				mCurState = eState_Idle;
			else
			{
				mCurState = eState_Move;
				unsigned int act = ::rand()%100000;
				if( mCurMapIdx == 1 )
				{
					if( act < 100000 )
						mCurActionInfo.type = eAction_MapChange;
				}
				else
				{
					if( act > 800000 )
						mCurActionInfo.type = eAction_MapChange;
				}
			}
		}
	}

	switch( mCurState )
	{
	case eState_Idle:
		{
			if( oldState == eState_SitDown )
			{
				MSG_REQ_PLAYER_STATE_SITDWON Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
				Msg.mSitDown = false;
				mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}

			mStateTime = THEAPP->GetWorldAccumTime() + (::rand()%15+5)*SECOND;
		}
		break;
	case eState_SitDown:
		{
			/// Network Msg
			MSG_REQ_PLAYER_STATE_SITDWON Msg;
			Msg.Category = NM_PLAYER;
			Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
			Msg.mSitDown = true;
			mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			mStateTime = THEAPP->GetWorldAccumTime() + (::rand()%55+5)*SECOND;
		}
		break;
	case eState_Move:
		{
			if( oldState == eState_SitDown )
			{
				MSG_REQ_PLAYER_STATE_SITDWON Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
				Msg.mSitDown = false;
				mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
			}

			cPathFinder* pathFinder = WORLDMAN->GetPathFinder( mCurMapIdx, mCurMapMode );
			if( pathFinder == 0 )
			{
				mCurState = eState_Idle;
				mStateTime = THEAPP->GetWorldAccumTime() + 10*SECOND;
				return;
			}

			NiPoint2 destPos;
			if( mCurActionInfo.type == eAction_None )
			{
				while(1)
				{
					destPos.x = mTrans.m_Translate.x + ( rand() % 10000 ) - ( rand() % 10000 );
					destPos.y = mTrans.m_Translate.y + ( rand() % 10000 ) - ( rand() % 10000 );

					if( pathFinder->EnableGoto( destPos.x, destPos.y ) == true )
						break;
				}
			}
			else if( mCurActionInfo.type == eAction_MapChange )
			{
				/// 이동맵 설정
				cPortal* portal = mpObjectManager->GetMapChangePortal( mCurMapIdx, mTrans.m_Translate );
				if( portal )
				{
					destPos.x = portal->GetPos().x;
					destPos.y = portal->GetPos().y;

					if( pathFinder->EnableGoto( destPos.x, destPos.y ) == true )
					{
						mCurActionInfo.destPos = NiPoint3( destPos.x, destPos.y, 0.0f );
						mCurActionInfo.range = portal->GetRange();
						mCurActionInfo.exData1 = 0;
						mCurActionInfo.exData2 = portal->GetPosScriptIdx();
					}
					else
					{
						mCurActionInfo.type = eAction_None;
					}
				}
			}
			else if( mCurActionInfo.type == eAction_Attack )
			{
				destPos.x = nearMon->GetPos().x;
				destPos.y = nearMon->GetPos().y;

				mCurActionInfo.destPos = nearMon->GetPos();
			}
			else if( mCurActionInfo.type == eAction_ItemPick )
			{
				destPos.x = nearItem->GetPos().x;
				destPos.y = nearItem->GetPos().y;

				mCurActionInfo.destPos = nearItem->GetPos();
			}

			/// 새로운 이동점 지정
			ResetMovePath( NiPoint2(mTrans.m_Translate.x, mTrans.m_Translate.y), destPos );
		}
		break;
	}
}


void cDummyClient::ChangeStage( eSTAGE stage )
{
	switch( stage )
	{
	case eStage_Login:
		{
			MSG_IDPASS sendMsg;
			::memset( &sendMsg, 0, sizeof(sendMsg) );

			sendMsg.Category = NM_USER;
			sendMsg.Protocol = NM_USER_LOGIN_REQ;
			strcpy( sendMsg.uid, mId.Cstr() );
			strcpy( sendMsg.pwd, mPwd.Cstr() );
			//strcpy( sendMsg.sec, mSec.Cstr() );

			mAsyncSelect->SendNetworkMsg( (char*)&sendMsg, sizeof(sendMsg) );
		}
		break;
	case eStage_ReLogin:
		{
			mReLogintime = 0;
		}
		break;
	case eStage_Load:
		{
			mLoadFlag.loadInfo = 0;

			MSGROOT msg;
			msg.Category = NM_USER;
			msg.Protocol = NM_USER_LOADDATA_REQ;
			mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );	

			mpObjectManager->Clear();
		}
		break;
	case eStage_MapChange:
		{
			mLoadFlag.loadInfo = 0;

			mCurActionInfo.type = eAction_None;

			mpObjectManager->Clear();
		}
		break;
	case eStage_GameIn:
		{
			/// 게임 진입 패킷을 준다..
			MSGROOT msg;
			msg.Category = NM_USER;
			msg.Protocol = NM_USER_GAMEIN_REQ;
			mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			SetState( eState_Idle );

			mCantMove = false;
			mCantSkillPhy = false;
			mCantSkillMag = false;
			mIsSleep = false;
			mIsStun = false;

			/// load
			tPointerArray<void*>* pMapArray = GAMERESOURCEMAN->GetMapChangeStarterArr( mCurMapIdx );
			if( pMapArray->IsEmpty() == false )
			{
				for( unsigned int i=0, end = pMapArray->GetSize(); i<end; ++i )
				{
					sStageChangePos* p = (sStageChangePos*)(*pMapArray)[i];

					if( p->mFirstRegen == false )
						continue;

					NiPoint3 pos = NiPoint3( p->mPosX, p->mPosY,0.0f );
					if( WORLDMAN->CalcHeight( mMapFolderIdx, &pos.z, pos.x, pos.y ) == false )
					{
						assert(0);
						continue;
					}

					NiMatrix3 rot;
					rot.FromEulerAnglesXYZ( 0, 0, p->mRotAngle );

					mpObjectManager->AddPortal( 0, p->mPosIdx, pos, rot );
				}
			}

			DUMMYMAN->checkCount++;
		}
		break;
	case eStage_Disconnet:
		{
			DUMMYMAN->checkCount--;
		}
		break;
	}

	mStage = stage;
}

void cDummyClient::NetWorkProc( WPARAM wparam, LPARAM lparam )
{
	if( mAsyncSelect )
		mAsyncSelect->NetWorkProc( wparam, lparam );
}

void cDummyClient::NetWorkMsgParser( char* pMsg )
{
	MSGROOT* pMsgRoot = (MSGROOT*)pMsg;
	switch( pMsgRoot->Category )
	{
	case NM_USER:
		MsgParser_User( pMsgRoot );
		break;
	case NM_PLAYER:
		MsgParser_Player( pMsgRoot );
		break;
	case NM_MONSTER:
		MsgParser_Monster( pMsgRoot );
		break;
	case NM_NPC:
		MsgParser_Npc( pMsgRoot );
		break;
	case NM_SKILL:
		MsgParser_Skill( pMsgRoot );
		break;
	case NM_ITEM:
		MsgParser_Item( pMsgRoot );
		break;
	case NM_QUEST:
		MsgParser_Quest( pMsgRoot );
		break;
	case NM_MAKESKILL:
		MsgParser_MakeSkill( pMsgRoot );
		break;
	}
}


void cDummyClient::MakeCharacter()
{
	MSG_REQ_CHARACTER_CREATE Msg;			
	Msg.Category = NM_USER;
	Msg.Protocol = NM_USER_CHARACTER_CREATE_REQ;

	cStringT name = _T("DUMMY");
	name += RandomString( MAX_NAME_SIZE-7 );

	///// 캐릭명 임의 생성
	::_tcsncpy_s( Msg.Name, MAX_NAME_BUFFER_SIZE, name.Cstr(), MAX_NAME_SIZE );
	Msg.Name[MAX_NAME_SIZE] = 0;

	Msg.Hair = (unsigned char)(::rand()%GAMERESOURCEMAN->GetHairCount());
	Msg.Face = (unsigned char)(::rand()%GAMERESOURCEMAN->GetFaceCount()) + 1;
	Msg.Gender = (char)(::rand()%eGENDER_MAX);
	Msg.HairColor = (unsigned short)(::rand()%GAMERESOURCEMAN->GetHairColorCount());

	Msg.Race = (char)(::rand()%eRACE_MAX);

	unsigned int job = ::rand()%2;
	switch( Msg.Race )
	{
	case eRACE_HUMAN: (job == 0)?Msg.Job = ePLAYER_FIGHTER:Msg.Job = ePLAYER_MAGE;break;
	case eRACE_BEAST: (job == 0)?Msg.Job = ePLAYER_WANDERER:Msg.Job = ePLAYER_WARLOCK;break;
	case eRACE_ELF: (job == 0)?Msg.Job = ePLAYER_ARMSMAN:Msg.Job = ePLAYER_RANGER;break;
	}

	Msg.StandIdx = 0;

	mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
}

cStringT cDummyClient::RandomString( int iLength ) 
{ 
	cStringT strReturn; 
	cStringT strLocal; 

	for(int i=0; i<iLength ; ++i ) 
	{ 
		int iNumber; 

		// 매번 실행할 때마다 다른 값을 얻기 위하여 
		// TickCount값을 난수 발생기의 근원값으로 설정한다. 
		::srand( (unsigned int)( (i+1)*iLength*GetTickCount() ) );

		iNumber=rand()%122; 
		if( 48 > iNumber ) iNumber += 48; 
		if( (57<iNumber) && (65>iNumber) ) iNumber += 7; 
		if( (90<iNumber) && (97>iNumber) ) iNumber += 6; 
		strReturn += (TCHAR)iNumber; 
	}
	return strReturn; 
}


//////////////////////////////////////////////////////////////////////////

bool cDummyClient::ResetMovePath( NiPoint2 start, NiPoint2 goal )
{
	/// 이동 불가
	if( IsCantMove() == true )
		return false;

	/// 길을 찾아보세~
	cPathFinder* pathFinder = WORLDMAN->GetPathFinder( mCurMapIdx, mCurMapMode );
	if( pathFinder == 0 )
		return false;

	if( pathFinder->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, start, goal ) == false )
		return false;

	mPathIndex = 0;
	mTargetPos.x = goal.x;
	mTargetPos.y = goal.y;
	mTargetPos.z = 0.0f;

	/// 이동
	Move( mPathArray[0].x, mPathArray[0].y );

	return true;
/*
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
*/
}

void cDummyClient::MoveProcess( unsigned long deltaTime )
{
	if( mPathCount == 0 )
		return;

	NiPoint3 pos = mTrans.m_Translate;
	long speed = mPlayerExrInfo.mMoveSpeed;

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
				SetState( eState_Idle );
			}
			else
			{
				ResetMovePath( NiPoint2(pos.x, pos.y), NiPoint2(mTargetPos.x, mTargetPos.y) );
			}
		}
	}

//	if( EnableGateMovePos( NiPoint2(GetXPos(), GetYPos()), pos.x, pos.y ) == false )
//	{
//		pos.x = GetXPos();
//		pos.y = GetYPos();
//	}

	if( WORLDMAN->CalcHeight( mMapFolderIdx, &pos.z, pos.x, pos.y ) )
	{
		mTrans.m_Translate = pos;
	}
	else
	{
		assert(0);
		return;
	}

	if( nextPath == true )
	{
		NiPoint2& nextPos = mPathArray[++mPathIndex];
		Move( nextPos.x, nextPos.y );
	}
}

bool cDummyClient::Move( float x, float y )
{
	if( x < 0 || y < 0 )
	{
		assert(0);
		return false;
	}

//	/// 게이트 검사
//	float goX = x;
//	float goY = y;
//	if( EnableGateMovePos( NiPoint2(GetPos().x, GetPos().y), goX, goY ) == false )
//	{
//		SetState( eOBJECT_STATE_IDLE );
//		return false;
//	}

	/// 현재 캐릭터의 방향
	NiPoint3 charDir = mDesiredDir;

	/// 바라봐야할 방향
	NiPoint3 viewPos = NiPoint3(x, y, 0.0f);
	NiPoint3 curPos = mTrans.m_Translate;
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
	mTrans.m_Rotate =  mTrans.m_Rotate* matRot;
	mDesiredDir = viewDir;

//	SetState( eOBJECT_STATE_MOVE );

	mGotoX = x;
	mGotoY = y;

	/// 패킷날리기
	MSG_REQ_MOVE Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_MOVE_REQ;
	Msg.startX = mTrans.m_Translate.x;
	Msg.startY = mTrans.m_Translate.y;
	Msg.destX = mGotoX;
	Msg.destY = mGotoY;
	Msg.mClientTime = 0;
	mAsyncSelect->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	return true;
}

void cDummyClient::MsgParser_User( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
	case NM_USER_CHARACTER_LIST_RES:
		{
			MSG_CHARACTER_LIST* pmsg = (MSG_CHARACTER_LIST*)pMsgRoot;
			if( pmsg->ErrorCode == 0 )
			{
				/// 캐릭터 Count가 0이면
				if( pmsg->CharacterNum == 0 )
				{
					/// 생성
					MakeCharacter();
				}
				else
				{
					::memcpy( &mPlayerInfo, &pmsg->BaseInfo[0].PlayerInfo, sizeof(sPlayerInfo) );

					/// 접속 버튼
					MSGROOT msg;
					memset( &msg, 0, sizeof(msg) );
					msg.Category    = NM_USER;
					msg.Protocol    = NM_USER_CHANNEL_LIST_REQ;
					mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
			else
			{
				/// 에러 처리..
				return;
			}
		}
		break;
	case NM_USER_CHANNEL_LIST_RES:
		{
			MSG_RES_CHANNELLIST* pmsg = (MSG_RES_CHANNELLIST*)pMsgRoot;
			if( pmsg->RowCount != 0 )
			{
				//mCurChannel = pmsg->Channels[0].NumOfChannel;

				MSG_REQ_COMEIN_GAME msg;
				memset( &msg, 0, sizeof(msg) );
				msg.Category     = NM_USER;
				msg.Protocol     = NM_USER_COME_IN_GAME_REQ;
				/// SET MSG_REQ_COMEIN_GAME.
				msg.NumOfChannel = mCurChannel;
				msg.CharacterIdx = mPlayerInfo.CharacterIdx;
				msg.Tutorial = false;

				mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else
			{
				/// 재접속 체크
				int a = 0;
				a = 1;

			}
		}
		break;
	case NM_USER_CHARACTER_SELECT_RES:
		{
			MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsgRoot;

			if( pmsg->ErrorCode == 0 )
			{
				SetMapIndex( pmsg->MapNum, pmsg->MapMode );

				///
				float height = 0.0f;
				if( WORLDMAN->CalcHeight( mMapFolderIdx, &height, pmsg->XPos, pmsg->YPos ) == false )
					assert(NULL);

				mTrans.m_Translate = NiPoint3( pmsg->XPos, pmsg->YPos, height );

				///
				NiMatrix3 rot;
				rot.FromEulerAnglesXYZ( 0.0f, 0.0f, pmsg->RotAngle );
				mTrans.m_Rotate = rot;

				///
				::memcpy( &mPlayerInfo, &pmsg->PlayerInfo, sizeof(sPlayerInfo) );
				::memcpy( &mPlayerExrInfo, &pmsg->Exrinfo, sizeof(sPlayerExrInfo) );
				::memcpy( &mPlayerWearInfo, &pmsg->WearInfo, sizeof(sPlayerWearInfo) );
				::memcpy( &mPlayerWeaponInfo, &pmsg->WeaponInfo, sizeof(sPlayerWeaponInfo) );

				UpdateWeaponState();

				ChangeStage( eStage_Load );
			}
			else
			{
				/// 재요청
				int a = 0;
				a = 1;
			}
		}
		break;
	case NM_USER_GAMEIN_RES:
		{
			MSG_RES_GAMEIN* msg = (MSG_RES_GAMEIN*)pMsgRoot;
			mCurChannel = msg->channelNum;

			SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, true );

			//					SetState( eOBJECT_STATE_IDLE );
			mPlayerExrInfo.mStateIdle = eIDLE_NORMAL;
			mPlayerExrInfo.mStateStop = eSTOP_NONE;

			mDesiredDir = mTrans.m_Rotate*mDesiredDir;
			mDesiredDir.Unitize();

			mGotoX = mTrans.m_Translate.x;
			mGotoY = mTrans.m_Translate.y;

			//					/// 스탯 계산..
			//					STATUSCALC->CalcPlayerInit();


			ChangeStage( eStage_GameProcess );
		}
		break;
	case NM_USER_CHARACTER_CREATE_RES:
		{
			MSG_ERROR* msg = (MSG_ERROR*)pMsgRoot;

			if( msg->ErrorCode == 0 )
			{
				/// 성공
				MSGROOT sendMsg;
				::memset( &sendMsg, 0, sizeof(sendMsg) );

				sendMsg.Category = NM_USER;
				sendMsg.Protocol = NM_USER_CHARACTER_LIST_REQ;

				mAsyncSelect->SendNetworkMsg( (char*)&sendMsg, sizeof(sendMsg) );
			}
			else
			{
				/// 실패
				MakeCharacter();
			}
		}
		break;
	}
}

void cDummyClient::MsgParser_Player( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
	case NM_PLAYER_HEROINFO_RES:
		{
			MSG_HEROINFO* pHInfo = (MSG_HEROINFO*)pMsgRoot;

			memcpy( &mHeroInfo, &pHInfo->mHeroInfo, sizeof(sHeroInfo) );
			memcpy( &mPlayerExrInfo, &pHInfo->mExrInfo, sizeof(sPlayerExrInfo) );

			mCantMove = pHInfo->mCanTMove;
			mCantSkillPhy = pHInfo->mCanTSkillPhy;
			mCantSkillMag = pHInfo->mCanTSkillMag;
			mIsSleep = pHInfo->mSleep;
			mIsStun = pHInfo->mStun;
			mPlayerExrInfo.mChgMonsterIdx = pHInfo->mChgMonsterIdx;


			mLoadFlag.flag.heroBaseInfoOn = true;
		}
		break;
	case NM_PLAYER_OPTION_VALUE_RES:
		{
			mLoadFlag.flag.optionInfoOn = true;
		}
		break;
	case NM_PLAYER_SHORTCUT_LIST_RES:
		{
			//MSG_RES_PLAYER_SHORTCUT_LIST* msgData = (MSG_RES_PLAYER_SHORTCUT_LIST*)pMsg;

			//for( int i = 0; i < MAX_SHORTCUT_SIZE; ++i )
			//{
			//	sShortcut& shorCut = msgData->mShortcut[i];
			//	QUICKMAN->NetSetData( i, shorCut );
			//}

			mLoadFlag.flag.shortcutInfoOn = true;
		}
		break;
	case NM_PLAYER_MONEY_RES:
		{
			MSG_RES_PLAYER_MONEY* msg = (MSG_RES_PLAYER_MONEY*)pMsgRoot;
			mMoney = msg->mMoney;
		}
		break;
	case NM_PLAYER_DEPOSIT_RES:
		{
			MSG_RES_PLAYER_DEPOSIT* msg = (MSG_RES_PLAYER_DEPOSIT*)pMsgRoot;
			mDeposit = msg->mDeposit;
		}
		break;
	case NM_PLAYER_TITLE_LIST_RES:
		{
			//MSG_RES_PLAYER_TITLELIST* pmsg = (MSG_RES_PLAYER_TITLELIST*)pMsg;

			//TITLEMAN->ClearList();

			//for( int i = 0; i < pmsg->rowCount; ++i )
			//	TITLEMAN->AddHaveTitle( pmsg->mTitleIndex[i] );

			mLoadFlag.flag.titleInfoOn = true;
		}
		break;

	case NM_PLAYER_MOVE_RES:
		{
			/// 070625 PKH 원래 좌표로 옮김
			MSG_RES_ERR_MOVE* pmsg = (MSG_RES_ERR_MOVE*)pMsgRoot;
			NiPoint3 pos = NiPoint3( pmsg->destX, pmsg->destY, 0.0f );

			WORLDMAN->CalcHeight( mMapFolderIdx, &pos.z, pos.x, pos.y );
			mTrans.m_Translate = pos;
		}
		break;
	case NM_PLAYER_MAPCHANGE_RES:
		{
			MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsgRoot;
			if( pmsg->ErrorCode == 0 )
			{
				SetMapIndex( pmsg->MapNum, pmsg->MapMode );

				///
				float height = 0.0f;
				if( WORLDMAN->CalcHeight( mMapFolderIdx, &height, pmsg->XPos, pmsg->YPos ) == false )
					assert(NULL);

				mTrans.m_Translate = NiPoint3( pmsg->XPos, pmsg->YPos, height );

				///
				NiMatrix3 rot;
				rot.FromEulerAnglesXYZ( 0.0f, 0.0f, pmsg->RotAngle );
				mTrans.m_Rotate = rot;

				///
				::memcpy( &mPlayerInfo, &pmsg->PlayerInfo, sizeof(sPlayerInfo) );
				::memcpy( &mPlayerExrInfo, &pmsg->Exrinfo, sizeof(sPlayerExrInfo) );
				::memcpy( &mPlayerWearInfo, &pmsg->WearInfo, sizeof(sPlayerWearInfo) );
				::memcpy( &mPlayerWeaponInfo, &pmsg->WeaponInfo, sizeof(sPlayerWeaponInfo) );

				UpdateWeaponState();

				ChangeStage( eStage_MapChange );
			}
			else
			{
				mCurActionInfo.type = eAction_None;
				SetState( eState_Idle );
			}
		}
		break;
	case NM_PLAYER_HP_SYN:
		{
			MSG_SYN_PLAYER_HP* msg = (MSG_SYN_PLAYER_HP*)pMsgRoot;

			if( mPlayerInfo.CharacterIdx == msg->mCharacterIdx )
			{
				if( msg->mDie == true )
				{
					mPlayerExrInfo.RestHP = 0;
					mPlayerExrInfo.MaxHP = msg->mMaxHp;
					SetState( eState_Die );
				}
				else
				{
					mPlayerExrInfo.RestHP = msg->mHp;
					mPlayerExrInfo.MaxHP = msg->mMaxHp;
				}
			}
			//else
			//{
			//	cPlayer* pPlayer = mpObjectManager->GetPlayer( msg->mCharacterIdx );
			//	if( pPlayer )
			//	{
			//		if( msg->mDie == true )
			//			pPlayer->HPChangeDie( msg->mMaxHp, 0, eDAMAGE_NORMAL );
			//		else
			//			pPlayer->HPChange( msg->mHp, msg->mMaxHp );
			//	}
			//}
		}
		break;
	case NM_PLAYER_MAXHP_SYN:
		{
			MSG_SYN_PLAYER_MAXHP* msg = (MSG_SYN_PLAYER_MAXHP*)pMsgRoot;

			if( mPlayerInfo.CharacterIdx == msg->mCharacterIdx )
			{
				mPlayerExrInfo.MaxHP = msg->mMaxHp;
			}
			//else
			//{
			//	cPlayer* pPlayer = mpObjectManager->GetPlayer( msg->mCharacterIdx );
			//	if( pPlayer )
			//		pPlayer->SetMaxHP( msg->mMaxHp );
			//}
		}
		break;
	case NM_PLAYER_MP_SYN:
		{
			MSG_SYN_PLAYER_MP* msg = (MSG_SYN_PLAYER_MP*)pMsgRoot;
			if( mPlayerInfo.CharacterIdx == msg->mCharacterIdx )
			{
				mPlayerExrInfo.RestMP = msg->mMp;
				mPlayerExrInfo.MaxMP = msg->mMaxMp;
			}
			//else
			//{
			//	cPlayer* pPlayer = mpObjectManager->GetPlayer( msg->mCharacterIdx );
			//	if( pPlayer )
			//		pPlayer->MPChange( msg->mMp, msg->mMaxMp );
			//}
		}
		break;
	case NM_PLAYER_MAXMP_SYN:
		{
			MSG_SYN_PLAYER_MAXMP* msg = (MSG_SYN_PLAYER_MAXMP*)pMsgRoot;
			if( mPlayerInfo.CharacterIdx == msg->mCharacterIdx )
			{
				mPlayerExrInfo.MaxMP = msg->mMaxMp;
			}
			//else
			//{
			//	cPlayer* pPlayer = mpObjectManager->GetPlayer( msg->mCharacterIdx );
			//	if( pPlayer )
			//		pPlayer->SetMaxMP( msg->mMaxMp );
			//}
		}
		break;
	case NM_PLAYER_RESURRECTION_RES:
		{
			MSG_RES_PLAYER_RESURRECTION* msg = (MSG_RES_PLAYER_RESURRECTION*)pMsgRoot;

			/// 부활실패
			if( msg->ErrorCode != ERROR_PLAYER_RESURRECTION_SUCCESS )
			{
				/// 재시도
				SetState( eState_Die );
			}
			else
			{
				mPlayerExrInfo.RestHP = msg->mHP;
				mPlayerExrInfo.MaxHP = msg->mMaxHP;

				mPlayerExrInfo.RestMP = msg->mMP;
				mPlayerExrInfo.MaxMP = msg->mMaxMP;

				NiPoint3 pos = NiPoint3( msg->mPosX, msg->mPosY, 0.0f );
				if( WORLDMAN->CalcHeight( mMapFolderIdx, &pos.z, pos.x, pos.y ) )
				{
					mTrans.m_Translate = pos;
				}

				SetState( eState_Idle );
			}
		}
		break;
	case NM_PLAYER_RESURRECTION_SYN:
		{
			MSG_SYN_PLAYER_RESURRECTION* msg = (MSG_SYN_PLAYER_RESURRECTION*)pMsgRoot;
			msg;

			//cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
			//if( pPlayer == NULL )
			//{
			//	assert(NULL);
			//	return;
			//}

			//if( pPlayer->GetState() != eOBJECT_STATE_DIE )
			//{
			//	assert(NULL);
			//	return;
			//}

			//pPlayer->SetState( eOBJECT_STATE_IDLE );
			//pPlayer->HPChange( msg->mHP, msg->mMaxHP );
			//pPlayer->MPChange( msg->mMP, msg->mMaxMP );

			//NiPoint3 pos( msg->mPosX, msg->mPosY, 0.0f );
			//if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
			//{
			//	assert(0);
			//	return;
			//}

			//pPlayer->SetPos( pos );
		}
		break;
	case NM_PLAYER_LEVEL_RES:
		{
			MSG_RES_LEVEL* resMsg = (MSG_RES_LEVEL*)pMsgRoot;

			mHeroInfo.Exp = resMsg->mExp;
			mPlayerInfo.Level = resMsg->mLevel;

			mPlayerExrInfo.RestHP = resMsg->mHP;
			mPlayerExrInfo.MaxHP = resMsg->mMaxHP;

			mPlayerExrInfo.RestMP = resMsg->mMP;
			mPlayerExrInfo.MaxMP = resMsg->mMaxMP;
		}
		break;
	case NM_PLAYER_SKILLLEVEL_RES:
		{
			MSG_RES_SKILLLEVEL* resMsg = (MSG_RES_SKILLLEVEL*)pMsgRoot;

			mHeroInfo.SkillExp = resMsg->mSkillExp;
			mHeroInfo.SkillLevel = resMsg->mSkillLevel;
		}
		break;
	case NM_PLAYER_SCALE_SYN:
		{
			MSG_SYN_PLAYER_SIZE* msg = (MSG_SYN_PLAYER_SIZE*)pMsgRoot;

			if( mPlayerInfo.CharacterIdx == msg->mCharacterIdx )
			{
				SetFixedObjectSizeScale( msg->mScale, false );
			}
			else
			{
				cPlayer* pPlayer = mpObjectManager->GetPlayer( msg->mCharacterIdx );
				if( pPlayer )
					pPlayer->SetFixedObjectSizeScale( msg->mScale, false );
			}
		}
		break;
	}
}

void cDummyClient::MsgParser_Monster( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
	case NM_MONSTER_MODELNUMBER_RES:
		{
			mLoadFlag.flag.monsterModelOn = true;
		}
		break;
	case NM_MONSTER_SIGHT_IN_RES:
		{
			MSG_MONSTER_INFO* msg = (MSG_MONSTER_INFO*)pMsgRoot;

			sMonsterData* monster = &msg->mMonsterInfo;
//			sMonsterInfluence* pInfluence = msg->mInfluence;

			if( monster )
			{
				mpObjectManager->AddMonster( monster );

				//cMonster* pMonster = OBJECTMAN->GetMonster( monster->mMonsterIdx );
				//if( pMonster )
				//{
				//	for( unsigned long i = 0 ; i < pMessage->mCount ; ++i )
				//	{
				//		cInfluenceObject* p = SKILLMAN->CreateInfluenceObject( pInfluence[i].mUniqueIdx, 
				//			pInfluence[i].mInfluenceClassIdx, 0, pMonster, true );
				//		if( p )
				//		{
				//			p->StartProcess( -1, true );
				//		}
				//		else
				//		{
				//			assert(0);
				//			return;
				//		}
				//	}
				//}
			}
		}
		break;
	case NM_MONSTER_REGEN_SYN:
		{
			MSG_MONSTER_INFO* msg = (MSG_MONSTER_INFO*)pMsgRoot;

			mpObjectManager->AddMonster( &msg->mMonsterInfo );

		}
		break;
	case NM_MONSTER_MOVE_SYN:
		{
			MSG_SYN_MOVE_MONSTER* msg = (MSG_SYN_MOVE_MONSTER*)pMsgRoot;

			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;
			if( monster->GetState() == eOBJECT_STATE_DIE )
				return;

			monster->SetMoveSpeed( msg->mMoveSpeed, (eMOVE_SPEED_TYPE)msg->mMoveSpeedType );
			monster->SetPathArray( msg->mPathArray, msg->mCount );
//			monster->SetMoveStopRange( 0 );

			/// 타겟 설정
			monster->SetTargetObject( eOBJECTTYPE_NONE, 0 ); 

		}
		break;
	case NM_MONSTER_ACTIONMOVE_SYN:
		{
			MSG_SYN_ACTIONMOVE_MONSTER* msg = (MSG_SYN_ACTIONMOVE_MONSTER*)pMsgRoot;

			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;
			if( monster->GetState() == eOBJECT_STATE_DIE )
				return;

			monster->SetMoveSpeed( msg->mMoveSpeed, eMOVESPEED_RUN );
			monster->SetPathArray( msg->mPathArray, msg->mCount );
//			monster->SetMoveStopRange( msg->mRange );

			/// 타겟 설정
			monster->SetTargetObject( msg->mTarget.type, msg->mTarget.index ); 
		}
		break;

	case NM_MONSTER_HP_SYN:
		{
			MSG_SYN_MONSTER_HP* msg = (MSG_SYN_MONSTER_HP*)pMsgRoot;
			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;

			if( msg->mDie == true )
			{
				monster->HPChangeDie( msg->mMaxHP );
				mpObjectManager->DelMonster( msg->mMonsterIdx );
			}
			else
				monster->HPChange( msg->mHP, msg->mMaxHP );
		}
		break;
	case NM_MONSTER_SIGHT_OUT_RES:
		{
			MSG_MONSTER_SIGHTOUT* msg = (MSG_MONSTER_SIGHTOUT*)pMsgRoot;
			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;

			mpObjectManager->DelMonster(msg->mMonsterIdx);

		}
		break;
	case NM_MONSTER_MOVESPEED_SYN:
		{
			MSG_SYN_MONSTER_MOVESPEED* msg = (MSG_SYN_MONSTER_MOVESPEED*)pMsgRoot;
			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;

			monster->SetMoveSpeed( msg->mMoveSpeed );
		}
		break;
	case NM_MONSTER_SCALE_SYN:
		{
			MSG_SYN_MONSTER_SCALE* pSynMsg = (MSG_SYN_MONSTER_SCALE*)pMsgRoot;
			cMonster* pMonster = mpObjectManager->GetMonster( pSynMsg->mMonsterIdx );
			if( pMonster )
			{
				pMonster->SetFixedObjectSizeScale( pSynMsg->mScale, false );
			}
		}
		break;
	case NM_MONSTER_MOVESTOP_SYN:
		{
			MSG_SYN_MONSTER_MOVESTOP* msg = (MSG_SYN_MONSTER_MOVESTOP*)pMsgRoot;
			cMonster* monster = mpObjectManager->GetMonster(msg->mMonsterIdx);
			if( !monster )
				return;

			monster->MoveStop( msg->mMonsterPos.x, msg->mMonsterPos.y );						
		}
		break;
	case NM_MONSTER_DESTROY_SYN:
		{
			MSG_SYN_MONSTER_DESTROY* msg = (MSG_SYN_MONSTER_DESTROY*)pMsgRoot;

			cMonster* monster = mpObjectManager->GetMonster( msg->mMonsterIdx );
			if( !monster )
				return;

			mpObjectManager->DelMonster(msg->mMonsterIdx);
		}
		break;
	}
}
void cDummyClient::MsgParser_Npc( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
	case NM_NPC_MODELNUMBER_RES:
		{
			mLoadFlag.flag.npcModelOn = true;
		}
		break;
	case NM_NPC_SIGHT_IN_RES:
		{
			MSG_NPC_INFO* msg = (MSG_NPC_INFO*)pMsgRoot;

			for( int i = 0 ; i < (int)msg->mCount ; ++i )
			{
				sNpcData* npc = &msg->mNpcInfo[i];
				if( npc )
				{
					mpObjectManager->AddNpc( npc );
				}
			}
		}
		break;
	case NM_NPC_SIGHT_OUT_RES:
		{
			MSG_NPCIDX* msg = (MSG_NPCIDX*)pMsgRoot;

			mpObjectManager->DelNpc( msg->mNpcIdx );
		}
		break;
	}
}

void cDummyClient::MsgParser_Skill( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
		/// 070828 PKH 스킬로드
	case NM_SKILL_LIST_RES:
		{
			mLoadFlag.flag.skillInfoOn = true;
		}
		break;
	case NM_SKILL_INFLUENCE_LIST_RES:
		{
//			MSG_RES_INFLUENCE_LIST* pmsg = (MSG_RES_INFLUENCE_LIST*)pMsg;
//			mInfluenceCount = pmsg->mRowCount;
//			assert( mInfluenceCount <= 50 );
//			::memcpy( mInfluence, pmsg->mTable, sizeof(sInfluenceClient) * mInfluenceCount );

			mLoadFlag.flag.InfluenceInfoOn = true;
		}
		break;
	case NM_SKILL_INFLUENCE_GUARDLIST_RES:
		{
			//MSG_SYN_SKILL_INFLUENCE_GUARDLIST* pmsg = (MSG_SYN_SKILL_INFLUENCE_GUARDLIST*)pMsg;

			//mGuardInfluenceCount = pmsg->mRowCount;
			//assert( mGuardInfluenceCount <= 20 );
			//::memcpy( mGuardInfluence, pmsg->mGuardList, sizeof(sInfGuardList) * mGuardInfluenceCount );

			mLoadFlag.flag.guardInfoOn = true;
		}
		break;

	case NM_SKILL_MONSTER_USED_SYN:
	case NM_SKILL_MONSTER_CAST_USED_SYN:
		{
			MSG_SYN_MONSTER_SKILL_USED* pSynMsg = (MSG_SYN_MONSTER_SKILL_USED*)pMsgRoot;
			if( pSynMsg->mAttacker.type != eOBJECTTYPE_MONSTER )
				return;

			cObject* pAttacker = mpObjectManager->GetObject( pSynMsg->mAttacker.type, pSynMsg->mAttacker.index );
			if( pAttacker == 0 )
				return;

			pAttacker->SetTargetObject( pSynMsg->mTarget.type, pSynMsg->mTarget.index );
		}
		break;
	case NM_SKILL_DAMAGE_APPLY_DIE_SYN:
		{
			MSG_SYN_SKILL_APPLY_DIE* msg = (MSG_SYN_SKILL_APPLY_DIE*)pMsgRoot;
			if( msg->mTarget.type == eOBJECTTYPE_HERO )
			{
				if( mPlayerInfo.CharacterIdx == msg->mTarget.index )
				{
					mPlayerExrInfo.RestHP = 0;
					mPlayerExrInfo.RestMP = 0;
					SetState( eState_Die );
				}
			}
			else if( msg->mTarget.type == eOBJECTTYPE_PLAYER )
			{
				if( mPlayerInfo.CharacterIdx == msg->mTarget.index )
				{
					mPlayerExrInfo.RestHP = 0;
					mPlayerExrInfo.RestMP = 0;
					SetState( eState_Die );
				}
				//else
				//{

				//}
			}
			else if( msg->mTarget.type == eOBJECTTYPE_MONSTER )
			{
				cMonster* monster = mpObjectManager->GetMonster( msg->mTarget.index );
				if( monster )
				{
					monster->HPChangeDie( monster->GetMaxHP() );
					mpObjectManager->DelMonster(msg->mTarget.index);
				}
			}
		}
		break;
	case NM_SKILL_DAMAGE_APPLY_HPMP_SYN:
		{
			MSG_SYN_SKILL_APPLY_HPMP* msg = (MSG_SYN_SKILL_APPLY_HPMP*)pMsgRoot;

			if( msg->mTarget.type == eOBJECTTYPE_HERO )
			{
				if( mPlayerInfo.CharacterIdx == msg->mTarget.index )
				{
					mPlayerExrInfo.RestHP = msg->mHP;
					mPlayerExrInfo.MaxHP = msg->mMaxHP;

					mPlayerExrInfo.RestMP = msg->mMP;
					mPlayerExrInfo.MaxMP = msg->mMaxMP;

					if( mCurState == eState_Idle || mCurState == eState_SitDown )
						ChangeState();
				}
			}
			else if( msg->mTarget.type == eOBJECTTYPE_PLAYER )
			{
				if( mPlayerInfo.CharacterIdx == msg->mTarget.index )
				{
					mPlayerExrInfo.RestHP = msg->mHP;
					mPlayerExrInfo.MaxHP = msg->mMaxHP;

					mPlayerExrInfo.RestMP = msg->mMP;
					mPlayerExrInfo.MaxMP = msg->mMaxMP;
				}
//				else
//				{
//					cPlayer* player = mpObjectManager->GetPlayer( msg->mTarget.index );
//					if( player )
//					{
//						player->HPChange( msg->mHP, msg->mMaxHP );
//					}
//				}
			}
			else if( msg->mTarget.type == eOBJECTTYPE_MONSTER )
			{
				cMonster* monster = mpObjectManager->GetMonster( msg->mTarget.index );
				if( monster )
				{
					monster->HPChange( msg->mHP, msg->mMaxHP );
				}
			}
		}
		break;
	case NM_SKILL_USED_RES:
		{
			MSG_RES_SKILL_USED* msg = (MSG_RES_SKILL_USED*)pMsgRoot;

			/// 발동 성공
			if( msg->ErrorCode == 0 )
			{
				/// mp setting
				mPlayerExrInfo.RestMP = msg->mObjectMP;
				mPlayerExrInfo.MaxMP = msg->mMaxMP;
			}
			else	/// 발동 실패
			{
				ChangeState();
			}
		}
		break;
	}
}

void cDummyClient::MsgParser_Item( MSGROOT* pMsgRoot )
{
	switch( pMsgRoot->Protocol )
	{
	case NM_ITEM_INVENTORY_RES:
		{
			MSG_RES_INVENTORY* msgData = (MSG_RES_INVENTORY*)pMsgRoot;
			sInventory* inv = msgData->Inventory;

			for( int i = 0, iend = msgData->RowCount; i < iend; ++i, inv++ )
			{
				memcpy( &mItemArray[inv->number].inv, inv, sizeof(sInventory) );
			}

			mLoadFlag.flag.itemInfoOn = true;
		}
		break;
	case NM_ITEM_INVENTORY_COOLTIME_RES:
		{
			//MSG_RES_INVENTORY_COOLTIME* msgData = (MSG_RES_INVENTORY_COOLTIME*)pMsg;

			//for( int i = 0; i < msgData->RowCount; ++i )
			//{
			//	sInventoryCooltime& cool = msgData->InventoryCooltime[i];
			//	COOLMAN->NetSetCool( cool );
			//}

			mLoadFlag.flag.itemCoolTimeInfoOn = true;
		}
		break;
	case NM_ITEM_DROP_IN_SYN:
	case NM_ITEM_SIGHT_IN_RES:
		{
			MSG_ITEM_INFO* msg = (MSG_ITEM_INFO*)pMsgRoot;
			mpObjectManager->AddDropItem( msg );
		}
		break;
	case NM_ITEM_DROP_OUT_SYN:
	case NM_ITEM_SIGHT_OUT_RES:
		{
			MSG_ITEMIDX* msg = (MSG_ITEMIDX*)pMsgRoot;
			mpObjectManager->DelDropItem(msg->idx);
		}
		break;
	case NM_ITEM_GET_OPEN_RES:
		{
			MSG_RES_ITEM_GET_OPEN* msg = (MSG_RES_ITEM_GET_OPEN*)pMsgRoot;
			if( msg->ErrorCode == ERROR_ITEM_GET_OPEN_SUCCESS )
			{
				MSG_REQ_ITEM_GET msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category = NM_ITEM;
				msg.Protocol = NM_ITEM_GET_REQ;
				msg.itemIndexOrInd = (unsigned long)ITEM_GET_ALL;
				mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else
			{
				SetState( eState_Idle, 2*SECOND );
			}
		}
		break;
	case NM_ITEM_GET_RES:
		{
			MSG_RES_ITEM_GET* msgData = (MSG_RES_ITEM_GET*)pMsgRoot;
			if( msgData->ErrorCode == ERROR_ITEM_GET_SUCCESS || msgData->ErrorCode == ERROR_ITEM_GET_AUTO )
			{
				sInventory* inv = msgData->Inventory;
				for ( long i = 0; i < msgData->RowCount; i++, inv++ )
					memcpy( &mItemArray[inv->number].inv, inv, sizeof(sInventory) );
			}

			SetState( eState_Idle, 2*SECOND );

			MSG_REQ_ITEM_GET_CLOSE msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_GET_CLOSE_REQ;
			mAsyncSelect->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	}
}

void cDummyClient::MsgParser_Quest( MSGROOT* pMsgRoot )
{
	mLoadFlag.flag.questInfoOn = true;
}

void cDummyClient::MsgParser_MakeSkill( MSGROOT* pMsgRoot )
{
	mLoadFlag.flag.makeSkillInfoOn = true;
}


void cDummyClient::SetMapIndex( unsigned short mapIdx, unsigned char mapMode )
{
	mCurMapIdx = mapIdx;
	mCurMapMode = mapMode;
	mMapFolderIdx = (unsigned short)GAMERESOURCEMAN->GetMapFolderIdx( mCurMapIdx, mCurMapMode );

	if( mpObjectManager )
		mpObjectManager->SetFolderIdx( mMapFolderIdx );
}

bool cDummyClient::IsCantSkill( unsigned long skillIdx )
{
	if( mIsSleep == true || mIsStun == true )
		return true;

	if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		return false;

	return true;
}


void cDummyClient::UpdateWeaponState()
{
	unsigned long lItemIndex = mPlayerWeaponInfo.WeaponIdx[eHAND_LEFT];
	unsigned long rItemIndex = mPlayerWeaponInfo.WeaponIdx[eHAND_RIGHT];

	eWEAPON_STATE weaponState = eWEAPON_STATE_NONE;

	cItemDefine* lItemDefine = 0;
	cItemDefine* rItemDefine = 0;

	if( lItemIndex )
		lItemDefine = ITEMMAN->GetItemDefine( lItemIndex );
	if( rItemIndex )
		rItemDefine = ITEMMAN->GetItemDefine( rItemIndex );

	cItemDefine* itemDefine = 0;
	if( lItemIndex == 0 && rItemDefine )
	{
		itemDefine = rItemDefine;
	}
	else if( rItemIndex == 0 && lItemDefine )
	{
		itemDefine = lItemDefine;
	}

	/// 양손 무기를 착용한 경우
	if( itemDefine && itemDefine->GetEquipType() == ITEM_EQUIP_TWO_HANDS )
	{
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

	if( weaponState != eWEAPON_STATE_NONE )
	{
		mPlayerWeaponState = weaponState;
	}
	else
	{
		/// 한손 무기를 착용한 경우
		if( rItemDefine )
		{
			assert( rItemDefine->GetSubType() == ITEM_WEAPON_SWORD );
			weaponState = eWEAPON_STATE_SWORD;
		}

		/// 방패를 착용한 경우
		if( lItemDefine )
		{
			assert( lItemDefine->GetSubType() == ITEM_WEAPON_SHIELD );
			if( weaponState == eWEAPON_STATE_SWORD )
				weaponState = eWEAPON_STATE_SWORD_SHEILD;
			else
				weaponState = eWEAPON_STATE_SHEILD;
		}

		mPlayerWeaponState = weaponState;
	}
}

float cDummyClient::GetFixedObjectSize()
{ 
	if( mFixedObjectSize <= 0.0f )
	{
		assert(NULL);
		return 1.0f;
	}

	return mFixedObjectSize;
}

void cDummyClient::SetFixedObjectSizeScale( unsigned short scalePer, bool init )
{
	mPlayerExrInfo.mFixedObjectSizePer = scalePer;

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		mFixedObjectSize = mPlayerExrInfo.mBaseFixedSize * (float)scalePer * 0.01f;
	}
	else
	{
		sMonsterScript* pMonsterInfo = GAMERESOURCEMAN->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
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
	}
}

void cDummyClient::CheckEquip()
{
	/// 착용 정보 체크

	/// 있다면 리턴

	/// 없다면 인벤 검사
}
