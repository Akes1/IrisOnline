#include "StdAfx.h"
#include "CheatManager.h"

#include "Protocol_Cheat.h"
#include "Cheat_Common.h"
#include "Notice_Common.h"
#include "WorldManager.h"
#include "SceneManager.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "UIRootNode.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "SkillManager.h"
//
#include "RevivalWindow.h"
#include "Hero.h"
#include "UIMsgboxEvent.h"
#include "SkillWindow.h"
#include "SkillScript.h"
//
#include "CameraManager.h"

#ifdef _GMTOOL
	#include "GMTool.h"
#endif

cCheatManager* cCheatManager::mInstance = 0;

cCheatManager::cCheatManager()
#ifdef _GMTOOL
: mCommandMap( 128 )
, mShotMode( false )
, mEarMode( true )
, mHideMode( false )
, mUndeadMode( false )
, mWatchMode( false )
#endif
{
	assert( mInstance == 0 && "bad singleton" );
	mInstance = this;

#ifdef _GMTOOL
	mCommandMap.Insert( L"shot", CHEAT_COMM_SHOT );
	mCommandMap.Insert( L"speed", CHEAT_COMM_SPEEDUP );
	mCommandMap.Insert( L"ear", CHEAT_COMM_EAR );
	mCommandMap.Insert( L"hide", CHEAT_COMM_HIDE );
	mCommandMap.Insert( L"undead", CHEAT_COMM_UNDEAD );
	mCommandMap.Insert( L"info", CHEAT_COMM_INFO );
	mCommandMap.Insert( L"watch", CHEAT_COMM_WATCH );
	mCommandMap.Insert( L"kick", CHEAT_COMM_KICK );
	mCommandMap.Insert( L"recover", CHEAT_COMM_RECOVER );
	mCommandMap.Insert( L"rebirth", CHEAT_COMM_REBIRTH );
	mCommandMap.Insert( L"teleport", CHEAT_COMM_TELEPORT );
	mCommandMap.Insert( L"go", CHEAT_COMM_GO );
	mCommandMap.Insert( L"chase", CHEAT_COMM_CHASE );
	mCommandMap.Insert( L"move", CHEAT_COMM_MOVE );
	mCommandMap.Insert( L"call", CHEAT_COMM_CALL );
	mCommandMap.Insert( L"nochat", CHEAT_COMM_NO_CHAT );
	mCommandMap.Insert( L"allowchat", CHEAT_COMM_ALLOW_CHAT );
//	mCommandMap.Insert( L"stop", CHEAT_COMM_STOP );
//	mCommandMap.Insert( L"cancelstop", CHEAT_COMM_CANCEL_STOP );
	mCommandMap.Insert( L"notice_server", CHEAT_COMM_NOTICE_SERVER );
	mCommandMap.Insert( L"notice_channel", CHEAT_COMM_NOTICE_CHANNEL );
	mCommandMap.Insert( L"notice_area", CHEAT_COMM_NOTICE_AREA );
	mCommandMap.Insert( L"removei", CHEAT_COMM_REMOVE_ITEM );
	mCommandMap.Insert( L"removem", CHEAT_COMM_REMOVE_MONSTER );
	mCommandMap.Insert( L"levelup", CHEAT_COMM_LEVELUP );
	mCommandMap.Insert( L"skillup", CHEAT_COMM_SKILLUP );
	mCommandMap.Insert( L"make", CHEAT_COMM_MAKE );
	mCommandMap.Insert( L"gold", CHEAT_COMM_GOLD );
	mCommandMap.Insert( L"summon", CHEAT_COMM_SUMMON );
	mCommandMap.Insert( L"remove", CHEAT_COMM_REMOVE );
	mCommandMap.Insert( L"kill", CHEAT_COMM_KILL );
	mCommandMap.Insert( L"playerinfo", CHEAT_COMM_PLAYERINFO );

#endif
}

cCheatManager::~cCheatManager()
{
	assert( mInstance );
	mInstance = 0;
}

#ifdef _GMTOOL
void cCheatManager::Clear()
{
	mShotMode = false;
	mEarMode = true;
	mHideMode = false;
	mUndeadMode = false;
	mWatchMode = false;
}

bool cCheatManager::Interpret( const charT* src, unsigned int srcLen )
{
	if( src == 0 || src[0] != '/' || srcLen < 3 )
		return false;

	cTokenizerT tokenizer( &src[1], srcLen-1, L" \t\r\n" );
	cStringT str;

	if( tokenizer.GetNext( &str ) == false )
		return false;

	/// 명령어를 검사
	str.ToLower();
	cCommandMap::cIterator i = mCommandMap.Find( str );
	if( i == mCommandMap.End() )
		return false;

	unsigned int commIndex = i->mSecond;

	/// 인자를 추가
	const wchar_t* argStr = tokenizer.GetTail();

	mArgArray.Clear();

	while( tokenizer.IsEnd() == false )
	{
		if( tokenizer.GetNext( &str ) == false )
			return false;

		mArgArray.PushBack( str );
	}

	/// 명령을 처리
	return HandleCommand( commIndex, argStr, mArgArray );
}

