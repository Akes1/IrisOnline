// Include files
#include "iocpudprecv.h"

// Local definitions
#pragma warning( disable: 4127 )

#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // 핸들를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // "

// Global data

// IocpUdpRecv Constructor
IocpUdpRecv::IocpUdpRecv(void)
{
	// Critical Section를 초기화.
	InitializeCriticalSectionAndSpinCount( &m_cs, 2000 );

	m_iocp                   = NULL;
	m_socket                 = INVALID_SOCKET;
	m_iocpWorkerThreadNumber = 0;
	m_ioContextPool          = NULL;
	m_iocpBackendThread      = NULL;

	m_ioContextFrontBuffer   = NULL;
	m_ioContextBackBuffer    = NULL;

	m_runServer              = false;
	m_endServer              = false;
}

// ~IocpUdpRecv Destructor.
IocpUdpRecv::~IocpUdpRecv(void)
{
	// 종료
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &m_cs );
}

// Initialize Method
bool IocpUdpRecv::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, unsigned int bufferLength)
{
	// CreateIoCompletionPort를 이용하여 completion port 오브젝트를 생성할 때 일반적으로 신경써야 할 파
	// 라메터는 NumberOfConcurrentThreads 이다. 처음 3개의 파라메터는 값을 설정하지 않는다.

	// NumberOfConcurrentThreads는 현재 completion port에 대하여 몇개의 스레드를 실행할 것인가를 결정한
	// 다. 가장 이상적인 경우는 스레드의 context switching을 방지하기 위하여 서비스에 대하여 하나의 프로
	// 세서당 하나의 스레드만 동작하도록 하는 것이다. NumberOfConcurrentThreads의 값을 0으로 설정하면 시
	// 스템은 프로세서에 적합한 스레드 개수를 할당하여 사용한다.
	// (하나의 CPU에 대하여 IOCP 하나당 하나의 스레드를 생성)
	m_iocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( m_iocp == NULL )
		return false;

	// CreateIoCompletionPort 오브젝트를 생성한 뒤에 작업스레드를 생성하고 소켓핸들을 지정할 수 있는데,
	// 이때 생성하는 작업스레드는, 스레드가 블록될 가능성이 있다면, NumberOfConcurrentThreads 의 갯수보
	// 다 많이 생성하는 것이 좋다. NumberOfConcurrentThreads만큼 스레드가 항상 실행되게 할 수 있기 때문
	// 에, 블록된 스레드가 있다면 다른 스레드가 대신 동작할 것이기 때문이다.
	SYSTEM_INFO si;
	DWORD       threadId;

	// 시스템에 몇 개의 프로세서가 있는지 확인한다.
	GetSystemInfo( &si );

	m_iocpWorkerThreadNumber = min( si.dwNumberOfProcessors * numWorkerThreads, IOCP_MAX_WORKER_THREAD );
	for ( int i = 0; i < m_iocpWorkerThreadNumber; i++ )
	{
		m_iocpWorkerThread[i] = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
		if ( m_iocpWorkerThread[i] == NULL )
			return false;
	}

	// I/O Context Pool를 생성한다.
	// 0xffff: UDP(65,535) 최대 패킷 사이즈
	// 0x0044: IPv4(60) + UDP(8) 의 해더 사이즈
	m_ioContextPool = new cIoContextPool( bufferLength );
	if ( m_ioContextPool == NULL )
		return false;

	// I/O Context Buffer를 생성한다.
	m_ioContextFrontBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	m_ioContextBackBuffer  = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	if ( !m_ioContextFrontBuffer && !m_ioContextBackBuffer )
		return false;

	// IPv4 주소를 만든다.
	ZeroMemory( (void*)&m_addr, sizeof(SOCKADDR_IN) );
	m_addr.sin_family = AF_INET;
	m_addr.sin_port   = htons( port );
	m_addr.sin_addr.s_addr = inet_addr( ipAddr );

	if ( m_addr.sin_addr.s_addr == INADDR_NONE )
	{
		PHOSTENT phe;

		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( ipAddr )) != NULL )
			CopyMemory( &m_addr.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}

	// connect할 클라이언트 소켓을 생성한다.
	m_socket = WSASocket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( m_socket == INVALID_SOCKET )
		return false;

	if ( bind( m_socket, (LPSOCKADDR)&m_addr, sizeof(SOCKADDR_IN) ) == SOCKET_ERROR )
		return false;

	// Disable receive buffering on the socket. Setting SO_RCVBUF to 0 causes winsock to stop bufferring
	// receive and perform receives directly from our buffers, thereby reducing CPU usage.

	// SO_RCVBUF를 0으로하여 수신버퍼를 사용하지 않는다. 수신버퍼가 0이 아니면 불필요하게 이 수신버퍼를
	// 거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
	// 수신버퍼 공간이 없으면 overlapped I/O를 호출했을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
	// 다.
	int zero = DEF_UDP_PACKET_SIZE;
	if ( setsockopt( m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero)) == SOCKET_ERROR )
	{
		closesocket( m_socket );
		return false;
	}

	// 생성된 소켓 핸들을 completion port에 할당한다.
	if ( CreateIoCompletionPort( (HANDLE)m_socket, m_iocp, (ULONG_PTR)NULL, 0 ) == NULL )
	{
		closesocket( m_socket );
		return false;
	}

	// 수신을 위해 I/O Context를 준비한다.
	PerIoContext* perIoContext = m_ioContextPool->GetIoContext( m_socket, IOCP_REQUEST_READ );
	RecvPost( NULL, perIoContext );

	m_endServer = false;
	m_runServer = true;

	// Backend Thread를 생성한다.
	m_iocpBackendThread = CreateThread( NULL, 0, BackendThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( m_iocpBackendThread == NULL )
		return false;
	return true;
}

