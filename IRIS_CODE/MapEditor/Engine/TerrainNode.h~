/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 지형 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Box.h"
#include "Terrain.h"

class cRay;
class cSceneNode;
class cLightSceneNode;
class cTerrainBranchNode;
class cTerrainLeafNode;
class cTerrainTexture;
class cTerrainBuffer;
#ifdef MAP_EDITOR
class cTerrainNodeBuildingInfo;
class cTerrainNodeBuilding;
class cTerrainNodePaintingInfo;
class cTerrainNodePainting;
class cTerrainNodeDetailingInfo;
class cTerrainNodeDetailing;
#endif

enum eTerrainViewMode;

/// 지형 노드
class cTerrainNode
{
	friend class cTerrain;

public:
	cTerrainNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cTerrainNode();

	/// 로딩
	virtual bool Load( cFileLoader& loader ) = 0;

#ifdef MAP_EDITOR
	/// 저장
	virtual bool Save( cFileSaver& saver ) = 0;
#endif

	/// 동기화
	virtual void SyncHeight() = 0;
	virtual void SyncAlpha() = 0;
	virtual void SyncPaintAlpha() = 0;
	virtual void SyncColor() = 0;

	/// 컬링
	virtual void Cull() = 0;
	virtual void AddToVisibleArray() = 0;

	/// 충돌 검사
	virtual bool CollideRay() = 0;

	/// 구와 충돌하는 노드들을 수집
	/// 바운딩 박스로 검사한다.
	virtual bool CollideSphere() = 0;

	/// 경계 상자
	const cBox& GetBoundBox() const;

	/// 중점
	const NiPoint3& GetCenter();

protected:
	/// 경계 상자를 갱신
	virtual void UpdateBoundUpward() = 0;

public:
	/// 부모
	cTerrainBranchNode* mParent;

	/// 경계
	cBox mBoundBox;
	NiPoint3 mCenter;
	float mRadius;

protected:
	/// 컬링용
	static NiFrustumPlanes* mFrustum;
	static tArray<cTerrainLeafNode*>* mVisibleArray;

	/// 충돌 검사용
	static const cRay* mRay;
	static float* mDistance;
	static NiPoint3* mContact;
	static const cSphere* mSphere;
	static tArray<cTerrainLeafNode*>* mPickedArray;
};

inline
const cBox& cTerrainNode::GetBoundBox() const
{
	return mBoundBox;
}

inline
const NiPoint3& cTerrainNode::GetCenter()
{
	return mCenter;
}

/// 지형 가지 노드
class cTerrainBranchNode : public cTerrainNode
{
	friend class cTerrainLeafNode;

public:
	cTerrainBranchNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cTerrainBranchNode();

	/// 로딩
	bool Load( cFileLoader& loader );

#ifdef MAP_EDITOR
	/// 저장
	bool Save( cFileSaver& saver );
#endif

	/// 동기화
	void SyncHeight();
	void SyncAlpha();
	void SyncPaintAlpha();
	void SyncColor();

	/// 컬링
	void Cull();
	void AddToVisibleArray();

	/// 충돌 검사
	bool CollideRay();
	bool CollideSphere();

protected:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

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
#ifdef MAP_EDITOR
	friend class cTerrainNodeBuilding;
	friend class cTerrainNodePainting;
	friend class cTerrainNodeDetailing;
#endif

public:
	cTerrainLeafNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cTerrainLeafNode();

	/// 지움
	void Clear();

	/// 로딩
	bool Load( cFileLoader& loader );

#ifdef MAP_EDITOR
	/// 저장
	bool Save( cFileSaver& saver );
#endif

	/// 동기화
	void SyncHeight();
	void SyncAlpha();
	void SyncPaintAlpha();
	void SyncColor();

	/// 컬링
	void Cull();

	/// 가시 여부
	void SetVisible( bool visible );

	/// 가시 배열에 추가
	void AddToVisibleArray();

	/// 충돌 검사
	bool CollideRay();
	bool CollideSphere();

	/// 조명
	void SetLight( cLightSceneNode* light, unsigned int indexByLight );
	const cLightSceneNode* GetLight() const;

#ifdef MAP_EDITOR
	/// 올림
	bool Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 낮춤
	bool Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 평평하게 만듦
	bool Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 부드럽게 만듦
	/// ratio는 [0.1, 1] 범위의 값을 가져야 한다.
	bool Smooth( const NiPoint3& pos, float outerRadius, float ratio );
#endif /// MAP_EDITOR

	/// 네비메시에 맞춤
	bool SyncToNaviMesh( const NiPoint3& pos, float innerRadius, float outerRadius );

	/// 픽킹 높이에 맞춤
	bool SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float z );

