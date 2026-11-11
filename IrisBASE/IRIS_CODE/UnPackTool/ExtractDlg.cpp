// ExtractDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "UnPackTool.h"
#include "ExtractDlg.h"


// cExtractDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cExtractDlg, CDialog)

cExtractDlg::cExtractDlg(CWnd* pParent /*=NULL*/)
	: CDialog(cExtractDlg::IDD, pParent)
{
    mpPackFile = NULL;
}

cExtractDlg::~cExtractDlg()
{
}

void cExtractDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EXTRACT_PATH, mPath);
    DDX_Control(pDX, IDC_PROGRESS, mProgress);
    DDX_Control(pDX, IDC_EXTRACT_PWD, mPwd);
}


BEGIN_MESSAGE_MAP(cExtractDlg, CDialog)
    ON_BN_CLICKED(IDC_EXTRACT, &cExtractDlg::OnExtract)
    ON_BN_CLICKED(IDC_CANCEL, &cExtractDlg::OnCancel)
    ON_BN_CLICKED(IDC_SELECT_FOLDER, &cExtractDlg::OnSelFolder)
//    ON_WM_PAINT()
END_MESSAGE_MAP()


// cExtractDlg 메시지 처리기입니다.
void cExtractDlg::Init( cPackFile* pack, TCHAR* folder )
{
    mpPackFile = pack;

    CString str;
    str.Format( _T("%s"), folder );
    int pos = str.ReverseFind( '\\' );

    sprintf_s( mDir,        "%s", str.Left( pos )   );
    sprintf_s( mFullPath,   "%s", folder            );
}

void cExtractDlg::OnExtract()
{
   if( mPwd.GetWindowTextLengthA() < 1 )
    {
        MSGBOX_OK( _T("비밀번호를 입력해야 합니다.") );
        return;
    }

    TCHAR extPath[_MAX_PATH]    = {0,};
    TCHAR extPwd[50]            = {0,};

    mPath.GetWindowText( (LPSTR)extPath, mPath.GetWindowTextLengthA() + 1 );
    mPwd.GetWindowText( (LPSTR)extPwd, mPwd.GetWindowTextLengthA() + 1 );

    if( strcmp( ZIP_PASSWORD, extPwd ) != 0 )
    {
        MSGBOX_OK( _T("비밀번호가 틀립니다.") );
        return;
    }

    GetDlgItem( IDC_EXTRACT )->EnableWindow( FALSE );
    GetDlgItem( IDC_CANCEL  )->EnableWindow( FALSE );

    CZipArchive zip;
    int count = mpPackFile->ExtractReady( &zip, mFullPath, extPwd );
    mProgress.SetRange(0, count );

    for( int i = 0; i < count; i++ )
    {
        mpPackFile->Extract( &zip, i, extPath );
        mProgress.SetPos( i );
    }
    zip.Close();

    GetDlgItem( IDC_EXTRACT )->EnableWindow( TRUE );
    GetDlgItem( IDC_CANCEL  )->EnableWindow( TRUE );
}

int CALLBACK cExtractDlg::BrowseInfoCall( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    switch( uMsg )
    {
    case BFFM_INITIALIZED:
        ::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData );
        break;
    }
    return 0;
}

void cExtractDlg::OnSelFolder()
{
    GetDlgItem( IDC_SELECT_FOLDER )->EnableWindow( FALSE );

    TCHAR selPath[_MAX_PATH] = {0,};
    if( mPath.GetWindowTextLengthA() <= 0 )
        mPath.SetWindowTextA( mDir );
    else
        mPath.GetWindowTextA( selPath, mPath.GetWindowTextLengthA() + 1 );

    BROWSEINFO brInfo;
    memset( &brInfo, 0, sizeof(brInfo) ); 

    brInfo.hwndOwner        = NULL; 
    brInfo.pidlRoot         = NULL; 
    brInfo.lpszTitle        = "Select Directory"; 
    brInfo.ulFlags          = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN;
    brInfo.lParam           = (LPARAM)(LPCSTR)selPath; 
    brInfo.lpfn             = BrowseInfoCall;

    ITEMIDLIST *pidlBrowse; 
    pidlBrowse = ::SHBrowseForFolder( &brInfo ); 

    if(pidlBrowse != NULL){ 
        SHGetPathFromIDList( pidlBrowse, selPath ); 
    }

    mPath.SetWindowTextA( selPath );
    GetDlgItem( IDC_SELECT_FOLDER )->EnableWindow( TRUE );
}


BOOL cExtractDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mPath.SetWindowTextA( mDir );

    return TRUE;
}