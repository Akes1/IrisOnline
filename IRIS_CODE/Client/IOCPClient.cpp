/*-- Include
*/
#include "StdAfx.h"

#include "iocpclient.h"
#include "criticalsectionlock.h"

#include "StageManager.h"
#include "UIMsgboxEvent.h"
#include "GameResourceManager.h"
#include "UIManager.h"
//#include "Application.h"

#include "packet.h"
//#include "HShield.h"
//#include "HackShield_Common.h"

/*-- Local definitions
*/
#pragma warning( disable: 4127 )
#pragma warning( disable: 4702 )

#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // 핸들를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // "

#define MAX_CONNECT_RETRY	3	// 재시도 제한 횟수 최대


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


/*-- Global data
*/
cIOCPClient* cIOCPClient::mIOCPClient = NULL;

#define MAX_RC5_KEY				16

RC5_32_KEY RC5_32_KEY_TABLE[ MAX_RC5_KEY ];

BYTE SY_RKEY_TABLE[16][4]=
{
	{ 0x0F,0xAD,0x37,0x6C },	//6
	{ 0x5C,0x55,0x6A,0xD8 },	//10
	{ 0x6C,0xA9,0xDB,0x64 },	//11
	{ 0x5A,0xBE,0xF2,0xF6 },	//12
	{ 0xB3,0xB7,0xEC,0x7B },	//8
	{ 0xE8,0x9B,0x3A,0x8A },	//5
	{ 0xD9,0x64,0x28,0x5C },	//7
	{ 0xA6,0x4D,0x8F,0xB9 },	//13
	{ 0x17,0xb3,0x67,0xaa },	//0
	{ 0x4B,0x1C,0x15,0x11 },	//4
	{ 0x9C,0x13,0xFE,0xF5 },	//9
	{ 0x53,0xf2,0x74,0xb1 },	//1
	{ 0x67,0x9F,0x7D,0xF6 },	//14
	{ 0x98,0xea,0x55,0x29 },	//3
	{ 0x8c,0x72,0x59,0x32 },	//2
	{ 0xCA,0x6A,0xA9,0xEF },	//15
};

BYTE SY_KEY_TABLE[16][4]=
{
	{ 0xC0,0x3C,0x06,0xEA },	//0
	{ 0x68,0xCC,0x56,0x3A },	//10
	{ 0xB9,0x78,0xB3,0x45 },	//1
	{ 0xA1,0x8A,0x0B,0x70 },	//12
	{ 0x3B,0xB1,0x60,0x0B },	//8
	{ 0x88,0x48,0xC0,0x71 },	//2
	{ 0x43,0xC8,0x53,0x63 },	//7
	{ 0x5C,0xF8,0x90,0x4A },	//11
	{ 0x54,0x1B,0x0F,0x9C },	//15
	{ 0x44,0x95,0xAF,0x99 },	//6
	{ 0x85,0x4C,0xB6,0xE7 },	//3
	{ 0x28,0x45,0x80,0xB1 },	//13
	{ 0x2D,0x83,0x06,0x55 },	//5
	{ 0x98,0xAE,0xD9,0xBF },	//4
	{ 0x71,0x9D,0x4A,0x80 },	//14
	{ 0x19,0x2D,0x57,0xC6 },	//9
};

// ErrorLog prototype
void ErrorLog( char* format, ... )
{
	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;

	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) + 1;
	msgBuf       = malloc( bufferLength );

	vsprintf_s( (char*)msgBuf, bufferLength, format, args );

	va_end( args );

	if ( msgBuf != NULL )
	{
		FILE*      stream = NULL;
		char       filename[ MAX_PATH ];
		SYSTEMTIME systemtime;
		char       buffer[ 1024 ];

		GetLocalTime( &systemtime );

		sprintf_s( filename,
				 "Network_%04d_%02d_%02d.log",
				 systemtime.wYear,
				 systemtime.wMonth,
				 systemtime.wDay );

		if ( fopen_s( &stream, filename, "at" ) != 0 )
		{
			sprintf_s( buffer,
					 "%04d %02d %02d::%02d %02d %02d : %s\n",
					 systemtime.wYear,
					 systemtime.wMonth,
					 systemtime.wDay,
					 systemtime.wHour,
					 systemtime.wMinute,
					 systemtime.wSecond,
					 (char*)msgBuf );

			fputs( buffer, stream );
			fclose( stream );
		}

		free( msgBuf );
	}
}
/*-- cIOCPClient Constructor
*/
cIOCPClient::cIOCPClient(HWND wnd, DWORD messageID) : mWnd( wnd ), mMessageID( messageID )
{
	// 정적변수 설정.
	mIOCPClient = this;

	// ws2_32.dll 준비
	mVersionRequested = MAKEWORD(2,2);
	::WSAStartup( mVersionRequested, &mWsaData );

	// Critical Section를 초기화.
	InitializeCriticalSection( &mCs );

	// 내부변수 초기화.
	mIocp                   = NULL;
	mSocket                 = INVALID_SOCKET;
	mIocpWorkerThreadNumber = 0;

	mIoContextPool          = NULL;

	mIoContextFrontBuffer   = NULL;
	mIoContextBackBuffer    = NULL;

	mConnectThread          = NULL;
	mConnected              = false;
	mConnectionResult       = true;
	mConnectRetry           = 0;

	mRC5Enable              = true;

	mLastTos                = 0;
	mLastCategory           = 0;
	mLastProtocol           = 0;

	mWepLuncherInit			= false;

	/*-- ...
	*/
	RC5_32_KEY*   rc532Key  = RC5_32_KEY_TABLE;		// ...
	unsigned char tiv1[ 4 ] = {'s','L','e','e'};	//
	unsigned char tiv2[ 4 ] = {'p','H','A','h'};	//
	DWORD         skey[ 2 ];						// ...

	memset( RC5_32_KEY_TABLE, 0, sizeof(RC5_32_KEY_TABLE) );

	for ( int i = 0; i < MAX_RC5_KEY; i++, rc532Key++ )
	{
		skey[ 0 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv1);
		skey[ 1 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv2);

		RC5_32_set_key( rc532Key, sizeof(skey), (unsigned char*)&skey, RC5_8_ROUNDS );
	}
}