// Shutdown Method
void IocpUdpRecv::Shutdown(DWORD maxWait)
{
	// 안전하게 종료한다.
	m_endServer = true;
	m_runServer = false;

	// Backend Thread가 종료될때까지 대기
	if ( m_iocpBackendThread != NULL )
	{
		WaitForSingleObjectEx( m_iocpBackendThread, maxWait, TRUE );
		CloseHandle( m_iocpBackendThread );
		m_iocpBackendThread = NULL;
	}

	// 안전하게 종료한다.
	SOCKET sockTemp = m_socket;

	if ( sockTemp != INVALID_SOCKET )
	{
		closesocket( sockTemp );
	}

	// Cause worker threads to exit
	if ( m_iocp != NULL )
	{
		for ( int i = 0; i < m_iocpWorkerThreadNumber; i++ )
		{
			PostQueuedCompletionStatus( m_iocp, 0, 0, IOCP_SHUTDOWN );
		}
	}

	// Make sure worker threads exits.
	for ( int i = 0; i < m_iocpWorkerThreadNumber; i++ )
	{
		if ( WaitForSingleObject( m_iocpWorkerThread[ i ], 60000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( m_iocpWorkerThread[ i ], &exitCode);
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( m_iocpWorkerThread[ i ], 0 );
			}
		}
		CloseHandle( m_iocpWorkerThread[ i ] );
		m_iocpWorkerThread[i] = NULL;
	}

	// 소켓 초기화.
	m_socket = INVALID_SOCKET;

	// Overlapped I/O Model Pool및 Socket Context Pool를 해제한다.
	if ( m_ioContextFrontBuffer )
	{
		GlobalFree( m_ioContextFrontBuffer );
		m_ioContextFrontBuffer = NULL;
	}
	if ( m_ioContextBackBuffer )
	{
		GlobalFree( m_ioContextBackBuffer );
		m_ioContextBackBuffer = NULL;
	}
	// Overlapped I/O Model Pool를 해제한다.
	SafeDelete( m_ioContextPool );

	// completion port를 해제한다.
	SafeCloseHandle( m_iocp );
}

// GetIoPoolUsage Method
void IocpUdpRecv::GetIoPoolUsage(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize)
{
	if ( m_ioContextPool != NULL )
	{
		m_ioContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	}
}

// QueueRequest Method
BOOL IocpUdpRecv::QueueRequest(ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred)
{
	return PostQueuedCompletionStatus( m_iocp, bytesTransferred, completionKey, overlapped );
}

