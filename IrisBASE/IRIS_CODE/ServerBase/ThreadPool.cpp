// Include files
#include "threadpool.h"

// Local definitions

// Global data

// cThreadPool Constructor
cThreadPool::cThreadPool(void) : mRequestQueue(NULL), mWorkerThreadNumber(0)
{
}

// ~cThreadPool Destructor
cThreadPool::~cThreadPool(void)
{
	Shutdown( );
}

// Initialize Method
bool cThreadPool::Initialize(HANDLE completion, int numThreads, int numWorkerThreads)
{
	// CreateIoCompletionPort를 이용하여 completion port 오브젝트를 생성할 때 일반적으로 신경써야 할 파
	// 라메터는 NumberOfConcurrentThreads 이다. 처음 3개의 파라메터는 값을 설정하지 않는다.
	//
	// NumberOfConcurrentThreads는 현재 completion port에 대하여 몇개의 스레드를 실행할 것인가를 결정한
	// 다. 가장 이상적인 경우는 스레드의 context switching을 방지하기 위하여 서비스에 대하여 하나의 프로
	// 세서당 하나의 스레드만 동작하도록 하는 것이다. NumberOfConcurrentThreads의 값을 0으로 설정하면 시
	// 스템은 프로세서에 적합한 스레드 개수를 할당하여 사용한다.
	// (하나의 CPU에 대하여 IOCP 하나당 하나의 스레드를 생성)
	mRequestQueue = CreateIoCompletionPort( completion, NULL, 0, numThreads );

	// CreateIoCompletionPort 오브젝트를 생성한 뒤에 작업스레드를 생성한다. 이때 생성하는 작업스레드는,
	// 스레드가 블록될 가능성이 있다면, NumberOfConcurrentThreads 의 갯수보다 많이 생성하는 것이 좋다.
	// NumberOfConcurrentThreads만큼 스레드가 항상 실행되게 할 수 있기 때문에, 블록된 스레드가 있다면 다
	// 른 스레드가 대신 동작할 것이기 때문이다.
	SYSTEM_INFO si;
	DWORD       threadId;

	GetSystemInfo(&si);

	mWorkerThreadNumber = min( si.dwNumberOfProcessors * numWorkerThreads, MAX_WORKER_THREAD );
	for ( int i = 0; i < mWorkerThreadNumber; i++ )
	{
		mWorkerThread[i] = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	}

	return (mRequestQueue != NULL) ? true : false;
}

// Shutdown Method
void cThreadPool::Shutdown( )
{
	// Not initialized
	if ( mRequestQueue == NULL )
		return;

	// Cause worker threads to exit
	if ( mRequestQueue != NULL )
	{
		for ( int i = 0; i < mWorkerThreadNumber; i++ )
		{
			PostQueuedCompletionStatus( mRequestQueue, 0, 0, POOL_SHUTDOWN );
		}
	}

	// Make sure worker threads exits.
	for ( int i = 0; i < mWorkerThreadNumber; i++ )
	{
		if ( WaitForSingleObject( mWorkerThread[i], 60000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( mWorkerThread[i], &exitCode);
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( mWorkerThread[i], 0 );
			}
		}
		CloseHandle( mWorkerThread[i] );
		mWorkerThread[i] = NULL;
	}

	// Close the request queue handle
	CloseHandle( mRequestQueue );

	// Clear the queue handle
	mRequestQueue = NULL;
}

// QueueRequest Method
BOOL cThreadPool::QueueRequest(ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred)
{
	return PostQueuedCompletionStatus( mRequestQueue, bytesTransferred, completionKey, overlapped );
}

// WorkerThread Method
DWORD cThreadPool::WorkerThread( )
{
	DWORD       bytesTransferred;
	ULONG_PTR   completionKey;
	OVERLAPPED* overlapped;

	while ( GetQueuedCompletionStatus( mRequestQueue, &bytesTransferred, &completionKey, &overlapped, INFINITE) )
	{
		// Shut down
		if ( overlapped == POOL_SHUTDOWN )
			break;
	}

	return 0L;
}

// WorkerThreadStartingPoint Method
DWORD cThreadPool::WorkerThreadStartingPoint(void* ptr)
{
	cThreadPool* threadPool = (cThreadPool*)ptr;
	return threadPool->WorkerThread( );
}
