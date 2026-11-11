#include "stdafx.h"
#include "ObjectTransformDialog.h"

#include "Resource.h"
#include "MapEditorView.h"
#include "ObjectEditor.h"

IMPLEMENT_DYNAMIC(cObjectTransformDialog, cDialog)

BEGIN_MESSAGE_MAP(cObjectTransformDialog, cDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_OFORM_PICK, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_OFORM_TO_TERRAIN, OnClickedButton)
	ON_BN_CLICKED(IDC_BUTTON_OFORM_TO_OBJECT, OnClickedButton)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedTransform)
END_MESSAGE_MAP()

cObjectTransformDialog::cObjectTransformDialog()
{
}

cObjectTransformDialog::~cObjectTransformDialog()
{
}

void cObjectTransformDialog::SetCheckedButton( int id )
{
	mButtonGroup.SetCheckedButton( id );
}

void cObjectTransformDialog::UpdateObject( cObjectGroup* obj )
{
	assert( obj && "null oject" );

	if( obj->IsEmpty() )
	{
		SetEnabled( false );
	}
	else
	{
		SetEnabled( true );

		/// 이동
		const NiPoint3& t = obj->GetTranslate();
		mTxEdit.SetValue( t.x / 100.0f );
		mTyEdit.SetValue( t.y / 100.0f );
		mTzEdit.SetValue( t.z / 100.0f );

		/// 회전
		const NiMatrix3& r = obj->GetRotate();
		float xangle, yangle, zangle;
		r.ToEulerAnglesXYZ( xangle, yangle, zangle );
		mRxEdit.SetValue( D3DXToDegree( xangle ) );
		mRyEdit.SetValue( D3DXToDegree( yangle ) );
		mRzEdit.SetValue( D3DXToDegree( zangle ) );

		/// 배율
		mSxEdit.SetValue( obj->GetScale() );
	}
}

void cObjectTransformDialog::DoDataExchange(CDataExchange* dx)
{
	cDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_OFORM_PICK, mPickButton );
	DDX_Control( dx, IDC_BUTTON_OFORM_TO_TERRAIN, mAttachToTerrainButton );
	DDX_Control( dx, IDC_BUTTON_OFORM_TO_OBJECT, mAttachToObjectButton );
}

BOOL cObjectTransformDialog::OnInitDialog()
{
	cDialog::OnInitDialog();

	mPickButton.SetGroup( &mButtonGroup );
	mAttachToTerrainButton.SetGroup( &mButtonGroup );
	mAttachToObjectButton.SetGroup( &mButtonGroup );

	mTxEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_OFORM_TX );
	mTyEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_OFORM_TY );
	mTzEdit.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_EDIT_OFORM_TZ );
	mRxEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_OFORM_RX );
	mRyEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_OFORM_RY );
	mRzEdit.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_EDIT_OFORM_RZ );
	mSxEdit.Create( 0.1f, 10.0f, 1.0f, 0.1f, this, IDC_EDIT_OFORM_SX );

	mTxEdit.SetReadOnly( false );
	mTyEdit.SetReadOnly( false );
	mTzEdit.SetReadOnly( false );


	SetEnabled( false );
	return TRUE;
}

void cObjectTransformDialog::OnClickedButton()
{
	VIEW->SetEditMode( eEDIT_OBJECT_TRANSFORM );
}

unsigned int cObjectTransformDialog::GetCheckedButton() const
{
	cCheckButton* btn = mButtonGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

LRESULT cObjectTransformDialog::OnChangedTransform( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_OFORM_TX:
	case IDC_EDIT_OFORM_TY:
	case IDC_EDIT_OFORM_TZ:
		{
			float x = mTxEdit.GetValue() * 100;
			float y = mTyEdit.GetValue() * 100;
			float z = mTzEdit.GetValue() * 100;
			OBJECTEDIT->SetGroupTranslate( NiPoint3(x, y, z), false );
		}
		break;
	case IDC_EDIT_OFORM_RX:
	case IDC_EDIT_OFORM_RY:
	case IDC_EDIT_OFORM_RZ:
		{
			float xangle = D3DXToRadian( mRxEdit.GetValue() );
			float yangle = D3DXToRadian( mRyEdit.GetValue() );
			float zangle = D3DXToRadian( mRzEdit.GetValue() );
			OBJECTEDIT->SetGroupRotate( xangle, yangle, zangle, false );
		}
		break;
	case IDC_EDIT_OFORM_SX:
		{
			float s = mSxEdit.GetValue();
			OBJECTEDIT->SetGroupScale( s, false );
		}
		break;
	}
	return 0;
}