bool cCheatManager::HandleCommand( unsigned int commIndex, const wchar_t* argStr, const tArray<cStringT>& argArray )
{
	switch( commIndex )
	{
	case CHEAT_COMM_SHOT:			return HandleCommand_Shot( argStr, argArray );
	case CHEAT_COMM_SPEEDUP:		return HandleCommand_SpeedUp( argStr, argArray );
	case CHEAT_COMM_EAR:			return HandleCommand_Ear( argStr, argArray );
	case CHEAT_COMM_HIDE:			return HandleCommand_Hide( argStr, argArray );
	case CHEAT_COMM_UNDEAD:			return HandleCommand_Undead( argStr, argArray );
	case CHEAT_COMM_INFO:			return HandleCommand_Info( argStr, argArray );
	case CHEAT_COMM_WATCH:			return HandleCommand_Watch( argStr, argArray );
	case CHEAT_COMM_KICK:			return HandleCommand_Kick( argStr, argArray );
	case CHEAT_COMM_RECOVER:		return HandleCommand_Recover( argStr, argArray );
	case CHEAT_COMM_REBIRTH:		return HandleCommand_Rebirth( argStr, argArray );
	case CHEAT_COMM_TELEPORT:		return HandleCommand_Teleport( argStr, argArray );
	case CHEAT_COMM_GO:				return HandleCommand_Go( argStr, argArray );
	case CHEAT_COMM_CHASE:			return HandleCommand_Chase( argStr, argArray );
	case CHEAT_COMM_MOVE:			return HandleCommand_Move( argStr, argArray );
	case CHEAT_COMM_CALL:			return HandleCommand_Call( argStr, argArray );
	case CHEAT_COMM_NO_CHAT:		return HandleCommand_NoChat( argStr, argArray );
	case CHEAT_COMM_ALLOW_CHAT:		return HandleCommand_AllowChat( argStr, argArray );
	case CHEAT_COMM_STOP:			return HandleCommand_Stop( argStr, argArray );
	case CHEAT_COMM_CANCEL_STOP:	return HandleCommand_CancelStop( argStr, argArray );
	case CHEAT_COMM_NOTICE_SERVER:	return HandleCommand_NoticeServer( argStr, argArray );
	case CHEAT_COMM_NOTICE_CHANNEL:	return HandleCommand_NoticeChannel( argStr, argArray );
	case CHEAT_COMM_NOTICE_AREA:	return HandleCommand_NoticeArea( argStr, argArray );
	case CHEAT_COMM_REMOVE_ITEM:	return HandleCommand_RemoveItem( argStr, argArray );
	case CHEAT_COMM_REMOVE_MONSTER:	return HandleCommand_RemoveMonster( argStr, argArray );
	case CHEAT_COMM_LEVELUP:		return HandleCommand_LevelUp( argStr, argArray );
	case CHEAT_COMM_SKILLUP:		return HandleCommand_SkillUp( argStr, argArray );
	case CHEAT_COMM_MAKE:			return HandleCommand_Make( argStr, argArray );
	case CHEAT_COMM_GOLD:			return HandleCommand_Gold( argStr, argArray );
	case CHEAT_COMM_SUMMON:			return HandleCommand_Summon( argStr, argArray );
	case CHEAT_COMM_REMOVE:			return HandleCommand_Remove( argStr, argArray );
	case CHEAT_COMM_KILL:			return HandleCommand_Kill( argStr, argArray );
	case CHEAT_COMM_PLAYERINFO:		return HandleCommand_PlayerInfo( argStr, argArray );
	}
	return false;
}

bool cCheatManager::HandleCommand_Shot( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	bool mode = false;

	if( argArray[0].CompareNoCase( L"on" ) == 0 )
	{
		mode = true;
	}
	else if( argArray[0].CompareNoCase( L"off" ) == 0 )
	{
		mode = false;
	}
	else
	{
		return false;
	}

	///
	mShotMode = mode;

	if( SCENEMAN )
	{
		SCENEMAN->SetScreenShotMode( mShotMode );
	}
	
	if( UIMAN )
	{
		cUIRootNode* uiroot = UIMAN->GetRootNode();
		if( uiroot )
		{
			if( mShotMode )
				uiroot->Hide();
			else
				uiroot->Show();
		}
	}
	return true;
}

