#include "StdAfx.h"
#include "QuestWindow.h"
#include "QuestManager.h"
#include "GameResourceManager.h"
#include "SkillScript.h"
#include "TipWindow.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "UIFuncText.h"
#include "ItemManager.h"
#include "NpcScript.h"
#include "Npc_Common.h"
#include "QuestSimpleWindow.h"
#include "TitleManager.h"
#include "CheckBox.h"
#include "PlaneObject.h"
#include "Label.h"
#include "TextBox.h"
#include "Button.h"
#include "NumberEditBox.h"
#include "ScrollBar.h"
#include "SoundSystem.h"
#include "TriggerManager.h"

const int REWARD_ITEM_IMG_GAP = 30;

cQuestWindow::cQuestWindow()
: mpListBox(0)
, mpCount(0)
, mpCompleteCount(0)
, mpProgressCount(0)
, mpDescBox(0)
, mpDelete(0)
, mpHighImage(0)
, mpLowImage(0)
, mTimeLine(-1)
, mSelectQuestIndex(0)
, mpViewInfo(0)
{

	for( unsigned int idx = 0; idx < eQUEST_REWARD_LABEL_MAX; ++idx )
	{
		mpRewardLabel[idx]   = NULL;
	}

	for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
	{
		mpRewordBack[idx]    = NULL;
	}

	for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
	{
		mpRewordBackImg[idx] = NULL;
		mIsShowBackImg[idx]  = false;
	}
}

cQuestWindow::~cQuestWindow()
{
	SAFE_DELETE( mpLowImage );
	SAFE_DELETE( mpHighImage );

	for( unsigned int cnt = 0; cnt < eQUEST_REWARD_IMG_MAX; ++cnt )
	{
		SAFE_DELETE( mpRewordBackImg[cnt] );
	}
}

/// 퀘스트 리스트 갱신될때 호출됨
void cQuestWindow::Clear()
{
	if( mpListBox )
		mpListBox->Clear();

	/// 초기화 
	if( mpDescBox )
		mpDescBox->Clear();
	
	/// 초기화
	ClearIcon();

	mTimeLine = -1;
	mSelectQuestIndex = 0;
	UpdateCount( 0, 0 );

	if( mpDelete )
		mpDelete->SetEnabled( false );

	for( unsigned int idx = 0; idx < eQUEST_REWARD_LABEL_MAX; ++idx )
	{
		mpRewardLabel[idx]->Hide();
	}

	for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
	{
		mIsShowBackImg[idx] = false;
	}

	if( mpCount )
		mpCount->Hide();
}

/// 아이콘 초기화
void cQuestWindow::ClearIcon()
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
			selectIcon->SetIconType( eICON_NORMAL );
			selectIcon->ChangeImage(0);
		}
	}
}

void cQuestWindow::Open()
{
}

void cQuestWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cQuestWindow::OnShow()
{
	cUIWindow::OnShow();

	mpDelete->SetEnabled( false );

	/// 보일때 갱신
	UpdateQuestInfo();
}

