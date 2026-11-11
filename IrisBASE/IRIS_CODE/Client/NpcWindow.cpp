#include "stdafx.h"
#include "NpcWindow.h"

#include "UIImage.h"
#include "UINpcTelling.h"
#include "UIFuncText.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "QuestManager.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "Hero.h"
#include "Label.h"
#include "TextBox.h"
#include "ListBox.h"
#include "NpcRewardWindow.h"
#include "Page.h"
#include "GuildManager.h"
#include "UIRootNode.h"
#include "TarotManager.h"
#include "ResourceManager.h"
#include "TriggerManager.h"
#include "FontAgent.h"

cNpcWindow::cNpcWindow()
: mNpcClassIndex( 0 )
, mTelling( 0 )
, mNameLabel(0)
, mpTextBox(0)
, mpQuestList(0)
, mNpcIndex(0)
, mHideBack( true )
, mIsUseQuest( false )
, mSceneImage(0)
, mSceneIndex(0)
, mSceneSkin(0)
, mpClose(0)
, mQuestIndex(0)
, mStatus( eNPCQUEST_NONE )
, mNextPageIdx(0)
, mShowScene( false )
{
	SetExceptEvent( true );
	SetExceptSnap( true );

	memset( mpFuncButton, 0, sizeof(mpFuncButton) );
}

cNpcWindow::~cNpcWindow()
{
	Clear();

	SAFE_DELETE( mSceneImage );
}

/// 퀘스트 정보 삭제
void cNpcWindow::Clear()
{
	if( mpQuestList )
		mpQuestList->Clear();

	if( mpTextBox )
		mpTextBox->Clear();

	for( unsigned int i = 0; i < mDelArr.GetSize(); ++i )
	{
		sQuestStatus* data = (sQuestStatus*)mDelArr[i];
		SAFE_DELETE( data );
	}
	mDelArr.Clear();

	mShowScene = false;
	mIsUseQuest = false;
	mQuestIndex = 0;
	mNextPageIdx = 0;
	mSceneIndex = 0;
	mStatus = eNPCQUEST_NONE;
}

/// 맵 마다 텍스쳐가 다르므로 삭제후 재 생성
void cNpcWindow::Open()
{
	if( mSceneSkin && mSceneImage == 0 )
	{
		mSceneImage = new cUIImage;
	}
}

void cNpcWindow::Close()
{
	cUIWindow::Close();

	SAFE_DELETE( mSceneImage );

	Clear();

	if( mNameLabel )
		mNameLabel->Clear();

	cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
	if( reward )
		reward->Hide();

	if( mpQuestList )
	{
		mpQuestList->SetEnabled( true );
		mpQuestList->Hide();
	}

	if( mHideBack )
	{
		HERO->CloseNpcTalk( false );
	}

	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->Hide();
	}

	mNpcClassIndex = 0;
	mNpcIndex = 0;
}

void cNpcWindow::Show( unsigned int npcClassIdx, unsigned long npcIndex, LPCTSTR npcName )
{
	mNpcClassIndex = npcClassIdx;
	mNpcIndex = npcIndex;
	mHideBack = true;
	mIsUseQuest = false;

	/// 이름을 설정
	if( mNameLabel )
		mNameLabel->SetText( npcName );

	/// 타겟창 닫기
	cUIWindow* wnd = (cUIWindow*)GAMEUI->GetTargetWindow();
	if ( wnd )
		wnd->Hide( false );

	/// npc 윈도우를 제외한 다른 윈도우 닫기
	GetRoot()->HideAllChild( this );
	UIMAN->AddCloseByNpcHide( this );

	//
	HERO->OpenNpcTalk( npcIndex );

	/// 퀘스트 리스트 세팅
	QUESTMAN->SetNpcQuestList( this, mNpcIndex, mNpcClassIndex );

	/// 자기자신 보이기
	cUIWindow::Show();

	/// 메세지 박스들만 앞으로 보내기
	UIMAN->GotoFrontMsgBox();
}

