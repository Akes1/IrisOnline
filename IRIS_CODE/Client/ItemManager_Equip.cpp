#include "stdafx.h"
#include "Item_Common.h"
#include "ItemManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "PartyManager.h"
#include "PartyUnionManager.h"

bool cItemManager::TestMoveItemDown( unsigned short i0, unsigned short i1 )
{
	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	if( item0.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
		/// 걍 넘어감
	}
	else
	{
		if( item1.mEquipType == ITEM_EQUIP_TWO_HANDS )
		{
			unsigned int i2 = 0;

			switch( i0 )
			{
			case INVENTORY_HAND_LEFT1:
				i2 = INVENTORY_HAND_RIGHT1;
				break;
			case INVENTORY_HAND_RIGHT1:
				i2 = INVENTORY_HAND_LEFT1;
				break;
			case INVENTORY_HAND_LEFT2:
				i2 = INVENTORY_HAND_RIGHT2;
				break;
			case INVENTORY_HAND_RIGHT2:
				i2 = INVENTORY_HAND_LEFT2;
				break;
			}

			cItem& item2 = mItemArray[i2];

			if( item2.mCount )
			{
				unsigned short i3 = 0;
				if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
					return false;
				}
			}
		}
	}
	return true;
}

bool cItemManager::TestMoveItemUp( unsigned short i0, unsigned short i1 )
{
	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	if( item0.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
		if( item1.mEquipType == ITEM_EQUIP_TWO_HANDS )
		{
		}
		else if( item1.mCount )
		{
			unsigned int i2 = 0;

			switch( i1 )
			{
			case INVENTORY_HAND_LEFT1:
				i2 = INVENTORY_HAND_RIGHT1;
				break;
			case INVENTORY_HAND_RIGHT1:
				i2 = INVENTORY_HAND_LEFT1;
				break;
			case INVENTORY_HAND_LEFT2:
				i2 = INVENTORY_HAND_RIGHT2;
				break;
			case INVENTORY_HAND_RIGHT2:
				i2 = INVENTORY_HAND_LEFT2;
				break;
			}

			cItem& item2 = mItemArray[i2];

			if( item2.mCount )
			{
				unsigned short i3 = 0;
				if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
					return false;
				}
			}
		}
	}
	return true;
}

bool cItemManager::TestEquipWeapon( unsigned short& i1, eItemEquip equipType )
{
	if( HERO->IsReadyUseSkill() == false )
	{
		/// 스킬 동작중 장비교체 불가
		return false;
	}

	switch( equipType )
	{
	case ITEM_EQUIP_ONE_HAND:
		if( TestEquipWeapon_OneHand( i1 ) == false )
			return false;
		break;

	case ITEM_EQUIP_TWO_HANDS:
		if( TestEquipWeapon_TwoHands( i1 ) == false )
			return false;
		break;

	case ITEM_EQUIP_SHIELD:
		if( TestEquipWeapon_Shield( i1 ) == false )
			return false;
		break;

	default:
		assert( 0 && "invalid item equip type" );
		return false;
	}
	return true;
}

bool cItemManager::TestEquipWeapon_OneHand( unsigned short& i1 )
{
	i1 = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_RIGHT2 : INVENTORY_HAND_RIGHT1);
	return true;
}

bool cItemManager::TestEquipWeapon_TwoHands( unsigned short& i1 )
{
	unsigned short right = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_RIGHT2 : INVENTORY_HAND_RIGHT1);
	unsigned short left = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_LEFT2 : INVENTORY_HAND_LEFT1);

	i1 = right;
	cItem& item1 = mItemArray[right];

	if( item1.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
		/// 걍 넘어감
	}
	else if( item1.mCount )
	{
		cItem& item2 = mItemArray[left];

		if( item2.mCount )
		{
			assert( item2.mEquipType == ITEM_EQUIP_SHIELD );

			unsigned short i3 = 0;
			if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				return false;
		}
	}
	return true;
}

