#include "stdafx.h"
#include "Patcher.h"
#include "PatcherDlg.h"
#include "httpdownloader.h"

#include "FileFinder.h"
#include "FileUnzipper.h"

#include <winsock2.h>
#include <tlhelp32.h>

#ifdef _DEBUG
#define DebugPrint	printf
#else
#define DebugPrint	//
#endif

class PackKey
{
private:
	charT mKey[20];
public:
	PackKey( );
	charT* GetKey(){ return mKey; }
	int    GetKeyLenth(){ return ::_tcslen( mKey ); }
};

PackKey::PackKey() 
{
	mKey[0] =    41;
	mKey[1] =   -82;
	mKey[2] =   109;
	mKey[3] =   -73;
	mKey[4] =    70;
	mKey[5] =    77;
	mKey[6] =   -48;
	mKey[7] =   -10;
	mKey[8] =   -24;
	mKey[9] =    92;
	mKey[10] =   75;
	mKey[11] =  106;
	mKey[12] =   -7;
	mKey[13] = -114;
	mKey[14] =  -17;
	mKey[15] =  -73;
	mKey[16] = -111;
	mKey[17] =  -65;
	mKey[18] =  -92;
	mKey[19] =    0;
}


/// [12/9/2008 Jo] loader.dll 관련 함수 추가
/*-- Local definitions--*/
#define SafeDelete(P)	{ if (P!=NULL) { delete(P); (P)=NULL; } }       // 메모리를 안전하게 해제.

typedef int			(LibNumberClasses)	(void);
typedef ClassDesc*	(LibClassDesc)		(int i);
typedef ULONG		(LibVersion)		(void);


/*-- Global data--*/
LibVersion*       g_LibVersion       = NULL; 
LibNumberClasses* g_LibNumberClasses = NULL;
LibClassDesc*     g_LibClassDesc     = NULL;

PackKey g_packKey;

cPatcher::cPatcher( cPatcherDlg* dlg )
: mDlg( dlg )
, mWnd( 0 )
, mNumPatchFiles( 0 )
, mEndApp( false )
, mEndIO( true )
, mDelAllPack( false )
, mExecuteMode( eCLIENT_EXECUTE_IDC )
{
	m_packlist.pack = NULL;

	memset( mLoginIP,   0, sizeof( mLoginIP ) );
    memset( mHostAddr,  0, sizeof( mHostAddr) );
    memset( mHostUrl,  0, sizeof( mHostUrl) );
	memset( mHostList,  0, sizeof( mHostList) );
	memset( m_currentDirectory, 0, sizeof( m_currentDirectory ) );

    // url.txt
    strcpy( mHostAddr,  dlg->GetHostAddr());
	strcpy( mHostUrl,   dlg->GetHostUrl());
}

cPatcher::~cPatcher()
{
	Clear();

	SafeDelete( m_libClassDesc );
}

void cPatcher::Init( HWND wnd, HINSTANCE instance)
{
	assert( wnd );

	mWnd = wnd;
	mInstance = instance;

	// 현재 폴더 저장.
	GetCurrentDirectory( sizeof(m_currentDirectory), m_currentDirectory );

	/// [12/9/2008 Jo] Dll 로드
	m_libClassDesc = new Loader;
	if ( m_libClassDesc == NULL )
	{
		MessageBox( wnd, "LOADER Memory allocation Fail", "Error", MB_OK | MB_ICONERROR );
		PostMessage( wnd, WM_CLOSE, 0, 0 );
	}

	// 버젼 정보 체크
	char filename[1024] = { 0, };
	sprintf_s( filename, "%s\\launcher\\ver.txt", m_currentDirectory );
	if( _access( filename, 0) != 0 )
		mDelAllPack = true;
}

void cPatcher::Clear()
{
	if( m_libClassDesc )
		m_libClassDesc->Close( );

	{
		tPointerHashMap< cStringT, Patchfile*>::cIterator iBegin = mPackPatchList.Begin();
		tPointerHashMap< cStringT, Patchfile*>::cIterator iEnd = mPackPatchList.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			Patchfile* file = (Patchfile*)(iBegin->mSecond);
			delete file;
		}
		mPackPatchList.Clear();
	}

	{
		tPointerHashMap< cStringT, Patchfile*>::cIterator iBegin = mUnPackPatchList.Begin();
		tPointerHashMap< cStringT, Patchfile*>::cIterator iEnd = mUnPackPatchList.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			Patchfile* file = (Patchfile*)(iBegin->mSecond);
			delete file;
		}
		mUnPackPatchList.Clear();
	}

	{
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iBegin = mPackUpdateFileMap.Begin();
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iEnd = mPackUpdateFileMap.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			sPackUpdateInfo* pInfo = (sPackUpdateInfo*)(iBegin->mSecond);
			delete pInfo;
		}
		mPackUpdateFileMap.Clear();
	}

	if( m_packlist.pack )
		GlobalFree( m_packlist.pack );

	mNumPatchFiles = 0;
}

void cPatcher::StopWorking()
{
	mEndApp = true;
	mDlg = NULL;
}

unsigned long cPatcher::GetLoginIP()
{
	int strLen = 0;
	strLen = ::strlen( mLoginIP );

	BYTE key[] = { 0x8b, 0x1c, 0x6c, 0x84 };
	for( int index = 0; index < 13; ++index )
	{
		mLoginIP[index] ^= key[index%4];
	}


	unsigned long loginIP    = inet_addr(mLoginIP);
	unsigned long reversPort = htonl( 14400 );

	loginIP ^= reversPort;
	unsigned long reversIP = htonl(loginIP);

	/*
	unsigned long uIP         = strtoul( cmdline, NULL, 10 );
	unsigned long reverseIP   = ntohl( uIP );
	unsigned long reversePort = htonl( 14400 );
	char*         loginIP = NULL;
	in_addr       addr;    

	reverseIP    ^= port;
	addr.s_addr  = reverseIP;

	loginIP = inet_ntoa( addr );
	*/
	return reversIP;
}

void cPatcher::SetProgressMax( ePatchStatus status, int max )
{
	if( mDlg != NULL )
		mDlg->SetProgressMax( status, max );
}

void cPatcher::SetProgressPos( const char* fileName, int pos )
{
	if( mDlg != NULL )
		mDlg->SetProgressPos( fileName, pos );
}

HANDLE cPatcher::GetProcessList( bool* bFuncFailed )
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		*bFuncFailed = true;
		return NULL;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );     // Must clean up the snapshot object!
		*bFuncFailed = true;
		return NULL;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if( stricmp( pe32.szExeFile, LAUNCHER_FILENAME) == 0 || stricmp( pe32.szExeFile, LAUNCHER_DEV_FILENAME ) == 0 || stricmp( pe32.szExeFile, LAUNCHER_TEST_FILENAME ) == 0 )
			return OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );

	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return NULL;
}