void cNpcWindow::ShowDefaultPage()
{
	Clear();

	/// 위치설정
	int x = 0;
	if( GetScreenWidth() - GetAbsoluteRect().GetWidth() > 0 )
	{
		x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	}

	SetRelativePos( cUIPos( x, GetRelativeRect().mTop ) );

	/// 기본 대사를 표시
	cUIFuncTextParam textParam;
	textParam.mName = HERO->GetName();

	if( mpTextBox )
		mpTextBox->Clear();

	mTelling = UIMAN->GetNpcTelling( mNpcClassIndex );
	if( mTelling )
	{
		sPageText* pageText = mTelling->GetDefaultTextRandomly();
		if( pageText )
		{
			for( unsigned int i = 0; i < pageText->numLine; ++i )
			{
				if( mpTextBox )
					mpTextBox->AddFucnText( UIMAN->GetNpcTextNode( pageText->textIndex + i ), &textParam );
			}
		}

		/// 장면있으면 이미지 출력
		mSceneIndex = mTelling->GetDefaultScene();
		UpdateScene();

		// 기능 버튼 보기
		ShowFuncButton();
	}

	cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
	if( reward )
		reward->Hide();

	if( mpQuestList )
		mpQuestList->Hide();
}

void cNpcWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 위치설정
	int x = 0;
	if( GetScreenWidth() - GetAbsoluteRect().GetWidth() > 0 )
	{
		x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	}
	SetRelativePos( cUIPos( x, GetRelativeRect().mTop ) );

	/// 기본 대사를 표시
	cUIFuncTextParam textParam;
	textParam.mName = HERO->GetName();

	if( mpTextBox )
		mpTextBox->Clear();

	mTelling = UIMAN->GetNpcTelling( mNpcClassIndex );
	if( mTelling )
	{
		sPageText* pageText = mTelling->GetDefaultTextRandomly();
		if( pageText )
		{
			for( unsigned int i = 0; i < pageText->numLine; ++i )
			{
				if( mpTextBox )
					mpTextBox->AddFucnText( UIMAN->GetNpcTextNode( pageText->textIndex + i ), &textParam );
			}
		}

		/// 장면있으면 이미지 출력
		mSceneIndex = mTelling->GetDefaultScene();
		UpdateScene();

		// 기능 버튼 보기
		ShowFuncButton();
	}
	else
	{
		Hide();
	}
}

void cNpcWindow::ShowFuncButton()
{
	// 버튼 닫기
	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->Hide();
	}

	cUIRect rect;
	cUIPos pos(0, 0);
	int column = 0;	// 닫기버튼 포함
	int row = 0;


	for( int i = 0; i < mTelling->GetFuncCount(); ++i )
	{
		eNpcFunc func = mTelling->GetFunc(i);

		switch( func )
		{
		case NPC_FUNC_STORE:
		case NPC_FUNC_WAREHOUSE:
		case NPC_FUNC_ITEMDISJOINT:
		case NPC_FUNC_GUILD:
		case NPC_FUNC_FORTUNETAROT:
		case NPC_FUNC_TAROT:
		case NPC_FUNC_DEAL:
		case NPC_FUNC_THEME:
		case NPC_FUNC_SKILLPREVIEW:
		case NPC_FUNC_EVENT:
			{
				if( mpFuncButton[func] == NULL )
				{
					assert(0);
					return;
				}

				// 첫번째 버튼 위치를 기준으로 셋팅
				if( i == 0) 
				{
					rect = mpFuncButton[func]->GetRelativeRect();
				}				

				// 기준을 위치로 버튼 위치 셋팅
				if( i >= 1 )
				{
					pos.mX = rect.mLeft - ( column * (rect.GetWidth() + FUNCBUTTON_GAPW) );
					pos.mY = rect.mTop - ( row * (rect.GetHeight() + FUNCBUTTON_GAPH) );

					mpFuncButton[func]->SetRelativePos( pos );
				}

				mpFuncButton[func]->Show();

				column++;
				if( column > 2 )
				{
					column = 0;
					row = 1;
				}
			}
			break;
		default:
			assert(0);
			break;
		}
	}
}

