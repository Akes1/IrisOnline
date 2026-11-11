/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.21
* 내    용 : 패처
* 주의사항 : 
*===========================================================================*/
#pragma once


#include "BSTreeLinkedList.h"
#include "PatcherDlg.h"

#define MAX_PACKS	128

class cPatcherDlg;
class ClassDesc;
enum  ePatchStatus;

/// .pack 파일로 저장이 안되야 하는 폴더들. - # script 폴더가 항상 마지막에 있어야 함.
const char* const gUnpackFolder[]  = { "sound", "launcher", "hackshield", "script" };

/// 클라이언트의 파일 정보를 모을 폴더들.
const char* const gCheckFolder[]    = { "data", "script", "sound", "miles", "language", "hackshield" };
const char* const SCRIPT_DIRECTORY = "script";
const DWORD       g16KBYTE         = 16384;


// # script 폴더가 항상 마지막에 있어야 함.
enum eUnpackFolder
{
	FOLDER_SOUND = 0,
	FOLDER_LAUNCHER,

    FOLDER_HACKSHIELD,

	FOLDER_SCRIPT,
	UNPACK_FOLDER_COUNT,
};

enum eCheckFolder
{
	CHECK_FOLDER_DATA = 0,
	CHECK_FOLDER_SCRIPT,
	CHECK_FOLDER_SOUND,
	CHECK_FOLDER_SOUND_MILES,
	CHECK_FOLDER_LANGUAGE,
	CHECK_FOLDER_HACKSHIELD,

	CHECK_FOLDER_COUNT
};

enum eMAKELIST_RETURN_VALUE
{
	eMAKELIST_SUCCESS,
	eERROR_MAKELIST_WRONG_HEADER,
	eERROR_MAKELIST_WRONG_EXECUTE,
	eERROR_MAKELIST_DIFF_PATCHLIST,
	eERROR_MAKELIST_PARSE,
	eERROR_MAKELIST_END_APP,
};

struct sFilePathInfo
{
	char fullPath  [ MAX_PATH ];
	char rootFolder[ _MAX_DIR ];
	char packFolder[ _MAX_DIR ];
	char fileName  [ _MAX_DIR ];

	sFilePathInfo()
	{
		memset( fullPath,    0, sizeof( fullPath ));
		memset( rootFolder,  0, sizeof( rootFolder ));
		memset( packFolder,  0, sizeof( packFolder ));
		memset( fileName,    0, sizeof( fileName ));
	}
};

struct Packfile
{
	char directory[ 1024 ];			// 디렉토리
};

struct Packlist
{
	Packfile* pack;
	ULONG     offset;				// 오프셋
	ULONG     length;				// 전체갯수
};

struct Patchfile
{
	sFilePathInfo fileInfo;			// 파일이름 정보
	DWORD		  sizeHigh;			// 파일크기
	DWORD		  sizeLow;			// 파일크기
	FILETIME	  writeTime;		// 파일날짜

};

typedef tPointerHashMap<cStringT ,Patchfile*> cPatchfileList;
struct sPackUpdateInfo
{
	cPatchfileList patchfiles;
	CZipStringArray updateFiles;
};

struct Patchlist
{
	Patchfile* file;				// 업데이트 파일 구조체
	ULONG      offset;				// 오프셋
	ULONG      length;				// 전체갯수
};

/// 패처
class cPatcher
{
public:
	cPatcher( cPatcherDlg* dlg );
	~cPatcher();

	void Clear();
	void Init( HWND wnd, HINSTANCE instance );

	DWORD           ThreadProc();
	void            StopWorking();
	bool            IsWorking() const;
	bool            IsEndIO() const;
	void            SetExcuteMode( eCLIENT_EXECUTE_MODE mode ) { mExecuteMode = mode; }
	unsigned long   GetLoginIP();
	bool            PatchlistCheck();
	char*			GetWorkDirectory(){ return m_currentDirectory; }

protected:
	eMAKELIST_RETURN_VALUE MakeList(char* buffer);

	void SetProgressMax( ePatchStatus status, int max );
	void SetProgressPos( const char* fileName, int pos );

private:
	int  GetString(int id, char* buffer, int bufferSize ); 
	bool UnpackPatchList( char** buffer, DWORD *bufferLen );
	void FileTimeToDOS(FILETIME* filetime);

	bool UpdateNoPackFile(Patchfile* patchfile, char* buffer, DWORD bufferLen);
	bool UpdatePackFile(  Patchfile* patchfile, char* buffer, DWORD bufferLen , cFilePack *pack );
	int  Search(char* root, char* sub, bool opcode);
	HANDLE GetProcessList( bool* bFuncFailed );

	bool CheckPackFile( sFilePathInfo* fileName );
	bool SetPackFileNameInfo( char* fullPath, sFilePathInfo* fileInfo );

	void EventWrite(LPCTSTR format, ...);
	void ErrorPrint(char *msg);
	void ErrorMessageBox(LPCTSTR caption, LPCTSTR format, ...);


private:
	/// 다이얼로그
	cPatcherDlg* mDlg;
	HWND	     mWnd;
	HINSTANCE    mInstance;
	bool         mEndApp;
	bool         mEndIO;

	char         m_currentDirectory[ MAX_PATH ];


	Packlist   m_packlist;

	/// 전체 정보
	tPointerHashMap<cStringT, Patchfile*> mPackPatchList;
	tPointerHashMap<cStringT, Patchfile*> mUnPackPatchList;


	/// 분석후 관리 정보(각 pack 파일별로 업데이트 해야할 정보들)
	tPointerHashMap<cStringT, sPackUpdateInfo*> mPackUpdateFileMap;

	/// 진행바
	unsigned int mNumPatchFiles;

	/// loader
	ClassDesc* m_libClassDesc;

	/// 버젼 정보 체크해서 모든 pack 파일 지우기
	bool	mDelAllPack;

	// eCLIENT_EXECUTE_MODE
	eCLIENT_EXECUTE_MODE    mExecuteMode;
	char                    mLoginIP [ MAX_PATH ];
	char                    mHostAddr[ MAX_PATH ];
	char                    mHostUrl [ MAX_PATH ];
	char                    mHostList[ MAX_PATH ];

public:
	static DWORD WINAPI LoaderProgressRoutine( DWORD totalFileSize, DWORD totalBytesTransferred, void* data );

};




inline
bool cPatcher::IsWorking() const
{
	return !mEndApp;
}

inline
bool cPatcher::IsEndIO() const
{
	return mEndIO;
}
