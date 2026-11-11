/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.10
* 내    용 : 몬스터 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Button.h"
#include "NumericEdit.h"
#include "CheckListCtrl.h"

#include "Resource.h"

class cMonsterInfo;
class cMonsterGroup;
class cMonsterSceneNode;
class cPattern;

/// 몬스터 다이얼로그
class cMonsterDialog : public CDialog
{
	DECLARE_DYNAMIC(cMonsterDialog)

public:
	cMonsterDialog();
	~cMonsterDialog();

	///
	void Clear();

	///
	void SelectGroup( cMonsterGroup* group );
	void SelectNode( cMonsterSceneNode* node );

	/// 데이타 갱신
	void UpdateCombo( const tMap<unsigned int, cMonsterInfo*>& infoMap );
	void UpdateGroupList( const tMap<unsigned int, cMonsterGroup*>& groupMap );
	void UpdateList( const tList<cMonsterSceneNode*>& nodeList );
	void UpdateNodeTransform( cMonsterSceneNode* node );
	void UpdateFamilyList( tMap<unsigned int, void*>* pFamilyMap );

	/// 활성화 여부
	void SetGroupEnabled( bool enabled );
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 회전
	float GetRotateAngle() const;

	void UpdatePatternCombo( tPointerHashMap<unsigned int, cPattern*>* infoMap );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	afx_msg void OnClickedSave();
	afx_msg void OnClickedGroupCreate();
	afx_msg void OnClickedGroupDelete();
	afx_msg void OnSelchangeGroup();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheckedGroupFirstRegen();
	afx_msg void OnCheckedChannelCheck();
	afx_msg void OnClickedCreate();
	afx_msg void OnClickedChange();
	afx_msg void OnClickedDelete();
	afx_msg void OnClickedFamilyNumNew();
	afx_msg void OnClickedFamilyNumChange();
	afx_msg void OnSelchangeMonsterPatternIndex();
	afx_msg void OnSelchangeNode();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mSaveButton;

	/// 몬스터 그룹
	cNumericEdit mGroupIndexNum;
	cNumericEdit mMapGroupIndex;
	cButton mGroupCreateButton;
	cButton mGroupDeleteButton;
	cNumericEdit mGroupRegenPercentNum;
	cNumericEdit mGroupRegenRangeNum;
	cNumericEdit mGroupRegenWaitTimeNum;

	cMonsterGroup* mSelMonsterGroup;

	/// 몬스터
	cButton mCreateButton;
	cButton mChangeButton;
	cButton mDeleteButton;
	cNumericEdit mTxNum;
	cNumericEdit mTyNum;
	cNumericEdit mRzNum;
	cNumericEdit mRegenWaitTimeNum;
	cNumericEdit mRegenLifeTimeNum;

	///
	CCheckListCtrl	mNextRegenGroupList;
	bool			mInitedNextRegenGroupList;

	/// 동족
	cNumericEdit mFamilyNum;
	cButton mFamilyNewButton;
	cButton mFamilyChangeButton;	

	cMonsterSceneNode* mSelMonster;
};

inline
void cMonsterDialog::SetGroupEnabled( bool enabled )
{
	if( enabled )
	{
		mMapGroupIndex.SetEnabled( true );
		mGroupRegenPercentNum.SetEnabled( true );
		mGroupRegenRangeNum.SetEnabled( true );
		mGroupRegenWaitTimeNum.SetEnabled( true );
	}
	else
	{
		mMapGroupIndex.SetEnabled( false );
		mGroupRegenPercentNum.SetEnabled( false );
		mGroupRegenRangeNum.SetEnabled( false );
		mGroupRegenWaitTimeNum.SetEnabled( false );
	}
}

inline
void cMonsterDialog::SetEnabled( bool enabled )
{

	mTxNum.SetEnabled( enabled );
	mTyNum.SetEnabled( enabled );
	mRzNum.SetEnabled( enabled );
	mRegenWaitTimeNum.SetEnabled( enabled );
	mRegenLifeTimeNum.SetEnabled( enabled );
	
	mFamilyNum.SetEnabled( enabled );
	mFamilyNewButton.EnableWindow( enabled );
	mFamilyChangeButton.EnableWindow( enabled );

	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER_FAMILY);
	list->EnableWindow( enabled );

	if( enabled == false )
	{
		GetDlgItem(IDC_EDIT_MONSTER_INDEX)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_MONSTER_NAME)->SetWindowText( "" );
		GetDlgItem(IDC_EDIT_MONSTER_MODEL)->SetWindowText( "" );
	}

	mNextRegenGroupList.EnableWindow( enabled );
}

inline
NiPoint3 cMonsterDialog::GetTranslate() const
{
	float x = mTxNum.GetValue() * 100;
	float y = mTyNum.GetValue() * 100;
	return NiPoint3( x, y, 0.0f );
}

inline
float cMonsterDialog::GetRotateAngle() const
{
	return D3DXToRadian( mRzNum.GetValue() );
}