// RecvPost Method
bool IocpUdpRecv::RecvPost(ULONG_PTR completionKey, PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  recvBytes = 0;
	DWORD  flags     = 0;
	int    addrlen   = sizeof(SOCKADDR_IN);
	int    retcode;

	wsaBuf.len = (perIoContext->length - perIoContext->offset);
	wsaBuf.buf = (perIoContext->buffer + perIoContext->offset);

	retcode = WSARecvFrom( perIoContext->socket,
						   &wsaBuf,
						   1,
						   &recvBytes,
						   &flags,
						   (SOCKADDR*)&perIoContext->addr,
						   &addrlen,
						   &(perIoContext->wsaOverlapped),
						   NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		PostQueuedCompletionStatus( m_iocp, 0, (ULONG_PTR)completionKey, (LPOVERLAPPED)perIoContext );
		return false;
	}
	return true;
}

// CallbackPost Method
bool IocpUdpRecv::CallbackPost(ULONG_PTR completionKey, PerIoContext* perIoContext)
{
	int retcode;

	retcode = PostQueuedCompletionStatus( m_iocp,
										  perIoContext->offset,
										  completionKey,
										  (LPOVERLAPPED)perIoContext );
	if ( retcode == 0 )
	{
		return false;
	}
	return true;;
}

// RecvComplete Method (Default Echo Server)
bool IocpUdpRecv::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &m_cs );

	// 사용이 완료된 I/O Context는 회수한다.
	perIoContext->offset = bytesTransferred;
	m_ioContextPool->ReleaseIoContext( perIoContext );

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = m_ioContextPool->GetIoContext( m_socket, IOCP_REQUEST_READ );
	return RecvPost( completionKey, perIoContext );
}

// CallbackComplete Method
bool IocpUdpRecv::CallbackComplete(ULONG_PTR /*completionKey*/, PerIoContext* perIoContext, DWORD /*bytesTransferred*/)
{
	// 사용이 완료된 I/O Context는 회수한다.
	m_ioContextPool->ReleaseIoContext( perIoContext );
	return true;
}

// WorkerThread Method
DWORD IocpUdpRecv::WorkerThread( )
{
	DWORD             bytesTransferred;
	ULONG_PTR         completionKey;
	OVERLAPPED*       overlapped;
	BOOL              retValue;

	PerIoContext*     perIoContext;

	while ( true )
	{
		// completion port에 할당된 모든 소켓의 I/O 완료를 기다린다.
		retValue = GetQueuedCompletionStatus( m_iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE );

		// Shutdown
		if ( overlapped == IOCP_SHUTDOWN )
			return 0;

		perIoContext = (PerIoContext*)overlapped;

		if ( retValue == FALSE || bytesTransferred == 0 )
		{
			// 사용이 완료된 I/O Context는 회수한다.
			perIoContext->offset = max( perIoContext->offset, bytesTransferred );
			m_ioContextPool->ReleaseIoContext( perIoContext );
			continue;
		}

		switch ( perIoContext->requestType )
		{
		case IOCP_REQUEST_READ:
			RecvComplete( completionKey, perIoContext, bytesTransferred );
			break; // Receive 완료 핸들러 함수.

		case IOCP_REQUEST_CALLBACK:
			CallbackComplete( completionKey, perIoContext, bytesTransferred );
			break; // Callback 완료 핸들러 함수.
		}
	}
	return 0;
}

// BackendThread Method
DWORD IocpUdpRecv::BackendThread( )
{
	while ( true )
	{
		// 서버종료, Thread를 나간다.
		if ( m_endServer == true )
			break;

		Sleep( 50 );
	}
	return 0;
}

// WorkerThreadStartingPoint Method ( Worker Thread 시작 포인트 )
DWORD IocpUdpRecv::WorkerThreadStartingPoint(void* ptr)
{
	IocpUdpRecv* iocpUdpRecv = (IocpUdpRecv*)ptr;
	return iocpUdpRecv->WorkerThread( );
}

// BackendThreadStartingPoint Method ( Backend Thread 시작 포인트 )
DWORD IocpUdpRecv::BackendThreadStartingPoint(void* ptr)
{
	IocpUdpRecv* iocpUdpRecv = (IocpUdpRecv*)ptr;
	return iocpUdpRecv->BackendThread( );
}
