#include "stdafx.h"
#include "Object.h"

#include "GameResourceManager.h"
#include "Stage_Common.h"


cPortal::cPortal( cObjectManager* manager )
: cObject( eOBJECTTYPE_PORTAL, manager )
{
	mRange = 0.0f;
}

cPortal::~cPortal()
{

}

bool cPortal::Init( unsigned long posScriptIdx, NiPoint3 pos, NiMatrix3 rot )
{
	mPosScriptIdx = posScriptIdx;

	mTrans.m_Translate = pos;
	mTrans.m_Rotate = rot;

	sStageChangePos* pInfo = GAMERESOURCEMAN->GetPosScriptInfo( mPosScriptIdx );
	if( pInfo == 0 )
		return false;

	mTargetScriptIdx = pInfo->mTargetPosIdx;
	mRange = pInfo->mRange;

	return true;
}

void cPortal::Process( unsigned long deltaTime, unsigned long accumTime )
{
	deltaTime;
	accumTime;
}
