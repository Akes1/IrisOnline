#include "stdafx.h"
#include "cacheagent.h"

#include "BinSearchAE.h"

cCacheAgent::cCacheAgent() :
mhDC(0), mhMemDC(0), mhBrush(0), //mpImageBuffer(0),
mpBlockDescPool(0), mpLineDescPool(0), mpLineDescSearch(0),
mpCacheHashBuffer(0), mpTextureBufferDesc(0), mpPrimaryBufferDesc(0)
{
	mTexWidth = 0;
	mTexHeight = 0;

	mMaxLineDescNum = 0;
	mMaxTextureBlockDescNum = 0;

	mTextureBufferNum = 0;
	mMaxCacheItemNum = 0;

	mImageDepth = 2;
}

cCacheAgent::~cCacheAgent()
{
	SAFE_DELETE(mpCacheHashBuffer);
	SAFE_DELETE(mpLineDescSearch);

	SAFE_DELETE(mpLineDescPool);
	SAFE_DELETE(mpBlockDescPool);

	sTextureBufferDesc*	pBufferDesc = mpTextureBufferDesc;
	sTextureBufferDesc*	pNext;
	for( unsigned long i=0; i<mTextureBufferNum; i++)
	{
		pNext = pBufferDesc->pNext;

		pBufferDesc->pLineDesc = NULL;
		pBufferDesc->pBlockDesc = NULL;

		assert(pBufferDesc->pNiTexture->GetRefCount() == 1);
		pBufferDesc->pNiTexture = 0;

		delete pBufferDesc;
		pBufferDesc = pNext;
	}

//	SAFE_DELETE_ARRAY(mpImageBuffer);

	if( mhBrush )
	{
		::DeleteObject( mhBrush );
		mhBrush = NULL;
	}

	if( mhMemDC )
	{
		::DeleteDC( mhMemDC );
		mhMemDC = NULL;
	}

	if( mhDC )
	{
		::ReleaseDC( NULL, mhDC );
		mhDC = NULL;
	}
}

