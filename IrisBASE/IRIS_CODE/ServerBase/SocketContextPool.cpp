// Include files
#include "socketcontextpool.h"

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if (L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if (R) (R)->parent = P; }

// Global data

// cSocketContextPool Constructor
cSocketContextPool::cSocketContextPool(CRITICAL_SECTION* cs, u_long bufferLength) : mCs(cs), mBufferLength(bufferLength)
{
	// Pool Usage Pointer
	mPagedPoolUsage         = NULL;
	mNonPagedPoolUsage      = NULL;

	// BST Root Pointer
	mBstRoot                = NULL;

	// 메모리 카운터 초기화.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
}

// ~cSocketContextPool Destructor.
cSocketContextPool::~cSocketContextPool(void)
{
	// cSocketContextPool를 종료.
	Shutdown( );
}

// DefaultWorkingSize Method
bool cSocketContextPool::DefaultWorkingSize(DWORD workingSize)
{
	cCSLock           lock( mCs );                  // Critical Section
	PerSocketContext* temp = NULL;

	try
	{
		while ( workingSize > 0 )
		{
			temp = GetPool( );

				if ( temp == NULL ) throw false;    // Socket Context를 준비.

			workingSize--;
		}
		throw true;
	}
	catch ( bool boolean )
	{
		while ( mPagedPoolUsage != NULL )
		{
			ReleasePool( mPagedPoolUsage );
		}
		return boolean;
	}
}

// GetProcessMemoryInfo Method
void cSocketContextPool::GetProcessMemoryInfo(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	cCSLock lock( mCs );
	quotaPagedPoolUsage    = mQuotaPagedPoolUsage;
	quotaNonPagedPoolUsage = mQuotaNonPagedPoolUsage;
	workingSetSize         = mWorkingSetSize;
}

// Shutdown Method
void cSocketContextPool::Shutdown( )
{
	cCSLock           lock( mCs );
	PerSocketContext* temp;

	// BST Shutdown
	while ( mBstRoot != NULL )
	{
		DetachBst( mBstRoot );
	}
	// Pool Usage Shutdown - PagedPoolUsage
	while ( mPagedPoolUsage != NULL )
	{
		temp               = mPagedPoolUsage;
		mPagedPoolUsage    = mPagedPoolUsage->next;
		FreeSocketContext( &temp );
	}
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreeSocketContext( &temp );
	}
}

// AllocSocketContext Method - PerSocketContext를 생성.
PerSocketContext* cSocketContextPool::AllocSocketContext( )
{
	PerSocketContext* perSocketContext = (PerSocketContext*)GlobalAlloc( GPTR, sizeof(PerSocketContext) );

	if ( perSocketContext != NULL )
	{
		// Socket Context를 초기화.
		perSocketContext->socket       = INVALID_SOCKET;                             // 클라이언트 소켓, PerIoContext와 참조.
		perSocketContext->seq          = 0;                                          // Sequence Number.
		perSocketContext->statusData   = 0;                                          // 상태 초기화.
		memset( &perSocketContext->addr, 0, sizeof(SOCKADDR_IN) );                   // 인터넷 주소.
		perSocketContext->timeToLive   = 0;                                          // Time To Live.
		perSocketContext->buffer       = (char*)GlobalAlloc( GPTR, mBufferLength );  // 메모리를 할당한다. - GPTR 사용시 0으로 초기화된다.
		perSocketContext->offset       = 0;                                          // 버퍼의 오프셋.
		perSocketContext->length       = mBufferLength;                              // 버퍼의 길이.
		perSocketContext->Internal     = 0;                                          // 내부 오프셋.
		perSocketContext->InternalHigh = 0;                                          // 내부 오프셋.
		perSocketContext->cid          = 0;                                          // Connection ID.
		perSocketContext->wsaBuf       = (char*)GlobalAlloc( GPTR, mBufferLength*2 );// 메모리를 할당한다. - GPTR 사용시 0으로 초기화된다.
		perSocketContext->wsaOff       = 0;                                          // 버퍼의 오프셋.
		perSocketContext->wsaLen       = mBufferLength*2;                            // 버퍼의 길이.
		perSocketContext->prev         = NULL;                                       // 선형리스트의 포인터 - 이전
		perSocketContext->next         = NULL;                                       // 선형리스트의 포인터 - 다음
		perSocketContext->parent       = NULL;                                       // BST 포인터 - 모체
		perSocketContext->left         = NULL;                                       // BST 포인터 - 왼쪽
		perSocketContext->right        = NULL;                                       // BST 포인터 - 오른쪽

		// mWorkingSetSize를 증가.
		mWorkingSetSize++;
	}

	return perSocketContext;
}