#ifdef MAP_EDITOR
	/// 디테일
	bool PaintAlpha( const NiPoint3& pos, float innerRadius, float outerRadius, const NiPoint3& alpha );
	bool BlurAlpha( const NiPoint3& pos, float outerRadius, float ratio );
	bool SharpenAlpha( const NiPoint3& pos, float outerRadius, float ratio );

	/// 색칠
	bool Color( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color );
	bool Brush( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color, float opacity );
	bool Lighter( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool Darker( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool BlurColor( const NiPoint3& pos, float outerRadius, float ratio );
#endif /// MAP_EDITOR

	/// 텍스처
	unsigned int GetTextureIndex0() const;
	unsigned int GetTextureIndex1() const;
	unsigned int GetTextureIndex2() const;
	unsigned int GetTextureValue0() const;
	unsigned int GetTextureValue1() const;
	LPDIRECT3DTEXTURE9 GetD3DTexture0() const;
	LPDIRECT3DTEXTURE9 GetD3DTexture1() const;
	LPDIRECT3DTEXTURE9 GetD3DTexture2() const;

private:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

	/// 기하 정보를 갱신
	void UpdatePosCoords();
	void UpdateColors();
	void UpdateAlphas();

#ifdef MAP_EDITOR
	/// 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupBuilding( cTerrainNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius );
	bool BackupPainting( cTerrainNodePaintingInfo* info, const NiPoint3& pos, float outerRadius );
	bool BackupDetailing( cTerrainNodeDetailingInfo* info, const NiPoint3& pos, float outerRadius );

	/// 지형과 동기화 및 갱신
	/// 동기화는 계산상의 오차로 인한 크랙을 방지한다.
	void UpdateBuilding( cTerrainNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius );
	void UpdatePainting( cTerrainNodePaintingInfo* info, const NiPoint3& pos, float outerRadius );
	void UpdateDetailing( cTerrainNodeDetailingInfo* info, const NiPoint3& pos, float outerRadius );

	/// Undo, Redo
	void Undo( const cTerrainNodeBuilding& doing );
	void Undo( const cTerrainNodePainting& doing );
	void Undo( const cTerrainNodeDetailing& doing );
	void Redo( const cTerrainNodeBuilding& doing );
	void Redo( const cTerrainNodePainting& doing );
	void Redo( const cTerrainNodeDetailing& doing );
#endif /// MAP_EDITOR

	/// 정점 범위를 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius );

	/// 텍스처
	void SetTextures( cTerrainTexture* tex0, cTerrainTexture* tex1, cTerrainTexture* tex2 );
	void SetTexture0( cTerrainTexture* tex );
	void SetTexture1( cTerrainTexture* tex );
	void SetTexture2( cTerrainTexture* tex );
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

	/// 높이 배열
	float* mHeights;

	/// 알파 배열
	NiPoint3* mAlphas;
	NiPoint3* mPaintAlphas;

	/// 색상 배열
	NiColor* mColors;

	/// Level of detail
	unsigned int mLod;
	unsigned int mCrackIndex;

	cTerrainLeafNode* mSiblingLeafNode[4];

private:
	/// 버전
	static unsigned int mVersion;

	/// 리프 노드의 그리드 크기
	static unsigned int mCellCount[TERRAIN_LOD_COUNT];
	static unsigned int mLineCount[TERRAIN_LOD_COUNT];
};

inline
void cTerrainLeafNode::SetVisible( bool visible )
{
	mVisible = visible;
}

inline
const cLightSceneNode* cTerrainLeafNode::GetLight() const
{
	return mLight;
}

inline
unsigned int cTerrainLeafNode::GetTextureIndex0() const
{
	if( mTexture0 == 0 )
	{
		assert( 0 );
		return 0;
	}
	return mTexture0->mIndexByTerrain;
}

inline
unsigned int cTerrainLeafNode::GetTextureIndex1() const
{
	if( mTexture1 == 0 )
	{
		assert( 0 );
		return 0;
	}
	return mTexture1->mIndexByTerrain;
}

inline
unsigned int cTerrainLeafNode::GetTextureIndex2() const
{
	if( mTexture2 == 0 )
	{
		assert( 0 );
		return 0;
	}
	return mTexture2->mIndexByTerrain;
}

inline
unsigned int cTerrainLeafNode::GetTextureValue0() const
{
	return mTextureValue0;
}

inline
unsigned int cTerrainLeafNode::GetTextureValue1() const
{
	return mTextureValue1;
}

inline
LPDIRECT3DTEXTURE9 cTerrainLeafNode::GetD3DTexture0() const
{
	return mTexture0 ? mTexture0->mTexture : 0;
}

inline
LPDIRECT3DTEXTURE9 cTerrainLeafNode::GetD3DTexture1() const
{
	return mTexture1 ? mTexture1->mTexture : 0;
}

inline
LPDIRECT3DTEXTURE9 cTerrainLeafNode::GetD3DTexture2() const
{
	return mTexture2 ? mTexture2->mTexture : 0;
}

/// 지형 노드 비교 함수자
class cTerrainNodeCompareForRendering
{
public:
	bool operator () ( const cTerrainLeafNode* left, const cTerrainLeafNode* right ) const
	{
		cTerrainBuffer* lbuff = left->mBuffer;
		cTerrainBuffer* rbuff = right->mBuffer;

		if( lbuff == rbuff )
		{
			unsigned int ltex = left->mTextureValue0;
			unsigned int rtex = right->mTextureValue0;

			if( ltex == rtex )
			{
				ltex = left->mTextureValue1;
				rtex = right->mTextureValue1;

				if( ltex == rtex )
					return left->mLod < right->mLod;
				return ltex < rtex;
			}
			return ltex < rtex;
		}
		return lbuff < rbuff;
	};
};
