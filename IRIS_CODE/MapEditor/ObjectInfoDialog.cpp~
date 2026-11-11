#include "stdafx.h"
#include "ObjectInfoDialog.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(cObjectInfoDialog, CDialog)

BEGIN_MESSAGE_MAP(cObjectInfoDialog, CDialog)
END_MESSAGE_MAP()

cObjectInfoDialog::cObjectInfoDialog()
{
}

cObjectInfoDialog::~cObjectInfoDialog()
{
}

void cObjectInfoDialog::UpdateScene( const NiPoint3& center, float minRadius, float maxRadius, unsigned int numObjects )
{
	/// 중점
	cString s;
	s.Format( "( %.2f, %.2f, %.2f )", center.x, center.y, center.z );
	GetDlgItem(IDC_EDIT_OINFO_CENTER)->SetWindowText( s.Cstr() );

	/// 장면 트리 반지름
	s.Format( "%.2f meters", minRadius / 100.0f );
	GetDlgItem(IDC_EDIT_OINFO_MIN_RADIUS)->SetWindowText( s.Cstr() );

	s.Format( "%.2f meters", maxRadius / 100.0f );
	GetDlgItem(IDC_EDIT_OINFO_MAX_RADIUS)->SetWindowText( s.Cstr() );

	/// 오브젝트 수
	SetDlgItemInt( IDC_EDIT_OINFO_NUM_OBJECTS, numObjects );
}

void cObjectInfoDialog::UpdateNumObjects( unsigned int numObjects )
{
	/// 오브젝트 수
	SetDlgItemInt( IDC_EDIT_OINFO_NUM_OBJECTS, numObjects );
}

void cObjectInfoDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}
