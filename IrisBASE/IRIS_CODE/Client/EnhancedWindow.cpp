#include "StdAfx.h"
#include "EnhancedWindow.h"
#include "ItemManager.h"
#include "Button.h"
#include "GameUIManager.h"
#include "Gauge.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "SoundSystem.h"
#include "DuelManager.h"

cEnhancedWindow::cEnhancedWindow()
: mpStart(0)
, mpCancel(0)
, mpGaugeBar(0)
, mItemSlot(0)
, mCardSlot(0)
, mCashProtectCardSlot(0)
, mpSuccess(0)
, mpFailure(0)
, mIsEnhanceNow( false )
{

}

cEnhancedWindow::~cEnhancedWindow()
{
}

void cEnhancedWindow::Clear()
{
	/// 아이템 클리어
	ClearItemSlot();

	/// 카드 클리어
	ClearCardSlot();

	/// 캐시아이템 강화 보호 카드 클리어
	ClearCahProtectCardSlot();

	///
	StopEnhanced();
}

void cEnhancedWindow::ClearItemSlot()
{
	if( mItemSlot )
	{
		/// 아이템 락 초기화
		unsigned short itemSlotIndex = mItemSlot->GetSlotIndex();
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
			{
				oldItem->SetLock( false );
			}

			mItemSlot->SetSlotIndex( USHRT_MAX );
		}

		/// 아이템 클리어
		mItemSlot->ChangeImage( 0 );
		mItemSlot->SetTipType( eTOOLTIP_NONE );
		mItemSlot->SetItemIndex( 0 );
	}

	StopEnhanced();

	mpStart->SetEnabled( false );
	mpCancel->SetEnabled( false );
}

void cEnhancedWindow::ClearCardSlot()
{
	if( mCardSlot )
	{
		unsigned short cardSlotIndex = mCardSlot->GetSlotIndex();
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldCard = ITEMMAN->GetItem( cardSlotIndex );
			if( oldCard )
			{
				oldCard->SetLock( false );
			}

			mCardSlot->SetSlotIndex( USHRT_MAX );
		}

		mCardSlot->ChangeImage( 0 );
		mCardSlot->SetTipType( eTOOLTIP_NONE );
		mCardSlot->SetItemIndex( 0 );
	}

	mpStart->SetEnabled( false );
	mpCancel->SetEnabled( false );
}

void cEnhancedWindow::ClearCahProtectCardSlot()
{
	if( mCashProtectCardSlot )
	{
		unsigned short cardSlotIndex = mCashProtectCardSlot->GetSlotIndex();
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldCard = ITEMMAN->GetItem( cardSlotIndex );
			if( oldCard )
			{
				oldCard->SetLock( false );
			}

			mCashProtectCardSlot->SetSlotIndex( USHRT_MAX );
		}

		mCashProtectCardSlot->ChangeImage( 0 );
		mCashProtectCardSlot->SetTipType( eTOOLTIP_NONE );
		mCashProtectCardSlot->SetItemIndex( 0 );
	}
}

void cEnhancedWindow::Open()
{
}

void cEnhancedWindow::Close()
{
	cUIWindow::Close();

	Clear();

	/// 결과 처리
	HideResult();
}

void cEnhancedWindow::Show( bool onsound )
{
	if( HERO->GetStopFlag() != eSTOP_ITEMMIX )
		cUIWindow::Show( onsound );
}

void cEnhancedWindow::OnShow()
{
	cUIWindow::OnShow();

	GAMEUI->HideItemSetWindow( this );
	
	if( mpStart )
		mpStart->SetEnabled( false );
	
 	if( mpCancel )
 		mpCancel->SetEnabled( false );
}

void cEnhancedWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();

	/// 결과 처리
	HideResult();
}

