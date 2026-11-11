/* ==========================================================================
* 파    일 : ChatWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-28
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const int HEIGHT_GAP = 8;
const int MAXROWNUM = 20;
const int DEFAULT_ROWNUM = 8;

enum eChatMsgKind;
enum eSystemMsgKind;
enum ePLAYER_JOB;
class cTextBox;
class cEditBox;
class cButton;
class cPlaneObject;
class cUIImage;
class cChatButtonWindow;
class cItemIcon;
class cPushButton;
class cChatWindow : public cUIWindow
{
public:
	cChatWindow();
	virtual ~cChatWindow();

	virtual void Open();
	virtual void Close();

	/// 외부에서 채팅박스에 직접적으로 문자열을 넣을 경우 사용
	virtual void InsertText( eChatMsgKind msgkind, unsigned long textIndex, LPCTSTR msg, unsigned long color, bool checkPress = true, bool paste = false );
	virtual void InsertTextSystem( eSystemMsgKind kind, LPCTSTR chatmsg );
	
	/// 아이템 획득 메세지용
	virtual void InsertItemText( unsigned long itemIndex, unsigned short slotIndex, LPCTSTR msg1, LPCTSTR msg2, bool IsQuest = false );

	/// 아이템 알림 메세지용
	virtual void InsertItemText( sInventory inventory, LPCTSTR name, LPCTSTR msg,  ePLAYER_JOB job, eChatMsgKind kind, void* exdata = NULL );
	
	/// 클리어
	virtual void Clear();

	/// 에디트 포커스 주기 
	void SetFocusToEdit();

	/// 채팅 에디트에 해당 문자열을 입력한다 
	void SetChatText( LPCTSTR text );

	/// 귓속말 친구 이름 등록
	void AddWhisperUser( const cStringT& name );
	void ApplyLastWhisperUser();
	void AddExtraWindow( cUIWindow* w );

	///	현재 선택중인 채팅 타입 갱신
	void UpdateChatType( eChatMsgKind type );

	/// 링크 아이템 등록
	bool AddLinkItem( unsigned short slotIndex, unsigned long itemIndex );
	void DeleteLinkItem();
	unsigned short GetLinkSlotIndex();
	unsigned long GetLinkItemIndex();
	bool IsLinked();

	///
	cUIRect GetSelectButtonRect();
	LPCTSTR GetWhisperName();
	unsigned int GetMaxRowInPage();

	///
	void InitBoxSize( unsigned int size );
	virtual void SetPress( bool normal, bool party, bool guild, bool shout, bool trade, bool whisper, bool system, bool megaphone );
	virtual bool IsPressNormal();
	virtual bool IsPressParty();
	virtual bool IsPressGuild();
	virtual bool IsPressShout();
	virtual bool IsPressTrade();
	virtual bool IsPressWhisper();
	virtual bool IsPressSystem();
	virtual bool IsPressMegaPhone();

	// 이동 기능 고정
	bool IsLockedMove();
	void SetMoveLock( bool lock );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateRect();
	bool HandleEvent( const cUIEvent& event );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnMouseMove( const cUIPos& pos );
	void OnEditBoxEntered( cUINode* pcaller, unsigned int id );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnButtonDowned(cUINode* pcaller, unsigned int id);
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );
	void UpdateSkin();
	void OnShow();
	void OnHide();

	/// 사이즈 업데이트 
	virtual void UpdateSize( int height );

	void KeyEvent( eKeyCode code );

private:
	/// 분리 채팅 윈도우 리스트 
	/// typedef tPointerList<cChatWindow*> cExtraList;
	typedef tPointerList<void*> cExtraList;
	cExtraList mExtraList;

	/// 귓속말 주고 받은 유저 저장 ( 최대 5개까지 )
	typedef tList<cStringT> cWhisperList;
	cWhisperList mWhisperList;

protected:
	cTextBox*		mpChatBox;
	cEditBox*		mpChatEdit;
	cEditBox*		mpChatEditWhisper;

	/// 채팅 텍스트박스의 늘어나는 배경.
	cPlaneObject*	mpChatBoxImage;
	cPlaneObject*	mpChatBoxImageUp;		/// 배경 윗쪽 틀
	cPlaneObject*	mpChatBoxImageDown;		/// 배경 아래쪽 틀

	/// 옵션 부분 이미지 
	cUIImage*		mpOptionImage;

	/// 선택한 채팅상태용 이미지
	cUIImage*		mpSelectImage;
	
	///
	cUINodeSkin*	mpSelectSkin;
	cUINodeSkin*	mpOptionSkin;
	cUINodeSkin*	mpUpSkin;

	/// 현재 채팅상태를 나타낼 텍스쳐 좌표 저장
	cUINodeSkin*	mpTexSkin[8];

	/// 기본 열 수 
	unsigned int mDefalutRowNum;

	/// 귓속말 순서
	int	mCount;

	/// 현재 채팅 상태
	eChatMsgKind	mSelectType;

	////
	cChatButtonWindow*	mpButtonWindow;
	
	/// 링크 아이템
	cItemIcon* mpLinkItemIcon;
	unsigned long mLinkItemIndex;

	///
	cUINode*	mpNormal;
	cUINode*	mpParty;
	cUINode*	mpGuild;
	cUINode*	mpShout;
	cUINode*	mpTrade;
	cUINode*	mpWhisper;
	cUINode*	mpSystem;
	cUINode*	mpMegaPhone;

	// 이동 고정 기능
	cPushButton*	mpMoveLock;
};

/// 버튼 윈도우
class cChatButtonWindow : public cUIWindow
{
public:
	cChatButtonWindow();
	virtual ~cChatButtonWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnMouseOtherClick( const cUIPos& pos );
	void UpdateSkin();

private:
	cUINode*	mpNormal;
	cUINode*	mpParty;
	cUINode*	mpGuild;
	cUINode*	mpShout;
	cUINode*	mpTrade;
	cUINode*	mpWhisper;
};
