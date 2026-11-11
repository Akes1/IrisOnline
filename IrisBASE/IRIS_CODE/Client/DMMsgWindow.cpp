#include "StdAfx.h"
#include "DMMsgWindow.h"
#include "Label.h"
#include "GameResourceManager.h"
#include "PVPManager.h"

cDMMsgWindow::cDMMsgWindow()
: mpJoin(0)
, mpCancel(0)
, mpCaption(0)
, mpText(0)
, mpCountCancel(0)
{
}

cDMMsgWindow::~cDMMsgWindow()
{
}

void cDMMsgWindow::Clear()
{
	if( mpJoin )
		mpJoin->Hide();
	if( mpCancel )
		mpCancel->Hide();
	if( mpCountCancel )
		mpCountCancel->Hide();

	if( mpCaption )
		mpCaption->Clear();

	if( mpText )
		mpText->Clear();
}

void cDMMsgWindow::Show( bool onsound )
{
	Clear();

	//
	ePVPState state = PVPMAN->GetPVPState();
	switch( state )
	{
	case ePVPState_Notice:
		{
			if( mpJoin )
				mpJoin->Show();
			if( mpCancel )
				mpCancel->Show();
		}
		break;
	case ePVPState_Count:
		{
			if( mpCountCancel )
				mpCountCancel->Show();
		}
		break;
	default:
		return;
	}

	unsigned long captionIndex = 301;
	unsigned short type = PVPMAN->GetPVPType();
	switch( type )
	{
	case MAP_DEATHMATCH:		captionIndex = 859;	break;
	case MAP_FLAG:				captionIndex = 860;	break;
	default:					assert(0);		break;
	}

	// 전장명 세텅
	if( mpCaption )
		mpCaption->SetText( GAMERESOURCEMAN->GetGameText( captionIndex ) );

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );

	cUIWindow::Show( onsound );
}

bool cDMMsgWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpCaption = (cLabel*)GetChild( eUIID_GAME_DMMSG_CAPTION );
	mpText = (cLabel*)GetChild( eUIID_GAME_DMMSG_TEXT );
	mpJoin = GetChild( eUIID_GAME_DMMSG_JOIN );
	mpCancel = GetChild( eUIID_GAME_DMMSG_CANCEL );
	mpCountCancel = GetChild( eUIID_GAME_DMMSG_COUNTCANCEL );

	if( mpCaption )
		mpCaption->SetTextColor( COLOR_WHITE );

	if( mpText )
		mpText->SetTextColor( COLOR_WHITE );

	return true;
}

void cDMMsgWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );
}

void cDMMsgWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cUIWindow::OnProcess( deltaTime, accumTime );

	cStringT str;
	unsigned long leftTime = PVPMAN->GetWaitLeftTime() / 1000;
	unsigned int m = leftTime / 60;
	unsigned int s = leftTime % 60;

	if( PVPMAN->GetPVPState() == ePVPState_Notice )
	{
		unsigned char min = PVPMAN->GetMinLevel();
		unsigned char max = PVPMAN->GetMaxLevel();
		str.Format( GAMERESOURCEMAN->GetGameText( 820 ), min, max, m, s );
	}
	else if( PVPMAN->GetPVPState() == ePVPState_Count )
	{
		str.Format( GAMERESOURCEMAN->GetGameText( 846 ), s );
	}
	else
		return;

	if( mpText )
		mpText->SetText( (LPCTSTR)str.Cstr() );
}

void cDMMsgWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_DMMSG_JOIN:
		if( PVPMAN->GetPVPState() == ePVPState_Notice )
		{
			/// 데스매치 초청
			MSGROOT msg;
			msg.Category = NM_PVP;
			msg.Protocol = NM_PVP_READY_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			Hide();
		}
		else
			assert(0);
		break;
	case eUIID_GAME_DMMSG_CANCEL:
		if( PVPMAN->GetPVPState() == ePVPState_Notice )
		{
			Hide();
		}
		else
			assert(0);
		break;
	case eUIID_GAME_DMMSG_COUNTCANCEL:
		if( PVPMAN->GetPVPState() == ePVPState_Count )
		{
			// 입장 대기 취소
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_PVP_READY_CANCEL, eSKIN_YESNO,
								GAMERESOURCEMAN->GetGameText( 858 ),
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
		}
		else
			assert(0);
		break;
	default:
		assert(0);
		return;
	}
	
}
