#include "stdafx.h"
#include "SettingViewDialog.h"

#include "MapEditorView.h"

#include "./Engine/SceneManager.h"

IMPLEMENT_DYNAMIC(cSettingViewDialog, CDialog)

BEGIN_MESSAGE_MAP(cSettingViewDialog, CDialog)
	ON_COMMAND_RANGE(IDC_CHECK_SETVIEW_LOD, IDC_CHECK_SETVIEW_HUD, OnCheckedButton)
END_MESSAGE_MAP()

cSettingViewDialog::cSettingViewDialog()
{
}

cSettingViewDialog::~cSettingViewDialog()
{
}

void cSettingViewDialog::ToggleScene()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE );
	check->SetCheck( !check->GetCheck() );

	((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE_SELGROUP ))->EnableWindow( check->GetCheck() ? TRUE : FALSE );

	VIEW->Update();
}

void cSettingViewDialog::ToggleTerrainSelGroup()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN );

	if( check->GetCheck() == 0 )
		return;

	check = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN_SELGROUP );
	check->SetCheck( !check->GetCheck() );

	VIEW->Update();
}

void cSettingViewDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}

BOOL cSettingViewDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// ºä ¸ðµå
	CButton* btn;
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN_SELGROUP );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE_SELGROUP );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_LIGHT );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SOUND );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_AREA );
	btn->SetCheck( 1 );
	btn = (CButton*)GetDlgItem( IDC_CHECK_SETVIEW_HUD );
	btn->SetCheck( 1 );
	return TRUE;
}

void cSettingViewDialog::OnCheckedButton( UINT /*id*/ )
{
	((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN_SELGROUP ))->EnableWindow( IsTerrainChecked() ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE_SELGROUP ))->EnableWindow( IsSceneChecked() ? TRUE : FALSE );

	//if( id == IDC_CHECK_SETVIEW_NAVIFIELD )
	//{
	//	SCENEMAN->SetStaticeObjectWireframe( IsNaviFieldChecked() );
	//}

	VIEW->Update();
}
