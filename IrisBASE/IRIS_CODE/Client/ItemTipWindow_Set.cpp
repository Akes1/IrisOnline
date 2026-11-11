#include "StdAfx.h"
#include "ItemTipWindow.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "ItemManager.h"
#include "Item_Common.h"
#include "InputSystem.h"
#include "NumberEditBox.h"
#include "PVPManager.h"
#include "MakeSkill_Common.h"
#include "SkillScript.h"
#include "MakeSkillScript.h"

#include "Application.h"
#include "CoolTimeManager.h"
#include "TutorialManager.h"

///
bool cItemTipWindow::SetName( cItemDefine* itemDefine, unsigned char enhance )
{
	/// 글씨 컬러 정하기
	unsigned long itemColor = COLOR_WHITE;
	ITEMMAN->GetTooltipColor( itemDefine->GetTipType(), itemColor );
	
	/// 아이템 정보 받아서 출력
	if( enhance > 0 )
	{
		/// 강화 단계 출력
		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, _T("+%d %s"), enhance, itemDefine->GetName() );
		SetText( temp, itemColor );
	}
	else
	{
		/// 이름만 출력
		LPCTSTR name = itemDefine->GetName();
		SetText( name, itemColor );
	}
	return true;
}

/// 레어도 관련 세팅
bool cItemTipWindow::SetRare( cItemDefine* itemDefine )
{
	if( !itemDefine )
		return false;

	int index = 0;
	switch( itemDefine->GetTipType() )
	{
	case eTOOLTIP_MALL:		index = 2000901;		break;
	case eTOOLTIP_ETC:		index = 2000902;		break;
	case eTOOLTIP_NORMAL:	index = 2000903;		break;
	case eTOOLTIP_MAGIC:	index = 2000904;		break;
	case eTOOLTIP_RARE:		index = 2000905;		break;
	case eTOOLTIP_UNIQUE:	index = 2000906;		break;
	case eTOOLTIP_EPIC:		index = 2000907;		break;
	case eTOOLTIP_REGEND:	index = 2000908;		break;
	}

	/// 컬러 지정
	unsigned long itemColor = COLOR_WHITE;
	ITEMMAN->GetTooltipColor( itemDefine->GetTipType(), itemColor );

	SetText( GAMERESOURCEMAN->GetItemToolTipText( index ), itemColor );
	return true;
}

