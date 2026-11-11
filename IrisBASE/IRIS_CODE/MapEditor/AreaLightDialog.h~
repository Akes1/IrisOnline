/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.09
* 내    용 : 영역 조명 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/ColorBox.h"

/// 영역 조명 다이얼로그
class cAreaLightDialog : public CDialog
{
	DECLARE_DYNAMIC(cAreaLightDialog)

public:
	cAreaLightDialog();
	virtual ~cAreaLightDialog();

	void UpdateSky( const NiColor& ambient, float dimmer );
	void UpdateTerrain( const NiColor& ambient, float dimmer );
	void UpdateStaticObject( const NiColor& ambient, float dimmer );
	void UpdateDynamicObject( const NiColor& ambient, float dimmer );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedSkyAmbientBox();
	//afx_msg void OnClickedSkyDiffuseBox();
	afx_msg void OnClickedTerrainAmbientBox();
	//afx_msg void OnClickedTerrainDiffuseBox();
	afx_msg void OnClickedStaticObjectAmbientBox();
	//afx_msg void OnClickedStaticObjectDiffuseBox();
	afx_msg void OnClickedDynamicObjectAmbientBox();
	//afx_msg void OnClickedDynamicObjectDiffuseBox();
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );

private:
	/// 색 상자
	cColorBox mSkyAmbientBox;
	//cColorBox mSkyDiffuseBox;
	cColorBox mTerrainAmbientBox;
	//cColorBox mTerrainDiffuseBox;
	cColorBox mStaticObjectAmbientBox;
	//cColorBox mStaticObjectDiffuseBox;
	cColorBox mDynamicObjectAmbientBox;
	//cColorBox mDynamicObjectDiffuseBox;
};