bool cEnhancedWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpStart = (cButton*)GetChild( eUIID_GAME_ENHANCED_START	);
	mpCancel = (cButton*)GetChild( eUIID_GAME_ENHANCED_CANCEL );
	mpGaugeBar = (cGauge*)GetChild( eUIID_GAME_ENHANCED_GAUGEBAR );
	mpSuccess = GetChild( eUIID_GAME_ENHANCED_SUCCESS );
	mpFailure = GetChild( eUIID_GAME_ENHANCED_FAILURE );
	

	if( mpGaugeBar )
		mpGaugeBar->SetMaxValue( 100 );

	/// 결과 처리
	HideResult();

	/// 아이템 아이콘 생성
	mItemSlot = new cItemIcon;
	if( mItemSlot->CreateBySkinName( "Enhanced_ItemSlot", this, ITEM_ENHANCE_SLOT ) == false )
		return false;

	/// 강화 카드 아이콘 생성
	mCardSlot = new cItemIcon;
	if( mCardSlot->CreateBySkinName( "Enhanced_CardSlot", this, CARD_ENHANCE_SLOT ) == false )
		return false;

	/// 강화 보호 보조제 아이콘 생성
	mCashProtectCardSlot = new cItemIcon;
	if( mCashProtectCardSlot->CreateBySkinName( "Enhanced_AssistanceSlot", this, CASHITEM_ENHANCE_SLOT ) == false )
		return false;

	/// hover 이벤트는 처리하지만 이동 불가 
	mItemSlot->SetUsedFlag( false, false );
	mCardSlot->SetUsedFlag( false, false );
	mCashProtectCardSlot->SetUsedFlag( true, false );

	///
	StopEnhanced();
	return true;
}

void cEnhancedWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	cUIWindow::OnRender( pKeeper );
}

void cEnhancedWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_ENHANCED_CLOSE:
		{
			 ITEMMAN->TryToEnhancedCancel();
		}
		break;
	case eUIID_GAME_ENHANCED_CANCEL:
		if( HERO->GetStopFlag() == eSTOP_ENHANCED )
		{
			ITEMMAN->TryToEnhancedCancel( false );
		}
		break;
	case eUIID_GAME_ENHANCED_START:
		if( mItemSlot && mCardSlot && mCashProtectCardSlot )
		{
			if( mItemSlot->GetSlotIndex() <= MAX_INVENTORY && mCardSlot->GetSlotIndex() <= MAX_INVENTORY )
			{
				if( mCashProtectCardSlot->GetSlotIndex() <= MAX_INVENTORY )	//캐시 아이템 사용한다면
				{
					ITEMMAN->TryToEnhancedStart( mItemSlot->GetSlotIndex(), mCardSlot->GetSlotIndex(), mCashProtectCardSlot->GetSlotIndex() );
				}
				else
				{
					ITEMMAN->TryToEnhancedStart( mItemSlot->GetSlotIndex(), mCardSlot->GetSlotIndex() );
				}

				/// 결과창 지우기
				HideResult();
			}
		}
		break;
	}
}

void cEnhancedWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	switch( id )
	{
	case ITEM_ENHANCE_SLOT:
		break;

	case CARD_ENHANCE_SLOT:
		break;

	// 캐쉬 아이템 보조제
	case CASHITEM_ENHANCE_SLOT:
		{
			ClearCahProtectCardSlot();
		}
		break;
	}
}