// FreeSocketContext Method - PerSocketContext를 삭제.
void cSocketContextPool::FreeSocketContext(PerSocketContext** perSocketContext)
{
	if ( perSocketContext != NULL )
	{
		if ( (*perSocketContext)->socket != INVALID_SOCKET )
		{
			// force the subsequent closesocket to be abortative.
			//
			LINGER linger;

			linger.l_onoff  = 1;
			linger.l_linger = 0;
			setsockopt( (*perSocketContext)->socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger) );

			closesocket( (*perSocketContext)->socket );
			(*perSocketContext)->socket = INVALID_SOCKET;
		}
		if ( (*perSocketContext)->wsaBuf != NULL )
		{
			GlobalFree ( (*perSocketContext)->wsaBuf );
			(*perSocketContext)->wsaBuf = NULL;
		}
		if ( (*perSocketContext)->buffer != NULL )
		{
			GlobalFree ( (*perSocketContext)->buffer );
			(*perSocketContext)->buffer = NULL;
		}
		GlobalFree ( (*perSocketContext) );
		(*perSocketContext) = NULL;

		// mWorkingSetSize를 감소.
		mWorkingSetSize--;
	}
}

// CompareCID Method
int cSocketContextPool::CompareCID(PerSocketContext* perSocketContext1, PerSocketContext* perSocketContext2)
{
	return (perSocketContext1->cid - perSocketContext2->cid);
}
int cSocketContextPool::CompareCID(int cid1, int cid2)
{
	return (cid1 - cid2);
}

// AttachPool Method
void cSocketContextPool::AttachPool(PerSocketContext** pool, PerSocketContext* perSocketContext)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = perSocketContext;

		perSocketContext->prev = NULL;
		perSocketContext->next = (*pool);
	}
	(*pool) = perSocketContext;
}

// DetachPool Method
void cSocketContextPool::DetachPool(PerSocketContext** pool, PerSocketContext* perSocketContext)
{
	PerSocketContext* prev = perSocketContext->prev;
	PerSocketContext* next = perSocketContext->next;

	if ( prev == NULL && next == NULL )
	{
		(*pool) = NULL;
	}
	else if ( prev == NULL && next != NULL )
	{
		next->prev = NULL;
		(*pool) = next;
	}
	else if ( prev != NULL && next == NULL )
	{
		prev->next = NULL;
	}
	else if ( prev != NULL && next != NULL )
	{
		prev->next = next;
		next->prev = prev;
	}

	perSocketContext->prev = NULL;
	perSocketContext->next = NULL;
}

// AttachBst Method
bool cSocketContextPool::AttachBst(PerSocketContext* perSocketContext)
{
	PerSocketContext* parent = NULL;
	PerSocketContext* child  = mBstRoot;
	int               result;

	while ( child != NULL )
	{
		parent = child;
		result = CompareCID( child->cid, perSocketContext->cid );

		if ( result == 0 ) return false;
		else if ( result > 0 ) child = child->left;
		else if ( result < 0 ) child = child->right;
	}

	if ( parent == NULL )
	{
		mBstRoot = perSocketContext;
	}
	else if ( CompareCID( parent, perSocketContext ) > 0 )
	{
		SetLeft( parent, perSocketContext );
	}
	else // if ( CompareCID( parent, perSocketContext ) < 0 )
	{
		SetRight( parent, perSocketContext );
	}
	return true;
}

// DetachBst Method
bool cSocketContextPool::DetachBst(PerSocketContext* perSocketContext)
{
	PerSocketContext* parent = perSocketContext->parent;
	PerSocketContext* left   = perSocketContext->left;
	PerSocketContext* right  = perSocketContext->right;
	PerSocketContext* child  = NULL;

	if ( left == NULL )
	{
		child = right;
	}
	else if ( right == NULL )
	{
		child = left;
	}
	else
	{
		child = right;
		while ( child->left != NULL ) child = child->left;

		if ( child->parent != perSocketContext )
		{
			SetLeft( child->parent, child->right );
			SetRight( child, right );
		}
		child->parent = parent;
		SetLeft( child, left );
	}

	if ( mBstRoot == perSocketContext )
	{
		mBstRoot = child;
		if ( mBstRoot != NULL ) mBstRoot->parent = NULL;
	}
	else if ( perSocketContext == parent->left )
	{
		SetLeft( parent, child );
	}
	else
	{
		SetRight( parent, child );
	}

	perSocketContext->parent = NULL;
	perSocketContext->left   = NULL;
	perSocketContext->right  = NULL;

	return true;
}

// GetPool Method
PerSocketContext* cSocketContextPool::GetPool( )
{
	PerSocketContext* perSocketContext = mNonPagedPoolUsage;

	if ( perSocketContext != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, perSocketContext );
		mQuotaNonPagedPoolUsage--;
	}
	else
	{
		// 페이지가 없을 경우 새로 생성.
		perSocketContext = AllocSocketContext( );
	}

	if ( perSocketContext != NULL )
	{
		AttachPool( &mPagedPoolUsage, perSocketContext );
		mQuotaPagedPoolUsage++;
	}

	return perSocketContext;
}

