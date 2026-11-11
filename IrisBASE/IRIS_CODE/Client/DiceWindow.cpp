#include "stdafx.h"
#include "DiceWindow.h"
#include "Label.h"
#include "GameResourceManager.h"
#include "Party_Common.h"

cDiceWindow::cDiceWindow()
: mpRollButton(0)
, mpGiveUpButton(0)
, mpTime(0)
, mDelayTime(0)
{

}

cDiceWindow::~cDiceWindow()
{

}

bool cDiceWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpRollButton = GetChild( eUIID_DICE_ROLL );
	mpGiveUpButton = GetChild( eUIID_DICE_GIVEUP );
	mpTime = (cLabel*)GetChild( eUIID_DICE_TIME );

	return true;
}

void cDiceWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( !mVisible )
		return;

	if( mDelayTime <= deltaTime )
	{
		mDelayTime = 0;
		Hide();
	}
	else
	{
		mDelayTime -= deltaTime;
	}

	if( mpTime )
	{
		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText( 9019 ), unsigned int(mDelayTime*0.001f) );
		mpTime->SetText( (LPCTSTR)str.Cstr() );
	}
}

void cDiceWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_DICE_ROLL:
		{
			// 굴리기
			MSG_REQ_PARTY_DICE_ROLL msg;
			::memset( &msg, 0, sizeof( msg ) );
			msg.Category = NM_PARTY;
			msg.Protocol = NM_PARTY_DICE_ROLL_REQ;
			msg.mRoll	 = 1;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
			Hide();
		}
		break;
	case eUIID_DICE_GIVEUP:
		{
			// 포기
			MSG_REQ_PARTY_DICE_ROLL msg;
			::memset( &msg, 0, sizeof( msg ) );
			msg.Category = NM_PARTY;
			msg.Protocol = NM_PARTY_DICE_ROLL_REQ;
			msg.mRoll	 = 0;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
			Hide();
		}
		break;
	}
}

void cDiceWindow::StartDice( unsigned long time )
{
	// 목표 시간
	mDelayTime = time;

	cUIWindow::Show();
}

void cDiceWindow::OnNetLock( int lockTry )
{
	if( mpRollButton )
		mpRollButton->NetLock( lockTry );
	if( mpGiveUpButton )
		mpGiveUpButton->NetLock( lockTry );
}

void cDiceWindow::OnNetUnLock( int lockTry )
{
	if( mpRollButton )
		mpRollButton->NetUnLock( lockTry );
	if( mpGiveUpButton )
		mpGiveUpButton->NetUnLock( lockTry );
}
