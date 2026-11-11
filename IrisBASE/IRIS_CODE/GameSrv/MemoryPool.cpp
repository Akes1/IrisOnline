// Include
#include "gamesrv.h"
#include "memorypool.h"

// Local definitions

// Global data
cMemoryPool* cMemoryPool::mpMemoryPool = NULL;

// cMemoryPool Constructor
cMemoryPool::cMemoryPool( )
{
	// Global Variable
	mpMemoryPool = this;
}

// ~cMemoryPool Destructor.
cMemoryPool::~cMemoryPool(void)
{
	// cMemoryPool를 해제.
	mpMemoryPool = NULL;
}

// AllocNode Method
PerNode* cMemoryPool::AllocNode(DWORD bytes)
{
	PerNode* node = (PerNode*)GlobalAlloc( GPTR, bytes );
	if ( node != NULL )
	{
		node->prev   = NULL;  // 선형리스트의 포인터 - 이전
		node->next   = NULL;  // 선형리스트의 포인터 - 다음
		node->parent = NULL;  // BST 포인터 - 상위
		node->left   = NULL;  // BST 포인터 - 왼쪽
		node->right  = NULL;  // BST 포인터 - 오른쪽
	}
	else
	{
		throw "Error - PerNode* cMemoryPool::AllocNode( ) is null.";
	}
	return node;
}

// FreeNode Method.
void cMemoryPool::FreeNode(void** node)
{
	if ( (*node) != NULL )
	{
		GlobalFree( (*node) );	// Free PerNode.
		(*node) = NULL;			// Reinitialize - null.
	}
}

// AttachPool Method
void cMemoryPool::AttachPool(PerNode** pool, PerNode* node)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = node;
		node->prev = NULL;
		node->next = (*pool);
	}
	(*pool) = node;
}

// DetachPool Method
void cMemoryPool::DetachPool(PerNode** pool, PerNode* node)
{
	PerNode* prev = node->prev;
	PerNode* next = node->next;

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

	node->prev = NULL;
	node->next = NULL;
}

// GetPool Method
PerNode* cMemoryPool::GetPool(PerNode** nonPagedPool, DWORD bytes)
{
	PerNode* node = (*nonPagedPool);
	if ( node != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( nonPagedPool, node );
	}
	else
	{
		// 페이지가 없을 경우 새로 생성한다.
		node = AllocNode( bytes );
	}
	return node;
}

// ReleasePool Method
void cMemoryPool::ReleasePool(PerNode** nonPagedPool, PerNode* node)
{
	AttachPool( nonPagedPool, node );
}

// AttachBst Method
bool cMemoryPool::AttachBst(PerNode** root, PerNode* node, COMPARE_ROUTINE compareRoutine)
{
	PerNode* parent = NULL;
	PerNode* child  = (*root);
	int      result;

	while ( child != NULL )
	{
		parent = child;
		result = (*compareRoutine)( (void*)child, (void*)node );
		if ( result > 0 )
			child = child->left;
		else if ( result < 0 )
			child = child->right;
		else
			return false;
	}

	if ( parent != NULL )
	{
		result = (*compareRoutine)( (void*)parent, (void*)node );
		if ( result > 0 )
			SetBstLeft( parent, node );
		else if ( result < 0 )
			SetBstRight( parent, node );
		else
			return false;
	}
	else
		(*root) = node;
	return true;
}

// DetachBst Method
bool cMemoryPool::DetachBst(PerNode** root, PerNode* node)
{
	PerNode* parent = node->parent;
	PerNode* left   = node->left;
	PerNode* right  = node->right;
	PerNode* child  = NULL;

	if ( left == NULL )
		child = right;
	else if ( right == NULL )
		child = left;
	else
	{
		child = right;
		while ( child->left != NULL ) child = child->left;

		if ( child->parent != node )
		{
			SetBstLeft( child->parent, child->right );
			SetBstRight( child, right );
		}

		child->parent = parent;
		SetBstLeft( child, left );
	}

	if ( (*root) == node )
	{
		(*root) = child;
		if ( (*root) != NULL ) (*root)->parent = NULL;
	}
	else if ( node == parent->left )
	{
		SetBstLeft( parent, child );
	}
	else
	{
		SetBstRight( parent, child );
	}

	node->parent = NULL;
	node->left   = NULL;
	node->right  = NULL;
	return true;
}

// SearchNode Method
PerNode* cMemoryPool::SearchNode(NodeRoot* nodeRoot, void* value, COMPARE_ROUTINE compareRoutine)
{
	PerNode* node = nodeRoot->root;
	int      result;
	while ( node != NULL )
	{
		result = (*compareRoutine)( (void*)node, value );
		if ( result > 0 )
			node = node->left;
		else if ( result < 0 )
			node = node->right;
		else
			return node;
	}
	return NULL;
}
