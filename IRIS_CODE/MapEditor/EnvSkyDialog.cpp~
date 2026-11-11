#include "stdafx.h"
#include "EnvSkyDialog.h"

#include "resource.h"
#include "EnvEditor.h"

IMPLEMENT_DYNAMIC(cEnvSkyDialog, CDialog)

BEGIN_MESSAGE_MAP(cEnvSkyDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ESKY_ENABLED, OnClickedEnabled)
	ON_BN_CLICKED(IDC_BUTTON_ESKY_OPEN, OnClickedOpen)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
	//ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedHeight)
END_MESSAGE_MAP()

cEnvSkyDialog::cEnvSkyDialog()
{
}

cEnvSkyDialog::~cEnvSkyDialog()
{
}

void cEnvSkyDialog::Update( const cString& fileName )
{
	GetDlgItem( IDC_EDIT_ESKY_NAME )->SetWindowText( fileName.Cstr() );
}

void cEnvSkyDialog::Update( const cString& fileName, bool enabled, float scale ) //, float height )
{
	GetDlgItem( IDC_EDIT_ESKY_NAME )->SetWindowText( fileName.Cstr() );
	((CButton*)GetDlgItem( IDC_CHECK_ESKY_ENABLED ))->SetCheck( enabled ? 1 : 0 );
	mScale.SetValue( scale );
	//mHeight.SetValue( height / 100.0f );
}

void cEnvSkyDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );
}

BOOL cEnvSkyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mScale.Create( 0.1f, 10.0f, 1.0f, 0.1f, this, IDC_EDIT_ESKY_SCALE );
	//mHeight.Create( -1000.0f, 1000.0f, 0.0f, 1.0f, this, IDC_EDIT_WSKY_HEIGHT );
	return TRUE;
}

void cEnvSkyDialog::OnClickedEnabled()
{
	int checked = ((CButton*)GetDlgItem( IDC_CHECK_ESKY_ENABLED ))->GetCheck();

	ENVEDIT->SetSkyEnabled( checked ? true : false );
}

void cEnvSkyDialog::OnClickedOpen()
{
	/// 높이맵을 로딩
	CFileDialog dlg(
		TRUE, "Load Sky", "*.nif",
		OFN_HIDEREADONLY,
		"Sky Files (*.nif)| *.nif| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName;
		pathName = (LPCTSTR)dlg.GetPathName();

		if( ENVEDIT->LoadSky( pathName ) == false )
		{
			MessageBox( "Failed to Load Sky", "World" );
		}
	}
}

LRESULT cEnvSkyDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_ESKY_SCALE:
		{
			float s = mScale.GetValue();
			ENVEDIT->SetSkyScale( s );
		}
		break;
	}
	return 0;
}

//LRESULT cEnvSkyDialog::OnChangedHeight( WPARAM /*id*/, LPARAM )
//{
//	float height = mHeight.GetValue() * 100.0f;
//	ENVEDIT->SetSkyHeight( height );
//	return 0;
//}
