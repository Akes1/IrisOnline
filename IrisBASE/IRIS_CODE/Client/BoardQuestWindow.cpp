#include "stdafx.h"
#include "BoardQuestWindow.h"
#include "TextBox.h"
#include "Label.h"
#include "GameUIManager.h"
#include "QuestManager.h"
#include "GameResourceManager.h"
#include "Npc_Common.h"
#include "NpcScript.h"
#include "ItemManager.h"
#include "UIFuncText.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "NumberEditBox.h"
#include "FontAgent.h"
#include "SkillScript.h"
#include "TitleManager.h"
#include "TipWindow.h"

cBoardQuestWindow::cBoardQuestWindow()
: mpDescBox(0)
, mpPage(0)
, mpAcceptButton(0)
, mQuestIndex(0)
{
	memset( &mRewardPos, 0, sizeof(mRewardPos) );
	memset( mpReward, 0, sizeof(mpReward) );

	memset( mDefaultIcon, 0, sizeof(mDefaultIcon) );
	memset( mSelectIcon, 0, sizeof(mSelectIcon) );
}

cBoardQuestWindow::~cBoardQuestWindow()
{

}

void cBoardQuestWindow::Clear()
{
	mQuestIndex = 0;

	if( mpDescBox )
		mpDescBox->Clear();

	if( mpPage )
		mpPage->Clear();
}

bool cBoardQuestWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpDescBox = (cTextBox*)GetChild( eUIID_BOARDQUEST_DESCBOX );
	mpPage = (cLabel*)GetChild( eUIID_BOARDQUEST_PAGE );
	mpPreviewButton = GetChild( eUIID_BOARDQUEST_PREV );
	mpNextButton = GetChild( eUIID_BOARDQUEST_NEXT );
	mpAcceptButton = GetChild( eUIID_BOARDQUEST_ACCEPT );

	mpReward[eLABEL_EXP] = (cLabel*)GetChild( eUIID_BOARDQUEST_EXP );
	mpReward[eLABEL_SXP] = (cLabel*)GetChild( eUIID_BOARDQUEST_SXP );
	mpReward[eLABEL_GOLD] = (cLabel*)GetChild( eUIID_BOARDQUEST_GOLD );
	mpReward[eLABEL_SP] = (cLabel*)GetChild( eUIID_BOARDQUEST_SP );
	mpReward[eLABEL_DEFAULT] = (cLabel*)GetChild( eUIID_BOARDQUEST_DEFAULT );
	mpReward[eLABEL_SELECT] = (cLabel*)GetChild( eUIID_BOARDQUEST_SELECT );

	mpDescBox->SetDrawTextBottom( false );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cString str;
	for( unsigned int i = 0; i < ePOS_MAX; ++i )
	{
		str.Format( "BoardQuest_Reward%d", i );

		cUINodeSkin* nodeSkin = pSkin->GetNodeSkin( str.Cstr() );
		if( nodeSkin )
		{
			mRewardPos[i].mX = nodeSkin->mSkinInfo->mX;
			mRewardPos[i].mY = nodeSkin->mSkinInfo->mY;
		}
	}

	/// 보상용 아이콘 생성
	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "BoardQuest1_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		mDefaultIcon[i].icon = icon;
		mDefaultIcon[i].skinPosX = icon->GetRelativeRect().mLeft;
		mDefaultIcon[i].skinPosY = icon->GetRelativeRect().mTop;
	}

	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "BoardQuest2_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i+10 ) == false )
			return false;

		mSelectIcon[i].icon = icon;
		mSelectIcon[i].skinPosX = icon->GetRelativeRect().mLeft;
		mSelectIcon[i].skinPosY = icon->GetRelativeRect().mTop;
	}

	return true;
}

void cBoardQuestWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	SetReward();
}

void cBoardQuestWindow::OnCommand( cUINode* caller, unsigned int id )
{
	switch( id )
	{
	case eUIID_BOARDQUEST_PREV:
	case eUIID_BOARDQUEST_NEXT:
	case eUIID_BOARDQUEST_ACCEPT:
	case eUIID_BOARDQUEST_RETURN:
		{
			SendEvent( mpParent, eUIEVENT_COMMAND, caller, id );
		}
		break;
	}
}

