/* ==========================================================================
* 파    일 : ItemMixWindow.h
* 목    적 : 아이템 조합 윈도우
* 작 성 자 : 김가람 
* 작 성 일 : 2009-01-09
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ListFolderBox.h"

class cLabel;
class cGauge;
class cComboBox;
class cUIImage;
class cDoubleItemIconListBox;
class cNumberEditBox;
class cItemIcon;
class cButton;
class cCheckBox;
class cItemMixListFolder;
struct sKeepRecipe;

class cItemMixWindow : public cUIWindow
{
public:
	cItemMixWindow();
	virtual ~cItemMixWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	/// 보조직업 갱신
	void UpdateMakeSkill();

	/// 레시피 정보 갱신
	void UpdateRecipeList( unsigned char index, bool bInit = true, bool bPossible = false );
	
	/// 조합 후 리스트 재 갱신 후 선택바 갱신
	void UpdateRecipeList();

	/// 선택바, 결과 재료 갱신
	void UpdateSelectedRecipe();

	/// 조합 할수 있는 아이템 갯수만 갱신
	void UpdateItemCount();		

	// 숙련도 갱신
	void SetMixExp( unsigned char skillIndex, unsigned long exp );

	void StartGauge( float speed );
	void StopGauge();
	void NetGaugeEnd( bool success = true  );

	// 현재 선택되어있는 스킬 
	unsigned char GetSelectSkill();

	void HideESC( bool onsound );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* , unsigned int );
	void OnComboSelected( cUINode* , unsigned int );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnCheckBoxChanged( cUINode* , unsigned int );
	void OnShow();
	void OnHide();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	void InitCoolTime();

	/// 완료 갯수 세팅
	void SetCompleteCount( int count );

	unsigned int GetMaxValue( unsigned long recipeIdx, bool bChatMsg = true );

private:
	cComboBox*				mpSkillCombo;	/// 조합 스킬 콤보박스
	cItemMixListFolder*		mpRecipeList;	/// 레시피
	cDoubleItemIconListBox*	mpItemList;		/// 조합 아이템 정보 리스트박스

	//
	cLabel*		mpCoolTime;			/// 조합 대기시간
	cLabel*		mpCompleteCount;	/// 조합 완성된 갯수

	cLabel*		mpGrade;			// 등급
	cLabel*		mpPoint;			// 숙련도
	cLabel*		mpGoalPoint;		// 목표숙련도

	cGauge*				mpGaugeBar;		/// 조합 게이지
	cNumberEditBox*		mpMixCount;		/// 조합갯수 입력 에디트
	cButton*			mpMax;
	cUIImage*			mpHighImage;	/// 폴더 박스 체크 이미지관리
	
	// 시작
	cButton*	mpMixStart;

	// 중단
	cButton*	mpMixStop;

	// 기술포기
	cButton*	mpMixGiveUp;

	// 요리 가능한 레시피 목록 
	cCheckBox*	mpPossibleRecipeCheck;

	///
	sKeepRecipe*	mpRecipeCool;	/// 레시피 쿨타임

	/// 결과 확률 라벨
	cItemIcon*	mpResultIcon[3];
	cLabel*		mpResultRate[3];

	unsigned int	mAutoRemainCount;
	unsigned int	mAutoCount;
	float			mAutoSpeed;
};




// 퀘스트용 리스트 폴더 박스
class cItemMixListFolder : public cListFolderBox
{
public:
	cItemMixListFolder();
	virtual ~cItemMixListFolder();

	/// extraData를 가지고있는 하위열을 찾아서 리턴 - 접혀있으면 리턴 안함
	int FindColumnByExtra( unsigned long type, unsigned long recipe );

};
