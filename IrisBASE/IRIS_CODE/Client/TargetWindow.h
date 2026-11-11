/* ==========================================================================
* 파    일 : TargetWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-28
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cGauge;
class cEffectSceneNode;
class cBaseObject;
class cTargetBuff;
class cUIImage;

class cTargetWindow : public cUIWindow
{
public:

	enum
	{
		eGauge_Hero = 0,
		eGauge_Monster,

		eGauge_Max
	};

	cTargetWindow();
	virtual ~cTargetWindow();

	virtual void Open();
	virtual void Close();

	void ClearBuff();

	// 파티관련 갱신
	void UpdateParty();
	void UpdateWindow();
	
	/// 게이지 차는 속도 조절
	void SetHPGaugeSpeed( float s );

	/// 타겟 버프 아이콘 세팅
	void AddBuff( unsigned long uniqueIdx, unsigned long influenceIdx );
	void DelBuff( unsigned long uniqueIdx );
	void DelDeBuff( unsigned long uniqueIdx );

	/// 윈도우 사이즈 조정
	void UpdateSize();

protected:
	/// 처리
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender(cUIFontItemKeeper* pKeeper );
	virtual void OnRButtonUp( const cUIPos& pos );
	virtual void OnShow();
	virtual void OnHide();
	virtual void UpdateRect();
	virtual void UpdateSkin();
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	virtual void OnMouseMove( const cUIPos& pos );	
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

	/// 타겟 정보 세팅 
	void SetTargetInfo( char level, LPCTSTR name );
	void SetTargetEffect( cBaseObject* pObj );

protected:
	/// 타겟 이름
	cLabel* mpTargetNameLabel;

	/// 타겟 레벨
	cLabel* mpTargetLevelLabel;

	/// 타겟 hp 게이지 틀
	cGauge*	 mpHPGauge;

	bool	 mShowHPImage;

	/// 타겟 이펙트
	unsigned long		mTargetEffectIdx[2];
	cEffectSceneNode*	mpTargetEffect[2];

	/// 타겟 버프
	cUINodeSkin*	mpNodeSkin;
	cTargetBuff*	mpTargetBuff;
	cUIRect			mpBuffRc;

	/// 타겟 직업 표시(이미지 표시용)
	cLabel* mpTargetJobLabel;

	/// 타겟 NPC 직업 표시(이미지 표시용)
	cLabel* mpTargetNpcJobLabel;

	/// 타겟이 NPC 일 경우 게이지 대신 보여주는 직업 라벨
	cLabel* mpTargetNpcJobLabelText;

	bool			mIsShowToolTip;

	// 
	cLabel*		mpIsParty;
	cLabel*		mpIsOtherParty;

	cUIPos	mTargetTexPos[eGauge_Max];
};


////////////////////////////////////////////////////
/// 타겟 버프
#include "PartyBuff.h"

/// 타겟 버프 윈도우
class cTargetBuff : public cPartyBuff
{
public:
	cTargetBuff();
	virtual ~cTargetBuff();

protected:
	bool SetSkin(const cUINodeSkin* pskin );
	void UpdateSkin();
	void UpdateRect();
	void UpdateWindowSize();
};
