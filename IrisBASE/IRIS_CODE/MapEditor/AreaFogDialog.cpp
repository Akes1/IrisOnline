#include "stdafx.h"
#include "AreaFogDialog.h"

#include "resource.h"
#include "AreaEditor.h"
#include "EnvCameraDialog.h"

IMPLEMENT_DYNAMIC(cAreaFogDialog, CDialog)

BEGIN_MESSAGE_MAP(cAreaFogDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_NFOG_ENABLED, OnClickedEnabled)
	ON_BN_CLICKED(IDC_BUTTON_NFOG_COLOR, OnClickedColorBox)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedDepth)
END_MESSAGE_MAP()

cAreaFogDialog::cAreaFogDialog()
{
}

cAreaFogDialog::~cAreaFogDialog()
{
}

void cAreaFogDialog::Update( bool enabled, const NiColor& color, float depth )
{
	((CButton*)GetDlgItem( IDC_CHECK_NFOG_ENABLED ))->SetCheck( enabled ? 1 : 0 );

	unsigned char r = (unsigned char)(color.r * 255.0f);
	unsigned char g = (unsigned char)(color.g * 255.0f);
	unsigned char b = (unsigned char)(color.b * 255.0f);
	mColorBox.SetColor( RGB(r,g,b) );
	
	mDepth.SetValue( depth );
}

void cAreaFogDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_NFOG_COLOR, mColorBox );
}

BOOL cAreaFogDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mColorBox.SetColor( RGB( 255, 255, 255 ) );
	mDepth.Create( 0.0f, 1.0f, 0.0f, 0.01f, this, IDC_EDIT_AFOG_DEPTH );
	return TRUE;
}

void cAreaFogDialog::OnClickedEnabled()
{
	int checked = ((CButton*)GetDlgItem( IDC_CHECK_NFOG_ENABLED ))->GetCheck();

	COLORREF rgb = mColorBox.GetColor();
	NiColor color;
	color.r = (float)GetRValue( rgb ) / 255.0f;
	color.g = (float)GetGValue( rgb ) / 255.0f;
	color.b = (float)GetBValue( rgb ) / 255.0f;
	
	float depth = mDepth.GetValue();

	AREAEDIT->SetFog( checked ? true : false, color, depth );
}

void cAreaFogDialog::OnClickedColorBox()
{
	CColorDialog dlg( mColorBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mColorBox.SetColor( rgb );

		AREAEDIT->SetFogColor( color );
	}
}

LRESULT cAreaFogDialog::OnChangedDepth( WPARAM /*id*/, LPARAM )
{
	float depth = mDepth.GetValue();

	AREAEDIT->SetFogDepth( depth );
	return 0;
}
