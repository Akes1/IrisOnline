#include "StdAfx.h"
#include "LookWindow.h"
#include "ItemIcon.h"
#include "Label.h"
#include "TipWindow.h"
#include "GameResourceManager.h"

cLookWindow::cLookWindow()
: mpName(0)
, mpInfo(0)
, mIsSet(false)
{
	memset( mInven, 0, sizeof(mInven) );
	SetBackupVisible( false );

	mFrontLeft.Set( 0, 0, 0, 0 );
	mFrontRight.Set( 0, 0, 0, 0 );
	mBackLeft.Set( 0, 0, 0, 0 );
	mBackRight.Set( 0, 0, 0, 0 );
}

cLookWindow::~cLookWindow()
{

}

void cLookWindow::Clear()
{
	if( mpName )
		mpName->Clear();

	if( mpInfo )
		mpInfo->Clear();

	for( int i = 0; i < EQUIP_SLOT_COUNT; ++i )
	{
		if( mInven[i].mpIcon )
		{
			mInven[i].mpIcon->ChangeImage(0);
			mInven[i].mpIcon->SetItemIndex( 0 );
			mInven[i].mpIcon->SetNumber( 0 );
			mInven[i].mpIcon->SetTipType( eTOOLTIP_NONE );
			mInven[i].mpIcon->ShowNumber( false );
			mInven[i].mpIcon->SetSeal( false );
			mInven[i].mpIcon->SetDBIndex( 0 );
			mInven[i].mpIcon->SetColor( NiColorA::WHITE );

			mInven[i].mpIcon->Show();
		}

		memset( &mInven[i].inven, 0, sizeof(sInventory) );
	}

	// 카드 슬롯 초기화
	for( int i = INVENTORY_WEAR_CARD1; i <= INVENTORY_WEAR_CARD3; ++i )
	{
		if( mInven[i].mpIcon )
		{
			cIconParam* pParam = UIMAN->GetIconParam( 217 );
			if( pParam )
			{
				mInven[i].mpIcon->ChangeImage( pParam );
				mInven[i].mpIcon->Show();
			}
		}
	}

	//
	SwitchWeapon( ItemActiveFront );

	mIsSet = false;
}

void cLookWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cLookWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

void cLookWindow::Show( bool onsound )
{
	if( mIsSet == false )
		return;

	cUIWindow::Show( onsound );
}

bool cLookWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpName = (cLabel*)GetChild( eUIID_GAME_LOOK_NAME );
	mpInfo = (cLabel*)GetChild( eUIID_GAME_LOOK_INFO );
	
	cString str;
	for( unsigned short i = 0, slotIndex = INVENTORY_EQUIP_BEGIN; i < EQUIP_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "Look_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		icon->SetSlotIndex( slotIndex );
		icon->SetUsedFlag( false, false );
		icon->SetLinked( true );

		mInven[i].mpIcon = icon;
	}

	// 좌표
	unsigned short backLeft   = (unsigned short)(INVENTORY_HAND_LEFT1 - INVENTORY_EQUIP_BEGIN);
	unsigned short backRight  = (unsigned short)(INVENTORY_HAND_RIGHT1 - INVENTORY_EQUIP_BEGIN);
	unsigned short frontLeft  = (unsigned short)(INVENTORY_HAND_LEFT2 - INVENTORY_EQUIP_BEGIN);
	unsigned short frontRight = (unsigned short)(INVENTORY_HAND_RIGHT2 - INVENTORY_EQUIP_BEGIN);

	if( mInven[backLeft].mpIcon )
	{
		mBackLeft = mInven[backLeft].mpIcon->GetRelativeRect();
	}
	if( mInven[backRight].mpIcon )
	{
		mBackRight = mInven[backRight].mpIcon->GetRelativeRect();
	}
	if( mInven[frontLeft].mpIcon )
	{
		mFrontLeft = mInven[frontLeft].mpIcon->GetRelativeRect();
	}
	if( mInven[frontRight].mpIcon )
	{
		mFrontRight = mInven[frontRight].mpIcon->GetRelativeRect();
	}
	return true;
}

void cLookWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_GAME_LOOK_CLOSE )
		Hide();
}

