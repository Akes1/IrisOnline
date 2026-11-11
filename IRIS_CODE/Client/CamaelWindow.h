#pragma once
#include "UIWindow.h"

class cEditBox;
class cUISkin;
class cUIImage;
class cButton;
class cLabel;

const int CAMAEL_TEXT_LENGTH = 4;

class cCamaelWindow : public cUIWindow
{
public:
	cCamaelWindow(void);
	virtual ~cCamaelWindow(void);

	void Open(){};

	bool SetCaptchaImage( unsigned char* pImageInfo, unsigned long imageSize );
	void SetTimeCheck( bool isCheck );
	void SetEnabledButtons( bool set );
	void SetRefreshButtonEnable( bool set );
	void SetOverRefresh( bool set );
	
	/// 신고하기
	void TryToReport( unsigned long targetIndex = 0 ); 

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* , unsigned int );
	bool HandleEvent( const cUIEvent& event );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnEditBoxEntered( cUINode* , unsigned int );	

protected:

	cEditBox* mpEdit;		/// 정답입력 에디트
	cUIImage* mpTextImg;    /// 문제 이미지
	cButton*  mpOK;			/// 확인
	cButton*  mpRefresh;    /// 새로고침
	cLabel*   mpTime;       /// 남은시간

	cUINodeSkin*  mpImageSkin;  /// 이미지 위치

	bool		  mIsCheckTime;	/// 현재 카마엘 심판 중인지 체크
	unsigned long mAnswerTime;

	bool		  mIsOverRefresh;
	
};
