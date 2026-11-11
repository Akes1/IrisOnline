/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 생성 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cStaticSceneNode;

/// 오브젝트 생성 정보
class cObjectCreatingInfo
{
public:
	cObjectCreatingInfo();

public:
	/// 장면 노드
	cStaticSceneNode* mNode;
};

inline
cObjectCreatingInfo::cObjectCreatingInfo()
: mNode( 0 )
{
}

/// 오브젝트 생성
class cObjectCreating : public cDoing
{
	friend class cDoingManager;
	friend class cObjectEditor;

private:
	cObjectCreating( const cObjectCreatingInfo& undo );
	~cObjectCreating();

	void Undo();
	void Redo();

private:
	/// Undo 정보
	cObjectCreatingInfo mUndo;
};
