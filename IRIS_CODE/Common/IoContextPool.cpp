// Include files
#include "StdAfx.h"
#include "iocontextpool.h"
#include "criticalSectionLock.h"
#include <stdio.h>

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if (L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if (R) (R)->parent = P; }

// Global data
bool g_ioContextBuffer = false;

// cIoContextPool Constructor
cIoContextPool::cIoContextPool(u_long bufferLength) : mBufferLength(bufferLength)
{
	// Critical Section를 초기화 한다.
	InitializeCriticalSection( &mCs );

	// Pool Usage Pointer
	mPagedPoolUsage         = NULL;
	mNonPagedPoolUsage      = NULL;

	// 메모리 카운터.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
}

// ~cIoContextPool Destructor.
cIoContextPool::~cIoContextPool(void)
{
	// cIoContextPool를 해제.
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &mCs );
}

// DefaultWorkingSize Method
bool cIoContextPool::DefaultWorkingSize(DWORD workingSize)
{
	cCSLock       lock( &mCs );                     // Critical Section
	PerIoContext* temp = NULL;

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

// GetProcessMemoryInfo Method.
void cIoContextPool::GetProcessMemoryInfo(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	cCSLock lock( &mCs );
	quotaPagedPoolUsage    = mQuotaPagedPoolUsage;
	quotaNonPagedPoolUsage = mQuotaNonPagedPoolUsage;
	workingSetSize         = mWorkingSetSize;
}

// Shutdown Method
void cIoContextPool::Shutdown( )
{
	cCSLock       lock( &mCs );
	PerIoContext* prev;
	PerIoContext* next;
	PerIoContext* temp;

	// Pool Usage Shutdown - PagedPoolUsage
	while ( mPagedPoolUsage != NULL )
	{
		prev               = mPagedPoolUsage->prev;
		next               = mPagedPoolUsage->next;
		temp               = mPagedPoolUsage;

		mPagedPoolUsage    = mPagedPoolUsage->next;
		FreeIoContext( &temp );
		mQuotaPagedPoolUsage--;
	}
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		prev               = mNonPagedPoolUsage->prev;
		next               = mNonPagedPoolUsage->next;
		temp               = mNonPagedPoolUsage;

		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreeIoContext( &temp );
		mQuotaNonPagedPoolUsage--;
	}
}

// AllocIoContext Method - I/O Context를 생성한다.
PerIoContext* cIoContextPool::AllocIoContext( )
{
	PerIoContext* perIoContext = (PerIoContext*)GlobalAlloc( GPTR, sizeof(PerIoContext) );

	if ( perIoContext != NULL )
	{
		ZeroMemory( &perIoContext->wsaOverlapped, sizeof(WSAOVERLAPPED) );          // overlapped I/O 모델 사용을 위한 구조체
		perIoContext->socket        = INVALID_SOCKET;                               // 클라이언트 소켓, PerSocketContext와 참조
		ZeroMemory( &perIoContext->addr, sizeof(SOCKADDR_IN) );                     // 클라이언트 주소 구조체
		perIoContext->buffer        = (char*)GlobalAlloc( GPTR, mBufferLength );    // 메모리를 할당한다. - GPTR 사용시 0으로 초기화된다.
		perIoContext->offset        = 0;                                            // 버퍼의 오프셋
		perIoContext->length        = mBufferLength;                                // 버퍼의 길이
		perIoContext->requestType   = IOCP_REQUEST_READ;                            // Request Type
		perIoContext->requestResult = IOCP_REQUEST_SUCCESS;                         // Request Result
		perIoContext->iParam        = 0;                                            // i Param
		perIoContext->lParam        = 0;                                            // l Param
		perIoContext->prev          = NULL;                                         // 선형리스트의 포인터 - 이전
		perIoContext->next          = NULL;                                         // 선형리스트의 포인터 - 다음

		// I/O Context::Buffer 생성 - 성공시, mWorkingSetSize 증가. 실패시, I/O Context 삭제.
		(perIoContext->buffer != NULL) ? mWorkingSetSize++ : FreeIoContext( &perIoContext );
	}

	if ( perIoContext == NULL )
	{
		throw "Error - PerIoContext* cIoContextPool::AllocIoContext( ) is null.";
	}

	return perIoContext;
}

// FreeIoContext Method - I/O Context를 삭제한다.
void cIoContextPool::FreeIoContext(PerIoContext** perIoContext)
{
	if ( (*perIoContext) != NULL )
	{
		if ( (*perIoContext)->buffer != NULL )
		{
			GlobalFree( (*perIoContext)->buffer );
			(*perIoContext)->buffer = NULL;
		}
		GlobalFree( (*perIoContext) );
		(*perIoContext) = NULL;

		// mWorkingSetSize를 감소.
		mWorkingSetSize--;
	}
}

