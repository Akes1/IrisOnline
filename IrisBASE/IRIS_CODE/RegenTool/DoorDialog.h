#pragma once

#include "Button.h"
#include "NumericEdit.h"

#include "Resource.h"

class cDoorInfo;
class cDoorSceneNode;

class cDoorDialog : public CDialog
{
	DECLARE_DYNAMIC(cDoorDialog)

public:
	cDoorDialog();
	virtual ~cDoorDialog();

	void UpdateList( const tMap<unsigned int, cDoorInfo*>& infoMap );
	void UpdateNodeTransform( cDoorSceneNode* node );

	void SelectNode( cDoorSceneNode* node );

	/// 활성화 여부
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 회전
	float GetRotateAngle() const;

protected:
	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* dx);    // DDX/DDV 지원입니다.

protected:
	afx_msg void OnClickedSave();
	afx_msg void OnClickedCreate();
	afx_msg void OnClickedDelete();
	afx_msg void OnSelchangeNode();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;
	cButton mCreateButton;
	cButton mDeleteButton;

	/// 인덱스
	cNumericEdit mIndexNum;

	/// 제한 라인
	cNumericEdit mRectT_WidthNum;
	cNumericEdit mRectB_WidthNum;
	cNumericEdit mRectL_HeightNum;
	cNumericEdit mRectR_HeightNum;

	cNumericEdit mRectAppendNum;

	/// 이동
	cNumericEdit mTxNum;
	cNumericEdit mTyNum;

	cNumericEdit mAzNum;

	/// 회전
	cNumericEdit mRzNum;
};


inline
void cDoorDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mRectT_WidthNum.SetEnabled( true );
		mRectB_WidthNum.SetEnabled( true );
		mRectL_HeightNum.SetEnabled( true );
		mRectR_HeightNum.SetEnabled( true );

		mRectAppendNum.SetEnabled( true );

		mTxNum.SetEnabled( true );
		mTyNum.SetEnabled( true );
		mAzNum.SetEnabled( true );
		mRzNum.SetEnabled( true );
	}
	else
	{
		GetDlgItem(IDC_EDIT_DOOR_INDEX)->SetWindowText( "" );

		mRectT_WidthNum.SetEnabled( false );
		mRectB_WidthNum.SetEnabled( false );
		mRectL_HeightNum.SetEnabled( false );
		mRectR_HeightNum.SetEnabled( false );

		mRectAppendNum.SetEnabled( false );

		mTxNum.SetEnabled( false );
		mTyNum.SetEnabled( false );
		mAzNum.SetEnabled( false );
		mRzNum.SetEnabled( false );
	}
}

inline
NiPoint3 cDoorDialog::GetTranslate() const
{
	float x = mTxNum.GetValue() * 100;
	float y = mTyNum.GetValue() * 100;
	return NiPoint3( x, y, 0.0f );
}

inline
float cDoorDialog::GetRotateAngle() const
{
	return D3DXToRadian( mRzNum.GetValue() );
}