/* ------------------------------------------------------------------
* 함수이름 :	Initialize( NiDX9Renderer* pRenderer, unsigned int maxCacheItemNum, unsigned int texWidth, unsigned int texHeight, unsigned int texNum )
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cCacheAgent::Initialize( unsigned int maxCacheItemNum, unsigned int texWidth, unsigned int texHeight, unsigned int texNum )
{
	NiDX9Renderer* pRenderer = NiDX9Renderer::GetRenderer();
	if( !pRenderer )
		return false;

	/// 텍스쳐를 생성하기 위한 기초 정보를 생성.
	NiPixelFormat format = NiPixelFormat::BGRA8888;
	NiTexture::FormatPrefs textureFormat;
	mImageDepth = 4;
	textureFormat.m_eAlphaFmt = NiTexture::FormatPrefs::BINARY;
	textureFormat.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
	textureFormat.m_eMipMapped = NiTexture::FormatPrefs::NO;

	/// 문자를 출력한 이미지를 생성하기 위한 기본 변수 생성/초기화
	mhDC = ::GetDC( NULL );
	mhMemDC = ::CreateCompatibleDC(mhDC);
	mhBrush = ::CreateSolidBrush( 0xff000000 );

	mTexWidth = texWidth;
	mTexHeight = texHeight;

//	mpImageBuffer = new unsigned char[mTexWidth*mTexHeight*mImageDepth];
//	::memset( mpImageBuffer,0, mTexWidth*mTexHeight*mImageDepth );

	mTextureBufferNum = texNum;
	mMaxCacheItemNum = maxCacheItemNum;


	/// 텍스쳐 버퍼 원형 리스트 생성 및 초기화
	mpTextureBufferDesc = new sTextureBufferDesc;

	sTextureBufferDesc*	pBufferDesc = mpTextureBufferDesc;
	::memset( pBufferDesc, 0, sizeof(sTextureBufferDesc) );

	for( unsigned long i=0; i<mTextureBufferNum-1; i++ )
	{
		pBufferDesc->pNext = new sTextureBufferDesc;
		pBufferDesc = pBufferDesc->pNext;
		::memset( pBufferDesc, 0,sizeof(sTextureBufferDesc) );

		pBufferDesc->testnum = i+1;
	}
	pBufferDesc->pNext = mpTextureBufferDesc;

	/// 텍스쳐 버퍼에 텍스쳐를 생성한다.
	for ( unsigned long i=0; i<mTextureBufferNum; i++ )
	{
		/// Screen Texture를 생성한다.
		NiSourceTexture* pTemp = NiSourceTexture::Create( NiNew NiPixelData(mTexWidth, mTexHeight, format), textureFormat );
		if( !pTemp )
		{
			assert(0);
			return false;
		}

		///
		pBufferDesc->pNiTexture = NiNew NiMeshScreenElements( false, false, 1, 
			1, 1, 4, 1, 2, 1, NiDataStream::ACCESS_GPU_READ | NiDataStream::ACCESS_CPU_WRITE_VOLATILE );

		NiTexturingProperty* pkTexture = NiNew NiTexturingProperty();
		pkTexture->SetApplyMode( NiTexturingProperty::APPLY_REPLACE );
		pkTexture->SetBaseTexture( pTemp );
		pBufferDesc->pNiTexture->AttachProperty(pkTexture);

		NiAlphaProperty* pkDefaultAlphaProperty = NiNew NiAlphaProperty;
		pkDefaultAlphaProperty->SetAlphaBlending( true );
		pkDefaultAlphaProperty->SetSrcBlendMode( NiAlphaProperty::ALPHA_SRCALPHA );
		pkDefaultAlphaProperty->SetDestBlendMode( NiAlphaProperty::ALPHA_INVSRCALPHA );
		pBufferDesc->pNiTexture->AttachProperty( pkDefaultAlphaProperty );

		// Create the default vertex color property
		NiVertexColorProperty* pkDefaultVertexColorsProperty = NiNew NiVertexColorProperty();
		pkDefaultVertexColorsProperty->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
		pkDefaultVertexColorsProperty->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
		pBufferDesc->pNiTexture->AttachProperty( pkDefaultVertexColorsProperty );

		// Create the default material property
		NiMaterialProperty* pkDefaultMaterialProperty = NiNew NiMaterialProperty();
		pkDefaultMaterialProperty->SetEmittance( NiColor::WHITE );
		pkDefaultMaterialProperty->SetAlpha( 1.0f );
		pBufferDesc->pNiTexture->AttachProperty( pkDefaultMaterialProperty );

		// Create the default Z Buffer property
		NiZBufferProperty* pkDefaultZBufferProperty = NiNew NiZBufferProperty;
		pkDefaultZBufferProperty->SetZBufferTest( false );
		pkDefaultZBufferProperty->SetZBufferWrite( false );
//		pkDefaultZBufferProperty->SetTestFunction( NiZBufferProperty::TEST_LESSEQUAL );
		pBufferDesc->pNiTexture->AttachProperty( pkDefaultZBufferProperty );

		pBufferDesc->pNiTexture->UpdateProperties();
		pBufferDesc->pNiTexture->UpdateEffects();

		//		/// Lock하기위한 DX텍스쳐를 참조 해 놓는다.
		//		NiDX9TextureData* pRenderData = (NiDX9TextureData*)pTemp->GetRendererData();
		//		pBufferDesc->pTexture = (D3DTexturePtr)pRenderData->GetD3DTexture();

		pBufferDesc = pBufferDesc->pNext;
	}

	/// 사용 버퍼의 할당
	mpPrimaryBufferDesc = mpTextureBufferDesc;
	mpPrimaryBufferDesc->availableHeight= mTexHeight;
	mpPrimaryBufferDesc->posY = 0;
	mpPrimaryBufferDesc->pBlockDesc = NULL;
	mpPrimaryBufferDesc->pLineDesc = NULL;

	mMaxLineDescNum = mTexHeight / MIN_FONT_HEIGHT * mTextureBufferNum;
	mMaxTextureBlockDescNum = mMaxCacheItemNum;

	mpBlockDescPool = new cBlockDescPool( mMaxTextureBlockDescNum );
	mpLineDescPool = new cLineDescPool( mMaxLineDescNum, mTexHeight / MIN_FONT_HEIGHT );

	mpLineDescSearch = new cBinSearchAE;
	mpLineDescSearch->Initialize( mMaxLineDescNum );

	mpCacheHashBuffer = new cCacheBuffer;
	mpCacheHashBuffer->Initialize( (mTexWidth/MIN_FONT_HEIGHT) * mMaxLineDescNum * mTextureBufferNum, MAX_STRING_BYTES_NUM );

	return true;
}

NiMeshScreenElements* cCacheAgent::GetCache( RECT* pRect, sFontPropertyDesc* pDesc )
{
	NiDX9Renderer* pRenderer = NiDX9Renderer::GetRenderer();
	if( !pRenderer )
		return 0;

	sTextureBlockDesc* pBlockDesc = AddCache( pRect, pDesc );
	if( !pBlockDesc )
		return 0;

	(pBlockDesc->pNiTexture)->AddNewScreenRect( (short)pRect->top, 
		(short)pRect->left, 
		(unsigned short)pBlockDesc->width, 
		(unsigned short)pBlockDesc->height, 
		(unsigned short)pBlockDesc->rect.top, 
		(unsigned short)pBlockDesc->rect.left );
	return pBlockDesc->pNiTexture;
}

sTextureBlockDesc* cCacheAgent::AddCache( RECT* pRect, sFontPropertyDesc* pDesc )
{
	NiDX9Renderer* pRenderer = NiDX9Renderer::GetRenderer();
	if( !pRenderer )
	{
		assert(0);
		return 0;
	}

	/// 기존 폰트 정보를 찾는다.
	sTextureBlockDesc* pBlockDesc = (sTextureBlockDesc*)mpCacheHashBuffer->SearchValue( pDesc->pszString, pDesc->strLen, pDesc->hFont, pDesc->color );
	if( !pBlockDesc )
	{
		/// 블럭 정보 생성..

//		HBITMAP hOldBitmap;
//		HBITMAP	hBitmap;

		void* pBuffer = 0;
		
		HBITMAP hBitmap = NULL;
		CreateBitmapFromText( pDesc->hFont, hBitmap, pDesc->pszString, pDesc->strLen, pDesc->color, pRect, pDesc->height, &pBuffer );

		if( hBitmap == NULL )
			return 0;

		BITMAP bitmap;
		::GetObject( hBitmap, sizeof(BITMAP), &bitmap );
		unsigned int width = bitmap.bmWidth;
		unsigned int height = bitmap.bmHeight;

		/// 이미지 생성
//		HBITMAP	hBitmap = CreateBitmapFromText( pDesc->hFont, pDesc->pszString, pDesc->strLen, pDesc->color, pRect, pDesc->height, 
//			width, height, mpImageBuffer );

//		DDBToDIB( mhMemDC, hBitmap, mpImageBuffer );

//		::SelectObject( mhMemDC, hOldBitmap );

		if( width > mTexWidth )
			width = mTexWidth;

		if( height > mTexHeight )
			height = mTexHeight;

		/// 텍스쳐블럭을 할당받는다..
		pBlockDesc = AllocTextureBlock( width, height );
		pBlockDesc->pHashHandle = mpCacheHashBuffer->InsertValue( pDesc->pszString, pDesc->strLen, pDesc->hFont, pDesc->color, pBlockDesc );
		if( pBlockDesc->pHashHandle == 0 )
		{
			assert(0);
		}

		/// 텍스쳐상에서의 Block위치
		pBlockDesc->rect.left = pBlockDesc->posX;
		pBlockDesc->rect.right = pBlockDesc->posX + pBlockDesc->width;
		pBlockDesc->rect.top = pBlockDesc->posY;
		pBlockDesc->rect.bottom = pBlockDesc->posY + pBlockDesc->height;

		/// 글자 색상을 알아 오기 위해서
		//DWORD fontColor = pDesc->color & 0x00ffffff;
		//DWORD alphaColor = pDesc->color & 0xff000000;

		/// 이미지 복사
		D3DLOCKED_RECT lockRect;
		D3DTexturePtr pD3DTexture = pBlockDesc->GetDxTexture();// ->pTexture;
		if( pD3DTexture == 0 )
			return 0;

		if( pD3DTexture->LockRect( 0, &lockRect, &pBlockDesc->rect, 0 ) == D3D_OK )
		{
			int	dPitch = lockRect.Pitch / mImageDepth;

			DWORD* pDst = (DWORD*)lockRect.pBits;
			DWORD* pSrc = (DWORD*)pBuffer;

			for( DWORD y=0; y<height; y++ )
				memcpy( &pDst[dPitch*y], &pSrc[width*y], sizeof(DWORD)*width );

			pD3DTexture->UnlockRect(0);
		}

		if( pBlockDesc->pNiTexture )
			pBlockDesc->pNiTexture->UpdateProperties();

		::DeleteObject(hBitmap);
	}

	return pBlockDesc;
}

/* ------------------------------------------------------------------
* 함수이름 :	CreateBitmapFromText(...)
* 목    적 :	글자를 찍은 DDB 비트맵을 생성한다.
* 주의사항 :	
* ------------------------------------------------------------------ */