///
bool cItemTipWindow::SetType( unsigned char type, unsigned char subtype, short defenseType )
{
	unsigned long index = 0;

	switch( type )
	{
	case ITEM_WEAPON:
		{
			switch( subtype )
			{
			case ITEM_WEAPON_SWORD:		index = 2000000;	break;
			case ITEM_WEAPON_BLADE:		index = 2000001;	break;
			case ITEM_WEAPON_DUAL:		index = 2000002;	break;
			case ITEM_WEAPON_CUTTER:	index = 2000003;	break;
			case ITEM_WEAPON_GUN:		index = 2000004;	break; 
			case ITEM_WEAPON_STAFF:		index = 2000005;	break; 
			case ITEM_WEAPON_SHIELD:	index = 2000006;	break;  
			}
		}
		break;
	case ITEM_WEAR:
		{
			switch( subtype )
			{
			case ITEM_WEAR_HEAD:		index = 2000007;	break;
			case ITEM_WEAR_UPPER:		index = 2000008;	break;
			case ITEM_WEAR_LOWER:		index = 2000009;	break;
			case ITEM_WEAR_HANDS:		index = 2000010;	break;
			case ITEM_WEAR_FEET:		index = 2000011;	break;
			case ITEM_WEAR_ONEPIECE:	index = 2000012;	break;

			case ITEM_WEAR_COS_ONEPIECE:	index = 2000061;	break;
			case ITEM_WEAR_COS_HEAD:		index = 2000062;	break;
			case ITEM_WEAR_COS_UPPER:		index = 2000065;	break;
			case ITEM_WEAR_COS_LOWER:		index = 2000066;	break;
			case ITEM_WEAR_COS_HANDS:		index = 2000067;	break;
			case ITEM_WEAR_COS_FEET:		index = 2000068;	break;
			}
		}
		break;
	case ITEM_ACCESSORY:
		{
			switch( subtype )
			{
			case ITEM_ACCESSORY_EARRING:	index = 2000013;	break;
			case ITEM_ACCESSORY_NECKLACE:	index = 2000014;	break;
			case ITEM_ACCESSORY_BROOCH:		index = 2000015;	break;
			case ITEM_ACCESSORY_BRACELET:	index = 2000016;	break;
			case ITEM_ACCESSORY_RING:		index = 2000017;	break;
			case ITEM_ACCESSORY_COS_FACE:	index = 2000063;	break;
			case ITEM_ACCESSORY_COS_BACK:	index = 2000064;	break;
			}
		}
		break;
	case ITEM_ETC1:
		{
			switch( subtype )
			{
			case ITEM_ETC1_POTION:	index = 2000018;	break;
			case ITEM_ETC1_MISC:	index = 2000019;	break;
			case ITEM_ETC1_QUEST:	index = 2000047;	break;
			case ITEM_ETC1_CHANGE:	index = 2000048;	break;
			case ITEM_ETC1_TITLE:		index = 2000055;	break;
			case ITEM_ETC1_SKILLRESET:  index = 2000018;	break;
			case ITEM_ETC1_EXP_RESTORE:		index = 2002001;	break;
			case ITEM_ETC1_THEME_RESET:		index = 2002003;	break;
			case ITEM_ETC1_FREE_MOVE:		index = 2002004;	break;
			case ITEM_ETC1_SHOUT:			index = 2002005;	break;
			case ITEM_ETC1_COOLTIME_CASH:	index = 2002006;	break;
			case ITEM_ETC1_AUTODETECT:		index = 2002002;	break;
			}
		}
		break;
	case ITEM_MATERIAL:
		{
			switch( subtype )
			{
			case ITEM_MATERIAL_SKILL:		index = 2000020;	break;
			case ITEM_MATERIAL_CLOTH:		index = 2000021;	break;
			case ITEM_MATERIAL_LEATHER:		index = 2000022;	break;
			case ITEM_MATERIAL_METAL:		index = 2000023;	break;
			case ITEM_MATERIAL_JEWEL:		index = 2000024;	break;
			case ITEM_MATERIAL_MISC:		index = 2000025;	break;
			case ITEM_MATERIAL_MAP:			index = 2000049;	break;
			case ITEM_MATERIAL_VEHICLE:		index = 2000056;	break;
			case ITEM_MATERIAL_ENHANCED_DOWN_30:	index = 2002002;	break; 
			case ITEM_MATERIAL_ENHANCED_DOWN_60:	index = 2002002;	break;      
			case ITEM_MATERIAL_ENHANCED_DESTROY_30: index = 2002002;	break;
			case ITEM_MATERIAL_ENHANCED_DESTROY_60: index = 2002002;	break;
			}
		}
		break;
	case ITEM_QUEST:	
		index = 2000026;
		break;
	case ITEM_ETC2:
		index = 2000027;
		break;
	case ITEM_SPREAD:
		index = 2000028;
		break;
	case ITEM_CARD:
		{
			switch( subtype )
			{
			case ITEM_CARD_TAROT:			index = 2000029; break;
			case ITEM_CARD_COMMON:			index = 2000030; break;
			case ITEM_CARD_EQUIP_C:			index = 2000033; break;
			case ITEM_CARD_EQUIP_A:			index = 2000034; break;
			case ITEM_CARD_EQUIP_B:			index = 2000035; break;
			case ITEM_CARD_EQUIP_AB:		index = 2000036; break;
			case ITEM_CARD_EQUIP_O:			index = 2000037; break;
			case ITEM_CARD_ENHANCE_WEAPON_LV1:
			case ITEM_CARD_ENHANCE_WEAPON_LV1_75:	index = 2000031; break;
			case ITEM_CARD_ENHANCE_WEAPON_LV2:
			case ITEM_CARD_ENHANCE_WEAPON_LV2_75:	index = 2000031; break;
			case ITEM_CARD_ENHANCE_WEAR_LV1:
			case ITEM_CARD_ENHANCE_WEAR_LV1_75:		index = 2000032; break;
			case ITEM_CARD_ENHANCE_WEAR_LV2:
			case ITEM_CARD_ENHANCE_WEAR_LV2_75:		index = 2000032; break;
			}
		}
		break;
	case ITEM_MONSTER:
		{
			switch( subtype )
			{
			case ITEM_MONSTER_UNKNOWN:	index = 2000038; break;
			case ITEM_MONSTER_HUMAN:	index = 2000039; break;
			case ITEM_MONSTER_ANIMAL:	index = 2000040; break;
			case ITEM_MONSTER_PLANT:	index = 2000041; break;
			case ITEM_MONSTER_INSECT:	index = 2000042; break;
			case ITEM_MONSTER_ELF:		index = 2000043; break;
			case ITEM_MONSTER_UNDEAD:	index = 2000044; break;
			case ITEM_MONSTER_GIANT:	index = 2000045; break;
			case ITEM_MONSTER_DRAGON:	index = 2000046; break;
			}
		}
		break;
	case ITEM_COIN:
		{
			switch( subtype )
			{
			case ITEM_COIN_GOLD:	index = 2000050; break;
			case ITEM_COIN_SILVER:  index = 2000051; break;
			case ITEM_COIN_COPPER:  index = 2000052; break;
			case ITEM_COIN_MEDAL_OLD:    index = 2000069; break;
			case ITEM_COIN_MEDAL_NORMAL: index = 2000069; break;	
			case ITEM_COIN_MEDAL_SILVER: index = 2000069; break;
			case ITEM_COIN_MEDAL_GOLD:   index = 2000069; break;
			}
		}
		break;
	case ITEM_MIX_SKILL:
		{
			switch( subtype )
			{
			case ITEM_MIX_SKILL_L1:	index = 2000053; break;
			case ITEM_MIX_SKILL_L2: index = 2000053; break;
			case ITEM_MIX_SKILL_L3: index = 2000053; break;
			case ITEM_MIX_SKILL_L4:	index = 2000053; break;
			case ITEM_MIX_RECIPE:		index = 2000054; break;
			case ITEM_MIX_RECIPEGROUP:	index = 2000054; break;
			}
		}
		break;
	case ITEM_INVENTORY:
		{
			switch( subtype )
			{
			case ITEM_INVENTORY_BAG:				index = 2000057; break;
			case ITEM_INVENTORY_CARD:				index = 2000057; break;
			case ITEM_INVENTORY_WAREHOUSE:			index = 2000057; break;
			case ITEM_INVENTORY_MAKESKILL:			index = 2000057; break;
			case ITEM_INVENTORY_AGENT_SUMMON:		index = 2002002; break;
			case ITEM_INVENTORY_POST_SUMMON:		index = 2002002; break;
			case ITEM_INVENTORY_WAREHOUSE_SUMMON:	index = 2002002; break;
			case ITEM_INVENTORY_USEFUL_PAPER:		index = 2002004; break;
			}
		}
		break;
	case ITEM_UNKNOWN:	
		index = 2000060;
		break;
	}

	/// 방어구면 재질을 조합해서 출력
	if( type == ITEM_WEAR && (subtype >= ITEM_WEAR_HEAD && subtype <= ITEM_WEAR_ONEPIECE) )
	{
		unsigned long defenseIdx = 0;
		switch( defenseType )
		{
		case 1:		defenseIdx = 2000101;		break;
		case 2:		defenseIdx = 2000102;		break;
		case 3:		defenseIdx = 2000103;		break;
		default:	assert(0); break;
		}

		SetText( GAMERESOURCEMAN->GetItemToolTipText( index ), GAMERESOURCEMAN->GetItemToolTipText( defenseIdx ), COLOR_WHITE );
	}
	else
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( index ), COLOR_WHITE );
	}

	return true;
}

