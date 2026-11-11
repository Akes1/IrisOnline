/* ==========================================================================
* 파    일 : GuildManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-10-31
* 주의사항 :
*===========================================================================*/
#pragma once

/// 길드 갱신시간
const unsigned long GUILD_UPDATETIME = 60 * 1000;			// 1분
const unsigned long GUILDDB_UPDATETIME = 60 * 60 * 1000;	// 1시간

class cGuild;
class cPlayer;

class cGuildManager
{
	static cGuildManager*	mpSingleton;

public:
	cGuildManager();
	~cGuildManager();

	bool Init();

	/// 해당 유저에게 길드정보 보내기
	void SendGuildInfo( ULONG_PTR socketContext, cPlayer* player, unsigned long idx );
	void SendGuildInfo( cPlayer* player, unsigned long idx );

	// 복구 정보 쏘기
	void SendGuildRestore( unsigned long idx );
	
	void SendGuildUserAdd( cGuild* guild, long characterIdx, char level, long job, short mapnum, wchar_t* name, char position, DWORD channelNum );	/// 길드에 추가 유저 정보 보내주기
	void SendGuildUserOut( cGuild* guild, unsigned long characterIdx );		/// 길드에 탈퇴 유저 정보 보내주기
	void SendGuildUserCut( cGuild* guild, unsigned long characterIdx );		/// 길드에 추방 유저 정보 보내주기
	void SendGuildUpdateInfo( cGuild* guild );								/// 일정시간마다 데이터 갱신후 보내기
	void SendGuildChangePosition( cGuild* guild, unsigned long characterIdx, char position );	/// 길드유저 직위 변경
	bool SendGuildNotice( unsigned long idx, wchar_t* notice );				/// 길드공지 변경 및 보내기
	void SendGuildMarkUpdate( cGuild* guild );								/// 마크 인덱스 변경 보내기
	bool SendGuildMark( unsigned long idx, cPlayer* player );				/// 마크 보내주기
	void SendGuildChat( cPlayer* hero, wchar_t* message, u_int length, unsigned short number );	/// 길드채팅
	void SendGuildUserConnect( unsigned long idx, unsigned long characterIdx, BYTE connect, DWORD channelNum, bool sendConnect );	/// 길드원 접속여부 보내기

	// 채널 SYNC
	void PostGuildUserConnect( unsigned long idx, unsigned long characterIdx, BYTE connect, DWORD channelnum, bool sendConnect );
	void PostGuildTTL( cGuild* guild );
	bool PostGuildCreate( GUILD_CREATE* guildCreate, cPlayer* player );
	bool PostGuildDelete( long idx );
	bool PostGuildUserAdd( long idx, cPlayer* player, short mapnum );
	bool PostGuildUserCut( long idx, long characterIdx );
	bool PostGuildUserOut( long idx, long characterIdx );
	bool PostGuildGivePosition( long idx, long characterIdx, char position );
	bool PostGuildUpdateMark( long idx, TIMESTAMP_STRUCT markRegistDate, BYTE* mark, int size );
	bool PostGuildNotice( long idx, wchar_t* notice );
	bool PostGuildChat( cPlayer* player, wchar_t* message, u_int length, sInventory* inventory );

	/// 전체 길드정보 저장
	void SaveAllDb();

	/// 신청정보 취소
	void ClearRequest( cPlayer* hero );

	/// 플레이어 들어옴 (맵이동)
	void PlayerMapIn( cPlayer* hero );

	/// 채널 정보 갱신
	bool GuildUpdateChannel( unsigned long idx, unsigned long characterIdx, DWORD channelNum, short mapnum );

	unsigned long CheckGuildUser( unsigned long characterIdx );						// 길드원 검사 
	TB_GUILD_USER* IsGuildUser( unsigned long idx, unsigned long characterIdx );	/// 길드원인지 검사

	bool CheckChangePosition( cPlayer* hero, unsigned long characterIdx, char newposition );	/// 직위변경 가능 여부 검사

	/// 길드 정보 갱신 및 전송 플래그
	bool RequestUpdateUser( unsigned long idx, unsigned long characterIdx );

public:
	void Release();
	void Process( unsigned long elasedTime, unsigned long accumTime );

	/// 길드 생성
	bool AddGuild( GUILD_CREATE* data, cPlayer* master );
	bool DeleteGuild( unsigned long idx );

	/// 신청 정보 저장
	bool AddRequest( unsigned long recvIdx, unsigned long reqIdx );
	void DelRequest( unsigned long recvIdx );
	unsigned long GetRequest( unsigned long recvIdx );

	/// 서버, 채널 SYN 메세지
	bool SynAddGuild( void* msg, DWORD channelNum );
	bool SynAddGuildUser( void* msg, DWORD channelNum );
	bool SynCutGuildUser( void* msg );
	bool SynOutGuildUser( void* msg );
	bool SynUpdateMark( void* msg );
	bool SynGuildChat( void* msg );

	/// 초기 길드 정보 생성
	void SetGuildTotalLength( DWORD length );
	void SetGuildLength( DWORD length );

	bool VerifyGuildSelect( TB_GUILD* guild );
	bool VerifyGuildUser( unsigned long idx, TB_GUILD_USER* user );

	///
	cGuild* GetGuild( unsigned long idx );
	wchar_t* GetGuildName( unsigned long idx );
	void GetGuildMarkDate( unsigned long idx, tm& date, bool& useMark );
	bool FindGuildName( wchar_t* name );

	/// 데이터 정확성 검사
	bool CheckGuildLength();
	bool CheckGuildTotal();
	bool CheckGuildSaveTotal();

protected:
	/// 길드정보 DB 업데이트
	void SaveDBGuild( cGuild* guild );

public:
	static cGuildManager* Get();

private:
	/// 길드 맵
	typedef tPointerHashMap<unsigned long, void*> cGuildMap;
	cGuildMap	mGuildMap;

	/// 신청 정보	
	typedef tHashMap<unsigned long, unsigned long> cJoinMap;	
	cJoinMap	mJoinMap;

	/// 데이터 확인용
	DWORD mGuildTotalLength; /// 총 select 해야할 길드 갯수 ( DB 레코드 수 )
	DWORD mGuildTotalOffset; /// 총 select 해야할 길드 갯수 ( DB 레코드 수 )
	DWORD mGuildSaveOffset;	/// 총 저장 해야할 길드 갯수

	DWORD mGuildSelectLength; /// 실제 존재하는 길드 갯수 ( DB 레코드 수 )

	/// 정보갱신 시작시간
	long	mAccumTime;
};

inline void cGuildManager::SetGuildTotalLength( DWORD length )
{
	mGuildTotalLength = length;
}

inline void cGuildManager::SetGuildLength( DWORD length )
{
	mGuildSelectLength = length;
}

inline cGuildManager* cGuildManager::Get()
{
	return mpSingleton;
}

#define GUILDMAN cGuildManager::Get()
