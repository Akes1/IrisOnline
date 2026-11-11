#include "stdafx.h"
#include "NpcDisjointWindow.h"
#include "ItemManager.h"
#include "Gauge.h"
#include "Label.h"
#include "NumberEditBox.h"
#include "Button.h"
#include "ItemIcon.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "Hero.h"

cNpcDisjointWindow::cNpcDisjointWindow()
: mpGaugeBar(0)
, mpPrice(0)
, mpDisjointButton(0)
, mItemIcon(0)
{
	for( int i = 0; i < DISJOINT_COUNT; ++i )
		mResultIcon[i] = 0;
}

cNpcDisjointWindow::~cNpcDisjointWindow()
{
}

/// 초기화
void cNpcDisjointWindow::Clear()
{
	/// 아이템 락 초기화
	unsigned short slotIndex = mItemIcon->GetSlotIndex();
	if( slotIndex <= MAX_INVENTORY )
	{
		cItem* oldItem = ITEMMAN->GetItem( slotIndex );
		if( oldItem )
		{
			oldItem->SetLock( false );
		}
		mItemIcon->SetSlotIndex( USHRT_MAX );
	}

	/// 아이콘 초기화
	mItemIcon->ChangeImage( 0 );

	for( unsigned int i = 0; i < DISJOINT_COUNT; ++i )
	{
		mResultIcon[i]->ChangeImage( 0 );
		mResultIcon[i]->SetItemIndex( 0 );
		mResultIcon[i]->ShowNumber( false );
		mResultIcon[i]->SetNumber( 0 );
	}

	if( mpGaugeBar )
		mpGaugeBar->ClearFront();

	SetPrice( 0 );

	///
	NetUnLock();

	if( mpDisjointButton )
		mpDisjointButton->SetEnabled( false );

	/// 아이템 분해 사운드 재생 취소하기.
	HERO->ClearItemDisjointSound();
}

void cNpcDisjointWindow::Open()
{
}

void cNpcDisjointWindow::Close()
{
	cUIWindow::Close();

	HERO->CloseNpcTalk( false );
	Clear();
}

void cNpcDisjointWindow::OnHide()
{
	cUIWindow::OnHide();

	HERO->CloseNpcTalk();
	Clear();
}

bool cNpcDisjointWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpGaugeBar = (cGauge*)GetChild( eUIID_GAME_NPCDISJOINT_GAUGEBAR );
	mpPrice	   = (cLabel*)GetChild( eUIID_GAME_NPCDISJOINT_PRICE );
	mpDisjointButton = (cButton*)GetChild( eUIID_GAME_NPCDISJOINT_DISJOINT );

	if( mpDisjointButton )
		mpDisjointButton->SetEnabled( false );

	if( mpGaugeBar )
		mpGaugeBar->SetMaxValue( 100 );

	/// 분해 결과 아이콘 생성
	cString str;
	for( unsigned int i = 0; i < DISJOINT_COUNT; ++i )
	{
		str.Format( "Disjoint_Result%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, i+1 ) == false )
			return false;

		icon->SetUsedFlag( false, false );
		mResultIcon[i] = icon;
	}

	/// 분해할 아이템 아이콘 생성
	mItemIcon = new cItemIcon;
	if( mItemIcon->CreateBySkinName( "Disjoint_Item", this, 100 ) == false )
		return false;

	/// hover 이벤트는 처리하지만 이동 불가 
	mItemIcon->SetUsedFlag( false, false );

	//
	SetPrice(0);
	return true;
}

void cNpcDisjointWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_NPCDISJOINT_CLOSE:
		{
			Hide();

			/// 아이템 분해 사운드 재생 취소하기.
			HERO->ClearItemDisjointSound();
		}
		break;
	case eUIID_GAME_NPCDISJOINT_CANCEL:
		if( mpGaugeBar && mpGaugeBar->IsProgress() == true )
		{
			if( mpGaugeBar )
				mpGaugeBar->ClearFront();

			if( mpDisjointButton )
				mpDisjointButton->SetEnabled( true );

			HERO->ClearItemDisjointSound();
		}
		else
		{
			Hide();
		}
		break;
	case eUIID_GAME_NPCDISJOINT_DISJOINT:
		if( mItemIcon->GetSlotIndex() <= MAX_INVENTORY )
		{
			unsigned long state = HERO->GetState();
			eSTOPFLAG stopFlag = HERO->GetStopFlag();
			if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_NPCSPEECH ) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
				return;
			}

			/// 분해 조건검사
			if( ITEMMAN->CheckDisjoint( mItemIcon->GetSlotIndex() ) == false )
				return;

			if( mpGaugeBar )
			{
				float speed = 100.0f / ((float)DISJOINT_DELAYTIME*0.001f);

				mpGaugeBar->SetGaugeSpeed( speed );
				mpGaugeBar->SetGaugeAni( 100.0f );
			}

			/// 아이템 분해 사운드 재생
			HERO->SetItemDisjointSound();

			if( mpDisjointButton )
				mpDisjointButton->SetEnabled( false );

			/// 락 걸기
			NetLock();
		}
		break;
	}
}