bool cCheatManager::HandleCommand_SpeedUp( const wchar_t* argStr, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	eCHEAT_SPEEDUP speedUp = (eCHEAT_SPEEDUP)argArray[0].ToInt();
	if( speedUp >= eCHEATSPEEDUP_MAX )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_SPEEDUP msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SPEEDUP_REQ;
		msg.mSpeedUp = speedUp;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Ear( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	bool mode = false;

	if( argArray[0].CompareNoCase( L"on" ) == 0 )
	{
		mode = true;
	}
	else if( argArray[0].CompareNoCase( L"off" ) == 0 )
	{
		mode = false;
	}
	else
	{
		return false;
	}
	
	if( mode == mEarMode )
	{
		return true;
	}

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_EAR msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_EAR_REQ;
		msg.mMode = mode;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Hide( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	bool mode = false;

	if( argArray[0].CompareNoCase( L"on" ) == 0 )
	{
		mode = true;
	}
	else if( argArray[0].CompareNoCase( L"off" ) == 0 )
	{
		mode = false;
	}
	else
	{
		return false;
	}

	if( mode == mHideMode )
	{
		return true;
	}

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_HIDE msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_HIDE_REQ;
		msg.mMode = mode;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Undead( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	bool mode = false;

	if( argArray[0].CompareNoCase( L"on" ) == 0 )
	{
		mode = true;
	}
	else if( argArray[0].CompareNoCase( L"off" ) == 0 )
	{
		mode = false;
	}
	else
	{
		return false;
	}

	/// 현재 셋팅한 값과 같으면 무시.
	if( mode == mUndeadMode )
	{
		return true;
	}

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_UNDEAD msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_UNDEAD_REQ;
		msg.mMode = mode;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Info( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_INFO msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_INFO_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Watch( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	bool mode = false;

	if( argArray[0].CompareNoCase( L"on" ) == 0 )
	{
		mode = true;
	}
	else if( argArray[0].CompareNoCase( L"off" ) == 0 )
	{
		mode = false;
	}
	else
	{
		return false;
	}

	if( mode == mWatchMode )
	{
		return true;
	}

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_WATCH msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_WATCH_REQ;
		msg.mMode = mode;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Kick( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_KICK msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_KICK_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Recover( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_RECOVER msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_RECOVER_REQ;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Rebirth( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_REBIRTH msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_REBIRTH_REQ;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Teleport( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 2 )
		return false;

	///
	float x = (float)argArray[0].ToInt()*100;
	float y = (float)argArray[1].ToInt()*100;
	float z = 0.0f;

	/// 내 캐릭터 이동
	if( WORLDMAN && WORLDMAN->CalcHeight( &z, x, y ) )
	{
		HERO->SetState( eOBJECT_STATE_IDLE );
		HERO->SetPos( NiPoint3(x, y, z) );
	}
	else
	{
		assert( 0 );
		return true;
	}

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_TELEPORT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_TELEPORT_REQ;
		msg.mDestX = x;
		msg.mDestY = y;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Go( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 3 )
		return false;

	short mapNum = (short)argArray[0].ToInt();
	float x = argArray[1].ToInt() * 100.0f;
	float y = argArray[2].ToInt() * 100.0f;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_GO msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_GO_REQ;
		msg.mMapNum  = mapNum;
		msg.mDestX = x;
		msg.mDestY = y;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Chase( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_CHASE msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_CHASE_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Move( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 맵이동 버튼이 활성화 된 경우만 이동
	if( GMTOOL->GetMapTeleportState() == FALSE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8060 ) );
		return false;
	}
	/// 인자 검사
	if( argArray.GetSize() != 4 )
		return false;

	const cStringT& charName = argArray[0];
	short mapNum = (short)argArray[1].ToInt();
	float x = argArray[2].ToInt() * 100.0f;
	float y = argArray[3].ToInt() * 100.0f;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_MOVE msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_MOVE_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );
		msg.mMapNum = mapNum;
		msg.mDestX = x;
		msg.mDestY = y;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Call( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_CALL msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_CALL_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_NoChat( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 2 )
		return false;

	const cStringT& charName = argArray[0];
	unsigned long duration = argArray[1].ToInt();

	if( duration < 1 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_NO_CHAT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_NO_CHAT_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );
		msg.mDuration = duration;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_AllowChat( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_ALLOW_CHAT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_ALLOW_CHAT_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Stop( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 2 )
		return false;

	const cStringT& charName = argArray[0];
	unsigned long duration = argArray[1].ToInt() * 60 * 1000;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_STOP msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_STOP_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );
		msg.mDuration = duration;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_CancelStop( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_CANCEL_STOP msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_CANCEL_STOP_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_NoticeServer( const wchar_t* argStr, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() == 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_NOTICE_SERVER msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_SERVER_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, argStr, MAX_CHAT_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );
	}
	return true;
}

bool cCheatManager::HandleCommand_NoticeChannel( const wchar_t* argStr, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() == 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_NOTICE_CHANNEL msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_CHANNEL_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, argStr, MAX_CHAT_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );
	}
	return true;
}

bool cCheatManager::HandleCommand_NoticeArea( const wchar_t* argStr, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() == 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_NOTICE_AREA msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_AREA_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, argStr, MAX_CHAT_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );
	}
	return true;
}

bool cCheatManager::HandleCommand_RemoveItem( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_REMOVE_ITEM msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_REMOVE_ITEM_REQ;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}


bool cCheatManager::HandleCommand_RemoveMonster( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_REMOVE_MONSTER msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_REMOVE_MONSTER_REQ;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_LevelUp( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	unsigned char add = (unsigned char)argArray[0].ToInt();

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_LEVELUP msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_LEVELUP_REQ;
		msg.mAddedValue = add;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_SkillUp( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	unsigned char add = (unsigned char)argArray[0].ToInt();

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_SKILLUP msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SKILLUP_REQ;
		msg.mAddedValue = add;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Make( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 아이템 잠금 검사
	if( ITEMMAN->IsLocked() )
		return false;

	/// 인자 검사
	if( argArray.GetSize() != 2 )
		return false;

	///
	unsigned long itemIndex = argArray[0].ToInt();
	unsigned char count = (unsigned char)argArray[1].ToInt();

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_MAKE msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_MAKE_REQ;
		msg.mItemIndex = itemIndex;
		msg.mCount = count;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Gold( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	///
	unsigned long val = argArray[0].ToInt();

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_GOLD msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_GOLD_REQ;
		msg.mValue = val;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Summon( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 2 )
		return false;

	///
	unsigned long monsterNum = argArray[0].ToInt();
	unsigned char val = (unsigned char)argArray[1].ToInt();

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_SUMMON msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SUMMON_REQ;
		msg.mMonsterNum = monsterNum;
		msg.mValue = val;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Remove( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget == NULL )
		return false;

	if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_REMOVE msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_REMOVE_REQ;
		msg.mObjIndex = pTarget->GetObjectID();

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_Kill( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 0 )
		return false;

	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget == NULL )
		return false;

	if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
		return false;

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_KILL msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_KILL_REQ;
		msg.mObjIndex = pTarget->GetObjectID();

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}

bool cCheatManager::HandleCommand_PlayerInfo( const wchar_t* /*argStr*/, const tArray<cStringT>& argArray )
{
	/// 인자 검사
	if( argArray.GetSize() != 1 )
		return false;

	const cStringT& charName = argArray[0];

	/// 메시지 전송
	if( NETWORK->IsConnected() )
	{
		MSG_REQ_CHEAT_PLAYERINFO msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SYSTEM_PLAYERINFO_REQ;
		::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName.Cstr(), MAX_NAME_SIZE );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	return true;
}
#endif // _GMTOOL

void cCheatManager::NetworkMsgParser( MSGROOT* msg )
{
	cHero* hero = HERO;

	switch( msg->Protocol )
	{
	case NM_CHEAT_RECOVER_SYN:
		{
			MSG_SYN_CHEAT_RECOVER* syn = (MSG_SYN_CHEAT_RECOVER*)msg;

			cPlayer* player = OBJECTMANAGER->GetPlayer( syn->mObjIndex );
			if( player == 0 )
				return;

			player->HPChange( syn->mMaxHP, syn->mMaxHP );
			player->MPChange( syn->mMaxMP, syn->mMaxMP );
		}
		break;
	case NM_CHEAT_REBIRTH_SYN:
		{
			MSG_SYN_CHEAT_REBIRTH* syn = (MSG_SYN_CHEAT_REBIRTH*)msg;

			cPlayer* player = OBJECTMANAGER->GetPlayer( syn->mObjIndex );
			if( player == 0 )
				return;

			player->SetState( eOBJECT_STATE_IDLE );
			player->HPChange( syn->mMaxHP, syn->mMaxHP );
			player->MPChange( syn->mMaxMP, syn->mMaxMP );
		}
		break;
	case NM_CHEAT_TELEPORT_RES:
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8000 ) );
		}
		break;
	case NM_CHEAT_TELEPORT_SYN:
		{
			MSG_SYN_CHEAT_TELEPORT* syn = (MSG_SYN_CHEAT_TELEPORT*)msg;

			cPlayer* player = OBJECTMAN->GetPlayer( syn->mObjIndex );
			if( player == 0 )
			{
				assert(0);
				return;
			}

			player->SetState( eOBJECT_STATE_IDLE );

			/// 높이값 계산
			if( WORLDMAN == 0 )
			{
				assert(0);
				return;
			}

			NiPoint3 pos( (float)syn->mDestX, (float)syn->mDestY, 0.0f );
			if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
			{
				assert(0);
				return;
			}

			player->SetPos( pos );

			if( player == HERO )
			{
				UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
 				cCamera* cam = CAMERAMAN->GetCurrent();
 				if( cam )
 				{
 					cam->ResetLookAt( pos );
 				}
				HERO->SetPos_ResetCam( pos );
			}

		}
		break;
	case NM_CHEAT_STOP_SYN:
		{
			MSG_SYN_CHEAT_STOP* syn = (MSG_SYN_CHEAT_STOP*)msg;

			cPlayer* player = OBJECTMAN->GetPlayer( syn->mObjIndex );
			if( player == 0 )
			{
				assert(0);
				return;
			}

			player->SetStopFlag( eSTOP_CHEATSTOP );
		}
		break;
	case NM_CHEAT_CANCEL_STOP_SYN:
		{
			MSG_SYN_CHEAT_CANCEL_STOP* syn = (MSG_SYN_CHEAT_CANCEL_STOP*)msg;

			cPlayer* player = OBJECTMAN->GetPlayer( syn->mObjIndex );
			if( player == 0 )
			{
				assert(0);
				return;
			}

			player->SetStopFlag( eSTOP_NONE );
		}
		break;
	case NM_CHEAT_NO_CHAT_SYN:
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8001 ) );
		}
		break;
	case NM_CHEAT_ALLOW_CHAT_SYN:
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8002 ) );
		}
		break;
	case NM_CHEAT_GM_SKILL_USE_PLAYER_SYN:
	case NM_CHEAT_GM_SKILL_USE_AREA_SYN:
		{
			MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER* pSynMsg = (MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER*)msg;

			sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( pSynMsg->mInfluenceIndex );
			if( pInfo )
			{
				cStringT temp;
				temp.Format( GAMERESOURCEMAN->GetGameText( 8900 ), GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameidx ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, temp.Cstr() );
			}
			else
			{
				assert(0);
				return;
			}
		}
		break;
	case NM_CHEAT_GM_THEME_RESET_SYN:
		{
			MSG_SYN_CHEAT_THEMERESET* pSynMsg = (MSG_SYN_CHEAT_THEMERESET*)msg;
			
			LPCTSTR modeName = UIMAN->GetUIText( 26500 + pSynMsg->mThemeMode );
			LPCTSTR mapName = GAMERESOURCEMAN->GetMapName( pSynMsg->mThemeNum );

			cStringT msg;
			msg.Format( GAMERESOURCEMAN->GetGameText( 8107 ), mapName, modeName );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
		}
		break;

