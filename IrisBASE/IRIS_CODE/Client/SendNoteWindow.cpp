#include "StdAfx.h"
#include "SendNoteWindow.h"
#include "Editbox.h"
#include "MultiEditBox.h"
#include "Chat_Common.h"
#include "Protocol.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "Button.h"

cSendNoteWindow::cSendNoteWindow()
: mpNameEdit(0)
, mpMultiEdit(0)
, mpSendButton(0)
{

}

cSendNoteWindow::~cSendNoteWindow()
{

}

/// 주의사항 : 
bool cSendNoteWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpNameEdit = (cEditBox*)GetChild( eUIID_GAME_SENDNOTE_IDEDIT );
	mpMultiEdit = (cMultiEditBox*)GetChild( eUIID_GAME_SENDNOTE_MULTIEDITBOX );
	mpSendButton = (cButton*)GetChild( eUIID_GAME_SENDNOTE_SEND_BUTTON );
	return true;
}

///
void cSendNoteWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_SENDNOTE_CLOSE_BUTTON:
	case eUIID_GAME_SENDNOTE_CANCEL_BUTTON:
		{
			Hide();
		}
		break;
	case eUIID_GAME_SENDNOTE_SEND_BUTTON:
		{			
			/// 보내기
			if( CheckNote() == true )
			{
				SendNote();
				Hide();
			}
		}
		break;
	}
}

///
bool cSendNoteWindow::CheckNote()
{
	if( !mpNameEdit || !mpMultiEdit )
		return false;

	cStringT Message;

	/// 받을아이디와, 내용이 없으면 리턴
	if( mpNameEdit->GetLength() == 0 || mpMultiEdit->GetLength() == 0 )
		return false;

	/// 자기자신과 이름이 동일하면 리턴 
	int result = ::_tcscmp( HERO->GetName(), mpNameEdit->GetText() );
	if( result == 0 )
	{
		Message.Format( GAMERESOURCEMAN->GetGameText( 35 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		return false;
	}

	/// 보낼내용이 없으면.
	if( mpMultiEdit->GetLength() <= 0 )
	{
		Message.Format( GAMERESOURCEMAN->GetGameText( 36 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		return false;
	}
	else
	{
		/// 내용에 " "를 제외한 문자가 입력되었는지 검사후 send
		bool search = false;
		LPCTSTR str = mpMultiEdit->GetText();
		for( int i = 0;i < mpMultiEdit->GetLength(); ++i )
		{
			if( *str != 32 )
			{
				search = true;
				break;
			}
			else
				str++;
		}

		if( search == false )
		{
			Message.Format( GAMERESOURCEMAN->GetGameText( 36 ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
			return false;
		}
	}
	return true;
}

void cSendNoteWindow::SendNote()
{
	/// 서버에 쪽지 보내기 
	MSG_REQ_NOTE_SEND	msg;
	::memset( &msg, 0, sizeof( msg ) );
	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_NOTE_SEND_REQ;
	Sstrncpy( msg.Name, MAX_NAME_BUFFER_SIZE, mpNameEdit->GetText(), MAX_NAME_SIZE );
	Sstrncpy( msg.Message, MAX_NOTE_MESSAGE_BUFFER_SIZE, mpMultiEdit->GetText(), MAX_NOTE_MESSAGE_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );

	/// 락 걸기
	NetLock();
}

///
void cSendNoteWindow::SetName( LPCTSTR name )
{
	if( mpNameEdit )
		mpNameEdit->SetText( name );
}

void cSendNoteWindow::OnShow()
{
	if( mpNameEdit )
		mpNameEdit->SetFocus();

	cUIWindow::OnShow();
}

///
void cSendNoteWindow::OnHide()
{
	cUIWindow::OnHide();

	///
	if( mpNameEdit )
	{
		mpNameEdit->Clear();
		mpNameEdit->ReleaseFocus();
	}

	if( mpMultiEdit )
	{
		mpMultiEdit->Clear();
		mpMultiEdit->ReleaseFocus();
	}
}

void cSendNoteWindow::OnNetLock( int lockTry )
{
	if( mpSendButton )
		mpSendButton->NetLock( lockTry );
}

void cSendNoteWindow::OnNetUnLock( int lockTry )
{
	if( mpSendButton )
		mpSendButton->NetUnLock( lockTry );
}

