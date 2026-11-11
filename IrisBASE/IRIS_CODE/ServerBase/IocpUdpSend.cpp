// Include files
#include "iocpudpsend.h"

// Local definitions
#pragma warning( disable: 4127 )

#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // 핸들를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // "

// Global data

// cIocpUdpSend Constructor
cIocpUdpSend::cIocpUdpSend(void)
{
	// Critical Section를 초기화.
	InitializeCriticalSectionAndSpinCount( &mCs, 2000 );

	mIocp                   = NULL;
	mSocket                 = INVALID_SOCKET;
	mIocpWorkerThreadNumber = 0;
	mIoContextPool          = NULL;
	mIocpBackendThread      = NULL;

	mIoContextFrontBuffer   = NULL;
	mIoContextBackBuffer    = NULL;

	mRunServer              = false;
	mEndServer              = false;
}

// ~cIocpUdpSend Destructor.
cIocpUdpSend::~cIocpUdpSend(void)
{
	// 종료
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &mCs );
}

// Initialize Method
bool cIocpUdpSend::Initialize(unsigned short numWorkerThreads, unsigned int bufferLength)
{
	// CreateIoCompletionPort를 이용하여 completion port 오브젝트를 생성할 때 일반적으로 신경써야 할 파
	// 라메터는 NumberOfConcurrentThreads 이다. 처음 3개의 파라메터는 값을 설정하지 않는다.
	//
	// NumberOfConcurrentThreads는 현재 completion port에 대하여 몇개의 스레드를 실행할 것인가를 결정한
	// 다. 가장 이상적인 경우는 스레드의 context switching을 방지하기 위하여 서비스에 대하여 하나의 프로
	// 세서당 하나의 스레드만 동작하도록 하는 것이다. NumberOfConcurrentThreads의 값을 0으로 설정하면 시
	// 스템은 프로세서에 적합한 스레드 개수를 할당하여 사용한다.
	// (하나의 CPU에 대하여 IOCP 하나당 하나의 스레드를 생성)
	
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
		mIocpWorkerThread[i] = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
		if ( mIocpWorkerThread[i] == NULL )
			return false;
	}

	// I/O Context Pool를 생성한다.
	// 0xffff: UDP(65,535) 최대 패킷 사이즈
	// 0x0044: IPv4(60) + UDP(8) 의 해더 사이즈
	mIoContextPool = new cIoContextPool( bufferLength );
	if ( mIoContextPool == NULL )
		return false;

	// I/O Context Buffer를 생성한다.
	mIoContextFrontBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mIoContextBackBuffer  = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	if ( !mIoContextFrontBuffer && !mIoContextBackBuffer )
		return false;

	// connect할 클라이언트 소켓을 생성한다.
	mSocket = WSASocket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( mSocket == INVALID_SOCKET )
		return false;

	// Disable send buffering on the socket. Setting SO_SNDBUF to 0 causes winsock to stop bufferring
	// sends and perform sends directly from our buffers, thereby reducing CPU usage.
	//
	// SO_SNDBUF를 0으로하여 송신버퍼를 사용하지 않는다. 송신버퍼가 0이 아니면 불필요하게 이 송신버퍼를
	// 거쳐서 overlapped I/O를 호출했을 때 제공한 버퍼로 복사가 이루어진다.
	// 송신버퍼 공간이 없으면 overlapped I/O를 호출햇을 때 제공한 버퍼로 시스템에서 직접 데이터가 복사된
	// 다.
	//
	int zero = DEF_UDP_PACKET_SIZE;
	if ( setsockopt( mSocket, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero) ) == SOCKET_ERROR )
	{
		closesocket( mSocket );
		return false;
	}

	// 브로드캐스트 데이터를 송신하기 위해서는 SO_BROADCAST를 설정해야 한다.
	int broadcast = TRUE;
	if ( setsockopt( mSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast) ) == SOCKET_ERROR )
	{
		closesocket( mSocket );
		return false;
	}

	// 생성된 소켓 핸들을 completion port에 할당한다.
	if ( CreateIoCompletionPort( (HANDLE)mSocket, mIocp, (ULONG_PTR)NULL, 0 ) == NULL )
	{
		closesocket( mSocket );
		return false;
	}

	mEndServer = false;
	mRunServer = true;

	// Backend Thread를 생성한다.
	mIocpBackendThread = CreateThread( NULL, 0, BackendThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( mIocpBackendThread == NULL )
		return false;

	return true;
}

// Shutdown Method
void cIocpUdpSend::Shutdown(DWORD maxWait)
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

	// 안전하게 종료한다.
	SOCKET sockTemp = mSocket;;

	mSocket = INVALID_SOCKET;

	if ( sockTemp != INVALID_SOCKET )
	{
		closesocket( sockTemp );
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
	// Overlapped I/O Model Pool를 해제한다.
	SafeDelete( mIoContextPool );

	// completion port를 해제한다.
	SafeCloseHandle( mIocp );
}

// GetIoPoolUsage Method
void cIocpUdpSend::GetIoPoolUsage(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize)
{
	if ( mIoContextPool != NULL )
	{
		mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	}
}

// QueueRequest Method
BOOL cIocpUdpSend::QueueRequest(ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred)
{
	return PostQueuedCompletionStatus( mIocp, bytesTransferred, completionKey, overlapped );
}