DWORD cPatcher::ThreadProc()
{
	assert( mWnd );

	/// Launcher 프로세서 끝났는지 확인.
	bool   bFuncFailed = false;
	HANDLE hLauncherProcess = NULL;

	hLauncherProcess = GetProcessList( &bFuncFailed );

	if( bFuncFailed == true )
	{
		EventWrite("Thread:Failed to scan processers. ");
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_INIT_, (LPARAM)0);
		return 0;
	}

	if( hLauncherProcess != NULL )
	{
		if ( WaitForSingleObject( hLauncherProcess, INFINITE ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode = 0;

			GetExitCodeThread( hLauncherProcess, &exitCode );
			//			if ( exitCode == STILL_ACTIVE )
			//			{
			//				TerminateProcess(hLauncherProcess, exitCode);
			//			}
		}
		CloseHandle( hLauncherProcess );
		hLauncherProcess = NULL;
	}

	Clear();

	char hostaddr[ 1024 ] = { 0, };
	char hosturl [ 1024 ] = { 0, };
	char hostlist[ 1024 ] = { 0, };

	char *pHostAddr = hostaddr;

	if( mInstance == NULL )
	{
		ErrorMessageBox( PATCHER, "Instance Initialize  Error." );
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_INIT_, (LPARAM)0);
		return 0;
	}


#ifdef _DEVSYS
	char DEV_IP[] = {-71, 45, 93, -86, -66, 42, 66, -74, -66, 46, 66, -80, -67, 0};     //211.56.252.46

	memcpy( mLoginIP, DEV_IP, sizeof(DEV_IP) ) ;

	/// 복호화해서 배열에 넣기
	int strLen = 0;
	strLen = ::strlen( DEV_IP );
	BYTE key[] = { 0x8b, 0x1c, 0x6c, 0x84 };
	for( int index = 0; index < strLen; ++index )
	{
		DEV_IP[index] ^= key[index%4];
	}
	pHostAddr = DEV_IP;
	memcpy( hostaddr, DEV_IP, sizeof(DEV_IP) );

	LoadString( mInstance, IDS_UPDATE_URL_INSIDE,  hosturl,  sizeof(hosturl) );
	LoadString( mInstance, IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );

#elif _TEST_SRV

	char TEST_IP[] = { -71, 44, 95, -86, -71, 45, 90, -86, -71, 45, 84, -86, -72, 49, 0};
	memcpy( mLoginIP, TEST_IP, sizeof(TEST_IP) ) ;

    strcpy( hostaddr, mHostAddr );
    strcpy( hosturl,  mHostUrl );
	LoadString( mInstance, IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );

#else

	char IDC_IP[] = { -71, 44, 95, -86, -71, 45, 90, -86, -71, 45, 84, -86, -71, 55, 0};
	memcpy( mLoginIP, IDC_IP, sizeof(IDC_IP) ) ;

	strcpy( hostaddr, mHostAddr );
    strcpy( hosturl,  mHostUrl );
	LoadString( mInstance, IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );

