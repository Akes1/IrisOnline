#include "stdafx.h"
#include "gamesrv.h"
#include "Party.h"
#include "PartyManager.h"
#include "RangeCheck.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Monster.h"
#include "Drop.h"
#include "DropScript.h"
#include "RandomTable.h"
#include "GridManager.h"

cParty::cParty( )
: mIndex(0)
, mDivideType( PARTY_DIVIDE_NONE )
, mCount(0)
, mLeaderIndex(0)
, mOwnUserNum(-1)
, mDropMoney(0)
, mDropExp(0)
, mDropSxp(0)
, mMonsterMaxHP(0)
, mDamage(0)
, mLastAccumTime(0)
{
	/// 유저 인덱스 초기화
	memset( mUserArr, 0, sizeof(mUserArr) );
	memset( mUserDat, 0, sizeof(mUserDat) );
	memset( mUserTsp, 0, sizeof(mUserTsp) );
	DiceRelease( );
}

cParty::~cParty( )
{
	DiceRelease( );
}

// operator new - 할당
void* cParty::operator new(size_t n)
{
	if ( n == sizeof(cParty) )
		return PARTYMAN->AllocParty( );
	else
		assert(0);

	return NULL;
}

// operator delete - 해제
void cParty::operator delete(void* ptr, size_t n)
{
	if ( ptr != NULL )
	{
		if ( n == sizeof(cParty) )
			PARTYMAN->FreeParty( static_cast<cParty*>(ptr) );
		else
			assert(0);
	}
	else
		assert(0);
}

void cParty::Init(unsigned long index, unsigned long leaderIdx, PARTY_DIVIDE_TYPE type, unsigned long accumTime)
{
	mIndex         = index;
	mLeaderIndex   = leaderIdx;
	mDivideType    = type;
	mLastAccumTime = accumTime;
}

bool cParty::IsUser(unsigned long userIdx)
{
	unsigned long* ptr = mUserArr;
	unsigned int   cnt = mCount;

	while ( cnt > 0 )
	{
		if ( (*ptr) == userIdx )
			return true;

		cnt--;
		ptr++;
	}
	return false;
}

bool cParty::AddUser(unsigned long userIdx)
{
	if ( mCount < MAX_PARTYUSER )
	{
		mUserArr[ mCount ] = userIdx;	/// 삽입
		mCount++;						/// 증가
		return true;					/// 파티레벨 다시 갱신하기.
	}
	return false;
}

bool cParty::DeleteUser(unsigned long userIdx)
{
	if ( mCount > 0 )
	{
		int index = -1;
		for ( unsigned int i = 0; i < mCount; ++i )
		{
			if ( mUserArr[ i ] == userIdx )
			{
				index = i;
				break;
			}
		}
		if ( index > -1 )
		{
			mCount--;	/// 감소
			for ( unsigned int i = index; i < mCount; ++i )
			{
				mUserArr[ i ] = mUserArr[ i+1 ];
				mUserDat[ i ] = mUserDat[ i+1 ];
				mUserTsp[ i ] = mUserTsp[ i+1 ];
			}
			mUserArr[ mCount ] = 0;
			memset( &mUserDat[ mCount ], 0, sizeof(sPartyUserData) );
			mUserTsp[ mCount ] = 0;

			/// 주사위 굴리기 참여자 포기 설정
			if ( mDiceEndTime > 0 )
				SetDiceRoll( userIdx, 0 );

			return true;
		}
	}
	return false;
}

bool cParty::SetUserData(sPartyUserData* userData, unsigned long tickCount)
{
	unsigned long* ptr = mUserArr;

	for ( unsigned int i = 0; i < mCount; ++i, ++ptr )
	{
		if ( (*ptr) == userData->userIndex )
		{
			mUserDat[ i ] = (*userData);
			mUserTsp[ i ] = tickCount;
			return true;
		}
	}
	return false;
}

bool cParty::SetUserData(cPlayer* player, unsigned long tickCount)
{
	unsigned long* ptr       = mUserArr;
	unsigned long  playerIdx = player->GetObjectID( );

	for ( unsigned int i = 0; i < mCount; ++i, ++ptr )
	{
		if ( (*ptr) == playerIdx )
		{
			player->GetPartyUserData( &mUserDat[ i ] );
			mUserTsp[ i ] = tickCount;
			return true;
		}
	}
	return false;
}