void cCacheAgent::CreateBitmapFromText( HFONT hFont, HBITMAP& hBitmap, LPTSTR string, unsigned long strLen, unsigned long color, RECT* pRect, unsigned long fontHeight, VOID** pOut )
{
	/// 색깔값 분리.
	unsigned char red	= (unsigned char)((color & 0xFF0000) >> 16);
	unsigned char green	= (unsigned char)((color & 0xFF00) >> 8);
	unsigned char blue	= (unsigned char)((color & 0xFF));
	COLORREF txtColor = RGB(red, green, blue);	

	unsigned long alphaColor = color & 0xFF000000;

	red	= (unsigned char)((0xff3c3c3c & 0xFF0000) >> 16);
	green	= (unsigned char)((0xff3c3c3c & 0xFF00) >> 8);
	blue	= (unsigned char)((0xff3c3c3c & 0xFF));
	COLORREF backColor = RGB(red, green, blue);

	RECT rect;
	unsigned int width = (pRect->right - pRect->left)+1;
	unsigned int height = (pRect->bottom - pRect->top)+1;

	if( height < fontHeight )
		height = fontHeight+1;

	width = ((width + 0x00000001) & 0xfffffffe);
	pRect->right = pRect->left + width;

	rect.right = width + 10;
	rect.bottom = height + 10;
	rect.left = 0;
	rect.top = 0;

	HFONT hOldFont;
	hOldFont = (HFONT)::SelectObject( mhMemDC, hFont );

	///
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -(LONG)height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = mImageDepth*8;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	hBitmap = CreateDIBSection( mhMemDC, &bmi, DIB_RGB_COLORS, pOut, NULL, 0x0 );
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject( mhMemDC, hBitmap );

	::SetTextAlign( mhMemDC, TA_TOP );
	::SetBkColor( mhMemDC, 0x00000000 );

	//	::SetBkColor( mhMemDC, backColor );
	//*
	::SetTextColor( mhMemDC, backColor );
	::ExtTextOut( mhMemDC, 1, 0, 0, &rect, string, strLen, NULL );

	::SetBkMode( mhMemDC, TRANSPARENT );
	::ExtTextOut( mhMemDC, 0, 1, 0, &rect, string, strLen, NULL );
	::ExtTextOut( mhMemDC, 1, 2, 0, &rect, string, strLen, NULL );


	for( unsigned int y =0;y<height;++y )
	{
		for( unsigned int x =0;x<width;++x )
		{
			float alpha = 1.0f;
			unsigned char r = (unsigned char)((((UINT32 *)*pOut)[x + y * width] & 0xFF0000) >> 16);
			unsigned char g = (unsigned char)((((UINT32 *)*pOut)[x + y * width] & 0xFF00) >> 8);
			unsigned char b = (unsigned char)((((UINT32 *)*pOut)[x + y * width] & 0xFF));

			if( r >= g )
			{
				if( r >= b )
					alpha = (float)r / (float)GetRValue(backColor);
				else
					alpha = (float)b / (float)GetBValue(backColor);
			}
			else
			{
				if( g >= b )
					alpha = (float)g / (float)GetGValue(backColor);
				else
					alpha = (float)b / (float)GetBValue(backColor);
			}

			if( alpha == 1.0f )
			{
				alpha = (float)alphaColor;
			}
			else
			{
				alpha = (float)((DWORD)(alpha*255) << 24);

				if( alpha <= (0xff000000 - alphaColor) )
					alpha = 0;
				else
					alpha -= (0xff000000 - alphaColor);
			}

			((UINT32 *)*pOut)[x + y * width] = (0xff3c3c3c & 0x00ffffff) | (DWORD)alpha;
		}
	}
	//*/

	//ETO_OPAQUE
	::SetTextColor( mhMemDC, txtColor );
	::ExtTextOut( mhMemDC, 0, 0, 0, &rect, string, strLen, NULL );

	for( unsigned int y =0;y<height;++y )
	{
		for( unsigned int x =0;x<width;++x )
		{
			if( (((UINT32*)*pOut)[x + y * width] & 0x00ffffff) == 0x003c3c3c )
				continue;

			float alpha = 1.0f;
			unsigned char r = (unsigned char)((((UINT32*)*pOut)[x + y * width] & 0xFF0000) >> 16);
			unsigned char g = (unsigned char)((((UINT32*)*pOut)[x + y * width] & 0xFF00) >> 8);
			unsigned char b = (unsigned char)((((UINT32*)*pOut)[x + y * width] & 0xFF));

			if( r >= g )
			{
				if( r >= b )
					alpha = (float)r / (float)GetRValue(txtColor);
				else
					alpha = (float)b / (float)GetBValue(txtColor);
			}
			else
			{
				if( g >= b )
					alpha = (float)g / (float)GetGValue(txtColor);
				else
					alpha = (float)b / (float)GetBValue(txtColor);
			}

			if( alpha >= 0.8f )
			{
				alpha = (float)alphaColor;
			}
			else if( alpha <= 0.2f )
			{
				alpha = 0;
			}
			else
			{
				alpha = (float)((DWORD)(alpha*255) << 24);
				if( alpha <= (0xff000000 - alphaColor)  )
					alpha = 0;
				else
					alpha -= (0xff000000 - alphaColor);
			}

			((UINT32*)*pOut)[x + y * width] = (color & 0x00ffffff) | (DWORD)alpha;
		}
	}

	::SelectObject( mhMemDC, hOldBitmap );
	::SelectObject( mhMemDC, hOldFont );
}

