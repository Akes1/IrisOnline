#include "stdafx.h"
#include "action.h"

#include "../Common/Protocol.h"
#include "Monster_Common.h"
#include "Skill_Common.h"
#include "Status_Common.h"
#include "BaseObject_Common.h"

#include "Monster.h"
#include "Player.h"
#include "AIManager.h"
#include "ObjectManager.h"
#include "AppTimer.h"
#include "Pathfinder.h"
#include "drop.h"
#include "ioContextPool.h"
#include "gamesrv.h"



cAction::cAction(eACTION_ID actid) : mActionid(actid)
{
}



void cAction::ActionInit( cMonster* /*pMonster*/, unsigned long /*accumTime*/ )
{
}



eACTION_ID cAction::GetActionID()
{
	return mActionid;
}


