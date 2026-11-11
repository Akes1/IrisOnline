#include "stdafx.h"
#include "SkinDialog.h"
#include "SkinResource.h"
#include "SkinFunc.h"

BEGIN_MESSAGE_MAP( cSkinDialog, CDialog )
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

cSkinDialog::cSkinDialog( CWnd* parent )
: CDialog( IDD_DIALOG_TEMPLATE, parent )
, mResource( 0 )
, mWidth( 0 )
, mHeight( 0 )
, mOldBitmap( 0 )
{
	mRect.SetRectEmpty();
}

cSkinDialog::~cSkinDialog()
{
}

int cSkinDialog::OnCreate( LPCREATESTRUCT cs ) 
{
	if( CDialog::OnCreate( cs ) == -1 )
		return -1;

	/// 폰트 설정
	SetFont( mResource->mFont );

	/// 메모리 DC 초기화
	CClientDC dc( this );

	mMemDC.CreateCompatibleDC( &dc );
	mOldBitmap = (CBitmap*)mMemDC.SelectObject( &mResource->mBitmap );

	/// 모양 설정
	HRGN hrgn = CreateRegion( &mMemDC, mWidth, mHeight, RGB(255,0,0) );
	SetWindowRgn( hrgn, TRUE );

	/// 위치 설정
	mRect.SetRect( 0, 0, mWidth, mHeight );
	MoveWindow( &mRect );
	CenterWindow();

	return 0;
}

void cSkinDialog::OnDestroy() 
{
	/// 메모리 DC 제거
	if( mOldBitmap )
		mMemDC.SelectObject( mOldBitmap );

	mMemDC.DeleteDC();
    
	/// 폰트 제거
	mFont.DeleteObject();

	CDialog::OnDestroy();
}

void cSkinDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.TransparentBlt( 0, 0, mWidth, mHeight, &mMemDC, 0, 0, mWidth, mHeight, RGB(255,0,0) );

	CDialog::OnPaint();
}

BOOL cSkinDialog::OnEraseBkgnd( CDC* dc ) 
{
	return 1;
}

void cSkinDialog::OnLButtonDown( UINT flags, CPoint point ) 
{
	if( mRect.PtInRect( point ) )
	{
		SendMessage( WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM( point.x, point.y ) );
	}

	CDialog::OnLButtonDown( flags, point );
}

BOOL cSkinDialog::PreTranslateMessage( MSG* msg ) 
{
	if( msg->message == WM_KEYDOWN )
	{
		switch( msg->wParam )
		{
		case VK_ESCAPE:
			OnCancel();
			break;

		case VK_RETURN:
			OnOK();
			break;
		}
	}

	return CDialog::PreTranslateMessage( msg );
}

INT_PTR cSkinDialog::DoModal( cSkinContainerResource* resource )
{
	ASSERT( resource );
	SetResource( (cSkinDialogResource*)resource );
	return CDialog::DoModal();
}

int cSkinDialog::Create( cSkinContainerResource* resource )
{
	ASSERT( resource );
	SetResource( (cSkinDialogResource*)resource );
	return CDialog::Create( IDD_DIALOG_TEMPLATE );
}

void cSkinDialog::SetResource( cSkinDialogResource* p )
{
	ASSERT( p );
	mResource = p;

	mRect = p->mRect;
	mWidth = p->mWidth;
	mHeight = p->mHeight;
}

void cSkinDialog::SetFont( const LOGFONT& lf )
{
	mFont.DeleteObject();
	mFont.CreateFontIndirect( &lf );
	CWnd::SetFont( &mFont );
}

void cSkinDialog::SetFont( LPCTSTR faceName, int size, BYTE charset )
{
	LOGFONT lf;
	mFont.GetLogFont( &lf );
	lf.lfHeight = size;
	lf.lfCharSet = charset;
	strcpy( lf.lfFaceName, faceName );

	mFont.DeleteObject();
	mFont.CreateFontIndirect( &lf );
	CWnd::SetFont( &mFont );
}
