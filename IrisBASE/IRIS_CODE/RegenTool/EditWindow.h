/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 편집 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "RollupWindow.h"

class cCameraDialog;
class cMapDialog;
class cNpcDialog;
class cMonsterDialog;
class cPortalDialog;
class cGatheringDialog;
class cPatternDialog;
class cDoorDialog;

/// 편집 윈도우
class cEditWindow : public CFormView
{
protected:
	cEditWindow();
	DECLARE_DYNCREATE(cEditWindow)

public:
	virtual void OnDraw(CDC* dc);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();

	cMapDialog* GetMapDialog() const;
	cNpcDialog* GetNpcDialog() const;
	cMonsterDialog* GetMonsterDialog() const;
	cPortalDialog* GetPortalDialog() const;
	cGatheringDialog* GetGatheringDialog() const;
	cPatternDialog* GetPatternDialog() const;
	cDoorDialog* GetDoorDialog() const;

public:
	virtual ~cEditWindow();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()
	
private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	///
	cCameraDialog* mCameraDialog;
	cMapDialog* mMapDialog;
	cNpcDialog* mNpcDialog;
	cMonsterDialog* mMonsterDialog;
	cPortalDialog* mPortalDialog;
	cGatheringDialog* mGatheringDialog;
	cPatternDialog* mPatternDialog;
	cDoorDialog* mDoorDialog;
};

inline
cMapDialog* cEditWindow::GetMapDialog() const
{
	return mMapDialog;
}

inline
cNpcDialog* cEditWindow::GetNpcDialog() const
{
	return mNpcDialog;
}

inline
cMonsterDialog* cEditWindow::GetMonsterDialog() const
{
	return mMonsterDialog;
}

inline
cPortalDialog* cEditWindow::GetPortalDialog() const
{
	return mPortalDialog;
}

inline
cGatheringDialog* cEditWindow::GetGatheringDialog() const
{
	return mGatheringDialog;
}

inline
cPatternDialog* cEditWindow::GetPatternDialog() const
{
	return mPatternDialog;
}

inline
cDoorDialog* cEditWindow::GetDoorDialog() const
{
	return mDoorDialog;
}