bool cEnhancedWindow::AddItem( unsigned short slotIndex, bool onSound, bool autoAdd  )
{
 	if( mpStart->IsNetLock() == true || mIsEnhanceNow == true )
 		return false;

	if( slotIndex > MAX_INVENTORY )
		return false;

	/// 결과 숨기기
	HideResult();

	if( DUELMAN->IsDuelEnable() == true )
	{
		return false;
	}
	
	/// 강화 아이템 삽입
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

	cItemLimit* pItemLimit		= define->GetLimit();
	if( pItemLimit  == NULL )
	{
		assert(0);
		return false;
	}

	if( !mItemSlot )
	{
		assert(0);
		return false;
	}

	/// 무기, 방어구 타입 아니면 리턴
	if( !(define->GetType() == ITEM_WEAPON || define->GetType() == ITEM_WEAR) )
		return false;

	/// 강화단계 검사 ( 최대 강화넘으면 리턴 )
	if( item->GetEnhanceStep() == MAX_ITEM_ENHANCED )
		return false;

	// 강화인덱스가 없으면 리턴
	if( define->GetEnhancedOptionArr() == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 207 ) );
		return false;
	}

	/// 미인증된 아이템이면 강화 불가능
	if( item->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	unsigned short itemSlotIndex = mItemSlot->GetSlotIndex();
	if( itemSlotIndex != slotIndex )
	{
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
				oldItem->SetLock( false );
		}

		/// 참조 슬롯 인덱스
		mItemSlot->SetSlotIndex( slotIndex );
		item->SetLock( true );

		// 기존의 적재된 슬롯의 아이템의 타입이 바뀌면 그에 따라 카드와 보조제가 있다면 강화창에서 제거해야 한다.
		
		// 강화 카드 슬롯에 카드가 적재되어 있는지
		unsigned char cardSubType = 0;
		if( mCardSlot->GetSlotIndex() <= MAX_INVENTORY )
		{
			unsigned int cardItemIndex		= mCardSlot->GetItemIndex();
			cItemDefine* cardDefine			= ITEMMAN->GetItemDefine( cardItemIndex );
			unsigned char itemLevel			= pItemLimit->GetLevel();

			if( cardDefine == NULL )
			{
				assert(0);
				return false;
			}

			cardSubType = cardDefine->GetSubType();			

			// 바뀐 아이템( 무기 -> 옷 또는 옷 -> 무기 ) 기존에 슬롯에 장착된 강화카드 타입과 맞지 않다면!! 
			if( define->GetType() == ITEM_WEAPON  )
			{		
				/// 아이템 타입과 카드 타입이 맞지만 아이템 레벨과 카드 사용 레벨 제한이 맞지 않다면
				if( cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV1 || cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV2 )
				{
					if( !(1 <= itemLevel && itemLevel <= 50) )
					{
						ClearCardSlot();
					}
				}
				else if( cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV1_75 || cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV2_75 )
				{
					if( !(51 <= itemLevel && itemLevel <= 75) )
					{
						ClearCardSlot();
					}
				}
				/// 아이템 타입과 카드 타입이 맞지 않으면
				else
				{
					ClearCardSlot();
				}
			}
			else if( define->GetType() == ITEM_WEAR )
			{
				if( cardSubType == ITEM_CARD_ENHANCE_WEAR_LV1 || cardSubType == ITEM_CARD_ENHANCE_WEAR_LV2 )
				{
					if( !(1 <= itemLevel && itemLevel <= 50) )
					{
						ClearCardSlot();
					}
				}
				else if( cardSubType == ITEM_CARD_ENHANCE_WEAR_LV1_75 || cardSubType == ITEM_CARD_ENHANCE_WEAR_LV2_75 )
				{
					if( !(51 <= itemLevel && itemLevel <= 75) )
					{
						ClearCardSlot();
					}
				}
				/// 아이템 타입과 카드 타입이 맞지 않으면
				else
				{
					ClearCardSlot();
				}
			}
		}

		// 캐쉬 아이템 강화 보호 슬롯에 보조제가 적재되어 있는지
		unsigned char cashSubType = 0;
		if( mCashProtectCardSlot->GetSlotIndex() <= MAX_INVENTORY )
		{
			unsigned int cashItemIndex		= mCashProtectCardSlot->GetItemIndex();
			cItemDefine* cashDefine = ITEMMAN->GetItemDefine( cashItemIndex );

			if( cashDefine == NULL )
			{
				assert(0);
				return false;
			}
			cashSubType = cashDefine->GetSubType();
			
			//적재된 보조제가 아이템이 바뀌면서 아이템 레벨 제한에 맞지 않다면 슬롯에서 제거!
			unsigned char itemLevel = pItemLimit->GetLevel();
			if( cashSubType != 0 )
			{
				if( cashSubType == ITEM_MATERIAL_ENHANCED_DOWN_30 || cashSubType == ITEM_MATERIAL_ENHANCED_DESTROY_30 )
				{
					if( !(1 <= itemLevel && itemLevel <= 30) )
						ClearCahProtectCardSlot();	
				}
				else
				{
					if( !(31 <= itemLevel && itemLevel <= 60) )
						ClearCahProtectCardSlot();	
				}			
			}
		}
	}

	if( autoAdd )
	{
		// 자동 등록 
		unsigned char cardSubType		= UCHAR_MAX;
		unsigned short cardSlotIndex	= mCardSlot->GetSlotIndex();
		unsigned int cardItemIndex		= mCardSlot->GetItemIndex();
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			cItem* card = ITEMMAN->GetItem( cardSlotIndex );
			if( !card )
			{
				assert(0);
				return false;
			}

			/// 연결된 갯수 검사
			if( card->GetCount() > 0 )
			{
				cItemDefine* cardDefine = card->GetDefine();
				if( cardDefine )
					cardSubType = cardDefine->GetSubType();
			}
			else
			{
				cItemDefine* cardDefine = ITEMMAN->GetItemDefine( cardItemIndex );
				if( cardDefine == NULL )
				{
					assert(0);
					return false;
				}

				cardSubType = cardDefine->GetSubType();
			}
			
			unsigned short cardslotIdx = ITEMMAN->FindSlotIndexByType( ITEM_CARD, (ITEM_TYPE)cardSubType );
			if( cardslotIdx != USHRT_MAX )
			{
				cItem* cardItem = ITEMMAN->GetItem( cardslotIdx );
				if( !cardItem )
				{
					if( item->IsLock() ) item->SetLock( false );
					assert(0);
					return false;
				}

				int ret = ITEMMAN->CheckLimit( cardItem->GetIndex() );
				if( ret != ITEMLIMIT_SUCCESS  )
				{
					if( item->IsLock() ) 
						item->SetLock( false );

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 196 ) );
					return false;
				}

				if( AddCard( cardslotIdx ) == false )
					ClearCardSlot();	
			}
			else
			{
				ClearCardSlot();
			}
		}
		else
		{
			ClearCardSlot();
		}

		/// ===================  캐쉬 아이템 강화 보호 주문서 ======================
		unsigned char cashSubType = UCHAR_MAX;
		unsigned short cashSlotIndex	= mCashProtectCardSlot->GetSlotIndex();
		unsigned int cashtemIndex	= mCashProtectCardSlot->GetItemIndex();
		if( cashSlotIndex <= MAX_INVENTORY )
		{
			cItem* card = ITEMMAN->GetItem( cashSlotIndex );
			if( !card )
			{
				assert(0);
				return false;
			}

			/// 연결된 갯수 검사
			if( card->GetCount() > 0 )
			{
				cItemDefine* cashDefine = card->GetDefine();
				if( cashDefine == NULL )
				{
					assert(0);
					return false;
				}

				cashSubType = cashDefine->GetSubType();
			}
			else
			{
				cItemDefine* cashDefine = ITEMMAN->GetItemDefine( cashtemIndex );
				if( cashDefine == NULL )
				{
					assert(0);
					return false;
				}

				cashSubType = cashDefine->GetSubType();
			}		
		
			unsigned short cashSlotIdx = ITEMMAN->FindSlotIndexByType( ITEM_MATERIAL, (ITEM_TYPE)cashSubType );

			if( cashSlotIdx != USHRT_MAX )
			{
				cItem* cashItem = ITEMMAN->GetItem( cashSlotIdx );
				if( !cashItem )
				{
					if( item->IsLock() ) item->SetLock( false );
					assert(0);
					return false;
				}

				int ret = ITEMMAN->CheckLimit( cashItem->GetIndex() );
				if( ret != ITEMLIMIT_SUCCESS )
				{					
					if( item->IsLock() ) 
						item->SetLock( false );

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12103 ) );
					return false;
				}

				if( AddCashProtectCard( cashSlotIdx ) == false )
					ClearCahProtectCardSlot();
			}
			else
			{
				ClearCahProtectCardSlot();
			}
		}
		else
		{
			ClearCahProtectCardSlot();
		}		
	}

	if( mCardSlot->GetItemIndex() != 0)
	{
		mpStart->SetEnabled( true );
		mpCancel->SetEnabled( false );
	}	

	/// 아이템 등록
	mItemSlot->SetItemIndex( item->GetIndex() );
	mItemSlot->ChangeImage( define->GetIconParam() );
	mItemSlot->SetTipType( define->GetTipType() );

	if( onSound )
	{
		/// 아이템 추가 시 사운드 재생
		ITEMMAN->PlayIconSettingSound( slotIndex );
	}

	return true;
}

