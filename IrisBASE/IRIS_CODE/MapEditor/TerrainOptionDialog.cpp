#include "stdafx.h"
#include "TerrainOptionDialog.h"

#include "resource.h"
#include "MapEditorView.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cTerrainOptionDialog, CDialog)

BEGIN_MESSAGE_MAP(cTerrainOptionDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIO_TOPTION_WIREFRAME, OnCheckedWireframe)
	ON_BN_CLICKED(IDC_RADIO_TOPTION_SOLID, OnCheckedSolid)
	ON_BN_CLICKED(IDC_RADIO_TOPTION_TEXTURED, OnCheckedTextured)
	ON_BN_CLICKED(IDC_RADIO_TOPTION_WIRETEXTURED, OnCheckedWireTextured)
END_MESSAGE_MAP()

cTerrainOptionDialog::cTerrainOptionDialog()
{
}

cTerrainOptionDialog::~cTerrainOptionDialog()
{
}

void cTerrainOptionDialog::ToggleWireTextured()
{
	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_WIREFRAME);
	btn->SetCheck( 0 );
	btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_SOLID);
	btn->SetCheck( 0 );
	btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_TEXTURED);
	btn->SetCheck( 0 );
	btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_WIRETEXTURED);
	btn->SetCheck( 0 );

	if( mViewMode != eTERRAIN_VIEW_WIRETEXTURED )
	{
		mViewMode = eTERRAIN_VIEW_WIRETEXTURED;

		CButton* btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_WIRETEXTURED);
		btn->SetCheck( 1 );
		TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_WIRETEXTURED );
	}
	else
	{
		mViewMode = eTERRAIN_VIEW_TEXTURED;

		CButton* btn = (CButton*)GetDlgItem(IDC_RADIO_TOPTION_TEXTURED);
		btn->SetCheck( 1 );
		TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_TEXTURED );
	}

	VIEW->Update();
}

void cTerrainOptionDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}

BOOL cTerrainOptionDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// ºä ¸ðµå
	mViewMode = eTERRAIN_VIEW_TEXTURED;
	CButton* btn = (CButton*)GetDlgItem( IDC_RADIO_TOPTION_TEXTURED );
	btn->SetCheck( 1 );
	return TRUE;
}

void cTerrainOptionDialog::OnCheckedWireframe()
{
	mViewMode = eTERRAIN_VIEW_WIREFRAME;
	TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_WIREFRAME );
	VIEW->Update();
}

void cTerrainOptionDialog::OnCheckedSolid()
{
	mViewMode = eTERRAIN_VIEW_SOLID;
	TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_SOLID );
	VIEW->Update();
}

void cTerrainOptionDialog::OnCheckedTextured()
{
	mViewMode = eTERRAIN_VIEW_TEXTURED;
	TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_TEXTURED );
	VIEW->Update();
}

void cTerrainOptionDialog::OnCheckedWireTextured()
{
	mViewMode = eTERRAIN_VIEW_WIRETEXTURED;
	TERRAINEDIT->SetViewMode( eTERRAIN_VIEW_WIRETEXTURED );
	VIEW->Update();
}
