#include "StdAfx.h"
#include "optionmanager.h"

#include "Token.h"
#include "Parser.h"
#include "FileSaver.h"
#include "FileLoader.h"

#include "Application.h"
#include "RenderSystem.h"
#include "SoundSystem.h"
#include "SceneManager.h"

#include "Player_Common.h"

#include "StageManager.h"

static WORD g_OldRamp[3*256];

cOptionLexer::cOptionLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "windowed", eTOKEN_WINMODE );
	BindKeyword( "resolution", eTOKEN_RESOLUTION );
	BindKeyword( "ramp", eTOKEN_RAMP );
	BindKeyword( "envvolume", eTOKEN_ENVVOLUME );
	BindKeyword( "effvolume", eTOKEN_EFFVOLUME );
	BindKeyword( "video", eTOKEN_VIDEOMODE );
	BindKeyword( "language", eTOKEN_LANGUAGE );
}


cOptionManager* cOptionManager::mpSingleton = NULL;


cOptionManager::cOptionManager()
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	mWindowed = false;
	mWidth = 1024;
	mHeight = 768;
	mBpp = 32;

	mGamma = 0.5f;

	mEnvVolume = 0.5f;	///0.0f ~ 1.0f
	mEffVolume = 0.5f;	///0.0f ~ 1.0f

	mVideoMode = 20;

	mLanguage = 82;
}

cOptionManager::~cOptionManager()
{
	Save( "./Option/option.txt" );

	mpSingleton = 0;
}

bool cOptionManager::Init()
{
	// 파일이 있으면 로딩한다.
	if( Load("./option/option.txt") == false )
	{
		mWindowed = false;

		mWidth = 1024;
		mHeight = 768;

		mBpp = 32;
		mGamma = 0.5f;

		mEnvVolume = 0.5f;	///0.0f ~ 1.0f
		mEffVolume = 0.5f;	///0.0f ~ 1.0f

		mVideoMode = 20;

		/// default
		mLanguage = 82;

		// 옵션파일 읽는데 실패 했다면 기본값으로 셋팅한다.
		if( Save( "./Option/option.txt" ) == false )
		{
			assert(0 && "failed option file load/save.");
			return false;
		}
	}
	return true;
}

bool cOptionManager::Load( const char* pathName )
{
	cFileLoader loader;
	if( loader.OpenNoPack( pathName, true ) == false )
	{
		return false;
	}

	cToken token;
	cOptionLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
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
		case eTOKEN_WINMODE:
			{
				mWindowed = parser.ParseInt()? true:false;
			}
			break;
		case eTOKEN_RESOLUTION:
			{
				mWidth = (unsigned int)parser.ParseInt();
				mHeight = (unsigned int)parser.ParseInt();
				mBpp = (unsigned char)parser.ParseInt();
			}
			break;
		case eTOKEN_RAMP:
			{
				mGamma = parser.ParseFloat();

				if( mGamma < 0.0f || mGamma > 1.0f )
					mGamma = 0.5f;
			}
			break;
		case eTOKEN_ENVVOLUME:
			{
				mEnvVolume = parser.ParseFloat();

				if( mEnvVolume < 0.0f || mEnvVolume > 1.0f )
					mEnvVolume = 0.5f;
			}
			break;
		case eTOKEN_EFFVOLUME:
			{
				mEffVolume = parser.ParseFloat();

				if( mEffVolume < 0.0f || mEffVolume > 1.0f )
					mEffVolume = 0.5f;
			}
			break;
		case eTOKEN_VIDEOMODE:
			{
				mVideoMode = parser.ParseInt();
			}
			break;
		case eTOKEN_LANGUAGE:
			{
				mLanguage = parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cOptionManager::Save( const char* pathName )
{
	///
	if( ::FileExist( "Option" ) == false )
	{
		::CreateDirectory( L"Option", 0 );
	}

	cFileSaver saver;
	if( saver.Open( pathName ) == false )
	{
		return false;
	}

	char buf[256] = {0,};

	/// window mode
	::sprintf_s( buf, "windowed %d\r\n", mWindowed? 1:0 );
	saver.Write( buf, strlen(buf) );

	/// resolution
	::sprintf_s( buf, "resolution %d %d %d\r\n", mWidth, mHeight, mBpp );
	saver.Write( buf, strlen(buf) );

	/// ramp
	::sprintf_s( buf, "ramp %f\r\n", mGamma );
	saver.Write( buf, strlen(buf) );

	/// back sound volume
	::sprintf_s( buf, "envvolume %f\r\n", mEnvVolume );
	saver.Write( buf, strlen(buf) );

	/// effect sound volume
	::sprintf_s( buf, "effvolume %f\r\n", mEffVolume );
	saver.Write( buf, strlen(buf) );

	/// video capture mode
	::sprintf_s( buf, "video %d\r\n", mVideoMode );
	saver.Write( buf, strlen(buf) );

	::sprintf_s( buf, "language %d", mLanguage );
	saver.Write( buf, strlen(buf) );

	return true;
}

void cOptionManager::SetWindowedOption( bool windowed, unsigned int width, unsigned int height, unsigned char bpp )
{
	if( mWindowed == windowed && mWidth == width && mHeight == height && mBpp == bpp )
		return;

	/// 적용
	THEAPP->UpdateScreenSize( width, height, bpp, windowed );

	mWindowed = windowed;
	mWidth = width;
	mHeight = height;
	mBpp = bpp;
}

void cOptionManager::SetVideoOption( int videoMode )
{
	mVideoMode = videoMode;
}

void cOptionManager::SetGammaPer( float r )
{
	mGamma = r;
	RENDERSYS->SetBrightness( mGamma );
}

void cOptionManager::SetVolume( float env, float eff )
{
	/// 적용
	mEnvVolume = env;
	mEffVolume = eff;

	SCENEMAN->UpdateSoundGain();
	SOUNDSYS->UpdateSoundGain();
}

void cOptionManager::ReqOptionValue()
{
	MSG_REQ_PLAYER_OPTION_UPDATE msg;
	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_OPTION_UPDATE_REQ;
	msg.option1 = mGameOption.optionData1;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

bool cOptionManager::IsShowHelmet()
{
	if( STAGEMAN->GetCurrentStage() != eStage_Game )
		return true;

	 return mGameOption.option1.showHelmet; 
}
