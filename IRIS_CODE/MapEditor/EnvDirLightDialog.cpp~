#include "stdafx.h"
#include "EnvDirLightDialog.h"

#include "resource.h"
#include "MapEditorView.h"
#include "EnvEditor.h"
#include "Engine/CameraManager.h"

IMPLEMENT_DYNAMIC(cEnvDirLightDialog, CDialog)

BEGIN_MESSAGE_MAP(cEnvDirLightDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_STATICOBJ_AMBIENT, OnClickedStaticObjAmbientBox)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_STATICOBJ_DIFFUSE, OnClickedStaticObjDiffuseBox)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_STATICOBJ_ROTATE_FROM_CAM, OnClickedStaticObjRotateFromCamera)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_AMBIENT, OnClickedDynamicObjAmbientBox)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_DIFFUSE, OnClickedDynamicObjDiffuseBox)
	ON_BN_CLICKED(IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_ROTATE_FROM_CAM, OnClickedDynamicObjRotateFromCamera)
	ON_WM_HSCROLL()
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedTransform)
END_MESSAGE_MAP()

cEnvDirLightDialog::cEnvDirLightDialog()
{
}

cEnvDirLightDialog::~cEnvDirLightDialog()
{
}

void cEnvDirLightDialog::Update( NiDirectionalLight* light0, NiDirectionalLight* light1 )
{
	const NiColor& ambient0 = light0->GetAmbientColor();
	unsigned char r = (unsigned char)(ambient0.r * 255.0f);
	unsigned char g = (unsigned char)(ambient0.g * 255.0f);
	unsigned char b = (unsigned char)(ambient0.b * 255.0f);
	mStaticObjAmbientBox.SetColor( RGB(r,g,b) );

	const NiColor& diffuse0 = light0->GetDiffuseColor();
	r = (unsigned char)(diffuse0.r * 255.0f);
	g = (unsigned char)(diffuse0.g * 255.0f);
	b = (unsigned char)(diffuse0.b * 255.0f);
	mStaticObjDiffuseBox.SetColor( RGB(r,g,b) );

	float dimmer0 = light0->GetDimmer();
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_DIRLIGHT_STATICOBJ_DIMMER );
	slider->SetPos( (int)(dimmer0 * 1000.0f) );

	cString str;
	str.Format( "%.3f", dimmer0 );
	SetDlgItemText( IDC_STATIC_DIRLIGHT_STATICOBJ_DIMMER, str.Cstr() );

	const NiMatrix3& r0 = light0->GetWorldRotate();
	float xangle, yangle, zangle;
	r0.ToEulerAnglesXYZ( xangle, yangle, zangle );

	mStaticObjRxEdit.SetValue( D3DXToDegree( xangle ) );
	mStaticObjRyEdit.SetValue( D3DXToDegree( yangle ) );
	mStaticObjRzEdit.SetValue( D3DXToDegree( zangle ) );

	///
	const NiColor& ambient1 = light1->GetAmbientColor();
	r = (unsigned char)(ambient1.r * 255.0f);
	g = (unsigned char)(ambient1.g * 255.0f);
	b = (unsigned char)(ambient1.b * 255.0f);
	mDynamicObjAmbientBox.SetColor( RGB(r,g,b) );

	const NiColor& diffuse1 = light1->GetDiffuseColor();
	r = (unsigned char)(diffuse1.r * 255.0f);
	g = (unsigned char)(diffuse1.g * 255.0f);
	b = (unsigned char)(diffuse1.b * 255.0f);
	mDynamicObjDiffuseBox.SetColor( RGB(r,g,b) );

	float dimmer1 = light1->GetDimmer();
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_DIRLIGHT_DYNAMICOBJ_DIMMER );
	slider->SetPos( (int)(dimmer1 * 1000.0f) );

	str.Format( "%.3f", dimmer1 );
	SetDlgItemText( IDC_STATIC_DIRLIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );

	const NiMatrix3& r1 = light1->GetWorldRotate();
	r1.ToEulerAnglesXYZ( xangle, yangle, zangle );

	mDynamicObjRxEdit.SetValue( D3DXToDegree( xangle ) );
	mDynamicObjRyEdit.SetValue( D3DXToDegree( yangle ) );
	mDynamicObjRzEdit.SetValue( D3DXToDegree( zangle ) );
}

void cEnvDirLightDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_STATICOBJ_AMBIENT, mStaticObjAmbientBox );
	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_STATICOBJ_DIFFUSE, mStaticObjDiffuseBox );
	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_STATICOBJ_ROTATE_FROM_CAM, mStaticObjRotFromCameraButton );

	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_AMBIENT, mDynamicObjAmbientBox );
	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_DIFFUSE, mDynamicObjDiffuseBox );
	DDX_Control( dx, IDC_BUTTON_DIRLIGHT_DYNAMICOBJ_ROTATE_FROM_CAM, mDynamicObjRotFromCameraButton );
}