///
bool cEnhancedWindow::AddCard( unsigned short slotIndex, bool onSound )
{
	if( mpStart->IsNetLock() == true || mIsEnhanceNow == true )
 		return false;

	if( slotIndex > MAX_INVENTORY )
		return false;

	/// 결과 숨기기
	HideResult();

	if( DUELMAN->IsDuelEnable() == true )
	{
		return false;
	}

	/// 강화 카드 삽입	
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( !item )
	{
		assert(0);
		return false;
	}

	/// 미인증된 카드면 강화 불가능
	if( item->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	cItemDefine* cardDefine = item->GetDefine();
	if( !cardDefine )
	{
		assert(0);
		return false;
	}

	cItemLimit*	cardLimit = cardDefine->GetLimit();
	if( !cardLimit )
	{
		assert(0);
		return false;
	}

	/// 강화카드가 아니면 리턴
	if( !( cardDefine->GetType() == ITEM_CARD && (cardDefine->GetSubType() >= ITEM_CARD_ENHANCE_WEAPON_LV1 && 
												  cardDefine->GetSubType() <= ITEM_CARD_ENHANCE_WEAR_LV2_75) ) )
	{
		return false;
	}

	/// 아이템이 채워져있을시, 무기, 방어구 분리
	unsigned short itemSlotIndex = mItemSlot->GetSlotIndex();
	if( itemSlotIndex <= MAX_INVENTORY )
	{
		cItem* tempItem = ITEMMAN->GetItem( itemSlotIndex );
		if( !tempItem )
		{
			assert(0);
			return false;
		}

		cItemDefine* define = tempItem->GetDefine();
		if( !define )
		{
			assert(0);
			return false;
		}

		cItemLimit* itemLimit = define->GetLimit();
		if( !itemLimit )
		{
			assert(0);
			return false;
		}

		/// 무기 + 무기강화카드 아니면 리턴
		char cardSubType = cardDefine->GetSubType();
		if( define->GetType() == ITEM_WEAPON )
		{
			unsigned char itemLevel = itemLimit->GetLevel();			
			if( cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV1 || cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV2 )
			{
				if( !(1 <= itemLevel && itemLevel <= 50) )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12500 ) );
					return false;
				}
				else
				{
					mpStart->SetEnabled( true );
					mpCancel->SetEnabled( false );
				}
			}
			else if( cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV1_75 || cardSubType == ITEM_CARD_ENHANCE_WEAPON_LV2_75 )
			{
				if( !(51 <= itemLevel && itemLevel <= 75) )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12500 ) );
					return false;
				}
				else
				{
					mpStart->SetEnabled( true );
					mpCancel->SetEnabled( false );
				}
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 262 ) );
				return false;				
			}
		}
		else if( define->GetType() == ITEM_WEAR )
		{			
			unsigned char itemLevel = itemLimit->GetLevel();
			if( cardSubType == ITEM_CARD_ENHANCE_WEAR_LV1 || cardSubType == ITEM_CARD_ENHANCE_WEAR_LV2 )
			{
				if( !(1 <= itemLevel && itemLevel <= 50) )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12500 ) );
					return false;
				}
				else
				{
					mpStart->SetEnabled( true );
					mpCancel->SetEnabled( false );
				}
			}
			else if( cardSubType == ITEM_CARD_ENHANCE_WEAR_LV1_75 || cardSubType == ITEM_CARD_ENHANCE_WEAR_LV2_75 )
			{
				if( !(51 <= itemLevel && itemLevel <= 75) )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12500 ) );
					return false;
				}
				else
				{
					mpStart->SetEnabled( true );
					mpCancel->SetEnabled( false );
				}
			}			
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 262 ) );
				return false;			
			}
		}
	}
	
	/// 카드 아이템 이미지 체인지
	if( mCardSlot )
	{
		sItemCount* p = ITEMMAN->GetItemCount( item->GetIndex() );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count > 0 )
		{
			mCardSlot->SetItemIndex( item->GetIndex() );
			mCardSlot->ChangeImage( cardDefine->GetIconParam() );
			mCardSlot->SetTipType( cardDefine->GetTipType() );
			mCardSlot->SetNumber(count);
			mCardSlot->ShowNumber(true);

			int ret0 = ITEMMAN->CheckLimit( mCardSlot->GetItemIndex() );
			int ret1 = ITEMMAN->CheckLimitEtc( mCardSlot->GetItemIndex() );

			NiColorA color = NiColorA::WHITE;
			if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
			{
				return false;
				//color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
			mCardSlot->SetColor( color );
		}
		else
		{
			assert(0);
			return false;
		}
	}
	else
		assert(0);

	/// 
	unsigned short cardSlotIndex = mCardSlot->GetSlotIndex();
	if( cardSlotIndex != slotIndex )
	{
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldCard = ITEMMAN->GetItem( cardSlotIndex );
			if( oldCard )
				oldCard->SetLock( false );
		}

		/// 참조 슬롯 인덱스
		mCardSlot->SetSlotIndex( slotIndex );
		item->SetLock( true );
	}

	if( onSound )
	{
		/// 카드 추가 시 사운드 재생
		SOUNDSYS->Play2DSound( 3035 );
	}

	return true;
}

