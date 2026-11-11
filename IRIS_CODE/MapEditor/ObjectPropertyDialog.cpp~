#include "stdafx.h"
#include "ObjectPropertyDialog.h"

#include "Engine/ResourceManager.h"

#include "ObjectEditor.h"

IMPLEMENT_DYNAMIC(cObjectPropertyDialog, CDialog)

BEGIN_MESSAGE_MAP(cObjectPropertyDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIO_OPROP_VISIBLE_LEVEL0, OnCheckedVisibleLevel0)
	ON_BN_CLICKED(IDC_RADIO_OPROP_VISIBLE_LEVEL1, OnCheckedVisibleLevel1)
	ON_BN_CLICKED(IDC_RADIO_OPROP_VISIBLE_LEVEL2, OnCheckedVisibleLevel2)
	ON_BN_CLICKED(IDC_RADIO_OPROP_VISIBLE_LEVEL3, OnCheckedVisibleLevel3)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_VISIBLE_LEVEL_SIBLING, OnClickedVisibleLevelSibling)
	ON_BN_CLICKED(IDC_RADIO_OPROP_OCCLUDE_LEVEL0, OnCheckedOccludeLevel0)
	ON_BN_CLICKED(IDC_RADIO_OPROP_OCCLUDE_LEVEL1, OnCheckedOccludeLevel1)
	ON_BN_CLICKED(IDC_RADIO_OPROP_OCCLUDE_LEVEL2, OnCheckedOccludeLevel2)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_OCCLUDE_LEVEL_SIBLING, OnClickedOccludeLevelSibling)
	ON_BN_CLICKED(IDC_CHECK_OPROP_FOG_APPLIED, OnCheckedFogApplied)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_FOG_APPLIED_SIBLING, OnClickedFogAppliedSibling)
	ON_BN_CLICKED(IDC_CHECK_OPROP_MATERIAL_APPLIED, OnCheckedMaterialApplied)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_MATERIAL_APPLIED_SIBLING, OnClickedMaterialAppliedSibling)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_AMBIENT, OnClickedAmbient)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_DIFFUSE, OnClickedDiffuse)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_EMISSIVE, OnClickedEmissive)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_AMBIENT_SIBLING, OnClickedAmbientSibling)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_DIFFUSE_SIBLING, OnClickedDiffuseSibling)
	ON_BN_CLICKED(IDC_BUTTON_OPROP_EMISSIVE_SIBLING, OnClickedEmissiveSibling)

END_MESSAGE_MAP()

cObjectPropertyDialog::cObjectPropertyDialog()
{
}

cObjectPropertyDialog::~cObjectPropertyDialog()
{
}

void cObjectPropertyDialog::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_OPROP_AMBIENT, mAmbientBox );
	DDX_Control( dx, IDC_BUTTON_OPROP_DIFFUSE, mDiffuseBox );
	DDX_Control( dx, IDC_BUTTON_OPROP_EMISSIVE, mEmissiveBox );

	DDX_Control( dx, IDC_BUTTON_OPROP_VISIBLE_LEVEL_SIBLING, mVisibleLevelSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_OCCLUDE_LEVEL_SIBLING, mOccludeLevelSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_FOG_APPLIED_SIBLING, mFogAppliedSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_MATERIAL_APPLIED_SIBLING, mMaterialAppliedSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_AMBIENT_SIBLING, mAmbientSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_DIFFUSE_SIBLING, mDiffuseSibling );
	DDX_Control( dx, IDC_BUTTON_OPROP_EMISSIVE_SIBLING, mEmissiveSibling );
}

BOOL cObjectPropertyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetEnabled( false );
	return TRUE;
}

void cObjectPropertyDialog::UpdateObject( cObjectGroup* obj )
{
	assert( obj && "null oject" );

	if( obj->GetSize() > 1 )
	{
		SetEnabled( true );

		/// 파일 이름을 설정
		GetDlgItem(IDC_EDIT_OPROP_NAME)->SetWindowText( obj->GetFileName() );

		/// 가시 레벨을 설정
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL1))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL2))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL3))->SetCheck( 0 );

		/// 가시 레벨을 설정
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL1))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL2))->SetCheck( 0 );

		/// 안개 적용 여부를 설정
		((CButton*)GetDlgItem(IDC_CHECK_OPROP_FOG_APPLIED))->SetCheck( 1 );

		/// 재질을 설정
		((CButton*)GetDlgItem(IDC_CHECK_OPROP_MATERIAL_APPLIED))->SetCheck( 0 );

		mAmbientBox.SetColor( obj->GetAmbient() );
		mDiffuseBox.SetColor( obj->GetDiffuse() );
		mEmissiveBox.SetColor( obj->GetEmissive() );
	}
	else if( obj->GetSize() == 1 )
	{
		SetEnabled( true );

		/// 파일 이름을 설정
		GetDlgItem(IDC_EDIT_OPROP_NAME)->SetWindowText( obj->GetFileName() );

		/// 가시 레벨을 설정
		unsigned int visibleLevel = obj->GetVisibleLevel();
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL1))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL2))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL3))->SetCheck( 0 );

		switch( visibleLevel )
		{
		case 0:
		case 1:
		case 2:
		case 3:
			((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0 + visibleLevel))->SetCheck( 1 );
			break;
		default:
			((CButton*)GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0))->SetCheck( 1 );
			break;
		}

		/// 가시 레벨을 설정
		unsigned int occludeLevel = obj->GetOccludeLevel();
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL1))->SetCheck( 0 );
		((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL2))->SetCheck( 0 );

		switch( occludeLevel )
		{
		case 0:
		case 1:
		case 2:
			((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0 + occludeLevel))->SetCheck( 1 );
			break;
		default:
			((CButton*)GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0))->SetCheck( 1 );
			break;
		}

		/// 안개 적용 여부를 설정
		((CButton*)GetDlgItem(IDC_CHECK_OPROP_FOG_APPLIED))->SetCheck( obj->GetFogApplied() ? 1 : 0 );

		/// 재질을 설정
		((CButton*)GetDlgItem(IDC_CHECK_OPROP_MATERIAL_APPLIED))->SetCheck( obj->GetMaterialApplied() ? 1 : 0 );

		mAmbientBox.SetColor( obj->GetAmbient() );
		mDiffuseBox.SetColor( obj->GetDiffuse() );
		mEmissiveBox.SetColor( obj->GetEmissive() );
	}
	else
	{
		SetEnabled( false );
	}
}

