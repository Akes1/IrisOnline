/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.20
* 
* 내    용 : 스킨 다이얼로그
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "../resource.h"

class cSkinContainerResource;
class cSkinDialogResource;

/// 스킨 다이얼로그
class cSkinDialog : public CDialog
{
public:
	cSkinDialog( CWnd* parent = 0 );
	virtual ~cSkinDialog();

public:
	/// 모달 다이얼로그 생성
	INT_PTR DoModal( cSkinContainerResource* resource );

	/// 모달리스 다이얼로그 생성
	BOOL Create( cSkinContainerResource* resource );

protected:
	/// 폰트 설정
	void SetFont( const LOGFONT& lf );
	void SetFont( LPCTSTR faceName, int size, BYTE charset = 0 );

protected:
	/// 리소스 설정
	void SetResource( cSkinDialogResource* resource );

public:
	virtual BOOL PreTranslateMessage( MSG* msg );

protected:
	/// 메시지
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

private:
	enum { IDD = IDD_DIALOG_TEMPLATE };

protected:
	/// 리소스
	cSkinDialogResource* mResource;

	/// 영역
	CRect mRect;

	/// 크기
	int mWidth;
	int mHeight;

	/// 메모리 DC
	CDC mMemDC;

	/// 비트맵
	CBitmap* mOldBitmap;

	/// 폰트
	CFont mFont;
};
