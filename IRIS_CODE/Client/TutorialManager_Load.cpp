#include "StdAfx.h"
#include "TutorialManager.h"
#include "BaseObject_Common.h"
#include "Parser.h"
#include "ResourceManager.h"
#include "Tutorial_Common.h"
#include "KeyboardAgent.h"
#include "Application.h"

bool cTutorialManager::LoadScript( const cString& pathName )
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

bool cTutorialManager::LoadImages( const cString& path, cParser& parser )
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
		unsigned int id = token.ToInt();
		parser.ParseString( &token );

		cString pathName;
		pathName.Format("./language/%s/%s", cApplication::mLangaugeFolder.Cstr(), token.Cstr() );
		NiTexture* p = RESOURCEMAN->LoadTexture( pathName, false );

		if( p == 0 )
		{
			assert( 0 && "failed to load texture" );
			return false;
		}

		if( mTextureMap.Insert( id, p ) == false )
		{
			assert(0);
		}
	}

	return true;
}

bool cTutorialManager::LoadMode( cParser& parser, cTutorialMode* mode )
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
				// 오브젝트
				if( LoadObject( parser, mode ) == false )
				{
					assert( 0 && "failed to load tutorial object data" );
					return false;
				}
			}
			break;
		case eTOKEN_IMAGE:
			{
				// 이미지 정보
				if( LoadImage( parser, mode ) == false )
				{
					assert( 0 && "failed to load tutorial image data" );
					return false;
				}
			}
			break;
		case eTOKEN_OBJECTIMAGE:
			{
				unsigned long i = (unsigned long)parser.ParseInt();

				// 객체 연결용 이미지 정보
				if( LoadObjectImage( parser, mode, i ) == false )
				{
					assert( 0 && "failed to load tutorial object image data" );
					return false;
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
bool cTutorialManager::LoadObject( cParser& parser, cTutorialMode* mode )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	eOBJECTTYPE type = eOBJECTTYPE_NONE;
	char set = 0;
	unsigned long imgIndex = 0;
	unsigned long classId = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_OBJMONSTER:
			{
				type = eOBJECTTYPE_MONSTER;

				set = (char)parser.ParseInt();
				imgIndex = (unsigned long)parser.ParseInt();
				classId = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_OBJNPC:
			{
				type = eOBJECTTYPE_NPC;

				set = (char)parser.ParseInt();
				imgIndex = (unsigned long)parser.ParseInt();
				classId = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_OBJITEM:
			{
				type = eOBJECTTYPE_ITEM;

				set = (char)parser.ParseInt();
				imgIndex = (unsigned long)parser.ParseInt();
				classId = 0;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}

		//
		sTutorialObject* p = new sTutorialObject;
		p->mObjectType = type;
		p->mSetAll = set;
		p->mImageIndex = imgIndex;
		p->mClassIndex = classId;

		mode->mObjectArr.PushBack( p );
	}

	return true;
}

bool cTutorialManager::LoadImage( cParser& parser, cTutorialMode* mode )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	sTutorialImage* p = new sTutorialImage;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_II:
			{
				unsigned long ii = (unsigned long)parser.ParseInt();
				NiTexture* pTex = GetTexture( ii );
				if( !pTex )
				{
					assert(0);
					SAFE_DELETE( p );
					return false;
				}

				p->mpTexture = pTex;
			}
			break;
		case eTOKEN_POS:
			{
				p->mPosX = (short)parser.ParseInt();
				p->mPosY = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_SIZE:
			{
				p->mWidth = (short)parser.ParseInt();
				p->mHeight = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_TEXPOS:
			{
				p->mTexPosX = (short)parser.ParseInt();
				p->mTexPosY = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNPOS:
			{
				p->mDownTexX = (short)parser.ParseInt();
				p->mDownTexY = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_ANIFRAME:
			{
				p->mFrameCount = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_ANIREPEAT:
			{
				char repeat = (char)parser.ParseInt();
				p->mRepeat = ( repeat == 1 );
			}
			break;
		case eTOKEN_ANITIME:
			{
				p->mOneFrameTime = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_KEYINPUT:
			{
				cString str = parser.ParseString();
				if( str == "KEY_W" )
				{
					p->mKey = KEY_W;
				}
				else if( str == "KEY_A" )
				{
					p->mKey = KEY_A;
				}
				else if( str == "KEY_S" )
				{
					p->mKey = KEY_S;
				}
				else if( str == "KEY_D" )
				{
					p->mKey = KEY_D;
				}
				else if( str == "KEY_SPACE" )
				{
					p->mKey = KEY_SPACE;
				}
				else if( str == "KEY_1" )
				{
					p->mKey = KEY_1;
				}
				else if( str == "KEY_2" )
				{
					p->mKey = KEY_2;
				}
				else if( str == "KEY_3" )
				{
					p->mKey = KEY_3;
				}
				else if( str == "KEY_4" )
				{
					p->mKey = KEY_4;
				}
				else if( str == "KEY_5" )
				{
					p->mKey = KEY_5;
				}
				else if( str == "KEY_TAB" )
				{
					p->mKey = KEY_TAB;
				}
				else if( str == "KEY_F6" )
				{
					p->mKey = KEY_F6;
				}
				else if( str == "KEY_F7" )
				{
					p->mKey = KEY_F7;
				}
				else if( str == "KEY_M" )
				{
					p->mKey = KEY_M;
				}
			}
			break;
		case eTOKEN_DEFAULTSHOW:
			{
				char show = (char)parser.ParseInt();
				p->mDefaultShow = ( show == 1 );
			}
			break;
		case eTOKEN_DRAMA:
			{
				p->mDramaInfo = (eTutorialDrama)parser.ParseInt();
			}
			break;
		case eTOKEN_LEFTTOP:
			{
				char lefttop = (char)parser.ParseInt();
				p->mLeftTop = ( lefttop == 1 );
			}
			break;
		case eTOKEN_IMGALIGN:
			{
				p->mAlign = (unsigned char)parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	// 데이터 저장
	mode->mImageArr.PushBack( p );
	return true;
}

bool cTutorialManager::LoadObjectImage( cParser& parser, cTutorialMode* mode, unsigned long index )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	sTutorialBaseImage* p = new sTutorialBaseImage;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_II:
			{
				unsigned long ii = (unsigned long)parser.ParseInt();
				NiTexture* pTex = GetTexture( ii );
				if( !pTex )
				{
					assert(0);
					SAFE_DELETE( p );
					return false;
				}

				p->mpTexture = pTex;
			}
			break;
		case eTOKEN_POS:
			{
				p->mPosX = (short)parser.ParseInt();
				p->mPosY = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_SIZE:
			{
				p->mWidth = (short)parser.ParseInt();
				p->mHeight = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_TEXPOS:
			{
				p->mTexPosX = (short)parser.ParseInt();
				p->mTexPosY = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_ANIFRAME:
			{
				p->mFrameCount = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_ANIREPEAT:
			{
				char repeat = (char)parser.ParseInt();
				p->mRepeat = ( repeat == 1 );
			}
			break;
		case eTOKEN_ANITIME:
			{
				p->mOneFrameTime = (unsigned long)parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	if( mode->mObjectImageMap.Insert( index, p ) == false )
	{
		SAFE_DELETE( p );
		assert(0);
		return false;
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

	BindKeyword( "monster", eTOKEN_OBJMONSTER );
	BindKeyword( "npc", eTOKEN_OBJNPC );
	BindKeyword( "item", eTOKEN_OBJITEM );

	BindKeyword( "ii", eTOKEN_II );
	BindKeyword( "pos", eTOKEN_POS );
	BindKeyword( "size", eTOKEN_SIZE );
	BindKeyword( "texpos", eTOKEN_TEXPOS );
	BindKeyword( "downpos", eTOKEN_DOWNPOS );
	BindKeyword( "frame", eTOKEN_ANIFRAME );
	BindKeyword( "repeat", eTOKEN_ANIREPEAT );
	BindKeyword( "time", eTOKEN_ANITIME );
	BindKeyword( "keyinput", eTOKEN_KEYINPUT );
	BindKeyword( "defaultshow", eTOKEN_DEFAULTSHOW );
	BindKeyword( "drama", eTOKEN_DRAMA );
	BindKeyword( "lefttop", eTOKEN_LEFTTOP );
	BindKeyword( "align", eTOKEN_IMGALIGN );
}
