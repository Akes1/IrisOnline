/* ==========================================================================
* 파    일 : PartyUnionManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-06-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Party_Common.h"
#include "UnionIcon.h"

const unsigned long PARTYUNION_DATAUPDATE = 100;
const unsigned long PARTYUNION_POSUPDATE = 1000;


class cPlayer;
class cPartyUnionManager
{
public:
	static cPartyUnionManager* mpSingleton;

public:
	cPartyUnionManager();
	~cPartyUnionManager();

	void Close();
	void Clear();
	void Process( unsigned long accumTime );

	/// 네트워크 메시지
	void NetworkMsgParser( char* msg );

	bool TryToMovePlayer( unsigned char i0, unsigned char i1 );
	bool TryToChangeLeader( unsigned long playerIndex );

	//
	void LinkIcon( cUnionIcon* icon );
	void UnLinkIcon();

	sUnionUserData* GetData( int i );

	bool IsHeroLeader();
	bool IsHeroParty( unsigned long userIndex );

	// ui 클릭으로 인한 타겟팅
	void SetTargetByUI( unsigned long userIndex );

	// hero의 타겟팅으로 인함
	void SetTargetByHero( unsigned long userIndex );

	// 타겟팅 클리어
	void ClearTarget();

	//
	int GetGroup( unsigned long userIndex );
	
	void ShowUI();
	void HideUI();

	void ChangeDisplayType( unsigned char type );

	// 큰 연합창과 작은연합창은 같이 열리지 않는다
	void ShowLargeUnionUI( bool show );

	void ClearAllBuff( unsigned long objectIdx );
	void AddBuff( unsigned long objectIdx, unsigned long uniqueIdx, unsigned long influenceIdx );
	void DeleteBuff( unsigned long objectIdx, unsigned long uniqueIdx );
	void DeleteDeBuff( unsigned long objectIdx, unsigned long uniqueIdx );

	void SetPvpLeader( unsigned long characterIndex ); /// 전장에서 수장 표시.
	void SetFlagPlayer( unsigned long characterIndex ); /// 플래그 매치에서 깃발 가지고 있는 유저 표시.
	void ClearFlagPlayer();	/// 플래그 매치에서 깃발 가지고 있는 유저 표시 삭제.

	LPCTSTR GetPlayerName( unsigned long characterIndex );

protected:
	bool AddPlayer( unsigned int i, sUnionUserData* data );
	bool DeletePlayer( unsigned int i );
	bool ChangePlayer( unsigned int i0, unsigned int i1 );
	void UpdateLeader();

	void UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned char level, unsigned int maxhp, unsigned int maxmp,
					   unsigned int hp, unsigned int mp, float posX, float posY );
	void UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned int maxhp, unsigned int hp, float posX, float posY );

	bool SetGroupList();					/// 파티UI 리스트세팅
	bool UpdateGroup( unsigned int i );	/// 파티UI 정보 갱신
	bool UpdateGroupBuff( unsigned int i, sInfluenceClient* data, long count );
	bool UpdateGroupBuff( unsigned int i, cPlayer* player );
	
	void UpdateMouseHover();
	void UpdateMinimap();

	/// hero 그룹인지 판단
	bool IsHeroGroup( int i );

	/// hero 그룹 정보 변경
	void ChangeHeroGroup( int i );

	//
	int FindUser( unsigned long playerIndex );

	//
	void ShowStateUI( int i );
	void HideStateUI( int i );

public:
	static cPartyUnionManager* GetSingleton();

private:
	unsigned long mIndex;

	/// 파티장 유저 인덱스
	unsigned long mLeaderIndex;

	/// 파티원 정보
	sUnionUserData mParty[MAX_PARTYUNION];

	/// HERO 가 포함된 그룹 정보
	ePARTYUNION_GROUP	mGroup;

	/// 링크 아이콘
	cUnionIcon* mpLinkedIcon;

	//
	unsigned int mTempSlot;

	// 시간 갱신
	unsigned long mHPAccumTime;
	unsigned long mPosAccumTime;

	/// 전장에서 수장 유저 인덱스
	unsigned long mPvpLeaderIndex;

	/// 플래그 매치에서 깃발을 가진 유저 인덱스
	unsigned long mFlagPlayerIndex;
};

inline sUnionUserData* cPartyUnionManager::GetData( int i )
{
	if( i < 0 || i >= MAX_PARTYUNION )
	{
		assert(0);
		return 0;
	}
	return &mParty[i];
}

inline cPartyUnionManager* cPartyUnionManager::GetSingleton()
{
	return mpSingleton;
}

#define PARTYUNIONMAN cPartyUnionManager::GetSingleton()
