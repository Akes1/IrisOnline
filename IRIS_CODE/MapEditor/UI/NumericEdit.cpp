#include "stdafx.h"
#include "NumericEdit.h"

IMPLEMENT_DYNAMIC( cNumEdit, CEdit )

BEGIN_MESSAGE_MAP( cNumEdit, CEdit )
	ON_WM_KEYDOWN()
	ON_WM_CHAR()

//	ON_CONTROL_REFLECT( EN_CHANGE, OnEnChange )
//	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

cNumEdit::cNumEdit()
: mParent( 0 )
{
	//m_fOverFlow = FALSE;
	//m_brError.CreateSolidBrush( RGB( 255,128,128 ) );
}

BOOL cNumEdit::Create( DWORD style, const RECT& rect, CWnd* parent, UINT id )
{
	mParent = (cNumericEdit*)parent;

	return CEdit::Create( style, rect, parent, id );
}

BOOL cNumEdit::PreTranslateMessage(MSG* pMsg)
{
	if( (GetStyle() & ES_READONLY) == 0 )
	{
		if (pMsg->wParam == VK_RETURN && pMsg->message== WM_KEYDOWN )
		{
			CString str;
			GetWindowText(str);

			if( mParent )
				mParent->SetValueNotify( (float)_tstof(str) );
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void cNumEdit::OnKeyDown( UINT c, UINT repeats, UINT flags )
{
	if( c == VK_UP || c == VK_DOWN )
		GetParent()->SendMessage( WM_KEYDOWN, (WPARAM)c, MAKELPARAM( repeats, flags ) );
	else
		CEdit::OnKeyDown( c, repeats, flags );
}

void cNumEdit::OnChar( UINT c, UINT repeats, UINT flags )
{
	if( !_istdigit( c ) && ( c != 8 ) & ( c != _T( '-' ) ) )
	{
		MessageBeep( 0 );
		return;
	}

	CEdit::OnChar( c, repeats, flags );
}

//
//void cNumEdit::OnEnChange()
//{
//	char str[256];
//	float min, max;
//
//	GetWindowText( str, 256 );
//	int value = _tstoi( str );
//
//
//	mParent->GetRange( &min, &max );
//	//GetParent()->SendMessage( NEM_GETRANGE,(WPARAM) &min,(LPARAM) &max );
//
//	BOOL fNewOverFlow = ( value < min ) || ( value > max );
//	if( fNewOverFlow != m_fOverFlow )
//	{
//		m_fOverFlow = fNewOverFlow;
//		Invalidate();
//	}
//
//	mParent->SetValue( value );
//	//GetParent()->SendMessage( NEM_SETPOS,0,(LPARAM) value );
//}


//HBRUSH cNumEdit::CtlColor( CDC* dc, UINT /*nCtlColor*/ )
//{
//	dc->SetBkMode( TRANSPARENT );
//
////	if( m_fOverFlow )
//		return m_brError;
////	else
////		return NULL;
//}

IMPLEMENT_DYNAMIC(cNumSpin, CSpinButtonCtrl)

BEGIN_MESSAGE_MAP(cNumSpin, CSpinButtonCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
END_MESSAGE_MAP()

cNumSpin::cNumSpin()
: mParent( 0 )
, mStep( 1.0f )
{
}

BOOL cNumSpin::Create( DWORD style, const RECT& rect, CWnd* parent, UINT id )
{
	mParent = (cNumericEdit*)parent;

	return CSpinButtonCtrl::Create( style, rect, parent, id );
}

void cNumSpin::SetStep( float step )
{
	mStep = step;
}

void cNumSpin::OnDeltapos( NMHDR* pnmhdr, LRESULT* presult )
{
	NM_UPDOWN* updown = (NM_UPDOWN*)pnmhdr;

	mParent->SetDeltaNotify( (float)updown->iDelta * mStep );
	*presult = 0;
}

IMPLEMENT_DYNAMIC( cNumEditButton, CButton )

BEGIN_MESSAGE_MAP( cNumEditButton, CButton )
	ON_MESSAGE( BM_SETSTYLE,OnBnSetStyle )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

cNumEditButton::cNumEditButton()
: mParent( 0 )
{
}

BOOL cNumEditButton::Create( LPCTSTR caption, DWORD style, const RECT& rect, CWnd* parent, UINT id )
{
	assert( parent && "null parent" );
	mParent = (cNumericEdit*)parent;

	return CButton::Create( caption, style, rect, parent, id );
}

LRESULT cNumEditButton::OnBnSetStyle( WPARAM wparam,LPARAM lparam )
{
	return DefWindowProc( BM_SETSTYLE, ( wparam & ~BS_DEFPUSHBUTTON ), lparam );
}

void cNumEditButton::OnLButtonDown( UINT flags, CPoint point )
{
	CButton::OnLButtonDown( flags, point );
	mParent->ShowPopup();
	SetState( TRUE );
}

IMPLEMENT_DYNAMIC( cNumEditPopup, CWnd )

BEGIN_MESSAGE_MAP( cNumEditPopup, CWnd )
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

cNumEditPopup::cNumEditPopup()
: mParent( 0 )
, mDragY( 0 )
, mStep( 1.0f )
{
}

BOOL cNumEditPopup::Create( cNumericEdit* parent, float step )
{
	assert( parent && "null parent" );
	mParent = parent;

	CString className;
	className = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,::LoadCursor( NULL, IDC_SIZEWE ),( HBRUSH )0 );

	if( CreateEx( 0,
		className,
		0,
		WS_POPUP|WS_DLGFRAME,
		0, 0, 0, 0,
		parent->GetSafeHwnd(),
		0 ) == FALSE )
	{
		assert( 0 && "Failed to create CNumericEdit popup" );
		return FALSE;
	}

	SetCapture();

	POINT point;
	::GetCursorPos( &point );
	mDragY = point.y;
	mStep = step;
	return TRUE;
}

void cNumEditPopup::OnMouseMove( UINT /*flags*/, CPoint point )
{
	ClientToScreen( &point );
	int dy = mDragY - point.y;
	mDragY = point.y;

	if( dy != 0 )
	{
		mParent->SetDeltaNotify( (float)dy * mStep );
	}
}

void cNumEditPopup::OnLButtonUp( UINT flags, CPoint point )
{
	ReleaseCapture();

	CWnd::OnLButtonUp( flags, point );
	mParent->HidePopup();

	DestroyWindow();
	delete this;
}

IMPLEMENT_DYNAMIC( cNumericEdit, CWnd )

BEGIN_MESSAGE_MAP( cNumericEdit, CWnd )
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

cNumericEdit::cNumericEdit()
{
	mID = 0;
	mMin = 0.0f;
	mMax = 100.0f;
	mValue = 0.0f;
	mDelta = 0.0f;
	mStep = 1.0f;
	mEnabled = true;
}

BOOL cNumericEdit::Create( float min, float max, float value, float step, CWnd* parent, UINT id, DWORD style )
{
	assert( parent && "null parent" );

	CWnd* wnd = parent->GetDlgItem(id);
	assert( wnd && "null dialog item" );

	/// 범위와 값을 설정
	mMin = min;
	mMax = max;
	mValue = value;
	mStep = step;
	mID = id;

	mSpin.SetStep( step );

	/// 윈도우 영역을 얻음
	CRect rect;
	wnd->GetWindowRect( &rect );
	parent->ScreenToClient( &rect );

	// 윈도우를 생성
	CString className;
	className = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,::LoadCursor( NULL, IDC_ARROW ),( HBRUSH )0 );

	if( CreateEx(
		WS_EX_CLIENTEDGE,
		className,
		0,
		WS_CHILD | style,
		rect.left, rect.top, rect.Width(), rect.Height(),
		parent->GetSafeHwnd(),
		( HMENU )0 ) == FALSE )
	{
		assert( 0 && "failed to create numeric edit" );
		return FALSE;
	}
	return TRUE;
}

void cNumericEdit::ShowPopup()
{
	cNumEditPopup* popup = new cNumEditPopup;
	popup->Create( this, mStep );
}

void cNumericEdit::HidePopup()
{
	mButton.SetState( FALSE );
	mEdit.SetFocus();
}

void cNumericEdit::SetDelta( float delta )
{
	SetValue( mValue + delta );
}

void cNumericEdit::SetDeltaNotify( float delta )
{
	SetValueNotify( mValue + delta );
}

void cNumericEdit::SetValue( float value )
{
	if( value < mMin )
		value = mMin;
	if( value > mMax )
		value = mMax;

	if( value != mValue )
	{
		cString str;
		str.Format( "%.7f", value );
		mEdit.SetWindowText( str.Cstr() );
		//int len = str.GetLength();
		//mEdit.SetSel( len, len );
		mSpin.SetPos( (int)value );

		mDelta = value - mValue;
		mValue = value;
	}
}

void cNumericEdit::SetValueNotify( float value )
{
	SetValue( value );
	GetParent()->SendMessage( NEN_CHANGED, mID, 0 );
}

void cNumericEdit::SetReadOnly( bool readOnly )
{
	mEdit.SetReadOnly( readOnly );
}

void cNumericEdit::SetRange( float min, float max )
{
	if( min != mMin || max != mMax )
	{
		mMin = min;
		mMax = max;

		if( mValue < mMin )
			mValue = mMin;
		if( mValue > mMax )
			mValue = mMax;

		cString str;
		str.Format( "%.7f", mValue );

		mEdit.SetWindowText( str.Cstr() );
		mSpin.SetRange32( (int)mMin, (int)mMax );
		mSpin.SetPos( (int)mValue );
	}
}

void cNumericEdit::SetEnabled( bool enabled )
{
	if( mEnabled != enabled )
	{
		mEnabled = enabled;

		EnableWindow( enabled );
		mEdit.EnableWindow( enabled );
		mSpin.EnableWindow( enabled );
		mButton.EnableWindow( enabled );
	}
}

void cNumericEdit::ResizeChildren( int cx,int cy )
{
	int w = GetSystemMetrics( SM_CXHSCROLL );
	int x = cx;

	x -= w;
	mButton.MoveWindow( x, 0, w, cy );

	x -= w;
	mSpin.MoveWindow( x, 0, w, cy );

	mEdit.MoveWindow( 0, 1, x, cy-2 );
}

int cNumericEdit::OnCreate( LPCREATESTRUCT cs )
{
	if( CWnd::OnCreate( cs ) == -1 )
		return -1;

	CRect r( 0,0,0,0 );
	mButton.Create( _T( "↕" ), WS_CHILD | WS_VISIBLE, r, this, 1 );
	mEdit.Create( WS_CHILD | WS_VISIBLE, r, this, 2 );
	mEdit.SetReadOnly( TRUE );
	mSpin.Create( WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS, r, this, 3 );

	if( mValue < mMin )
		mValue = mMin;
	if( mValue > mMax )
		mValue = mMax;

	cString str;
	str.Format( "%.7f", mValue );

	mEdit.SetWindowText( str.Cstr() );
	mSpin.SetRange32( (int)mMin, (int)mMax );
	mSpin.SetPos( (int)mValue );
	return 0;
}

void cNumericEdit::OnSize( UINT type, int cx, int cy )
{
	CWnd::OnSize( type, cx, cy );

	ResizeChildren( cx,cy );
}

//void cNumericEdit::OnVScroll( UINT sbcode, UINT pos, CScrollBar* pscrollBar )
//{
//	SetValue( (float)pos );
//
//	CWnd::OnVScroll( sbcode, pos, pscrollBar );
//}

void cNumericEdit::OnSetFocus( CWnd* poldWnd )
{
	CWnd::OnSetFocus( poldWnd );

	mEdit.SetFocus();
}

void cNumericEdit::OnKeyDown( UINT c, UINT repeats, UINT flags )
{
	if( c == VK_UP && mValue < mMax )
		SetValueNotify( mValue + 1.0f );
	else if( c == VK_DOWN && mValue > mMin )
		SetValueNotify( mValue - 1.0f );
	else
		CWnd::OnKeyDown( c, repeats, flags );
}
