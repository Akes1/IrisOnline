#include "stdafx.h"
#include "BoardListWindow.h"
#include "ListBox.h"
#include "GameUIManager.h"
#include "BoardWindow.h"
#include "QuestManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "TriggerManager.h"
#include "ObjectManager.h"

cBoardListWindow::cBoardListWindow()
: mpListBox(0)
, mpAccept(0)
{

}

cBoardListWindow::~cBoardListWindow()
{

}

bool cBoardListWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpListBox = (cListBox*)GetChild( eUIID_BOARDLIST_LIST );
	mpAccept = GetChild( eUIID_BOARDLIST_ACCEPT );

	return true;
}

void cBoardListWindow::OnListBoxChanged( cUINode* caller, unsigned int )
{
	if( mpParent && mpListBox && caller == mpListBox )
	{
		cBoardWindow* board = (cBoardWindow*)mpParent;
		if( board )
		{
			board->SelectChanged( mpListBox->GetSelectIndex() );
		}
	}
}

void cBoardListWindow::OnListBoxSelected( cUINode* caller, unsigned int )
{
	if( mpParent && mpListBox && caller == mpListBox )
	{
		cBoardWindow* board = (cBoardWindow*)mpParent;
		if( board )
		{
			board->SelectList( mpListBox->GetSelectIndex() );
		}
	}
}

void cBoardListWindow::OnCommand( cUINode* caller, unsigned int id )
{
	if( id == eUIID_BOARDLIST_ACCEPT )
	{
		SendEvent( mpParent, eUIEVENT_COMMAND, caller, id );
	}
}

void cBoardListWindow::UpdateQuestList()
{
	cBoardWindow* boardWin = GAMEUI->GetBoardWindow();
	if( !boardWin )
	{
		assert(0);
		return;
	}

	if( mpListBox )
		mpListBox->Clear();

	if( mpAccept )
		mpAccept->SetEnabled( false );

	//
	typedef tArray<unsigned long>* cQuestArr;
	cQuestArr questArr = boardWin->GetQuestArr();
	if( questArr )
	{
		for( unsigned int i = 0; i < questArr->GetSize(); ++i )
		{
			unsigned long questIndex = (*questArr)[i];
			AddQuest( questIndex );
		}

		if( questArr->GetSize() > 0 )
		{
			if( mpAccept )
				mpAccept->SetEnabled( true );
		}
	}
}

