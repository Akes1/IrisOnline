/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once
#include "BaseStage.h"

/// 엔진부 작업을 하는데 서버 접속 없이 작업하기 위해서..
/// 그래픽 작업물 뷰어 차원에서도 필요한 루틴임...
//#define NOCONNECT_SERVER

class cGameResourceManager;
class cWorldManager;
class cObjectManager;
class cSkillManager;
class cDramaturgyManager;
class cUIManager;
class cItemManager;
class cTarotManager;
class cShortcutManager;
class cFilterManager;
class cCoolTimeManager;
class cQuickManager;
class cPartyManager;
class cQuestManager;
class cTriggerManager;
class cDuelManager;
class cCheatManager;
class cPVPManager;
class cTitleManager;
class cGuildManager;
class cTipManager;
class cItemMixManager;
class cPartyUnionManager;
class cTutorialManager;
class cWebManager;
class cThemeManager;
class cIOCPClient;


class cStageManager
{
public:
	static cStageManager*	mpStageManager;

public:
	cStageManager();
	~cStageManager();

	bool Init( HWND hWnd, unsigned int screenWidth, unsigned int screenHeight );
	void Exit();
	void Process( unsigned long deltaTime, unsigned long accumTime );
	void ProcessWorld( unsigned long deltaTime, unsigned long accumTime );

	void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );	

	void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

	/// 게임 진행상의 각종 키 입력에 대한 처리
	void ProcessKeyPressed( cKeyboardAgent* pKeyboard );
	void ProcessKeyUpDown( unsigned long time, cKeyboardAgent* pKeyboard );

	/// 게임 진행상의 마우스 이벤트에 대한 처리
	void ProcessMouseEvent( unsigned long time, cMouseAgent* pMouse );

	void ChangeStage( eSTAGE stage );

	/// Get/Set UserIdx & LoginIdx
	void SetUserIdx  ( long idx ) { mUserIdx  = idx; }
	void SetLoginIdx ( long idx ) { mLoginIdx = idx; }

	long GetUserIdx  ( void )     { return mUserIdx;  }
	long GetLoginIdx ( void )     { return mLoginIdx; }

	void SetTwitterUid ( char* uid );
	void SetTwitterPwd ( char* pwd );

	char* GetTwitterUid ( ) { return mTwitterUid; }
	char* GetTwitterPwd ( ) { return mTwitterPwd; }

	void SetAdultServer( bool set ) { mIsAdultServer = set; }
	bool IsAdultServer( ) { return mIsAdultServer; }

	void SetMapIdx( unsigned long idx ) { mCurrentMapIdx = idx; }
	unsigned long GetMapIdx() { return mCurrentMapIdx; }
	void SetMapMode( unsigned char mode ) { mCurrentMapMode = mode; }
	unsigned char GetMapMode() { return mCurrentMapMode; }

	///
	unsigned long GetChannelIdx() { return mChannelIdx; }
	void SetChannelIdx( unsigned long idx );

	long GetServerIdx() { return mServerIdx; }
	void SetServerIdx( long idx ) { mServerIdx = idx; }

	eSTAGE	GetCurrentStage()						{ return mCurStage; }
	cBaseStage*	GetStage( eSTAGE stage )			{ return mpStage[stage]; }

	void AddScreenShotMsg( const charT* fileName );
	void AddVideoCaptureMsg( const charT* fileName );

protected:
	void ManagedBGM( eSTAGE curStage, eSTAGE newStage );

	void ManagedStageMap( eSTAGE curStage, eSTAGE newStage );

protected:
	cBaseStage*	mpStage[eStage_Max];

	eSTAGE		mCurStage;

	long            mUserIdx;
	long            mLoginIdx;

	char            mTwitterUid[LEN_TWITTER_UID];
	char            mTwitterPwd[LEN_TWITTER_PWD];

	unsigned long   mChannelIdx;
	unsigned long	mCurrentMapIdx;
	unsigned char	mCurrentMapMode;

	// 성인 서버 인증 (0:일반 1:성인)
	long			mServerIdx;
	bool			mIsAdultServer;

protected:
	/// 각 manager 및 static 변수들...

	/// 070418 BaseNetwork.dll 모듈 제거.
	/// 070330 Client Network Model (신규통신 모듈)
	cIOCPClient* mIOCPCLient;

	/// 게임 자원 관리자
	cGameResourceManager* mpGameResourceManager;

	/// 게임 세계 관리자
	cWorldManager* mpWorldManager;

	/// 게임 객체 관리자
    cObjectManager* mpObjectManager;

	/// Skill 관리자
	cSkillManager* mpSkillManager;

	/// 연출 관리자
	cDramaturgyManager* mpDramaturgyManager;

	/// 아이템 관리자
	cItemManager* mpItemManager;

	/// 타롯 관리자 
	cTarotManager* mpTarotManager;

	/// 단축키 관리자
	cShortcutManager* mpShortcutManager;

	/// UI 관리자
	cUIManager* mpUIManager;
	
	/// 필터링 관리자
	cFilterManager* mpFilterManager;

	/// 쿨타임 관리자
	cCoolTimeManager* mpCoolManager;

	/// 퀵슬롯 관리자 
	cQuickManager* mpQuickManager;

	/// 파티 관리자
	cPartyManager* mpPartyManager;

	/// 퀘스트 관리자
	cQuestManager* mpQuestManager;

	/// 퀘스트 트리거 관리자
	cTriggerManager* mpTriggerManager;

	/// 치트 관리자
	cCheatManager*	mpCheatManager;

	/// pvp 관리자
	cPVPManager*	mpPvpManager;

	/// 호칭 관리자
	cTitleManager*	mpTitleManager;

	/// 길드 관리자
	cGuildManager*	mpGuildManager;

	/// 팁 관리자
	cTipManager*	mpTipManager;

	/// 아이템 조합 관리자
	cItemMixManager*	mpItemMixManager;

	/// 파티 연합 관리자
	cPartyUnionManager* mpPartyUnionManager;

	//
	cTutorialManager*	mpTutorialManager;

	// 테마
	cThemeManager*		mpThemeManager;

	cWebManager*	mpWebManager;

	/// 스테이지별 BGM Sound 처리
};

#define	STAGEMAN cStageManager::mpStageManager
