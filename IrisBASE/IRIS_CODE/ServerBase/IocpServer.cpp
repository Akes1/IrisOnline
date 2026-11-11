// Include files
#include "iocpserver.h"

// Local definitions
#pragma warning( disable: 4127 )

#define MAX_BUFFER          0x10000                                    // 64KByte

#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // 핸들를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // "

// Global data

// cIocpServer Constructor
cIocpServer::cIocpServer(void)
{
	// Critical Section를 초기화.
	InitializeCriticalSectionAndSpinCount( &mCs, 2000 );

	mIocp                   = NULL;
	mSocket                 = INVALID_SOCKET;
	mIocpAcceptThread       = NULL;
	mIocpWorkerThreadNumber = 0;
	mIocpBackendThread      = NULL;

	mIoContextPool          = NULL;
	mSocketContextPool      = NULL;

	mIoContextFrontBuffer   = NULL;
	mIoContextBackBuffer    = NULL;

	mRunServer              = false;
	mEndServer              = false;
}

// ~cIocpServer Destructor.
cIocpServer::~cIocpServer(void)
{
	// 종료
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &mCs );
}

// Initialize Method
bool cIocpServer::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, u_long bufferLength)
{
	// CreateIoCompletionPort를 이용하여 completion port 오브젝트를 생성할 때 일반적으로 신경써야 할 파
	// 라메터는 NumberOfConcurrentThreads 이다. 처음 3개의 파라메터는 값을 설정하지 않는다.
	//
	// NumberOfConcurrentThreads는 현재 completion port에 대하여 몇개의 스레드를 실행할 것인가를 결정한
	// 다. 가장 이상적인 경우는 스레드의 context switching을 방지하기 위하여 서비스에 대하여 하나의 프로
	// 세서당 하나의 스레드만 동작하도록 하는 것이다. NumberOfConcurrentThreads의 값을 0으로 설정하면 시
	// 스템은 프로세서에 적합한 스레드 개수를 할당하여 사용한다.
	// (하나의 CPU에 대하여 IOCP 하나당 하나의 스레드를 생성)
	//
	mIocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( mIocp == NULL )
		return false;

	// CreateIoCompletionPort 오브젝트를 생성한 뒤에 작업스레드를 생성하고 소켓핸들을 지정할 수 있는데,
	// 이때 생성하는 작업스레드는, 스레드가 블록될 가능성이 있다면, NumberOfConcurrentThreads 의 갯수보
	// 다 많이 생성하는 것이 좋다. NumberOfConcurrentThreads만큼 스레드가 항상 실행되게 할 수 있기 때문
	// 에, 블록된 스레드가 있다면 다른 스레드가 대신 동작할 것이기 때문이다.
	//
	SYSTEM_INFO si;
	DWORD       threadId;

	// 시스템에 몇 개의 프로세서가 있는지 확인한다.
	GetSystemInfo( &si );

	mIocpWorkerThreadNumber = min( si.dwNumberOfProcessors * numWorkerThreads, IOCP_MAX_WORKER_THREAD );
	for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
	{
		// Worker Thread생성.
		mIocpWorkerThread[i] = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
		if ( mIocpWorkerThread[i] == NULL )
			return false;
	}

	// Socket Context Pool를 생성한다.
	mSocketContextPool = new cSocketContextPool( &mCs );
	if ( mSocketContextPool == NULL )
		return false;

	// I/O Context Pool를 생성한다.
	// Recv/Send를 위해 클라이언트 * 2로 계산한다.
	//
	mIoContextPool = new cIoContextPool( bufferLength );
	if ( mIoContextPool == NULL )
		return false;

	// I/O Context Buffer를 생성한다.
	mIoContextFrontBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mIoContextBackBuffer  = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	if ( !mIoContextFrontBuffer && !mIoContextBackBuffer )
		return false;

	// 서버 소켓을 생성한다.
	//
	PHOSTENT    phe;
	int         zero;
	LINGER      linger;

	// listen할 서버 소켓을 생성한다.
	mSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( mSocket == INVALID_SOCKET )
		return false;

	ZeroMemory( (void*)&mAddr, sizeof(mAddr) );
	mAddr.sin_family = AF_INET;
	mAddr.sin_port   = htons( port );
	mAddr.sin_addr.s_addr = strlen( ipAddr ) ? inet_addr( ipAddr ) : INADDR_NONE;

	if ( mAddr.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		// 서버의 호스트 이름이 도트 형식(IPv4)이 아니므로, 문자열로 재 시도.
		//
		if ( (phe = gethostbyname( "" )) != NULL )
			CopyMemory( &mAddr.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}

	mPort = port;

	if ( bind( mSocket, (LPSOCKADDR)&mAddr, sizeof(mAddr) ) == SOCKET_ERROR )
		return false;

	if ( listen( mSocket, SOMAXCONN ) == SOCKET_ERROR )
		return false;

	// The purpose of this algorithm is to reduce the number of very small segments sent, especially on
	// high-delay (remote) links.
	//
	// Windows Sockets applications can disable the Nagle algorithm for their connections by setting the
	// TCP_NODELAY socket option. However, this practice should be used wisely and only when needed because
	// this practice increases network traffic. Usually, applications that need to disable the Nagle algorithm
	// are applications that need to send small packets and require that the peer application receive them
	// immediately. Some network applications may not perform well if their design does not take into account
	// the effects of transmitting large numbers of small packets and the Nagle algorithm.
	//
	// Nagel 알고리즘의 목적은 특히 고지연(원격) 링크에서 크기가 작은 세그먼트가 전송되는 횟수를 줄이는
	// 것이다.
	//
	// Windows 소켓 응용 프로그램에서는 자신의 연결에 대해 Nagle 알고리즘을 사용할 수 없도록 TCP_NODELAY
	// 소켓 옵션을 설정할 수 있습니다. 그러나 이러한 작업은 네트워크 이용을 늘리기 때문에 반드시 필요한
	// 경우가 아니면 가급적 사용하지 않아야 합니다. 다수의 소형 패킷 전송으로 인해 발생하는 영향과 Nagle
	// 알고리즘을 고려하지 않은 상태로 설계된 네트워크 응용 프로그램은 제대로 수행되지 않을 수 있습니다.
	// Nagle 알고리즘은 성능상의 이유로 인해 TCP 연결을 루프백하는 데는 적용되지 않습니다. Windows 2000
	// Netbt는 직접 호스트된 리디렉터/서버 연결뿐만 아니라 TCP를 통한 NetBIOS 연결에 대해서도 네이글링을
	// 사용할 수 없게 만듭니다. 따라서 다수의 소형 파일 조작 명령을 발행하는 응용 프로그램의 경우 성능이
	// 향상될 수 있습니다. 그러한 응용 프로그램으로는 파일 잠금/잠금 해제 기능을 자주 사용하는 응용 프로
	// 그램을 들 수 있습니다. 
	//
	 int nodelay = 1;
	 if ( setsockopt( mSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay) ) == SOCKET_ERROR )
		return false;

	// Disable receive buffering on the socket. Setting SO_RCVBUF to 0 causes winsock to stop bufferring
	// receive and perform receives directly from our buffers, thereby reducing CPU usage.
	//
	// SO_RCVBUF를 0으로하여 수신버퍼를 사용하지 않는다. 수신버퍼가 0이 아니면 불필요하게 이 수신버퍼를
	// 거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
	// 수신버퍼 공간이 없으면 overlapped I/O를 호출했을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
	// 다.
	//
	zero = 0;
	if ( setsockopt( mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero)) == SOCKET_ERROR )
		return false;

	// Disable send buffering on the socket. Setting SO_SNDBUF to 0 causes winsock to stop bufferring
	// sends and perform sends directly from our buffers, thereby reducing CPU usage.
	//
	// SO_SNDBUF를 0으로하여 송신버퍼를 사용하지 않는다. 송신버퍼가 0이 아니면 불필요하게 이 송신버퍼를
	// 거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
	// 송신버퍼 공간이 없으면 overlapped I/O를 호출햇을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
	// 다.
	//
	zero = 0;
	if ( setsockopt( mSocket, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero) ) == SOCKET_ERROR )
		return false;

	// closesocket 함수가 호출된 이후에 보내진 데이터에 대한 처리를 제어하기 위한 옵션이다.
	// linger::u_short l_onoff  : option on/off
	// linger::u_short l_linger : linger time
	//
	// SO_LINGER 옵션의 타임아웃을 0으로 설정하면 (linger 구조체의 l_onoff 필드를 0이 아닌값, l_linger
	// 를 0으로 설정), 아직 전송되지 않은 데이터가 있더라도 closesocket 함수 호출은 블록되지 않고 즉시
	// 끝난다. 이렇게 호출된 소켓의 가성연결 (virtual circuit)은 즉시 단절(reset)되고 전송되지 않은 데
	// 이터는 손실된다. 데이터 수신하는 측은 수신 함수가 WSAECONNRESET로 실패한다.
	//
	linger.l_onoff  = 1;
	linger.l_linger = 0;
	if ( setsockopt( mSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger) ) == SOCKET_ERROR )
		return false;

	mEndServer = false;
	mRunServer = true;

	// Accept Thread를 생성한다.
	mIocpAcceptThread = CreateThread( NULL, 0, AcceptThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( mIocpAcceptThread == NULL )
		return false;

	// Backend Thread를 생성한다.
	mIocpBackendThread = CreateThread( NULL, 0, BackendThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( mIocpBackendThread == NULL )
		return false;

	return true;
}

// Shutdown Method
void cIocpServer::Shutdown(DWORD maxWait)
{
	// 안전하게 종료한다.
	mEndServer = true;
	mRunServer = false;

	// Backend Thread가 종료될때까지 대기
	if ( mIocpBackendThread != NULL )
	{
		WaitForSingleObjectEx( mIocpBackendThread, maxWait, TRUE );
		CloseHandle( mIocpBackendThread );
		mIocpBackendThread = NULL;
	}

	// 소켓을 종료한다.
	SOCKET sockTemp = mSocket;

	mSocket = INVALID_SOCKET;

	if ( sockTemp != INVALID_SOCKET )
	{
		closesocket( sockTemp );
	}

	// Accept Thread가 종료될때까지 대기
	if ( mIocpAcceptThread != NULL )
	{
		WaitForSingleObjectEx( mIocpAcceptThread, maxWait, TRUE );
		CloseHandle( mIocpAcceptThread );
		mIocpAcceptThread = NULL;
	}

	// Cause worker threads to exit
	if ( mIocp != NULL )
	{
		for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
		{
			PostQueuedCompletionStatus( mIocp, 0, 0, IOCP_SHUTDOWN );
		}
	}

	// Make sure worker threads exits.
	for ( int i = 0; i < mIocpWorkerThreadNumber; i++ )
	{
		if ( WaitForSingleObject( mIocpWorkerThread[i], 60000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( mIocpWorkerThread[i], &exitCode);
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( mIocpWorkerThread[i], 0 );
			}
		}
		CloseHandle( mIocpWorkerThread[i] );
		mIocpWorkerThread[i] = NULL;
	}

	// Overlapped I/O Model Pool및 Socket Context Pool를 해제한다.
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
	SafeDelete( mIoContextPool     );
	SafeDelete( mSocketContextPool );

	// completion port를 해제한다.
	SafeCloseHandle( mIocp );
}

// GetPoolUsage Method
void cIocpServer::GetIoPoolUsage(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize)
{
	if ( mIoContextPool != NULL )
	{
		mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	}
}

// GetPoolUsage Method
void cIocpServer::GetSocketPoolUsage(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize)
{
	if ( mSocketContextPool != NULL )
	{
		mSocketContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	}
}

// QueueRequest Method
BOOL cIocpServer::QueueRequest(ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred)
{
	return PostQueuedCompletionStatus( mIocp, bytesTransferred, completionKey, overlapped );
}

// AcceptComplete Method
bool cIocpServer::AcceptComplete(PerSocketContext* /*perSocketContext*/)
{
	return true;
}

// AcceptThread Method
DWORD cIocpServer::AcceptThread( )
{
	while ( true )
	{
		PerSocketContext* perSocketContext = NULL;
		PerIoContext*     perIoContext     = NULL;
		SOCKET            socket;
		SOCKADDR_IN       addr;
		int               addrlen;

		// 연결을 수락하고 completion port를 지정한다.
		addrlen = sizeof( addr );
		socket  = WSAAccept( mSocket, (sockaddr*)&addr, &addrlen, NULL, 0 );
		if ( socket == SOCKET_ERROR ) 
			break;

		// 서버종료, Thread를 나간다.
		if ( mEndServer == true )
			break;

		// 하나의 연결별로 TCP keepalive를 활성화시키고 keepalive 간격을 설정할 때 사용한다.
		// keepaliveinterval: 응답을 받을 때까지 keepalive 패킷을 재전송할 간격이다.
		// keepalivetime    : 연결이 유효한지 확인하기 위하여 keepalive 패킷을 전송할 간격이다.
		//
		// Header           : Declared in Mstcpip.h.
		//
		// tcp_keepalive keepAlive;
		// DWORD         bytesReturned;
		// 
		// keepAlive.onoff             = 1;
		// keepAlive.keepalivetime     = 3000;
		// keepAlive.keepaliveinterval = 1000;
		// 
		// if ( WSAIoctl( socket, SIO_KEEPALIVE_VALS, &keepAlive, sizeof(keepAlive), 0, 0, &bytesReturned, NULL, NULL ) == SOCKET_ERROR )
		// {
		// 	closesocket( socket );
		// 	continue;
		// }

		// cSocketContextPool class에서 PerSocketContext를 가져온다.
		perSocketContext = mSocketContextPool->GetPerSocketContext( socket, addr, MAX_TTL );
		if ( perSocketContext == NULL )
			break;

		// Accept에 의하여 리턴된 소켓 핸들을 completion port에 할당한다.
		if ( CreateIoCompletionPort( (HANDLE)socket, mIocp, (ULONG_PTR)perSocketContext, 0 ) == NULL )
			break;

		// Accept 완료 핸들러 함수.
		if ( AcceptComplete( perSocketContext ) == false )
		{
			// SocketContext 회수.
			mSocketContextPool->ReleasePerSocketContext( perSocketContext );
			continue;
		}

		// 수신을 위해 I/O Context를 준비한다.
		perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_READ );
		if ( RecvPost( perIoContext ) == false )
		{
			Close( perSocketContext );
			continue;
		}
	}
	return 0;
}

