#include "stdafx.h"
#include "ItemMallWindow.h"
#include "WebManager.h"

cItemMallWindow::cItemMallWindow()
{

}

cItemMallWindow::~cItemMallWindow()
{

}

void cItemMallWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	WEBMAN->Show();
}

void cItemMallWindow::OnHide()
{
	cUIWindow::OnHide();

	WEBMAN->Hide();
}

bool cItemMallWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	return true;
}

void cItemMallWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	unsigned int x = GetAbsoluteRect().mLeft + 3;
	unsigned int y = GetAbsoluteRect().mTop + 31;
	unsigned int w = GetAbsoluteRect().GetWidth() - 6;
	unsigned int h = GetAbsoluteRect().GetHeight() - 31 - 3;
	WEBMAN->UpdateRect( x, y, w, h );
}

void cItemMallWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_ITEMMALL_CLOSE:
		Hide();
		break;
	}
}
