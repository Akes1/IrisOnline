/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.20
* 
* 내    용 : 
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSkinComponentResource;
class cSkinImageSlideResource;

/// 스킨 이미지 슬라이드
class cSkinImageSlide : public CStatic
{
public:
	cSkinImageSlide();
	virtual ~cSkinImageSlide();

	/// 생성
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	/// 이미지 인덱스를 설정
	void SetImageIndex( unsigned int index );

protected:
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:
	/// 리소스
	cSkinImageSlideResource* mResource;

	/// 크기
	int mWidth;
	int mHeight;

	/// 이미지 수
	unsigned int mNumImages;

	/// 이미지 인덱스
	unsigned int mImageIndex;
};

inline
void cSkinImageSlide::SetImageIndex( unsigned int index )
{
	if( index < mNumImages )
	{
		mImageIndex = index;
	}
}
