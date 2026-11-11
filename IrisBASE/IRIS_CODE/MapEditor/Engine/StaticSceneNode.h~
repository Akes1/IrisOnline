/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 정적 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"

class cNifAnimationInfo;

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
};

inline
cStaticSceneNodeParam::cStaticSceneNodeParam()
: mFogApplied( true )
, mMaterialApplied( false )
, mVisibleLevel( 0 )
, mOccludeLevel( 1 )
{
	mPickFrontOnly = true;
}

/// 정적 장면 노드
/// 처음에 좌표를 설정하고 절대 이동하지 않는다.
/// 제자리에서 반복 애니메이션을 할 수는 있다.
/// 재질, 가시 레벨, 차폐 레벨 등의 정보를 가진다.
class cStaticSceneNode : public cSceneNode
{
	friend class cSceneManager;
	friend class cObjectEditor;

public:
	/// 처리
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );
	virtual void ProcessVisibleLevel( unsigned long deltaTime );

	/// 재질
	void SetMaterialApplied( bool matApplied );
	bool GetMaterialApplied() const;

	void SetMaterial( const cMaterialData& material );
	const cMaterialData& GetMaterial() const;

	/// 주변광 반사율
	void SetAmbient( const NiColor& color );
	const NiColor& GetAmbient() const;

	/// 산란광 반사율
	void SetDiffuse( const NiColor& color );
	const NiColor& GetDiffuse() const;

	/// 발산색
	void SetEmissive( const NiColor& color );
	const NiColor& GetEmissive() const;

	/// 가시 레벨
	void SetVisibleLevel( unsigned int level );
	unsigned int GetVisibleLevel() const;

	/// 차폐 레벨
	void SetOccludeLevel( unsigned int );
	unsigned int GetOccludeLevel() const;

	/// 안개 적용
	void SetFogApplied( bool fogApplied );
	bool GetFogApplied() const;

	/// 삼각형 수
	unsigned int GetNumVerts() const;

	/// 애니메이션 가능 여부
	bool IsAnimated() const;

#ifdef MAP_EDITOR
	void SetWirframe( bool set ) { mWireProp->SetWireframe( set ); }
#endif

protected:
	cStaticSceneNode( eSceneNodeType type = SCENENODE_STATIC );
	virtual ~cStaticSceneNode();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 초기화
	virtual bool Init( cStaticSceneNodeParam& param );

	/// 
	void RestoreMaterial();

	/// 가시 레벨
	unsigned int CalcVisibleLevel();

protected:
	/// 애니 컨트롤러의 정보
	cNifAnimationInfo* mNifAniInfo;

	/// 정점 수
	unsigned int mNumVerts;

	/// 안개
	bool mFogApplied;

	/// 재질
	bool mMaterialApplied;
	cMaterialData mMaterial;

	/// 가시 레벨
	unsigned int mVisibleLevel;

	/// 차폐 레벨
	unsigned int mOccludeLevel;

	/// 카메라에 걸림 여부
	bool mPickedByCamera;

	cString mLightMapFileName;

#ifdef MAP_EDITOR
	NiWireframeProperty* mWireProp;
#endif
};

inline
unsigned int cStaticSceneNode::GetNumVerts() const
{
	return mNumVerts;
}

inline
const cMaterialData& cStaticSceneNode::GetMaterial() const
{
	return mMaterial;
}

inline
const NiColor& cStaticSceneNode::GetAmbient() const
{
	return mMaterial.mAmbient;
}

inline
const NiColor& cStaticSceneNode::GetDiffuse() const
{
	return mMaterial.mDiffuse;
}

inline
const NiColor& cStaticSceneNode::GetEmissive() const
{
	return mMaterial.mEmissive;
}

inline
unsigned int cStaticSceneNode::GetVisibleLevel() const
{
	return mVisibleLevel;
}

inline
unsigned int cStaticSceneNode::GetOccludeLevel() const
{
	return mOccludeLevel;
}

inline
bool cStaticSceneNode::GetFogApplied() const
{
	return mFogApplied;
}

inline
bool cStaticSceneNode::GetMaterialApplied() const
{
	return mMaterialApplied;
}
