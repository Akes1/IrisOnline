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
class cSkinProgressBarResource;

/// 스킨 진행바
class cSkinProgressBar : public CStatic
{
public:
	cSkinProgressBar();
	virtual ~cSkinProgressBar();

	/// 생성
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	/// [0.0, 1.0] 범위의 값을 설정
	void SetValue( float value );
	float GetValue() { return mValue; }

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:
	/// 리소스
	cSkinProgressBarResource* mResource;

	/// 크기
	int mWidth;
	int mHeight;

	/// 색상
	COLORREF mColor;
	COLORREF mBackColor;

	/// 값 [0.0, 1.0]
	float mValue;
};

inline
void cSkinProgressBar::SetValue( float value )
{
	mValue = value;
}