bool cItemManager::TestEquipWeapon_Shield( unsigned short& i1 )
{
	unsigned short right = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_RIGHT2 : INVENTORY_HAND_RIGHT1);
	unsigned short left = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_LEFT2 : INVENTORY_HAND_LEFT1);

	i1 = left;
	cItem& item1 = mItemArray[left];
	cItem& item2 = mItemArray[right];

	if( item2.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
	}
	else if( item1.mCount )
	{
		assert( item1.mEquipType == ITEM_EQUIP_SHIELD );
	}
	return true;
}

bool cItemManager::TestEquipWear( unsigned short& i1, unsigned char subType )
{
	switch( subType )
	{
	case ITEM_WEAR_HEAD:
		i1 = INVENTORY_WEAR_HAT;
		break;
	case ITEM_WEAR_UPPER:
		i1 = INVENTORY_WEAR_BODY1;
		break;
	case ITEM_WEAR_LOWER:
		i1 = INVENTORY_WEAR_BODY2;
		break;
	case ITEM_WEAR_HANDS:
		i1 = INVENTORY_WEAR_HAND;
		break;
	case ITEM_WEAR_FEET:
		i1 = INVENTORY_WEAR_FOOT;
		break;
	/*case ITEM_WEAR_ONEPIECE:
		break;*/
	case ITEM_WEAR_COS_ONEPIECE:
		{
			// 하의, 손, 발
			cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY2];
			cItem& item1 = mItemArray[INVENTORY_COSTUME_HAND];
			cItem& item2 = mItemArray[INVENTORY_COSTUME_FOOT];

			unsigned int cnt = 0;
			
			if( item0.mCount > 0 )
				cnt++;
			if( item1.mCount > 0 )
				cnt++;
			if( item2.mCount > 0 )
				cnt++;

			// 
			if( cnt > 0 )
			{
				// 빈슬롯 체크
				unsigned short emptySlot[3] = {0,};
				if( FindEmptySlotArr( emptySlot, cnt, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
					return false;
				}
			}

			i1 = INVENTORY_COSTUME_BODY1;
		}
		break;
	case ITEM_WEAR_COS_HEAD:
		i1 = INVENTORY_COSTUME_HAT;
		break;
	case ITEM_WEAR_COS_UPPER:
		i1 = INVENTORY_COSTUME_BODY1;
		break;
	case ITEM_WEAR_COS_LOWER:
		{
			cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
			if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
			{
				unsigned short slot = 0;
				if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
					return false;
				}
			}

			i1 = INVENTORY_COSTUME_BODY2;
		}
		break;
	case ITEM_WEAR_COS_HANDS:
		{
			cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
			if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
			{
				unsigned short slot = 0;
				if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
					return false;
				}
			}

			i1 = INVENTORY_COSTUME_HAND;
		}
		break;
	case ITEM_WEAR_COS_FEET:
		{
			cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
			if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
			{
				unsigned short slot = 0;
				if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
					return false;
				}
			}

			i1 = INVENTORY_COSTUME_FOOT;
		}
		break;
	default:
		assert( 0 && "invalid wear item subtype" );
		return false;
	}
	return true;
}


bool cItemManager::TestEquipAccessory( unsigned short& i1, unsigned char subType )
{
	switch( subType )
	{
	case ITEM_ACCESSORY_EARRING:
		i1 = INVENTORY_WEAR_EARRING;
		break;
	case ITEM_ACCESSORY_NECKLACE:
		i1 = INVENTORY_WEAR_NECKLACE;
		break;
	case ITEM_ACCESSORY_BROOCH:
		i1 = INVENTORY_WEAR_BROOCH;
		break;
	case ITEM_ACCESSORY_BRACELET:
		i1 = INVENTORY_WEAR_BRACELET;
		break;
	case ITEM_ACCESSORY_RING:
		i1 = INVENTORY_WEAR_RING;
		break;
	case ITEM_ACCESSORY_COS_FACE:
		i1 = INVENTORY_COSTUME_FACE;
		break;
	case ITEM_ACCESSORY_COS_BACK:
		i1 = INVENTORY_COSTUME_BACK;
		break;
	default:
		assert( 0 && "invalid accessory item subtype" );
		return false;
	}
	return true;
}