/// 아이템 정보 출력
bool cItemTipWindow::SetBasic( cItemDefine* itemDefine, unsigned char enhance )
{
	bool usePrint = false;
	//unsigned char type = itemDefine->GetType();
	//unsigned char subtype = itemDefine->GetSubType();

	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	short physicMinAttack = 0;
	short physicMaxAttack = 0;
	short magicMinAttack = 0;
	short magicMaxAttack = 0;
	short physicDefense = 0;
	short magicDefense = 0;
	short healValue = 0;
	float useRange = 0;

	/// 일반공격력(소) - 일반공격력(대)
	physicMinAttack = pAbil->GetPhysicMinAttack();
	physicMaxAttack = pAbil->GetPhysicMaxAttack();

	/// 일반마법공격력(소) - 일반마법공격력(대)
	magicMinAttack = pAbil->GetMagicMinAttack();
	magicMaxAttack = pAbil->GetMagicMaxAttack();

	/// 일반 물리방어력
	physicDefense = pAbil->GetPhysicDefense();

	/// 일반 마법방어력
	magicDefense = pAbil->GetMagicDefense();

	/// 힐 회복량
	healValue = pAbil->GetHealValue();

	/// 사거리
	useRange = (float)(pAbil->GetAttackRange() * 0.01);

	/// 강화되어있으면 정보 출력
	unsigned char enhanceStep = enhance;
	if( enhance > 0 )
	{
		/// 강화단계에 따른 + 수치 출력
		short addPhysicAttack = 0;
		short addMagicAttack = 0;
		short addPhysicDefense = 0;
		short addMagicDefense = 0;
		short addHealValue	  = 0;


		cEnhancedOptionArr* enhanceOption = itemDefine->GetEnhancedOptionArr();
		if( !enhanceOption || enhanceOption->IsEmpty() )
			return false;

		unsigned int OptionSize = enhanceOption->GetSize();

		for( unsigned int idx = 0; idx < OptionSize; idx++ )
		{
			sItemEnhanced* pEnhance = (sItemEnhanced*)(*enhanceOption)[idx];
			if( !pEnhance )
			{
				assert(0);
				return false;
			}

			/// 물리공격력 세팅
			if( physicMaxAttack > 0 )
			{
				if( pEnhance->mEnhancedType == eENHANCE_PHYSICATTACK )
				{
					addPhysicAttack = pEnhance->mValue[enhanceStep-1];
				}
			}

			/// 마법공격력 수치 & 세팅
			if( magicMaxAttack > 0 )
			{
				if( pEnhance->mEnhancedType == eENHANCE_MAGICATTACK )
				{
					addMagicAttack = pEnhance->mValue[enhanceStep-1];
				}
			}

			/// 물리 방어력 세팅 
			if( physicDefense > 0 )
			{
				if( pEnhance->mEnhancedType == eENHANCE_PHYSICDEFENSE ) 
				{
					addPhysicDefense = pEnhance->mValue[enhanceStep-1];
				}
			}

			/// 마법 방어력 세팅 
			if( magicDefense > 0 )
			{
				if( pEnhance->mEnhancedType == eENHANCE_MAGICDEFENSE )
				{
					addMagicDefense = pEnhance->mValue[enhanceStep-1];
				}
			}

			/// 힐 회복량
			if( healValue > 0 )
			{
				if( pEnhance->mEnhancedType == eENHANCE_HEAL )
				{
					addHealValue = pEnhance->mValue[enhanceStep-1];
				}
			}
		}

		/// 강화 단계 출력
		TCHAR temp[128] = {0,};
		if( addPhysicAttack > 1 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000204 ), physicMinAttack, physicMaxAttack, addPhysicAttack );
			SetText( temp, COLOR_GREEN );
		}
		else if( physicMaxAttack > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000200 ), physicMinAttack, physicMaxAttack );
			SetText( temp, COLOR_WHITE );
		}

		::memset( temp, 0, sizeof(temp) );
		if( addMagicAttack > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000205 ), magicMinAttack, magicMaxAttack, addMagicAttack );
			SetText( temp, COLOR_GREEN );
		}
		else if( magicMaxAttack > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000201 ), magicMinAttack, magicMaxAttack );
			SetText( temp, COLOR_WHITE );
		}

		::memset( temp, 0, sizeof(temp) );
		if( addPhysicDefense > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000206 ), physicDefense, addPhysicDefense );
			SetText( temp, COLOR_GREEN );
		}
		else if( physicDefense > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000202 ), physicDefense );
			SetText( temp, COLOR_WHITE );
		}

		::memset( temp, 0, sizeof(temp) );
		if( addMagicDefense > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000207 ), magicDefense, addMagicDefense );
			SetText( temp, COLOR_GREEN );
		}
		else if( magicDefense > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000203 ), magicDefense );
			SetText( temp, COLOR_WHITE );
		}

		::memset( temp, 0, sizeof(temp) );
		if( addHealValue > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000210 ), healValue, addHealValue );
			SetText( temp, COLOR_GREEN );
		}
		else if( healValue > 0 )
		{
			::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000209 ), healValue );
			SetText( temp, COLOR_WHITE );
		}

		if( useRange > 0 )
			SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000208 ), (float)useRange, COLOR_WHITE );

		return true;
	}

	/// 인벤토리 외부정보, 혹은 강화가 안된 아이템이면 이대로 출력 
	/// 물리공격력 세팅
	if( physicMaxAttack > 0 )
	{
		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000200 ), physicMinAttack, physicMaxAttack );
		SetText( temp, COLOR_WHITE );

		usePrint = true;
	}

	/// 마법공격력 수치 & 세팅
	if( magicMaxAttack > 0 )
	{
		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000201 ), magicMinAttack, magicMaxAttack );
		SetText( temp, COLOR_WHITE );

		usePrint = true;
	}

	if( healValue > 0 )
	{
		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000209 ), healValue );
		SetText( temp, COLOR_WHITE );
		usePrint = true;
	}

	/// 물리, 마법 방어력 세팅 
	if( physicDefense > 0 )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000202 ), (unsigned long)physicDefense, COLOR_WHITE );
		usePrint = true;
	}

	if( magicDefense > 0 )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000203 ), (unsigned long)magicDefense, COLOR_WHITE );
		usePrint = true;
	}

	/// 사거리 세팅
	if( useRange > 0 )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000208 ), (float)useRange, COLOR_WHITE );
		usePrint = true;
	}

	return usePrint;
}

