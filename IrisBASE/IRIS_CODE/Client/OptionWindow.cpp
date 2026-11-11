#include "StdAfx.h"
#include "OptionWindow.h"
#include "OptionSheet.h"

cOptionWindow::cOptionWindow()
: mpOptSheet1(0)
, mpOptSheet2(0)
, mpOptSheet3(0)
{
}

cOptionWindow::~cOptionWindow()
{

}

void cOptionWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpOptSheet1 )
		mpOptSheet1->UpdateOptionData();
	if( mpOptSheet2 )
		mpOptSheet2->UpdateOptionData();
	if( mpOptSheet3 )
		mpOptSheet3->UpdateOptionData();
}

bool cOptionWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cTabWindow::OnCreate( pproperty ) == false )
		return false;

	mpOptSheet1 = (cOptionSheet1*)GetChild(eUIID_OPTION_SHEET1);
	mpOptSheet2 = (cOptionSheet2*)GetChild(eUIID_OPTION_SHEET2);
	mpOptSheet3 = (cOptionSheet3*)GetChild(eUIID_OPTION_SHEET3);

	SetCurrent(0);
	return true;
}

void cOptionWindow::OnCommand(cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_OPTION_DEFAULT:
		{
			if( mpOptSheet1 )
				mpOptSheet1->InitOptionData();
			if( mpOptSheet2 )
				mpOptSheet2->InitOptionData();
			if( mpOptSheet3 )
				mpOptSheet3->InitOptionData();
		}
		break;
	case eUIID_OPTION_APPLY:
		{
			if( mpOptSheet1 )
				mpOptSheet1->ApplyOptionData();
			if( mpOptSheet2 )
				mpOptSheet2->ApplyOptionData();
			if( mpOptSheet3 )
				mpOptSheet3->ApplyOptionData();

			Hide();
		}
		break;
	case eUIID_OPTION_CANCEL:
	case eUIID_OPTION_CLOSE:
		{
			if( mpOptSheet1 )
				mpOptSheet1->CancelOptionData();
			if( mpOptSheet2 )
				mpOptSheet2->CancelOptionData();
			if( mpOptSheet3 )
				mpOptSheet3->CancelOptionData();

			Hide();
		}
		break;
	}
}
