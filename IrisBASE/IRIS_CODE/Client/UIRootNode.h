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
	/// 자식 윈도우들을 숨김
	void HideAllChild( cUIWindow* except );

	/// 숨긴 자식 윈도우들을 보임
	void ShowAllChild();

	void AddProcessChild( cUINode* n );
	void RemoveProcessChild( cUINode* n );

	// 루트 차일드 검색용
	void AddSearchChild( cUINode* n );
	void RemoveSearchChild( unsigned long id );
	cUINode* FindNodeByID( unsigned long id );	// 루트의 자식에서만 검색된다

private:
	cUIRootNode( eUINodeType type = eUINODE_ROOT );
	virtual ~cUIRootNode();

	/// 생성
	/// 아무 일도 하지 않는다. 디버그 모드에서는 assert를 발생한다.
	virtual bool Create( const cString& skinName, cUIContainer* pparent, unsigned int id, bool bTransform = false);

	/// 맨 앞창부터 닫기 ( false : 더이상 닫을창이 없음 )
	bool HideESC();

	// stage 변경시점
	void Open();
	void Close();

	/// 갱신
	/// 아무 일도 하지 않는다. 디버그 모드에서는 assert를 발생한다.
	void UpdateRect();

	/// 해상도 변경에 따른 객체의 스킨 재적용
	void UpdateSkin();

	/// 스킨을 설정
	/// 아무 일도 하지 않는다. 무조건 true를 리턴한다.
	bool SetSkin( const cUINodeSkin* pskin );

	/// 적합 여부를 리턴
	/// 아무 일도 하지 않는다. 무조건 true를 리턴한다.
	bool IsValid() const;

	/// 이벤트를 처리
	/// 아무 일도 하지 않는다. 무조건 true를 리턴한다.
	bool HandleEvent( const cUIEvent& event );

	/// 렌더링
	void OnPreRender();
	void OnRender( cUIFontItemKeeper* pKeeper = 0 );
	void OnRenderTip();

	/// 업데이트
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	/// 모든 UI Lock 풀기
	void NetUnLockAllUI();

	//// 마우스 좌표값에 있는 윈도우 구하기 
	cUIContainer* GetContainer( const cUIPos& pos ) const;

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

