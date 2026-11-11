#include "StdAfx.h"
#include "ChatManager.h"
#include "ChatCommands.h"
//
#include "Protocol.h"
#include "Chat_Common.h"
#include "Player_Common.h"
//
#include "ChatWindow.h"
#include "EditBox.h"
#include "FriendWindow.h"
#include "MailWindow.h"
#include "MailSheet.h"
#include "HeroStateWindow.h"
#include "MegaphoneWindow.h"
//
#include "gameResourceManager.h"
#include "ObjectManager.h"
#include "GameUIManager.h"
#include "Hero.h"
#include "FontAgent.h"
#include "PartyManager.h"
#include "ItemManager.h"
#include "CommunityScript.h"

#ifdef _GMTOOL
#include "CheatManager.h"
#endif

cChatManager* cChatManager::mInstance = 0;

bool FriendSort( const void *arg1, const void *arg2 )
{
	sFriendData* data1 = (sFriendData*)arg1;
	sFriendData* data2 = (sFriendData*)arg2;

	if( data1->ChannelNum > data2->ChannelNum )
		return true;
	return false;
}

cChatManager::cChatManager()
: mpCommands(0)
{
	assert( mInstance == 0 && "bad singleton!" );
	mInstance = this;
}

cChatManager::~cChatManager()
{
	ClearMail();
	ClearFriend();

	mFocusArr.Clear();
	mChatWindowList.Clear();
	
	SAFE_DELETE( mpCommands );

	mInstance = 0;
}

/// 주의사항 :
bool cChatManager::Init()
{
	mpCommands = new cChatCommands;
	if( !mpCommands )
	{
		assert( 0 && "mpCommands is null" );
		return false;
	}

	/// apply community script 
	cCommunityScript::cComHashMap* pCom = COMMUNITYSCRIPT->GetComSkillMap();
	if( pCom == 0 )
		return false;

	cCommunityScript::cComHashMap::cIterator iter = pCom->Begin();
	for( ;iter != pCom->End() ; ++iter )
	{
		sCommunitySkillInfo* info = (sCommunitySkillInfo*)(*iter).mSecond;
		if( info == 0 )
			continue;

		for( unsigned int i = 0; i< 5; i++ )
		{
			if( info->mCommandIndex[i] == 0 )
				continue;

			LPCTSTR str = UIMAN->GetUIText( info->mCommandIndex[i] );
			AddCommunityData( str, info->mIndex );
		}
	}

	return true;
}

void cChatManager::Close()
{
	ClearMail();
}

void cChatManager::ClearChat()
{
	/// 채팅창 클리어
	cChatWindowList::cIterator b = mChatWindowList.Begin();
	cChatWindowList::cIterator end = mChatWindowList.End();

	cChatWindow* w = 0;

	for( ; b != end; ++b )
	{
		w = (cChatWindow*)(*b);
		if( w )
		{
			w->Clear();
		}
		else
		{
			assert(0);
		}
	}
}

void cChatManager::ClearMail()
{
	cMailDataMap::cIterator b = mMailDataMap.Begin();
	cMailDataMap::cIterator end = mMailDataMap.End();

	for( ; b != end; ++b )
	{
		POST_OPEN_RESULT* data = (POST_OPEN_RESULT*)(*b).mSecond;
		SAFE_DELETE( data );
	}

	mMailDataMap.Clear();
}

void cChatManager::ClearFriend()
{
	cFriendDataMap::cIterator b = mFriendDataMap.Begin();
	cFriendDataMap::cIterator end = mFriendDataMap.End();
	
	for( ; b != end; ++b )
	{
		sFriendData* data = (sFriendData*)(*b).mSecond;
		SAFE_DELETE( data );
	}

	mFriendDataMap.Clear();
}

/// 주의사항 : 채팅 메시지 파서 
bool cChatManager::SendChatMsg( eChatMsgKind type, LPCTSTR msg )
{
	if( msg == 0 )
		return false;

	if( !mpCommands )
	{
		assert(0);
		return false;
	}

#ifdef _GMTOOL
	/// 치트 명령을 처리
	if( CHEATMAN->Interpret( msg, ::_tcslen(msg) ) )
		return false;
#endif

	/// "/" 명령어인지 검사
	TCHAR temp[2] = {0,};
	::_tcsncpy_s( temp, &msg[0], 1 );

	/// 명령어가 있는경우 우선 처리
	if( ::_tcsncmp( temp, _T("/"), 2 ) == 0 )
	{
		if( mpCommands->MessageParser( msg ) == false )
			return false;
	}
	/// 명령어가 없는경우 타입 보고 결정
	else
	{
		if( mpCommands->MessageParserByType( (unsigned char)type, msg ) == false )
			return false;
	}
	return true;
}