/*-- ~cIOCPClient Destructor
*/
cIOCPClient::~cIOCPClient(void)
{
	// 종료
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &mCs );

	// ws2_32.dll 종료
	::WSACleanup( );

	// 윈도우 핸들 & 메시지 초기.
	mWnd        = NULL;
	mMessageID  = 0;

	// 정적변수 값 삭제.
	mIOCPClient = NULL;
}

/*-- Initialize Method
*/
bool cIOCPClient::Initialize(unsigned short numWorkerThreads)
{
	/*-- CreateIoCompletionPort를 이용하여 completion port 오브젝트를 생성할 때 일반적으로 신경써야 할 파
		 라메터는 NumberOfConcurrentThreads 이다. 처음 3개의 파라메터는 값을 설정하지 않는다.

		 NumberOfConcurrentThreads는 현재 completion port에 대하여 몇개의 스레드를 실행할 것인가를 결정한
		 다. 가장 이상적인 경우는 스레드의 context switching을 방지하기 위하여 서비스에 대하여 하나의 프로
		 세서당 하나의 스레드만 동작하도록 하는 것이다. NumberOfConcurrentThreads의 값을 0으로 설정하면 시
		 스템은 프로세서에 적합한 스레드 개수를 할당하여 사용한다.
		 (하나의 CPU에 대하여 IOCP 하나당 하나의 스레드를 생성)
	*/
	mIocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( mIocp == NULL )
		return false;

	/*-- CreateIoCompletionPort 오브젝트를 생성한 뒤에 작업스레드를 생성하고 소켓핸들을 지정할 수 있는데,
	     이때 생성하는 작업스레드는, 스레드가 블록될 가능성이 있다면, NumberOfConcurrentThreads 의 갯수보
		 다 많이 생성하는 것이 좋다. NumberOfConcurrentThreads만큼 스레드가 항상 실행되게 할 수 있기 때문
		 에, 블록된 스레드가 있다면 다른 스레드가 대신 동작할 것이기 때문이다.
	*/
	SYSTEM_INFO si;
	DWORD       threadId;

	// 시스템에 몇 개의 프로세서가 있는지 확인한다.
	GetSystemInfo( &si );

	mIocpWorkerThreadNumber = min( si.dwNumberOfProcessors * numWorkerThreads, IOCP_MAX_WORKER_THREAD );
	for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
	{
		mIocpWorkerThread[i] = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
		if ( mIocpWorkerThread[i] == NULL )
			return false;
	}

	/*-- I/O Context Pool를 생성한다.
	*/
	mIoContextPool = new cIoContextPool( );
	if ( mIoContextPool == NULL )
		return false;

	// I/O Context Buffer를 생성한다.
	mIoContextFrontBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mIoContextBackBuffer  = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	if ( !mIoContextFrontBuffer && !mIoContextBackBuffer )
		return false;

	// 환경변수 설정.
	mConnected        = false;
	mConnectionResult = true;
	return true;
}

