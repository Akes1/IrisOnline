/* ==========================================================================
* 파    일 : UIRootNode.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.14
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIContainer.h"

/// UI 루트 노드
/// 모든 UI 노드들의 부모이다.
/// 오직 UI 관리자에서만 생성 및 삭제할 수 있다.
class cUIRootNode : public cUIContainer
{
	friend class cUIManager;

public:

private:
	cUIRootNode( eUINodeType type = eUINODE_ROOT );
	virtual ~cUIRootNode();

private:
	/// 폰트
	cUIFontItemKeeper* mFontItemKeeper;

	/// 팁
	cUINode* mTipWindow;

	// 루트 차일드 검색용
	typedef tPointerHashMap<unsigned long, void*> cFindChildMap;
	cFindChildMap mFindChildMap;

	/// 자식 윈도우 가시 정보
	typedef tPointerArray<void*> cChildVisArray;
	cChildVisArray mChildVisArray;

	typedef tPointerList<void*> cProcessChildList;
	cProcessChildList mProcessList;
};