bool cItemManager::TestEquipCard( unsigned short& slotIndex1 )
{
	/// 카드 슬롯이 뚤려 있는지 체크
	unsigned short slotEndIndex = HERO->GetWearCardEndSlot();
	if( slotEndIndex < INVENTORY_WEAR_RING || slotEndIndex > INVENTORY_WEAR_CARD3 )	
	{
		assert( 0 && "Bad slot index" );
		return false;
	}

	/// 뚫려 있는 카드 슬롯이 없는 경우
	if( slotEndIndex == INVENTORY_WEAR_RING )
	{
		///에러 출력
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 3100 ) );
		return false;
	}
	else
	{
		/// 해당 슬롯 인덱스까지  비어있는 슬롯에 장착
		cItem& item1 = mItemArray[INVENTORY_WEAR_CARD1];
		cItem& item2 = mItemArray[INVENTORY_WEAR_CARD2];
		cItem& item3 = mItemArray[INVENTORY_WEAR_CARD3];

		if( item1.mCount == 0 && INVENTORY_WEAR_CARD1 <= slotEndIndex )
		{
			slotIndex1 = INVENTORY_WEAR_CARD1;
		}
		else if( item2.mCount == 0 && INVENTORY_WEAR_CARD2 <= slotEndIndex )
		{
			slotIndex1 = INVENTORY_WEAR_CARD2;
		}
		else if( item3.mCount == 0 && INVENTORY_WEAR_CARD3 <= slotEndIndex )
		{
			slotIndex1 = INVENTORY_WEAR_CARD3;
		}
		else /// 자리없으면 첫번쨰 슬롯 리턴
		{
			slotIndex1 = INVENTORY_WEAR_CARD1;
		}
		return true;
	}
}

/// 인벤토리 내부가 아니면 절대 들어올수 없다.
bool cItemManager::TestUseItem( unsigned short slotIndex0 )
{
	cItem& item = mItemArray[slotIndex0];
	cItemDefine* itemDefine = item.mDefine;
	if( itemDefine == 0 )
	{
		assert(0);
		return false;
	}

	cItemAbility* itemAbil = itemDefine->GetAbililty();
	if( itemAbil == 0 )
	{
		assert(0);
		return false;
	}

	/// 이미 쿨타임 적용중인지 검사
	if( item.IsCoolTime() == true )
		return false;

	/// 타겟 타입 비교
	const sObject* target = HERO->GetTargetInfo();
	if( !target )
	{
		assert(0);
		return false;
	}

	/// 사용타입이 자기자신이면 
	if( itemAbil->GetTargetType() == ITEM_TARGET_MYSELF )
	{
		return true;
	}
	/// 사용타입이 몬스터이면
	else if( itemAbil->GetTargetType() == ITEM_TARGET_MONSTER )
	{
		/// 타입이 맞음
		if( target->type == eOBJECTTYPE_MONSTER )
		{
			cBaseObject* pMonster = OBJECTMAN->GetObject( target->type, target->index );
			if( pMonster )
			{
				NiPoint3 monPos = pMonster->GetPos();
				NiPoint3 heroPos = HERO->GetPos();

				/// 거리
				if( (short)(heroPos - monPos).Length() < itemAbil->GetUseRange() )
				{
					return true;
				}
				else
				{
					/// 거리오류
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 238 ) );
				}
			}
			else
			{
				assert(0);
			}
		}
		else /// 타겟오류
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 237 ) );
	}
	/// 범용적인 사용타입 검사  ( 자기자신 + 플레이어 = 5, 자기자신 + 파티원 = 9...)
	else
	{
		if( itemAbil->GetTargetType() & ITEM_TARGET_MYSELF )
		{
			if( target->type == eOBJECTTYPE_HERO )
				return true;
		}
		
		if( itemAbil->GetTargetType() & ITEM_TARGET_PLAYER )
		{
			if( target->type == eOBJECTTYPE_PLAYER )
			{
				/// 플레이어는 일정 거리에 있지 않으면 생성되지 않으므로 맵검사는 필요없음.
				cBaseObject* pPlayer = OBJECTMAN->GetObject( target->type, target->index );
				if( pPlayer )
				{
					NiPoint3 playerPos = pPlayer->GetPos();
					NiPoint3 heroPos = HERO->GetPos();
					
					if( (short)(heroPos - playerPos).Length() < itemAbil->GetUseRange() )
						return true;
					else
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 238 ) );
				}
				else
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 238 ) );
			}
		}

		if( itemAbil->GetTargetType() & ITEM_TARGET_PARTY )
		{
			if( HERO->GetPartyIndex() > 0 || HERO->GetPartyUnionIndex() > 0 )
			{
				/// hero 가 파티중일때 대상이 파티원인지 검사
				if( PARTYMAN->IsHeroParty( target->index ) == true ||
					PARTYUNIONMAN->IsHeroParty( target->index ) == true )
				{
					/// 플레이어는 일정 거리에 있지 않으면 생성되지 않으므로 맵검사는 필요없음.
					cBaseObject* pPlayer = OBJECTMAN->GetObject( target->type, target->index );
					if( pPlayer )
					{
						NiPoint3 playerPos = pPlayer->GetPos();
						NiPoint3 heroPos = HERO->GetPos();

						if( (short)(heroPos - playerPos).Length() < itemAbil->GetUseRange() )
							return true;
						else
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 238 ) );
					}
					else
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 238 ) );

				}
				return true;
			}
		}	
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 아이템 내리기
bool cItemManager::MoveItemDown( unsigned short i0, unsigned short i1 )
{
	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	// i1 : 빈슬롯
	if( item1.mCount == 0 )
	{
		item1 = item0;
		item0 = 0;
	}
	else
	{
		// i1 : 존재
		if( item1.mDefine )
		{
			unsigned char type = item1.mDefine->GetType();
			unsigned char subType = item1.mDefine->GetSubType();

			switch( type )
			{
			case ITEM_WEAPON:
				if( EquipWeapon( i1, i0, item1.mEquipType, item0.mEquipType ) == false )	// i1 에서 i0으로 장착됨
					return false;
				break;
			case ITEM_WEAR:
				if( EquipWear( i1, i0, subType ) == false )
					return false;
				break;
			case ITEM_ACCESSORY:
				if( EquipAccessory( i1, i0, subType ) == false )
					return false;
				break;
			case ITEM_CARD:
				if( subType == ITEM_CARD_EQUIP_C )
				{
					if( EquipCard( i1, i0 ) == false )
						return false;
				}
				break;
			}
		}
	}

	return true;
}