// SendExec Method
bool cIocpServer::SendExec(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  sendBytes = 0;
	DWORD  flags     = 0;
	int    retcode;

	wsaBuf.len = perIoContext->offset;
	wsaBuf.buf = perIoContext->buffer;

	retcode = WSASend( perIoContext->socket,
					   &wsaBuf,
					   1,
					   &sendBytes,
					   flags,
					   &(perIoContext->wsaOverlapped),
					   NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		mIoContextPool->ReleaseIoContext( perIoContext );
		return false;
	}
	return true;
}

// SendPost Method
bool cIocpServer::SendPost(PerIoContext* perIoContext)
{
	cCSLock        lock( &mCs );
	PerIoContext** buffer = mIoContextBackBuffer->buffer;
	long&          offset = mIoContextBackBuffer->offset;

	if ( offset < MAX_IO_CONTEXT_BUFFER_LEN )
	{
		if ( perIoContext->offset < mIoContextPool->GetBufferLength( ) )
		{
			buffer[ offset ] = perIoContext;
			offset++;
			return true;
		}
		else
			throw "Error cIocpServer::SendPost:Exceed the length of the buffer used.";
	}
	else
	{
		mIoContextPool->ReleaseIoContext( perIoContext );
	}

	return false;
}

// RecvPost Method
bool cIocpServer::RecvPost(PerIoContext* perIoContext)
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
		mIoContextPool->ReleaseIoContext( perIoContext );
		return false;
	}
	return true;
}

