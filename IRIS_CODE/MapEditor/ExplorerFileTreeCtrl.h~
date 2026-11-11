/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.03
* 내    용 : 탐색기 윈도우에 들어가는 파일 트리 컨트롤
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/FileTreeCtrl.h"

/// 탐색기 파일 트리 컨트롤
class cExplorerFileTreeCtrl : public cFileTreeCtrl
{
public:
	cExplorerFileTreeCtrl();
	virtual ~cExplorerFileTreeCtrl();

protected:
	/// 팝업 메뉴를 띄움
	virtual void PopMenu( CPoint point );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLoadProject();
	afx_msg void OnCreateObject();
	afx_msg void OnSelectObject();
	afx_msg void OnCreateSound();
	afx_msg void OnOpenTerrain();
	afx_msg void OnOpenScene();
	afx_msg void OnAddScene();
	afx_msg void OnOpenSound();
	afx_msg void OnOpenNaviMesh();
	afx_msg void OnOpenNaviField();
	afx_msg void OnLoadAreaGroup();
	afx_msg void OnLoadOscr();
	afx_msg void OnLoadTerrainTexture();
	afx_msg void OnImportHeightMap();
	afx_msg void OnImportAlphaMap();
	afx_msg void OnImportColorMap();
};
