#include "StdAfx.h"
#include "UIManager.h"
#include "UIRootNode.h"
#include "MsgBox.h"
#include "TipWindow.h"
#include "DragWindow.h"
#include "UnionIcon.h"
#include "UIIDList.h"
#include "UIText.h"
#include "InputSystem.h"
#include "StageManager.h"
#include "FontAgent.h"
#include "UIFuncText.h"
#include "UINpcTelling.h"
#include "ItemManager.h"
#include "ItemTipWindow.h"
#include "SkillTipWindow.h"
#include "BuffToolTipWindow.h"
#include "SimpleTipWindow.h"
#include "PlaneObject.h"
#include "Page.h"
#include "PartyManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameUIManager.h"
#include "RenderSystem.h"
#include "PartyUnionManager.h"
#include "UIPrintImageText.h"
#include "SceneManager.h"
#include "Application.h"
#include "RButtonMenuWindow.h"

cUIManager* cUIManager::mpSingleton = 0;

cUIManager::cUIManager()
: mMousePos(0, 0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	for( unsigned int i = 0; i < eStage_Max; ++i )
	{
		mpStageRootNode[i] = new cUIRootNode;
	}

	mpRootNode = 0;
	mpSkinText = 0;
	mpIDList = 0;
	mpSkin = 0;
	mpProperty = 0;
	mpCaptureNode = 0;
	mpHoveredNode = 0;
	mpFocusedNode = 0;
	mpTopMsgBox = 0;
	mTipWindow = 0;
	mpDragWindow = 0;
	mpUnionDragWindow = 0;
	mChatItemTipWindow = 0;
	mNoticePrintText = 0;
	mEnhancePrintText = 0;
	mDummyNoticeText = 0;
	mpModeImage = 0;

	/// Npc
	mpNpcText = 0;
	
	mNoticePrintText = new cCenterNoticeText;
	mEnhancePrintText = new cCenterNoticeText;
	mDummyNoticeText = new cCenterNoticeText;
}