#ifdef _GMTOOL
	case NM_CHEAT_NO_CHAT_LIST_RES:
		{
			MSG_RES_CHEAT_NO_CHAT_LIST* res = (MSG_RES_CHEAT_NO_CHAT_LIST*)msg;

			GMTOOL->SetNoChatUser( res->mRowCount, res->table );			
		}
		break;
	case NM_CHEAT_EAR_RES:
		{
			MSG_RES_CHEAT_EAR* res = (MSG_RES_CHEAT_EAR*)msg;
			
			mEarMode = res->mMode;

			if( mEarMode )
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8004 ) );
			else
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8005 ) );
		}
		break;
	case NM_CHEAT_HIDE_RES:
		{
			MSG_RES_CHEAT_HIDE* res = (MSG_RES_CHEAT_HIDE*)msg;

			mHideMode = res->mMode;
			
			GMTOOL->SetHideModeText( res->mMode );

			if( mHideMode )
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8006 ) );
			else
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8007 ) );
		}
		break;
	case NM_CHEAT_UNDEAD_RES:
		{
			MSG_RES_CHEAT_UNDEAD* res = (MSG_RES_CHEAT_UNDEAD*)msg;

			mUndeadMode = res->mMode;
			
			GMTOOL->SetUndeadModeText( res->mMode );

			if( mUndeadMode )
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8008 ) );
			else
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8009 ) );
			
		}
		break;
	case NM_CHEAT_INFO_RES:
		{
			MSG_RES_CHEAT_INFO* res = (MSG_RES_CHEAT_INFO*)msg;

			int xpos = (int)(res->mPosX * 0.01);
			int ypos = (int)(res->mPosY * 0.01);

			LPCTSTR job = GAMERESOURCEMAN->GetJobName( res->mJob );

			cStringT str;
			str.Format( _T("[%s-%d] %d(%d,%d), %d"), /*res->mUserID,*/ job, res->mLevel, 
													res->mMapNum, xpos, ypos, res->mMoney );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_CHEAT_WATCH_RES:
		{
			MSG_RES_CHEAT_UNDEAD* res = (MSG_RES_CHEAT_UNDEAD*)msg;

			mWatchMode = res->mMode;
		}
		break;
	case NM_CHEAT_KICK_RES:
		{
			// 서버로부터 연결이 해제 되었을 경우...
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL, 
								 eMBEVENT_PLAYER_KICKED, eSKIN_OK,
								 GAMERESOURCEMAN->GetGameText( 182 ),
								 GAMERESOURCEMAN->GetGameText( 301 )
								 );
		}
		break;
	case NM_CHEAT_KICK_RES_GM:
		{
			MSG_RES_CHEAT_KICK_GM* res = (MSG_RES_CHEAT_KICK_GM*)msg;

			if( res->ErrorCode == ERROR_CHEAT_KICK_SUCCESS )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8010 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
				
			}
			else
			{
				unsigned long errorIndex = 0;
				switch( res->ErrorCode )
				{
				case ERROR_CHEAT_KICK_FAIL:			 errorIndex = 8011; break;
				case ERROR_CHEAT_KICK_UNKNOWN_USER : errorIndex = 8136; break;
				case ERROR_CHEAT_KICK_NOT_CONNECTED: errorIndex = 8137; break;
				default:assert(0); break;
				}

				/// 결과 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( errorIndex ) );
			}
		}
		break;
	case NM_CHEAT_RECOVER_RES:
		{
			MSG_RES_CHEAT_RECOVER* res = (MSG_RES_CHEAT_RECOVER*)msg;

			if( hero == 0 )
				return;

			hero->HPChange( res->mMaxHP, res->mMaxHP );
			hero->MPChange( res->mMaxMP, res->mMaxMP );
		}
		break;
	case NM_CHEAT_REBIRTH_RES:
		{
			MSG_RES_CHEAT_REBIRTH* res = (MSG_RES_CHEAT_REBIRTH*)msg;

			if( hero == 0 )
				return;

			cRevivalWindow* win = GAMEUI->GetReVivalWindow();
			if( win )
				win->Hide();

			hero->SetState( eOBJECT_STATE_IDLE );
			hero->HPChange( res->mMaxHP, res->mMaxHP );
			hero->MPChange( res->mMaxMP, res->mMaxMP );
		}
		break;
	case NM_CHEAT_STOP_RES:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_STOP_SUCCESS )
			{
				hero->SetStopFlag( eSTOP_CHEATSTOP );
				CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8012 ) );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8013 ) );
			}
		
		}
		break;
	case NM_CHEAT_STOP_RES_GM:
		{
			MSG_RES_CHEAT_STOP_GM* res = (MSG_RES_CHEAT_STOP_GM*)msg;

			if( hero == 0 )
				return;
			
			if( res->ErrorCode == ERROR_CHEAT_STOP_SUCCESS )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8014 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8015 ) );
			}
		}
		break;
	case NM_CHEAT_CANCEL_STOP_RES:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_STOP_SUCCESS )
			{
				hero->SetStopFlag( eSTOP_NONE );
				CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8016 ) );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_GM, GAMERESOURCEMAN->GetGameText( 8017 ) );
			}
		}
		break;
	case NM_CHEAT_CANCEL_STOP_RES_GM:
		{
			MSG_RES_CHEAT_CANCEL_STOP_GM* res = (MSG_RES_CHEAT_CANCEL_STOP_GM*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_NO_ERROR )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8018 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8019 ) );
			}
		}
		break;
	case NM_CHEAT_MAKE_RES:
		{
			MSG_RES_CHEAT_MAKE* res = (MSG_RES_CHEAT_MAKE*)msg;

			if( res->ErrorCode == ERROR_CHEAT_MAKE_SUCCESS )
			{
				sInventory* ptr = res->inventory;

				for( int cnt = 0; cnt < res->rowCount ; ++cnt, ptr++ )
					ITEMMAN->CheatMakeItem( ptr );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8020 ) );
			}
		}
		break;
	case NM_CHEAT_NO_CHAT_RES_GM:
		{
			MSG_RES_CHEAT_NO_CHAT_GM* res = (MSG_RES_CHEAT_NO_CHAT_GM*)msg;

			if( hero == 0 )
				return;

			unsigned long errorMsg = 8022;
			switch( res->ErrorCode )
			{
				case ERROR_CHEAT_NO_CHAT_SUCCESS:   errorMsg = 8021; break;
				case ERROR_CHEAT_NO_CHAT_FAIL:      errorMsg = 8022; break;
				case ERROR_CHEAT_NO_CHAT_NO_PLAYER: errorMsg = 8075; break;
			}

			/// 에러 출력
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( errorMsg ) );
		}
		break;
	case NM_CHEAT_ALLOW_CHAT_RES_GM:
		{
			MSG_RES_CHEAT_ALLOW_CHAT_GM* res = (MSG_RES_CHEAT_ALLOW_CHAT_GM*)msg;

			if( hero == 0 )
				return;

			unsigned long errorMsg = 8023;
			switch( res->ErrorCode )
			{
			case ERROR_CHEAT_ALLOW_CHAT_SUCCESS:   errorMsg = 8023; break;
			case ERROR_CHEAT_ALLOW_CHAT_FAIL:      errorMsg = 8024; break;
			case ERROR_CHEAT_ALLOW_CHAT_NO_PLAYER: errorMsg = 8075; break;
			}

			/// 에러 출력
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( errorMsg ) );
		}
		break;
	case NM_CHEAT_NO_CHAT_LIST_RES_GM:
		{
			///리스트 박스 초기화 시키기
			GMTOOL->InitStopListBox();

			MSG_RES_CHEAT_NO_CHAT_LIST_GM* resMsg = (MSG_RES_CHEAT_NO_CHAT_LIST_GM*)msg;

			if( hero == 0 )
				return;

			unsigned long errorMsg = 8076;

			switch( resMsg->ErrorCode )
			{
			case ERROR_CHEAT_NO_CHAT_LIST_FAIL:		 errorMsg = 8076; break;
			case ERROR_CHEAT_NO_CHAT_LIST_NO_PLAYER: errorMsg = 8075; break;
			}

			/// 에러 출력
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( errorMsg ) );
		}
		break;
	case NM_CHEAT_CALL_RES_GM:
		{
			
			MSG_RES_CHEAT_CALL_GM* res = (MSG_RES_CHEAT_CALL_GM*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_CALL_SUCCESS )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8025 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
			}
			else if( res->ErrorCode == ERROR_CHEAT_CALL_FAIL_USER_STOP_STATE )
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8050 ));
				GMTOOL->HideChasePosImg();
			}
			else if( res->ErrorCode == ERROR_CHEAT_CALL_FAIL_MAP_CHANGE )
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8051 ));
				GMTOOL->HideChasePosImg();
			}
			else if( res->ErrorCode == ERROR_CHEAT_CALL_FAIL_DIFF_MAP )
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8079 ));
				GMTOOL->HideChasePosImg();
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8080 ));
				GMTOOL->HideChasePosImg();
			}
		}
		break;
	case NM_CHEAT_CHASE_RES_GM:
		{
			MSG_RES_CHEAT_CHASE_GM* res = (MSG_RES_CHEAT_CHASE_GM*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_NO_ERROR )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8027 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8028 ) );
			}
		}
		break;
	case NM_CHEAT_MOVE_RES_GM:
		{
			MSG_RES_CHEAT_MOVE_GM* res = (MSG_RES_CHEAT_MOVE_GM*)msg;

			if( hero == 0 )
				return;

			if( res->ErrorCode == ERROR_CHEAT_NO_ERROR )
			{
				/// 결과 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 8029 ), res->mCharName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8030 ) );
			}
		}
		break;
	case NM_CHEAT_SEARCH_RES_GM:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;
			if( res->ErrorCode == ERROR_CHEAT_SEARCH_SUCCESS )
			{
				MSG_RES_CHEAT_SEARCH_GM* resMsg = (MSG_RES_CHEAT_SEARCH_GM*)msg;

				///맵과 좌표를 표시해준다.
				GMTOOL->ShowSearchPlayer( resMsg->mMapNum, resMsg->mPosX, resMsg->mPosY );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8077 ) );
			}
		}
		break;
	case NM_CHEAT_SEARCH_CHANNEL_RES_GM:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;
			if( res->ErrorCode == ERROR_CHEAT_SEARCH_CHANNEL_SUCCESS )
			{
				MSG_RES_CHEAT_SEARCH_CHANNEL_GM* resMsg = (MSG_RES_CHEAT_SEARCH_CHANNEL_GM*)msg;

				if( resMsg->mChannelNum != 0 )
				{
					///채널 번호를 표시해 준다.
					GMTOOL->ShowSearchChannelPlayer( resMsg->mChannelNum );
				}
				else
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8075 ) );
			}
			else
			{
				/// 에러 출력
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8077 ) );
			}
		}
		break;
	case NM_CHEAT_GM_NAME_RES:
		{
			MSG_RES_CHEAT_GM_NAME* res	= (MSG_RES_CHEAT_GM_NAME*)msg;

			switch( res->ErrorCode )
			{
			case ERROR_CHEAT_GM_NAME_SUCCESS:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 8037 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_CHEAT_GM_NAME_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 8038 ), 
					GAMERESOURCEMAN->GetGameText(301)
					);
				break;
			case ERROR_CHEAT_GM_NAME_ALREADY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 8039 ), 
					GAMERESOURCEMAN->GetGameText(301)
					);
				break;
			case ERROR_CHEAT_GM_NAME_EXISTS:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 8040 ), 
					GAMERESOURCEMAN->GetGameText(301) 
					);
				break;
			}
		}
		break;
	case NM_CHEAT_GOTO_INSTANT_DUNGEON_RES:
		{
			MSG_RES_GOTO_INSTANT_DUNGEON* res = (MSG_RES_GOTO_INSTANT_DUNGEON*)msg;

			HERO->SetStopFlag( eSTOP_NONE );

			if( res->ErrorCode == ERROR_GOTO_INSTANT_DUNGEON_SUCCESS )
			{
			}
			else if( res->ErrorCode == ERROR_GOTO_INSTANT_DUNGEON_FAIL )
			{
				UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8052 ) );
			}
			else if( res->ErrorCode == ERROR_GOTO_INSTANT_DUNGEON_FAIL_ALREADY_IN)
			{
				UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8056 ) );
			}
			else
				assert( 0 && "bad message" );

			HERO->SetStopFlag( eSTOP_NONE );
			UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
		}
		break;
	case NM_CHEAT_USED_ROOM_ID_RES:
		{
			MSG_RES_USED_ROOM_ID* res = (MSG_RES_USED_ROOM_ID*)msg;
			if( res->ErrorCode == ERROR_USED_ROOM_ID_SUCCESS )
			{
				GMTOOL->SetPvpThemeList( res );
			}
			else if( res->ErrorCode == ERROR_USED_ROOM_ID_FAIL )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8053 ) );
			}
			else
				assert( 0 && "bad message" );
		}
		break;
	case NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_RES:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;
			if( res->ErrorCode == ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL )
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8052 ) );
			else if( res->ErrorCode == ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL_ALREADY_IN)
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8056 ) );
			}
			else
				assert( 0 && "bad message" );

			HERO->SetStopFlag( eSTOP_NONE );
			UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
		}
		break;
	case NM_CHEAT_GOTO_GAMESERVER_RES:
		{
			MSG_ERROR* res = (MSG_ERROR*)msg;
			if( res->ErrorCode == ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL )
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8052 ) );
			else if( res->ErrorCode == ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL_ALREADY_IN)
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8056 ) );
			}
			else
				assert( 0 && "bad message" );

			HERO->SetStopFlag( eSTOP_NONE );
			UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GOTO_DUNGEON );
		}
		break;
	case NM_CHEAT_SKILL_GET_RES_GM:
		{
			MSG_RES_SKILL_ADD* pResMsg = (MSG_RES_SKILL_ADD*)msg;

			if( SKILLMAN->GetKeepInfo( pResMsg->mSkillClassIdx ) == 0 )
			{
				/// add
				SKILLMAN->AddKeepSkill( pResMsg->mSkillClassIdx, pResMsg->mSkillStep );
			}
			else
			{
				/// update
				SKILLMAN->UpdateSkillStep( pResMsg->mSkillClassIdx, pResMsg->mSkillStep, true );
			}

			/// 071024 PKH 스테이터스 재계산
			HERO->CalcStatus();

			/// 이펙트 생략
/* 			NiTransform trans;
 			trans.MakeIdentity();
 			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 30.0f );
 			HERO->SetLinkdEffect( eLINK_HEAD, "./Data/Effect/Eff_PC_Get_01.nif", &trans, false, true );
*/
			/// 스킬 계속 습득하기
			GMTOOL->GetSkill();
			
		}
		break;
	case NM_CHEAT_SKILL_GET_ERR_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;

			unsigned long errorMsgIndex = 8068;
			switch( pResMsg->ErrorCode )
			{
				case ERROR_CHEAT_SKILL_ADD_ERR_RACE: errorMsgIndex = 6106; break;
				case ERROR_CHEAT_SKILL_ADD_ERR_JOB:  errorMsgIndex = 6107; break;
				case ERROR_CHEAT_SKILL_ADD_DBERROR :
				case ERROR_CHEAT_SKILL_ADD_ERR_PLAYER:
				case ERROR_CHEAT_SKILL_ADD_ERR_SCRIPT:
				case ERROR_CHEAT_SKILL_ADD_ERR_SKILLSTEP:
				case ERROR_CHEAT_SKILL_ADD_ERR_MAPINSERT:
				default: 
					errorMsgIndex = 8068;
					break;
			}

			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( errorMsgIndex ) );

			cSkillWindow* win = GAMEUI->GetSkillWindow();
			if( win )
				win->NetUnLock();
		}
		break;
	case NM_CHEAT_GM_SKILL_USE_PLAYER_RES_GM:
	case NM_CHEAT_GM_SKILL_USE_AREA_RES_GM:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;
			if( pResMsg->ErrorCode != 0 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8081 ) );
			}
		}
		break;
	case NM_CHEAT_GM_JOB_CHANGE_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;
			if( pResMsg->ErrorCode == ERROR_CHEAT_GM_JOB_CHANGE_ERR_NOT_MATCH )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8088 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_JOB_CHANGE_DB_ERR )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8089 ) );
			}
		}
		break;
	case NM_CHEAT_GM_FORCE_CHANGE_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;
			if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_CHANGE_SUCCESS )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8092 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_CHANGE_SAMETYPE )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8091 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_CHANGE_BADTYPE )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8090 ) );
			}
		}
		break;
	case NM_CHEAT_GM_FORCE_ADDPOINT_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;
			if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_POINT_SUCCESS )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8094 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_POINT_NOFORCE )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8095 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_POINT_BADTYPE )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8090 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_POINT_FAIL )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8096 ) );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_GM_FORCE_POINT_BADPOINT )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8093 ) );
			}
		}
		break;
	case NM_CHEAT_SYSTEM_PLAYERINFO_RES:
		{
			MSG_RES_CHEAT_PLAYERINFO* pResMsg = (MSG_RES_CHEAT_PLAYERINFO*)msg;
			if( pResMsg->ErrorCode == ERROR_CHEAT_SEARCH_SUCCESS )
			{
				cStringT str;
				str.Format( _T("%u %d %u %d %u / %u %u %d %u %u / %u %u %u"),
					pResMsg->mState,
					pResMsg->mStateStop,
					pResMsg->mRequestRejection,
					pResMsg->mCashInfo == false ? 0 : 1,
					pResMsg->mInventoryDB,
					pResMsg->mCommonDatabase,
					pResMsg->mGameInDatabase,
					pResMsg->mGameInDBEnd == false ? 0 : 1,
					pResMsg->mSkillDatabase,
					pResMsg->mQuestDatabase,
					pResMsg->mGuildDatabase,
					pResMsg->mRequestData,
					pResMsg->mCompleteData
					);

				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER2 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER2") );
			}
			else if( pResMsg->ErrorCode == ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER") );
			}
		}
		break;
	case NM_CHEAT_GM_THEME_INFO_RES:
		{
			MSG_RES_CHEAT_THEME_INFO* pResMsg = (MSG_RES_CHEAT_THEME_INFO*)msg;
			if( pResMsg )
			{
				switch( pResMsg->ErrorCode )
				{
				case ERROR_CHEAT_GM_THEME_INFO_SUCCESS:
				case ERROR_CHEAT_GM_THEME_INFO_NO_DATA:
					{
						GMTOOL->SetThemeResetInfo( pResMsg );
					}
					break;
				case ERROR_CHEAT_GM_THEME_INFO_FAIL:
					GMTOOL->ClearThemeResetList();
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8096 ) );
					break;
				case ERROR_CHEAT_GM_THEME_INFO_NO_PLAYER:
					GMTOOL->ClearThemeResetList();
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8075 ) );
					break;
				}
			}
		}
		break;
	case NM_CHEAT_GM_THEME_RESET_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)msg;
			unsigned long msgIndex = 8096;

			switch( pResMsg->ErrorCode )
			{
			case ERROR_CHEAT_GM_THEME_RESET_SUCCESS:		  msgIndex = 8113; break;
			case ERROR_CHEAT_GM_THEME_RESET_FAIL:			  msgIndex = 8096; break;
			case ERROR_CHEAT_GM_THEME_RESET_NOT_SAME_CHANNEL: msgIndex = 8108; break; /// 동일 채널 아님
			case ERROR_CHEAT_GM_THEME_RESET_THEME_IN:		  msgIndex = 8109; break; /// 유저가 테마에 있음
			case ERROR_CHEAT_GM_THEME_RESET_TIME:			  msgIndex = 8110; break; /// 테마던젼 초기화 준비중
			case ERROR_CHEAT_GM_THEME_RESET_NO_PLAYER:		  msgIndex = 8111; break; /// 초기화할 유저가 존재하지 않음
			case ERROR_CHEAT_GM_THEME_RESET_DB_USE:			  msgIndex = 8096; break; /// GM유저 DB 사용중
			case ERROR_CHEAT_GM_THEME_RESET_NO_DATA:		  msgIndex = 8114; break; /// 해당 테마 정보가 존재하지 않음

			default: assert(0);
			}

			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( msgIndex ) );
		}
		break;
	case NM_CHEAT_GM_EVENT_SEARCH_RES:
		{
			MSG_RES_CHEAT_GM_EVENT_SEARCH* pResMsg = (MSG_RES_CHEAT_GM_EVENT_SEARCH*)msg;
			switch( pResMsg->ErrorCode )
			{
			case ERROR_CHEAT_GM_EVENT_SEARCH_SUCCESS :
			case ERROR_CHEAT_GM_EVENT_SEARCH_NO_DATA :
				{
					GMTOOL->SetGMEventInfo( pResMsg );
				}
				break;
			case ERROR_CHEAT_GM_EVENT_SEARCH_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8096 ) );
				break;
			}
		}
		break;
	case NM_CHEAT_GM_EVENT_APPLY_RES:
		{
			MSG_RES_CHEAT_GM_EVENT_APPLY* pResMsg = (MSG_RES_CHEAT_GM_EVENT_APPLY*)msg;
			switch( pResMsg->ErrorCode )
			{
			case ERROR_CHEAT_GM_EVENT_APPLY_SUCCESS :
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8119 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_APPLY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8096 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_APPLY_ALREADY_REGIST:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8118 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_APPLY_DB_USE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8112 ) );
				break;	
			case ERROR_CHEAT_GM_EVENT_APPLY_DB_INSERT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8120 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_APPLY_BAD_VALIDTHRU:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8121 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_APPLY_BAD_INFLUENCE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8129 ) );
				break;
			}
		}
		break;
	case NM_CHEAT_GM_EVENT_RELEASE_RES:
		{
			MSG_RES_CHEAT_GM_EVENT_RELEASE* pResMsg = (MSG_RES_CHEAT_GM_EVENT_RELEASE*)msg;
			switch( pResMsg->ErrorCode )
			{
			case ERROR_CHEAT_GM_EVENT_RELEASE_SUCCESS :
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8125 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_RELEASE_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8096 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_RELEASE_DB_USE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8112 ) );
				break;
			case ERROR_CHEAT_GM_EVENT_RELEASE_BAD_INDEX:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8126 ) );
				break;
			}
		}
		break;

#endif // _GMTOOL
	default:	assert(0);
	}
}
