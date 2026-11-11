#include "stdafx.h"
#include "ThemeManager.h"
#include "Theme_Common.h"
#include "Protocol.h"
#include "UIManager.h"
#include "UIMsgboxEvent.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "ThemeEnterWindow.h"
#include "ThemeStateWindow.h"
#include "GameUIManager.h"
#include "Application.h"

bool ThemeSort( void* first, void* second )
{
	sPlayerThemeList* pFirst  = (sPlayerThemeList*)first;
	sPlayerThemeList* pSecond = (sPlayerThemeList*)second;

	if( pFirst->mMapNumber == pSecond->mMapNumber )
		return (pFirst->mThemeMode < pSecond->mThemeMode);

	return ( pFirst->mMapNumber < pSecond->mMapNumber );
}

cThemeManager* cThemeManager::mpSingleton = 0;

cThemeManager::cThemeManager()
: mThemeExpireTime(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;
}

cThemeManager::~cThemeManager()
{
	cThemeInfoList::cIterator i = mThemeInfoList.Begin();
	cThemeInfoList::cIterator end = mThemeInfoList.End();
	for( ; i != end; ++i )
	{
		sPlayerThemeList* p = (sPlayerThemeList*)(*i);
		SAFE_DELETE( p );
	}
	mThemeInfoList.Clear();

	mpSingleton = 0;
}

void cThemeManager::Close()
{
	Clear();

	mThemeExpireTime = 0;
}

void cThemeManager::Clear()
{
	ClearThemeUI();

	cThemeInfoList::cIterator i = mThemeInfoList.Begin();
	cThemeInfoList::cIterator end = mThemeInfoList.End();
	for( ; i != end; ++i )
	{
		sPlayerThemeList* p = (sPlayerThemeList*)(*i);
		SAFE_DELETE( p );
	}
	mThemeInfoList.Clear();
}

void cThemeManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// 시간값 출력
	if( mThemeExpireTime > 0 )
	{
		unsigned long time = mThemeExpireTime / 1000;

		unsigned int oldm = ((unsigned int)time ) / 60 + 1;
		unsigned int olds = ((unsigned int)time % 60) % 60;

		// 줄이기

		if( mThemeExpireTime < deltaTime )
			mThemeExpireTime = 0;
		else
			mThemeExpireTime -= deltaTime;

		time = mThemeExpireTime / 1000;
		unsigned int newm = ((unsigned int)time ) / 60 + 1;
		unsigned int news = ((unsigned int)time % 60) % 60;

		if( newm <= 5 )
		{
			// 분단위가 바뀌었을경우.
			if( newm > 0 && oldm != newm )
			{
				cStringT Message;
				Message.Format( GAMERESOURCEMAN->GetGameText( 9039 ), newm );
				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
				CHATMANAGER->AddChatMsgEx( Message.Cstr(), COLOR_RED, infoIndex, false, true );

			}
			// 1분이상이고,
			else if( newm == 1 && olds != news )
			{
				// 30초면
				if( news == 30 )
				{
					cStringT Message;
					Message.Format( GAMERESOURCEMAN->GetGameText( 9040 ), news );
					unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
					CHATMANAGER->AddChatMsgEx( Message.Cstr(), COLOR_RED, infoIndex, false, true );
				}
				else if( news <= 10 )
				{
					cStringT Message;
					Message.Format( GAMERESOURCEMAN->GetGameText( 9040 ), news );
					unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
					CHATMANAGER->AddChatMsgEx( Message.Cstr(), COLOR_RED, infoIndex, false, true );
				}
			}
		}

	}
	else
	{
		mThemeExpireTime = 0;
	}

	// 테마던전 귀속시간
	cThemeInfoList::cIterator b = mThemeInfoList.Begin();
	cThemeInfoList::cIterator end = mThemeInfoList.End();
	for( ; b != end; )
	{
		sPlayerThemeList* p = (sPlayerThemeList*)(*b);
		if( p )
		{
			if( p->mResetLeftTime < deltaTime )
				p->mResetLeftTime = 0;
			else
				p->mResetLeftTime -= deltaTime;

			if( p->mResetLeftTime <= 0 )
			{
				// 정보 삭제
				b = mThemeInfoList.Erase( b );
				SAFE_DELETE( p );

				// UI 갱신
				ClearThemeUI();
				UpdateThemeUI();
				continue;
			}
		}

		b++;
	}
}

void cThemeManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	cStringT str;
	switch( tempMsg->Protocol )
	{
	case NM_THEME_READYQUESTION_SYN:
		{
			/// 대기중이였다면 대기 창 닫기.
			UIMAN->DestroyMsgBox( eMBEVENT_THEME_ENTER_INFO_WAIT );

			cThemeEnterWindow* pWin = GAMEUI->GetThemeEnterWindow();
			if( pWin )
				pWin->SetThemeEnterInfo( (MSG_SYN_THEME_READYQUESTION*)msg );
		}
		break;
	case NM_THEME_READY_RES: /// 테마 인던 입장 한명이라도 취소 했을 경우.
		{
			/// 대기중이였다면 대기 창 닫기.
			UIMAN->DestroyMsgBox( eMBEVENT_THEME_ENTER_WAIT );

			MSG_RES_THEME_READY *pSynMsg = (MSG_RES_THEME_READY*)msg;

			/// 잘못된 유저 알려주기. 한명이라도 반대하면 바로 받는 메시지
			int textNum = 9042;
			switch( pSynMsg->ErrorCode )
			{
			case ERROR_THEME_READY_ITEM:				textNum = 9033; break;
			case ERROR_THEME_READY_REJECTION:			textNum = 9013; break;
			case ERROR_THEME_READY_DEPENDENCY:			textNum = 9034; break;
			case ERROR_THEME_READY_SCRIPT:				textNum = 9022; break;
			case ERROR_THEME_READY_PLAYER_LEVEL:		textNum = 9035; break;
			case ERROR_THEME_READY_PLAYER_FULL:			textNum = 9024; break;
			case ERROR_THEME_READY_SERVER_NOTRUN:		textNum = 9025; break;
			case ERROR_THEME_READY_SERVER_READY:		textNum = 9032; break;
			case ERROR_THEME_READY_REQUESTDEPENDENCY:	textNum = 9027; break;
			case ERROR_THEME_READY_EXIST:				textNum = 9029; break;
			case ERROR_THEME_READY_NOT_EXIST:			textNum = 9030; break;
			case ERROR_THEME_READY_STATE:				textNum = 9036; break;
			case ERROR_THEME_READY_NOT_JOIN:			textNum = 9038; break;
			case ERROR_THEME_READY_ALREADY_JOIN:		textNum = 9041; break;
			case ERROR_THEME_READY_FULL_MAKEROOM:		textNum = 9043; break;
			}

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( textNum ), 
				GAMERESOURCEMAN->GetGameText( 9000 )
				);
		}
		break;
	case NM_THEME_HAVE_LIST_SYN: /// 테마 보유리스트
		{
			MSG_SYN_THEME_LIST* pSynMsg = (MSG_SYN_THEME_LIST*)msg;

			// 초기화
			Clear();
			
			sPlayerThemeList* list = pSynMsg->mThemeList;
			for( unsigned long i = 0; i < pSynMsg->mCount; ++i, ++list )
			{
				sPlayerThemeList* p = new sPlayerThemeList;
				*p = *list;

				mThemeInfoList.PushBack( p );
			}

			// 정렬
			if( mThemeInfoList.GetSize() > 1 )
				mThemeInfoList.Sort( ThemeSort );

			// UI갱신
			UpdateThemeUI();
		}
		break;
	case NM_THEME_DEPENDENCY_SYN:
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 9021 ) );
		}
		break;
	case NM_THEME_END_SYN:
		{
			MSGROOT msg;
			msg.Category = NM_THEME;
			msg.Protocol = NM_THEME_ENDACCEPT_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case NM_THEME_EXPIRE_TIME_SYN: /// 테마 초기화 잔여 시간
		{
			MSG_SYN_THEME_EXPIRE_TIME* pSynMsg = (MSG_SYN_THEME_EXPIRE_TIME*)msg;

			mThemeExpireTime = pSynMsg->mExpireTime;
		}
		break;
	default:
		assert(0);
		break;
	}
}

void cThemeManager::ClearThemeUI()
{
	cThemeStateWindow* stateWin = GAMEUI->GetThemeStateWindow();
	cThemeResetWindow* resetWin = GAMEUI->GetThemeResetWindow();
	if( stateWin && resetWin )
	{
		stateWin->Clear();
		resetWin->Clear();
	}
	else
		assert(0);
}

void cThemeManager::UpdateThemeUI()
{
	cThemeStateWindow* stateWin = GAMEUI->GetThemeStateWindow();
	cThemeResetWindow* resetWin = GAMEUI->GetThemeResetWindow();
	if( !(stateWin && resetWin) )
	{
		assert(0);
		return;
	}

	cThemeInfoList::cIterator b = mThemeInfoList.Begin();
	cThemeInfoList::cIterator bend = mThemeInfoList.End();
	for( ; b != bend; ++b )
	{
		sPlayerThemeList* p = (sPlayerThemeList*)(*b);
		if( p )
		{
			stateWin->AddInfo( p );
			resetWin->AddInfo( p );
		}
		else
			assert(0);
	}
}

