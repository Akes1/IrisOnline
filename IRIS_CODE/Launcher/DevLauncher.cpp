#include "DevLauncher.h"
#include <assert.h>
#include "resource.h"
#include <time.h>
#include <io.h>

cDevLauncher*	g_DevLauncher = NULL;

LRESULT CALLBACK SetModeDlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_DevLauncher->SetModeMsgProc( wnd, msg, wParam, lParam );
}

cDevLauncher::cDevLauncher()
: mLauncherMode( eLAUNCHER_MODE_MAX )
,mMainWnd( NULL )
,mDevLauncherDlgWnd( NULL )
{
	g_DevLauncher = this;

	memset( mModeArray, 0, sizeof(mModeArray) );
}

cDevLauncher::~cDevLauncher()
{
}

HWND cDevLauncher::Init( HWND wnd )
{
	mDevLauncherDlgWnd = CreateDialog( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SET_RUN_MODE), NULL, (DLGPROC)SetModeDlgProc );
	if( mDevLauncherDlgWnd == NULL )
		return 0;

	mMainWnd = wnd;

	SetDlgOption();

	ShowWindow( mDevLauncherDlgWnd, TRUE );

	return mDevLauncherDlgWnd;
}

bool cDevLauncher::SetDlgOption()
{
#ifdef _SERVER

	char SERVER_TEST_HOST[] = {  63,  -38, -113, -66,  56,  -35, -112, -94, 56, -39, -112, -92, 59, 0 };
	char SERVER_TEST_ADDR[] = {  71, -118,  -50, -15,  99,  -58, -19, -11, 127, -99, -37, -30, 0 };
	char SERVER_IDC_HOST[]  = {  60,  -37, -112, -92,  62,  -59,-113, -66,  62, -38, 0 };
    char SERVER_IDC_ADDR[]  = {  71, -118,  -50, -15,  99,  -58, -19, -11, 127, -99, -37, -30, 0 };

    // SERVER-DEV
//	strcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_DEV].mMode, "SERVER-DEV" );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_DEV].mHost, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_DEV].mHost), SERVER_DEV_HOST, sizeof(SERVER_DEV_HOST) );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_DEV].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_DEV].mAddr), SERVER_DEV_ADDR, sizeof(SERVER_DEV_ADDR) );
//	mModeArray[eLAUNCHER_MODE_SERVER_DEV].mPort = 8080;

    // SERVER-TEST
	strcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_TEST].mMode, "SERVER-TEST" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_TEST].mHost, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_TEST].mHost), SERVER_TEST_HOST, sizeof(SERVER_TEST_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_TEST].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_TEST].mAddr), SERVER_TEST_ADDR, sizeof(SERVER_TEST_ADDR) );
	mModeArray[eLAUNCHER_MODE_SERVER_TEST].mPort = 8081;

    // SERVER-IDC
	strcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_IDC].mMode, "SERVER-IDC" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_IDC].mHost, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_IDC].mHost), SERVER_IDC_HOST,  sizeof(SERVER_IDC_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_SERVER_IDC].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_SERVER_IDC].mAddr), SERVER_IDC_ADDR,  sizeof(SERVER_IDC_ADDR) );
	mModeArray[eLAUNCHER_MODE_SERVER_IDC].mPort = 8081;

    mLauncherMode = eLAUNCHER_MODE_SERVER_IDC;

