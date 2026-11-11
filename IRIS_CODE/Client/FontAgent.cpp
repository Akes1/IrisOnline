#include "stdafx.h"
#include "fontagent.h"

#include "RenderSystem.h"

#include "D3DXFont.h"

#include "Application.h"

cFontAgent* cFontAgent::mpFontAgent = NULL;

cFontAgent::cFontAgent()
{
	mpFontAgent = this;

	mpSprite = 0;
	mpSpriteTemp = 0;

	//AddFontResourceEx( _T("./2002_EYA.ttf"), FR_PRIVATE, 0 );
//*
	cFileLoader loader;

	cString path;
	path.Format("./language/%s/iris_gamefont.ttf", cApplication::mLangaugeFolder.Cstr() );
	if( loader.Open( path.Cstr(), true ) == false )
	{
		return;
	}

	DWORD cnt;
	mhFont = AddFontMemResourceEx( (PVOID)loader.GetBufferPtr(), loader.GetSize(), 0, &cnt );
	if( mhFont == 0 )
	{
		assert(0);
		return;
	}
//*/
//	mFontArray.Reserve( eFont_Max );
}

cFontAgent::~cFontAgent()
{
	SAFE_RELEASE( mpSprite );

//	RemoveFontResourceEx( _T("./2002_EYA.ttf"), FR_PRIVATE, 0 );

	if( mhFont )
		RemoveFontMemResourceEx( mhFont );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cFontAgent::Init()
{
	sCacheInfo info;
	cFontObject* pFont = 0;
	HWND hwnd = RENDERSYS->GetHWND();
	HDC  dc = GetDC( hwnd );

	HRESULT hr = D3DXCreateSprite( RENDERSYS->GetRenderer()->GetD3DDevice(), &mpSprite );
	if( FAILED(hr) )
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// UI용 폰트 ( UI용, 고정 SystemMsg )
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;
	info.texNum = 2;
	if( pFont && pFont->CreateFont( hwnd, NULL, info ) == true )
	{
		mFontArray.PushBack( pFont );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// UI용 BOLD
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;

	int fontHeight = -MulDiv( 9, GetDeviceCaps( dc, LOGPIXELSY ), 72 );

	LOGFONT		font;
	font.lfHeight = fontHeight;
	font.lfWidth = -font.lfHeight / 2;
	font.lfEscapement = 0;
	font.lfOrientation = 0;
	font.lfWeight = FW_BOLD;
	font.lfItalic = 0;
	font.lfUnderline = 0;
	font.lfStrikeOut = 0;
	font.lfCharSet = DEFAULT_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font.lfQuality =  PROOF_QUALITY;
	font.lfPitchAndFamily = 0;
	::lstrcpy( font.lfFaceName, _T("M+2P+IPAG Regular") );
	//	::lstrcpy( font.lfFaceName, _T("2002_EYA") );

	info.texNum = 1;
	if( pFont && pFont->CreateFont( hwnd, &font, info ) == true )
	{
		mFontArray.PushBack( pFont );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 시스템 폰트 (Tooltip, Name, GameInfo, 유동SystemMsg)
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;
	info.texNum = 2;

	if( pFont && pFont->CreateFont( hwnd, NULL, info ) )
	{
		mFontArray.PushBack( pFont );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 채팅용
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;
	info.texNum = 2;

	if( pFont && pFont->CreateFont( hwnd, NULL, info ) )
	{
		mFontArray.PushBack( pFont );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 공지용
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;
	if( !pFont )
	{
		ReleaseDC( hwnd, dc );
		return false;
	}

	fontHeight = -MulDiv( 15, GetDeviceCaps( dc, LOGPIXELSY ), 72 );

	font.lfHeight = fontHeight;
	font.lfWidth = -font.lfHeight / 2;
	font.lfEscapement = 0;
	font.lfOrientation = 0;
	font.lfWeight = FW_NORMAL;
	font.lfItalic = 0;
	font.lfUnderline = 0;
	font.lfStrikeOut = 0;
	font.lfCharSet = DEFAULT_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font.lfQuality =  PROOF_QUALITY;
	font.lfPitchAndFamily = 0;
	::lstrcpy( font.lfFaceName, _T("M+2P+IPAG Regular") );
	//	::lstrcpy( font.lfFaceName, _T("2002_EYA") );

	info.texNum = 1;
	if( pFont && pFont->CreateFont( hwnd, &font, info ) == true )
	{
		mFontArray.PushBack( pFont );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 깊이가 있는 면에 그려질 폰트용
	pFont = new cD3DXFont;
//	pFont = new cCacheFont;

	fontHeight = -MulDiv( 9, GetDeviceCaps( dc, LOGPIXELSY ), 72 );

	font.lfHeight = fontHeight;
	font.lfWidth = -font.lfHeight / 2;
	font.lfEscapement = 0;
	font.lfOrientation = 0;
	font.lfWeight = FW_NORMAL;
	font.lfItalic = 0;
	font.lfUnderline = 0;
	font.lfStrikeOut = 0;

	font.lfCharSet = DEFAULT_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font.lfQuality =  PROOF_QUALITY;
	font.lfPitchAndFamily = 0;
	::lstrcpy( font.lfFaceName, _T("M+2P+IPAG Regular") );
	//	::lstrcpy( font.lfFaceName, _T("2002_EYA") );

	info.texNum = 2;
	if( pFont && pFont->CreateFont( hwnd, &font, info ) == true )
	{
		mFontArray.PushBack( pFont );
		pFont->SetAlpha( false );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// NPC 대화창 UI에 쓰일 폰트!!
	pFont = new cD3DXFont;
	//	pFont = new cCacheFont;

	fontHeight = -MulDiv( 13, GetDeviceCaps( dc, LOGPIXELSY ), 72 );

	font.lfHeight = fontHeight;
	font.lfWidth = -font.lfHeight / 2;
	font.lfEscapement = 0;
	font.lfOrientation = 0;
	font.lfWeight = FW_NORMAL;
	font.lfItalic = 0;
	font.lfUnderline = 0;
	font.lfStrikeOut = 0;

	font.lfCharSet = DEFAULT_CHARSET;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font.lfQuality =  PROOF_QUALITY;
	font.lfPitchAndFamily = 0;
	::lstrcpy( font.lfFaceName, _T("M+2P+IPAG Regular") );
	//	::lstrcpy( font.lfFaceName, _T("2002_EYA") );

	info.texNum = 2;
	if( pFont && pFont->CreateFont( hwnd, &font, info ) == true )
	{
		mFontArray.PushBack( pFont );
		pFont->SetAlpha( false );
	}
	else
	{
		assert(0);
		ReleaseDC( hwnd, dc );
		return false;
	}

	
	ReleaseDC( hwnd, dc );

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	cFontAgent::Exit()
* 목    적 :	종료부
* 주의사항 :	
* ------------------------------------------------------------------ */
void cFontAgent::Exit()
{
	SAFE_RELEASE( mpSprite );

	if( mFontArray.GetSize() )
	{
		for( unsigned int i=0; i<mFontArray.GetSize(); i++ )
		{
			cFontObject* pFont = (cFontObject*)mFontArray[i];
			if( pFont )
			{
				SAFE_DELETE(pFont);
			}
		}
		mFontArray.Clear();

	}
}

/* ------------------------------------------------------------------
 * 함수이름 :	cFontAgent::Invalidate()
 * 목    적 :	장치 소실시 유효하지 않은 메모리 해제
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cFontAgent::Invalidate()
{
	if( mpSprite )
		mpSprite->OnLostDevice();

	for( unsigned int i=0; i<eFont_Max; i++ )
	{
		cFontObject* pFont = (cFontObject*)mFontArray[i];
		if( pFont )
		{
			pFont->Invalidate();
		}
	}
}

/* ------------------------------------------------------------------
 * 함수이름 :	cFontAgent::Restore()
 * 목    적 :	장치 소실시 유효하지 않은 메모리 재생성
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cFontAgent::Restore()
{
	if( mpSprite )
		mpSprite->OnResetDevice();

	for( unsigned int i=0; i<eFont_Max; i++ )
	{
		cFontObject* pFont = (cFontObject*)mFontArray[i];
		if( pFont )
		{
			pFont->Restore();
		}
	}
}

/* ------------------------------------------------------------------
* 함수이름 :	
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
void cFontAgent::DrawText( unsigned short fontIdx, LPTSTR text, int textLen, unsigned int posX, unsigned int posY, unsigned long textColor, bool outerLine )
{
	POINT ps;
	ps.x = (LONG)posX;
	ps.y = (LONG)posY;
	DrawText( fontIdx, text, textLen, &ps, textColor, outerLine );
}

void cFontAgent::DrawText( unsigned short fontIdx, LPTSTR text, int textLen, POINT* textPos, unsigned long textColor, bool outerLine )
{
	/// 예외 처리
	if( fontIdx >= eFont_Max )
		return;

	if( textPos->x<0 || textPos->y<0 )
		return;

	if( textPos->x >= (int)RENDERSYS->GetScreenWidth() ||  textPos->y >= (int)RENDERSYS->GetScreenHeight() )
		return;

	cFontObject* pFont = (cFontObject*)mFontArray[fontIdx];
	if( pFont == NULL )
		return;

	if( textLen == -1 )
		textLen = ::_tcslen( text );
	else if( textLen == 0 )
		return;

	/// 그려질 영역 계산
	RECT rect = { textPos->x, textPos->y, 0, 0 };
	rect.right = rect.left + GetTextExtent( fontIdx, text, textLen ) + GetTextWidth( fontIdx ) / 4;
	rect.bottom = rect.top + GetTextHeight( fontIdx );

	/// 영역이 화면 밖이면 
	if( rect.right < 0 || rect.bottom < 0 )
		return;
	if( rect.right > (LONG)RENDERSYS->GetScreenWidth() || rect.bottom  > (LONG)RENDERSYS->GetScreenHeight() )
		return;


	BeginRender();

	/// 텍스트 등록..
	pFont->DrawText( text, textLen, &rect, textColor, mpSpriteTemp, outerLine );

	EndRender();
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetTextExtent( unsigned short fontIdx, char* text, unsigned int textLen )
 * 목    적 :	UI에서 문자열의 길이를 알고자 할때 사용한다.
 * 주의사항 :	
 * ------------------------------------------------------------------ */
long cFontAgent::GetTextExtent( unsigned short fontIdx, LPCTSTR text, unsigned int textLen )
{
	cFontObject* pFont = (cFontObject*)mFontArray[fontIdx];
	if( !pFont ) 
	{
		return 0;
	}

	return pFont->GetTextExtent( RENDERSYS->GetHWND(), text, textLen);
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetTextWidth( unsigned short fontIdx )
 * 목    적 :	글꼴에서 문자의 평균 너비를 반환
 * 주의사항 :	
 * ------------------------------------------------------------------ */
long cFontAgent::GetTextWidth( unsigned short fontIdx )
{
	cFontObject* pFont = (cFontObject*)mFontArray[fontIdx];
	if( !pFont ) 
	{
		return 0;
	}

	return pFont->GetTextWidth();
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetTextHeight( unsigned short fontIdx )
 * 목    적 :	문자 셀 또는 글꼴 문자의 높이를 반환
 * 주의사항 :	
 * ------------------------------------------------------------------ */
long cFontAgent::GetTextHeight( unsigned short fontIdx )
{
	cFontObject* pFont = (cFontObject*)mFontArray[fontIdx];
	if( !pFont ) 
	{
		return 0;
	}

	/// 1은 밑줄 표시를 위해서 더해준다.
	return pFont->GetTextHeight() + 1;
}

void cFontAgent::BeginRender()
{
	mpSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
	mpSpriteTemp = mpSprite;
}

void cFontAgent::EndRender()
{
	mpSprite->End();
	mpSpriteTemp = NULL;
}