bool cQuestWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpListBox = (cQuestListFolder*)GetChild( eUIID_QUEST_FOLDER );
	if( !mpListBox )
		return false;

	/// 보상용 아이콘 생성
	cString str;
	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "RewardDefault_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		mDefaultIcon[i].icon = icon;
	}

	for( unsigned int i = 0; i < 5; ++i )
	{
		str.Format( "RewardSelect_Icon%d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, i+10 ) == false )
			return false;

		mSelectIcon[i].icon = icon;
	}

		
	for( unsigned int cnt = 0; cnt < eQUEST_REWARD_LABEL_MAX; ++cnt )
	{
		mpRewardLabel[cnt] = (cLabel*)GetChild( eUIID_QUEST_REWARD_LABEL0 + cnt  );
	}

	mpCount = (cLabel*)GetChild( eUIID_QUEST_COUNT );

	mpCompleteCount = (cLabel*)GetChild( eUIID_QUEST_COMPLETE );
	mpProgressCount = (cLabel*)GetChild( eUIID_QUEST_PROGRESS );
	mpDescBox = (cTextBox*)GetChild( eUIID_QUEST_DESCBOX );
	mpDelete = (cButton*)GetChild( eUIID_QUEST_DELETE );
	mpViewInfo = (cCheckBox*)GetChild( eUIID_QUEST_VIEWINFO );

	if( mpViewInfo )
		mpViewInfo->SetPress( true );

	if( mpProgressCount )
		mpProgressCount->SetTextColor( COLOR_GREEN );

	if( mpCompleteCount )
		mpCompleteCount->SetTextColor( COLOR_GREEN );

	if( mpDescBox )
		mpDescBox->SetDrawTextBottom( false );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	for( unsigned int cnt = 0; cnt < eQUEST_REWARD_IMG_MAX; ++cnt )
	{
		cString name;
		name.Format( "QuestReward_Back%02d", cnt );
		cUINodeSkin* pNodeSkin = pSkin->GetNodeSkin( name.Cstr() );
		if( pNodeSkin )
		{
			mpRewordBack[cnt]	 = pNodeSkin;
			mpRewordBackImg[cnt] = new cUIImage( pNodeSkin->mpTexture );
			
			unsigned int tx =  pNodeSkin->mSkinInfo->mTexX;
			unsigned int ty =  pNodeSkin->mSkinInfo->mTexY;
			unsigned int  w =  pNodeSkin->mSkinInfo->mWidth;
			unsigned int  h =  pNodeSkin->mSkinInfo->mHeight;
			unsigned int  x =  pNodeSkin->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  pNodeSkin->mSkinInfo->mY + mAbsoluteRect.mTop;

			mpRewordBackImg[cnt]->SetTextureRect( tx, ty, tx + w, ty + h );
			mpRewordBackImg[cnt]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
		}
		else
		{
			assert(0);
			return false;
		}
	}

	/// 이미지 생성
	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "Quest_FolderHigh" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "Quest_FolderHighNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		unsigned int tx = pHighSkin->mSkinInfo->mTexX;
		unsigned int ty = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		unsigned int x = pHighSkin->mSkinInfo->mX;
		unsigned int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pHighNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pHighNoSkin->mSkinInfo->mTexY;

		mpListBox->SetHighNoCheckPos( tx, ty );
		mpListBox->SetHighCheckPos( tx2, ty2 );
	}


	cUINodeSkin* pLowSkin = pSkin->GetNodeSkin( "Quest_FolderLow" );
	cUINodeSkin* pLowNoSkin = pSkin->GetNodeSkin( "Quest_FolderLowNoCheck" );
	if( pLowSkin && pLowNoSkin )
	{
		mpLowImage = new cUIImage( pLowSkin->mpTexture );

		unsigned int tx = pLowSkin->mSkinInfo->mTexX;
		unsigned int ty = pLowSkin->mSkinInfo->mTexY;
		unsigned int w = pLowSkin->mSkinInfo->mWidth;
		unsigned int h = pLowSkin->mSkinInfo->mHeight;
		unsigned int x = pLowSkin->mSkinInfo->mX;
		unsigned int y = pLowSkin->mSkinInfo->mY;
		
		mpLowImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLowImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pLowNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pLowNoSkin->mSkinInfo->mTexY;

		mpListBox->SetLowCheckPos( tx, ty );
		mpListBox->SetLowNoCheckPos( tx2, ty2 );
	}
	return true;
}

void cQuestWindow::UpdateRect()
{
	cUIWindow::UpdateRect();
	for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
	{
		if( mIsShowBackImg[idx] )
		{
			SetQuestReward();
			break;
		}
	}
}

void cQuestWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_QUEST_CLOSE:
		/// 닫기
		Hide();
		break;
	case eUIID_QUEST_DELETE:
		{
			/// 퀘스트 포기	
			unsigned int row  = mpListBox->GetSelectRow();
			unsigned int column = mpListBox->GetSelectRowColumn();
			if( column > 0 )
			{
				unsigned int arrIdx = (unsigned int)mpListBox->GetColumnData( row, column );
				unsigned long questIdx = QUESTMAN->GetQuestIndex( arrIdx );
				cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
				if( !define )
				{
					assert(0);
					return;
				}

				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 416 ), GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ));

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_QUEST_DELETE, eSKIN_YESNO,
									(LPCTSTR)str.Cstr(),
									GAMERESOURCEMAN->GetGameText( 301 ),	
									(void*)arrIdx );
			}
		}
		break;
	}
}

void cQuestWindow::OnCheckBoxChanged( cUINode* , unsigned int id )
{
	if( id == eUIID_QUEST_VIEWINFO )
	{
		cQuestSimpleWindow* simple = GAMEUI->GetQuestSimpleWindow();
		if( mpViewInfo && simple )
		{
			if( mpViewInfo->IsPress() == true )
			{
				// 켜기
				simple->Show();
			}
			else
			{
				// 끄기
				simple->Hide();
			}	
		}
		else
			assert(0);
	}
}

void cQuestWindow::OnListBoxChanged( cUINode* caller, unsigned int id )
{
	cQuestWindow::OnListBoxSelected( caller, id );
}