unsigned long cParty::NextLeader( )
{
	unsigned long* userArr = mUserArr;
	unsigned int   index   = 0;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		if ( mLeaderIndex == (*userArr) )
		{
			index = (i + 1) % mCount;	// 인원수를 넘어가면 첫번째 유저로 바굼
			return mUserArr[ index ];
		}
	}

	return 0;
}

unsigned long cParty::GetTurnUser(unsigned short mapNumber, float x, float y)
{
	NiPoint2 monPos( x, y );

	for ( unsigned int i = 0; i < mCount; ++i )
	{
		/// 그다음 유저로 증가
		mOwnUserNum++;

		if ( mOwnUserNum >= (int)mCount )
			mOwnUserNum = 0;

		/// 해당유저 찾기
		cPlayer* user = OBJECTMANAGER->GetPlayer( mUserArr[ mOwnUserNum ] );
		if ( user != NULL )	
		{
			if ( mapNumber != user->GetMapNumber() )
				continue;

			NiPoint2 playerPos = user->GetPos( ); // ( user->GetXPos(), user->GetYPos() );

			cRangeCheck rangeCheck( PARTY_RANGE );
			if ( rangeCheck.IsRange( monPos, playerPos ) == true )
			{
				return mUserArr[ mOwnUserNum ];
			}
		}
	}

	return 0;
}

unsigned long cParty::GetRandomUser(unsigned short mapNumber, float x, float y)
{
	NiPoint2      monPos( x, y );
	unsigned long tempArr[ MAX_PARTYUSER ] = {0,};
	int           count                    = 0;

	/// 사정거리안에 있는 유저만 따로 모은다
	for ( unsigned int i = 0; i < mCount; ++i )
	{
		cPlayer* user = OBJECTMANAGER->GetPlayer( mUserArr[ i ] );
		if ( user != NULL )
		{
			if ( mapNumber != user->GetMapNumber( ) )
				continue;

			NiPoint2 playerPos = user->GetPos( ); // ( user->GetXPos(), user->GetYPos() );

			cRangeCheck rangeCheck( PARTY_RANGE );
			if ( rangeCheck.IsRange( monPos, playerPos ) == true )
			{
				tempArr[ count ] = mUserArr[ i ];
				count++;
			}
		}
	}
	if ( count == 0 )
	{
		assert(0);
		return 0;
	}

	/// 랜덤으로 뽑기
	int random  = rand() % count;
	mOwnUserNum = tempArr[ random ];
	return mOwnUserNum;
}

