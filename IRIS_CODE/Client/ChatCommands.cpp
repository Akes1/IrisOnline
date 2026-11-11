#include "StdAfx.h"
#include ".\chatcommands.h"
#include "ChatManager.h"
#include "Hero.h"
#include "ObjectManager.h"
#include "gameResourceManager.h"
#include "FilterManager.h"
#include "GuildManager.h"
//
#include "Protocol.h"
#include "Chat_Common.h"

//
#include "CommunityScript.h"
#include "ChatWindow.h"
#include "SkillWindow.h"
#include "MegaphoneWindow.h"
//
#include "PVPManager.h"

cChatCommands::cChatCommands()
{
}

cChatCommands::~cChatCommands()
{
}

bool cChatCommands::AddCommunityData( LPCTSTR key, unsigned long index )
{
	if( mCommunityCommandMap.Insert( key, index ) == false )
	{
		assert( 0 && "failed to add community command string" );
		return false;
	}
	return true;
}

bool cChatCommands::MessageParser( LPCTSTR message )
{
	/// 메세지 파싱
	cTokenizerT tokenizer( message, ::_tcslen(message), _T(" ") );

	cStringT keyToken, tailToken;
	tokenizer.GetNext( &keyToken );

	if( tokenizer.GetTail() )
		tailToken = tokenizer.GetTail();

	keyToken.ToLower();

	/// 감정표현검사
	cCommunityCommandMap::cIterator i = mCommunityCommandMap.Find( (LPCTSTR)keyToken.Cstr() );
	if( i != mCommunityCommandMap.End() )
	{
		/// 기능인덱스 구하기
		unsigned long index = (*i).mSecond;
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( index );
		if( pInfo )
		{
			if( pInfo->mType == 0 )
			{
				cSkillWindow* win =  (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
				if( win )
				{
					switch( pInfo->mRoleIndex )
					{
					case eCOMMU_LIFE_REST:
					case eCOMMU_LIFE_DUEL:
					case eCOMMU_LIFE_STREETSTALL:
					case eCOMMU_LIFE_ENHANCED:
					case eCOMMU_LIFE_TITLE:
					case eCOMMU_LIFE_ITEMMIX:
					case eCOMMU_LIFE_FOLLOW:
					case eCOMMU_LIFE_REPORT_AUTO:
						{
							if( tailToken.GetLength() <= 0 )
								win->RequestLifeCommand( pInfo->mIndex, 0, 0 );
							else
								CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 57 ) );
						}
						break;
					case eCOMMU_LIFE_PARTY: /// /파티
					case eCOMMU_LIFE_TRADE:
						{
							LPCTSTR name = 0;
							if( tailToken.GetLength() > 0 )
							{
								///  + ID 인지 검사
								cTokenizerT token( tailToken.Cstr(), tailToken.GetLength(), _T(" ") );
								cStringT IDToken, endToken;
								token.GetNext( &IDToken );
								
								if( token.GetTail() )
									endToken = token.GetTail();

								/// id 뒤에 쓸데 없는 구문이 붙지 않도록 한다.
								if( endToken.GetLength() > 0 )
								{
									/// 잘못된 명령어
									CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 57 ) );
									return true;
								}

								name = (LPCTSTR)IDToken.Cstr();
							}
							win->RequestLifeCommand( pInfo->mIndex, name, 0 );
						}
						break;
					case eCOMMU_LIFE_PUTCARD:
					case eCOMMU_LIFE_TAROT:
						{
							win->RequestLifeCommand( pInfo->mIndex, 0, 0 );
						}
						break;
					}
				}
				return true;
			}
			else
			{
				/// emotion command
				if( HERO->GetState() == eOBJECT_STATE_IDLE )
				{
					/// network msg
					cSkillWindow* win =  (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
					if( win )
						win->RequestCommuSkill( pInfo->mIndex );
				}
				return true;
			}
		}
	}

	/// 채팅명령어 검사
	/// 귓말 키워드 /W, /w, /ㅈ
	if( !keyToken.CompareNoCase( UIMAN->GetUIText(10002) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10004) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			/// 명령어만 입력했을시 자동 선택 사항 바뀜
			cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
			if( chatWin )
				chatWin->UpdateChatType( eCHAT_WHISPER );
		}
		else
		{
			/// 아이디 파싱 
			cTokenizerT token( tailToken.Cstr(), tailToken.GetLength(), _T(" ") );

			cStringT IDToken, msgToken;
			token.GetNext( &IDToken );

			if( token.GetTail() )
				msgToken = token.GetTail();

			if( msgToken.GetLength() == 0 )
			{
				cStringT Message;
				Message.Format( GAMERESOURCEMAN->GetGameText( 28 ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
			}	
			else
			{
				/// send
				SendWhisperMsg( IDToken.Cstr(), msgToken.Cstr() );

				/// 귓속말 유저 아이디 등록 테스트
				cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
				if( chatWin )
					chatWin->AddWhisperUser( IDToken );

			}	
		}
	}
	/// 파티 키워드 /P, /p, /ㅔ
	else if(!keyToken.CompareNoCase( UIMAN->GetUIText(10011) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10012) ) )
	{
		if( HERO->GetPartyIndex() > 0 || HERO->GetPartyUnionIndex() > 0 )
		{
			if( tailToken.GetLength() == 0 )
			{
				/// 명령어만 입력했을시 자동 선택 사항 바뀜
				cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
				if( chatWin )
					chatWin->UpdateChatType( eCHAT_PARTY );
			}
			else
			{
				SendPartyMsg( (LPCTSTR)tailToken.Cstr() );
			}
		}
	}
	/// 외치기 키워드 /Y, /y, /ㅛ
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10021) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10022) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			/// 명령어만 입력했을시 자동 선택 사항 바뀜
			cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
			if( chatWin )
				chatWin->UpdateChatType( eCHAT_SHOUT );
		}
		else
		{
			SendShoutMsg( (LPCTSTR)tailToken.Cstr() );
		}
	}
	/// 길드 키워드 /G, /g, /ㅎ
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10031) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10032) ) )
	{
		if( HERO->GetGuildIndex() > 0 )
		{
			if( tailToken.GetLength() == 0 )
			{
				/// 명령어만 입력했을시 자동 선택 사항 바뀜
				cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
				if( chatWin )
					chatWin->UpdateChatType( eCHAT_GUILD );
			}
			else
			{
				SendGuildMsg( (LPCTSTR)tailToken.Cstr() );
			}
		}
	}
	/// 일반 키워드 /N, /n, /ㅜ
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10006) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10007) ) )
	{
		/// 일반 메시지 출력
		assert( HERO );

		if( tailToken.GetLength() == 0 )
		{
			/// 명령어만 입력했을시 자동 선택 사항 바뀜
			cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
			if( chatWin )
				chatWin->UpdateChatType( eCHAT_NORMAL );
		}
		else
		{
			SendNormalMsg( (LPCTSTR)tailToken.Cstr() );
		}
	}
	/// 거래 키워드 /T, /t, /ㅅ
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10008) ) || !keyToken.CompareNoCase( UIMAN->GetUIText(10009) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			/// 명령어만 입력했을시 자동 선택 사항 바뀜
			cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
			if( chatWin )
				chatWin->UpdateChatType( eCHAT_TRADE );
		}
		else
		{
			SendTradeMsg( (LPCTSTR)tailToken.Cstr() );
		}
	}
	/// "/포기"
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10131) ) )
	{
		HERO->DuelGiveUp();
	}
	/// "/표정"
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10141) ) )
	{
		///
		if( tailToken.GetLength() <= 0 )
		{
			/// 잘못된 명령어
			HERO->SetStaticFaceReq( 0 );
		}
		else
		{
			/// 보통 check
			cTokenizerT token( tailToken.Cstr(), tailToken.GetLength(), _T(" ") );
			cStringT keyToken, endToken;
			token.GetNext( &keyToken );

			if( token.GetTail() )
				endToken = token.GetTail();

			/// id 뒤에 쓸데 없는 구문이 붙지 않도록 한다.
			if( endToken.GetLength() <= 0 )
			{
				///
				if( !keyToken.CompareNoCase( UIMAN->GetUIText(10142) ) )
				{
					/// static face = 0
					HERO->SetStaticFaceReq( 0 );
				}
				else
				{
					unsigned long faceIdx = FILTERMAN->GetEmontionIndex( keyToken.Cstr() );
					if( faceIdx != 0 )
						HERO->SetStaticFaceReq( faceIdx );
				}
			}
			else
			{
				/// 잘못된 명령어
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 57 ) );
			}
		}
	}
	/// "/길드"
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10171) ) || 
		     !keyToken.CompareNoCase( UIMAN->GetUIText(10172) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			GUILDMAN->GuildAdd( _T("") );
		}
		else
		{
			/// 보통 check
			cTokenizerT token( tailToken.Cstr(), tailToken.GetLength(), _T(" ") );
			cStringT IDToken, endToken;
			token.GetNext( &IDToken );

			if( token.GetTail() )
				endToken = token.GetTail();

			/// id 뒤에 쓸데 없는 구문이 붙지 않도록 한다.
			if( endToken.GetLength() <= 0 )
			{
				if( IDToken.GetLength() > 0 )
				{
					GUILDMAN->GuildAdd( (LPCTSTR)IDToken.Cstr() );
				}
				else
				{
					GUILDMAN->GuildAdd( _T("") );
				}
			}
			else
			{
				/// 잘못된 명령어
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
			}

		}
	}
	/// "/전장신고"
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10190) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			PVPMAN->RequestReport( _T("") );
		}
		else
		{
			cTokenizerT token( tailToken.Cstr(), tailToken.GetLength(), _T(" ") );
			cStringT IDToken, endToken;
			token.GetNext( &IDToken );

			if( token.GetTail() )
				endToken = token.GetTail();

			/// id 뒤에 쓸데 없는 구문이 붙지 않도록 한다.
			if( endToken.GetLength() <= 0 )
			{
				if( IDToken.GetLength() > 0 )
				{
					PVPMAN->RequestReport( (LPCTSTR)IDToken.Cstr() );
				}
				else
				{
					PVPMAN->RequestReport( _T("") );
				}
			}
			else
			{
				/// 잘못된 명령어
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 871 ) );
			}
		}
	}
	/// "/살펴보기"
	else if( !keyToken.CompareNoCase( UIMAN->GetUIText(10195) ) )
	{
		if( tailToken.GetLength() == 0 )
		{
			HERO->RequestLookPlayer( 0 );
		}
		else
		{
			/// 잘못된 명령어
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 880 ) );
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool cChatCommands::MessageParserByType( unsigned char type, LPCTSTR message )
{
	unsigned int msgLen = ::_tcslen(message);

	/// 일반
	if( type == eCHAT_NORMAL )
	{
		/// 일반 메시지 출력
		assert( HERO );

#ifdef _GMTOOL
		SendGMNormalMsg( message );
#else 
		SendNormalMsg( message );
#endif
		
	}
	/// 파티
	else if( type == eCHAT_PARTY )
	{
		if( HERO->GetPartyIndex() > 0 || HERO->GetPartyUnionIndex() > 0 )
		{
			SendPartyMsg( message );
		}
		else
		{
			/// 파티 없음
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 114 ) ); 
		}

	}
	/// 길드
	else if( type == eCHAT_GUILD )
	{
		if( HERO->GetGuildIndex() > 0 )
			SendGuildMsg( message );
		else
		{
			/// 길드 없음
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) ); 
		}
	}
	/// 외치기
	else if( type == eCHAT_SHOUT )
	{
		SendShoutMsg( message );
	}
	/// 거래
	else if( type == eCHAT_TRADE )
	{
		SendTradeMsg( message );
	}
	/// 귓말
	else if( type == eCHAT_WHISPER )
	{
		cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
		if( !chatWin )
			return false;

		cStringT Message;
		if( chatWin->IsLinked() == false )
		{
			if( msgLen == 0 )
			{
				Message.Format( GAMERESOURCEMAN->GetGameText( 28 ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
				return false;
			}
		}

		LPCTSTR toName = chatWin->GetWhisperName();	
		unsigned int toLen = ::_tcslen(toName);
		if( toLen == 0 )
		{
			Message.Format( GAMERESOURCEMAN->GetGameText( 30 ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		}
		else
		{
			SendWhisperMsg( toName, message );
			chatWin->AddWhisperUser( toName );
		}
	}
	/// 메가폰
	else if( type == eCHAT_MEGAPHONE )
	{
		if( SendMegaphoneMsg( message ) == false )
			return false;
	}
	else
	{
		return false;
	}

	return true;
}


bool cChatCommands::CheckChatMsg( LPCTSTR chatmsg, unsigned int len )
{
	if( len <= 0 )
		return false;

	/// 내용에 " "를 제외한 문자가 입력되었는지 검사후 send
	bool search = false;
	LPCTSTR str = chatmsg;
	for( unsigned int i = 0;i < len; ++i )
	{
		if( *str != 32 )
		{
			search = true;
			break;
		}
		else
			str++;
	}

	return search;
}

///
void cChatCommands::SendNormalMsg( LPCTSTR chatmsg )
{
	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}
	
	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_NORMAL_REQ;

	msg.emotionIdx = FILTERMAN->GetEmontionIndex( chatmsg );
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex		= chatWin->GetLinkItemIndex();

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, len );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

///
void cChatCommands::SendWhisperMsg( LPCTSTR toname, LPCTSTR chatmsg )
{
	assert( HERO );

	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT_WHISPER msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_WHISPER_REQ;
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex = chatWin->GetLinkItemIndex();
	
	Sstrncpy( msg.Name, MAX_NAME_BUFFER_SIZE, toname, MAX_NAME_SIZE );
	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, len );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

///
void cChatCommands::SendShoutMsg( LPCTSTR chatmsg )
{
	assert( HERO );

	int len = ::_tcslen( chatmsg );
	if( len == 0 )
		return;

	// 아이템이 없을때만 검사
	if( CheckChatMsg( chatmsg, len ) == false )
		return;
	
	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT_SHOUT msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_SHOUT_REQ;

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

///
void cChatCommands::SendPartyMsg( LPCTSTR chatmsg )
{
	assert( HERO );

	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT_PARTY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_PARTY_REQ;
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex		= chatWin->GetLinkItemIndex();

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );
	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}


///
void cChatCommands::SendTradeMsg( LPCTSTR chatmsg )
{
	assert( HERO );

	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	// 전장에서는 거래 채팅 불가
	if( PVPMAN->IsEnableBattle() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 862 ) );
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT_TRADE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_TRADE_REQ;
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex		= chatWin->GetLinkItemIndex();
	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

void cChatCommands::SendGuildMsg( LPCTSTR chatmsg )
{
	assert( HERO );

	// 전장에서는 길드 채팅 불가
	if( PVPMAN->IsEnableBattle() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 861 ) );
		return;
	}

	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT_GUILD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_GUILD_REQ;
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex		= chatWin->GetLinkItemIndex();

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

void cChatCommands::SendGMNormalMsg( LPCTSTR chatmsg )
{
	cChatWindow* chatWin = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
	if( !chatWin )
	{
		assert(0);
		return;
	}

	/// 링크내용도 없고 메세지도 없으면 보내지 않는다.
	int len = ::_tcslen( chatmsg );
	if( len == 0 && chatWin->IsLinked() == false )
		return;

	// 링크아이템 없을때만 공백문자열 검사
	if( chatWin->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		cStringT errorMsg;
		errorMsg.Format( GAMERESOURCEMAN->GetGameText( 80 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
		return;
	}

	MSG_REQ_CHAT msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_GM_NORMAL_REQ;

	msg.emotionIdx = FILTERMAN->GetEmontionIndex( chatmsg );
	msg.InventoryNumber = chatWin->GetLinkSlotIndex();
	msg.itemIndex		= chatWin->GetLinkItemIndex();

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
}

bool cChatCommands::SendMegaphoneMsg( LPCTSTR chatmsg )
{
	assert( HERO );

	cMegaphoneWindow* win = (cMegaphoneWindow*)UIMAN->FindNodeByID( eUIID_MEGAPHONE_WINDOW );
	if( !win )
	{
		assert(0);
		return false;
	}

	int len = ::_tcslen( chatmsg );
	if( len == 0 && win->IsLinked() == false )
		return false;

	//
	if( win->IsLinked() == false )
	{
		if( CheckChatMsg( chatmsg, len ) == false )
			return false;
	}

	/// filtering
	if( FILTERMAN->CheckWholeMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 12001 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
		return false;
	}

	/// filtering
	if( FILTERMAN->CheckPartMatch( chatmsg ) == true )
	{
		/// 에러메시지 출력
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 12001 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
		return false;
	}

	MSG_REQ_CHAT_MEGAPHONE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_MEGAPHONE_REQ;

	msg.InventoryIdx = win->GetInventoryIndex();
	msg.ItemIndex = win->GetLinkItemIndex();
	msg.InventoryNumber = win->GetLinkSlotIndex();
	msg.EmotionIdx = 0;

	Sstrncpy( msg.Message, MAX_CHAT_BUFFER_SIZE, chatmsg, MAX_CHAT_SIZE );
	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );

	return true;
}
