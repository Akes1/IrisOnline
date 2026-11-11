/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.09
* 내    용 : 방향광 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/ColorBox.h"
#include "UI/Button.h"
#include "UI/NumericEdit.h"

/// 방향광 다이얼로그
class cEnvDirLightDialog : public CDialog
{
	DECLARE_DYNAMIC(cEnvDirLightDialog)

public:
	cEnvDirLightDialog();
	~cEnvDirLightDialog();

	/// 갱신
	void Update( NiDirectionalLight* light0, NiDirectionalLight* light1 );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedStaticObjAmbientBox();
	afx_msg void OnClickedStaticObjDiffuseBox();
	afx_msg void OnClickedStaticObjRotateFromCamera();
	afx_msg void OnClickedDynamicObjAmbientBox();
	afx_msg void OnClickedDynamicObjDiffuseBox();
	afx_msg void OnClickedDynamicObjRotateFromCamera();
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );
	afx_msg LRESULT OnChangedTransform( WPARAM id, LPARAM );

private:
	/// 정적 오브젝트
	cColorBox mStaticObjAmbientBox;
	cColorBox mStaticObjDiffuseBox;
	cButton mStaticObjRotFromCameraButton;
	cNumericEdit mStaticObjRxEdit;
	cNumericEdit mStaticObjRyEdit;
	cNumericEdit mStaticObjRzEdit;

	/// 동적 오브젝트
	cColorBox mDynamicObjAmbientBox;
	cColorBox mDynamicObjDiffuseBox;
	cButton mDynamicObjRotFromCameraButton;
	cNumericEdit mDynamicObjRxEdit;
	cNumericEdit mDynamicObjRyEdit;
	cNumericEdit mDynamicObjRzEdit;
};