/* ------------------------------------------------------------------
* 함수이름 :	DDBToDIB( HDC hDC, HBITMAP hBitmap, unsigned char* pOut )
* 목    적 :	DIB비트맵으로 변환
* 주의사항 :	
* ------------------------------------------------------------------ */
void cCacheAgent::DDBToDIB( HDC hDC, HBITMAP hBitmap, unsigned char* pOut )
{
	/// Spectify DIB properties.
	BITMAP bitmap;
	::GetObject( hBitmap, sizeof(BITMAP), &bitmap );

	BITMAPINFOHEADER bih;
	bih.biClrImportant	= 0;
	bih.biClrUsed		= 0;			
	bih.biXPelsPerMeter	= 0;
	bih.biYPelsPerMeter	= 0;
	bih.biSize			= sizeof(BITMAPINFOHEADER);
	bih.biWidth			= bitmap.bmWidth;
	bih.biHeight		= -bitmap.bmHeight;
	bih.biPlanes		= 1;
	bih.biBitCount		= mImageDepth*8;//16;
	bih.biCompression	= BI_RGB;	

	bih.biSizeImage = 0;

	/// Get Pixel data.
	BITMAPINFO bif;
	bif.bmiHeader = bih;

	::GetDIBits( hDC, hBitmap, 0, bitmap.bmHeight, pOut, &bif, DIB_RGB_COLORS );
}

