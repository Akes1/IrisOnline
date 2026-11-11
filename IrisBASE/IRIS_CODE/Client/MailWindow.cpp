#include "stdafx.h"
#include "MailWindow.h"
#include "MailSheet.h"
#include "ObjectManager.h"
#include "GameUIManager.h"
#include "Hero.h"
#include "Npc_Common.h"
#include "Protocol.h"

cMailWindow::cMailWindow()
: mpListSheet(0)
, mpReadSheet(0)
, mpSendSheet(0)
{

}

cMailWindow::~cMailWindow()
{

}

void cMailWindow::Clear()
{
	if( mpReadSheet )
		mpReadSheet->Clear();

	if( mpSendSheet )
		mpSendSheet->Clear();
}

void cMailWindow::Open()
{
}

void cMailWindow::Close()
{
	cUIWindow::Close();

	if( HERO->GetNpcFunc() == NPC_FUNC_MAIL )
	{
		HERO->CloseNpcTalk( false );
	}
	
	Clear();
}

void cMailWindow::Show( unsigned long npcIndex )
{
	if( npcIndex != 0 )
	{
		HERO->OpenNpcTalk( npcIndex, false );
		HERO->SetNpcFunc( NPC_FUNC_MAIL );
	}

	cTabWindow::Show();
}

void cMailWindow::OnShow()
{
	cTabWindow::OnShow();
	
	SetCurrent( 0 );
}

void cMailWindow::OnHide()
{
	cTabWindow::OnHide();

	if( HERO->GetNpcFunc() == NPC_FUNC_MAIL )
	{
		HERO->CloseNpcTalk();
	}
	else if( HERO->GetNpcFunc() == NPC_FUNC_MAX && HERO->GetStopFlag() == eSTOP_SUMMON_PAPER )
	{
		HERO->CloseBySummonItem( NM_TRADE, NM_TRADE_POST_SUMMON_CLOSE_REQ );
	}
	
	Clear();
}

bool cMailWindow::OnCreate( cUINodeProperty* property )
{
	if( cTabWindow::OnCreate( property ) == false )
		return false;

	mpListSheet = (cMailListSheet*)GetChild( eUIID_MAIL_LISTSHEET );
	mpReadSheet = (cMailReadSheet*)GetChild( eUIID_MAIL_READSHEET );
	mpSendSheet = (cMailSendSheet*)GetChild( eUIID_MAIL_SENDSHEET );

	return true;
}

void cMailWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	cTabWindow::OnRender( pKeeper );
}


void cMailWindow::OnCommand( cUINode*, unsigned int id )
{
	if( id == eUIID_MAIL_CLOSE )
	{
		Hide();
	}
}

void cMailWindow::OnTabButtonPressed( cUINode* caller, unsigned int )
{
	if( caller->IsKindof( eUINODE_TABBUTTON ) == false )
	{
		assert(0);
		return;
	}

	if( (cUINode*)mCurrentButton == caller )
		return;

	for( unsigned int i = 0, iend = mTabButtonArray.GetSize(); i < iend; ++i )
	{
		cTabButton* button = (cTabButton*)mTabButtonArray[i];
		//cUIWindow* sheet = (cUIWindow*)mTabSheetArray[i];
		if( (cUINode*)button == caller )
		{
			/// 편지 쓰기 탭이면 우편목록 탭으로 자동 바꿔준다
			if( i == 2 )
			{
				SetCurrent( 0 );
			}

			SetCurrent( i );
			break;
		}
	}
}