void cLookWindow::OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos )
{
	cItemIcon* itemIcon = (cItemIcon*)caller;
	if( itemIcon )
	{
		unsigned short slot = itemIcon->GetSlotIndex();
		if( !(slot >= INVENTORY_EQUIP_BEGIN && slot <= INVENTORY_EQUIP_END) )
		{
			assert(0);
			return;
		}

		unsigned short i = (unsigned short)(itemIcon->GetSlotIndex() - INVENTORY_EQUIP_BEGIN);
		UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, mInven[i].inven );
	}
}

void cLookWindow::AddInfo( LPCTSTR name, unsigned char level, unsigned short job, unsigned short wearEnd )
{
	if( mpName )
		mpName->SetText( name );

	cStringT str;
	str.Format( GAMERESOURCEMAN->GetGameText( 881 ), level, GAMERESOURCEMAN->GetJobName( job ) );

	if( mpInfo )
		mpInfo->SetText( (LPCTSTR)str.Cstr() );

	// 카드 슬롯 적용
	if( wearEnd >= INVENTORY_WEAR_CARD1 && wearEnd <= INVENTORY_WEAR_CARD3 )
	{
		int end = wearEnd - INVENTORY_WEAR_CARD1;
		for( int i = 0; i <= end; ++i )
		{
			mInven[INVENTORY_WEAR_CARD1+i].mpIcon->Hide();
		}
	}
}

