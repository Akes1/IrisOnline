// Include
#include "packetpool.h"

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }

// Global data

// PacketPool Constructor
PacketPool::PacketPool( )
{
	// Pool Usage Pointer
	mPagedPoolUsage         = NULL;
	mNonPagedPoolUsage      = NULL;

	// BST Root Pointer
	mBstRoot                = NULL;

	// 메모리 카운터.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
}

// ~PacketPool Destructor.
PacketPool::~PacketPool(void)
{
	// PacketPool를 해제.
	Shutdown();
}

// Shutdown Method
void PacketPool::Shutdown()
{
	PacketBuffer* temp;

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
		FreePacketBuffer( &temp );
	}
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreePacketBuffer( &temp );
	}
}

// AllocPacketBuffer Method
PacketBuffer* PacketPool::AllocPacketBuffer()
{
	PacketBuffer* packetBuffer = (PacketBuffer*)GlobalAlloc( GPTR, sizeof(PacketBuffer) );

	if ( packetBuffer != NULL )
	{
		mWorkingSetSize++;	// mWorkingSetSize를 증가.

		packetBuffer->pid    = 0;	// Packet ID
		packetBuffer->offset = 0;	// Offset

		packetBuffer->prev   = NULL;	// 선형리스트의 포인터 - 이전
		packetBuffer->next   = NULL;	// 선형리스트의 포인터 - 다음
		packetBuffer->parent = NULL;	// BST 포인터 - 상위
		packetBuffer->left   = NULL;	// BST 포인터 - 왼쪽
		packetBuffer->right  = NULL;	// BST 포인터 - 오른쪽
	}

	return packetBuffer;
}

// FreePacketBuffer Method.
void PacketPool::FreePacketBuffer(PacketBuffer** packetBuffer)
{
	if ( (*packetBuffer) != NULL )
	{
		GlobalFree( (*packetBuffer) );
		(*packetBuffer) = NULL;

		// mWorkingSetSize를 감소.
		mWorkingSetSize--;
	}
}

// Compare Method
int PacketPool::Compare(long cid1, long cid2)
{
	return (cid1 - cid2);
}

// AttachPool Method
void PacketPool::AttachPool(PacketBuffer** pool, PacketBuffer* packetBuffer)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = packetBuffer;

		packetBuffer->prev = NULL;
		packetBuffer->next = (*pool);
	}
	(*pool) = packetBuffer;
}

// DetachPool Method
void PacketPool::DetachPool(PacketBuffer** pool, PacketBuffer* packetBuffer)
{
	PacketBuffer* prev = packetBuffer->prev;
	PacketBuffer* next = packetBuffer->next;

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

	packetBuffer->prev = NULL;
	packetBuffer->next = NULL;
}

// AttachBst Method
bool PacketPool::AttachBst(PacketBuffer* packetBuffer)
{
	PacketBuffer* parent = NULL;
	PacketBuffer* child  = mBstRoot;
	int           result;

	while ( child != NULL )
	{
		parent = child;
		result = Compare( child->pid, packetBuffer->pid );

		if ( result == 0 ) return false;
		else if ( result > 0 ) child = child->left;
		else if ( result < 0 ) child = child->right;
	}

	if ( parent == NULL )
	{
		mBstRoot = packetBuffer;
	}
	else if ( Compare( parent->pid, packetBuffer->pid ) > 0 )
	{
		SetLeft( parent, packetBuffer );
	}
	else /*if ( Compare( parent->pid, packetBuffer->pid ) < 0 )*/
	{
		SetRight( parent, packetBuffer );
	}
	return true;
}

// DetachBst Method
bool PacketPool::DetachBst(PacketBuffer* packetBuffer)
{
	PacketBuffer* parent = packetBuffer->parent;
	PacketBuffer* left   = packetBuffer->left;
	PacketBuffer* right  = packetBuffer->right;
	PacketBuffer* child  = NULL;

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

		if ( child->parent != packetBuffer )
		{
			SetLeft( child->parent, child->right );
			SetRight( child, right );
		}
		child->parent = parent;
		SetLeft( child, left );
	}

	if ( mBstRoot == packetBuffer )
	{
		mBstRoot = child;
		if ( mBstRoot != NULL ) mBstRoot->parent = NULL;
	}
	else if ( packetBuffer == parent->left )
	{
		SetLeft( parent, child );
	}
	else
	{
		SetRight( parent, child );
	}

	packetBuffer->parent = NULL;
	packetBuffer->left   = NULL;
	packetBuffer->right  = NULL;

	return true;
}

// GetPool Method
PacketBuffer* PacketPool::GetPool(long pid)
{
	PacketBuffer* packetBuffer = mNonPagedPoolUsage;

	if ( packetBuffer != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, packetBuffer );
		mQuotaNonPagedPoolUsage--;
	}
	else
	{
		// 페이지가 없을 경우 새로 생성.
		packetBuffer = AllocPacketBuffer( );
	}

	if ( packetBuffer != NULL )
	{
		packetBuffer->pid = pid;

		// BST - 연결시도 / 페이지 된 풀 사용량을 증감.
		if ( AttachBst( packetBuffer ) == true )
		{
			AttachPool( &mPagedPoolUsage, packetBuffer );
			mQuotaPagedPoolUsage++;
			return packetBuffer;
		}
		// BST - 연결실패 / 페이지 안된 풀 사용량을 증감.
		else
		{
			AttachPool( &mNonPagedPoolUsage, packetBuffer );
			mQuotaNonPagedPoolUsage++;
			return NULL;
		}
	}

	return packetBuffer;
}

// ReleasePool Method
void PacketPool::ReleasePool(PacketBuffer* packetBuffer)
{
	// BST - 연결종료.
	DetachBst( packetBuffer );

	// 페이지 된 풀 사용량을 차감.
	DetachPool( &mPagedPoolUsage, packetBuffer );
	mQuotaPagedPoolUsage--;

	// 페이지 안된 풀 사용량을 증감.
	AttachPool( &mNonPagedPoolUsage, packetBuffer );
	mQuotaNonPagedPoolUsage++;
}

// SearchPacketBuffer Method
PacketBuffer* PacketPool::SearchPacketBuffer(long pid)
{
	PacketBuffer* ch = mBstRoot;
	int           result;

	while ( ch != NULL )
	{
		result = Compare( ch->pid, pid );
		if ( result == 0 )
			return ch;
		else if ( result > 0 )
			ch = ch->left;
		else if ( result < 0 )
			ch = ch->right;
	}

	return NULL;
}

// GetPacketBuffer Method - 선형리스트의 HEAD를 넘겨준다.
// ReleasePacketBuffer와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PacketBuffer* PacketPool::GetPacketBuffer(long pid)
{
	return GetPool( pid );
}

// ReleasePacketBuffer Method - 선형리스트의 HEAD로 회수한다.
// GetPacketBuffer와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void PacketPool::ReleasePacketBuffer(PacketBuffer* packetBuffer)
{
	packetBuffer->seq        = 0;
	packetBuffer->ack        = 0;
	memset( packetBuffer->buffer, 0, sizeof(packetBuffer->buffer) );
	packetBuffer->offset     = 0;
	packetBuffer->update = 0;

	ReleasePool( packetBuffer );
}