// 아이템 장착
bool cItemManager::MoveItemUp( unsigned short i0, unsigned short i1 )
{
	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	if( item0.mDefine )
	{
		unsigned char type = item0.mDefine->GetType();
		unsigned char subType = item0.mDefine->GetSubType();
	
		switch( type )
		{
		case ITEM_WEAPON:
			if( EquipWeapon( i0, i1, item0.mEquipType, item1.mEquipType ) == false )
				return false;
			break;
		case ITEM_WEAR:
			if( EquipWear( i0, i1, subType ) == false )
				return false;
			break;
		case ITEM_ACCESSORY:
			if( EquipAccessory( i0, i1, subType ) == false )
				return false;
			break;
		case ITEM_CARD:
			if( subType == ITEM_CARD_EQUIP_C )
			{
				if( EquipCard( i0, i1 ) == false )
					return false;
			}
			break;
		}
	}

	return true;
}

bool cItemManager::EquipWeapon( unsigned short i0, unsigned short i1, eItemEquip equip0, eItemEquip equip1 )
{
	unsigned short i2 = 0;
	switch( i1 )
	{
	case INVENTORY_HAND_LEFT1:		i2 = INVENTORY_HAND_RIGHT1;		break;
	case INVENTORY_HAND_RIGHT1:		i2 = INVENTORY_HAND_LEFT1;		break;
	case INVENTORY_HAND_LEFT2:		i2 = INVENTORY_HAND_RIGHT2;		break;
	case INVENTORY_HAND_RIGHT2:		i2 = INVENTORY_HAND_LEFT2;		break;
	}

	switch( equip1 )
	{
	case ITEM_EQUIP_NORMAL:
		{
			switch( equip0 )
			{
			case ITEM_EQUIP_ONE_HAND:
				{
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];

					item1 = item0;
					item0 = 0;
				}
				break;
			case ITEM_EQUIP_TWO_HANDS:
				{
					// 방패 검사, 양검 삽입 
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];
					cItem& item2 = mItemArray[i2];

					if( item2.mEquipType == ITEM_EQUIP_SHIELD )
					{
						unsigned short i3 = 0;
						if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
						{
							assert(0);
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
						}

						// 방패 빈슬롯
						cItem& item3 = mItemArray[i3];
						item3 = item2;
						item2 = 0;
					}

					//
					item1 = item0;
					item0 = 0;
				}
				break;
			case ITEM_EQUIP_SHIELD:
				{
					// 양검 착용 검사, 방패 삽입
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];
					cItem& item2 = mItemArray[i2];

					if( item2.mEquipType == ITEM_EQUIP_TWO_HANDS )
					{
						unsigned short i3 = 0;
						if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
						{
							assert(0);
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
						}

						// 양검 빈슬롯
						cItem& item3 = mItemArray[i3];
						item3 = item2;
						item2 = 0;
					}

					//
					item1 = item0;
					item0 = 0;
				}
				break;
			}
		}
		break;
	case ITEM_EQUIP_ONE_HAND:
		{
			switch( equip0 )
			{
			case ITEM_EQUIP_ONE_HAND:	// i1 한검 i0 한검	: 스왑
				{
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];

					cItem tempItem = item1;
					item1 = item0;
					item0 = tempItem;
				}
				break;
			case ITEM_EQUIP_TWO_HANDS:	// i1 한검 i0 양검	: 기존 방패 착용 검사, 방패는 빈슬롯 그리고 스왑
				{
					// 기존 방패 착용 검사
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];
					cItem& item2 = mItemArray[i2];

					if( item2.mEquipType == ITEM_EQUIP_SHIELD )
					{
						unsigned short i3 = 0;
						if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
						{
							assert(0);
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
						}

						// 방패 빈슬롯
						cItem& item3 = mItemArray[i3];
						item3 = item2;
						item2 = 0;
					}

					// 스왑
					cItem tempItem = item1;
					item1 = item0;
					item0 = tempItem;
				}
				break;
			case ITEM_EQUIP_SHIELD:		// i1 한검 i0 방패	: 리턴
				assert(0);
				break;
			}
		}
		break;
	case ITEM_EQUIP_TWO_HANDS:
		{
			switch( equip0 )
			{
			case ITEM_EQUIP_ONE_HAND:				// i1 양검 i0 한검	: 스왑
			case ITEM_EQUIP_TWO_HANDS:				// i1 양검 i0 양검	: 스왑
				{
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];

					cItem tempItem = item1;
					item1 = item0;
					item0 = tempItem;
				}
				break;
			case ITEM_EQUIP_SHIELD:					// i1 양검 i0 방패	: 양검 빈슬롯, 방패 삽입
				assert(0);
				break;
			}
		}
		break;
	case ITEM_EQUIP_SHIELD:
		{
			switch( equip0 )
			{
			case ITEM_EQUIP_ONE_HAND:		// i1 방패 i0 한검	: 리턴
				assert(0);
				break;
			case ITEM_EQUIP_TWO_HANDS:		// i1 방패 i0 양검	: 방패 빈슬롯, 양검 삽입
				{
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];

					unsigned short i3 = 0;
					if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
					{
						assert(0);
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
					}

					// 방패 빈슬롯
					cItem& item3 = mItemArray[i3];
					item3 = item1;
					item1 = 0;

					// 삽입
					item1 = item0;
					item0 = 0;
				}
				break;
			case ITEM_EQUIP_SHIELD:			// i1 방패 i0 방패	: 스왑
				{
					cItem& item0 = mItemArray[i0];
					cItem& item1 = mItemArray[i1];

					cItem tempItem = item1;
					item1 = item0;
					item0 = tempItem;
				}
				break;
			}
		}
		break;
	}
	return true;
}