cUIManager::~cUIManager()
{
	mpModeImage = 0;

	SAFE_DELETE( mDummyNoticeText );
	SAFE_DELETE( mEnhancePrintText );
	SAFE_DELETE( mNoticePrintText );
	SAFE_DELETE( mpNpcText );
	SAFE_DELETE( mpSkin );
	SAFE_DELETE( mpSkinText );
	SAFE_DELETE( mpProperty );
	SAFE_DELETE( mpIDList );

	// 
	{
		cImageHashMap::cIterator i = mImageHashMap.Begin();
		cImageHashMap::cIterator end = mImageHashMap.End();
		for( ; i != end; ++i )
		{
			sPlaneImage* p = (sPlaneImage*)(i->mSecond);
			if( p )
				SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
		mImageHashMap.Clear();
	}

	mImageIndexGenerator.RestoreAll();

	/// 이미지 폰트
	{
		cImageFontMap::cIterator i = mImageFontMap.Begin();
		cImageFontMap::cIterator iend = mImageFontMap.End();

		for( ; i != iend; ++i )
		{
			delete (cUIImageFont*)(i->mSecond);
		}
	}

	/// 아이콘
	{
		cIconParamMap::cIterator i = mIconParamMap.Begin();
		cIconParamMap::cIterator iend = mIconParamMap.End();

		for( ; i != iend; ++i )
		{
			delete (cIconParam*)(i->mSecond);
		}
	}

	/// NPC
	{
		cNpcTellingMap::cIterator i = mNpcTellingMap.Begin();
		cNpcTellingMap::cIterator iend = mNpcTellingMap.End();

		for( ; i != iend; ++i )
		{
			delete (cUINpcTelling*)(i->mSecond);
		}
	}

	/// page
	{
		cPageMap::cIterator i = mPageMap.Begin();
		cPageMap::cIterator end = mPageMap.End();

		for( ; i != end; ++i )
		{
			delete (cPage*)(i->mSecond);
		}
		mPageMap.Clear();
	}

	/// scene
	{
		cSceneMap::cIterator i = mSceneMap.Begin();
		cSceneMap::cIterator end = mSceneMap.End();

		for( ; i != end; ++i )
		{
			delete (sSceneData*)(i->mSecond);
		}
		mSceneMap.Clear();
	}

	for( unsigned int i = 0; i < eStage_Max; ++i )
	{
		SAFE_DELETE( mpStageRootNode[i] );
	}

	/// 메세지 박스
	mModalList.Clear();
	mModallessList.Clear();

	///
	mOtherClickList.Clear();

	mpTopMsgBox = 0;
	mpSingleton = 0;

	mManagedSortUIList.Clear();
}

bool cUIManager::Init()
{
	mpIDList = new cUIIDList;
	if( !mpIDList )
	{
		assert(0 && "mpIDList not create!" );
		return false;
	}

	mpProperty = new cUIProperty;
	if( !mpProperty )
	{
		assert(0 && "mpProperty not create!" );
		return false;
	}

	mpSkinText = new cUIText;
	if( !mpSkinText )
	{
		assert(0 && "mpSkinText not create!" );
		return false;
	}

	mpSkin = new cUISkin;
	if( !mpSkin )
	{
		assert(0 && "mpskin not create!" );
		return false;
	}

	if( LoadIDList( "./Script/Interface/List_ID.txt" ) == false )
	{
		assert(0 && "LoadIDList fail" );
		return false;
	}

	if( LoadProperty( "./Script/Interface/List_Property.txt" ) == false )
	{
		assert(0 && "LoadProperty fail" );
		return false;
	}

	cStringW path;
	path.Format( L"./language/%s/InterfaceMsg.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( LoadSkinText( path.Cstr() ) == false )
	{
		assert(0 && "LoadSkinText fail" );
		return false;
	}

	if( LoadSkin( "./Script/Interface/List_SkinImage.txt" ) == false )
	{
		assert(0 && "LoadSkin fail" );
		return false;
	}

	if( LoadIconParams( "./Script/Interface/Path_Icon.txt" ) == false )
	{
		assert(0 && "LoadIconParams fail" );
		return false;
	}

	if( LoadImageFonts( "./Script/Interface/Path_Font.txt" ) == false )
	{
		assert( 0 && "failed to load ui image fonts" );
		return false;
	}

	//
	mpNpcText = new cUIFuncText;

	cStringT pathname;
	pathname.Format( L"./language/%s/NPC_Note.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( LoadNpcText( pathname.Cstr() ) == false )
	{
		assert( 0 && "failed to load LoadNpcText" );
		return false;
	}
	if( LoadNpcScript( "./Script/Resource/NPC_Func.txt" ) == false )
	{
		assert( 0 && "failed to load LoadNpcScript" );
		return false;
	}
	if( LoadNpcScene( "./Script/Resource/NPC_Scene.txt" ) == false )
	{
		assert( 0 && "failed to load LoadNpcScene" );
		return false;
	}
	return true;
}

void cUIManager::Open()
{
	UIOpen();
}

void cUIManager::Close()
{
	ReleaseEventNode();
	ClearFocusedNode();

	if( mTipWindow )
		HideTip();

	if( mChatItemTipWindow && mChatItemTipWindow->IsVisible() )
		mChatItemTipWindow->Hide();

	if( mpDragWindow )
		mpDragWindow->Hide();

	if( mpUnionDragWindow )
		mpUnionDragWindow->Hide();

	if( HERO && HERO->IsOpenNpcTalk() == true )
		HERO->CloseNpcTalk();

	mFrontIdGen.RestoreAll();
	mApplyFrontMap.Clear();

	/// 공지 사항 지우기
	DeleteNoticeText();
	DeleteEnhanceText();

	/// 모든 ui의 lock 풀기 
	NetUnLockAllUI();	

	//
	UIClose();

	// 
	DestroyAllMsgBox();

	if( !mDestroyMsgBoxList.IsEmpty() )
	{
		cDestroyMsgBoxList::cIterator i = mDestroyMsgBoxList.Begin();
		cDestroyMsgBoxList::cIterator end = mDestroyMsgBoxList.End();

		for( ; i != end; ++i )
		{
			cMsgBox* pbox = (cMsgBox*)(*i);
			GetRootNode()->RemoveChildComplete( pbox );
		}
		mDestroyMsgBoxList.Clear();
	}
}

/// 주의사항 :
bool cUIManager::LoadIDList( const cString& pathName )
{
	/// 스킨 로딩 
	if( mpIDList->Load( pathName ) == false )
	{
		assert( 0 && "failed to load idlist" );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUIManager::LoadSkin( const cString& pathName )
{
	/// 스킨 로딩 
	if( mpSkin->Load( pathName ) == false )
	{
		assert( 0 && "failed to load ui skin" );
		return false;
	}
	return true;
}

bool cUIManager::LoadProperty( const cString& pathName )
{
	/// 속성 로딩
	if( mpProperty->Load( pathName ) == false )
	{
		assert( 0 && "failed to load ui property" );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUIManager::LoadSkinText( const cStringW& pathName )
{
	if( mpSkinText->Load( pathName ) == false )
	{
		assert( 0 && "failed to load skin text" );
		return false;
	}
	return true;
}

///
bool cUIManager::ReloadSkin( const cString& pathName )
{
	/// 스킨 로딩 
	if( mpSkin->Reload( pathName ) == false )
	{
		assert( 0 && "failed to Reload ui skin" );
		return false;
	}
	return true;
}

/// 주의사항 : 
void cUIManager::ChangeSkin( const cString& pathName )
{
	if( ReloadSkin( pathName ) == false )
	{
		assert( 0 && "failed to Reload ui skin" );
		return;
	}

	mpRootNode->UpdateSkin();
}

/// 주의사항 : 
void cUIManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	mManagedSortUIList.Clear();

	if( !mDestroyMsgBoxList.IsEmpty() )
	{
		cDestroyMsgBoxList::cIterator i = mDestroyMsgBoxList.Begin();
		cDestroyMsgBoxList::cIterator end = mDestroyMsgBoxList.End();

		for( ; i != end; ++i )
		{
			cMsgBox* pbox = (cMsgBox*)(*i);
			GetRootNode()->RemoveChildComplete( pbox );
		}
		mDestroyMsgBoxList.Clear();
	}

	if( mApplyFrontMap.IsEmpty() == false )
	{
		cApplyFrontMap::cIterator i = mApplyFrontMap.Begin();
		cApplyFrontMap::cIterator end = mApplyFrontMap.End();
		for( ; i != end; ++i )
		{
			cUINode* node = (cUINode*)(*i).mSecond;
			ApplyFrontNode( node );
		}

		mFrontIdGen.RestoreAll();
		mApplyFrontMap.Clear();
	}

	mpRootNode->OnProcess( deltaTime, accumTime );

	if( mNoticePrintText )
		mNoticePrintText->Update( deltaTime, accumTime );

	if( mEnhancePrintText )
		mEnhancePrintText->Update( deltaTime, accumTime );

	if( mDummyNoticeText )
		mDummyNoticeText->Update( deltaTime, accumTime );

	//
	cImageHashMap::cIterator i = mImageHashMap.Begin();
	cImageHashMap::cIterator end = mImageHashMap.End();
	for( ; i != end; ++i )
	{
		sPlaneImage* p = (sPlaneImage*)(i->mSecond);
		if( p && p->mpImage && p->mShow )
			p->mpImage->Process( accumTime );
	}
}

void cUIManager::PreRender()
{
	if( mpRootNode )
		mpRootNode->OnPreRender();
}


/// 주의사항 : 
void cUIManager::Render()
{
	mpRootNode->OnRender();

	if( mpRootNode->IsVisible() )
	{
		// npc 대화중이 아닐때만 표시
		if( HERO && HERO->GetNpcIdx() == 0 )
		{
			cImageHashMap::cIterator i = mImageHashMap.Begin();
			cImageHashMap::cIterator end = mImageHashMap.End();
			for( ; i != end; ++i )
			{
				sPlaneImage* p = (sPlaneImage*)(i->mSecond);
				if( p && p->mpImage && p->mShow )
				{
					p->mpImage->Draw();
				}
			}

			if( mpModeImage )
				mpModeImage->Draw();
		}

		mpRootNode->OnRenderTip();

		if( mNoticePrintText )
			mNoticePrintText->Draw();

		if( mEnhancePrintText )
			mEnhancePrintText->Draw();

		if( mDummyNoticeText )
			mDummyNoticeText->Draw();
	}
}

void cUIManager::ShowGameUI()
{
	if( mpRootNode )
		mpRootNode->Show();

	if( HERO )
		HERO->ShowNameGauge( true );

	SCENEMAN->ShowDepthPlane( true );
}

void cUIManager::HideGameUI()
{
	if( mpRootNode )
		mpRootNode->Hide();

	if( HERO )
		HERO->ShowNameGauge( false );

	SCENEMAN->ShowDepthPlane( false );
}

// printTime : 시간 재는 메세지 박스의 경우, 남은 시간 출력 여부설정
// enableLen : edit 타입 메세지박스에서 enableLen 이상 글자가 입력되야 버튼 활성화
// maxLen : edit 에 들어갈 최대 글자 길이
bool cUIManager::CreateMsgBox( eSTAGE handler, eMSGBox_Type modaltype, eMSGBox_Event eventtype, eBoxSkin_Type skintype, LPCTSTR text, LPCTSTR caption, 
									 void* extraData, cUIWindow* caller, unsigned long total, bool printTime, bool result, unsigned int enableLen, unsigned int maxLen )
{
	// 검증
	if( !(modaltype == eMSGBOX_MODAL || modaltype == eMSGBOX_MODALLESS) )
	{
		assert(0);
		return false;
	}

	if( eventtype != eMBEVENT_NONE )
	{
		// 동일한게 맨앞에 떠있으면 리턴
		if( mpTopMsgBox && mpTopMsgBox->GetEvent() == eventtype )
			return true;

		DestroyMsgBox( eventtype );
	}

	cBaseStage* msgHandler = STAGEMAN->GetStage( handler );
	cMsgBox* box = new cMsgBox;
	if( box->Create( msgHandler, modaltype, eventtype, skintype, extraData ) == false )
	{
		assert( 0 && "failed to create modal msgbox" );
		SAFE_DELETE( box );
		return false;
	}

	// 세팅
	box->SetTime( total, printTime, result );
	box->SetMaxLength( maxLen );
	box->SetText( text, caption );
	box->SetEnableLength( enableLen );

	// 호출자에
	if( caller )
	{
		// 등록된 박스가 있으면 삭제
		cUIWindow* regist = caller->GetRegistMsgBox();
		if( regist )
		{
			DestroyMsgBox( regist );
		}

		caller->RegistMsgBox( box );
		box->SetCaller( caller );
	}

	ReleaseEventNode();
	ClearFocusedNode();

	//
	ShowGameUI();

	int size = 0;
	if( modaltype == eMSGBOX_MODAL )
	{
		mModalList.PushFront( (cUIWindow*)box );
		size = mModalList.GetSize();
	}
	else if( modaltype == eMSGBOX_MODALLESS )
	{
		mModallessList.PushFront( (cUIWindow*)box );
		size = mModallessList.GetSize();
	}

	// 모달박스를 맨앞으로 보낸다
	if( mModalList.IsEmpty() == false )
	{
		mpTopMsgBox = (cUIWindow*)mModalList.Front();
	}
	else if( mModallessList.IsEmpty() == false )
	{
		mpTopMsgBox = (cUIWindow*)mModallessList.Front();
	}

	box->Show();

	if( mpTopMsgBox )
		GotoFrontNode( mpTopMsgBox );

	// 위치 갱신
	if( size > 1 )
	{
		cUIRect rc = box->GetAbsoluteRect();
		box->SetRelativePos( cUIPos(rc.mLeft + (size*3), rc.mTop + (size*3)) );
	}
	return true;
}

// 삭제
void cUIManager::DestroyMsgBox( cUIWindow* window )
{
	if( window )
	{
		bool find = false;

		if( window->GetModalType() == eMSGBOX_MODAL )
		{
			// 검색 후 삭제
			cMsgBoxList::cIterator i = mModalList.Begin();
			cMsgBoxList::cIterator end = mModalList.End();
			for( ; i != end; ++i )
			{
				cUIWindow* p = (cUIWindow*)(*i);
				if( p && p == window )
				{
					mModalList.Erase( i );

					find = true;
					break;		
				}
			}
		}
		else if( window->GetModalType() == eMSGBOX_MODALLESS )
		{
			// 검색 후 삭제
			cMsgBoxList::cIterator i = mModallessList.Begin();
			cMsgBoxList::cIterator end = mModallessList.End();
			for( ; i != end; ++i )
			{
				cUIWindow* p = (cUIWindow*)(*i);
				if( p && p == window )
				{
					mModallessList.Erase( i );

					find = true;
					break;		
				}
			}
		}
		else
		{
			assert(0);
			return;
		}
		
		if( find == false )
		{
			assert(0);
			return;
		}

		ReleaseEventNode();

		window->SetVisible( false );

		// 자신을 등록한 윈도우에게 알림
		window->DeleteCallerMsgBox();

		// 메세지박스 종류면 삭제 리스트에 삽입
		if( window->IsKindof( eUINODE_MESSAGEBOX ) )
		{
			mDestroyMsgBoxList.PushBack( window );
		}

		// 다음 모달을 맨앞으로 보낸다.
		mpTopMsgBox = 0;

		if( mModalList.IsEmpty() == false )
		{
			mpTopMsgBox = (cUIWindow*)mModalList.Front();
		}
		else if( mModallessList.IsEmpty() == false )
		{
			mpTopMsgBox = (cUIWindow*)mModallessList.Front();
		}

		if( mpTopMsgBox )
			GotoFrontNode( mpTopMsgBox );
	}
	else
	{
		assert(0);
	}
}

// 이벤트 타입으로 검사해서 삭제
void cUIManager::DestroyMsgBox( eMSGBox_Event eventType )
{
	cUIWindow* window = 0;

	// 검색 후 삭제
	if( mModalList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModalList.Begin();
		cMsgBoxList::cIterator end = mModalList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);
			if( p && p->GetEvent() == eventType )
			{
				window = p;

				mModalList.Erase( i );
				break;		
			}
		}
	}

	if( window == 0 && mModalList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModallessList.Begin();
		cMsgBoxList::cIterator end = mModallessList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);
			if( p && p->GetEvent() == eventType )
			{
				window = p;

				mModallessList.Erase( i );
				break;		
			}
		}
	}

	if( window == 0 )
		return;

	ReleaseEventNode();

	window->SetVisible( false );

	// 자신을 등록한 윈도우에게 알림
	window->DeleteCallerMsgBox();

	// 메세지박스 종류면 삭제 리스트에 삽입
	if( window->IsKindof( eUINODE_MESSAGEBOX ) )
	{
		mDestroyMsgBoxList.PushBack( window );
	}

	// 다음 모달을 맨앞으로 보낸다.
	mpTopMsgBox = 0;

	if( mModalList.IsEmpty() == false )
	{
		mpTopMsgBox = (cUIWindow*)mModalList.Front();
	}
	else if( mModallessList.IsEmpty() == false )
	{
		mpTopMsgBox = (cUIWindow*)mModallessList.Front();
	}

	if( mpTopMsgBox )
		GotoFrontNode( mpTopMsgBox );
}

// 전체 메세지 박스 삭제
void cUIManager::DestroyAllMsgBox()
{
	ReleaseEventNode();
	ClearFocusedNode();

	mpTopMsgBox = 0;

	if( mModalList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModalList.Begin();
		cMsgBoxList::cIterator end = mModalList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* win = (cUIWindow*)(*i);
			if( win )
			{
				win->SetVisible( false );

				// 자신을 등록한 윈도우에게 알림
				win->DeleteCallerMsgBox();

				if( win->IsKindof( eUINODE_MESSAGEBOX ) )
				{
					mDestroyMsgBoxList.PushBack( win );
				}
			}
		}

		mModalList.Clear();
	}

	if( mModallessList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModallessList.Begin();
		cMsgBoxList::cIterator end = mModallessList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* win = (cUIWindow*)(*i);
			if( win )
			{
				win->SetVisible( false );

				// 자신을 등록한 윈도우에게 알림
				win->DeleteCallerMsgBox();

				if( win->IsKindof( eUINODE_MESSAGEBOX ) )
				{
					mDestroyMsgBoxList.PushBack( win );
				}
			}
		}

		mModallessList.Clear();
	}
}

