// ResourceCheckerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ResourceChecker.h"
#include "ResourceCheckerDlg.h"

//////////////////////////////////////////////////////////////////////////
#include "FilePackSystem.h"
#include "./client/FilterManager.h"
#include "./client/GameResourceManager.h"
#include "./client/uimanager.h"
#include "./client/TarotManager.h"
#include "./client/TipManager.h"
#include "./client/TutorialManager.h"
#include "./client/MakeStage.h"
#include "./client/DramaturgyManager.h"
#include "./client/ItemManager.h"
#include "./client/TriggerManager.h"
#include "./client/QuestManager.h"
#include "./client/ShortcutManager.h"
#include "./client/RenderSystem.h"
#include "./client/ResourceManager.h"
#include "./client/SoundSystem.h"
#include "./client/TitleManager.h"
#include "./client/LoadStage.h"
#include "./client/WorldManager.h"
#include "./client/SceneManager.h"
#include "FilePack.h"
#include "./application.h"

//#include "./common/SkillScript.h"
// #include "./client/ItemManager.h"
 #include "./client/ItemDefine.h"
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CResourceCheckerDlg 대화 상자




CResourceCheckerDlg::CResourceCheckerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResourceCheckerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CResourceCheckerDlg::~CResourceCheckerDlg()
{
	SAFE_DELETE(mpMakeStage);
	SAFE_DELETE(mpTutorialManager);
	SAFE_DELETE(mpTipManager);
	SAFE_DELETE(mpTarotManager);
	SAFE_DELETE(mpUIManager);
	SAFE_DELETE(mpGameResourceManager);
	SAFE_DELETE(mpFilterManager);
	SAFE_DELETE(mpShortcutManager);
	SAFE_DELETE(mpQuestManager);
	SAFE_DELETE(mpFileSystem);
	SAFE_DELETE(mpTriggerManager);
	SAFE_DELETE(mpItemManager);
	SAFE_DELETE(mpDramaturgyManager);
	SAFE_DELETE(mpRenderSystem);
	SAFE_DELETE(mpResourceManager);
	SAFE_DELETE(mpSoundSystem);
	SAFE_DELETE(mpTitleManager);
	SAFE_DELETE(mpLoadStage);
	SAFE_DELETE(mpWorldManager);
	SAFE_DELETE(mpSceneManager);

	NiShutdown();

}

void CResourceCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, mEditPath);
	DDX_Control(pDX, IDC_COMBO_LANG, mComboLanguage);
	DDX_Control(pDX, ID_BUTTON_START, mButtonStart);
	DDX_Control(pDX, IDCANCEL, mButtonCancel);
	DDX_Control(pDX, IDC_BUTTON_PATH, mButtonPath);
	DDX_Control(pDX, IDC_NOTICE, mEditNotice);
}

