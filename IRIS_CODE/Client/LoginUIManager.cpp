#include "stdafx.h"
#include "LoginUIManager.h"
#include "LoginWindow.h"

cLoginUIManager* cLoginUIManager::mpSingleton = 0;

cLoginUIManager::cLoginUIManager()
:mpLoginWindow( 0 )
,mpVersionWindow( 0 )
{
	assert( mpSingleton == 0 && "bad singleton" );
	mpSingleton = this;
}

cLoginUIManager::~cLoginUIManager()
{
	mpSingleton     = 0;
}

/// 주의사항 :
void cLoginUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Login );

	/// 로그인 윈도우 생성 
	mpLoginWindow = new cLoginWindow;
	mpLoginWindow->CreateByPropertyName( "P_LoginWindow", 0 );

	mpVersionWindow = new cVersionWindow;
	mpVersionWindow->CreateByPropertyName( "P_VersionWindow", 0 );
}

/// 주의사항 :
void cLoginUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Login );

	UIMAN->Open();
}

/// 주의사항 :
void cLoginUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_Login );

	UIMAN->Close();
}


/// 주의사항 :
void cLoginUIManager::Process( unsigned long /*accumtime*/ )
{
}

void cLoginUIManager::FocusIDEdit()
{
	mpLoginWindow->ClearPass();
	mpLoginWindow->ClearID( true );
}

void cLoginUIManager::FocusPassEdit()
{
	mpLoginWindow->ClearPass( true );
}

// 
void cLoginUIManager::Visible()
{
	mpLoginWindow->SetEnabled( true );
	mpLoginWindow->Show();
}

void cLoginUIManager::SetSendLoginFlag( bool set )
{
	mpLoginWindow->SetSendLoginFlag( set );
}
