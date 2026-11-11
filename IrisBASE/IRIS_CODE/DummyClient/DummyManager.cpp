#include "StdAfx.h"
#include "DummyManager.h"

#include "DummyClient.h"
#include "Application.h"

cDummyLexer::cDummyLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "start", eTOKEN_STARTINDEX );
	BindKeyword( "count", eTOKEN_MAXCOUNT );
	BindKeyword( "ip", eTOKEN_IP );
	BindKeyword( "port", eTOKEN_PORT );
	BindKeyword( "server", eTOKEN_SERVERIDX );
	BindKeyword( "channel", eTOKEN_CHANNELIDX );
}



cDummyManager* cDummyManager::mpSingleton = 0;


cDummyManager::cDummyManager()
{
	mpSingleton = this;

	checkCount = 0;

	mDummyStartIndex = 1000;
	mDummyMaxCount = 500;

	mCurrentCount = 0;
	mLoginGap = 0;

	mWnd = 0;

	//mIP = "192.168.3.48";
	mIP = "203.216.218.27";
	mPort = 14400;

	mCurServer = 1;
	mCurChannel = 0;
}

cDummyManager::~cDummyManager()
{
	Exit();
}

void cDummyManager::Exit()
{
	cDummyClientMap::cIterator i,end;
	cDummyClient* client = NULL;

	/// 按眉 
	i = mDummyMap.Begin();
	end = mDummyMap.End();
	for( ; i != end; ++i )
	{
		client = (cDummyClient*)(*i).mSecond;
		SAFE_DELETE(client);
	}
	mDummyMap.Clear();

	WSACleanup( );
	CoUninitialize( );
}

void cDummyManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mLoginGap != 0 && mLoginGap <= accumTime )
	{
		if( mCurrentCount < mDummyMaxCount )
		{
			cDummyClient* client = new cDummyClient;
			unsigned int msgIdx = WM_USER + mCurrentCount + 1;

			cString id;
			id.Format( "eyadummy%04d", mDummyStartIndex+mCurrentCount );

			cString pwd = "eyadummy";

			cString spwd = "0000";

			if( client->Init( mWnd, msgIdx, (char*)id.Cstr(), (char*)pwd.Cstr(), (char*)spwd.Cstr(), mCurServer, mCurChannel ) == false )
				delete client;
			else
			{
				if( mDummyMap.Insert( msgIdx, client ) == false )
					delete client;
			}

			mCurrentCount++;
			mLoginGap = accumTime + 1*SECOND;
		}
		else
		{
			mLoginGap = 0;
		}
	}

	cDummyClientMap::cIterator i,end;
	cDummyClient* client = NULL;

	/// 按眉 
	i = mDummyMap.Begin();
	end = mDummyMap.End();
	for( ; i != end; ++i )
	{
		client = (cDummyClient*)(*i).mSecond;
		if( client )
			client->ProcessManager( deltaTime, accumTime );
	}


	/// 按眉 磊眉 贸府
	i = mDummyMap.Begin();
	end = mDummyMap.End();
	for( ; i != end; ++i )
	{
		client = (cDummyClient*)(*i).mSecond;
		if( client )
			client->Process( deltaTime, accumTime );
	}
}

bool cDummyManager::Init( HWND wnd )
{
	if ( CoInitialize( NULL ) != S_OK )
	{
//		MessageBox( NULL, "CoInitialize Failed", g_applicationName, MB_OK );
		return false;
	}

	WORD    versionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;

	if ( WSAStartup( versionRequested, &wsaData ) != 0 )
	{
//		MessageBox( NULL, "WSAStartup Failed", g_applicationName, MB_OK );
		return false;
	}

	mDummyMap.Clear();

	/// 按眉 积己 棺 檬扁拳

	if( LoadDummyInfo( "./dummyOption.txt" ) == false )
	{
		NiMessageBox( "Failed dummy option file load.", "Error" );
		return false;
	}

	mCurrentCount = 0;
	mLoginGap = THEAPP->GetWorldAccumTime() + 1*SECOND;

	mWnd = wnd;

	return true;
}

void cDummyManager::NetWorkProc( UINT msg, WPARAM wparam, LPARAM lparam )
{
	cDummyClient* client = (cDummyClient*)mDummyMap.GetAt( msg );
	if( client )
		client->NetWorkProc( wparam, lparam );
}

bool cDummyManager::LoadDummyInfo( const char* pathName )
{
	cFileLoader loader;

	if( loader.OpenNoPack( pathName, true ) == false )
	{
		assert( 0 && "failed to load dummy option.txt" );
		return false;
	}

	cToken token;
	cDummyLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
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
		case eTOKEN_STARTINDEX:
			{
				mDummyStartIndex = parser.ParseInt();
			}
			break;
		case eTOKEN_MAXCOUNT:
			{
				mDummyMaxCount = parser.ParseInt();
			}
			break;
		case eTOKEN_IP:
			{
				mIP = parser.ParseString();
			}
			break;
		case eTOKEN_PORT:
			{
				mPort = (unsigned short)parser.ParseInt();
			}
			break;

		case eTOKEN_SERVERIDX:
			{
				mCurServer = (long)parser.ParseInt();
			}
			break;
		case eTOKEN_CHANNELIDX:
			{
				mCurChannel = (long)parser.ParseInt();
			}
			break;
		default:
			break;
		}
	}

	return true;
}
