#include "stdafx.h"
#include "Object.h"

#include "Npc_Common.h"

cNpc::cNpc( cObjectManager* manager )
: cObject( eOBJECTTYPE_NPC, manager )
{

}

cNpc::~cNpc()
{

}

bool cNpc::Init( unsigned long objId, unsigned long npcClassIdx, NiPoint3 sPos, NiMatrix3 sRot )
{
	mObjectId = objId;
	mClassId = npcClassIdx;

	mTrans.m_Translate = sPos;
	mTrans.m_Rotate = sRot;

	return true;
}

void cNpc::Process( unsigned long deltaTime, unsigned long accumTime )
{
	deltaTime;
	accumTime;

}
