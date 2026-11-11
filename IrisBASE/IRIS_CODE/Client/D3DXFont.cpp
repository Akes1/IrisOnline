#include "stdafx.h"
#include "d3dxfont.h"

cD3DXFont::cD3DXFont()
{
	mpD3dxFont = 0;
}

cD3DXFont::~cD3DXFont()
{
	SAFE_RELEASE( mpD3dxFont );
}

/* ------------------------------------------------------------------
* 함수이름 :	cD3DXFont::CreateFont( HWND hwnd, LOGFONT* pLogFont )
* 목    적 :	D3DXFont의 생성 및 폰트 객체의 기본정보를 셋팅한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cD3DXFont::CreateFont( HWND hwnd, LOGFONT* pLogFont, sCacheInfo& cacheInfo )
{
	D3DXFONT_DESC* pFontDesc = NULL;
	D3DXFONT_DESC font;

	pFontDesc = &font;

	LOGFONT logfont;

	if( !pLogFont )
	{
		pLogFont = &logfont;
		HDC dc = GetDC(hwnd);
		int fontHeight = -MulDiv( 9, GetDeviceCaps( dc, LOGPIXELSY ), 72 );
		
		font.Height = fontHeight;
		font.Width = -font.Height / 2;
		font.Weight = FW_NORMAL;
		font.Italic = FALSE;
		font.MipLevels = D3DX_DEFAULT;

		font.CharSet = DEFAULT_CHARSET;
		font.OutputPrecision = 0;
		font.Quality =  PROOF_QUALITY;//NONANTIALIASED_QUALITY;//PROOF_QUALITY;
		font.PitchAndFamily = 0;
		::lstrcpy( font.FaceName, _T("M+2P+IPAG Regular") );
		//		::lstrcpy( font.FaceName, _T("2002_EYA") );

		logfont.lfHeight = fontHeight;
		logfont.lfWidth = -pLogFont->lfHeight / 2;
		logfont.lfEscapement = 0;
		logfont.lfOrientation = 0;
		logfont.lfWeight = FW_NORMAL;
		logfont.lfItalic = 0;
		logfont.lfUnderline = 0;
		logfont.lfStrikeOut = 0;

		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logfont.lfQuality =  PROOF_QUALITY;//NONANTIALIASED_QUALITY;//PROOF_QUALITY;
		logfont.lfPitchAndFamily = 0;
		::lstrcpy( logfont.lfFaceName, _T("M+2P+IPAG Regular") );
		//		::lstrcpy( logfont.lfFaceName, _T("2002_EYA") );
		ReleaseDC( hwnd, dc );
	}
	else
	{
		font.Height = pLogFont->lfHeight;
		font.Width = pLogFont->lfWidth;
		font.Weight = pLogFont->lfWeight;
		font.Italic = pLogFont->lfItalic;
		font.MipLevels = D3DX_DEFAULT;

		font.CharSet = pLogFont->lfCharSet;
		font.OutputPrecision = pLogFont->lfOutPrecision;
		font.Quality =  pLogFont->lfQuality;
		font.PitchAndFamily = pLogFont->lfPitchAndFamily;
		lstrcpy( font.FaceName, pLogFont->lfFaceName );
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

	/// 사용하려는 폰트를 생성한다.
	HRESULT hr = D3DXCreateFontIndirect( mpRenderer->GetD3DDevice(), pFontDesc, &mpD3dxFont );
	if( FAILED(hr) )
	{
		assert(0);
		return false;
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cD3DXFont::DrawText( LPTSTR str, int strLen, RECT* pRect, unsigned long color, LPD3DXSPRITE pSprite, bool outerLine )
{
	if( pSprite == 0 )
	{
		assert(0);
		return false;
	}

	if( outerLine )
	{
		RECT Out = *pRect;
		Out.left+= 1;
		Out.right+= 1;
		mpD3dxFont->DrawText( pSprite, str, -1, &Out, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff3c3c3c );

		Out = *pRect;
		Out.top += 1;
		Out.bottom += 1;
		mpD3dxFont->DrawText( pSprite, str, -1, &Out, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff3c3c3c );

		Out = *pRect;
		Out.left += 1;
		Out.right+= 1;
		Out.top += 1;
		Out.bottom += 1;
		mpD3dxFont->DrawText( pSprite, str, -1, &Out, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff3c3c3c );
	}

	mpD3dxFont->DrawText( pSprite, str, -1, pRect, DT_LEFT | DT_TOP | DT_NOCLIP, color );
	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	cD3DXFont::Invalidate()
* 목    적 :	장치 소실시 처리
* 주의사항 :	
* ------------------------------------------------------------------ */
void cD3DXFont::Invalidate()
{
	if( mpD3dxFont )
		mpD3dxFont->OnLostDevice();
}

/* ------------------------------------------------------------------
* 함수이름 :	cD3DXFont::Restore()
* 목    적 :	장치 소실후 재설정 처리
* 주의사항 :	
* ------------------------------------------------------------------ */
void cD3DXFont::Restore()
{
	if( mpD3dxFont )
		mpD3dxFont->OnResetDevice();
}