/// 캐시 아이템 강화 보호 카드 삽입
bool cEnhancedWindow::AddCashProtectCard( unsigned short slotIndex, bool onSound )		
{
	if( mpStart->IsNetLock() == true || mIsEnhanceNow == true )
		return false;

	if( slotIndex > MAX_INVENTORY )
		return false;

	/// 결과 숨기기
	HideResult();

	if( DUELMAN->IsDuelEnable() == true )
	{
		return false;
	}

	/// 강화 보호 카드 삽입	
	cItem* protectitem = ITEMMAN->GetItem( slotIndex );
	if( !protectitem )
	{
		assert(0);
		return false;
	}

	cItemDefine* protectDefine = protectitem->GetDefine();
	if( !protectDefine )
	{
		assert(0);
		return false;
	}

	if( mItemSlot->GetItemIndex() == 0 )	// 강화창에 아이템이 적재되지 않은 경우
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12101 ) );
		return false;
	}
	
	/// 미인증된 카드면 강화 불가능
	if( protectitem->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	/// 캐쉬 아이템 강화 보호 카드 아니라면
	if( !( protectDefine->GetType() == ITEM_MATERIAL && (protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_30 || 
														 protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_60 || 
														 protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DESTROY_30 ||
														 protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DESTROY_60 ) ) )
	{
		return false;
	}

	///==========아이템 레벨과 보조제의 아이템 레벨 제한 범위와 비교
	cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( mItemSlot->GetItemIndex() );
	if( pItemDefine == NULL )	
	{
		assert(0);
		return false;
	}

	cItemLimit* pItemLimit		= pItemDefine->GetLimit();
	if( pItemLimit  == NULL )
	{
		assert(0);
		return false;
	}

	unsigned char itemLevel = pItemLimit->GetLevel();
	if( protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_30 || protectDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DESTROY_30 )
	{
		if( !(1 <= itemLevel && itemLevel <= 30) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12100 ) );
			return false;
		}
	}
	else
	{
		if( !(31 <= itemLevel && itemLevel <= 60) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12100 ) );
			return false;
		}
	}
	
	/// 카드 아이템 이미지 체인지
	if( mCashProtectCardSlot )
	{
		sItemCount* p = ITEMMAN->GetItemCount( protectitem->GetIndex() );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count > 0 )
		{

			mCashProtectCardSlot->SetItemIndex( protectitem->GetIndex() );
			mCashProtectCardSlot->ChangeImage( protectDefine->GetIconParam() );
			mCashProtectCardSlot->SetTipType( protectDefine->GetTipType() );
			mCashProtectCardSlot->SetNumber(count);
			mCashProtectCardSlot->ShowNumber(true);		

			int ret0 = ITEMMAN->CheckLimit( mCashProtectCardSlot->GetItemIndex() );
			int ret1 = ITEMMAN->CheckLimitEtc( mCashProtectCardSlot->GetItemIndex() );

			NiColorA color = NiColorA::WHITE;
			if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
			{
				return false;
				//color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
			mCashProtectCardSlot->SetColor( color );
		}
		else
		{
			assert(0);
			return false;
		}
	}
	else
		assert(0);

	/// 
	unsigned short cardSlotIndex = mCashProtectCardSlot->GetSlotIndex();
	if( cardSlotIndex != slotIndex )
	{
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldCard = ITEMMAN->GetItem( cardSlotIndex );
			if( oldCard )
				oldCard->SetLock( false );
		}

		/// 참조 슬롯 인덱스
		mCashProtectCardSlot->SetSlotIndex( slotIndex );
