#include "StdAfx.h"
#include "NoteWindow.h"
#include "SendNoteWindow.h"
#include "ReceiveNoteWindow.h"
#include "StyleListBox.h"
#include "CheckBox.h"
#include "TabWindow.h"
#include "TabButton.h"
#include "ChatManager.h"
#include "UINodeData.h"
#include "GameUIManager.h"

#include "Protocol.h"
#include "Chat_Common.h"
#include "UIImage.h"
#include "Button.h"
#include "Radiobutton.h"
#include "GameResourceManager.h"

cNoteWindow::cNoteWindow()
: mpNoteList(0)
, mpDelButton(0)
{
	for( int i = 0; i < MAXNUM; ++i )
		mpRadioNode[i] = 0;

	mCheckRect.Set( 0, 0, 0, 0 );
}

cNoteWindow::~cNoteWindow()
{
}

///
void cNoteWindow::Clear()
{
	if( mpNoteList )
		mpNoteList->Clear();

	cStyleCheckList::cIterator i = mStyleCheckList.Begin();
	cStyleCheckList::cIterator end = mStyleCheckList.End();
	for( ; i != end; ++i )
	{
		sStyleCheckBox* p = (sStyleCheckBox*)(*i);
		if( p )
			SAFE_DELETE( p->mpImage );
		SAFE_DELETE( p );
	}
	mStyleCheckList.Clear();

	for( int i = 0 ; i < MAXNUM; ++i )
	{
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
}

/// 주의사항 : 
bool cNoteWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// chat 매니저에 등록 
	CHATMANAGER->SetNoteWindow( this );

	/// 리스트박스 가져오기
	mpNoteList = (cStyleListBox*)GetChild( eUIID_GAME_NOTE_LISTBOX );
	mpDelButton = (cButton*)GetChild( eUIID_GAME_NOTE_DELETE_BUTTON );

	if( !mpNoteList )
	{
		assert(0);
		return false;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 정렬영역
	cUINodeSkin* pRectSkin = pSkin->GetNodeSkin( "Note_ButtonRect" );
	if( pRectSkin )
	{
		mCheckRect.mLeft = pRectSkin->mSkinInfo->mX;
		mCheckRect.mTop = pRectSkin->mSkinInfo->mY;
		mCheckRect.mRight = mCheckRect.mLeft + pRectSkin->mSkinInfo->mWidth;
		mCheckRect.mBottom = mCheckRect.mTop + pRectSkin->mSkinInfo->mHeight;
	}
	
	for( int i = 0; i < MAXNUM; ++i )
	{
		mpRadioNode[i] = (cRadioButton*)GetChild( eUIID_GAME_NOTE_RADIOBUTTON1 + i );
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
	return true;
}

///
void cNoteWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_NOTE_CLOSE_BUTTON:
		{
			Hide();
		}
		break;
	case eUIID_GAME_NOTE_WRITE_BUTTON:
		{
			cSendNoteWindow* sendNote = GAMEUI->GetSendNoteWindow();
			if( sendNote && sendNote->IsVisible() == false )
				sendNote->ShowTop();
		}
		break;
	case eUIID_GAME_NOTE_DELETE_BUTTON:
		{
			///
			DeleteNote();	
		}
		break;
	}
}

///
void cNoteWindow::OnRadioPressed( cUINode*, unsigned int id )
{
	int row = 0;
	switch( id )
	{
	case eUIID_GAME_NOTE_RADIOBUTTON1:	row = 0; break;
	case eUIID_GAME_NOTE_RADIOBUTTON2:	row = 10; break;
	case eUIID_GAME_NOTE_RADIOBUTTON3:	row = 20; break;
	case eUIID_GAME_NOTE_RADIOBUTTON4:	row = 30; break;
	case eUIID_GAME_NOTE_RADIOBUTTON5:	row = 40; break;
	default: assert(0); return;
	}	

	if( mpNoteList )
		mpNoteList->ChangeTopIndex( row );
}

///
void cNoteWindow::OnListBoxSelected( cUINode*, unsigned int )
{
	if( !mpNoteList )
		return;

	cReceiveNoteWindow* recvNote = GAMEUI->GetReceiveNoteWindow();
	if( recvNote )
	{
		unsigned int selectIdx = mpNoteList->GetSelectIndex();

		/// 받은 이름, 내용 받아오기 
		sNoteList* data = (sNoteList*)mpNoteList->GetExtraData( selectIdx );
		if( data )
		{
			recvNote->SetName( data->CharacterName );
			recvNote->SetText( data->Message );
			recvNote->ShowTop();
		}
		else
		{
			assert(0);
		}
	}
}

