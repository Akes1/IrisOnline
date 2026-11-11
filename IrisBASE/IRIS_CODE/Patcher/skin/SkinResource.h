/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.19
* 
* 내    용 : 스킨 리소스
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( disable: 4244 )

#include <afxtempl.h>

class cSkinComponentResource;
class cSkinContainerResource;
class cSkinDialogResource;
class cSkinButtonResource;
class cSkinProgressBarResource;
class cSkinImageSlideResource;
class cSkinHtmlViewResource;
class cSkinManager;

/// 스킨 콤포넌트 리소스
class cSkinComponentResource
{
	friend class cSkinManager;

public:
	cSkinComponentResource();
	virtual ~cSkinComponentResource();

public:
	/// 비트맵
	CBitmap mBitmap;

	/// 영역
	CRect mRect;

	/// 크기
	int mWidth;
	int mHeight;

protected:
	/// 비트맵 파일 로딩
	bool LoadImage( CBitmap* bitmap, int* width, int* height, const cString& fileName );

private:
	/// 경로
	static cString mPath;
};

/// 스킨 컨테이너 리소스
class cSkinContainerResource : public cSkinComponentResource
{
public:
	cSkinContainerResource();
	virtual ~cSkinContainerResource();

	/// 로딩
	virtual bool Load( cParser& parser ) = 0;

	/// 자식을 리턴
	cSkinComponentResource* GetChild( const cString& name );

public:
	/// 콤포넌트 리소스 맵
	typedef tHashMap<cString, cSkinComponentResource*> cComponentResourceMap;
	cComponentResourceMap mComponentResourceMap;
};

/// 스킨 다이얼로그 리소스
class cSkinDialogResource : public cSkinContainerResource
{
public:
	cSkinDialogResource();
	virtual ~cSkinDialogResource();

	/// 로딩
	virtual bool Load( cParser& parser );

	const cString& GetText( unsigned int id );

protected:
	bool LoadText( cParser& parser );

public:
	/// 폰트
	LOGFONT mFont;

	typedef tHashMap<unsigned int, cString> cStringMap;
	cStringMap mStringMap;
	cString mErrorTxt;
};

/// 스킨 버튼 리소스
class cSkinButtonResource : public cSkinComponentResource
{
public:
	cSkinButtonResource();
	virtual ~cSkinButtonResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 스타일 (0 - 푸시, 1 - 체크 박스)
	int mStyle;
};

/// 스킨 진행바 리소스
class cSkinProgressBarResource : public cSkinComponentResource
{
public:
	cSkinProgressBarResource();
	virtual ~cSkinProgressBarResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 색상
	COLORREF mColor;
	COLORREF mBackColor;
};

/// 스킨 원형 게이지 리소스
class cSkinRoundGaugeResource : public cSkinComponentResource
{
public:
	cSkinRoundGaugeResource();
	virtual ~cSkinRoundGaugeResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 화살표
	CBitmap mArrowBitmap;
	//CRect mArrowRect;
};

/// 스킨 이미지 슬라이드 리소스
class cSkinImageSlideResource : public cSkinComponentResource
{
public:
	cSkinImageSlideResource();
	virtual ~cSkinImageSlideResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 비트맵 배열
	CArray<CBitmap> mBitmapArray;
};

/// 스킨 Html 뷰 리소스
class cSkinHtmlViewResource : public cSkinComponentResource
{
public:
	cSkinHtmlViewResource();
	virtual ~cSkinHtmlViewResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// Url
	cString mUrl;
};

class cSkinComboBoxResource : public cSkinComponentResource
{
public:
	cSkinComboBoxResource();
	virtual ~cSkinComboBoxResource();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	CRect mRectLeft;
	CRect mRectCenter;
	CRect mRectRight;

	CBitmap mScrollBitmap;

	int mScrollImageWidth;
	int mScrollImageHeight;

};

/// 스킨 관리자
class cSkinManager
{
public:
	cSkinManager( const cString& path );
	~cSkinManager();

	/// 로딩
	bool Load( const cString& fileName );

	/// 컨테이너 리소스를 리턴
	cSkinContainerResource* GetContainer( const cString& name );

private:
	/// 컨테이너 리소스 맵
	typedef tHashMap<cString, cSkinContainerResource*> cContainerResourceMap;
	cContainerResourceMap mContainerResourceMap;
};
