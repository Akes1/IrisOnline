#include "stdafx.h"
#include "TarotControlWindow.h"
#include "Label.h"
#include "TarotManager.h"
#include "NumberEditBox.h"
#include "GameResourceManager.h"
#include "PushButton.h"

cTarotControlWindow::cTarotControlWindow()
: mpShopName(0)
, mpPay(0)
, mpTarotCount(0)
, mpStuffNum(0)
, mpStatus(0)
, mpUserNum(0)
, mpMoney(0)
, mpPoint(0)
{
	mPrice = 0;
}

cTarotControlWindow::~cTarotControlWindow()
{
}

void cTarotControlWindow::Clear()
{
	if( mpShopName )
		mpShopName->Clear();
	if( mpPay )
		mpPay->SetValue(0);
	if( mpTarotCount )
		mpTarotCount->Clear();
	if( mpStuffNum )
		mpStuffNum->Clear();
	if( mpStatus )
		mpStatus->Clear();
	if( mpUserNum )
		mpUserNum->Clear();
	if( mpMoney )
		mpMoney->SetValue(0);
	if( mpPoint )
		mpPoint->Clear();
	if( mpClose )
		mpClose->SetPress( false );

	mUserName.Clear();
}

void cTarotControlWindow::Open()
{
}

void cTarotControlWindow::Close()
{
	cUIWindow::Close();
	Clear();
}

void cTarotControlWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

///
bool cTarotControlWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpShopName = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_SHOPNAME );
	mpPay = (cNumberEditBox*)GetChild( eUIID_GAME_TAROTCONTROL_PAY );
	mpTarotCount = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_TAROTCOUNT );
	mpStuffNum = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_STUFFNUM );
	mpStatus = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_STATUS );
	mpUserNum = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_USERNUM );
	mpMoney = (cNumberEditBox*)GetChild( eUIID_GAME_TAROTCONTROL_MONEY );
	mpPoint = (cLabel*)GetChild( eUIID_GAME_TAROTCONTROL_POINT );
	mpClose = (cPushButton*)GetChild( eUIID_GAME_TAROTCONTROL_CLOSE2 );
	return true;
}

///
void cTarotControlWindow::OnCommand( cUINode*, unsigned int id )
{
	switch(id)
	{
		case eUIID_GAME_TAROTCONTROL_CLOSE:
			if( mpClose->IsPress() == false )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_TAROTLEADER_EXIT, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 313 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else
			{
				/// 닫기 예약 취소 메시지 보내기
				MSG_REQ_TAROT_READER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				NetLock();
			}
			break;
	default:
		assert(0);
		break;
	}
}

///
void cTarotControlWindow::SetShopName( LPCTSTR name )
{
	if( mpShopName )
	{
		mpShopName->SetText( name );
	}
	else
	{
		assert(0);
	}
}

///
void cTarotControlWindow::SetPay( unsigned int pay )
{
	if( mpPay )
	{
		mpPay->SetValue( pay );
	}
	else
	{
		assert(0);
	}

	mPrice = pay;
}

///
void cTarotControlWindow::SetTarotCount( unsigned int count )
{
	/// 문자열 출력 
	if( mpTarotCount )
	{
		mpTarotCount->SetValue( count );
	}
	else
	{
		assert(0);
	}
}

///
void cTarotControlWindow::SetStuffNum( unsigned int num )
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
void cTarotControlWindow::SetStatus( LPCTSTR name )
{
	if( mpStatus == 0 )
	{
		assert(0);
		return;
	}

	mUserName = name;

	cStringT str;
	str.Format( GAMERESOURCEMAN->GetGameText( 317 ), name );
	mpStatus->SetText( (LPCTSTR)str.Cstr() );
}

///
void cTarotControlWindow::ReleaseStatus( LPCTSTR name )
{
	if( mpStatus )
        mpStatus->Clear();
}

///
void cTarotControlWindow::SetUserNum( unsigned int num )
{
	if( mpUserNum )
	{
		mpUserNum->SetValue( num );
	}
	else
	{
		assert(0);
	}

	SetMoney( mPrice * num );
}

///
void cTarotControlWindow::SetMoney( unsigned int money )
{
	if( mpMoney )
	{
		mpMoney->SetValue( money );
	}
	else
	{
		assert(0);
	}
}

///
void cTarotControlWindow::SetPoint( unsigned int point )
{
	if( mpPoint )
	{
		mpPoint->SetValue( point );
	}
	else
	{
		assert(0);
	}
}

void cTarotControlWindow::OnPushButtonPressed( cUINode* , unsigned int id )
{
	
	if( mpClose )
	{
		/// 닫기 예약 버튼 이미지 변경은 실제로 네트워크 메시지를 받은 후에 처리한다.
		bool isPress = mpClose->IsPress();
		mpClose->SetPress( !isPress );

		if( id == eUIID_GAME_TAROTCONTROL_CLOSE2 )
		{
			if( mpClose->IsPress() == false )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_TAROTLEADER_EXIT, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 313 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else
			{
				/// 닫기 예약 취소 메시지 보내기
				MSG_REQ_TAROT_READER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				NetLock();
			}
		}
	}
}

void cTarotControlWindow::SetReserve( bool press )
{
	if( mpClose )
		mpClose->SetPress( press );
}
