#include "stdafx.h"
#include "ServerUIManager.h"
#include "ServerWindow.h"

cServerUIManager::cServerUIManager()
{
	mpServerWindow = 0;
}

cServerUIManager::~cServerUIManager()
{
	mpServerWindow = 0;
}

void cServerUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Server );

	/// 서버 리스트 창 생성 
	mpServerWindow = new cServerWindow;
	mpServerWindow->CreateByPropertyName( "P_ServerWindow", 0 );
}

void cServerUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Server );

	UIMAN->Open();

	SetEnableConnectBtn( true );
}

void cServerUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_Server );

	UIMAN->Close();
}

void cServerUIManager::Process( unsigned long /*accumtime*/ )
{

}

void cServerUIManager::Visible()
{
	if( mpServerWindow )
	{
		mpServerWindow->Show();
	}
}

void cServerUIManager::SetEnableWindow( bool enable )
{
	if( mpServerWindow )
	{
		mpServerWindow->SetEnabled( enable );
	}
}

void cServerUIManager::SetEnableConnectBtn( bool enable )
{
	if( mpServerWindow )
	{
		mpServerWindow->SetEnableConnectBtn( enable );
	}
}