/* ==========================================================================
* 파    일 : HeroStateWindow.h
* 목    적 : 캐릭터 상태 창( 화면 왼쪽 상단 )
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-08
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cGauge;
class cDoubleGauge;
class cUIImage;
class cArcGauge;

const int MAX_IMAGE = 4;
class cHeroStateWindow : public cUIWindow
{
public:
	cHeroStateWindow();
	virtual ~cHeroStateWindow();

	void Open();
	void Close();

	void InitHeroInfo();

	void UpdateHeroInfo();
	void UpdateHeroHPMP();
	void UpdateExpSxp();

	///
	void SetLevel( int level );
    void SetMAXSXP( float value );		/// MAX SXP
	void SetSXP( float value );			///		SXP
	void SetMaxEXPGauge( float value );	/// MAX EXP	
	void SetEXPGauge( float value );	///		EXP

	void ShowLeaderImage( bool show );
	bool ShowNewMail( bool show );

private:
	bool OnCreate( cUINodeProperty* pproperty );
	bool HandleEvent( const cUIEvent& event );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnMouseLeft( const cUIPos& pos );
	void OnMouseMove( const cUIPos& pos );
	void UpdateSkin();

	/// 종족과 성별로 이미지 인덱스 찾기
	char GetImageIndex( char race, char gender );

private:

	cUINodeSkin*	mpHeroStateWinSkin;

	/// 얼굴 이미지
	cUINodeSkin*	mpFaceSkin;

	cLabel*		mpFace;

	/// Hero 이름
	cLabel*		mpName;
	
	/// Hero 레벨
	cStringT	mLevel;
	cUIImage*	mpLevelImage;
	cUINodeSkin* mpLevelSkin;

	///
	cDoubleGauge*	mpHPGauge;
	cDoubleGauge*	mpMPGauge;
	cGauge*			mpSXPGauge;
	cGauge*			mpEXPGauge;

	/// 파티장 이미지
	cUIPos			mLeaderPos;
	bool			mShowLeader;
	cUIImage*		mpLeaderImage;

	/// 새 메일 알림
	bool			mShowNewMail;
	cLabel*			mpNewMailLabel;

	/// 직업 이미지
	cUIImage*		mpJobImage;
	cUINodeSkin*	mpJobSkin;
	bool			mIsShowJobTip;
};