// Close Method - CompletionKey와 Overlapped를 비교후 CompletionKey를 삭제한다.
void cIocpServer::Close(PerSocketContext* perSocketContext, PerIoContext* perIoContext)
{
	cCSLock lock( &mCs );

	// 사용이 완료된 Socket Context를 회수.
	if ( perSocketContext->socket == perIoContext->socket )
	{
		// 송/수신 비활성화.
		if ( !perSocketContext->status.connectionDead )
		{
			shutdown( perSocketContext->socket, SD_RECEIVE );
			perSocketContext->status.connectionDead = 1;
		}
	}

	// 사용이 완료된 I/O Context를 회수.
	mIoContextPool->ReleaseIoContext( perIoContext );
}

void cIocpServer::Close(PerSocketContext* perSocketContext)
{
	cCSLock lock( &mCs );

	// 송/수신 비활성화.
	if ( !perSocketContext->status.connectionDead )
	{
		shutdown( perSocketContext->socket, SD_RECEIVE );
		perSocketContext->status.connectionDead = 1;
	}
}

// SendComplete Method
bool cIocpServer::SendComplete(PerSocketContext* /*perSocketContext*/,
							   PerIoContext*     perIoContext,
							   DWORD             /*bytesTransferred*/)
{
	// 사용이 완료된 I/O Context는 회수한다.
	mIoContextPool->ReleaseIoContext( perIoContext );
	return true;
}

