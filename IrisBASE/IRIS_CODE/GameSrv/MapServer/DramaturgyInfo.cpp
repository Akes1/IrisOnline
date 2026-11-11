#include "stdafx.h"
#include "DramaturgyInfo.h"

#include "Token.h"
#include "Parser.h"
#include "DramaturgyLexer.h"

#include "FileLoader.h"


cDramaturgyInfo::cDramaturgyInfo( const char* pName )
{
	mFileName = pName;

	mpDramaState = 0;
}

cDramaturgyInfo::~cDramaturgyInfo()
{
	SAFE_DELETE( mpDramaState );
}

bool cDramaturgyInfo::LoadFile()
{
	cString file = "./Script/Resource/";
	file += mFileName.Cstr();

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( file.Cstr(), true ) == false )
	{
		return false;
	}

	/// lexer & parser 선언
	cToken token;
	cDramaturgyLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, file );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_HEADER:
			{
				if( LoadHeader( parser ) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_CASTING:
			{
				if( LoadContents( parser ) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_ACTIVITY:
			{
				if( LoadContents( parser, false ) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_APPLY:
			{
				if( LoadContents( parser ) == false )
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

bool cDramaturgyInfo::LoadHeader( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_TYPE:
			{
				parser.ParseInt();
			}
			break;
		case eTOKEN_EFFECT:
			{
				parser.ParseInt();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();

				parser.ParseFloat();
				parser.ParseString();
			}
			break;
		case eTOKEN_SOUND:
			{
				parser.ParseInt();
				parser.ParseString();
			}
			break;
		case eTOKEN_TRAILTEX:
			{
				parser.ParseInt();
				parser.ParseString();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cDramaturgyInfo::LoadContents( cParser& parser, bool stateSkip  )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	if( stateSkip == false )
	{
		assert( mpDramaState == 0 );
		mpDramaState = new sDramaState;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			goto error;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_ANI:
			{
				parser.ParseInt();
			}
			break;
		case eTOKEN_ACTION:
			{
				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseInt();

				parser.ParseInt();
				parser.ParseInt();

				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();
			}
			break;
		case eTOKEN_ACTIONEX:
			{
				parser.ParseInt();
				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseInt();

				parser.ParseInt();
				parser.ParseInt();

				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();
			}
			break;
		case eTOKEN_SOUNDPLAY:
			{
				parser.ParseFloat();
				parser.ParseInt();
				parser.ParseInt();
			}
			break;
		case eTOKEN_CHANNELING:
			{
				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseInt();

				parser.ParseInt();
				parser.ParseInt();

				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();
			}
			break;
		case eTOKEN_ACTIVE:
			{
				mpDramaState->mActivityEndTime = parser.ParseInt();
			}
			break;
		case eTOKEN_BULLET:
			{
				mpDramaState->mIsBullet = true;
				/// 발사체 정보 저장
				mpDramaState->mActivityEndTime = parser.ParseInt();	/// 생성 시점

				parser.ParseInt();

				mpDramaState->mBulletSpeed = parser.ParseFloat();		/// 스피드(현재는 값으로 넣는다)

				parser.ParseInt();

				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();

				parser.ParseString();
			}
			break;
		case eTOKEN_CRASH:
			{
				parser.ParseInt();
				parser.ParseInt();

				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();
			}
			break;
		case eTOKEN_DAMAGE:
			{
				sDramaAction* pAct = new sDramaAction;
				pAct->actionType = eDRAMA_DAMAGE;
				pAct->time = parser.ParseInt();

				pAct->percent = parser.ParseFloat() / 100.0f;

				/// 추가
				mpDramaState->Push(pAct);
			}
			break;
		case eTOKEN_TRAIL:
			{
				parser.ParseInt();
				parser.ParseInt();

				parser.ParseInt();

				parser.ParseString();
				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			goto error;
		}
	}

	return true;

error:
	return false;
}


unsigned long cDramaturgyInfo::GetActivityEndTime()
{
	if( mpDramaState == 0 )
		return 0;
	return mpDramaState->mActivityEndTime;
}

bool cDramaturgyInfo::IsBullet()
{ 
	if( mpDramaState == 0 )
		return false;
	return mpDramaState->mIsBullet;
}

float cDramaturgyInfo::GetBulletSpeed()
{ 
	if( mpDramaState == 0 )
		return 0;
	return mpDramaState->mBulletSpeed;
}
