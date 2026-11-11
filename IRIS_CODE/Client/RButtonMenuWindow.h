#pragma once
#include "UIWindow.h"

const int MENU_GAPX = 4;
const int MENU_GAPY = 9;

enum ePopupPos
{
	ePopupPos_Character = 0,
	ePopupPos_TargetWindow,
	ePopupPos_PartyWindow,
};

enum ePopupMenuType
{
	ePopupMenuType_Whisper = 1,	/// 귓속말		 800
	ePopupMenuType_Friend,		/// 친구등록	 802
	ePopupMenuType_PartyIn,		/// 파티초대	 803
	ePopupMenuType_PartyLeader,	/// 파티장 위임	 804
	ePopupMenuType_PartyOut,	/// 파티원 추방	 805
	ePopupMenuType_Duel,		/// 결투 신청	 806
	ePopupMenuType_Trade,		/// 거래 요청	 807
	ePopupMenuType_GuildIn,		/// 길드 초대	 808
	ePopupMenuType_Follow,		/// 따라가기     809
	ePopupMenuType_UnionLeader,	/// 연합장 위임  810
	ePopupMenuType_Camael,		/// 자동사냥신고 811
	ePopupMenuType_PvPReport,	/// 전장신고	 812
	ePopupMenuType_LookPlayer,	/// 살펴보기	 813
};

class cListBox;
class cLabel;
class cPlaneObject;

class cRButtonMenuWindow : public cUIWindow
{
public:
	cRButtonMenuWindow();
	virtual ~cRButtonMenuWindow();

	virtual void Open();
	virtual void Close();

	void PopUp( int posX, int posY, unsigned long playerIdx, LPCTSTR playerName, ePopupPos caller );

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnRender( cUIFontItemKeeper* fontKeeper );

	void OnMouseOtherClick( const cUIPos& pos );
	void OnListBoxChanged( cUINode* caller, unsigned int id );
	void OnHide();

	void Clear();

protected:
	enum
	{
		eBACKIMAGE_TOP = 0,
		eBACKIMAGE_CENTER,
		eBACKIMAGE_BOTTOM,
		eBACKIMAGE_LEFT,
		eBACKIMAGE_RIGHT,

		eBACKIMAGE_MAX
	};

	cListBox*	mpMenuList;
	cLabel*		mpName;

	/// 배경 이미지
	cUINodeSkin*	mpBackSkin[eBACKIMAGE_MAX];
	cPlaneObject*	mpBackImage[eBACKIMAGE_MAX];

	unsigned long mPlayerIdx;
	cStringT mPlayerName;

	unsigned int	mInitWidth;
};

// NPC 대화하기 버튼
class cTalkButtonWindow : public cUIWindow
{
public:
	cTalkButtonWindow();
	virtual ~cTalkButtonWindow();

	void Open() {}

	bool Create( const cString& skinName );

	void SetNpcIndex( unsigned long npcIndex ) { mNpcIndex = npcIndex; }

	void Show( unsigned long px, unsigned long py );
	void Show( bool onsound = true ) { assert(0); }

	void HideESC( bool onsound = true ) {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );

private:
	unsigned long mNpcIndex;
};
