/* ==========================================================================
* 파    일 : UINode.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-30
* 주의사항 :
*===========================================================================*/
#pragma once

#include "UIRect.h"
#include "UIKeyCode.h"
#include "UIFontItemKeeper.h"
#include "UIManager.h"

class cUINodeSkin;
class cUIEvent;
class cUIImageBase;
class cUIRootNode;
class cUIContainer;
class cUINodeProperty;
class cUIManager;

typedef tUIRect<int> cUIRect;
typedef tUIPos<int> cUIPos;
typedef tUISize<int> cUISize;

/// 자석기능 거리 적용 
const int AUTO_MAGNATIC_DIST = 20;

/// 아이콘 사이즈
const int ICON_SIZE = 28;

/// UI 노드 타입
enum eUINodeType
{
	eUINODE_NULL			= 0,
	eUINODE_CONTAINER		= (1 <<  0),
	eUINODE_ROOT			= (1 <<  1) | eUINODE_CONTAINER,
	eUINODE_WINDOW			= (1 <<  2) | eUINODE_CONTAINER,
	eUINODE_BUTTON			= (1 <<  3),
	eUINODE_PUSHBUTTON		= (1 <<  4) | eUINODE_BUTTON,
	eUINODE_CHECKBOX		= (1 <<  5),
	eUINODE_LABEL			= (1 <<  6),
	eUINODE_EDITBOX			= (1 <<  8),
	eUINODE_LISTBOX			= (1 <<  9) | eUINODE_WINDOW,
	eUINODE_SCROLLBAR		= (1 << 10) | eUINODE_WINDOW,
	eUINODE_BARBUTTON		= (1 << 11) | eUINODE_BUTTON,
	eUINODE_COMBOBOX		= (1 << 12) | eUINODE_WINDOW,
	eUINODE_GAUGE			= (1 << 13),
	eUINODE_GAUGETB			= (1 << 14),
	eUINODE_DOUBLEGAUGE		= (1 << 15),
	eUINODE_TEXTBOX			= (1 << 16) | eUINODE_WINDOW,
	eUINODE_MESSAGEBOX		= (1 << 17) | eUINODE_WINDOW,
	eUINODE_TABWINDOW		= (1 << 18) | eUINODE_WINDOW,
	eUINODE_TABBUTTON		= (1 << 19) | eUINODE_PUSHBUTTON,
	eUINODE_SPINBOX			= (1 << 20) | eUINODE_WINDOW,
	eUINODE_ICON			= (1 << 21), 
	eUINODE_NUMBEREDIT		= (1 << 22) | eUINODE_EDITBOX,
	eUINODE_RADIOBUTTON		= (1 << 23) | eUINODE_BUTTON,
	eUINODE_MULTIEDITBOX	= (1 << 24),
	eUINODE_OPTIONGAUGE		= (1 << 25) | eUINODE_WINDOW, 
	eUINODE_DRAGWINDOW		= (1 << 26) | eUINODE_WINDOW,
	eUINODE_TABSHEET		= (1 << 27) | eUINODE_WINDOW,
	eUINODE_LISTFOLDERBOX	= (1 << 28) | eUINODE_WINDOW,

	eUINODE_FORCE_32BIT	= 0x7FFFFFFF
};

/// 텍스트 정렬 
enum eText_Align
{
	eALIGN_NONE = 0,
	eALIGN_LEFT,
	eALIGN_RIGHT,		
	eALIGN_CENTER,		
};

/// 기본 색상
const unsigned long COLOR_BLACK = 0xFF111111;
const unsigned long COLOR_WHITE = 0xFFFFFFFF;
const unsigned long COLOR_YELLOW = 0xFFFFFC31;
const unsigned long COLOR_PINK = 0xFFFF50FF;
const unsigned long COLOR_GREEN = 0xFF89F116;
const unsigned long COLOR_SKYBLUE = 0xFF00FFEA;
const unsigned long COLOR_RED = 0xFFF03719;
const unsigned long COLOR_GRAY = 0xFFDCDCDC;
const unsigned long COLOR_BLUE = 0xFF00FCFF;
const unsigned long COLOR_ORANGE = 0xFFFFA820;
const unsigned long COLOR_BRIGHTGREEN = 0xFFFFCF3B;
const unsigned long COLOR_DARKGRAY = 0xFF808080;
const unsigned long COLOR_OLIVE = 0xFF808000;
const unsigned long COLOR_BROWN = 0xFF643200;

const unsigned long COLOR_DEFAULT = 0xFFE6D8BB;