void cNpcWindow::HideESC( bool onsound )
{
	if( IsNetLock() == false )
		cUIWindow::HideESC( onsound );
}

void cNpcWindow::OnHide()
{
	cUIWindow::OnHide();

	/// ESC 버튼으로 닫거나, 닫기버튼을 누르면 배경 없앰.
	/// 상점관련 윈도우가 열린거라면 배경 놔둠.
	if( mHideBack )
	{
		HERO->CloseNpcTalk();
	}

	mNpcClassIndex = 0;
	mNpcIndex = 0;

	/// 퀘스트 정보 클리어 
	Clear();

	cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
	if( reward )
		reward->Hide();

	if( mNameLabel )
		mNameLabel->Clear();

	if( mpQuestList )
	{
		mpQuestList->SetEnabled( true );
		mpQuestList->Hide();
	}

	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->Hide();
	}
}


bool cNpcWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	///
	mNameLabel = (cLabel*)GetChild( eUIID_NPCTELL_NAME );
	mpTextBox = (cTextBox*)GetChild( eUIID_NPCTELL_TEXTBOX );

	mpFuncButton[NPC_FUNC_STORE]		= GetChild( eUIID_NPCTELL_STORE );
	mpFuncButton[NPC_FUNC_WAREHOUSE]	= GetChild( eUIID_NPCTELL_WAREHOUSE );
	mpFuncButton[NPC_FUNC_ITEMDISJOINT] = GetChild( eUIID_NPCTELL_DISJOINT );
	mpFuncButton[NPC_FUNC_GUILD]		= GetChild( eUIID_NPCTELL_GUILD );
	mpFuncButton[NPC_FUNC_FORTUNETAROT]	= GetChild( eUIID_NPCTELL_FORTUNETAROT );
	mpFuncButton[NPC_FUNC_TAROT]		= GetChild( eUIID_NPCTELL_TAROT );
	mpFuncButton[NPC_FUNC_DEAL]			= GetChild( eUIID_NPCTELL_DEAL );
	mpFuncButton[NPC_FUNC_THEME]		= GetChild( eUIID_NPCTELL_THEME );
	mpFuncButton[NPC_FUNC_SKILLPREVIEW] = GetChild( eUIID_NPCTELL_SKILLPREVIEW );
	mpFuncButton[NPC_FUNC_EVENT]		= GetChild( eUIID_NPCTELL_EVENT );

	mpClose = GetChild( eUIID_NPCTELL_CLOSE );

	mpQuestList = (cListBox*) GetChild( eUIID_NPCTELL_QUESTLISTBOX );

	if( mNameLabel )
	{
		mNameLabel->SetFontType( cFontAgent::eFont_Npc );
	}

	if( mpTextBox )
	{
		mpTextBox->SetDrawTextBottom( false );
		mpTextBox->SetFontType( cFontAgent::eFont_Npc );
	}

	if( mpQuestList )
	{
		mpQuestList->SetFontType( cFontAgent::eFont_Npc );
		mpQuestList->SetContinueEvent( true );
		mpQuestList->Hide();
	}

	/// 장면이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	/// 이미지 생성
	mSceneSkin = pSkin->GetNodeSkin( "NpcTelling_Scene" );
	if( mSceneSkin )
	{
		mSceneImage = new cUIImage;
	}

	/// npc 이름색
	if( mNameLabel )
		mNameLabel->SetTextColor( 0xFFE9E47C );
	return true;
}

void cNpcWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 위치설정
	int x = 0;
	if( GetScreenWidth() - GetAbsoluteRect().GetWidth() > 0 )
	{
		x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	}
	SetRelativePos( cUIPos( x, GetRelativeRect().mTop ) );

	if( mSceneIndex > 0 && mSceneImage )
	{
		sSceneData* scene = UIMAN->GetSceneData( mSceneIndex );
		if( scene )
		{
			unsigned int x = (GetScreenWidth()-mOldScreenWidth) + GetAbsoluteRect().mLeft + mSceneSkin->mSkinInfo->mX;
			unsigned int y = GetScreenHeight() - scene->mTexHeight;

			unsigned int w = scene->mTexWidth;
			unsigned int h = scene->mTexHeight;
			mSceneImage->SetScreenRect( cUIRect( x, y, x+w, y+h ) );
		}
	}
}

void cNpcWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// NPC 이미지 그린후, 윈도우 그리기
	if( mSceneIndex > 0 && mSceneImage && mShowScene == true )
		mSceneImage->Draw();

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();
	
	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}

	pKeeper->DrawAll();
}

void cNpcWindow::OnCommand( cUINode*, unsigned int id )
{
	if( IsNetLock() == true )
		return;

	mHideBack = false;

	switch( id )
	{
	case eUIID_NPCTELL_CLOSE:
		HERO->SetNpcFunc( NPC_FUNC_MAX );
		mHideBack = true;
		break;
	case eUIID_NPCTELL_STORE:
		{
			HERO->SetNpcFunc( NPC_FUNC_STORE );

			//
			GAMEUI->UpdateInvenWithUI( (cUIWindow*)GAMEUI->GetNpcStore() );
			GAMEUI->ShowNpcStoreWindow( mNpcClassIndex );
			GAMEUI->ShowInventory();

			/// 
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetInventoryWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetNpcStore() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemCountWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemMsgWindow() );
		}
		break;
	case eUIID_NPCTELL_WAREHOUSE:
		{
			HERO->SetNpcFunc( NPC_FUNC_WAREHOUSE );

			GAMEUI->UpdateInvenWithWare();
			GAMEUI->ShowWarehouse();
			GAMEUI->ShowInventory();
			
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetInventoryWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetWareHouseWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetWareHouseBagWindow( 0 ) );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetWareHouseBagWindow( 1 ) );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetWareHouseBagWindow( 2 ) );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetMoneyWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemCountWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemMsgWindow() );
		}
		break;
	case eUIID_NPCTELL_DISJOINT:
		{
			HERO->SetNpcFunc( NPC_FUNC_ITEMDISJOINT );

			GAMEUI->UpdateInvenWithUI( (cUIWindow*)GAMEUI->GetNpcDisjointWindow() );
			GAMEUI->ShowNpcDisjointWindow();
			GAMEUI->ShowInventory();

			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetInventoryWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetNpcDisjointWindow() );
		}
		break;
	case eUIID_NPCTELL_EVENT:
		{

			if( mTelling == NULL )
			{
				mHideBack = true;
				Hide();
				return;
			}

			if( mTelling->GetEventIndex() == 0 )
			{
				mHideBack = true;
				Hide();
				return;
			}

 			HERO->SetNpcFunc( NPC_FUNC_EVENT );
 			
 			MSG_REQ_NPC_EVENT_ITEM_GIVE msg;
 			memset( &msg, 0, sizeof(msg) );
 
 			msg.Category = NM_NPC;
 			msg.Protocol = NM_NPC_EVENT_ITEM_GIVE_REQ;
 			msg.npcIndex = mNpcIndex;
			msg.eventIndex = mTelling->GetEventIndex();
 			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
 
 			ITEMMAN->LockItemTry( ITEM_TRY_NPCEVENT, 0 );
 
 			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 				eMBEVENT_PVP_NPC_REWARD, eSKIN_NONE,
 				GAMERESOURCEMAN->GetGameText( 13804 ),
 				GAMERESOURCEMAN->GetGameText( 301 )
 				);

			///예외처리.
			mHideBack = true;
		}
		break;
	case eUIID_NPCTELL_GUILD:
		{
			HERO->SetNpcFunc( NPC_FUNC_GUILD );

			// 1. 길드 가입검사
			if( HERO->GetGuildIndex() > 0 )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 1202 ), 
					GAMERESOURCEMAN->GetGameText( 1233 )
					);
				
				mHideBack = true;
				break;
			}

			// 2. 레벨 검사
			if( HERO->GetLevel() < GUILDLIMIT_LEVEL )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 1237 ), 
					GAMERESOURCEMAN->GetGameText( 1233 )
					);

				mHideBack = true;
				break;
			}

			// 3. 금액 검사
			if( HERO->GetMoney() < GUILDLIMIT_MONEY )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 1238 ), 
					GAMERESOURCEMAN->GetGameText( 1233 )
					);

				mHideBack = true;
				break;
			}

				// 길드 생성 확인
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_GUILD_CREATE, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 1231 ),
				GAMERESOURCEMAN->GetGameText( 1233 ),
				(void*)mNpcIndex );
			
		}
		break;
	case eUIID_NPCTELL_FORTUNETAROT:
		{
			HERO->SetNpcFunc( NPC_FUNC_FORTUNETAROT );
			GAMEUI->ShowNpcTarotWindow( mTelling->GetTarotArr(), mTelling->GetTarotPrice(), mTelling->GetSpreadArr(), mTelling->GetSpreadCount() );

			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetTarotWindow() );
		}
		break;
	case eUIID_NPCTELL_TAROT:
		{
			bool retvalue = TAROTMAN->TarotTutorialOpen( mNpcIndex );
			if( retvalue == true )
			{
				HERO->SetNpcFunc( NPC_FUNC_TAROT );
				UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetTarotWindow() );
			}
		}
		break;
	case eUIID_NPCTELL_DEAL:
		{
			HERO->SetNpcFunc( NPC_FUNC_DEAL );

			GAMEUI->UpdateInvenWithUI( (cUIWindow*)GAMEUI->GetNpcDealWindow() );
			GAMEUI->ShowNpcDealWindow();
			GAMEUI->ShowInventory();

			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetNpcDealWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemCountWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetItemMsgWindow() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetInventoryWindow() );
			
			/// 인증 받기
			ITEMMAN->TryToNpcDealOpen( mNpcIndex );
		}
		break;
	case eUIID_NPCTELL_THEME:
		{
			HERO->SetNpcFunc( NPC_FUNC_THEME );

			/// 던젼 참여 메시지 보내기.
			MSG_REQ_NPC_THEME_JOIN msg;
			msg.Category = NM_NPC;
			msg.Protocol = NM_NPC_THEME_JOIN_REQ;
			msg.npcIndex = mNpcIndex;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			/// 메시지 박스 띄우기
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_THEME_ENTER_INFO_WAIT, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 9001 ), 
				GAMERESOURCEMAN->GetGameText( 9000 ) );
		}
		break;
	case eUIID_NPCTELL_SKILLPREVIEW:
		if( mTelling )
		{
			HERO->SetNpcFunc( NPC_FUNC_SKILLPREVIEW );

			GAMEUI->ShowSkillPreviewWindow( mTelling->GetJobData1(), mTelling->GetJobData2() );
			UIMAN->AddCloseByNpcHide( (cUIWindow*)GAMEUI->GetSkillPreviewWindow() );
		}
		else
			assert(0);
		break;
	default:
		assert(0);
		return;
	}

	Hide( false );
}

