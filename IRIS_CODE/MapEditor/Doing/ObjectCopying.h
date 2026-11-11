/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 복사 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cStaticSceneNode;

/// 오브젝트 복사 정보
class cObjectCopyingInfo
{
public:
	cObjectCopyingInfo();

public:
	/// 오브젝트 수
	unsigned int mNumObjects;

	/// 장면 노드 배열
	cStaticSceneNode** mNodes;
};

inline
cObjectCopyingInfo::cObjectCopyingInfo()
: mNumObjects( 0 )
, mNodes( 0 )
{
}

/// 오브젝트 복사
class cObjectCopying : public cDoing
{
	friend class cDoingManager;
	friend class cObjectEditor;

private:
	cObjectCopying( const cObjectCopyingInfo& undo );
	~cObjectCopying();

	void Undo();
	void Redo();

private:
	/// 오브젝트 수
	unsigned int mNumObjects;

	/// 장면 노드 배열
	cStaticSceneNode** mNodes;
};
