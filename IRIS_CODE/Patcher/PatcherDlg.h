	/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.25
* 내    용 : 패처 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "skin/SkinDialog.h"
#include "skin/SkinButton.h"
#include "skin/SkinProgressBar.h"
#include "skin/SkinRoundGauge.h"
#include "skin/SkinImageSlide.h"
#include "skin/SkinHtmlView.h"
#include "skin/SkinComboBox.h"
//#include "skin/SkinText.h"
#include "cPatchLexer.h"

#define WM_USER_MESSAGE (WM_APP + 1)	// User defined message.


/// 패치 메시지 박스 캡션
#define PATCHER					"IrisPatcher"
#define SKNINFO_FILENAME		"skin.txt"
#define URLINFO_FILENAME		"url.txt"
#define GAMEEXE_FILENAME		"irisclient.exe"
#define GAMEEXE_FILENAME_GM		"irisclient_gmtool.exe"
#define LAUNCHER_FILENAME		"irislauncher.exe"
#define LAUNCHER_DEV_FILENAME	"irislauncher_dev.exe"
#define LAUNCHER_TEST_FILENAME  "irislaunchertest.exe"

class cPatcher;

/// 컨트롤 아이디
enum
{
	CID_OK = 1000,
	CID_CLOSE,
	CID_HOME,
	CID_JOIN,
	CID_WINMODE,
	CID_PROGRESS,
	CID_PROGAUGE,
	CID_STATUS,
	CID_BOARD,
	CID_LANGUAGE,
	CID_FULL,
	CID_RESOLUTION,
};

/// URL 주소
enum eUrlAddress
{
	URL_HOME,
	URL_JOIN,
	URL_DOWNLOAD_ADDR,
	URL_DOWNLOAD_PATH,

	URL_COUNT,
};

/// 패치 상태
enum ePatchStatus
{
	/// 검색 중
	PATCH_SEARCHING = 0,
	/// 패치 중
	PATCH_PATCHING,
	/// 패치 완료
	PATCH_COMPLETE,
	/// 패치리스트 동기화 체크
	PATCH_LIST_CHECKING,
	PATCH_LIST_CHECK_END,
	///
	STEP_COUNT
};


/// 유저 에러 메시지.
enum ERROR_MSG_TYPE
{
	_UMT_ERROR_INIT_,
	_UMT_ERROR_SAVE_,
	_UMT_ERROR_CONNECT_,
	_UMT_ERROR_DOWNLOAD_,
	_UMT_ERROR_DOWNLOAD_PATCHLIST_,
	_UMT_ERROR_NOT_EXIST_PATCHLIST_,
	_UMT_ERROR_MEMORY_ALLOC_,
	_UMT_ERROR_MEMORY_FREE_,
	_UMT_ERROR_UPDATE_PACK_FILE,
	_UMT_ERROR_UPDATE_NOPACK_FILE,
	_UMT_ERROR_UPDATE_OPEN_PACK_FILE_,
	_UMT_ERROR_UPDATE_ACCESS_PACK_FILE_,
	_UMT_ERROR_SEARCH_,
	_UMT_ERROR_CREATE_PACK_FILE_,
	_UMT_ERROR_CREATE_FOLDER_,

	_UMT_ERROR_MAKELIST_WRONG_HEADER,
	_UMT_ERROR_MAKELIST_WRONG_EXECUTE,
	_UMT_ERROR_MAKELIST_DIFF_PATCHLIST,
	_UMT_ERROR_MAKELIST_PARSE,
	_UMT_ERROR_MAKELIST_END_APP,

	_UMT_ERROR_PATCH_CHECK_FAIL,            // 패치 동기화가 맞지 않음
};

enum eCLIENT_EXECUTE_MODE
{
	eCLIENT_EXECUTE_IDC,
	eCLIENT_EXECUTE_DEV,
	eCLIENT_EXECUTE_TEST,

	eCLIENT_EXECUTE_GMTOOL_IDC,
	eCLIENT_EXECUTE_GMTOOL_DEV,
	eCLIENT_EXECUTE_GMTOOL_TEST,

	eCLIENT_EXECUTE_MAX,
    eCLIENT_EXECUTE_GMTOOL_BEGIN = eCLIENT_EXECUTE_GMTOOL_IDC,
};

static const char* gPATCH_TYPE[eCLIENT_EXECUTE_MAX] = 
{
	"CLIENT-IRIS_IDC",
	"CLIENT-MAIN_DEV",
	"CLIENT-IRIS_TEST",

//	"GM_CLIENT-IRIS_IDC",
//	"GM_CLIENT-MAIN_DEV",
//	"GM_CLIENT-IRIS_TEST",
};

