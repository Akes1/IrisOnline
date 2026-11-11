/* ==========================================================================
* 파    일 : MakeWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

enum 
{
	eChange_None = 0,
	eChange_Object,
	eChange_Form,
};

class cEditBox;
class cSpinBox;
class cLabel;
class cButton;
class cPushButton;
class cUIImage;
struct sPlayerInfo;

const unsigned int MAX_RACE = 3;
const unsigned int MAX_JOB = 6;

const unsigned int HAIR_NAMING_START_INDEX = 11300;
const unsigned int HAIRCOLOR_NAMING_START_INDEX = 11400;
const unsigned int FACE_NAMING_START_INDEX	= 12001;

/// 회전버튼 윈도우
class cMakeRotateWindow : public cUIWindow
{
public:
	cMakeRotateWindow();
	virtual ~cMakeRotateWindow();

protected:
	bool OnCreate(cUINodeProperty* pproperty );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* , unsigned int );
	void OnButtonContinueDowned(cUINode* , unsigned int id );
	void OnPushButtonPressed( cUINode* caller, unsigned int id );
	void UpdateSkin();

private:
	/// 버튼 배경 이미지
	cUIImage*	mpButtonBackImg;
	cUINodeSkin*	mpButtonSkin;

	/// 오른쪾 회전
	cButton* mpRotateRight;

	/// 왼쪽회전
	cButton* mpRotateLeft;

	cPushButton* mpZoomInOut;

};

/// 생성정보 라벨 윈도우
class cMakeInfoWindow : public cUIWindow
{
public:
	cMakeInfoWindow();
	virtual ~cMakeInfoWindow();

	void Open();

	void UpdateRaceImage( int i );
	void UpdateJobImage( int i );

protected:
	/// 이벤트
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateSkin();

private:
	cButton*		mpMakeSelect;

	/// 종족 이미지
	cUIImage*	mpRaceImage[MAX_RACE];

	/// 직업 이미지
	cUIImage*	mpJobImage[MAX_JOB];

	cUINodeSkin*	mpRaceSkin;
	cUINodeSkin*	mpJobSkin;

	cUIPos		mRacePos;
	cUIPos		mJobPos;

	int		mCurrentRace;
	int		mCurrentJob;
};

/// 생성 윈도우
class cMakeWindow : public cUIWindow
{
public:
	cMakeWindow();
	virtual ~cMakeWindow();

	void Open();

	/// 생성버튼 활성화 세팅
	void SetEnableCreate( bool enable );

	void GetDisplayInfo( sPlayerInfo* info );
	inline unsigned char CheckDisplayUpdate();

	void SetMakeInfoWindow( cMakeInfoWindow* win );

protected:
	/// 이벤트
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnShow();
	void OnHide();

	///
	void ResetSpinBox();
	void ResetHairSpin();
	void ResetFaceSpin();
	void ResetHairColorSpin();

protected:
	unsigned char	mDisplayUpdate;

protected:
	/// 이름
	cEditBox*	mpName;

	/// 헤어 스핀박스
	cSpinBox* mpHairSpin;
	cSpinBox* mpHairColorSpin;

	/// 얼굴 스핀 박스 
	cSpinBox* mpFaceSpin;

	/// gender 스핀 박스 
	cSpinBox* mpGenderSpin;
	
	/// 종족 스핀 박스 
	cSpinBox* mpRaceSpin;

	/// 직업 스핀 박스 
	cSpinBox* mpJobSpin;

	/// 캐릭터 생성 버튼 
	cButton* mpCreate;

	///
	cUISize mUpdaetSize;

	///
	cMakeInfoWindow*	mpInfoWindow;
};

inline unsigned char cMakeWindow::CheckDisplayUpdate()
{
	unsigned char r = mDisplayUpdate;
	if( mDisplayUpdate != eChange_None )
		mDisplayUpdate = eChange_None;
	return r;
}

inline void cMakeWindow::SetMakeInfoWindow( cMakeInfoWindow* win )
{
	if( win )
		mpInfoWindow = win;
}
