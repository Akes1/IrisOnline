#include "stdafx.h"
#include "LightPropertyDialog.h"

#include "LightEditor.h"

IMPLEMENT_DYNAMIC(cLightPropertyDialog, CDialog)

BEGIN_MESSAGE_MAP(cLightPropertyDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_LPROP_AMBIENT, OnClickedAmbientBox)
	ON_BN_CLICKED(IDC_BUTTON_LPROP_DIFFUSE0, OnClickedDiffuse0Box)
	ON_BN_CLICKED(IDC_BUTTON_LPROP_DIFFUSE1, OnClickedDiffuse1Box)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedAttenuation)
END_MESSAGE_MAP()

cLightPropertyDialog::cLightPropertyDialog()
{
}

cLightPropertyDialog::~cLightPropertyDialog()
{
}

void cLightPropertyDialog::UpdateLight( const NiColor& ambient,
									   const NiColor& diffuse0, const NiColor& diffuse1, float diffuseAnimTime,
									   float constant0, float constant1, float constantAttenAnimTime,
									   float linear, float quadric )
{
	/// ÁÖº¯±¤
	unsigned char r = (unsigned char)(ambient.r * 255.0f);
	unsigned char g = (unsigned char)(ambient.g * 255.0f);
	unsigned char b = (unsigned char)(ambient.b * 255.0f);
	mAmbientBox.SetColor( RGB(r,g,b) );

	/// »ê¶õ±¤
	r = (unsigned char)(diffuse0.r * 255.0f);
	g = (unsigned char)(diffuse0.g * 255.0f);
	b = (unsigned char)(diffuse0.b * 255.0f);
	mDiffuse0Box.SetColor( RGB(r,g,b) );

	r = (unsigned char)(diffuse1.r * 255.0f);
	g = (unsigned char)(diffuse1.g * 255.0f);
	b = (unsigned char)(diffuse1.b * 255.0f);
	mDiffuse1Box.SetColor( RGB(r,g,b) );

	mDiffuseAnimTimeEdit.SetValue( diffuseAnimTime );

	/// °¨¼è °è¼ö
	mConstant0Edit.SetValue( 10.0f - constant0 );
	mConstant1Edit.SetValue( 10.0f - constant1 );
	mConstantAttenAnimTimeEdit.SetValue( constantAttenAnimTime );

	mLinearEdit.SetValue( 1.0f - linear );
	mQuadricEdit.SetValue( 1.0f - quadric );

	SetEnabled( true );
}

void cLightPropertyDialog::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_LPROP_AMBIENT, mAmbientBox );
	DDX_Control( dx, IDC_BUTTON_LPROP_DIFFUSE0, mDiffuse0Box );
	DDX_Control( dx, IDC_BUTTON_LPROP_DIFFUSE1, mDiffuse1Box );
}

BOOL cLightPropertyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// »ê¶õ±¤
	mDiffuseAnimTimeEdit.Create( 0.0f, 10.0f, 0.0f, 0.1f, this, IDC_EDIT_LPROP_DIFFUSE_TIME );

	/// °¨¼è °è¼ö
	mConstant0Edit.Create( 0.0f, 10.0f, 10.0f, 0.1f, this, IDC_EDIT_LPROP_CONSTANT_ATTEN0 );
	mConstant1Edit.Create( 0.0f, 10.0f, 10.0f, 0.1f, this, IDC_EDIT_LPROP_CONSTANT_ATTEN1 );
	mConstantAttenAnimTimeEdit.Create( 0.0f, 10.0f, 0.0f, 0.1f, this, IDC_EDIT_LPROP_ATTEN_TIME );
	mLinearEdit.Create( 0.0f, 1.0f, 1.0f, 0.00001f, this, IDC_EDIT_LPROP_LINEAR_ATTEN );
	mQuadricEdit.Create( 0.0f, 1.0f, 1.0f, 0.0000001f, this, IDC_EDIT_LPROP_QUADRIC_ATTEN );
	SetEnabled( false );
	return TRUE;
}

