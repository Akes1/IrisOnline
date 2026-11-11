#include "stdafx.h"
#include "SelectUIManager.h"

#include "SelectWindow.h"
#include "ChannelWindow.h"

#include "GameResourceManager.h"

cSelectUIManager::cSelectUIManager()
: mpSelectWindow(0)
, mpChannelWindow(0)
, mpSelectInfoWindow(0)
, mpSelectButtonWindow(0)
{
}

cSelectUIManager::~cSelectUIManager()
{
	mpChannelWindow = 0;
	mpSelectWindow = 0;
	mpSelectInfoWindow = 0;
	mpSelectButtonWindow = 0;
}

void cSelectUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Select );

	/// 캐릭터 선택 창 생성 
	mpSelectWindow = new cSelectWindow;
	mpSelectWindow->CreateByPropertyName( "P_SelectWindow", 0 );

	mpSelectInfoWindow = new cSelectInfoWindow;
	mpSelectInfoWindow->CreateByPropertyName( "P_SelectInfoWindow", 0 );

	/// 채널 선택 창 생성
	mpChannelWindow = new cChannelWindow;
	mpChannelWindow->CreateByPropertyName( "P_ChannelWindow", 0 );

	/// 선택 버튼 창 생성
	mpSelectButtonWindow = new cSelectButtonWindow;
	mpSelectButtonWindow->CreateByPropertyName( "P_SelectButtonWindow", 0 );

	/// 등급 표시 창 생성
	mpGradeWindow = new cUIWindow;
	mpGradeWindow->CreateByPropertyName( "P_SelectGradeWindow", 0 );

}

void cSelectUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Select );

	UIMAN->Open();

	SetEnableWindow( true );
}

void cSelectUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_Select );

	UIMAN->Close();
}

void cSelectUIManager::Process( unsigned long /*accumtime*/ )
{
}

void cSelectUIManager::VisibleListState()
{
	if( mpSelectWindow )
		mpSelectWindow->Show();

	if( mpSelectInfoWindow )
		mpSelectInfoWindow->Show();

	if( mpSelectButtonWindow )
	{
		mpSelectButtonWindow->Show();
		//mpSelectButtonWindow->ShowDelete( true );
	}

	if( mpChannelWindow )
		mpChannelWindow->Hide( false );
}

void cSelectUIManager::VisibleChannelState()
{
	if( mpSelectWindow )
		mpSelectWindow->Hide();

	if( mpSelectButtonWindow )
	{
		//mpSelectButtonWindow->ShowDelete( false );
		mpSelectButtonWindow->Hide();
	}

	if( mpChannelWindow )
		mpChannelWindow->Show();
}

void cSelectUIManager::SetSelectInfo( int level, LPCTSTR name, unsigned long jobIdx, unsigned int mapIdx )
{
	mpSelectInfoWindow->SetLevel( level );
	mpSelectInfoWindow->SetName( name );
	mpSelectInfoWindow->SetJob( GAMERESOURCEMAN->GetJobName( jobIdx ) );

	unsigned long textIndex = mapIdx;
	if( mapIdx == 0 )
	{
		textIndex = MAP_TUTORIAL; 
	}
	mpSelectInfoWindow->SetMapName( GAMERESOURCEMAN->GetMapName( textIndex ) );
}

void cSelectUIManager::ClearSelectInfo()
{
	mpSelectInfoWindow->ClearInfo();

	if( mpSelectButtonWindow )
		mpSelectButtonWindow->ShowDelete( false );

	if( mpSelectButtonWindow )
		mpSelectButtonWindow->ShowDeleteCancel( false );
}


long cSelectUIManager::GetCurrentChannelNum()
{
	return mpChannelWindow->GetSelectedChannelParam();
}

void cSelectUIManager::SetSelectWindowFrame( unsigned char state, bool enable, bool deleteWait )
{
	mpSelectWindow->ShowFrame( state );
	mpSelectWindow->SetEnabled( enable );

	mpSelectButtonWindow->ShowButtons( true );
	mpSelectButtonWindow->SetEnableButtons( true );

	mpSelectButtonWindow->ShowDelete( false );
	mpSelectButtonWindow->ShowDeleteCancel( false );

	if( state == cSelectWindow::eShow_StartButton )
	{
		if( deleteWait == false )
			mpSelectButtonWindow->ShowDelete( true );
		else
			mpSelectButtonWindow->ShowDeleteCancel( true );
	}
}

void cSelectUIManager::SetEnableWindow( bool enable )
{
	if( mpSelectWindow )
	{
		mpSelectWindow->SetEnabled( enable );
	}

	if( mpSelectButtonWindow )
	{
		mpSelectButtonWindow->SetEnabled( enable );
	}

	if( mpChannelWindow )
	{
		mpChannelWindow->SetEnabled( enable );
	}
}
