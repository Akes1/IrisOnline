/* ====================================================================
* 파    일 :	CacheAgent.h
* 목    적 :	캐쉬를 생성하고, 버퍼를 연결하여 관리한다.
* 작 성 자 :	이법석
* 작 성 일 :	2006.11.10
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		06.11.10    이법석		생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

#include "CacheBuffer.h"

#define MAX_SEARCH_TABLE_NUM	10
#define MIN_FONT_HEIGHT			8
#define MAX_STRING_BYTES_NUM	256

struct sTextureBufferDesc;

struct sFontPropertyDesc
{
	HFONT			hFont;
	unsigned int	width;
	unsigned int	height;

	LPTSTR			pszString;
	unsigned long	strLen;
	unsigned long	color;	
};

/// 특정 텍스쳐 버퍼중에서 할당공간 정보
struct sTextureLineDesc
{
	/// key값
	unsigned int			height;

	/// 남아있는 사용 가능 넓이
	unsigned int			availableWidth;

	/// 할당받을 X좌표
	unsigned int			currentPosX;

	/// 가리키는 라인의 위치값
	unsigned int			posY;

	/// 참조 텍스쳐
	//	D3DTexturePtr			pTexture;
	NiMeshScreenElements*	pNiTexture;

	/// sContainerAE* 핸들
	void*					pSearchHandle;

	sTextureBufferDesc*		pBufferDesc;

	sTextureLineDesc*		pNext;

	sTextureLineDesc()
	{
		pSearchHandle = 0;
		pBufferDesc = 0;

		pNext = 0;

		//		pTexture = 0;
		pNiTexture = 0;
	}
	~sTextureLineDesc()
	{
		pSearchHandle = 0;
		pBufferDesc = 0;

		pNext = 0;

		//		pTexture = 0;
		pNiTexture = 0;
	}
};

struct sTextureBlockDesc
{
	unsigned int			posX;
	unsigned int			posY;
	unsigned int			width;
	unsigned int			height;

	/// 참조 텍스쳐
	//	D3DTexturePtr			pTexture;
	NiMeshScreenElements*	pNiTexture;
	RECT					rect;

	/// sCacheKey 핸들
	void*					pHashHandle;
	sTextureBufferDesc*		pBufferDesc;

	sTextureBlockDesc*		pNext;

	sTextureBlockDesc()
	{
		pHashHandle = 0;
		pBufferDesc = 0;

		pNext = 0;

		//		pTexture = 0;
		pNiTexture = 0;
	}
	~sTextureBlockDesc()
	{
		pHashHandle = 0;
		pBufferDesc = 0;

		pNext = 0;

		//		pTexture = 0;
		pNiTexture = 0;
	}

	D3DTexturePtr GetDxTexture()
	{
		if( pNiTexture == 0 )
			return 0;
		NiTexturingProperty* prop = (NiTexturingProperty*)pNiTexture->GetProperty( NiProperty::TEXTURING );
		if( prop == 0 )
			return 0;
		NiSourceTexture* tex = (NiSourceTexture*)prop->GetBaseTexture();
		if( tex == 0 )
			return 0;

		NiDX9TextureData* pRenderData = (NiDX9TextureData*)tex->GetRendererData();
		if( pRenderData == 0 )
			return 0;

		return (D3DTexturePtr)pRenderData->GetD3DTexture();
	}
};

/// 텍스쳐 버퍼 정보
struct sTextureBufferDesc
{
	int						testnum;
	/// 원본
	//	D3DTexturePtr			pTexture;
	NiMeshScreenElementsPtr	pNiTexture;

	/// 남아있는 사용 가능 높이
	unsigned int			availableHeight;

	/// 할당받을 Y좌표
	unsigned int			posY;

	sTextureLineDesc*		pLineDesc;
	sTextureBlockDesc*		pBlockDesc;

	/// 원형 리스트 구성을 위한 변수
	sTextureBufferDesc*		pNext;

	sTextureBufferDesc()
	{
		//		pTexture = 0;

		pLineDesc = 0;
		pBlockDesc = 0;

		pNext = 0;

		pNiTexture = 0;
	}
	~sTextureBufferDesc()
	{
		//		pTexture = 0;

		pLineDesc = 0;
		pBlockDesc = 0;
		pNext = 0;

		pNiTexture = 0;
	}
};

class NiDX9Renderer;
class cBinSearchAE;

class cCacheAgent
{
public:
	cCacheAgent();
	~cCacheAgent();

	/// 초기화
	bool Initialize( unsigned int maxCacheItemNum, unsigned int texWidth, unsigned int texHeight, unsigned int texNum );

	/// 2D 폰트 캐쉬를 획득
	NiMeshScreenElements* GetCache( RECT* pRect, sFontPropertyDesc* pDesc );

protected:
	/// 이미지를 생성한다..
	void CreateBitmapFromText( HFONT hFont, HBITMAP& hBitmap, LPTSTR string, unsigned long strLen, unsigned long color, RECT* pRect, unsigned long fontHeight, VOID** pOut );

	void DDBToDIB( HDC hDC,HBITMAP hBitmap, unsigned char* pOut );

	/// 새로 작성된 문자열을 검색하여 없다면 새로 생성하는 루틴을 제공한다.
	sTextureBlockDesc* AddCache( RECT* pRect, sFontPropertyDesc* pDesc );

	sTextureBlockDesc*	AllocTextureBlock( unsigned int width, unsigned int height );
	sTextureLineDesc*	AllocLineDesc( unsigned int height );
	void				ChangeTextureBuffer();

protected:
	/// 캐쉬 버퍼
	cCacheBuffer*			mpCacheHashBuffer;

	typedef tPool<sTextureBlockDesc> cBlockDescPool;
	typedef tPool<sTextureLineDesc> cLineDescPool;

	cBlockDescPool*			mpBlockDescPool;
	cLineDescPool*			mpLineDescPool;

	cBinSearchAE*			mpLineDescSearch;


	unsigned long			mMaxTextureBlockDescNum;
	unsigned long			mMaxLineDescNum;

	/// 원형 리스트 시작점이면서, 주 버퍼
	sTextureBufferDesc*		mpTextureBufferDesc;

	/// 현재 할당되어 사용중인 버퍼
	sTextureBufferDesc*		mpPrimaryBufferDesc;

	/// 원형 리스트 구성정보 변수
	unsigned long			mTextureBufferNum;
	unsigned long			mMaxCacheItemNum;


	/// 문자를 출력하기 위한 변수
	unsigned int		mTexWidth;
	unsigned int		mTexHeight;

//	unsigned char*		mpImageBuffer;
	unsigned char		mImageDepth; /// 깊이 16 or 32 -> 2byte or 4byte

	HDC					mhDC;
	HDC					mhMemDC;
	HBRUSH				mhBrush;
};