void cParty::DivideDropValue(unsigned long monsterIdx, unsigned char monsterLevel)
{
	unsigned long dropMoney    = FloatToInt( mDropMoney );
	unsigned long dropExp      = FloatToInt( mDropExp );
	unsigned long dropSxp      = FloatToInt( mDropSxp );
	unsigned long damage       = mDamage;
	unsigned long monsterMaxHP = mMonsterMaxHP;

	mDropMoney    = 0;
	mDropExp      = 0;
	mDropSxp      = 0;
	mMonsterMaxHP = 0;
	mDamage       = 0;

	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if ( pMonster == NULL )
	{
		assert(NULL);
		return;
	}

	/// 몬스터에 파티전체가 공격한 데미지가 hp의 1%미만이면 보상 없음
	if ( monsterMaxHP * 0.01f > damage )
		return;

	unsigned short mapNumber = pMonster->GetMapNumber( );
	NiPoint2       monPos    = pMonster->GetPos( );

	/// 경험치를 분배해줄 파티원 목록
	unsigned long dropUserIdx[MAX_PARTYUSER] = { 0, };
	unsigned char dropUserCnt = 0;      /// 드랍 유저 수
    unsigned char calcUserCnt = 0;      /// 파티 증감치 계산 B식의 파티원 수
	unsigned char maxLevel    = 0;		/// 파티 레벨

	NiPoint2 playerPos = NiPoint2::ZERO;
	cRangeCheck rangeCheck( PARTY_RANGE );

    /// 파티 최고 레벨 검색
	cPlayer* pPlayer = NULL;
	for ( unsigned int i = 0; i < mCount; ++i )
	{
		pPlayer = OBJECTMANAGER->GetPlayer( mUserArr[ i ] );
		if ( pPlayer == NULL )
			continue;

		playerPos = pPlayer->GetPos();

		/// 위치 확인
		if ( mapNumber != pPlayer->GetMapNumber( ) || rangeCheck.IsRange( monPos, playerPos ) != true )
		{
			/// 위치가 틀리고 공격도 안했다면 드롭대상에서 빼버림
			if ( pMonster->GetTakeDamage( mUserArr[ i ] ) == 0 )
				continue;
		}

		if ( maxLevel < pPlayer->GetLevel( ) )
			maxLevel = pPlayer->GetLevel( );
    }


	float totalValue = 0;
	float value104   = DROPSCRIPT->GetPartyCalcInfo( 104 );

	/// 전체 파티원 검색 - 위치, 몬스터 공격 해당 파티원 리스트만 추려냄
	for ( unsigned int i = 0; i < mCount; ++i )
	{
		pPlayer = OBJECTMANAGER->GetPlayer( mUserArr[ i ] );
		if ( pPlayer == NULL )
			continue;

		playerPos = pPlayer->GetPos();

		/// 위치 확인
		if ( mapNumber != pPlayer->GetMapNumber( ) || rangeCheck.IsRange( monPos, playerPos ) != true )
		{
			/// 위치가 틀리고 공격도 안했다면 드롭대상에서 빼버림
			if ( pMonster->GetTakeDamage( mUserArr[ i ] ) == 0 )
				continue;
		}

        /// 6레벨 이상 차이가 나는 유저는 계산하지 않는다.
        if( (maxLevel - 6) < pPlayer->GetLevel() )
            ++calcUserCnt;
        
        /// 드롭 해당 유저 카운트와 파티 레벨 설정
        dropUserIdx[ dropUserCnt ] = mUserArr[ i ];
        ++dropUserCnt;

		/// 경험치 계산에 쓸 내용 (파티적용범위에 있는 유저만 체크 )
		totalValue += pow( pPlayer->GetLevel( ), value104 );
	}

	/// 몬스터를 공격 했던 파티가 몬스터가 죽기전에 다 떠남
	if ( dropUserCnt == 0 )
		return;

	if ( totalValue == 0 )
	{
		assert( NULL );
		return;
	}

	/// 레벨차 증감치 계산
	float calcExp = DROP->MonsterDropExpIncrease( (float)dropExp, maxLevel, monsterLevel );
	float calcSxp = DROP->MonsterDropExpIncrease( (float)dropSxp, maxLevel, monsterLevel );

/*------------------------------------------------------------------------------------*/
	/// 파티 증감치 계산
	// A = [몬스터 경험치] 
	// B = ( ([몬스터 경험치] / {NUM101} * {NUM102}) * (파티원 수 - 1))
	// C = 자신의 레벨 ^ {NUM103} / (파티원의 레벨 ^ {NUM104} 의 합)
	// D = A * C (기본 경험치)
	// E = B * C (파티인원에 따른 추가 경험치)
	// F = D + E	최대 A * 0.9
/*------------------------------------------------------------------------------------*/

	float value101 = DROPSCRIPT->GetPartyCalcInfo( 101 );
	float value102 = DROPSCRIPT->GetPartyCalcInfo( 102 );
	float value103 = DROPSCRIPT->GetPartyCalcInfo( 103 );

	/// 선정된 플레이어에게 드롭
	for ( unsigned char i = 0 ; dropUserCnt > i ; ++i )
	{
		pPlayer = OBJECTMANAGER->GetPlayer( dropUserIdx[ i ] );
		if ( pPlayer == NULL || pPlayer->GetStateDie( ) == true )
			continue;

		playerPos = pPlayer->GetPos( ); // { playerPos.x = pPlayer->GetXPos( ); playerPos.y = pPlayer->GetYPos( ); }

		/// 드롭 대상 이지만 드롭 분배 당시 멀리 있으면 대상에서 제외
		if ( mapNumber != pPlayer->GetMapNumber( ) || rangeCheck.IsRange( monPos, playerPos ) != true )
			continue;

		/// 개인 경험치 검사
		{
			float eachValue = pow( pPlayer->GetLevel(), value103 );

			// B = ( ([몬스터 경험치] / {NUM101} * {NUM102}) * (파티원 수 - 1))
			float resultB = ( calcExp / value101 * value102 ) * (calcUserCnt - 1);	// 실제로 적용받을 인원수
			float resultC = 1.0f;
			if ( dropUserCnt > 1 )
			{
				// C = 자신의 레벨 ^ {NUM103} / (파티원의 레벨 ^ {NUM104} 의 합)
				resultC = eachValue / totalValue;
			}

			float resultD = calcExp * resultC;
			float resultE = resultB * resultC;

			/// 최대치 지정
			if ( (resultD + resultE) > calcExp )
			{
				resultD = (float)(resultD * 0.9);
				resultE = (float)(resultE * 0.9);
			}

			pPlayer->AddPartyExp( (unsigned long)(resultD + resultE), true, false, (unsigned long)resultD, (unsigned long)resultE );

		}

		/// 개인 스킬 경험치 검사
		{
			float eachValue = pow( pPlayer->GetLevel(), value103 );
			float resultB   = ( calcSxp / value101 * value102 ) * (calcUserCnt -1 );	// 실제로 적용받을 인원수
			float resultC   = 1.0f;
			if ( dropUserCnt > 1 )
			{
				resultC = eachValue / totalValue;
			}

			float resultD = calcSxp * resultC;
			float resultE = resultB * resultC;

			/// 최대치 지정
			if ( (resultD + resultE) > calcSxp )
			{
				resultD = (float)(resultD * 0.9);
				resultE = (float)(resultE * 0.9);
			}

			pPlayer->AddPartySxp( (unsigned long)(resultD + resultE), true, (unsigned long)resultD, (unsigned long)resultE );
		}

		/// money는 균등 분배
		unsigned long money = dropMoney / dropUserCnt;
		if ( money != 0 )
		{
			float addMoneyPer = pPlayer->GetStatusEtc()->mMoneyAddPer;
			if( addMoneyPer < 0.0f )
			{
				NETWORK2->PostServerEvent("cParty::DivideDropValue addMoneyPer < 0.0f[%f]", addMoneyPer );				
				pPlayer->AddMoney( pMonster->GetObject(), money );
			}
			else
			{
				unsigned long addMoney = (unsigned long)(money + ( (float)money * addMoneyPer / 100.0f ));
				pPlayer->AddMoney( pMonster->GetObject(), addMoney );
			}
		}
	}
}