void cNpcWindow::OnListBoxChanged( cUINode* , unsigned int )
{
	if( IsNetLock() == true )
		return;

	/// 퀘스트 사용상태인지 여부
	if( mIsUseQuest == false )
	{
		SelectQuest();

		cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
		if( reward )
			reward->Show();
	}
	else
	{
		unsigned int selectIdx = mpQuestList->GetSelectIndex();
		unsigned int nextPageIdx = (unsigned int)mpQuestList->GetExtraData( selectIdx );

		/// 처음으로 처리
		if( nextPageIdx == 0 )
		{
			ShowDefaultPage();

			//
			QUESTMAN->SetNpcQuestList( this, mNpcIndex, mNpcClassIndex );
		}
		/// 닫기 처리
		else if( nextPageIdx == ULONG_MAX )
		{
			Hide();
		}
		/// 장면 갱신
		else
		{
			cPage* nextPage = mTelling->GetPage( nextPageIdx );
			if( !nextPage )
			{
				assert(0);
				return;
			}

			eAnswerEvent event = nextPage->GetAnswerEvent();
			bool retvalue = false;

			if( mStatus == eNPCQUEST_NEW && event == eEVENT_YES )
			{
				retvalue = QUESTMAN->QuestAddByNpc( mQuestIndex, mNpcIndex );
			}
			else if( mStatus == eNPCQUEST_COMPLETE && event == eEVENT_COMPLETE )
			{
				retvalue = QUESTMAN->QuestComplete( mQuestIndex, mNpcIndex );
			}
			else if( mStatus == eNPCQUEST_COMPLETE && event == eEVENT_REWARD )
			{
				retvalue = QUESTMAN->QuestReward( mQuestIndex, mNpcIndex );
			}
			else if( mStatus == eNPCQUEST_REWARD && event == eEVENT_REWARD )
			{
				retvalue = QUESTMAN->QuestReward( mQuestIndex, mNpcIndex );
			}
			else
			{
				/// 이벤트가 없으면 그냥 다음페이지로 넘긴다
				mNextPageIdx = nextPageIdx;
				UpdateDialog();
			}

			/// 결과에따른 페이지 저장
			if( retvalue == true )
				mNextPageIdx = nextPageIdx;
		}
	}
}

