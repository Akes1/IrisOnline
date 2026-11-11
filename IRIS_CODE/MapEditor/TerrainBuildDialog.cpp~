#include "stdafx.h"
#include "TerrainBuildDialog.h"

#include "MapEditorView.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cTerrainBuildDialog, cDialog)

BEGIN_MESSAGE_MAP(cTerrainBuildDialog, cDialog)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_RAISE, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_LOWER, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_FLATTEN, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_SMOOTH, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_SYNC, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_SYNCALL, OnClickedSyncAllButton)
	ON_BN_CLICKED(IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS, OnClickedButton)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

cTerrainBuildDialog::cTerrainBuildDialog()
{
}

cTerrainBuildDialog::~cTerrainBuildDialog()
{
}

void cTerrainBuildDialog::UpdateInnerRadius( float radius )
{
	cString str;
	str.Format( "%.1f", radius );
	int innerRadius = int(radius * 10.0f);
	CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_INNER_RADIUS);
	CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_OUTER_RADIUS);

	if( innerRadius > outer->GetPos() )
	{
		outer->SetPos( innerRadius );
		SetDlgItemText( IDC_EDIT_TBUILD_OUTER_RADIUS, str.Cstr() );
	}
	inner->SetPos( innerRadius );
	SetDlgItemText( IDC_EDIT_TBUILD_INNER_RADIUS, str.Cstr() );
}

void cTerrainBuildDialog::UpdateOuterRadius( float radius )
{
	cString str;
	str.Format( "%.1f", radius );
	int outerRadius = int(radius * 10.0f);
	CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_INNER_RADIUS);
	CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_OUTER_RADIUS);

	if( outerRadius < inner->GetPos() )
	{
		inner->SetPos( outerRadius );
		SetDlgItemText( IDC_EDIT_TBUILD_INNER_RADIUS, str.Cstr() );
	}
	outer->SetPos( outerRadius );
	SetDlgItemText( IDC_EDIT_TBUILD_OUTER_RADIUS, str.Cstr() );
}

void cTerrainBuildDialog::SetCheckedButton( int id )
{
	mGroup.SetCheckedButton( id );
}

void cTerrainBuildDialog::SetPickPos( const NiPoint3& pos )
{
	cString s;
	s.Format( "( %.2f, %.2f, %.2f )", pos.x, pos.y, pos.z );

	GetDlgItem( IDC_EDIT_TBUILD_PICK_POS )->SetWindowText( s.Cstr() );
}

void cTerrainBuildDialog::DoDataExchange( CDataExchange* dx )
{
	cDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_TBUILD_RAISE, mRaiseButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_LOWER, mLowerButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_FLATTEN, mFlattenButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_SMOOTH, mSmoothButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_SYNC, mSyncButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_SYNCALL, mSyncAllButton );
	DDX_Control( dx, IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS, mSyncToPickPosButton );
}

BOOL cTerrainBuildDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mRaiseButton.SetGroup( &mGroup );
	mLowerButton.SetGroup( &mGroup );
	mFlattenButton.SetGroup( &mGroup );
	mSmoothButton.SetGroup( &mGroup );
	mSyncButton.SetGroup( &mGroup );
	mSyncToPickPosButton.SetGroup( &mGroup );

	mSyncButton.SetWindowText("Sync to Navi");
	mSyncAllButton.SetWindowText("Sync All to Navi");

	/// 안쪽 반지름
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_INNER_RADIUS );
	slider->SetRange( 5, 3000 );
	slider->SetPos( 20 );
	SetDlgItemText( IDC_EDIT_TBUILD_INNER_RADIUS, "2.0" );

	/// 바깥쪽 반지름
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_OUTER_RADIUS );
	slider->SetRange( 5, 3000 );
	slider->SetPos( 40 );
	SetDlgItemText( IDC_EDIT_TBUILD_OUTER_RADIUS, "4.0" );

	/// 강도
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_STRENGTH );
	slider->SetRange( 1, 100 );
	slider->SetPos( 10 );
	SetDlgItemInt( IDC_EDIT_TBUILD_STRENGTH, slider->GetPos() );

	/// 부드러움 비율
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_SMOOTH_RATIO );
	slider->SetRange( 10, 100 );
	slider->SetPos( 50 );
	SetDlgItemInt( IDC_EDIT_TBUILD_SMOOTH_RATIO, slider->GetPos() );

	SetPickPos( NiPoint3::ZERO );
	return TRUE;
}

void cTerrainBuildDialog::OnClickedButton()
{
	TERRAINEDIT->SetPickPosChanged( true );
 	VIEW->SetEditMode( eEDIT_TERRAIN_BUILD );
}

void cTerrainBuildDialog::OnClickedSyncAllButton()
{
	int mb = MessageBox( "Do you really want to sync all to NaviMesh?", "Terrain", MB_YESNO );

	if( mb == IDYES )
	{
		TERRAINEDIT->SyncAllToNaviMesh();
	}
}

void cTerrainBuildDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_TBUILD_INNER_RADIUS:
		{
			CSliderCtrl* outer = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_OUTER_RADIUS);
			int innerRadius = slider->GetPos();
			int outerRadius = outer->GetPos();
			cString str;
			str.Format( "%.1f", innerRadius / 10.0f );

			if( innerRadius > outerRadius )
			{
				outer->SetPos( innerRadius );
				SetDlgItemText( IDC_EDIT_TBUILD_OUTER_RADIUS, str.Cstr() );
			}
			SetDlgItemText( IDC_EDIT_TBUILD_INNER_RADIUS, str.Cstr() );

			TERRAINEDIT->SetPickPosChanged( true );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_TBUILD_OUTER_RADIUS:
		{
			CSliderCtrl* inner = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TBUILD_INNER_RADIUS);
			int innerRadius = inner->GetPos();
			int outerRadius = slider->GetPos();
			cString str;
			str.Format( "%.1f", outerRadius / 10.0f );

			if( outerRadius < innerRadius )
			{
				inner->SetPos( outerRadius );
				SetDlgItemText( IDC_EDIT_TBUILD_INNER_RADIUS, str.Cstr() );
			}
			SetDlgItemText( IDC_EDIT_TBUILD_OUTER_RADIUS, str.Cstr() );

			TERRAINEDIT->SetPickPosChanged( true );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_TBUILD_STRENGTH:
		SetDlgItemInt( IDC_EDIT_TBUILD_STRENGTH, slider->GetPos() );
		break;
	case IDC_SLIDER_TBUILD_SMOOTH_RATIO:
		SetDlgItemInt( IDC_EDIT_TBUILD_SMOOTH_RATIO, slider->GetPos() );
		break;
	}
}