void cParty::AddDropValue(unsigned long maxHP, unsigned long damage, float money, float exp, float sxp)
{
	mDropMoney    = mDropMoney + money;
	mDropExp      = mDropExp + exp;
	mDropSxp      = mDropSxp + sxp;
	mMonsterMaxHP = maxHP;
	mDamage       = mDamage + damage;
}

void cParty::DiceRelease( )
{
    mDiceEndTime = 0;
    mDiceCount   = 0;
    mDiceRollCnt = 0;

    memset( mDiceMember, 0, sizeof(mDiceMember) );
}

bool cParty::DiceStart( )
{
    if ( mDiceEndTime > 0 )
    {
        //NETWORK2->PostServerEvent("WARNNING cParty::DiceStart - already start");
        return false;
    }

    for ( unsigned int i = 0; i < mCount; i++ )
    {
        if ( mUserArr[i] > 0 )
        {
            cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mUserArr[i] );
            if ( pPlayer != NULL )
			{
				pPlayer->SendPartyDiceStart( );

				if ( mDiceCount < MAX_PARTYUSER )
				{
					mDiceMember[mDiceCount].mUserIdx = mUserArr[i];
					mDiceMember[mDiceCount].mRandom  = 0;
					mDiceMember[mDiceCount].mRoll    = false;

					wcscpy( mDiceMember[mDiceCount].mCharName, mUserDat[ i ].name );
					++mDiceCount;
				}
			}
        }
    }

    mDiceEndTime = NETWORK2->GetAccumTime( ) + PARTY_DICETIME;
    return true;
}

sDiceMember* cParty::GetDiceMember(unsigned long userIdx)
{
    for ( unsigned int i = 0; i < mDiceCount; i++ )
    {
        if ( mDiceMember[i].mUserIdx == userIdx )
            return &mDiceMember[i];
    }
    return NULL;
}

int cParty::DiceRollReq(unsigned long userIdx, unsigned short randVal)
{
    /// 주사위 굴리기 진행 여부
    if ( mDiceEndTime == 0 )
        return ERROR_DICE_ROLL_NOT_START;

    sDiceMember* pMember = GetDiceMember( userIdx );
    if ( pMember == NULL )
        return ERROR_DICE_ROLL_MEMBER;

    /// 이미 선택 했는지
    if ( pMember->mRoll ) 
        return ERROR_DICE_ROLL_ALREADY;

    /// 랜덤값 추출
    pMember->mRandom = randVal;
    pMember->mRoll   = true;
    ++mDiceRollCnt;

    return ERROR_DICE_ROLL_SUCCESS;
}

void cParty::SetDiceRoll(unsigned long userIdx, unsigned short randVal)
{
    sDiceMember* pMember = GetDiceMember( userIdx );
    if ( pMember != NULL )
    {
        if ( pMember->mRoll == false )
            ++mDiceRollCnt;

        pMember->mRoll   = true;
        pMember->mRandom = randVal;
    }
}

