#include "stdafx.h"
#include "UIManager.h"

#include "UIFuncText.h"
#include "UINpcTelling.h"
#include "Token.h"
#include "Parser.h"
#include "Page.h"
#include "NpcScript.h"

bool cUIManager::LoadNpcText( const cStringW& pathName )
{
	if( mpNpcText == 0 )
	{
		assert(0);
		return false;
	}

	if( mpNpcText->Load( pathName ) == false )
	{
		assert( 0 && "failed to load npc text" );
		return false;
	}
	return true;
}

bool cUIManager::LoadNpcScripts( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load npc scripts" );
		return false;
	}

	/// 경로 추출
	cString path;
	::GetFilePath( &path, pathName );

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	if( parser.ExpectTokenString( "npcfiles" ) == false )
		return false;
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_RCURLY )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_STR:
			{
				cString pathName( path );
				pathName += token;

				if( LoadNpcScript( pathName ) == false )
				{
					assert( 0 && "failed to load npc script" );
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

bool cUIManager::LoadNpcScript( const cString& pathName )
{
	/// 파일 로딩
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cNpcLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_NPC_TELLING:
			{
				/// Npc 대화 인덱스
				unsigned int i = parser.ParseInt();

				/// Npc 대화 생성
				cUINpcTelling* p = new cUINpcTelling;

				/// 로딩
				if( p->Load( parser ) == false )
				{
					delete p;
					return false;
				}

				/// Npc 대화 배열에 추가
				if( mNpcTellingMap.Insert( i, p ) == false )
				{
					assert( 0 && "failed to insert npc telling, maybe already exist" );
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

bool cUIManager::LoadNpcScene( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load npc scene" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();
		assert( index > 0 );

		/// 텍스쳐이름
		if( tokenizer.GetNext( &str ) == false )
			return false;
		cString name = str;

		/// x 좌표
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int x = (unsigned int)str.ToInt();

		/// y 좌표
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int y = (unsigned int)str.ToInt();

		/// width
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int w = (unsigned int)str.ToInt();

		/// heigth
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int h = (unsigned int)str.ToInt();
		
		/// 장면 데이터 생성
		sSceneData* p = new sSceneData;
		p->mTexName = name;// mpTexture = tex;
		p->mTexPosX = x;
		p->mTexPosY = y;
		p->mTexWidth = w;
		p->mTexHeight = h;

		if( mSceneMap.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert npc scene, maybe already exist" );
			return false;
		}
	}
	return true;

}

cUINpcTelling* cUIManager::GetNpcTelling( unsigned int npcId ) const
{
	cNpcTellingMap::cConstIterator i = mNpcTellingMap.Find( npcId );

	if( i == mNpcTellingMap.End() )
		return 0;
	else
		return (cUINpcTelling*)(i->mSecond);
}

cUIFuncTextNode* cUIManager::GetNpcTextNode( unsigned int textId )
{
	assert( mpNpcText );

	return mpNpcText->GetNode( textId );
}
