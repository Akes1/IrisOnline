/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.10
* 내    용 : 맵 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Button.h"
#include "NumericEdit.h"

struct sMapInfo;

/// 맵 다이얼로그
class cMapDialog : public CDialog
{
	DECLARE_DYNAMIC(cMapDialog)

public:
	cMapDialog();
	~cMapDialog();

	bool IsNaviFieldChecked() const;

	void SetMapInfo( tMap<unsigned int,sMapInfo*>* mapIdxSet );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

protected:
	afx_msg void OnClickedLoad();
	afx_msg void OnCheckedButton();
	afx_msg void OnSelchangeMapIdx();
	afx_msg void OnSelchangeMapDegee();
	DECLARE_MESSAGE_MAP()

private:
	cButton mLoadButton;
};
