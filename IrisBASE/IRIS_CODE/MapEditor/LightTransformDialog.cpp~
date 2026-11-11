#include "stdafx.h"
#include "LightTransformDialog.h"

#include "Resource.h"
#include "MapEditorView.h"
#include "LightEditor.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cLightTransformDialog, cDialog)

BEGIN_MESSAGE_MAP(cLightTransformDialog, cDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_LFORM_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_PICK, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_TO_TERRAIN, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_TO_OBJECT, OnClickedButton)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedTransform)
END_MESSAGE_MAP()

cLightTransformDialog::cLightTransformDialog()
{
}

cLightTransformDialog::~cLightTransformDialog()
{
}

void cLightTransformDialog::SetCheckedButton( int id )
{
	mButtonGroup.SetCheckedButton( id );
}

void cLightTransformDialog::UpdateLight( const NiPoint3& translate, float radius )
{
	SetEnabled( true );

	/// 이동
	mTxEdit.SetValue( translate.x / 100.0f );
	mTyEdit.SetValue( translate.y / 100.0f );
	mTzEdit.SetValue( translate.z / 100.0f );

	/// 배율
	mRadiusEdit.SetValue( radius / 100.0f );
}

void cLightTransformDialog::DoDataExchange(CDataExchange* dx)
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_LFORM_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_PICK, mPickButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_TO_TERRAIN, mAttachToTerrainButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_TO_OBJECT, mAttachToObjectButton );
}

BOOL cLightTransformDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mPickButton.SetGroup( &mButtonGroup );
	mAttachToTerrainButton.SetGroup( &mButtonGroup );
	mAttachToObjectButton.SetGroup( &mButtonGroup );

	mTxEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TX );
	mTyEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TY );
	mTzEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TZ );
	mRadiusEdit.Create( 1.0f, 60.0f, 3.0f, 0.1f, this, IDC_EDIT_LFORM_RADIUS );

	SetEnabled( false );
	return TRUE;
}

void cLightTransformDialog::OnClickedButton()
{
	VIEW->SetEditMode( eEDIT_LIGHT_TRANSFORM );
}

void cLightTransformDialog::OnClickedCreate()
{
	VIEW->SetEditMode( eEDIT_LIGHT_TRANSFORM );
	LIGHTEDIT->CreateLight();
}

unsigned int cLightTransformDialog::GetCheckedButton() const
{
	cCheckButton* btn = mButtonGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

LRESULT cLightTransformDialog::OnChangedTransform( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_LFORM_TX:
	case IDC_EDIT_LFORM_TY:
	case IDC_EDIT_LFORM_TZ:
		{
			float x = mTxEdit.GetValue() * 100.0f;
			float y = mTyEdit.GetValue() * 100.0f;
			float z = mTzEdit.GetValue() * 100.0f;
			LIGHTEDIT->SetLightTranslate( NiPoint3(x, y, z), false );
		}
		break;
	case IDC_EDIT_LFORM_RADIUS:
		{
			float r = mRadiusEdit.GetValue() * 100.0f;
			float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

			if( r > maxr )
			{
				r = maxr;
				mRadiusEdit.SetValue( maxr / 100.0f );
			}
			LIGHTEDIT->SetLightRadius( r, false );
		}
		break;
	}
	return 0;
}
