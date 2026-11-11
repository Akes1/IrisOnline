#pragma once

#include "Button.h"
#include "NumericEdit.h"

#include "Resource.h"

class cGatheringInfo;
class cGatheringGroup;
class cGatheringSceneNode;


class cGatheringDialog : public CDialog
{
	DECLARE_DYNAMIC(cGatheringDialog)

public:
	cGatheringDialog();
	~cGatheringDialog();

	///
	void Clear();

	///
	void SelectGroup( cGatheringGroup* group );
	void SelectNode( cGatheringSceneNode* node );

	/// 데이타 갱신
	void UpdateCombo( const tMap<unsigned int, cGatheringInfo*>& infoMap );
	void UpdateGroupList( const tMap<unsigned int, cGatheringGroup*>& groupMap );
	void UpdateList( const tList<cGatheringSceneNode*>& nodeList );
	void UpdateNodeTransform( cGatheringSceneNode* node );

	/// 활성화 여부
	void SetGroupEnabled( bool enabled );
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 회전
	float GetRotateAngle() const;

protected:
	virtual void DoDataExchange(CDataExchange* dx);
	BOOL OnInitDialog();

protected:
	afx_msg void OnClickedSave();
	afx_msg void OnClickedGroupCreate();
	afx_msg void OnClickedGroupDelete();
	afx_msg void OnSelchangeGroup();
	afx_msg void OnSelchangeGroupRegenIndex();
	afx_msg void OnClickedCreate();
	afx_msg void OnClickedChange();
	afx_msg void OnClickedDelete();
	afx_msg void OnSelchangeNode();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;

	/// 채집 그룹
	cNumericEdit mGroupIndexNum;
	cButton mGroupCreateButton;
	cButton mGroupDeleteButton;
	cNumericEdit mGroupRegenWaitTimeNum;
	cNumericEdit mGroupRegenRandTimeNum;

	cGatheringGroup* mSelGatheringGroup;

	/// 채집 객체
	cButton mCreateButton;
	cButton mChangeButton;
	cButton mDeleteButton;
	cNumericEdit mTxNum;
	cNumericEdit mTyNum;
	cNumericEdit mAzNum;
	cNumericEdit mRzNum;

	cGatheringSceneNode* mSelGathering;
};


inline
void cGatheringDialog::SetGroupEnabled( bool enabled )
{
	if( enabled )
	{
		mGroupRegenWaitTimeNum.SetEnabled( true );
		mGroupRegenRandTimeNum.SetEnabled( true );
	}
	else
	{
		mGroupRegenWaitTimeNum.SetEnabled( false );
		mGroupRegenRandTimeNum.SetEnabled( false );
	}
}

inline
void cGatheringDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mTxNum.SetEnabled( true );
		mTyNum.SetEnabled( true );
		mAzNum.SetEnabled( true );
		mRzNum.SetEnabled( true );
	}
	else
	{
		GetDlgItem(IDC_EDIT_GATHERING_INDEX)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_GATHERING_NAME)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_GATHERING_MODEL)->SetWindowText( "" );

		///
		mTxNum.SetEnabled( false );
		mTyNum.SetEnabled( false );
		mAzNum.SetEnabled( false );
		mRzNum.SetEnabled( false );
	}
}

inline
NiPoint3 cGatheringDialog::GetTranslate() const
{
	float x = mTxNum.GetValue() * 100;
	float y = mTyNum.GetValue() * 100;
	return NiPoint3( x, y, 0.0f );
}

inline
float cGatheringDialog::GetRotateAngle() const
{
	return D3DXToRadian( mRzNum.GetValue() );
}
