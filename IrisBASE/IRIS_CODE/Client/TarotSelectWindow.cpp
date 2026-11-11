#include "stdafx.h"
#include "TarotSelectWindow.h"
#include "TarotManager.h"
#include "TarotWindow.h"
#include "Label.h"
#include "Button.h"
#include "UIMsgboxEvent.h"
#include "GameResourceManager.h"
#include "Npc_Common.h"

cTarotSelectWindow::cTarotSelectWindow()
: mpCount(0)
, mpShopName(0)
, mpMain(0)
, mpComplete(0)
, mpRandomSelect(0)
, mpNameBack(0)
, mpNameTextBack(0)
{
}

cTarotSelectWindow::~cTarotSelectWindow()
{
}

bool cTarotSelectWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;

	mpCount = (cLabel*)GetChild( eUIID_GAME_TAROTSELECT_COUNT );
	mpShopName = (cLabel*)GetChild( eUIID_GAME_TAROTSELECT_SHOPNAME );
	mpComplete = (cButton*)GetChild( eUIID_GAME_TAROTSELECT_COMPLETE );
	mpRandomSelect = (cButton*)GetChild( eUIID_GAME_TAROTSELECT_RANDOM );
	mpNameBack = GetChild( eUIID_GAME_TAROTSELECT_NAMEBACKIMG );
	mpNameTextBack = GetChild( eUIID_GAME_TAROTSELECT_NAMETEXT );

	if( mpNameBack )
		mpNameBack->SetEnabled( false );
	return true;
}

///
void cTarotSelectWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TAROTSELECT_CANCEL:
		{
			eTarotType type = TAROTMAN->GetTarotType();
			if( type == eTAROT_USER )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_TAROTSEEKER_EXIT, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 312 ), // 종료 경고 메시지
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else if( type == eTAROT_SELF )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_TAROTSEEKER_EXIT, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 328 ), // 종료 경고 메시지
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else if( type == eTAROT_TUTORIAL )
			{
				MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				mpMain->NetLock( ePAGE_SELECT );
				mpMain->NetLock( ePAGE_SELFVIEW );
				mpMain->NetLock( ePAGE_RESULT );
			}
		}
		break;
	case eUIID_GAME_TAROTSELECT_COMPLETE:
		{
			if( mpMain->GetSelectedCount() >= 10 )
			{
				eTarotType type = TAROTMAN->GetTarotType();
				if( type == eTAROT_USER )
				{
					/// 점괘 선택 관련 내용 서버에 보내기 
					MSG_REQ_TAROT_SEEKER_RESULT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_SEEKER_RESULT_REQ;
					msg.SpreadIndex = mpMain->GetSelectedSpreadIndex();
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_SELECT );
				}
				else if( type == eTAROT_SELF )
				{
					MSG_REQ_TAROT_SELF_RESULT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_SELF_RESULT_REQ;
					msg.SpreadIndex = mpMain->GetSelectedSpreadIndex();
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_SELECT );
				}
				else if( type == eTAROT_TUTORIAL )
				{
					MSG_REQ_TAROT_TUTORIAL_SEEKER_RESULT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_RESULT_REQ;
					msg.SpreadIndex = mpMain->GetSelectedSpreadIndex();
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_SELECT );
				}
				else
					assert(0);
			}
			else
			{
				/// 타로카드 10장 선택 안됨
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 311 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
		}
		break;
	case eUIID_GAME_TAROTSELECT_RANDOM:
		if( mpMain->IsEnalbeOpenDrama() == false )
		{
			for(;;)
			{
				if( mpMain->GetSelectedCount() >= 10 )
					break;

				mpMain->OpenSelectCard( 0 );
			}
		}
		break;
	}
}

