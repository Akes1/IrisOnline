#include "stdafx.h"
#include "AreaLightDialog.h"

#include "resource.h"
#include "MapEditorView.h"
#include "AreaEditor.h"

IMPLEMENT_DYNAMIC(cAreaLightDialog, CDialog)

BEGIN_MESSAGE_MAP(cAreaLightDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ALIGHT_SKY_AMBIENT, OnClickedSkyAmbientBox)
	//ON_BN_CLICKED(IDC_BUTTON_ALIGHT_SKY_DIFFUSE, OnClickedSkyDiffuseBox)
	ON_BN_CLICKED(IDC_BUTTON_ALIGHT_TERRAIN_AMBIENT, OnClickedTerrainAmbientBox)
	//ON_BN_CLICKED(IDC_BUTTON_ALIGHT_TERRAIN_DIFFUSE, OnClickedTerrainDiffuseBox)
	ON_BN_CLICKED(IDC_BUTTON_ALIGHT_STATICOBJ_AMBIENT, OnClickedStaticObjectAmbientBox)
	//ON_BN_CLICKED(IDC_BUTTON_ALIGHT_STATICOBJ_DIFFUSE, OnClickedStaticObjectDiffuseBox)
	ON_BN_CLICKED(IDC_BUTTON_ALIGHT_DYNAMICOBJ_AMBIENT, OnClickedDynamicObjectAmbientBox)
	//ON_BN_CLICKED(IDC_BUTTON_ALIGHT_DYNAMICOBJ_DIFFUSE, OnClickedDynamicObjectDiffuseBox)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

cAreaLightDialog::cAreaLightDialog()
{
}

cAreaLightDialog::~cAreaLightDialog()
{
}

void cAreaLightDialog::UpdateSky( const NiColor& ambient, float dimmer )
{
	unsigned char r = (unsigned char)(ambient.r * 255.0f);
	unsigned char g = (unsigned char)(ambient.g * 255.0f);
	unsigned char b = (unsigned char)(ambient.b * 255.0f);
	mSkyAmbientBox.SetColor( RGB(r,g,b) );

	//r = (unsigned char)(diffuse.r * 255.0f);
	//g = (unsigned char)(diffuse.g * 255.0f);
	//b = (unsigned char)(diffuse.b * 255.0f);
	//mSkyDiffuseBox.SetColor( RGB(r,g,b) );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_SKY_DIMMER );
	slider->SetPos( (int)(dimmer * 1000.0f) );

	cString str;
	str.Format( "%.3f", dimmer );
	SetDlgItemText( IDC_STATIC_ALIGHT_SKY_DIMMER, str.Cstr() );
}

void cAreaLightDialog::UpdateTerrain( const NiColor& ambient, float dimmer )
{
	unsigned char r = (unsigned char)(ambient.r * 255.0f);
	unsigned char g = (unsigned char)(ambient.g * 255.0f);
	unsigned char b = (unsigned char)(ambient.b * 255.0f);
	mTerrainAmbientBox.SetColor( RGB(r,g,b) );

	//r = (unsigned char)(diffuse.r * 255.0f);
	//g = (unsigned char)(diffuse.g * 255.0f);
	//b = (unsigned char)(diffuse.b * 255.0f);
	//mTerrainDiffuseBox.SetColor( RGB(r,g,b) );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_TERRAIN_DIMMER );
	slider->SetPos( (int)(dimmer * 1000.0f) );

	cString str;
	str.Format( "%.3f", dimmer );
	SetDlgItemText( IDC_STATIC_ALIGHT_TERRAIN_DIMMER, str.Cstr() );
}

void cAreaLightDialog::UpdateStaticObject( const NiColor& ambient, float dimmer )
{
	unsigned char r = (unsigned char)(ambient.r * 255.0f);
	unsigned char g = (unsigned char)(ambient.g * 255.0f);
	unsigned char b = (unsigned char)(ambient.b * 255.0f);
	mStaticObjectAmbientBox.SetColor( RGB(r,g,b) );

	//r = (unsigned char)(diffuse.r * 255.0f);
	//g = (unsigned char)(diffuse.g * 255.0f);
	//b = (unsigned char)(diffuse.b * 255.0f);
	//mStaticObjectDiffuseBox.SetColor( RGB(r,g,b) );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_STATICOBJ_DIMMER );
	slider->SetPos( (int)(dimmer * 1000.0f) );

	cString str;
	str.Format( "%.3f", dimmer );
	SetDlgItemText( IDC_STATIC_ALIGHT_STATICOBJ_DIMMER, str.Cstr() );
}