/*-- Shutdown Method
*/
void cIOCPClient::Shutdown(DWORD maxWait)
{
	/*-- Connect Thread가 종료될때까지 대기.
	*/
	WaitForSingleObjectEx( mConnectThread, maxWait, TRUE );
	SafeCloseHandle( mConnectThread );

	/*-- 서버와 연결종료.
	*/
	SOCKET sockTemp = mSocket;

	/*-- 소켓을 안전하게 종료한다.
	*/
	mSocket           = INVALID_SOCKET;
	mConnected        = false;
	mConnectionResult = false;

	if ( sockTemp != INVALID_SOCKET )
	{
		closesocket( sockTemp );
	}

	/*-- Worket Thread 종료요청.
	*/
	if ( mIocp != NULL )
	{
		for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
		{
			PostQueuedCompletionStatus( mIocp, 0, 0, IOCP_SHUTDOWN );
		}
	}

	/*-- Worket Thread 종료확인 및 강제종료.
	*/
	for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
	{
		if ( WaitForSingleObject( mIocpWorkerThread[ i ], 60000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( mIocpWorkerThread[ i ], &exitCode);
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( mIocpWorkerThread[ i ], 0 );
			}
		}
		SafeCloseHandle( mIocpWorkerThread[ i ] );
	}

	/*-- Overlapped I/O Model Pool및 Socket Context Pool를 해제한다.
	*/
	if ( mIoContextFrontBuffer )
	{
		GlobalFree( mIoContextFrontBuffer );
		mIoContextFrontBuffer = NULL;
	}
	if ( mIoContextBackBuffer )
	{
		GlobalFree( mIoContextBackBuffer );
		mIoContextBackBuffer = NULL;
	}
	SafeDelete( mIoContextPool );

	/*-- completion port를 해제한다.
	*/
	SafeCloseHandle( mIocp );
}

/// PostConnect Method
bool cIOCPClient::PostConnect(char* addr, unsigned short port)
{
	cCSLock lock( &mCs );

	if ( mConnectThread != NULL )
		return false;

	// 주소변환.
	ZeroMemory( (void*)&mAddr, sizeof(mAddr) );
	mAddr.sin_family = AF_INET;
	mAddr.sin_port   = htons( port );
	mAddr.sin_addr.s_addr = inet_addr( addr );

	if ( mAddr.sin_addr.s_addr == INADDR_NONE )
	{
		PHOSTENT phe = gethostbyname( addr );
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( phe != NULL )
			CopyMemory( &mAddr.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}

	mSeq = 0;
	mConnectRetry = 0;
	return PostConnect( );
}
bool cIOCPClient::PostConnect(unsigned long ipv4, unsigned short port)
{
	cCSLock lock( &mCs );

	if ( mConnectThread != NULL )
		return false;

	// 주소변환.
	ZeroMemory( (void*)&mAddr, sizeof(mAddr) );
	mAddr.sin_family = AF_INET;
	mAddr.sin_port   = htons( port );
	mAddr.sin_addr.s_addr = ipv4;

	if ( mAddr.sin_addr.s_addr == INADDR_NONE )
		return false;

	mSeq = 0;
	mConnectRetry = 0;
	return PostConnect( );
}
bool cIOCPClient::PostConnect( )
{
	if ( mConnectRetry < MAX_CONNECT_RETRY )
	{
		// 소켓검사.
		if ( mSocket != INVALID_SOCKET )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
		}

		// connect할 클라이언트 소켓을 생성한다.
		mSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED );
		if ( mSocket == INVALID_SOCKET )
			return false;

		// 연결 결과값 초기화.
		mConnected        = false;
		mConnectionResult = false;
		mConnectRetry++;

		DWORD threadId;
		mConnectThread = CreateThread( NULL, 0, ConnectThreadStartingPoint, (LPVOID)this, 0, &threadId );
		return (mConnectThread != NULL) ? true : false;
	}
	return false;
}

