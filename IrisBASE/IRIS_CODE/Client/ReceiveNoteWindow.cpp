#include "StdAfx.h"
#include "ReceiveNoteWindow.h"
#include "Editbox.h"
#include "Label.h"
#include "SendNoteWindow.h"

cReceiveNoteWindow::cReceiveNoteWindow()
: mpNameEdit(0)
, mpLabel(0)
{

}

cReceiveNoteWindow::~cReceiveNoteWindow()
{

}

/// 주의사항 : 
bool cReceiveNoteWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpNameEdit = (cEditBox*)GetChild( eUIID_GAME_RECEIVENOTE_IDEDIT );
	mpLabel = (cLabel*)GetChild( eUIID_GAME_RECEIVENOTE_LABEL );
	return true;
}

///
void cReceiveNoteWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_RECEIVENOTE_CLOSE_BUTTON:
	case eUIID_GAME_RECEIVENOTE_OK_BUTTON:
		{
			Hide();
		}
		break;
	case eUIID_GAME_RECEIVENOTE_REPLY_BUTTON:
		{
			/// 답장버튼
			cSendNoteWindow* pSend = (cSendNoteWindow*)UIMAN->GetContainer( eUIID_GAME_SENDNOTE_WINDOW );
			if( pSend && mpNameEdit )
			{
				pSend->SetName( mpNameEdit->GetText() );
				pSend->ShowTop();
			}
		}
		break;
	}
}

///
void cReceiveNoteWindow::SetName( LPCTSTR name )
{
	if( mpNameEdit )
		mpNameEdit->SetText( name );
}

///
void cReceiveNoteWindow::SetText( LPCTSTR text )
{
	if( mpLabel )
	{
		mpLabel->SetTextColor( eCOLOR_WHITE );
		mpLabel->SetText( text );
	}
}