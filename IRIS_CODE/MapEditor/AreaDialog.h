/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.09
* 내    용 : 영역 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cAreaTransformDialog;
class cAreaLightDialog;
class cAreaFogDialog;
class cAreaEtcDialog;

/// 영역 다이얼로그
class cAreaDialog : public CDialog
{
	DECLARE_DYNAMIC(cAreaDialog)

public:
	cAreaDialog();
	virtual ~cAreaDialog();

	/// 자식 다이얼로그를 리턴
	cAreaTransformDialog* GetTransformDialog() const;
	cAreaLightDialog* GetLightDialog() const;
	cAreaFogDialog* GetFogDialog() const;
	cAreaEtcDialog* GetEtcDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cAreaTransformDialog* mTransformDialog;
	cAreaLightDialog* mLightDialog;
	cAreaFogDialog* mFogDialog;
	cAreaEtcDialog* mEtcDialog;
};

inline
cAreaTransformDialog* cAreaDialog::GetTransformDialog() const
{
	return mTransformDialog;
}

inline
cAreaLightDialog* cAreaDialog::GetLightDialog() const
{
	return mLightDialog;
}

inline
cAreaFogDialog* cAreaDialog::GetFogDialog() const
{
	return mFogDialog;
}

inline
cAreaEtcDialog* cAreaDialog::GetEtcDialog() const
{
	return mEtcDialog;
}
