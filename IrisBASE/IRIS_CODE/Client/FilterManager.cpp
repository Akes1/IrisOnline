#include "stdafx.h"
#include "FilterManager.h"
#include "Tokenizer.h"
#include "CommonDefines.h"

#include "Application.h"

cFilterManager* cFilterManager::mSingleton = 0;

cFilterManager::cFilterManager()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cFilterManager::~cFilterManager()
{
	mSingleton = 0;

	/// 전체 매치 삭제
	{
		mWholeSet.Clear();
	}

	/// 부분 매치 삭제 
	{
		cPartMap::cIterator i = mPartMap.Begin();
		cPartMap::cIterator end = mPartMap.End();

		for( ; i != end; ++i )
		{
			cSiblingArr* pSiblingArr = (cSiblingArr*)(i->mSecond);
			pSiblingArr->Clear();
			SAFE_DELETE( pSiblingArr );
		}
		mPartMap.Clear();
	}

	{
		cEmotionFilterMap::cIterator i = mEmotionFilterMap.Begin();
		cEmotionFilterMap::cIterator end = mEmotionFilterMap.End();

		for( ; i != end; ++i )
		{
			cEmotionSibling* pSib = (cEmotionSibling*)(i->mSecond);
			pSib->Clear();
			SAFE_DELETE( pSib );
		}
		mEmotionFilterMap.Clear();
	}
}


bool cFilterManager::Init()
{
	cStringW path;
	path.Format( L"./language/%s/filtertable.txt", cApplication::mLangaugeFolderW.Cstr() );

	if( LoadFilter( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load filter script" );
		return false;
	}
	return true;
}

///
bool cFilterManager::CheckWholeMatch( LPCTSTR msg )
{
	unsigned int len = ::_tcslen( msg );
	if( len == 0 )
		return false;

	cStringT str = msg;
	str.ToLower();

	/// 필터할 단어와 동일하면 필터링!
	if( FindWholeMatch( str ) == true )
	{
		return true;
	}
	return false;
}

///
bool cFilterManager::CheckPartMatch( LPCTSTR msg )
{
	unsigned int len = ::_tcslen( msg );
	if( len == 0 )
		return false;

	cStringT str = msg;
	str.ToLower();

	msg = (LPCTSTR)str.Cstr();

	TCHAR temp[MAX_CHAT_SIZE] = {0,};
	unsigned int tempLen = 0;

	for( unsigned int i = 0; i < len; ++i )
	{
		TCHAR key[2] = {0,};
		::_tcsncpy_s( key, &msg[i], 1 );

		bool search = false;
		if( tempLen > 0 )
		{
			/// 동일한 내용이 있는지 검사
			for( unsigned int t = 0; t < tempLen; ++t )
			{
				/// 동일하면 넘기기
				if( ::_tcsncmp( &temp[t], &msg[i], 1 ) == 0 )
				{
					search = true;
					break;
				}
			}
		}
		
		if( search == false )
		{
			/// 동일하지 않으면 삽입
			::_tcsncat_s( temp, &msg[i], 1 );
			tempLen++;
		}
	}

    ///
	for( unsigned int i = 0; i < tempLen; ++i )
	{
		TCHAR key[2] = {0,};
		::_tcsncpy_s( key, &temp[i], 1 );

		/// 한글자를 키로 해서 동일한 키가 있는지 검사 
		cPartMap::cIterator b = mPartMap.Find( key );
		if( b != mPartMap.End() )
		{
			/// 찾으면 연결 리스트에 있는 단어가 메세지에 포함되는지 검사 
			cSiblingArr* pSiblingArr = (cSiblingArr*)(b->mSecond);

			///
			for( unsigned int i = 0; i < pSiblingArr->GetSize(); ++i )
			{
				cStringT findKey;
				cStringT srcMsg = msg;
				
				///
				pSiblingArr->GetAt( &findKey, i );

				if( srcMsg.Find( findKey ) != -1 )
				{
					return true;
				}
			}
		}
	}
	return false;
}

unsigned long cFilterManager::GetEmontionIndex( LPCTSTR msg )
{
	cStringT srcMsg = msg;
	unsigned int len = ::_tcslen( msg );

	for( unsigned int i = 0; i < len; ++i )
	{
		TCHAR key[2] = {0,};
		::_tcsncpy_s( key, &msg[i], 1 );

		/// 한글자를 키로 해서 동일한 키가 있는지 검사 
		cEmotionFilterMap::cIterator iter = mEmotionFilterMap.Find( key );
		if( iter == mEmotionFilterMap.End() )
			continue;

		cEmotionSibling* pSib = (cEmotionSibling*)iter->mSecond;
		if( pSib == 0 )
			continue;

		cEmotionSibling::cIterator b = pSib->Begin();
		cEmotionSibling::cIterator end = pSib->End();
		for( ; b != end; ++b )
		{
			cStringT filter = (cStringT)b->mFirst;

			if( srcMsg.Find( filter ) != -1 )
				return (unsigned long)b->mSecond;
		}
	}
	return 0;
}

void cFilterManager::AddEmotionFilter( LPCTSTR text, unsigned long index )
{
	unsigned int len = ::_tcslen( text );
	if( len == 0 )
	{
		assert(0);
		return;
	}

	TCHAR key[2] = {0,};
	::_tcsncpy_s( key, text, 1 );
	cEmotionFilterMap::cIterator i = mEmotionFilterMap.Find( key );
	if( i == mEmotionFilterMap.End() )
	{
		cEmotionSibling* sib = new cEmotionSibling;
		sib->Insert( text, index );

		if( mEmotionFilterMap.Insert( key, sib ) == false )
		{
			assert(0);
			return;
		}
	}
	else
	{
		cEmotionSibling* sib = (cEmotionSibling*)i->mSecond;
		if( sib->Insert( text, index ) == false )
		{
			assert(0);
			return;
		}
	}
}