BOOL cEnvDirLightDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 정적 오브젝트
	mStaticObjAmbientBox.SetColor( RGB( 255, 255, 255 ) );
	mStaticObjDiffuseBox.SetColor( RGB( 255, 255, 255 ) );

	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_DIRLIGHT_STATICOBJ_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 500 );

	cString str;
	str.Format( "%.1f", 0.5f );
	SetDlgItemText( IDC_STATIC_DIRLIGHT_STATICOBJ_DIMMER, str.Cstr() );


	mStaticObjRxEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_STATICOBJ_RX );
	mStaticObjRyEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_STATICOBJ_RY );
	mStaticObjRzEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_STATICOBJ_RZ );

	/// 동적 오브젝트
	mDynamicObjAmbientBox.SetColor( RGB( 255, 255, 255 ) );
	mDynamicObjDiffuseBox.SetColor( RGB( 255, 255, 255 ) );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_DIRLIGHT_DYNAMICOBJ_DIMMER );
	slider->SetRange( 0, 1000 );
	slider->SetPos( 500 );

	str.Format( "%.1f", 0.5f );
	SetDlgItemText( IDC_STATIC_DIRLIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );

	mDynamicObjRxEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RX );
	mDynamicObjRyEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RY );
	mDynamicObjRzEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RZ );
	return TRUE;
}

void cEnvDirLightDialog::OnClickedStaticObjAmbientBox()
{
	CColorDialog dlg( mStaticObjAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mStaticObjAmbientBox.SetColor( rgb );

		ENVEDIT->SetDirLightAmbient( 0, color );
		VIEW->Update();
	}
}

void cEnvDirLightDialog::OnClickedStaticObjDiffuseBox()
{
	CColorDialog dlg( mStaticObjDiffuseBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mStaticObjDiffuseBox.SetColor( rgb );

		ENVEDIT->SetDirLightDiffuse( 0, color );
		VIEW->Update();
	}
}

void cEnvDirLightDialog::OnClickedStaticObjRotateFromCamera()
{
	cCamera* cam = CAMERAMAN->GetCurrent();

	const NiMatrix3& r = cam->GetWorldRotate();
	float xangle, yangle, zangle;
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );
	
	ENVEDIT->SetDirLightRotate( 0, xangle, yangle, zangle );

	mStaticObjRxEdit.SetValue( D3DXToDegree( xangle ) );
	mStaticObjRyEdit.SetValue( D3DXToDegree( yangle ) );
	mStaticObjRzEdit.SetValue( D3DXToDegree( zangle ) );
}

void cEnvDirLightDialog::OnClickedDynamicObjAmbientBox()
{
	CColorDialog dlg( mDynamicObjAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mDynamicObjAmbientBox.SetColor( rgb );

		ENVEDIT->SetDirLightAmbient( 1, color );
		VIEW->Update();
	}
}

void cEnvDirLightDialog::OnClickedDynamicObjDiffuseBox()
{
	CColorDialog dlg( mDynamicObjDiffuseBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		COLORREF rgb = dlg.GetColor();
		NiColor color;
		color.r = (float)GetRValue( rgb ) / 255.0f;
		color.g = (float)GetGValue( rgb ) / 255.0f;
		color.b = (float)GetBValue( rgb ) / 255.0f;

		mDynamicObjDiffuseBox.SetColor( rgb );

		ENVEDIT->SetDirLightDiffuse( 1, color );
		VIEW->Update();
	}
}

void cEnvDirLightDialog::OnClickedDynamicObjRotateFromCamera()
{
	cCamera* cam = CAMERAMAN->GetCurrent();

	const NiMatrix3& r = cam->GetWorldRotate();
	float xangle, yangle, zangle;
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );

	mDynamicObjRxEdit.SetValue( D3DXToDegree( xangle ) );
	mDynamicObjRyEdit.SetValue( D3DXToDegree( yangle ) );
	mDynamicObjRzEdit.SetValue( D3DXToDegree( zangle ) );

	ENVEDIT->SetDirLightRotate( 1, xangle, yangle, zangle );
}

void cEnvDirLightDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_DIRLIGHT_STATICOBJ_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			ENVEDIT->SetDirLightDimmer( 0, dimmer );

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_DIRLIGHT_STATICOBJ_DIMMER, str.Cstr() );

			VIEW->Update();
		}
		break;
	case IDC_SLIDER_DIRLIGHT_DYNAMICOBJ_DIMMER:
		{
			int pos = slider->GetPos();
			float dimmer = (float)pos / 1000.0f;

			cString str;
			str.Format( "%.3f", dimmer );
			SetDlgItemText( IDC_STATIC_DIRLIGHT_DYNAMICOBJ_DIMMER, str.Cstr() );

			ENVEDIT->SetDirLightDimmer( 1, dimmer );
			VIEW->Update();
		}
		break;
	}
}

LRESULT cEnvDirLightDialog::OnChangedTransform( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_DIRLIGHT_STATICOBJ_RX:
	case IDC_EDIT_DIRLIGHT_STATICOBJ_RY:
	case IDC_EDIT_DIRLIGHT_STATICOBJ_RZ:
		{
			float xangle = D3DXToRadian( mStaticObjRxEdit.GetValue() );
			float yangle = D3DXToRadian( mStaticObjRyEdit.GetValue() );
			float zangle = D3DXToRadian( mStaticObjRzEdit.GetValue() );
			ENVEDIT->SetDirLightRotate( 0, xangle, yangle, zangle );
		}
		break;
	case IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RX:
	case IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RY:
	case IDC_EDIT_DIRLIGHT_DYNAMICOBJ_RZ:
		{
			float xangle = D3DXToRadian( mDynamicObjRxEdit.GetValue() );
			float yangle = D3DXToRadian( mDynamicObjRyEdit.GetValue() );
			float zangle = D3DXToRadian( mDynamicObjRzEdit.GetValue() );
			ENVEDIT->SetDirLightRotate( 1, xangle, yangle, zangle );
		}
		break;
	}
	return 0;
}
