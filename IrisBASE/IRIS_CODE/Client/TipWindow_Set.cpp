#include "StdAfx.h"
#include "TipWindow.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "ItemManager.h"

///
bool cTipWindow::SetName( const wchar_t* name )
{
	SetText( name, eCOLOR_YELLOW );
	return true;
}

///
bool cTipWindow::SetType( unsigned char type, unsigned char subtype )
{
	unsigned long index = 0;

	switch( type )
	{
	case ITEM_WEAPON:
		{
			switch( subtype )
			{
			case ITEM_WEAPON_SWORD:		index = 3000;	break;
			case ITEM_WEAPON_BLADE:		index = 3001;	break;
			case ITEM_WEAPON_DUAL:		index = 3002;	break;
			case ITEM_WEAPON_CUTTER:	index = 3003;	break;
			case ITEM_WEAPON_GUN:		index = 3004;	break; 
			case ITEM_WEAPON_STAFF:		index = 3005;	break; 
			case ITEM_WEAPON_SHIELD:	index = 3006;	break;  
			}
		}
		break;
	case ITEM_WEAR:
		{
			switch( subtype )
			{
			case ITEM_WEAR_HEAD:		index = 3007;	break;
			case ITEM_WEAR_UPPER:		index = 3008;	break;
			case ITEM_WEAR_LOWER:		index = 3009;	break;
			case ITEM_WEAR_HANDS:		index = 3010;	break;
			case ITEM_WEAR_FEET:		index = 3011;	break;
			case ITEM_WEAR_ONEPIECE:	index = 3012;	break;
			}
		}
		break;
	case ITEM_ACCESSORY:
		{
			switch( subtype )
			{
			case ITEM_ACCESSORY_EARRING:	index = 3013;	break;
			case ITEM_ACCESSORY_NECKLACE:	index = 3014;	break;
			case ITEM_ACCESSORY_BROOCH:		index = 3015;	break;
			case ITEM_ACCESSORY_BRACELET:	index = 3016;	break;
			case ITEM_ACCESSORY_RING:		index = 3017;	break;
			}
		}
		break;
	case ITEM_ETC1:
		{
			switch( subtype )
			{
			case ITEM_ETC1_POTION:	index = 3018;	break;
			case ITEM_ETC1_MISC:	index = 3019;	break;
			}
		}
		break;
	case ITEM_MATERIAL:
		{
			switch( subtype )
			{
			case ITEM_MATERIAL_CLOTH:		index = 3020;	break;
			case ITEM_MATERIAL_LEATHER:		index = 3021;	break;
			case ITEM_MATERIAL_METAL:		index = 3022;	break;
			case ITEM_MATERIAL_JEWEL:		index = 3023;	break;
			case ITEM_MATERIAL_MISC:		index = 3024;	break;
			}
		}
		break;
	case ITEM_QUEST:	
		index = 3025;
		break;
	case ITEM_ETC2:
		index = 3026;
		break;
	case ITEM_SPREAD:
		index = 3027;
		break;
	case ITEM_CARD:
		{
			switch( subtype )
			{
			case ITEM_CARD_TAROT:	index = 3028;	break;
			case ITEM_CARD_COMMON:	index = 3029;	break;
			}
		}
		break;
	case ITEM_UNKNOWN:	
		index = 3030;
		break;
	}

	SetText( GAMERESOURCEMAN->GetGameText( index ), eCOLOR_WHITE );
	return true;
}

