#include "stdafx.h"
#include "NpcTutorialWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Label.h"
#include "TextBox.h"
#include "ListBox.h"
#include "QuestManager.h"
#include "UIImage.h"
#include "UINpcTelling.h"
#include "UIFuncText.h"
#include "GameResourceManager.h"
#include "Page.h"
#include "ResourceManager.h"
#include "FontAgent.h"

cNpcTutorialWindow::cNpcTutorialWindow()
{
	SetExceptEvent( true );
	SetExceptSnap( true );
}

cNpcTutorialWindow::~cNpcTutorialWindow()
{
	Clear();
	SAFE_DELETE( mSceneImage );
}

void cNpcTutorialWindow::Close()
{
	cUIWindow::Close();

	SAFE_DELETE( mSceneImage );

	Clear();

	if( mNameLabel )
		mNameLabel->Clear();

	if( mpQuestList )
		mpQuestList->Hide();

	HERO->CloseNpcTalk( false );
	
	mNpcClassIndex = 0;
	mNpcIndex = 0;
}

void cNpcTutorialWindow::Show( unsigned int npcClassIdx, unsigned long npcIndex, LPCTSTR npcName )
{
	mNpcClassIndex = npcClassIdx;
	mNpcIndex = npcIndex;
	mIsUseQuest = false;

	/// 이름을 설정
	if( mNameLabel )
		mNameLabel->SetText( npcName );

	//
	HERO->OpenNpcTalk( npcIndex, false );

	/// 퀘스트 리스트 세팅
	QUESTMAN->SetNpcQuestList( this, mNpcIndex, mNpcClassIndex );

	/// 자기자신 보이기
	cUIWindow::Show();
}

void cNpcTutorialWindow::OnShow()
{
	cUIWindow::OnShow();

	HERO->SetNpcFunc( NPC_FUNC_TUTORIALBOOK );

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );

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
	}
	else
	{
		Hide();
	}
}

void cNpcTutorialWindow::OnHide()
{
	cUIWindow::OnHide();

	if( HERO->GetNpcFunc() == NPC_FUNC_TUTORIALBOOK )
	{
		HERO->CloseNpcTalk();
	}

	mNpcClassIndex = 0;
	mNpcIndex = 0;

	/// 퀘스트 정보 클리어 
	Clear();

	if( mNameLabel )
		mNameLabel->Clear();

	if( mpQuestList )
		mpQuestList->Hide();
}

void cNpcTutorialWindow::ShowDefaultPage()
{
	Clear();

	/// 위치설정
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );

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
	}
	
	if( mpQuestList )
		mpQuestList->Hide();
}

bool cNpcTutorialWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mNameLabel = (cLabel*)GetChild( eUIID_TUTORIALBOOK_NAME );
	mpTextBox = (cTextBox*)GetChild( eUIID_TUTORIALBOOK_TEXT );
	mpQuestList = (cListBox*) GetChild( eUIID_TUTORIALBOOK_QUESTLIST );
	mpClose = GetChild( eUIID_TUTORIALBOOK_CLOSE );

	if( mpTextBox )
	{
		mpTextBox->SetFontType( cFontAgent::eFont_Npc );
		mpTextBox->SetDrawTextBottom( false );
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
	mSceneSkin = pSkin->GetNodeSkin( "TutorialBook_Scene" );
	if( mSceneSkin )
	{
		mSceneImage = new cUIImage;
	}

	/// npc 이름색
	if( mNameLabel )
	{
		mNameLabel->SetFontType( cFontAgent::eFont_Npc);
		mNameLabel->SetTextColor( 0xFFE9E47C );
	}

	return true;
}

void cNpcTutorialWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 위치설정
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );

	if( mSceneIndex > 0 && mSceneImage )
	{
		sSceneData* scene = UIMAN->GetSceneData( mSceneIndex );
		if( scene )
		{
			unsigned int x = mSceneSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mSceneSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			unsigned int w = scene->mTexWidth;
			unsigned int h = scene->mTexHeight;
			mSceneImage->SetScreenRect( cUIRect( x, y, x+w, y+h ) );
		}
	}
}

void cNpcTutorialWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

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

	/// NPC 이미지 그린후, 윈도우 그리기
	if( mSceneIndex > 0 && mSceneImage )
		mSceneImage->Draw();
}

void cNpcTutorialWindow::OnCommand( cUINode*, unsigned int id )
{
	if( IsNetLock() == true )
		return;

	switch( id )
	{
	case eUIID_TUTORIALBOOK_CLOSE:
		Hide();
		break;
	}
}

void cNpcTutorialWindow::OnListBoxChanged( cUINode* , unsigned int )
{
	if( IsNetLock() == true )
		return;

	/// 퀘스트 사용상태인지 여부
	if( mIsUseQuest == false )
	{
		SelectQuest();
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

			//if( (mStatus == eNPCQUEST_LOW_LEVEL_NEW || mStatus == eNPCQUEST_NEW) && event == eEVENT_YES )
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


void cNpcTutorialWindow::SelectQuest()
{
	if( !mpQuestList ) return;

	mIsUseQuest = true;

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
		}
	}
}


/// 퀘스트 관련 대사정보 가져와서 화면 구성
bool cNpcTutorialWindow::SetDialog( unsigned long quetIdx, unsigned int status )
{
	mQuestIndex = quetIdx;
	mStatus = status;
	mNextPageIdx = 0;

	/// 연결 대사정보 획득
	sQuestLinkPage* linkPage = mTelling->GetQuestLinkPage( quetIdx );
	if( !linkPage )
		return false;

	unsigned long pageIdx = 0;
	switch( mStatus )
	{
	case eNPCQUEST_PLAYING:			pageIdx = linkPage->playIndex;		break;
//	case eNPCQUEST_LOW_LEVEL_NEW:	pageIdx = linkPage->newIndex;		break;
	case eNPCQUEST_NEW:				pageIdx = linkPage->newIndex;		break;
	case eNPCQUEST_COMPLETE:		pageIdx = linkPage->completeIndex;	break;
	/// complete 페이지로부터 EVENT_COMPLETE 나오는 부분찾기
	case eNPCQUEST_REWARD:			pageIdx = linkPage->rewardIndex;	break;
	default:						assert(0);	return false;
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
//	case eNPCQUEST_LOW_LEVEL_NEW:
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
bool cNpcTutorialWindow::UpdateDialog( bool rewardItemSeleted )
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

	//cQuestDefine* define = QUESTMAN->GetQuestDefine( mQuestIndex );
	//eAnswerEvent event = page->GetAnswerEvent();

	/// 퀘스트 포상 아이템 선택해야 할 경우 안내문 강제 출력
	if( mpQuestList->IsEnabled() == false )
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
	return true;
}

void cNpcTutorialWindow::UpdateScene()
{
	if( mSceneIndex == 0 || !mSceneImage )
		return;

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
			unsigned int x = mSceneSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mSceneSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mSceneImage->SetTextureRect( tx, ty, tx + tw, ty + th );
			mSceneImage->SetScreenRect( cUIRect( x, y, x+tw, y+th ) );
		}
	}
	else
		assert(0);
}

void cNpcTutorialWindow::OnNetLock( int lockTry )
{
	if( mpClose )
		mpClose->NetLock( lockTry );
}

void cNpcTutorialWindow::OnNetUnLock( int lockTry )
{
	if( mpClose )
		mpClose->NetUnLock( lockTry );
}