#include "stdafx.h"
#include "fontobject.h"


cFontObject::cFontObject()
: mpRenderer(0)
{
	mhFont = NULL;
	mOuter = false;

	ZeroMemory( &mLogFont, sizeof( mLogFont ) );

	mFontWidth = 0;
	mFontHeight = 0;
}

cFontObject::~cFontObject()
{
	if( mhFont )
	{
		::DeleteObject(mhFont);
	}

	mpRenderer = 0;
}

/* ------------------------------------------------------------------
 * 함수이름 :	cFontObject::SetFontInfo( HWND hwnd, LOGFONT* pLogFont )
 * 목    적 :	폰트의 기본 정보들을 모은다.
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cFontObject::SetFontInfo( HWND /*hwnd*/, LOGFONT* pLogFont )
{
	if( !pLogFont ) 
	{
		return false;
	}

	mLogFont = *pLogFont;
	mhFont = ::CreateFontIndirect( pLogFont );

	if( mhFont == NULL )
	{
		return false;
	}

	/// 
/*		묵향 로컬에서 별도로 처리한 부분이다. 
#ifdef _TL_LOCAL_
	HDC hdc = ::GetDC( hwnd );
	HFONT OldFont = (HFONT)::SelectObject( hdc, mhFont );

	SIZE size;
	::GetTextExtentPoint32( hdc, "Gg", 1, &size );

	::SelectObject( hdc, OldFont );

	::ReleaseDC( hwnd, hdc );

	mFontWidth = (unsigned short)pLogFont->lfWidth;
	mFontHeight = size.cy;
#else
*/

	mFontWidth  = (unsigned short)pLogFont->lfWidth;
	mFontHeight = (unsigned short)(-pLogFont->lfHeight);

	return true;
}

long cFontObject::GetTextExtent( HWND hwnd, LPCTSTR text, unsigned int textLen )
{
//* 로컬시에...
//#ifdef _TL_LOCAL_
	HDC hdc = ::GetDC( hwnd );
	if( hdc == NULL ) 
	{
		assert(0);
		return 0;
	}

	HFONT OldFont = (HFONT)::SelectObject( hdc, mhFont );

	SIZE size;
	::GetTextExtentPoint32( hdc, text, textLen, &size );

	::SelectObject( hdc, OldFont );

	::ReleaseDC( hwnd, hdc );

	return size.cx;
//#else
/*/
	unsigned short DBCNT = 0;
	unsigned short SBCNT = 0;
	for( unsigned int i = 0 ; i < textLen ; i++ )
	{
#ifndef UNICODE
		if( ::IsDBCSLeadByte( text[i] ) )
		{
			DBCNT++;
			i++;
		}
		else
		{
			SBCNT++;
		}
#else
		text;
		SBCNT++;
#endif
	}

	long TextWidth = 0;

	TextWidth = SBCNT*mFontWidth+DBCNT*2*mFontWidth; 

	return TextWidth;
//*/
}