//		protectitem->SetLock( true );
	}

	if( onSound )
	{
		/// 카드 추가 시 사운드 재생
		SOUNDSYS->Play2DSound( 3035 );
	}

	return true;
}

void cEnhancedWindow::UpdateCardSlot()
{
	if( mCardSlot && mCardSlot->GetItemIndex() )
	{
		cItem* item = ITEMMAN->GetItem( mCardSlot->GetSlotIndex() );
		if( item->GetDefine() == NULL )
		{
			cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( mCardSlot->GetItemIndex() );
			if( pItemDefine )
			{
				/// 동일한 카드가 있을 경우에만 자동 추가한다.
				unsigned char findSubType = pItemDefine->GetSubType();
				unsigned short slotIdx = ITEMMAN->FindSlotIndexByType( ITEM_CARD, (ITEM_TYPE)findSubType );
				if( slotIdx != USHRT_MAX )
				{
					AddCard( slotIdx );
				}
				else 
					ClearCardSlot();
			}
			else
				ClearCardSlot();
		}
	}
}


void cEnhancedWindow::UpdateCashProtectSlot()
{
	if( mCashProtectCardSlot && mCashProtectCardSlot->GetItemIndex() )
	{
		cItem* item = ITEMMAN->GetItem( mCashProtectCardSlot->GetSlotIndex() );

		if( item->GetDefine() == NULL )
		{
			cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( mCashProtectCardSlot->GetItemIndex() );
			if( pItemDefine && mItemSlot)
			{
				///==========아이템 레벨과 보조제의 아이템 레벨 제한 범위와 비교
				unsigned short itemSlotIndex = mItemSlot->GetSlotIndex();
				if( itemSlotIndex <= MAX_INVENTORY )
				{
					cItem* tempItem = ITEMMAN->GetItem( itemSlotIndex );
					if( !tempItem )
					{
						assert(0);
						return ;
					}

					cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( mItemSlot->GetItemIndex() );
					if( pItemDefine == NULL )	
					{
						assert(0);
						return ;
					}

					cItemLimit* pItemLimit		= pItemDefine->GetLimit();
					if( pItemLimit  == NULL )
					{
						assert(0);
						return ;
					}

					unsigned char itemLevel = pItemLimit->GetLevel();
					unsigned short slotIdx = 0;
					
					if( 1 <= itemLevel && itemLevel <= 30 )
					{
						slotIdx = ITEMMAN->FindSlotIndexByType( ITEM_MATERIAL, ITEM_MATERIAL_ENHANCED_DOWN_30 );
						if( slotIdx == USHRT_MAX )
							slotIdx = ITEMMAN->FindSlotIndexByType( ITEM_MATERIAL, ITEM_MATERIAL_ENHANCED_DESTROY_30 );
					}
					else if( 31 <= itemLevel && itemLevel <= 60 )
					{
						slotIdx = ITEMMAN->FindSlotIndexByType( ITEM_MATERIAL, ITEM_MATERIAL_ENHANCED_DOWN_60 );
						if( slotIdx == USHRT_MAX )
							slotIdx = ITEMMAN->FindSlotIndexByType( ITEM_MATERIAL, ITEM_MATERIAL_ENHANCED_DESTROY_60 );
					}
					
					if( slotIdx != USHRT_MAX )
					{
						AddCashProtectCard( slotIdx );
					}
					else 
					{
						ClearCahProtectCardSlot();
					}
				}
			}
			else
			{
				ClearCahProtectCardSlot();
			}
		}
	}
}