unsigned long cChatManager::GetTextColor( eChatMsgKind kind )
{
	unsigned long color = COLOR_WHITE;

	switch( kind )
	{
	case eCHAT_NORMAL:		color = COLOR_WHITE;	break;
	case eCHAT_PARTY:		color = COLOR_PARTY;	break;
	case eCHAT_GUILD:		color = COLOR_GUILD;	break;
	case eCHAT_SHOUT:		color = COLOR_SHOUT;	break;
	case eCHAT_TRADE:		color = COLOR_TRADE;	break;
	case eCHAT_NOTICE:		color = COLOR_RED;		break;
	case eCHAT_WHISPER:		color = COLOR_WHISPER;	break;
	case eCHAT_GM:			color = COLOR_GM;		break;
	case eCHAT_MEGAPHONE:	color = COLOR_MEGAPHONE; break;
	case eCHAT_GUILD_NOTICE:color = COLOR_GUILD;	break;
	}

	return color;
}

unsigned long cChatManager::GetInfoTextIndex( eChatMsgKind kind )
{
	unsigned long textIndex = 0;
	switch( kind )
	{
	case eCHAT_NORMAL:		textIndex = 12700;	break;
	case eCHAT_PARTY:		textIndex = 12701;	break;
	case eCHAT_GUILD:		textIndex = 12702;	break;
	case eCHAT_TRADE:		textIndex = 12703;	break;
	case eCHAT_SHOUT:		textIndex = 12704;	break;	
	case eCHAT_MEGAPHONE:	textIndex = 12705;	break;
	case eCHAT_SYSTEM:		textIndex = 91;		break;
	case eCHAT_NOTICE:		textIndex = 92;		break;
	case eCHAT_GM:			textIndex = 180;	break;	
	}

	return textIndex;
}

/// 아이템 획득시를 위한 메세지
void cChatManager::AddItemMsg( unsigned long itemIndex, unsigned short slotIndex, LPCTSTR msg1, LPCTSTR msg2, bool IsQuest )
{
	cChatWindowList::cIterator b = mChatWindowList.Begin();
	cChatWindowList::cIterator end = mChatWindowList.End();

	cChatWindow* w = 0;

	for( ; b != end; ++b )
	{
		w = (cChatWindow*)(*b);
		if( w )
			w->InsertItemText( itemIndex, slotIndex, msg1, msg2, IsQuest );
	}
}

/// 아이템 거래 메세지
void cChatManager::AddItemMsg( sInventory inventory, LPCTSTR name, LPCTSTR msg, ePLAYER_JOB job, eChatMsgKind msgkind, void* exdata )
{
	cChatWindowList::cIterator b = mChatWindowList.Begin();
	cChatWindowList::cIterator end = mChatWindowList.End();

	cChatWindow* w = 0;

	for( ; b != end; ++b )
	{
		w = (cChatWindow*)(*b);
		if( w )
			w->InsertItemText( inventory, name, msg, job, msgkind, exdata );
	}
}

// checkPress : 버튼 눌림 체크
void cChatManager::AddChatMsg( LPCTSTR chatmsg, eChatMsgKind msgkind, bool checkPress, bool onlychatmain )
{
	if( onlychatmain == true )
	{
		cChatWindow* chat = GAMEUI->GetChatWindow();
		if( chat )
		{
			chat->InsertText( (eChatMsgKind)msgkind, GetInfoTextIndex(msgkind), chatmsg, GetTextColor(msgkind), checkPress );
		}
	}
	else
	{
		cChatWindowList::cIterator b = mChatWindowList.Begin();
		cChatWindowList::cIterator end = mChatWindowList.End();

		cChatWindow* w = 0;

		for( ; b != end; ++b )
		{
			w = (cChatWindow*)(*b);
			if( w )
			{
				w->InsertText( (eChatMsgKind)msgkind, GetInfoTextIndex(msgkind), chatmsg, GetTextColor(msgkind), checkPress );
			}
		}
	}
}

// paste : 기존 열에 붙이기 기능
// onlychat : 메인 채팅창만 출력
void cChatManager::AddChatMsgEx( LPCTSTR chatmsg, unsigned long color, unsigned long infoIndex, bool paste, bool onlychatmain, bool checkPress, eChatMsgKind msgkind )
{
	if( onlychatmain == true )
	{
		cChatWindow* chat = GAMEUI->GetChatWindow();
		if( chat )
		{
			chat->InsertText( msgkind, infoIndex, chatmsg, color, checkPress, paste );
		}
	}
	else
	{
		cChatWindowList::cIterator b = mChatWindowList.Begin();
		cChatWindowList::cIterator end = mChatWindowList.End();

		cChatWindow* w = 0;
		for( ; b != end; ++b )
		{
			w = (cChatWindow*)(*b);
			if( w )
			{
				w->InsertText( msgkind, infoIndex, chatmsg, color, checkPress, paste );
			}
		}
	}
}

