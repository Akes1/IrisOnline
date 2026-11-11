/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.15
* 내    용 : 주인공
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Engine/Sphere.h"
#include "Engine/PlayerSceneNode.h"

#include "Lines.h"

const unsigned int MAX_PATH_COUNT = 2048;

class cLookAtCamera;
class cRibbonGeometry;

/// 주인공
class cHero
{
	static cHero* mSingleton;

public:
	cHero( cLookAtCamera* cam );
	~cHero();

	void Clear();

	/// 초기화
	bool Init();

	/// 리셑
	void Reset();

	/// 처리
	void Process( unsigned long time );

	/// 렌더링
	void RenderLines();

	/// 이동
	void SetTranslate( const NiPoint3& pos );
	const NiPoint3& GetWorldTranslate() const;

	/// 회전
	const NiMatrix3& GetWorldRotate() const;

	/// 목표 지점
	void SetTargetPos( const NiPoint3& pos );

	/// 경계 구
	const cSphere& GetBoundSphere() const;
	const NiPoint3& GetCenter() const;

	/// 장면 노드
	cPlayerSceneNode* GetSceneNode() const;

protected:
	void SetActiveLines( unsigned int count );

public:
	/// 단일체를 리턴
	static cHero* GetSingleton();

private:
	/// 위치
	NiPoint3 mPos;

	/// 길찾기 결과 이동 경로
	NiPoint2 mPathArray[MAX_PATH_COUNT];
	unsigned int mPathCount;
	unsigned int mPathIndex;

	/// 길찾기 이동 경로 선
	NiMeshPtr mPathLines;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// 가시 레벨 원
	cCircleLines mLevel0Lines;
	cCircleLines mLevel1Lines;
	cCircleLines mLevel2Lines;

	/// 카메라
	cLookAtCamera* mCamera;

	/// 노드
	cPlayerSceneNode* mNode;

	/// 경계 구
	cSphere mBoundSphere;

	/// 궤적
	//LPDIRECT3DTEXTURE9 mRibbonTexture;
	//cRibbonGeometry* mRibbonGeom;

	/// 갱신 여부
	bool mNeedUpdate;
};

inline
const cSphere& cHero::GetBoundSphere() const
{
	return mBoundSphere;
}

inline
const NiPoint3& cHero::GetCenter() const
{
	if( mNode )
		return mNode->GetCenter();
	else
		return NiPoint3::ZERO;
}

inline
const NiPoint3& cHero::GetWorldTranslate() const
{
	return mNode->GetWorldTranslate();
}

inline
const NiMatrix3& cHero::GetWorldRotate() const
{
	return mNode->GetWorldRotate();
}

inline
cPlayerSceneNode* cHero::GetSceneNode() const
{
	return mNode;
}

inline
cHero* cHero::GetSingleton()
{
	return mSingleton;
}

#define HERO cHero::GetSingleton()
