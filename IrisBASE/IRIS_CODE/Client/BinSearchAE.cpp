#include "stdafx.h"
#include "BinSearchAE.h"

cBinSearchAE::cBinSearchAE()
{
	memset(this,0,sizeof(cBinSearchAE));
}

cBinSearchAE::~cBinSearchAE()
{
	SAFE_DELETE_ARRAY(mppContainerTable);
	SAFE_DELETE_ARRAY(mppNodeTable);
	SAFE_DELETE_ARRAY(mpContainerPool);
	SAFE_DELETE_ARRAY(mpNodePool);
}

bool cBinSearchAE::Initialize( unsigned long dwMaxItemNum )
{

	mMaxItemNum = dwMaxItemNum;
	mContainerSize = sizeof(sContainerAE) - sizeof(unsigned long) + sizeof(unsigned long);

	mMaxItemNum = dwMaxItemNum;

	mpNodePool = new sBsaeNode[mMaxItemNum];
	memset(mpNodePool,0,sizeof(sBsaeNode)*mMaxItemNum);

	mpContainerPool = new char[mContainerSize*mMaxItemNum];
	memset(mpContainerPool,0,mContainerSize*mMaxItemNum);

	mppNodeTable = new sBsaeNode*[mMaxItemNum];
	memset(mppNodeTable,0,sizeof(sBsaeNode*)*mMaxItemNum);

	mppContainerTable = new sContainerAE*[mMaxItemNum];
	memset(mppContainerTable,0,sizeof(sContainerAE*)*mMaxItemNum);

	for (unsigned long i=0; i<mMaxItemNum; i++)
	{
		mppNodeTable[i] = mpNodePool+i;
		mppContainerTable[i] = (sContainerAE*)(mpContainerPool + mContainerSize*i);
	}
	mReservedNodeNum = mMaxItemNum;
	mReservedItemNum = mMaxItemNum;

	return true;
}
sBsaeNode* cBinSearchAE::AllocNode()
{
	sBsaeNode*	pNode;

	pNode = mppNodeTable[0];
	mppNodeTable[0] = mppNodeTable[mReservedNodeNum-1];
	mReservedNodeNum--;

	pNode->dwRefCount = 0;
	pNode->pParent = NULL;
	pNode->pLeft = NULL;
	pNode->pRight = NULL;
	pNode->dwRefCount = 0;
	pNode->pContainer = NULL;

	return pNode;

}
void cBinSearchAE::FreeNode(sBsaeNode* pNode)
{

	mppNodeTable[mReservedNodeNum] = pNode;
	pNode->dwKey = 0xcccccccc;
	mReservedNodeNum++;
}
sContainerAE* cBinSearchAE::AllocContainer()
{

	sContainerAE*	pContainer;

	pContainer = mppContainerTable[0];
	mppContainerTable[0] = mppContainerTable[mReservedItemNum-1];
	mReservedItemNum--;

	pContainer->pOwner = NULL;
	pContainer->pPrv = NULL;
	pContainer->pNext = NULL;
	pContainer->pItem = NULL;


	return pContainer;

}
void cBinSearchAE::FreeContaiener(sContainerAE* pContainer)
{
	mppContainerTable[mReservedItemNum] = pContainer;
	pContainer->pItem = NULL;
	pContainer->pOwner = NULL;
	mReservedItemNum++;
}


sContainerAE*	cBinSearchAE::InsertItem(unsigned long dwKey,void* pItem)
{
	sBsaeNode*			pParent;
	sBsaeNode*			pCur = NULL;
	sContainerAE*	pNewContainer = NULL;

	//	IsExist(pItem);

	if ( !mpRootNode )
	{
		mpRootNode = pCur = AllocNode();
		goto lb_true;
	}
	pParent = mpRootNode;

lb_check_left:

	if (dwKey == pParent->dwKey)
	{
		pCur = pParent;

		goto lb_true;
	}
	if (dwKey < pParent->dwKey)
	{
		if (!pParent->pLeft)
		{
			pCur = AllocNode();
			pParent->pLeft = pCur;
			pCur->pParent = pParent;
			goto lb_true;
		}
		else
		{
			pParent = pParent->pLeft;
			goto lb_check_left;
		}
	}

//lb_check_right:
	if (dwKey > pParent->dwKey)
	{
		if (!pParent->pRight)
		{
			pCur = AllocNode();
			pParent->pRight = pCur;
			pCur->pParent = pParent;
		}
		else
		{
			pParent = pParent->pRight;
			goto lb_check_left;
		}
	}

lb_true:


	pCur->dwKey = dwKey;
	pNewContainer = AllocContainer();
	if (!pNewContainer)
		goto lb_false;

	pNewContainer->dwKey = dwKey;

	pNewContainer->pItem = pItem;	// 아이템
	pNewContainer->pOwner = pCur;	// 이 아이템을 소유하고 있는 노드 포인터.

	if (pCur->pContainer)
	{
		pNewContainer->pNext = pCur->pContainer;
		pCur->pContainer->pPrv = pNewContainer;
		pCur->pContainer = pNewContainer;
	}
	else
	{
		pCur->pContainer = pNewContainer;
	}
	pCur->dwRefCount++;


	mItemNum++;


lb_false:
	if (!pNewContainer)
		__asm int 3
		return pNewContainer;
}


