#include "StdAfx.h"
#include "actiondie.h"
#include "GameSrv.h"

#include "../Common/Protocol.h"

#include "GridManager.h"

#include "Monster.h"
#include "SkillManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Drop.h"
#include "AIManager.h"
#include "QuestManager.h"
#include "PartyManager.h"

cActionDie::cActionDie(void)
: cAction( eACT_DIE )
{
}



cActionDie::~cActionDie(void)
{
}



void cActionDie::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );
	/// 여기서 몬스터를 삭제하게되면 damage 메세지 발송하기전에 삭제되서 발송을 못한다.

	mFirstRun = true;
	mDieEndTime = accumTime + 3000;
}



eACTION_CHANGE cActionDie::Action( cMonster* pMonster, unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_CONTINUE;

	if( mFirstRun == true )
	{
		mFirstRun = false;

		/// 죽으면서 몬스터를 소환하는지 체크
		pMonster->DeathGenMonster();

		if( pMonster->IsCriDie() == true )
			pMonster->SendSpeech( eMONSTERTALK_DIE2 );
		else
			pMonster->SendSpeech( eMONSTERTALK_DIE1 );	

		/// 리젠 대기 몬스터 목록에 넣음
		if( pMonster->GetDeathMonster() == false )
			pMonster->Die();		/// 죽는시점 이벤트 처리
	}

	/// 일정시간후에 오브젝트 삭제 처리를 한다. 
	/// ※오브젝트가 바로 사라지면 관련된 다른오브젝트가 처리를 못하고 끝남
	if( mDieEndTime < accumTime )
	{
		/// 그리드/오브젝트 메니저에서 제거
		GRIDMANAGER->RemoveMonster( pMonster );

		/// 죽으면서 소환을 하는몬스터 처리
		if( pMonster->GetDeathMonster() == false )
			OBJECTMANAGER->InsertDeleteMonster( pMonster->GetObjectID() );		/// 일반몬스터
		else
		{
			/// 소환하면서 죽는 몬스터
			OBJECTMANAGER->AddDeathMonster( pMonster );		/// objectmanager 안의 몬스터map 목록에서만 제거 pool만유지

			pMonster->SetIdleEndTime( ULONG_MAX );			/// idle상태 설정
			return eACTCH_DIE_END;
		}
	}

	return eACTCH_CONTINUE;
}
