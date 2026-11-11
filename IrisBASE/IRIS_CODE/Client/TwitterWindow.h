#pragma once

//==========================================================================
//			* 트위터 시스템 참고( API 요청 제한 ) *
//
//	트위터는 Client 어플에서 다양한 요청을 하는데,
//	기본적으로 트위터서 시간당 150회로 제한하고 있다.
//	그 이상의 요청을 하였을 때는 트위터에서 Limit 상태로 전환시켜
//	약 1시간 동안 어플에서의 요청을 제한한다.
//
//==========================================================================

#include "UIWindow.h"
#include "StyleListBox.h"
#include "twitcurl.h"
#include "Queue.h"


const unsigned long		SHOW_FAIL_MSG_TIME			=	3000;		// 3초( 중복 메시지 송신으로 인한 실패를 3초간 보여준다. )
const unsigned long		HTTP_REQUEST_TIME			=	60000;		// 1분 ( N분 마다 트위터 Data 재 송신 요청 )
const unsigned long		TWIT_REQUEST_LIMIT_TIME		=	3600000;	// 1시간 ( 1시간 동안 150건 이상의 트위터에 요청시 제한이 걸림 )
const unsigned char		TWIT_REQUEST_LIMIT_COUNT	=	127;		// 1시간 동안 Api 에서 Request 요청 제한 갯수
const unsigned char		MAX_TWITTER_RENDER_LIST		=	6;			// 트위터 리스트 박스에서 보여주는 갯수
const unsigned char		MAX_TWITTER_HAVE_LIST		=	20;			// 트위터 리스트 박스 최대 보유 갯수
const unsigned int		PRINT_TEXT_WIDTH			=	285;		// 트위터 리스트 박스 텍스크 내용 가로 크기
const unsigned char		PROFILE_ICON_SIZE			=	28;			// 트위터 프로필 이미지 크기
const unsigned char		DUPLICATION_COUNT_CHECK		=	10;			// 트위터 프로필 이미지 크기

const unsigned char		TWIT_EVENT_ENHANCE_STEP		=	7;			// 트위터 이벤트 메시지 강화 단계
const unsigned char		TWIT_EVENT_LEVELUP			=	10;			// 트위터 이벤트 메시지 레벨 단위

class cEditBox;
class cMultiEditBox;
class cButton;
class cLabel;
class cTwitterListBox;
class cCheckBox;


/// Thread에서 HTTP로 부터 수신한 버퍼 저장
struct sImgBufInfo
{
	char*	mBuf;	
	DWORD	mBufLen;	

	sImgBufInfo()
	{
		::ZeroMemory( this, sizeof(sImgBufInfo) );
	}
};

/// Thread에서 Encoding한 텍스트 정보 저장
struct sTwitDataInfo
{
	unsigned long	mUserIdx;
	__int64			mTextIdx;
	cStringT		mText;
	cStringT		mDate;
	cStringT		mID;

	sTwitDataInfo()
	{
		::ZeroMemory( this,sizeof(sTwitDataInfo));
	}
};

struct sThreadInfo
{
	unsigned char	mCommandType;	
	void*			mExtraData;

	sThreadInfo()
	{
		::ZeroMemory( this,sizeof(sThreadInfo));
	}
};

// Twitter 리스트 박스 Extradata Struct
struct sTwitterExdata
{
	unsigned long nUserIdx;
	__int64 nTextIdx;

	sTwitterExdata()
	{
		::ZeroMemory( this,sizeof(sTwitterExdata));
	}
};

typedef tHashMap< unsigned long, void* >		cImgBufferMap;		// Thread에서 Http로 부터 전송받은 버퍼 주소를 저장
typedef tArray< void* >							cTwitDataMap;		// Thread에서 트위터 메시지 정보 저장
typedef tHashMap< unsigned long, NiTexturePtr > cTwitterTextureMap;	// 텍스쳐 정보 저장
typedef tList< __int64 >						cTwitterTextIdxList;	// 텍스쳐 정보 저장

struct sThreadShareData
{
	cImgBufferMap			mImgBufferMap;			// 이미지 임시 버퍼
	cTwitDataMap			mTwitDataMap;			// 트위터 Data
	cTwitterTextureMap		mTwitterTextureMap;		// 텍스쳐 리스트
	cTwitterTextIdxList		mTiwtTextIdxList;		// 작성한 최근 10개 게시물의 글 idx 저장

