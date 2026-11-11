#include "stdafx.h"

#include "MapDialog.h"
#include "Resource.h"
#include "RegenToolView.h"

IMPLEMENT_DYNAMIC(cMapDialog, CDialog)

BEGIN_MESSAGE_MAP(cMapDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MAP_LOAD, OnClickedLoad)
	ON_BN_CLICKED(IDC_CHECK_MAP_NAVIFIELD, OnCheckedButton)
	ON_LBN_SELCHANGE(IDC_COMBO_MAP_INDEX, OnSelchangeMapIdx)
	ON_LBN_SELCHANGE(IDC_COMBO_MAP_DEGREE, OnSelchangeMapDegee)
END_MESSAGE_MAP()

cMapDialog::cMapDialog()
{
}

cMapDialog::~cMapDialog()
{
}

void cMapDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_MAP_LOAD, mLoadButton );
}

void cMapDialog::SetMapInfo( tMap<unsigned int,sMapInfo*>* mapIdxSet )
{
	if( mapIdxSet == 0 )
	{
		assert(0);
		return;
	}

	///
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MAP_INDEX);
	if( combo )
	{
		combo->ResetContent();
		cString str;

		tMap<unsigned int,sMapInfo*>::cConstIterator ii = mapIdxSet->Begin();
		tMap<unsigned int,sMapInfo*>::cConstIterator end = mapIdxSet->End();
		for( unsigned int c = 0; ii != end; c++, ii++ )
		{
			unsigned int mapIdx = (unsigned int)ii->mFirst;
			sMapInfo* info = (sMapInfo*)ii->mSecond;

			str = VIEW->GetMapName( mapIdx);
			str.Format( "(%.3d)%s", mapIdx, str.Cstr() );

			combo->AddString( str.Cstr() );
			combo->SetItemDataPtr( c, info );
		}
		combo->SetCurSel( -1 );
	}

	///
	combo = (CComboBox*)GetDlgItem( IDC_COMBO_MAP_DEGREE );
	if( combo )
		combo->EnableWindow( false );

	mLoadButton.EnableWindow( false );
}

void cMapDialog::OnClickedLoad()
{
	///
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MAP_INDEX);
	int cursel = combo->GetCurSel();
	if( cursel == LB_ERR )
		return;
	sMapInfo* info = (sMapInfo*)combo->GetItemData( cursel );

	///
	unsigned int mapDegree = 0;
	combo = (CComboBox*)GetDlgItem(IDC_COMBO_MAP_DEGREE);
	if( combo->IsWindowEnabled() )
	{
		cursel = combo->GetCurSel();
		if( cursel == LB_ERR )
			return;

		mapDegree = (unsigned int)combo->GetItemData( cursel );
	}

	cString str;
	switch( mapDegree )
	{
	case 0:
		str.Format( "Do you really want to load Map%02d?", info->mapIdx );
		break;
	case 1:
		str.Format( "Do you really want to load Map%02d Normal?", info->mapIdx, mapDegree );
		break;
	case 2:
		str.Format( "Do you really want to load Map%02d Hard?", info->mapIdx, mapDegree );
		break;
	default:
		{
			assert(0);
			return;
		}
	}

	int mb = MessageBox( str.Cstr(), "Map", MB_YESNO );

	if( mb == IDNO )
		return;

	if( VIEW->LoadMap( info->mapIdx, info->mapfolerIdx, mapDegree ) == false )
		MessageBox( "Failed to load Map" );
}

void cMapDialog::OnCheckedButton()
{
	VIEW->Update();
}

void cMapDialog::OnSelchangeMapIdx()
{
	CComboBox* combo = (CComboBox*)GetDlgItem( IDC_COMBO_MAP_INDEX );
	CComboBox* comDegree = (CComboBox*)GetDlgItem( IDC_COMBO_MAP_DEGREE );
	comDegree->ResetContent();
	mLoadButton.EnableWindow( FALSE );

	int i = combo->GetCurSel();
	if( i == LB_ERR )
	{
		comDegree->EnableWindow( FALSE );
		return;
	}

	sMapInfo* info = (sMapInfo*)combo->GetItemDataPtr( i );
	if( info->degreeInfo.IsEmpty() == false )
	{
		comDegree->EnableWindow( TRUE );

		tList<unsigned int>::cIterator ii = info->degreeInfo.Begin();
		tList<unsigned int>::cIterator iend = info->degreeInfo.End();
		for(unsigned int c=0; ii != iend; ii++, c++ )
		{
			unsigned int degree = (unsigned int)*ii;
			cString str;
			if( degree == 1 )
			{
				str = "Normal";
			}
			else if( degree == 2 )
			{
				str = "Hard";
			}
			else
			{
				assert(0);
				continue;
			}

			comDegree->AddString( str.Cstr() );
			comDegree->SetItemData( c, (DWORD)degree );
		}
		comDegree->SetCurSel(-1);
	}
	else
	{
		comDegree->EnableWindow( FALSE );
		mLoadButton.EnableWindow( TRUE );
	}
}

void cMapDialog::OnSelchangeMapDegee()
{
	CComboBox* combo = (CComboBox*)GetDlgItem( IDC_COMBO_MAP_DEGREE );

	int i = combo->GetCurSel();
	if( i == LB_ERR )
		return;

	mLoadButton.EnableWindow( TRUE );
}

bool cMapDialog::IsNaviFieldChecked() const
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_MAP_NAVIFIELD );
	return check->GetCheck() != 0;
}