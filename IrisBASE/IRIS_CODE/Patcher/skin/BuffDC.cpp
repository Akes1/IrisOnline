#include "stdafx.h"
#include "BuffDC.h"

cBuffDC::cBuffDC( CWnd *wnd, int width, int height, COLORREF backColor, bool transparent )
{
	CPaintDC dc(wnd);
	mWnd = wnd;
	mWidth = width;
	mHeight = height;
	mBackColor = backColor;
	mTransparent = transparent;

	mMemBitmap.CreateCompatibleBitmap( &dc, width, height );
	mMemDC.CreateCompatibleDC( &dc );
	mOldBitmap = mMemDC.SelectObject( &mMemBitmap );
}

cBuffDC::~cBuffDC()
{
	mMemDC.SelectObject( mOldBitmap );
}

void cBuffDC::Draw( CDC* dc, int destX, int destY, int destWidth, int destHeight )
{
	if( mTransparent )
		dc->TransparentBlt( destX, destY, destWidth, destHeight, &mMemDC, 0, 0, mWidth, mHeight, mBackColor );
	else
		dc->BitBlt( destX, destY, destWidth, destHeight, &mMemDC, 0, 0, SRCCOPY );
}

void cBuffDC::Rotate( float degree )
{
	CPoint origin( mWidth/2, mHeight/2 );
	Rotate( degree, origin );
}

void cBuffDC::Rotate( float degree, const CPoint& origin )
{
	double rad = degree / 180.00f * 3.14159265358979f;

	// Temp canvas to rotate image onto
	CPaintDC tempCanvas( mWnd );
	CDC tempDC;
	CBitmap tempBmp;

	tempBmp.CreateCompatibleBitmap( &tempCanvas, mWidth, mHeight );
	tempDC.CreateCompatibleDC( &tempCanvas );

	CBitmap* oldBmp = tempDC.SelectObject( &tempBmp );

	// Go through the final rows and columns, filling in the data
	POINT p0;
	POINT p1;

	for( int x = 0; x < mWidth; x++ ) 
	{
		for( int y = 0; y < mHeight; y++ ) 
		{
			p1.x = x;
			p1.y = y;

			// Calculate the original x and y
			p0.x = int((p1.x-origin.x) * cos(rad) - (p1.y-origin.y) * sin(rad) + origin.x);
			p0.y = int((p1.y-origin.y) * cos(rad) + (p1.x-origin.x) * sin(rad) + origin.y);

			// Do the copy
			if( p0.x > 0 && p0.x < mWidth && p0.y > 0 && p0.y < mHeight )
				tempDC.SetPixel( p1, mMemDC.GetPixel(p0) );
			else
				tempDC.SetPixel( p1, mBackColor );
		}
	}

	// Copy temp image back to the original
	mMemDC.BitBlt( 0, 0, mWidth, mHeight, &tempDC, 0, 0, SRCCOPY );

	tempDC.SelectObject( oldBmp );
}
