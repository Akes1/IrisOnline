#include "stdafx.h"
#include "InputSystem.h"

cInputSystem* cInputSystem::mSingleton = 0;

cInputSystem::cInputSystem()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cInputSystem::~cInputSystem()
{
	mSingleton = 0;
}

void cInputSystem::Process()
{
	mMouseAgent.Process();
}