///
void cChatManager::AddSystemMsg( eSystemMsgKind kind, LPCTSTR chatmsg, bool onlychatmain )
{
	if( onlychatmain == true )
	{
		cChatWindow* chat = GAMEUI->GetChatWindow();
		if( chat )
		{
			chat->InsertTextSystem( kind, chatmsg );
		}
	}
	else
	{
		cChatWindowList::cIterator b = mChatWindowList.Begin();
		cChatWindowList::cIterator end = mChatWindowList.End();

		cChatWindow* w = 0;

		for( ; b != end; ++b )
		{
			w = (cChatWindow*)(*b);
			if( w )
				w->InsertTextSystem( kind, chatmsg );
		}
	}
}

void cChatManager::AddNoticeMsg( LPCTSTR msg, unsigned long color )
{
	UIMAN->AddNoticeText( cFontAgent::eFont_Notice, msg, color, 100 );

	AddChatMsg( msg, eCHAT_NOTICE, false, true );
}

void cChatManager::AddPvPNoticeMsg( LPCTSTR msg, unsigned long color )
{
	UIMAN->AddEnhanceText( cFontAgent::eFont_System, msg, color, 160 );
	
	AddChatMsg( msg, eCHAT_NOTICE, false, true );
}

/// 주의사항 :
void cChatManager::SetChatWindow( cUIWindow* w )
{
	/// 채팅윈도우 세팅 
	if( w )
		mChatWindowList.PushBack( w );
}

void cChatManager::AddFocusNode( cUINode* n )
{
	if( n )
		mFocusArr.PushBack( n );
}

/// 주의사항 : 일대일대화창, 채팅창 간의 포커스 이동
void cChatManager::ChangeFocus()
{
	/// 전체적으로 포커스 없으면 리턴 
	if( !UIMAN->GetFocusedNode() )
		return;

	if( mFocusArr.GetSize() < 4 )
		return;

	/// 채팅창을 제외한 나머지 창이 닫혀있으면 체크 안함
	if( ((cEditBox*)mFocusArr[1])->IsVisible() == false &&
		((cEditBox*)mFocusArr[2])->IsVisible() == false &&
		((cEditBox*)mFocusArr[3])->IsVisible() == false )
		return;

	cEditBox* pEdit = 0;
	int focusIdx = -1;
	for( int i = 0; i < (int)mFocusArr.GetSize(); ++i )
	{
		pEdit = (cEditBox*)mFocusArr[i];
		if( pEdit == 0 )
		{
			assert(0);
			return;
		}

		/// 범위를 넘어가면 다시처음으로 검사
		if( focusIdx >= (int)mFocusArr.GetSize() )
		{
			focusIdx = 0;
		}

		if( pEdit->IsFocus() )	
		{
			focusIdx = i + 1;
		}

		/// 동일하면		
		if( i == focusIdx )
		{
			cEditBox* pNewEdit = (cEditBox*)mFocusArr[ focusIdx ];
            if( pNewEdit == 0 )
			{
				assert(0);
				return;
			}
			
			if( pNewEdit->IsVisible() )
			{
				/// 보이면 이놈이 포커스
				pNewEdit->SetFocus();
				return;
			}
			else
			{
				/// 안보이면 고다음으로 넘긴다
				focusIdx = i + 1;
			}
		}
	}

	/// 결국없으면 채팅창에 포커스 주기
	pEdit = (cEditBox*)mFocusArr[0];
	pEdit->SetFocus();
}

bool cChatManager::AddCommunityData( LPCTSTR key, unsigned long index )
{
	if( mpCommands->AddCommunityData( key, index ) == false )
		return false;
	return true;
}

// 우편 오픈 요청
void cChatManager::SendPostOpen()
{

}

