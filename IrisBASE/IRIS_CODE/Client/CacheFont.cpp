#include "stdafx.h"
#include "cachefont.h"


cCacheFont::cCacheFont() :
mpCacheAgent(0)
{
	::memset( &mFontPropertyDesc, 0, sizeof(sFontPropertyDesc) );
}

cCacheFont::~cCacheFont()
{
	SAFE_DELETE(mpCacheAgent);
}

/* ------------------------------------------------------------------
* 함수이름 :	CreateFont(  HWND hwnd, LOGFONT* pLogFont )
* 목    적 :	폰트 생성
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cCacheFont::CreateFont( HWND hwnd, LOGFONT* pLogFont, sCacheInfo& cacheInfo )
{
	LOGFONT		font;
	if( !pLogFont )
	{
		HDC dc = GetDC(hwnd);
		pLogFont = &font;

		int fontHeight = -MulDiv( 9, GetDeviceCaps( dc, LOGPIXELSY ), 72 );

		font.lfHeight = fontHeight;
		font.lfWidth = -pLogFont->lfHeight / 2;
		font.lfEscapement = 0;
		font.lfOrientation = 0;
		font.lfWeight = FW_NORMAL;
		font.lfItalic = 0;
		font.lfUnderline = 0;
		font.lfStrikeOut = 0;
/*
		font.lfCharSet = DEFAULT_CHARSET;
		font.lfOutPrecision = 0;
		font.lfClipPrecision = 0;
		font.lfQuality = PROOF_QUALITY;
		font.lfPitchAndFamily = 0;
*/
		font.lfCharSet = DEFAULT_CHARSET;
		font.lfOutPrecision = OUT_DEFAULT_PRECIS;//OUT_OUTLINE_PRECIS;
		font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		font.lfQuality =  PROOF_QUALITY;
		font.lfPitchAndFamily = FF_SWISS;
		::lstrcpy( font.lfFaceName, _T("M+2P+IPAG Regular") );
		//		::lstrcpy( font.lfFaceName, _T("2002_EYA") );
		ReleaseDC( hwnd, dc );
	}

	mpRenderer = (NiDX9Renderer*)NiRenderer::GetRenderer();
	if( !mpRenderer )
	{
		return false;
	}

	/// Font의 확장 정보들을 셋팅한다.
	if( !cFontObject::SetFontInfo( hwnd, pLogFont ) )
	{
		return false;
	}

	/// CacheAgent 생성..
	mpCacheAgent = new cCacheAgent;
	if( !mpCacheAgent->Initialize( cacheInfo.maxCacheItemNum, cacheInfo.texWidth, cacheInfo.texHeight, cacheInfo.texNum ) )
	{
		return false;
	}

	/// 폰트에 대한 기본 정보 셋팅
	mFontPropertyDesc.hFont = mhFont;

	if( pLogFont->lfHeight < 0 )
		mFontPropertyDesc.height = (pLogFont->lfHeight) * -1;
	else
		mFontPropertyDesc.height = pLogFont->lfHeight;

	mFontPropertyDesc.width = mFontPropertyDesc.height / 2;

	mFontPropertyDesc.pszString = NULL;
	mFontPropertyDesc.strLen = 0;
	mFontPropertyDesc.color = 0xffffffff;

	mCacheInfo = cacheInfo;

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	DrawText( char* str, int strLen, RECT* pRect, unsigned long color )
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cCacheFont::DrawText( LPTSTR str, int strLen, RECT* pRect, unsigned long color, LPD3DXSPRITE /*pSprite*/, bool /*outerLine*/ )
{
	mFontPropertyDesc.pszString = str;
	mFontPropertyDesc.strLen = strLen;
	mFontPropertyDesc.color = color;

	//*
	NiMeshScreenElements* pElements = mpCacheAgent->GetCache( pRect, &mFontPropertyDesc );
	if( pElements )
	{
		/// 화면 출력
		pElements->RenderImmediate( mpRenderer );
		pElements->RemoveAll();
	}
	return true;
}

void cCacheFont::Invalidate()
{
	SAFE_DELETE(mpCacheAgent);
}

void cCacheFont::Restore()
{
	mpCacheAgent = new cCacheAgent;
	if( !mpCacheAgent->Initialize( mCacheInfo.maxCacheItemNum, mCacheInfo.texWidth, mCacheInfo.texHeight, mCacheInfo.texNum ) )
	{
		assert(0);
		return;
	}
}