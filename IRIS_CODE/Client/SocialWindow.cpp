#include "StdAfx.h"
#include "SocialWindow.h"
#include "Label.h"
#include "NumberEditBox.h"
#include "ObjectManager.h"
#include "Hero.h"

cSocialWindow::cSocialWindow()
: mpFirePoint(0)
, mpWaterPoint(0)
, mpWindPoint(0)
, mpEarthPoint(0)
{

}

cSocialWindow::~cSocialWindow()
{

}

void cSocialWindow::Clear()
{
	if( mpFirePoint )
		mpFirePoint->SetValue( 0 );

	if( mpWaterPoint )
		mpWaterPoint->SetValue( 0 );

	if( mpWindPoint )
		mpWindPoint->SetValue( 0 );

	if( mpEarthPoint )
		mpEarthPoint->SetValue( 0 );
}

bool cSocialWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty) == false )
		return false;

	mpFirePoint = (cLabel*)GetChild( eUIID_GAME_SOCIAL_FIREPOINT );
	mpWaterPoint = (cLabel*)GetChild( eUIID_GAME_SOCIAL_WATERPOINT );
	mpWindPoint = (cLabel*)GetChild( eUIID_GAME_SOCIAL_WINDPOINT );
	mpEarthPoint = (cLabel*)GetChild( eUIID_GAME_SOCIAL_EARTHPOINT );

	Clear();
	return true;
}

void cSocialWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_GAME_SOCIAL_CLOSE )
		Hide();
}

void cSocialWindow::SetFirePoint( unsigned int point )
{
	if( mpFirePoint )
	{
		LPCTSTR fire = cNumberEditBox::FormatNumber( point );
		mpFirePoint->SetText( fire );
	}
}

void cSocialWindow::SetWaterPoint( unsigned int point )
{
	if( mpWaterPoint )
	{
		LPCTSTR water = cNumberEditBox::FormatNumber( point );
		mpWaterPoint->SetText( water );
	}
}

void cSocialWindow::SetWindPoint( unsigned int point )
{
	if( mpWindPoint )
	{
		LPCTSTR wind = cNumberEditBox::FormatNumber( point );
		mpWindPoint->SetText( wind );
	}
}

void cSocialWindow::SetEarthPoint( unsigned int point )
{
	if( mpEarthPoint )
	{
		LPCTSTR earth = cNumberEditBox::FormatNumber( point );
		mpEarthPoint->SetText( earth );
	}
}

void cSocialWindow::UpdatePoint()
{
	SetFirePoint( HERO->GetFirePoint() );
	SetWaterPoint( HERO->GetWaterPoint() );
	SetWindPoint( HERO->GetWindPoint() );
	SetEarthPoint( HERO->GetEarthPoint() );
}