BEGIN_MESSAGE_MAP(CResourceCheckerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_PATH, &CResourceCheckerDlg::OnBnClickedButtonPath)
	ON_BN_CLICKED(ID_BUTTON_START, &CResourceCheckerDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()


// CResourceCheckerDlg 메시지 처리기

BOOL CResourceCheckerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	mComboLanguage.AddString( _T( "korea") );
	mComboLanguage.SetCurSel( 0 );
	
	TCHAR path[MAX_PATH] = {0,};
	GetCurrentDirectory( sizeof(path), path );
	mEditPath.SetWindowText( path );	

	mpDramaturgyManager = 0;
	mpItemManager = 0;
	mpTriggerManager = 0;
	mpFileSystem = 0;
	mpQuestManager = 0;
	mpShortcutManager = 0;
	mpFilterManager = 0;
	mpGameResourceManager = 0;
	mpUIManager = 0;
	mpTarotManager = 0;
	mpTipManager = 0;
	mpTutorialManager = 0;
	mpMakeStage = 0;
	mpRenderSystem = 0;
	mpResourceManager = 0;
	mpSoundSystem = 0;
	mpTitleManager = 0;
	mpLoadStage = 0;
	mpWorldManager = 0;
	mpSceneManager = 0;

	/// Gamebryo 스마트 포인트 사용객체에 대한 체크
#ifdef	_DEBUG
	unsigned int initialCount = NiRefObject::GetTotalObjectCount();
#endif

	/// Static Data Manager 초기화 : library 들을 요구한다. 즉 엔진 시스템의 load라고 보여짐..
	//NiInitOptions option;
	mBryoOption.SetFloodgateParallelExecution( false );
	NiStaticDataManager::Init( &mBryoOption );

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CResourceCheckerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX )
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CResourceCheckerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CResourceCheckerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CResourceCheckerDlg::OnBnClickedButtonPath()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR    folderPath[MAX_PATH] = {0,};
	TCHAR*	 pTitle = _T("Select IrisOnline Directory");

	ITEMIDLIST        *pidlBrowse  = NULL;

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.pidlRoot  = NULL;

	memset( &BrInfo, 0, sizeof(BrInfo) );
	BrInfo.pszDisplayName = folderPath;
	BrInfo.lpszTitle = pTitle;
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	// 다이얼로그를 띄우기
	pidlBrowse = ::SHBrowseForFolder(&BrInfo);    

	if( pidlBrowse != NULL)
	{
		// 패스를 얻어옴
		::SHGetPathFromIDList( pidlBrowse, folderPath );    
		mEditPath.SetWindowText( folderPath );	
		
		if( SetCurrentDirectory( folderPath ) == FALSE )
		{
			MessageBoxA( GetSafeHwnd(), "작업 폴더 설정 실패", "Error", MB_OK | MB_ICONERROR );
		}
	}

}

