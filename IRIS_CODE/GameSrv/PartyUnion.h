/* ==========================================================================
* 파    일 : PartyUnion.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-06-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Party_Common.h"

class cPartyUnion
{
public:
	cPartyUnion();
	~cPartyUnion();

	void* operator new( size_t n );
	void operator delete( void* ptr, size_t n );

	unsigned long* GetUserArr();
	unsigned long* GetGroupArr( unsigned long userIdx );

	inline unsigned int GetCount() { return mCount; }

	void SetIndex( unsigned long index );
	void SetLeader( unsigned long userIdx );
	inline unsigned long GetIndex() { return mIndex; }
	inline unsigned long GetLeader() { return mLeaderIndex; }
	
	int AddUser( unsigned long userIdx );
	bool DeleteUser( unsigned long userIdx );
	bool MoveUser( unsigned char i0, unsigned char i1 );

	int FindNextLeader();
	bool IsUser( unsigned long userIdx1, unsigned long userIdx2 );

	inline void SetLastAccumTime( unsigned long accumTime ) { mLastAccumTime = accumTime; }
	inline unsigned long GetLastAccumTime() { return mLastAccumTime; }

	int GetGroup( unsigned long userIdx );	// 그룹인덱스 찾기

protected:
	int GetGroup( int i );					// 그룹인덱스 찾기

private:
	// 파티 인덱스
	unsigned long mIndex;

	// 파티 유저인덱스
	unsigned long mUserArr[ MAX_PARTYUNION ];

	// 파티연합장
	unsigned long mLeaderIndex;

	// 파티 인원수
	unsigned int mCount;
	
	/// 파티 갱신 시간
	unsigned long mLastAccumTime;

};

inline unsigned long* cPartyUnion::GetUserArr()
{
	return mUserArr;
}

inline void cPartyUnion::SetIndex( unsigned long index )
{
	mIndex = index;
}

inline void cPartyUnion::SetLeader( unsigned long userIdx )
{
	mLeaderIndex = userIdx;
}