// RecvComplete Method (Default Echo Server)
bool cIocpServer::RecvComplete(PerSocketContext* perSocketContext,
							   PerIoContext*     perIoContext,
							   DWORD             bytesTransferred)
{
	// 수신된 데이터를 Echo로 돌려 보낸다.
	perIoContext->offset      = bytesTransferred;
	perIoContext->requestType = IOCP_REQUEST_WRITE;

	if ( SendPost( perIoContext ) == false )
	{
		Close( perSocketContext );
		return false;
	}

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_READ );
	if ( RecvPost( perIoContext ) == false )
	{
		Close( perSocketContext );
		return false;
	}
	return true;
}

// CallbackComplete Method
bool cIocpServer::CallbackComplete(PerSocketContext* /*perSocketContext*/,
								   PerIoContext*     perIoContext,
								   DWORD             /*bytesTransferred*/)
{
	// 사용이 완료된 I/O Context는 회수한다.
	mIoContextPool->ReleaseIoContext( perIoContext );
	return true;
}

/*-- IoContextPresent Method
*/
void cIocpServer::IoContextPresent( )
{
	IoContextBuffer* temp;

	// Double Buffering 처리.
	CSBlock( &mCs )
	{
		temp                  = mIoContextBackBuffer;
		mIoContextBackBuffer  = mIoContextFrontBuffer;
		mIoContextFrontBuffer = temp;
	}

	// 전송시작.
	PerIoContext** buffer = mIoContextFrontBuffer->buffer;
	long&          offset = mIoContextFrontBuffer->offset;

	while ( offset > 0 )
	{
		SendExec( (*buffer) );
		(*buffer) = NULL;

		buffer++;
		offset--;
	}
}