bool cItemManager::EquipCard( unsigned short i0, unsigned short i1 )
{
	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	cItem tempItem = item1;
	item1 = item0;
	item0 = tempItem;
	return true;
}


bool cItemManager::EquipAccessory( unsigned short i0, unsigned short i1, unsigned char subType )
{
	// 검증
	unsigned short i = 0;
	switch( subType )
	{
	case ITEM_ACCESSORY_EARRING:
		i = INVENTORY_WEAR_EARRING;
		break;
	case ITEM_ACCESSORY_NECKLACE:
		i = INVENTORY_WEAR_NECKLACE;
		break;
	case ITEM_ACCESSORY_BROOCH:
		i = INVENTORY_WEAR_BROOCH;
		break;
	case ITEM_ACCESSORY_BRACELET:
		i = INVENTORY_WEAR_BRACELET;
		break;
	case ITEM_ACCESSORY_RING:
		i = INVENTORY_WEAR_RING;
		break;
	case ITEM_ACCESSORY_COS_FACE:
		i = INVENTORY_COSTUME_FACE;
		break;
	case ITEM_ACCESSORY_COS_BACK:
		i = INVENTORY_COSTUME_BACK;
		break;
	default:
		assert( 0 && "invalid accessory item subtype" );
	}

	// 
	if( i != i1 )
	{
		assert( 0 && "invalid accessory item subtype" );
	}

	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	cItem tempItem = item1;
	item1 = item0;
	item0 = tempItem;
	return true;
}