#else
/*
    // DEV
	char CLIENT_DEV_HOST[] = {  63,  -38, -113, -66,  56,  -35, -112, -94,  56,  -39, -112, -92, 59, 0 };
	char CLIENT_DEV_ADDR[] = {  93, -118,  -54, -13, 101, -114,  -52,   0	};
    char GMTOOL_DEV_ADDR[] = {  78, -121,  -41, -11,  99,  -97,   -7, -35,  32,  -81,   -5, -58, 0	};
    
	strcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mMode, "CLIENT-DEV" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mHost, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mHost), CLIENT_DEV_HOST, sizeof(CLIENT_DEV_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mAddr), CLIENT_DEV_ADDR, sizeof(CLIENT_DEV_ADDR) );
	mModeArray[eLAUNCHER_MODE_CLIENT_DEV].mPort = 80;

	strcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mMode, "GMTOOL-DEV" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mHost, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mHost), CLIENT_DEV_HOST, sizeof(CLIENT_DEV_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mAddr), GMTOOL_DEV_ADDR, sizeof(GMTOOL_DEV_ADDR) );
	mModeArray[eLAUNCHER_MODE_GMTOOL_DEV].mPort = 8080;

//	char CLIENT_TEST_ADDR[] = { 71,	-118,  -50, -15, 99, -58,  -18, -15, 121, -120,  -42, -11, 127, 0 };
//	strcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mMode, "CLIENT-TEST" );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mHost, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mHost), CLIENT_TEST_HOST, sizeof(CLIENT_TEST_HOST) );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mAddr), CLIENT_TEST_ADDR, sizeof(CLIENT_TEST_ADDR) );
//	mModeArray[eLAUNCHER_MODE_CLIENT_TEST].mPort = 80;
//	char CLIENT_IDC_ADDR[] = { 100, -103,  -41, -29,  34, -101, -33, -28, 110, -125,  -31,  -7, 127,  -60, -12, -15, 125, -118, -48, -67, 93, -118, -54, -13, 101, -114, -52, 0 };
//	strcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mMode, "CLIENT-IDC" );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mHost, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mHost), CLIENT_IDC_HOST, sizeof(CLIENT_IDC_HOST) );
//	memcpy_s( mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mAddr), CLIENT_IDC_ADDR, sizeof(CLIENT_IDC_ADDR) );
//	mModeArray[eLAUNCHER_MODE_CLIENT_IDC].mPort = 80;
*/

    // TEST
	char CLIENT_TEST_HOST[] = { 63,  -37, -115, -66, 63, -38, -120, -66,  63,  -38, -122, -66,  62, -38,   0 };
	char GMTOOL_TEST_ADDR[] = { 71,	-118,  -50, -15, 99, -58,  -18, -15, 121, -120,  -42, -11, 127 };

	strcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mMode, "GMTOOL-TEST" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mHost, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mHost), CLIENT_TEST_HOST,  sizeof(CLIENT_TEST_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mAddr), GMTOOL_TEST_ADDR,  sizeof(GMTOOL_TEST_ADDR) );
	mModeArray[eLAUNCHER_MODE_GMTOOL_TEST].mPort = 80;

    // IDC
	char CLIENT_IDC_HOST[] = { 110, -113,  -48, -66, 106, -101, -47, -28, 108,  -97,  -47, -66, 103, -101,   0 };
    char GMTOOL_IDC_ADDR[] = {  71, -118,  -50, -15,  99,  -58, -18, -15, 121, -120,  -42, -11, 127,  -94,  -6, -45, 0 };

	strcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mMode, "GMTOOL-IDC" );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mHost, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mHost), CLIENT_TEST_HOST, sizeof(CLIENT_TEST_HOST) );
	memcpy_s( mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mAddr, sizeof(mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mAddr), GMTOOL_IDC_ADDR,  sizeof(GMTOOL_IDC_ADDR) );
	mModeArray[eLAUNCHER_MODE_GMTOOL_IDC].mPort = 80;

    mLauncherMode = eLAUNCHER_MODE_GMTOOL_IDC;

#endif


	/// Load Run Mode File
	char currentDirectory[MAX_PATH] = {0,};
	char optionFilePath[MAX_PATH]	= {0,};

	GetCurrentDirectory( MAX_PATH, currentDirectory );
	sprintf_s( optionFilePath, "%s\\%s", currentDirectory, OPTION_FILENAME );

	FILE* file;
	errno_t error = fopen_s( &file, optionFilePath, "rt" );

    if( file && feof(file) == false )
    {
		int mode = 0;
		fscanf_s( file, "%d", &mode );

		if( mode >= 0 && mode < eLAUNCHER_MODE_MAX )
			mLauncherMode = (eLAUNCHER_MODE)mode;
//		else
//			mLauncherMode = eLAUNCHER_MODE_CLIENT_IDC;
		fclose( file );
    }

	/// 런처 실행 모드 ComboBox 추가
	for( unsigned int cnt = 0; cnt< eLAUNCHER_MODE_MAX; ++cnt )
	{
        if ( mModeArray[cnt].mMode[0] > 0 )
        {
            LRESULT idx = SendDlgItemMessage( mDevLauncherDlgWnd, IDC_COMBO_RUN_MODE, CB_ADDSTRING, NULL, (LPARAM)mModeArray[cnt].mMode );
            if( idx != CB_ERR && idx != CB_ERRSPACE )
            {
                SendDlgItemMessage( mDevLauncherDlgWnd, IDC_COMBO_RUN_MODE, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)cnt );

                if( mLauncherMode == cnt )
				{
					SendDlgItemMessage( mDevLauncherDlgWnd, IDC_COMBO_RUN_MODE, CB_SETCURSEL, (WPARAM)idx, NULL );
					NoticeMsgUpdate();
				}
            }
            else
                return false;
        }
	}

	return true;
}

