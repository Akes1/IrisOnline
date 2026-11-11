#include "stdafx.h"
#include "MakeUIManager.h"
#include "MakeWindow.h"

//#include "Player_Common.h"
//#include "GameResourceManager.h"

cMakeUIManager::cMakeUIManager()
: mpMakeWindow(0)
, mpInfoWindow(0)
, mpRotateWindow(0)
{
}

cMakeUIManager::~cMakeUIManager()
{
}

void cMakeUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Make );

	/// 캐릭터 생성 윈도우 
	mpMakeWindow = new cMakeWindow;
	mpMakeWindow->CreateByPropertyName( "P_MakeWindow", 0 );

	mpInfoWindow = new cMakeInfoWindow;
	mpInfoWindow->CreateByPropertyName( "P_MakeInfoWindow", 0 );
	mpMakeWindow->SetMakeInfoWindow( mpInfoWindow );

	mpRotateWindow = new cMakeRotateWindow;
	mpRotateWindow->CreateByPropertyName( "P_MakeRotateWindow", 0 );

	mpGradeWindow = new cUIWindow;
	mpGradeWindow->CreateByPropertyName( "P_MakeGradeWindow", 0 );

}

void cMakeUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Make );
	
	UIMAN->Open();
}

void cMakeUIManager::Close()
{
	UIMAN->SetCurrentStage( eStage_Make );

	UIMAN->Close();
}

void cMakeUIManager::Process( unsigned long /*accumtime*/ )
{

}

void cMakeUIManager::GetDisplayInfo( sPlayerInfo* info )
{
	mpMakeWindow->GetDisplayInfo( info );
}

unsigned char cMakeUIManager::CheckDisplayUpdate()
{
	unsigned char state = mpMakeWindow->CheckDisplayUpdate();

	if( state == eChange_None )
		return eChange_None;

	if( state == eChange_Object )
		return eChange_Object;

	return eChange_Form;
}

void cMakeUIManager::SetEnableWindow( bool enable )
{
	if( mpMakeWindow )
		mpMakeWindow->SetEnabled( enable );

	if( mpInfoWindow )
		mpInfoWindow->SetEnabled( enable );

	if( mpRotateWindow )
		mpRotateWindow->SetEnabled( enable );
}

void cMakeUIManager::SetMakeInfoWindow( unsigned char race, unsigned char gender, unsigned long job )
{
	race;
	gender;
	job;
/*
	int textIdx = 0;
	switch( race )
	{
	case eRACE_HUMAN:	textIdx =  1000; break;
	case eRACE_BEAST:	textIdx =  1001; break;
	case eRACE_ELF:		textIdx =  1002; break;
	default:	assert( 0 );			return;
	}
	mpInfoWindow->SetRace( GAMERESOURCEMAN->GetGameText( textIdx ) );

	switch( gender )
	{
	case eGENDER_MALE:		textIdx =  1005;	break;
	case eGENDER_FEMALE:	textIdx =  1006;	break;
	default: assert(0);						return;
	}

	mpInfoWindow->SetGender( GAMERESOURCEMAN->GetGameText( textIdx ) );

	switch( job )
	{
	case ePLAYER_FIGHTER:	textIdx = 1010; break;
	case ePLAYER_MAGE:		textIdx = 1015; break;
	case ePLAYER_RANGER:	textIdx = 1020; break;
	case ePLAYER_SHAMAN:	textIdx = 1025; break;
	case ePLAYER_WARRIOR:	textIdx = 1030; break;
	case ePLAYER_THIEF:		textIdx = 1035; break;
	default:				assert( 0 );   return;
	}
	mpInfoWindow->SetJob( GAMERESOURCEMAN->GetGameText( textIdx ) );
*/
}