void cUIManager::SetModal( cUIWindow* window )
{
	if( window )
	{
		int size = 0;
		if( window->GetModalType() == eMSGBOX_MODAL )
		{
			mModalList.PushFront( window );
			size = mModalList.GetSize();
		}
		else if( window->GetModalType() == eMSGBOX_MODALLESS )
		{
			mModallessList.PushFront( window );
			size = mModallessList.GetSize();
		}
		else
		{
			assert(0);
			return;
		}

		ReleaseEventNode();
		ClearFocusedNode();

		ShowGameUI();

		// 모달박스를 맨앞으로 보낸다
		if( mModalList.IsEmpty() == false )
		{
			mpTopMsgBox = (cUIWindow*)mModalList.Front();
		}
		else if( mModallessList.IsEmpty() == false )
		{
			mpTopMsgBox = (cUIWindow*)mModallessList.Front();
		}

		window->Show();

		if( mpTopMsgBox )
			GotoFrontNode( mpTopMsgBox );

		// 위치 갱신
		if( size > 1 )
		{
			cUIRect rc = window->GetAbsoluteRect();
			window->SetRelativePos( cUIPos(rc.mLeft + (size*3), rc.mTop + (size*3)) );
		}
	}
	else
	{
		assert(0);
		return;
	}
}

void cUIManager::ChangeTextMsgBox( eMSGBox_Event type, LPCTSTR text )
{
	// 모달 박스랑 동일한지 검사
	if( mpTopMsgBox && mpTopMsgBox->GetEvent() == type )
	{
		if( mpTopMsgBox->IsKindof( eUINODE_MESSAGEBOX ) )
		{
			cMsgBox* box = (cMsgBox*)mpTopMsgBox;
			box->ChangeText( text );
		}
		return;
	}

	if( mModalList.IsEmpty() == false )
	{
		// 리스트에서 검색
		cMsgBoxList::cIterator i = mModalList.Begin();
		cMsgBoxList::cIterator end = mModalList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);

			/// 동일한 윈도우 찾으면 해당
			if( p && p->GetEvent() == type && p->IsKindof( eUINODE_MESSAGEBOX ) )
			{
				cMsgBox* box = (cMsgBox*)p;
				if( box )
					box->ChangeText( text );

				return;
			}
		}
	}

	if( mModallessList.IsEmpty() == false )
	{
		// 리스트에서 검색
		cMsgBoxList::cIterator i = mModallessList.Begin();
		cMsgBoxList::cIterator end = mModallessList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);

			/// 동일한 윈도우 찾으면 해당
			if( p && p->GetEvent() == type && p->IsKindof( eUINODE_MESSAGEBOX ) )
			{
				cMsgBox* box = (cMsgBox*)p;
				if( box )
					box->ChangeText( text );

				return;
			}
		}
	}
}