unsigned long cItemTipWindow::GetTooltipIdxByOpt( short type )
{
	unsigned long index = 0;
	switch ( type )
	{
	case ITEM_OPT_STR:	index = 2000250;	break; // 힘
	case ITEM_OPT_DEX:	index = 2000251;	break; // 민첩
	case ITEM_OPT_CON:	index = 2000252;	break; // 체력
	case ITEM_OPT_INT:	index = 2000253;	break; // 지능
	case ITEM_OPT_WIS:	index = 2000254;	break; // 지혜
	case ITEM_OPT_ALL:	index = 2000255;	break; // 모두(힘:민첩:체력:지능:지혜)
		// Option - Plus
	case ITEM_OPT_PLUS_PHYSIC_ATTACK:	index = 2000301; break; // 물리공격력
	case ITEM_OPT_PLUS_MAGIC_ATTACK:	index = 2000302; break; // 마법공격력
	case ITEM_OPT_PLUS_PHYSIC_DEFENSE:	index = 2000303; break; // 물리방어력
	case ITEM_OPT_PLUS_MAGIC_DEFENSE:	index = 2000304; break;	// 마법방어력
	case ITEM_OPT_PLUS_PHYSIC_ATTACK_RATE:	index = 2000305; break; // 물리명중률
	case ITEM_OPT_PLUS_MAGIC_ATTACK_RATE:	index = 2000306; break; // 마법명중률
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL:	index = 2000307; break; // 물리크리티컬
	case ITEM_OPT_PLUS_MAGIC_CRITICAL:	index = 2000308; break; // 마법크리티컬
	case ITEM_OPT_PLUS_DOD:				index = 2000309; break; // 회피
	case ITEM_OPT_PLUS_WEI:				index = 2000310; break; // 무게
	case ITEM_OPT_PLUS_MAX_HP:			index = 2000312; break; // 최대량 HP
	case ITEM_OPT_PLUS_MAX_MP:			index = 2000313; break; // 최대량 MP
	case ITEM_OPT_PLUS_MOV:				index = 2000311; break; // 이동
	case ITEM_OPT_PLUS_HP:				index = 2000314; break; // HP 회복
	case ITEM_OPT_PLUS_MP:				index = 2000315; break; // MP 회복
	case ITEM_OPT_PLUS_ATTACK_SPEED:			index = 2000508; break;
	case ITEM_OPT_PLUS_SKILL_COOLTIME_SPEED:	index = 2000509; break;
	case ITEM_OPT_PLUS_SHORT_RANGE:				index = 2000510; break;
	case ITEM_OPT_PLUS_DISTANCE_RANGE:			index = 2000510; break;
	case ITEM_OPT_PLUS_HEAL:					index = 2000503; break;
	case ITEM_OPT_PLUS_HP_RECOVERTY:			index = 2000507; break;
	case ITEM_OPT_PLUS_ALL_ATTACK:				index = 2000513; break;
	case ITEM_OPT_PLUS_ALL_DEFENSE:				index = 2000514; break;
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL_DEC:		index = 2000511; break;
	case ITEM_OPT_PLUS_MAGIC_CRITICAL_DEC:		index = 2000512; break;
	case ITEM_OPT_PLUS_RESISTANCE:				index = 2000506; break;

		// Option - Percent
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK:	index = 2000301; break; // 물리공격력
	case ITEM_OPT_PERCENT_MAGIC_ATTACK:		index = 2000302; break; // 마법공격력
	case ITEM_OPT_PERCENT_PHYSIC_DEFENSE:	index = 2000303; break;// 물리방어력
	case ITEM_OPT_PERCENT_MAGIC_DEFENSE:	index = 2000304; break;// 마법방어력
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK_RATE: index = 2000305; break;// 물리명중률
	case ITEM_OPT_PERCENT_MAGIC_ATTACK_RATE: index = 2000306; break;// 마법명중률
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL:	 index = 2000307; break;// 물리크리티컬
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL:	 index = 2000308; break;// 마법크리티컬
	case ITEM_OPT_PERCENT_DOD:				 index = 2000309; break;// 회피
	case ITEM_OPT_PERCENT_WEI:				 index = 2000310; break;// 무게
	case ITEM_OPT_PERCENT_MAX_HP:			index = 2000312; break;// 최대량 HP
	case ITEM_OPT_PERCENT_MAX_MP:			index = 2000313; break;// 최대량 MP
	case ITEM_OPT_PERCENT_MOV:				index = 2000311; break;// 이동
	case ITEM_OPT_PERCENT_HP:				index = 2000314; break;// HP 회복
	case ITEM_OPT_PERCENT_MP:				index = 2000315; break;// MP 회복
	case ITEM_OPT_PERCENT_ATTACK_SPEED:		index = 2000508; break;
	case ITEM_OPT_PERCENT_SKILL_COOLTIME_SPEED: index = 2000509; break;
	case ITEM_OPT_PERCENT_SHORT_RANGE:		index = 2000510; break;
	case ITEM_OPT_PERCENT_DISTANCE_RANGE:	index = 2000510; break;
	case ITEM_OPT_PERCENT_HEAL:				index = 2000503; break;
	case ITEM_OPT_PERCENT_HP_RECOVERTY:		index = 2000507; break;
	case ITEM_OPT_PERCENT_ALL_ATTACK:		index = 2000513; break;
	case ITEM_OPT_PERCENT_ALL_DEFENSE:		index = 2000514; break;
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL_DEC:	index = 2000511; break;
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL_DEC:	index = 2000512; break;
	case ITEM_OPT_PERCENT_RESISTANCE:			index = 2000506; break;
	
		// Option - Plus Etc
	case ITEM_OPT_PLUS_DRAIN_HP:			index = 2000501; break;
	case ITEM_OPT_PLUS_DRAIN_MP:			index = 2000502; break;
	case ITEM_OPT_PLUS_DAMAGE_DEC:			index = 2000504; break;
	case ITEM_OPT_PLUS_DAMAGE_REF:			index = 2000505; break;

		// Option - Etc
	case ITEM_OPT_PERCENT_DRAIN_HP:			index = 2000501; break;	// HP 흡수
	case ITEM_OPT_PERCENT_DRAIN_MP:			index = 2000502; break;	// MP 흡수
	case ITEM_OPT_PERCENT_DAMAGE_DEC:		index = 2000504; break;	// 데미지 흡수/감소
	case ITEM_OPT_PERCENT_DAMAGE_REF:		index = 2000505; break;	// 데미지 반사
	}

	return index;
}