// WorkerThread Method
DWORD cIocpServer::WorkerThread( )
{
	DWORD             bytesTransferred;
	ULONG_PTR         completionKey;
	OVERLAPPED*       overlapped;
	BOOL              retValue;

	PerSocketContext* perSocketContext;
	PerIoContext*     perIoContext;

	while ( true )
	{
		// completion port에 할당된 모든 소켓의 I/O 완료를 기다린다.
		retValue = GetQueuedCompletionStatus( mIocp, &bytesTransferred, &completionKey, &overlapped, INFINITE );

		// Shutdown
		if ( overlapped == IOCP_SHUTDOWN )
			return 0;

		perSocketContext = (PerSocketContext*)completionKey;
		perIoContext     = (PerIoContext*)overlapped;

		// client connection dropped, continue to service remaining (and possibly new) client connections
		if ( retValue == FALSE || bytesTransferred == 0 ) 
		{
			// 사용이 완료된 I/O Context는 회수한다.
			perIoContext->offset = max( perIoContext->offset, bytesTransferred );
			Close( perSocketContext, perIoContext );
			continue;
		}

		switch ( perIoContext->requestType )
		{
		case IOCP_REQUEST_READ:
			RecvComplete( perSocketContext, perIoContext, bytesTransferred );
			break; // Receive 완료 핸들러 함수.

		case IOCP_REQUEST_WRITE:
			SendComplete( perSocketContext, perIoContext, bytesTransferred );
			break; // Send 완료 핸들러 함수.

		case IOCP_REQUEST_CALLBACK:
			CallbackComplete( perSocketContext, perIoContext, bytesTransferred );
			break; // Callback 완료 핸들러 함수.
		}
	}
	return 0;
}