/*-- GetConnectionResult Method;
*/
bool cIOCPClient::GetConnectionResult( )
{
	cCSLock lock( &mCs );

	/*-- Connect Thread가 종료될때까지 대기.
	*/
	WaitForSingleObjectEx( mConnectThread, INFINITE, TRUE );
	SafeCloseHandle( mConnectThread );

	if ( mConnected == true )
	{
		/*-- The purpose of this algorithm is to reduce the number of very small segments sent, especially on
		high-delay (remote) links.

		Windows Sockets applications can disable the Nagle algorithm for their connections by setting the
		TCP_NODELAY socket option. However, this practice should be used wisely and only when needed because
		this practice increases network traffic. Usually, applications that need to disable the Nagle algorithm
		are applications that need to send small packets and require that the peer application receive them
		immediately. Some network applications may not perform well if their design does not take into account
		the effects of transmitting large numbers of small packets and the Nagle algorithm.

		Nagel 알고리즘의 목적은 특히 고지연(원격) 링크에서 크기가 작은 세그먼트가 전송되는 횟수를 줄이는
		것이다.

		Windows 소켓 응용 프로그램에서는 자신의 연결에 대해 Nagle 알고리즘을 사용할 수 없도록 TCP_NODELAY
		소켓 옵션을 설정할 수 있습니다. 그러나 이러한 작업은 네트워크 이용을 늘리기 때문에 반드시 필요한
		경우가 아니면 가급적 사용하지 않아야 합니다. 다수의 소형 패킷 전송으로 인해 발생하는 영향과 Nagle
		알고리즘을 고려하지 않은 상태로 설계된 네트워크 응용 프로그램은 제대로 수행되지 않을 수 있습니다.
		Nagle 알고리즘은 성능상의 이유로 인해 TCP 연결을 루프백하는 데는 적용되지 않습니다. Windows 2000
		Netbt는 직접 호스트된 리디렉터/서버 연결뿐만 아니라 TCP를 통한 NetBIOS 연결에 대해서도 네이글링을
		사용할 수 없게 만듭니다. 따라서 다수의 소형 파일 조작 명령을 발행하는 응용 프로그램의 경우 성능이
		향상될 수 있습니다. 그러한 응용 프로그램으로는 파일 잠금/잠금 해제 기능을 자주 사용하는 응용 프로
		그램을 들 수 있습니다. 
		*/

		int nodelay = 1;
		if ( setsockopt( mSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay) ) == SOCKET_ERROR )
			return false;

		int zero;
		/*-- Disable receive buffering on the socket. Setting SO_RCVBUF to 0 causes winsock to stop bufferring
			receive and perform receives directly from our buffers, thereby reducing CPU usage.

			SO_RCVBUF를 0으로하여 수신버퍼를 사용하지 않는다. 수신버퍼가 0이 아니면 불필요하게 이 수신버퍼를
			거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
			수신버퍼 공간이 없으면 overlapped I/O를 호출했을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
			다.
		*/
		zero = 0;
		if ( setsockopt( mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero)) == SOCKET_ERROR )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}

		/*-- Disable send buffering on the socket. Setting SO_SNDBUF to 0 causes winsock to stop bufferring
			sends and perform sends directly from our buffers, thereby reducing CPU usage.

			SO_SNDBUF를 0으로하여 송신버퍼를 사용하지 않는다. 송신버퍼가 0이 아니면 불필요하게 이 송신버퍼를
			거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
			송신버퍼 공간이 없으면 overlapped I/O를 호출햇을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
			다.
		*/
		zero = 0;
		if ( setsockopt( mSocket, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero) ) == SOCKET_ERROR )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}

		LINGER linger;
		/*-- closesocket 함수가 호출된 이후에 보내진 데이터에 대한 처리를 제어하기 위한 옵션이다.
			linger::u_short l_onoff  : option on/off
			linger::u_short l_linger : linger time

			SO_LINGER 옵션의 타임아웃을 0으로 설정하면 (linger 구조체의 l_onoff 필드를 0이 아닌값, l_linger
			를 0으로 설정), 아직 전송되지 않은 데이터가 있더라도 closesocket 함수 호출은 블록되지 않고 즉시
			끝난다. 이렇게 호출된 소켓의 가성연결 (virtual circuit)은 즉시 단절(reset)되고 전송되지 않은 데
			이터는 손실된다. 데이터 수신하는 측은 수신 함수가 WSAECONNRESET로 실패한다.
		*/
		linger.l_onoff  = 1;
		linger.l_linger = 0;
		if ( setsockopt( mSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger) ) == SOCKET_ERROR )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}

		/*-- connect에 의하여 리턴된 소켓 핸들을 completion port에 할당한다. --*/
		if ( CreateIoCompletionPort( (HANDLE)mSocket, mIocp, (ULONG_PTR)NULL, 0 ) == NULL )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}

		/*-- 수신을 위해 I/O Context를 준비한다. --*/
		PerIoContext* perIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_READ );
		if ( perIoContext == NULL )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}
		if ( RecvPost( perIoContext ) == false )
		{
			closesocket( mSocket );
			mSocket = INVALID_SOCKET;
			return (mConnectionResult = false);
		}
		else
			return (mConnectionResult = true);
	}
	else
	{
		closesocket( mSocket );
		mSocket = INVALID_SOCKET;
		return (mConnectionResult = false);
	}
}

/*-- Disconnect Method
*/
bool cIOCPClient::Disconnect(void)
{
	if ( mConnected == true && mConnectionResult == true )
	{
		cCSLock lock( &mCs );
		SOCKET  sockTemp = mSocket;

		/*-- 소켓을 안전하게 종료한다.
		*/
		mSocket           = INVALID_SOCKET;
		mConnected        = false;
		mConnectionResult = false;

		if ( sockTemp != INVALID_SOCKET )
		{
			closesocket( sockTemp );
		}

		PostMessage( mWnd, mMessageID, (WPARAM)ICT_DISCONNECT, (LPARAM)0L );
		return true;
	}
	return false;
}