void cNoteWindow::AddNote( sNoteList* data )
{
	if( !mpNoteList || !data ) 
	{
		assert(0);
		return;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	cUINodeSkin* pCheckSkin = pSkin->GetNodeSkin( "Note_Check" );
	cUINodeSkin* pNoCheckSkin = pSkin->GetNodeSkin( "Note_NoCheck" );

	cUIImage* image = 0;
	if( pCheckSkin && pNoCheckSkin )
	{
		image = new cUIImage( pNoCheckSkin->mpTexture );
		unsigned int tx = pNoCheckSkin->mSkinInfo->mTexX;
		unsigned int ty = pNoCheckSkin->mSkinInfo->mTexY;
		unsigned int x = pNoCheckSkin->mSkinInfo->mX;
		unsigned int y = pNoCheckSkin->mSkinInfo->mY;
		unsigned int w = pNoCheckSkin->mSkinInfo->mWidth;
		unsigned int h = pNoCheckSkin->mSkinInfo->mHeight;

		image->SetTextureRect( cUIRect(tx, ty, tx+w, ty+h ) );
		image->SetScreenRect( cUIRect(x, y, x+w, y+h ) );
	}

	///
	cStringT day;
	cStringT time;
	cStringT remain;

	if( mpNoteList->GetRowCount() < mpNoteList->GetMaxRowNum() )
	{
		day.Format( GAMERESOURCEMAN->GetGameText( 64 ), data->Month, data->Day );
		time.Format( GAMERESOURCEMAN->GetGameText( 65 ), data->Hour, data->Minute );

		/// 남은 일수 계산
		long remainDay = data->RemainTime / (86400*1000);
		remain.Format( GAMERESOURCEMAN->GetGameText( 66 ), remainDay );

		/// 쪽지 삽입 
		sStyleCheckBox* listData = new sStyleCheckBox;
		listData->mpCheckSkin = pCheckSkin;
		listData->mpNoCheckSkin = pNoCheckSkin;
		listData->mCheck = false;
		listData->mpImage = image;

		mStyleCheckList.PushBack( listData );

		int row = mpNoteList->AddRow( listData, data->CharacterName, mDefaultColor, data );
		if( row != -1 )
		{
			mpNoteList->AddColume( row, (LPCTSTR)day.Cstr(), mDefaultColor );
			mpNoteList->AddColume( row, (LPCTSTR)time.Cstr(), mDefaultColor );
			mpNoteList->AddColume( row, (LPCTSTR)remain.Cstr(), mDefaultColor );
		}
		else
			assert(0);
	}
	else
		assert(0);
}

///
void cNoteWindow::DeleteNote()
{
	if( !mpNoteList )
		return;

	/// 눌려있는 버튼 찾기 
	unsigned int page = 0;
	for( int i = 0; i < MAXNUM; ++i )
	{
		if( mpRadioNode[i]->IsPress() == true )
		{
			page = i;
			break;
		}
	}

	MSG_REQ_NOTE_DEL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_NOTE_DEL_REQ;
	msg.RowCount = 0;

	unsigned int maxCount = (page + 1) * 10;
	unsigned int rowCount = mpNoteList->GetRowCount();

	/// 페이지단위로 검사
	for( unsigned int i = page * 10; i < maxCount; ++i )
	{
		if( i >= rowCount )
			break;

		sRowData* rowData = mpNoteList->GetRowData( i );
		if( rowData )
		{
			sStyleCheckBox* checkData = (sStyleCheckBox*)rowData->mStyleData;
			sNoteList*		noteData  = (sNoteList*)rowData->mExtraData;
			if( checkData && checkData->mCheck )
			{
				if( noteData )
					msg.Idx[ msg.RowCount++ ] = noteData->Idx;
				else
					assert(0);
			}
		}
	}
	
	if ( msg.RowCount > 0 )
	{
		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength( ) );

		Clear();
		CHATMANAGER->DeleteNote();

		/// 락
		NetLock();
	}
}

void cNoteWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	UpdateButton();
}

void cNoteWindow::UpdateButton()
{
	/// 버튼 보이고 안보이고 하기
	int total = mpNoteList->GetRowCount() - 1;
	int page = ( total / 10 );

	for( int i = 0; i < MAXNUM; ++i )
	{
		if( mpRadioNode[i] )
		{
			if( i <= page )
				mpRadioNode[i]->Show();
			else
				mpRadioNode[i]->Hide();
		}
	}

	int w = mpRadioNode[0]->GetAbsoluteRect().GetWidth();	
	int x = mCheckRect.mLeft + (int)((mCheckRect.GetWidth() - (w*(page+1))) * 0.5f);
	for( int i = 0; i <= page; ++i )
	{
		cUIPos pos;
		pos.mX = x + (w*i);
		pos.mY = mpRadioNode[i]->GetRelativeRect().mTop;
		mpRadioNode[i]->SetRelativePos( pos );
	}

	if( mpRadioNode[0] )
		mpRadioNode[0]->SetPress( true );
	
}

void cNoteWindow::OnNetLock( int lockTry )
{
	if( mpDelButton )
		mpDelButton->NetLock( lockTry );
}

void cNoteWindow::OnNetUnLock( int lockTry )
{
	if( mpDelButton )
		mpDelButton->NetUnLock( lockTry );
}