void cDevLauncher::SaveOptionFile()
{
	FILE *file = NULL;
	char currentDirectory[MAX_PATH] = {0,};
	char optionFilePath[MAX_PATH]   = {0,};

	GetCurrentDirectory( MAX_PATH, currentDirectory );
	sprintf_s( optionFilePath, "%s\\%s", currentDirectory, OPTION_FILENAME );

	errno_t error =  fopen_s( &file, optionFilePath, "wt" );
	if( error == 0 )
		fprintf_s( file, "%d", mLauncherMode );

    if( file )
	    fclose( file );
}

char* cDevLauncher::GetHost()
{
	if( mLauncherMode == eLAUNCHER_MODE_MAX )
		return NULL;

	BYTE key[] = { 0x0d, 0xeb, 0xbe, 0x90 };

	size_t strLen = 0;
	strLen = ::strlen( mModeArray[mLauncherMode].mHost );
	
	for( size_t index = 0; index < strLen; ++index )
		mModeArray[mLauncherMode].mHost[index] ^= key[index%4];

	return mModeArray[mLauncherMode].mHost; 
}

char* cDevLauncher::GetAddr()
{ 
	if( mLauncherMode == eLAUNCHER_MODE_MAX )
		return NULL;

	BYTE key[] = { 0x0d, 0xeb, 0xbe, 0x90 };

	size_t strLen = 0;
	strLen = ::strlen( mModeArray[mLauncherMode].mAddr );

	for( size_t index = 0; index < strLen; ++index )
		mModeArray[mLauncherMode].mAddr[index] ^= key[index%4];

	return mModeArray[mLauncherMode].mAddr; 
}

int	cDevLauncher::GetPort()
{ 
	if( mLauncherMode == eLAUNCHER_MODE_MAX )
		return 0;

	return mModeArray[mLauncherMode].mPort; 
}