/// 주의사항 :
void cChatManager::NetworkMsgParser( char* pMsg, u_long length )
{
	cStringT Message;
	MSGROOT* pTemMsg = (MSGROOT*)pMsg;

	switch( pTemMsg->Protocol )
	{
	case NM_CHAT_NORMAL_RES:
		{
			MSG_RES_CHAT* pmsg = (MSG_RES_CHAT*)pMsg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_SUCCESS:
				break;
			case ERROR_CHAT_BLOCK:		/// 채팅 블럭
				{
					struct tm validThru = *localtime( &pmsg->validTrue );
					Message.Format( GAMERESOURCEMAN->GetGameText( 69 )
								   ,validThru.tm_year+1900
								   ,validThru.tm_mon+1
								   ,validThru.tm_mday
								   ,validThru.tm_hour
								   ,validThru.tm_min
								   ,validThru.tm_sec );
					AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
				}
				break;
			case ERROR_CHAT_FAIL:		/// 채팅 실패
				break;
			}
		}
		break;
	case NM_CHAT_NORMAL_SYN:
		{
			MSG_SYN_CHAT* pmsg = (MSG_SYN_CHAT*)pMsg;

			pMsg[ length ] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->Job, eCHAT_NORMAL );
			
			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->CharacterIdx );
			if( p && ::_tcslen(pmsg->Message) > 0 )
			{
				p->ActiveChatBubble( pmsg->Message, (unsigned long)0xFFFCFF9E );

				if( pmsg->emotionIdx != 0 )
					p->SetEmotionFace( pmsg->emotionIdx );
			}
		}
		break;
	case NM_CHAT_SHOUT_RES:
		break;
	case NM_CHAT_SHOUT_SYN:
		{
			MSG_SYN_CHAT_SHOUT* pmsg = (MSG_SYN_CHAT_SHOUT*)pMsg;

			pMsg[ length ] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SHOUT);	

			unsigned long nameColor = COLOR_WHITE;
			ePLAYER_JOB nJobType = (ePLAYER_JOB)pmsg->Job;

			if( nJobType != ePLAYER_NONEJOB )
			{
				if(	ePLAYER_FIGHTER			<= nJobType && nJobType <= ePLAYER_SOULBLADER )		nameColor = COLOR_JOB_FIGHTER; 
				else if( ePLAYER_MAGE		<= nJobType && nJobType <= ePLAYER_HIGHPRIEST )		nameColor = COLOR_JOB_MAGE; 
				else if( ePLAYER_WANDERER	<= nJobType && nJobType <= ePLAYER_SNIPER )			nameColor = COLOR_JOB_WANDERER; 
				else if( ePLAYER_WARLOCK	<= nJobType && nJobType <= ePLAYER_MENTALIST )		nameColor = COLOR_JOB_WARLOCK; 
				else if( ePLAYER_ARMSMAN	<= nJobType && nJobType <= ePLAYER_HIGHLANDER )		nameColor = COLOR_JOB_ARMSMAN; 
				else if( ePLAYER_RANGER		<= nJobType && nJobType <= ePLAYER_SHARPSHOOTER )	nameColor = COLOR_JOB_RANGER;
			}

			Message.Format( GAMERESOURCEMAN->GetGameText( 12706 ), pmsg->Message );

			AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 12707 ), COLOR_SHOUT, infoIndex, false, false, true, eCHAT_SHOUT );			
			AddChatMsgEx( (LPCTSTR)pmsg->Name, nameColor, 0, true, false, true, eCHAT_SHOUT );
			AddChatMsgEx( (LPCTSTR)Message.Cstr(), COLOR_SHOUT, 0, true, false, true, eCHAT_SHOUT );			
		}	
		break;
	case NM_CHAT_TRADE_SYN:
		{
			MSG_SYN_CHAT_TRADE* pmsg = (MSG_SYN_CHAT_TRADE*)pMsg;

			pMsg[ length ] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;			

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->Job, eCHAT_TRADE );
		}
		break;
	case NM_CHAT_WHISPER_RES:
		{
			MSG_RES_CHAT_WHISPER* pmsg = (MSG_RES_CHAT_WHISPER*)pMsg;

			pMsg[ length ] = 0;

			switch ( pmsg->ErrorCode )
			{
			case ERROR_CHAT_WHISPER_SUCCESS:
				{
					pmsg->Name[MAX_NAME_SIZE] = 0;
					bool bRecv = false ;
					AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->Job, eCHAT_WHISPER, (LPVOID)bRecv );
				}				
				break;
			case ERROR_CHAT_WHISPER_CHARACTER_NAME:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 24 ) );
				break;	
			case ERROR_CHAT_WHISPER_DISCONNECT:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 25 ) );
				break;	
			case ERROR_CHAT_WHISPER_NO_MESSAGE:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 28 ) );
				break;	
			case ERROR_CHAT_WHISPER_INPUT_TIME:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 26 ) );
				break;
			case ERROR_CHAT_WHISPER_MYSELF:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 27 ) );
				break;
			case ERROR_CHAT_WHISPER_OPTIONREFUSE:
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2704 ) );
				break;
			}
		}
		break;
	case NM_CHAT_WHISPER_SYN:
		{
			MSG_SYN_CHAT_WHISPER* pmsg = (MSG_SYN_CHAT_WHISPER*)pMsg;

			pMsg[ length ] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;
			
			bool bRecv = true;
			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->Job, eCHAT_WHISPER, (LPVOID)bRecv );

			cChatWindow* win = GAMEUI->GetChatWindow();
			if( win )
				win->AddWhisperUser( pmsg->Name );
		}
		break;
		/// Friend
	case NM_CHAT_FRIEND_LIST_RES:
		{
			if( !GAMEUI )
			{
				assert(0);
				return;
			}

			// 초기화
			cFriendWindow* pFriend = GAMEUI->GetFriendWindow();
			if( pFriend )
				pFriend->Clear();
			else
				assert(0);
			
			ClearFriend();

			// 전체 리스트 셋팅
			MSG_RES_FRIEND_LIST* msg = (MSG_RES_FRIEND_LIST*)pMsg;
			
			for( long i = 0; i < msg->RowCount; ++i )
			{
				sFriendData* p		= new sFriendData;
				p->Idx				= msg->table[i].Idx;
				p->CharacterIdx		= msg->table[i].CharacterIdx;
				wcscpy( p->CharacterName, msg->table[i].CharacterName );
				p->Race				= msg->table[i].Race;
				p->Gender			= msg->table[i].Gender;
				p->Level			= msg->table[i].Level;
				p->Job				= msg->table[i].Job;
				p->MapNum			= msg->table[i].MapNum;
				p->Status			= msg->table[i].Status;
				p->ChannelNum		= msg->table[i].ChannelNum;
				p->Block			= msg->table[i].Block;

				if( mFriendDataMap.Insert( p->CharacterIdx, p ) == false )
				{
					assert(0);
					SAFE_DELETE( p );
				}
			}

			// 접속 소팅
			typedef tList<void*> cSortList;
			cSortList list;

			cFriendDataMap::cIterator i = mFriendDataMap.Begin();
			cFriendDataMap::cIterator end = mFriendDataMap.End();
			for( ; i != end; ++i )
			{
				sFriendData* p = (sFriendData*)(i->mSecond);
				list.PushBack( p );
			}

			if( list.GetSize() > 1 )
				list.Sort( FriendSort );

			// UI 셋팅
			cSortList::cIterator li = list.Begin();
			cSortList::cIterator lend = list.End();
			for( ; li != lend; ++li )
			{
				sFriendData* p = (sFriendData*)(*li);
				
				if( pFriend )
					pFriend->AddFriend( p );
			}
		}
		break;
	case NM_CHAT_FRIEND_CONNECT_SYN:
		{
			MSG_SYN_FRIEND_CONNECT* msg = (MSG_SYN_FRIEND_CONNECT*)pMsg;
			
			sFriendData* pPlayer = GetFriendData( msg->characterIndex );
			if( !pPlayer )
			{
				assert(0);
				return;
			}

			// 정보 갱신
			pPlayer->ChannelNum = msg->channelNum;
			pPlayer->Level		= msg->level;
			pPlayer->Job		= msg->job;
			pPlayer->MapNum		= msg->mapNum;
			
			// UI 초기화
			cFriendWindow* pFriend = GAMEUI->GetFriendWindow();
			if( pFriend )
				pFriend->Clear();
			else
				assert(0);

			// 접속 소팅
			typedef tList<void*> cSortList;
			cSortList list;

			cFriendDataMap::cIterator i = mFriendDataMap.Begin();
			cFriendDataMap::cIterator end = mFriendDataMap.End();
			for( ; i != end; ++i )
			{
				sFriendData* p = (sFriendData*)(i->mSecond);
				list.PushBack( p );
			}

			if( list.GetSize() > 1 )
				list.Sort( FriendSort );

			// UI 셋팅
			cSortList::cIterator li = list.Begin();
			cSortList::cIterator lend = list.End();
			for( ; li != lend; ++li )
			{
				sFriendData* p = (sFriendData*)(*li);

				if( pFriend )
					pFriend->AddFriend( p );
			}

			// 알림
			cStringT str;
			unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);				

			AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 11102 ), COLOR_WHITE, infoIndex, false, true );
			AddChatMsgEx( (LPCTSTR)pPlayer->CharacterName, COLOR_FRIEND_EX, 0, true, true );

			// 접속 하였을 때
			if( msg->connect )
			{					
				AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 11103 ), COLOR_WHITE, 0, true, true );
			}
			// 접속 종료 하였을 때
			else
			{
				AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 11104 ), COLOR_WHITE, 0, true, true );
			}			
		}
		break;
	case NM_CHAT_FRIEND_ADD_RES:
		{
			MSG_RES_FRIEND_ADD* pRes = (MSG_RES_FRIEND_ADD*)pMsg;

			unsigned long textIdx = 0;
			switch( pRes->ErrorCode )
			{
			case ERROR_CHAT_FRIEND_SUCCESS:			textIdx = 10100; break;
			case ERROR_CHAT_FRIEND_CHARACTER_NAME:  textIdx = 10102; break;
			case ERROR_CHAT_FRIEND_OVER1:			textIdx = 10103; break;
			case ERROR_CHAT_FRIEND_OVER2:		    textIdx = 10104; break;
			case ERROR_CHAT_FRIEND_UNKNOWN:         textIdx = 10101; break;
			default:	assert(0); return;
			}

			AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIdx ) );
		}
		break;
	case NM_CHAT_FRIEND_DEL_RES:
		{
			AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 32 ) );
		}
		break;
	case NM_CHAT_PARTY_SYN:
		{			
			/// 파티 대화
			MSG_SYN_CHAT_PARTY* pmsg = (MSG_SYN_CHAT_PARTY*)pMsg;

			pMsg[length] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->job, eCHAT_PARTY );
						
			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->playerIndex );
			if( p && ::_tcslen(pmsg->Message) > 0 )
			{
				p->ActiveChatBubble( pmsg->Message, COLOR_PARTY );
			}
		}
		break;
	case NM_CHAT_PARTYUNION_SYN:
		{
			/// 파티 연합 대화
			MSG_SYN_CHAT_PARTYUNION* pmsg = (MSG_SYN_CHAT_PARTYUNION*)pMsg;

			pMsg[length] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->job, eCHAT_PARTYUNION, (void*)pmsg->playerIndex );

			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->playerIndex );
			if( p && ::_tcslen(pmsg->Message) > 0 )
			{
				p->ActiveChatBubble( pmsg->Message, COLOR_PARTY );
			}
		}
		break;
	case NM_CHAT_GUILD_SYN:
		{
			/// 길드 대화
			MSG_SYN_CHAT_GUILD* pmsg = (MSG_SYN_CHAT_GUILD*)pMsg;

			pMsg[length] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->job, eCHAT_GUILD );
		}
		break;

	case NM_CHAT_GM_NORMAL_RES:
		break;
	case NM_CHAT_GM_NORMAL_SYN:
		{
			MSG_SYN_CHAT* pmsg = (MSG_SYN_CHAT*)pMsg;

			pMsg[length] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			AddItemMsg( pmsg->inventory, pmsg->Name, pmsg->Message, ePLAYER_NONEJOB, eCHAT_GM );
			
			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->CharacterIdx );
			if( p && ::_tcslen(pmsg->Message) > 0 )
			{
				p->ActiveChatBubble( pmsg->Message, COLOR_GM );

				if( pmsg->emotionIdx != 0 )
					p->SetEmotionFace( pmsg->emotionIdx );
			}
		}
		break;

	case NM_CHAT_ENHANCED_SYN:
		{
			MSG_SYN_CHAT_SYSTEM* pmsg = (MSG_SYN_CHAT_SYSTEM*)pMsg;
			unsigned int len = ::_tcslen(pmsg->characterName);
			
			if( len == 0 )
			{
				cChatWindow* chatWin = GAMEUI->GetChatWindow();
				if( chatWin )
				{
					chatWin->InsertItemText( pmsg->inventory, NULL, GAMERESOURCEMAN->GetGameText( 12601 ), (ePLAYER_JOB)ePLAYER_NONEJOB, eCHAT_NOTICE );
				}
			}
			else
			{
				pmsg->characterName[len] = 0;
				Message.Format( GAMERESOURCEMAN->GetGameText( 12600 ), pmsg->characterName );
				cChatWindow* chatWin = GAMEUI->GetChatWindow();
				if( chatWin )
				{
					chatWin->InsertItemText( pmsg->inventory, pmsg->characterName, (LPCTSTR)Message.Cstr(), (ePLAYER_JOB)ePLAYER_NONEJOB, eCHAT_NOTICE );
				}
			}
		}
		break;
	case NM_CHAT_UNIQUEITEM_SYN:
		{
			MSG_SYN_CHAT_SYSTEM* pmsg = (MSG_SYN_CHAT_SYSTEM*)pMsg;

			cChatWindow* chatWin = GAMEUI->GetChatWindow();
			if( chatWin )
			{
				chatWin->InsertItemText( pmsg->inventory, pmsg->characterName, GAMERESOURCEMAN->GetGameText( 12602 ), (ePLAYER_JOB)ePLAYER_NONEJOB, eCHAT_NOTICE );
			}
		}
		break;
	case NM_CHAT_POST_OPEN_RES:
		{
			// 우편열기 결과
			MSG_RES_CHAT_POST_OPEN* pmsg = (MSG_RES_CHAT_POST_OPEN*)pMsg;

			cMailWindow* mail = GAMEUI->GetMailWindow();
			if( !mail )
			{
				assert(0);
				return;
			}

			cMailListSheet* sheet = mail->GetListSheet();
			if( !sheet )
			{
				assert(0);
				return;
			}

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_POST_OPEN_SUCCESS:
				{
					sheet->Clear();
					ClearMail();

					bool isNewMail = false;
					POST_OPEN_RESULT* data = pmsg->Result;
					for( long i = 0; i < pmsg->RowCount; ++i, ++data )
					{
						POST_OPEN_RESULT* p = new POST_OPEN_RESULT;
						*p = *data;

						if( mMailDataMap.Insert( data->idx, p ) )
						{
							sheet->AddMail( data );

							if( data->status == 0 )
								isNewMail = true;
						}
						else
						{
							assert(0);
							SAFE_DELETE( p );
						}
					}

					sheet->SetTotalPage( pmsg->endPage );
					sheet->ChangePage( pmsg->startPage );

					cHeroStateWindow* pHeroStateWin = GAMEUI->GetHeroStateWindow();
					if( pHeroStateWin )
						pHeroStateWin->ShowNewMail( ( pmsg->noneRead > 0 ) );
				}
				break;
			case ERROR_CHAT_POST_OPEN_FAIL:
				{
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 771 ) );

					//
					HERO->SetState( eOBJECT_STATE_IDLE );
				}
				break;
			}

			sheet->NetUnLock();
		}
		break;
	case NM_CHAT_POST_CLOSE_RES:
		{
			MSG_RES_CHAT_POST_CLOSE* pmsg = (MSG_RES_CHAT_POST_CLOSE*)pMsg;

			if( pmsg->ErrorCode == ERROR_CHAT_POST_CLOSE_FAIL )
			{
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 780 ) );
			}
		}
		break;
	case NM_CHAT_POST_READ_RES:
		{
			MSG_RES_CHAT_POST_READ* pmsg = (MSG_RES_CHAT_POST_READ*)pMsg;

			if( pmsg->ErrorCode == ERROR_CHAT_POST_READ_FAIL )
			{
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 779 ) );
			}
			else
			{
				cHeroStateWindow* pHeroStateWin = GAMEUI->GetHeroStateWindow();
				if( pHeroStateWin )
					pHeroStateWin->ShowNewMail( ( pmsg->noneRead > 0 ) );
			}
		}
		break;
	case NM_CHAT_POST_WRITE_RES:
		{
			// 쓰기
			MSG_RES_CHAT_POST_WRITE* pmsg = (MSG_RES_CHAT_POST_WRITE*)pMsg;

			cMailWindow* mail = GAMEUI->GetMailWindow();
			if( !mail )
			{
				assert(0);
				return;
			}
			
			cMailSendSheet* sheet = mail->GetSendSheet();
			if( !sheet )
			{
				assert(0);
				return;
			}

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_POST_WRITE_SUCCESS:
				{
					sheet->DeleteItemBySend();

					// 성공
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 772 ) );
				}
				break;
			case ERROR_CHAT_POST_WRITE_FAIL:
				// 실패
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 773 ) );
				break;
			case ERROR_CHAT_POST_WRITE_NOT_EXISTS:
				// 실패-존재하지 않는 캐릭터
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 774 ) );
				break;
			case ERROR_CHAT_POST_WRITE_UNKNOW:
				// 실패-알수없는 오류(DB)
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 773 ) );
				break;
			case ERROR_CHAT_POST_WRITE_TRADE_MONEY:
				//우편으로 돈을 최대 거래 금액 이상 보내려고 할때!
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11206 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
            case ERROR_CHAT_POST_WRITE_EXCHANGE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 2019 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
                break;
			}

			sheet->NetUnLock();

			ITEMMAN->UnlockItemTry( ITEM_TRY_MAILSEND );
		}
		break;
	case NM_CHAT_POST_DOWNLOAD_RES:
		{
			// 아이템 받기
			MSG_RES_CHAT_POST_DOWNLOAD* pmsg = (MSG_RES_CHAT_POST_DOWNLOAD*)pMsg;

			cMailWindow* mail = GAMEUI->GetMailWindow();
			if( !mail )
			{
				assert(0);
				return;
			}

			cMailReadSheet* sheet = mail->GetReadSheet();
			if( !sheet )
			{
				assert(0);
				return;
			}

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_POST_DOWNLOAD_SUCCESS:
				{
					sheet->DownLoadData( pmsg );
				}
				break;
			case ERROR_CHAT_POST_DOWNLOAD_FAIL:
				{
					// 다운로드 실패
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 781 ) );
				}
				break;
			}

			sheet->NetUnLock();

			if( ITEMMAN->GetItemTry() == ITEM_TRY_MAILDOWNLOAD )
			{
				ITEMMAN->UnlockItemTry( ITEM_TRY_MAILDOWNLOAD );
			}
		}
		break;
	case NM_CHAT_POST_DELETE_RES:
		{
			// 삭제
			MSG_RES_CHAT_POST_DELETE* pmsg = (MSG_RES_CHAT_POST_DELETE*)pMsg;

			cMailWindow* mail = GAMEUI->GetMailWindow();
			if( !mail )
			{
				assert(0);
				return;
			}
			
			cMailListSheet* sheet = mail->GetListSheet();
			if( !sheet )
			{
				assert(0);
				return;
			}

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_POST_DELETE_SUCCESS:
				{
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 783 ) );

					//
					sheet->Clear();
					ClearMail();

					bool isNewMail = false;
					POST_OPEN_RESULT* data = pmsg->Result;
					for( long i = 0; i < pmsg->RowCount; ++i, ++data )
					{
						POST_OPEN_RESULT* p = new POST_OPEN_RESULT;
						*p = *data;

						if( mMailDataMap.Insert( data->idx, p ) )
						{
							sheet->AddMail( data );

							if( data->status == 0 )
								isNewMail = true;
						}
						else
						{
							assert(0);
							SAFE_DELETE( p );
						}
					}

					sheet->SetTotalPage( pmsg->endPage );
					sheet->ChangePage( pmsg->startPage );

					mail->SetCurrent( 0 );

					cHeroStateWindow* pHeroStateWin = GAMEUI->GetHeroStateWindow();
					if( pHeroStateWin )
						pHeroStateWin->ShowNewMail( ( pmsg->noneRead > 0 ) );
				}
				break;
			case ERROR_CHAT_POST_DELETE_FAIL:
				{
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 784 ) );
				}
				break;
			}

			//
			cMailReadSheet* readSheet = mail->GetReadSheet();
			if( readSheet )
				readSheet->NetUnLock();

			sheet->NetUnLock();
		}
		break;
	case NM_CHAT_POST_RECEIVED_SYN:
		{
			// 새 메일 표시
			cHeroStateWindow* state = GAMEUI->GetHeroStateWindow();
			if( state )
			{
				bool isSet = state->ShowNewMail( true );
				if( isSet )
					AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 770 ) );
			}
		}
		break;
	case NM_CHAT_MEGAPHONE_RES:
		{
			MSG_RES_CHAT_MEGAPHONE* pmsg = (MSG_RES_CHAT_MEGAPHONE*)pMsg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_CHAT_MEGAPHONE_SUCCESS:
				break;
			case ERROR_CHAT_MEGAPHONE_BLOCK:	// 채팅 금지 적용중
				{
					struct tm validThru = *localtime( &pmsg->validTrue );
					Message.Format( GAMERESOURCEMAN->GetGameText( 69 )
						,validThru.tm_year+1900
						,validThru.tm_mon+1
						,validThru.tm_mday
						,validThru.tm_hour
						,validThru.tm_min
						,validThru.tm_sec );
					AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
				}
				break;
			case ERROR_CHAT_MEGAPHONE_COOLTIME:	// 쿨타임 적용중
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12003 ) );
				break;
			case ERROR_CHAT_MEGAPHONE_FAIL:		// 실패
				AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12003 ) );
				break;
			}

			cUIWindow* win = (cUIWindow*)GAMEUI->GetMegaphoneWindow();
			if( win )
				win->NetUnLock();
		}
		break;
	case NM_CHAT_MEGAPHONE_SYN:
		{
			MSG_SYN_CHAT_MEGAPHONE* pmsg = (MSG_SYN_CHAT_MEGAPHONE*)pMsg;
			
			pMsg[ length ] = 0;
			pmsg->Name[MAX_NAME_SIZE] = 0;

			// 12000	"[확성기][%s]:"
			AddItemMsg( pmsg->Inventory, pmsg->Name, pmsg->Message, (ePLAYER_JOB)pmsg->Job, eCHAT_MEGAPHONE );

			if( pmsg->CharacterIdx == HERO->GetObjectID() )
			{
				cMegaphoneWindow* win = (cMegaphoneWindow*)GAMEUI->GetMegaphoneWindow();
				if( win )
				{
					// 아이템 쿨타임 적용
					win->NetUnLock();
					win->StartCoolTime();
					win->Hide();
				}
			}
		}
		break;
	case NM_CHAT_AUTODUMMY_SYN:
		{
			// 오토더미 공격시
			unsigned long color = 0xFFFFFFFF;

			unsigned long colorR = rand() % 255;
			unsigned long colorG = rand() % 255;
			unsigned long colorB = rand() % 255;

			color = (((BYTE)(colorR)|((WORD)((BYTE)(colorG))<<8))|(((DWORD)(BYTE)(colorB))<<16)) | 0xFF000000;

			//
			unsigned long height = rand() % 300 + 150;
			
			UIMAN->AddDummyNoticeText( cFontAgent::eFont_System, GAMERESOURCEMAN->GetGameText( 14001 ), color, height );
		}
		break;
	}
}

POST_OPEN_RESULT* cChatManager::GetMailData( long idx )
{
	return (POST_OPEN_RESULT*)mMailDataMap.GetAt( idx );
}

void cChatManager::CheckNewMail()
{
	// 아직 읽지 않은 메일이 있는지 체크
	bool search = false;
	cMailDataMap::cIterator i = mMailDataMap.Begin();
	cMailDataMap::cIterator end = mMailDataMap.End();
	for( ; i != end; ++i )
	{
		POST_OPEN_RESULT* p = (POST_OPEN_RESULT*)(i->mSecond);
		if( p && p->status == 0 )
		{
			search = true;
			break;
		}
	}

	cHeroStateWindow* state = GAMEUI->GetHeroStateWindow();
	if( state )
		state->ShowNewMail( search );
}
