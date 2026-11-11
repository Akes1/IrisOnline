#include "stdafx.h"
#include "KeyAgent.h"

cKeyAgent* cKeyAgent::mSingleton = 0;

cKeyAgent::cKeyAgent()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	memset( mKeyStates, false, sizeof(bool) * 256 );
}

cKeyAgent::~cKeyAgent()
{
	mSingleton = 0;
}

void cKeyAgent::SetDown( eKeyCode code )
{
	mKeyStates[code] = true;
}

void cKeyAgent::SetUp( eKeyCode code )
{
	mKeyStates[code] = false;
}
