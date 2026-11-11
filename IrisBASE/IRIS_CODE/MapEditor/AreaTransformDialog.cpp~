#include "stdafx.h"
#include "AreaTransformDialog.h"

#include "Resource.h"
#include "MapEditorView.h"
#include "AreaEditor.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cAreaTransformDialog, cDialog)

BEGIN_MESSAGE_MAP(cAreaTransformDialog, cDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_LFORM_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_PICK, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_TO_TERRAIN, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_LFORM_TO_OBJECT, OnClickedButton)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedTransform)
END_MESSAGE_MAP()

cAreaTransformDialog::cAreaTransformDialog()
{
}

cAreaTransformDialog::~cAreaTransformDialog()
{
}

void cAreaTransformDialog::SetCheckedButton( int id )
{
	mButtonGroup.SetCheckedButton( id );
}

void cAreaTransformDialog::UpdateArea( const NiPoint3& translate, float radius )
{
	SetEnabled( true );

	/// 이동
	mTxEdit.SetValue( translate.x / 100.0f );
	mTyEdit.SetValue( translate.y / 100.0f );
	mTzEdit.SetValue( translate.z / 100.0f );

	/// 배율
	mRadiusEdit.SetValue( radius / 100.0f );
}

void cAreaTransformDialog::DoDataExchange(CDataExchange* dx)
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_LFORM_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_PICK, mPickButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_TO_TERRAIN, mAttachToTerrainButton );
	DDX_Control( dx, IDC_BUTTON_LFORM_TO_OBJECT, mAttachToObjectButton );
}

BOOL cAreaTransformDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mPickButton.SetGroup( &mButtonGroup );
	mAttachToTerrainButton.SetGroup( &mButtonGroup );
	mAttachToObjectButton.SetGroup( &mButtonGroup );

	mTxEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TX );
	mTyEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TY );
	mTzEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_LFORM_TZ );
	mRadiusEdit.Create( 6.0f, 200.0f, 1.0f, 0.1f, this, IDC_EDIT_LFORM_RADIUS );

	SetEnabled( false );
	return TRUE;
}

void cAreaTransformDialog::OnClickedButton()
{
	VIEW->SetEditMode( eEDIT_AREA_TRANSFORM );
}

void cAreaTransformDialog::OnClickedCreate()
{
	VIEW->SetEditMode( eEDIT_AREA_TRANSFORM );
	AREAEDIT->CreateArea();
}

unsigned int cAreaTransformDialog::GetCheckedButton() const
{
	cCheckButton* btn = mButtonGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

LRESULT cAreaTransformDialog::OnChangedTransform( WPARAM id, LPARAM )
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
			AREAEDIT->SetAreaTranslate( NiPoint3(x, y, z), false );
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
			AREAEDIT->SetAreaRadius( r, false );
		}
		break;
	}
	return 0;
}
