/* ==========================================================================
* 파    일 : HeroInfoWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Hero.h"

/*
TODO : 모든 속성치들을 외부에서 설정할수 있도록 해야하며,
텍스트들은 스킨텍스트에서 가져오도록 한다.
남자 + 1 남자, 
그리고 각 속성치에 따른 색변화도 필요하다 
*/

enum eStateInfo_Type;

enum eHeroInfoImage
{
	eHERO_INFO_PHYSICAL_IMAGE,
	eHERO_INFO_MAGIC_IMAGE,
	eHERO_INFO_IMAGE_MAX,
};

class cGauge;
class cDoubleGauge;
class cLabel;
class cRadioButton;
class cUIImage;
class cUIImageFont;

const int HERO_LEVEL_IMAGE_INDEX = 220;

class cHeroInfoWindow : public cUIWindow
{
public:
	cHeroInfoWindow();
	virtual ~cHeroInfoWindow();

	virtual void Open() {}
	virtual void Close();

	// hero 세팅된 후에 초기화해야할 데이터
	void InitHeroInfo();

	void UpdateHeroInfo();
	void UpdateHeroHPMP();
	void UpdateExpSxp();

	void SetEXP( double value );				/// 현재 EXP 세팅 
	void SetMAXEXP( double value );			/// 최대 EXP 세팅 

	void SetSXP( double value );				/// 현재 SXP 세팅 
	void SetMAXSXP( double value );			/// 최대 SXP 세팅 

	void SetSkillLevel( int slevel );		/// SkillLeve 세팅
	
	void SetLevel( int level );				/// 레벨 세팅
	void SetClass( unsigned long job );		/// 클래스
	void SetTitle( LPCTSTR text );			/// 호칭 
	void SetGuild( LPCTSTR text );			/// 길드
	void SetPosition( LPCTSTR text );		/// 길드 직위셋팅
	
	/*
	TODO: 우선 스테이터스 추가량은 색 구별이 확연하므로 타입을 보고
	색을 바꿔주도록 한다.
	일반 스테이터스 수치는 한색으로 동일할 가능성이 크므로,
	스크립트에서 세팅해주거나, 한색으로 세팅하도록해서
	색 인자를 지우자.
	*/
	
	void SetStr( int value, unsigned long color );		/// 힘
	void SetDex( int value, unsigned long color );		/// 민첩
	void SetCon( int value, unsigned long color );		/// 체력
	void SetInt( int value, unsigned long color );		/// 지능
	void SetWis( int value, unsigned long color );		/// 지혜

	/// 추가량은 항상 색변화가 있으므로 반드시 넣어준다 
	void SetPlusStr( int value, eStateInfo_Type type );		/// 힘 추가량
	void SetPlusAgi( int value, eStateInfo_Type type );		/// 민첩 추가량
	void SetPlusCon( int value, eStateInfo_Type type );		/// 체력 추가량
	void SetPlusInt( int value, eStateInfo_Type type );		/// 지능 추가량
	void SetPlusWis( int value, eStateInfo_Type type );		/// 지혜 추가량