///
void cTarotSelectWindow::Show( bool onsound )
{
	eTarotType type = TAROTMAN->GetTarotType();
	switch( type )
	{
	case eTAROT_USER:
		{
			if( mpShopName )
			{
				mpShopName->SetText( mpMain->GetTitle() );
				mpShopName->Show();
			}

			if( mpNameBack )
				mpNameBack->Show();

			if( mpNameTextBack )
				mpNameTextBack->Show();
		}
		break;
	case eTAROT_SELF:
	case eTAROT_TUTORIAL:
		{
			if( mpShopName )
				mpShopName->Hide();

			if( mpNameBack )
				mpNameBack->Hide();

			if( mpNameTextBack )
				mpNameTextBack->Hide();
		}
		break;
	default:
		assert(0);
		return;
	}

	SetSelectCardCount( mpMain->GetSelectedCount() );

	if( mpComplete )
		mpComplete->SetEnabled( false );

	if( mpRandomSelect )
		mpRandomSelect->SetEnabled( true );

	cUIWindow::Show( onsound );
}

///
void cTarotSelectWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpShopName )
		mpShopName->Clear();
}

///
void cTarotSelectWindow::SetSelectCardCount( unsigned int count )
{
	/// 문자열 출력 
	if( mpCount )
	{
		cStringT str;
		str.Format( _T("%d / 10"), count );
		mpCount->SetText( (LPCTSTR)str.Cstr() );
	}
	else
	{
		assert(0);
	}

	if( count == 10 )
	{
		if( mpComplete )
			mpComplete->SetEnabled( true );
		if ( mpRandomSelect )
			mpRandomSelect->SetEnabled( false );
	}
}

//------------------------------------------------------------------------
cNpcTarotSelectWindow::cNpcTarotSelectWindow()
: mpMain(0)
, mpCount(0)
, mpComplete(0)
, mpRandomSelect(0)
{

}

cNpcTarotSelectWindow::~cNpcTarotSelectWindow()
{
}

bool cNpcTarotSelectWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;
	mpCount = (cLabel*)GetChild( eUIID_GAME_NPCTAROTSELECT_COUNT );
	mpComplete = (cButton*)GetChild( eUIID_GAME_NPCTAROTSELECT_COMPLETE );
	mpRandomSelect = (cButton*)GetChild( eUIID_GAME_NPCTAROTSELECT_RANDOM );

	if( !mpMain )
	{
		assert(0);
		return false;
	}

	return true;
}

void cNpcTarotSelectWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_NPCTAROTSELECT_COMPLETE:
		{
			sNpcSpread* spread = TAROTMAN->GetNpcSpread( mpMain->GetSelectedSpreadIndex() );
			if( spread )
			{
				unsigned long compare = 0x00010000;
				compare = compare >> (4 * spread->mFortuneProp);
				if( (mpMain->GetFortuneData() & compare) == compare ) 
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 332 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				else
				{
					MSG_REQ_NPC_TAROT_FORTUNE msg;
					msg.Category = NM_NPC;
					msg.Protocol = NM_NPC_TAROT_FORTUNE_REQ;
					msg.fortuneProp = spread->mFortuneProp;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					mpMain->NetLock( ePAGE_SELECT );
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	case eUIID_GAME_NPCTAROTSELECT_CANCEL:
		{
			/// 종료
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NPCTAROT_EXIT, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 312 ),
				GAMERESOURCEMAN->GetGameText( 31 )
				);
		}
		break;
	case eUIID_GAME_NPCTAROTSELECT_RANDOM:
		{
			/// 랜덤 뽑기
			for(;;)
			{
				if( mpMain->GetSelectedCount() >= mpMain->GetMaxCount() )
					break;

				mpMain->OpenSelectNpcCard( 0 );
			}
		}
		break;
	}
}

///
void cNpcTarotSelectWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpComplete )
		mpComplete->SetEnabled( false );

	if( mpRandomSelect )
		mpRandomSelect->SetEnabled( true );
}

///
void cNpcTarotSelectWindow::SelectCount( unsigned int count, unsigned int max )
{
	if( mpCount )
	{
		cStringT str;
		str.Format( _T("%d / %d"), count, max );
		mpCount->SetText( (LPCTSTR)str.Cstr() );
	}
	else
	{
		assert(0);
	}
}

void cNpcTarotSelectWindow::SetEnableComplete( bool enable )
{
	if( mpComplete )
		mpComplete->SetEnabled( enable );

	if( mpRandomSelect )
		mpRandomSelect->SetEnabled( !enable );
}
