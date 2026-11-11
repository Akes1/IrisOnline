/* ====================================================================
 * 颇    老 :	MemoryPool.h
 * 格    利 :	
 * 累 己 磊 :	
 * 累 己 老 :	
 * 林狼荤亲 :	
 * =================================================================== */
#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

typedef int (WINAPI *COMPARE_ROUTINE)(void* arg1, void* arg2);

struct PerNode
{
	struct PerNode* next;			// Linked List - next
	struct PerNode* prev;			// Linked List - prev
	struct PerNode* parent;			// Binary Tree - Parent
	struct PerNode* left;			// Binary Tree - Child, left
	struct PerNode* right;			// Binary Tree - Child, right

	PerNode(){};
	virtual ~PerNode(){};

};

struct NodeRoot
{
	PerNode* pool;
	PerNode* root;
};

class cMemoryPool
{
public:
// Linked List
	void     AttachPool  ( PerNode** pool, PerNode* node );
	void     DetachPool  ( PerNode** pool, PerNode* node );

public:
// Binary Tree
	bool     AttachBst   ( PerNode** root, PerNode* node, COMPARE_ROUTINE compareRoutine );
	bool     DetachBst   ( PerNode** root, PerNode* node );

	void     SetBstLeft  ( PerNode* parent, PerNode* child );
	void     SetBstRight ( PerNode* parent, PerNode* child );

public:
// Common
	cMemoryPool( );
	virtual ~cMemoryPool(void);

	PerNode* AllocNode   ( DWORD bytes );
	void     FreeNode    ( void** node );

	PerNode* SearchNode  ( NodeRoot* nodeRoot, void* value, COMPARE_ROUTINE compareRoutine );

	PerNode* GetPool     ( PerNode** nonPagedPool, DWORD bytes );
	void     ReleasePool ( PerNode** nonPagedPool, PerNode* node );

public:
	static cMemoryPool* mpMemoryPool;
};

// SetBstLeft Method
inline void cMemoryPool::SetBstLeft(PerNode* parent, PerNode* child)
{
	parent->left = child;
	if ( child ) child->parent = parent;
}

// SetBstRight Method
inline void cMemoryPool::SetBstRight(PerNode* parent, PerNode* child)
{
	parent->right = child;
	if ( child ) child->parent = parent;
}

#define MEMORY_POOL	cMemoryPool::mpMemoryPool

#endif /// __MEMORY_POOL_H__