/// 레어도 관련 색상
const unsigned long COLOR_MALL = 0xFFFFCD00;
const unsigned long COLOR_EPIC = 0xFFFFA800;
const unsigned long COLOR_REGEND = 0xFFD800FF;
const unsigned long COLOR_ETC = 0xFF808080;
const unsigned long COLOR_UNIQUE = 0xFFFFF600;
const unsigned long COLOR_RARE = 0xFF00FFFC;
const unsigned long COLOR_MAGIC = 0xFF00FF00;

/// 채팅 색상
const unsigned long COLOR_SYSTEM = 0xFFFACD78;
const unsigned long COLOR_PARTY = 0xFF90FF00;
const unsigned long COLOR_GUILD = 0xFFFFA300;
const unsigned long COLOR_SHOUT = 0xFF6EBDFF;
const unsigned long COLOR_TRADE = 0xFFFF7100;
const unsigned long COLOR_WHISPER = 0xFFFF564D;
const unsigned long COLOR_TIP = 0xFFEEF41E;
const unsigned long COLOR_GM  = 0xFF32FFC6;
const unsigned long COLOR_MEGAPHONE  = 0xFF99A3FF;

const unsigned long COLOR_DICENUM = 0xFFFFFF00;

const unsigned long COLOR_FRIEND_EX = 0xFFA5FF00;
const unsigned long COLOR_GUILD_EX = 0xFFFF9B82;
const unsigned long COLOR_PARTY_EX = 0xFF00F0FF;

const unsigned long COLOR_LOW_LEVEL_QUEST = 0xFF969696;

const unsigned long COLOR_TWITTER_INSERTMSG = 0xFF00FFEA;
const unsigned long COLOR_TWITTER_DATE = 0xFFACACCF;
const unsigned long COLOR_TWITTER_ID = 0xFF75BEFB;

/// 직업별 NameCard 색상
const unsigned long COLOR_JOB_ARMSMAN	= 0xFF9857BA;
const unsigned long COLOR_JOB_FIGHTER	= 0xFF448FDC;
const unsigned long COLOR_JOB_RANGER	= 0xFFEFA45A;
const unsigned long COLOR_JOB_WANDERER	= 0xFFFDFD55;
const unsigned long COLOR_JOB_WARLOCK	= 0xFF008E6B;
const unsigned long COLOR_JOB_MAGE		= 0xFF65E965;

// 스킬 툴팁 컬러
const unsigned long COLOR_SKILL_NOT_OVERLAP		= 0xFFF03718;
const unsigned long COLOR_SKILL_ACTIVE_EFFECT	= 0xFFFEFB30;


/// 자석기능 타입 
enum eSnap_Type
{
	eSNAP_NONE,
	eSNAP_LEFT,
	eSNAP_RIGHT,
	eSNAP_TOP,
	eSNAP_BOTTOM,

	eSNAP_TOP_OUT,	/// 바깥쪽 TOP 위치에 붙는경우
	eSNAP_BOTTOM_OUT,
	eSNAP_RIGHT_OUT,
	eSNAP_LEFT_OUT,
};

/// UI 노드
/// UI 노드 오브젝트들의 최상위 추상 기반 클래스이다.
class cUINode
{
	friend class cUIManager;
	friend class cUIContainer;
	friend class cImeBase;

public:
	cUINode( eUINodeType type = eUINODE_NULL );
	virtual ~cUINode();

	///프로퍼티를 이용한 생성 ( 프로퍼티 이름을 직접준다 - 각 윈도우의 최고 부모생성시에만 사용 )
	virtual bool CreateByPropertyName( const cString& propertyName, cUIContainer* pparent );

	///프로퍼티를 이용한 생성 ( 자식윈도우를 생성할때 사용 )
	virtual bool CreateByProperty( cUINodeProperty* pProperty, cUIContainer* pparent );

	/// 프로퍼티, id 값 으로 생성 
	virtual bool CreateByPropertyId( cUINodeProperty* pProperty, cUIContainer* pparent, unsigned long id );

	/// 스킨 이름을 주고 노드 생성시 사용 
	virtual bool CreateBySkinName( const cString& skinName, cUIContainer* pparent, unsigned long id );

	/// 소멸
	virtual void Destroy();

	/// 갱신
	virtual void UpdateRect();

	/// 스킨을 적용
	virtual bool ApplySkin();

	/// 해상도 변경에 따른 객체의 스킨 재적용
	virtual void UpdateSkin();
	
	/// 색상값
	void SetColor( float r, float g, float b, float a );
	void SetColor( const NiColorA& color );

	/// 알파값
	void SetAlpha( float a );
	float GetAlpha();

	/// 이벤트를 처리
	virtual bool HandleEvent( const cUIEvent& event );

	/// 이동
	void Move( const cUIPos& value );