void cQuestWindow::OnListBoxSelected(cUINode* , unsigned int )
{
	if( !mpListBox ) return;

	unsigned int row  = mpListBox->GetSelectRow();
	unsigned int column = mpListBox->GetSelectRowColumn();
	unsigned long questIdx = 0;
	if( column > 0 )
	{
		unsigned int arrIdx = (unsigned int)mpListBox->GetColumnData( row, column );
		questIdx = QUESTMAN->GetQuestIndex( arrIdx );
	}

	// 추적 선택 
	if( questIdx == 0 )
	{
		/// 초기화 
		if( mpDescBox )
			mpDescBox->Clear();

		/// 초기화
		ClearIcon();
		
		if( mpDelete )
			mpDelete->SetEnabled( false );

		for( unsigned int idx = 0; idx < eQUEST_REWARD_LABEL_MAX; ++idx )
		{
			mpRewardLabel[idx]->Hide();
		}

		for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
		{
			mIsShowBackImg[idx] = false;
		}

		if( mpCount )
			mpCount->Hide();

		/// 추적 초기화
		QUESTMAN->ClearChaseQuest();
	}
	else
		QUESTMAN->UpdateChaseQuest( questIdx );
}

void cQuestWindow::UpdateSelect()
{
	if( !mpListBox )
	{
		assert(0);
		return;
	}

	// 추적 퀘스트 인덱스를 가진 열을 선택하도록 함.
	unsigned long questIndex = QUESTMAN->GetChangeQuestIndex();
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
	{
		mpListBox->ShowOn( false );
		return;
	}

	int arrIdx = QUESTMAN->GetQuestArrIdx( questIndex );
	if( arrIdx == -1 )
		return;

	int column = mpListBox->FindColumnByExtra( define->mTakeMapIndex, arrIdx );
	mpListBox->ChangeSelectColumn( column );

	if( column != -1 )
	{
		// 표시해줄 정보가 있는지 없는지 판단
		bool show = ( define->mQuestInfo ) ? true : false;
		mpListBox->ShowOn( show );
	}
	else
	{
		mpListBox->ShowOn( false );
	}
	
	/// 퀘스트 정보 업데이트
	SetQuestReward();
	UpdateQuestInfo();
}

void cQuestWindow::OnImageClicked( cUINode* , unsigned int )
{
	if( mpListBox )
	{
		unsigned int row = mpListBox->GetSelectRow();
		unsigned int column = mpListBox->GetSelectRowColumn();
		unsigned int arrIdx = (unsigned int)mpListBox->GetColumnData( row, column );
		unsigned long questIdx = QUESTMAN->GetQuestIndex( arrIdx );

		bool check = QUESTMAN->IsSelectQuest( questIdx );
		if( check == true )
		{
			/// 체크 풀기
			sFolderData* data = mpListBox->GetFolderData( row, column );
			if( data )
				data->mCheck = false;

			QUESTMAN->DeleteQuestSelect( questIdx );
		}
		else
		{
			/// 체크 추가
			sFolderData* data = mpListBox->GetFolderData( row, column );
			if( data )
				data->mCheck = true;

			QUESTMAN->AddQuestSelect( questIdx );
		}
	}
}