#endif

	//memcpy_s( mHostList, sizeof(mHostList), hostlist,   strlen(hostlist) );

	if( m_libClassDesc->Open( pHostAddr ) ) 
	{
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CONNECT_, (LPARAM)0);
		return 0;
	}

	char  url[ 1024 ];
	char* buffer    = NULL;
	DWORD bufferLen = 0;
	DWORD exitCode  = LPR_CONTINUE;
	DWORD retValue  = 0;

	SetProgressMax( PATCH_SEARCHING, 0 );

	sprintf(url, "%s/%s", hosturl, hostlist);

	retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );


	try
	{
		if( retValue != 0 )
		{

			if(  retValue == ERROR_INTERNET_INVALID_URL )
			{
				PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_NOT_EXIST_PATCHLIST_, (LPARAM)0 );
				return 0;
			}

			int retryConnetion = 0;

			do 
			{
				/// 재 접속
				if( m_libClassDesc->Open( hostaddr ) ) 
				{
					mEndIO = true;
					::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CONNECT_, (LPARAM)0);
					return 0;
				}

				exitCode = LPR_CONTINUE;
				retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

				if( ++retryConnetion > 20 )
				{
					throw(retValue);
				}

				if( mEndApp == true )
				{
					mEndIO = true;
					return 0;
				}

			} while( retValue != 0 );
		}
	} catch( ... ){
		PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_PATCHLIST_, (LPARAM)0 );
		return 0;
	}

	/// 패치리스트 다운로드 및 파싱
	if ( buffer != NULL && bufferLen > 0 && retValue == 0)
	{
		if( UnpackPatchList( &buffer, &bufferLen ) == false ) 
		{
			if( buffer )
				GlobalFree( buffer );
			::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_SAVE_, (LPARAM)0);
			return 0;
		}
		/// 패치리스트 파싱
		eMAKELIST_RETURN_VALUE retMake = MakeList( buffer );

		GlobalFree( buffer );
		if ( retMake != eMAKELIST_SUCCESS )
		{
			int converter = 0;
			switch( retMake )
			{
			case eERROR_MAKELIST_WRONG_HEADER:  converter = _UMT_ERROR_MAKELIST_WRONG_HEADER;   break;
			case eERROR_MAKELIST_WRONG_EXECUTE: converter = _UMT_ERROR_MAKELIST_WRONG_EXECUTE;  break;
			case eERROR_MAKELIST_DIFF_PATCHLIST:converter = _UMT_ERROR_MAKELIST_DIFF_PATCHLIST; break;
			case eERROR_MAKELIST_PARSE:         converter = _UMT_ERROR_MAKELIST_PARSE;          break;
			case eERROR_MAKELIST_END_APP:       converter = _UMT_ERROR_MAKELIST_END_APP;		break;
			default: assert(0); break;
			}
			::PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)converter, (LPARAM)0 );
			return 0;
		}
	}
	else
	{
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_PATCHLIST_, (LPARAM)0);
		return 0;
	}

	/// 클라이언트 파일 정보 수집
	if ( Search( m_currentDirectory, NULL, true ) == -1 )
	{
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_SEARCH_, (LPARAM)0);
		return 0;
	}

	///버젼 정보 파일 저장하기.
	char filename[1024] = { 0, };

	sprintf_s( filename, "%s\\launcher\\ver.txt", m_currentDirectory );

	FILE* stream;
	errno_t  err;
	err = fopen_s( &stream, filename, "wt" );
	if ( err == 0 )
	{
		fputs( "v110", stream );
		fclose( stream );
		stream = NULL;
	}

	/// 폴더 생성
	Packfile* pack = m_packlist.pack;
	for ( ULONG i = 0; i < m_packlist.length && mEndApp == false ; i++, pack++ )
	{
		char     makeDir[ 1024 ]    = { 0, };
		char     foldername[ 1024 ] = { 0, };
		char    *token = NULL;
		const char	seps[] = "/\\";
		bool     makeSubDirectory = false;

		strncpy( foldername, pack->directory , strlen(pack->directory)+1 );

		token = strtok( foldername, seps );

		/// 압축해서 넣지 않는 폴더들은 하위 폴더까지 생성한다. (ex : sound 폴더와 launcher 폴더 )
		for(unsigned int index = 0; index < UNPACK_FOLDER_COUNT-1; ++index )
		{
			if( stricmp( token, gUnpackFolder[index] ) == 0 )
				makeSubDirectory = true;
		}

		sprintf( makeDir, "%s\\", m_currentDirectory );

		while ( token != NULL )
		{
			if( mEndApp == true )
				return 0L;

			strcat( makeDir, token );
			strcat( makeDir, "\\" );

			if ( GetFileAttributes( makeDir ) == INVALID_FILE_ATTRIBUTES )
			{
				if ( CreateDirectory( makeDir, NULL ) == FALSE )
				{
					::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CREATE_FOLDER_, (LPARAM)0);
					return 0L;
				}
			}
			token = strtok( NULL, seps );

			if( makeSubDirectory == false )
				break;
		}
	}


	/// 업데이트 해야할 파일의 개수로 프로그래스바 값 설정.
	mNumPatchFiles = mPackPatchList.GetSize() +  mUnPackPatchList.GetSize();

	if( mNumPatchFiles == 0 )
	{
		SetProgressMax( PATCH_COMPLETE, 0 );

		// 업데이트 완료.
		if( mDlg != NULL && mEndApp == false )
			mDlg->GameReady( true );

		return 0L;
	}

	SetProgressMax( PATCH_PATCHING, mNumPatchFiles);

	int progPos = 0;
	buffer = NULL;
	bufferLen = 0;

	/// none pack
	{
		tPointerHashMap<cStringT, Patchfile*>::cIterator iBegin = mUnPackPatchList.Begin();
		tPointerHashMap<cStringT, Patchfile*>::cIterator iEnd = mUnPackPatchList.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			Patchfile* patchfile = (Patchfile*)(iBegin->mSecond);
			if( patchfile == 0 )
			{
				assert(0);
				continue;
			}

			sprintf( url, "%s/%s.z", hosturl, patchfile->fileInfo.fullPath );
			exitCode = LPR_CONTINUE;

			retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

			try
			{
				if( retValue != 0 )
				{
					if( retValue == ERROR_INTERNET_INVALID_URL )
						throw 0;

					int retryConnetion = 0;

					do 
					{
						/// 재 접속
						if( m_libClassDesc->Open( hostaddr ) ) 
						{
							mEndIO = true;
							::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CONNECT_, (LPARAM)0);
							return 0;
						}

						exitCode = LPR_CONTINUE;
						retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

						if( ++retryConnetion > 20 )
						{
							throw(retValue);
						}

						if( mEndApp == true )
						{
							if( buffer )
								GlobalFree( buffer );
							mEndIO = true;
							return 0;
						}

					} while( retValue != 0 );
				}

				if ( buffer != NULL && bufferLen > 0 && mEndApp == false && retValue == 0 )
				{
					if( UpdateNoPackFile( patchfile , buffer, bufferLen ) == false )
					{
						if( buffer )
							GlobalFree( buffer );
						PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_UPDATE_NOPACK_FILE, (LPARAM)0 );
						return 0;
					}	
				}
				else 
					throw( 0 );

			} catch ( ... )	{
				if( buffer )
					GlobalFree( buffer );
				ErrorMessageBox( PATCHER, "Fail to download [%s] unpack file Error:[%d].", patchfile->fileInfo.fileName, retValue );
				PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_, (LPARAM)0 );
				return 0;
			}

			if( buffer )
				GlobalFree( buffer );
			/// 진행바
			SetProgressPos( 0, ++progPos );	
		}
	}

	/// pack 정리하기
	{
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iBegin = mPackUpdateFileMap.Begin();
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iEnd = mPackUpdateFileMap.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			cStringT packName = (cStringT)iBegin->mFirst;
			sPackUpdateInfo* pInfo = (sPackUpdateInfo*)(iBegin->mSecond);

			bool bAddFiles = false;
			cFilePack pack;
			if( _access( packName.Cstr(), 0 ) == 0 )
			{
				/// 파일 존재
				if( pack.Open( packName.Cstr(), cFilePack::OPEN) == true )
				{
					/// 열기 성공
					if( pack.SetPasswordEx( g_packKey.GetKey(), g_packKey.GetKeyLenth() ) == true )
					{
						mEndIO = false;
						/// 패스워드 성공. 기존에 .pack 파일 안에 있는 파일을 업데이트 시킨것이라면 기존 파일을 삭제
						pack.DeleteFiles( &(pInfo->updateFiles) );
						bAddFiles = true;
					}
					else
					{
						/// 패스워드 실패
						EventWrite("Thread:Fail to UNKNOWN ERROR [%s]pack file. ",packName.Cstr() );
						pack.Close();
						DeleteFile( packName.Cstr() );
						PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_UPDATE_ACCESS_PACK_FILE_, (LPARAM)0 );
						return 0;
					}
				}
				else
				{
					/// 열기 실패
					EventWrite("Thread:Fail to Open [%s]pack file. ",packName.Cstr() );
					pack.Close();
					DeleteFile( packName.Cstr() );
					PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_UPDATE_OPEN_PACK_FILE_, (LPARAM)0 );
					return 0;
				}
			}
			else
			{
				/// 파일 존재 안함
				mEndIO = false;
				if( pack.Open( packName.Cstr(), cFilePack::CREATE) == true )
				{
					/// 생성 성공
					if( pack.SetPasswordEx( g_packKey.GetKey() , g_packKey.GetKeyLenth() ) == true )
					{
						/// 패스워드 성공
						bAddFiles = true;
						mEndIO = true;
						if( mEndApp )
							return 0;
					}
					else
					{
						/// 패스워드 실패
						EventWrite("Thread:Fail to UNKNOWN ERROR [%s]pack file. ",packName.Cstr() );
						pack.Close();
						DeleteFile( packName.Cstr() );
						PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_UPDATE_ACCESS_PACK_FILE_, (LPARAM)0 );
						return 0;
					}
				}
				else
				{
					/// 생성 실패
					EventWrite("Thread:Fail to Create [%s]pack file. ",packName.Cstr() );
					m_libClassDesc->Close( );
					mEndIO = true;
					PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CREATE_PACK_FILE_, (LPARAM)0 );
					return 0;
				}
			}

			if( bAddFiles == true )
			{

				cPatchfileList::cIterator i = pInfo->patchfiles.Begin();
				cPatchfileList::cIterator e = pInfo->patchfiles.End();
				for( ; i != e; ++i )
				{
					Patchfile* file = (Patchfile*)(*i).mSecond;

					sprintf( url, "%s/%s.z", hosturl, file->fileInfo.fullPath );
					exitCode = LPR_CONTINUE;

					retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

					printf("Get [%s]file : [%d]byte -> error value[%d]\n", file->fileInfo.fileName, bufferLen, retValue );

					try
					{
						if( retValue != 0 )
						{
							if( retValue == ERROR_INTERNET_INVALID_URL )
								throw 0;

							int retryConnetion = 0;

							do 
							{
								/// 재 접속
								if( m_libClassDesc->Open( hostaddr ) ) 
								{
									mEndIO = true;
									pack.Close();
									::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CONNECT_, (LPARAM)0);
									return 0;
								}

								retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

								if( ++retryConnetion > 20 )
								{
									throw(retValue);
								}

								if( mEndApp == true )
								{
									if( buffer )
										GlobalFree( buffer );
									pack.Close();
									mEndIO = true;
									return 0;
								}

							} while( retValue != 0 );
						}

						if ( buffer != NULL && bufferLen > 0 && mEndApp == false && retValue == 0 )
						{
							if( UpdatePackFile( file , buffer, bufferLen, &pack ) == false )
							{
								if( buffer )
									GlobalFree( buffer );
								pack.Close();
								mEndIO = true;
								PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_UPDATE_PACK_FILE, (LPARAM)0 );
								return 0;
							}
						}
						else 
							throw( 0 );

					} catch ( ... )	{

						if( buffer )
							GlobalFree( buffer );
						pack.Close();
						mEndIO = true;
						ErrorMessageBox( PATCHER, "Fail to download [%s] pack file Error:[%d].",  file->fileInfo.fileName, retValue );
						PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_, (LPARAM)0 );
						return 0;
					}

					if( buffer )
						GlobalFree( buffer );
					/// 진행바
					SetProgressPos( 0, ++progPos );	

					if( mEndApp == true )
					{
						pack.Close();
						mEndIO = true;
						return 0;
					}
				}
			}

			pack.Close();
			mEndIO = true;
			if( mEndApp == true )
				return 0;
		}
	}

	SetProgressMax( PATCH_COMPLETE, 0 );

	// 업데이트 완료.
	if( mDlg != NULL && mEndApp == false )
		mDlg->GameReady( true );

	return 0L;
}