bool cItemManager::EquipWear( unsigned short i0, unsigned short i1, unsigned char subType )
{
	if( subType == ITEM_WEAR_COS_ONEPIECE )
	{
		// 인벤 원피스 슬롯
		cItem& item0 = mItemArray[i1];

		// 상의, 하의, 손, 발
		cItem& item1 = mItemArray[INVENTORY_COSTUME_BODY1];
		cItem& item2 = mItemArray[INVENTORY_COSTUME_BODY2];
		cItem& item3 = mItemArray[INVENTORY_COSTUME_HAND];
		cItem& item4 = mItemArray[INVENTORY_COSTUME_FOOT];

		// 이미 착용시
		if( item1.mCount )
		{
			// 체인지 
			cItem temp = item0;
			item0 = item1;
			item1 = temp;
		}
		else
		{
			// 미착용시
			item1 = item0;
			item0 = 0;
		}

		// 하의
		if( item2.mCount )
		{
			unsigned short i2 = 0;
			if( FindEmptySlot( &i2, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
				return false;
			}

			//
			cItem& temp2 = mItemArray[i2];
			temp2 = item2;

			item2 = 0;
		}

		// 손
		if( item3.mCount )
		{
			unsigned short i3 = 0;
			if( FindEmptySlot( &i3, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
				return false;
			}

			//
			cItem& temp3 = mItemArray[i3];
			temp3 = item3;

			item3 = 0;
		}

		// 발
		if( item4.mCount )
		{
			unsigned short i4 = 0;
			if( FindEmptySlot( &i4, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
				return false;
			}

			//
			cItem& temp4 = mItemArray[i4];
			temp4 = item4;

			item4 = 0;
		}
	}
	else
	{
		switch( subType )
		{
		case ITEM_WEAR_COS_LOWER:
			{
				cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
				if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
				{
					unsigned short slot = 0;
					if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
						return false;
					}

					cItem& temp = mItemArray[slot];
					temp = item0;

					item0 = 0;
				}
			}
			break;
		case ITEM_WEAR_COS_HANDS:
			{
				cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
				if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
				{
					unsigned short slot = 0;
					if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
						return false;
					}

					cItem& temp = mItemArray[slot];
					temp = item0;

					item0 = 0;
				}

				CheckItemSlot( INVENTORY_COSTUME_HAND );
			}
			break;
		case ITEM_WEAR_COS_FEET:
			{
				cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
				if( item0.mDefine && item0.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
				{
					unsigned short slot = 0;
					if( FindEmptySlot( &slot, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 267 ) );
						return false;
					}

					cItem& temp = mItemArray[slot];
					temp = item0;

					item0 = 0;
				}
			}
			break;
		}
	}

	cItem& item0 = mItemArray[i0];
	cItem& item1 = mItemArray[i1];

	item1.SetTwoHanded( false );

	cItem tempItem = item1;
	item1 = item0;
	item0 = tempItem;
	return true;
}
