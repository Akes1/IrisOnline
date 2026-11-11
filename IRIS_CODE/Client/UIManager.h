/* ==========================================================================
* 파    일 : UIManager.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.25
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIEvent.h"
#include "UISkin.h"
#include "UIProperty.h"
#include "UIRect.h"
#include "BaseStage.h"
#include "IndexGenerator.h"

#include "Item_Common.h"

const unsigned int MAXEDIT_DEFAULTLENGTH = 8;
const unsigned int DEFAULT_SCREENWIDTH = 1024;
const unsigned int DEFAULT_SCREENHEIGHT = 768;

const int MODEIMAGE_W = 256;
const int MODEIMAGE_H = 60;
const int MODEIMAGE_Y = 100;

const int LANGUAGE_ICON_INDEX = 10000;

class cUINode;
class cUIContainer;
class cUIRootNode;
class cUIWindow;
class cUICursor;
class cUIIDList;
class cUIText;
class cUIImageFont;
class cUIFuncText;
class cUIFuncTextNode;
class cUINpcTelling;
class cMsgBox;
class cDragWindow;
class cUnionDragWindow;
class cUnionIcon;
class cApplication;
class cIconParam;
class cBaseIcon;
class cUIRoundGauge;
class cPage;
class cCenterText;
class cCenterNoticeText;
class cModeCenterImage;
class cPlaneObject;
struct sSceneData;

class cTipWindow;
class cItemTipWindow;
class cSkillTipWindow;
class cBuffToolTipWindow;
class cSimpleTipWindow;
class cChatTipWindow;
class cQuestTipWindow;

enum eMSGBox_Type;
enum eMSGBox_Event;
enum eBoxSkin_Type;
enum eDragFrom;
enum eTip_Kind;

struct sPlaneImage
{
	cPlaneObject* mpImage;
	bool		  mShow;
	bool		  mLeftTop;	// 좌표 기준 ( 1 : left top, 0 : right bottom )
	cUIPos		  mOriginPos;
	unsigned char mAlign;

	sPlaneImage()
	{
		mpImage = 0;
		mShow = false;
		mLeftTop = true;
		mOriginPos.mX = 0;
		mOriginPos.mY = 0;
		mAlign = 0;
	}
};

/// UI 관리자
class cUIManager
{
	friend class cApplication;
	static cUIManager* mpSingleton;

public:
	cUIManager();
	~cUIManager();

	/// 초기화
	bool Init();
	void Open();
	void Close();

	void SetCurrentStage( eSTAGE stageIndex );

	/// 최상위로 올라가야하는 Node를 등록한다.
	void GotoFrontNode( cUINode* node );
	void RemoveFrontNode( unsigned long frontId );

	/// 메세지박스들을 맨앞으로 보낸다
	void GotoFrontMsgBox();

	/// 윈도우 닫기
	bool HideESC();
	void NetUnLockAllUI();

	// stage 변경시점
	void UIOpen();
	void UIClose();
	
	/// 스킨 체인지
	void ChangeSkin( const cString& pathName );

	/// 업데이트
	void Process( unsigned long deltaTime, unsigned long accumTime );

	void PreRender();
	void Render();

	/// root visible 세팅
	void ShowGameUI();
	void HideGameUI();

	// 메세지박스 생성
	bool CreateMsgBox( eSTAGE handler, eMSGBox_Type modaltype, eMSGBox_Event eventtype, eBoxSkin_Type skintype, LPCTSTR text, LPCTSTR caption, 
							void* extraData = 0, cUIWindow* caller = 0, unsigned long total = 0, bool printTime = false, bool result = false, 
						   unsigned int enableLen = 0, unsigned int maxLen = MAXEDIT_DEFAULTLENGTH );

	void SetModal( cUIWindow* window );

	void DestroyMsgBox( eMSGBox_Event eventType );
	void DestroyMsgBox( cUIWindow* window );
	void DestroyAllMsgBox();		/// msgbox 타입만 찾아서 삭제

	// 최상단 메세지 박스에 이벤트 전달
	void SendEventTopMsgBox( bool result );

	void SendEventToMsgbox( eMSGBox_Event eventtype, bool result );
	void ChangeTextMsgBox( eMSGBox_Event type, LPCTSTR text );		// 텍스트 변경

	/// 사용자로부터 이벤트를 통지
	bool PostEventFromUser( const cUIEvent& event );

	/// 새로운 윈도우 show 시 리셋할 데이터처리
	void CheckEventNode( cUINode* n );

	/// 마우스 캡쳐 노드를 설정 및 해제
	void SetCaptureNode( cUINode* node );
	void ReleaseCaptureNode( cUINode* node );

	/// 호버노드 해제
	void ReleaseHoverNode( cUINode* node );

	/// 포커스 노드를 설정 및 해제
	void SetFocusedNode( cUINode* node );
	void ReleaseFocusedNode( cUINode* node );

	/// 마우스 이벤트 받는 노드 초기화( 호버, 캡쳐 )
	void ReleaseEventNode();

	/// 현재 등록된 포커스 노드를 강제 제거한다.
	void ClearFocusedNode();

	/// 팁
	void ShowItemTip( eTip_Kind tipkind, const cUIPos& pos, unsigned long itemIndex, unsigned short slotIndex, bool forcedShowEquipTip );
	void ShowItemTip( eTip_Kind tipkind, const cUIPos& pos, unsigned long itemIndex, unsigned char enhance );
	void ShowOtherItemTip( eTip_Kind tipkind, const cUIPos& pos, sInventory& inven );
	void ShowSkillTip( eTip_Kind tipkind, cSkillTipWindow* pTip, const cUIPos& pos, unsigned long skillIdx, unsigned char stepIdxskill, unsigned long jobIdx = ULONG_MAX);
	void ShowBuffTip( cBuffToolTipWindow* pTip, const cUIPos& pos, unsigned long uniqueIdx, unsigned long influenceIdx, bool showTime = true );
	void ShowCommunityTip( cSimpleTipWindow* pTip, const cUIPos& pos, cStringT str );
	void ShowQuestTip( cQuestTipWindow* pTip, const cUIPos& pos );
	void ShowChatItemTip( const cUIPos& pos, sInventory& inven );
	void ShowUITip( const cUIPos& pos, unsigned long tipIdx );
	void HideTip();

	/// 드래그 윈도우
    void SetDragWindow( cDragWindow* dragWindow );
	void ShowDrag( cUIWindow* pcaller, eDragFrom dragFrom, cBaseIcon* icon, const cUIPos& pos, bool shift );
	bool IsVisibleDrag();
	
	void SetUnionDragWindow( cUnionDragWindow* win );
	void ShowUnionDrag( cUIWindow* pcaller, cUnionIcon* icon, const cUIPos& pos );
	bool IsVisibleUnionDrag();

	void HideDrag( cUIWindow* pcaller );

	/// npc 대화상태에서 열렸던 윈도우들( 대화상태 끝나면 닫아야 하는 윈도우리스트 )
	void AddCloseByNpcHide( cUIWindow* n );
	void CloseByNpcHide();
	
	/// 윈도우 show 예약
	void SetRestoreShow( cUIWindow* n );

	/// 특별한 윈도우들의 show 복구 ( npc 창 닫았을시 )
	void RestoreShow();

	/// 자기 자신이 아닌 다른 윈도우가 마우스 클릭 이벤트 받는경우 통지 메세지 발생
	void AddEventOtherClick( cUIWindow* win );

	/// 채팅 아이템 팁 윈도우
	void SetChatItemTipWindow( cChatTipWindow* win );

	/// 현재 스킨을 리턴
	cUISkin* GetSkin() const;

	/// 속성을 리턴 
	cUIProperty* GetProperty() const;

	/// UI ID 리스트 포인터 리턴
	cUIIDList* GetUIIDList() const;

	/// 루트 노드를 리턴
	cUIRootNode* GetRootNode() const;

	/// 호버 노드를 리턴 
	cUINode* GetHoverNode() const;

	/// 마우스 캡쳐 노드를 리턴
	cUINode* GetCaptureNode() const;

	/// 포커스 노드를 리턴
	cUINode* GetFocusedNode() const;

	/// 
	cUINode* FindNodeByID( unsigned long id );
	
	cUIWindow* GetTopMsgBox() { return mpTopMsgBox; }
	bool IsShowModalBox() { return (mModalList.IsEmpty() == false); }
	bool IsShowModalessBox() { return (mModallessList.IsEmpty() == false); }

	/// 가장가까운 컨테이너 리턴 
	cUIContainer* UpdateNearContainer( cUIContainer* con, bool axisX ) const;

	LPCTSTR GetUIText( unsigned int textId );
	bool GetUIText( cStringT*, unsigned int textId );

	/// 이미지 폰트
	cUIImageFont* GetImageFont( unsigned int fontIndex );

	/// 아이콘
	cIconParam* GetIconParam( unsigned int iconId );

	/// Npc
	cUINpcTelling* GetNpcTelling( unsigned int npcId ) const;
	cUIFuncTextNode* GetNpcTextNode( unsigned int textId );

	/// page
	bool AddPage( unsigned long pageIdx, cPage* page );
	cPage* GetPage( unsigned long pageIdx );

	/// scene
	sSceneData* GetSceneData( unsigned long sceneIdx );

	// 상위부모로 올라가면서 con이 부모로 포함되는지 검사
	bool IsIncludeParent( cUINode* node, cUIContainer* con );

	bool IsCameraEnableContainUI();

	/// 화면 출력 텍스트
	void AddNoticeText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height );
	void DeleteNoticeText();

	void AddEnhanceText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height );
	void DeleteEnhanceText();

	//
	void AddDummyNoticeText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height );
	void DeleteDummyText();

	//
	void SetMonModeImage( cPlaneObject* p );
	void ClearMonModeImage( cPlaneObject* p );

	void AddManagedSortUI( cUINode* node );
	inline tPointerList<void*>* GetManagedSortUIList() { return &mManagedSortUIList; }

	//
	bool ModifyText( LPCTSTR text, TCHAR* out, int outSize, int maxWidth );

	// 이미지 관리
	unsigned long AddImage( cPlaneObject* pImage, bool show, int originX, int originY, unsigned char align = 0, bool lefttop = true );
	void DeleteImage( unsigned long id );
	void UpdateImage( unsigned long id, bool show );
	cPlaneObject* GetImage( unsigned long id );

	// npc 대화버튼들
	cUIWindow* AddNpcTalkButton( unsigned long npcIndex );
	void DeleteNpcTalkButton( unsigned long npcIndex );

protected:
	/// 로딩
	bool LoadIDList( const cString& pathName );
	bool LoadSkin( const cString& pathName );
	bool LoadProperty( const cString& pathName );
	bool LoadSkinText( const cStringW& pathName );

	/// 이미지 폰트
	bool LoadImageFonts( const cString& pathName );

	/// 아이콘
	bool LoadIconParams( const cString& pathName );

	/// Npc
	bool LoadNpcText( const cStringW& pathName );
	bool LoadNpcScripts( const cString& pathName );
	bool LoadNpcScript( const cString& pathName );
	bool LoadNpcScene( const cString& pathName );

	/// Reload 기능( 스킨스크립트 변경시 리로드 )
	bool ReloadSkin( const cString& pathName );

	/// 마우스 캡쳐 노드를 갱신
	void UpdateHoveredNode( cUIContainer* root, const cUIPos& pos );

	/// 자신이 아닌 다른 영역 클릭
	void EventOtherClick( const cUIPos& pos );

	void ShowTip();

	bool MsgBoxEventProcess( const cUIEvent& event );

	/// 최상위 등록 Node가 있으면 적용한다.
	void ApplyFrontNode( cUINode* node );

	bool SetEquipItemTip( const cUIPos& pos, sInventory& inven, bool forcedShow );

public:
	/// 단일체를 리턴
	static cUIManager* GetSingleton();

	void UpdateScreenSize( unsigned int width, unsigned int height );

protected:
	/// 스킨
	cUISkin* mpSkin;
	/// 속성 
	cUIProperty* mpProperty;
	/// 텍스트
	cUIText* mpSkinText;
	/// UI ID 리스트 
	cUIIDList* mpIDList;
	/// 루트 노드
	cUIRootNode* mpRootNode;
	cUIRootNode* mpStageRootNode[eStage_Max];

	/// 마우스 캡쳐 노드
	cUINode* mpCaptureNode;
	/// 마우스 오버 노드
	cUINode* mpHoveredNode;
	/// 포커스 노드
	cUINode* mpFocusedNode;

	/// 메시지 박스
	cUIWindow* mpTopMsgBox;
	
	/// 마우스 이벤트 발생시의 포지션값 저장 
	cUIPos	mMousePos;

	/// 팁
	cTipWindow* mTipWindow;

	/// 채팅 팁 윈도우
	cChatTipWindow* mChatItemTipWindow;

	/// 드래그윈도우
	cDragWindow* mpDragWindow;
	cUnionDragWindow* mpUnionDragWindow;
	
	/// 제거 되어질 MsgBox들에 대한 처리..
	typedef	tPointerList<void*> cDestroyMsgBoxList;
	cDestroyMsgBoxList	mDestroyMsgBoxList;

	typedef tPointerHashMap<unsigned long, void*> cApplyFrontMap;
	cApplyFrontMap						mApplyFrontMap;
	cIndexGenerator<unsigned long>		mFrontIdGen;

	/// 이미지 폰트
	typedef tHashMap<unsigned int, void*> cImageFontMap;
	cImageFontMap mImageFontMap;

	/// 아이콘
	typedef tHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mIconTextureMap;

	typedef tHashMap<unsigned int, void*> cIconParamMap;
	cIconParamMap mIconParamMap;

	/// Npc
	cUIFuncText* mpNpcText;
	typedef tHashMap<unsigned int, void*> cNpcTellingMap;
	cNpcTellingMap mNpcTellingMap;

	/// 페이지 관리용 (페이지인덱스, 페이지 내용)
	typedef tPointerHashMap<unsigned long, void*> cPageMap;
	cPageMap mPageMap;

	/// 페이지 장면 이미지 스크립트
	typedef tPointerHashMap<unsigned long, void*> cSceneMap;
	cSceneMap mSceneMap;

	/// npc 윈도우가 끝나면 보여줘야할 윈도우 
	typedef tArray<void*> cRestoreShow;
	cRestoreShow mRestore;

	/// 메세지박스 관리용
	typedef tPointerList<void*> cMsgBoxList;
	cMsgBoxList	mModalList;

	typedef tPointerList<void*> cMsgBoxList;
	cMsgBoxList	mModallessList;

	/// eUIEVENT_MOUSE_OTHERCLICK 통지 메세지 보낼 UI 목록
	typedef tPointerList<void*> cOtherClickList;
	cOtherClickList	mOtherClickList;

	/// npc 윈도우가 닫힐때 같이 닫아줘야하는 윈도우들
	typedef tPointerArray<void*> cCloseByNpcHideArr;
	cCloseByNpcHideArr mCloseByNpcHideArr;

	/// 공지 출력용
	cCenterNoticeText* mNoticePrintText;
	cCenterNoticeText* mEnhancePrintText;
	cCenterNoticeText* mDummyNoticeText;

	// 화면 출력용 이미지
	cPlaneObject* mpModeImage;

	// 
	cIndexGenerator<unsigned long>	mImageIndexGenerator;

	// sPlaneImage
	typedef tHashMap<unsigned long, void*> cImageHashMap;
	cImageHashMap	mImageHashMap;

	///
	typedef tPointerList<void*> cManagedSortUIList;
	cManagedSortUIList mManagedSortUIList;

	typedef tPointerHashMap<unsigned long, void*> cNpcTalkButtonMap;
	cNpcTalkButtonMap	mTalkButtonMap;
};

inline void cUIManager::SetCurrentStage( eSTAGE i )
{
	mpRootNode = mpStageRootNode[i];
}

inline void cUIManager::SetDragWindow( cDragWindow* dragWindow )
{
	if( dragWindow )
		mpDragWindow = dragWindow;
}

inline void cUIManager::SetUnionDragWindow( cUnionDragWindow* win )
{
	if( win )
		mpUnionDragWindow = win;
}

inline void cUIManager::SetChatItemTipWindow( cChatTipWindow* win )
{
	if( win )
		mChatItemTipWindow = win;
}

inline cUISkin* cUIManager::GetSkin() const
{
	return mpSkin;
}

inline cUIProperty* cUIManager::GetProperty() const
{
	return mpProperty;
}

inline cUIIDList* cUIManager::GetUIIDList() const
{
	return mpIDList;
}

inline cUIRootNode* cUIManager::GetRootNode() const
{
	return mpRootNode;
}

inline cUINode* cUIManager::GetHoverNode() const
{
	return mpHoveredNode;
}

inline cUINode* cUIManager::GetCaptureNode() const
{
	return mpCaptureNode;
}

inline cUINode* cUIManager::GetFocusedNode() const
{
	return mpFocusedNode;
}

inline cUIManager* cUIManager::GetSingleton()
{
	return mpSingleton;
}

#define UIMAN cUIManager::GetSingleton()