/// 퀘스트 목록 추가
void cNpcWindow::AddQuest( unsigned long questIdx, unsigned int status )
{
	if( !mpQuestList )
		return;

	if( mpQuestList && mpQuestList->IsVisible() == false )
		mpQuestList->Show();

	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	assert(define);
	if( define )
	{
		LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );

		int textIdx = 0;
		switch( status )
		{
		case eNPCQUEST_PLAYING:			textIdx = 462;		break;
		case eNPCQUEST_NEW:				textIdx = 461;		break;
		case eNPCQUEST_COMPLETE:		textIdx = 463;		break;
		case eNPCQUEST_REWARD:			textIdx = 463;		break;
		default:						assert(0);			return;
		}

		cStringT str;
		unsigned long textColor = mDefaultColor;
		str.Format( GAMERESOURCEMAN->GetGameText( textIdx ), define->mLevel, title );
//		if( define->mRepeatType == eQUEST_REPEAT )
//			str.Append( GAMERESOURCEMAN->GetGameText( 472 ) );

		/// 저 레벨 퀘스트 필터링
		short lowLevel = TRIGGERMAN->GetLowLevel( questIdx );
		if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
		{
			str.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
			textColor = COLOR_LOW_LEVEL_QUEST;
		}

		/// 확장정보 저장
		sQuestStatus* exData = new sQuestStatus;
		exData->questIdx = questIdx;
		exData->status = status;
		mDelArr.PushBack( exData );

		mpQuestList->AddRow( (LPCTSTR)str.Cstr(), textColor, exData );
	}
}

void cNpcWindow::SelectQuest()
{
	if( !mpQuestList ) return;

	mIsUseQuest = true;
	
	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->Hide();
	}

	unsigned int selectIdx = mpQuestList->GetSelectIndex();
	sQuestStatus* exData = (sQuestStatus*)mpQuestList->GetExtraData( selectIdx );

	/// 다이얼로그 정보 세팅
	if( mTelling )
	{
		if( SetDialog( exData->questIdx, exData->status ) == true )
		{
			cQuestDefine* define = QUESTMAN->GetQuestDefine( exData->questIdx );
			assert(define);
			if( define )
			{
				if( mNameLabel )
					mNameLabel->SetText( GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );
			}

			mHideBack = true;
		}
	}
}


