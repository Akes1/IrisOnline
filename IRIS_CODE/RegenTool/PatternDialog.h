#pragma once

#include "Button.h"
#include "NumericEdit.h"

#include "Resource.h"

class cPattern;
class cPatternSceneNode;
struct sPatternPos;

/// 패턴 다이얼로그
class cPatternDialog : public CDialog
{
	DECLARE_DYNAMIC(cPatternDialog)

public:
	cPatternDialog();
	~cPatternDialog();

	void SetEnabled( bool enabled );
	void SetPatternEnabled( bool enabled );
	void SetPositionEnabled( bool enabled );
	void Clear();

	void UpdatePatternList( tPointerHashMap<unsigned int, cPattern*>* pPatternMap );
	void SelectPattern( cPattern* pPattern );

	void UpdatePosList( tPointerList<cPatternSceneNode*>* nodelist );
	void SelectPatternPos( cPatternSceneNode* node );	

	bool IsCheckedNew();
	bool IsCheckedUpper();
	bool IsCheckedModify();

	void SetCheckNew( bool check );
	void SetCheckModify( bool check );

	cPatternSceneNode* GetSelectedPos();

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	afx_msg void OnSelchangePattern();
	afx_msg void OnClickedSave();
	afx_msg void OnClickedPatternCreate();
	afx_msg void OnClickedPatternDelete();
	afx_msg void OnCheckedPatternCirculate();
	afx_msg void OnSelchangePatternPosition();
	afx_msg void OnClickedPatternPositionDelete();
	afx_msg void OnCheckedPatternPositionRestingplace();
	afx_msg void OnCheckedPatternPosNew();
	afx_msg void OnCheckedPatternPosUpper();
	afx_msg void OnCheckedPatternPosModify();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;

	/// 패턴 리스트
	cNumericEdit mPatternIndex;
	cButton mPatternCreate;
	cButton mPatternDelete;

	/// 위치정보
	cButton mPositionDelete;
	cNumericEdit mPositionRange;
	cNumericEdit mPositionWaitTime;

	cPattern* mSelPattern;
	cPatternSceneNode* mSelPatternPosNode;
};

inline
void cPatternDialog::SetPatternEnabled( bool enabled )
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_UPPER );
	check->EnableWindow( enabled );
	check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_NEW );
	check->EnableWindow( enabled );
}

inline
void cPatternDialog::SetPositionEnabled( bool enabled )
{
	/// 위치정보
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_MODIFY );
	check->EnableWindow( enabled );
	mPositionDelete.EnableWindow( enabled );

	mPositionRange.SetEnabled( enabled );
	check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_RESTINGPLACE );
	check->EnableWindow( enabled );
	mPositionWaitTime.SetEnabled( enabled );
}

inline
void cPatternDialog::SetEnabled( bool /*enabled*/ )
{
	/*SetPatternEnabled( enabled );
	SetPositionEnabled( enabled );*/
}