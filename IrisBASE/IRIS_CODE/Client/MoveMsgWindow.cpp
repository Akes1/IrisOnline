#include "stdafx.h"
#include "MoveMsgWindow.h"
#include "FontAgent.h"
#include "UIImage.h"

cMoveMsgWindow::cMoveMsgWindow()
: mTextPosY(0) 
{
}

cMoveMsgWindow::~cMoveMsgWindow()
{
}

void cMoveMsgWindow::Close()
{
	cUIWindow::Close();
	Hide();
}

void cMoveMsgWindow::AddMoveMsg( LPCTSTR msg1, unsigned long color1, LPCTSTR msg2, unsigned long color2 )
{
	mMoveMsg[0].mMoveMsg.Clear();
	mMoveMsg[1].mMoveMsg.Clear();

	int msg1Len = _tcslen( msg1 );
	int msg2Len = _tcslen( msg2 );
	
	if( msg1Len < 1 || msg2Len < 1 )
	{
		assert(0);
		return;
	}

	int msg1Width = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, msg1, msg1Len );
	int msg2Width = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, msg2, msg2Len );
	mMoveMsg[0].mTextPosX = ( mAbsoluteRect.GetWidth() - ( msg1Width + msg2Width ) ) / 2;
	mMoveMsg[0].mMoveMsgColor = color1;
	mMoveMsg[0].mMoveMsg = msg1;
	mMoveMsg[1].mTextPosX = mMoveMsg[0].mTextPosX + msg1Width;
	mMoveMsg[1].mMoveMsgColor = color2;
	mMoveMsg[1].mMoveMsg = msg2;

	mTextPosY = ( mAbsoluteRect.GetHeight() - FONTAGENT->GetTextHeight( cFontAgent::eFont_UI ) ) / 2;

	Show();
}

void cMoveMsgWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}
	
	pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mMoveMsg[0].mMoveMsg.Cstr()), mAbsoluteRect.mLeft + mMoveMsg[0].mTextPosX, mAbsoluteRect.mTop + mTextPosY, mMoveMsg[0].mMoveMsgColor );
	pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mMoveMsg[1].mMoveMsg.Cstr()), mAbsoluteRect.mLeft + mMoveMsg[1].mTextPosX, mAbsoluteRect.mTop + mTextPosY, mMoveMsg[1].mMoveMsgColor );

	pKeeper->DrawAll();
}