/// 추가수치
bool cItemTipWindow::SetSpecial( cItemDefine* itemDefine )
{
	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	bool usePrint = false;
	sAblityOption* opt = pAbil->GetOption();
	for( int i = 0; i < 5; ++i, ++opt )
	{
		// 번호 대역으로 %,+ 사용 구분
		bool usePercent = false;
		if( opt->mOptType > 100 )
		{
			short value = ( opt->mOptType / 100 );
			usePercent = ( value % 2 == 0 );
		}
		
		usePrint = AddSpecial( usePrint, GetTooltipIdxByOpt( opt->mOptType ), opt->mOptValue, usePercent );
	}
	return usePrint;
}

bool cItemTipWindow::AddSpecial( bool usePrint, unsigned long tooltipIdx, short value, bool usePercent )
{
	if( value == 0 )
		return usePrint;

	LPCTSTR tip = GAMERESOURCEMAN->GetItemToolTipText( tooltipIdx );
	TCHAR temp[128]={0,};
	if( value > 0 )
	{
		if( usePercent == true )
			::_stprintf_s( temp, _T("%s +%d%%"), tip, value );
		else
			::_stprintf_s( temp, _T("%s +%d"), tip, value );
	}
	else if( value < 0 )
	{
		if( usePercent == true )
			::_stprintf_s( temp, _T("%s %d%%"), tip, value );
		else
			::_stprintf_s( temp, _T("%s %d"), tip, value );
	}

	SetText( temp, COLOR_GREEN );
	return true;
}

bool cItemTipWindow::SetSpecialCard( cItemDefine* itemDefine )
{
	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	bool usePrint = true;
	sAblityOption* opt = pAbil->GetOption();
	for( int i = 0; i < 5; ++i, ++opt )
	{
		usePrint = AddSpecialEquipCard( usePrint, GetTooltipIdxByOpt( opt->mOptType ), opt->mOptValue, (opt->mOptType >= ITEM_OPT_PERCENT_PHYSIC_ATTACK) );
	}

	/// 수치가 하나라도 입력되었으면
	return (!usePrint);
}

bool cItemTipWindow::AddSpecialEquipCard( bool IsFront, unsigned long tooltipIdx, short value, bool usePercent )
{
	if( value == 0 )
		return IsFront;

	cStringT str = GAMERESOURCEMAN->GetItemToolTipText( 2000706 );		/// 장착효과
	cStringT empty = GAMERESOURCEMAN->GetItemToolTipText( 2000707 );	/// 공백라인

	/// 문자열 조합
	cStringT tip = GAMERESOURCEMAN->GetItemToolTipText( tooltipIdx );
	if( IsFront == true )
		str = ( str + tip );
	else
		str = ( empty + tip );

	TCHAR temp[128]={0,};
	if( value > 0 )
	{
		if( usePercent == true )
			::_stprintf_s( temp, _T("%s +%d%%"), (LPCTSTR)str.Cstr(), value );
		else
			::_stprintf_s( temp, _T("%s +%d"), (LPCTSTR)str.Cstr(), value );
	}
	/// 감소
	else
	{
		if( usePercent == true )
			::_stprintf_s( temp, _T("%s %d%%"), (LPCTSTR)str.Cstr(), value );
		else
			::_stprintf_s( temp, _T("%s %d"), (LPCTSTR)str.Cstr(), value );
	}

	SetText( temp, COLOR_GREEN );
	return false;
}

