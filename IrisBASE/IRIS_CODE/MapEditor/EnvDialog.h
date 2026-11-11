/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.09
* 내    용 : 환경 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cEnvCameraDialog;
class cEnvLevelDistanceDialog;
class cEnvSkyDialog;
class cEnvDirLightDialog;

/// 환경 다이얼로그
class cEnvDialog : public CDialog
{
	DECLARE_DYNAMIC(cEnvDialog)

public:
	cEnvDialog();
	virtual ~cEnvDialog();

	cEnvCameraDialog* GetCameraDialog() const;
	cEnvLevelDistanceDialog* GetLevelDistanceDialog() const;
	cEnvSkyDialog* GetSkyDialog() const;
	cEnvDirLightDialog* GetDirLightDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cEnvCameraDialog* mCameraDialog;
	cEnvLevelDistanceDialog* mLevelDistanceDialog;
	cEnvSkyDialog* mSkyDialog;
	cEnvDirLightDialog* mDirLightDialog;
};

inline
cEnvCameraDialog* cEnvDialog::GetCameraDialog() const
{
	return mCameraDialog;
}

inline
cEnvLevelDistanceDialog* cEnvDialog::GetLevelDistanceDialog() const
{
	return mLevelDistanceDialog;
}

inline
cEnvSkyDialog* cEnvDialog::GetSkyDialog() const
{
	return mSkyDialog;
}

inline
cEnvDirLightDialog* cEnvDialog::GetDirLightDialog() const
{
	return mDirLightDialog;
}