/*-- SendNetworkMsg Method;
*/
bool cIOCPClient::SendNetworkMsg(char* buf, u_short len)
{
	if ( IsConnected( ) == true )
	{
		cCSLock       lock( &mCs );
		try {
			PerIoContext* perIoConect = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
			Packet*       packet      = (Packet*)perIoConect->buffer;
			char*         buffer      = NULL;

			packet->ver  = PHVer;
			packet->hlen = PHLen;
			packet->tos  = TOS_CLIENT;
			packet->tlen = packet->hlen + len;
			packet->seq  = (++mSeq);

			buffer = perIoConect->buffer + packet->hlen;
			memcpy( buffer, buf, len );

			perIoConect->offset = packet->tlen;
			return SendPost( perIoConect );
		} catch ( ... ) {
			ErrorLog( "Throwing 'cIOCPClient::SendNetworkMsg' exception" );
			throw;
		}
	}
	return false;
}

bool cIOCPClient::SendHackShieldMsg(char* buf, u_short len)
{
	if ( IsConnected( ) == true )
	{
		cCSLock       lock( &mCs );
		try {
			PerIoContext* perIoConect = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
			Packet*       packet      = (Packet*)perIoConect->buffer;
			char*         buffer      = NULL;

			packet->ver  = PHVer;
			packet->hlen = PHLen;
			packet->tos  = TOS_HACKSHIELD;
			packet->tlen = packet->hlen + len;
			packet->seq  = (++mSeq);

			buffer = perIoConect->buffer + packet->hlen;
			memcpy( buffer, buf, len );

			perIoConect->offset = packet->tlen;
			return SendPost( perIoConect );
		} catch ( ... ) {
			ErrorLog( "Throwing 'cIOCPClient::SendHackShieldMsg' exception" );
			throw;
		}
	}
	return false;
}

/*-- ReleaseNetworkMsg Method;
*/
bool cIOCPClient::ReleaseNetworkMsg(PerIoContext* perIoContext)
{
	DWORD error = 0;
	if ( perIoContext->socket == mSocket )
	{
		mIoContextPool->ReleaseIoContext( perIoContext );
		if ( error != 0 )
			assert( "WARNING - cIOCPClient::ReleaseNetworkMsg." );
		return true;
	}
	return false;
}

// SendHeartbeat Method
bool cIOCPClient::SendHeartbeat( )
{
	if ( IsConnected( ) == true )
	{
		cCSLock       lock( &mCs );
		PerIoContext* perIoConect = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
		Packet*       packet      = (Packet*)perIoConect->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_TTL;
		packet->tlen = packet->hlen;
		packet->seq  = (++mSeq);

		perIoConect->offset = packet->tlen;
		return SendPost( perIoConect );
	}
	return false;
}

