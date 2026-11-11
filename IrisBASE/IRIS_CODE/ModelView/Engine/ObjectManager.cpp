#include "stdafx.h"
#include "ObjectManager.h"

cObjectManager* cObjectManager::mSingleton = 0;

cObjectManager::cObjectManager()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	//mpHero = 0;
}

cObjectManager::~cObjectManager()
{
	//delete mHero;
}

bool cObjectManager::Init()
{
	return true;
}

void cObjectManager::Exit()
{
}

void cObjectManager::Open()
{
	//AddHero();
}

void cObjectManager::Close()
{
}

void cObjectManager::Process( float /*time*/ )
{
	//if( mHero )
 //       mHero->OnProcess( time );
}

/*
void cObjectManager::MoveHeroByMouse( int mouseX, int mouseY )
{
	float posX, posY;
	if(	WORLDMAN->GetTerrain()->GetPickPosition( mouseX, mouseY, posX, posY ) == false )
		return;

	// 이동..
	if( mHero )
		mHero->Move( posX, posY );
}

void cObjectManager::AddHeroInfo(unsigned long id, int kind )
{
	mHero = new cHero;
//	mkHero->SetObjectIdx( id );
//	mkHero->SetKind( kind );
}

void cObjectManager::AddHero()
{
	if( !mHero )	return;

	sPlayerInfo sBaseInfo; 
	sBaseInfo.Gender = eGENDER_FEMALE;
	sBaseInfo.Race = eRACE_BEAST;

	sPlayerWearInfo sWearInfo;
	sPlayerWeaponInfo sWeaponInfo;

	// 위치 정보 생성..
	NiPoint3 pos = NiPoint3(19509.0f, 23417.0f, 0);
	WORLDMAN->GetTerrain()->GetHeight(pos.x,pos.y,pos.z);

	mHero->Create( &sBaseInfo, &sWearInfo, &sWeaponInfo, pos );
}
*/

void cObjectManager::AddPlayer( unsigned long /*id*/, int /*kind*/, int x, int y )
{
//	mkPlayer[id] = NiNew cPlayer;
//	mkPlayer[id]->SetObjectIdx( id );

	// 위치 정보 생성..
	NiPoint3 pos = NiPoint3(float(x), (float)y, 0);
	//WORLDMAN->GetTerrain()->GetHeight(pos.x,pos.y,pos.z);

//	CreateModel( kind, mPlayer[id], pos );
}
