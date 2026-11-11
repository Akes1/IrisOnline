#include "stdafx.h"
#include "SummonWindow.h"
#include "CamaelWindow.h"
#include "NpcDealWindow.h"
#include "Button.h"
#include "ItemManager.h"
#include "Trade_Common.h"
#include "ObjectManager.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "CoolTimeManager.h"
#include "Hero.h"



cSummonWindow::cSummonWindow()
: mSummonSlotIdx( MAX_INVENTORY )
{
	::ZeroMemory( mpSummonBtn, sizeof(mpSummonBtn) );

	SetBackupVisible( false );
}

cSummonWindow::~cSummonWindow()
{
}

void cSummonWindow::Open()
{
	Clear();
}

void cSummonWindow::Close()
{
	Clear();

	cUIWindow::Close();
}

bool cSummonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;
	
	mpSummonBtn[0] = (cButton*)GetChild( eUIID_GAME_SUMMON_WAREHOUSE );
	mpSummonBtn[1] = (cButton*)GetChild( eUIID_GAME_SUMMON_POST );
	mpSummonBtn[2] = (cButton*)GetChild( eUIID_GAME_SUMMON_DEAL );

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	return true;
}

void cSummonWindow::OnHide()
{
	// 락 풀기
	if( mSummonSlotIdx != MAX_INVENTORY )
	{		
		cItem* Item = ITEMMAN->GetItem( mSummonSlotIdx );
		if( Item )
			Item->SetLock( false );
	}

	cUIWindow::OnHide();
}

void cSummonWindow::Clear()
{
	mSummonSlotIdx = MAX_INVENTORY;
}

void cSummonWindow::Show( unsigned short slotIdx )
{
	if( mSummonSlotIdx != MAX_INVENTORY )
	{
		cItem* Item = ITEMMAN->GetItem( mSummonSlotIdx );
		if( Item )
			Item->SetLock( false );

		mSummonSlotIdx = MAX_INVENTORY; 
	}

	mSummonSlotIdx = slotIdx;

	cItem* item = ITEMMAN->GetItem( mSummonSlotIdx );
	if( item )
		item->SetLock( true );

	cUIWindow::Show();
}

void cSummonWindow::OnCommand( cUINode* caller, unsigned int id )
{
	if( ITEMMAN == NULL || HERO == NULL )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_GAME_SUMMON_CLOSE:
		{
			Hide();
		}
		break;
	case eUIID_GAME_SUMMON_POST:		
		{
			if( CheckUsableState() == false )
				return;

			MSG_REQ_TRADE_POST_SUMMON_OPEN msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category	= NM_TRADE;
			msg.Protocol	= NM_TRADE_POST_SUMMON_OPEN_REQ;
			msg.number		= mSummonSlotIdx;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
			ITEMMAN->LockItemTry( ITEM_TRY_SUMMON, mSummonSlotIdx );				
		}	
		break;
	case eUIID_GAME_SUMMON_DEAL:
		{
			if( CheckUsableState() == false )
				return;

			cNpcDealWindow* pNpcDealWin = GAMEUI->GetNpcDealWindow();
			if( pNpcDealWin == NULL )
			{
				assert(0);
				return;
			}

			if( pNpcDealWin->IsNetLock() )
				return;

			MSG_REQ_TRADE_AGENT_SUMMON_OPEN msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category	= NM_TRADE;
			msg.Protocol	= NM_TRADE_AGENT_SUMMON_OPEN_REQ;
			msg.number		= mSummonSlotIdx;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
			ITEMMAN->LockItemTry( ITEM_TRY_SUMMON, mSummonSlotIdx );

			pNpcDealWin->NetLock();						
		}
		break;	
	case eUIID_GAME_SUMMON_WAREHOUSE:
		{
			if( CheckUsableState() == false )
				return;

			MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category	= NM_ITEM;
			msg.Protocol	= NM_ITEM_WAREHOUSE_SUMMON_OPEN_REQ;
			msg.number		= mSummonSlotIdx;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
			ITEMMAN->LockItemTry( ITEM_TRY_SUMMON, mSummonSlotIdx );				
		}	
		break;
	}	
}

bool cSummonWindow::CheckUsableState()
{
	if( mSummonSlotIdx == MAX_INVENTORY )
		return false;

	if( ITEMMAN->IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( !(stopFlag > eSTOP_NONE && stopFlag <= eSTOP_NPCSPEECH) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	return true;
}

void cSummonWindow::StartCoolTime()
{
	if( mSummonSlotIdx != MAX_INVENTORY )
	{
		cItem* item = ITEMMAN->GetItem( mSummonSlotIdx );
		if( item )
		{
			cItemDefine* define = item->GetDefine();
			if( define )
			{
				cItemAbility* abil = define->GetAbililty();

				if( abil && abil->GetCoolTime() > 0 )
				{
					COOLMAN->StartCoolTime( item->GetIndex() );
				}
				else
					assert(0);
			}
			else
				assert(0);
		}
		else
			assert(0);
	}
}

void cSummonWindow::OnNetLock( int lockTry )
{
	for( int i = 0; i < 3; i++ )
	{
		if( mpSummonBtn[ i ] )
			mpSummonBtn[ i ]->NetLock( lockTry );
	}
}

void cSummonWindow::OnNetUnLock( int lockTry )
{
	for( int i = 0; i < 3; i++ )
	{
		if( mpSummonBtn[ i ] )
			mpSummonBtn[ i ]->NetUnLock( lockTry );
	}
}	
