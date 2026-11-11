#include "stdafx.h"
#include "BaseObject.h"
#include "gamesrv.h"

#include "Monster_Common.h"

#include "ObjectManager.h"
#include "SkillManager.h"
#include "StatusCalc_Server.h"
#include "Aimanager.h"
#include "GridManager.h"
#include "InfluenceObject.h"
#include "AIManager.h"

cBaseObject::cBaseObject(unsigned char objectType)
{
	mObject.type = objectType;
	mMapNumber   = MAP_MIN;
	mGridPos     = DEF_GRID_POS;

	SetStateDie( false );
}

cBaseObject::~cBaseObject()
{
	SetStateDie( true );
}

cBaseObject* cBaseObject::GetTarget( )
{
	/// 타겟 종류에 따라
	switch( mTarget.type )
	{
	case eOBJECTTYPE_PLAYER:
		/// 플레이어 포인터 반환
		return (cBaseObject*)OBJECTMANAGER->GetPlayer( mTarget.index );

	case eOBJECTTYPE_MONSTER:
		/// 몬스터 포인터 반환
		return (cBaseObject*)OBJECTMANAGER->GetMonster( mTarget.index );

	default:
		/// 없으면 NULL포인터 반환
		return NULL;
	}
}

bool cBaseObject::SetTarget( unsigned char type, unsigned long idx )
{
	mTarget.index = idx;
	mTarget.type = type;
	return true;
}

float cBaseObject::Height()
{
	float height = 0.0f;
	AIMANAGER->CalcHeight( mMapNumber, &height, mObjectPos.x, mObjectPos.y );
	return height;
}

bool cBaseObject::GetStateDie( )
{ 
	NETWORK2->PostServerEvent("cBaseObject::GetStateDie call");
	return false; 
}

void cBaseObject::SetPos(float x, float y)
{
	//if( AIMANAGER != NULL && AIMANAGER->IsPossible( mMapNumber, x, y, mObject ) == false )
	//{
	//	Verbose->WriteLog("cBaseObject::SetPos ERROR %d,%.0f,%.0f,%d,%d", mMapNumber, x, y, mObject.index, mObject.type );
	//	NETWORK2->PostServerEvent("cBaseObject::SetPos ERROR %d,%f,%f,%d,%d", mMapNumber, x, y, mObject.index, mObject.type );
	//	return;
	//}

	if ( x < 0 || y < 0 )
	{
		assert(NULL);
		return;
	}
	mObjectPos.x = x; mObjectPos.y = y;
}