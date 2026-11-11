#include "stdafx.h"
#include "NpcDealWindow.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealSearchSheet.h"
#include "Label.h"
#include "Npc_Common.h"
#include "NumberEditBox.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "UIManager.h"
#include "GameUIManager.h"

cNpcDealWindow::cNpcDealWindow()
: mpRegistSheet(0)
, mpSearchSheet(0)
, mpHaveMoney(0)
{

}

cNpcDealWindow::~cNpcDealWindow()
{

}

void cNpcDealWindow::Open()
{
}

void cNpcDealWindow::Close()
{
	cUIWindow::Close();

	UIMAN->HideTip();

	if( HERO->GetNpcFunc() == NPC_FUNC_DEAL )
	{
		HERO->CloseNpcTalk( false );
	}

	if( mpSearchSheet )
		mpSearchSheet->Clear();

	if( mpRegistSheet )
		mpRegistSheet->Clear();
}

void cNpcDealWindow::OnShow()
{
	cTabWindow::OnShow();

	SetCurrent( 0 );

	UpdateHaveMoney();
}

void cNpcDealWindow::OnHide()
{
	cTabWindow::OnHide();

	UIMAN->HideTip();

	if( HERO->GetNpcFunc() == NPC_FUNC_DEAL )
	{
		HERO->CloseNpcTalk();
	}
	else if( HERO->GetNpcFunc() == NPC_FUNC_MAX && HERO->GetStopFlag() == eSTOP_SUMMON_PAPER )
	{	
		HERO->CloseBySummonItem( NM_TRADE, NM_TRADE_AGENT_SUMMON_CLOSE_REQ );
	}

	//
	if( mpSearchSheet )
		mpSearchSheet->Clear();

	if( mpRegistSheet )
	{
		mpRegistSheet->Clear();
		mpRegistSheet->DeleteRegistItem();
	}
}

bool cNpcDealWindow::OnCreate( cUINodeProperty* property )
{
	if( cTabWindow::OnCreate( property ) == false )
		return false;

	mpRegistSheet	= (cNpcDealRegistSheet*)GetChild( eUIID_NPCDEAL_REGISTSHEET );
	mpSearchSheet	= (cNpcDealSearchSheet*)GetChild( eUIID_NPCDEAL_SEARCHSHEET );
	mpHaveMoney		= (cLabel*)GetChild( eUIID_NPCDEAL_HAVEMONEY );

	return true;
}

void cNpcDealWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_NPCDEAL_CLOSE:
	case eUIID_NPCDEAL_CLOSE2:
		Hide();
		break;
	}
}

void cNpcDealWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	cTabWindow::OnRender( pKeeper );
}


void cNpcDealWindow::UpdateHaveMoney()
{
	if( mpHaveMoney )
	{
		unsigned long money = HERO->GetMoney();
		LPCTSTR str = cNumberEditBox::FormatNumber( money );

		mpHaveMoney->SetText( str );
	}
}

void cNpcDealWindow::OnNetLock( int lockTry )
{
	if( mpRegistSheet )
		mpRegistSheet->NetLock( lockTry );

	if( mpSearchSheet )
		mpSearchSheet->NetLock( lockTry );
}

void cNpcDealWindow::OnNetUnLock( int lockTry )
{
	if( mpRegistSheet )
		mpRegistSheet->NetUnLock( lockTry );

	if( mpSearchSheet )
		mpSearchSheet->NetUnLock( lockTry );
}
