/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.06.18
* 내    용 : 스킨 원형 게이지
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSkinComponentResource;
class cSkinRoundGaugeResource;

/// 스킨 원형 게이지
class cSkinRoundGauge : public CStatic
{
public:
	cSkinRoundGauge();
	virtual ~cSkinRoundGauge();

	/// 생성
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	void SetAngle( float degree );

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );

	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:
	/// 리소스
	cSkinRoundGaugeResource* mResource;

	/// 크기
	int mWidth;
	int mHeight;

	/// 각도
	float mAngle;

	/// 메모리 비트맵
	CBitmap mMemBitmap;
};

inline
void cSkinRoundGauge::SetAngle( float ang )
{
	if( ang < 0.0f )
		mAngle = 0.0f;
	else if( ang > 180.0f )
		mAngle = 180.0f;
	else
		mAngle = ang;
}
