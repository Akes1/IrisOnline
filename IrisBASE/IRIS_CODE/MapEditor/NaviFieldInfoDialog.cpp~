#include "stdafx.h"
#include "NaviFieldInfoDialog.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(cNaviFieldInfoDialog, CDialog)

BEGIN_MESSAGE_MAP(cNaviFieldInfoDialog, CDialog)
END_MESSAGE_MAP()

cNaviFieldInfoDialog::cNaviFieldInfoDialog()
{
}

cNaviFieldInfoDialog::~cNaviFieldInfoDialog()
{
}

void cNaviFieldInfoDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}

void cNaviFieldInfoDialog::UpdateNaviField( unsigned int resolution, unsigned int leafResolution, float metersPerVertex, unsigned int unitsPerMeter )
{
	/// 그리드 크기를 설정
	cString s;
	s.Format( "%d * %d cells", resolution, resolution );
	SetDlgItemText( IDC_EDIT_TINFO_GRIDSIZE, s.Cstr() );

	/// 정점당 미터수를 설정
	if( metersPerVertex == 1.0f )
	{
		SetDlgItemText( IDC_EDIT_TINFO_METERSPERVERTEX, "1.00 meter" );
	}
	else
	{
		s.Format( "%.2f meters", metersPerVertex );
		SetDlgItemText( IDC_EDIT_TINFO_METERSPERVERTEX, s.Cstr() );
	}

	/// 미터당 단위를 설정
	s.Format( "%d units", unitsPerMeter );
	SetDlgItemText( IDC_EDIT_TINFO_UNITSPERMETER, s.Cstr() );

	/// 크기를 설정
	float sizeMeters = resolution * metersPerVertex;
	s.Format( "%.2f * %.2f meters", sizeMeters, sizeMeters );
	SetDlgItemText( IDC_EDIT_TINFO_SIZEMETERS, s.Cstr() );

	float sizeUnits = sizeMeters * unitsPerMeter;
	s.Format( "%.2f * %.2f units", sizeUnits, sizeUnits );
	SetDlgItemText( IDC_EDIT_TINFO_SIZEUNITS, s.Cstr() );

	/// 타일 수를 설정
	unsigned int tiles = resolution / leafResolution;
	s.Format( "%d * %d = %d tiles", tiles, tiles, tiles * tiles );
	SetDlgItemText( IDC_EDIT_TINFO_TILES, s.Cstr() );
}