/*-- MakeList Method
*/
eMAKELIST_RETURN_VALUE cPatcher::MakeList(char* buffer)
{
	// 메모리 토큰.
	char  seps[] = "\r\n/;";
	char* token;

	// Header
	token = strtok( (char*)buffer, seps );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	if ( strcmp( token, "patchlist" ) )
		return eERROR_MAKELIST_WRONG_HEADER;

	token = strtok( NULL, seps );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	if ( strcmp( token, "ver.110" ) )
		return eERROR_MAKELIST_WRONG_HEADER;		

	token = strtok( NULL, seps );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	m_packlist.offset  = 0;
	m_packlist.length  = atoi( token );
	m_packlist.pack    = (Packfile*)GlobalAlloc( GPTR, sizeof(Packfile) * m_packlist.length );

	if( m_packlist.pack == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	token = strtok( NULL, seps );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	unsigned long patchFileCount = ::atoi(token);

	// Body
	Packfile* packfile = m_packlist.pack;
	for ( ULONG i = 0; i < m_packlist.length && mEndApp == false; i++, packfile++ )
	{
		// 폴더이름
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		strcpy( packfile->directory, token );
	}

	Patchfile* patchfile  = 0;
	bool       IsPackFile = false;
	for( unsigned long i = 0; i< patchFileCount && mEndApp == false; i++ )
	{
		patchfile = new Patchfile;

		// 파일이름
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		//strcpy( patchfile->fileName, token );
		IsPackFile = SetPackFileNameInfo( token, &patchfile->fileInfo );

		// 파일크기-Low
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->sizeLow = (DWORD)atof( token );

		// 파일크기-High
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->sizeHigh = (DWORD)atof( token );

		// 파일날짜-Low
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->writeTime.dwLowDateTime = (DWORD)atof( token );

		// 파일날짜-High
		if ( (token = strtok( NULL, seps )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->writeTime.dwHighDateTime = (DWORD)atof( token );

		if( IsPackFile == true )
		{
			if( mPackPatchList.Insert( patchfile->fileInfo.fullPath, patchfile ) == false )
			{
				assert(0);
			}
		}
		else
		{
			if( mUnPackPatchList.Insert( patchfile->fileInfo.fullPath, patchfile ) == false )
			{
				assert(0);
			}
		}
	}

	if(mEndApp == true)
		return eERROR_MAKELIST_END_APP;

	return eMAKELIST_SUCCESS;
}

/*-- GetString Prototype --*/
int cPatcher::GetString(int id, char* buffer, int bufferSize )
{
	return ( mInstance != NULL) ? LoadString( mInstance, id, buffer, bufferSize ) : 0;
}

/*-- LoaderProgressRoutine Method --*/
DWORD cPatcher::LoaderProgressRoutine(DWORD totalFileSize, DWORD totalBytesTransferred, void* data)
{
	if( totalFileSize == totalBytesTransferred ) //파일 전송을 다 했다면
		return 1;	

	return 0;
}


/*-- FileTimeToDOS Method --*/
void cPatcher::FileTimeToDOS(FILETIME* filetime)
{
	WORD fatDate, fatTime;
	FileTimeToDosDateTime( filetime, &fatDate, &fatTime );
	DosDateTimeToFileTime( fatDate, fatTime, filetime );
}



/*-- UnpackPatchList Method --*/
bool cPatcher::UnpackPatchList( char** buffer, DWORD *bufferLen )
{
	char   hostlist[1024];
	char   packname[1024];
	HANDLE file;
	DWORD  written=0;
	BOOL   ret=FALSE;

	GetString( IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );
	sprintf( packname, "%s", hostlist );

	mEndIO = false;
	/// 다운 받은 patchlist.txt.z파일 저장.
	file = CreateFile( packname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( file != INVALID_HANDLE_VALUE )
	{
		ret = WriteFile( file, (*buffer), (*bufferLen), &written, NULL );
		CloseHandle( file );
		mEndIO = true;
		GlobalFree( (*buffer) );
	}

	if ( file == INVALID_HANDLE_VALUE || ret == FALSE || mEndApp == true )
	{
		ErrorMessageBox( "UnpackPathchList", "Fail to save patch list file ");
		CloseHandle( file );
		mEndIO = true;
		GlobalFree( (*buffer) );
		return false;
	}

	/// 파일 압축 풀기
	if ( written == (*bufferLen) )
	{
		mEndIO = false;
		gzFile gz = gzopen( packname, "rb" );
		if ( gz != NULL )
		{
			char  buf[g16KBYTE];			// CHUNK 16KByte
			DWORD len = sizeof(buf);
			DWORD bytesRead    = 1;
			DWORD totalBytes   = 0;

			/// 파일 크기 얻기
			while ( bytesRead > 0 )
			{
				bytesRead = gzread( gz, buf, len );
				totalBytes += bytesRead;

				if( mEndApp == true)
				{
					gzclose( gz );
					mEndIO = true;
					DeleteFile( packname );
					return false;
				}
			}

			//메모리 할당
			(*bufferLen) = totalBytes;
			(*buffer) =  (char*)GlobalAlloc( GPTR, (*bufferLen)+1 );

			if ( (*buffer) == NULL || mEndApp == true )
			{
				ErrorMessageBox( "UnpackPathchList", "Failed to allocation patchlist buffer. ");
				GlobalFree( (*buffer) );
				gzclose( gz );
				mEndIO = true;
				DeleteFile( packname );
				return false;
			}

			gzrewind(gz);

			bytesRead = gzread( gz, (*buffer), (*bufferLen) );

			if( bytesRead != (*bufferLen) )
			{
				ErrorMessageBox( "UnpackPathchList", "Failed to read patchlist file.");
				gzclose( gz );
				mEndIO = true;
				GlobalFree( (*buffer) );
				return false;
			}

			gzclose( gz );
			DeleteFile( packname );
			mEndIO = true;
			if( mEndApp == true )
				return false;
		}
		mEndIO = true;
		return true;
	}

	return false;
}


/*-- UpdateFile Method --*/
bool cPatcher::UpdateNoPackFile(Patchfile* patchfile, char* buffer, DWORD bufferLen )
{
	HANDLE file;
	DWORD  written;
	bool   retcode = true;
	char  *buf = NULL;
	DWORD  bufLen = 0;

	DWORD start_time;

	start_time = GetTickCount();

	mEndIO = false;

	/// 임시 폴더/파일명 얻기
	unsigned int ret;
	char tempDirectory[MAX_PATH];
	char tempname[MAX_PATH];

	/// 파일 다운로드 및 압축을 풀어서 버퍼에 저장.
	GetTempPath( sizeof(tempDirectory), tempDirectory );
	ret = GetTempFileName( tempDirectory, "Iris\0", 0, tempname );
	if( ret == 0 )
	{
		ErrorPrint( "GetTempFileName Error" );
		return false;
	}

	file = CreateFile( tempname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file != INVALID_HANDLE_VALUE )
	{

		if( WriteFile( file, buffer, bufferLen, &written, NULL ) == FALSE )
		{
			ErrorMessageBox( "UpdateNoPackFile", "Fail to save [%s] file. Error:[%d]", patchfile->fileInfo.fileName, GetLastError() );
			CloseHandle( file );
			DeleteFile( tempname );
			mEndIO = true;
			return false;
		}

		CloseHandle( file );
		mEndIO = true;
		if( mEndApp == true )
		{
			DeleteFile( tempname );
			return false;
		}

		if( written == bufferLen )
		{
			mEndIO = false;
			gzFile gz = gzopen( tempname, "rb" );
			if ( gz != NULL )
			{
				char  readBuf[g16KBYTE];			// CHUNK 16KByte
				DWORD len = sizeof(readBuf);
				DWORD bytesRead    = 0;
				DWORD totalBytes   = 0;

				/// 파일 크기 얻기
				while ( true )
				{
					bytesRead = gzread( gz, readBuf, len );
					totalBytes += bytesRead;

					if( mEndApp == true )
					{
						gzclose( gz );
						mEndIO = true;
						DeleteFile( tempname );
						return false;
					}

					if( bytesRead < 1 )
						break;

				}

				//메모리 할당
				bufLen = totalBytes;
				buf =  (char*)GlobalAlloc( GPTR, bufLen+1 );
				printf(" downloaded [%s]file size:[%d]byte, after decompression file size:[%d]byte\n", patchfile->fileInfo.fileName, bufferLen, totalBytes );

				if ( buf == NULL )
				{
					ErrorMessageBox( "UpdateNoPackFile", "Fail to Memory Allocation." );

					gzclose( gz );
					mEndIO = true;
					GlobalFree( buf );
					DeleteFile( tempname );
					return false;
				}

				gzrewind(gz);

				bytesRead = gzread( gz, buf, bufLen );
				if( bytesRead != bufLen )
				{
					ErrorMessageBox( "UpdateNoPackFile", "Fail to Copy for buffer from [%s] Unpackfile.", patchfile->fileInfo.fileName );
					gzclose( gz );
					mEndIO = true;
					GlobalFree( buf );
					DeleteFile( tempname );
					return false;
				}

				char reverse_buf[ g16KBYTE ];
				DWORD copied_size;
				char *pBuf;
				char *pCopyied_Pos;
				char *pReverse;

				pBuf         = buf;
				pCopyied_Pos = buf; // 현재 복사 위치
				copied_size  = 0;

				/// 16k 씩 거꾸로 저장했던 파일을 원상태로 돌리기
				while ( copied_size < bufLen )
				{
					// 복사해야 할 메모리 크기
					DWORD remain_size = bufLen - copied_size;

					pReverse = reverse_buf;

					// 복사할 메모리의 크기가 16k 이하인 경우
					if( remain_size < g16KBYTE )
					{
						pBuf     = pCopyied_Pos + remain_size - 1;

						for( DWORD cnt = 0; cnt < remain_size; ++cnt )
						{
							*pReverse = *pBuf;
							--pBuf;
							++pReverse;
						}
						memcpy( &buf[copied_size] , reverse_buf, remain_size );
						copied_size += remain_size;
					}
					else
					{
						pBuf = pCopyied_Pos + g16KBYTE - 1;

						for( DWORD cnt = 0; cnt < g16KBYTE; ++cnt )
						{
							*pReverse = *pBuf;
							--pBuf;
							++pReverse;
						}
						memcpy( &buf[ copied_size ], reverse_buf, g16KBYTE );
						copied_size += g16KBYTE;
						pCopyied_Pos += g16KBYTE;
					}

				}
			}
			else
			{
				ErrorMessageBox( "UpdateNoPackFile", "Fail to open download [%s] file.", patchfile->fileInfo.fileName );
				DeleteFile( tempname );
				mEndIO = true;
				return false;
			}

			gzclose( gz );
			mEndIO=true;

		}
		else 
		{
			/// 파일이 제대로 안써졌다면 파일 쓰기 에러.
			ErrorMessageBox( "UpdateNoPackFile", "Fail to save temp update [%s] file.", patchfile->fileInfo.fileName );
			DeleteFile( tempname );
			return false;
		}

		if( DeleteFile( tempname ) == FALSE )
		{
			EventWrite( "UpdateNoPackFile: Fail to delete temp [%s] file. ",patchfile->fileInfo.fileName );
		}
	}
	else
	{
		ErrorMessageBox( "UpdateNoPackFile", "Fail to Create temp [%s] file. Error:[%d]",patchfile->fileInfo.fileName, GetLastError() );
		CloseHandle( file );
		mEndIO = true;
		DeleteFile( tempname );
		return false;
	}

	if( mEndApp == true )
		return false;

	char newName[ 1024 ] = { 0, };
	sprintf(newName, "%s\\%s\0", m_currentDirectory, patchfile->fileInfo.fullPath );

	/// 압축이 필요없는 파일은 바로 파일로 저장하여 기존 파일을 삭제하고 추가한다.
	DWORD bytesWritten = 0;
	if( DeleteFile( newName ) == FALSE && GetLastError() != ERROR_FILE_NOT_FOUND)
	{
		EventWrite( "UpdateFile: Fail to Delete old [%s] file. Error Code:[%d] ", newName, GetLastError() );
	}

	mEndIO = false;
	file = CreateFile( newName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( file == INVALID_HANDLE_VALUE )
	{
		ErrorMessageBox( "UpdateNoPackFile", "Fail to Create file [%s] file. Error Code:[%d] ", newName, GetLastError() );
		CloseHandle( file );
		mEndIO = true;
		return false;
	}

	if ( WriteFile( file, buf, bufLen, &bytesWritten, NULL ) == FALSE || file == INVALID_HANDLE_VALUE )
	{
		ErrorMessageBox( "UpdateNoPackFile", "Fail to Write File [%s] file. Error Code:[%d] ", newName, GetLastError() );
		CloseHandle( file );
		mEndIO = true;
		return false;
	}

	/// 파일 날짜 설정
	SetFileTime( file, NULL, NULL, &patchfile->writeTime );

	DWORD    sizeHigh;
	DWORD    sizeLow;
	sizeLow = GetFileSize( file, &sizeHigh );

	CloseHandle( file );
	mEndIO = true;
	if( mEndApp == true )
		return false;

	retcode = patchfile->sizeLow == sizeLow && patchfile->sizeHigh == sizeHigh;

	GlobalFree( buf );

	if( retcode == false )
	{	
		ErrorMessageBox( "UpdateNoPackFile", "download [%s] file different on the list.", patchfile->fileInfo.fileName ); 
	}

	return retcode;
}

bool cPatcher::UpdatePackFile(Patchfile* patchfile, char* buffer, DWORD bufferLen, cFilePack *pack  )
{
	HANDLE file;
	DWORD  written;
	char  *buf = NULL;
	DWORD  bufLen = 0;

	DWORD start_time;

	start_time = GetTickCount();

	/// 임시 폴더/파일명 얻기
	unsigned int ret;
	char tempDirectory[MAX_PATH];
	char tempname[MAX_PATH];

	/// 파일 다운로드 및 압축을 풀어서 버퍼에 저장.
	GetTempPath( sizeof(tempDirectory), tempDirectory );
	ret = GetTempFileName( tempDirectory, "Iris\0", 0, tempname );
	if( ret == 0 )
	{
		ErrorPrint( "GetTempFileName Error" );
		return false;
	}

	mEndIO = false;
	file = CreateFile( tempname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file != INVALID_HANDLE_VALUE )
	{		
		if( WriteFile( file, buffer, bufferLen, &written, NULL ) == FALSE )
		{
			ErrorMessageBox( "UpdateFile", "Fail to save [%s] file. Error:[%d]", patchfile->fileInfo.fileName, GetLastError() );
			CloseHandle( file );
			DeleteFile( tempname );
			pack->Close();
			mEndIO = true;
			return false;
		}

		CloseHandle( file );

		if( written == bufferLen )
		{
			gzFile gz = gzopen( tempname, "rb" );
			if ( gz != NULL )
			{
				char  readBuf[g16KBYTE];			// CHUNK 16KByte
				DWORD len = sizeof(readBuf);
				DWORD bytesRead    = 0;
				DWORD totalBytes   = 0;

				/// 파일 크기 얻기
				while ( true )
				{
					bytesRead = gzread( gz, readBuf, len );
					totalBytes += bytesRead;

					if( mEndApp == true )
					{
						gzclose( gz );
						DeleteFile( tempname );
						mEndIO = true;
						return false;
					}

					if( bytesRead < 1 )
						break;

				}

				//메모리 할당
				bufLen = totalBytes;
				buf =  (char*)GlobalAlloc( GPTR, bufLen+1 );
				printf(" downloaded [%s]file size:[%d]byte, after decompression file size:[%d]byte\n", patchfile->fileInfo.fileName, bufferLen, totalBytes );

				if ( buf == NULL )
				{
					ErrorMessageBox( "UpdatePackFile", "Fail to Memory Allocation." );
					gzclose( gz );
					GlobalFree( buf );
					DeleteFile( tempname );
					return false;
				}
				gzrewind(gz);

				bytesRead = gzread( gz, buf, bufLen );
				if( bytesRead != bufLen )
				{
					ErrorMessageBox( "UpdatePackFile", "Fail to Copy for buffer from [%s] Unpackfile. ",patchfile->fileInfo.fileName );
					gzclose( gz );
					GlobalFree( buf );
					DeleteFile( tempname );
					return false;
				}

				char reverse_buf[ g16KBYTE ];
				DWORD copied_size;
				char *pBuf;
				char *pCopyied_Pos;
				char *pReverse;

				pBuf         = buf;
				pCopyied_Pos = buf; // 현재 복사 위치
				copied_size  = 0;

				/// 16k 씩 거꾸로 저장했던 파일을 원상태로 돌리기
				while ( copied_size < bufLen )
				{
					// 복사해야 할 메모리 크기
					DWORD remain_size = bufLen - copied_size;

					pReverse = reverse_buf;

					// 복사할 메모리의 크기가 16k 이하인 경우
					if( remain_size < g16KBYTE )
					{
						pBuf     = pCopyied_Pos + remain_size - 1;

						for( DWORD cnt = 0; cnt < remain_size; ++cnt )
						{
							*pReverse = *pBuf;
							--pBuf;
							++pReverse;
						}
						memcpy( &buf[copied_size] , reverse_buf, remain_size );
						copied_size += remain_size;
					}
					else
					{
						pBuf = pCopyied_Pos + g16KBYTE - 1;

						for( DWORD cnt = 0; cnt < g16KBYTE; ++cnt )
						{
							*pReverse = *pBuf;
							--pBuf;
							++pReverse;
						}
						memcpy( &buf[ copied_size ], reverse_buf, g16KBYTE );
						copied_size += g16KBYTE;
						pCopyied_Pos += g16KBYTE;
					}

				}
			}
			else
			{
				ErrorMessageBox( "UpdatePackFile", "Fail to open download [%s] file.", patchfile->fileInfo.fileName );
				DeleteFile( tempname );
				return false;
			}

			gzclose( gz );
		}
		else 
		{
			/// 파일이 제대로 안써졌다면 파일 쓰기 에러.
			ErrorMessageBox( "UpdatePackFile", "Fail to save temp update [%s] file. ",patchfile->fileInfo.fileName );
			DeleteFile( tempname );
			return false;
		}

		if( DeleteFile( tempname ) == FALSE )
		{
			EventWrite( "UpdatePackFile: Fail to delete temp [%s] file. ",patchfile->fileInfo.fileName );
		}
	}
	else
	{
		ErrorMessageBox( "UpdateFile", "Fail to Create temp [%s] file. Error:[%d] ",patchfile->fileInfo.fileName, GetLastError() );
		CloseHandle( file );
		DeleteFile( tempname );
		return false;
	}

	if( mEndApp == true )
	{
		pack->Close();
		mEndIO = true;
		return false;
	}

	/// 파일 하나의 크기가 DWORD 형 크기를 안넘어간다고 가정. 후에 DWORD 형 크기(4.2GByte)를 넘어가는 크기의 파일이 있다면 개선해야함
	if( bufLen != patchfile->sizeLow )
	{
		ErrorMessageBox( "UpdateFile", "PatchFile Size Error[%s].",patchfile->fileInfo.fileName );

		pack->Close();
		mEndIO = true;
		return false;
	}

	/// .pack 파일에 추가 루틴
	cFileInPackInfo fipi;
	fipi.SetPathName( patchfile->fileInfo.fullPath );

	time_t time;
	FILETIME write;

	write = patchfile->writeTime;

	const __int64 SecsTo100ns = 10000000;
	const __int64 SecsBetweenEpochs = 11644473600;
	__int64 converted_time;
	converted_time = ((__int64)write.dwHighDateTime << 32) + write.dwLowDateTime;
	converted_time -= (SecsBetweenEpochs * SecsTo100ns);
	converted_time /= SecsTo100ns;
	time = (time_t)converted_time;

	fipi.SetWriteTime( time );
	mEndIO = false;
	if( pack->OpenNewFile( fipi ) == false )
	{	
		ErrorMessageBox( "UpdatePackFile", "Fail to OpenNewFile [%s] file.", patchfile->fileInfo.fileName );
		pack->CloseNewFile();
		GlobalFree( buf );
		pack->Close();
		mEndIO = true;
		return false;
	}
	///엑스트라 필드 추가 됐는지 검사하기.
	FILETIME* retTime = NULL;
	retTime = (FILETIME*)pack->SetExtraFieldEx( ( char* )&(patchfile->writeTime), sizeof(FILETIME) );
	if ( memcmp( retTime, &(patchfile->writeTime), sizeof(FILETIME) ) != 0 )
	{
		ErrorMessageBox( "UpdatePackFile", "Fail to SetExtraField [%s] file.", patchfile->fileInfo.fileName );
		pack->CloseNewFile();
		GlobalFree( buf );
		pack->Close();
		mEndIO = true;
		return false;
	}
	if( pack->WriteNewFile( buf, bufLen ) == false )
	{
		ErrorMessageBox( "UpdatePackFile", "Fail to WriteNewFile [%s] file.", patchfile->fileInfo.fileName );
		pack->CloseNewFile();
		GlobalFree( buf );
		pack->Close();
		mEndIO = true;
		return false;
	}
	/// 파일 닫기 에러시 추가 했던 파일 다시 삭제
	if( pack->CloseNewFile() != true )
	{
		CZipStringArray arr;
		arr.Add( patchfile->fileInfo.fullPath );
		pack->DeleteFiles( &arr );
	}
	GlobalFree( buf );


	mEndIO = true;
	return true;
}

/*-- Search Method
*/
int cPatcher::Search(char* root, char* sub, bool opcode)
{
	/// no pack file check
	{
		tPointerHashMap< cStringT, Patchfile*>::cIterator iBegin = mUnPackPatchList.Begin();
		tPointerHashMap< cStringT, Patchfile*>::cIterator iEnd = mUnPackPatchList.End();
		for( ; iBegin != iEnd; )
		{
			Patchfile* file = (Patchfile*)(*iBegin++).mSecond;
			if( file == 0 )
			{
				assert(0);
				continue;
			}

			HANDLE search = NULL;
			char filename[ 1024 ] = {0,};
			WIN32_FIND_DATA fileData;
			sprintf( filename, "%s\\%s", m_currentDirectory, file->fileInfo.fullPath );

			search = FindFirstFile( filename, &fileData );
			if( search != INVALID_HANDLE_VALUE ) // && fileData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE )
			{
				if( fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
					SetFileAttributes( file->fileInfo.fullPath, FILE_ATTRIBUTE_ARCHIVE );

				DWORD sizeLow = fileData.nFileSizeLow;
				FILETIME fileTime = fileData.ftLastWriteTime;

				FileTimeToDOS( &fileTime );
				FileTimeToDOS( &file->writeTime );

				if( (file->sizeLow == sizeLow) && 
					(memcmp( &file->writeTime, &fileTime, sizeof(FILETIME) ) == 0) )
				{
					mUnPackPatchList.Erase( file->fileInfo.fullPath );
					delete file;
				}
			}
		}
	}

	/// pack 파일 분석
	{
		tPointerHashMap<cStringT, Patchfile*>::cIterator iBegin = mPackPatchList.Begin();
		tPointerHashMap<cStringT, Patchfile*>::cIterator iEnd = mPackPatchList.End();

		for( ; iBegin != iEnd; ++iBegin )
		{
			Patchfile* files = (Patchfile*)(iBegin->mSecond);

			char packFileNameWithPath[1024] = {0,};
			sprintf( packFileNameWithPath,"%s\\%s\\%s.pack", m_currentDirectory, files->fileInfo.rootFolder, files->fileInfo.packFolder );

			sPackUpdateInfo* pInfo = mPackUpdateFileMap.GetAt( packFileNameWithPath );
			if( pInfo == 0 )
			{
				pInfo = new sPackUpdateInfo;
				mPackUpdateFileMap.Insert( packFileNameWithPath, pInfo );
			}

			pInfo->patchfiles.Insert( files->fileInfo.fullPath, files );
		}
	}

	/// pack file check
	{
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iBegin = mPackUpdateFileMap.Begin();
		tPointerHashMap< cStringT, sPackUpdateInfo*>::cIterator iEnd = mPackUpdateFileMap.End();
		for( ; iBegin != iEnd; ++iBegin )
		{
			cStringT packName = (*iBegin).mFirst;
			sPackUpdateInfo* pInfo = (sPackUpdateInfo*)(*iBegin).mSecond;
			if( pInfo == 0 )
			{
				assert(0);
				continue;
			}

			if( mDelAllPack == true )
			{
				if( _access( packName.Cstr(), 0) == 0 )
					DeleteFile( packName.Cstr() );
			}

			cFilePack pack;
			if( pack.Open( packName.Cstr(), cFilePack::OPEN_READONLY ) == true )
			{
				for( unsigned int index = 0, index_limit = pack.GetNumFiles(); index < index_limit && mEndApp == false; ++index )
				{
					/// .pack 열기
					cFileInPackInfo fipi;
					if( pack.GetFileInfo( &fipi, index) == false )
						continue;

					if( fipi.IsDirectory() )
						continue;

					Patchfile* patchFile = pInfo->patchfiles.GetAt( fipi.GetPathName().Cstr() );
					if( patchFile )
					{
						DWORD sizeLow = fipi.GetSize();
						FILETIME* fileTime = fipi.GetWriteTimeInExtraBuf();

						if( fileTime )
						{
							if( (patchFile->sizeLow == sizeLow) && (memcmp( &patchFile->writeTime, fileTime, sizeof(FILETIME) ) == 0) )
							{
								if( patchFile )
								{
									delete patchFile;
									patchFile = 0;
								}
								
								pInfo->patchfiles.Erase( fipi.GetPathName().Cstr() );
								mPackPatchList.Erase( fipi.GetPathName().Cstr() );
							}
						}
					}
				}
				pack.Close();
			}
			else
			{
				if( _access( packName.Cstr(), 0) == 0 )
				{
					pack.Close();
					DeleteFile( packName.Cstr() );
				}
			}

			if( mEndApp == true )
				return 0;

			cPatchfileList::cIterator i = pInfo->patchfiles.Begin();
			cPatchfileList::cIterator e = pInfo->patchfiles.End();
			for( ; i != e; ++i )
			{
				Patchfile* files = (Patchfile*)(*i).mSecond;
				if( files )
					pInfo->updateFiles.Add( files->fileInfo.fullPath );
			}
		}
	}

	return 0;
}

bool cPatcher::SetPackFileNameInfo( char* fullPath, sFilePathInfo* fileInfo )
{
	char* pCurrentPos = fullPath;
	char* pFindPos    = fullPath;

	strncpy( fileInfo->fullPath, fullPath, strlen(fullPath) );

	pFindPos = ::strchr( pCurrentPos, '\\');
	if( pFindPos == 0 )
	{
		/// 폴더에 속하지 않는 파일 셋팅
		strncpy( fileInfo->fileName, fullPath, strlen(fullPath) );
		return false;
	}
	else
	{
		/// 폴더 이름 셋팅
		strncpy( fileInfo->rootFolder, fullPath, pFindPos - fullPath );

		/// \ 길이(1) 만큼 이동
		pCurrentPos = pFindPos + 1;
		pFindPos = ::strchr( pCurrentPos, '\\');
		if( pFindPos == 0 )
		{
			/// 폴더 안에 포함된 파일 이름 셋팅
			strncpy( fileInfo->fileName, pCurrentPos, strlen(pCurrentPos) );
			return false;
		}
		else
		{
			/// 팩 폴더 이름 셋팅
			strncpy( fileInfo->packFolder, pCurrentPos, pFindPos - pCurrentPos );	
			pCurrentPos = pFindPos + 1;
			/// 파일 이름 셋팅
			strncpy( fileInfo->fileName, pCurrentPos, strlen(pCurrentPos) );

			for(unsigned int index = 0; index < UNPACK_FOLDER_COUNT; ++index )
			{
				/// pack 검사
				if( ::stricmp( fileInfo->rootFolder, gUnpackFolder[index] ) == 0 )
				{
					return false;
				}
			}
			return true;
		}
	}
}

bool cPatcher::PatchlistCheck( )
{
	Clear();

	SetProgressMax( PATCH_LIST_CHECKING, 1 );

	char    url[ 1024 ];
	char*   buffer    = NULL;
	DWORD   bufferLen = 0;
	DWORD   exitCode  = 0;
	DWORD   retValue  = 0;

	sprintf(url, "%s/%s", mHostUrl, mHostList);

	// patchlist.txt.z 파일 다운
	retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );
	try
	{
		if( retValue != 0 )
		{
			if( retValue == ERROR_INTERNET_INVALID_URL )
				throw 0;

			int retryConnetion = 0;

			do 
			{
				/// 재 접속
				if( m_libClassDesc->Open( mHostAddr ) ) 
				{
					mEndIO = true;
					::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_CONNECT_, (LPARAM)0);
					return 0;
				}

				retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &exitCode, this, &buffer, &bufferLen );

				if( ++retryConnetion > 20 )
				{
					throw(retValue);
				}

				if( mEndApp == true )
				{
					mEndIO = true;
					return 0;
				}

			} while( retValue != 0 );
		}
	} catch( ... ){
		PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_PATCHLIST_, (LPARAM)0 );
		return false;
	}


	// patchlist.txt 내용 파싱
	if ( buffer != NULL && bufferLen > 0 && retValue == 0)
	{
		if( UnpackPatchList( &buffer, &bufferLen ) == false ) 
		{
			if( buffer )
				GlobalFree( buffer );
			::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_SAVE_, (LPARAM)0);
			return 0;
		}
		/// 패치리스트 파싱
		eMAKELIST_RETURN_VALUE retMake = MakeList( buffer );

		GlobalFree( buffer );
		if ( retMake != eMAKELIST_SUCCESS )
		{
			int converter = 0;
			switch( retMake )
			{
			case eERROR_MAKELIST_WRONG_HEADER:  converter = _UMT_ERROR_MAKELIST_WRONG_HEADER;   break;
			case eERROR_MAKELIST_WRONG_EXECUTE: converter = _UMT_ERROR_MAKELIST_WRONG_EXECUTE;  break;
			case eERROR_MAKELIST_DIFF_PATCHLIST:converter = _UMT_ERROR_MAKELIST_DIFF_PATCHLIST; break;
			case eERROR_MAKELIST_PARSE:         converter = _UMT_ERROR_MAKELIST_PARSE;          break;
			case eERROR_MAKELIST_END_APP:       converter = _UMT_ERROR_MAKELIST_END_APP;		break;
			default: assert(0); break;
			}
			::PostMessage( mWnd, WM_USER_MESSAGE, (WPARAM)converter, (LPARAM)0 );
			return 0;
		}
	}
	else
	{
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_DOWNLOAD_PATCHLIST_, (LPARAM)0);
		return 0;
	}

	mNumPatchFiles = mUnPackPatchList.GetSize() + mPackPatchList.GetSize();
	SetProgressMax( PATCH_LIST_CHECKING, mNumPatchFiles );

	/// 클라이언트 파일 정보 수집
	if ( Search( m_currentDirectory, NULL, true ) == -1 )
	{
		::PostMessage(mWnd, WM_USER_MESSAGE, (WPARAM)_UMT_ERROR_SEARCH_, (LPARAM)0);
		return 0;
	}

	SetProgressMax( PATCH_LIST_CHECK_END, mNumPatchFiles );

	if ( mUnPackPatchList.IsEmpty() == false || mPackPatchList.IsEmpty() == false )
		return false;

	return true;
}

void cPatcher::ErrorPrint(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	MessageBox(mWnd, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

/*-- EventWrite Method
*/
void cPatcher::EventWrite(LPCTSTR format, ...)
{
	LPVOID  msgBuf = NULL;

	if( mEndApp == true )
		return;

#if defined(_DEBUG)// || defined(_DEVSYS)
	DWORD   bufferLength;
	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) +1;
	msgBuf       = malloc( bufferLength );
	vsprintf( (char*)msgBuf, format, args );

	va_end( args );

	MessageBox(mWnd, (LPCTSTR)msgBuf, "Error", MB_ICONERROR);
	free( msgBuf );
#endif
}

/*-- EventWrite Method
*/
void cPatcher::ErrorMessageBox(LPCTSTR caption, LPCTSTR format, ...)
{
	LPVOID  msgBuf = NULL;

	if( mEndApp == true )
		return;

	DWORD   bufferLength;
	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) +1;
	msgBuf       = malloc( bufferLength );
	vsprintf( (char*)msgBuf, format, args );

	va_end( args );

	MessageBox( NULL, (LPCTSTR)msgBuf, caption, MB_OK | MB_ICONERROR );
	free( msgBuf );
}
