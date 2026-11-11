#include "stdafx.h"
#include "BoardWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Npc_Common.h"
#include "BoardQuestWindow.h"
#include "BoardListWindow.h"
#include "QuestManager.h"

cBoardWindow::cBoardWindow()
: mpListWindow(0)
, mpQuestWindow(0)
, mCurrentPage(eBOARD_MAX)
, mSelectIndex(0)
{

}

cBoardWindow::~cBoardWindow()
{

}
	
void cBoardWindow::Clear()
{
	// 퀘스트 정보 삭제
	mQuestArr.Clear();
	mSelectIndex = 0;
}

void cBoardWindow::Show( unsigned long npcIndex )
{
	HERO->OpenNpcTalk( npcIndex, false );

	// 퀘스트 리스트 요청
	QUESTMAN->SetBoardQuestList();

	cUIWindow::Show();
}

void cBoardWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );

	HERO->SetNpcFunc( NPC_FUNC_BOARD );
}

void cBoardWindow::OnHide()
{
	cUIWindow::OnHide();

	if( HERO->GetNpcFunc() == NPC_FUNC_BOARD )
	{
		HERO->CloseNpcTalk();
	}

	Clear();
}

bool cBoardWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpListWindow = (cBoardListWindow*)GetChild( eUIID_BOARDLIST_WINDOW );
	mpQuestWindow = (cBoardQuestWindow*)GetChild( eUIID_BOARDQUEST_WINDOW );

	return true;
}

void cBoardWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );
}

void cBoardWindow::OnCommand( cUINode* , unsigned int id )
{
	unsigned int size = mQuestArr.GetSize();

	switch( id )
	{
	case eUIID_BOARD_CLOSE:
		Hide();
		break;
		// quest page로부터 옴
	case eUIID_BOARDQUEST_PREV:
		if( mSelectIndex > 0 )
		{
			if( mSelectIndex >= mQuestArr.GetSize() )
			{
				assert(0);
				return;
			}

			mSelectIndex--;
			mpQuestWindow->SetPage( mQuestArr[mSelectIndex] );
			mpQuestWindow->UpdatePage( mSelectIndex, mQuestArr.GetSize() );
		}
		break;
	case eUIID_BOARDQUEST_NEXT:
		if( mSelectIndex < (int)(size - 1) )
		{
			if( mSelectIndex >= mQuestArr.GetSize() )
			{
				assert(0);
				return;
			}

			mSelectIndex++;
			mpQuestWindow->SetPage( mQuestArr[mSelectIndex] );
			mpQuestWindow->UpdatePage( mSelectIndex, mQuestArr.GetSize() );
		}
		break;
	case eUIID_BOARDQUEST_ACCEPT:
	case eUIID_BOARDLIST_ACCEPT:
		{
			if( mSelectIndex >= mQuestArr.GetSize() )
			{
				assert(0);
				return;
			}
	
			QUESTMAN->QuestAddByNpc( mQuestArr[mSelectIndex], HERO->GetNpcIdx() );
		}
		break;
	case eUIID_BOARDQUEST_RETURN:
		{
			ChangePage( eBOARD_LIST );
		}
		break;
	}
}

// 리스트 윈도우로부터 오는 메세지
void cBoardWindow::SelectList( unsigned int selectIdx )
{
	mSelectIndex = selectIdx;

	// questPage로 넘기기
	ChangePage( eBOARD_QUEST );
}

void cBoardWindow::ChangePage( unsigned char page )
{
	if( mpListWindow )
		mpListWindow->Hide();

	if( mpQuestWindow )
		mpQuestWindow->Hide();

	switch( page )
	{
	case eBOARD_LIST:
		if( mpListWindow )
		{
			// 리스트 페이지
			mpListWindow->Show();
			mpListWindow->UpdateQuestList();

			if( mSelectIndex < mQuestArr.GetSize() )
				mpListWindow->SelectRow( mSelectIndex );
		}
		break;
	case eBOARD_QUEST:
		if( mpQuestWindow )
		{
			if( mSelectIndex >= mQuestArr.GetSize() )
			{
				assert(0);
				return;
			}

			// 퀘스트 페이지
			mpQuestWindow->Show();
			mpQuestWindow->SetPage( mQuestArr[mSelectIndex] );
			mpQuestWindow->UpdatePage( mSelectIndex, mQuestArr.GetSize() );
		}
		break;
	}

	mCurrentPage = page;
}

void cBoardWindow::AddQuest( unsigned long questIndex )
{
	mQuestArr.PushBack( questIndex );
}

void cBoardWindow::OnNetLock( int lockTry )
{
	if( mpListWindow )
		mpListWindow->NetLock( lockTry );

	if( mpQuestWindow )
		mpQuestWindow->NetLock( lockTry );
}

void cBoardWindow::OnNetUnLock( int lockTry )
{
	if( mpListWindow )
		mpListWindow->NetUnLock( lockTry );

	if( mpQuestWindow )
		mpQuestWindow->NetUnLock( lockTry );
}
