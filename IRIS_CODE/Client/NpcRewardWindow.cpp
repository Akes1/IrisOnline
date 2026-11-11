#include "stdafx.h"
#include "NpcRewardWindow.h"
#include "QuestManager.h"
#include "ItemManager.h"
#include "SkillScript.h"
#include "GameUIManager.h"
#include "TipWindow.h"
#include "Label.h"
#include "TitleManager.h"
#include "NpcWindow.h"
#include "NumberEditBox.h"

cNpcRewardWindow::cNpcRewardWindow()
: mpCount(0)
, mpEXP(0)
, mpSXP(0)
, mpSkillPoint(0)
, mpMoney(0)
, mCount(0)
, mMaxCount(0)
, mSelectSize(0, 0)
, mUseReward( false )
{
	::memset( mBackData, 0, sizeof(mBackData) );

	::memset( mDefaultIcon, 0, sizeof(mDefaultIcon) );
	::memset( mSelectIcon, 0, sizeof(mSelectIcon) );

	for( int i = 0; i < 5; ++i )
		mpSelectImg[i] = 0;
}

cNpcRewardWindow::~cNpcRewardWindow()
{
	for( unsigned int i = 0; i < 5; ++i )
		SAFE_DELETE( mpSelectImg[i] );

	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		sRewardBack* back = mBackData[i];
		if( back )
		{
			back->mpNodeSkin = 0;
			SAFE_DELETE( back->mpImage );

			delete back;
			back = 0;
		}

	}
}

/// 아이콘 초기화
void cNpcRewardWindow::ClearIcon()
{
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
			mSelectIcon[i].select = false;
			selectIcon->SetIconType( eICON_NORMAL );
			selectIcon->ChangeImage(0);
		}
	}
}

void cNpcRewardWindow::Open()
{
}

void cNpcRewardWindow::Close()
{
	cUIWindow::Close();

	/// 초기화
	ClearIcon();

	mUseReward = false;
	mCount = 0;
	mMaxCount = 0;
	SetExp( 0 );
	SetSxp( 0 );
	SetMoney( 0 );
	SetSkillPoint( 0 );
	SetSelectCount( 0 );

	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		sRewardBack* back = mBackData[i];
		if( back )
			back->mShow = false;
	}
}

void cNpcRewardWindow::OnHide()
{
	cUIWindow::OnHide();

	/// 초기화
	ClearIcon();
	
	mUseReward = false;
	mCount = 0;
	mMaxCount = 0;
	SetExp( 0 );
	SetSxp( 0 );
	SetMoney( 0 );
	SetSkillPoint( 0 );
	SetSelectCount( 0 );

	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		sRewardBack* back = mBackData[i];
		if( back )
			back->mShow = false;
	}
}

bool cNpcRewardWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	/// 보상용 아이콘 생성
	cString str;
	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "NpcDefault_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		mDefaultIcon[i].icon = icon;
	}

	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "NpcSelect_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i+10 ) == false )
			return false;

		mSelectIcon[i].icon = icon;
	}

	mpEXP = (cLabel*)GetChild( eUIID_NPCREWARD_EXP );
	mpSXP = (cLabel*)GetChild( eUIID_NPCREWARD_SXP );
	mpSkillPoint = (cLabel*)GetChild( eUIID_NPCREWARD_SKILLPOINT );
	mpMoney = (cLabel*)GetChild( eUIID_NPCREWARD_MONEY );
	mpCount = (cLabel*)GetChild( eUIID_NPCREWARD_COUNT );
	
	/// 선택용 이미지 생성	
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 이미지 스킨
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "NpcReward_Select" );
	if( selectSkin == 0 )
	{
		assert( 0 && "failed to find selectSkin" );
		return false;
	}

	for( unsigned int i = 0; i < 5; ++i )
	{
		mpSelectImg[i] = new cUIImage( selectSkin->mpTexture );
		if( mpSelectImg[i] && mSelectIcon[i].icon )
		{
			//const cUIRect& iconRc = mSelectIcon[i].icon->GetAbsoluteRect();

			unsigned int tx = selectSkin->mSkinInfo->mTexX;
			unsigned int ty = selectSkin->mSkinInfo->mTexY;
			unsigned int w = selectSkin->mSkinInfo->mWidth;
			unsigned int h = selectSkin->mSkinInfo->mHeight;

			mSelectSize.mWidth = w;
			mSelectSize.mHeight = h;

			mpSelectImg[i]->SetTextureRect( tx, ty, tx + w, ty + h );
		}
	}

	/// 각 보상요소 배경 이미지 생성
	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		mBackData[i] = new sRewardBack;
		if( !mBackData[i] )
			return false;
		
		str.Format( "NPCReward_Back%02d", i );
		mBackData[i]->mpNodeSkin = pSkin->GetNodeSkin( str );
		if( mBackData[i]->mpNodeSkin )
		{
			mBackData[i]->mpImage = new cUIImage( mBackData[i]->mpNodeSkin->mpTexture );
			unsigned int x = mBackData[i]->mpNodeSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mBackData[i]->mpNodeSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			unsigned int tx = mBackData[i]->mpNodeSkin->mSkinInfo->mTexX;
			unsigned int ty = mBackData[i]->mpNodeSkin->mSkinInfo->mTexY;
			unsigned int w = mBackData[i]->mpNodeSkin->mSkinInfo->mWidth;
			unsigned int h = mBackData[i]->mpNodeSkin->mSkinInfo->mHeight;
			mBackData[i]->mpImage->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ) );
			mBackData[i]->mpImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
		}
	}

	SetExp( 0 );
	SetSxp( 0 );
	SetMoney( 0 );
	SetSkillPoint( 0 );
	SetSelectCount( 0 );
	return true;
}

void cNpcRewardWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	for( unsigned int i = 0; i < 5; ++i )
	{
		if( mpSelectImg[i] && mSelectIcon[i].icon )
		{
			const cUIRect& iconRc = mSelectIcon[i].icon->GetAbsoluteRect();

			unsigned int x = iconRc.mLeft-2;
			unsigned int y = iconRc.mTop-2;
			mpSelectImg[i]->SetScreenRect( cUIRect( x, y, x + mSelectSize.mWidth, y + mSelectSize.mHeight ) );
		}
	}

	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		if( mBackData[i] && mBackData[i]->mpNodeSkin )
		{
			unsigned int x = mBackData[i]->mpNodeSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mBackData[i]->mpNodeSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mBackData[i]->mpImage->SetPos( x, y );
		}
	}
}

void cNpcRewardWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 각 배경이미지 그리기
	for( unsigned int i = 0; i < eBACK_MAX; ++i )
	{
		sRewardBack* back = mBackData[i];
		if( back && back->mpImage && back->mShow == true )
			back->mpImage->Draw();
	}

	if( mCount > 0 )
	{
		for( unsigned int i = 0; i < 5; ++i )
		{
			if( mSelectIcon[i].select == true )
				mpSelectImg[i]->Draw();
		}
	}

	cUIWindow::OnRender( pKeeper );
}

void cNpcRewardWindow::OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos )
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

	sRewardIcon& data = ( defaultIcon == true )? mDefaultIcon[index] : mSelectIcon[index];
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

void cNpcRewardWindow::OnIconClicked( cUINode* caller, unsigned int )
{
	if( mUseReward == false )
		return;

	cIcon* selectIcon = (cIcon*)caller;

	int index = -1;
	for( unsigned int i = 0; i < 5; ++i )
	{
		if( mSelectIcon[i].icon == selectIcon )
		{
			index = i;
			break;
		}
	}

	if( index == -1 )
		return;

	/// 선택 (토글선택)
	if( mSelectIcon[index].select == true && mCount > 0 )
	{
		/// 선택 풀기
		mSelectIcon[index].select = false;
		mCount--;
	}
	else if( mSelectIcon[index].select == false )
	{
		/// 더이상 선택할 수 없으면 가장 맨처음 선택 풀기
		if( mCount >= mMaxCount )
		{
			for( unsigned int i = 0; i < 5; ++i )
			{
				if( mSelectIcon[i].select == true )
				{
					mSelectIcon[i].select = false;
					mCount--;
					break;
				}
			}
		}

		/// 선택 갱신
		mSelectIcon[index].select = true;
		mCount++;

		cNpcWindow* win = GAMEUI->GetNpcWindow();
		if( win )
		{
			win->UpdateDialog( true );
		}
	}
}

void cNpcRewardWindow::UpdateUseReward( long questIdx )
{
	int arrIdx = QUESTMAN->GetQuestArrIdx( questIdx );
	if( arrIdx != -1 )
	{
		unsigned char status = QUESTMAN->GetQuestStatus( arrIdx );

		/// 퀘스트가 진행완료 상태이면 보상 선택 가능 세팅
		mUseReward = ( status == eQUEST_COMPLETE ) ? true : false;
	}
}