unsigned long cParty::GetLostPlayer(unsigned long tickCount)
{
	unsigned long* userTsp = mUserTsp;

	for ( unsigned int i = 0; i < mCount; ++i, ++userTsp )
	{
		if ( 0 < (*userTsp) && tickCount > (*userTsp) )
			return mUserArr[ i ];
	}

	return 0;
}

void cParty::SendPartyAddoneSyn(sPartyUserData* userData)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyAddoneSyn( userData );
	}
}

void cParty::SendPartyOutSyn(unsigned long playerIdx)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyOutSyn( playerIdx );
	}
}

void cParty::SendPartyThrowSyn(unsigned long playerIdx)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyThrowSyn( playerIdx );
	}
}

void cParty::SendPartyLeaderSyn(unsigned long leaderIdx)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyLeaderSyn( leaderIdx );
	}
}

void cParty::SendPartyDivideType(char type)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyDivideType( type );
	}
}

void cParty::SendPartyDataLevelSyn(unsigned long playerIdx, unsigned char level)
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyDataLevelSyn( playerIdx, level );
	}
}

void cParty::SendPartyMapchangeSyn(unsigned long playerIdx, short map, short room, DWORD channelnum )
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
			player->SendPartyMapchangeSyn( playerIdx, map, room, channelnum );
	}
}

void cParty::SendPartyDeleteSyn( )
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
		{
			player->SetPartyIndex( 0 );
			player->SetPartyState( ePARTY_NONE );
			player->SendPartyDeleteSyn( );
		}
	}
}

void cParty::SendChat(sInventory* inventory, wchar_t* name, wchar_t* message, unsigned long characterIdx, unsigned short job )
{
	unsigned long* userArr = mUserArr;
	cPlayer*       player;

	for ( unsigned int i = 0; i < mCount; ++i, ++userArr )
	{ 
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( player != NULL )
		{
			HANDLE              handle  = NULL;
			MSG_SYN_CHAT_PARTY* sendMsg = (MSG_SYN_CHAT_PARTY*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_CHAT, NM_CHAT_PARTY_SYN );
			if ( sendMsg != NULL )
			{
				sendMsg->playerIndex = characterIdx;
                sendMsg->job         = job;
				sendMsg->inventory = (*inventory);
				memcpy( sendMsg->Name, name, sizeof(sendMsg->Name) );
				memcpy( sendMsg->Message, message, sizeof(sendMsg->Message) );

				NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
			}
		}
	}
}

// SendUpdate Method - 파티정보 업데이트 및 채널 동기화
void cParty::SendUpdate( )
{
	cSender*       sender  = g_gameSrv->GetSender( );
	cPlayer*       player  = NULL;
	cPlayer*       others  = NULL;

	unsigned long* rootPtr = &mUserArr[ 0 ];
	unsigned int   rootCnt = 0;
	unsigned int   rootLen = mCount;

	while ( rootCnt < rootLen )
	{
		player = OBJECTMANAGER->GetPlayer( (*rootPtr) );
		++rootPtr; ++rootCnt;

		if ( player != NULL )
		{
			if ( rootCnt < rootLen )
			{
				unsigned long* tailPtr = &mUserArr[ rootCnt ];
				unsigned int   tailCnt = rootCnt;
				unsigned int   tailLen = mCount;

				while ( tailCnt < tailLen )
				{
					others = OBJECTMANAGER->GetPlayer( (*tailPtr) );
					++tailPtr; ++tailCnt;

					if ( others != NULL )
					{
						// 1. 맵위치 검사 - 최소한의 정보만 보낸다
						if ( player->GetMapNumber() != others->GetMapNumber() )
						{
							player->SendPartyDataLevelSyn( others->GetConnectionIdx( ) );
							others->SendPartyDataLevelSyn( player->GetConnectionIdx( ) );
						}
						// 2. 그리드 검사 - 근접 그리드안에 없는 유저에게만 쏘도록 한다
						else if ( GRIDMANAGER->IsSight( player, others ) == false )
						{
							player->SendPartyDataSyn( others->GetConnectionIdx( ) );
							others->SendPartyDataSyn( player->GetConnectionIdx( ) );
						}
					}
				}
			}

			// 채널동기화
			PARTY_DATA_SYNC sync;
			memset( &sync, 0, sizeof(sync) );

			sync.protocol = PSP_SYN_LEVEL_DATA;
			sync.synLevelData.partyIdx = mIndex;
			player->GetPartyUserData( &sync.synLevelData.player );

			sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNLEVELDATA( )  );
		}
	}
}
