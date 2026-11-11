#include "stdafx.h"
#include "MouseAgent.h"

cMouseAgent* cMouseAgent::mSingleton = 0;

cMouseAgent::cMouseAgent()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mLeft = false;
	mRight = false;
	mMiddle = false;

	mX = mY = 0;
	mOldX = mOldY = 0;
	mDeltaX = mDeltaY = 0;

	mWheel = 0.0f;
	mOldWheel = 0.0f;
	mDeltaWheel = 0.0f;
}

cMouseAgent::~cMouseAgent()
{
	mSingleton = 0;
}

void cMouseAgent::Process()
{
	mDeltaX = mX - mOldX;
	mDeltaY = mY - mOldY;
	mOldX = mX;
	mOldY = mY;

	mDeltaWheel = mWheel - mOldWheel;
	mOldWheel = mWheel;
}

void cMouseAgent::SetLButtonDown( int x, int y )
{
	mLeft = true;
	SetPosition( x, y );
}

void cMouseAgent::SetLButtonUp( int x, int y )
{
	mLeft = false;
	SetPosition( x, y );
}

void cMouseAgent::SetRButtonDown( int x, int y )
{
	mRight = true;
	SetPosition( x, y );
}

void cMouseAgent::SetRButtonUp( int x, int y )
{
	mRight = false;
	SetPosition( x, y );
}

void cMouseAgent::SetMButtonDown( int x, int y )
{
	mMiddle = true;
	SetPosition( x, y );
}

void cMouseAgent::SetMButtonUp( int x, int y )
{
	mMiddle = false;
	SetPosition( x, y );
}

void cMouseAgent::SetPosition( int x, int y )
{
	mOldX = mX;
	mOldY = mY;
	mX = x;
	mY = y;
}

void cMouseAgent::SetDeltaWheel( float dw )
{
	mOldWheel = mWheel;
	mWheel += dw;
}