// AttachPool Method
void cIoContextPool::AttachPool(PerIoContext** pool, PerIoContext* perIoContext)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = perIoContext;

		perIoContext->prev = NULL;
		perIoContext->next = (*pool);
	}
	(*pool) = perIoContext;
}

// DetachPool Method
void cIoContextPool::DetachPool(PerIoContext** pool, PerIoContext* perIoContext)
{
	PerIoContext* prev = perIoContext->prev;
	PerIoContext* next = perIoContext->next;

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

	perIoContext->prev = NULL;
	perIoContext->next = NULL;
}

// GetPool Method
PerIoContext* cIoContextPool::GetPool( )
{
	PerIoContext* perIoContext = mNonPagedPoolUsage;

	if ( perIoContext != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, perIoContext );
		mQuotaNonPagedPoolUsage--;
	}
	else
	{
		// 페이지가 없을 경우 새로 생성한다.
		perIoContext = AllocIoContext( );
	}

	if ( perIoContext != NULL )
	{
		// 페이지 된 풀 사용량을 증감.
		AttachPool( &mPagedPoolUsage, perIoContext );
		mQuotaPagedPoolUsage++;
	}

	return perIoContext;
}

// ReleasePool Method
void cIoContextPool::ReleasePool(PerIoContext* perIoContext, bool deleteIoContext)
{
	// 페이지 된 풀 사용량을 차감.
	DetachPool( &mPagedPoolUsage, perIoContext );
	mQuotaPagedPoolUsage--;

	if ( deleteIoContext != true )
	{
		// 페이지 안된 풀 사용량을 증감.
		AttachPool( &mNonPagedPoolUsage, perIoContext );
		mQuotaNonPagedPoolUsage++;
	}
	else
	{
		// 페이지 삭제.
		FreeIoContext( &perIoContext );
	}
}

// GetIoContext Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseIoContext와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerIoContext* cIoContextPool::GetIoContext( )
{
	cCSLock       lock( &mCs );
	PerIoContext* perIoContext = GetPool( );
	return perIoContext;
}
PerIoContext* cIoContextPool::GetIoContext(SOCKET socket, u_long request)
{
	cCSLock       lock( &mCs );
	PerIoContext* perIoContext = GetPool( );

	if ( perIoContext != NULL )
	{
		perIoContext->socket      = socket;
		perIoContext->requestType = request;
	}

	return perIoContext;
}
PerIoContext* cIoContextPool::GetIoContext(SOCKET socket, u_long request, SOCKADDR_IN addr)
{
	cCSLock       lock( &mCs );
	PerIoContext* perIoContext = GetPool( );

	if ( perIoContext != NULL )
	{
		perIoContext->socket      = socket;
		perIoContext->addr        = addr;
		perIoContext->requestType = request;
	}

	return perIoContext;
}

// ReleaseIoContext Method - 선형리스트의 HEAD로 회수한다.
// GetIoContext와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cIoContextPool::ReleaseIoContext(PerIoContext* perIoContext, bool deleteIoContext, DWORD* errorPtr)
{
	cCSLock lock( &mCs );
	u_long  offset = perIoContext->offset + perIoContext->InternalHigh;		// 사용된 오프셋.

	if ( offset > perIoContext->length )
		throw "Error - void cIoContextPool::ReleaseIoContext( ) is overflow.";

	if ( g_ioContextBuffer == true )
	{
		if ( deleteIoContext != true && errorPtr != NULL )
		{
			u_long len = perIoContext->length - offset;							// 사용후 사이즈.
			char*  buf = perIoContext->buffer + offset;							// 사용후 포인터.

			while ( len > 0 )
			{
				if ( (*buf) != 0 )
				{
					(*errorPtr)++;      // 오류처리량.
					(*buf) = 0;			// 초기화.
				}
				buf++; len--;
			}
		}
	}

	ZeroMemory( &perIoContext->wsaOverlapped, sizeof(WSAOVERLAPPED) );      // Overlapped 초기화.
	perIoContext->socket        = INVALID_SOCKET;                           // 소켓 초기화.
	ZeroMemory( &perIoContext->addr, sizeof(SOCKADDR_IN) );                 // 주소 초기화.
	ZeroMemory( perIoContext->buffer, offset );                             // 메모리 초기화.
	perIoContext->offset        = 0;                                        // 오프셋 초기화.
	perIoContext->Internal      = 0;                                        // 내부 초기화.
	perIoContext->InternalHigh  = 0;                                        // 내부 초기화.
	perIoContext->requestType   = IOCP_REQUEST_READ;                        // 요구값 초기화.
	perIoContext->requestResult = IOCP_REQUEST_SUCCESS;                     // 결과값 초기화.
	perIoContext->iParam        = 0;                                        // 파라메터 초기화.
	perIoContext->lParam        = 0;                                        // 파라메터 초기화.

	ReleasePool( perIoContext, deleteIoContext );
}