enum ePATCH_MODE
{
    ePATCH_MODE_DOWNLOAD,           // 패치 중
    ePATCH_MODE_DOWNLOAD_END,       // 패치 완료
    ePATCH_MODE_PATCH_CHECK,        // 패치 동기화
    ePATCH_MODE_GAME_RUN,           // 게임 실행
    ePATCH_MODE_EXIT,               // 프로그램 종료
};

struct sOption
{
	bool	        mWindowed;
	unsigned int    mWidth;
	unsigned int    mHeight;
	unsigned char   mBpp;
	float           mGamma;

	float           mEnvVolume;
	float           mEffVolume;

	int             mVideoMode; ///enum BCAP_PRESET

	unsigned int    mLanguage;

	sOption()
	{
		mWindowed = false;
		mWidth  = 1024;
		mHeight = 768;
		mBpp    = 32;
		mGamma  = 0.5f;

		mEnvVolume = 0.5f;	///0.0f ~ 1.0f
		mEffVolume = 0.5f;	///0.0f ~ 1.0f

		mVideoMode = 2;

		mLanguage = 82;
	}
};

/// 해상도
struct sResolution
{
	cString			strResolution;

	unsigned int	width;
	unsigned int	height;
	unsigned char	bpp;
};

struct sLangauge
{
	cString			str;
	unsigned int	code;
};

/// 패처 다이얼로그
class cPatcherDlg : public cSkinDialog
{
public:
	cPatcherDlg( const cString& path, const cString& cmdLine, CWnd* parent = 0 );
	~cPatcherDlg();

	void BeginThread();
	void GameReady( bool result );

	void SetProgressMax( ePatchStatus status, unsigned int max );
	void SetProgressPos( const char* fileName,unsigned int pos );

	void SetExecuteGM( eCLIENT_EXECUTE_MODE execute ){ mExecuteMode = execute; }

    ePATCH_MODE GetPatchMode( )                     { return mPatchMode; }
    void        SetPatchMode( ePATCH_MODE mode )    { mPatchMode = mode; }

    const char* GetHostAddr()                       { return mUrlArray[URL_DOWNLOAD_ADDR].Cstr(); }
    const char* GetHostUrl()                        { return mUrlArray[URL_DOWNLOAD_PATH].Cstr(); }

	/// 게임을 실행
	bool RunGame();


protected:

	bool LoadUrl();
    bool LoadPatchConfig( const char* pathName );
	bool LoadOption( const char* pathName );
	void EnumResolution();

    bool SaveOption( const char* pathName );
    WORD GetLocalCode(  const char* local  );


	void SetStatus( ePatchStatus status );
	void InitStatus( );

protected:
	static unsigned int __stdcall ThreadProc( LPVOID param );

protected:
	void OnOK();
	void OnCancel();

	/// 메시지
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

	/// 스킨 메시지
	afx_msg void OnOkButton();
	afx_msg void OnCloseButton();
	afx_msg void OnHomeButton();
	afx_msg void OnJoinButton();
	afx_msg void OnScreenModeButton();
	afx_msg void OnSelChangeLanguage();
	afx_msg void OnSelChangeResolution();

	afx_msg LRESULT OnUserMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:

	eCLIENT_EXECUTE_MODE    mExecuteMode;

	cString                 mPath;
	cString                 mAuthCode;
    cString                 mIP;

	cString                 mUrlArray[URL_COUNT];

	static cPatcher*        mPatcher;
	HANDLE                  mThread;

	HICON mIcon;

	/// patch config file
	typedef tArray<sLangauge*> cLanguageArr;
	cLanguageArr mLanguageArr;

	/// option file
	sOption mOptionData;

	/// resolution
	typedef tArray<sResolution*> cResolutionArr;
	cResolutionArr mResolutionArr;



	cSkinButton             mStartButton;
	cSkinButton             mCloseButton;
	cSkinButton             mHomeButton;
	cSkinButton             mJoinButton;

	/// 진행 바
	unsigned int            mProgressTotalNum;
	unsigned int            mProgressProcessCnt;
	cSkinProgressBar        mProgressBar;

	/// 공지 사항 뷰
	cSkinHtmlView*          mBoardView;

	cSkinComboBox           mLanguageCombo;
	cSkinButton             mScreenCheck;
	cSkinComboBox           mResolutionCombo;

	CStatic	                mStatusStatic;
	ePatchStatus            mStatus;

	bool mSaveOption;
    ePATCH_MODE mPatchMode;
};