void cBoardQuestWindow::OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos )
{
	cIcon* hoverIcon = (cIcon*)caller;

	bool defaultIcon = true;
	int index = -1;
	for( unsigned int i = 0; i < 5; ++i )
	{
		if( mDefaultIcon[i].icon == hoverIcon || mSelectIcon[i].icon == hoverIcon )
		{
			if( mSelectIcon[i].icon == hoverIcon )
				defaultIcon = false;

			index = i;
			break;
		}
	}

	if( index == -1 )
		return;

	sBoardRewardIcon& data = ( defaultIcon == true )? mDefaultIcon[index] : mSelectIcon[index];
	if( data.icon )
	{
		int type = data.icon->GetIconType();
		if( type == eICON_ITEM )
		{
			UIMAN->ShowItemTip( eTIP_ITEM, pos, data.index, USHRT_MAX, true );
		}
		else if( type == eICON_SKILL )
		{
			cBuffToolTipWindow* pTip = GAMEUI->GetBuffTip();
			UIMAN->ShowBuffTip( pTip, pos, (unsigned long)-1, data.index, false );
		}
	}
}

void cBoardQuestWindow::SetPage( unsigned long questIndex )
{
	Clear();

	if( questIndex == 0 )
		return;

	if( !mpDescBox )
	{
		assert(0);
		return;
	}

	/// 초기화 
	mpDescBox->Clear();

	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	mQuestIndex = questIndex;

	/// 퀘스트명
	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 443 ), COLOR_GREEN );
	cStringT title;
	title.Format( GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );

	/// 반복여부
//	if( define->mRepeatType == eQUEST_REPEAT )
//		title.Append( GAMERESOURCEMAN->GetGameText( 472 ) );
	
	mpDescBox->AddNewRow( (LPCTSTR)title.Cstr(), mDefaultColor );

	mpDescBox->AddEmptyRow(1);

	/// [의뢰정보] - 의뢰자, 의뢰맵
	cStringT info, mapinfo;
	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 440 ), COLOR_GREEN );

	sNPCList* pNpc = NPCSCRIPT->GetNPCList( define->mTakeNpcIndex );
	if( pNpc )
	{
		info.Format( GAMERESOURCEMAN->GetGameText( 441 ), pNpc->mName );
		mpDescBox->AddNewRow( (LPCTSTR)info.Cstr(), mDefaultColor );
	}

	mapinfo.Format( GAMERESOURCEMAN->GetGameText( 442 ), GAMERESOURCEMAN->GetMapName( define->mTakeMapIndex ) );
	mpDescBox->AddNewRow( (LPCTSTR)mapinfo.Cstr(), mDefaultColor );
	mpDescBox->AddEmptyRow(1);

	/// [ 목표 ]
	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 450 ), COLOR_GREEN );

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

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			case eDUTY_COLLECT:		/// 수집
				{
					LPCTSTR itemName = ITEMMAN->GetItemName( targetIdx );
					str.Format( GAMERESOURCEMAN->GetGameText( 453 ), itemName, count );

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			case eDUTY_MEET:		/// npc 만나기
				{
					LPCTSTR npcName = GAMERESOURCEMAN->GetNpcName( targetIdx );
					str.Format( GAMERESOURCEMAN->GetGameText( 455 ), npcName );

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			case eDUTY_ACTIVESKILL:	// 스킬발동
				{
					LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( skillIndex );
					LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
					str.Format( GAMERESOURCEMAN->GetGameText( 456 ), skillName, monName );

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			case eDUTY_ENHANCE:	
				{
					str.Format( GAMERESOURCEMAN->GetGameText( 457 ) );

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			case eDUTY_PUTCARD:	
				{
					str.Format( GAMERESOURCEMAN->GetGameText( 458 ) );

					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
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
					mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
				}
				break;
			}
		}
	}

	/// 라인 띄기
	mpDescBox->AddEmptyRow(1);

	/// [ 간략정보 ]
	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 452 ), COLOR_GREEN );

	cUIFuncTextParam textParam;
	textParam.mName = HERO->GetName();
	for( unsigned int i = 0; i < define->mDestLineNum; ++i )
	{
		mpDescBox->AddFucnText( QUESTMAN->GetNpcTextNode( define->mDestIndex + i), &textParam );
	}

	// 보상정보 세팅
	SetReward();
}

