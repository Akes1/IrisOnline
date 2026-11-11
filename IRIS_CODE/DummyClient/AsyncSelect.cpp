#include "stdafx.h"
#include "AsyncSelect.h"

#include "packet.h"
#include "commondefines.h"
#include "commonnetworkmsgstruct.h"
#include "protocol_category.h"
#include "protocol.h"

#include "DummyClient.h"

#define MAX_RC5_KEY				16

RC5_32_KEY RC5_32_KEY_TABLE[ MAX_RC5_KEY ];

BYTE SY_RKEY_TABLE[16][4]=
{
	{ 0x17,0xb3,0x67,0xaa },	//0
	{ 0x53,0xf2,0x74,0xb1 },	//1
	{ 0x8c,0x72,0x59,0x32 },	//2
	{ 0x98,0xea,0x55,0x29 },	//3
	{ 0x4B,0x1C,0x15,0x11 },	//4
	{ 0xE8,0x9B,0x3A,0x8A },	//5
	{ 0x0F,0xAD,0x37,0x6C },	//6
	{ 0xD9,0x64,0x28,0x5C },	//7
	{ 0xB3,0xB7,0xEC,0x7B },	//8
	{ 0x9C,0x13,0xFE,0xF5 },	//9
	{ 0x5C,0x55,0x6A,0xD8 },	//10
	{ 0x6C,0xA9,0xDB,0x64 },	//11
	{ 0x5A,0xBE,0xF2,0xF6 },	//12
	{ 0xA6,0x4D,0x8F,0xB9 },	//13
	{ 0x67,0x9F,0x7D,0xF6 },	//14
	{ 0xCA,0x6A,0xA9,0xEF },	//15
};

BYTE SY_KEY_TABLE[16][4]=
{
	{ 0xC0,0x3C,0x06,0xEA },	//0
	{ 0xB9,0x78,0xB3,0x45 },	//1
	{ 0x88,0x48,0xC0,0x71 },	//2
	{ 0x85,0x4C,0xB6,0xE7 },	//3
	{ 0x98,0xAE,0xD9,0xBF },	//4
	{ 0x2D,0x83,0x06,0x55 },	//5
	{ 0x44,0x95,0xAF,0x99 },	//6
	{ 0x43,0xC8,0x53,0x63 },	//7
	{ 0x3B,0xB1,0x60,0x0B },	//8
	{ 0x19,0x2D,0x57,0xC6 },	//9
	{ 0x68,0xCC,0x56,0x3A },	//10
	{ 0x5C,0xF8,0x90,0x4A },	//11
	{ 0xA1,0x8A,0x0B,0x70 },	//12
	{ 0x28,0x45,0x80,0xB1 },	//13
	{ 0x71,0x9D,0x4A,0x80 },	//14
	{ 0x54,0x1B,0x0F,0x9C },	//15
};

#pragma pack(1) // 1Byte 정렬 설정

struct Login2Game : Packet
{
	unsigned long  ipv4;
	unsigned short port;
};
struct Game2Game : Packet
{
	unsigned long  ipv4;
	unsigned short port;
};

#pragma pack( ) // 1Byte 정렬 해제


cAsyncSelect::cAsyncSelect(HWND wnd, unsigned int msg, cDummyClient* client) 
: m_wnd( wnd )
, m_msg( msg )
, mDummyClient(client)
{
	m_socket        = INVALID_SOCKET;
	m_connectThread = NULL;
	m_connected     = false;

	m_rc5       = true;
	m_seq       = 0;
	m_loginIdx  = 0;
	m_memberIdx = 0;

	mLastTickCount = 0;

	m_backBuffer.offset  = 0;
	m_backBuffer.length  = MAX_BUFFER;
	m_backBuffer.data    = (char*)GlobalAlloc( GPTR, MAX_BUFFER );

	m_frontBuffer.offset = 0;
	m_frontBuffer.length = MAX_BUFFER;
	m_frontBuffer.data   = (char*)GlobalAlloc( GPTR, MAX_BUFFER );

	m_sendBuffer.offset  = 0;
	m_sendBuffer.length  = MAX_BUFFER;
	m_sendBuffer.data    = (char*)GlobalAlloc( GPTR, MAX_BUFFER );

	/*-- RC5 --*/
	RC5_32_KEY*   rc532Key  = RC5_32_KEY_TABLE;
	unsigned char tiv1[ 4 ] = {'F','e','e','L'};
	unsigned char tiv2[ 4 ] = {'G','o','o','D'};
	DWORD         skey[ 2 ];

	memset( RC5_32_KEY_TABLE, 0, sizeof(RC5_32_KEY_TABLE) );

	for ( int i = 0; i < MAX_RC5_KEY; i++, rc532Key++ )
	{
		skey[ 0 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv1);
		skey[ 1 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv2);

		RC5_32_set_key( rc532Key, sizeof(skey), (unsigned char*)&skey, RC5_8_ROUNDS );
	}
}