/// 기본수치 
bool cTipWindow::SetBasic( cItemDefine* itemDefine )
{
	unsigned char type = itemDefine->GetType();
	unsigned char subtype = itemDefine->GetSubType();

	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	short physicMinAttack = 0;
	short physicMaxAttack = 0;
	short magicMinAttack = 0;
	short magicMaxAttack = 0;
	short physicDefense = 0;
	short magicDefense = 0;
	long coolTime = 0;

	/// 무기중에 방패만 제외
	if( type == ITEM_WEAPON && subtype != ITEM_WEAPON_SHIELD )
	{
		/// 물리공격력 수치
		physicMinAttack = pAbil->GetPhysicMinAttack();
		physicMaxAttack = pAbil->GetPhysicMaxAttack();
		coolTime = pAbil->GetCoolTime();

		/// 물리공격력 세팅
		mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetGameText( 4000 ), physicMinAttack, physicMaxAttack );
		mToolTip[mIndex].mColor = eCOLOR_WHITE;
		CalcMaxWidth( mToolTip[mIndex].mString );
		mIndex++;
		if( mIndex > MAX_TIP )
		{
			assert( 0 && "error tip index" );
		}

		/// 마법공격력 수치 & 세팅
		magicMinAttack = pAbil->GetMagicMinAttack();
		magicMaxAttack = pAbil->GetMagicMaxAttack();
		mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetGameText( 4001 ), magicMinAttack, magicMaxAttack );
		mToolTip[mIndex].mColor = eCOLOR_WHITE;
		CalcMaxWidth( mToolTip[mIndex].mString );
		mIndex++;

		if( mIndex > MAX_TIP )
		{
			assert( 0 && "error tip index" );
		}

	}
	/// 방어구 & 액세서리 & 방패류 
	else if( type == ITEM_WEAR || type == ITEM_ACCESSORY || subtype == ITEM_WEAPON_SHIELD )
	{
		/// 물리, 마법 방어력 수치
		physicDefense = pAbil->GetPhysicDefense();
		magicDefense = pAbil->GetMagicDefense();

		/// 물리, 마법 방어력 세팅 
		SetText( GAMERESOURCEMAN->GetGameText( 4004 ), physicDefense, eCOLOR_WHITE );
		SetText( GAMERESOURCEMAN->GetGameText( 4005 ), magicDefense, eCOLOR_WHITE );
	}

	return true;
}

/// 추가수치
bool cTipWindow::SetSpecial( cItemDefine* itemDefine )
{
	unsigned char type = itemDefine->GetType();
	unsigned char subtype = itemDefine->GetSubType();

	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	/// 힘
	if( pAbil->GetStr() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4010 ), pAbil->GetStr(), eCOLOR_WHITE );
	}

	/// 민첩
	if( pAbil->GetDex() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4011 ), pAbil->GetDex(), eCOLOR_WHITE );
	}

	/// 체력
	if( pAbil->GetCon() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4012 ), pAbil->GetCon(), eCOLOR_WHITE );
	}

	/// 지능
	if( pAbil->GetInt() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4013 ), pAbil->GetInt(), eCOLOR_WHITE );
	}

	/// 지혜
	if( pAbil->GetWis() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4014 ), pAbil->GetWis(), eCOLOR_WHITE );
	}

	/// 조건검사 
	if( type == ITEM_WEAR || type == ITEM_ACCESSORY || subtype == ITEM_WEAPON_SHIELD )
	{
		/// 물리공격력
		if( pAbil->GetPhysicMaxAttack() > 0 )
		{
			short attack = ( pAbil->GetPhysicMaxAttack() + pAbil->GetPhysicMinAttack() ) / 2;
			SetText( GAMERESOURCEMAN->GetGameText( 4015 ), attack, eCOLOR_WHITE );
		}
	}

	/// 조건검사 
	if( type == ITEM_WEAR || type == ITEM_ACCESSORY || subtype == ITEM_WEAPON_SHIELD )
	{
		/// 마법공격력
		if( pAbil->GetMagicMaxAttack() > 0 )
		{
			short magicAttack = ( pAbil->GetMagicMaxAttack() + pAbil->GetMagicMinAttack() ) / 2;
			SetText( GAMERESOURCEMAN->GetGameText( 4016 ), magicAttack, eCOLOR_WHITE );
		}
	}

	/// 조건검사
	if( type == ITEM_WEAPON && subtype != ITEM_WEAPON_SHIELD )
	{
		/// 물리방어력
		if( pAbil->GetPhysicDefense() > 0 )
		{
			SetText( GAMERESOURCEMAN->GetGameText( 4004 ), pAbil->GetPhysicDefense(), eCOLOR_WHITE );
		}
	}

	/// 조건검사
	if( type == ITEM_WEAPON && subtype != ITEM_WEAPON_SHIELD )
	{
		/// 마법방어력 
		if( pAbil->GetMagicDefense() > 0 )
		{
			SetText( GAMERESOURCEMAN->GetGameText( 4005 ), pAbil->GetMagicDefense(), eCOLOR_WHITE );
		}
	}

	/// 물리명중률
	if( pAbil->GetPhysicAttackRate() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4017 ), pAbil->GetPhysicAttackRate(), eCOLOR_WHITE );
	}

	/// 마법명중률 
	if( pAbil->GetMagicAttackRate() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4018 ), pAbil->GetMagicAttackRate(), eCOLOR_WHITE );
	}

	/// 물리크리티컬
	if( pAbil->GetPhysicCritical() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4019 ), pAbil->GetPhysicCritical(), eCOLOR_WHITE );
	}

	/// 마법크리티컬
	if( pAbil->GetMagicCritical() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4020 ), pAbil->GetMagicCritical(), eCOLOR_WHITE );
	}

	/// 회피
	if( pAbil->GetDod() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4021 ), pAbil->GetDod(), eCOLOR_WHITE );
	}

	/// 조건검사
	if( type == ITEM_WEAR || type == ITEM_ACCESSORY || subtype == ITEM_WEAPON_SHIELD )
	{
		/// 공격속도 (쿨타임)
		if( pAbil->GetCoolTime() > 0 )
		{
			float coolTime = (float)(pAbil->GetCoolTime() / 1000);
			SetText( GAMERESOURCEMAN->GetGameText( 4022 ), coolTime, eCOLOR_WHITE );
		}
	}

	/// 이동속도
	if( pAbil->GetMove() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4023 ), pAbil->GetMove(), eCOLOR_WHITE );
	}

	/// 최대 HP
	if( pAbil->GetMaxHP() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4024 ), pAbil->GetMaxHP(), eCOLOR_WHITE );
	}

	/// 최대 MP
	if( pAbil->GetMaxMP() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4025 ), pAbil->GetMaxMP(), eCOLOR_WHITE );
	}

	/// HP 회복력 ( % )
	if( pAbil->GetPlusHP() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4026 ), pAbil->GetPlusHP(), eCOLOR_WHITE );
	}

	/// MP 회복력 ( % )
	if( pAbil->GetPlusMP() > 0 )
	{
		SetText( GAMERESOURCEMAN->GetGameText( 4027 ), pAbil->GetPlusMP(), eCOLOR_WHITE );
	}
	return true;
}