void cBoardListWindow::AddQuest( unsigned long questIndex )
{
	// 리스트 삽입
	if( !mpListBox )
		return;

	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( define )
	{
		cStringT title;
		title.Format( GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );

		///저레벨 퀘스트 체크
		unsigned long titleColor = mDefaultColor;
		short lowLevel = TRIGGERMAN->GetLowLevel( questIndex );
		if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
		{
			title.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
			titleColor = COLOR_LOW_LEVEL_QUEST;
		}

		cStringT level;
		level.Format( _T("%d"), define->mLevel );

		// 레벨
		int row = mpListBox->AddRow( (LPCTSTR)level.Cstr(), mDefaultColor );
		if( row != -1 )
		{
			// 제목 20자 잘라내기
			if( title.GetLength() > MAXTITLE_LEN )
			{
				TCHAR temp[256] = {0,};

				unsigned int colWidth = mpListBox->GetColumnMaxWidth( 1 );
				bool ret = UIMAN->ModifyText( (LPCTSTR)title.Cstr(), temp, 256, colWidth );
				if( ret )
					mpListBox->AddColume( row, temp, titleColor );
				else
					mpListBox->AddColume( row, (LPCTSTR)title.Cstr(), titleColor );
			}
			else
			{
				mpListBox->AddColume( row, (LPCTSTR)title.Cstr(), titleColor );
			}

			// 내용
			cStringT text;
			
			/// 진행정보 구성 및 세팅
			cQuestDuty* duty = define->mQuestDuty;
			if( duty )
			{
				cStringT str;
				for( unsigned int i = 0; i < MAX_DUTY; ++i )
				{
					eDutyType type = duty->mDuty[i].dutyType;
					unsigned long targetIdx = duty->mDuty[i].targetIdx;
					unsigned int count = duty->mDuty[i].count;
					unsigned long skillIndex = duty->mDuty[i].skillIndex;

					if( type == eDUTY_NONE )
						break;

					/// 정보 출력
					switch( type )
					{
					case eDUTY_HUNT:		/// 사냥
						{
							LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
							str.Format( GAMERESOURCEMAN->GetGameText( 451 ), monName, count );
						}
						break;
					case eDUTY_COLLECT:		/// 수집
						{
							LPCTSTR itemName = ITEMMAN->GetItemName( targetIdx );
							str.Format( GAMERESOURCEMAN->GetGameText( 453 ), itemName, count );
						}
						break;
					case eDUTY_MEET:		/// npc 만나기
						{
							LPCTSTR npcName = GAMERESOURCEMAN->GetNpcName( targetIdx );
							str.Format( GAMERESOURCEMAN->GetGameText( 455 ), npcName );
						}
						break;
					case eDUTY_ACTIVESKILL:	// 스킬발동
						{
							LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( skillIndex );
							LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );

							str.Format( GAMERESOURCEMAN->GetGameText( 456 ), skillName, monName );
						}
						break;
					case eDUTY_ENHANCE:
						{
							str.Format( GAMERESOURCEMAN->GetGameText( 457 ) );
						}
						break;
					case eDUTY_PUTCARD:
						{
							str.Format( GAMERESOURCEMAN->GetGameText( 458 ) );
						}
						break;
					case eDUTY_PVPJOIN:
					case eDUTY_PVPWIN:
					case eDUTY_PVPLOSE:
						{
							unsigned int pvpIndex = 0;
							switch( targetIdx )
							{
							case MAP_DEATHMATCH:		pvpIndex = 859;			break;
							case MAP_FLAG:				pvpIndex = 860;			break;
							}

							unsigned int textIndex = 0;
							switch( type )
							{
							case eDUTY_PVPJOIN:		textIndex = 465;			break;
							case eDUTY_PVPWIN:		textIndex = 466;			break;
							case eDUTY_PVPLOSE:		textIndex = 467;			break;
							}

							str.Format( GAMERESOURCEMAN->GetGameText( textIndex ), GAMERESOURCEMAN->GetGameText( pvpIndex ), count );
						}
						break;
					default:
						assert(0);
						continue;
					}

					if( text.IsEmpty() == false )
					{
						text.Append( _T(", ") );
					}

					text.Append( (LPCTSTR)str.Cstr() );
					str.Clear();
				}

				// 라인 20자 잘라내기
				if( text.GetLength() > MAXTITLE_LEN )
				{
					TCHAR temp[256] = {0,};

					unsigned int colWidth = mpListBox->GetColumnMaxWidth( 2 );
					
					bool ret = UIMAN->ModifyText( (LPCTSTR)text.Cstr(), temp, 256, colWidth );
					if( ret )
						mpListBox->AddColume( row, temp, mDefaultColor );
					else
						mpListBox->AddColume( row, (LPCTSTR)text.Cstr(), mDefaultColor );
				}
				else
				{
					mpListBox->AddColume( row, (LPCTSTR)text.Cstr(), mDefaultColor );
				}
			}
		}
	}
	else
		assert(0);
}

void cBoardListWindow::SelectRow( int row )
{
	if( mpListBox )
	{
		mpListBox->ChangeSelectRow( row );
	}
}

void cBoardListWindow::OnNetLock( int lockTry )
{
	if( mpListBox )
		mpListBox->NetLock( lockTry );

	if( mpAccept )
		mpAccept->NetLock( lockTry );
}

void cBoardListWindow::OnNetUnLock( int lockTry )
{
	if( mpListBox )
		mpListBox->NetUnLock( lockTry );

	if( mpAccept )
		mpAccept->NetUnLock( lockTry );
}
