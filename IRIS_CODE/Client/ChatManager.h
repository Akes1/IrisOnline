/* ==========================================================================
* 파    일 : ChatManager.h
* 목    적 : 채팅 매니져
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-20
* 주의사항 :
*===========================================================================*/
#pragma once

class cChatCommands;
class cUIWindow;
class cUINode;
struct sInventory;
enum ePLAYER_JOB;

enum eChatMsgKind
{
	eCHAT_NORMAL,
	eCHAT_PARTY,
	eCHAT_PARTYUNION,
	eCHAT_GUILD,
	eCHAT_SHOUT,
	eCHAT_TRADE,
	eCHAT_WHISPER,
	eCHAT_SYSTEM,
	eCHAT_NOTICE,	// GM툴 공지 및 아이템 강화
	eCHAT_GM,		// GM 의 일반 채팅
	eCHAT_MEGAPHONE,
	eCHAT_GUILD_NOTICE,

	eCHAT_MAX,
};

enum eSystemMsgKind
{
	eSYSTEM_NORMAL,		/// 일반적인 시스템 메시지(레벨업, 경험치 획득, 스킬 사용 등)
	eSYSTEM_GETMONEY,	/// 돈 획득
	eSYSTEM_GETITEM,	/// 일반 아이템 획득(단 아이템이름 색상은 레어도 색상값 사용)
	eSYSTEM_QUEST,		/// 퀘스트 관련 메시지(퀘스트 시작 종료, 퀘스트 아이템 획득)
	eSYSTEM_DOITEM,		/// 조합, 분해, 인챈트, 카드 장착시 알림 메시지
	eSYSTEM_CASH,		/// 캐쉬 아이템 사용 및 기간 만료 시 알림 메시지
	eSYSTEM_NPCSTORE,	/// NPC 상점 판매/구입시
	eSYSTEM_PVP,		/// PVP 알림
	eSYSTEM_GM,         /// GM 에게 특정 조치를 당했을 때 알림 메시지
};

struct POST_OPEN_RESULT;
struct sFriendData;

class cChatManager
{
	static cChatManager* mInstance;

public:
	/// 단일 객체를 리턴 
	static cChatManager* Get();

public:
	cChatManager();
	~cChatManager();

	/// 초기화
	bool Init();
	void Close();
	void ClearChat();

	/// 메시지 파서
	void NetworkMsgParser( char* pMsg, u_long length);

	/// 포커스를 가지는 에디트 추가 ( 포커스관리 )
	void AddFocusNode( cUINode* n );

	/// 시스템 메세지 출력
	void AddSystemMsg( eSystemMsgKind kind, LPCTSTR chatmsg, bool onlychatmain = false );

	/// 아이템 획득 메세지 출력
	void AddItemMsg( unsigned long itemIndex, unsigned short slotIndex, LPCTSTR msg1, LPCTSTR msg2, bool IsQuest = false );
	void AddItemMsg( sInventory inventory, LPCTSTR name, LPCTSTR msg, ePLAYER_JOB job, eChatMsgKind msgkind, void* exdata = NULL );

	/// 채팅 메시지 보내기 ( 채팅창 -> 서버 )
	bool SendChatMsg( eChatMsgKind type, LPCTSTR msg );

	/// 각 윈도우 저장
	void SetChatWindow( cUIWindow* w );
	void SetMtoMChatWindow( cUIWindow* w );

	/// 채팅창 포커스 바꾸기 ( shift + tab : 채팅창 <-> 1:1대화창 )
	void ChangeFocus();

	/// 감정표현 관련데이터 삽입
	bool AddCommunityData( LPCTSTR key, unsigned long index );

	/// 메세지 출력
	void AddChatMsg( LPCTSTR chatmsg, eChatMsgKind msgkind, bool checkPress = true, bool onlychatmain = false );

	/// 메세지 출력
	void AddChatMsgEx( LPCTSTR chatmsg, unsigned long color, unsigned long infoIndex, bool paste, bool onlychatmain, bool checkPress = false, eChatMsgKind msgkind = eCHAT_MAX );

	/// 공지내용 보내기
	void AddNoticeMsg( LPCTSTR msg, unsigned long color );
	void AddPvPNoticeMsg( LPCTSTR msg, unsigned long color );

	void AddDiceMsg();

	//
	void SendPostOpen();
	POST_OPEN_RESULT* GetMailData( long idx );

	// 새 메일 체크
	void CheckNewMail();

	//
	unsigned long GetTextColor( eChatMsgKind kind );
	unsigned long GetInfoTextIndex( eChatMsgKind kind );

	//
	sFriendData* GetFriendData( unsigned long characterIdx );

protected:
	void ClearMail();

	//
	void ClearFriend();

private:
	/// 채팅창 ( 기본 + 분리창 3개 )
	typedef tPointerList<void*> cChatWindowList;
	cChatWindowList		mChatWindowList;
	
	/// 각창 포커스관리 ( 채팅창, 일대일대화창만 등록 )
	typedef tPointerArray<void*> cFocusArr;
	cFocusArr	mFocusArr;

	/// 우편 정보 
	typedef tPointerHashMap<unsigned long, void*> cMailDataMap;
	cMailDataMap		mMailDataMap;

	/// 친구 정보 (sFriendData*)
	typedef tPointerHashMap<unsigned long, void*> cFriendDataMap;
	cFriendDataMap		mFriendDataMap;

	///
	cChatCommands*	mpCommands;
};

inline sFriendData* cChatManager::GetFriendData( unsigned long characterIdx )
{
	return (sFriendData*)mFriendDataMap.GetAt( characterIdx );
}

inline cChatManager* cChatManager::Get()
{
	return mInstance;
}

#define CHATMANAGER cChatManager::Get()
