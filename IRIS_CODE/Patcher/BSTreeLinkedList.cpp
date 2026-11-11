#include "StdAfx.h"
#include "bstreelinkedlist.h"
#include "Patcher.h"

CNode::CNode(void)
{
	m_pParent     = NULL;
	m_pLeftChild  = NULL;
	m_pRightChild = NULL;

	m_pNext_node  = NULL;
	m_pPrev_node  = NULL;

	m_pcKey		  = NULL;    //데이터 키값
	
	m_psPatchfile = new Patchfile;
	memset( m_psPatchfile->fileName, 0, sizeof( m_psPatchfile->fileName));
	//m_psPatchfile = NULL;
}



CNode::~CNode(void)
{
	m_pParent     = NULL;
	m_pLeftChild  = NULL;
	m_pRightChild = NULL;

	m_pNext_node  = NULL;
	m_pPrev_node  = NULL;

	m_pcKey = NULL;        //데이터 키값
	
	if( m_psPatchfile != NULL )
		delete m_psPatchfile;
	
	m_psPatchfile = NULL;

}


CBSTreeLinkedList::CBSTreeLinkedList(void)
{
	m_pLinked_List_Head = NULL;
	m_pLinked_List_Tail = NULL;
    m_pBSTreeRoot		= NULL;
}



CBSTreeLinkedList::~CBSTreeLinkedList(void)
{
	if( m_pLinked_List_Head != NULL)
		if( false == Delete_linked_list(m_pLinked_List_Head) )
		{
			assert(0);
			printf("error:링크드 리스트 삭제에 실패하였습니다.\n");
		}
	m_pLinked_List_Head = NULL;
	m_pLinked_List_Tail = NULL;
	m_pBSTreeRoot		= NULL;

}

bool CBSTreeLinkedList::ReleaseMemory()
{
	if( m_pLinked_List_Head != NULL)
		if( false == Delete_linked_list(m_pLinked_List_Head) )
		{
			assert(0);
			printf("error:링크드 리스트 삭제에 실패하였습니다.\n");
			return false;
		}
	m_pLinked_List_Head = NULL;
	m_pLinked_List_Tail = NULL;
	m_pBSTreeRoot		= NULL;

	return true;
}