cAsyncSelect::~cAsyncSelect(void)
{
	if ( m_connectThread != NULL )
	{
		WaitForSingleObjectEx( m_connectThread, INFINITE, TRUE );
		CloseHandle( m_connectThread );
		m_connectThread = NULL;
	}

	Disconnect( );

	m_backBuffer.offset = 0;
	m_backBuffer.length = 0;
	GlobalFree( m_backBuffer.data );

	m_frontBuffer.offset = 0;
	m_frontBuffer.length = 0;
	GlobalFree( m_frontBuffer.data );

	m_sendBuffer.offset = 0;
	m_sendBuffer.length = 0;
	GlobalFree( m_sendBuffer.data );
}

// Decrypt Method
void cAsyncSelect::Decrypt(BYTE* ptr, RC5_32_KEY key)
{
	unsigned char tiv[8] = {'I','R','I','S','I','R','I','S'};
	int           num    = 0;

	LONG  decryptlen = ((Packet*)ptr)->tlen;
	BYTE* decryptptr = ptr;

	decryptlen -= PHLen;
	decryptptr += PHLen;

	if ( decryptlen > 0 )
	{
		RC5_32_cfb64_encrypt( decryptptr, decryptptr, decryptlen, &key, tiv, &num, RC5_DECRYPT );
	}
}

// Encrypt Method
void cAsyncSelect::Encrypt(BYTE* ptr, RC5_32_KEY key)
{
	unsigned char tiv[8] = {'I','R','I','S','I','R','I','S'};
	int           num    = 0;

	LONG  encryptlen = ((Packet*)ptr)->tlen;
	BYTE* encryptptr = ptr;

	encryptlen -= PHLen;
	encryptptr += PHLen;

	if ( encryptlen > 0 )
	{
		RC5_32_cfb64_encrypt( encryptptr, encryptptr, encryptlen, &key, tiv, &num, RC5_ENCRYPT );
	}
}

