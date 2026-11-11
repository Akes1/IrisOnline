/*-- Include
*/
#include "stdafx.h"
#include "PatchMaker.h"
#include "zlib.h"
#include "ZipArchive.h"
#include <stdio.h>
#include <time.h>
#include <io.h>

/*-- Global data
*/
Patchmaker* g_patchmaker = NULL;

/*-- WorkerThreadStartingPoint Prototype
*/
DWORD WINAPI WorkerThreadStartingPoint(void* ptr)
{
	return g_patchmaker->WorkerThread( );
}

DWORD CALLBACK CopyProgressRoutine( LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
								    LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
								    DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	return g_patchmaker->Progress( TotalFileSize.LowPart, TotalBytesTransferred.LowPart );
}

/*-- Patchmaker Constructor
*/
Patchmaker::Patchmaker(void)
{
	g_patchmaker   = this;
 	m_wnd          = NULL;
 	m_windowName   = TEXT( "Patchmaker-IRIS ONLINE" );
	m_endApp       = false;
	m_VersionCount = 0;
	m_isUseBackupDirectory	    = true;
	m_isUseSourceList           = false;

	memset( m_currentDirectory,    0, sizeof( m_currentDirectory ) );
	memset( m_sourceDirectory,     0, sizeof( m_sourceDirectory  ) );
	memset( m_targetDirectory,     0, sizeof( m_targetDirectory  ) );
	memset( m_workerDirectory,	   0, sizeof( m_currentDirectory ) );
	memset( m_backupDirectory,	   0, sizeof( m_backupDirectory  ) );
	memset( m_sourceListDirectory, 0, sizeof( m_sourceListDirectory ) );

	m_workerThread = NULL;
}

/*-- ~Patchmaker Destructor
*/
Patchmaker::~Patchmaker(void)
{
	g_patchmaker = NULL;
	m_workerThread = NULL;

	ClearList();
}

