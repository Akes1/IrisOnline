/* ====================================================================
 * 파    일 :	BinSearchAE.h
 * 목    적 :	검색을 지원하는 메모리 리스트를 제공한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.11.10
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		06.11.10	이법석		묵향 소스 참조
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

struct sBsaeNode;

struct sContainerAE
{
	sBsaeNode*			pOwner;

	sContainerAE*		pPrv;
	sContainerAE*		pNext;

	void*				pItem;
	unsigned long		dwKey;
};


struct sBsaeNode
{
	unsigned long		dwKey;
	sBsaeNode*			pParent;
	sBsaeNode*			pLeft;
	sBsaeNode*			pRight;
	unsigned long		dwRefCount;

	sContainerAE*		pContainer;
};

class cBinSearchAE
{	
	sBsaeNode*				mpRootNode;

	sBsaeNode*				mpNodePool;
	char*					mpContainerPool;

	sBsaeNode**				mppNodeTable;
	sContainerAE**			mppContainerTable;

	unsigned long			mReservedNodeNum;
	unsigned long			mReservedItemNum;

	unsigned long			mItemNum;
	unsigned long			mMaxItemNum;
	unsigned long			mContainerSize;

	sBsaeNode*				AllocNode();
	void					FreeNode( sBsaeNode* pNode );
	sContainerAE*			AllocContainer();
	void					FreeContaiener( sContainerAE* pContainer );
	sBsaeNode*				SearchNode( unsigned long dwKey );
	void					DeleteNode( sBsaeNode* pDel );

public:
	unsigned long 			GetItemNum() {return mItemNum;}
	bool					Initialize( unsigned long dwMaxItemNum );
	sContainerAE*			InsertItem( unsigned long dwKey,void* pItem );
	bool					DeleteItem( sContainerAE* pItemContainer );
	void*					SearchItem( unsigned long dwKey );
	sContainerAE*			SearchContainer( unsigned long dwKey );

	cBinSearchAE();
	~cBinSearchAE();
};