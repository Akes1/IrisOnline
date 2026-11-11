#include "stdafx.h"
#include "SkinRoundGauge.h"
#include "SkinResource.h"
#include "SkinFunc.h"
#include "BuffDC.h"

BEGIN_MESSAGE_MAP( cSkinRoundGauge, CStatic )
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

cSkinRoundGauge::cSkinRoundGauge()
: mResource( 0 )
, mWidth( 0 )
, mHeight( 0 )
, mAngle( 0.0f )
{
}

cSkinRoundGauge::~cSkinRoundGauge()
{
}

BOOL cSkinRoundGauge::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	/// 리소스 설정
	ASSERT( resource );
	mResource = (cSkinRoundGaugeResource*)resource;

	mWidth = mResource->mWidth;
	mHeight = mResource->mHeight;

	/// 생성
	DWORD style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
	CRect rc( mResource->mRect );
	rc.MoveToXY( rc.left - mWidth/2, rc.top - mHeight/2 );
	return CStatic::Create( 0, style, rc, parent, id );
}

int cSkinRoundGauge::OnCreate( LPCREATESTRUCT cs )
{
	CClientDC dc( this );
	mMemBitmap.CreateCompatibleBitmap( &dc, mWidth, mHeight );

	return CStatic::OnCreate( cs );
}

BOOL cSkinRoundGauge::OnEraseBkgnd( CDC* dc ) 
{
	return 1;
}

void cSkinRoundGauge::OnPaint() 
{
	CPaintDC dc( this );

	CDC backDC;
	backDC.CreateCompatibleDC( &dc );
	CBitmap* oldBmp1 = backDC.SelectObject( &mMemBitmap );

	/// 배경 이미지
	{
		CDC memDC;
		memDC.CreateCompatibleDC( &dc );
		CBitmap* oldBmp2 = memDC.SelectObject( &mResource->mBitmap );

		backDC.BitBlt( 0, 0, mWidth, mHeight, &memDC, 0, 0, SRCCOPY );
		memDC.SelectObject( oldBmp2 );
	}

	/// 화살표
	{
		cBuffDC buffDC( this, mWidth, mHeight, RGB(255,0,0), true );

		CDC memDC;
		memDC.CreateCompatibleDC( &dc );
		CBitmap* oldBmp3 = memDC.SelectObject( &mResource->mArrowBitmap );

		buffDC.GetDC()->BitBlt( 0, 0, mWidth, mHeight, &memDC, 0, 0, SRCCOPY );
		memDC.SelectObject( oldBmp3 );

		buffDC.Rotate( mAngle - 90.0f );
		buffDC.Draw( &backDC, 0, 0, mWidth, mHeight );

	}

	dc.TransparentBlt( 0, 0, mWidth, mHeight, &backDC, 0, 0, mWidth, mHeight, RGB(255,0,0) );
	backDC.SelectObject( oldBmp1 );
}
