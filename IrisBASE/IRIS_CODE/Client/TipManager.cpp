#include "stdafx.h"
#include "TipManager.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "OptionManager.h"
#include "ChatManager.h"
#include "UINode.h"
#include "StageManager.h"

cTipManager* cTipManager::mpSingleton = 0;

cTipManager::cTipManager()
: mLastAccumTime(0)
, mGradeTipLastTime(0)
{
	mpSingleton = this;
}

cTipManager::~cTipManager()
{
	for( unsigned int i = 0; i < MAX_TIPCOUNT; ++i )
	{
		mTipArr[i].Clear();
	}

	for( unsigned int i = 0; i < mLoadTipArr.GetSize(); ++i )
	{
		delete (sLoadTipData*)mLoadTipArr[i];
	}
	mLoadTipArr.Clear();

	mpSingleton = 0;
}

bool cTipManager::Init()
{
	if( LoadGameTip( "./Script/Resource/Tip_Game.txt" ) == false )
	{
		assert( 0 && "failed to load tip_game" );
		return false;
	}
	if( LoadLoadTip( "./Script/Resource/Tip_LoadingMap.txt" ) == false )
	{
		assert( 0 && "failed to load tip_game" );
		return false;
	}
	
	return true;
}

/// 팁 데이터 로드
bool cTipManager::LoadGameTip( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
	{
		assert( 0 && "failed to load game tip" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathname.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 대사 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();

		/// 최소 레벨
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long minLevel = str.ToInt();

		/// 최대 레벨
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long maxLevel = str.ToInt();

		if( maxLevel > MAX_TIPCOUNT )
		{
			assert(0);
			return false;
		}

		/// 데이터 삽입
		for( long i = minLevel; i <= maxLevel; ++i )
		{
			mTipArr[i-1].PushBack( index );
		}
	}
	return true;
}

/// 로딩 팁 데이터 
bool cTipManager::LoadLoadTip( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
	{
		assert( 0 && "failed to load load tip" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathname.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 팁 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long index = (long)str.ToInt();
		(long)str.ToInt();

		/// 텍스쳐 명
		if( tokenizer.GetNext( &str ) == false )
			return false;

		cString name = str;

		/// 텍스처 좌표 X
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texX = (unsigned int)str.ToInt();

		/// 텍스처 좌표 Y
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texY = (unsigned int)str.ToInt();

        /// 
		sLoadTipData* p = new sLoadTipData;
		p->mTexName = name;
		p->mTexX = texX;
		p->mTexY = texY;
	
		mLoadTipArr.PushBack( p );
	}
	return true;
}

/// 게임 팁 띄우기
void cTipManager::Process( unsigned long accumTime )
{
	/// 등급위원회 권장 Tip 출력
	if( accumTime - mGradeTipLastTime >= HOUR )
	{
		unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);

		unsigned long textIndex = ( STAGEMAN->IsAdultServer() == false ) ? 186 : 187; 
		CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText(textIndex), COLOR_RED, infoIndex, false, true );

		mGradeTipLastTime = accumTime;
	}

	if( OPTIONMAN->IsShowTip() == false )
		return;

	if( accumTime - mLastAccumTime >= TIP_UPDATETIME )
	{
		/// 팁 띄우기
		unsigned char level = HERO->GetLevel();
		
		if( level <= MAX_TIPCOUNT )
		{
			int size = mTipArr[level-1].GetSize();
			if( size > 0 )
			{
				int key = rand() % size;
				unsigned long textIdx = mTipArr[level-1][key];

				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
				CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetTipText( textIdx ), COLOR_TIP, infoIndex, false, true );
			}
		}

		mLastAccumTime = accumTime;
	}
}

sLoadTipData* cTipManager::GetLoadTipData()
{
	int size = mLoadTipArr.GetSize();
	if( size > 0 )
	{
		int key = rand() % size;
		return (sLoadTipData*)mLoadTipArr[key];
	}
	return 0;
}