/// 퀘스트 관련 대사정보 가져와서 화면 구성
bool cNpcWindow::SetDialog( unsigned long quetIdx, unsigned int status )
{
	mQuestIndex = quetIdx;
	mStatus = status;
	mNextPageIdx = 0;

	/// 퀘스트 보상정보 세팅
	cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
	if( reward && reward->SetQuestReward( mQuestIndex ) == false )
	{
		assert(0);
	}

	/// 연결 대사정보 획득
	sQuestLinkPage* linkPage = mTelling->GetQuestLinkPage( quetIdx );
	if( !linkPage )
		return false;

	unsigned long pageIdx = 0;
	switch( mStatus )
	{
	case eNPCQUEST_PLAYING:		  pageIdx = linkPage->playIndex;	 break;
	case eNPCQUEST_NEW:			  pageIdx = linkPage->newIndex;		 break;
	case eNPCQUEST_COMPLETE:	  pageIdx = linkPage->completeIndex; break;
	/// complete 페이지로부터 EVENT_COMPLETE 나오는 부분찾기
	case eNPCQUEST_REWARD:	pageIdx = linkPage->rewardIndex;	break;
	default:				assert(0);	return false;
	}

	/// 화면갱신 페이지
	mNextPageIdx = pageIdx;

	/// 페이지 정보
	cPage* page = mTelling->GetPage( pageIdx );
	if( !page )
	{
		assert(0);
		return false;
	}
		
	eAnswerEvent event = page->GetAnswerEvent();
	bool retvalue = true;
	switch( mStatus )
	{
	case eNPCQUEST_NEW:			/// 신규요청
		if( event == eEVENT_YES )
		{
			retvalue = QUESTMAN->QuestAddByNpc( mQuestIndex, mNpcIndex );
		}
		break;
	case eNPCQUEST_COMPLETE:	/// 진행완료요청
		if( event == eEVENT_COMPLETE )
		{
			retvalue = QUESTMAN->QuestComplete( mQuestIndex, mNpcIndex );
		}
		break;
	case eNPCQUEST_REWARD:		/// 보상요청
		if( event == eEVENT_REWARD )
		{
			retvalue = QUESTMAN->QuestReward( mQuestIndex, mNpcIndex );
		}
		break;
	case eNPCQUEST_PLAYING:
		break;
	default:
		assert(0);
		return false;
	}
	/// 장면갱신
	return UpdateDialog();
}

/// 다음 장면 구성
bool cNpcWindow::UpdateDialog( bool rewardItemSeleted )
{
	if( !mpQuestList ) 
		return false;

	if( !mpTextBox )
		return false;

	/// 초기화
	mpTextBox->Clear();
	mpQuestList->Clear();

	/// 출력용 페이지 가져오기
	cPage* page = mTelling->GetPage( mNextPageIdx );
	if( !page )
		return false;

	/// 퀘스트 보상 선택가능 검사
	cNpcRewardWindow* reward = GAMEUI->GetNpcRewardWindow();
	if( reward )
		reward->UpdateUseReward( mQuestIndex );

	/// 장면 세팅
	mSceneIndex = page->GetSceneIdx();
	UpdateScene();

	/// 기본 대사를 표시
	cUIFuncTextParam textParam;
	textParam.mName = HERO->GetName();

	/// 기본대사 출력
	sPageText* pageText = page->GetPageText();
	if( pageText && mpTextBox )
	{
		for( unsigned int i = 0; i < pageText->numLine; ++i )
		{
			mpTextBox->AddFucnText( UIMAN->GetNpcTextNode( pageText->textIndex + i ), &textParam );
		}
	}

	cQuestDefine* define = QUESTMAN->GetQuestDefine( mQuestIndex );
	eAnswerEvent event = page->GetAnswerEvent();
	
	/// 퀘스트 포상 아이템 선택해야 할 경우 안내문 강제 출력
 	if( ( mStatus == eNPCQUEST_REWARD || mStatus == eNPCQUEST_COMPLETE ) && event == eEVENT_COMPLETE && 
		  rewardItemSeleted == false  && define->mSelectCount != 0 )
 	{
		mpQuestList->AddRow( GAMERESOURCEMAN->GetGameText( 473 ), mDefaultColor );
 		mpQuestList->SetEnabled( false );
 	}
 	else
 	{
		mpQuestList->SetEnabled( true );
		
		/// 대답용 대사 출력
		typedef tArray<void*> cAnswerArr;
		cAnswerArr& ansArr = page->GetAnswerArr();

		for( unsigned int b = 0; b < ansArr.GetSize(); ++b )
		{
			sPageAnswer* ansText = (sPageAnswer*)ansArr[b];
			if( !ansText )
			{
				assert(0);
				return false;
			}

			unsigned long textcolor = mDefaultColor;
			for( unsigned i = 0; i < ansText->numLine; ++i )
			{
					cUIFuncTextNode* node = UIMAN->GetNpcTextNode( ansText->textIndex + i );
					if( !node )
					{
						assert(0);
						continue;
					}

					for( ; node; node = node->GetNext() )
					{
						if( node->mColor )
							textcolor = node->mColor;

						if( node->mpString )
							mpQuestList->AddRow( node->mpString, textcolor, (void*)ansText->nextPageIndex );
					}
			}
		}
	
		/// 대사가 없고 next 이면 다음페이지 넘기기
		if( ansArr.GetSize() == 0 )
		{
			if( page->GetNextPageIdx() > 0 )
			{
				mpQuestList->AddRow( UIMAN->GetUIText( 83 ), mDefaultColor, (void*)page->GetNextPageIdx() );
			}
			else
			{
				/// 대사가 없으면
				/// 신규페이지이면 "닫기"출력
				if( page->GetAnswerEvent() == eEVENT_YES )
				{
					mpQuestList->AddRow( UIMAN->GetUIText( 112 ), mDefaultColor, (void*)(ULONG_MAX) );
				}
				/// 나머지는 "처음으로" 출력
				else
				{
					mpQuestList->AddRow( UIMAN->GetUIText( 84 ), mDefaultColor );
				}
				
			}
		}
	}

	return true;
}

