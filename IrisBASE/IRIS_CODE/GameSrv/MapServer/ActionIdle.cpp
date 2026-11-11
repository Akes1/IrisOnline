#include "StdAfx.h"
#include "actionidle.h"
#include "GameSrv.h"

#include "Monster.h"


cActionIdle::cActionIdle(void) : cAction( eACT_IDLE )
{
}


cActionIdle::~cActionIdle(void)
{
}


void cActionIdle::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );

	pMonster->MoveStop( true );
}


eACTION_CHANGE cActionIdle::Action( cMonster* pMonster, unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		NETWORK2->PostServerEvent("cActionIdle::Action pMonsterScript == NULL");
		return eACTCH_ERROR;
	}

	if( accumTime > pMonster->GetIdleEndTime() )
		return eACTCH_MODE_END;

	return eACTCH_CONTINUE;
}
