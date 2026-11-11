/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 탭 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"
#include "UI/Button.h"
#include "UI/TabButton.h"
#include "UI/TabButtonGroup.h"
#include "UI/NumericEdit.h"

class cTerrainBuildingDialog;
class cTerrainPaintingDialog;
class cObjectDialog;
class cLightDialog;
class cSoundDialog;
class cAreaDialog;
class cEnvDialog;
class cNaviMeshDialog;
class cNaviFieldDialog;
class cSettingDialog;

/// 탭 윈도우
class cTabWindow : public CFormView
{
	static cTabWindow* mSingleton;
	DECLARE_DYNCREATE(cTabWindow)

public:
	cTabWindow();
	virtual ~cTabWindow();

	/// 카메라
	void LoadCamera( const cString& pathName );
	void SelectCamera( unsigned int i );
	void SetDefaultCameraFarDist( float farDist );

	/// 다이얼로그
	cTerrainBuildingDialog* GetTerrainBuildingDialog() const;
	cTerrainPaintingDialog* GetTerrainPaintingDialog() const;
	cObjectDialog* GetObjectDialog() const;
	cLightDialog* GetLightDialog() const;
	cSoundDialog* GetSoundDialog() const;
	cAreaDialog* GetAreaDialog() const;
	cEnvDialog* GetEnvDialog() const;
	cNaviMeshDialog* GetNaviMeshDialog() const;
	cNaviFieldDialog* GetNaviFieldDialog() const;
	cSettingDialog* GetSettingDialog() const;

	/// 해당 버튼의 체크 여부
	void SetButtonChecked( int id, bool checked );

protected:
	virtual void DoDataExchange(CDataExchange* dx);
	virtual void OnInitialUpdate();

public:
	void OnEscape();

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeCamera();
	afx_msg void OnClickedZoomPersp();
	afx_msg void OnClickedZoomOrtho();
	afx_msg void OnClickedNewCamera();
	afx_msg void OnClickedDeleteCamera();
	afx_msg void OnClickedLoadCamera();
	afx_msg void OnClickedSaveCamera();
	afx_msg LRESULT OnChangedFarDist( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cTabWindow* GetSingleton();

private:
	/// 버튼
	cTabButton mTerrainBuildingButton;
	cTabButton mTerrainPaintingButton;
	cTabButton mObjectButton;
	cTabButton mLightButton;
	cTabButton mSoundButton;
	cTabButton mAreaButton;
	cTabButton mEnvButton;
	cTabButton mNaviMeshButton;
	cTabButton mNaviFieldButton;
	cTabButton mSettingButton;

	/// 카메라
	cButton mZoomPerspButton;
	cButton mZoomOrthoButton;
	cButton mNewCameraButton;
	cButton mDelCameraButton;
	cButton mLoadCameraButton;
	cButton mSaveCameraButton;
	cNumericEdit mFarDist;

	/// 다이얼로그
	cTerrainBuildingDialog* mTerrainBuildingDialog;
	cTerrainPaintingDialog* mTerrainPaintingDialog;
	cObjectDialog* mObjectDialog;
	cLightDialog* mLightDialog;
	cSoundDialog* mSoundDialog;
	cAreaDialog* mAreaDialog;
	cEnvDialog* mEnvDialog;
	cNaviMeshDialog* mNaviMeshDialog;
	cNaviFieldDialog* mNaviFieldDialog;
	cSettingDialog* mSettingDialog;
};

inline
cTerrainBuildingDialog* cTabWindow::GetTerrainBuildingDialog() const
{
	return mTerrainBuildingDialog;
}

inline
cTerrainPaintingDialog* cTabWindow::GetTerrainPaintingDialog() const
{
	return mTerrainPaintingDialog;
}

inline
cObjectDialog* cTabWindow::GetObjectDialog() const
{
	return mObjectDialog;
}

inline
cLightDialog* cTabWindow::GetLightDialog() const
{
	return mLightDialog;
}

inline
cSoundDialog* cTabWindow::GetSoundDialog() const
{
	return mSoundDialog;
}

inline
cAreaDialog* cTabWindow::GetAreaDialog() const
{
	return mAreaDialog;
}

inline
cEnvDialog* cTabWindow::GetEnvDialog() const
{
	return mEnvDialog;
}

inline
cNaviMeshDialog* cTabWindow::GetNaviMeshDialog() const
{
	return mNaviMeshDialog;
}

inline
cNaviFieldDialog* cTabWindow::GetNaviFieldDialog() const
{
	return mNaviFieldDialog;
}

inline
cSettingDialog* cTabWindow::GetSettingDialog() const
{
	return mSettingDialog;
}

inline
cTabWindow* cTabWindow::GetSingleton()
{
	return mSingleton;
}

#define TABWIN cTabWindow::GetSingleton()