void cNpcDisjointWindow::OnGaugeEnd( cUINode* n , unsigned int )
{
	if( mpGaugeBar && n == mpGaugeBar )
	{
		/// 분해시작버튼
		if( ITEMMAN->TryToDisjoint( mItemIcon->GetSlotIndex() ) == false )
		{
			NetUnLock();

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 261 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);

			if( mpGaugeBar )
				mpGaugeBar->ClearFront();
		}

		UIMAN->HideTip();
	}
}

/// 분해 아이템 등록
bool cNpcDisjointWindow::AddItem( unsigned short slotIndex )
{
	/// 이미 분해중이면 리턴
	if( mpDisjointButton->IsNetLock() == true )
		return false;

	if( slotIndex > MAX_INVENTORY )
		return false;

	/// 제한 슬롯 체크
	unsigned short bagEnd = HERO->GetBagEndSlot();
	if ( slotIndex > bagEnd )
	{
		assert( 0 && "Bad slot index" );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 3101 ) );
		return false;
	}

	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( !item )
	{
		assert(0);
		return false;
	}

	cItemDefine* define = item->GetDefine();
	if( !define )
	{
		assert(0);
		return false;
	}

	sItemDisjoint* data = define->GetDisjoint();
	if( !data )
	{
		UIMAN->CreateMsgBox(eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 299 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		return false;
	}

	/// 미인증된 아이템이면 강화 불가능
	if( item->IsSeal() == true )
	{
		UIMAN->CreateMsgBox(eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 293 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		return false;
	}

	/// 결과 정보 초기화
	for( unsigned int i = 0; i < DISJOINT_COUNT; ++i )
	{
		mResultIcon[i]->ChangeImage( 0 );
		mResultIcon[i]->SetTipType( eTOOLTIP_NONE );
		mResultIcon[i]->SetNumber( 0 );
		mResultIcon[i]->SetItemIndex( 0 );
		mResultIcon[i]->ShowNumber( false );
	}

	/// 기존 등록 아이템 삭제
	unsigned short itemSlotIndex = mItemIcon->GetSlotIndex();
	if( itemSlotIndex <= MAX_INVENTORY && itemSlotIndex != slotIndex )
	{
		cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
		if( oldItem )
		{
			oldItem->SetLock( false );
		}
	}

	/// 참조 슬롯 인덱스
	mItemIcon->SetSlotIndex( slotIndex );
	item->SetLock( true );

	/// 아이템 이미지 체인지
	mItemIcon->SetItemIndex( item->GetIndex() );
	mItemIcon->ChangeImage( define->GetIconParam() );
	mItemIcon->SetTipType( define->GetTipType() );

	/// 분해 결과 아이템 정보 등록
	int result = 0;
	for( int i = 0 ; i < MAX_DISJOINT; ++i, ++result )
	{
		if( data->mDisjoint[i].itemIndex == 0 )
			break;

		cItemDefine* resultDefine = ITEMMAN->GetItemDefine( data->mDisjoint[i].itemIndex );
		if( !resultDefine )
		{
			assert(0);
			return false;
		}

		/// 정보 세팅
		mResultIcon[result]->SetItemIndex( data->mDisjoint[i].itemIndex );
		mResultIcon[result]->ChangeImage( resultDefine->GetIconParam() );
		mResultIcon[result]->SetNumber( data->mDisjoint[i].count );
		mResultIcon[result]->SetTipType( resultDefine->GetTipType() );

		bool show = ( resultDefine->GetCapacity() > 1 ) ? true : false;
		mResultIcon[result]->ShowNumber( show );
	}

	SetPrice( data->mPrice );

	/// 아이템 등록 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slotIndex );

	if( mpDisjointButton )
		mpDisjointButton->SetEnabled( true );

	return true;
}

void cNpcDisjointWindow::SetPrice( unsigned int price )
{
	if( mpPrice )
	{
		LPCTSTR value = cNumberEditBox::FormatNumber( price );
		mpPrice->SetText( value );
	}
}

///
void cNpcDisjointWindow::OnNetLock( int lockTry )
{
	if( mpDisjointButton )
		mpDisjointButton->NetLock( lockTry );
}

void cNpcDisjointWindow::OnNetUnLock( int lockTry )
{
	if( mpDisjointButton )
		mpDisjointButton->NetUnLock( lockTry );
}

unsigned short cNpcDisjointWindow::GetSlotIndex()
{
	return ( mItemIcon ) ? mItemIcon->GetSlotIndex() : USHRT_MAX;
}