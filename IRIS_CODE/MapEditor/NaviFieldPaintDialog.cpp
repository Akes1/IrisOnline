#include "stdafx.h"
#include "NaviFieldPaintDialog.h"

#include "MapEditorView.h"
#include "NaviFieldEditor.h"

IMPLEMENT_DYNAMIC(cNaviFieldPaintDialog, cDialog)

BEGIN_MESSAGE_MAP(cNaviFieldPaintDialog, cDialog)
	ON_BN_CLICKED(IDC_BUTTON_NPAINT_COLOR, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_NPAINT_PICK, OnClickedButton)
	ON_WM_HSCROLL()
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedValue)
END_MESSAGE_MAP()

cNaviFieldPaintDialog::cNaviFieldPaintDialog()
{
	mColor[0] = RGB(0, 0, 0);
	mColor[1] = RGB(255, 0, 0);
	mColor[2] = RGB(0, 255, 0);
	mColor[3] = RGB(0, 0, 255);
	mColor[4] = RGB(255, 255, 0);
	mColor[5] = RGB(255, 0, 255);
	mColor[6] = RGB(0, 255, 255);
	mColor[7] = RGB(128, 0, 0);
	mColor[8] = RGB(0, 128, 0);
	mColor[9] = RGB(0, 0, 128);
	mColor[10] = RGB(128, 128, 0);
	mColor[11] = RGB(128, 0, 128);
	mColor[12] = RGB(0, 128, 128);
	mColor[13] = RGB(255, 255, 255);
	mColor[14] = RGB(128, 128, 128);
	mColor[15] = RGB(64, 64, 64);
}

cNaviFieldPaintDialog::~cNaviFieldPaintDialog()
{
}

void cNaviFieldPaintDialog::UpdateRadius( float radius )
{
	cString str;
	str.Format( "%.1f", radius );
	int outerRadius = int(radius * 10.0f);
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_NPAINT_RADIUS);

	slider->SetPos( outerRadius );
	SetDlgItemText( IDC_EDIT_NPAINT_RADIUS, str.Cstr() );
}

void cNaviFieldPaintDialog::SetCheckedButton( int id )
{
	mGroup.SetCheckedButton( id );
}

void cNaviFieldPaintDialog::DoDataExchange( CDataExchange* dx )
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_NPAINT_COLOR, mColorButton );
	DDX_Control( dx, IDC_BUTTON_NPAINT_PICK, mPickButton );

	DDX_Control( dx, IDC_BUTTON_NPAINT_COLORBOX, mColorBox );
}

BOOL cNaviFieldPaintDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mColorButton.SetGroup( &mGroup );
	mPickButton.SetGroup( &mGroup );

	/// 속성 값
	mValueEdit.Create( 0.0f, 15.0f, 0.0f, 1.0f, this, IDC_EDIT_NPAINT_FIELD_VALUE );

	/// 반지름
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_NPAINT_RADIUS );
	slider->SetRange( 5, 500 );
	slider->SetPos( 20 );
	SetDlgItemText( IDC_EDIT_NPAINT_RADIUS, "2.0" );
	return TRUE;
}

void cNaviFieldPaintDialog::OnClickedButton()
{
	VIEW->SetEditMode( eEDIT_NAVIFIELD_PAINT );
}

void cNaviFieldPaintDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_NPAINT_RADIUS:
		{
			int radius = slider->GetPos();
			cString str;
			str.Format( "%.1f", radius / 10.0f );
			SetDlgItemText( IDC_EDIT_NPAINT_RADIUS, str.Cstr() );

			NAVIFIELDEDIT->SetPickPosChanged( true );
			VIEW->Update();
		}
		break;
	}
}

LRESULT cNaviFieldPaintDialog::OnChangedValue( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_NPAINT_FIELD_VALUE:
		{
			unsigned char i = (unsigned char)mValueEdit.GetValue();
			assert( i < 16 );

			mColorBox.SetColor( mColor[i] );
		}
		break;
	}
	return 0;
}
