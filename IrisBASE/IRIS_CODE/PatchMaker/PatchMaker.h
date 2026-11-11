/**********************************************************************
 *<
	FILE: patchmaker.h

	DESCRIPTION:

	CREATED BY: Kwak CJ

	HISTORY: Created 08/11/19

 *>	Copyright (c) 2008, All Rights Reserved.
 **********************************************************************/
#ifndef __PATCHMAKER_H__
#define __PATCHMAKER_H__

#ifndef WINVER
#define WINVER          0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#pragma once

#define WM_PATCH_MSG		 WM_USER+0x10

enum ePatchMaker_ERROR
{
	ePM_COMPLETE,
	ePM_ERROR,
	ePM_ERROR_XCOPY,
	ePM_ERROR_STRLOWER,
	ePM_ERROR_MAKE_PATH,
	ePM_ERROR_WRITE_PATCHLIST,

	ePM_ERROR_NO_SOURCE,
	ePM_ERROR_NO_WORKER,
	ePM_ERROR_NO_TARGET,
	ePM_ERROR_NO_BACKUP,
};


#include <windows.h>        // 윈도우즈 헤더
#include <commctrl.h>
#include "resource.h"

#include <vector>
#include <hash_map>
#include <set>
#include <string>

#define SafeDelete(P)	{ if (P!=NULL) { delete(P); (P)=NULL; } }       // 메모리를 안전하게 해제.

const char* const ZIP_PASSWORD = "#7qlcRkfanwlrodldi#";
const char* const CHANGED_FILES_DIRECTORY_NAME = "changed files";

const char* const SKIP_MAKE_SUB_DIRECTORY[] = { "script" };
//const char* const PACK_DIRECTORY[] = {  "script\\direct", "script\\interface", "script\\language", "script\\resource" };
const char* const PACK_DIRECTORY[] = {  "script\\direct", "script\\interface", "script\\resource" };
const char* const LOG_DIRECTORY_NAME = "LOG-PatchMaker";


enum eSkipDirectory
{
	eSKIP_FOLDER_SCRIPT = 0,

	eSKIP_FOLDER_MAX_COUNT,
};

/// pack 파일 자체로 구성되어 있는 경로들
enum ePackDirectory
{
	eFOLDER_SCRIPT_DIRECT = 0,
	eFOLDER_SCRIPT_INTERFACE,
//	eFOLDER_SCRIPT_LANGUAGE,
	eFOLDER_SCRIPT_RESOURCE,

	ePACK_SCRIPT_FOLDER_MAX_COUNT = eFOLDER_SCRIPT_RESOURCE+1,
};


struct Packfile
{
	char directory[ 1024 ];			// 디렉토리
};

struct Patchfile
{
	char     fileName[ 1024 ];		// 파일이름
	DWORD    sizeHigh;				// 파일크기
	DWORD    sizeLow;				// 파일크기
	FILETIME writeTime;				// 파일날짜
};

struct sFileInfo
{
	std::string	fullPath;				// 절대 경로
	std::string	filename;				// 상대 경로
};

class Patchmaker
{
private:
 	HWND       m_wnd;
 	TCHAR*     m_windowName;
	bool       m_endApp;

 	char       m_currentDirectory[ MAX_PATH ];
 	char       m_sourceDirectory[ 1024 ];
 	char       m_targetDirectory[ 1024 ];
 	char       m_workerDirectory[ 1024 ];
 	char       m_backupDirectory[ 1024 ]; // 새로 패치 되면서 덮어씌우기 될 기존의 파일을 백업시킬 폴더. taget폴더 안에 생성한다.
	char       m_sourceListDirectory[ 1024 ];

	long	   m_VersionCount;

	bool	   m_isUseBackupDirectory;		// checkbox option1
	bool	   m_isUseSourceList;			

	/// source file list 관련 맵
	typedef std::pair <std::string, Packfile*> Packfile_Pair;
	std::hash_map<std::string, Packfile*>   m_SourcePackList; 

	typedef std::pair <std::string, Patchfile*> Patchfile_Pair;
	std::hash_map<std::string, Patchfile*>  m_SourcePatchList; 

	/// patch file list 관련 셋
	std::set<Packfile*>  m_PatchFolderList;
	std::set<Patchfile*> m_PatchFileList;

	/// 파일 검색용
	typedef std::vector< sFileInfo > CollectFile_Info;
	CollectFile_Info	 m_CollectFileInfo;

	HANDLE     m_workerThread;

	void       EventWrite      ( LPCTSTR format, ... );

	bool       GetFileTimeUTC  ( char* filename, SYSTEMTIME* utc );
	bool       SetFileTimeUTC  ( char* filename, SYSTEMTIME* utc );

	void       FileTimeToUTC   ( FILETIME* filetime );
	void       FileTimeToDOS   ( FILETIME* filetime );
	long	   FileTimeCompareToList( char* fileName, int* error );


	bool       Pack            ( char* source, char* target, SYSTEMTIME* utc=NULL,  bool bReverseWrite = true );
	bool       Unpack          ( char* source, char* target, SYSTEMTIME* utc=NULL );

	bool       StartPatch      ( );
	int        Xcopy           ( char* sub );

	int		   MakePatchList   (); /// 패치할 파일들 목록

	int		   MakeSourceList  (); /// 소스 디렉토리 안에 있는 파일들 목록


	bool	   CreateTodayBackupDirectory();
	
public:
	Patchmaker(void);

	int        Run             ( HWND wnd );

 	HWND       GetHwnd         ( ) { return m_wnd; }
 	TCHAR*     GetWindowName   ( ) { return m_windowName; }

	bool	   SetDirectory	   ( char* worker, char* source, char* target, char* backup, char* sourceList );
	BOOL       SetStatus       ( HWND wnd, char* msg );
	DWORD      Progress        ( DWORD length, DWORD offset );

	DWORD      WorkerThread    ( );
	void	   ErrorPrint(char *msg);

	bool	   MakePath		   ( char* path ); /// 해당 경로에 해당하는 폴더를 모두 만든다.
	LRESULT    PatchMsg        ( HWND wnd, WPARAM wParam, LPARAM lParam );
	void	   Destroy         ( );
	
	bool	   ReadSourceList  ( char* buffer );
	int        CollectListInfo ( char* root, char* sub );
	
	void	   SetUseBackupDirectoryFlag( bool flag ){   m_isUseBackupDirectory = flag; }
	void	   SetUseSourceListFlag( bool flag ){   m_isUseSourceList = flag; }
	void	   ClearList	   ();

	bool	   DeleteSubFiles  ( char* root, char* sub ); // 해당 경로 안에 있는 파일과 서브 폴더들을 지움.(※주의: 해당 경로는 지우지 않음)
	int        CollectFileInfo ( char* root, const char* sub ); // 해당 경로 안에 있는 파일 정보를 m_searchFileInfo에 삽입.

public:
	virtual ~Patchmaker(void);
};

#endif // __PATCHMAKER_H__
