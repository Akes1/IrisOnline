/* ==========================================================================
* 파    일 : SkillWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-06-18
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"


class cLabel;
class cUIImage;
class cCheckBox;
class cCommuSheetWindow;
class cSkillStepListWindow;


class cSkillWindow : public cTabWindow
{
public:

	enum eSkillPostImg{

		eSKILL_USE_IMG,
		eSKILL_REST_IMG,
		eSKILL_IMG_MAX

	};
	cSkillWindow();
	virtual ~cSkillWindow();

	virtual void Open();
	virtual void Close();

	void Clear();
	
	void InitSkillData();

	unsigned int AddJobTab( unsigned long jobIdx );
	virtual void SetCurrent( unsigned int index );

	void UpdateSkillStep( unsigned long idx, unsigned char step );
	void StartCoolTime( unsigned long idx, unsigned long restTime, unsigned long totalTime );
	void ClearCoolTime( unsigned long exceptSkillIdx );

	/// 오라스킬 세팅
	void SetOnAura( unsigned long skillIdx, unsigned long AuraUniqueIdx, bool on );
	unsigned long  GetAuraUniqueIdx( unsigned long idx );

	/// 스킬포인트 세팅 
	void UpdateSkillPoint();
	void UpdateSkillUsePoint();

	/// 스킬 래밸 업 버튼 visiable 갱신
	void ShowSkillUpButton();

	virtual void UpdateSkin();

	/// 커뮤니티 스킬 발동 요청
	void RequestCommuSkill( unsigned long skillIndex );
	void RequestLifeCommand( unsigned long comIdx, LPCTSTR value1, LPCSTR value2 );
	void NetUnLockCommu();

	void BackupWindowPos();

	virtual void UpdateRect();

	cSkillStepListWindow*	GetStepListWindow(){ return mpSkillStepListWindow; }
	cCommuSheetWindow*		GetCommunityWindow(){ return mpCommuSheetWindow; }

	void SetPressStepListView( bool bView );
	bool IsShowStepListWin();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnCheckBoxChanged( cUINode* caller, unsigned int id );
	void OnShow();
	void OnHide();
	

	void UpdateCommuButtonPos();

	void ApplyBackupPos();

 	void OnNetLock( int lockTry );
 	void OnNetUnLock( int lockTry );

private:
	/// 스킬 포인트
	cLabel*		mpSkillPoint;
	cLabel*		mpSkillUsePoint;

	// 스킬 포인트 라벨 이미지
//	cUINode*	mpSkillPointImg;

	cUINode*	 mpSkillPointImg[eSKILL_IMG_MAX];

	// 스킬 단계별 표시 창 Visible 버튼
	cCheckBox*	mpVisibleStepWinCheckBtn;

	//	스킬 단계별 표시 윈도우
	cSkillStepListWindow*	mpSkillStepListWindow;

	/// community skill sheet info
	cTabButton*				mpCommuButton;
	cCommuSheetWindow*		mpCommuSheetWindow;

	unsigned char			mJobTabCount;

	cUIRect					mCommuBtnBaseRect;
		
	bool	mIsBackup;
	cUIPos	mBackupPos;

	cUISize mImageSize;
};