void cBoardQuestWindow::SetReward()
{
	if( mQuestIndex == 0 )
		return;

	cQuestDefine* define = QUESTMAN->GetQuestDefine( mQuestIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	// 초기화
	for( int i = 0; i < eLABEL_MAX; ++i )
	{
		if( mpReward[i] )
			mpReward[i]->Hide();
	}

	int posIdx = ePOS1;
	int iconPosIdx = ePOS_ITEM1;

	// 보상 세팅
	cQuestReward* reward = define->mDefaultReward;
	if( reward )
	{
		if( reward->mExp > 0 )
		{
			SetDefaultReward( eLABEL_EXP, reward->mExp, posIdx );
			posIdx++;
		}
		if( reward->mSxp > 0 )
		{
			SetDefaultReward( eLABEL_SXP, reward->mSxp, posIdx );
			posIdx++;
		}
		if( reward->mMoney > 0 )
		{
			SetDefaultReward( eLABEL_GOLD, reward->mMoney, posIdx );
			posIdx++;
		}
		if( reward->mSkillPoint > 0 )
		{
			SetDefaultReward( eLABEL_SP, reward->mSkillPoint, posIdx );
			posIdx++;
		}

		if( SetIconReward( reward, mDefaultIcon, iconPosIdx ) == true )
		{
			if( mpReward[eLABEL_DEFAULT] )
			{
				mpReward[eLABEL_DEFAULT]->SetRelativePos( mRewardPos[iconPosIdx] );
				mpReward[eLABEL_DEFAULT]->Show();
			}

			iconPosIdx++;
		}
	}

	cQuestReward* select = define->mSelectReward;
	if( select && define->mSelectCount > 0 )
	{
		if( SetIconReward( select, mSelectIcon, iconPosIdx ) == true )
		{
			if( mpReward[eLABEL_SELECT] )
			{
				mpReward[eLABEL_SELECT]->SetRelativePos( mRewardPos[iconPosIdx] );
				mpReward[eLABEL_SELECT]->Show();
			}
		}
	}
}

void cBoardQuestWindow::SetDefaultReward( unsigned int i, unsigned long value, unsigned int posIdx )
{
	if( mpReward[i] )
	{
		mpReward[i]->SetRelativePos( mRewardPos[posIdx] );
		mpReward[i]->Show();

		LPCTSTR valueStr = cNumberEditBox::FormatNumber( value );
		int len = ::_tcslen( valueStr );

		// 정렬
		cUIRect rc = mpReward[i]->GetRelativeRect();
		int x = TEXT_X + ( TEXT_W - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, valueStr, len ) ) / 2;
		int y = ( rc.GetHeight() - FONTAGENT->GetTextHeight( cFontAgent::eFont_UI) ) / 2;

		mpReward[i]->SetText( valueStr );
		mpReward[i]->SetTextPos( cUIPos( x, y ) );
	}
}

