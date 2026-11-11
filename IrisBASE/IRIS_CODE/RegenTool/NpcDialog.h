/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.10
* 내    용 : Npc 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Button.h"
#include "NumericEdit.h"

#include "Resource.h"

class cNpcInfo;
class cNpcSceneNode;

/// Npc 다이얼로그
class cNpcDialog : public CDialog
{
	DECLARE_DYNAMIC(cNpcDialog)

public:
	cNpcDialog();
	~cNpcDialog();

	void SelectNode( cNpcSceneNode* node );

	/// 데이타 갱신
	void UpdateCombo( const tMap<unsigned int, cNpcInfo*>& infoMap );
	void UpdateList( const tMap<unsigned int, cNpcSceneNode*>& nodeMap );
	void UpdateNodeTransform( cNpcSceneNode* node );

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
	afx_msg void OnClickedChange();
	afx_msg void OnClickedDelete();
	afx_msg void OnSelchangeNode();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;
	cButton mCreateButton;
	cButton mChangeButton;
	cButton mDeleteButton;
	
	/// 이동
	cNumericEdit mTxNum;
	cNumericEdit mTyNum;

	/// 회전
	cNumericEdit mRzNum;
};

inline
void cNpcDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mTxNum.SetEnabled( true );
		mTyNum.SetEnabled( true );
		mRzNum.SetEnabled( true );
	}
	else
	{
		GetDlgItem(IDC_EDIT_NPC_INDEX)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_NPC_NAME)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_NPC_MODEL)->SetWindowText( "" );

		///
		mTxNum.SetValue( 0.0f );
		mTyNum.SetValue( 0.0f );
		mRzNum.SetValue( 0.0f );

		mTxNum.SetEnabled( false );
		mTyNum.SetEnabled( false );
		mRzNum.SetEnabled( false );
	}
}

inline
NiPoint3 cNpcDialog::GetTranslate() const
{
	float x = mTxNum.GetValue() * 100;
	float y = mTyNum.GetValue() * 100;
	return NiPoint3( x, y, 0.0f );
}

inline
float cNpcDialog::GetRotateAngle() const
{
	return D3DXToRadian( mRzNum.GetValue() );
}
