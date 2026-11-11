/* ==========================================================================
* 파    일 : PartyStateWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cUIImage;
class cGauge;
class cLabel;
class cPartyBuff;
enum ePLAYER_JOB;

// 작은 사이즈
class cPartyStateMiniWindow : public cUIWindow
{
public:
	cPartyStateMiniWindow();
	virtual ~cPartyStateMiniWindow();

	virtual void Open();
	virtual void Close();

	virtual void Clear();

	// 처음 생성시 세팅
	virtual void InitPosition( int i );

	// 정보 세팅
	void SetInfo( unsigned long userIdx, LPCTSTR name,
					unsigned long maxhp, unsigned long hp, 
					unsigned long maxmp, unsigned long mp, 
					unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx );

	void UpdateInfo( unsigned long maxhp, unsigned long hp, 
					unsigned long maxmp, unsigned long mp );

	// 맵
	virtual void UpdateMap( unsigned short mapnum, unsigned short roomnum, DWORD channelnum );

	virtual void SetSelect( bool set );

	///
	virtual inline unsigned long GetUserIndex() { return mUserIndex; }

	void SetPvpLeaderVisible( bool show );
	void SetFlagPlayerVisible( bool show );

protected:
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender(cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shiftPress );
	virtual void OnLButtonDoubleClick(const cUIPos& pos );
	virtual void OnRButtonUp( const cUIPos& pos );
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	virtual void UpdateRect();
	virtual void UpdateSkin();

	// 링크 배열 인덱스
	virtual void SetLinkArrIndex( int i ) { mLinkArrIndex = i; }

protected:
	unsigned long mUserIndex;

	// 배열 인덱스
	int mLinkArrIndex;

	// 노드 위치 인덱스
	int mNodeIndex;

	// 맵 번호
	unsigned short mMapNum;
	unsigned short mRoomNum;
	DWORD		   mChannelNum;

	// 파티장
	bool mIsLeader;

	// 선택
	bool mIsSelect;

	//
	cGauge*		mpHPGauge;
	cGauge*		mpMPGauge;
	cLabel*		mpName;

	/// 선택 이미지
	cUIImage*	 mpSelectedImage;

	/// 원래 윈도우 스킨 정보
	cUINodeSkin*	mpNodeSkin;

	/// 파티장 표시
	cUINodeSkin*	mpLeaderSkin;
	cUIImage*		mpLeaderImage;

	/// 전장에서 수장 표시
	cUINodeSkin*	mpPvpLeaderSkin;
	cUIImage*		mpPvpLeaderImage;
	bool			mShowPvpLeader;

	/// 플래그 매치에서 깃발 든 유저 표시
	cUINodeSkin*	mpFlagPlayerSkin;
	cUIImage*		mpFlagPlayerImage;
	bool			mShowFlagPlayerLeader;
};

//----------------------------------------------------------------------------------
/// 파티원 상태 윈도우
class cPartyStateWindow : public cPartyStateMiniWindow
{
public:
	cPartyStateWindow();
	virtual ~cPartyStateWindow();

	void Clear();
	void ClearBuff();

	// 처음 생성시 세팅
	void InitPosition( int i );

	// 부모꺼 사용 (불가
	void SetInfo( unsigned long userIdx, LPCTSTR name,
				unsigned long maxhp, unsigned long hp, 
				unsigned long maxmp, unsigned long mp, 
				unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx );

	// 정보 세팅
	void SetInfo( unsigned long userIdx, LPCTSTR name,
					char level, ePLAYER_JOB job, 
					unsigned long maxhp, unsigned long hp, 
					unsigned long maxmp, unsigned long mp, 
					unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx );

	void UpdateInfo( char level, unsigned long maxhp, unsigned long hp, 
					unsigned long maxmp, unsigned long mp );

	// 맵
	void UpdateMap( unsigned short mapnum, unsigned short roomnum, DWORD channelnum );

	// 직업
	void UpdateJob( ePLAYER_JOB job );

	/// 파티원 버프 아이콘 세팅
	void AddBuff( unsigned long uniqueIdx, unsigned long influenceIdx );
	void DelBuff( unsigned long uniqueIdx );
	void DelDeBuff( unsigned long uniqueIdx );

	void SetLevel( unsigned char level );

	// 버프추가 삭제로 인한 
	void UpdateWindowSize();

	//
	void UpdateMouseHover( cUIPos pos );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnMouseMove( const cUIPos& pos );
	void OnMouseHovered( const cUIPos& pos );
	void OnMouseLeft( const cUIPos& pos );
	void UpdateRect();
	void UpdateSkin();
	void OnShow();
	void OnHide();

protected:
	// 직업 정보
	ePLAYER_JOB	mJob;

	//
	cLabel*		mpLevel;

	/// 버프윈도우
	cPartyBuff*	mpBuff;

	//
	cUIImage*		mpJobImage;
	cUINodeSkin*	mpJobSkin;

	//
	bool	mIsShowMapTip;
	bool	mIsShowJobTip;
};