	/// 상대 영역
	virtual void SetRelativeRect( const cUIRect& rect );
	virtual void SetRelativePos( const cUIPos& pos  );
	void SetRelativeSize( const cUISize& size );
	const cUIRect& GetRelativeRect() const;

	/// 절대 영역
	virtual bool ContainPoint( const cUIPos& pos ) const;
	const cUIRect& GetAbsoluteRect() const;

	/// 가시 여부 ( 자기자신만 설정 )
	virtual void Show();
	virtual void Hide();
	bool IsVisible() const;	

	/// 활성화 여부 ( 자기자신만 설정 )
	virtual void SetEnabled( bool enabled );
	bool IsEnabled() const;
	
	virtual void SetOutLine( bool set ) { mOutLine = set; }
	
	/// 눌림 여부 ( 자기자신만 설정 )
	virtual void SetPress( bool press );
	bool IsPress() const;

	/// 마우스 갱신
	void CheckEventNode();

	/// 마우스 캡처
	void SetCapture();
	void ReleaseCapture();

	/// 포커스
	virtual void SetFocus();
	void ReleaseFocus();

	/// 호버
	void ReleaseHover();

	/// 캡쳐, 호버 노드 클리어 
	void ReleaseEventNode();

	/// 타입
	eUINodeType GetType() const;

	/// 스킨 이름
	const cString& GetSkinName() const;

	/// 부모
	cUIContainer* GetParent();

	/// 아이디
	int GetID() const;

	/// 같은 종류인지 여부를 리턴
	bool IsKindof( eUINodeType type ) const;
	
	/// 이벤트 전달
	static bool SendEvent( cUINode* eventHandler, eUIEventType eventType, cUINode* caller, unsigned int id, bool shift = false );

	/// 락 걸기
	virtual void NetLock( int lockTry = 0 );
	virtual void NetUnLock( int lockTry = 0 );
	virtual bool IsNetLock() { return mNetLock; }

	/// 모든 ui 락 풀기
	virtual void NetUnLockAllUI();

	inline void SetExceptEvent( bool set ) { mExceptionEvent = set; }
	inline bool IsExceptEvent() { return mExceptionEvent; }

	/// 해당 플래그가 서있으면, 모든 이벤트 처리후, 마우스의 커서를 Default 상태로 되돌린다
	inline void SetCursorDefault( bool set ) { mIsSetCursorDefault = set; }
	inline bool IsSetCursorDefault() { return mIsSetCursorDefault; }

	inline void EnableProcess( bool set ) { mProcessEnable = set; }
	inline bool IsEnableProcess() { return mProcessEnable; }

	inline virtual void EnableBoldText( bool set ) { mUseBoldText = set; }

	inline void SetBackupVisible( bool set ) { mBackupVisible = set; }
	inline bool IsBackupVisible() { return mBackupVisible; }

	inline void SetFontType( unsigned char nFontType ){ mFontType = nFontType; }
	inline unsigned char GetFontType(){ return mFontType; }

protected:
	/// 각노드의 나머지 부분들을 세팅
	bool SetupNode( cUIContainer* pparent );
	/// 스킨을 설정
	virtual bool SetSkin( const cUINodeSkin* pskin );
	/// 적합 여부
	virtual bool IsValid() const;

	/// 컨테이너 가 움직일때 자석기능 주기
	void ApplySnap();

	///
	void SetVisible( bool visible );

	/// 화면 크기
	static unsigned int GetScreenWidth();
	static unsigned int GetScreenHeight();

	/// 루트 노드
	static cUIRootNode* GetRoot();

	/// 마우스 캡처 노드
	static cUINode* GetCapture();

	/// 포커스 노드
	static cUINode* GetFocus();

	/// 호버 노드 
	static cUINode* GetHover();
    
public:
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime ) = 0;
	virtual void OnRender( cUIFontItemKeeper* pKeeper ) = 0;
	virtual void OnPreRender() {}

protected:
	virtual bool OnCreate( cUINodeProperty* pproperty = 0 ) { return true; }
	virtual void OnDestroy() {}
	virtual void OnCaptureLost() {}
	virtual void OnFocusLost() {}
	virtual void OnMouseMove( const cUIPos& pos ) {}
	virtual void OnMouseWheel( const cUIPos& pos, int wheel ) {}
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift ) {}
	virtual void OnLButtonUp( const cUIPos& pos ) {}
	virtual void OnRButtonUp( const cUIPos& pos ) {}
	virtual void OnLButtonDoubleClick( const cUIPos& pos ) {}
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	
	virtual void OnKeyDown( eKeyCode code ) {}
	virtual void OnKeyUp( eKeyCode code ) {}
	
	/// IME Messages
	virtual void OnChar( unsigned int wparam ) {}
	virtual void OnImeNotify( unsigned int wparam ) {}
	virtual void OnImeStartComposition( unsigned int wparam, unsigned int lparam ) {}
	virtual void OnImeComposition( unsigned int wparam, unsigned int lparam ) {}
	virtual void OnImeEndComposition( unsigned int wparam, unsigned int lparam ) {}