// Decrypt Method
void cAsyncSelect::Decrypt(char* ptr, BYTE key)
{
	LONG  encryptlen = ((Packet*)ptr)->tlen;
	BYTE* encryptptr = (BYTE*)ptr;

	encryptlen -= PHLen;
	encryptptr += PHLen;

	if ( encryptlen > 0 )
	{
		for ( long i = 0; i < encryptlen; i++, encryptptr++ )
		{
			(*encryptptr) = (*encryptptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

/*-- Encrypt Method
*/
void cAsyncSelect::Encrypt(char* ptr, BYTE key)
{
	LONG  encryptlen = ((Packet*)ptr)->tlen;
	BYTE* encryptptr = (BYTE*)ptr;

	encryptlen -= PHLen;
	encryptptr += PHLen;

	if ( encryptlen > 0 )
	{
		for ( LONG i = 0; i < encryptlen; i++, encryptptr++ )
		{
			(*encryptptr) = (*encryptptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

bool cAsyncSelect::NmUserComeInGameReq( )
{
	MSG_REQ_COMEIN_GAME sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );

	sendMsg.Category     = NM_USER;
	sendMsg.Protocol     = NM_USER_COME_IN_GAME_REQ;
	sendMsg.NumOfServer	 = mDummyClient->GetCurServer();
	sendMsg.NumOfChannel = mDummyClient->GetCurChannel();
	sendMsg.Tutorial	 = 0;

	return SendNetworkMsg( (char*)&sendMsg, sizeof(sendMsg) );
}

bool cAsyncSelect::NmUserGameSrvReq( )
{
	MSG_REQ_GAMESRV sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );

	sendMsg.Category  = NM_USER;
	sendMsg.Protocol  = NM_USER_GAMESRV_REQ;
	sendMsg.LoginIdx  = m_loginIdx;
	sendMsg.MemberIdx = m_memberIdx;

	return SendNetworkMsg( (char*)&sendMsg, sizeof(sendMsg) );
}

bool cAsyncSelect::NmUserCharacterListReq( )
{
	MSGROOT sendMsg;
	::memset( &sendMsg, 0, sizeof(sendMsg) );

	sendMsg.Category = NM_USER;
	sendMsg.Protocol = NM_USER_CHARACTER_LIST_REQ;

	return SendNetworkMsg( (char*)&sendMsg, sizeof(sendMsg) );
}

bool cAsyncSelect::Connect(char* addr, u_short port)
{
	if ( strlen( addr ) )
	{
		m_addr = inet_addr( addr );
		m_port = htons( port );

		DWORD thread;
		m_connectThread = CreateThread( NULL, 0, ConnectThreadStartingPoint, (LPVOID)this, 0, &thread );
		return (m_connectThread != NULL) ? true : false;
	}
	return false;
}

bool cAsyncSelect::Connect(unsigned long ipv4, unsigned short port)
{
	m_addr = ipv4;
	m_port = htons( port );

	DWORD thread;
	m_connectThread = CreateThread( NULL, 0, ConnectThreadStartingPoint, (LPVOID)this, 0, &thread );
	return (m_connectThread != NULL) ? true : false;
}

void cAsyncSelect::Disconnect( )
{
	if ( m_socket != INVALID_SOCKET )
	{
		closesocket( m_socket );
		m_socket    = INVALID_SOCKET;
		m_connected = false;

		m_rc5 = true;
		m_seq = 0;

		memset( m_backBuffer.data, 0, m_backBuffer.offset );
		m_backBuffer.offset = 0;

		memset( m_frontBuffer.data, 0, m_frontBuffer.offset );
		m_frontBuffer.offset = 0;

		memset( m_sendBuffer.data, 0, m_sendBuffer.offset );
		m_sendBuffer.offset = 0;
	}
}

bool cAsyncSelect::SendHeartbeat( )
{
	if ( m_connected == true )
	{
		Packet packet;
		int    result;

		packet.ver  = PHVer;
		packet.hlen = PHLen;
		packet.tos  = TOS_TTL;
		packet.tlen = packet.hlen;
		packet.seq  = (++m_seq);

		result = send( m_socket, (char*)&packet, packet.tlen, 0 );
		if ( result == SOCKET_ERROR )
		{
			if ( GetLastError( ) != WSAEWOULDBLOCK )
				return false;
		}
		return true;
	}
	return false;
}

bool cAsyncSelect::SendNetworkMsg(char* msg, int len)
{
	if ( m_connected == true )
	{
		char*   ptr    = (m_sendBuffer.data + m_sendBuffer.offset);
		Packet* packet = (Packet*)ptr;
		char*   buffer = NULL;
		int     result;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_CLIENT;
		packet->tlen = packet->hlen + len;
		packet->seq  = (++m_seq);

		buffer = (ptr + packet->hlen);
		memcpy( buffer, msg, len );

		m_sendBuffer.offset += packet->tlen;

		/*-- --*/
		BYTE key = (BYTE)(rand( ) & 0x0f);
		if ( m_rc5 == true )
		{
			(*ptr) = ((*ptr) & 0xf0) | key;
			Encrypt( (BYTE*)ptr, RC5_32_KEY_TABLE[ ((*ptr) & 0x0f) ] );
		}
		else
		{
			Encrypt( ptr, key );
			(*ptr) = ((*ptr) & 0xf0) | key;
		}

		result = send( m_socket, ptr, packet->tlen, 0 );
		if ( result == SOCKET_ERROR )
		{
			if ( GetLastError( ) != WSAEWOULDBLOCK )
				return false;
		}

		memset( m_sendBuffer.data, 0, m_sendBuffer.offset );
		m_sendBuffer.offset = 0;
		return true;
	}
	return false;
}

bool cAsyncSelect::Batcomplete(char* ptr, int len)
{
	Packet* packet = (Packet*)ptr;

	if ( m_rc5 == true )
	{
		Decrypt( (BYTE*)ptr, RC5_32_KEY_TABLE[ packet->ver ] );
	}
	else
	{
		Decrypt( ptr, packet->ver );
	}

	switch ( packet->tos )
	{
	case TOS_LOGIN:
		{
			MSGROOT* msgRoot = (MSGROOT*)(ptr + packet->hlen);
			switch ( msgRoot->Category )
			{
			case NM_USER:
				switch ( msgRoot->Protocol )
				{
				case NM_USER_LOGIN_RES:
					{
						MSG_DIST_USERINFO* recvMsg = (MSG_DIST_USERINFO*)msgRoot;
						if ( recvMsg->ErrorCode == 0 )
						{
							m_loginIdx  = recvMsg->LoginIdx;
							m_memberIdx = recvMsg->UserIdx;

							NmUserComeInGameReq( );
						}
						else
						{
							if( mDummyClient )
								mDummyClient->ChangeStage( eStage_ReLogin );
						}
					}
					break;
				}
			}
		}
		return true;

	case TOS_LOGIN_ACCEPT:
		{
			m_rc5 = true;									// 암호화 변경.
			if( mDummyClient )
				mDummyClient->ChangeStage( eStage_Login );
		}
		return true;

	case TOS_LOGIN_2_GAME:
		{
			// 채널선택.
			Login2Game*    login2Game = (Login2Game*)packet;
			unsigned long  ipv4       = login2Game->ipv4;
			unsigned short port       = login2Game->port;

			Disconnect( );
			Connect( ipv4, port );
		}
		return false;

	case TOS_GAME:
		{
			MSGROOT* msgRoot = (MSGROOT*)(ptr + packet->hlen);
			switch ( msgRoot->Category )
			{
			case NM_USER:
				{
					if( msgRoot->Protocol == NM_USER_GAMESRV_RES )
					{
						MSG_ERROR* recvMsg = (MSG_ERROR*)msgRoot;
						if ( recvMsg->ErrorCode == 0 )
						{
							NmUserCharacterListReq( );
						}
					}
				}
				break;
			}

			if( mDummyClient )
				mDummyClient->NetWorkMsgParser( (char*)msgRoot );
		}
		return true;

	case TOS_GAME_ACCEPT:
		{
			m_rc5 = false;								// 암호화 변경.
			NmUserGameSrvReq( );						// 게임서버 인증시도.
		}
		return true;

	case TOS_GAME_2_GAME:
		{
			// 채널변경.
			Game2Game*     game2Game = (Game2Game*)packet;
			unsigned long  ipv4       = game2Game->ipv4;
			unsigned short port       = game2Game->port;

			Disconnect( );
			Connect( ipv4, port );
		}
		return false;
	}
	return false;
}

void cAsyncSelect::Process( )
{
	DWORD currentTickCount = GetTickCount( );

	if( currentTickCount > mLastTickCount )
	{
		SendHeartbeat( );
		mLastTickCount = currentTickCount + 60000;
	}
}

void cAsyncSelect::FdReceive(SOCKET socket)
{
	Packet* packet;
	char*   buffer;
	long    length;
	long    offset;
	int     nbr;

	// 이전에 사용된 데이터 삭제.
	if ( m_frontBuffer.offset > 0 )
	{
		memset( m_frontBuffer.data, 0, m_frontBuffer.offset );
		m_frontBuffer.offset = 0;
	}

	// 이어받기 할 경우, 이전 데이터 복사.
	if ( m_backBuffer.offset > 0 )
	{
		memcpy( m_frontBuffer.data, m_backBuffer.data, m_backBuffer.offset );
		m_frontBuffer.offset = m_backBuffer.offset;

		memset( m_backBuffer.data, 0, m_backBuffer.offset );
		m_backBuffer.offset = 0;
	}

	// 소켓을 사용하여 데이터를 읽어 온다.
	do {
		buffer = m_frontBuffer.data   + m_frontBuffer.offset;
		length = m_frontBuffer.length - m_frontBuffer.offset;
		nbr    = recv( socket, buffer, length, 0 );

		if ( nbr == SOCKET_ERROR )
		{
			if ( WSAGetLastError( ) != WSAEWOULDBLOCK )
			{
				FdClose( socket );
				return;
			}
		}
		if ( nbr > 0 )
		{
			m_frontBuffer.offset += nbr;
		}
	} while ( nbr > 0 );

	buffer = m_frontBuffer.data;
	length = m_frontBuffer.offset;
	offset = 0;

	while ( length >= PHLen )
	{
		packet = (Packet*)(buffer + offset);

		if ( packet->hlen != PHLen || packet->tlen < PHLen )
		{
			FdClose( socket );
			return;
		}

		if ( length < (long)packet->tlen )
			break;

		// Todo here
		if ( Batcomplete( (char*)packet, packet->tlen ) == false )
			return;

		length -= packet->tlen;
		offset += packet->tlen;
	}

	if ( length > 0 )
	{
		buffer = (m_frontBuffer.data + offset);
		memcpy( m_backBuffer.data, buffer, length );
		m_backBuffer.offset = length;
	}
}

void cAsyncSelect::FdWrite(SOCKET socket)
{}

void cAsyncSelect::FdClose(SOCKET socket)
{
	PostMessage( m_wnd, m_msg, socket, FD_DISCONNECT );
}

void cAsyncSelect::FdConnected(SOCKET socket)
{
	if( mDummyClient == 0 )
		return;

	if ( m_connected )
	{
		// success
	}
	else
	{
		// fail
		if( mDummyClient )
			mDummyClient->ChangeStage( eStage_Disconnet );
	}
}

void cAsyncSelect::FdDisconnect(SOCKET socket)
{
	Disconnect( );

	if( mDummyClient )
		mDummyClient->ChangeStage( eStage_Disconnet );
}

HRESULT cAsyncSelect::NetWorkProc( WPARAM wParam, LPARAM lParam)
{
	SOCKET socket = (SOCKET)wParam;
	switch ( WSAGETSELECTEVENT( lParam ) )
	{
	case FD_READ:
		FdReceive( socket );
		break;

	case FD_WRITE:
		FdWrite( socket );
		break;

	case FD_CLOSE:
		FdClose( socket );
		break;

	case FD_CONNECTED:
		FdConnected( socket );
		break;

	case FD_DISCONNECT:
		FdDisconnect( socket );
		break;
	}
	return 0L;
}

DWORD cAsyncSelect::ConnectThread(void* ptr)
{
	// 소켓 핸들이 열여 있을경우, 종료후 사용.
	Disconnect( );

	m_socket    = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	m_connected = false;
	if ( m_socket != INVALID_SOCKET )
	{
		sockaddr_in addrIn;
		int         result;

		addrIn.sin_family = AF_INET;
		addrIn.sin_port   = m_port;
		addrIn.sin_addr.s_addr = m_addr;

		result = connect( m_socket, (LPSOCKADDR)&addrIn, sizeof(addrIn) );
		if ( result != SOCKET_ERROR )
		{
			result = WSAAsyncSelect( m_socket, m_wnd, m_msg, FD_READ | FD_WRITE | FD_CLOSE );
			if ( result != SOCKET_ERROR )
			{
				m_connected = true;
				PostMessage( m_wnd, m_msg, m_socket, FD_CONNECTED );
				return 1L;
			}
		}
		Disconnect( );
	}
	PostMessage( m_wnd, m_msg, m_socket, FD_CONNECTED );
	return 0L;
}

DWORD cAsyncSelect::ConnectThreadStartingPoint(void* ptr)
{
	cAsyncSelect* asyncSelect = (cAsyncSelect*)ptr;
	return asyncSelect->ConnectThread( ptr );
}
