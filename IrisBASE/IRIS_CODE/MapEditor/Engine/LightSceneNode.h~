/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.23
* 내    용 : 조명 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"

class cTerrainLeafNode;

/// 조명 장면 노드 인자
class cLightSceneNodeParam : public cSceneNodeParam
{
public:
	cLightSceneNodeParam();

public:
	/// 반지름
	float mRadius;

	/// 주변색
	NiColor mAmbient;

	/// 산란색
	NiColor mDiffuse0;
	NiColor mDiffuse1;
	float mDiffuseAnimTime;
	bool mFlickering;

	/// 반사색
	NiColor mSpecular;

	/// 감쇠 계수
	float mConstantAtten0;
	float mConstantAtten1;
	float mConstantAttenAnimTime;
	float mLinearAtten;
	float mQuadricAtten;
};

inline
cLightSceneNodeParam::cLightSceneNodeParam()
: mRadius( 300.0f )
, mAmbient( NiColor::BLACK )
, mDiffuse0( NiColor::WHITE )
, mDiffuse1( NiColor::WHITE )
, mDiffuseAnimTime( 0.0f )
, mFlickering( false )
, mSpecular( NiColor::WHITE )
, mConstantAtten0( 0.0f )
, mConstantAtten1( 0.0f )
, mConstantAttenAnimTime( 0.0f )
, mLinearAtten( 0.0f )
, mQuadricAtten( 0.0f )
{
	mQueryType = NiPick::QUERY_FIRST;
	mPickSort = NiPick::SORT_OFF;
	mPickIntersect = NiPick::INTERSECT_BOUND;
	mPickFrontOnly = true;

}

/// 조명 장면 노드
class cLightSceneNode : public cSceneNode
{
	friend class cSceneManager;
	friend class cLightEditor;

public:
	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 픽킹
	bool Pick( const cRay& ray );

	/// 중심점
	const NiPoint3& GetCenter();

	/// 반지름
	void SetRadius( float radius );
	float GetRadius();

	/// 장면 노드를 탈부착
	void Attach( cSceneNode* node );
	void Detach( cSceneNode* node );
	void Detach( cTerrainLeafNode* node );
	void DetachAll();

	/// 주변광
	const NiColor& GetAmbient() const;

	/// 산란광
	const NiColor& GetDiffuse() const;

	/// 감쇠 계수
	float GetConstantAtten() const;
	float GetLinearAtten() const;
	float GetQuadricAtten() const;

	virtual inline NiNode* GetNiNode() { return 0; }
	virtual inline NiAVObject* GetNiObj() { return (NiAVObject*)mLight; }

protected:
	cLightSceneNode( eSceneNodeType type = SCENENODE_LIGHT );
	virtual ~cLightSceneNode();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 초기화
	virtual bool Init( const cLightSceneNodeParam& param );

	/// 주변광
	void SetAmbient( const NiColor& color );

	/// 산란광
	void SetDiffuse0( const NiColor& color );
	void SetDiffuse1( const NiColor& color );
	void SetDiffuseAnimTime( float time );
	const NiColor& GetDiffuse0() const;
	const NiColor& GetDiffuse1() const;
	float GetDiffuseAnimTime() const;

	/// 반사광
	void SetSpecular( const NiColor& color );
	const NiColor& GetSpecular() const;

	/// 감쇠 계수
	void SetConstantAtten0( float value );
	void SetConstantAtten1( float value );
	void SetConstantAttenAnimTime( float time );
	void SetLinearAtten( float value );
	void SetQuadricAtten( float value );
	float GetConstantAtten0() const;
	float GetConstantAtten1() const;
	float GetConstantAttenAnimTime() const;

protected:
	/// 엔진 노드
	NiPointLightPtr mLight;

	/// 반지름
	float mRadius;

	/// 산란광
	NiColor mDiffuse0;
	NiColor mDiffuse1;
	float mDiffuseAnimTime;
	float mDiffuseTime;
	bool mDiffuseDir;
	bool mFlickering;

	/// 감쇠
	float mConstantAtten0;
	float mConstantAtten1;
	float mConstantAttenAnimTime;
	float mConstantAttenTime;
	bool mConstantAttenDir;

	/// 영향받는 노드 셑
	typedef tHashSet<cSceneNode*> cAffectedSet;
	cAffectedSet mAffectedSet;

	/// 지형 노드 배열
	typedef tArray<cTerrainLeafNode*> cTerrainArray;
	cTerrainArray mTerrainArray;
};

inline
const NiPoint3& cLightSceneNode::GetCenter()
{
	return mLight->GetWorldTranslate();
}

inline
void cLightSceneNode::SetRadius( float radius )
{
	mNeedUpdateTransform = true;
	mRadius = radius;
}

inline
float cLightSceneNode::GetRadius()
{
	return mRadius;
}

inline
const NiColor& cLightSceneNode::GetAmbient() const
{
	return mLight->GetAmbientColor();
}

inline
const NiColor& cLightSceneNode::GetDiffuse() const
{
	return mLight->GetDiffuseColor();
}

inline
const NiColor& cLightSceneNode::GetDiffuse0() const
{
	return mDiffuse0;
}

inline
const NiColor& cLightSceneNode::GetDiffuse1() const
{
	return mDiffuse1;
}

inline
float cLightSceneNode::GetDiffuseAnimTime() const
{
	return mDiffuseAnimTime;
}

inline
const NiColor& cLightSceneNode::GetSpecular() const
{
	return mLight->GetSpecularColor();
}

inline
float cLightSceneNode::GetConstantAtten() const
{
	return mLight->GetConstantAttenuation();
}

inline
float cLightSceneNode::GetConstantAtten0() const
{
	return mConstantAtten0;
}

inline
float cLightSceneNode::GetConstantAtten1() const
{
	return mConstantAtten1;
}

inline
float cLightSceneNode::GetConstantAttenAnimTime() const
{
	return mConstantAttenAnimTime;
}

inline
float cLightSceneNode::GetLinearAtten() const
{
	return mLight->GetLinearAttenuation();
}

inline
float cLightSceneNode::GetQuadricAtten() const
{
	return mLight->GetQuadraticAttenuation();
}