// SendExec Method
bool cIocpUdpSend::SendExec(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  sendBytes = 0;
	DWORD  flags     = 0;
	int    addrlen   = sizeof(SOCKADDR_IN);
	int    retcode;

	if ( perIoContext->offset > perIoContext->length )
	{
		PostQueuedCompletionStatus( mIocp, 0, (ULONG_PTR)NULL, (LPOVERLAPPED)perIoContext );
		return false;
	}

	wsaBuf.len = perIoContext->offset;
	wsaBuf.buf = perIoContext->buffer;

	retcode = WSASendTo( perIoContext->socket,
						 &wsaBuf,
						 1,
						 &sendBytes,
						 flags,
						 (SOCKADDR*)&perIoContext->addr,
						 addrlen,
						 &(perIoContext->wsaOverlapped),
						 NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		PostQueuedCompletionStatus( mIocp, 0, (ULONG_PTR)NULL, (LPOVERLAPPED)perIoContext );
		return false;
	}
	return true;
}

// SendPost Method
bool cIocpUdpSend::SendPost(PerIoContext* perIoContext)
{
	cCSLock        lock( &mCs );
	PerIoContext** buffer = mIoContextBackBuffer->buffer;
	long&          offset = mIoContextBackBuffer->offset;

	if ( offset < MAX_IO_CONTEXT_BUFFER_LEN )
	{
		buffer[ offset ] = perIoContext;
		offset++;
		return true;
	}

	mIoContextPool->ReleaseIoContext( perIoContext );
	return false;
}

/*-- IoContextPresent Method
*/
void cIocpUdpSend::IoContextPresent( )
{
	IoContextBuffer* temp;

	// Double Buffering 처리.
	CSBlock( &mCs )
	{
		temp                  = mIoContextBackBuffer;
		mIoContextBackBuffer  = mIoContextFrontBuffer;
		mIoContextFrontBuffer = temp;
	}

	// 발신된 데이터 처리.
	PerIoContext** buffer = mIoContextFrontBuffer->buffer;
	long&          offset = mIoContextFrontBuffer->offset;

	while ( offset > 0 )
	{
		SendExec( (*buffer) );
		(*buffer) = NULL;

		buffer++;
		offset--;

		Sleep( 5 );
	}
}

// SendComplete Method
bool cIocpUdpSend::SendComplete(ULONG_PTR     /*completionKey*/,
							    PerIoContext* perIoContext,
							    DWORD         /*bytesTransferred*/)
{
	// 사용이 완료된 I/O Context는 회수한다.
	mIoContextPool->ReleaseIoContext( perIoContext );
	return true;
}

// CallbackComplete Method
bool cIocpUdpSend::CallbackComplete(ULONG_PTR     /*completionKey*/,
								    PerIoContext* perIoContext,
								    DWORD         /*bytesTransferred*/)
{
	// 사용이 완료된 I/O Context는 회수한다.
	mIoContextPool->ReleaseIoContext( perIoContext );
	return true;
}

// WorkerThread Method
DWORD cIocpUdpSend::WorkerThread( )
{
	DWORD         bytesTransferred;
	ULONG_PTR     completionKey;
	OVERLAPPED*   overlapped;
	BOOL          retValue;

	PerIoContext* perIoContext;

	while ( true )
	{
		// completion port에 할당된 모든 소켓의 I/O 완료를 기다린다.
		retValue = GetQueuedCompletionStatus( mIocp, &bytesTransferred, &completionKey, &overlapped, INFINITE );

		// Shutdown
		if ( overlapped == IOCP_SHUTDOWN )
			return 0;

		perIoContext = (PerIoContext*)overlapped;

		if ( retValue == FALSE || bytesTransferred == 0 )
		{
			if ( perIoContext != NULL )
			{
				// 사용이 완료된 I/O Context는 회수한다.
				perIoContext->offset = max( perIoContext->offset, bytesTransferred );
				mIoContextPool->ReleaseIoContext( perIoContext );
			}
			continue;
		}

		switch ( perIoContext->requestType )
		{
		case IOCP_REQUEST_WRITE:
			SendComplete( completionKey, perIoContext, bytesTransferred );
			break; // Send 완료 핸들러 함수.

		case IOCP_REQUEST_CALLBACK:
			CallbackComplete( completionKey, perIoContext, bytesTransferred );
			break; // Callback 완료 핸들러 함수.
		}
	}
	return 0;
}

// BackendThread Method
DWORD cIocpUdpSend::BackendThread( )
{
	while ( true )
	{
		// 서버종료, Thread를 나간다.
		if ( mEndServer == true )
			break;

		IoContextPresent( );
		Sleep( 50 );
	}

	IoContextPresent( );
	return 0;
}

// WorkerThreadStartingPoint Method ( Worker Thread 시작 포인트 )
DWORD cIocpUdpSend::WorkerThreadStartingPoint(void* ptr)
{
	cIocpUdpSend* iocpUdpSend = (cIocpUdpSend*)ptr;
	return iocpUdpSend->WorkerThread( );
}

// BackendThreadStartingPoint Method ( Backend Thread 시작 포인트 )
DWORD cIocpUdpSend::BackendThreadStartingPoint(void* ptr)
{
	cIocpUdpSend* iocpUdpSend = (cIocpUdpSend*)ptr;
	return iocpUdpSend->BackendThread( );
}
