/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 사운드 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cSoundTransformDialog;
class cSoundPropertyDialog;
class cSoundInfoDialog;

/// 사운드 다이얼로그
class cSoundDialog : public CDialog
{
	DECLARE_DYNAMIC(cSoundDialog)

public:
	cSoundDialog();
	virtual ~cSoundDialog();

	/// 자식 다이얼로그를 리턴
	cSoundTransformDialog* GetTransformDialog() const;
	cSoundPropertyDialog* GetPropertyDialog() const;
	cSoundInfoDialog* GetInfoDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

	virtual void OnOK() {}
	virtual void OnCancel() {}

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cSoundTransformDialog* mTransformDialog;
	cSoundPropertyDialog* mPropertyDialog;
	cSoundInfoDialog* mInfoDialog;
};

inline
cSoundTransformDialog* cSoundDialog::GetTransformDialog() const
{
	return mTransformDialog;
}

inline
cSoundPropertyDialog* cSoundDialog::GetPropertyDialog() const
{
	return mPropertyDialog;
}

inline
cSoundInfoDialog* cSoundDialog::GetInfoDialog() const
{
	return mInfoDialog;
}
