// UnPackToolDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "UnPackTool.h"
#include "UnPackToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUnPackToolDlg 대화 상자

CUnPackToolDlg::CUnPackToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnPackToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);

    //mStatus     = NULL;
    mpPackFile  = NULL;
    mSortColumn = -1;
    mIsAsc      = true;
}

void CUnPackToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PACK_LIST, mPackList);
    DDX_Control(pDX, IDC_FILE_LIST, mFileList);
}

BEGIN_MESSAGE_MAP(CUnPackToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_FILE_OPEN, &CUnPackToolDlg::OnFileOpen)
    ON_BN_CLICKED(IDC_EXTRACT, &CUnPackToolDlg::OnExtract)
    ON_NOTIFY(NM_CLICK, IDC_PACK_LIST, &CUnPackToolDlg::OnSelectPack)
    ON_WM_MEASUREITEM()
    ON_BN_CLICKED(IDC_CLOSE, &CUnPackToolDlg::OnClose)
    ON_NOTIFY(LVN_KEYDOWN, IDC_PACK_LIST, &CUnPackToolDlg::OnKeyDownFromPackList)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILE_LIST, &CUnPackToolDlg::OnOrderFileList)
    ON_NOTIFY(NM_DBLCLK, IDC_PACK_LIST, &CUnPackToolDlg::OnViewExtFile)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()

// CUnPackToolDlg 메시지 처리기
BOOL CUnPackToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	/// packList 초기화
	mPackList.InsertColumn( 0,	_T(" pack 파일 "), LVCFMT_CENTER );
	mPackList.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
	mPackList.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES ); 

    /// fileList 초기화
	mFileList.InsertColumn( 0,	_T(" file "     ), LVCFMT_CENTER,   190 );
	mFileList.InsertColumn( 1,	_T(" 크기 "     ), LVCFMT_RIGHT,     50 );
	mFileList.InsertColumn( 2,	_T(" 수정 날짜 "), LVCFMT_LEFT,     150 );
	mFileList.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES ); 

    int strPartDim[]= {-1, 500, -1, 600, -1 };
    mStatusBar.Create( WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0,0,0,0), this, 0 );
    mStatusBar.SetParts(4, strPartDim);

    /// packFile class 초기화
    mpPackFile = new cPackFile;

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CUnPackToolDlg::OnPaint()
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
HCURSOR CUnPackToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUnPackToolDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    for( int i = 0; i < mPackList.GetItemCount(); i++ )
    {
        sPackInfo* p = (sPackInfo*)mPackList.GetItemData( i );
        SAFE_DELETE( p );
    }
    SAFE_DELETE( mpPackFile );

//    for( int i = 0; i < mFileList.GetItemCount(); i++ )
//    {
//         sFileInfo* p = (sFileInfo*)mFileList.GetItemData( i );
//        SAFE_DELETE( p );
//    }
}

void CUnPackToolDlg::OnFileOpen()
{	
    CFileDialog dlg( TRUE,"pack","*.pack", OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_NONETWORKBUTTON, "pack 파일 (*.pack) | *.pack |" );
    if( dlg.DoModal() == IDOK )
    {
        int         nItem   = mPackList.GetItemCount();
        POSITION    pos     = dlg.GetStartPosition();

        while( pos )
        {
            CString path = dlg.GetNextPathName(pos);

            if( FindPackList( path ) )
                continue;

            sPackInfo* pack = new sPackInfo;

            /// 파일 경로
            strcpy_s( pack->path, path );

            /// 파일 이름
            int     sPos        = path.ReverseFind( '\\' );
            CString fileName    = path.Right( (path.GetLength() - sPos - 1) ); 
            strcpy_s( pack->name, fileName );

            mPackList.InsertItem( nItem, fileName );
            mPackList.SetItemData( nItem, (DWORD_PTR)pack );
            ++nItem;
        }
    }
}
void CUnPackToolDlg::OnExtract()
{
    int pos  = mPackList.GetSelectionMark();
    if( pos < 0 )
        return;

    sPackInfo*  pack = (sPackInfo*)mPackList.GetItemData( pos );
    if( pack )
    {
        cExtractDlg extDlg;
        extDlg.Init( mpPackFile, pack->path );
        if( extDlg.DoModal() )
        {
        }
    }
}

void CUnPackToolDlg::OnSelectPack(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *pResult = 0;

    int pos  = mPackList.GetSelectionMark();
    if( pos < 0 )
        return;
/*
    sPackInfo* pack = (sPackInfo*)mPackList.GetItemData( pos );

    /// 상태바에 .pack파일 정보 표시
    SetStatusBar( pack->path );
*/
}

void CUnPackToolDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
    if( nIDCtl == IDC_PACK_LIST || nIDCtl == IDC_FILE_LIST )
    {
        lpMeasureItemStruct->itemHeight = 15;
    }

    //CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CUnPackToolDlg::SetStatusBar(const TCHAR* path)
{
    CFileStatus status;
    if( CFile::GetStatus( path, status ) )
    {
        CString msg;

        msg.Format("압축된 파일:%d개 수정한 날짜:%d년%d월%d일 %d시%d분", 
                                            mFileList.GetItemCount() ,
                                            status.m_mtime.GetYear(), 
                                            status.m_mtime.GetMonth(),
                                            status.m_mtime.GetDay(),
                                            status.m_mtime.GetHour(),
                                            status.m_mtime.GetMinute() );

        mStatusBar.SetText( msg,  0, SB_SETTEXT);
        //mStatusBar.SetText( "111",  2, SB_SETTEXT);
    }
}