void CResourceCheckerDlg::OnBnClickedButtonStart()
{	mButtonStart.EnableWindow( FALSE );
	mButtonCancel.EnableWindow( FALSE );
	mButtonPath.EnableWindow( FALSE );

 	mpDramaturgyManager = new cDramaturgyManager;
 	if( !mpDramaturgyManager )
 	{
 		MessageBoxA( NULL, "Failed to Init DramaturgyManager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	/// 아이템 관리자 생성 및 초기화
 	mpItemManager = new cItemManager;
 	if( mpItemManager == 0 )
 	{
 		MessageBoxA( NULL, "Failed to Init Itemmanager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	mpTriggerManager = new cTriggerManager;
 	if( mpTriggerManager == 0  )
 	{
 		MessageBoxA( NULL, "Failed to Init Triggermanager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	//////////////////////////////////////////////////////////////////////////
	// application.cpp Init() 부
	// 파일 팩 시스템을 생성
	mpFileSystem = new cFilePackSystem;
	if( !mpFileSystem || !mpFileSystem->Init() )
	{
		MessageBoxA( NULL, "Failed to Init FileSystem", "Error", MB_OK | MB_ICONERROR );
		Clear();
		return ;
	}
 
 	/// 렌더링 시스템을 생성 및 초기화
 	mpRenderSystem = new cRenderSystem( GetSafeHwnd() );
 	if( !mpRenderSystem || !mpRenderSystem->Init() )
 	{
 		MessageBoxA( NULL, "Failed to Init RenderSystem", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	/// 자원 관리자를 생성 및 초기화
 	mpResourceManager = new cResourceManager;
 	if( !mpResourceManager || !mpResourceManager->Init() )
 	{
 		MessageBoxA( NULL, "Failed to Init Resource manager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	/// 사운드 시스템 생성 및 초기화
 	mpSoundSystem = new cSoundSystem;
 	if( !mpSoundSystem  || !mpSoundSystem->Init() )
 	{
 		MessageBoxA( NULL,"Failed to Init SoundSystem", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 	mpSoundSystem->SetHWND( GetSafeHwnd() );
 
 	/// 장면 관리자를 생성
 	mpSceneManager = new cSceneManager;
 	if( !mpSceneManager || !mpSceneManager->Init() )
 	{
 		MessageBoxA( NULL,"Failed to Init scene manager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return ;
 	}
 	//////////////////////////////////////////////////////////////////////////
 
 	mpQuestManager = new cQuestManager;
 	if( mpQuestManager == 0 )
 	{
 		MessageBoxA( NULL, "Failed to Init Questmanager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
 	mpShortcutManager = new cShortcutManager;
 	if( !mpShortcutManager )
 	{
 		MessageBoxA( NULL, "Failed to Init ShortcutManager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
  	mpFilterManager = new cFilterManager;
  	if( mpFilterManager == 0 || mpFilterManager->Init() == false )
  	{
 		MessageBoxA( NULL, "Failed to Init FilterManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
  	/// 게임 리소스 관리자 생성 및 초기화
  	mpGameResourceManager = new cGameResourceManager;
  	if( !mpGameResourceManager || !mpGameResourceManager->Init() )
  	{
 		MessageBoxA( NULL, "Failed to Init GameResourceManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
 	mpUIManager = new cUIManager;
  	if( !mpUIManager || mpUIManager->Init() == false )
  	{
 		MessageBoxA( NULL, "Failed to Init UIManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
  	/// 타롯 관리자 생성 및 초기화 
   mpTarotManager = new cTarotManager;
  	if( mpTarotManager == 0 || mpTarotManager->Init() == false )
  	{
 		MessageBoxA( NULL, "Failed to Init TarotManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
  	/// 팁 관리자 생성
  	mpTipManager = new cTipManager;
  	if( !mpTipManager || !mpTipManager->Init() )
  	{
 		MessageBoxA( NULL, "Failed to Init TipManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
  	mpTutorialManager = new cTutorialManager;
  	if( !mpTutorialManager || !mpTutorialManager->Init() )
  	{
 		MessageBoxA( NULL, "Failed to Init TutorialManager", "Error", MB_OK | MB_ICONERROR );
  		Clear();
  		return;
  	}
  
 	/// 호칭 관리자 생성
 	mpTitleManager = new cTitleManager;
 	if( !mpTitleManager )
 	{
 		MessageBoxA( NULL, "Failed to Init TitleManager", "Error", MB_OK | MB_ICONERROR );
 		Clear();
 		return;
 	}
 
	{
		///메모리 중간 해제
		SAFE_DELETE( mpTarotManager );
		SAFE_DELETE( mpTipManager );
		SAFE_DELETE( mpTutorialManager );
	}

  	mpGameResourceManager->IntroLoadThread();

	mpResourceManager->Clean();

	mpGameResourceManager->LoginLoadThread();

	mpResourceManager->Clean();
	
  	mpGameResourceManager->InitLogin();
  	mpGameResourceManager->InitServer();
 	mpGameResourceManager->ServerLoadThread();
 
   	mpMakeStage = new cMakeStage;
   	if( !mpMakeStage )
   	{
 		MessageBoxA( NULL, "Failed to Init MakeStage", "Error", MB_OK | MB_ICONERROR );
  		Clear();
   		return;
   	}
  	mpMakeStage->InitStage();

 	// 스킬 연출 부분 강제 로딩 [12/14/2009 Jo_Client]
 	typedef tPointerHashMap<unsigned long, void*> cSkillMap;
 	cSkillMap* skillListMap = SKILLSCRIPT->GetPlayerSkillInfoMap();
 	if( skillListMap && skillListMap->IsEmpty() == false )
 	{
 		cSkillMap::cConstIterator pos = skillListMap->Begin();
 		cSkillMap::cConstIterator end = skillListMap->End();
 
 		for( ; pos != end; ++pos )
 		{
 			unsigned long skillIndex = (*pos).mFirst;
 			sPlayerSkillBaseInfo* pSkillInfo = (sPlayerSkillBaseInfo*)(*pos).mSecond;
 			if( pSkillInfo && pSkillInfo->mType != eSKILLTYPE_PASSIVE )
 				DRAMATURGYMAN->DirectLoadPlayerDramaObj( skillIndex );
 		}
 	}

	mpResourceManager->Clean();

	/// 소스에서 직접 로딩 하는 파일들 로딩
	for( int cnt = 0; cnt < eLoadTexture; ++cnt )
	{
		if( RESOURCEMAN->LoadTexture( LoadInSource[cnt], false ) == false )
			assert(0);
	}
	
	for( int cnt = eLoadTexture; cnt < eLoadMapTexture; ++cnt )
	{
		if( RESOURCEMAN->LoadMapTexture( LoadInSource[cnt], false ) == false )
			assert(0);
	}

	for( int cnt = eLoadMapTexture; cnt < eLoadNif; ++cnt )
	{
		if( RESOURCEMAN->LoadNIF( LoadInSource[cnt], false ) == false )
			assert(0);
	}

	for( int cnt = eLoadNif; cnt < eLoadMapNif; ++cnt )
	{
		if( RESOURCEMAN->LoadMapNIF( LoadInSource[cnt] ) == false )
			assert(0);
	}


 //////////////////////////////////////////////////////////////////////////
	// 월드 관리자 생성 및 초기화
	mpWorldManager = new cWorldManager;
	if( !mpWorldManager )//|| !mpWorldManager->Init() )
	{
		MessageBoxA( NULL, "Failed to Init WorldManager", "Error", MB_OK | MB_ICONERROR );
		Clear();
		return;
	}

	mpLoadStage = new cLoadStage;
	if( !mpLoadStage )
	{
		MessageBoxA( NULL, "Failed to Init LoadStage", "Error", MB_OK | MB_ICONERROR );
		Clear();
		return;
	}

////////////////////////////////////////////////////////////////////////
  	typedef tPointerHashMap<unsigned long, void*> cArrayHashMap;
  	cArrayHashMap* mapArr = mpGameResourceManager->GetMapInfo();
  
  	cArrayHashMap::cConstIterator pos;
  	cArrayHashMap::cConstIterator end;
  	if( mapArr )
  	{
  		pos = mapArr->Begin();
  		end = mapArr->End();
  	}
  	else
  		pos = end;
  
  	/// 난이도에 따른 중복된 맵을 읽지 않도록 체크
  	tHashSet<unsigned long> loadMapSet;
  
  	for( ; pos != end; ++pos )
  	{
  		unsigned long mapIndex = (*pos).mFirst;
  		cArrayHashMap* mapModeArr = (cArrayHashMap*)(*pos).mSecond;
  		unsigned long mapMode = 0;
  
  		if( mapModeArr )
  		{
  			cArrayHashMap::cConstIterator pos2 = mapModeArr->Begin();
  
  			sMapInfo* pMapInfo = (sMapInfo*)(*pos2).mSecond;
  			if( pMapInfo )
  			{
  				/// 중복된 맵 체크
//  				if( loadMapSet.Find( pMapInfo->folderIdx ) != loadMapSet.End() )
//  					continue;
  
  				loadMapSet.Insert( pMapInfo->folderIdx );
  				mapMode = pMapInfo->mapMode;

				/// 미니맵 로딩
				cString pathName;
				pathName.Format( "./language/%s/MapImage_%02d.tga", cApplication::mLangaugeFolder.Cstr(), pMapInfo->folderIdx );

				NiTexture* ptex = RESOURCEMAN->LoadTexture( pathName, false );
				if( ptex == 0 )
				{
					assert( 0 && "failed to load texture" );
					return;
				}

  			}
  			else
  				assert(0);
  		}
  		else
  			assert(0);
  
    	mpLoadStage->Open();
 		while( 1 )
    		{
 			mpLoadStage->Process( mapIndex, mapMode );
 
 			if( mpLoadStage->GetLoadState() == cLoadStage::eLOADSTATE_END || mpLoadStage->GetLoadState() == cLoadStage::eLOADSTATE_NONE )
 				break;
 		}

    	mpSceneManager->Clear();
    	//mpResourceManager->Close();
 		mpResourceManager->Clean();
 
  		cStringT msg;
  		msg.Format( _T("%s 맵(난이도:%d) 로딩 중"), GAMERESOURCEMAN->GetMapName( mapIndex ), mapMode );
  		mEditNotice.SetWindowText( msg.Cstr() );
  		mEditNotice.Invalidate();
  	}

	/// 클라이언트 정보 수집
	char currentPath[MAX_PATH] = {0,};
	::GetCurrentDirectoryA( MAX_PATH, currentPath );
	Search( currentPath, gCheckFolder[0] );
	Search( currentPath, gCheckFolder[1] );
	Search( currentPath, gCheckFolder[2] );
	Search( currentPath, gCheckFolder[3] );
	
	/// 로드한 데이터 정보 수집
	tHashSet<cString>& logList = mpFileSystem->GetLog();
	if( logList.GetSize() < 1 )
		assert(0);

	/// 클라이언트 리스트에서 안쓰는 파일 뽑아내기
	tHashSet<cString>::cConstIterator logPos = logList.Begin();
	tHashSet<cString>::cConstIterator logEnd = logList.End();
	for( ; logPos != logEnd; ++logPos )
	{
		cString fileName = (*logPos);
		tSet<cString>::cIterator findPos;
		findPos = mFileList.Find( fileName );
		if( findPos != mFileList.End() )
		{
			mFileList.Erase( findPos );
		}
	}

	/// 클라이언트 리스트에서 안쓰는 파일 텍스트 파일로 저장하기
	cString fileLIstName;
	fileLIstName.Format( "%s\\FileList.txt", currentPath );

	FILE* stream;
	errno_t  err;
	err = fopen_s( &stream, fileLIstName.Cstr(), "wt" );
	if ( stream != NULL )
	{
		tSet<cString>::cConstIterator pos = mFileList.Begin();
		tSet<cString>::cConstIterator end = mFileList.End();
		cString fileName;
		for( ; pos != end; ++pos )
		{
			fileName = (*pos);

			if( fileName.GetLength() < 1 )
				assert(0);
			char buffer[ 1024 ] = { 0, };
			sprintf_s( buffer ,"%s\n" , fileName.Cstr() );
			fputs( buffer, stream );
		}
	}
	fclose( stream );
	stream = NULL;

	Clear();

	MessageBoxA( NULL, "Resource files Check End", "Notice", MB_OK | MB_APPLMODAL );
}

void CResourceCheckerDlg::Clear()
{
	if( mButtonStart )
		mButtonStart.EnableWindow( TRUE );
	if( mButtonCancel )
		mButtonCancel.EnableWindow( TRUE );
	if( mButtonPath )
		mButtonPath.EnableWindow( TRUE );

	SAFE_DELETE(mpMakeStage);
	SAFE_DELETE(mpTutorialManager);
	SAFE_DELETE(mpTipManager);
	SAFE_DELETE(mpTarotManager);
	SAFE_DELETE(mpUIManager);
	SAFE_DELETE(mpGameResourceManager);
	SAFE_DELETE(mpFilterManager);
	SAFE_DELETE(mpShortcutManager);
	SAFE_DELETE(mpQuestManager);
	SAFE_DELETE(mpFileSystem);
	SAFE_DELETE(mpTriggerManager);
	SAFE_DELETE(mpItemManager);
	SAFE_DELETE(mpDramaturgyManager);
	SAFE_DELETE(mpRenderSystem);
	SAFE_DELETE(mpResourceManager);
	SAFE_DELETE(mpSoundSystem);
	SAFE_DELETE(mpTitleManager);
	SAFE_DELETE(mpLoadStage);
	SAFE_DELETE(mpWorldManager);
	SAFE_DELETE(mpSceneManager);
	mFileList.Clear();
};

int CResourceCheckerDlg::Search( char* root, char* sub )
{
	HANDLE          search = NULL;
	char            where[ 1024 ];

	char            filename[ 1024 ];
	WIN32_FIND_DATAA  fileData;

	// 하위 디렉토리 적용여부
	if ( sub )
		sprintf_s( where, "%s\\%s", root, sub );
	else
		sprintf_s( where, "%s", root );

	sprintf_s( filename, "%s\\*.*", where );

	search = FindFirstFileA( filename, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( (strcmp( fileData.cFileName, "."  ) != 0) && (strcmp( fileData.cFileName, ".." ) != 0) )
				{
					bool IsCheckFolder = false;

					char directory[ 1024 ] = {0,};

					if ( sub )
						sprintf_s( directory, "%s\\%s", sub, fileData.cFileName );
					else
						sprintf_s( directory, "%s", fileData.cFileName );

					bool isCheckFolder = false;
					for( int index = CHECK_FOLDER_DATA; index < CHECK_FOLDER_COUNT ; ++index )
					{
						if( _strnicmp( directory, gCheckFolder[index], strlen( gCheckFolder[index] ) ) == 0 )
						{
							isCheckFolder = true;
							break;
						}
					}
					
					if( isCheckFolder == false )
						break;

					Search( root, directory );	// 재귀호출.
				}
			}
			else if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
			{
				char  fullpath[ 1024 ] = {0,};;
				charT fullpathW[ 1024 ] = {0,};

				if ( sub )
					sprintf_s( fullpath, "%s\\%s\\%s", root, sub, fileData.cFileName );
				else
					sprintf_s( fullpath, "%s\\%s", root, fileData.cFileName );

				::ConvertToUnicode( fullpath, fullpathW, sizeof( fullpathW ) );

				cString fileName;

				if( sub )
					fileName.Format( "%s\\%s", sub, fileData.cFileName );
				else
					fileName.Format( "%s", fileData.cFileName );

				if( ::strstr( fileData.cFileName, ".pack" ) != NULL ) 
				{
					cFilePack pack;
					if( pack.Open( fullpathW, cFilePack::OPEN_READONLY ) == true )
					{
						for( unsigned int index = 0, index_limit = pack.GetNumFiles(); index < index_limit ; ++index )
						{
							/// .pack 열기
							cFileInPackInfo fipi;
							if( pack.GetFileInfo( &fipi, index) == false )
								continue;

							if( fipi.IsDirectory() )
								continue;

							char filePathInPack[1024] = {0,};
							ConvertToAscii( fipi.GetPathName().Cstr(), filePathInPack, sizeof( filePathInPack) );

							/// 명확성 검사. fipi.pathname 과 filename을 비교하여 현재 맞는 경로에 있는 pack 파일을 열었는지 검사.
							/// 게임 설치 폴더 안에 pack 파일이 다른 곳으로 옮겨 졌거나, 다른 이름으로 저장되었다면 현재 pack 파일 무시.
							/// 그리고 이름은 다르지만 안의 내용물이 동일한 파일이 있다면 key 값으로 들어가는 fipi.pathname 이 겹치므로 현재 pack 파일 무시해야 함.
							if( index == 0 && _strnicmp( fileName.Cstr(), filePathInPack, ::strlen(fileName.Cstr()) - 5 ) != 0 )
								break;

							//cString path = filePathInPack;

							cString path;
							path.Format( "%s/%s", root, filePathInPack );
							path.Replace( '\\', '/' );
							path.ToLower();

							if( strstr( path.Cstr(), ".setting" ) == NULL )
								mFileList.Insert( path );

						}
					}

					pack.Close();
				}
				else
				{
					cString path;
					
// 					if ( sub )
// 						path.Format( "./%s\\%s", sub, fileData.cFileName ); 
// 					else
// 						path.Format( "./%s", fileData.cFileName );
					
					if ( sub )
						path.Format( "%s\\%s\\%s", root, sub, fileData.cFileName );
					else
						path.Format( "%s\\%s", root, fileData.cFileName );

					path.Replace( '\\', '/' );
					path.ToLower();
					
					if( strstr( path.Cstr(), ".setting" ) == NULL )
					{
						if( mFileList.Insert( path ) == false )
						{
							assert(0);
						}
					}
				}
			}
		} while ( FindNextFileA( search, &fileData ) );

		FindClose( search );

		return 0;
	}
	return -1; // 검색 실패.
}