void cAreaLightDialog::UpdateDynamicObject( const NiColor& ambient, float dimmer )
{
	unsigned char r = (unsigned char)(ambient.r * 255.0f);
	unsigned char g = (unsigned char)(ambient.g * 255.0f);
	unsigned char b = (unsigned char)(ambient.b * 255.0f);
	mDynamicObjectAmbientBox.SetColor( RGB(r,g,b) );

	//r = (unsigned char)(diffuse.r * 255.0f);
	//g = (unsigned char)(diffuse.g * 255.0f);
	//b = (unsigned char)(diffuse.b * 255.0f);
	//mDynamicObjectDiffuseBox.SetColor( RGB(r,g,b) );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_DYNAMICOBJ_DIMMER );
	slider->SetPos( (int)(dimmer * 1000.0f) );

	cString str;
	str.Format( "%.3f", dimmer );
	SetDlgItemText( IDC_STATIC_ALIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );
}

void cAreaLightDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_ALIGHT_SKY_AMBIENT, mSkyAmbientBox );
	DDX_Control( dx, IDC_BUTTON_ALIGHT_TERRAIN_AMBIENT, mTerrainAmbientBox );
	DDX_Control( dx, IDC_BUTTON_ALIGHT_STATICOBJ_AMBIENT, mStaticObjectAmbientBox );
	DDX_Control( dx, IDC_BUTTON_ALIGHT_DYNAMICOBJ_AMBIENT, mDynamicObjectAmbientBox );
}

BOOL cAreaLightDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// ÇÏ´Ã ¹à±â
	mSkyAmbientBox.SetColor( 0 );
	//mSkyDiffuseBox.SetColor( 0 );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_SKY_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 1000 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_ALIGHT_SKY_DIMMER ), FALSE );

	cString str;
	str.Format( "%.3f", 1.0f );
	SetDlgItemText( IDC_STATIC_ALIGHT_SKY_DIMMER, str.Cstr() );

	/// ÁöÇü ¹à±â
	mTerrainAmbientBox.SetColor( RGB( 255, 255, 255 ) );
	//mTerrainDiffuseBox.SetColor( RGB( 255, 255, 255 ) );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_TERRAIN_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 1000 );

	str.Format( "%.3f", 1.0f );
	SetDlgItemText( IDC_STATIC_ALIGHT_TERRAIN_DIMMER, str.Cstr() );

	/// Á¤Àû ¿ÀºêÁ§Æ® ¹à±â
	mStaticObjectAmbientBox.SetColor( RGB( 255, 255, 255 ) );
	//mStaticObjectDiffuseBox.SetColor( RGB( 255, 255, 255 ) );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_STATICOBJ_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 1000 );

	str.Format( "%.3f", 1.0f );
	SetDlgItemText( IDC_STATIC_ALIGHT_STATICOBJ_DIMMER, str.Cstr() );

	/// µ¿Àû ¿ÀºêÁ§Æ® ¹à±â
	mDynamicObjectAmbientBox.SetColor( RGB( 255, 255, 255 ) );
	//mDynamicObjectDiffuseBox.SetColor( RGB( 255, 255, 255 ) );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ALIGHT_DYNAMICOBJ_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 1000 );

	str.Format( "%.3f", 1.0f );
	SetDlgItemText( IDC_STATIC_ALIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );

	return TRUE;
}

void cAreaLightDialog::OnClickedSkyAmbientBox()
{
	CColorDialog dlg( mSkyAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mSkyAmbientBox.SetColor( rgb );

		AREAEDIT->SetSkyAmbient( color );
		VIEW->Update();
	}
}

//void cAreaLightDialog::OnClickedSkyDiffuseBox()
//{
//	CColorDialog dlg( mSkyDiffuseBox.GetColor(), CC_FULLOPEN );
//
//	if( dlg.DoModal() == IDOK )
//	{
//		COLORREF rgb = dlg.GetColor();
//		NiColor color;
//		color.r = (float)GetRValue( rgb ) / 255.0f;
//		color.g = (float)GetGValue( rgb ) / 255.0f;
//		color.b = (float)GetBValue( rgb ) / 255.0f;
//
//		mSkyDiffuseBox.SetColor( rgb );
//
//		AREAEDIT->SetSkyDiffuse( color );
//		VIEW->Update();
//	}
//}

