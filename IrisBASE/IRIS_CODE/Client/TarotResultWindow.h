/* ==========================================================================
* 파    일 : TarotResultWindow.h
* 목    적 : 점괘 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cGauge;
class cButton;
class cUIImage;
class cTarotWindow;

class cTarotResultWindow : public cUIWindow
{
public:
	cTarotResultWindow();
	virtual ~cTarotResultWindow();

	/// 타로점괘로 나올수 있는 최대 점수세팅
	void SetMaxScore( unsigned int score );

	/// 카드 해석 
	void SetDescription( LPCTSTR desc );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnShow();
	void OnHide();
	void UpdateSkin();

	/// 오픈 카드 총 수치
	void SetTotalValue();

	/// 카드오픈 버튼 보기
	void ShowButton( bool show );
	void UpdateButton();

private:
	cTarotWindow*		mpMain;

	cLabel*	mpDescription;		/// 카드 해석
	cLabel* mpOpenValue[10];	/// 오픈 카드 수치 총합
	cLabel*	mpOpenTotal;

	cButton*	mpRightBtn;	/// 카드 오픈용 버튼 (오른쪽)
	cButton*	mpUpBtn;	/// 카드 오픈용 버튼(위)
	cButton*	mpResult;	/// 운세 결과받기버튼
	cButton*	mpAllRandom;	/// 모두 랜덤
	cButton*	mpAllStraight;	/// 모두 정방향
	cButton*	mpAllInverse;	/// 모두 역방향
	cButton*	mpOneStraight;	/// 하나만 정방향
	cButton*	mpOneInverse;	/// 하나만 역방향

	/// 점괘수치 게이지
	cGauge*		mpResultGauge;
};



//----------------------------------------------------------------------
class cTarotWindow;
struct sNpcSpread;

class cNpcTarotResultWindow : public cUIWindow
{
public:
	cNpcTarotResultWindow();
	virtual ~cNpcTarotResultWindow();

	void SetDesc( LPCTSTR desc );			/// 카드 해석 삽입
	void SetQuestion( LPCTSTR question );	/// 질문
	void SetCardInfo( LPCTSTR info );		/// 오픈된 카드정보
	void SetComment( LPCTSTR comment );		/// 질문에 대한 해석

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateSkin();
	void OnHide();
	void OnShow();

	void ShowButton( bool show );
	void UpdateButton();

private:
	cTarotWindow*	mpMain;

	cLabel*		mpDesc;
	cLabel*		mpQuestion;
	cLabel*		mpOpenCardInfo;
	cLabel*		mpComment;

	cButton*	mpResult;
	cButton*	mpRightBtn;
	cButton*	mpUpBtn;
	cButton*	mpStraightBtn;
	cButton*	mpInverseBtn;

	sNpcSpread*	mpSpread;
};