/*-- EventWrite Method
*/
void Patchmaker::EventWrite(LPCTSTR format, ...)
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
		char filename[1024] = { 0, };
		sprintf_s( filename, "%s\\%s\\[%s]%s.log", m_currentDirectory, LOG_DIRECTORY_NAME, today, GetWindowName() );

		/// 폴더가 존재하는지 검사
		if( _access( filename, 0 ) != 0 )
		{
			/// 없다면 만들기
			if( MakePath( filename ) == false )
				ErrorPrint("Fail to make log directory" );	
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

/*-- GetFileTimeUTC Method
*/
bool Patchmaker::GetFileTimeUTC(char* filename, SYSTEMTIME* utc)
{
	HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( file != INVALID_HANDLE_VALUE ) 
	{
		FILETIME write;
		GetFileTime( file, NULL, NULL, &write );
		FileTimeToSystemTime( &write, utc );
		CloseHandle( file );
		return true;
	}
	return false;
}

/*-- SetFileTimeUTC Method
*/
bool Patchmaker::SetFileTimeUTC(char* filename, SYSTEMTIME* utc)
{
	HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( file != INVALID_HANDLE_VALUE ) 
	{
		FILETIME write;
		SystemTimeToFileTime( utc, &write );
		SetFileTime( file, NULL, NULL, &write );
		CloseHandle( file );
		return true;
	}
	return false;
}

/*-- FileTimeToUTC Method
*/
void Patchmaker::FileTimeToUTC(FILETIME* filetime)
{
	SYSTEMTIME utc;
	FileTimeToSystemTime( filetime, &utc );
	SystemTimeToFileTime( &utc, filetime );
}

/*-- FileTimeToDOS Method
*/
void Patchmaker::FileTimeToDOS(FILETIME* filetime)
{
	WORD fatDate, fatTime;
	FileTimeToDosDateTime( filetime, &fatDate, &fatTime );
	DosDateTimeToFileTime( fatDate, fatTime, filetime );
}

long Patchmaker::FileTimeCompareToList( char* fileName, int* error )
{
	HANDLE   file    = NULL;
	long     retcode = 0;

	FILETIME filetime1;
	FILETIME filetime2;

	char worker[1024] = {0,};
	sprintf_s( worker, "%s\\%s", m_workerDirectory, fileName );
	file = CreateFile( worker, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( file != INVALID_HANDLE_VALUE ) 
	{
		GetFileTime( file, NULL, NULL, &filetime1 );
		CloseHandle( file );
	}
	else
		return -1;

	errno_t err = _strlwr_s( fileName, sizeof( char[1024] ) );
	if ( err != 0 )
	{
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
		return -2;
	}

	/// 맵에서 검색
	std::hash_map<std::string, Patchfile*>::iterator findPos;
	findPos = m_SourcePatchList.find( fileName );
	if( findPos != m_SourcePatchList.end() )
	{
		Patchfile* patchfile = findPos->second;
		filetime2 = patchfile->writeTime;
	}
	else
	{
		/// 파일에서 못 찾았다면 실재로 소스 파일에 없는지 체크한후에 있으면 맵 리스트에 추가한다.
		char source[1024] = { 0, };
		sprintf_s( source, "%s\\%s", m_sourceDirectory, fileName );
		
		file = CreateFile( source, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if ( file != INVALID_HANDLE_VALUE ) 
		{
			GetFileTime( file, NULL, NULL, &filetime2 );
			CloseHandle( file );

			HANDLE search = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA filedata;

			search = FindFirstFile( source, &filedata );	
			if( search != INVALID_HANDLE_VALUE )
			{
				Patchfile* patchfile = new Patchfile;
				strcpy_s( patchfile->fileName, fileName );
				patchfile->sizeHigh  = filedata.nFileSizeHigh;
				patchfile->sizeLow   = filedata.nFileSizeLow;
				patchfile->writeTime = filedata.ftLastWriteTime;

				m_SourcePatchList.insert( Patchfile_Pair( fileName, patchfile ) );

				FindClose( search );
			}
			else
				return -2;
		}
		else
			return -2;
	}

	retcode  = CompareFileTime( &filetime1, &filetime2 );
	(*error) = 0;
	if ( retcode != 0 )
	{
		FileTimeToUTC( &filetime1 );
		FileTimeToUTC( &filetime2 );
		retcode = CompareFileTime( &filetime1, &filetime2 );
		(*error) = 1;
	}
	if ( retcode != 0 )
	{
		FileTimeToDOS( &filetime1 );
		FileTimeToDOS( &filetime2 );
		retcode = CompareFileTime( &filetime1, &filetime2 );
		(*error) = 2;
	}
	return retcode;
}


/*-- Pack Method
*/
bool Patchmaker::Pack(char* source, char* target, SYSTEMTIME* utc, bool bReverseWrite )
{
	HANDLE file    = CreateFile( source, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	bool   retcode = false;

	if ( file != INVALID_HANDLE_VALUE )
	{
		FILETIME   create;
		FILETIME   access;
		FILETIME   write;

		DWORD      sizeHigh;
		DWORD      sizeLow;

		GetFileTime( file, &create, &access, &write );
		if ( utc != NULL )
			FileTimeToSystemTime( &write, utc );

		sizeLow = GetFileSize( file, &sizeHigh );

		if( MakePath( target ) == false )
		{
			PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_MAKE_PATH, (LPARAM)0 );
			return false;
		}

		gzFile gz = gzopen( target, "wb" );
		if ( gz != NULL )
		{
			char  buf[16384];			// CHUNK 16KByte
			char  reverse_buf[16384];
			DWORD len = sizeof(buf);
			DWORD bytesRead;

			while ( true )
			{
				ReadFile( file, buf, len, &bytesRead, NULL );
				if ( bytesRead > 0 )
				{
					if( bReverseWrite == true )
					{
						char* pbuf = buf + bytesRead - 1;
						char* prev = reverse_buf;
						for( DWORD cnt = 0; cnt < bytesRead; ++cnt )
						{
							*prev = *pbuf;
							pbuf--;
							prev++;
						}
						gzwrite( gz, reverse_buf, bytesRead );
					}
					else 
						gzwrite( gz, buf, bytesRead );
				}
				else
					break;
			}
			gzclose( gz );
			gz = NULL;

			retcode = true;
		}

		CloseHandle( file );
		file = NULL;
	}

	return retcode;
}

/*-- Unpack Method
*/
bool Patchmaker::Unpack(char* source, char* target, SYSTEMTIME* utc)
{
	gzFile gz = gzopen( source, "rb" );
	bool   retcode = false;

	if ( gz != NULL )
	{
		if( MakePath( target ) == false )
		{
			PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_MAKE_PATH, (LPARAM)0 );
			return false;
		}

		HANDLE file = CreateFile( target, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( file != INVALID_HANDLE_VALUE )
		{
			char  buf[16384];			// CHUNK 16KByte
			DWORD len = sizeof(buf);

			DWORD bytesRead;
			DWORD bytesWritten;
			while ( true )
			{
				bytesRead = gzread( gz, buf, len );
				if ( bytesRead > 0 )
					WriteFile( file, buf, bytesRead, &bytesWritten, NULL );
				else
					break;
			}

			if ( utc != NULL )
			{
				FILETIME write;
				SystemTimeToFileTime( utc, &write );
				SetFileTime( file, NULL, NULL, &write );
			}

			CloseHandle( file );
			file = NULL;

			retcode = true;
		}
		gzclose( gz );
		gz = NULL;
	}

	return retcode;
}

/*-- Run Method
*/
//int Patchmaker::Run(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
int Patchmaker::Run( HWND wnd )
{
 	// 현재 폴더 저장.
 	GetCurrentDirectory( sizeof(m_currentDirectory), m_currentDirectory );

	m_wnd = wnd;

	if( StartPatch() == false )
		return -1;

	return 0;
}

/*-- SetStatus Method
*/
BOOL Patchmaker::SetStatus(HWND wnd, char* msg)
{
	if( m_endApp == false )
	{
  		HWND dlgItem = GetDlgItem( wnd, IDC_EDIT_STATUS );
  		BOOL retcode = SetWindowText( dlgItem, msg );
  		UpdateWindow( dlgItem );
 		return retcode;
	}
	return FALSE;
}
 
// /*-- Progress Method
// */
DWORD Patchmaker::Progress(DWORD length, DWORD offset)
{
 	if ( m_wnd != NULL && m_endApp == false )
 	{
 		int percent = (int)((float)offset / (float)length * 100.f);
 		SendMessage( GetDlgItem( m_wnd, IDC_PROGRESS ), PBM_SETPOS, percent, 0 );
 	}
	return PROGRESS_CONTINUE;
}

/*-- Destroy Method
*/
void Patchmaker::Destroy()
{
	// 안전종료.
	m_endApp = true;

	Sleep(1);

	// m_workerThread 핸들를 해제.
	if ( m_workerThread != NULL )
	{
		// WorkerThread가 종료될때까지 대기.
		if ( WaitForSingleObject( m_workerThread, 120000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( m_workerThread, &exitCode );
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( m_workerThread, 0 );
			}
		}
		CloseHandle( m_workerThread );
		m_workerThread = NULL;
	}

	return ;
}

/*-- PatchMsg Method
*/
LRESULT Patchmaker::PatchMsg(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case ePM_COMPLETE:
		MessageBox( wnd, "Success", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_XCOPY:
		MessageBox( wnd, "Error Xcopy", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR:
		MessageBox( wnd, "Error", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_STRLOWER:
		MessageBox( wnd, "Error STRLOWER", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_MAKE_PATH:
		MessageBox( wnd, "Error Make Path", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_NO_WORKER:
		MessageBox( wnd, "Error Worker Path. Check the worker path.", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_NO_TARGET:
		MessageBox( wnd, "Error Target Path. Check the target path.", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_NO_SOURCE:
		MessageBox( wnd, "Error Source Path. Check the source path.", GetWindowName( ), MB_OK );
		break;
	case ePM_ERROR_NO_BACKUP:
		MessageBox( wnd, "Error Backup Path. Check the backup path.", GetWindowName( ), MB_OK );
		break;
	default:
		MessageBox( wnd, "Unkonwn Error", GetWindowName( ), MB_OK );
		break;
	}
	//PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	return 1L;
}

/*-- Xcopy Method
*/
int Patchmaker::Xcopy( char* sub )
{
	/// 응용 프로그램 취소 검사.
	if( m_endApp == true )
		return 0;

	int             retcode = 0;	 // 복사 성공.
	HANDLE          search  = NULL;
	char            where[ 1024 ] = {0,};

	char            filename[ 1024 ] = {0,};
	WIN32_FIND_DATA fileData;

	/// script폴더 내의 .pack을 다시 만들어야 하는지 체크
	bool bNeedPackRemake[ePACK_SCRIPT_FOLDER_MAX_COUNT] = { false, };

	// 하위 디렉토리 적용여부
	if ( sub )
		sprintf_s( where, "%s\\%s", m_workerDirectory, sub );
	else
		sprintf_s( where, "%s", m_workerDirectory );

	sprintf_s( filename, "%s\\*.*", where );

	search = FindFirstFile( filename, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( ( strcmp( fileData.cFileName, "."  ) != 0 ) && ( strcmp( fileData.cFileName, ".." ) != 0) )
				{
					char directory[ 1024 ] = {0,};
					bool skipMakeSubDirectory = false;

					if ( sub )
					{
						sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
 						
#ifdef  _CLIENT_MODE_
						for( int idx = 0; idx < eSKIP_FOLDER_MAX_COUNT; ++idx )
						{
							if( _stricmp( sub, SKIP_MAKE_SUB_DIRECTORY[idx] ) == 0 )
								skipMakeSubDirectory = true;
						}
#endif

					}
					else
					{
						sprintf_s( directory, "%s", fileData.cFileName );
					}
					
					if( m_isUseSourceList == true )
					{
						errno_t err = _strlwr_s( directory );
						if ( err != 0 )
						{
							PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
							return -8;
						}

						std::hash_map<std::string, Packfile*>::iterator findPos;
						findPos = m_SourcePackList.find( directory );
						if( findPos == m_SourcePackList.end() )
						{	
							Packfile* pack = new Packfile;
							strcpy_s( pack->directory, directory );
							
							errno_t err = _strlwr_s( pack->directory );
							if ( err != 0 )
							{
								PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
								return -8;
							}

							m_SourcePackList.insert( Packfile_Pair( pack->directory, pack ) );
						}	
					}
					
					// 재귀호출.
					retcode = Xcopy( directory );
				}
			}
			else if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
			{
				char worker[ 1024 ] = { 0, };
				char source[ 1024 ] = { 0, };
				char target[ 1024 ] = { 0, };
				char directory[ 1024 ] = { 0, };

				if ( sub )
				{
					sprintf_s( worker, "%s\\%s\\%s", m_workerDirectory, sub, fileData.cFileName );
					sprintf_s( source, "%s\\%s\\%s", m_sourceDirectory, sub, fileData.cFileName );
					sprintf_s( target, "%s\\%s\\%s.z", m_targetDirectory, sub, fileData.cFileName );
					sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
				}
				else
				{
					sprintf_s( worker, "%s\\%s", m_workerDirectory, fileData.cFileName );
					sprintf_s( source, "%s\\%s", m_sourceDirectory, fileData.cFileName );
					sprintf_s( target, "%s\\%s.z", m_targetDirectory, fileData.cFileName );
					sprintf_s( directory, "%s", fileData.cFileName );
				}

				int  error    = 0;
				long retvalue = -1;
				bool skipPack = false;

				/// 소스 리스트를 사용할건가에 따라 함수가 다름
				retvalue = FileTimeCompareToList( directory, &error );

				if ( retvalue != 0 )
				{
					EventWrite( "CompareFileTime(%d,%d) - Xcopy from(%s) to(%s)\n", retvalue, error, worker, source );

#ifdef _CLIENT_MODE_
					/// 팩으로 만들 폴더의 파일이 바뀐 경우 pack 파일을 갱신시켜야 함.
					if( sub != NULL )
					{
						for(int index=0; index < ePACK_SCRIPT_FOLDER_MAX_COUNT; ++index)
						{
							if( _strnicmp( sub, PACK_DIRECTORY[index], strlen( PACK_DIRECTORY[index] ) ) == 0 )
							{
								bNeedPackRemake[index] = true;
								skipPack = true;
							}
						}
					}
#endif
					bool retPack = true;
					SYSTEMTIME utc;

					/// script 파일안에 pack 파일을 생성 및 .z로 압축 할때에는 체크가 모두 끝난뒤에 한번에 한다.
					if( skipPack == false )
					{
						errno_t err = _strlwr_s(target);
						if ( err != 0 )
						{
							PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
							return -8;
						}
						retPack = Pack( worker, target, &utc );
					}
					
					if( retPack == true )
					{
						bool isReadOnlyFile = false;
						DWORD fileAttibute  = 0;
						DWORD error_code =0;
						
						/// 덮여 씌워질 파일 백업
						if( _access( source, 0 ) == 0 )
						{
							fileAttibute = GetFileAttributes( source );
							
							if( fileAttibute & FILE_ATTRIBUTE_READONLY )
							{
								if( SetFileAttributes( source, FILE_ATTRIBUTE_ARCHIVE ) == 0 )
									ErrorPrint( "XCopy:Fail to change file attribute" )	;
								isReadOnlyFile = true;
							}

							/// target 폴더 안에 새로운 폴더를 만들어서 기존 파일들 백업시키기. 패치를 롤백할 때 쓰임 [2/9/2009 Jo_2]
							char changedFiles[ 1024 ] = { 0, };

							if( sub )
								sprintf_s( changedFiles, "%s\\%s\\%s", m_backupDirectory, sub, fileData.cFileName );
							else
								sprintf_s( changedFiles, "%s\\%s", m_backupDirectory, fileData.cFileName );

							if( MakePath( changedFiles ) == true )
							{
								errno_t err = _strlwr_s( changedFiles );
								if ( err != 0 )
								{
									PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
									return -8;
								}

								// 소스에서 파일을 읽어와서 백업폴더내의 바뀐 파일들 폴더 안으로 파일 옴기기
								if ( MoveFile( source, changedFiles ) == FALSE )
									ErrorPrint( "Fail to backup Original file" );
							}
							else
							{
								ErrorPrint( "Fail to Make backup file Path" );
							}
						}
						else if( MakePath( source ) == false )
						{
							retcode = -5; // 파일 복사 경로 생성 실패.
							ErrorPrint( "XCopy:Fail to create source file path" )	;
						}
	
						// 소스에 복사. [12/11/2008 Jo]
						if ( CopyFileEx( worker, source, CopyProgressRoutine, NULL, NULL, 0 ) == TRUE ) 
						{
							GetFileTimeUTC( worker, &utc );
							SetFileTimeUTC( source, &utc );
							
 							if( isReadOnlyFile == true )
 							{
 								SetFileAttributes( worker, FILE_ATTRIBUTE_READONLY );
  							}

							/// 맵 리스트 이용시에는 맵 리스트 정보 갱신
							if( m_isUseSourceList == true )
							{
								std::hash_map<std::string, Patchfile*>::iterator findPos;
								findPos = m_SourcePatchList.find( directory );
								if( findPos != m_SourcePatchList.end() )
								{	
									Patchfile* patchfile = findPos->second;
									WIN32_FIND_DATA filedata;
									HANDLE search2 = INVALID_HANDLE_VALUE;
									search2 = FindFirstFile( source, &filedata );	
									if( search2 != INVALID_HANDLE_VALUE )
									{									
										patchfile->sizeHigh  = filedata.nFileSizeHigh;
										patchfile->sizeLow   = filedata.nFileSizeLow;
										patchfile->writeTime = filedata.ftLastWriteTime;
										FindClose( search2 );
									}
									else
									{
										ErrorPrint("Fail to Modify patchfile MapList");
										return -7;
									}
								}
								else
								{
									Patchfile* patchfile = new Patchfile; ///새로 추가된 파일이라면
									WIN32_FIND_DATA filedata;
									HANDLE search2 = INVALID_HANDLE_VALUE;
									search2 = FindFirstFile( source, &filedata );	
									if( search2 != INVALID_HANDLE_VALUE )
									{
										strcpy_s( patchfile->fileName, directory );
										patchfile->sizeHigh  = filedata.nFileSizeHigh;
										patchfile->sizeLow   = filedata.nFileSizeLow;
										patchfile->writeTime = filedata.ftLastWriteTime;

										m_SourcePatchList.insert( Patchfile_Pair( source, patchfile ) );
										FindClose( search2 );
									}
									else
									{
										ErrorPrint("Fail to Modify patchfile MapList");
										return -7;
									}
								}
								
							}
						}
						else
						{
							retcode = -4; // 파일 복사 실패.
							DeleteFile( source );  // 복사중이던 파일 삭제
							ErrorPrint( "XCopy:Fail to copy file" )	;
						}
					}
					else
						retcode = -3; // 압축 복사 실패.
				}
			}
		} while ( FindNextFile( search, &fileData ) && retcode == 0 );

		FindClose( search );
		search = NULL;


#ifdef _CLIENT_MODE_
		/// script 폴더안의 pack을 다시 생성해야 할 필요가 있는지 체크 한뒤에 압축하여 target/script 폴더에 저장.
		for(int index=0; index < ePACK_SCRIPT_FOLDER_MAX_COUNT; ++index)
		{
			if( bNeedPackRemake[index] == true )
			{
				CZipArchive zip;
				char fileNameDummy[1024]   = {0,} ;
				char packfilename[1024]    = {0,} ;
				char backupFilename[1024]  = {0,} ;
				char *packFolderName       = NULL ;
				char *nextToken	    	   = NULL ;

				sprintf_s( packfilename, "%s\\%s.pack", m_sourceDirectory, PACK_DIRECTORY[index] );

				
				/// 기존 pack 파일을 백업 폴더 생성하여 추가 하기  [2/18/2009 Jo_2]
				if( _access( packfilename, 0) == 0 )
				{
					sprintf_s( backupFilename, "%s\\%s.pack", m_backupDirectory, PACK_DIRECTORY[index] );

					if( MakePath( backupFilename ) == false )
					{
						PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_MAKE_PATH, (LPARAM)0 );
						return -1;
					}

					errno_t err = _strlwr_s( backupFilename );
					if ( err != 0 )
					{
						PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
						return -8;
					}

					if( MoveFile( packfilename, backupFilename ) == FALSE )
						ErrorPrint( "Fail to old script pack file backup" );
				}
				
				strcpy_s( fileNameDummy, PACK_DIRECTORY[index] );
				
				packFolderName = strtok_s( fileNameDummy, "/\\", &nextToken );
				packFolderName = strtok_s( NULL, "/\\", &nextToken );
				
				/// 새로 압축하기
				zip.Open( packfilename, CZipArchive::zipCreate );  
				zip.SetPassword(ZIP_PASSWORD);
							
				m_CollectFileInfo.clear();
				if( CollectFileInfo( m_sourceDirectory, PACK_DIRECTORY[index] ) == 0 )
				{
					CollectFile_Info::iterator pos = m_CollectFileInfo.begin();
					CollectFile_Info::iterator end = m_CollectFileInfo.end();
					for( ; pos != end; ++pos )
					{
						zip.AddNewFile( pos->fullPath.c_str(), pos->filename.c_str(), Z_NO_COMPRESSION );
					}
				}

				zip.Close();

				SYSTEMTIME utc;
				char	   target[1024] = {0,};
				
				sprintf_s( target, "%s\\%s.pack.z", m_targetDirectory, PACK_DIRECTORY[index] );
				
				errno_t err2 = _strlwr_s(target);
				if ( err2 != 0 )
				{
					PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
					return -8;
				}
				
				if( Pack( packfilename, target, &utc ) == true )
				{
					/// 복사된 파일 정보를 m_patchMapList에 추가하기
					char scriptName[1024] = {0,};
					sprintf_s( scriptName,"%s.pack", PACK_DIRECTORY[index] );
					std::hash_map<std::string, Patchfile*>::iterator findPos;
					findPos = m_SourcePatchList.find( scriptName );
					if( findPos != m_SourcePatchList.end() )
					{	
						Patchfile* patchfile = findPos->second;
						WIN32_FIND_DATA filedata2;
						HANDLE search2 = INVALID_HANDLE_VALUE;
						search2 = FindFirstFile( packfilename, &filedata2 );	
						if( search2 != INVALID_HANDLE_VALUE )
						{									
							patchfile->sizeHigh  = filedata2.nFileSizeHigh;
							patchfile->sizeLow   = filedata2.nFileSizeLow;
							patchfile->writeTime = filedata2.ftLastWriteTime;
							FindClose( search2 );
						}
						else
						{
							ErrorPrint("Fail to Modify patchfile MapList");
							return -7;
						}
					}
					else
					{
						Patchfile* patchfile = new Patchfile; ///새로 추가된 파일이라면
						WIN32_FIND_DATA filedata2;
						HANDLE search2 = INVALID_HANDLE_VALUE;
						search2 = FindFirstFile( packfilename, &filedata2 );	
						if( search2 != INVALID_HANDLE_VALUE )
						{
							strcpy_s( patchfile->fileName, scriptName );
							patchfile->sizeHigh  = filedata2.nFileSizeHigh;
							patchfile->sizeLow   = filedata2.nFileSizeLow;
							patchfile->writeTime = filedata2.ftLastWriteTime;

							m_SourcePatchList.insert( Patchfile_Pair( scriptName, patchfile ) );
							FindClose( search2 );
						}
						else
						{
							ErrorPrint("Fail to Modify patchfile MapList");
							return -7;
						}
					}
				}	
				else
					retcode = -3; // 압축 복사 실패.		
			}
		}
#endif

		return retcode;
	}
	return -1; // 복사실패
}

bool Patchmaker::StartPatch( )
{
	DWORD threadId;
	m_workerThread = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	return (m_workerThread != NULL) ? true : false;
}

/*-- WorkerThread Method
*/
DWORD Patchmaker::WorkerThread( )
{
	bool success = false;

	/// 응용 프로그램 취소 검사.
	if( m_endApp == true )
		return 0;

	/// 백업 경로에 오늘 날짜 폴더 만들기
	CreateTodayBackupDirectory();

	if( m_isUseSourceList == false )
	{
		SetStatus( m_wnd, "Collect Source List..." );
		ClearList();

		if( CollectListInfo( m_sourceDirectory, NULL ) != 0 )
		{
			PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_XCOPY, (LPARAM)0 ); ///에러 메시지 수정
			SetStatus( m_wnd, "Error Source List..." );
			return 0;
		}
	}

	/// 응용 프로그램 취소 검사.
	if( m_endApp == true )
		return 0;

	SetStatus( m_wnd, "Start Xcopy..." );

	if ( Xcopy( NULL ) != 0 )
	{
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_XCOPY, (LPARAM)0 );
		SetStatus( m_wnd, "Error Xcopy..." );
		return 0;
	}

	SetStatus( m_wnd, "End Xcopy..." );

		
	SetStatus( m_wnd, "Start Make List..." );

	if( MakePatchList() != 0 )
	{
		ErrorPrint("Fail to Make List");
		return 0;
	}
	else if( m_isUseSourceList == true )
	{
		if( MakeSourceList() != 0 )
		{
			ErrorPrint("Fail to Make List");
			return 0;
		}
		else 
			success = true;
	}
	else
		success = true;

	SetStatus( m_wnd, "End Make..." );

	if ( success )
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_COMPLETE, (LPARAM)0 );
	else
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR, (LPARAM)0 );

	return 0;
}

void Patchmaker::ErrorPrint(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	MessageBox(m_wnd, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

bool Patchmaker::SetDirectory( char* worker, char* source, char* target, char* backup, char* sourceList )
{
	/// 경로 체크
	if( _access( worker, 0 ) == 0 )
		strcpy_s( m_workerDirectory, worker );
 	else
	{
 		MessageBox( m_wnd, "Error Worker Path. Check the worker path.", GetWindowName( ), MB_OK | MB_TASKMODAL );
		return false;
	}
	
	if( _access( source, 0 ) == 0 )
		strcpy_s( m_sourceDirectory, source );
	else
	{
		MessageBox( m_wnd, "Error Source Path. Check the source path.", GetWindowName( ), MB_OK | MB_TASKMODAL );
		return false;
	}

	if( _access( target, 0 ) == 0 )
		strcpy_s( m_targetDirectory, target );
	else
	{
		MessageBox( m_wnd, "Error Target Path. Check the target path.", GetWindowName( ), MB_OK | MB_TASKMODAL );
		return false;
	}

	if( m_isUseBackupDirectory == true )
	{
		if( _access( backup, 0 ) == 0 )
			strcpy_s( m_backupDirectory, backup );
		else
		{
			MessageBox( m_wnd, "Error Backup Path. Check the backup path.", GetWindowName( ), MB_OK | MB_TASKMODAL );
			return false;
		}
	}

	if( _access( sourceList, 0 ) == 0 )
	{
		strcpy_s( m_sourceListDirectory, sourceList );
		m_isUseSourceList = true;
	}
	else
		GetCurrentDirectory( sizeof(m_sourceListDirectory), m_sourceListDirectory );

	return true;
}

bool Patchmaker::MakePath( char* path )
{
	char drive    [ _MAX_DRIVE ]	 = {0,};
	char directory[ _MAX_DIR   ]	 = {0,};
	char makeDir  [ MAX_PATH   ]	 = {0,};
	char backupDirectory[ MAX_PATH ] = {0,};
	
	char  seps[] = "/\\";
	char* token  = NULL;
	char* nextToken = NULL;

	strcpy_s( backupDirectory, path );

	errno_t err = _strlwr_s( backupDirectory );
	if ( err != 0 )
	{
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
		return false;
	}

	_splitpath_s( backupDirectory, drive, _MAX_DRIVE, directory, _MAX_DIR, NULL, 0, NULL, 0 );

	strcat_s( makeDir, drive );
	strcat_s( makeDir, "/" );

	token = strtok_s( directory, seps, &nextToken );
	while ( token != NULL )
	{
		strcat_s( makeDir, token );
		strcat_s( makeDir, "/" );

		if ( GetFileAttributes( makeDir ) == 0xFFFFFFFF )
		{
			if ( CreateDirectory( makeDir, NULL ) == FALSE )
			{
				ErrorPrint(" Fail to create backup directory" );
				return false;
			}
		}
		token = strtok_s( NULL, seps, &nextToken );
	}

	return true;
}

int Patchmaker::CollectListInfo( char* root, char* sub )
{
	HANDLE          search = NULL;
	char            where[ 1024 ] = {0,};

	char            filename[ 1024 ] = {0,};
	WIN32_FIND_DATA fileData;

	// 하위 디렉토리 적용여부
	if ( sub )
		sprintf_s( where, "%s\\%s", root, sub );
	else
		sprintf_s( where, "%s", root );

	sprintf_s( filename, "%s\\*.*", where );

	search = FindFirstFile( filename, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( (strcmp( fileData.cFileName, "."  ) != 0) && (strcmp( fileData.cFileName, ".." ) != 0) )
				{
					char directory[ 1024 ] = {0,};

					if ( sub )
						sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
					else
						sprintf_s( directory, "%s", fileData.cFileName );

					Packfile* pack = new Packfile;
					
					strcpy_s( pack->directory, directory );
					
					errno_t err = _strlwr_s( pack->directory, sizeof( pack->directory ) );
					if ( err != 0 )
					{
						PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
						return -2;
					}
					
					m_SourcePackList.insert( Packfile_Pair( pack->directory, pack ) );

					CollectListInfo( root, directory );	// 재귀호출.
					
				}
			}
			else if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
			{
				char fullpath[ 1024 ] = {0,};

				if ( sub )
					sprintf_s( fullpath, "%s\\%s\\%s", root, sub, fileData.cFileName );
				else
					sprintf_s( fullpath, "%s\\%s", root, fileData.cFileName );

				
					Patchfile* file = new Patchfile; 

					if ( sub )
						sprintf_s( file->fileName, "%s\\%s", sub, fileData.cFileName );
					else
						sprintf_s( file->fileName, "%s", fileData.cFileName );

					file->sizeHigh  = fileData.nFileSizeHigh;
					file->sizeLow   = fileData.nFileSizeLow;
					file->writeTime = fileData.ftLastWriteTime;

					FileTimeToDOS( &file->writeTime );

					errno_t err = _strlwr_s( file->fileName, sizeof( file->fileName ) );
					if ( err != 0 )
					{
						PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
						return -2; // 소문자 변환 실패
					}

					m_SourcePatchList.insert( Patchfile_Pair( file->fileName, file ) );
			}
		} while ( FindNextFile( search, &fileData ) );

		FindClose( search );
		return 0;
	}
	return -1; // 검색 실패.

}


bool Patchmaker::CreateTodayBackupDirectory()
{

	//백업 경로를 옵션에서 사용하지 않는다고 체크 하면 타겟 폴더에 생성한다.
	if( m_isUseBackupDirectory == false )
		strcpy_s( m_backupDirectory, m_targetDirectory );

	char todayDirectory[ 1024 ] = { 0, };
	char todaySubDirectory[ 1024 ] = { 0, };
	int  todayCounter = 1;

	time_t t;
	tm    pt;
	errno_t err;

	time(&t);
	err = localtime_s( &pt, &t );

	sprintf_s( todaySubDirectory, "%04d-%02d-%02d_%d",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, todayCounter );
	sprintf_s( todayDirectory, "%s\\%s", m_backupDirectory, todaySubDirectory );

	while( _access( todayDirectory, 0 ) == 0 )
	{
		++todayCounter;
		sprintf_s( todaySubDirectory, "%04d-%02d-%02d_%d\0",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, todayCounter );
		sprintf_s( todayDirectory, "%s\\%s\0", m_backupDirectory, todaySubDirectory );
	}

	if ( CreateDirectory( todayDirectory, NULL ) == FALSE )
	{
		if ( GetLastError( ) != ERROR_ALREADY_EXISTS )
		{
			ErrorPrint( "today backup directory create fail" );
			return false;
		}
	}
	
	strcpy_s( m_backupDirectory, todayDirectory );

	if( m_isUseBackupDirectory == false )
	{
		strcpy_s( m_targetDirectory, m_backupDirectory );
		strcat_s( m_backupDirectory, "\\Backup Folder" );
	}

	return true;
}

bool Patchmaker::DeleteSubFiles( char* root, char* sub )
{
	HANDLE          search = NULL;
	char            where[ 1024 ] = { 0, };

	char            filename[ 1024 ] = { 0, };
	WIN32_FIND_DATA fileData;

	// 하위 디렉토리 적용여부
	if ( sub )
		sprintf_s( where, "%s\\%s", root, sub );
	else
		sprintf_s( where, "%s", root );

	sprintf_s( filename, "%s\\*.*", where );

	search = FindFirstFile( filename, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( (strcmp( fileData.cFileName, "."  ) != 0) && (strcmp( fileData.cFileName, ".." ) != 0) )
				{
					char directory[ 1024 ] = { 0, };

					if ( sub )
						sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
					else
						sprintf_s( directory, "%s", fileData.cFileName );

					DeleteSubFiles( root, directory );
				}
			}
			else if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
			{
				char fullpath[ 1024 ] = { 0, };

				if ( sub )
					sprintf_s( fullpath, "%s\\%s\\%s", root, sub, fileData.cFileName );
				else
					sprintf_s( fullpath, "%s\\%s", root, fileData.cFileName );

				if( fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
				{
					if( SetFileAttributes( fullpath, FILE_ATTRIBUTE_ARCHIVE ) == 0 )
						return false;
				}

				///파일 삭제
				if( DeleteFile( fullpath ) == FALSE )
					return false;
			}
		} while ( FindNextFile( search, &fileData ) );

		FindClose( search );

		///폴더 삭제
		if( _stricmp( root, where) != 0 )
			RemoveDirectory( where );

		return true;
	}
	return false; // 검사 실패.
}

bool Patchmaker::ReadSourceList(char* buffer)
{
	// 목록 초기화
	ClearList();

	// 메모리 토큰.
	char  seps[] = "\r\n/;";
	char* token		= NULL;
	char* nextToken = NULL;
	ULONG packLen	   = 0;
	ULONG patchfileLen = 0;
	char  versionCount[32] = { 0, };

	// Header
	token = strtok_s( (char*)buffer, seps, &nextToken );
	if ( token == NULL )
		return false;

	if ( strcmp( token, "patchlist" ) )
		return false;

	token = strtok_s( NULL, seps, &nextToken  );
	if ( token == NULL )
		return false;

	if ( strcmp( token, "ver.110" ) )
		return false;
	
	token = strtok_s( NULL, seps, &nextToken  );
	if ( token == NULL )
		return false;

	packLen = ::atoi( token );

	token = strtok_s( NULL, seps, &nextToken  );
	if ( token == NULL )
		return false;

	patchfileLen = ::atoi( token );


	Packfile* packfile = NULL;
	for ( ULONG i = 0; i < packLen; i++)
	{
		packfile = new Packfile;
		// 파일이름
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		strcpy_s( packfile->directory, token );

		m_SourcePackList.insert( Packfile_Pair( packfile->directory, packfile ) );
	}

	//for( unsigned long i = 0; i< patchfileLen && mEndApp == false; i++ )
	Patchfile* patchfile = NULL;
	for( unsigned long i = 0; i< patchfileLen; i++ )
	{
		patchfile = new Patchfile;

		// 파일이름
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		strcpy_s( patchfile->fileName, token );
		
		// 파일크기-Low
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		patchfile->sizeLow = (DWORD)atof( token );

		// 파일크기-High
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		patchfile->sizeHigh = (DWORD)atof( token );

		// 파일날짜-Low
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		patchfile->writeTime.dwLowDateTime = (DWORD)atof( token );

		// 파일날짜-High
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return false;
		patchfile->writeTime.dwHighDateTime = (DWORD)atof( token );

		errno_t err = _strlwr_s(patchfile->fileName);
		if ( err != 0 )
		{
			PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
			SafeDelete( patchfile );
			continue;
		}

		m_SourcePatchList.insert( Patchfile_Pair(patchfile->fileName, patchfile) );
	}

	return true;
}

void Patchmaker::ClearList()
{
	std::hash_map<std::string, Packfile*>::iterator pos1 =  m_SourcePackList.begin();
	std::hash_map<std::string, Packfile*>::iterator end1 =  m_SourcePackList.end();
	for( ; pos1 != end1; ++pos1 )
	{
		Packfile*  packfile = pos1->second;
		SafeDelete( packfile );
	}
	m_SourcePackList.clear();

	std::hash_map<std::string, Patchfile*>::iterator pos2 =  m_SourcePatchList.begin();
	std::hash_map<std::string, Patchfile*>::iterator end2 =  m_SourcePatchList.end();
	for( ; pos2 != end2; ++pos2 )
	{
		Patchfile*  patchfile = pos2->second;
		SafeDelete( patchfile );
	}
	m_SourcePatchList.clear();

	m_PatchFolderList.clear();
	m_PatchFileList.clear();
}

int Patchmaker::MakePatchList()
{
	/// 패치 리스트에 기록 될 데이터 추출
	std::hash_map<std::string, Packfile*>::iterator pos1 =  m_SourcePackList.begin();
	std::hash_map<std::string, Packfile*>::iterator end1 =  m_SourcePackList.end();

	Packfile* packfile = NULL;
	for( ; pos1 != end1; ++pos1 )
	{
		packfile = pos1->second;

#ifdef _CLIENT_MODE_
		bool skipWriteList = false;

		for(int index=0; index < ePACK_SCRIPT_FOLDER_MAX_COUNT; ++index)
		{
			char sub_directory[MAX_PATH];
			sprintf_s(sub_directory, "%s\\", PACK_DIRECTORY[index] );

			if( _strnicmp( packfile->directory, sub_directory, strlen( sub_directory ) ) == 0 )
				skipWriteList = true;
		}

		if( skipWriteList == true )
			continue;
#endif
		m_PatchFolderList.insert( packfile );
	}

	std::hash_map<std::string, Patchfile*>::iterator pos2 =  m_SourcePatchList.begin();
	std::hash_map<std::string, Patchfile*>::iterator end2 =  m_SourcePatchList.end();
	Patchfile* patchfile = NULL;

	for( ; pos2 != end2; ++pos2 )
	{
		patchfile = pos2->second;

#ifdef _CLIENT_MODE_
		bool skipWriteList = false;

		for(int index=0; index < ePACK_SCRIPT_FOLDER_MAX_COUNT; ++index)
		{
			char sub_directory[MAX_PATH];
			sprintf_s( sub_directory, "%s\\", PACK_DIRECTORY[index] );

			if( _strnicmp( patchfile->fileName, sub_directory, strlen( sub_directory ) ) == 0 )
				skipWriteList = true;
		}

		if( skipWriteList == true )
			continue;
#endif
		m_PatchFileList.insert( patchfile );
	}


	/// 패치 리스트 작성하기.
	char filename[1024]     = { 0, };
	char packfilename[1024] = { 0, };
	char sourceFileLIst[1024] = { 0, };

	sprintf_s( filename, "%s\\patchlist.txt", m_targetDirectory );
	sprintf_s( packfilename, "%s.z", filename );

	FILE* stream;
	errno_t  err;
	err = fopen_s( &stream, filename, "wt" );
	if ( stream != NULL )
	{
		char buffer[ 1024 ] = { 0, };

		unsigned int packListSize  = (unsigned int)m_PatchFolderList.size();
		unsigned int patchListSize = (unsigned int)m_PatchFileList.size();

		// header
 		sprintf_s( buffer, "patchlist/ver.110/%d/%d\n", packListSize, patchListSize );
		fputs( buffer, stream );

		std::set<Packfile*>::iterator pos1 =  m_PatchFolderList.begin();
		std::set<Packfile*>::iterator end1 =  m_PatchFolderList.end();
		DWORD cnt = 0;

		Packfile* packfile = NULL;
		for( ; pos1 != end1; ++pos1 )
		{
			Progress( packListSize + patchListSize , ++cnt );

			packfile = *pos1;

			errno_t err = _strlwr_s(packfile->directory);
			if ( err != 0 )
			{
				PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
				return -1;
			}

			sprintf_s( buffer ,"%s\n" , packfile->directory );
			fputs( buffer, stream );
		}

		std::set<Patchfile*>::iterator pos2 =  m_PatchFileList.begin();
		std::set<Patchfile*>::iterator end2 =  m_PatchFileList.end();
		Patchfile* patchfile = NULL;

		for( ; pos2 != end2; ++pos2 )
		{
			Progress( packListSize + patchListSize , ++cnt );

			patchfile = *pos2;

			errno_t err = _strlwr_s(patchfile->fileName);
			if ( err != 0 )
			{
				PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
				return -1;
			}

			sprintf_s( buffer
				,"%s;%u;%u;%u;%u\n"
				,patchfile->fileName
				,patchfile->sizeLow
				,patchfile->sizeHigh
				,patchfile->writeTime.dwLowDateTime
				,patchfile->writeTime.dwHighDateTime );
			fputs( buffer, stream );

		}

		fclose( stream );
		stream = NULL;
	}
	else
		return -2;

	/// patchlist.txt 파일을 압축한 뒤 삭제 [12/11/2008 Jo]
	SYSTEMTIME utc;
	if ( Pack( filename, packfilename, &utc, false ) == false )
	{
		SetStatus( m_wnd, "Fail To Pack Patchlist.txt.z" );
		return -3;
	}
	
	if( DeleteFile( filename ) == false )
	{
		SetStatus( m_wnd, "Fail To Delete Patchlist.txt" );
		return -5;
	}

	return 0;
}

int Patchmaker::MakeSourceList()
{
	char filename[1024]     = { 0, };
	char packfilename[1024] = { 0, };
	char sourceFileLIst[1024] = { 0, };

	sprintf_s( sourceFileLIst, "%s\\SourceList.txt", m_sourceListDirectory );
	
	FILE* stream;
	errno_t  err;
	err = fopen_s( &stream, sourceFileLIst, "wt" );
	if ( stream != NULL )
	{
		char buffer[ 1024 ] = { 0, };

		unsigned int packListSize  = (unsigned int)m_SourcePackList.size();
		unsigned int patchListSize = (unsigned int)m_SourcePatchList.size();

		// header
		sprintf_s( buffer, "patchlist/ver.110/%d/%d\n", packListSize, patchListSize );
		fputs( buffer, stream );

		std::hash_map<std::string, Packfile*>::iterator pos1 =  m_SourcePackList.begin();
		std::hash_map<std::string, Packfile*>::iterator end1 =  m_SourcePackList.end();
		DWORD cnt = 0;

		Packfile* packfile = NULL;
		for( ; pos1 != end1; ++pos1 )
		{
			Progress( packListSize + patchListSize , ++cnt );

			packfile = pos1->second;

			errno_t err = _strlwr_s(packfile->directory);
			if ( err != 0 )
			{
				PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
				return -1;
			}

			sprintf_s( buffer ,"%s\n" , packfile->directory );
			fputs( buffer, stream );
		}

		std::hash_map<std::string, Patchfile*>::iterator pos2 =  m_SourcePatchList.begin();
		std::hash_map<std::string, Patchfile*>::iterator end2 =  m_SourcePatchList.end();
		Patchfile* patchfile = NULL;

		for( ; pos2 != end2; ++pos2 )
		{
			Progress( packListSize + patchListSize , ++cnt );

			patchfile = pos2->second;

			errno_t err = _strlwr_s(patchfile->fileName);
			if ( err != 0 )
			{
				PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)ePM_ERROR_STRLOWER, (LPARAM)0 );
				return -1;
			}

			sprintf_s( buffer
				,"%s;%u;%u;%u;%u\n"
				,patchfile->fileName
				,patchfile->sizeLow
				,patchfile->sizeHigh
				,patchfile->writeTime.dwLowDateTime
				,patchfile->writeTime.dwHighDateTime );
			fputs( buffer, stream );

		}

		fclose( stream );
		stream = NULL;
	}
	else
		return -2;

	return 0;
}