void cAreaLightDialog::OnClickedTerrainAmbientBox()
{
	CColorDialog dlg( mTerrainAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mTerrainAmbientBox.SetColor( rgb );

		AREAEDIT->SetTerrainAmbient( color );
		VIEW->Update();
	}
}

//void cAreaLightDialog::OnClickedTerrainDiffuseBox()
//{
//	CColorDialog dlg( mTerrainDiffuseBox.GetColor(), CC_FULLOPEN );
//
//	if( dlg.DoModal() == IDOK )
//	{
//		COLORREF rgb = dlg.GetColor();
//		NiColor color;
//		color.r = (float)GetRValue( rgb ) / 255.0f;
//		color.g = (float)GetGValue( rgb ) / 255.0f;
//		color.b = (float)GetBValue( rgb ) / 255.0f;
//
//		mTerrainDiffuseBox.SetColor( rgb );
//
//		AREAEDIT->SetTerrainDiffuse( color );
//		VIEW->Update();
//	}
//}

void cAreaLightDialog::OnClickedStaticObjectAmbientBox()
{
	CColorDialog dlg( mStaticObjectAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mStaticObjectAmbientBox.SetColor( rgb );

		AREAEDIT->SetStaticObjectAmbient( color );
		VIEW->Update();
	}
}

//void cAreaLightDialog::OnClickedStaticObjectDiffuseBox()
//{
//	CColorDialog dlg( mStaticObjectDiffuseBox.GetColor(), CC_FULLOPEN );
//
//	if( dlg.DoModal() == IDOK )
//	{
//		COLORREF rgb = dlg.GetColor();
//		NiColor color;
//		color.r = (float)GetRValue( rgb ) / 255.0f;
//		color.g = (float)GetGValue( rgb ) / 255.0f;
//		color.b = (float)GetBValue( rgb ) / 255.0f;
//
//		mStaticObjectDiffuseBox.SetColor( rgb );
//
//		AREAEDIT->SetStaticObjectDiffuse( color );
//		VIEW->Update();
//	}
//}

void cAreaLightDialog::OnClickedDynamicObjectAmbientBox()
{
	CColorDialog dlg( mDynamicObjectAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mDynamicObjectAmbientBox.SetColor( rgb );

		AREAEDIT->SetDynamicObjectAmbient( color );
		VIEW->Update();
	}
}

//void cAreaLightDialog::OnClickedDynamicObjectDiffuseBox()
//{
//	CColorDialog dlg( mDynamicObjectDiffuseBox.GetColor(), CC_FULLOPEN );
//
//	if( dlg.DoModal() == IDOK )
//	{
//		COLORREF rgb = dlg.GetColor();
//		NiColor color;
//		color.r = (float)GetRValue( rgb ) / 255.0f;
//		color.g = (float)GetGValue( rgb ) / 255.0f;
//		color.b = (float)GetBValue( rgb ) / 255.0f;
//
//		mDynamicObjectDiffuseBox.SetColor( rgb );
//
//		AREAEDIT->SetDynamicObjectDiffuse( color );
//		VIEW->Update();
//	}
//}

void cAreaLightDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_ALIGHT_SKY_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_ALIGHT_SKY_DIMMER, str.Cstr() );

			AREAEDIT->SetSkyDimmer( dimmer );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_ALIGHT_TERRAIN_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_ALIGHT_TERRAIN_DIMMER, str.Cstr() );

			AREAEDIT->SetTerrainDimmer( dimmer );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_ALIGHT_STATICOBJ_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_ALIGHT_STATICOBJ_DIMMER, str.Cstr() );

			AREAEDIT->SetStaticObjectDimmer( dimmer );
			VIEW->Update();
		}
		break;
	case IDC_SLIDER_ALIGHT_DYNAMICOBJ_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_ALIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );

			AREAEDIT->SetDynamicObjectDimmer( dimmer );
			VIEW->Update();
		}
		break;
	}
}