void cEnhancedWindow::UpdateCashProtectCount()
{
	if( mCashProtectCardSlot )
	{
		unsigned short cardSlotIndex = mCashProtectCardSlot->GetSlotIndex();
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mCashProtectCardSlot->GetItemIndex() );
			unsigned int count = ( p ) ? p->bag : 0;
			if( count > 0 )
			{
				mCashProtectCardSlot->SetNumber(count);
				mCashProtectCardSlot->ShowNumber(true);

				int ret0 = ITEMMAN->CheckLimit( mCashProtectCardSlot->GetItemIndex() );
				int ret1 = ITEMMAN->CheckLimitEtc( mCashProtectCardSlot->GetItemIndex() );

				NiColorA color = NiColorA::WHITE;
				if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
				{
					color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
				mCashProtectCardSlot->SetColor( color );
			}
			else
			{
				ClearCahProtectCardSlot();
			}
		}
	}
	else
		assert(0);
}

void cEnhancedWindow::UpdateCardCount()
{
	if( mCardSlot )
	{
		unsigned short cardSlotIndex = mCardSlot->GetSlotIndex();
		if( cardSlotIndex <= MAX_INVENTORY )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mCardSlot->GetItemIndex() );
			unsigned int count = ( p ) ? p->bag : 0;
			if( count > 0 )
			{
				mCardSlot->SetNumber(count);
				mCardSlot->ShowNumber(true);

				int ret0 = ITEMMAN->CheckLimit( mCardSlot->GetItemIndex() );
				int ret1 = ITEMMAN->CheckLimitEtc( mCardSlot->GetItemIndex() );

				NiColorA color = NiColorA::WHITE;
				if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
				{
					color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
				mCardSlot->SetColor( color );
			}
			else
			{
				ClearCardSlot();
			}
		}
	}
	else
		assert(0);
}