int Patchmaker::CollectFileInfo( char* root, const char* sub )
{
	HANDLE          search = NULL;
	char            where[ 1024 ] = {0,};

	char            filename[ 1024 ] = {0,};
	WIN32_FIND_DATA fileData;

	// 하위 디렉토리 적용여부
	if ( sub )
		sprintf_s( where, "%s\\%s", root, sub );
	else
		sprintf_s( where, "%s", root );

	sprintf_s( filename, "%s\\*.*", where );

	search = FindFirstFile( filename, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( (strcmp( fileData.cFileName, "."  ) != 0) && (strcmp( fileData.cFileName, ".." ) != 0) )
				{
					char directory[ 1024 ] = {0,};

					if ( sub )
						sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
					else
						sprintf_s( directory, "%s", fileData.cFileName );
					
					CollectFileInfo( root, directory );	// 재귀호출.
				}
			}
			else if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
			{
 				char fullPath[ 1024 ] = {0,};
 				char filePath[ 1024 ] = {0,};
 
 				if ( sub )
				{
					sprintf_s( fullPath, "%s\\%s\\%s", root, sub, fileData.cFileName );
					sprintf_s( filePath, "%s\\%s", sub, fileData.cFileName );
				}
 				else
				{
 					sprintf_s( fullPath, "%s\\%s", root, fileData.cFileName );
					sprintf_s( filePath, "%s", fileData.cFileName );
				}
				
				sFileInfo fileInfo;
				fileInfo.fullPath = fullPath;
				fileInfo.filename = filePath;
				m_CollectFileInfo.push_back( fileInfo );
			}
		} while ( FindNextFile( search, &fileData ) );

		FindClose( search );
		return 0;
	}
	return -1; // 검색 실패.

}