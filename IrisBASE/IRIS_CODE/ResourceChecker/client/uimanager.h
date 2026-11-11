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
//#include "BaseStage.h"
//#include "IndexGenerator.h"

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
//class cPlaneObject;
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
	cUISkin* GetSkin() const;
	
	/// 속성을 리턴 
	cUIProperty* GetProperty() const;

	LPCTSTR GetUIText( unsigned int textId );
	bool GetUIText( cStringT*, unsigned int textId );

	/// Npc
	cUINpcTelling* GetNpcTelling( unsigned int npcId ) const;
	cUIFuncTextNode* GetNpcTextNode( unsigned int textId );

	/// page
	bool AddPage( unsigned long pageIdx, cPage* page );
	cPage* GetPage( unsigned long pageIdx );

	/// 아이콘
	cIconParam* GetIconParam( unsigned int iconId );

protected:
	/// 로딩
	bool LoadIDList( const cString& pathName );
	bool LoadSkin( const cString& pathName );
	bool LoadProperty( const cString& pathName );
	bool LoadSkinText( const cString& pathName );

	/// 이미지 폰트
	bool LoadImageFonts( const cString& pathName );

	/// 아이콘
	bool LoadIconParams( const cString& pathName );

	/// Npc
	bool LoadNpcText( const cString& pathName );
	bool LoadNpcScripts( const cString& pathName );
	bool LoadNpcScript( const cString& pathName );
	bool LoadNpcScene( const cString& pathName );

public:
	/// 단일체를 리턴
	static cUIManager* GetSingleton();

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
//	cUIRootNode* mpStageRootNode[eStage_Max];

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
	//cIndexGenerator<unsigned long>		mFrontIdGen;

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
// 	cCenterNoticeText* mNoticePrintText;
// 	cCenterNoticeText* mEnhancePrintText;

	// 화면 출력용 이미지
	//cPlaneObject* mpModeImage;

	// 
	//cIndexGenerator<unsigned long>	mImageIndexGenerator;

	// pair<cPlaneObject*, bool show>
	// 이미지 관리 : 윈도우에 소속되지 않은 출력용 이미지들 (image ID, cImagePair )
	typedef tPair<void*, bool> cImagePair; // 
	typedef tHashMap<unsigned long, cImagePair> cImageHashMap;
	cImageHashMap	mImageHashMap;

	///
	typedef tPointerList<void*> cManagedSortUIList;
	cManagedSortUIList mManagedSortUIList;
};

inline cUIManager* cUIManager::GetSingleton()
{
	return mpSingleton;
}

inline cUISkin* cUIManager::GetSkin() const
{
	return mpSkin;
}

inline cUIProperty* cUIManager::GetProperty() const
{
	return mpProperty;
}

#define UIMAN cUIManager::GetSingleton()