void cQuestWindow::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
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
bool cQuestWindow::SetQuestReward()
{
	/// 초기화
	ClearIcon();
	ClearLabels();

	/// 선택된 퀘스트의 보상정보 세팅
	unsigned int row  = mpListBox->GetSelectRow();
	unsigned int column = mpListBox->GetSelectRowColumn();

	/// 상위열을 선택하였을 경우.
	if( column == 0 )
		return false;

	unsigned int arrIdx = (unsigned int)mpListBox->GetColumnData( row, column );
	unsigned long questIdx = QUESTMAN->GetQuestIndex( arrIdx );
	
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;
	}

	SetSelectCount( define->mSelectCount );

	bool		 isShowDefaultIcon = false;
	bool		 isShowSelectIcon  = false;
	unsigned int rewardCounter = 0;

	/// 기본 보상 세팅
	cQuestReward* reward = define->mDefaultReward;
	if( reward )
	{
		if( reward->mExp > 0 )
		{
			
			/// 글자 출력
			LPCTSTR expStr = cNumberEditBox::FormatNumber( reward->mExp  );
			mpRewardLabel[rewardCounter]->SetText( expStr );
			mpRewardLabel[rewardCounter]->Show();

			///셋팅해야 할 위치값을 얻어와 해당 이미지를 셋팅해 준다.
			unsigned int  w =  mpRewordBack[rewardCounter]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[rewardCounter]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[rewardCounter]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[rewardCounter]->mSkinInfo->mY + mAbsoluteRect.mTop;
			mpRewordBackImg[eQUEST_REWARD_EXP_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_EXP_IMG] = true;
			rewardCounter++;
		}
		
		if( reward->mSxp > 0 )
		{
			/// 글자 출력
			LPCTSTR sxpStr = cNumberEditBox::FormatNumber( reward->mSxp  );
			mpRewardLabel[rewardCounter]->SetText( sxpStr );
			mpRewardLabel[rewardCounter]->Show();

			///셋팅해야 할 위치값을 얻어와 해당 이미지를 셋팅해 준다.
			unsigned int  w =  mpRewordBack[rewardCounter]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[rewardCounter]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[rewardCounter]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[rewardCounter]->mSkinInfo->mY + mAbsoluteRect.mTop;
			mpRewordBackImg[eQUEST_REWARD_SXP_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_SXP_IMG] = true;
			rewardCounter++;
		}
		
		if( reward->mMoney > 0 )
		{
			/// 글자 출력
			LPCTSTR moneyStr = cNumberEditBox::FormatNumber( reward->mMoney  );
			mpRewardLabel[rewardCounter]->SetText( moneyStr );
			mpRewardLabel[rewardCounter]->Show();

			///셋팅해야 할 위치값을 얻어와 해당 이미지를 셋팅해 준다.
			unsigned int  w =  mpRewordBack[rewardCounter]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[rewardCounter]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[rewardCounter]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[rewardCounter]->mSkinInfo->mY + mAbsoluteRect.mTop;
			mpRewordBackImg[eQUEST_REWARD_GOLD_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_GOLD_IMG] = true;
			rewardCounter++;
		}
		
		if( reward->mSkillPoint > 0 )
		{
			/// 글자 출력
			LPCTSTR spStr = cNumberEditBox::FormatNumber( reward->mSkillPoint  );
			mpRewardLabel[rewardCounter]->SetText( spStr );
			mpRewardLabel[rewardCounter]->Show();

			///셋팅해야 할 위치값을 얻어와 해당 이미지를 셋팅해 준다.
			unsigned int  w =  mpRewordBack[rewardCounter]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[rewardCounter]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[rewardCounter]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[rewardCounter]->mSkinInfo->mY + mAbsoluteRect.mTop;
			mpRewordBackImg[eQUEST_REWARD_SP_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_SP_IMG] = true;
			rewardCounter++;
		}

		isShowDefaultIcon = UpdateRewardIcon( reward, mDefaultIcon );
	}

	/// 선택 보상 세팅
	cQuestReward* selectReward = define->mSelectReward;
	if( selectReward )
	{
		 isShowSelectIcon = UpdateRewardIcon( selectReward, mSelectIcon );
	}

	///기본지급 이미지와 선택 지급 이미지 위치 조정
 	if( rewardCounter < 3 )
 	{
		if( isShowDefaultIcon )
 		{
 			///기본지급 위치를 한칸 위로 옮겨서 표시한다.
			unsigned int  w =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop - REWARD_ITEM_IMG_GAP;
			mpRewordBackImg[eQUEST_REWARD_BASE_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = true;

 			if( isShowSelectIcon )
 			{
 				/// 선택 지급 위치를 한칸 위로 옮겨서 표시한다.
				unsigned int  w =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mWidth;
				unsigned int  h =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mHeight;
				unsigned int  x =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
				unsigned int  y =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop - REWARD_ITEM_IMG_GAP;
				mpRewordBackImg[eQUEST_REWARD_SELECT_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
				mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = true;
 			}
 			else
 			{
				/// 선택 지급 이미지 숨기기
 				mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = false;
				mpCount->Hide();
 			}
 		}
		else if( isShowSelectIcon )
 		{
			/// 기본지급 이미지 숨기기
 			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = false;
 
 			/// 선택 지급 위치를 위로 2칸 옮겨서 표시한다.
			unsigned int  w =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop - (REWARD_ITEM_IMG_GAP*2);
			mpRewordBackImg[eQUEST_REWARD_SELECT_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = true;
 
 		}
 		else
 		{
 			//기본지급, 선택지급 이미지 숨기기
			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = false;
			mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = false;
			mpCount->Hide();
 		}
	}
	else
	{
		if( isShowDefaultIcon )
		{
			///기본지급 위치를 원래 자리에 표시한다.
			unsigned int  w =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[eQUEST_REWARD_BASE_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop;
			mpRewordBackImg[eQUEST_REWARD_BASE_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = true;

			if( isShowSelectIcon )
			{
				/// 선택 지급 위치를 원래 자리에 표시한다.
				unsigned int  w =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mWidth;
				unsigned int  h =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mHeight;
				unsigned int  x =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
				unsigned int  y =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop;
				mpRewordBackImg[eQUEST_REWARD_SELECT_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
				mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = true;
			}
			else
			{
				/// 선택 지급 이미지 숨기기
				mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = false;
				mpCount->Hide();
			}
		}
		else if( isShowSelectIcon )
		{
			/// 기본지급 이미지 숨기기
			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = false;

			/// 선택 지급 위치를 한칸 위로 옮겨서 표시한다.
			unsigned int  w =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mWidth;
			unsigned int  h =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mHeight;
			unsigned int  x =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpRewordBack[eQUEST_REWARD_SELECT_ITEM_IMG]->mSkinInfo->mY + mAbsoluteRect.mTop - REWARD_ITEM_IMG_GAP;
			mpRewordBackImg[eQUEST_REWARD_SELECT_ITEM_IMG]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
			mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = true;

		}
		else
		{
			//기본지급, 선택지급 이미지 숨기기
			mIsShowBackImg[eQUEST_REWARD_BASE_ITEM_IMG] = false;
			mIsShowBackImg[eQUEST_REWARD_SELECT_ITEM_IMG] = false;
			mpCount->Hide();
		}
	}

	/// 아이콘 위치 업데이트
	if( isShowDefaultIcon )
	{
		int relativeY = mpRewordBackImg[eQUEST_REWARD_BASE_ITEM_IMG]->GetScreenPosY() - mAbsoluteRect.mTop + 3;
		for( unsigned int i = 0; i < 5; ++i )
		{
			cIcon* pIcon = mDefaultIcon[i].icon;
			if( pIcon )
			{
				pIcon->SetRelativePos( cUIPos( pIcon->GetRelativeRect().mLeft, relativeY ) );
			}
			else
			{
				assert(0);
				return false;
			}
		}
	}
	if( isShowSelectIcon )
	{
		int relativeY = mpRewordBackImg[eQUEST_REWARD_SELECT_ITEM_IMG]->GetScreenPosY() - mAbsoluteRect.mTop + 3;
		for( unsigned int i = 0; i < 5; ++i )
		{
			cIcon* pIcon = mSelectIcon[i].icon;
			if( pIcon )
			{
				pIcon->SetRelativePos( cUIPos( pIcon->GetRelativeRect().mLeft, relativeY ) );
			}
			else
			{
				assert(0);
				return false;
			}
		}
		mpCount->SetRelativePos( cUIPos(mpCount->GetRelativeRect().mLeft, relativeY + 7) );
		mpCount->Show();
	}

	return true;
}

bool cQuestWindow::UpdateRewardIcon( cQuestReward* reward, sRewardIcon* icon )
{
	bool retValue = false;

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
				retValue = true;
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
					icon->ShowNumber( false );
					icon->ChangeImage( pParam, ICON_SIZE, ICON_SIZE );
					iconIdx++;
					retValue = true;
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
						retValue = true;
					}
				}
				else
				{
					assert(0);
				}
			}
		}
	}

	return retValue;
	
}

void cQuestWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mTimeLine == -1 || mSelectQuestIndex <= 0 )
		return;

	/// 시간갱신
	long restTime = QUESTMAN->GetQuestRestTime( mSelectQuestIndex );
	int time = (int)(restTime*0.001);
	if( time >= 0 )
	{
		int h = time / 3600;
		int m = time / 60;
		int s = time % 60;

		cStringT th,tm,ts;
		if( h != 0 )
			th.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );
		
		if( m != 0 )
			tm.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
		ts.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );

		th = (th+tm+ts);
		
		cStringT timeStr;
		timeStr.Format( GAMERESOURCEMAN->GetGameText( 471 ), (LPCTSTR)th.Cstr() );
		mpDescBox->UpdateRowText( mTimeLine, (LPCTSTR)timeStr.Cstr() );
	}
}

void cQuestWindow::OnRender( cUIFontItemKeeper* pKeeper )
{

	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	for( unsigned int idx = 0; idx < eQUEST_REWARD_IMG_MAX; ++idx )
	{
		if( mIsShowBackImg[idx] )
		{
			mpRewordBackImg[idx]->Draw();
		}
	}

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


/// 모든정보는 퀘스트매니저의 퀘스트정보, 배열인덱스로 참조
bool cQuestWindow::AddQuest( unsigned int arrIdx, unsigned long questIdx, bool complete, bool IsNewQuest )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;
	}

	///
	cStringT title;
	unsigned long textColor = mDefaultColor;
	title.Format( GAMERESOURCEMAN->GetGameText( 444 ), define->mLevel, GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );

	///반복 체크