/* ------------------------------------------------------------------
* 함수이름 :	AllocTextureBlock( unsigned int width, unsigned int height )
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
sTextureBlockDesc*	cCacheAgent::AllocTextureBlock( unsigned int width, unsigned int height )
{
	// height가 맞는 라인 디스크립터부터 찾는다.
	// 검색
	sTextureLineDesc*	pLineDesc;
	sTextureBlockDesc*	pBlockDesc = NULL;

	sContainerAE*	pCur;
	//	height += 1;
	unsigned long lineSearchKey = height;

lb_alloc_texblock:

	pBlockDesc = mpBlockDescPool->Alloc();
	if( !pBlockDesc )
	{
		ChangeTextureBuffer();
		goto lb_alloc_texblock;
	}

lb_begin_search:
	pCur = mpLineDescSearch->SearchContainer( lineSearchKey );
	if( !pCur )
	{
		/// 없으면 새로 할당...
		goto lb_alloc_linedesc;
	}

	if( pCur )
		lineSearchKey = ((sTextureLineDesc*)pCur->pItem)->height + 1;

	while (pCur)
	{
		pLineDesc = (sTextureLineDesc*)pCur->pItem;
		if( pLineDesc->availableWidth >= width )
			goto lb_find_ok;

		pCur = pCur->pNext;
	}

	//lb_research:
	//	lineSearchKey = (unsigned long)pLineDesc->height + 1;
	goto lb_begin_search;

	/// 그래도 없으면 새로 할당..	
lb_alloc_linedesc:
	pLineDesc = AllocLineDesc( height );
	pLineDesc->pSearchHandle = mpLineDescSearch->InsertItem( pLineDesc->height, pLineDesc );

lb_find_ok:
	sTextureBufferDesc*	pBufferDesc;
	pBufferDesc = pLineDesc->pBufferDesc;

	//	pBlockDesc->pTexture = pLineDesc->pTexture;
	pBlockDesc->pNiTexture = pLineDesc->pNiTexture;

	pBlockDesc->height = pLineDesc->height;
	pBlockDesc->width = width;

	pBlockDesc->posX = pLineDesc->currentPosX;
	pBlockDesc->posY = pLineDesc->posY;

	/// 기존의 라인 디스크립터가 가지고 있던 버퍼 디스크립터 체인에 연결.한번에 지우기 위해서다.
	pBlockDesc->pBufferDesc = pBufferDesc;
	pBlockDesc->pNext = pBufferDesc->pBlockDesc;
	pBufferDesc->pBlockDesc = pBlockDesc;

	/// 다음번 할당받을 X좌표를 옮긴다.
	pLineDesc->currentPosX += width;
	/// 남은 폭을 계산한다.
	pLineDesc->availableWidth -= width;

	//lb_return:
	return pBlockDesc;
}

/* ------------------------------------------------------------------
* 함수이름 :	AllocLineDesc( unsigned int height )
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
sTextureLineDesc* cCacheAgent::AllocLineDesc( unsigned int height )
{
	int reservedHeight = (int)mpPrimaryBufferDesc->availableHeight - height;
	if( reservedHeight < 0 )
	{
		ChangeTextureBuffer();
	}

	sTextureLineDesc* pLineDesc = mpLineDescPool->Alloc();

	pLineDesc->pBufferDesc = mpPrimaryBufferDesc;
	pLineDesc->pNext = mpPrimaryBufferDesc->pLineDesc;
	mpPrimaryBufferDesc->pLineDesc = pLineDesc;

	//	pLineDesc->pTexture = mpPrimaryBufferDesc->pTexture;
	pLineDesc->pNiTexture = mpPrimaryBufferDesc->pNiTexture;

	pLineDesc->availableWidth = mTexWidth;
	pLineDesc->currentPosX = 0;
	pLineDesc->height = height;
	pLineDesc->posY = mpPrimaryBufferDesc->posY;

	/// 다음번 할당받을 Y좌표를 옮긴다.
	mpPrimaryBufferDesc->posY += height;
	/// 남은 높이를 계산한다.
	mpPrimaryBufferDesc->availableHeight -= height;

	//lb_return:
	return pLineDesc;
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeTextureBuffer()
* 목    적 :	현재 텍스쳐가 모두 사용되면 다음 텍스쳐를 할당한다.
* 주의사항 :	새로 할당되는 텍스쳐가 기존에 사용 되었다면, 기존 데이터는 모두 초기화 된다.
* ------------------------------------------------------------------ */
void cCacheAgent::ChangeTextureBuffer()
{
	sTextureBufferDesc*	pBackBufferDesc;
	pBackBufferDesc = mpPrimaryBufferDesc->pNext;

	sTextureLineDesc*	pNextLineDesc;
	sTextureLineDesc*	pCurLineDesc = pBackBufferDesc->pLineDesc;
	while(pCurLineDesc)
	{
		pNextLineDesc = pCurLineDesc->pNext;
		mpLineDescSearch->DeleteItem( (sContainerAE*)pCurLineDesc->pSearchHandle );
		mpLineDescPool->Free( pCurLineDesc );
		pCurLineDesc = pNextLineDesc;
	}

	sTextureBlockDesc*	pNextBlockDesc;
	sTextureBlockDesc*	pCurBlockDesc = pBackBufferDesc->pBlockDesc;
	while(pCurBlockDesc)
	{
		pNextBlockDesc = pCurBlockDesc->pNext;

		/// 블럭디스크럽터를해쉬에서 제거.
		mpCacheHashBuffer->DeleteValue( pCurBlockDesc->pHashHandle );
		//
		//////////////////////////////

		mpBlockDescPool->Free( pCurBlockDesc );
		pCurBlockDesc = pNextBlockDesc;
	}

	mpPrimaryBufferDesc = pBackBufferDesc;
	mpPrimaryBufferDesc->availableHeight= mTexHeight;
	mpPrimaryBufferDesc->posY = 0;
	mpPrimaryBufferDesc->pBlockDesc = NULL;
	mpPrimaryBufferDesc->pLineDesc = NULL;
}