// ReleasePool Method
void cSocketContextPool::ReleasePool(PerSocketContext* perSocketContext, bool isDelete)
{
	// 페이지 된 풀 사용량을 차감.
	DetachPool( &mPagedPoolUsage, perSocketContext );
	mQuotaPagedPoolUsage--;

	if ( isDelete != true )
	{
		// 페이지 안된 풀 사용량을 증감.
		AttachPool( &mNonPagedPoolUsage, perSocketContext );
		mQuotaNonPagedPoolUsage++;
	}
	else
	{
		// 페이지 삭제.
		FreeSocketContext( &perSocketContext );
	}
}

// ClearCID Method
bool cSocketContextPool::ClearCID(PerSocketContext* perSocketContext)
{
	// BST - 연결삭제.
	if ( perSocketContext->status.connectionID )
	{
		perSocketContext->status.connectionID = false;
		perSocketContext->status.closeWait    = true;
		return DetachBst( perSocketContext );
	}
	return false;
}

// GetCID Method
PerSocketContext* cSocketContextPool::GetCID(DWORD cid)
{
	cCSLock           lock( mCs );
	PerSocketContext* perSocketContext = mBstRoot;
	int               result;

	while ( perSocketContext != NULL )
	{
		result = CompareCID( perSocketContext->cid, cid );

		if ( result == 0 )
		{
			return (perSocketContext->socket != INVALID_SOCKET ? perSocketContext : NULL);
		}
		else if ( result > 0 )
		{
			perSocketContext = perSocketContext->left;
		}
		else if ( result < 0 )
		{
			perSocketContext = perSocketContext->right;
		}
	}

	return NULL;
}

// SetCID Method
bool cSocketContextPool::SetCID(PerSocketContext* perSocketContext, DWORD cid)
{
	perSocketContext->cid = cid;
	perSocketContext->status.connectionID = AttachBst( perSocketContext );
	return perSocketContext->status.connectionID;
}

// GetPerSocketContext Method
// ReleasePerSocketContext와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerSocketContext* cSocketContextPool::GetPerSocketContext(SOCKET socket, SOCKADDR_IN addr, DWORD ttl)
{
	cCSLock           lock( mCs );                                      // Critical Section
	PerSocketContext* perSocketContext = GetPool( );                    // Socket Context를 준비.

	if ( perSocketContext != NULL )
	{
		perSocketContext->socket     = socket;                          // 소켓 값 복사.
		perSocketContext->addr       = addr;                            // IP 주소 복사.
		perSocketContext->timeToLive = GetTickCount( ) + ttl;           // TTL.
	}

	return perSocketContext;
}

// ReleasePerSocketContext Method
// GetPerSocketContext와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cSocketContextPool::ReleasePerSocketContext(PerSocketContext* perSocketContext, bool isDelete)
{
	cCSLock lock( mCs );                                                // Critical Section
	LINGER  linger;

	// force the subsequent closesocket to be abortative.
	//
	linger.l_onoff  = 1;
	linger.l_linger = 0;
	setsockopt( perSocketContext->socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger) );
	closesocket( perSocketContext->socket );

	// BST - 연결종료.
	if ( perSocketContext->status.connectionID )
		DetachBst( perSocketContext );

	perSocketContext->socket       = INVALID_SOCKET;                    // 소켓, 초기화.
	perSocketContext->seq          = 0;                                 // SEQ,  초기화.
	perSocketContext->ack          = 0;                                 // ACK,  초기화.
	perSocketContext->statusData   = 0;                                 // 상태, 초기화.
	ZeroMemory( &perSocketContext->addr, sizeof(SOCKADDR_IN) );         // 주소, 초기화.
	perSocketContext->timeToLive   = 0;                                 // TTL 초기화.
	ZeroMemory( perSocketContext->buffer, perSocketContext->offset + perSocketContext->InternalHigh );
	                                                                    // 메모리 초기화.
	perSocketContext->offset       = 0;                                 // 오프셋 초기화.
	perSocketContext->Internal     = 0;                                 // 내부   초기화.
	perSocketContext->InternalHigh = 0;                                 // 내부   초기화.
	perSocketContext->cid          = 0;                                 // CID    초기화.
	ZeroMemory( perSocketContext->wsaBuf, perSocketContext->wsaOff );
	                                                                    // 메모리 초기화.
	perSocketContext->wsaOff       = 0;                                 // 오프셋 초기화.

	// Socket Context를 회수.
	ReleasePool( perSocketContext, isDelete );
}