//	if( define->mRepeatType == eQUEST_REPEAT )
//		title.Append( GAMERESOURCEMAN->GetGameText( 472 ) );

	///저 레벨 퀘스트 체크.
	short lowLevel = TRIGGERMAN->GetLowLevel( questIdx );
	if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
	{
		title.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
		textColor = COLOR_LOW_LEVEL_QUEST;
	}

	////완료 체크
	if( complete == true )
		title.Append( GAMERESOURCEMAN->GetGameText( 445 ) );


	/// 체크 상태 가져오기
	bool check = QUESTMAN->IsSelectQuest( questIdx );

	/// 맵이 존재하면 정보 세팅
	if( define->mTakeMapIndex > 0 )
	{
		mpListBox->AddHighData( define->mTakeMapIndex, GAMERESOURCEMAN->GetMapName( define->mTakeMapIndex ), COLOR_YELLOW, 0, mpHighImage );

		/// 퀘스트 정보 삽입
		mpListBox->AddLowData( define->mTakeMapIndex, (LPCTSTR)title.Cstr(), textColor, (void*)arrIdx, mpLowImage, check );
	}
	else
	{
		assert(0);
	}

	/// 새로운 퀘스트 습득 시 이펙트 보여주기
	if( IsNewQuest )
	{
		if( complete == true )
		{
			/// 획득 과 동시에 완료인 경우
			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
			HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_get_04.nif", &trans, false, true );
			SOUNDSYS->Play2DSound( 2011 );
		}
		else
		{
			/// 퀘스트 신규 획득 효과
			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
			HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_get_03.nif", &trans, false, true );
			SOUNDSYS->Play2DSound( 2010 );

		}
	}

	return true;
}

void cQuestWindow::SetCheck( unsigned int arrIdx, unsigned long questIdx, bool check )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( define )
	{
		sFolderData* data = mpListBox->GetFolderDataByExtra( define->mTakeMapIndex, (void*)arrIdx );
		if( data )
		{
			data->mCheck = check;
		}
		else
			assert(0);
	}
}

void cQuestWindow::SetCompleteQuest( unsigned int arrIdx, unsigned long questIdx )
{
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
	if( define )
	{
		unsigned long textColor = mDefaultColor;
		cStringT title;
		title.Format( GAMERESOURCEMAN->GetGameText( 444 ), define->mLevel, GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );
		
		/// 반복여부
//		if( define->mRepeatType == eQUEST_REPEAT )
//			title.Append( GAMERESOURCEMAN->GetGameText( 472 ) );

		///저 레벨 퀘스트 체크.
		short lowLevel = TRIGGERMAN->GetLowLevel( questIdx );
		if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
		{
			title.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
			textColor = COLOR_LOW_LEVEL_QUEST;
		}

		title.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

		sFolderData* data = mpListBox->GetFolderDataByExtra( define->mTakeMapIndex, (void*)arrIdx );
		if( data )
		{
			data->mText = title;
			data->mTextColor = textColor;
		}
		else
			assert(0);

		/// 퀘스트 조건 충족 이펙트 보여주기
		NiTransform trans;
		trans.MakeIdentity();
		trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
		HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_get_04.nif", &trans, false, true );
		SOUNDSYS->Play2DSound( 2011 );
	}
}


