#include "StdAfx.h"
#include "simpletipwindow.h"

#include "PlaneObject.h"
#include "FontAgent.h"

cSimpleTipWindow::cSimpleTipWindow()
{
	SetExceptSnap( true );
}

cSimpleTipWindow::~cSimpleTipWindow()
{
}


//void cSimpleTipWindow::Show( bool onsound )
//{
//	cTipWindow::Show( onsound );
//}
//
//void cSimpleTipWindow::Hide( bool onsound )
//{
//	cTipWindow::Hide( onsound );
//
//	mTimerEnabled = false;
//}

void cSimpleTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	if( mVisible == false )
		return;

	/// 배경
	if( mMiddleBack )
		mMiddleBack->Draw();

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력 
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;

		/// 다음줄 출력 
		unsigned int color = mToolTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );

		py += mLineHeight;
	}

	px = mX + GAP_SIZE + mMaxWidth;
	py = mY;

	///
	fontKeeper->DrawAll();
}

bool cSimpleTipWindow::SetSimpleTip( const cUIPos& pos, cStringT str )
{
	Clear();

	AddGap(1);

//	cStringT temp;
//	temp.Format( str.Cstr() );
	SetText( str.Cstr(), COLOR_WHITE );

	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 최대 너비 제한 설정
	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;

	/// 세팅
	mX = pos.mX + 25;
	mY = pos.mY + 25;

	unsigned int w = mMaxWidth + GAP_SIZE * 2; 
	unsigned int h = mIndex * mLineHeight + (mGapCount+1)*GAP_SIZE;

	/// 화면 안벗어나게 조정
	if( mX + w > GetScreenWidth() )
	{	
		mX -= ((mX + w) - GetScreenWidth());
	}

	if( mY + h > GetScreenHeight() )
	{
		mY -= ((mY + h) - GetScreenHeight());
	}

	if( mMiddleBack )
		mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );

	SetRelativeRect( cUIRect(mX, mY, mX+w, mY+h) );

	UpdateEdge();
	return true;
}

///----------------------------------------------------------------
cQuestTipWindow::cQuestTipWindow()
{
	SetExceptSnap( true );
}

cQuestTipWindow::~cQuestTipWindow()
{
}

void cQuestTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	if( mVisible == false )
		return;

	/// 배경
	if( mMiddleBack )
		mMiddleBack->Draw();

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력 
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;

		/// 다음줄 출력 
		unsigned int color = mToolTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );

		py += mLineHeight;
	}

	px = mX + GAP_SIZE + mMaxWidth;
	py = mY;

	///
	fontKeeper->DrawAll();
}

void cQuestTipWindow::OnHide()
{
	cTipWindow::OnHide();

	Clear();
}

bool cQuestTipWindow::AddQuestInfo( LPCTSTR str, unsigned long color, bool addgap )
{
	if( addgap == true )
	{
		AddGap(1);
	}

	cStringT temp;
	temp.Format( str );
	SetText( temp.Cstr(), color );
	return true;
}

bool cQuestTipWindow::ShowQuestInfo( const cUIPos& pos )
{
	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 최대 너비 제한 설정
	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;

	/// 세팅
	mX = pos.mX + 25;
	mY = pos.mY + 25;

	unsigned int w = mMaxWidth + GAP_SIZE * 2; 
	unsigned int h = mIndex * mLineHeight + (mGapCount+1)*GAP_SIZE;

	/// 화면 안벗어나게 조정
	if( mX + w > GetScreenWidth() )
	{	
		mX -= ((mX + w) - GetScreenWidth());
	}

	if( mY + h > GetScreenHeight() )
	{
		mY -= ((mY + h) - GetScreenHeight());
	}

	if( mMiddleBack )
		mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );

	SetRelativeRect( cUIRect(mX, mY, mX+w, mY+h) );

	UpdateEdge();
	return true;
}
