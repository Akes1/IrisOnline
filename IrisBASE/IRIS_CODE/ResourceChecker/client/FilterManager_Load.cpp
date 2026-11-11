#include "stdafx.h"
#include "FilterManager.h"
#include "./util/Tokenizer.h"

///
bool cFilterManager::LoadFilter( const cString& pathName )
{
	/// 파일열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load filter script" );
		return false;
	}
	
	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	/// 전체 매치 필터단어 로드
	while( tokenizer.IsEnd() == false )
	{
		tokenizer.GetNext( &str );

		if( str.Compare( "WholeMatch" ) == 0 )
			continue;

		/// 이 단어가나오면 부분 필터 로드로 바뀌도록 한다
		if( str.Compare( "PartMatch" ) == 0 )
			break;

		str.ToLower();

		/// 데이터 삽입
		TCHAR buffer[256] = {0,};
		int len = ConvertToUnicode( str.Cstr(), buffer, 256 );
		assert( len > 0 );

		if( mWholeSet.Insert( buffer ) == false )
		{
			assert( 0 && "failed to insert filter set" );
			cString msg;
			msg.Format("[%s] %s", pathName.Cstr(), str.Cstr() );
			MessageBoxA( NULL, msg.Cstr(), "failed to insert filter set", MB_OK | MB_ICONERROR );
			return false;
		}
	}

	/// 부분 매치 필터 단어 로드 
	while( tokenizer.IsEnd() == false )
	{
		tokenizer.GetNext( &str );
		
		str.ToLower();

		/// 데이터 삽입
		TCHAR buffer[256] = {0,};
		int len = ConvertToUnicode( str.Cstr(), buffer, 256 );
		assert( len > 0 );

		/// 키값 빼내기
		TCHAR key[2] = {0,};
		::_tcsncpy_s( key, buffer, 1 );

		///
		cPartMap::cIterator i = mPartMap.Find( key );
		if( i == mPartMap.End() )
		{
			/// 기존 키가 없으면 생성 
			/// 같은 글자로 시작되는 단어들 묶음 배열
			cSiblingArr* pSiblingArr = new cSiblingArr;
			pSiblingArr->PushBack( buffer );

			if( mPartMap.Insert( key, pSiblingArr ) == false )
			{
				assert( 0 && "failed to insert partMatch word" );
				cString msg;
				msg.Format("[%s] %s", pathName.Cstr(), str.Cstr() );
				MessageBoxA( NULL, msg.Cstr(), "failed to insert partMatch word", MB_OK | MB_ICONERROR );
				return false;
			}
		}
		else
		{	
			/// 기존 키가 존재하면 배열에 삽입하기
			cSiblingArr* pSiblingArr = (cSiblingArr*)(i->mSecond);
			pSiblingArr->PushBack( buffer );
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