// Decrypt Method
void cIOCPClient::Decrypt(BYTE* ptr, RC5_32_KEY key)
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
void cIOCPClient::Encrypt(BYTE* ptr, RC5_32_KEY key)
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
void cIOCPClient::Decrypt(BYTE key, long len, char* ptr)
{
	len -= PHLen;
	ptr += PHLen;

	if ( len > 0 )
	{
		for ( long i = 0; i < len; i++, ptr++ )
		{
			(*ptr) = (*ptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

/*-- Encrypt Method
*/
void cIOCPClient::Encrypt(BYTE key, long len, char* ptr)
{
	len -= PHLen;
	ptr += PHLen;

	if ( len > 0 )
	{
		for ( LONG i = 0; i < len; i++, ptr++ )
		{
			(*ptr) = (*ptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

/*-- SendPost Method
*/
bool cIOCPClient::SendPost(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  sendBytes = 0;
	DWORD  flags     = 0;
	int    retcode;

	wsaBuf.len = perIoContext->offset;
	wsaBuf.buf = perIoContext->buffer;

	if ( mRC5Enable )
	{
		BYTE  key = (BYTE)(rand( ) & 0x0f);
		(*wsaBuf.buf) = ((*wsaBuf.buf) & 0xf0) | key;

		Encrypt( (BYTE*)wsaBuf.buf, RC5_32_KEY_TABLE[ ((*wsaBuf.buf) & 0x0f) ] );
	}
	else
	{
		BYTE  key = (BYTE)(rand( ) & 0x0f);

		Encrypt( key, wsaBuf.len, wsaBuf.buf );
		(*wsaBuf.buf) = ((*wsaBuf.buf) & 0xf0) | key;
	}

	retcode = WSASend( perIoContext->socket,
					   &wsaBuf,
					   1,
					   &sendBytes,
					   flags,
					   &(perIoContext->wsaOverlapped),
					   NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		PostQueuedCompletionStatus( mIocp, 0, NULL, (LPOVERLAPPED)perIoContext );
		return false;
	}
	return true;
}

/*-- RecvPost Method
*/
bool cIOCPClient::RecvPost(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  recvBytes = 0;
	DWORD  flags     = 0;
	int    retcode;

	wsaBuf.len = (perIoContext->length - perIoContext->offset);
	wsaBuf.buf = (perIoContext->buffer + perIoContext->offset);

	retcode = WSARecv( perIoContext->socket,
					   &wsaBuf,
					   1,
					   &recvBytes,
					   &flags,
					   &(perIoContext->wsaOverlapped),
					   NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		PostQueuedCompletionStatus( mIocp, 0, NULL, (LPOVERLAPPED)perIoContext );
		return false;
	}
	return true;
}

/*-- Close Method - CompletionKey와 Overlapped를 비교후 CompletionKey를 삭제한다.
*/
void cIOCPClient::Close(ULONG_PTR /*completionKey*/, PerIoContext* perIoContext)
{
	cCSLock lock( &mCs );
	DWORD   error = 0;

	// 연결해제 원인 검사.
	if ( (mConnected == true) && (mConnectionResult == true) )
	{
		SOCKET  sockTemp = mSocket;

		/*-- 소켓을 안전하게 종료한다.
		*/
		mSocket           = INVALID_SOCKET;
		mConnected        = false;
		mConnectionResult = false;

		if ( sockTemp != INVALID_SOCKET )
		{
			closesocket( sockTemp );
		}

		PostMessage( mWnd, mMessageID, ICT_CONNECTION_DEAD, (LPARAM)GetLastError( ) );
	}

	// 사용이 완료된 I/O Context를 회수.
	mIoContextPool->ReleaseIoContext( perIoContext );
	if ( error != 0 )
		assert( "WARNING - cIOCPClient::Close." );
}

/*-- SendComplete Method
*/
bool cIOCPClient::SendComplete(ULONG_PTR, PerIoContext* perIoContext, DWORD)
{
	// 사용이 완료된 I/O Context는 회수한다.
	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext );
	if ( error != 0 )
		assert( "WARNING - cIOCPClient::SendComplete." );
	return true;
}

/*-- RecvComplete Method (Default Echo Server)
*/
bool cIOCPClient::RecvComplete(ULONG_PTR, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock        lock( &mCs );
	PerIoContext** backBuffer = mIoContextBackBuffer->buffer;
	long&          backOffset = mIoContextBackBuffer->offset;

	// 오프셋의 크기를 누적한다.
	perIoContext->offset += bytesTransferred;

	DWORD length = perIoContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset = 0;                    // 수신된 누적버퍼의 오프셋

	while ( length >= PHLen )
	{
		Packet* packet    = (Packet*)(perIoContext->buffer + offset);
		DWORD   packetLen = packet->tlen;

		// 데이터의 길이가 최소 단위보다 작을 경우 오류 처리해야 한다.
		if ( packetLen < PHLen )
			return false;

		// 데이터의 길이가 완전할때까지 이어 받기를 해야 한다.
		if ( packetLen > length )
			break;

		// 여기서부터 데이터 처리를 위한 코드를 삽입한다.
		PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_READ );

			memcpy( ioContext->buffer, packet, packet->tlen );
			ioContext->offset = packet->tlen;

			if ( mRC5Enable )
			{
				Decrypt( (BYTE*)ioContext->buffer, RC5_32_KEY_TABLE[ packet->ver ] );
			}
			else
			{
				Decrypt( packet->ver, packet->tlen, ioContext->buffer );
			}

			if ( backOffset < MAX_IO_CONTEXT_BUFFER_LEN )
			{
				backBuffer[ backOffset ] = ioContext;
				backOffset++;
			}

		// 처리후 Offset 및 Length를 다시 계산한다.
		length -= packetLen;
		offset += packetLen;
	}

	// 이어 받기를 위해 데이터를 보관한한다.
	if ( offset > 0 )
	{
		PerIoContext* recvIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_READ );
		DWORD         error         = 0;

		if ( length > 0 )
		{
			memcpy( recvIoContext->buffer, (perIoContext->buffer + offset), length );
			recvIoContext->offset = length;
		}

		mIoContextPool->ReleaseIoContext( perIoContext );
		if ( error != 0 )
			assert( "WARNING - cIOCPClient::RecvComplete." );

		perIoContext = recvIoContext;
	}

	return RecvPost( perIoContext );
}

/*-- Process Method;
*/
DWORD cIOCPClient::ConnectThread(void)
{
	cCSLock lock( &mCs );
	mConnected = (connect( mSocket, (LPSOCKADDR)&mAddr, sizeof(mAddr) ) != SOCKET_ERROR ) ? true : false;
	PostMessage( mWnd, mMessageID, (WPARAM)ICT_CONNECT, (LPARAM)0L );
	return 0L;
}

/*-- Process Method;
*/
bool cIOCPClient::Process(WPARAM wParam, LPARAM lParam)
{
	IOCP_CLIENT_TYPE type = (IOCP_CLIENT_TYPE)wParam;
	switch ( type )
	{
	case ICT_CONNECT:
		// 클라언트에서 연결를 요청한 경우...
		if ( GetConnectionResult( ) )
		{
			/// 연결성공.
			if( mWepLuncherInit == false )
			{
				/*long authcode = THEAPP->GetAuthCode();

				if( authcode != -1 )
				{
					MSG_REQ_WEB_LAUNCHER msg;
					msg.Category = NM_USER;
					msg.Protocol = NM_USER_WEB_LAUNCHER_REQ;
					msg.LoginIdx = authcode;
					SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}*/

				mWepLuncherInit = true;
			}
		}
		else if ( PostConnect( ) == false )
		{
			/// 연결실패.
			UIMAN->SendEventTopMsgBox( false );
			UIMAN->CreateMsgBox( eStage_Login
								,eMSGBOX_MODAL
								,eMBEVENT_CONNECT_LOGINFAILE
								,eSKIN_OK
								,GAMERESOURCEMAN->GetGameText( 3 )
								,GAMERESOURCEMAN->GetGameText( 301 ) );
		}
		break;

	case ICT_DISCONNECT:
		// 클라언트에서 연결 해제를 요청한 경우만...
		break;

	case ICT_CONNECTION_DEAD:
		{
			// 서버로부터 연결이 해제 되었을 경우...
			UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
				eMBEVENT_CONNECT_DEAD, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 61 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;

	case ICT_RECV:
		{
			IoContextBuffer* temp;

			// Double Buffering 처리.
			EnterCriticalSection( &mCs );

				temp                  = mIoContextBackBuffer;
				mIoContextBackBuffer  = mIoContextFrontBuffer;
				mIoContextFrontBuffer = temp;

			LeaveCriticalSection( &mCs );

			// 전송시작.
			PerIoContext** frontBuffer = mIoContextFrontBuffer->buffer;
			long&          frontOffset = mIoContextFrontBuffer->offset;

			while ( frontOffset > 0 )
			{
				PerIoContext* perIoContext = (*frontBuffer);
				Packet*       packet       = (Packet*)perIoContext->buffer;

				char*         buffer       = perIoContext->buffer + packet->hlen;
				u_long        length       = packet->tlen - packet->hlen;

				mLastTos = packet->tos;
				switch ( packet->tos )
				{
				case TOS_LOGIN:
					try {
						MSGROOT* msgRoot = (MSGROOT*)buffer;
						mLastCategory = msgRoot->Category;
						mLastProtocol = msgRoot->Protocol;

						STAGEMAN->NetworkMsgParser( 0, buffer, length );
						ReleaseNetworkMsg( perIoContext );
					} catch ( ... ) {
						ErrorLog( "In TOS_LOGIN:NetworkMsgParser(%d,%d) Method. Throwing 'cIOCPClient::Process' exception", mLastCategory, mLastProtocol );
						throw;
					}
					break;

				case TOS_LOGIN_ACCEPT:
					mRC5Enable = true;

					/// 로그인 서버와의 연결 성공
					UIMAN->SendEventTopMsgBox( true );
					ReleaseNetworkMsg( perIoContext );
					break;

				case TOS_LOGIN_2_GAME:
					{
						/// 게임서버 접속.
						Login2Game*    login2Game = (Login2Game*)packet;
						unsigned long  ipv4       = login2Game->ipv4;
						unsigned short port       = login2Game->port;

						ReleaseNetworkMsg( perIoContext );
						PostConnect( ipv4, port );
					}
					break;

				case TOS_GAME:
					try {
						MSGROOT* msgRoot = (MSGROOT*)buffer;
						mLastCategory = msgRoot->Category;
						mLastProtocol = msgRoot->Protocol;

						STAGEMAN->NetworkMsgParser( 0, buffer, length );
						ReleaseNetworkMsg( perIoContext );
					} catch ( ... ) {
						ErrorLog( "In TOS_GAME:NetworkMsgParser(%d,%d) Method. Throwing 'cIOCPClient::Process' exception", mLastCategory, mLastProtocol );
						throw;
					}
					break;

				case TOS_GAME_ACCEPT:
					{
						mRC5Enable = false;

						/// 게임서버 인증시도.
						MSG_REQ_GAMESRV gamein;
						::memset( &gamein, 0, sizeof(gamein) );

						gamein.Category  = NM_USER;
						gamein.Protocol  = NM_USER_GAMESRV_REQ;
						gamein.LoginIdx  = STAGEMAN->GetLoginIdx( );
						gamein.MemberIdx = STAGEMAN->GetUserIdx( );

						SendNetworkMsg( (char*)&gamein, sizeof(gamein) );

						/// 메세지 갱신
						UIMAN->ChangeTextMsgBox( eMBEVENT_CONNECT_GAMESRV, GAMERESOURCEMAN->GetGameText( 12 ) );
						ReleaseNetworkMsg( perIoContext );
					}
					break;

				case TOS_GAME_2_GAME:
					{
						/// 게임내 채널변경.
						Game2Game*     game2Game = (Game2Game*)packet;
						unsigned long  ipv4       = game2Game->ipv4;
						unsigned short port       = game2Game->port;

						ReleaseNetworkMsg( perIoContext );
						PostConnect( ipv4, port );
					}
					break;
/*
				case TOS_HACKSHIELD:
					{
						MSGROOT* msgRoot = (MSGROOT*)buffer;
						mLastCategory = msgRoot->Category;
						mLastProtocol = msgRoot->Protocol;

						switch( msgRoot->Category )
						{
						case NM_HACKSHIELD:
							{
								switch( msgRoot->Protocol )
								{
								case NM_HACKSHIELD_SYN:
									{
										MSG_SYN_HACKSHIELD* pSyn = (MSG_SYN_HACKSHIELD*)msgRoot;

										AHNHS_TRANS_BUFFER stResponseBuf;

										unsigned long ulRet = _AhnHS_MakeResponse( pSyn->byBuffer, pSyn->nLength, &stResponseBuf );

										if( stResponseBuf.nLength > ANTICPX_TRANS_BUFFER_MAX )
										{
											cString str;
											str.Format("Error ANTICPX_TRANS_BUFFER_MAX %u", ulRet );

											::MessageBoxA(NULL, str.Cstr(), "Error", MB_OK );
											break;
										}

										MSG_REQ_HACKSHIELD reqMsg;
										::memset( &reqMsg, 0, sizeof(reqMsg) );

										reqMsg.Category = NM_HACKSHIELD;
										reqMsg.Protocol = NM_HACKSHIELD_REQ;
										reqMsg.nLength = stResponseBuf.nLength;
										memcpy( reqMsg.byBuffer, stResponseBuf.byBuffer, stResponseBuf.nLength );

										SendHackShieldMsg( (char*)&reqMsg, (unsigned short)reqMsg.GetMsgLength() );

									}
									break;
								default:
									assert(NULL);
								}
							}
							break;							
						default:
							assert(NULL);
						}	

						ReleaseNetworkMsg( perIoContext );
					}
					break;
*/
				default:
					assert( 0 && "UNKNOWN - TYPE OF SERVICE." );
					ReleaseNetworkMsg( perIoContext );
					break;
				}

				(*frontBuffer) = NULL;
				frontBuffer++;
				frontOffset--;
			}
		}
		break;
	}
	return true;
}

/*-- WorkerThread Method
*/
DWORD cIOCPClient::WorkerThread(void)
{
	DWORD             bytesTransferred;
	ULONG_PTR         completionKey;
	OVERLAPPED*       overlapped;
	BOOL              retValue;

	PerIoContext*     perIoContext;

	try {
		while ( true )
		{
			/*-- completion port에 할당된 모든 소켓의 I/O 완료를 기다린다.
			*/
			retValue = GetQueuedCompletionStatus( mIocp, &bytesTransferred, &completionKey, &overlapped, INFINITE );

			/*-- Shutdown
			*/
			if ( overlapped == IOCP_SHUTDOWN )
				return 0;

			perIoContext = (PerIoContext*)overlapped;

			/*-- client connection dropped, continue to service remaining (and possibly new) client connections
			*/
			if ( retValue == FALSE || bytesTransferred == 0 ) 
			{
				perIoContext->offset = max( perIoContext->offset, bytesTransferred );
				Close( completionKey, perIoContext );
				continue;
			}

			switch ( perIoContext->requestType )
			{
			case IOCP_REQUEST_READ:
				if ( RecvComplete( completionKey, perIoContext, bytesTransferred ) == false )
				{
					Close( completionKey, perIoContext );
				}
				break; // Receive 완료 핸들러 함수.

			case IOCP_REQUEST_WRITE:
				if ( SendComplete( completionKey, perIoContext, bytesTransferred ) == false )
				{
					Close( completionKey, perIoContext );
				}
				break; // Send 완료 핸들러 함수.
			}
		}
	} catch ( ... ) {
		ErrorLog( "Throwing 'cIOCPClient::WorkerThread' exception" );
		throw;
	}
	return 0;
}

/*-- ConnectThreadStartingPoint Method ( Connect Thread 시작 포인트 )
*/
DWORD cIOCPClient::ConnectThreadStartingPoint(void* ptr)
{
	cIOCPClient* client = (cIOCPClient*)ptr;
	return client->ConnectThread( );
}

/*-- WorkerThreadStartingPoint Method ( Worker Thread 시작 포인트 )
*/
DWORD cIOCPClient::WorkerThreadStartingPoint(void* ptr)
{
	cIOCPClient* client = (cIOCPClient*)ptr;
	return client->WorkerThread( );
}
