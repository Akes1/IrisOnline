/* ==========================================================================
* 파    일 : UnionIcon.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-06-05
* 주의사항 :
*===========================================================================*/
#pragma once
#include "BaseIcon.h"

const int UNIONGAUGE_WIDTH = 117;

// 연합정보 아이콘
class cGauge;
class cUnionIcon : public cBaseIcon
{
public:
	cUnionIcon( eUINodeType type = eUINODE_ICON );
	virtual ~cUnionIcon();

	/// 정보 세팅 함수
	void SetInfo( LPCTSTR name, unsigned int maxhp, unsigned int hp, unsigned short job, unsigned char level );

	/// 정보 삭제 함수
	void ClearInfo();

	/// 게이지변경 정보
	void UpdateGaugeInfo( unsigned int maxhp, unsigned int hp );

	inline unsigned int GetMaxHP() { return mMaxValueHP; }
	inline unsigned int GetHP() { return mValueHP; }
	inline LPCTSTR GetName() { return (LPCTSTR)mName.Cstr(); }
	inline unsigned short GetJob() { return mJob; }
	inline unsigned char GetLevel() { return mLevel; }

protected:
	bool HandleEvent( const cUIEvent& event );
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnMouseHovered(const cUIPos& pos );
	void OnMouseLeft( const cUIPos& pos );

private:
	/// 정보 세팅 여부
	bool mIsSetInfo;

	// 이름
	cStringT mName;
	
	//직업 레벨
	cStringT mJobAndLv;

	// HP 게이지
	cUIImage* mpGauge;

	// 게이지 값
	cUIPos	mGaugePos;
	unsigned int mHeight;
	unsigned int mMaxValueHP;
	unsigned int mValueHP;
	unsigned short mJob;
	unsigned char mLevel;

	// 이름 좌표
	cUIPos mNamePos;
};