	twitCurl				mpTwitterContoller;		// 트위터 컨트롤러
	CRITICAL_SECTION		mCs;

	unsigned char			mStateThread;			// Thread 진행 상태			
	unsigned char			mResultThread;			// Thread 결과
	bool					mShutDown;				// Thread ShuDown
	int						mResetRemainMinute;		// 시간당 150 Req ResetTime
};

// 트위터 이벤트 종류
enum eTwitEventType
{
	eTWITEVENT_QUEST,
	eTWITEVENT_MONSTER,
	eTWITEVENT_LEVEL,
	eTWITEVENT_ENHANCE,	
	eTWITEVENT_MAX,
};

/// 트위터 이벤트
struct sTwitterEvent
{
	eTwitEventType	mEvent;
	void*			mExData;

	sTwitterEvent()
	{
		::ZeroMemory( this, sizeof(sTwitterEvent) );
	}
};

struct sTwitterLogInfo
{
	unsigned char mCommandType;
	unsigned long mReqTime;
};

/// LBS : thread와 공통으로 사용하는 enum형을 twitterwindow에서 뺍니다.
/// enumeration
enum
{	// Thread 동작 상태
	eState_None,					// eNone 이외에는 모두 쓰레드 동작중
	eState_Getting_TimeLineHomeMsg,	// 리스트 갱신 처리 상태
	eState_Sending_UpdateMsg,		// 메세지 작성 요청 상태
	eState_Sending_EventMsg,		// 메세지 작성 요청 상태
	eState_Sending_LoginMsg,		// 로그인 처리 진행 상태		
	eState_Sending_RetweetMsg,		// 리트윗 처리 진행 상태
	eState_Sending_FollowMsg,		// Follow 처리 진행 상태
};

/// 쓰레드 결과 타입에 따라 알림창을 구분하여 연다.
enum
{
	eResult_None,					//	
	eResult_Success_SendMsg,		// 메시지 전송 결과
	eResult_Success_EventMsg,		// 이벤트 메시지 전송 결과
	eResult_Success_GetList,		// 리스트 갱신 결과
	eResult_Success_Login,			// 로그인 성공시
	eResult_Success_Retweet,		// 리트윗 성공시
	eResult_Success_Follow,			// Follow 성공시

	eResult_Error_Default,			// 에러 발생 상태
	eResult_Error_NoAccount,		// 로그인 시도시 계정 정보가 없는 경우, 로그인 실패시
	eResult_Error_UpdateStatus,		// 글 남기기 실패
	eResult_Error_EventUpdateStatus,// 이벤트 글 남기기 실패
	eResult_Error_GetTimeLineHome,	// 리스트 가져오기 실패
	eResult_Error_NoLogin,			// 로그인 X	
	eResult_Error_Retweet,			// 리트윗 실패
	eResult_Error_Follow,			// Follow 실패
	eResult_Error_LimitExceed,		// 1시간당 150번 이상의 Request 요청시 리밋 상태라 실패!		
	eResult_Error_StatusUpdateLimit,// 일일 1000건의 글을 올리거나 또는 짧은 시간의 많은 글을 올려서 트위터에 의해 제한상태일때
};


class cTwitterWindow : public cUIWindow
{
public:
	enum
	{	// Thread 동작 타입
		eCommand_SendUpdate,			// 글쓰기
		eCommand_EventUpdate,			// 이벤트 메시지 전송
		eCommand_GetTimeLineHome,		// 리스트 갱신
		eCommand_Login,					// 로그인
		eCommand_AutoLogin,				// 자동 로그인
		eCommand_Retweet,				// RT 리트윗
		eCommand_Follow					// Follow
	};

	enum
	{	// DB 갱신 타입
		eRegister_OnlySave,				// 계정만 저장
		eRegister_OnlyLogin,			// 로그인
		eRegister_LoginAndSave,			// 로그인 + 계정 저장
		eRegister_Refresh				// 계정 초기화
	};

	// Method
	cTwitterWindow();
	virtual ~cTwitterWindow();

	virtual void		Clear();	
	virtual void		Open();	

protected:
	// Method

	virtual void		OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual bool		OnCreate( cUINodeProperty* pproperty );
	virtual void		OnCommand( cUINode* caller, unsigned int id );
	virtual void		OnShow();
	virtual void		OnHide();	
	virtual void		OnNetLock( int lockTry );
	virtual void		OnNetUnLock( int lockTry );

public:

	bool				IsLogin(){ return mIsLogin;}
	void				SetLogin( bool bLogin ){ mIsLogin = bLogin;}
	void				LogOut();

	void				SetDBAccountSave( bool bSave );
	bool				IsDBSaveAccount(){ return mDBAccountSave; }
	
	void				SetRegistType( unsigned char nType ){ mIsAccountRegister = nType; }
	unsigned char		GetRegistType(){ return mIsAccountRegister; }

	LPCSTR				GetID();
	LPCSTR				GetPW();

	void				CreateTwitterThread( unsigned char nCommandType, void* pExData = NULL );
	void				DestroyTwitterThread();	
	
	static DWORD WINAPI PerformTwitterThread( void* arg );

	void				AddEvent( sTwitterEvent& twitEvent );
	unsigned char		ProcessEvent();

	bool				IsTwitterTextData( __int64 nFindTextIdx );

private:
	///============[ UI ]============
	cMultiEditBox*		mpSendMessage;		// 메시지
	cTwitterListBox*	mpTwitListBox;		// 리스트 박스
	cButton*			mpSendBtn;			// 메시지 전송
	cButton*			mpRegisterBtn;		// 계정 등록/ 변경
	cButton*			mpCloseBtn;			// 닫기
	cLabel*				mpStateLabel;		// 트위터 작동 상태	( 로그인, 글쓰기, 리스트 갱신 등등 )
	
	//
	unsigned long		mUpdateTime;		// 다음 갱신 시간
	unsigned long		mFailShowTime;		// 에러 메시지 출력 시간
	bool				mIsLogin;			// 로그인 성공 여부	
	bool				mAccountSaveType;	// DB에 저장 할지 여부
	bool				mDBAccountSave;		// DB에 저장 상태
	unsigned char		mIsAccountRegister;	// 새로이 계정 등록인지, 초기화인지 구분
	
	cString				mOldID;
	cString				mOldPW;
	cStringT			mEventMsg[ eTWITEVENT_MAX ];

	// Thread
	HANDLE				mTwitterThread;		// 트위터 요청 처리 쓰레드	

	bool				mEventUpdate;		// 이벤트 갱신 여부
	bool				mIsSendMsgFail;		// 중복 메시지 전송으로 인한 실패 여부
	bool				mUpdateReady;		// 갱신 준비 상태
		
	// DLL Load
	typedef tArray< HINSTANCE > cDLLArray;
	cDLLArray mDllArray;

	// 캐릭터 이벤트를 적재 처리(레벨업, 아이템 강화, 퀘스트, 특정 몬스터)
	typedef tQueue<sTwitterEvent> cEventQueue;
	cEventQueue	mTwitterEventQueue;

};



class cTwitterRegisterWindow : public cUIWindow
{
public:
	// Method

	cTwitterRegisterWindow();
	virtual ~cTwitterRegisterWindow();

	virtual void Open();
	virtual void Clear();

protected:

	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnCommand( cUINode* caller, unsigned int id );
	virtual void OnHide();
	virtual void OnNetLock( int lockTry );
	virtual void OnNetUnLock( int lockTry );

public:	

	LPCTSTR		GetID();
	LPCTSTR		GetPW();
	void		SetResetBtnEnable( bool bEnable );

	bool		IsDBSaveCheckPress();
	void		SetPressDBSaveCheck( bool bPress );

private:

	cEditBox*	mpID;				// ID
	cEditBox*	mpPw;				// Password

	cButton*	mpConnectAndSave;	// 접속 / 저장
	cButton*	mpCancelBtn;		// 취소
	cButton*	mpResetBtn;			// 초기화

	cCheckBox*	mpDBSaveCheck;		// DB 저장
	
};

class cTwitterListBox : public cImageListBox
{
public:
	cTwitterListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cTwitterListBox();

	// 리스트에 0번째 인뎃스에 자료 삽입
	int AddRowAtFirst( LPCTSTR str, unsigned long color, cUIImage* image, void* pExtraData = 0 );
	void SetMaxRowCount( unsigned short nCnt );

protected:
			
	virtual void OnMouseMove( const cUIPos& pos );
	virtual	void OnMouseLeft( const cUIPos& pos );
	virtual void OnMouseWheel( const cUIPos& pos, int wheel );	
	virtual void RenderData( cUIFontItemKeeper* pKeeper );

	virtual void ShowTwitterTextTip( const cUIPos& pos );
	
};

