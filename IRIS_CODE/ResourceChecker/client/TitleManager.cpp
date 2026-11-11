#include "stdafx.h"
#include "TitleManager.h"
#include "GameResourceManager.h"
#include "ItemManager.h"

cTitleManager* cTitleManager::mpSingleton = 0;

cTitleManager::cTitleManager()
: mRequestTitleIndex(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;
}

cTitleManager::~cTitleManager()
{
	mpSingleton = 0;

	cTitleDefineMap::cIterator i = mTitleDefineMap.Begin();
	cTitleDefineMap::cIterator end = mTitleDefineMap.End();
	
	for( ; i != end; ++i )
	{
		delete (cTitleDefine*)(i->mSecond);
	}
	mTitleDefineMap.Clear();
}

bool cTitleManager::Init()
{
	if( Load( "./Script/Resource/Title_Define.txt" ) == false )
	{
		assert( 0 && "failed to load title define" );
		return false;
	}
	return true;
}

/// 스크립트 로드
bool cTitleManager::Load( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
	{
		assert( 0 && "failed to load title define" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathname.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();

		/// 이름인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long nameIdx = str.ToInt();

		/// 내용인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long textIdx = str.ToInt();

		/// 효과인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long influenceIdx = str.ToInt();

		/// 색상 R
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorR = (unsigned char)str.ToInt();

		/// 색상 G
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorG = (unsigned char)str.ToInt();

		/// 색상 B
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorB = (unsigned char)str.ToInt();

		/// 타이틀 정의 생성
		cTitleDefine* define = new cTitleDefine;
// 		define->mName = GAMERESOURCEMAN->GetHeroTitle( nameIdx );
// 		define->mDescription = GAMERESOURCEMAN->GetHeroTitle( textIdx );
		define->mInfluenceIndex = influenceIdx;

		/// 변환 ( R,G,B 순서로 )
		unsigned long color = (unsigned long)( (DWORD)((BYTE)colorR << 16) | (WORD)((BYTE)colorG << 8) | (BYTE)colorB );
		define->mColor = color | 0xff000000;

		if( mTitleDefineMap.Insert( index, define ) == false )
		{
			assert( 0 && "failed to add title define" );
			return false;
		}
	}
	return true;
}

cTitleDefine* cTitleManager::GetTitleDefine( unsigned long titleIdx )
{
	return (cTitleDefine*)mTitleDefineMap.GetAt( titleIdx );
}