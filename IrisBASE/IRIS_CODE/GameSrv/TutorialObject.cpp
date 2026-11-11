#include "stdafx.h"
#include "gamesrv.h"
#include "TutorialObject.h"
#include "ThemeManager.h"
#include "Protocol.h"
#include "ObjectManager.h"
#include "StageScript.h"
#include "GridManager.h"
#include "AIManager.h"

cTutorialObject::cTutorialObject()
{
}

cTutorialObject::~cTutorialObject()
{
}

void* cTutorialObject::operator new( size_t n )
{
	if( n != sizeof(cTutorialObject) ) 
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator new %d != sizeof(cTutorialObject)", n );
		return NULL;
	}

	return THEMEMANAGER->AllocThemeObject();
}


void cTutorialObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator delete ptr == 0" );
		return;
	}

	if( n != sizeof(cTutorialObject) ) 
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator delete n != sizeof(cTutorialObject)", n );
		return;
	}

	THEMEMANAGER->FreeThemeObject( static_cast<cTutorialObject*>(ptr) );
	return;
}

bool cTutorialObject::InitTheme( unsigned short themeIdx, unsigned char mode )
{
	if( cThemeObject::InitTheme( themeIdx, mode ) == false )
		return false;

	mDependency = true;

	return true;
}
