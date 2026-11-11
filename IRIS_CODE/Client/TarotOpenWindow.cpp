#include "stdafx.h"
#include "TarotOpenWindow.h"

#include "Tarot_Common.h"
#include "TarotWindow.h"

#include "Label.h"
#include "NumberEditBox.h"
#include "MultiEditBox.h"
#include "TarotManager.h"

#include "ItemManager.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "Player_Common.h"

cTarotOpenWindow::cTarotOpenWindow()
: mpTotal(0)
, mpTarotCount(0)
, mpStuffNum(0)
, mpPrice(0)
, mpName(0)
, mpMain(0)
{
}

cTarotOpenWindow::~cTarotOpenWindow()
{
}

///
bool cTarotOpenWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;

	mpTotal = (cLabel*)GetChild( eUIID_GAME_TAROTOPEN_TOTAL );
	mpTarotCount = (cLabel*)GetChild( eUIID_GAME_TAROTOPEN_TAROTCOUNT );
	mpStuffNum = (cLabel*)GetChild( eUIID_GAME_TAROTOPEN_STUFFNUM );
	mpPrice = (cNumberEditBox*)GetChild( eUIID_GAME_TAROTOPEN_PRICE );
	mpName = (cMultiEditBox*)GetChild( eUIID_GAME_TAROTOPEN_NAME );
	return true;
}

///
void cTarotOpenWindow::OnCommand( cUINode*, unsigned int id )
{
	if( !mpPrice || !mpName )
		return;

	switch( id )
	{
	case eUIID_GAME_TAROTOPEN_CANCEL:
		{
			MSG_REQ_TAROT_READER_CLOSE msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			if( mpMain )
				mpMain->NetLock( ePAGE_OPEN );
		}
		break;
	case eUIID_GAME_TAROTOPEN_OPEN:
		{
			/// 가격 세팅 요청 메세지
			unsigned long nPrice = mpPrice->GetValue();
			if( nPrice == 0 )
			{	
				TAROTMAN->ShowErrorMsg( ERROR_PRICE );
				return;
			}
			
			/// 등록한 이용요금 + 캐릭터 보유 금액이 최대 보유 금액보다 크게 된다면
			if( nPrice + HERO->GetMoney() > MAX_HAVE_MONEY )
			{
				TAROTMAN->ShowErrorMsg( ERROR_MAX_MONEY );
				return;
			}

			if( mpName->GetLength() == 0 )
			{
				TAROTMAN->ShowErrorMsg( ERROR_TITLE );
				return;
			}

			/// 점집 열수 있는지 여부 체크..
			unsigned int error = TAROTMAN->CheckOpenTarot();
			if( error != ERROR_NONE )
			{
				TAROTMAN->ShowErrorMsg( error );
				return;
			}

			/// 점집 오픈 
			MSG_REQ_TAROT_READER_OPEN msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_READER_OPEN_REQ;

			assert(mpName->GetLength() < MAX_TAROT_TITLE);
			Sstrncpy( msg.strTitle, MAX_TAROT_TITLE, mpName->GetText(), mpName->GetLength() );
			msg.Price = mpPrice->GetValue();
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	}
}

void cTarotOpenWindow::OnHide()
{
	cUIWindow::OnHide();
	
	if( mpTotal )
		mpTotal->Clear();
	if( mpTarotCount )
		mpTarotCount->Clear();
	if( mpStuffNum )
		mpStuffNum->Clear();
	if( mpPrice )
		mpPrice->Clear();
	if( mpName )
		mpName->Clear();
}

///
void cTarotOpenWindow::OnShow()
{
	cUIWindow::OnShow();

	SetEnabled( true );


	SetTotalValue( mpMain->GetCardTotalPoint() );

	sItemCount* p = ITEMMAN->GetItemCount( TAROT_MATERIAL_ITEM );
	unsigned int invenCount = ( p ) ? p->bag : 0;
	SetStuffNum( invenCount );

	/// 타이틀에 포커스
	if( mpName )
		mpName->SetFocus();

	if( mpPrice )
	{
		mpPrice->SetFocus();
		mpPrice->SetAllSelect();
	}
}

///
void cTarotOpenWindow::SetTarotCount( unsigned int count, unsigned int total )
{
	if( mpTarotCount )
	{
		/// 문자열 출력 
		cStringT str;
		str.Format( _T("%d / %d"), count, total );
		mpTarotCount->SetText( (LPCTSTR)str.Cstr() );
	}
	else
		assert( 0 );
}

///
void cTarotOpenWindow::SetStuffNum( unsigned int num )
{
	if( mpStuffNum )
	{
		mpStuffNum->SetValue( num );
	}
	else
	{
		assert(0);
	}
}

///
void cTarotOpenWindow::SetTotalValue( unsigned int total )
{
	if( mpTotal )
	{
		mpTotal->SetValue( total );
	}
	else
	{
		assert(0);
	}
}