///
bool cTipWindow::SetLimit( cItemDefine* itemDefine )
{
	cItemLimit* itemLimit = itemDefine->GetLimit();
	if( itemLimit == 0 )
		return false;

	/// 최소요구레벨 세팅 
	SetText( GAMERESOURCEMAN->GetGameText( 4050 ), itemLimit->GetLevel(), eCOLOR_WHITE );

	/// 사용종족 세팅
	//SetText( GAMERESOURCEMAN->GetGameText( 4051 ), itemLimit->GetRace(), eCOLOR_WHITE );

	/// 사용직업 세팅
	//SetText( GAMERESOURCEMAN->GetGameText( 4052 ), itemLimit->GetJob(), eCOLOR_WHITE );

	/// 사용성별 세팅 
	//SetText( GAMERESOURCEMAN->GetGameText( 4053 ), itemLimit->GetLevel(), eCOLOR_WHITE );

	//HERO->GetRace();
	//HERO->GetGender();
	//HERO->GetLevel();
	//HERO->GetJob();
	return true;
}

///
bool cTipWindow::SetEffect( cItemDefine* itemDefine )
{
	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	long index = pAbil->GetDescription();

	SetText( GAMERESOURCEMAN->GetGameText( 4054 ), GAMERESOURCEMAN->GetGameText( index ), eCOLOR_GREEN );
	return true;
}

/// 재사용시간 
bool cTipWindow::SetUseTime( cItemDefine* itemDefine )
{
	cItemAbility* pAbil = itemDefine->GetAbililty();
	if( pAbil == 0 )
		return false;

	long coolTime = pAbil->GetCoolTime() / 1000;

	SetText( GAMERESOURCEMAN->GetGameText( 4055 ), (short)coolTime, eCOLOR_WHITE );
	return true;
}