// BackendThread Method
DWORD cIocpServer::BackendThread( )
{
	DWORD beginTick;
	DWORD endTick;
	DWORD tickDiff;

	while ( true )
	{
		beginTick = GetTickCount( );

		// 서버종료, Thread를 나간다.
		if ( mEndServer == true )
			break;

		// 사용자의 TTL(Time To Live)를 검사.
		if ( mSocketContextPool != NULL )
		{
			cCSLock           lock( &mCs );
			PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
			PerSocketContext* next          = NULL;

			while ( socketContext != NULL )
			{
				next = socketContext->next;

				if ( socketContext->status.connectionDead )
				{
					// Close Socket On.
					socketContext->status.closeSocket = 1;
				}
				else
				{
					// TTL값 확인
					if ( beginTick > socketContext->timeToLive )
					{
						PerIoContext* cbIoContext = mIoContextPool->GetIoContext( socketContext->socket, IOCP_REQUEST_CALLBACK );
						PostQueuedCompletionStatus( mIocp, 0, (ULONG_PTR)socketContext, (LPOVERLAPPED)cbIoContext );
					}
				}

				if ( socketContext->status.closeSocket )
				{
					// SocketContext 회수.
					mSocketContextPool->ReleasePerSocketContext( socketContext );
				}

				socketContext = next;
			} // while ( socketContext != NULL )
		}

		// I/O Context 처리.
		IoContextPresent( );

		endTick  = GetTickCount( );
		tickDiff = endTick - beginTick;
		Sleep( 10 );
	}
	return 0;
}

// AcceptThreadStartingPoint Method ( Accept Thread 시작 포인트 )
DWORD cIocpServer::AcceptThreadStartingPoint(void* ptr)
{
	cIocpServer* iocpServer = (cIocpServer*)ptr;
	return iocpServer->AcceptThread( );
}

// WorkerThreadStartingPoint Method ( Worker Thread 시작 포인트 )
DWORD cIocpServer::WorkerThreadStartingPoint(void* ptr)
{
	cIocpServer* iocpServer = (cIocpServer*)ptr;
	return iocpServer->WorkerThread( );
}

// BackendThreadStartingPoint Method ( Backend Thread 시작 포인트 )
DWORD cIocpServer::BackendThreadStartingPoint(void* ptr)
{
	cIocpServer* iocpServer = (cIocpServer*)ptr;
	return iocpServer->BackendThread( );
}
