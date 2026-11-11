#include "stdafx.h"
#include "FilterManager.h"
#include "Tokenizer.h"

bool cFilterManager::LoadFilter( const cStringW& pathName )
{
	/// 파일열기
	cFileLoaderW loader;

	if( loader.Open( pathName.Cstr(), true ) == false )
	{
		assert( 0 && "failed to load filter script" );
		return false;
	}

	cTokenizerW tokenizer( loader.GetBufferPtr(), loader.GetSize(), L"\t\r\n", pathName.Cstr() );
	cStringW str;

	/// 전체 매치 필터단어 로드
	while( tokenizer.IsEnd() == false )
	{
		tokenizer.GetNext( &str );

		if( str.Compare( L"WholeMatch" ) == 0 )
			continue;

		/// 이 단어가나오면 부분 필터 로드로 바뀌도록 한다
		if( str.Compare( L"PartMatch" ) == 0 )
			break;

		str.ToLower();

		if( mWholeSet.Insert( str ) == false )
		{
			assert( 0 && "failed to insert filter set" );
			return false;
		}
	}

	/// 부분 매치 필터 단어 로드 
	while( tokenizer.IsEnd() == false )
	{
		tokenizer.GetNext( &str );

		str.ToLower();

		/// 키값 빼내기
		TCHAR key[2] = {0,};
		::_tcsncpy_s( key, str.Cstr(), 1 );

		///
		cPartMap::cIterator i = mPartMap.Find( key );
		if( i == mPartMap.End() )
		{
			/// 기존 키가 없으면 생성 
			/// 같은 글자로 시작되는 단어들 묶음 배열
			cSiblingArr* pSiblingArr = new cSiblingArr;
			pSiblingArr->PushBack( str );

			if( mPartMap.Insert( key, pSiblingArr ) == false )
			{
				assert( 0 && "failed to insert partMatch word" );
				return false;
			}
		}
		else
		{	
			/// 기존 키가 존재하면 배열에 삽입하기
			cSiblingArr* pSiblingArr = (cSiblingArr*)(i->mSecond);
			pSiblingArr->PushBack( str );
		}	
	}
	return true;
}

bool cFilterManager::FindWholeMatch( const cStringT& str )
{
	cWholeSet::cIterator i = mWholeSet.Find( str );

	if( i == mWholeSet.End() )
		return false;
	else
		return true;
}