void cUIManager::SendEventTopMsgBox( bool result )
{
	if( mpTopMsgBox )
	{
		cUIEvent event;
		event.mType = eUIEVENT_COMMAND;
		event.mID = ( result == true ) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO;
		mpTopMsgBox->HandleEvent( event );
	}
}

/// 해당 이벤트를 가진 메세지박스에 이벤트 전달
void cUIManager::SendEventToMsgbox( eMSGBox_Event eventtype, bool result )
{
	if( mpTopMsgBox && mpTopMsgBox->GetEvent() == eventtype )
	{
		cUIEvent event;
		event.mType = eUIEVENT_COMMAND;
		event.mID = ( result == true ) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO;
		mpTopMsgBox->HandleEvent( event );
		return;
	}

	if( mModalList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModalList.Begin();
		cMsgBoxList::cIterator end = mModalList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);
			if( p && p->GetEvent() == eventtype )
			{
				cUIEvent event;
				event.mType = eUIEVENT_COMMAND;
				event.mID = ( result == true ) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO;
				p->HandleEvent( event );
				
				return;
			}
		}
	}

	if( mModallessList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModallessList.Begin();
		cMsgBoxList::cIterator end = mModallessList.End();
		for( ; i != end; ++i )
		{
			cUIWindow* p = (cUIWindow*)(*i);
			if( p && p->GetEvent() == eventtype )
			{
				cUIEvent event;
				event.mType = eUIEVENT_COMMAND;
				event.mID = ( result == true ) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO;
				p->HandleEvent( event );

				return;
			}
		}
	}
}

bool cUIManager::HideESC()
{
	return mpRootNode->HideESC();
}

void cUIManager::NetUnLockAllUI()
{
	return mpRootNode->NetUnLockAllUI();
}

void cUIManager::UIOpen()
{
	return mpRootNode->Open();
}

void cUIManager::UIClose()
{
	return mpRootNode->Close();
}	


