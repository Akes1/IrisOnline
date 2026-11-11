#include "StdAfx.h"
#include "ShortcutManager.h"

#include "Application.h"

#include "UIManager.h"

#include "Parser.h"

cShortcutManager* cShortcutManager::mpShortcutManager = NULL;
const char* const SCRIPT_PATH_FILE = "./script/interface/path_shortcut.txt";

cShortcutLexer::cShortcutLexer(const char *buffer, unsigned int size)
: cLexer( buffer, size )
{
	BindKeyword( "shortcut", eTOKEN_SHORTCUT );
	BindKeyword( "keyboard", eTOKEN_KEYBORAD );
}

cShortcutManager::cShortcutManager()
{
	mpShortcutManager = this;
}

cShortcutManager::~cShortcutManager()
{
	mpShortcutManager = 0;


	sKeyboardMap::cIterator pos = mKeyboardKey.Begin();
	sKeyboardMap::cIterator end = mKeyboardKey.End();
	for( ; pos != end; ++pos )
	{
		sKeyboardInfo* pKeyInfo = (sKeyboardInfo*)(*pos).mSecond;
		SAFE_DELETE( pKeyInfo );
	}

	sKeyMap::cIterator pos2 = mKeyboardScriptMap.Begin();
	sKeyMap::cIterator end2 = mKeyboardScriptMap.End();
	for( ; pos2 != end2; ++pos2 )
	{
		sKeyboardScript* pKeyboardInfo = (sKeyboardScript*)(*pos2).mSecond;
		SAFE_DELETE( pKeyboardInfo );
	}

	sKeyMap::cIterator pos3 = mShortcutScriptMap.Begin();
	sKeyMap::cIterator end3 = mShortcutScriptMap.End();
	for( ; pos3 != end3; ++pos3 )
	{
		sShortcutScript* pShorcutInfo = (sShortcutScript*)(*pos3).mSecond;
		SAFE_DELETE( pShorcutInfo );
	}
}


bool cShortcutManager::Init()
{
	/// scirpt load
	if ( LoadShortcutPath( SCRIPT_PATH_FILE ) )
	{
		//if( InitShortcut() == true && InitKeyboard() == true )
			return true;
	}

	return false;
}

bool cShortcutManager::LoadShortcutPath( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load path shortcut" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
		return false;
	}

	/// ·º¼­¿¡ ¹äÁÖ°í ÆÄ½Ì
	cToken token;
	cShortcutLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			{ goto ERR; }
		case eTOKEN_NULL:
			continue;
		case cShortcutLexer::eTOKEN_SHORTCUT:
			{
				if( parser.ExpectTokenString( "{" ) == false )
				{
					assert( 0 && "wrong script" );
					{ goto ERR; }
				}

				cToken token;
				cLexer* lexer = parser.GetLexer();

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
					{
						break;
					}
					
					int index = token.ToInt();
					sShortcutScript *pShortcutInfo = new sShortcutScript;
					if( pShortcutInfo )
					{
						pShortcutInfo->mPathIconIndex = parser.ParseInt();
						pShortcutInfo->mKeyboardIndex = parser.ParseInt();
						pShortcutInfo->mIconPosX = parser.ParseInt();
						pShortcutInfo->mIconPosY = parser.ParseInt();
						
						if( mShortcutScriptMap.Insert( index, pShortcutInfo ) == false )
						{
							assert(0);
							SAFE_DELETE( pShortcutInfo );
							{ goto ERR; }
						}
					}
				}
			}
			break;
		case cShortcutLexer::eTOKEN_KEYBORAD:
			{
				if( parser.ExpectTokenString( "{" ) == false )
				{
					assert( 0 && "wrong script" );
					{ goto ERR; }
				}

				cToken token;
				cLexer* lexer = parser.GetLexer();

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
					{
						break;
					}

					int index = token.ToInt();
					sKeyboardScript *pKeyboardInfo = new sKeyboardScript;
					if( pKeyboardInfo  )
					{
						pKeyboardInfo->mKey      = (unsigned char)parser.ParseInt();
						pKeyboardInfo->mIconPosX = parser.ParseInt();
						pKeyboardInfo->mIconPosY = parser.ParseInt();
						
						if( mKeyboardScriptMap.Insert( index, pKeyboardInfo ) == false )
						{
							assert(0);
							SAFE_DELETE( pKeyboardInfo );
							{ goto ERR; }
						}
					}
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			{ goto ERR; }
		}
	}

	return true;

ERR:
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	assert(0);
	return false;
}