LRESULT cDevLauncher::SetModeMsgProc ( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_DESTROY:
		SaveOptionFile();
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD(wParam) )
		{
		case IDOK:
			{
				// 실행 모드 옵션 설정
				LRESULT lResult = SendDlgItemMessage( wnd, IDC_COMBO_RUN_MODE, CB_GETCURSEL, 0, 0 );
				if( lResult != CB_ERR )
				{
					eLAUNCHER_MODE runMode;
					runMode = (eLAUNCHER_MODE)SendDlgItemMessage( wnd, IDC_COMBO_RUN_MODE, CB_GETITEMDATA, lResult, 0 );
					mLauncherMode = runMode;
				}

				// 응용 프로그램, 다이얼로그 표시
				ShowWindow( mDevLauncherDlgWnd, FALSE );

				PostMessage( mMainWnd, WM_LAUNCHER_RUN_MSG, 0, 0);

				mDevLauncherDlgWnd = NULL;
				DestroyWindow( wnd );
			}
			return TRUE;
		case IDCANCEL:
			DestroyWindow( wnd );
			PostMessage( mMainWnd, WM_CLOSE, 0, 0);
			mDevLauncherDlgWnd = NULL;
			return TRUE;
		case IDC_COMBO_RUN_MODE:
			{
				switch( HIWORD(wParam) )
				{
				case CBN_SELCHANGE:
					{
						NoticeMsgUpdate();
					}
					break;
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}

void cDevLauncher::EventWrite(LPCTSTR format, ...)
{
	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;
	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) +1;
	msgBuf       = malloc( bufferLength );
	vsprintf_s( (char*)msgBuf, bufferLength, format, args );

	va_end( args );

	time_t t;
	tm    pt;
	char time_array[ 1024 ] = { 0, };
	char today[ 1024 ]  = { 0, };

	errno_t err;

	time(&t);
	err = localtime_s( &pt, &t);

	sprintf_s( time_array, "%d/%d/%d %d:%d:%d \0",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, pt.tm_hour,pt.tm_min,pt.tm_sec );
	sprintf_s( today, "%02d_%02d_%02d", pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday );

	if ( msgBuf != NULL )
	{
		char currentDirectory[MAX_PATH] = {0,};
		GetCurrentDirectory( MAX_PATH, currentDirectory );

		char filename[1024] = { 0, };
		char foldername[ 1024 ] = { 0, };
		sprintf_s( filename, "%s\\LOG-LAUNCHER\\[%s]LAUNCHER.log", currentDirectory, today  );
		sprintf_s( foldername, "%s\\LOG-LAUNCHER", currentDirectory );
		
		/// 폴더가 존재하는지 검사
		if( _access( foldername, 0 ) != 0 )
		{
			/// 없다면 만들기
			if( CreateDirectory( foldername, NULL ) == FALSE )
				MessageBox( NULL, "Fail to make log directory","error", MB_OK );	
		}

		FILE* stream;
		errno_t fopenError;
		fopenError = fopen_s( &stream, filename, "at" );
		if ( fopenError == 0 )
		{
			fputs( time_array   , stream );
			fputs( (char*)msgBuf, stream );
			fclose( stream );
			stream = NULL;
		}

		free( msgBuf );
	}
}

void cDevLauncher::NoticeMsgUpdate()
{
	// 실행 모드 옵션 설정
	LRESULT lResult = SendDlgItemMessage( mDevLauncherDlgWnd, IDC_COMBO_RUN_MODE, CB_GETCURSEL, 0, 0 );
	if( lResult != CB_ERR )
	{
		eLAUNCHER_MODE runMode;
		runMode = (eLAUNCHER_MODE)SendDlgItemMessage( mDevLauncherDlgWnd, IDC_COMBO_RUN_MODE, CB_GETITEMDATA, lResult, 0 );
		if( runMode < 0 || runMode >= eLAUNCHER_MODE_MAX )
		{
			SetDlgItemTextA( mDevLauncherDlgWnd, IDC_EDIT_NOTICE, "" );
		}

		char host[MAX_PATH]={0,};
		char addr[MAX_PATH]={0,};

		size_t strLen = 0;

		BYTE key[] = { 0x0d, 0xeb, 0xbe, 0x90 };

		//if( runMode != eLAUNCHER_MODE_CLIENT_DEV  && runMode != eLAUNCHER_MODE_GMTOOL_DEV &&
	    //    runMode != eLAUNCHER_MODE_GMTOOL_TEST && runMode != eLAUNCHER_MODE_GMTOOL_IDC )
#ifdef _SERVER
		if ( runMode == eLAUNCHER_MODE_SERVER_TEST || runMode == eLAUNCHER_MODE_SERVER_IDC )
		{
			strLen = ::strlen( mModeArray[runMode].mHost );
			::memcpy_s( host, sizeof(host), mModeArray[runMode].mHost, strLen );

			for( size_t index = 0; index < strLen; ++index )
				host[index] ^= key[index%4];
		}
		else
		{
			sprintf_s( host, "Iris Test Server" );
		}
#else
		{
			sprintf_s( host, "Iris Test Server" );
		}
#endif

		strLen = ::strlen( mModeArray[runMode].mAddr );
		::memcpy_s( addr, sizeof(addr), mModeArray[runMode].mAddr, strLen );

		for( size_t index = 0; index < strLen; ++index )
			addr[index] ^= key[index%4];

		char msg[MAX_PATH]={0,};

		sprintf_s( msg, "HOST:%s\r\nADDR:%s", host, addr );

		SetDlgItemTextA( mDevLauncherDlgWnd, IDC_EDIT_NOTICE, msg );
	}
	else
		SetDlgItemTextA( mDevLauncherDlgWnd, IDC_EDIT_NOTICE, "" );
}
