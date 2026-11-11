#include "gamesrv.h"

#include "StdAfx.h"
#include "PvPScript.h"
#include "Tokenizer.h"

cPvPScript* cPvPScript::mpPvPScript = NULL;


bool PvPTimeSort( const void *arg1, const void *arg2 ) 
{ 
	sPvPStartTime* left = (sPvPStartTime*)arg1;
	sPvPStartTime* right = (sPvPStartTime*)arg2;

	return left->mHour * 100 + left->mMinute < right->mHour * 100 + right->mMinute;
} 

cPvPScript::cPvPScript()
{

	/// 싱글톤
	if( !mpPvPScript )
	{
		mpPvPScript = this;
	}

}

cPvPScript::~cPvPScript()
{
	Release();
}


bool cPvPScript::Init()
{
	Release();

	if( LoadDMInfo() == false )
		return false;

	if( LoadMemberGive() == false )
		return false;

#ifndef _CLIENT
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_ID_PVP_:
		{
			if( LoadTime( NETWORK2->GetInDunMapNum() ) == false )
				return false;
		}
		break;		
	}
#endif

	return true;
}


void cPvPScript::Release()
{
	///
	cHashMap::cIterator i = mDMInfoMap.Begin();
	cHashMap::cIterator iend = mDMInfoMap.End();
	for( ; i!=iend; i++ )
	{
		delete (sDMInfo*)(*i).mSecond;
	}
	mDMInfoMap.Clear();
}

bool cPvPScript::LoadDMInfo()
{
	cFileLoader loader;
	cString pathName = "./script/resource/pvp_map.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load pvp_map.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sDMInfo* pDMInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pDMInfo = new sDMInfo;

		// 전장 종류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mType = static_cast<unsigned char>(str.ToInt());

		/// 입장주기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mTotalDMTime = static_cast<unsigned long>(str.ToInt());

		/// 진영최소인원
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mTeamPlayerMin = static_cast<unsigned char>(str.ToInt());

		/// 진영최대인원
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mTeamPlayerMax = static_cast<unsigned char>(str.ToInt());

		/// 수호석등장여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mStone = str.ToInt() == 1;

		/// A진영 수호석 위치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mStoneAPos.x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mStoneAPos.y = str.ToFloat();

		/// B진영 수호석 위치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mStoneBPos.x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pDMInfo->mStoneBPos.y = str.ToFloat();

		/// 레벨 구간별 리스트
		pDMInfo->mLevelBlockInfo.Clear();

		tokenizer.GetNext( &str );
		if( str != "{" )
			goto ERR;

		while( tokenizer.IsEnd() == false )
		{
			tokenizer.GetNext( &str );
			if( str == "}" )
				break;

			sDMBlockInfo* pBlockInfo = new sDMBlockInfo;

			/// 최소레벨
			/// if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mLevelMin = static_cast<unsigned char>(str.ToInt());				

			/// 최대레벨
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mLevelMax = static_cast<unsigned char>(str.ToInt());				

			/// 목표 포인트
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mGoalPoint = static_cast<unsigned long>(str.ToInt());				

			/// 석상 종류
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mStoneAIdx = str.ToInt();	

			/// 석상 종류
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mStoneBIdx = str.ToInt();	

			/// 패배시 전장포인트 보상기준
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pBlockInfo->mLoseDMBasePoint = static_cast<unsigned long>(str.ToInt());		

			/// 패배시 친밀도 보상기준
			//if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			//pBlockInfo->mLoseFriendlyBasePoint = static_cast<unsigned long>(str.ToInt());	

			pDMInfo->mLevelBlockInfo.PushBack( pBlockInfo );
		}

		if( mDMInfoMap.Insert( pDMInfo->mType, pDMInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	SAFE_DELETE(pDMInfo);
	return false;
}

bool cPvPScript::LoadMemberGive()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/pvp_character.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load pvp_character.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sDMGiveInfo* giveInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		giveInfo = new sDMGiveInfo;

		// 전장 Type
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned char type = static_cast<unsigned char>(str.ToInt());

		// 캐릭터 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLevel = static_cast<unsigned char>(str.ToInt());

		// 돈 보상-승리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mWinMoney = str.ToInt();

		// 돈 보상-패배
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLoseMoney = str.ToInt();

		// EXP 보상-승리(%)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mWinExpPer = str.ToFloat();
		if( giveInfo->mWinExpPer < 0.0f )				{ goto ERR; }

		// EXP 보상-패배(%)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLoseExpPer = str.ToFloat();
		if( giveInfo->mLoseExpPer < 0.0f )				{ goto ERR; }

		// SXP 보상-승리(%)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mWinSxpPer = str.ToFloat();
		if( giveInfo->mWinSxpPer < 0.0f )				{ goto ERR; }

		// SXP 보상-패배(%)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLoseSxpPer = str.ToFloat();
		if( giveInfo->mLoseSxpPer < 0.0f )				{ goto ERR; }

		// kill-일반
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mKillPoint = str.ToInt();					

		// kill-수장
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLeaderKillPoint = str.ToInt();

		// 자기세력포인트-승리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mWinMyForcePoint = str.ToInt();

		// 자기세력친밀도-승리
		//if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		//giveInfo->mWinMyForceFriendly = str.ToInt();

		// 자기세력포인트-패배
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		giveInfo->mLoseMyForcePoint = str.ToInt();

		// 자기세력친밀도-패배
		//if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		//giveInfo->mLoseMyForceFriendly = str.ToInt();

		// 동맹세력포인트-승리
		//if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		//giveInfo->mWinFamilyForcePoint = str.ToInt();

		// 동맹세력친밀도-승리
		//if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		//giveInfo->mWinFamilyForceFriendly = str.ToInt();


		/// 목록에 담는다.
		sDMInfo* pDMInfo = GetDmInfo( type );
		if( pDMInfo )
		{
			if( pDMInfo->mGiveInfo.Insert( giveInfo->mLevel, giveInfo ) == false )
			{
				assert(0);
				goto ERR;
			}
		}
		else
		{
			assert(0);
			goto ERR;
		}
	}
	return true;

ERR:
	SAFE_DELETE( giveInfo );
	return false;
}

sDMInfo* cPvPScript::GetDmInfo( unsigned int type )
{
	return (sDMInfo*)mDMInfoMap.GetAt( type );
}


bool cPvPScript::LoadTime( unsigned short pvpType )
{
	cFileLoader loader;
	cString pathName = "./Script/resource/pvp_time.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load pvp_time.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sPvPStartTime* pStartTime = NULL;
	while( tokenizer.IsEnd() == false )
	{
		// 전장 Type
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned short type = (unsigned short)str.ToInt();

		if( pvpType != type )
		{
			tokenizer.GetNext( &str );
			tokenizer.GetNext( &str );
			continue;
		}

		pStartTime = new sPvPStartTime;

		/// 시작 시
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pStartTime->mHour = (unsigned char)str.ToInt();

		/// 시작 분
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pStartTime->mMinute = (unsigned char)str.ToInt();

		mPvPStarttimeAry.PushBack( pStartTime );
	}

	std::sort( mPvPStarttimeAry.Begin(), mPvPStarttimeAry.End(), PvPTimeSort );

	return true;

ERR:
	SAFE_DELETE( pStartTime );
	return false;
}

