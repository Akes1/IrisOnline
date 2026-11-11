#include "StdAfx.h"
#include "TutorialScript.h"
#include "BaseObject_Common.h"
#include "Parser.h"
#include "Tutorial_Common.h"

cTutorialScript* cTutorialScript::mpSingleton = NULL;

cTutorialScript::cTutorialScript()
{
	mpSingleton = this;
}

cTutorialScript::~cTutorialScript()
{
	Release();

	mpSingleton = 0;
}

void cTutorialScript::Release()
{
	cModeMap::cIterator i = mModeMap.Begin();
	cModeMap::cIterator end = mModeMap.End();

	for( ; i != end; ++i )
	{
		cTutorialMode* define = (cTutorialMode*)(i->mSecond);
		SAFE_DELETE( define );
	}
	mModeMap.Clear();
}

bool cTutorialScript::Init()
{
	if( LoadScript( "./Script/Resource/tutorialmode.txt"  ) == false )
	{
		assert( 0 && "failed to read tutorial script" );
		return false;
	}
	return true;
}

bool cTutorialScript::LoadScript( const cString& pathName )
{
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
		return false;

	cString path;
	::GetFilePath( &path, pathName );

	cToken token;
	cTutorialLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
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
		case eTOKEN_IMAGES:
			{
				// 이미지 파일 로딩
				if( LoadImages( path, parser ) == false )
				{
					assert( 0 && "fail to load imagefile" );
					return false;
				}
			}
			break;
		case eTOKEN_MODE:
			{
				// 스크립트 로딩
				unsigned long i = (unsigned long)parser.ParseInt();

				cTutorialMode* p = new cTutorialMode;
				p->mModeIndex = i;

				if( LoadMode( parser, p ) == false )
				{
					assert( 0 && "failed to load tutorial mode data" );
					return false;
				}

				if( mModeMap.Insert( i, p ) == false )
				{
					assert( 0 && "failed to insert tutorial" );
					SAFE_DELETE( p );
					return false;
				}

				// 스크립트 검사
				if( p->mStartPosIndex == 0 )
				{
					assert(0);
					return false;
				}

				if( p->mDieTextIndex == 0 )
				{
					assert(0);
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

bool cTutorialScript::LoadImages( const cString&, cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		//
		parser.ParseString( &token );
	}

	return true;
}

bool cTutorialScript::LoadMode( cParser& parser, cTutorialMode* mode )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_OBJECT:
			{
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
						break;
				}
			}
			break;
		case eTOKEN_IMAGE:
			{
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
						break;
				}
			}
			break;
		case eTOKEN_OBJECTIMAGE:
			{
				parser.ParseInt();

				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
						break;
				}
			}
			break;
		case eTOKEN_STARTPOSIDX:
			{
				// 시작 좌표 ( 부활, 재시작시 )
				mode->mStartPosIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_DIETEXTINDEX:
			{
				// 부활안내문 인덱스
				mode->mDieTextIndex = (unsigned long)parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////
cTutorialLexer::cTutorialLexer( const char* buffer, unsigned int size )
: cLexer( buffer, size )
{
	BindKeyword( "images", eTOKEN_IMAGES );
	BindKeyword( "mode", eTOKEN_MODE );

	BindKeyword( "object", eTOKEN_OBJECT );
	BindKeyword( "image", eTOKEN_IMAGE );
	BindKeyword( "objectimage", eTOKEN_OBJECTIMAGE );
	BindKeyword( "startposIndex", eTOKEN_STARTPOSIDX );
	BindKeyword( "dietextIndex", eTOKEN_DIETEXTINDEX );
}