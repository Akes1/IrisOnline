#include "stdafx.h"
#include "ItemManager.h"

#include "./util/Tokenizer.h"
#include "UIManager.h"

#include "./client/GameResourceManager.h"
//#include "CoolTimeManager.h"
#include "ItemDefine.h"

#include "./client/ResourceManager.h"

bool cItemManager::LoadAbility( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item ability" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = str.ToInt();
		assert( index > 0 );

		/// 사용효과설명
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long discription = (long)str.ToInt();

		/// 방어구재질
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short defenseType = (short)str.ToInt();

		/// 범위방식
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short rangeType = (short)str.ToInt();

		/// 효과적용타겟
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short targetType = (short)str.ToInt();

		/// 사용거리
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short useRange = (short)str.ToInt();

		/// 물리 방어력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short physicDefense = (short)str.ToInt();

		/// 마법 방어력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short magicDefense = (short)str.ToInt();

		/// 사정거리
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short attackRange = (short)str.ToInt();

		/// 공격 속도
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short attackSpeed = (short)str.ToInt();

		/// 쿨타임
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long coolTime = (long)str.ToInt();

		/// 그룹 쿨타임 번호
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int groupTime = (unsigned int)str.ToInt();

		/// 물리:최소공격력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short physicMinAttack = (short)str.ToInt();

		/// 물리:최대공격력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short physicMaxAttack = (short)str.ToInt();

		/// 마법:최소공격력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short magicMinAttack = (short)str.ToInt();

		/// 마법:최대공격력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short magicMaxAttack = (short)str.ToInt();

 		/// 힐 회복량
 		if( tokenizer.GetNext( &str ) == false )
 			{ goto ERR; }
 
 		short healValue = (short)str.ToInt();

		/// 옵션 값
		sAblityOption opt[5];
		for( int i = 0; i < 5; ++i )
		{
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			opt[i].mOptType = (short)str.ToInt();

			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			opt[i].mOptValue = (short)str.ToInt();
		}

		/// 스킬효과
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long InfluenceIndex = (long)str.ToInt();

		/// 발동효과
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long activeIndex = (long)str.ToInt();
	
		/// 아이템 능력을 생성
		cItemAbility* p = new cItemAbility;
		p->mDescription = discription;
		p->mDefenseType = defenseType;
		p->mRangeType = rangeType;
		p->mTargetType = targetType;
		p->mUseRange = useRange;
		p->mPhysicDefense = physicDefense;
		p->mMagicDefense = magicDefense;
		p->mAttackRange = attackRange;
		p->mAttackSpeed = attackSpeed;
		p->mCoolTime = coolTime;
		p->mGroupNum = groupTime;
		p->mPhysicMinAttack = physicMinAttack;
		p->mPhysicMaxAttack = physicMaxAttack;
		p->mMagicMinAttack = magicMinAttack;
		p->mMagicMaxAttack = magicMaxAttack;
		p->mHealValue      = healValue;
		p->mInfluenceIndex = InfluenceIndex;
		p->mActiveInfluenceIndex = activeIndex;

		for( int i = 0; i < 5; ++i )
		{
			p->mOption[i] = opt[i];
		}

		if( mAbilityMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item ability, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadLimit( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item limit" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = str.ToInt();
		assert( index > 0 );

		/// 캐릭터:종족
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char charRace = (char)str.ToInt();

		/// 캐릭터:성별
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char charGender = (char)str.ToInt();

		/// 직업1
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long job1 = (long)str.ToInt();

		/// 직업2
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long job2 = (long)str.ToInt();

		/// 캐릭터:레벨
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char charLevel = (char)str.ToInt();

		/// 캐릭터:최대레벨
		if( tokenizer.GetNext( &str ) == false )
			return false;

		char charMaxLevel = (char)str.ToInt();

		/// 전장 사용제한
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char usePVPOnly = (char)str.ToInt();

		/// 세력
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char forcetype = (char)str.ToInt();

		/// 보조직업
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long makeSkill = (long)str.ToInt();

		/// 보조직업 경험치
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long makeSkillExp = (unsigned long)str.ToInt();

		/// 아이템 제한을 생성
		cItemLimit* p = new cItemLimit;

		p->mCharRace = charRace;
		p->mCharGender = charGender;
		p->mCharJob1 = job1;
		p->mCharJob2 = job2;
		p->mCharLevel = charLevel;
		p->mCharMaxLevel = charMaxLevel;
		p->mUsePVPOnly = usePVPOnly;
		p->mForceType = forcetype;
		p->mMakeSkill = makeSkill;
		p->mMakeSkillExp = makeSkillExp;

		if( mLimitMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item limit, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadCard( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load tarot define" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();
		assert( index >= 0 );

		/// 판
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char episode = (char)str.ToInt();

		/// 카드넘버
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int cardnum = (unsigned int)str.ToInt();

		/// 희귀도
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char rarelevel = (char)str.ToInt();

		/// 카드타입
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char cardtype = (char)str.ToInt();

		/// 장착위치
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char wearPos = (char)str.ToInt();

		/// 카드이미지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int imageIndex = (unsigned int)str.ToInt();

		/// 데이터 세팅 
		cItemCard* p = new cItemCard;
		p->mEpisode = episode;
		p->mCardNumber = cardnum;
		p->mRareLevel = rarelevel;
		p->mCardType = cardtype;
		p->mWearPos = wearPos;
		p->mImageIndex = imageIndex;

		if( mCardMap.Insert( index, p ) == false )
		{	
			assert( 0 && "failed to insert card define, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;
ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadTarot( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load tarot define" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();
		assert( index >= 0 );

		/// 이미지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int imageIndex = (unsigned int)str.ToInt();
		assert( imageIndex >= 0 );

		/// 고유 번호
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char number = (char)str.ToInt();

		/// 카드 레벨
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char level = (char)str.ToInt();

		/// 정방향수치
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short strvalue = (short)str.ToInt();

		/// 역방향수치
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short invvalue = (short)str.ToInt();

		/// 속성
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char prop = (char)str.ToInt();

		/// 에피소드(판)
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char episode = (char)str.ToInt();

		/// 정방향 해석 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long strdescIdx = (unsigned long)str.ToInt();
		assert( strdescIdx >= 0 );

		/// 역방향 해석 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long invdescIdx = (unsigned long)str.ToInt();
		assert( invdescIdx >= 0 );

		/// 데이터 세팅 
		cItemTarot* p = new cItemTarot;

		p->mImageIndex = imageIndex;
		p->mNumber = number;
		p->mLevel = level;
		p->mStraightValue = strvalue;
		p->mInverseValue = invvalue;
		p->mProperty = prop;
		p->mEpisode = episode;
		p->mStraightIdx = strdescIdx;
		p->mInverseIdx = invdescIdx;

		if( mTarotMap.Insert( index, p ) == false )
		{	
			assert( 0 && "failed to insert tarot define, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;
ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadSpread( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load spread define" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		long index = 0;
		cItemSpread* p = new cItemSpread;

		for( int i = 0; i < 10; ++i )
		{
			/// 스프레드 인덱스
			if( tokenizer.GetNext( &str ) == false )
			{
				assert( 0 && "spread must 10 list" );;
				{ goto ERR; }
			}

			index = (long)str.ToInt();
			assert( index >= 0 );

			/// 이미지 인덱스 
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			int imageIdx = (int)str.ToInt();

			/// 좌표 인덱스 
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			long axisIndex = (long)str.ToInt();
			assert( axisIndex >= 0 );

			/// x 좌표 
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			unsigned int x = (unsigned int)str.ToInt();

			/// y 좌표 
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			unsigned int y = (unsigned int)str.ToInt();

			/// 방향
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			int dir = (int)str.ToInt();

			/// 계산식 
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			unsigned int calc = 0;
			if( !str.CompareNoCase("*") )
			{
				calc = 1;
			}
			else if( !str.CompareNoCase("/") )
			{
				calc = 2;
			}
			else if( !str.CompareNoCase("+") )
			{
				calc = 3;
			}

			/// 계산수치
			if( tokenizer.GetNext( &str ) == false )
				{ goto ERR; }

			int value = (int)str.ToInt();

			/// 데이터 삽입
			p->mParam[i].mImageIndex = imageIdx;
			p->mParam[i].mX = x;
			p->mParam[i].mY = y;
			p->mParam[i].mDir = dir;
			p->mParam[i].mCalc = calc;
			p->mParam[i].mValue = value;

			if( axisIndex < 1 || axisIndex > 10 )
			{
				assert( 0 && "error axis index" );
				{ goto ERR; }
			}
		}

		if( mSpreadMap.Insert( index, p ) == false )
		{	
			assert( 0 && "failed to insert spread define, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}


/// 분해 관련 데이터 로딩
bool cItemManager::LoadDisjoint( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item disjoint" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();
		assert( index > 0 );

		/// 비용
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long price = (long)str.ToInt();

		/// 아이템1
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx1 = (long)str.ToInt();

		/// 아이템1 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count1 = (int)str.ToInt();

		/// 아이템2
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx2 = (long)str.ToInt();

		/// 아이템2 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count2 = (int)str.ToInt();

		/// 아이템3
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx3 = (long)str.ToInt();

		/// 아이템3 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count3 = (int)str.ToInt();

		/// 아이템4
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx4 = (long)str.ToInt();

		/// 아이템4 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count4 = (int)str.ToInt();

		/// 아이템5
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx5 = (long)str.ToInt();

		/// 아이템5 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count5 = (int)str.ToInt();

		/// 아이템6
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long itemIdx6 = (long)str.ToInt();

		/// 아이템6 수량
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		int count6 = (int)str.ToInt();

		/// 
		sItemDisjoint* p = new sItemDisjoint;

		p->mPrice = price;
		p->mDisjoint[0].itemIndex = itemIdx1;
		p->mDisjoint[0].count	  = count1;
		p->mDisjoint[1].itemIndex = itemIdx2;
		p->mDisjoint[1].count	  = count2;
		p->mDisjoint[2].itemIndex = itemIdx3;
		p->mDisjoint[2].count	  = count3;
		p->mDisjoint[3].itemIndex = itemIdx4;
		p->mDisjoint[3].count	  = count4;
		p->mDisjoint[4].itemIndex = itemIdx5;
		p->mDisjoint[4].count	  = count5;
		p->mDisjoint[5].itemIndex = itemIdx6;
		p->mDisjoint[5].count	  = count6;

		if( mDisjointMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item disjoint, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;

}

bool cItemManager::LoadEnhanced( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item enhanced" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// index 구분자
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		if( str.CompareNoCase("Index") != 0 )
			{ goto ERR; }

		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }
				
		long index = (long)str.ToInt();

		cEnhancedOptionArr* pEnhancedOption = new cEnhancedOptionArr;
		
		while( tokenizer.GetNext( &str ) )
		{			
			if( str.CompareNoCase("End") == 0 )
				break;

			sItemEnhanced* p = new sItemEnhanced;

			p->mEnhancedType = (unsigned char)str.ToInt();

			/// 증가값
			for( unsigned int i = 0; i < MAX_ITEM_ENHANCED; ++i )
			{
				if( tokenizer.GetNext( &str ) == false )
					{ goto ERR; }

				p->mValue[i] = (short)str.ToInt();
			}

			pEnhancedOption->PushBack( p );
		}

		///
		if( mEnhancedMap.Insert( index, pEnhancedOption ) == false )
		{
			assert( 0 && "failed to insert item enhanced, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

/// 카드 슬롯 로딩
bool cItemManager::LoadCardSlot( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load card slot" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();

		/// 슬롯 타입1
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short type1 = (short)str.ToInt();

		/// 슬롯 타입2
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short type2 = (short)str.ToInt();

		/// 슬롯 타입3
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short type3 = (short)str.ToInt();

		/// 슬롯 타입4
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short type4 = (short)str.ToInt();

		/// 슬롯 타입5
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short type5 = (short)str.ToInt();

		/// 데이터 삽입
		sItemPutCardSlot* p = new sItemPutCardSlot;
		p->mSlotType[0] = type1;
		p->mSlotType[1] = type2;
		p->mSlotType[2] = type3;
		p->mSlotType[3] = type4;
		p->mSlotType[4] = type5;

		///
		if( mPutCardSlotMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item putcard, maybe already exist" );
			{ goto ERR; }
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadDefine( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item define" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();
		assert( index > 0 );

		/// 이름
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		/// 이름 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int nameIndex = (unsigned int)str.ToInt();

		/// 툴팁 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int tipIndex = (unsigned int)str.ToInt();

		/// 종류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned char type = (unsigned char)str.ToInt();

		/// 세부 종류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned char subType = (unsigned char)str.ToInt();

		/// 검색 대분류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int highType = (unsigned int)str.ToInt();

		/// 검색 중분류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int middleType = (unsigned int)str.ToInt();

		/// 검색 소분류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int lowType = (unsigned int)str.ToInt();

		/// 무게
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short weight = (short)str.ToInt();

		/// 겹침갯수
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short capacity = (short)str.ToInt();

		/// 판매방법
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long sellType = (unsigned long)str.ToInt();

		/// 판매가격
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long sellPrice = (unsigned long)str.ToInt();

		/// 구입방법
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long buyType = (unsigned long)str.ToInt();

		/// 구입가격
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long buyPrice = (unsigned long)str.ToInt();

		/// 최대 출현개수
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long presentNum = (unsigned long)str.ToInt();

		/// 보유 가능갯수
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int maxHaveInven = (unsigned int)str.ToInt();

		// 기간제 설정
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		// 사용기한
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		// 인증여부
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		/// 사용자거래 여부(0거래가능/1거래불가)
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char exchange = (char)str.ToInt();

		/// 능력 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long abilityIndex = (long)str.ToInt();

		/// 제한 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long limitIndex = (long)str.ToInt();

		/// 카드 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		//long cardIndex = (long)str.ToInt();

		/// 타로 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long tarotIndex = (long)str.ToInt();

		/// 스프레드 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long spreadIndex = (long)str.ToInt();

		/// 분해 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long disjointIndex = (long)str.ToInt();

		/// 카드슬롯 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long cardSlotIndex = (long)str.ToInt();

		/// 강화확률 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		//long enhanceRateIndex = (long)str.ToInt();
		(long)str.ToInt();

		/// 강화 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long enhanceIndex = (long)str.ToInt();

		/// 체인지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		/// 호칭 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long titleIndex = (long)str.ToInt();

		/// 세트 인덱스
		if( tokenizer.GetNext( &str) == false )
			{ goto ERR; }

		long setIndex = (long)str.ToInt();

		/// 아이콘 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int iconIndex = (unsigned int)str.ToInt();

		/// 모델 인덱스0
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int fileIndex0 = (unsigned int)str.ToInt();

		/// 모델 인덱스1
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int fileIndex1 = (unsigned int)str.ToInt();

		/// 투구 착용시 귀 표시여부 (0:귀표시, 1:귀표시 안함)
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char hideEar = (char)str.ToInt();

		/// 아이템 툴팁컬러 ( 레어도관련 )
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned short tooltipColor = (unsigned short)str.ToInt();

		// 홈페이지 공개여부
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		/// 아이템 정의를 생성
		cItemDefine* p = new cItemDefine;

		//p->mName = GetItemName( nameIndex );
		//p->mDescription = ( tipIndex > 0 ) ? GAMERESOURCEMAN->GetItemToolTipText( tipIndex ) : 0;
		p->mType = type;
		p->mSubType = subType;
		p->mHighType = highType;
		p->mMiddleType = middleType;
		p->mLowType = lowType;
		p->mWeight = weight;
		p->mCapacity = capacity;
		p->mSellType = sellType;
		p->mSellPrice = sellPrice;
		p->mBuyType = buyType;
		p->mBuyPrice = buyPrice;
		p->mPresentNum = presentNum;
		p->mMaxHaveInven = maxHaveInven;
		p->mExchange = exchange;
		p->mTitleIndex = titleIndex;
		p->mSetIndex = setIndex;
// 		p->mAbility = GetItemAbility( abilityIndex );
 		p->mLimit = GetItemLimit( limitIndex );
// 		//p->mCard = GetItemCard( cardIndex );
// 		p->mTarot = GetItemTarot( tarotIndex );
// 		p->mSpread = GetItemSpread( spreadIndex );
// 		p->mDisjoint = GetItemDisjoint( disjointIndex );
// 		p->mEnhanceArr = GetItemEnhanced( enhanceIndex );
// 		p->mPutCardSlot = GetItemPutCardSlot( cardSlotIndex );
 		p->mIconParam = UIMAN->GetIconParam( iconIndex );
 		p->mFileNames0 = GetItemFileNames( fileIndex0 );
 		p->mFileNames1 = GetItemFileNames( fileIndex1 );
		p->mHideEar = hideEar;
		p->mTooltipColor = tooltipColor;

		if( p->mIconParam == 0 )
		{
			assert( 0 && "failed to find icon param" );
			{ goto ERR; }
		}
		if( p->mLimit == 0 )
		{
			assert( 0 && "failed to find item limit" );
			{ goto ERR; }
		}

		/// 장착 타입을 설정
		if( type == ITEM_WEAPON )
		{
			switch( subType )
			{
				/// 한손검
			case ITEM_WEAPON_SWORD:
				p->mEquipType = ITEM_EQUIP_ONE_HAND;

				if( p->mFileNames0 == 0 )
				{
					assert( 0 && "failed to find left item file name" );
					{ goto ERR; }
				}
				break;
	
				/// 양손검, 단검, 총, 지팡이, 이도류
			case ITEM_WEAPON_BLADE:
			case ITEM_WEAPON_GUN:
			case ITEM_WEAPON_STAFF:
				p->mEquipType = ITEM_EQUIP_TWO_HANDS;

				if( p->mFileNames0 == 0 )
				{
					assert( 0 && "failed to find left item file name" );
					{ goto ERR; }
				}
				break;

			case ITEM_WEAPON_DUAL:
			case ITEM_WEAPON_CUTTER:
				p->mEquipType = ITEM_EQUIP_TWO_HANDS;

				if( p->mFileNames0 == 0 )
				{
					assert( 0 && "failed to find left item file name" );
					{ goto ERR; }
				}

				if( p->mFileNames1 == 0 )
				{
					assert( 0 && "failed to find right item file name" );
					{ goto ERR; }
				}
				break;

				/// 방패
			case ITEM_WEAPON_SHIELD:
				p->mEquipType = ITEM_EQUIP_SHIELD;
				break;
			}
		}

		/// 플랙값을 설정
		switch( type )
		{
		case ITEM_WEAPON:
			switch( subType )
			{
			case ITEM_WEAPON_SWORD:
				p->mFlag = ITEM_FLAG_SWORD;
				break;
			case ITEM_WEAPON_BLADE:
				p->mFlag = ITEM_FLAG_BLADE;
				break;
			case ITEM_WEAPON_DUAL:
				p->mFlag = ITEM_FLAG_DUAL;
				break;
			case ITEM_WEAPON_CUTTER:
				p->mFlag = ITEM_FLAG_CUTTER;
				break;
			case ITEM_WEAPON_GUN:
				p->mFlag = ITEM_FLAG_GUN;
				break;
			case ITEM_WEAPON_STAFF:
				p->mFlag = ITEM_FLAG_STAFF;
				break;
			case ITEM_WEAPON_SHIELD:
				p->mFlag = ITEM_FLAG_SHIELD;
				break;
			default:
				{
					assert( 0 && "invalid item weapon subtype" );
					goto ERR; 
				}
			}
			break;

		case ITEM_WEAR:
			switch( subType )
			{
			case ITEM_WEAR_HEAD:
				p->mFlag = ITEM_FLAG_HEAD;
				break;
			case ITEM_WEAR_UPPER:
				p->mFlag = ITEM_FLAG_UPPER;
				break;
			case ITEM_WEAR_LOWER:
				p->mFlag = ITEM_FLAG_LOWER;
				break;
			case ITEM_WEAR_HANDS:
				p->mFlag = ITEM_FLAG_HANDS;
				break;
			case ITEM_WEAR_FEET:
				p->mFlag = ITEM_FLAG_FEET;
				break;
			case ITEM_WEAR_ONEPIECE:
				p->mFlag = ITEM_FLAG_ONEPIECE;
				break;
			case ITEM_WEAR_COS_ONEPIECE:
				p->mFlag = ITEM_FLAG_COSTUME_UPPER;
				break;
			case ITEM_WEAR_COS_HEAD:
				p->mFlag = ITEM_FLAG_COSTUME_HEAD;
				break;
			case ITEM_WEAR_COS_UPPER:
				p->mFlag = ITEM_FLAG_COSTUME_UPPER;
				break;
			case ITEM_WEAR_COS_LOWER:
				p->mFlag = ITEM_FLAG_COSTUME_LOWER;
				break;
			case ITEM_WEAR_COS_HANDS:
				p->mFlag = ITEM_FLAG_COSTUME_HANDS;
				break;
			case ITEM_WEAR_COS_FEET:
				p->mFlag = ITEM_FLAG_COSTUME_FEET;
				break;
			default:
				{
					assert( 0 && "invalid item wear subtype" );
					goto ERR; 
				}
			}
			break;

		case ITEM_ACCESSORY:
			switch( subType )
			{
			case ITEM_ACCESSORY_EARRING:
				p->mFlag = ITEM_FLAG_EARRING;
				break;
			case ITEM_ACCESSORY_NECKLACE:
				p->mFlag = ITEM_FLAG_NECKLACE;
				break;
			case ITEM_ACCESSORY_BROOCH:
				p->mFlag = ITEM_FLAG_BROOCH;
				break;
			case ITEM_ACCESSORY_BRACELET:
				p->mFlag = ITEM_FLAG_BRACELET;
				break;
			case ITEM_ACCESSORY_RING:
				p->mFlag = ITEM_FLAG_RING;
				break;
			case ITEM_ACCESSORY_COS_FACE:
				p->mFlag = ITEM_FLAG_COSTUME_FACE;
				break;
			case ITEM_ACCESSORY_COS_BACK:
				p->mFlag = ITEM_FLAG_COSTUME_BACK;
				break;
			default:
				{
					assert( 0 && "invalid item accessory subtype" );
					goto ERR; 
				}
			}
			break;

		case ITEM_CARD:	/// 캐릭터 장착 카드
			p->mFlag = ( subType == ITEM_CARD_EQUIP_C ) ? ITEM_FLAG_EQUIPCARD : ITEM_FLAG_UNKNOWN;
			break;
		default:
			p->mFlag = ITEM_FLAG_UNKNOWN;
			break;
		}

		/// 카드장착 가능 슬롯 카운트 세기
		sItemPutCardSlot* slot = p->mPutCardSlot;
		if( slot )
		{
			for( int i = 0; i < MAX_CARD_SLOT; ++i )
			{
				/// 장착가능한 슬롯 세기
				if( slot->mSlotType[i] > 0 )
					p->mPutSlotCount++;
			}
		}

		if( mDefineMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item define, maybe already exist" );
			{ goto ERR; }
		}

		/// 세트 정보 삽입
		if( p->mSetIndex != 0 )
		{
			cItemSetInfo* setInfo = (cItemSetInfo*)mSetInfoMap.GetAt( p->mSetIndex );
			if( setInfo )
			{
				setInfo->AddItem( index );
			}
			else
			{
				assert(0);
				goto ERR;
			}
		}
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}

bool cItemManager::LoadItemSet( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item set info" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		long index = (long)str.ToInt();
		assert( index > 0 );

		/// 갯수
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int count = (unsigned int)str.ToInt();

		/// 세트 이름 
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned int nameIdx = (unsigned int)str.ToInt();

		/// 옵션
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short optType1 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short optValue1 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short optType2 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short optValue2 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		unsigned long actIdx = str.ToInt();

		cItemSetInfo* setInfo = (cItemSetInfo*)mSetInfoMap.GetAt(index);
		if( setInfo == 0 )
		{
			setInfo = new cItemSetInfo;
			if( setInfo == 0 || mSetInfoMap.Insert( index, setInfo ) == false )
			{
				assert(0);
				continue;
			}
		}

		/// 이름정보
		setInfo->mSetNameIdx = nameIdx;

		/// 갯수별 옵션정보
		sSetAblityOption* opt = new sSetAblityOption;
		opt->mOptType[0] = optType1;
		opt->mOptValue[0] = optValue1;
		opt->mOptType[1] = optType2;
		opt->mOptValue[1] = optValue2;

		opt->mActIdx = actIdx;

		/// opt정보 셋팅
		setInfo->AddOption( count, opt );
	}

	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;

}


bool cItemManager::LoadFileName( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item file name" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	unsigned int iend = eRACE_MAX * eGENDER_MAX;

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();
		assert( index > 0 );

		iend = eRACE_MAX * eGENDER_MAX;

		/// 아이템 파일 이름 배열을 생성
		cString* p = new cString[iend];

		for( unsigned int i = 0; i < iend; ++i )
		{
			if( tokenizer.GetNext( &(p[i]) ) == false )
			{
				delete [] p;
				cString msg;
				msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
				MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
				return false;
			}
			else
			{
				// 캐릭터 아이템 이미지 로딩 루틴 추가 [12/14/2009 Jo_Client]
				if( ::_strcmpi( p[i].Cstr(), "None" ) != 0 )
				{
					cString path;
					path.Format( "./Data/Character/%s", p[i].Cstr() );
					if( RESOURCEMAN->LoadNIF( path, false ) == false )
					{
						assert(0);
						cString msg;
						msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
						MessageBoxA( NULL, msg.Cstr(), "fail to load nif", MB_OK | MB_ICONERROR );
					}
				}
			}
		}

		if( mFileNamesMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item file names, maybe already exist" );
			cString msg;
			msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
			MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
			return false;
		}
	}
	return true;
}

/// 판매대행 시스템 카테고리 정보
bool cItemManager::LoadDealCategoryTable( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item file name" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 대분류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		char highKey = (char)str.ToInt();

		/// 중분류
		if( tokenizer.GetNext( &str ) == false )
			{ goto ERR; }

		short midKey = (short)str.ToInt();

		sDealCategory* data = new sDealCategory;
		data->mHighKey		= highKey;
		data->mMiddleKey	= midKey;

		cDealArr* middleArr = (cDealArr*)mDealCategoryMap.GetAt( highKey );
		if( middleArr == 0 )
		{
			middleArr = new cDealArr;
			if( mDealCategoryMap.Insert( highKey, middleArr ) == false )
			{
				assert(0);
				delete data;
				continue;
			}

			mDealHighArr.PushBack( highKey );
		}
		middleArr->PushBack( data );
	}
	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;

}

///
void cItemManager::LoadCoolData()
{
	cAbilityMap::cIterator i = mAbilityMap.Begin();
	cAbilityMap::cIterator end = mAbilityMap.End();

	for( ; i != end; ++i )
	{
		cItemAbility* p = (cItemAbility*)(i->mSecond);
		if( !p )
		{
			assert(0);
			continue;
		}

		/// 쿨타임 데이터 세팅
		if( p->mCoolTime > 0 )
		{
			unsigned long itemIndex = (unsigned long)(i->mFirst);
//			COOLMAN->LoadData( itemIndex, p->mGroupNum, p->mCoolTime );
		}
	}
}