void CUnPackToolDlg::OnClose()
{
    PostMessage( WM_CLOSE );
}

void CUnPackToolDlg::OnKeyDownFromPackList(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *pResult = 0;

    if( pLVKeyDow->wVKey == VK_DELETE )
    {
        int pos = mPackList.GetSelectionMark();
        
        sPackInfo* p = (sPackInfo*)mPackList.GetItemData( pos );
        SAFE_DELETE( p );

        mPackList.DeleteItem( pos );
//        for( int i = 0; i < mFileList.GetItemCount(); i++ )
//        {
//            sFileInfo* p = (sFileInfo*)mFileList.GetItemData( i );
//            SAFE_DELETE( p );
//        }
        mFileList.DeleteAllItems();
    }
}
bool CUnPackToolDlg::FindPackList(const TCHAR* path)
{
    for( int i = 0; i < mPackList.GetItemCount(); i++ )
    {
        sPackInfo* p = (sPackInfo*)mPackList.GetItemData( i );
        if( p )
        {
            if( strcmp( p->path, path ) == 0 )
                return true;
        }
    }
    return false;
}

void CUnPackToolDlg::OnOrderFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *pResult = 0;

    int column = pNMLV->iSubItem;
    if( column == mSortColumn )
        mIsAsc = !mIsAsc;
    else
        mIsAsc = TRUE;

    mSortColumn = column;

    int itemCnt = mFileList.GetItemCount();
    CString** pStr = new CString*[itemCnt];
    for( int i = 0; i < itemCnt; i++ )
    {
        pStr[i] = new CString( mFileList.GetItemText(i, column) );
        mFileList.SetItemData( i, (DWORD_PTR)pStr[i] );
    }

    mFileList.SortItems( CompareFunc, (LPARAM)mIsAsc );

    for( int i = 0; i < itemCnt; i++ )
        if( pStr[i] ) delete pStr[i];
    delete []pStr;
}

static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3 )
{
    CString*  str1 = (CString*)lParam1;
    CString*  str2 = (CString*)lParam2;
    BOOL      IsAsc = (BOOL)lParam3;

    if( !str1 || !str2 )
        return 0;

    int ret = strcmp( *str1, *str2 );
    if( IsAsc )
        return ret;
    else
        return -ret;
}
void CUnPackToolDlg::OnViewExtFile(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *pResult = 0;

    int pos  = mPackList.GetSelectionMark();
    if( pos < 0 )
        return;

    sPackInfo* pack = (sPackInfo*)mPackList.GetItemData( pos );

    /// 압축된 file list
    mpPackFile->View( pack->path );

    /// 화면 출력
    mFileList.DeleteAllItems();

    TCHAR fsize[52] = {0,};
    TCHAR ftime[52] = {0,};
    for( unsigned int i = 0; i < mpPackFile->GetFileCount(); i++ )
    {
        /// 파일 크기
        WORD byteSize   = mpPackFile->GetFileSize( i );
        WORD kbSize     = ((double)byteSize / 1024) + 1;
        sprintf_s( fsize, "%dKB",  kbSize );

        /// 수정한 날짜
        cFileTime   mWriteTime  = mpPackFile->GetFileUpdateTime( i );
        FILETIME    fileTime    = mWriteTime.GetFileTime( );
        FILETIME    localTime;
        SYSTEMTIME  utc;
        SYSTEMTIME  utc2;
        SystemTimeToFileTime( &utc, &fileTime );
        FileTimeToLocalFileTime( &fileTime, &localTime );
        FileTimeToSystemTime( &localTime, &utc2 );

        int hour = utc2.wHour - 12;
        if( hour <= 0 ) hour = utc2.wHour;

        sprintf_s( ftime, "%4d-%02d-%02d %s %02d:%02d", utc2.wYear, utc2.wMonth, utc2.wDay, 
                                                        ((utc2.wHour - 12) > 0 ) ? "오후" : "오전", hour, utc2.wMinute );

        mFileList.InsertItem( i, (LPCTSTR)mpPackFile->GetFileName( i ) );
        mFileList.SetItemText( i, 1,  (LPCTSTR)fsize );
        mFileList.SetItemText( i, 2,  (LPCTSTR)ftime );
    }

    /// 상태바에 .pack파일 정보 표시
    SetStatusBar( pack->path );
}

void CUnPackToolDlg::OnDropFiles(HDROP hDropInfo)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

    char file[_MAX_PATH] = {0,};
    
    UINT count = DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );

    int         nItem               = mPackList.GetItemCount();

    for( UINT i = 0; i < count; i++ )
    {
        DragQueryFile( hDropInfo, i, file, _MAX_PATH );
        CString path( file );

        if( FindPackList( path ) )
            continue;

        sPackInfo* pack = new sPackInfo;

        /// 파일 경로
        strcpy_s( pack->path, path );

        /// 파일 이름
        int     sPos        = path.ReverseFind( '\\' );
        CString fileName    = path.Right( (path.GetLength() - sPos - 1) ); 
        strcpy_s( pack->name, fileName );

        mPackList.InsertItem( nItem, fileName );
        mPackList.SetItemData( nItem, (DWORD_PTR)pack );
        ++nItem;
    }

    ::DragFinish( hDropInfo );
//    CDialog::OnDropFiles(hDropInfo);
}