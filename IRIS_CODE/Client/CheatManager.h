/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.06.04
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

struct MSGROOT;

/// 치트 명령 인덱스
enum eCheatCommandIndex
{
	/// 1단계: 정보, 이동, 소환
	CHEAT_COMM_NULL = 0,
	CHEAT_COMM_SHOT,
	CHEAT_COMM_SPEEDUP,
	CHEAT_COMM_EAR,
	CHEAT_COMM_HIDE,
	CHEAT_COMM_UNDEAD,
	CHEAT_COMM_INFO,
	CHEAT_COMM_WATCH,
	CHEAT_COMM_KICK,
	CHEAT_COMM_RECOVER,
	CHEAT_COMM_REBIRTH,
	CHEAT_COMM_TELEPORT,
	CHEAT_COMM_GO,
	CHEAT_COMM_CHASE,
	CHEAT_COMM_MOVE,
	CHEAT_COMM_CALL,
	CHEAT_COMM_NO_CHAT,
	CHEAT_COMM_ALLOW_CHAT,

	/// 2단계: 공지, 경고, 편집
	CHEAT_COMM_STOP,
	CHEAT_COMM_CANCEL_STOP,
	CHEAT_COMM_NOTICE_SERVER,
	CHEAT_COMM_NOTICE_CHANNEL,
	CHEAT_COMM_NOTICE_AREA,
	CHEAT_COMM_REMOVE_ITEM,
	CHEAT_COMM_REMOVE_MONSTER,
	CHEAT_COMM_LEVELUP,
	CHEAT_COMM_SKILLUP,

	/// 3단계: 생성 및 기타
	CHEAT_COMM_MAKE,
	CHEAT_COMM_GOLD,
	CHEAT_COMM_SUMMON,
	CHEAT_COMM_REMOVE,
	CHEAT_COMM_KILL,

	CHEAT_COMM_PLAYERINFO,
};

/// 치트 관리자
class cCheatManager
{
	static cCheatManager* mInstance;

public:
	static cCheatManager* GetInstance();

public:
	cCheatManager();
	~cCheatManager();

#ifdef _GMTOOL
	bool Interpret( const charT* src, unsigned int srcLen );

	bool GetShotMode() const;
	bool GetHideMode() const;
	bool GetUndeadMode() const;
	void SetHideMode( bool mode );
	void SetUndeadMode( bool mode );

	void Clear();
#endif
	
	/// 네트워크 메시지
	void NetworkMsgParser( MSGROOT* msg );

protected:
#ifdef _GMTOOL
	bool HandleCommand( unsigned int index, const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Shot( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_SpeedUp( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Ear( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Hide( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Undead( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Info( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Watch( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Kick( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Recover( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Rebirth( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Teleport( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Go( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Chase( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Move( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Call( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_NoChat( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_AllowChat( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Stop( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_CancelStop( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_NoticeServer( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_NoticeChannel( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_NoticeArea( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_RemoveItem( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_RemoveMonster( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_LevelUp( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_SkillUp( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Make( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Gold( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Summon( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Remove( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_Kill( const wchar_t* argStr, const tArray<cStringT>& argArray );
	bool HandleCommand_PlayerInfo( const wchar_t* argStr, const tArray<cStringT>& argArray );
#endif

protected:
#ifdef _GMTOOL
	/// 명령 맵
	typedef tHashMap<cStringT, unsigned int> cCommandMap;
	cCommandMap mCommandMap;

	/// 인자 배열
	typedef tArray<cStringT> cArgArray;
	cArgArray mArgArray;

	/// 스샷
	bool mShotMode;

	/// 귓말
	bool mEarMode;

	/// 투명
	bool mHideMode;

	/// 무적
	bool mUndeadMode;

	/// 감시
	bool mWatchMode;
#endif
};

#ifdef _GMTOOL
inline
bool cCheatManager::GetShotMode() const
{
	return mShotMode;
}

inline
bool cCheatManager::GetHideMode() const
{
	return mHideMode;
}

inline
bool cCheatManager::GetUndeadMode() const
{
	return mUndeadMode;
}

inline
void cCheatManager::SetHideMode( bool mode ) 
{
	mHideMode = mode;
}

inline
void cCheatManager::SetUndeadMode( bool mode )
{
	mUndeadMode = mode;
}

#endif

inline
cCheatManager* cCheatManager::GetInstance()
{
	return mInstance;
}

#define CHEATMAN	cCheatManager::GetInstance()