void* cBinSearchAE::SearchItem(unsigned long dwKey)
{
	void*	pItem = NULL;

	sBsaeNode*	pNode = SearchNode(dwKey);
	if (!pNode)
		goto lb_return;

	pItem = pNode->pContainer->pItem;

	/*__asm
	{
		mov		esi,dword ptr[pItem]
		mov		eax,dword ptr[esi];
		mov		dword ptr[dwSize],eax
	}
	if (dwSize != pNode->pContainer->dwKey)
		__asm int 3*/
lb_return:
	return pItem;

}
sContainerAE*	cBinSearchAE::SearchContainer(unsigned long dwKey)
{
	sContainerAE*	pItem = NULL;

	sBsaeNode*	pNode = SearchNode(dwKey);
	if (!pNode)
		goto lb_return;

	pItem = pNode->pContainer;

lb_return:
	return pItem;
}
sBsaeNode* cBinSearchAE::SearchNode(unsigned long dwKey)
{

	sBsaeNode*	pParent = mpRootNode;
	sBsaeNode*	pResult = NULL;

	// 같거나 크면 ok
lb_search_left:

	if (!pParent)
		goto lb_return;



	if (pParent->dwKey == dwKey)
	{

		pResult = pParent;
		goto lb_return;
	}
	if (dwKey < pParent->dwKey)
	{

		pResult = pParent;
		pParent = pParent->pLeft;
		goto lb_search_left;
	}

	if (dwKey > pParent->dwKey)
	{
		// 이전에 왼쪽으로 진행했었고 현재 비교값보다 값이 클 경우는 
		// 이전의 노드를 얻어야한다.

		pParent = pParent->pRight;
		goto lb_search_left;
	}

lb_return:
	return pResult;
}

bool cBinSearchAE::DeleteItem(sContainerAE*	pContainer)
{
#ifdef _DEBUG
	if (!mItemNum)
		__asm int 3
#endif

//	void* pItem = pContainer->pItem;

	bool bResult = false;
	sBsaeNode*	pNode = pContainer->pOwner;

	sContainerAE*		pCur = pContainer;

	sContainerAE*		pNext;
	sContainerAE*		pPrv;
	pNext = pCur->pNext;
	pPrv = pCur->pPrv;

	if (pPrv)
		pPrv->pNext = pNext;
	else
	{
		// 이게 맨 처음이다.
		pNode->pContainer = pNext;
	}

	if (pNext)
		pNext->pPrv = pPrv;


	FreeContaiener(pCur);
	//	if (!pNode->dwRefCount)
	//		__asm int 3

	pNode->dwRefCount--;
	if(!pNode->dwRefCount)
	{
		DeleteNode(pNode);

	}
	mItemNum--;
	bResult = true;
	//	IsExist(pItem);

	return bResult;
}

void cBinSearchAE::DeleteNode(sBsaeNode* pDel)
{
	if (!mpRootNode)
		return;

	sBsaeNode*		pTemp;
	sBsaeNode*		pNewChild;
	sBsaeNode*		pNewParent;
	sBsaeNode*		pParent;

	pParent = pDel->pParent;
	unsigned long	dwKey = pDel->dwKey;

	if (!pDel->pRight)
	{
		// 삭제할 노드의 오른쪽 자식이 없는 경우.
		// pDel->pLeft로 pDel의 위치를 대치한다.
		pNewChild = pNewParent = pDel->pLeft; 
		if (pNewChild)
			pNewChild->pParent = pParent;	


		goto lb_return;
	}

	if (!pDel->pRight->pLeft)
	{
		// 삭제할 노드의 오른쪽 자식의 왼쪽 자식이 없는 경우
		pNewChild = pNewParent = pDel->pRight;
		pNewParent->pLeft = pDel->pLeft;	// 새로운 부모노드의 왼쪽 자식 노드

		if (pNewParent->pLeft)
			pNewParent->pLeft->pParent = pNewParent;	

		pNewChild->pParent = pParent;

		goto lb_return;
	}

	// 그 외..
	pTemp = pDel->pRight;

lb_seek_left_end:
	if (pTemp->pLeft)
	{
		pTemp = pTemp->pLeft;
		goto lb_seek_left_end;
	}
	pNewParent = pTemp;					// 삭제할 노드의 왼쪽 자식에 대해 새로운 부모가 될 노드.

	pNewChild = pDel->pRight;						// 새로운 자식 노드가 될 노드 
	pNewChild->pParent = pParent;

	pNewParent->pLeft = pDel->pLeft;	// 새로운 부모 노드의 왼쪽 자식 노드 세팅

	if (pNewParent->pLeft)
		pNewParent->pLeft->pParent = pNewParent;	// 삭제할 노드의 왼쪽 자식의 새로운 부모노드를 세팅 


lb_return:
	if (pParent)
	{
		if (dwKey < pParent->dwKey)
			pParent->pLeft = pNewChild;
		else
			pParent->pRight = pNewChild;
	}

	if (pDel == mpRootNode)
		mpRootNode = pNewChild;

	FreeNode(pDel);
}

/*
BOOL cBinSearchAE::IsExist(void* pItem)
{
	BOOL	bResult = FALSE;


	for (unsigned long i=0; i<mMaxItemNum; i++)
	{
		if (m_ppItemContainerTable[i]->pItem == pItem)
		{
			__asm int 3
				bResult = TRUE;
			goto lb_return;
		}

	}

lb_return:
	return bResult;
}
*/