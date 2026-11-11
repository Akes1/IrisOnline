#include "stdafx.h"
#include "BaseStage.h"

cBaseStage::cBaseStage()
{
	mpStageManager = NULL;
	mInit = false;
}

cBaseStage::~cBaseStage()
{
}

//void* cBaseStage::GetStageManager() const
//{
//	assert( mpStageManager );
//	return static_cast< void* >( mpStageManager );
//}
