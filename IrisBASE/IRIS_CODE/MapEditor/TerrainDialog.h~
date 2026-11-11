/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 지형 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cTerrainBuildDialog;
class cTerrainPaintDialog;
class cTerrainOptionDialog;
class cTerrainInfoDialog;

/// 지형 다이얼로그
class cTerrainBuildingDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainBuildingDialog)

public:
	cTerrainBuildingDialog();
	virtual ~cTerrainBuildingDialog();

	/// 자식 다이얼로그를 리턴
	cTerrainBuildDialog* GetBuildDialog() const;
	cTerrainOptionDialog* GetOptionDialog() const;
	cTerrainInfoDialog* GetInfoDialog() const;

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
	cTerrainBuildDialog* mBuildDialog;
	cTerrainOptionDialog* mOptionDialog;
	cTerrainInfoDialog* mInfoDialog;
};

inline
cTerrainBuildDialog* cTerrainBuildingDialog::GetBuildDialog() const
{
	return mBuildDialog;
}

inline
cTerrainOptionDialog* cTerrainBuildingDialog::GetOptionDialog() const
{
	return mOptionDialog;
}

inline
cTerrainInfoDialog* cTerrainBuildingDialog::GetInfoDialog() const
{
	return mInfoDialog;
}

///
class cTerrainPaintingDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainPaintingDialog)

public:
	cTerrainPaintingDialog();
	virtual ~cTerrainPaintingDialog();

	/// 자식 다이얼로그를 리턴
	cTerrainPaintDialog* GetPaintDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cTerrainPaintDialog* mPaintDialog;
};

inline
cTerrainPaintDialog* cTerrainPaintingDialog::GetPaintDialog() const
{
	return mPaintDialog;
}
