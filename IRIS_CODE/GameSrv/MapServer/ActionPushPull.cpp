#include "stdafx.h"
#include "actionpushpull.h"
#include "gamesrv.h"

#include "Monster_Common.h"

#include "MonsterScript.h"

#include "Monster.h"


cActionPushPull::cActionPushPull(void): cAction( eACT_PUSHPULL )
{
}



cActionPushPull::~cActionPushPull(void)
{
}



void cActionPushPull::ActionInit( cMonster* pMonster, unsigned long /*accumTime*/ )
{
	if( pMonster == NULL )
		return;
}



eACTION_CHANGE cActionPushPull::Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionRegen::Action pRegenGroupScript == NULL");
		return eACTCH_ERROR;
	}

	/// 몬스터 이동
	if( pMonster->PushPullUpdate( deltaTime ) == false )
	{
		if( pMonster->GetIdleEndTime() > accumTime )
			return eACTCH_PUSHPULL_IDLE;
		else
			return eACTCH_PUSHPULL_END;
	}

	/// 특별한 이벤트에 걸리지 않는 한 계속 이동
	return eACTCH_CONTINUE;
}