bool CBSTreeLinkedList::Add_Node(Patchfile* input_data)
{
	if( strlen( input_data->fileName ) == 0 )
	{
		assert(0);
		return false;
	}
	else
	{
		//--------트리에 추가--------
		CNode  *pInsert_pos_finder = NULL, *pInsert_pos_parent = NULL;
		CNode  *new_node = NULL;

		pInsert_pos_finder = m_pBSTreeRoot; //root부터 탐색

		//트리에 들어갈 위치 찾기.
		while( pInsert_pos_finder != NULL )
		{
			int ret= stricmp( pInsert_pos_finder->m_pcKey, input_data->fileName );

			pInsert_pos_parent = pInsert_pos_finder; //부모 노드 셋팅

			//추가할 데이터가 들어갈 노드 위치를 찾기 위해 값 비교
			if( ret < 0 )
			{
				pInsert_pos_finder = pInsert_pos_finder->m_pRightChild; //값이 더 크다면 오른쪽 노드로~
			}
			else if( ret > 0 )
			{
				pInsert_pos_finder = pInsert_pos_finder->m_pLeftChild; //값이 더 작다면 왼쪽 노드로
			}
			else /*if( pInsert_pos_finder->m_pcKey == m_pcKey )*///값이 같다면 중복된 값이 들어왔으므로 에러 발생
			{
				// assert(0);
				return false;
			}
		}

		//새롭게 추가할 수 있는 조건을 만족했으므로 메모리 할당
		new_node = new CNode;

		if(new_node == NULL)
		{
			printf("error:새로운 노드 메모리 할당 실패로 노드 추가 실패.\n");
			return false;
		}
		
		new_node->m_psPatchfile->sizeHigh = input_data->sizeHigh;
		new_node->m_psPatchfile->sizeLow = input_data->sizeLow;
		new_node->m_psPatchfile->writeTime = input_data->writeTime;
		strncpy( new_node->m_psPatchfile->fileName, input_data->fileName, strlen( input_data->fileName ) );
        		
		new_node->m_pcKey = new_node->m_psPatchfile->fileName;

		//트리에 들어갈 위치에 삽입.
		if( m_pBSTreeRoot == pInsert_pos_finder ) //비어있는 트리에 삽입
		{
			m_pBSTreeRoot = new_node;
		}
		else if( stricmp( new_node->m_pcKey, pInsert_pos_parent->m_pcKey ) > 0 ) //부모노드보다 클 경우 오른쪽 노드로 셋팅
		{
			pInsert_pos_parent->m_pRightChild = new_node;
			new_node->m_pParent = pInsert_pos_parent;
		}
		else /*if( strcmp( new_node->m_pcKey, pInsert_pos_parent->m_pcKey ) < 0 )*/ //부모노드보다 작을 경우 왼쪽 노드로 셋팅
		{
			pInsert_pos_parent->m_pLeftChild = new_node;
			new_node->m_pParent = pInsert_pos_parent;
		}

		//--------링크드 리스트에 추가--------
	
		//비어있는 링크드 리스트에 추가.
		if(m_pLinked_List_Head == NULL)
		{
			m_pLinked_List_Head = new_node;
			m_pLinked_List_Tail = new_node;
		}
		else //비어있지 않은 리스트에 추가.
		{
			m_pLinked_List_Tail->m_pNext_node = new_node;
			new_node->m_pPrev_node = m_pLinked_List_Tail;
			m_pLinked_List_Tail = m_pLinked_List_Tail->m_pNext_node;
		}

		return true;
	}

	return false;
}


bool CBSTreeLinkedList::Delete_linked_list(CNode *header) //링크드 리스트의 모든 데이터 지우기
{
	if(header->m_pPrev_node != NULL)
	{
		assert(0);
		//printf("error:삭제실패! 링크드 리스트의 헤더가 아닙니다.\n");
		return false;
	}
	
	CNode *pdel_node = NULL;

	while( header != NULL) //모든 데이터를 지울때까지
	{
		pdel_node = header;
		header = header->m_pNext_node;
		delete pdel_node;
	}
	return true;
}



Patchfile* CBSTreeLinkedList::Search_Node(char *key, bool msg_off) //노드 찾기.
{

	CNode *pNode = m_pBSTreeRoot;

	if(pNode == NULL)
	{
		if(msg_off == false)
			printf("error:트리가 비어있습니다\n");

		return NULL;
	}

	if( strlen( key ) == 0 ) //키가 잘못되었다면 검색 실패
	{
		if(msg_off == false)
			printf("error:잘못된 데이터를 이용하여 검색하셨습니다.\n");

		assert(0);
		return NULL;
	}

	while(pNode != NULL)
	{
		int ret;
		
		ret = stricmp( pNode->m_pcKey,  key);
		
		//추가할 데이터가 들어갈 노드 위치를 찾기 위해 값 비교
		if( ret < 0 )
		{
			pNode = pNode->m_pRightChild; //값이 더 크다면 오른쪽 노드로~
		}
		else if( ret > 0)
		{
			pNode = pNode->m_pLeftChild; //값이 더 작다면 왼쪽 노드로
		}
		else /*if( pNode->m_pcKey == key_num )*/ //값이 같다면 검색 성공
		{
			return pNode->m_psPatchfile;
		}	
	}

	//while문을 빠져나왔다면 비어있는 곳을 찾았으므로 검색 실패
//	if(msg_off == false)
//		printf("검색실패:해당 데이터를 찾을 수 없습니다.\n");

	return NULL;

}