	/// 물리수치와 마법수치는 자체적으로 색이 변하므로, 
	/// 노말상태가 있다. 
	void SetAtt( int min, int max, eStateInfo_Type type = eSTATE_NORMAL );	/// 공격력
	void SetDef( int value, eStateInfo_Type type = eSTATE_NORMAL );	/// 방어력
	void SetHit( int value, eStateInfo_Type type = eSTATE_NORMAL );	/// 명중
	void SetAvoid( int value, eStateInfo_Type type = eSTATE_NORMAL );	/// 회피
	void SetMag( int min, int max, eStateInfo_Type type = eSTATE_NORMAL );		/// 마법력
	void SetRes( int value, eStateInfo_Type type = eSTATE_NORMAL );		/// 저항력
	void SetAttackCritical( float value, eStateInfo_Type type = eSTATE_NORMAL );	/// 물리 크리티컬
	void SetMagicCritical( float value, eStateInfo_Type type = eSTATE_NORMAL );	/// 마법 크리티컬
	void SetHPrecovery( float value, eStateInfo_Type type = eSTATE_NORMAL );	/// HP 회복량
	void SetMPrecovery( float value, eStateInfo_Type type = eSTATE_NORMAL );	/// MP 회복량
	void SetFast( int value, eStateInfo_Type type = eSTATE_NORMAL );
	void SetHeal( int value, eStateInfo_Type type = eSTATE_NORMAL );
	void SetMagDef( int value, eStateInfo_Type type = eSTATE_NORMAL );
	void SetMagHit( int value, eStateInfo_Type type = eSTATE_NORMAL );
	void SetPhyProtect( float value, eStateInfo_Type type = eSTATE_NORMAL );	// 물리 방호
	void SetMagProtect( float value, eStateInfo_Type type = eSTATE_NORMAL );	// 마법 방호

	/// 세력 타입
	void SetForceType( unsigned char type );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnRadioPressed( cUINode* , unsigned int );
	void UpdateRect();
	void OnShow();

	/// 스탯 타입을 보고 색을 지정한다 
	unsigned long GetColorForStateType( eStateInfo_Type type );

private:
	/// 각 게이지
	cDoubleGauge* mpHPGauge;
	cDoubleGauge* mpMPGauge;
	cGauge* mpEXPGauge;
	cGauge* mpSXPGauge;

	/// Hero 기본정보
	cLabel* mpName;
	cLabel* mpRace;
	cLabel* mpGender;
	cLabel* mpClass;
	cLabel* mpTitle;
	cLabel* mpGuild;
	cLabel* mpPosition;	/// 직위
	
	/// 래밸  수치
	cUIImageFont* mpLevelFont;
	cUIPos		  mLevelFontPos;

	cLabel* mpSkillLevel;	/// 스킬 레벨

	/// 스테이터스 
	cLabel* mpStr;	/// 힘
	cLabel* mpDex;	/// 민첩
	cLabel* mpCon;	/// 체력
	cLabel* mpInt;	/// 지능
	cLabel* mpWis;	/// 지혜

	/// 스테이터스 추가량 
	cLabel* mpPlusStr;	/// 힘 증가
	cLabel* mpPlusDex;	/// 민첩 증가
	cLabel* mpPlusCon;	/// 체력 증가
	cLabel* mpPlusInt;	/// 지능 증가
	cLabel* mpPlusWis;	/// 지혜 증가

	/// 물리수치
	cLabel* mpAtt;	/// 공격력
	cLabel* mpDef;	/// 방어력
	cLabel* mpHit;	/// 명중
	cLabel* mpAvoid;	/// 회피
	
	/// 마법수치 
	cLabel* mpMag;	/// 마법력
	cLabel* mpRes;	/// 저항력

	cLabel* mpAttackCri;	/// 물리 크리티컬
	cLabel* mpMagicCri;		/// 마법 크리티컬
	cLabel* mpHPRecover;	/// HP 회복량
	cLabel* mpMPRecover;	/// MP 회복량

	cLabel* mpFast;			/// 이동속도
	cLabel* mpHeal;			/// 힐회복량
	cLabel* mpMagDef;		/// 마법방어
	cLabel* mpMagHit;		/// 마법명중

	cLabel* mpForceType;	/// 세력 타입

	cLabel* mpPhyProtect;	/// 물리 방호
	cLabel* mpMagProtect;	/// 마법 방호

	cRadioButton* mpPhysicalButton;
	cRadioButton* mpMagicButton;

	cUINodeSkin* mpInfoImgSkin[eHERO_INFO_IMAGE_MAX];
	cUIImage*	 mpInfoImg[eHERO_INFO_IMAGE_MAX];

	cUINodeSkin* mpLevelPosSkin;
	cString mHeroLevel;

};
