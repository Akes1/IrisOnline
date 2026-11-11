#include "stdafx.h"
#include "TarotCheckWindow.h"

#include "Tarot_Common.h"
#include "TarotWindow.h"
#include "Tarotmanager.h"
#include "NumberEditBox.h"

#include "Label.h"
#include "UIMsgboxEvent.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"

cTarotCheckWindow::cTarotCheckWindow()
: mpShopName(0)
, mpPrice(0)
, mpTotal(0)
, mpMain(0)
, mpNameBack(0)
, mpPayBack(0)
, mpPriceTextBack(0)
, mpNameTextBack(0)
{
}

cTarotCheckWindow::~cTarotCheckWindow()
{
}

///
bool cTarotCheckWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;

	mpShopName = (cLabel*)GetChild( eUIID_GAME_TAROTCHECK_SHOPNAME );
	mpPrice = (cLabel*)GetChild( eUIID_GAME_TAROTCHECK_PAY );
	mpTotal = (cLabel*)GetChild( eUIID_GAME_TAROTCHECK_TOTAL );
	mpNameTextBack = GetChild( eUIID_GAME_TAROTCHECK_NAMETEXT );
	mpPriceTextBack = GetChild( eUIID_GAME_TAROTCHECK_PRICETEXT );

	mpNameBack = GetChild( eUIID_GAME_TAROTCHECK_NAMEBACKIMG );
	if( mpNameBack )
		mpNameBack->SetEnabled( false );

	mpPayBack = GetChild( eUIID_GAME_TAROTCHECK_PAYIMG );
	if( mpPayBack )
		mpPayBack->SetEnabled( false );
	return true;
}

///
void cTarotCheckWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TAROTCHECK_CANCEL:
		{
			eTarotType type = TAROTMAN->GetTarotType();
			if( type == eTAROT_USER )
			{
				/// 취소
				MSG_REQ_TAROT_SEEKER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SEEKER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				if( mpMain )
					mpMain->NetLock( ePAGE_VIEW );
			}
			else if( type == eTAROT_SELF )
			{
				/// 취소
				MSGROOT msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SELF_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				if( mpMain )
					mpMain->NetLock( ePAGE_SELFVIEW );
			}
			else if( type == eTAROT_TUTORIAL )
			{
				MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				if( mpMain )
					mpMain->NetLock( ePAGE_VIEW );
			}
			else
				assert(0);
		}
		break;
	case eUIID_GAME_TAROTCHECK_START:
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_TAROTSEEKER_START, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 314 ), // 점괘시작확인 메세지
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	}
}

void cTarotCheckWindow::Show( bool onsound )
{
	if( mpNameBack )
		mpNameBack->Hide();
	if( mpPayBack )
		mpPayBack->Hide();
	if( mpPriceTextBack )
		mpPriceTextBack->Hide();
	if( mpNameTextBack )
		mpNameTextBack->Hide();
	if( mpShopName )
		mpShopName->Hide();
	if( mpPrice )
		mpPrice->Hide();

	eTarotType type = TAROTMAN->GetTarotType();
	switch( type )
	{
	case eTAROT_USER:
		{
			if( mpNameBack )
				mpNameBack->Show();
			if( mpPayBack )
				mpPayBack->Show();
			if( mpPriceTextBack )
				mpPriceTextBack->Show();
			if( mpNameTextBack )
				mpNameTextBack->Show();
			
			if( mpShopName )
			{
				mpShopName->SetText( mpMain->GetTitle() );
				mpShopName->Show();
			}

			if( mpPrice )
			{
				LPCTSTR price = cNumberEditBox::FormatNumber( mpMain->GetPrice() );
				mpPrice->SetText( price );
				mpPrice->Show();
			}		
		}
		break;
	case eTAROT_SELF:
	case eTAROT_TUTORIAL:
		break;
	default:
		assert(0);
		return;
	}

	if( mpTotal )
		mpTotal->SetValue( mpMain->GetCardTotalPoint() );

	cUIWindow::Show( onsound );
}

void cTarotCheckWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpShopName )
		mpShopName->SetText( mpMain->GetTitle() );

	if( mpPrice )
	{
		LPCTSTR price = cNumberEditBox::FormatNumber( mpMain->GetPrice() );
		mpPrice->SetText( price );
	}

	if( mpTotal )
		mpTotal->SetValue( mpMain->GetCardTotalPoint() );
}

///
void cTarotCheckWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpShopName )
		mpShopName->Clear();
	if( mpPrice )
		mpPrice->SetValue(0);
}

//--------------------------------------------------------------------------------
cNpcTarotCheckWindow::cNpcTarotCheckWindow()
: mpMain(0)
, mpPrice(0)
{

}

cNpcTarotCheckWindow::~cNpcTarotCheckWindow()
{

}

bool cNpcTarotCheckWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;
	mpPrice = (cLabel*)GetChild( eUIID_GAME_NPCTAROTCHECK_PAY );

	cLabel* payBack = (cLabel*)GetChild( eUIID_GAME_NPCTAROTCHECK_PAYIMAGE );
	if( payBack )
		payBack->SetEnabled( false );

	if( !mpMain )
	{
		assert(0);
		return false;
	}

	return true;
}

void cNpcTarotCheckWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_NPCTAROTCHECK_START:
		{
			unsigned int maxCount = mpMain->GetNpcHaveSpreadCount();

			unsigned long compare = 0;
			for( unsigned int i = 0; i < maxCount; ++i )
			{
				compare = compare << 4;
				compare = compare | 1;
			}

			/// 비트 밀기
			for( unsigned int i = 0; i < MAX_SPREAD - maxCount; ++i )
			{
				compare = compare << 4;
			}

			if( mpMain->GetFortuneData() == compare )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 335 ), // 경고 메시지
					GAMERESOURCEMAN->GetGameText( 2 )
					);
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NPCTAROT_START, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 314 ), // 점괘시작확인 메세지
					GAMERESOURCEMAN->GetGameText( 2 )
					);
			}
		}
		break;
	case eUIID_GAME_NPCTAROTCHECK_CANCEL:
		{
			HERO->CloseNpcTalk();
			TAROTMAN->ChangePage( ePAGE_NONE );
		}
		break;
	}
}

void cNpcTarotCheckWindow::OnShow()
{
	cUIWindow::OnShow();

	LPCTSTR price = cNumberEditBox::FormatNumber( mpMain->GetPrice() );
	if( mpPrice )
		mpPrice->SetText( price );
}

void cNpcTarotCheckWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpPrice )
		mpPrice->SetValue( 0 );
}

