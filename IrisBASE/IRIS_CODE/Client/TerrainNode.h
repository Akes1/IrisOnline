/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 지형 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Box.h"
#include "Terrain.h"

class cLightSceneNode;
class cTerrainBranchNode;
class cTerrainLeafNode;
class cTerrainTexture;
class cTerrainBuffer;

/// 지형 노드
class cTerrainNode
{
public:
	cTerrainNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cTerrainNode();

	/// 로딩
	virtual bool Load( cFileLoader& loader ) = 0;
	virtual void AddToLoadArray( cTerrainNode** loadArray, unsigned int& count ) = 0;

	/// 컬링
	virtual void Cull() = 0;
	virtual void AddToVisibleArray() = 0;

	/// 충돌 검사
	virtual bool CollideSphere() = 0;

	/// 경계 상자
	const cBox& GetBoundBox() const;

	/// 중점
	const NiPoint3& GetCenter() const;

public:
	/// 경계
	cBox mBoundBox;
	NiPoint3 mCenter;
	float mRadius;

protected:
	/// 컬링용
	static NiFrustumPlanes* mFrustum;
	static tArray<void*>* mVisibleArray;

	/// 충돌 검사용
	static const cSphere* mSphere;
	static tArray<void*>* mPickedArray;
};

inline
const cBox& cTerrainNode::GetBoundBox() const
{
	return mBoundBox;
}

inline
const NiPoint3& cTerrainNode::GetCenter() const
{
	return mCenter;
}

/// 지형 가지 노드
class cTerrainBranchNode : public cTerrainNode
{
public:
	cTerrainBranchNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cTerrainBranchNode();

	/// 로딩
	bool Load( cFileLoader& loader );
	void AddToLoadArray( cTerrainNode** loadArray, unsigned int& count );

	/// 컬링
	void Cull();
	void AddToVisibleArray();

	/// 충돌 검사
	bool CollideSphere();

public:
	/// 자식 배열
	cTerrainNode* mChild[4];
};

/// 지형 리프 노드
class cTerrainLeafNode : public cTerrainNode
{
	friend class cTerrain;
	friend class cTerrainBranchNode;
	friend class cTerrainNodeCompareForRendering;
	friend class cLightSceneNode;

public:
	cTerrainLeafNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cTerrainLeafNode();

	/// 지움
	void Clear();

	/// 로딩
	bool Load( cFileLoader& loader );
	void AddToLoadArray( cTerrainNode** loadArray, unsigned int& count );

	/// 컬링
	void Cull();
	void AddToVisibleArray();

	/// 충돌 검사
	bool CollideSphere();

	/// 조명
	void SetLight( cLightSceneNode* light, unsigned int indexByLight );
	const cLightSceneNode* GetLight() const;

private:
	/// 텍스처
	void SetTextures( cTerrainTexture* tex0, cTerrainTexture* tex1, cTerrainTexture* tex2 );
	void CalcTextureValue();

	void UpdateLODInfo();

private:
	/// 좌표 (지형 그리드 상의 위치)
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// 가시 여부
	bool mVisible;

	/// 버퍼
	cTerrainBuffer* mBuffer;

	/// 디테일 텍스처
	cTerrainTexture* mTexture0;
	cTerrainTexture* mTexture1;
	cTerrainTexture* mTexture2;
	unsigned int mTextureValue0;
	unsigned int mTextureValue1;

	/// 조명
	cLightSceneNode* mLight;
	unsigned int mIndexByLight;

	/// Level of detail
	unsigned int mLod;
	unsigned int mCrackIndex;

	cTerrainLeafNode* mSiblingLeafNode[4];

private:
	/// 버전
	static unsigned int mVersion;

	/// 리프 노드의 그리드 크기
//	static unsigned int mCellCount[TERRAIN_LOD_COUNT];
//	static unsigned int mLineCount[TERRAIN_LOD_COUNT];
};

inline
const cLightSceneNode* cTerrainLeafNode::GetLight() const
{
	return mLight;
}

/// 지형 노드 비교 함수자
class cTerrainNodeCompareForRendering
{
public:
	bool operator () ( const void* left, const void* right ) const
	{
		cTerrainLeafNode* lnode = (cTerrainLeafNode*)left;
		cTerrainLeafNode* rnode = (cTerrainLeafNode*)right;
		cTerrainBuffer* lbuff = lnode->mBuffer;
		cTerrainBuffer* rbuff = rnode->mBuffer;

		if( lbuff == rbuff )
		{
			unsigned int ltex = lnode->mTextureValue0;
			unsigned int rtex = rnode->mTextureValue0;

			if( ltex == rtex )
			{
				ltex = lnode->mTextureValue1;
				rtex = rnode->mTextureValue1;

				if( ltex == rtex )
				{
					if( lnode->mYIndex == rnode->mYIndex )
					{
						return lnode->mXIndex < rnode->mXIndex;
					}
					return lnode->mYIndex < rnode->mYIndex;
				}
				return ltex < rtex;
			}
			return ltex < rtex;
		}
		return lbuff < rbuff;
	};
};