void cLookWindow::AddInven( sInventory* inven )
{
	if( !inven )
	{
		assert(0);
		return;
	}

	// 슬롯 번호 검증
	unsigned short slot = inven->number;
	if( !(slot >= INVENTORY_EQUIP_BEGIN && slot <= INVENTORY_EQUIP_END) )
	{
		assert(0);
		return;
	}

	unsigned short i = (unsigned short)(slot - INVENTORY_EQUIP_BEGIN);
	if( i < 0 || i >= EQUIP_SLOT_COUNT )
	{
		assert(0);
		return;
	}

	if( !mInven[i].mpIcon )
	{
		assert(0);
		return;
	}

	if( inven->count == 0 )
	{
		assert(0);
		return;
	}

	cItemDefine* define = ITEMMAN->GetItemDefine( inven->ItemIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	// 데이터 저장
	mInven[i].inven = *inven;

	// UI 갱신
	bool seal = (inven->seal==1);
	bool showNum = (define->GetCapacity() > 1);

	cItemIcon* icon = mInven[i].mpIcon;
	icon->SetItemIndex( inven->ItemIndex );
	icon->SetNumber( inven->count );
	icon->SetSeal( seal );
	icon->ShowNumber( showNum );
	icon->ChangeImage( define->GetIconParam() );
	icon->SetTipType( define->GetTipType() );
	icon->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
	icon->Show();

	UpdateSlot( slot );

	mIsSet = true;
}

//ItemActiveFront = left2, right2
//ItemActiveBack = left1, right1
void cLookWindow::SwitchWeapon( eItemActiveWeapon activeWeapon )
{
	if( !(activeWeapon == ItemActiveFront || activeWeapon == ItemActiveBack) )
	{
		assert(0);
		return;
	}

	unsigned short right0 = (unsigned short)(INVENTORY_HAND_RIGHT1 - INVENTORY_EQUIP_BEGIN);
	unsigned short right1 = (unsigned short)(INVENTORY_HAND_RIGHT2 - INVENTORY_EQUIP_BEGIN);
	unsigned short left0 = (unsigned short)(INVENTORY_HAND_LEFT1 - INVENTORY_EQUIP_BEGIN);
	unsigned short left1 = (unsigned short)(INVENTORY_HAND_LEFT2 - INVENTORY_EQUIP_BEGIN);
	
	if( activeWeapon == ItemActiveFront )
	{
		if( mInven[right1].mpIcon )
			mInven[right1].mpIcon->SetRelativeRect( mFrontRight );
		if( mInven[left1].mpIcon )
			mInven[left1].mpIcon->SetRelativeRect( mFrontLeft );

		//
		if( mInven[right0].mpIcon )
			mInven[right0].mpIcon->SetRelativeRect( mBackRight );
		if( mInven[left0].mpIcon )
			mInven[left0].mpIcon->SetRelativeRect( mBackLeft );
	}
	else if( activeWeapon == ItemActiveBack )
	{
		if( mInven[right0].mpIcon )
			mInven[right0].mpIcon->SetRelativeRect( mFrontRight );
		if( mInven[left0].mpIcon )
			mInven[left0].mpIcon->SetRelativeRect( mFrontLeft );

		//
		if( mInven[right1].mpIcon )
			mInven[right1].mpIcon->SetRelativeRect( mBackRight );
		if( mInven[left1].mpIcon )
			mInven[left1].mpIcon->SetRelativeRect( mBackLeft );
	}
	else
		assert(0);
}

void cLookWindow::UpdateSlot( unsigned short slot )
{
	if( !(slot >= INVENTORY_EQUIP_BEGIN && slot <= INVENTORY_EQUIP_END) )
	{
		assert(0);
		return;
	} 

	//
	switch( slot )
	{
	case INVENTORY_COSTUME_BODY1:
		{
			// 코스튬 상의, 하의, 손, 발
			unsigned short body1 = (unsigned short)(INVENTORY_COSTUME_BODY1 - INVENTORY_EQUIP_BEGIN);
			unsigned short body2 = (unsigned short)(INVENTORY_COSTUME_BODY2 - INVENTORY_EQUIP_BEGIN);
			unsigned short hand  = (unsigned short)(INVENTORY_COSTUME_HAND  - INVENTORY_EQUIP_BEGIN);
			unsigned short foot  = (unsigned short)(INVENTORY_COSTUME_FOOT  - INVENTORY_EQUIP_BEGIN);

			sInventory& inven = mInven[body1].inven;
			cItemDefine* define = ITEMMAN->GetItemDefine( inven.ItemIndex );
			if( !define )
			{
				assert(0);
				return;
			}

			// 원피스이면
			if( define->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
			{
				// 하의
				cItemIcon* bodyIcon2 = mInven[body2].mpIcon;
				if( bodyIcon2 )
				{
					bodyIcon2->ChangeImage( define->GetIconParam() );
					bodyIcon2->SetTipType( define->GetTipType() );
					bodyIcon2->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}
				else
					assert(0);

				// 손
				cItemIcon* handIcon = mInven[hand].mpIcon;
				if( handIcon )
				{
					handIcon->ChangeImage( define->GetIconParam() );
					handIcon->SetTipType( define->GetTipType() );
					handIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}
				else
					assert(0);

				// 발
				cItemIcon* footIcon = mInven[foot].mpIcon;
				if( footIcon )
				{
					footIcon->ChangeImage( define->GetIconParam() );
					footIcon->SetTipType( define->GetTipType() );
					footIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}
				else
					assert(0);
			}
		}
		break;
	case INVENTORY_HAND_LEFT1:
	case INVENTORY_HAND_RIGHT1:
	case INVENTORY_HAND_LEFT2:
	case INVENTORY_HAND_RIGHT2:
		{
			unsigned short left = (unsigned short)(INVENTORY_HAND_LEFT1 - INVENTORY_EQUIP_BEGIN);
			unsigned short right = (unsigned short)(INVENTORY_HAND_RIGHT1 - INVENTORY_EQUIP_BEGIN);

			if( slot == INVENTORY_HAND_LEFT2 || slot == INVENTORY_HAND_RIGHT2 )
			{
				left = (unsigned short)(INVENTORY_HAND_LEFT2 - INVENTORY_EQUIP_BEGIN);
				right = (unsigned short)(INVENTORY_HAND_RIGHT2 - INVENTORY_EQUIP_BEGIN);
			}

			sInventory& rightInven = mInven[right].inven;
			cItemDefine* define = ITEMMAN->GetItemDefine( rightInven.ItemIndex );
			if( !define )
				return;

			if( define->GetEquipType() == ITEM_EQUIP_TWO_HANDS )
			{
				// 왼손 셋팅
				cItemIcon* leftIcon = mInven[left].mpIcon;
				if( leftIcon )
				{
					leftIcon->ChangeImage( define->GetIconParam() );
					leftIcon->SetTipType( define->GetTipType() );
					leftIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}
				else
					assert(0);
			}
		}
		break;
	}
}
