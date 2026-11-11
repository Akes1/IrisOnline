/* ==========================================================================
* 파    일 : TitleWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-09-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cListBox;
class cTextBox;

/// 호칭 관리 윈도우
class cTitleWindow : public cUIWindow
{
public:
	cTitleWindow();
	virtual ~cTitleWindow();

	virtual void Open() {}

	/// 초기화
	void Clear();

	/// 호칭 추가
	bool AddTitle( unsigned long titleIdx, bool bQuick = false );

	void SelectTitle( unsigned int nListType );

	void Sort();

	void GetQuickTitle( long* pListArr );

	void ClearQuick();	

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnListBoxSelected( cUINode* , unsigned int );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	/// 호칭 전체 리스트
	cListBox*	mpTitleList;

	/// 호칭 퀵 리스트
	cListBox*	mpTitleQuickList;

	/// 호칭 설명 박스
	cTextBox*	mpDescBox;

	/// 효과 설명 박스
	cTextBox*	mpEffectBox;

	/// 선택버튼
	cUINode*	mpSelect;

	unsigned long mSelectedList;
};