///
void cItemTipWindow::SetLimit( cItemDefine* itemDefine )
{
	if( itemDefine == 0 )
		return;

	cItemLimit* itemLimit = itemDefine->GetLimit();
	if( itemLimit == 0 )
		return;

	// 요구레벨 설정 되었을시
	unsigned char minLevel = itemLimit->GetLevel();
	unsigned char maxLevel = itemLimit->GetMaxLevel();
	unsigned char heroLevel = HERO->GetLevel();
	if( minLevel > ITEMLIMIT_MINLEVEL || maxLevel < ITEMLIMIT_MAXLEVEL )
	{
		cStringT str;

		// ~ 이상 ~이하
		if( minLevel > ITEMLIMIT_MINLEVEL && maxLevel < ITEMLIMIT_MAXLEVEL )
		{
			unsigned long color = ( heroLevel >= minLevel && heroLevel <= maxLevel ) ? COLOR_WHITE : COLOR_RED;
			str.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000858 ), minLevel, maxLevel );
			SetText( (LPCTSTR)str.Cstr(), color );
		}
		// ~ 이하
		else if( maxLevel < ITEMLIMIT_MAXLEVEL )
		{
			unsigned long color = ( heroLevel <= maxLevel ) ? COLOR_WHITE : COLOR_RED;
			str.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000856 ), maxLevel );
			SetText( (LPCTSTR)str.Cstr(), color );
		}
		// ~ 이상
		else if( minLevel > ITEMLIMIT_MINLEVEL )
		{
			unsigned long color = ( heroLevel >= minLevel ) ? COLOR_WHITE : COLOR_RED;
			str.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000857 ), minLevel );
			SetText( (LPCTSTR)str.Cstr(), color );
		}
	}

	/// 사용종족 세팅
	/// ITEM_RACE_ALL 이면 모든종족이 가능하므로 스킵
	if( itemLimit->GetRace() == ITEM_RACE_HUMAN )
	{
		/// 인간
		unsigned long color = ( HERO->GetRace() == eRACE_HUMAN ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000802 ), GAMERESOURCEMAN->GetItemToolTipText( 2000810 ), color );
	}
	else if( itemLimit->GetRace() == ITEM_RACE_BEAST )
	{
		/// 수인
		unsigned long color = ( HERO->GetRace() == eRACE_BEAST ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000802 ), GAMERESOURCEMAN->GetItemToolTipText( 2000811 ), color );
	}
	else if( itemLimit->GetRace() == ITEM_RACE_ELF )
	{
		/// 정령
		unsigned long color = ( HERO->GetRace() == eRACE_ELF ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000802 ), GAMERESOURCEMAN->GetItemToolTipText( 2000812 ), color );
	}

	/// 사용성별 세팅 
	if( itemLimit->GetGender() == ITEM_GENDER_MALE )
	{
		unsigned long color = ( itemLimit->GetGender() == HERO->GetGender() ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000804 ), GAMERESOURCEMAN->GetItemToolTipText( 2000813 ), color );
	}
	else if( itemLimit->GetGender() == ITEM_GENDER_FEMALE )
	{
		unsigned long color = ( itemLimit->GetGender() == HERO->GetGender() ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000804 ), GAMERESOURCEMAN->GetItemToolTipText( 2000814 ), color );
	}

	/// 직업 세팅
	if( itemLimit->GetJob1() != ePLAYER_NONEJOB && itemLimit->GetJob2() != ePLAYER_NONEJOB )
	{
		LPCTSTR jobName1 = GAMERESOURCEMAN->GetJobName( itemLimit->GetJob1() );
		LPCTSTR jobName2 = GAMERESOURCEMAN->GetJobName( itemLimit->GetJob2() );
		
		cStringT str;
		str.Format( _T("%s"), jobName1 );
		
		// 동일한 직업이 아니면 출력
		if( itemLimit->GetJob1() != itemLimit->GetJob2() )
		{
			str.Append( _T(" ") );
			str.Append( jobName2 );
		}	

		// HERO 의 직업 체크
		unsigned long color = COLOR_WHITE;
		if( ITEMMAN->CheckLimitJob( itemLimit ) == false )
			color = COLOR_RED;

		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000803 ), str.Cstr(), color );
	}

	/// 세력 ( 0 : 전부, 1: 헬렌, 2:네로, 3:아네스, 4:데라 )
	if( itemLimit->GetForceType() != eFORCETYPE_NONE )
	{
		unsigned long color = ( itemLimit->GetForceType() == HERO->GetForceType() ) ? COLOR_WHITE : COLOR_RED;

		unsigned long textIndex = 0;
		switch( itemLimit->GetForceType() )
		{
		case eFORCETYPE_FIRE:	textIndex = 2000849; break;
		case eFORCETYPE_WATER:	textIndex = 2000850; break;
		case eFORCETYPE_WIND:	textIndex = 2000851; break;
		case eFORCETYPE_EARTH:	textIndex = 2000852; break;
		default: assert(0); return;
		}

		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000853 ), GAMERESOURCEMAN->GetItemToolTipText( textIndex ), color );
	}

	/// 보조직업 제한 있을시 
	if( itemLimit->GetMakeSkill() > 0 )
	{
		// 사용 보조직업명
		unsigned long textIndex = 0;
		switch( itemLimit->GetMakeSkill() )
		{
		case eMAKESKILL_COOK:		textIndex = 2000881; break;
		case eMAKESKILL_PAINTCARD:	textIndex = 2000882; break;
		case eMAKESKILL_AlCHEMY:	textIndex = 2000883; break;
		default: assert(0); break;
		}

		sMakeSkill* mySkill = 0;

		// 익힌 스킬인지 검사
		sHeroInfo* info = HERO->GetHeroInfo();
		if( itemLimit->GetMakeSkill() == info->MakeSkill1.mIndex )
		{
			mySkill = &info->MakeSkill1;
		}
		else if( itemLimit->GetMakeSkill() == info->MakeSkill2.mIndex )
		{
			mySkill = &info->MakeSkill2;
		}

		unsigned long color = COLOR_RED;
		// 해당 보조직업 익혔는지 검사
		if( mySkill != 0  )
		{
			/// 캐릭터 보조직업 숙련도와 아이템 보조직업 숙련도 체크
			if( mySkill->mExp >= itemLimit->GetMakeSkillExp() )
			{
				color = COLOR_WHITE;					
			}
		}

		// 등급
		cStringT temp;
		temp.Format( _T("%s (%d)"), GAMERESOURCEMAN->GetItemToolTipText( textIndex ), itemLimit->GetMakeSkillExp() );
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000854 ), (LPCTSTR)temp.Cstr(), color );				
	}

	/// 전장 사용 세팅
	if( itemLimit->GetUseMapType() == eUSE_MAP_PVP )
	{
		unsigned long color = ( PVPMAN->IsEnableBattle() ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000712 ), color );
	}
	/// 튜토리얼 전용 사용 세팅
	else if( itemLimit->GetUseMapType() == eUSE_MAP_TUTORIAL )
	{
		unsigned long color = ( TUTORIALMAN->IsTutorialMode() ) ? COLOR_WHITE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000721 ), color );
	}
	/// 필드 맵에서만 사용 세팅
	else if( itemLimit->GetUseMapType() == eUSE_MAP_FIELD )
	{
		unsigned long  color = COLOR_RED;
		unsigned short mapIndex = HERO->GetMapNum();
		if( mapIndex < MAP_MIN || mapIndex > MAP_MAX )
			color = COLOR_RED;
		else
			color = COLOR_WHITE ;

		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000729 ), color );
	}
}

