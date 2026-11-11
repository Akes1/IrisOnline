#include "StdAfx.h"
#include "mapchangeuimanager.h"
#include "UIBackWindow.h"
#include "LoadingWindow.h"

#include "GameResourceManager.h"
#include "ResourceManager.h"


cMapChangeUIManager::cMapChangeUIManager()
: mpBackWindow(0)
, mpLoadingWindow(0)
{
}

cMapChangeUIManager::~cMapChangeUIManager()
{
}

void cMapChangeUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_MapChange );

	/// 로딩 배경 
	mpBackWindow = new cLoadBackWindow;
	mpBackWindow->CreateByPropertyName( "P_LoadingBack2", 0 );

	mpLoadingWindow = new cLoadingWindow;
	mpLoadingWindow->CreateByPropertyName( "P_LoadingWindow2", 0 );

	mpBackWindow->SetOnlyBackground( true );
}

void cMapChangeUIManager::Exit()
{
	mpLoadingWindow = 0;
	mpBackWindow = 0;
}

void cMapChangeUIManager::Open( unsigned long mapIdx, unsigned char mapMode )
{
	UIMAN->SetCurrentStage( eStage_MapChange );

	cString pathName;
	LPCTSTR texname = GAMERESOURCEMAN->GetMapLoadTexture( mapIdx, mapMode );
	pathName.Format( "./Data/2DData/%s", texname );
	NiTexture* tex = RESOURCEMAN->LoadMapTexture( pathName, false );
	if( tex )
	{
		if( mpBackWindow )
			mpBackWindow->SetBackTexture( tex );
		RESOURCEMAN->ChangeStageImageTexture( tex );
	}

	/// 로딩 전에 로딩배경윈도우나 로딩 윈도우(로딩게이지&팁)이 보여있으면 안된다. [8/10/2009 Jo_2]
	if( mpBackWindow && mpBackWindow->IsVisible() == true )
	{
		assert(0 && "Warning! mpBackWindow Must invisible!");
		mpBackWindow->Hide();
	}

	if( mpLoadingWindow && mpLoadingWindow->IsVisible() == true )
	{
		assert(0 && "Warning! mpLoadingWindow Must invisible!");
		mpLoadingWindow->Hide();
	}

	UIMAN->Open();
}

void cMapChangeUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_MapChange );

	UIMAN->Close();
}

void cMapChangeUIManager::Process()
{

}

bool cMapChangeUIManager::UpdateLoadingGauge( float curMax )
{
	assert(mpLoadingWindow);

	mpLoadingWindow->IncreaseGaugeValue( curMax );
	return mpLoadingWindow->IsGaugeEnd();
}