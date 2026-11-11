#include "stdafx.h"
#include "gamesrv.h"
#include "TitleManager.h"
#include "tokenizer.h"

cTitleManager* cTitleManager::mpSingleton = 0;

cTitleManager::cTitleManager()
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

void cTitleManager::Release()
{
	cTitleDefineMap::cIterator start = mTitleDefineMap.Begin();
	cTitleDefineMap::cIterator end = mTitleDefineMap.End();

	cTitleDefine* define = NULL;
	while( start != end )
	{
		define = (cTitleDefine*)(*start++).mSecond;

        SAFE_DELETE( define );
	}
	mTitleDefineMap.Clear();
}

cTitleDefine* cTitleManager::GetTitleDefine( unsigned long index )
{
	return (cTitleDefine*)mTitleDefineMap.GetAt( index );
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

		/// 내용인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 효과인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long influenceIdx = str.ToInt();

		/// 색상 R
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 색상 G
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 색상 B
		if( tokenizer.GetNext( &str ) == false )
            return false;

        if( index > 0 )
        {
            /// 타이틀 정의 생성
            cTitleDefine* define = new cTitleDefine;
            define->mInfluenceIndex = influenceIdx;

            if( mTitleDefineMap.Insert( index, define ) == false )
            {
                assert( 0 && "failed to add title define" );
                return false;
            }
        }
    }
	return true;
}