///
bool cItemTipWindow::SetEffect( cItemDefine* itemDefine )
{
	if( itemDefine == 0 )
		return false;

	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	long index = pAbil->GetDescription();

	if( itemDefine->GetType() == ITEM_CARD )
	{
		if( itemDefine->GetSubType() >= ITEM_CARD_EQUIP_C && itemDefine->GetSubType() <= ITEM_CARD_EQUIP_O )
		{
			AppendText( GAMERESOURCEMAN->GetItemToolTipText( 2000706 ), GAMERESOURCEMAN->GetItemToolTipText( index ), COLOR_GREEN );
			return true;
		}
	}

	AppendText( GAMERESOURCEMAN->GetItemToolTipText( 2000805 ), GAMERESOURCEMAN->GetItemToolTipText( index ), COLOR_GREEN );
	return true;
}

/// 재사용시간 
bool cItemTipWindow::SetUseTime( cItemDefine* itemDefine )
{
	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	short coolTime = (short)(pAbil->GetCoolTime() / 1000);

	/// 재사용시간 구해서 출력
	unsigned int m = ((unsigned int)coolTime ) / 60;
	unsigned int s = ((unsigned int)coolTime % 60) % 60;

	/// 재사용시간이 0 초면 리턴
	if( m == 0 && s == 0 )
		return false;

	cStringT tm,ts;
	if( m != 0 )
		tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
	if( s != 0 )
		ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );
	tm += ts;

	cStringT temp;
	temp.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000806 ), tm.Cstr() );
	
	//
	if( mCheckTime )
	{
		mTimerEnabled = true;
		mUpdateSizebyTime = true;
	}

	// 정보구성
	mTimeLine[ eTIMELINE_COOLTIME ].saveCoolTime = temp;
	mTimeLine[ eTIMELINE_COOLTIME ].timeTipIdx = mIndex;

	SetText( temp.Cstr(), COLOR_WHITE );
	return true;
}

/// 쿨타임 시간 세팅
void cItemTipWindow::OnTimer()
{
	cUIWindow::OnTimer();

	if( mCheckTime == false )
		return;

	// 쿨타임
	if( mTimeLine[eTIMELINE_COOLTIME].timeTipIdx != 0 )
	{
		cStringT str1 = mTimeLine[eTIMELINE_COOLTIME].saveCoolTime;

		sCoolTime* timeInfo = COOLMAN->GetProcessData( mItemIndex );
		if( timeInfo )
		{
			/// 사용대기시간 구해서 출력
			long elapsedTime = THEAPP->GetWorldAccumTime() - timeInfo->startTime;

			unsigned long restTime = 0;
			if( timeInfo->restTime > elapsedTime )
			{
				restTime = timeInfo->restTime - elapsedTime;
			}
			restTime /= 1000;

			cStringT str2;
			/// 초를 출력시 반올림해서 출력
			unsigned int m = restTime / 60;
			unsigned int s = restTime % 60 + 1;

			cStringT tm,ts;
			if( m != 0 )
				tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
			ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );

			tm += ts;
			str2.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000807 ), tm.Cstr() );
			str1.Append( str2 );
		}

		unsigned int timeTipIdx = mTimeLine[eTIMELINE_COOLTIME].timeTipIdx;
		mToolTip[timeTipIdx].mString = str1;
		mToolTip[timeTipIdx].mColor = COLOR_WHITE;
		CalcMaxWidth( mToolTip[timeTipIdx].mString );
	}

	if( mTimeLine[eTIMELINE_LIMITDATE].timeTipIdx != 0 )
	{
		cStringT str1 = mTimeLine[eTIMELINE_LIMITDATE].saveCoolTime;

		if( mItemBill && mItemBill->type == 2 )
		{
			time_t ltime;
			long   result;

			time( &ltime );

			result = (long)difftime( ((sItemBillEx*)mItemBill)->begin, ltime );
            if( result < 0 )
                result = 0;

			unsigned int d = (result / 60) / 60 / 24;
			unsigned int h = (result / 60) / 60 % 24;
			unsigned int m = (result / 60) % 60;
			unsigned int s = (result % 60) % 60 + 1;

			cStringT td,th,tm,ts;
			if( d != 0 )
				td.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000728 ), d );
			if( h != 0 )
				th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
			if( m != 0 )
				tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000727 ), m );
			if( s != 0 )
				ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000726 ), s );

			td += th;
			td += tm;
			td += ts;
			
			if( td.GetLength() == 0 )
				td.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), 0 );

			str1.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), td.Cstr() );
		}

		unsigned int timeTipIdx = mTimeLine[eTIMELINE_LIMITDATE].timeTipIdx;
		mToolTip[timeTipIdx].mString = str1;
		mToolTip[timeTipIdx].mColor = COLOR_WHITE;
		CalcMaxWidth( mToolTip[timeTipIdx].mString );
	}

	if( mUpdateSizebyTime == true )
	{
		SetBackImg( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
		mUpdateSizebyTime = false;
	}
}

/// 가격 세팅
void cItemTipWindow::SetPrice( unsigned long type, cItemDefine* itemDefine, unsigned long priceTooltipIdx, unsigned long price )
{
	TCHAR temp[128]={0,};

	LPCTSTR tooltip = GAMERESOURCEMAN->GetItemToolTipText( priceTooltipIdx );
	LPCTSTR priceStr = cNumberEditBox::FormatNumber( price );

	if( type == ItemSellNone )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000703 ), COLOR_RED );
	}	
	else
	{
		unsigned long unitIndex = ITEMMAN->GetPriceUnit( type );
		::_stprintf_s( temp, _T("%s %s %s"), tooltip, priceStr, GAMERESOURCEMAN->GetItemToolTipText( unitIndex ) );
		SetText( temp, COLOR_WHITE );
	}
}

typedef tPair<cItemDefine*, bool> cSetInfoPair;
typedef tList<cSetInfoPair> cSetInfoList;