void cObjectPropertyDialog::OnCheckedVisibleLevel0()
{
	OBJECTEDIT->SetGroupVisibleLevel( 0 );
}

void cObjectPropertyDialog::OnCheckedVisibleLevel1()
{
	OBJECTEDIT->SetGroupVisibleLevel( 1 );
}

void cObjectPropertyDialog::OnCheckedVisibleLevel2()
{
	OBJECTEDIT->SetGroupVisibleLevel( 2 );
}

void cObjectPropertyDialog::OnCheckedVisibleLevel3()
{
	OBJECTEDIT->SetGroupVisibleLevel( 3 );
}

void cObjectPropertyDialog::OnClickedVisibleLevelSibling()
{
	int mb = MessageBox( "Do you really want to set Visible Level to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetVisibleLevelToSiblings();
}

void cObjectPropertyDialog::OnCheckedOccludeLevel0()
{
	OBJECTEDIT->SetGroupOccludeLevel( 0 );
}

void cObjectPropertyDialog::OnCheckedOccludeLevel1()
{
	OBJECTEDIT->SetGroupOccludeLevel( 1 );
}

void cObjectPropertyDialog::OnCheckedOccludeLevel2()
{
	OBJECTEDIT->SetGroupOccludeLevel( 2 );
}

void cObjectPropertyDialog::OnClickedOccludeLevelSibling()
{
	int mb = MessageBox( "Do you really want to set Occlude Level to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetOccludeLevelToSiblings();
}

void cObjectPropertyDialog::OnCheckedFogApplied()
{
	int checked = ((CButton*)GetDlgItem( IDC_CHECK_OPROP_FOG_APPLIED ))->GetCheck();

	OBJECTEDIT->SetGroupFogApplied( checked != 0 );
}

void cObjectPropertyDialog::OnClickedFogAppliedSibling()
{
	int mb = MessageBox( "Do you really want to set Fog to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetFogToSiblings();
}

void cObjectPropertyDialog::OnCheckedMaterialApplied()
{
	int checked = ((CButton*)GetDlgItem( IDC_CHECK_OPROP_MATERIAL_APPLIED ))->GetCheck();

	OBJECTEDIT->SetGroupMaterialApplied( checked != 0 );
}

void cObjectPropertyDialog::OnClickedMaterialAppliedSibling()
{
	int mb = MessageBox( "Do you really want to set Material Applied to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetMaterialToSiblings();
}

void cObjectPropertyDialog::OnClickedAmbient()
{
	CColorDialog dlg( mAmbientBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mAmbientBox.SetColor( dlg.GetColor() );
		OBJECTEDIT->SetGroupAmbient( dlg.GetColor() );
	}
}

void cObjectPropertyDialog::OnClickedDiffuse()
{
	CColorDialog dlg( mDiffuseBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mDiffuseBox.SetColor( dlg.GetColor() );
		OBJECTEDIT->SetGroupDiffuse( dlg.GetColor() );
	}
}

void cObjectPropertyDialog::OnClickedEmissive()
{
	CColorDialog dlg( mEmissiveBox.GetColor(), CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mEmissiveBox.SetColor( dlg.GetColor() );
		OBJECTEDIT->SetGroupEmissive( dlg.GetColor() );
	}
}

void cObjectPropertyDialog::OnClickedAmbientSibling()
{
	int mb = MessageBox( "Do you really want to set Ambient to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetAmbientSiblings();
}

void cObjectPropertyDialog::OnClickedDiffuseSibling()
{
	int mb = MessageBox( "Do you really want to set Diffuse to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetDiffuseSiblings();
}

void cObjectPropertyDialog::OnClickedEmissiveSibling()
{
	int mb = MessageBox( "Do you really want to set Emissive to Siblings?", "Object", MB_YESNO );

	if( mb == IDYES )
		OBJECTEDIT->SetEmissiveSiblings();
}