/// 주의사항 : 어떤 윈도우가 show 될때 리셋할 값 정하기
void cUIManager::CheckEventNode( cUINode* n )
{
	if( n == 0 )
		return;

	/// 팁 윈도우 닫기
	if( mTipWindow && mTipWindow->IsVisible() && n != mTipWindow )
		HideTip();

	/// 드래그 윈도우 닫기
	if( mpDragWindow && mpDragWindow->IsVisible() && n != mpDragWindow )
		mpDragWindow->Hide();

	if( mpUnionDragWindow && mpUnionDragWindow->IsVisible() && n != mpUnionDragWindow )
		mpUnionDragWindow->Hide();

	if( !mTipWindow && !mpDragWindow && !mpUnionDragWindow )
		UpdateHoveredNode( mpRootNode, cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
}

/// 주의사항 : 
void cUIManager::UpdateHoveredNode( cUIContainer* root, const cUIPos& pos )
{
	cUINode* pnewHovered = root->GetNode( pos );

	if( mpHoveredNode && mpHoveredNode != pnewHovered )
	{
		cUIEvent event;
		event.mType = eUIEVENT_MOUSE_LEFT;
		event.mPos = pos;
		mpHoveredNode->HandleEvent( event );
	}

	if( pnewHovered && pnewHovered != mpRootNode )
	{
		if( pnewHovered != mpHoveredNode )
		{
			cUIEvent event;
			event.mType = eUIEVENT_MOUSE_HOVERED;
			event.mPos = pos;
			pnewHovered->HandleEvent( event );

			mpHoveredNode = pnewHovered;
		}
	}
	else
	{
		mpHoveredNode = 0;
	}
}

void cUIManager::AddEventOtherClick( cUIWindow* win )
{
	if( win )
	{
		mOtherClickList.PushBack( win );
	}
}

void cUIManager::EventOtherClick( const cUIPos& pos )
{
	/// 각 UI 에 메세지 발생
	cOtherClickList::cIterator i = mOtherClickList.Begin();
	cOtherClickList::cIterator end = mOtherClickList.End();
	for( ; i != end; ++i )
	{
		cUIWindow* win = (cUIWindow*)(*i);
		if( win )
		{
			cUIEvent event;
			event.mType = eUIEVENT_MOUSE_OTHERCLICK;
			event.mPos = pos;
			win->HandleEvent( event );
		}
	}
}

/// 주의사항 : 
void cUIManager::SetCaptureNode( cUINode* n )
{
	assert( n && "null ui node" );

	if( mpCaptureNode != n )
	{
		if( mpCaptureNode )
		{
			ReleaseCaptureNode( mpCaptureNode );
		}
		mpCaptureNode = n;
	}
}

/// 주의사항 : 
void cUIManager::ReleaseCaptureNode( cUINode* n )
{
	assert( n && "null ui node" );

	if( mpCaptureNode == 0 )
		return;

	if( mpCaptureNode->GetParent() == n || 
		mpCaptureNode == n )
	{
		cUIEvent event;
		event.mType = eUIEVENT_CAPTURE_LOST;

		mpCaptureNode->HandleEvent( event );
		mpCaptureNode = 0;
	}
}

/// 주의사항 : 
void cUIManager::ReleaseHoverNode( cUINode* n )
{
	assert( n && "null ui node" );

	if( mpHoveredNode == 0 )
		return;

	if( mpHoveredNode->GetParent() == n || 
		mpHoveredNode == n )
	{
		cUIEvent event;
		event.mType = eUIEVENT_MOUSE_LEFT;

		mpHoveredNode->HandleEvent( event );
		mpHoveredNode = 0;
	}
}

/// 주의사항 : 
void cUIManager::SetFocusedNode( cUINode* n )
{
	assert( n && "null ui node" );

	if( mpFocusedNode != n )
	{
		if( mpFocusedNode )
		{
			ReleaseFocusedNode( mpFocusedNode );
		}
		mpFocusedNode = n;

		/// 키보드 이동 관련 처리
		if( HERO && STAGEMAN->GetCurrentStage() == eStage_Game )
			HERO->UpdateKeyState();
	}
}

/// 주의사항 : 
void cUIManager::ReleaseFocusedNode( cUINode* n )
{
	assert( n && "null ui node" );

	if( mpFocusedNode == n )
	{
		cUIEvent event;
		event.mType = eUIEVENT_FOCUS_LOST;

		mpFocusedNode->HandleEvent( event );
		mpFocusedNode = 0;
	}
}

void cUIManager::ClearFocusedNode()
{
	if( mpFocusedNode )
	{
		cUIEvent event;
		event.mType = eUIEVENT_FOCUS_LOST;

		mpFocusedNode->HandleEvent( event );
		mpFocusedNode = 0;
	}
}

/// 주의사항 : 
void cUIManager::ReleaseEventNode()
{
	if( mpCaptureNode )
	{
		cUIEvent event;
		event.mType = eUIEVENT_CAPTURE_LOST;
		mpCaptureNode->HandleEvent( event );
	}
	
	if( mpHoveredNode )
	{
		cUIEvent event;
		event.mType = eUIEVENT_MOUSE_LEFT;
		mpHoveredNode->HandleEvent( event );
	}

	mpHoveredNode = 0;
	mpCaptureNode = 0;
}

void cUIManager::ShowItemTip( eTip_Kind tipkind, const cUIPos& pos, unsigned long itemIndex, unsigned short slotIndex, bool forcedShowEquipTip )
{
	HideTip();

	/// data setting
	sInventory itemInfo;
	::memset( &itemInfo, 0, sizeof(itemInfo) );

	itemInfo.ItemIndex = itemIndex;
	itemInfo.number = slotIndex;

	sItemBill* bill = 0;
	if( slotIndex >= MIN_INVENTORY && slotIndex <= MAX_INVENTORY )
	{
		cItem* item = ITEMMAN->GetItem( slotIndex );
		if( item )
		{
			if( item->GetDefine() != 0)
			{
				itemInfo.enhanced  = item->GetEnhanceStep();
				itemInfo.cardSlot1 = item->GetCardIndex1();
				itemInfo.cardSlot2 = item->GetCardIndex2();
				itemInfo.cardSlot3 = item->GetCardIndex3();
				itemInfo.cardSlot4 = item->GetCardIndex4();
				itemInfo.cardSlot5 = item->GetCardIndex5();
				itemInfo.seal	   = item->IsSeal();
				itemInfo.cash	   = item->IsCash();
				bill			   = item->GetBill();
			}
			else
			{
				assert(0);
			}
		}
		else
			assert(0);
	}

	/// show tip
	cItemTipWindow* pTip = GAMEUI->GetItemTip();
	if( pTip && pTip->SetItemInfo( tipkind, pos, itemInfo, bill, true ) )
	{
		if( SetEquipItemTip( pos, itemInfo, forcedShowEquipTip ) == true )
		{
			cItemTipWindow* tip = GAMEUI->GetEquipItemTip();
			if( tip )
			{
				tip->Show(false);

				/// 팁과 장착팁의 위치 보정
				cUIRect rect1 = pTip->GetAbsoluteRect();
				cUIRect rect2 = tip->GetAbsoluteRect();

				unsigned int x = rect1.mLeft;
				unsigned int y = rect1.mTop;
				unsigned int w1 = rect1.GetWidth();
				unsigned int w2 = rect2.GetWidth();
				unsigned int h = (rect1.GetHeight() > rect2.GetHeight())? rect1.GetHeight():rect2.GetHeight();

				if( x + w1 + w2 + 1 > RENDERSYS->GetScreenWidth() )
				{	
					x -= ((x + w1 + w2 + 1) - RENDERSYS->GetScreenWidth());
				}

				if( y + h > RENDERSYS->GetScreenHeight() )
				{
					y -= ((y + h) - RENDERSYS->GetScreenHeight());
				}


				pTip->SetTipPos( x, y );
				tip->SetTipPos( x+w1+1, y );
			}
		}

		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowItemTip( eTip_Kind tipkind, const cUIPos& pos, unsigned long itemIndex, unsigned char enhance )
{
	HideTip();

	/// data setting
	sInventory itemInfo;
	::memset( &itemInfo, 0, sizeof(itemInfo) );

	itemInfo.ItemIndex = itemIndex;
	itemInfo.number = (unsigned short)-1;

	cItemTipWindow* pTip = GAMEUI->GetItemTip();
	if( pTip && pTip->SetItemInfo( tipkind, pos, itemInfo, 0, false ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowOtherItemTip( eTip_Kind tipkind, const cUIPos& pos, sInventory& inven )
{
	HideTip();

	cItemTipWindow* pTip = GAMEUI->GetItemTip();
	if( pTip && pTip->SetItemInfo( tipkind, pos, inven, 0, false ) )
	{
		if( SetEquipItemTip( pos, inven, true ) == true )
		{
			cItemTipWindow* tip = GAMEUI->GetEquipItemTip();
			if( tip )
			{
				tip->Show(false);

				/// 팁과 장착팁의 위치 보정
				cUIRect rect1 = pTip->GetAbsoluteRect();
				cUIRect rect2 = tip->GetAbsoluteRect();

				unsigned int x = rect1.mLeft;
				unsigned int y = rect1.mTop;
				unsigned int w1 = rect1.GetWidth();
				unsigned int w2 = rect2.GetWidth();
				unsigned int h = (rect1.GetHeight() > rect2.GetHeight())? rect1.GetHeight():rect2.GetHeight();

				if( x + w1 + w2 + 1 > RENDERSYS->GetScreenWidth() )
				{	
					x -= ((x + w1 + w2 + 1) - RENDERSYS->GetScreenWidth());
				}

				if( y + h > RENDERSYS->GetScreenHeight() )
				{
					y -= ((y + h) - RENDERSYS->GetScreenHeight());
				}


				pTip->SetTipPos( x, y );
				tip->SetTipPos( x+w1+1, y );
			}
		}

		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowSkillTip( eTip_Kind tipkind, cSkillTipWindow* pTip, const cUIPos& pos, 
							   unsigned long skillIdx, unsigned char stepIdx, unsigned long jobIdx )
{	
	HideTip();

	if( pTip && pTip->SetSkill( tipkind, pos, skillIdx, stepIdx, jobIdx ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowCommunityTip( cSimpleTipWindow* pTip, const cUIPos& pos, cStringT str )
{
	HideTip();

	if( pTip && pTip->SetSimpleTip( pos, str ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowQuestTip( cQuestTipWindow* pTip, const cUIPos& pos )
{
	HideTip();

	if( pTip && pTip->ShowQuestInfo( pos ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowUITip( const cUIPos& pos, unsigned long tipIdx )
{
	HideTip();

	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip && pTip->SetSimpleTip( pos, GetUIText( tipIdx ) ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowBuffTip( cBuffToolTipWindow* pTip, const cUIPos& pos, unsigned long uniqueIdx, unsigned long influenceIdx, bool showTime )
{
	HideTip();

	if( pTip && pTip->SetBuff( pos, uniqueIdx, influenceIdx, showTime ) )
	{
		mTipWindow = pTip;
		ShowTip();
	}
}

void cUIManager::ShowChatItemTip( const cUIPos& pos, sInventory& inven )
{
	if( mChatItemTipWindow )
	{
		mChatItemTipWindow->Hide();

		if( mChatItemTipWindow->SetItemInfo( eTIP_ITEM, pos, inven, 0, false ) )
			mChatItemTipWindow->Show();
	}
}

void cUIManager::ShowTip()
{
	if( mTipWindow == 0 )
		return;

	bool onsound = false;
	mTipWindow->Show( onsound );
	mpRootNode->mTipWindow = mTipWindow;
}

void cUIManager::HideTip()
{
	if( mTipWindow )
		mTipWindow->Hide();

	mTipWindow = 0;

	cTipWindow* win = GAMEUI->GetEquipItemTip();
	if( win )
		win->Hide(false);
}

void cUIManager::ShowDrag( cUIWindow* pcaller, eDragFrom dragFrom, cBaseIcon* icon, const cUIPos& pos, bool shift )
{
	if( mpDragWindow )
		mpDragWindow->Show( pcaller, dragFrom, icon, icon->GetIconType(), pos, shift );
}

void cUIManager::ShowUnionDrag( cUIWindow* pcaller, cUnionIcon* icon, const cUIPos& pos )
{
	if( mpUnionDragWindow )
		mpUnionDragWindow->Show( pcaller, icon, pos );
}

void cUIManager::HideDrag( cUIWindow* pcaller )
{
	if( mpDragWindow )
		mpDragWindow->Hide( pcaller );

	if( mpUnionDragWindow )
		mpUnionDragWindow->Hide( pcaller );
}

bool cUIManager::IsVisibleDrag()
{
	return ( mpDragWindow ) ? mpDragWindow->IsVisible() : false;
}

bool cUIManager::IsVisibleUnionDrag()
{
	return ( mpUnionDragWindow ) ? mpUnionDragWindow->IsVisible() : false;
}

void cUIManager::AddCloseByNpcHide( cUIWindow* n )
{
	if( n )
		mCloseByNpcHideArr.PushBack( n );
}

void cUIManager::CloseByNpcHide()
{
	for( unsigned int i = 0; i < mCloseByNpcHideArr.GetSize(); ++i )
	{
		cUIWindow* w = (cUIWindow*)mCloseByNpcHideArr[i];
		if( w && w->IsVisible() )
			w->Hide();
	}

	mCloseByNpcHideArr.Clear();
}

/// 복구해야할 윈도우 세팅
void cUIManager::SetRestoreShow( cUIWindow* n )
{
	if( n && mpRootNode )
		mRestore.PushBack( n );
}

/// 주의사항 : npc 창 열려 있을때 생성되었던 윈도우들을
///			   npc 창이 닫히고 나면 보여주기 위함.
void cUIManager::RestoreShow()
{
	/// 복구용 윈도우들 복구 
	if( mRestore.GetSize() > 0 )
	{
		for( unsigned int i = 0; i < mRestore.GetSize(); ++i )
		{
			cUIWindow* n = (cUIWindow*)mRestore[i];
			if( n )
				n->Show();
		}

		mRestore.Clear();
	}

	unsigned long partyIndex = HERO->GetPartyIndex();
	unsigned long partyUnonIndex = HERO->GetPartyUnionIndex();
	if( partyIndex > 0 )
	{
		PARTYMAN->ShowUI();
	}
	else if( partyUnonIndex > 0 )
	{
		PARTYUNIONMAN->ShowUI();
	}
	else
	{
		PARTYMAN->HideUI();
		PARTYUNIONMAN->HideUI();
	}
}

cUINode* cUIManager::FindNodeByID( unsigned long id )
{
	return mpRootNode->FindNodeByID( id );
}

/// 주의사항 :
void cUIManager::UpdateScreenSize( unsigned int, unsigned int )
{
	ReleaseEventNode();
	ClearFocusedNode();

	if( mTipWindow )
		HideTip();

	if( mpDragWindow )
		mpDragWindow->Hide();

	if( mpUnionDragWindow)
		mpUnionDragWindow->Hide();

	for( unsigned int i = 0; i < eStage_Max; ++i )
	{
		mpStageRootNode[i]->UpdateSkin();
	}

	// 공지
	if( mNoticePrintText )
		mNoticePrintText->UpdateScreen();

	// 강화
	if( mEnhancePrintText )
		mEnhancePrintText->UpdateScreen();

	// 더미경고
	if( mDummyNoticeText )
		mDummyNoticeText->UpdateScreen();

	unsigned int screenWidth = RENDERSYS->GetScreenWidth();
	unsigned int screenHeight = RENDERSYS->GetScreenHeight();

	// 관리 이미지들
	cImageHashMap::cIterator i = mImageHashMap.Begin();
	cImageHashMap::cIterator end = mImageHashMap.End();
	for( ; i != end; ++i )
	{
		sPlaneImage* p = (sPlaneImage*)(i->mSecond);
		if( p && p->mpImage )
		{
			int x = 0;
			int y = 0;
			unsigned int w = p->mpImage->GetScreenW();
			unsigned int h = p->mpImage->GetScreenH();

			// 해상도 변경에 따른 좌표 갱신
			if( p->mLeftTop == true )
			{
				x = ( screenWidth * p->mOriginPos.mX ) / DEFAULT_SCREENWIDTH;
				y = ( screenHeight * p->mOriginPos.mY ) / DEFAULT_SCREENHEIGHT;
			}
			else
			{
				// right bottom 기준임
				x = ( screenWidth - p->mOriginPos.mX );
				y = ( screenHeight - p->mOriginPos.mY );
			}

			// 정렬기준
			if( p->mAlign == 1 )
			{
				// width 
				x = (int)(( screenWidth - w ) * 0.5f);
			}
			else if( p->mAlign == 2 )
			{
				// height 
				y = (int)(( screenHeight - h ) * 0.5f);
			}
			else if( p->mAlign == 3 )
			{
				// width & height
				x = (int)(( screenWidth - w ) * 0.5f);
				y = (int)(( screenHeight - h ) * 0.5f);
			}

			p->mpImage->SetScreenXY( x, y );
		}
	}

	if( mpModeImage )
	{
		unsigned int x = (unsigned int)((RENDERSYS->GetScreenWidth() - MODEIMAGE_W) * 0.5);
		mpModeImage->SetScreenXY( x, MODEIMAGE_Y );
	}
}

bool cUIManager::AddPage( unsigned long pageIdx, cPage* page )
{
	if( !page )
		return false;

	if( mPageMap.Insert( pageIdx, page ) == false )
	{
		assert( 0 && "failed to insert page" );
		return false;
	}

	return true;
}

cPage* cUIManager::GetPage( unsigned long pageIdx )
{
	return (cPage*)mPageMap.GetAt( pageIdx );
}

sSceneData* cUIManager::GetSceneData( unsigned long sceneIdx )
{
	return (sSceneData*)mSceneMap.GetAt( sceneIdx );
}

/// 주의사항 : 교차된 컨테이너 업데이트 
cUIContainer* cUIManager::UpdateNearContainer( cUIContainer* con, bool axisX ) const
{	
	return mpRootNode->GetNearRootChild( con, axisX );
}

// node의 상위부모들중 con 이 존재하는지 검사
bool cUIManager::IsIncludeParent( cUINode* node, cUIContainer* con )
{
	if( !node || !con )
		return false;

	if( node == con )
		return true;

	if( node->GetParent() == GetRootNode() )
		return false;

	if( node->GetParent() == con )
		return true;

	return IsIncludeParent( node->GetParent(), con );
}

LPCTSTR cUIManager::GetUIText( unsigned int textId )
{
	assert( mpSkinText );
	return mpSkinText->GetText( textId ).Cstr();
}

bool cUIManager::GetUIText( cStringT* str, unsigned int idx )
{
	assert( mpSkinText );
	return mpSkinText->GetText( str, idx );
}

/// 주의사항 : 유저로부터 받은 이벤트 처리
bool cUIManager::PostEventFromUser( const cUIEvent& event )
{
	// 모달 메세지박스가 하나라도 있으면 처리
	if( mpTopMsgBox && mModalList.IsEmpty() == false )
	{
		if( MsgBoxEventProcess( event ) == true )
			return true;
	}


	switch( event.mType )
	{
	case eUIEVENT_LBUTTON_DOWN:
	case eUIEVENT_RBUTTON_DOWN:
		if( mpFocusedNode )
		{
			UpdateHoveredNode( mpRootNode, event.mPos );

			if( mpHoveredNode != mpFocusedNode )
			{
				/// 에디트박스같이, 포커스가 있는 노드가있으면
				/// 마우스가 눌리면 무조건 포커스를 없앤다 
				cUIEvent e;
				e.mType = eUIEVENT_FOCUS_LOST;
				mpFocusedNode->HandleEvent( e );

				mpFocusedNode = 0;
				mpCaptureNode = 0;
			}
		}
		
		if( mTipWindow && mTipWindow->IsVisible() )
			HideTip();

		/// 다른 영역 클릭 검사
		EventOtherClick( event.mPos );

	case eUIEVENT_LBUTTON_UP:
	case eUIEVENT_RBUTTON_UP:
	case eUIEVENT_MOUSE_MOVE:
	case eUIEVENT_MOUSE_WHEEL:
	case eUIEVENT_LBUTTON_DBLCLK:
		/// 캡쳐노드가 존재하고, 마우스를 캡쳐하는 노드이고, 
		if( mpCaptureNode && mpCaptureNode->HandleEvent( event ) )
		{
			return true;
		}

		/// 캡쳐노드가 없고, 오버노드이면 그 노드의 이벤트를 실행
		if( mpCaptureNode == 0 )
		{
			UpdateHoveredNode( mpRootNode, event.mPos );

			if( mpHoveredNode )
			{
				bool used = mpHoveredNode->HandleEvent( event );

				if( event.mType == eUIEVENT_MOUSE_MOVE )
					/// 카메라 앵글 이동 가능하도록 한다.
					return false;
				else
				{
 					if( mpHoveredNode && mpHoveredNode->IsSetCursorDefault() == true )
 						return false;

					return used;
				}
			}
		}
		break;
	case eUIEVENT_KEY_DOWN:
		if( mpFocusedNode == 0 && mpTopMsgBox == 0 )
			return false;

		if( mpTopMsgBox )
		{
			mpTopMsgBox->HandleEvent( event );
		}
	case eUIEVENT_KEY_UP:
	case eUIEVENT_IME_NOTIFY:
	case eUIEVENT_IME_STARTCOMPOSITION:
	case eUIEVENT_IME_COMPOSITION:
	case eUIEVENT_IME_ENDCOMPOSITION:
	case eUIEVENT_CHAR:
		if( mpFocusedNode )
		{
			mpFocusedNode->HandleEvent( event );
		}
		return true;
	}

	mMousePos = event.mPos;
	return false;
}

bool cUIManager::MsgBoxEventProcess( const cUIEvent& event )
{
	switch( event.mType )
	{
	case eUIEVENT_MOUSE_MOVE:
		{
			/// 캡쳐노드가 존재하고, 마우스를 캡쳐하는 노드이고, 
			if( mpCaptureNode && mpCaptureNode->HandleEvent( event ) )
			{
				return true;
			}

			/// 캡쳐노드가 없고, 오버노드이면 그 노드의 이벤트를 실행
			if( mpCaptureNode == 0 )
			{
				UpdateHoveredNode( mpRootNode, event.mPos );

				if( mpHoveredNode )
				{
					if( mpHoveredNode == mpTopMsgBox )
					{
						mpTopMsgBox->HandleEvent( event );
						return true;
					}
					else
					{
						bool used = mpHoveredNode->HandleEvent( event );

						if( event.mType == eUIEVENT_MOUSE_MOVE )
							/// 카메라 앵글 이동 가능하도록 한다.
							return false;
						else
							return used;
					}
				}
			}
		}
		break;
	case eUIEVENT_LBUTTON_DOWN:
		{
			if( mpFocusedNode )
			{
				UpdateHoveredNode( mpRootNode, event.mPos );
				if( mpHoveredNode != mpFocusedNode )
				{
					/// 에디트박스같이, 포커스가 있는 노드가있으면
					/// 마우스가 눌리면 무조건 포커스를 없앤다 
					cUIEvent e;
					e.mType = eUIEVENT_FOCUS_LOST;
					mpFocusedNode->HandleEvent( e );

					mpFocusedNode = 0;
					mpCaptureNode = 0;

					mpTopMsgBox->HandleEvent( event );
					return true;
				}
			}
			else
			{
				mpTopMsgBox->HandleEvent( event );
				return true;
			}
		}
		break;
	case eUIEVENT_KEY_DOWN:
		{
			if( mpFocusedNode == 0 )
			{
				mpTopMsgBox->HandleEvent( event );
				return true;
			}
		}
		break;
	case eUIEVENT_KEY_UP:
	case eUIEVENT_IME_NOTIFY:
	case eUIEVENT_IME_STARTCOMPOSITION:
	case eUIEVENT_IME_COMPOSITION:
	case eUIEVENT_IME_ENDCOMPOSITION:
	case eUIEVENT_CHAR:
		break;
	default:
		mpTopMsgBox->HandleEvent( event );
		return true;
	}

	return false;
}

void cUIManager::GotoFrontNode( cUINode* node )
{
	unsigned long frontId = mFrontIdGen.GenerateIndex();
	if( mApplyFrontMap.Insert( frontId, node ) == false )
	{
		assert(0);
		return;
	}
	node->mFrontID = frontId;

	/// capture node가 존재하면 최상위 윈도우가 올라와도
	/// 이벤트가 기존 노드로 넘어가는 문제 해결
	if( mpCaptureNode )
		ReleaseCaptureNode( mpCaptureNode );
}

void cUIManager::RemoveFrontNode( unsigned long frontId )
{
	mApplyFrontMap.Erase(frontId);
}

void cUIManager::ApplyFrontNode( cUINode* node )
{
	cUIContainer* parent = node->GetParent();
	if( parent == 0 )
		return;

	tPointerList<void*>* plist = parent->GetChildList();

	bool bFind = false;
	tPointerList<void*>::cIterator i = plist->Begin();
	tPointerList<void*>::cIterator end = plist->End();
	for( ; i != end; ++i )
	{	
		if( node == (*i) )
		{
			/// 찾았으면 앞으로 보냄 
			plist->Erase( i );
			plist->PushBack( node );

			node->mFrontID = 0;
			bFind = true;
			break;
		}
	}

	if( bFind == true && parent->GetType() != eUINODE_ROOT )
	{
		ApplyFrontNode( parent );
	}
}

void cUIManager::GotoFrontMsgBox()
{
	if( mModalList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModalList.Begin();
		cMsgBoxList::cIterator end = mModalList.End();
		for( ; i != end; ++i )
		{
			cUINode* node = (cUINode*)(*i);
			if( node )
			{
				GotoFrontNode( node );
			}
		}
	}

	if( mModallessList.IsEmpty() == false )
	{
		cMsgBoxList::cIterator i = mModallessList.Begin();
		cMsgBoxList::cIterator end = mModallessList.End();
		for( ; i != end; ++i )
		{
			cUINode* node = (cUINode*)(*i);
			if( node )
			{
				GotoFrontNode( node );
			}
		}
	}
}

bool cUIManager::IsCameraEnableContainUI()
{
	if( mpHoveredNode )
	{
		cUINode* pMinimap = (cUINode*)GAMEUI->GetMinimapWindow();
		cUINode* pCashmap = (cUINode*)GAMEUI->GetCashFreeMoveWindow();

		if( mpHoveredNode == pMinimap || mpHoveredNode == pCashmap )
			return true;

		return false;
	}

	return true;
}

bool cUIManager::SetEquipItemTip( const cUIPos& pos, sInventory& inven, bool forcedShow )
{
	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( inven.ItemIndex );
	if( itemDefine == 0 )
		return false;

	cItemTipWindow* win = GAMEUI->GetEquipItemTip();
	if( win == 0 )
		return false;

	eItemIn itemIn = ITEMMAN->GetItemIn( inven.number );

	/// 팁 아이템이 어디에 있더라도 강제로 장비 아이템을 보여준다.
	if( forcedShow == true )
		itemIn = ITEM_IN_INVEN_PRIVATE;

	switch( itemIn )
	{
	case ITEM_IN_INVEN_PRIVATE:
	case ITEM_IN_WAREHOUSE_PRIVATE:
	case ITEM_IN_NPCSTORE:
	case ITEM_IN_USERSELL:
		{
			/// 장착 툴팁 체크
			unsigned char type = itemDefine->GetType();
			unsigned char subType = itemDefine->GetSubType();

			cItem* pItem = 0;
			unsigned short slotIdx = USHRT_MAX;
			if( type == ITEM_WEAPON )
			{
				if( subType == ITEM_WEAPON_SHIELD )
				{
					if( ITEMMAN->GetActiveWeapon() == ItemActiveFront )						
						slotIdx = INVENTORY_HAND_LEFT2;

					pItem = ITEMMAN->GetHeroLeftWeapon();
				}
				else
				{
					if( ITEMMAN->GetActiveWeapon() == ItemActiveFront )	
						slotIdx = INVENTORY_HAND_RIGHT2;

					pItem = ITEMMAN->GetHeroRightWeapon();
				}
			}
			else if( type == ITEM_WEAR )
			{
				switch( subType )
				{
				case ITEM_WEAR_HEAD:	slotIdx = INVENTORY_WEAR_HAT; 	break;
				case ITEM_WEAR_UPPER:	slotIdx = INVENTORY_WEAR_BODY1; break;
				case ITEM_WEAR_LOWER:	slotIdx = INVENTORY_WEAR_BODY2; break;
				case ITEM_WEAR_HANDS:	slotIdx = INVENTORY_WEAR_HAND;	break;
				case ITEM_WEAR_FEET:	slotIdx = INVENTORY_WEAR_FOOT;	break;
				default:
					return false;
				}
				pItem = ITEMMAN->GetItem( slotIdx );
			}
			else if( type == ITEM_ACCESSORY )
			{
				switch( subType )
				{
				case ITEM_ACCESSORY_EARRING:	slotIdx = INVENTORY_WEAR_EARRING;	break;
				case ITEM_ACCESSORY_NECKLACE:	slotIdx = INVENTORY_WEAR_NECKLACE;	break;
				case ITEM_ACCESSORY_BROOCH:		slotIdx = INVENTORY_WEAR_BROOCH;	break;
				case ITEM_ACCESSORY_BRACELET:	slotIdx = INVENTORY_WEAR_BRACELET;	break;
				case ITEM_ACCESSORY_RING:		slotIdx = INVENTORY_WEAR_RING;		break;
				default:
					return false;
				}
				pItem = ITEMMAN->GetItem( slotIdx );
			}
			else
			{
				return false;
			}

			if( pItem == 0 || pItem->GetDefine() == 0 )
				return false;

			/// 장착 아이템 팁 출력
			sInventory heroInven;
			heroInven.ItemIndex = pItem->GetIndex();
			heroInven.number = slotIdx;
			heroInven.enhanced = pItem->GetEnhanceStep();
			heroInven.cardSlot1 = pItem->GetCardIndex1();
			heroInven.cardSlot2 = pItem->GetCardIndex2();
			heroInven.cardSlot3 = pItem->GetCardIndex3();
			heroInven.cardSlot4 = pItem->GetCardIndex4();
			heroInven.cardSlot5 = pItem->GetCardIndex5();
			heroInven.seal = pItem->IsSeal();
			heroInven.cash = pItem->IsCash();
			
			win->SetItemInfo( eTIP_ITEM, pos, heroInven, pItem->GetBill(), false );
		}
		break;
	default:
		return false;
	}

	return true;
}

void cUIManager::AddNoticeText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height )
{
	if( mNoticePrintText )
		mNoticePrintText->AddText( fontIdx, msg, color, height );
}

void cUIManager::DeleteNoticeText()
{
	if( mNoticePrintText )
		mNoticePrintText->DeleteText();
}

void cUIManager::AddEnhanceText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height )
{
	if( mEnhancePrintText )
		mEnhancePrintText->AddText( fontIdx, msg, color, height );
}

void cUIManager::DeleteEnhanceText()
{
	if( mEnhancePrintText )
		mEnhancePrintText->DeleteText();
}

void cUIManager::AddDummyNoticeText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height )
{
	if( mEnhancePrintText )
		mEnhancePrintText->AddText( fontIdx, msg, color, height, false, NOTICEMSG_DUMMYACTIVE_TIME );
}

void cUIManager::AddManagedSortUI( cUINode* node )
{
	mManagedSortUIList.PushFront( node );
}

/// 텍스트 줄여서 리턴
bool cUIManager::ModifyText( LPCTSTR text, TCHAR* out, int outSize, int maxWidth )
{
	/// 길이 조절
	int len = ::_tcslen( text );
	int cutlen = 0; 
	unsigned int cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, text, len );

	while( (int)cutWidth > maxWidth ) 
	{
		/// 계속 검사할필요 없음.
		if( cutlen > len )
			break;

		cutlen++;
		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, text, len - cutlen );
	}

	if( cutlen > 0 )
	{
		if( len - cutlen > 1 )
		{
			::_tcsncpy_s( out, outSize, &text[0], len - cutlen - 1 );
			::_tcsncat_s( out, outSize, _T("…"), 1 );
			return true;
		}
	}
		
	return false;
}

void cUIManager::SetMonModeImage( cPlaneObject* p )
{
	mpModeImage = p;
}

void cUIManager::ClearMonModeImage( cPlaneObject* p )
{
	if( mpModeImage == p )
		mpModeImage = 0;
}

// 삽입
unsigned long cUIManager::AddImage( cPlaneObject* pImage, bool show, int originX, int originY, unsigned char align, bool lefttop )
{
	if( !pImage )
	{
		assert(0);
		return ULONG_MAX;
	}

	unsigned long key = mImageIndexGenerator.GenerateIndex();

	sPlaneImage* p = new sPlaneImage;
	p->mpImage = pImage;
	p->mShow = show;
	p->mOriginPos.mX = originX;
	p->mOriginPos.mY = originY;
	p->mLeftTop = lefttop;
	p->mAlign = align;

	if( mImageHashMap.Insert( key, p ) == false )
	{
		assert(0);
		return ULONG_MAX;
	}
	return key;
}

// 삭제
void cUIManager::DeleteImage( unsigned long id )
{
	if( id == 0 )
		return;

	cImageHashMap::cIterator i = mImageHashMap.Find( id );
	if( i != mImageHashMap.End() )
	{
		sPlaneImage* p = (sPlaneImage*)(i->mSecond);
		if( p )
			SAFE_DELETE( p->mpImage );
		SAFE_DELETE( p );

		mImageHashMap.Erase( id );
		mImageIndexGenerator.RestoreIndex( id );
	}
	else
		assert(0);
}

void cUIManager::UpdateImage( unsigned long id, bool show )
{
	if( id == 0 )
		return;

	cImageHashMap::cIterator i = mImageHashMap.Find( id );
	if( i != mImageHashMap.End() )
	{
		sPlaneImage* p = (sPlaneImage*)(i->mSecond);
		if( p )
			p->mShow = show;
	}
	else
		assert(0);
}

cPlaneObject* cUIManager::GetImage( unsigned long id )
{
	if( id == 0 )
		return 0;

	cImageHashMap::cIterator i = mImageHashMap.Find( id );
	if( i != mImageHashMap.End() )
	{
		sPlaneImage* p = (sPlaneImage*)(i->mSecond);
		return ( p ) ? p->mpImage : 0;
	}
	return 0;
}

//
cUIWindow* cUIManager::AddNpcTalkButton( unsigned long npcIndex )
{
	cTalkButtonWindow* p = new cTalkButtonWindow;
	if( !p )
	{
		assert(0);
		SAFE_DELETE( p );
		return 0;
	}

	//
	if( p->Create( "TalkButtonWindow" ) == false )
	{
		assert(0);
		SAFE_DELETE( p );
		return 0;
	}

	p->SetNpcIndex( npcIndex );
	p->Hide( false );

	if( mTalkButtonMap.Insert( npcIndex, p ) == false )
	{
		assert(0);
		SAFE_DELETE( p );
		return 0;
	}
	
	return (cUIWindow*)p;
}

void cUIManager::DeleteNpcTalkButton( unsigned long npcIndex )
{
	cTalkButtonWindow* p = (cTalkButtonWindow*)mTalkButtonMap.GetAt( npcIndex );
	if( p )
	{
		mTalkButtonMap.Erase( npcIndex );
		
		if( mpRootNode )
		{
			mpRootNode->RemoveChildComplete( p );
		}
		else
			assert(0);
	}
}