protected:
	/// 타입
	eUINodeType mType;

	/// 부모
	cUIContainer* mpParent;
	
	/// 스킨 이름
	cString mSkinName;

	/// 아이디
	unsigned long mID;

	unsigned long mFrontID;

	///
	unsigned long mHoverTipIndex;

	/// 이미지
	cUIImageBase* mpImage;

	/// 상대 영역 ( 부모의 대한 상대 영역 )
	cUIRect mRelativeRect;

	/// 절대 영역
	cUIRect mAbsoluteRect;

	/// 무브 여부
	bool mMove;

    /// 가시 여부
	bool mVisible;

	/// 활성화 여부
	bool mEnabled;

	/// 눌림 여부 
	bool mPressed;
	
	/// 알파값
	float mAlpha;

	// 텍스트 출력시 테두리 출력
	bool mOutLine;

	/// 자석 적용 타입 
	eSnap_Type mSnapType;

	/// 해상도 변경전의 해상도 저장
	unsigned int mOldScreenWidth;
	unsigned int mOldScreenHeight;

	/// 스크립트의 기본 해상도 ( DEFAULT_SCREENWIDTH * DEFAULT_SCREENHEIGHT )
	unsigned int mDefaultWidth;
	unsigned int mDefaultHeight;

	/// 기본 색상 ( 텍스트류 )
	unsigned long mDefaultColor;

	/// 네트워크 관련 락
	bool	mNetLock;
	int		mLockTry;

	/// 이벤트처리에서 제외한다.
	bool mExceptionEvent;

	/// LButton 클릭 이벤트를 스테이지에 전달한다.
	bool mIsSetCursorDefault;

	bool mProcessEnable;

	bool mUseBoldText;

	/// npc 대화상태에 들어가기전 visible상태 백업 여부
	bool mBackupVisible;

	/// UI 폰트 타입 설정
	unsigned char mFontType;
};

inline void cUINode::NetLock( int lockTry )
{
	mNetLock = true;
	mLockTry = lockTry;
	SetEnabled( false );

	if( GetCapture() == this )
		ReleaseCapture();
}

inline void cUINode::NetUnLock( int lockTry )
{
	if( IsNetLock() == false )
		return; 

	if( mLockTry == lockTry )
	{
		mNetLock = false;
		mLockTry = -1;
		SetEnabled( true );
	}
	else
	{
		assert(0);
	}
}

inline void cUINode::NetUnLockAllUI()
{
	if( mNetLock == true )
	{
		mNetLock = false;
		mLockTry = -1;
		SetEnabled( true );
	}
}

inline int cUINode::GetID() const
{
	return mID;
}

inline void cUINode::SetRelativeRect( const cUIRect& rect )
{
	mRelativeRect = rect;
	UpdateRect();
}

inline void cUINode::SetRelativePos( const cUIPos& pos  )
{
	mRelativeRect.SetPos( pos );
	UpdateRect();
}

inline void cUINode::SetRelativeSize( const cUISize& size )
{
	mRelativeRect.mRight = mRelativeRect.mLeft + size.mWidth;
	mRelativeRect.mBottom = mRelativeRect.mTop + size.mHeight;
	UpdateRect();
}

inline const cUIRect& cUINode::GetRelativeRect() const
{
	return mRelativeRect;
}

inline const cUIRect& cUINode::GetAbsoluteRect() const
{
	return mAbsoluteRect;
}

inline void cUINode::SetEnabled( bool enabled )
{
	mEnabled = enabled;
}

inline bool cUINode::IsEnabled() const
{
	return mEnabled;
}

inline float cUINode::GetAlpha()
{
	return mAlpha;
}

inline eUINodeType cUINode::GetType() const
{
	return mType;
}

inline const cString& cUINode::GetSkinName() const
{
	return mSkinName;
}

inline cUIContainer* cUINode::GetParent()
{
	return mpParent;
}

inline void cUINode::SetVisible( bool visible )
{
	mVisible = visible;
}

inline bool cUINode::IsVisible() const
{
	return mVisible;
}

inline bool cUINode::IsPress() const
{
	return mPressed;
}

inline bool cUINode::IsKindof( eUINodeType type ) const
{
	return (mType & type) == type;
}
