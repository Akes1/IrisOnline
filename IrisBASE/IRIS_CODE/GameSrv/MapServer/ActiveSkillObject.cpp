
#include "stdafx.h"
#include "BaseObject.h"
#include "Player.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "ActiveSkillObject.h"
#include "SkillManager.h"
#include "AppTimer.h"
#include "Protocol.h"
#include "BaseObject_Common.h"
#include "Monster_Common.h"
#include "PackedData.h"
#include "StatusCalc.h"



cActiveSkillObject::cActiveSkillObject()
{
	mGroundSkillRange = 0.0f;
}



cActiveSkillObject::~cActiveSkillObject()
{
}



void* cActiveSkillObject::operator new( size_t n )
{
	if(n != sizeof(cActiveSkillObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocSkillObject();
}



void cActiveSkillObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0)
	{
		assert(0);
		return;
	}

	if(n != sizeof(cActiveSkillObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreeSkillObject( static_cast<cActiveSkillObject*>(ptr) );

	return;
}



void cActiveSkillObject::CastingProcess()
{
}



void cActiveSkillObject::SkillActProcess()
{
}



void cActiveSkillObject::SkillDamageProcess()
{
}



void cActiveSkillObject::Process()
{
	///// 캐스팅 시간을 먼저 검사한다.
	//if( mCastingTime != 0.0f && mWhereIs == WHEREIS_USER )
	//{
	//	/// 캐스팅 시간을 넘어 서면 - 스킬을 유저 스킬 메세지큐에 넣는다.
	//	/// ※ eSKILL_CASTING_END_SKILLACT_START 메세지를 user 관련 큐에 기록
	//	/// 현재 process는 종료되고 user process에서  eSKILL_CASTING_END_SKILLACT_START 처리
	//	/// 하고나면 현재 process 의 캐스팅 다음 루틴을 적용한다.
	//	if( ( TIMER->GetCurrentTime() - mCurrentActTime ) > mCastingTime )
	//	{
	//		/// 스킬 발동시간은 캐스팅이 끝난 현재 시간
	//		mCurrentActTime = TIMER->GetCurrentTime();

	//		/// 데미지 게산이 이루어지는 시간
	//		mDamageCalcuTime = mActLifeTime*(mDamagePersent*0.01f);

	//		/// user의 큐에 캐스팅끝 스킬 시작 메세지를 넣어줌
	//		mpUser->SetSkillMessage( this, eSKILL_CASTING_END_SKILLACT_START );

	//		/// 캐스팅은 끝남
	//		mCastingTime = 0.0f;

	//		return;
	//	}
	//	else
	//	{
	//		return;
	//	}
	//}

	//cBaseObject* pUser = OBJECTMANAGER->GetObject( GetAttacker().type, GetAttacker().index );
	///// 공격자의 포인터를 확인
	//if( pUser == NULL )	
	//{ 
	//	CONSOLE->WriteLog("ASSERT-070612-ActiveSkillObject ERROR");
	//	assert(NULL);
	//	return; 
	//}

	///// 스킬의 데미지가 적용될 타이밍 검사 - 사용자일 경우만 적용된다.

	switch( mTarget.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
	case eOBJECTTYPE_MONSTER:
		{
			///// 스킬이 현재 유저에 있고 데미지 적용시점까지 가지 않은 상태
			//if( !mbDamageSend )
			//{
			//	/// 데미지 적용시간 까지 도달 하지 않은경우 프로세스 통과
			//	if( ( TIMER->GetCurrentTime() - mCurrentActTime ) < mDamageCalcuTime )	{ break; }

			//	/// 스킬이 사용될때 필요한 요구조건을 적용한다.
			//	if( SKILLMANAGER->AdjustRequireValue( pUser->GetObjectType(), pUser->GetObjectID(), mSkillIdx ) == false )
			//	{
			//		mbSkillLife = false;
			//		return;
			//	}

			//	cBaseObject* pTarget = pUser->GetTarget();
			//	/// 공격자가 지정한 타겟의 포인터를 확인
			//	if( pTarget != NULL )	
			//	{ 
			//		///	데미지 적용과 함께 공격자가 누구인지 알린다.
			//		pTarget->SetSkillMessage( this, eSKILL_CONFIRM , pUser->GetObjectID(), pUser->GetObjectType() );
			//		mbDamageSend = true;
			//	}
			//
			//}
		}break;
	case eOBJECTTYPE_NONE:
		{
			///// 스킬 발동 시점에 타겟이 없을 순 없다. - 스킬을 받는 경우는 타겟이 없다.
			//if(mWhereIs == WHEREIS_USER) 
			//{
			//	assert(0);
			//}
		}break;
		//case eOBJECTTYPE_GROUND:
		//	{
		//		/// 데미지가 적용될 타겟들을 선별한다.
		//		cMonster*		pMonster;
		//		tPointerHashMap<unsigned long, cMonster*>::cConstIterator i = OBJECTMANAGER->GetMonsterMap()->Begin();
		//		tPointerHashMap<unsigned long, cMonster*>::cConstIterator end = OBJECTMANAGER->GetMonsterMap()->End();
		//		for( ; i != end; ++i )
		//		{
		//			pMonster = (*i).mSecond;
		//			if(pMonster->GetStateDie() == true) continue;


		//			cRangeCheck rangecheck;
		//			rangecheck.SetRadius(mGroundSkillRange);

		//			if(rangecheck.IsRange(mGroundSkillPos, NiPoint3(pMonster->GetXPos() , pMonster->GetYPos(), 0.0f)) == true)
		//			{
		//				/// 스킬이 사용될때 필요한 요구조건을 적용한다.
		//				/// 요구조건 알아오기


		//				///	데미지 적용과 함께 공격자가 누구인지 알린다.
		//				pMonster->SetSkillMessage(this, eSKILL_CONFIRM , mpUser->GetObjectID(), mUserKind);
		//			}
		//		}
		//		/// 몬스터들의 위치를 알아낸다.
		//		/// 실시간으로 체크하여 모두에게 데미지를 준다.
		//	}
		//	break;
		//case eOBJECTTYPE_MULTY: break;
	}

	///// 스킬의 활성시간 검사
	//float t = TIMER->GetCurrentTime() - mCurrentActTime;
	//if( t > mActLifeTime )
	//{
	//	if(!mbSkillLife) assert(0);

	//	if(t > mCoolTime)
	//		mbSkillLife = false;
	//}

	///// 스킬이 생명 주기가 끝난경우
	//if( false == GetSkillLife() )
	//{

	//	/// 저거항목배열저장
	//	SKILLMANAGER->InsertDeletSkillObject( this );

	//	///// 스킬 사용자에게서 생명주기가 끝난 경우 타겟에게 스킬을 전도
	//	//if( GetWhereIs() == WHEREIS_USER )
	//	//{
	//		/// 유저가 살아 있는지 체크
	//		if( !(pUser->GetStateDie()) )
	//		{
	//			/// 사용자의 큐에 스킬 종료 메세지를 전달함
	//			( pUser )->SetSkillMessage( this, eSKILL_END_USING );

	//			///// 사용한 경우
	//			//switch( SKILLMANAGER->GetSkillShape( GetUserKind(), GetSkillInfoIndex()) )
	//			//{

	//			//case SKILLSHAPE_BUFF:
	//			//	{
	//			//		if( GetTarget().kind == eOBJECTTYPE_HERO )
	//			//		{
	//			//			( pUser )->SetSkillMessage( this, eSKILL_END_BUFF);
	//			//		}
	//			//	}
	//			//	break;
	//			//default:
	//			//	{
	//			//		( pUser )->SetSkillMessage( this, eSKILL_END_USING);
	//			//	}
	//			//	break;
	//			//}

	//			cBaseObject* pTarget = pUser->GetTarget();
	//			/// 공격자가 지정한 타겟의 포인터를 확인
	//			if( pTarget == NULL )	
	//			{ 
	//				CONSOLE->WriteLog("ASSERT-070612-ActiveSkillObject ERROR");
	//				assert(NULL);
	//				return; 
	//			}

	//			/// 스킬 사용대상이 자기 자신 인경우 타겟에서 스킬을 넘기지 않음
	//			if( pTarget->GetObjectType() != eOBJECTTYPE_HERO )
	//			{

	//				/// 타겟이 살아 있는지 체크
	//				if( !( pTarget->GetStateDie() ) )
	//				{
	//					/// 스킬의 자동 추가 - 새로운 객체가 얻어진다.
	//					/// 공격자의 타겟정보를 사용하기 위해 공격자 포인터를 넘긴다.
	//					cSkillObject* pTargetSkill = SKILLMANAGER->SkillTransferTarget( GetSkillIdx(), pUser );
	//					if(!pTargetSkill)
	//					{
	//						CONSOLE->WriteLog("ASSERT-070612-SkillTransferTarget ERROR");
	//						assert(NULL);
	//						return;
	//					}

	//					/// 객체에게 스킬을 추가해 달라고 메시지를 날린다.
	//					( pTarget )->SetSkillMessage( pTargetSkill, eSKILL_REGIST_BY_OTHER_OBJECT );
	//				}
	//			}
	//		}
		//}
		//else /// 객체에게 스킬을 제거 해 달라고 메시지를 날린다.
		//{
		//	/// 유저가 살아 있는지 체크
		//	/// Monster Q로 제거 메시지 보낸다.

		//	/// 받은 경우 
		//	//switch( SKILLMANAGER->GetSkillShape( GetUserKind(), GetSkillInfoIndex()) )
		//	//{
		//	//case SKILLSHAPE_BUFF:
		//	//	{
		//	//		( pUser )->SetSkillMessage( this, eSKILL_END_OTHER_LONGTIME );
		//	//	}
		//	//	break;
		//	//default:
		//	//	{
		//			( pUser )->SetSkillMessage( this, eSKILL_END_OTHER_1TIME );
		//	//	}
		//	//	break;
		//	//}
		//}
	//}
}



void cActiveSkillObject::NonCastingSkillStart()
{
	//if(mUserKind == eOBJECTTYPE_PLAYER)
	//{
	//	///버프라면 클라이언트에 정보를 알린다.
	//	if(SKILLMANAGER->GetSkillShape(mUserKind,mSkillInfoIndex) == SKILLSHAPE_BUFF)
	//	{
	//		MSG_CHAT msg;
	//		msg.Category = NM_CHAT;
	//		msg.Protocol = NM_CHAT_NORMAL_SYN;
	//		msg.CharacterIdx	= mpUser->GetObjectID();


	//		if(mTarget.kind == eOBJECTTYPE_HERO)
	//		{
	//			mpUser->AddSkillList(this);

	//			MSG_PLAYER_SKILL Msg;

	//			Msg.Category = NM_SKILL;
	//			Msg.Protocol = NM_SKILL_START_BY_PLAYER;

	//			Msg.playerid		= mpUser->GetObjectID();
	//			Msg.skillinfonumber = mSkillInfoIndex;
	//			Msg.uniqueindex		= mUniqueIndex;
	//			Msg.target			= mTarget;

	//			PACKEDDATA->QuickSend( (cBaseObject*)mpUser, (char*)&Msg, sizeof(Msg) );

	//			/// 070214 PKH 스텟 재계산
	//			STATUSCALC->CalcPlayer( Msg.playerid );

	//			swprintf( msg.ChatMsg, L"%d - %d버프 적용",mpUser->GetObjectID(),mSkillInfoIndex );
	//		}
	//		else
	//		{
	//			swprintf( msg.ChatMsg, L"%d - %d버프 시전 시작",mpUser->GetObjectID(),mSkillInfoIndex );
	//		}

	//		wcscpy(msg.Name , L"operator");

	//		NM_CHAT_MsgParserFromServer( 0, ( char * )&msg, msg.GetMsgLength() );
	//	}
	//	else
	//	{
	//		mpUser->AddSkillList(this);

	//		MSG_PLAYER_SKILL Msg;

	//		Msg.Category = NM_SKILL;
	//		Msg.Protocol = NM_SKILL_START_BY_PLAYER;

	//		Msg.playerid		= mpUser->GetObjectID();
	//		Msg.skillinfonumber = mSkillInfoIndex;
	//		Msg.uniqueindex		= mUniqueIndex;
	//		Msg.target			= mTarget;

	//		PACKEDDATA->QuickSend( (cBaseObject*)mpUser, (char*)&Msg, sizeof(Msg) );

	//		/// 070214 PKH 스텟 재계산
	//		STATUSCALC->CalcPlayer( Msg.playerid );

	//	}
	//
	//}
	//else
	//{
	//	/// 공격자가 플레이어가 아닌경우

	//}

}



void cActiveSkillObject::TargetSkillStart()
{
	//switch(SKILLMANAGER->GetSkillShape(mUserKind,mSkillInfoIndex))
	//{
	//case SKILLSHAPE_BUFF:
	//	{
	//		mpUser->AddSkillList(this);

	//		MSG_PLAYER_SKILL Msg;

	//		Msg.Category = NM_SKILL;
	//		Msg.Protocol = NM_SKILL_START_BY_PLAYER;

	//		Msg.playerid		= mpUser->GetObjectID();
	//		Msg.skillinfonumber = mSkillInfoIndex;
	//		Msg.uniqueindex		= mUniqueIndex;
	//		Msg.target			= mTarget;

	//		PACKEDDATA->QuickSend( (cBaseObject*)mpUser, (char*)&Msg, sizeof(Msg) );

	//		/// 070214 PKH 스텟 재계산
	//		STATUSCALC->CalcPlayer( Msg.playerid );

	//		MSG_CHAT msg;
	//		msg.Category = NM_CHAT;
	//		msg.Protocol = NM_CHAT_NORMAL_SYN;
	//		msg.CharacterIdx	= mpUser->GetObjectID();

	//		swprintf( msg.ChatMsg, L"%d - %d버프 적용",mpUser->GetObjectID(),mSkillInfoIndex );

	//		wcscpy(msg.Name , L"operator");

	//		NM_CHAT_MsgParserFromServer( 0, ( char * )&msg, msg.GetMsgLength() );

	//	}
	//	break;
	//}

}



void cActiveSkillObject::SetSkillActTime( float castingtime, float actlifetime, float cooltime, unsigned int DamagePersent )
{
	///// 기본공격이 아니고 캐스팅 시간 없는 스킬
	//if(mSkillInfoIndex > MAXPLAYERBASESKILLINFOIDX && mWhereIs == WHEREIS_USER && castingtime == 0.0f)
	//{
	//	NonCastingSkillStart();
	//}

	///// 기본 공격이 아니고 현재 스킬이 타겟에게 적용되었을 경우
	//if(mSkillInfoIndex > MAXPLAYERBASESKILLINFOIDX && mWhereIs == WHEREIS_TARGET)
	//{
	//	TargetSkillStart();
	//}

	/// 시간에 대한 정보 셋팅
	mbSkillLife = true;
	mCurrentActTime = TIMER->GetCurrentTime();
	mCastingTime = castingtime;
	mActLifeTime = actlifetime;
	mCoolTime = cooltime;
	mDamageCalcuTime = mActLifeTime*(DamagePersent*0.01f);
	mDamagePersent = DamagePersent;
	mbDamageSend = false;

	/// 공격생명주기가 쿨타임보다 클경우 쿨타임을 공격생명주기랑 같게함
	/// ※쿨타임이 짧을 경우 공격이 다 끝나기 전에 다시 공격을 할 수 있다.
	if(mActLifeTime > mCoolTime) mCoolTime = mActLifeTime;

}