#pragma once

struct Patchfile;

class CNode
{
public:
	CNode *m_pParent;
	CNode *m_pLeftChild;
	CNode *m_pRightChild;

	CNode *m_pNext_node;
	CNode *m_pPrev_node;

	char  *m_pcKey;        //데이터 키값

	Patchfile *m_psPatchfile; //데이터

public:
	CNode(void);
	~CNode(void);
};


class CBSTreeLinkedList
{
public:

	CNode *m_pLinked_List_Head, *m_pLinked_List_Tail;
	CNode *m_pBSTreeRoot;

public:
	CBSTreeLinkedList(void);
	~CBSTreeLinkedList(void);

	bool Add_Node(Patchfile* input_data);	//노드 삽입
	bool Delete_linked_list(CNode *header); //링크드 리스트의 모든 데이터 지우기
	Patchfile* Search_Node( char *key, bool msg_off = false); //노드 찾기.
	bool ReleaseMemory(); //메모리 해제
};
