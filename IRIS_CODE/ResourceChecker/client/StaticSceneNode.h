/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 정적 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"
#include "MaterialData.h"

class cNifAnimationInfo;
class cNifTimeManager;

/// 정적 장면 노드 인자
class cStaticSceneNodeParam : public cSceneNodeParam
{
public:
	cStaticSceneNodeParam();

public:
	/// 안개 적용 여부
	bool mFogApplied;

	/// 재질
	bool mMaterialApplied;
	cMaterialData mMaterial;

	/// 가시 레벨
	unsigned int mVisibleLevel;

	/// 차폐 레벨
	unsigned int mOccludeLevel;

	bool mNoAttachLight;

	bool mDisableAnimation;
};

inline
cStaticSceneNodeParam::cStaticSceneNodeParam()
: mFogApplied( true )
, mMaterialApplied( false )
, mVisibleLevel( 0 )
, mOccludeLevel( 1 )
, mDisableAnimation( false )
{
	mPickFrontOnly = false;
	mNoAttachLight = false;
}

/// 정적 장면 노드
/// 처음에 좌표를 설정하고 절대 이동하지 않는다.
/// 제자리에서 반복 애니메이션을 할 수는 있다.
/// 재질, 가시 레벨, 차폐 레벨 등의 정보를 가진다.
class cStaticSceneNode : public cSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void ProcessVisibleLevel( unsigned long delatime );

	/// 차폐 레벨
	unsigned int GetOccludeLevel() const;

	virtual bool UpdateCameraPickInfo();

protected:
	cStaticSceneNode( eType type = eSTATIC );
	virtual ~cStaticSceneNode();

	/// 초기화
	virtual bool Init( const cStaticSceneNodeParam& param );

	/// 재질
	void SetMaterial( const cMaterialData& material );

protected:
	/// 애니 컨트롤러의 정보
	cNifAnimationInfo*	mNifAniInfo;

	/// 재질
	cMaterialData mMaterial;

	/// 가시 레벨
	unsigned int mVisibleLevel;

	/// 차폐 레벨
	unsigned int mOccludeLevel;

	/// 카메라에 걸림 여부
	bool mPickedByCamera;
};

inline
unsigned int cStaticSceneNode::GetOccludeLevel() const
{
	return mOccludeLevel;
}


//////////////////////////////////////////////////////////////////////////
class cManagedStaticSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

public:
	void SetMouseOver( bool set ) { mIsMouseOver = set; }

protected:
	cManagedStaticSceneNode( eType type = eMANAGEDSTATIC );
	virtual ~cManagedStaticSceneNode();

protected:
	bool		mIsMouseOver;
};