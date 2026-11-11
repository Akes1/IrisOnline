/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 선택 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cStaticSceneNode;

/// 오브젝트 선택 정보
class cObjectSelectingInfo
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
	cObjectSelectingInfo();
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
cObjectSelectingInfo::cObjectSelectingInfo()
: mTranslate( NiPoint3::ZERO )
, mRotate( NiMatrix3::IDENTITY )
, mScale( 1.0f )
, mNumObjects( 0 )
, mObjectInfos( 0 )
{
}

inline
void cObjectSelectingInfo::Clear()
{
	delete [] mObjectInfos;
}

/// 오브젝트 선택
class cObjectSelecting : public cDoing
{
	friend class cDoingManager;
	friend class cObjectGroup;

private:
	cObjectSelecting( const cObjectSelectingInfo& undo, const cObjectSelectingInfo& redo );
	~cObjectSelecting();

	void Undo();
	void Redo();

private:
	/// Undo 정보
	cObjectSelectingInfo mUndo;

	/// Redo 정보
	cObjectSelectingInfo mRedo;
};
