#include "stdafx.h"
#include "MakeButtonWindow.h"
#include "StageManager.h"

cMakeButtonWindow::cMakeButtonWindow()
{

}

cMakeButtonWindow::~cMakeButtonWindow()
{
}	

bool cMakeButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
	{
		return false;
	}
	return true;
}

