/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.24
* 내    용 : 포탈 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Button.h"
#include "NumericEdit.h"

#include "Resource.h"

class cPortalInfo;
class cPortalSceneNode;

/// 포탈 다이얼로그
class cPortalDialog : public CDialog
{
	DECLARE_DYNAMIC(cPortalDialog)

public:
	cPortalDialog();
	~cPortalDialog();

	void SelectNode( cPortalSceneNode* node );

	/// 데이타 갱신
	void UpdateList( const tMap<unsigned int, cPortalInfo*>& infoMap );
	void UpdateNodeTransform( cPortalSceneNode* node );

	/// 활성화 여부
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 회전
	float GetRotateAngle() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	afx_msg void OnClickedSave();
	afx_msg void OnClickedCreate();
	afx_msg void OnClickedDelete();
	afx_msg void OnSelchangeNode();
	afx_msg void OnSelchangeTargetIndex();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	afx_msg void OnCheckedButton();
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;
	cButton mCreateButton;
	cButton mDeleteButton;
	
	/// 인덱스
	cNumericEdit mIndexNum;

	/// 범위
	cNumericEdit mRangeNum;

	/// 이동
	cNumericEdit mTxNum;
	cNumericEdit mTyNum;

	/// 회전
	cNumericEdit mRzNum;

	/// 이름 높이값
	cNumericEdit mNameHeightNum;

	///
	cPortalInfo* mSelInfo;
};

inline
void cPortalDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mRangeNum.SetEnabled( true );
		mTxNum.SetEnabled( true );
		mTyNum.SetEnabled( true );
		mRzNum.SetEnabled( true );
	}
	else
	{
		GetDlgItem(IDC_EDIT_PORTAL_INDEX)->SetWindowText( "" );

		mRangeNum.SetEnabled( false );
		mTxNum.SetEnabled( false );
		mTyNum.SetEnabled( false );
		mRzNum.SetEnabled( false );
	}
}

inline
NiPoint3 cPortalDialog::GetTranslate() const
{
	float x = mTxNum.GetValue() * 100;
	float y = mTyNum.GetValue() * 100;
	return NiPoint3( x, y, 0.0f );
}

inline
float cPortalDialog::GetRotateAngle() const
{
	return D3DXToRadian( mRzNum.GetValue() );
}
