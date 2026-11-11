#include <Windows.h>
#include <stdio.h>

#define WM_LAUNCHER_RUN_MSG		WM_USER+0x12

enum eLAUNCHER_MODE
{
#ifdef _SERVER
//	eLAUNCHER_MODE_SERVER_DEV,
	eLAUNCHER_MODE_SERVER_TEST,
	eLAUNCHER_MODE_SERVER_IDC,
#endif

//  eLAUNCHER_MODE_CLIENT_DEV,
//	eLAUNCHER_MODE_GMTOOL_DEV,

//	eLAUNCHER_MODE_CLIENT_TEST,
	eLAUNCHER_MODE_GMTOOL_TEST,

//	eLAUNCHER_MODE_CLIENT_IDC,
	eLAUNCHER_MODE_GMTOOL_IDC,

	eLAUNCHER_MODE_MAX,
};

static const char* gPATCH_TYPE[eLAUNCHER_MODE_MAX] = 
{
#ifdef _SERVER
//	"SERVER-MAIN_DEV",
	"SERVER-IRIS_TEST",
	"SERVER-IRIS_IDC",
#endif

//	"PATCHER-MAIN_DEV",
//	"GM_CLIENT-MAIN_DEV",

//	"PATCHER-IRIS_TEST",
	"GM_CLIENT-IRIS_TEST",

//	"PATCHER-IRIS_IDC",
 	"GM_CLIENT-IRIS_IDC",
};


const char* const OPTION_FILENAME = "IrisLauncherMode.txt";

struct sRunMode
{
	char mMode[MAX_PATH];
	char mHost[MAX_PATH];
	char mAddr[MAX_PATH];
	int  mPort;

	sRunMode()
	{
		memset( this, 0, sizeof( sRunMode ) );
	}
};

class cDevLauncher
{
public:
	cDevLauncher();
	~cDevLauncher();

	HWND Init( HWND wnd );

	bool AddComboBoxList();
	bool SetDlgOption();

	void SetRunMode( sRunMode* );

	char* GetHost();
	char* GetAddr();
	int	  GetPort();

	eLAUNCHER_MODE GetLauncherMode(){ return mLauncherMode; }

	/// 可记 颇老 历厘
	void SaveOptionFile();

	LRESULT SetModeMsgProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void EventWrite(LPCTSTR format, ...);
	
	void NoticeMsgUpdate();

private:
	eLAUNCHER_MODE  mLauncherMode;
    sRunMode        mModeArray[eLAUNCHER_MODE_MAX];

	HWND	mMainWnd;
	HWND	mDevLauncherDlgWnd;
};