void cLightPropertyDialog::OnClickedAmbientBox()
{
	CColorDialog dlg( mAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mAmbientBox.SetColor( dlg.GetColor() );
		LIGHTEDIT->SetLightAmbient( dlg.GetColor() );
	}
}

void cLightPropertyDialog::OnClickedDiffuse0Box()
{
	CColorDialog dlg( mDiffuse0Box.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mDiffuse0Box.SetColor( dlg.GetColor() );
		LIGHTEDIT->SetLightDiffuse0( dlg.GetColor() );

		if( mDiffuse0Box.GetColor() == mDiffuse1Box.GetColor() )
		{
			mDiffuseAnimTimeEdit.SetValue( 0.0f );
			mDiffuseAnimTimeEdit.SetEnabled( false );
			LIGHTEDIT->SetLightDiffuseAnimTime( 0.0f );
		}
		else
		{
			mDiffuseAnimTimeEdit.SetEnabled( true );
		}
	}
}

void cLightPropertyDialog::OnClickedDiffuse1Box()
{
	CColorDialog dlg( mDiffuse1Box.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mDiffuse1Box.SetColor( dlg.GetColor() );
		LIGHTEDIT->SetLightDiffuse1( dlg.GetColor() );

		if( mDiffuse0Box.GetColor() == mDiffuse1Box.GetColor() )
		{
			mDiffuseAnimTimeEdit.SetValue( 0.0f );
			mDiffuseAnimTimeEdit.SetEnabled( false );
			LIGHTEDIT->SetLightDiffuseAnimTime( 0.0f );
		}
		else
		{
			mDiffuseAnimTimeEdit.SetEnabled( true );
		}
	}
}

LRESULT cLightPropertyDialog::OnChangedAttenuation( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_LPROP_DIFFUSE_TIME:
		{
			float t = mDiffuseAnimTimeEdit.GetValue();
			LIGHTEDIT->SetLightDiffuseAnimTime( t );
		}
		break;
	case IDC_EDIT_LPROP_CONSTANT_ATTEN0:
		{
			float a = mConstant0Edit.GetValue();
			LIGHTEDIT->SetLightConstantAtten0( 10.0f - a );

			if( a == mConstant1Edit.GetValue() )
			{
				mConstantAttenAnimTimeEdit.SetValue( 0.0f );
				mConstantAttenAnimTimeEdit.SetEnabled( false );
				LIGHTEDIT->SetLightConstantAttenAnimTime( 0.0f );
			}
			else
			{
				mConstantAttenAnimTimeEdit.SetEnabled( true );
			}
		}
		break;
	case IDC_EDIT_LPROP_CONSTANT_ATTEN1:
		{
			float a = mConstant1Edit.GetValue();
			LIGHTEDIT->SetLightConstantAtten1( 10.0f - a );

			if( a == mConstant0Edit.GetValue() )
			{
				mConstantAttenAnimTimeEdit.SetValue( 0.0f );
				mConstantAttenAnimTimeEdit.SetEnabled( false );
				LIGHTEDIT->SetLightConstantAttenAnimTime( 0.0f );
			}
			else
			{
				mConstantAttenAnimTimeEdit.SetEnabled( true );
			}
		}
		break;
	case IDC_EDIT_LPROP_ATTEN_TIME:
		{
			float t = mConstantAttenAnimTimeEdit.GetValue();
			LIGHTEDIT->SetLightConstantAttenAnimTime( t );
		}
		break;
	case IDC_EDIT_LPROP_LINEAR_ATTEN:
		{
			float a = mLinearEdit.GetValue();
			LIGHTEDIT->SetLightLinearAtten( 1.0f - a );
		}
		break;
	case IDC_EDIT_LPROP_QUADRIC_ATTEN:
		{
			float a = mQuadricEdit.GetValue();
			LIGHTEDIT->SetLightQuadricAtten( 1.0f - a );
		}
		break;
	}
	return 0;
}
