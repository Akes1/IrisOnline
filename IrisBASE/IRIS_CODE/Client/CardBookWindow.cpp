#include "StdAfx.h"
#include "CardBookWindow.h"
#include "TarotSheetWindow.h"

cCardBookWindow::cCardBookWindow()
: mPage(ePAGE_DATA)
, mTarotSheet(0)
{

}

cCardBookWindow::~cCardBookWindow()
{

}

void cCardBookWindow::Clear()
{
	if( mTarotSheet )
		mTarotSheet->Clear();
}

void cCardBookWindow::Open()
{
	if( mTarotSheet )
		mTarotSheet->Open();
}

bool cCardBookWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cTabWindow::OnCreate( pproperty ) == false )
		return false;

	mTarotSheet = (cTarotSheetWindow*)GetChild( eUIID_GAME_CARDBOOK_TAROTSHEET );

	/// Å¸·Î ÅÇ ¿ì¼±
	SetCurrent( ePAGE_TAROT );
	return true;
}

void cCardBookWindow::OnRender( cUIFontItemKeeper* pkeeper )
{
	cTabWindow::OnRender( pkeeper );
}

void cCardBookWindow::OnCommand( cUINode*, unsigned int id )
{
	if( id == eUIID_GAME_CARDBOOK_CLOSE )
	{
		Hide();
	}
}

void cCardBookWindow::OnTabButtonPressed( cUINode* caller, unsigned int id )
{
	cTabWindow::OnTabButtonPressed( caller, id );
}

void cCardBookWindow::SetCurrent( unsigned int index )
{
	cTabWindow::SetCurrent( index );
	mPage = (eBookPage)index;
}
