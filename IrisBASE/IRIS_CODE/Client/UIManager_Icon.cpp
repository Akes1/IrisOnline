#include "stdafx.h"
#include "UIManager.h"

#include "ResourceManager.h"

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Icon.h"
#include "Application.h"

bool cUIManager::LoadIconParams( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load icon params" );
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	/// 아이콘 이미지를 로딩
	if( parser.ExpectTokenString( "images" ) == false )
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
		case eTOKEN_INT:
			{
				/// 텍스처 로딩
				unsigned int id = token.ToInt();
				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_STR );


				cString pathName;
				if( id >= LANGUAGE_ICON_INDEX )
				{
					pathName.Format( "./Language/%s/%s", cApplication::mLangaugeFolder.Cstr(), token.Cstr() );
				}
				else
					pathName.Format( "./Data/2DData/%s", token.Cstr() );

				NiTexture* p = RESOURCEMAN->LoadTexture( pathName.Cstr(), false );

				if( p == 0 )
				{
					assert( 0 && "failed to load texture" );
					return false;
				}

				/// 텍스처 맵에 추가
				mIconTextureMap.Insert( id, p );

				unsigned int width, height;
				width = p->GetWidth();
				height = p->GetHeight();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	/// 아이콘 인자 리스트를 파싱
	if( parser.ExpectTokenString( "iconParams" ) == false )
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
		case eTOKEN_INT:
			{
				unsigned int iconIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int imageIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texx = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texy = token.ToInt();

				/// 아이콘 인자 맵에 추가
				cTextureMap::cIterator i = mIconTextureMap.Find( imageIndex );
				if( i == mIconTextureMap.End() )
				{
					assert( 0 && "failed to find icon texture by id" );
					return false;
				}

				cIconParam* param = new cIconParam;
				param->mpTexture = (NiTexture*)i->mSecond;
				//test
				assert( param->mpTexture );
				//
				param->mTexPos.mX = texx;
				param->mTexPos.mY = texy;

				if( mIconParamMap.Insert( iconIndex, param ) == false )
				{
					assert( 0 && "failed to insert icon param, icon id maybe already exist" );
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

cIconParam* cUIManager::GetIconParam( unsigned int iconId )
{
	cIconParamMap::cIterator i = mIconParamMap.Find( iconId );

	if( i == mIconParamMap.End() )
		return 0;
	else
		return (cIconParam*)(i->mSecond);
}