bool cBoardQuestWindow::SetIconReward( cQuestReward* reward, sBoardRewardIcon* icon, unsigned int posIdx )
{
	// 초기화
	for( unsigned int i = 0; i < 5; ++i )
	{
		cIcon* icon = mDefaultIcon[i].icon;
		if( icon )
		{
			mDefaultIcon[i].index = 0;
			icon->SetIconType( eICON_NORMAL );
			icon->ChangeImage(0);
		}

		cIcon* selectIcon = mSelectIcon[i].icon;
		if( selectIcon )
		{
			mSelectIcon[i].index = 0;
			selectIcon->SetIconType( eICON_NORMAL );
			selectIcon->ChangeImage(0);
		}
	}

	bool showImage = false;

	/// 아이콘 세팅
	int iconIdx = 0;
	cItemDefine* itemDefine = 0;
	for( int i = 0; i < QUEST_ITEM_MAX; ++i )
	{
		sQuestItem& item = reward->mRewardItem[i];
		itemDefine = ITEMMAN->GetItemDefine( item.itemIndex );
		if( !itemDefine )
			continue;

		cIconParam* pParam = itemDefine->GetIconParam();
		if( pParam && iconIdx < 5 )
		{
			sBoardRewardIcon* rewardIcon = (icon + iconIdx);
			if( rewardIcon && rewardIcon->icon )
			{
				cIcon* icon = rewardIcon->icon;

				rewardIcon->index = item.itemIndex;
				icon->SetIconType( eICON_ITEM );
				icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
				icon->SetNumber( item.count );

				bool show = ( itemDefine->GetCapacity() > 1 ) ? true : false;
				icon->ShowNumber( show );

				// 아이콘 위치 지정
				int x = rewardIcon->skinPosX + mRewardPos[posIdx].mX;
				int y = rewardIcon->skinPosY + mRewardPos[posIdx].mY;
				icon->SetRelativePos( cUIPos(x, y) );

				iconIdx++;

				showImage = true;
			}
		}
		else
		{
			assert(0);
		}
	}

	/// 버프 세팅
	for( int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		unsigned long buffIdx = reward->mRewardBuff[i];

		sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( buffIdx );
		if( pInfo )
		{
			cIconParam* pParam = UIMAN->GetIconParam( pInfo->mIconIdx );
			if( pParam && iconIdx < 5 )
			{
				sBoardRewardIcon* rewardIcon = (icon + iconIdx);
				if( rewardIcon && rewardIcon->icon )
				{
					cIcon* icon = rewardIcon->icon;

					rewardIcon->index = buffIdx;
					icon->SetIconType( eICON_SKILL );
					icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
					icon->ShowNumber( false );
					iconIdx++;

					// 아이콘 위치 지정
					int x = rewardIcon->skinPosX + mRewardPos[posIdx].mX;
					int y = rewardIcon->skinPosY + mRewardPos[posIdx].mY;
					icon->SetRelativePos( cUIPos(x, y) );

					showImage = true;
				}
			}
			else 
			{
				assert(0);
			}
		}
	}

	/// 타이틀 보상 세팅
	if( reward->mGiveTitleIndex > 0 )
	{
		cTitleDefine* define = TITLEMAN->GetTitleDefine( reward->mGiveTitleIndex );
		if( define )
		{
			sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( define->mInfluenceIndex );
			if( pInfo )
			{
				cIconParam* pParam = UIMAN->GetIconParam( pInfo->mIconIdx );
				if( pParam && iconIdx < 5 )
				{
					sBoardRewardIcon* rewardIcon = (icon + iconIdx);
					if( rewardIcon && rewardIcon->icon )
					{
						cIcon* icon = rewardIcon->icon;

						rewardIcon->index = define->mInfluenceIndex;
						icon->SetIconType( eICON_SKILL );
						icon->ShowNumber( false );
						icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
						iconIdx++;

						// 아이콘 위치 지정
						int x = rewardIcon->skinPosX + mRewardPos[posIdx].mX;
						int y = rewardIcon->skinPosY + mRewardPos[posIdx].mY;
						icon->SetRelativePos( cUIPos(x, y) );

						showImage = true;
					}
				}
				else 
				{
					assert(0);
				}
			}
		}
	}

	return showImage;
}

// 버튼 및 페이지 갱신
void cBoardQuestWindow::UpdatePage( unsigned int selectIdx, unsigned int maxSize )
{
	if( mpPage )
	{
		cStringT str;
		str.Format( _T("%d / %d"), selectIdx+1, maxSize );

		mpPage->SetText( (LPCTSTR)str.Cstr() );
	}

	// 버튼 enable 갱신
	bool preEnable = true;
	bool nextEnable = true;

	// 첫 페이지면서 마지막페이지면
	if( selectIdx == 0 && selectIdx + 1 == maxSize )
	{
		preEnable = false;
		nextEnable = false;
	}
	else
	{
		if( selectIdx == 0 )
		{
			// 첫 페이지면
			preEnable = false;
			nextEnable = true;
		}
		else if( selectIdx + 1 == maxSize )
		{
			// 마지막 페이지면
			preEnable = true;
			nextEnable = false;
		}
	}

	if( mpPreviewButton )
		mpPreviewButton->SetEnabled( preEnable );

	if( mpNextButton )
		mpNextButton->SetEnabled( nextEnable );
}

void cBoardQuestWindow::OnNetLock( int lockTry )
{
	if( mpAcceptButton )
		mpAcceptButton->NetLock( lockTry );

	if( mpPreviewButton )
		mpPreviewButton->NetLock( lockTry );

	if( mpNextButton )
		mpNextButton->NetLock( lockTry );
}

void cBoardQuestWindow::OnNetUnLock( int lockTry )
{
	if( mpAcceptButton )
		mpAcceptButton->NetUnLock( lockTry );

	if( mpPreviewButton )
		mpPreviewButton->NetUnLock( lockTry );

	if( mpNextButton )
		mpNextButton->NetUnLock( lockTry );
}