/// 퀘스트 설명및 진행상황 출력
void cQuestWindow::UpdateQuestInfo()
{
	if( !mVisible ) return;

	if( mpDelete )
		mpDelete->SetEnabled( false );

	if( !mpDescBox ) 
	{
		assert(0);
		return;
	}

	if( mpListBox->GetRowCount() == 0 )
		return;

	/// 초기화 
	mpDescBox->Clear();
	mSelectQuestIndex = 0;
	mTimeLine = -1;

	/// 선택된 퀘스트의 보상정보 세팅
	unsigned int row  = mpListBox->GetSelectRow();
	unsigned int column = mpListBox->GetSelectRowColumn();

	/// 상위열을 선택하였을 경우.
	if( column == 0 )
		return;

	unsigned int arrIdx = (unsigned int)mpListBox->GetColumnData( row, column );
	unsigned char status = QUESTMAN->GetQuestStatus( arrIdx );
		
	mSelectQuestIndex = QUESTMAN->GetQuestIndex( arrIdx );
	cQuestDefine* define = QUESTMAN->GetQuestDefine( mSelectQuestIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	/// [ 목표 ]
	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 450 ), COLOR_GREEN );
	if( status == eQUEST_COMPLETE )
	{
		mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 449 ), mDefaultColor );
	}
	else if( status == eQUEST_PLAYING )
	{
		/// 남은시간 문자열 출력 ( 퀘스트매니저에서 가져오기 )
		cQuestLimit* limit = define->mQuestLimit;
		if( limit && limit->mTimeType != eTIME_NONE )
		{
			long restTime = QUESTMAN->GetQuestRestTime( mSelectQuestIndex );
			cStringT time;
			int h = restTime / 3600;
			int m = restTime / 60;
			int s = restTime % 60;

			cStringT th,tm,ts;
			if( h != 0 )
				th.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );

			if( m != 0 )
				tm.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
			ts.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );

			th = (th+tm+ts);

			cStringT timeStr;
			timeStr.Format( GAMERESOURCEMAN->GetGameText( 471 ), (LPCTSTR)th.Cstr() );
			mTimeLine = mpDescBox->AddNewRow( (LPCTSTR)timeStr.Cstr(), mDefaultColor );
		}

		/// 진행정보 구성 및 세팅
		cQuestDuty* duty = define->mQuestDuty;
		if( duty )
		{
			cStringT str;
			__int64 check = QUESTMAN->GetProgress( arrIdx );

			__int64 end = 0xFF;
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

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
						/// 진행중
						else
						{
							__int64 cnt = check & end;
							for( unsigned int b = 0; b < i; ++b)
								cnt = cnt >> 8;

							cStringT str1;
							str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

							str += str1;
						}

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				case eDUTY_COLLECT:		/// 수집
					{
						LPCTSTR itemName = ITEMMAN->GetItemName( targetIdx );
						str.Format( GAMERESOURCEMAN->GetGameText( 453 ), itemName, count );

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
						/// 진행중
						else
						{
							__int64 cnt = check & end;
							for( unsigned int b = 0; b < i; ++b)
								cnt = cnt >> 8;

							cStringT str1;
							str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

							str += str1;
						}

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				case eDUTY_MEET:		/// npc 만나기
					{
						LPCTSTR npcName = GAMERESOURCEMAN->GetNpcName( targetIdx );
						str.Format( GAMERESOURCEMAN->GetGameText( 455 ), npcName );

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				case eDUTY_ACTIVESKILL:	// 스킬발동
					{
						LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( skillIndex );
						LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
						str.Format( GAMERESOURCEMAN->GetGameText( 456 ), skillName, monName );

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				case eDUTY_ENHANCE:
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 457 ) );

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				case eDUTY_PUTCARD:
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 458 ) );

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );

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

						/// 완료
						if( (check & end) == (define->mComplete & end) )
							str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
						/// 진행중
						else
						{
							__int64 cnt = check & end;
							for( unsigned int b = 0; b < i; ++b)
								cnt = cnt >> 8;

							cStringT str1;
							str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

							str += str1;
						}

						mpDescBox->AddNewRow( (LPCTSTR)str.Cstr(), mDefaultColor );
					}
					break;
				}

				/// 완료 체크용
				end = end << 8;
			}
		}
	}

	/// 라인 띄기
	mpDescBox->AddEmptyRow(1);

	/// 반복여부
//	if( define->mRepeatType == eQUEST_REPEAT )
//	{
//		mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 472 ), mDefaultColor );
//		mpDescBox->AddEmptyRow(1);
//	}

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

	mpDescBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 452 ), COLOR_GREEN );

	cUIFuncTextParam textParam;
	textParam.mName = HERO->GetName();
	for( unsigned int i = 0; i < define->mDestLineNum; ++i )
	{
		mpDescBox->AddFucnText( QUESTMAN->GetNpcTextNode( define->mDestIndex + i), &textParam );
	}

	// 튜토리얼 형 퀘스트가 아니면 삭제 가능
	if( define->mType != eQUEST_TYPE_TUTORIAL )
	{
		if( mpDelete )
			mpDelete->SetEnabled( true );
	}
}

void cQuestWindow::SetSelectCount( unsigned int count )
{
	if( mpCount )
	{
		cStringT str;
		str.Format( _T("%d"), count );

		/// 세팅 
		mpCount->SetText( (LPCTSTR)str.Cstr() );
	}
}


void cQuestWindow::UpdateCount( unsigned int progress, unsigned int complete )
{
	if( mpProgressCount && mpCompleteCount )
	{
		cStringT str;
		str.Format( _T("%d"), progress );
		mpProgressCount->SetText( (LPCTSTR)str.Cstr() );

		str.Format( _T("%d"), complete );
		mpCompleteCount->SetText( (LPCTSTR)str.Cstr() );
	}
}

void cQuestWindow::ClearLabels()
{
	for( unsigned int idx = 0; idx < eQUEST_REWARD_LABEL_MAX; ++idx )
	{
		mpRewardLabel[idx]->Hide();
		mIsShowBackImg[idx] = false;
	}
}

void cQuestWindow::SetShowInfo( bool show )
{
	if( mpViewInfo )
	{
		/// press : 켜기, no press : 끄기
		mpViewInfo->SetPress( show );
	}
}

bool cQuestWindow::IsShowInfo()
{
	return ( mpViewInfo ) ? mpViewInfo->IsPress() : false;
}

void cQuestWindow::OnNetLock( int lockTry )
{
	if( mpDelete )
		mpDelete->NetLock( lockTry );
}

void cQuestWindow::OnNetUnLock( int lockTry )
{
	if( mpDelete )
		mpDelete->NetUnLock( lockTry );
}


void cQuestWindow::GetHideMapCategorySet( tHashSet<unsigned int>& mapSet )
{
	if( mpListBox )
		mpListBox->GetClosedCategory( mapSet );
}

