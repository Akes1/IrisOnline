#include "stdafx.h"
#include "IntroUIManager.h"
#include "IntroWindow.h"

#include "ResourceManager.h"
#include "Application.h"

///
cIntroUIManager::cIntroUIManager()
: mpBackWindow(0)
{
}

///
cIntroUIManager::~cIntroUIManager()
{
}

///
void cIntroUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Login );

	mpBackWindow = new cIntroWindow;
	mpBackWindow->CreateByPropertyName( "P_IntroBack", 0 );
	mpBackWindow->SetOnlyBackground( true );
}

///
void cIntroUIManager::InitStage()
{
}

///
void cIntroUIManager::Exit()
{
	if( mpBackWindow )
		mpBackWindow->SetBackTexture( 0 );

	mpBackWindow = 0;
}

///
void cIntroUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Login );

	if( mpBackWindow == 0 )
	{
		assert(0);
		UIMAN->Open();
		return;
	}

	cString path;

	//path.Format("./language/%s/LoadingMap_Image00.dds", cApplication::mLangaugeFolder.Cstr() );
	NiTexture* tex = RESOURCEMAN->LoadMapTexture( "./Data/2ddata/LoadingMap_Image00.dds", false );
	if( tex )
	{
		mpBackWindow->SetBackTexture( tex );
		mpBackWindow->SetAlpha(0.0f);
	}

	path.Format("./language/%s/loadingmap_image_grade.dds", cApplication::mLangaugeFolder.Cstr() );
	tex = RESOURCEMAN->LoadMapTexture( path.Cstr(), false );
	if( tex )
		mpBackWindow->SetGradeTexture( tex );
	else
		mpBackWindow->SetAlpha( 1.0f );

	UIMAN->Open();
}

///
void cIntroUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_Login );

	UIMAN->Close();
}
