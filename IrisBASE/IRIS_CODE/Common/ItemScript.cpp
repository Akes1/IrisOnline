#include "StdAfx.h"
#include "ItemScript.h"

cItemNpcStoreLexer::cItemNpcStoreLexer( const char* buffer, unsigned long size )
: cLexer( buffer, size )
{
	BindKeyword( "npc_store", eTOKEN_NPCSTORE );
	BindKeyword( "tab", eTOKEN_TAB );
}

/// cItemNpcStore
cItemNpcStore::cItemNpcStore()
{
}
cItemNpcStore::~cItemNpcStore()
{
    for(short i = 0; i < (short)mTabAry.GetSize(); i++)
    {
        sNpcItemData* p = (sNpcItemData*)(mTabAry[i]);
        p->mItem.Clear();
        SAFE_DELETE( p );
    }

    mTabAry.Clear();
}

bool cItemNpcStore::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();
	if( !lexer )
	{
		assert(0);
		return false;
	}

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_TAB:
			{
				unsigned long textIdx   = 0;        /// 텍스트 인덱스
                unsigned long rate      = 0;        /// 판매금 비율
                unsigned char force     = 0;        /// 세력 구분 - [ 0:테르/코인,1:불,2:물,3:바람,4:땅 ]

                textIdx = (unsigned long)parser.ParseInt();
                rate    = (unsigned long)parser.ParseInt();
                force   = (unsigned char)parser.ParseInt();

				if( mTabAry.GetSize() > NPCSTORE_TAB_COUNT )
				{
					assert( 0 && "tab index out of range" );
					return false;
				}
                
                sNpcItemData* pItemData = new sNpcItemData;
                pItemData->mTextIdx = textIdx;
                pItemData->mRate    = rate;
                pItemData->mForce   = force;

                mTabAry.PushBack( pItemData );

				if( LoadTab( pItemData, parser ) == false )
				{
					assert( 0 );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cItemNpcStore::LoadTab( sNpcItemData* pTab, cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();
	if( !lexer )
	{
		assert(0);
		return false;
	}

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_INT:
			{
				unsigned long itemIndex = (unsigned long)token.ToInt();
                pTab->mItem.PushBack( itemIndex );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cItemNpcStore::IsItemIdx( unsigned long itemIdx ) 
{
    for( short i = 0; i < (short)mTabAry.GetSize(); i++ )
    {
        sNpcItemData* p = (sNpcItemData*)(mTabAry[i]);
        for( short i = 0; i < (short)p->mItem.GetSize(); i++ )
        {
            if( p->mItem[i] == itemIdx )
                return true;
        }
    }
    return false;
}

bool cItemNpcStore::IsItemIdx( unsigned long tabIndex, unsigned long itemIdx ) 
{
    if( tabIndex >= mTabAry.GetSize() )
        return false;

    sNpcItemData* p = (sNpcItemData*)(mTabAry[tabIndex]);
    for( short i = 0; i < (short)p->mItem.GetSize(); i++ )
    {
        if( p->mItem[i] == itemIdx )
            return true;
    }
    return false;
}

unsigned long cItemNpcStore::GetItemIndex( unsigned long tabIndex, unsigned int cellIndex )
{
    sNpcItemData* pTab = (sNpcItemData*)(mTabAry[tabIndex]);
    if( pTab )
    {
        if( pTab->mItem.GetSize() > cellIndex )
            return pTab->mItem[cellIndex];
    }
    return 0;
}

unsigned long cItemNpcStore::GetTextIndex( unsigned long tabIndex )
{
    sNpcItemData* pTab = (sNpcItemData*)(mTabAry[tabIndex]);
    if( pTab )
        return pTab->mTextIdx;

    return 0;
}

unsigned long cItemNpcStore::GetItemCount( unsigned long tabIndex )
{
    sNpcItemData* pTab = (sNpcItemData*)(mTabAry[tabIndex]);
    if( pTab )
        return pTab->mItem.GetSize();

    return 0;
}

sNpcItemData* cItemNpcStore::GetNpcItemData( unsigned long tabIndex )
{
   if( tabIndex > mTabAry.GetSize() )
        return false;

    sNpcItemData* p = (sNpcItemData*)(mTabAry[tabIndex]);
    return ( ( p ) ? p : NULL );
}

unsigned long cItemNpcStore::GetItemSellRate(  unsigned long tabIndex )
{
	if( tabIndex > mTabAry.GetSize() || tabIndex < 0 )
	{
		assert( 0 && "Bad tabIndex" );
		return ULONG_MAX;
	}

	sNpcItemData* p = (sNpcItemData*)(mTabAry[tabIndex]);
	if( p )
		return p->mRate;
	else
	{
		assert( 0 && "Bad NpcItemData" );
		return ULONG_MAX;
	}
}

unsigned char cItemNpcStore::GetItemSellType( unsigned long tabIndex )
{
	if( tabIndex > mTabAry.GetSize() || tabIndex < 0 )
	{
		assert( 0 && "Bad tabIndex" );
		return 1;
	}

	sNpcItemData* p = (sNpcItemData*)(mTabAry[tabIndex]);
	if( p )
		return p->mForce;
	else
	{
		assert( 0 && "Bad NpcItemData" );
		return 1;
	}
}


/// cItemScript
cItemScript* cItemScript::mpItemScript = NULL;
cItemScript::cItemScript(void)
{
	if( mpItemScript )
	{
		assert(NULL);
	}
	else
	{
		mpItemScript = this;
	}
}

cItemScript::~cItemScript(void)
{
    Release();
}

bool cItemScript::Init()
{
	if( !LoadNpcStore( "./Script/Resource/Item_NpcStore.txt" ) )
	{
		assert(NULL);
		return false;
	}

	return true;

}

void cItemScript::Release()
{
    cNpcStoreMap::cIterator i = mNpcStoreMap.Begin();
    cNpcStoreMap::cIterator iend = mNpcStoreMap.End();

    for( ; i != iend; ++i )
    {
       cItemNpcStore* p = (cItemNpcStore*)(i->mSecond);
       SAFE_DELETE( p );
    }
    mNpcStoreMap.Clear();
}

bool cItemScript::LoadNpcStore(const cString& pathName)
{
	/// 파일 열기
	cFileLoader loader;

	if ( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load npc store" );
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken             token;
	cItemNpcStoreLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser            parser( &lexer, pathName );

	while ( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_NPCSTORE:
			{
				unsigned long i = parser.ParseInt( );		/// Npc 인덱스
				cItemNpcStore* p = new cItemNpcStore;		/// Npc 상점 생성

				if ( p->Load( parser ) == false )			/// 로딩
				{
					delete p;
					return false;
				}

				if ( mNpcStoreMap.Insert( i, p ) == false )	/// Npc 상점 맵에 추가
				{
					assert( 0 && "failed to insert npc store, maybe already exist" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

cItemNpcStore* cItemScript::GetItemNpcStore(unsigned long npcIndex) const
{
	cNpcStoreMap::cConstIterator i = mNpcStoreMap.Find( npcIndex );
	return ( i != mNpcStoreMap.End( ) ) ? (cItemNpcStore*)i->mSecond : NULL;
}
