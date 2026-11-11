#include "stdafx.h"
#include "SoundTransformDialog.h"

#include "Resource.h"
#include "MapEditorView.h"
#include "SoundEditor.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cSoundTransformDialog, cDialog)

BEGIN_MESSAGE_MAP(cSoundTransformDialog, cDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SFORM_PICK, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_SFORM_TO_TERRAIN, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_SFORM_TO_OBJECT, OnClickedButton)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedTransform)
END_MESSAGE_MAP()

cSoundTransformDialog::cSoundTransformDialog()
{
}

cSoundTransformDialog::~cSoundTransformDialog()
{
}

void cSoundTransformDialog::SetCheckedButton( int id )
{
	mButtonGroup.SetCheckedButton( id );
}

void cSoundTransformDialog::UpdateSound( const NiPoint3& translate, float radius, float attenDist )
{
	SetEnabled( true );

	/// 이동
	mTxEdit.SetValue( translate.x / 100.0f );
	mTyEdit.SetValue( translate.y / 100.0f );
	mTzEdit.SetValue( translate.z / 100.0f );

	/// 감쇠 거리
	mRadiusEdit.SetValue( radius / 100.0f );
	mAttenDistEdit.SetValue( attenDist / 100.0f );
}

void cSoundTransformDialog::DoDataExchange( CDataExchange* dx )
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_SFORM_PICK, mPickButton );
	DDX_Control( dx, IDC_BUTTON_SFORM_TO_TERRAIN, mAttachToTerrainButton );
	DDX_Control( dx, IDC_BUTTON_SFORM_TO_OBJECT, mAttachToObjectButton );
}

BOOL cSoundTransformDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mPickButton.SetGroup( &mButtonGroup );
	mAttachToTerrainButton.SetGroup( &mButtonGroup );
	mAttachToObjectButton.SetGroup( &mButtonGroup );

	mTxEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_SFORM_TX );
	mTyEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_SFORM_TY );
	mTzEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_SFORM_TZ );
	mRadiusEdit.Create( 5.0f, 50.0f, 10.0f, 0.1f, this, IDC_EDIT_SFORM_RADIUS );
	mAttenDistEdit.Create( 1.0f, 50.0f, 1.0f, 0.1f, this, IDC_EDIT_SFORM_ATTEN_DIST );

	SetEnabled( false );
	return TRUE;
}

void cSoundTransformDialog::OnClickedButton()
{
	VIEW->SetEditMode( eEDIT_SOUND_TRANSFORM );
}

unsigned int cSoundTransformDialog::GetCheckedButton() const
{
	cCheckButton* btn = mButtonGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

LRESULT cSoundTransformDialog::OnChangedTransform( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_SFORM_TX:
	case IDC_EDIT_SFORM_TY:
	case IDC_EDIT_SFORM_TZ:
		{
			float x = mTxEdit.GetValue() * 100.0f;
			float y = mTyEdit.GetValue() * 100.0f;
			float z = mTzEdit.GetValue() * 100.0f;
			SOUNDEDIT->SetSoundTranslate( NiPoint3(x, y, z), false );
		}
		break;
	case IDC_EDIT_SFORM_RADIUS:
		{
			float a = mAttenDistEdit.GetValue() * 100.0f;
			float r = mRadiusEdit.GetValue() * 100.0f;
			float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

			if( r > maxr )
			{
				r = maxr;
				mRadiusEdit.SetValue( r / 100.0f );
			}
			else if( r < a + 100.0f )
			{
				r = a + 100.0f;
				mRadiusEdit.SetValue( r / 100.0f );
			}
			SOUNDEDIT->SetSoundRadius( r, false );
		}
		break;
	case IDC_EDIT_SFORM_ATTEN_DIST:
		{
			float a = mAttenDistEdit.GetValue() * 100.0f;
			float r = mRadiusEdit.GetValue() * 100.0f;

			if( a > r - 100.0f )
			{
				a = r - 100.0f;
				mAttenDistEdit.SetValue( a / 100.0f );
			}
			SOUNDEDIT->SetSoundAttenDistance( a, false );
		}
		break;
	}
	return 0;
}