void cQuestWindow::SetHideMapCategory( unsigned long mapIndex )
{
	if( mpListBox )
		mpListBox->SetClosedCategory( mapIndex );
}


//-------------------------------------------------------------------------
cQuestListFolder::cQuestListFolder()
: mpOnImage(0)
, mpOnSkin(0)
, mShowOn(false)
{

}

cQuestListFolder::~cQuestListFolder()
{
	SAFE_DELETE( mpOnImage );
}

void cQuestListFolder::Clear()
{
	cListFolderBox::Clear();

	mShowOn = false;
}

bool cQuestListFolder::OnCreate( cUINodeProperty* pproperty )
{
	if( cListFolderBox::OnCreate( pproperty ) == false )
		return false;

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	mpOnSkin = pSkin->GetNodeSkin( "Quest_ChaseOn" );
	if( mpOnSkin && mpOnSkin->mpTexture )
	{
		mpOnImage = new cPlaneObject;

		unsigned short tx = (unsigned short)mpOnSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)mpOnSkin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)mpOnSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)mpOnSkin->mSkinInfo->mTexHeight;
		unsigned int x = GetAbsoluteRect().mLeft + GetAbsoluteRect().GetWidth() - tw;
		unsigned int y = GetAbsoluteRect().mTop;

		if( mpOnImage->Create( mpOnSkin->mpTexture, x, y, tw, th, tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create mpOnSkin image");
			return false;
		}
	}
	else
		return false;
	return true;
}

void cQuestListFolder::UpdateRect()
{
	cListFolderBox::UpdateRect();

	UpdateSelectImage();
}

void cQuestListFolder::OnRender( cUIFontItemKeeper* pKeeper )
{
	cListFolderBox::OnRender( pKeeper );

	if( mpOnImage && mShowOn )
	{
		if( mSelectedColumnIdx >= mTopColumnIdx &&  mSelectedColumnIdx < mTopColumnIdx + mMaxRowInPage && mSelectedRowColumn > 0 )
			mpOnImage->Draw();
	}
}

// 해당 데이터를 가지고있는 컬럼 리턴
// 해당 열이 접혀있으면 리턴 안함
int cQuestListFolder::FindColumnByExtra( unsigned short mapIndex, int arrIndex )
{
	int totalCount = 0;
	for( unsigned int ri = 0; ri < mRows.GetSize(); ++ri )
	{
		sRowData* p = (sRowData*)mRows[ri];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( !data )
			{
				assert(0);
				continue;
			}

			if( data->key != mapIndex )
			{
				if( data->mCheck == true )
					totalCount += p->mColumes.GetSize();
				else
					totalCount += 1;
			}
			else
			{
				// 접혀있으면 리턴
				if( data->mCheck == false )
					return -1;

				/// 동일 데이터 검색
				for( unsigned int ci = 0; ci < p->mColumes.GetSize(); ++ci )
				{
					sFolderData* data = (sFolderData*)p->mColumes[ci];
					if( !data )
					{
						assert(0);
						continue;
					}

					if( ci > 0 && data->mExtraData == (void*)arrIndex )
						return totalCount;

					totalCount++;
				}
				break;
			}
		}
	}
	return -1;
}

void cQuestListFolder::UpdateSelectImage()
{
	if( !mpSelectImage || !mpOnImage )
		return;

	mShowSelect = true;

	int scrollWidth = 0;
	if( mpScrollBar && mpScrollBar->IsVisible() )
		scrollWidth = mpScrollBar->GetAbsoluteRect().GetWidth();

	cUIRect imageRc;
	imageRc.mLeft   = GetAbsoluteRect().mLeft;
	imageRc.mRight  = imageRc.mLeft + GetAbsoluteRect().GetWidth() - scrollWidth;
	imageRc.mTop    = GetAbsoluteRect().mTop + ( mRowHeight * ( mSelectedColumnIdx - mTopColumnIdx) );
	imageRc.mBottom = imageRc.mTop + mRowHeight;

	/// 위치 세팅 
	mpSelectImage->SetScreenXY( (unsigned short)imageRc.mLeft, (unsigned short)imageRc.mTop );
	mpSelectImage->SetScreenWH( (unsigned short)imageRc.GetWidth(), (unsigned short)imageRc.GetHeight() );

	// 추적 이미지 세팅
	if( mpOnSkin )
	{
		int tw = (short)mpOnSkin->mSkinInfo->mTexWidth;
		int th = (short)mpOnSkin->mSkinInfo->mTexHeight;
		int x = imageRc.mRight - tw;
		int y = (int)(imageRc.mTop + (int)(mRowHeight - th) * 0.5f);
		mpOnImage->SetScreenXY( x, y );
	}
}