void cNpcWindow::UpdateScene()
{
	if( mSceneIndex == 0 || !mSceneImage )
		return;

	cUISize sceneSize;
	sceneSize.mWidth = 0;
	sceneSize.mHeight = 0;
	mShowScene = false;

	sSceneData* scene = UIMAN->GetSceneData( mSceneIndex );
	if( scene )
	{
		cString path;
		path.Format("./Data/2DData/%s", scene->mTexName.Cstr() );
		NiTexture* tex = RESOURCEMAN->LoadMapTexture( path, false );
		if( tex )
		{
			mSceneImage->SetTexture( tex );

			unsigned int tx = scene->mTexPosX;
			unsigned int ty = scene->mTexPosY;
			unsigned int tw = scene->mTexWidth;
			unsigned int th = scene->mTexHeight;
			unsigned int x = GetAbsoluteRect().mLeft + mSceneSkin->mSkinInfo->mX;
			unsigned int y = GetScreenHeight() - scene->mTexHeight;

			mSceneImage->SetTextureRect( tx, ty, tx + tex->GetWidth(), ty + tex->GetHeight() );
			mSceneImage->SetScreenRect( cUIRect( x, y, x+tw, y+th ) );

			sceneSize.mWidth = tw;
			sceneSize.mHeight = th;

			mShowScene = true;
		}
	}

	// 위치 이동
	int x = 0;
	int y = mAbsoluteRect.mTop;

	int cx = GetScreenWidth() - (mAbsoluteRect.GetWidth() + sceneSize.mWidth);
	if( cx > 0 )
	{
		x = (int)( cx * 0.5f);
	}
	else
		x = 0;

	SetRelativePos( cUIPos(x, y) );

	//
	if( mSceneImage && mShowScene )
	{
		y = GetScreenHeight() - sceneSize.mHeight;

		if( mAbsoluteRect.mRight + sceneSize.mWidth < (int)GetScreenWidth() )
		{
			x = mAbsoluteRect.mRight;
		}
		else
		{
			x = GetScreenWidth() - sceneSize.mWidth;	
		}

		mSceneImage->SetPos( x, y );
	}
}

void cNpcWindow::NetLock( int lockTry )
{
	mNetLock = true;
	mLockTry = lockTry;
}

void cNpcWindow::NetUnLock( int lockTry )
{
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

void cNpcWindow::OnNetLock( int lockTry )
{
	if( mpClose )
		mpClose->NetLock( lockTry );

	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->NetLock( lockTry );
	}
}

void cNpcWindow::OnNetUnLock( int lockTry )
{
	if( mpClose )
		mpClose->NetUnLock( lockTry );

	for( int i = 0; i < NPC_FUNC_MAX; ++i )
	{
		if( mpFuncButton[i] )
			mpFuncButton[i]->NetUnLock( lockTry );
	}
}
