#include "stdafx.h"
#include "LanguageDlg.h"

#include "RegenToolApp.h"
#include "Application.h"

IMPLEMENT_DYNAMIC(cLanguageDlg, CDialog)

BEGIN_MESSAGE_MAP(cLanguageDlg, CDialog)
END_MESSAGE_MAP()

cLanguageDlg::cLanguageDlg()
: CDialog(IDD_LANGUAGE)
{
}

cLanguageDlg::~cLanguageDlg()
{
}

void cLanguageDlg::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_COMBO_LANGUAGE, mCombo );
}

BOOL cLanguageDlg::OnInitDialog()
{
	if( CDialog::OnInitDialog() == FALSE )
		return FALSE;

	cString pathName;
	pathName.Format( "%s\\Language\\*", theApp.GetBaseDir().Cstr() );

	WIN32_FIND_DATA info;
	HANDLE h_dir_info = ::FindFirstFile( pathName.Cstr(), &info);
	if( h_dir_info == INVALID_HANDLE_VALUE ) 
		return FALSE;

	do {
		if( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{     
			// 검색되어진 정보가 폴더라면...
			if( info.cFileName[0] != '.' )
			{
				/// 콤보 채우기
				if( mCombo )
					mCombo.AddString( info.cFileName );
			}
		} 
	} while( ::FindNextFile(h_dir_info, &info) );

	::FindClose( h_dir_info );

	mCombo.SelectString(-1, "korea");
	return TRUE;
}

void cLanguageDlg::OnCancel()
{
	CDialog::OnCancel();

	int cursel = mCombo.GetCurSel();
	if(  cursel == -1 )
	{
		THEONE->mLangaugeFolder = "korea";
		return;
	}

	char text[256] = {0,};
	mCombo.GetLBText( cursel, text );

	THEONE->mLangaugeFolder = text;
}
