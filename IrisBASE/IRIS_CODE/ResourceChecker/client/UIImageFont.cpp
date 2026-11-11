#include "StdAfx.h"
#include "UIImageFont.h"

cUIImageFont::cUIImageFont( NiTexture* tex, unsigned int tx, unsigned int ty, unsigned int cw, unsigned int ch )
: cUIImage( tex )
, mTexX( tx )
, mTexY( ty )
, mCharWidth( cw )
, mCharHeight( ch )
{
// 	/// 텍스처 영역을 설정
// 	SetTextureRect( tx, ty, tx + cw, ty + ch );
// 
// 	/// 이미지 영역을 갱신
// 	cUIRect rect( 0, 0, mCharWidth, mCharHeight);
// 	SetScreenRect( rect );
}

cUIImageFont::~cUIImageFont()
{
}

void cUIImageFont::DrawText( unsigned int x, unsigned int y, const char* text )
{
	assert( text && "null text" );

	for( char c = *text; c; c = *(++text), x += mCharWidth )
	{
		unsigned int tx = mTexX + mCharWidth * (c - '0');
		SetTextureRect( tx, mTexY, tx + mCharWidth, mTexY + mCharHeight );
		SetPos( x, y );
		Draw();
	}
}

unsigned int cUIImageFont::GetTextWidth( unsigned int num ) const
{
	unsigned int count = 1;

	while( num /= 10 )
		++count;

	return count * mCharWidth;
}
