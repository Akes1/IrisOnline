/* ==========================================================================
* 파    일 : Guild.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-10-31
* 주의사항 :
*===========================================================================*/
#pragma once

const unsigned long	GUILD_TTL_TICK = 1000 * 60 * 3;	// 접속유지 동기화 시간

/// 길드
class cGuild
{
public:
	cGuild();
	~cGuild();

	void Clear();
	
	bool AddPlayer( cPlayer* player, char position, DWORD channelNum );
	bool AddPlayer( long characterIdx, char level, long job, short mapnum, wchar_t* name, char position, DWORD channelNum );
	bool DeletePlayer( unsigned long characterIdx );

	/// 길드생성시 기본정보 세팅
	void SetInfo( long idx, long masterIdx, wchar_t* name, wchar_t* masterName, TIMESTAMP_STRUCT initTime );

	/// 길드장 설정
	void SetMaster( unsigned long characterIdx, wchar_t* name );

	/// 공지내용 설정
	void SetNotice( wchar_t* notice );
	
	/// 마크 갱신
	void UpdateMark( TIMESTAMP_STRUCT markDate, BYTE* mark, int size );

	// 정보 갱신
	void UpdateInfo( sGuildConnect* data, unsigned long tickCount );

	// 접속 유지 정보 갱신 안됨
	unsigned long GetLostPlayer( unsigned long tickCount );

	/// 서버시작에 세팅되는 내용
	void VerifySetInfo( TB_GUILD* info );	
	bool VerifyAddPlayer( TB_GUILD_USER* data );

	TB_GUILD*	GetGuildInfo() { return &mInfo; }
	unsigned long GetGuildIndex() { return mInfo.idx; }
	tPointerHashMap<unsigned long, void*>* GetGuildUser() { return &mGuildUserMap; }
	unsigned long GetGuildSize() { return mGuildUserMap.GetSize(); }
	wchar_t* GetGuildName();
	unsigned long GetGuildMasterIndex() { return mInfo.masterIndex; }
	TIMESTAMP_STRUCT GetGuildMarkDate() { return mInfo.markDate; }
	
	void SetAccumTime( unsigned long time ) { mLastAccumTime = time; }
	unsigned long GetAccumTime() { return mLastAccumTime; }

	void SetDBAccumTime( unsigned long time ) { mDBLastAccumTime = time; }
	unsigned long GetDBAccumTime() { return mDBLastAccumTime; }

private:
	/// 길드 정보
	TB_GUILD		mInfo;

	/// 길드 유저정보 (TB_GUILD_USER)
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;
	cGuildUserMap	mGuildUserMap;

	unsigned long	mLastAccumTime;		/// 클라이언트 최신 send 시간
	unsigned long	mDBLastAccumTime;	/// DB 갱신 시간
};

/// 길드 정보 복사
inline void cGuild::VerifySetInfo( TB_GUILD* info )
{
	mInfo = *info;
}

inline wchar_t* cGuild::GetGuildName()
{
	return mInfo.name;
}
