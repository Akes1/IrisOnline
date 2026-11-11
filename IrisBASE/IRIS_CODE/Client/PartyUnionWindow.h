/* ==========================================================================
* 파    일 : PartyUnionWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-06-05
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Party_Common.h"

class cUnionIcon;
class cUIImage;
class cPartyUnionWindow : public cUIWindow
{
public:
	cPartyUnionWindow();
	virtual ~cPartyUnionWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	/// 정보 갱신
	void SetInfo( unsigned int i, LPCTSTR name, unsigned int maxhp, unsigned int hp, unsigned short job, unsigned char level );
	void UpdateInfo( unsigned int i, unsigned int maxhp, unsigned int hp );
	void ClearInfo( unsigned int i );

	///
	void SetLeader( unsigned int i );
	void SetSelect( unsigned int i );
	void SetPvpLeader( unsigned int i ); /// 전장에서 수장 셋팅
	void SetFlagPlayer( unsigned int i ); /// 플래그매치에서 깃발 든 유저 표시

	/// 리더 기능 적용
	void EnableLeader( bool enable );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* , unsigned int );
	void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );
	void OnIconClicked( cUINode* , unsigned int );
	void OnIconRightClicked( cUINode* , unsigned int , bool );
	void UpdateRect();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	bool			mEnableIcon;

	cUnionIcon*		mIcon[MAX_PARTYUNION];

	//
	cUIImage*		mpLeaderImage;	  /// 연합장표시
	cUIImage*		mpSelectImage;
	cUIImage*		mpPvpLeaderImage;  /// 전장에서 수장표시
	cUIImage*		mpFlagPlayerImage; /// 플래그 매치에서 깃발 소유자 표시

	// 
	unsigned int	mLeaderArrIndex;
	unsigned int	mSelectArrIndex;
	unsigned int	mPvpLeaderArrIndex;
	unsigned int	mFlagPlayerArrIndex;
};

//----------------------------------------------------
// 연합윈도우 접기
class cPartyUnionSmallWindow : public cUIWindow
{
public:
	cPartyUnionSmallWindow();
	virtual ~cPartyUnionSmallWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
};
