/* ==========================================================================
* 파    일 : OptionSheet.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-10-24
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cCheckBox;
class cComboBox;
class cOptionGauge;
class cRadioButton;
class cButton;

enum eVIDEO_MODE
{
	eVIDEO_MODE_DEFAULT = 0,		///해상도 320x240.avi 포멧
	eVIDEO_MODE_640X480,
	eVIDEO_MODE_800X600,
	eVIDEO_MODE_1024X768,
	eVIDEO_MODE_1280X720,
	eVIDEO_MODE_GAME,

	eVIDEO_MODE_MAX,
};


/// 옵션 시트 윈도우 - 환경
class cOptionSheet1 : public cUIWindow
{
public:
	cOptionSheet1( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cOptionSheet1();

	void InitOptionData();
	void ApplyOptionData();
	void CancelOptionData();

	///
	void UpdateOptionData();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnShow();

	void OnCheckBoxChanged(cUINode*, unsigned int id);
	void OnRadioPressed(cUINode*, unsigned int id );

protected:
	cCheckBox* mpShowHeroNameCard;
	cRadioButton* mShowHeroTotal[2];

	cCheckBox* mpShowPlayerNameCard;
	cRadioButton* mShowPlayerTotal[2];

	cCheckBox* mpShowNPCNameCard;
	cCheckBox* mpShowMonsterNameCard;
	cCheckBox* mpShowGauge;
	cCheckBox* mpShowBubble;
	cCheckBox* mpShowHelmet;
	cCheckBox* mpShowTip;

	cCheckBox* mpRejectionTrade;
	cCheckBox* mpRejectionDuel;
	cCheckBox* mpRejectionWhisper;
	cCheckBox* mpRejectionParty;
	cCheckBox* mpRejectionFriend;
	cCheckBox* mpRejectionPvP;

	bool mOptionChange;
	bool mHelmetStateChange;
};

/// 옵션 시트 윈도우 - 그래픽/사운드
class cOptionSheet2 : public cUIWindow
{
public:
	cOptionSheet2( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cOptionSheet2();

	void InitOptionData();
	void ApplyOptionData();
	void CancelOptionData();

	///
	void UpdateOptionData();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnShow();

	virtual void OnCheckBoxChanged(cUINode*, unsigned int id);
	virtual void OnComboSelected( cUINode* caller, unsigned int id );
	virtual void OnOptiongaugeUpdate( cUINode*, unsigned int id );

protected:
	void UpdateResolutionComboData(); ///지원 해상도를 combo box 에 셋팅

protected:
	cCheckBox* mpWindowMode;

	cComboBox* mpWindowSize;
	
	cComboBox* mpVideoMode;

	cOptionGauge* mpLight;
	cOptionGauge* mpBackSound;
	cOptionGauge* mpEffSound;

	bool mGraphicChange; ///그래픽 해상도 변경
	bool mSpinMoved;	 ///볼륨조정
	bool mVideoChanged;  ///비디오 녹화 모드 변경

	float mDefaultRamp;
	float mDefaultEnvVol;
	float mDefaultEffVol;

};


/// 옵션 시트 윈도우 - 그래픽/사운드
class cOptionSheet3 : public cUIWindow
{
public:
	cOptionSheet3( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cOptionSheet3();

public:

	void InitOptionData();
	void ApplyOptionData();
	void CancelOptionData();
	void UpdateOptionData();

	inline bool IsPressAutoSendCheck();
	void SetResetBtnEnable( bool bEnable );	

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnShow();

	virtual void OnCommand( cUINode* caller, unsigned int id );
	virtual void OnCheckBoxChanged(cUINode* caller, unsigned int id);

protected:

	cCheckBox*	mpAutoSendMsg;
	cButton*	mpAccountRegist;
	cButton*	mpAccountRefresh;

	bool		mCheckStateChange;

};