void cItemTipWindow::SetItemSetInfo( cItemDefine* itemDefine, unsigned int setIndex )
{
	if( setIndex == 0 )
		return;

	cItemSetInfo* pSetInfo = ITEMMAN->GetItemSetInfo( setIndex );
	if( pSetInfo == 0 )
		return;

	/// 세트 갯수
	unsigned int itemCnt = pSetInfo->GetItemCount();
	if( itemCnt == 0 )
		return;

	/// 착용 갯수
	unsigned int equipCnt = 0;

	cSetInfoList setInfoList;

	/// 정보 수집
	{
		tList<unsigned long>* itemList = pSetInfo->GetItemList();
		tList<unsigned long>::cIterator b = itemList->Begin();
		tList<unsigned long>::cIterator end = itemList->End();
		for( ; b != end; ++b)
		{
			unsigned long idx = (unsigned long)(*b);
			cItemDefine* p = ITEMMAN->GetItemDefine( idx );
			if( p == 0 )
			{
				assert(0);
				continue;
			}

			bool equiped = ITEMMAN->IsEquipedItem( idx );
			setInfoList.PushBack( cSetInfoPair( p, equiped) );

			if( equiped == true )
				equipCnt++;
		}
	}

	/// 정보 출력
	{
		/// setName 입력
		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, _T("%s (%d/%d)"), ITEMMAN->GetItemName(pSetInfo->GetNameIndex()), equipCnt, itemCnt );
		SetText( temp, COLOR_MALL );

		/// 아이템 리스트 표시
		cSetInfoList::cConstIterator b = setInfoList.Begin();
		cSetInfoList::cConstIterator end = setInfoList.End();
		for( ; b != end; ++b)
		{
			unsigned long color = COLOR_DARKGRAY;
			if( b->mSecond == true )
				color = COLOR_WHITE;

			cItemDefine* p = b->mFirst;

			TCHAR temp[128]={0,};
			::_stprintf_s( temp, _T(" %s"), p->GetName() );
			SetText( temp, color );
		}
		AddGap(1);

		/// 세트 효과 표시
		LPCTSTR setTip = GAMERESOURCEMAN->GetItemToolTipText( 2000714 );
		for( unsigned int cnt = 1; cnt <= itemCnt; ++cnt )
		{
			unsigned long color = COLOR_DARKGRAY;
			if( cnt <= equipCnt )
				color = COLOR_MAGIC;

			sSetAblityOption* opt = pSetInfo->GetItemOption( cnt );
			if( opt )
			{
				if( opt->mOptType[0] != 0 )
				{
					LPCTSTR tip = GAMERESOURCEMAN->GetItemToolTipText( GetTooltipIdxByOpt( opt->mOptType[0] ) );
					TCHAR temp[128]={0,};

					/// option 0
					if( opt->mOptValue[0] > 0 )
					{
						if( opt->mOptType[0] >= ITEM_OPT_PERCENT_PHYSIC_ATTACK )
							::_stprintf_s( temp, _T("(%d) %s%s +%d%%"), cnt, setTip, tip, opt->mOptValue[0] );
						else
							::_stprintf_s( temp, _T("(%d) %s%s +%d"), cnt, setTip, tip, opt->mOptValue[0] );
					}
					else if( opt->mOptValue[0] < 0 )
					{
						if( opt->mOptType[0] >= ITEM_OPT_PERCENT_PHYSIC_ATTACK )
							::_stprintf_s( temp, _T("(%d) %s%s %d%%"), cnt, setTip, tip, opt->mOptValue[0] );
						else
							::_stprintf_s( temp, _T("(%d) %s%s %d"), cnt, setTip, tip, opt->mOptValue[0] );
					}

					SetText( temp, color );
				}

				/// option 1
				if( opt->mOptType[1] != 0 )
				{
					LPCTSTR tip = GAMERESOURCEMAN->GetItemToolTipText( GetTooltipIdxByOpt( opt->mOptType[1] ) );
					TCHAR temp[128]={0,};

					if( opt->mOptValue[1] > 0 )
					{
						if( opt->mOptType[1] >= ITEM_OPT_PERCENT_PHYSIC_ATTACK )
							::_stprintf_s( temp, _T("(%d) %s%s +%d%%"), cnt, setTip, tip, opt->mOptValue[1] );
						else
							::_stprintf_s( temp, _T("(%d) %s%s +%d"), cnt, setTip, tip, opt->mOptValue[1] );
					}
					else if( opt->mOptValue[1] > 0 )
					{
						if( opt->mOptType[1] >= ITEM_OPT_PERCENT_PHYSIC_ATTACK )
							::_stprintf_s( temp, _T("(%d) %s%s %d%%"), cnt, setTip, tip, opt->mOptValue[1] );
						else
							::_stprintf_s( temp, _T("(%d) %s%s %d"), cnt, setTip, tip, opt->mOptValue[1] );
					}

					SetText(temp, color);
				}

				/// active
				if( opt->mActIdx != 0 )
				{	
					TCHAR temp[256]={0,};

					sActInfluenceScript* active = SKILLSCRIPT->GetActInfluenceInfo( opt->mActIdx );
					if( active )
					{
						unsigned long textIndex = 0;
						switch( active->mActivityState )
						{
						case eACTSTATUS_ACTIVESKILL:	textIndex = 2000951; break;
						case eACTSTATUS_HEALSKILL:		textIndex = 2000952; break;
						case eACTSTATUS_DAMAGE:			textIndex = 2000953; break;
						default: assert(0); break;
						}

						if( textIndex > 0 )
						{
							::_stprintf_s( temp, _T("(%d) %s%s%s"), cnt, setTip, GAMERESOURCEMAN->GetItemToolTipText( textIndex )
																			   , GAMERESOURCEMAN->GetItemToolTipText( active->mTooltipIdx ) );

							SetText( temp, color );
						}
					}
					else
						assert(0);
				}
			}
		}
		AddGap(1);
	}

	setInfoList.Clear();
}

unsigned long cItemTipWindow::GetMixGradeIndex( char step )
{
	if( step < 0 )
	{
		assert(0);
		return 0;
	}

	unsigned long gradeIndex = 0;
	switch( step )
	{
	case ITEM_MIX_SKILL_L1:		gradeIndex = 2000871; break;
	case ITEM_MIX_SKILL_L2:		gradeIndex = 2000872; break;
	case ITEM_MIX_SKILL_L3:		gradeIndex = 2000873; break;
	case ITEM_MIX_SKILL_L4:		gradeIndex = 2000874; break;
	default:					assert(0);			  break;
	}
	return gradeIndex;
}

