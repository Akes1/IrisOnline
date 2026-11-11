#include "stdafx.h"
#include "TarotManager.h"
#include "ItemManager.h"
#include "UIImage.h"
#include "CardImage.h"
#include "UIMsgboxEvent.h"
#include "GameResourceManager.h"
#include "UINpcTelling.h"
#include "UIManager.h"

#define TAROT_PARAMS_FILENAME		"./Script/Interface/Path_Tarot.txt"
#define NPCTAROT_SPREAD_FILE		"./Script/Resource/FortuneTarot_Spread.txt"
#define NPCTAROT_COMMENT_FILE		"./Script/Resource/FortuneTarot_Comment.txt"
#define NPCTAROT_CARD_FILE			"./Script/Resource/FortuneTarot_Card.txt"

cTarotManager*	cTarotManager::mSingleton = 0;

cTarotManager::cTarotManager()
{	
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mCurrentPage = ePAGE_NONE;
	mTarotType = eTAROT_NONE;
}

cTarotManager::~cTarotManager()
{
	mSingleton = 0;

	/// 타롯 이미지 파람
	{
		cTarotParamMap::cIterator i = mTarotParamMap.Begin();
		cTarotParamMap::cIterator end = mTarotParamMap.End();

		for( ; i != end; ++i )
		{
			delete (cCardParam*)(i->mSecond);
		}
		mTarotParamMap.Clear();
	}
	/// 스프레드
	{
		cNpcSpreadMap::cIterator i = mNpcSpreadMap.Begin();
		cNpcSpreadMap::cIterator end = mNpcSpreadMap.End();
		for( ; i != end; ++i )
		{
			sNpcSpread* p = (sNpcSpread*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcSpreadMap.Clear();
	}

	/// npc 카드
	{
		cNpcTarotMap::cIterator i = mNpcTarotMap.Begin();
		cNpcTarotMap::cIterator end = mNpcTarotMap.End();
		for( ; i != end; ++i )
		{
			sNpcTarotCard* p = (sNpcTarotCard*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcTarotMap.Clear();
	}

	/// 해석 
	{
		cNpcCommentMap::cIterator i = mNpcCommentMap.Begin();
		cNpcCommentMap::cIterator end = mNpcCommentMap.End();
		for( ; i != end; ++i )
		{
			sNpcTarotComment* p = (sNpcTarotComment*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcCommentMap.Clear();
	}

	/// 선택 타롯 인덱스
	ClearCardinfo();
}


void cTarotManager::Clear()
{
	/// 새로운 점이 시작되기전에 데이터 클리어 
	/// 선택 타롯 인덱스
	ClearCardinfo();
}

///
bool cTarotManager::Init()
{
	/// 타롯 이미지 스크립트 로딩
	if( LoadTarotParams( TAROT_PARAMS_FILENAME) == false )
	{
		assert( 0 && "failed to load tarot params file" );
		cString msg;
		msg.Format("[%s]", TAROT_PARAMS_FILENAME );
		MessageBoxA( NULL, msg.Cstr(), "failed to load tarot params file", MB_OK | MB_ICONERROR );
		return false;
	}
	/// 스프레드 정보 로드
	if( LoadNpcSpread( NPCTAROT_SPREAD_FILE ) == false )
	{
		assert( 0 && "failed to load npc spread" );
		cString msg;
		msg.Format("[%s]", NPCTAROT_SPREAD_FILE );
		MessageBoxA( NULL, msg.Cstr(), "failed to load npc spread", MB_OK | MB_ICONERROR );
		return false;
	}

	/// 해석 정보 로드
	if( LoadComment( NPCTAROT_COMMENT_FILE ) == false )
	{
		assert( 0 && "failed to load tarot comment" );
		cString msg;
		msg.Format("[%s]", NPCTAROT_COMMENT_FILE );
		MessageBoxA( NULL, msg.Cstr(), "failed to load tarot comment", MB_OK | MB_ICONERROR );
		return false;
	}

	if( LoadTarotDeck( NPCTAROT_CARD_FILE ) == false )
	{
		assert( 0 && "failed to load tarot" );
		cString msg;
		msg.Format("[%s]", NPCTAROT_CARD_FILE );
		MessageBoxA( NULL, msg.Cstr(),  "failed to load tarot", MB_OK | MB_ICONERROR );
		return false;
	}
	return true;
}

void cTarotManager::ClearCardinfo()
{
	for( unsigned int i = 0; i < mSelectInfoArr.GetSize(); ++i )
	{
		sResultInfo* info = (sResultInfo*)mSelectInfoArr[i];
		SAFE_DELETE( info );
	}
	mSelectInfoArr.Clear();
}
