/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 삭제 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cStaticSceneNode;

/// 오브젝트 삭제 정보
class cObjectDeletingInfo
{
public:
	/// 오브젝트 정보
	class cObjectInfo
	{
	public:
		/// 장면 노드
		cStaticSceneNode* mNode;

		/// 변환
		NiPoint3 mTranslate;
		NiMatrix3 mRotate;
		float mScale;
	};

public:
	cObjectDeletingInfo();
	void Clear();

public:
	/// 그룹 오브젝트 정보
	NiPoint3 mTranslate;
	NiMatrix3 mRotate;
	float mScale;

	/// 오브젝트 수
	unsigned int mNumObjects;

	/// 오브젝트 정보 배열
	cObjectInfo* mObjectInfos;
};

inline
cObjectDeletingInfo::cObjectDeletingInfo()
: mTranslate( NiPoint3::ZERO )
, mRotate( NiMatrix3::IDENTITY )
, mScale( 1.0f )
, mNumObjects( 0 )
, mObjectInfos( 0 )
{
}

/// 오브젝트 삭제
class cObjectDeleting : public cDoing
{
	friend class cDoingManager;
	friend class cObjectGroup;

private:
	cObjectDeleting( const cObjectDeletingInfo& undo, const cObjectDeletingInfo& redo );
	~cObjectDeleting();

	void Undo();
	void Redo();

private:
	/// Undo 정보
	cObjectDeletingInfo mUndo;

	/// Redo 정보
	cObjectDeletingInfo mRedo;
};