bool cNpcRewardWindow::SetQuestReward( long questIdx )
{
	/// 초기화
	ClearIcon();

	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;
	}
	
	/// 보상 선택가능한지 세팅
	UpdateUseReward( questIdx );

	/// 기본 보상 세팅
	cQuestReward* reward = define->mDefaultReward;
	if( reward )
	{
		if( reward->mExp > 0 )
		{
			SetExp( reward->mExp );
			mBackData[eBACK_EXP]->mShow = true;
		}

		if( reward->mSxp > 0 )
		{
			SetSxp( reward->mSxp );
			mBackData[eBACK_SXP]->mShow = true;
		}

		if( reward->mMoney > 0 )
		{
			SetMoney( reward->mMoney );
			mBackData[eBACK_MONEY]->mShow = true;
		}

		if( reward->mSkillPoint > 0 )
		{
			SetSkillPoint( reward->mSkillPoint );
			mBackData[eBACK_SKILLPOINT]->mShow = true;
		}

		bool isShowImage = UpdateReward( reward, mDefaultIcon );
		mBackData[eBACK_DEFAULT]->mShow = isShowImage;
	}
	
	/// 선택 보상 세팅
	cQuestReward* selectReward = define->mSelectReward;
	if( selectReward && define->mSelectCount > 0 )
	{
		mMaxCount = define->mSelectCount;
		SetSelectCount( define->mSelectCount );

		bool isShowImage = UpdateReward( selectReward, mSelectIcon );
		mBackData[eBACK_SELECT]->mShow = isShowImage;
	}
	return true;
}

bool cNpcRewardWindow::UpdateReward( cQuestReward* reward, sRewardIcon* icon )
{
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
			sRewardIcon* rewardIcon = (icon + iconIdx);
			if( rewardIcon && rewardIcon->icon )
			{
				cIcon* icon = rewardIcon->icon;

				rewardIcon->index = item.itemIndex;
				icon->SetIconType( eICON_ITEM );
				icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
				icon->SetNumber( item.count );

				bool show = ( itemDefine->GetCapacity() > 1 ) ? true : false;
				icon->ShowNumber( show );

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
				sRewardIcon* rewardIcon = (icon + iconIdx);
				if( rewardIcon && rewardIcon->icon )
				{
					cIcon* icon = rewardIcon->icon;

					rewardIcon->index = buffIdx;
					icon->SetIconType( eICON_SKILL );
					icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
					icon->ShowNumber( false );
					iconIdx++;

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
					sRewardIcon* rewardIcon = (icon + iconIdx);
					if( rewardIcon && rewardIcon->icon )
					{
						cIcon* icon = rewardIcon->icon;

						rewardIcon->index = define->mInfluenceIndex;
						icon->SetIconType( eICON_SKILL );
						icon->ShowNumber( false );
						icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
						iconIdx++;

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

void cNpcRewardWindow::SetExp( unsigned int exp )
{
	if( mpEXP )
	{
		LPCTSTR expStr = cNumberEditBox::FormatNumber( exp );
		mpEXP->SetText( expStr );

		if( exp > 0 )
			mpEXP->Show();
		else
			mpEXP->Hide();
	}
}

void cNpcRewardWindow::SetSxp( unsigned int sxp )
{
	if( mpSXP )
	{
		LPCTSTR sxpStr = cNumberEditBox::FormatNumber( sxp );
		mpSXP->SetText( sxpStr );

		if( sxp > 0 )
			mpSXP->Show();
		else
			mpSXP->Hide();
	}
}

void cNpcRewardWindow::SetMoney( unsigned long money )
{
	if( mpMoney )
	{
		LPCTSTR moneyStr = cNumberEditBox::FormatNumber( money );
		mpMoney->SetText( moneyStr );

		if( money > 0 )
			mpMoney->Show();
		else
			mpMoney->Hide();
	}
}

void cNpcRewardWindow::SetSkillPoint( unsigned int sp )
{
	if( mpSkillPoint )
	{
		LPCTSTR spStr = cNumberEditBox::FormatNumber( sp );
		mpSkillPoint->SetText( spStr );

		if( sp > 0 )
			mpSkillPoint->Show();
		else
			mpSkillPoint->Hide();
	}
}

void cNpcRewardWindow::SetSelectCount( unsigned int count )
{
	if( mpCount )
	{
		mpCount->SetValue( count );

		if( count > 0 )
			mpCount->Show();
		else
			mpCount->Hide();
	}
}