void cEnhancedWindow::OnNetLock( int lockTry )
{
	if( mpStart )
		mpStart->NetLock( lockTry );
}

void cEnhancedWindow::OnNetUnLock( int lockTry )
{
	if( mpStart )
		mpStart->NetUnLock( lockTry );
}

void cEnhancedWindow::HideResult()
{
	if( mpSuccess )
		mpSuccess->Hide();

	if( mpFailure )
		mpFailure->Hide();
}

void cEnhancedWindow::HideESC( bool onsound )
{
	ITEMMAN->TryToEnhancedCancel();
}

/// 결과 이미지 보여주기
void cEnhancedWindow::SetResult( bool success )
{
	if( success == true )
	{
		if( mpSuccess )
			mpSuccess->Show();
	}
	else
	{
		if( mpFailure )
			mpFailure->Show();
	}
}

bool cEnhancedWindow::IsEnableEnhance()
{
	 if( mItemSlot->GetSlotIndex() <= MAX_INVENTORY && mCardSlot->GetSlotIndex() <= MAX_INVENTORY )
	 {
		 return true;
	 }

	 return false;
}

void cEnhancedWindow::StartEnhanced()
{
	if( mpGaugeBar )
	{
		float speed = 100.0f / ((float)ENHANCE_DELAYTIME*0.001f);

		mpGaugeBar->SetGaugeSpeed( speed );
		mpGaugeBar->SetGaugeAni( 100 );
	}

	if( mpStart )
		mpStart->SetEnabled( false );

 	if( mpCancel )
 		mpCancel->SetEnabled( true );

	if( mCashProtectCardSlot )
		mCashProtectCardSlot->SetUsedFlag( false, false );

	mIsEnhanceNow = true;

}

void cEnhancedWindow::StopEnhanced()
{
	if( mpGaugeBar )
		mpGaugeBar->ClearFront();

	if( mpStart )
		mpStart->SetEnabled( true );

 	if( mpCancel )
 		mpCancel->SetEnabled( false );

	if( mCashProtectCardSlot )
		mCashProtectCardSlot->SetUsedFlag( true, false );

	mIsEnhanceNow = false;

}

void cEnhancedWindow::SetStartBtnEnable( bool bEnable )
{
	if( mpStart )
		mpStart->SetEnabled( bEnable );
}
