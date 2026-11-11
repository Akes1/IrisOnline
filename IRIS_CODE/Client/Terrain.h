/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 지형
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Shader.h"

class cBox;
class cSphere;
class cTerrainNode;
class cTerrainBranchNode;
class cTerrainLeafNode;

/// 지형 상수
const unsigned int TERRAIN_DEFAULT_RESOLUTION = 512;
const unsigned int TERRAIN_BUFF_CELL_COUNT = 128;
const unsigned int TERRAIN_BUFF_LINE_COUNT = TERRAIN_BUFF_CELL_COUNT + 1;
const unsigned int TERRAIN_BUFF_VERT_COUNT = TERRAIN_BUFF_LINE_COUNT * TERRAIN_BUFF_LINE_COUNT;
const unsigned int TERRAIN_LEAF_CELL_COUNT = 8;
const unsigned int TERRAIN_LEAF_LINE_COUNT = TERRAIN_LEAF_CELL_COUNT + 1;
const unsigned int TERRAIN_LEAF_LINE_COUNT_X2 = TERRAIN_LEAF_LINE_COUNT * 2;
const unsigned int TERRAIN_LEAF_VERT_COUNT = TERRAIN_LEAF_LINE_COUNT * TERRAIN_LEAF_LINE_COUNT;

const unsigned int TERRAIN_RENDER_PRIME_COUNT = TERRAIN_LEAF_CELL_COUNT*TERRAIN_LEAF_CELL_COUNT * 2;


/// 지형 텍스처 상수
const unsigned int TERRAIN_TEXTURE_COUNT = 21;
const unsigned int TERRAIN_TEXTURE_COUNT_VER2 = 16;

/// 지형 Lod
enum eTerrainLod
{
	TERRAIN_LOD_0 = 0,
	TERRAIN_LOD_1,
	TERRAIN_LOD_2,
	TERRAIN_LOD_COUNT
};

/// 지형 파일 헤더
#pragma pack( push, 1 )
class cTerrainFileHeader
{
public:
	/// 식별 코드
	char mCode[12];

	/// 버전
	unsigned int mVersion;

	/// 그리드 크기 (셀 수)
	unsigned int mCellCount;

	/// 정점당 미터 수
	float mMetersPerVertex;

	/// 미터당 단위 수
	unsigned int mUnitsPerMeter;

	/// 텍스처 수
	unsigned int mNumTextures;
};
#pragma pack( pop )

/// 지형 텍스처
class cTerrainTexture
{
	friend class cTerrain;
	friend class cTerrainLeafNode;

public:
	cTerrainTexture( unsigned int index, LPDIRECT3DTEXTURE9 tex );
	~cTerrainTexture();

	void Grab();
	void Drop();

private:
	/// 지형에 의해 설정된 내부 인덱스
	unsigned int mIndexByTerrain;

	/// 참조 카운트
	unsigned int mRefCount;

	/// DX9
	LPDIRECT3DTEXTURE9 mTexture;
};

/// 지형 버퍼
class cTerrainBuffer
{
	friend class cTerrain;
	friend class cTerrainLeafNode;

public:
	cTerrainBuffer();
	~cTerrainBuffer();

	bool Init( cTerrain* terrain, unsigned int xi, unsigned int yi );

protected:
	void UpdatePosCoord( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count );
	void UpdateColor( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count );
	void UpdateAlpha( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count );

private:
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// DX9
	LPDIRECT3DVERTEXBUFFER9 mPosCoordBuffer;
	LPDIRECT3DVERTEXBUFFER9 mNormalBuffer;
	LPDIRECT3DVERTEXBUFFER9 mColorBuffer;
	LPDIRECT3DVERTEXBUFFER9 mAlphaBuffer;
};

/// 지형
class cTerrain
{
	static cTerrain* mSingleton;
	friend class cTerrainLeafNode;
	friend class cTerrainTexture;
	friend class cTerrainBuffer;

public:
	static const unsigned int MAX_NODES = 21845;

public:
	cTerrain();
	~cTerrain();

	/// 지움
	void Clear();

	/// 로딩
	bool LoadHeader( unsigned int& count, const cString& pathName );
	int LoadNodes( unsigned int count );

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 구와 충돌하는 노드들을 수집
	/// 바운딩 박스로 검사한다.
	bool CollideSphere( tArray<void*>* pickedArray, const cSphere& sphere );

	/// 단위
	float GetUnitsPerVertex() const;
	float GetUnitsPerLeafNode() const;

	/// 보이는 리프 노드 수
	unsigned int GetNumVisibleNodes() const;

	/// 경계 상자
	const cBox& GetBoundBox() const;

	/// 한 변의 길이
	float GetSegmentLength() const;

protected:
	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 텍스처
	bool LoadTexture( unsigned int index, const cString& pathName );
	cTerrainTexture* GetTexture( unsigned int index );
	void AddTexture( unsigned int index, cTerrainTexture* tex );
	void RemoveTexture( unsigned int index );

	/// 버퍼
	cTerrainBuffer* GetBuffer( unsigned int xi, unsigned int yi );

	/// 리프 노드
	void SetLeafNode( unsigned int xi, unsigned int yi, cTerrainLeafNode* node );
	cTerrainLeafNode* GetLeafNode( float x, float y ) const;

	/// 높이값
	void SetHeight( unsigned int xi, unsigned int yi, float height );
	bool GetHeight( float* height, unsigned int xi, unsigned int yi );
	float GetHeightFast( unsigned int xi, unsigned int yi ) const;

	/// 법선값
	void ComputeNormals( unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend );
	const NiPoint3& GetNormalFast( unsigned int xi, unsigned int yi ) const;

	/// 색상값
	void SetColor( unsigned int xi, unsigned int yi, const NiColor& color );
	const NiColor& GetColorFast( unsigned int xi, unsigned int yi ) const;

	/// 알파값
	void SetAlpha( unsigned int xi, unsigned int yi, const NiPoint3& alpha );
	const NiPoint3& GetAlphaFast( unsigned int xi, unsigned int yi ) const;

	/// 전역광을 리턴
	NiAmbientLight* GetAmbientLight() const;

	void CreateIndexedBuffer();

	unsigned int CalcLOD( float sqrDist );

public:
	/// 단일체를 리턴
	static cTerrain* GetSingleton();

protected:
	/// 그리드 크기 ( 512의 배수 + 1 )
	unsigned int mCellCount;
	unsigned int mLineCount;

	/// 단위
	float mMetersPerVertex;
	unsigned int mUnitsPerMeter;
	float mUnitsPerVertex;
	float mUnitsPerLeafNode;

	/// 한 변의 길이
	float mSegmentLength;

	/// 루트 노드
	cTerrainBranchNode* mRootNode;

	/// 리프 노드 배열
	typedef tArray<void*> cLeafNodeArray;
	cLeafNodeArray mNodeArray;

	/// 가시 배열
	cLeafNodeArray mVisibleArray;

	/// 높이 배열
	float* mHeights;

	/// 법선 배열
	NiPoint3* mNormals;

	/// 색상 배열
	NiColor* mColors;

	/// 알파 배열
	NiPoint3* mAlphas;

	/// Level of detail
	float mSquaredDistanceLod[TERRAIN_LOD_COUNT];

	/// 텍스처 배열
	cTerrainTexture* mTextures[TERRAIN_TEXTURE_COUNT];

	/// 셰이더
	cTerrainShader mShader;

	/// 버퍼
	cTerrainBuffer** mBuffer;

	/// DX9
	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration;

	//////////////////////////////////////////////////////////////////////////
	/// LOD index buffer 추가
	/// 8x8 LOD-0
	enum
	{
		eCRACK_LEFT = 0,
		eCRACK_RIGHT,
		eCRACK_UP,
		eCRACK_DOWN,

		eCRACK_LEFTUP,
		eCRACK_LEFTDOWN,
		eCRACK_RIGHTUP,
		eCRACK_RIGHTDOWN,

		eCRACK_MAX,
	};
	unsigned int mIndexCount8x8;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer8x8;
	/// 4x4 LOD-1
	unsigned int mIndexCount4x4;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer4x4;
	unsigned int mCrackIndexCount4x4[eCRACK_MAX];
	LPDIRECT3DINDEXBUFFER9 mCrackIndexBuffer4x4[eCRACK_MAX];
	/// 2x2 LOD-2
	unsigned int mIndexCount2x2;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer2x2;
	unsigned int mCrackIndexCount2x2[eCRACK_MAX];
	LPDIRECT3DINDEXBUFFER9 mCrackIndexBuffer2x2[eCRACK_MAX];
	//////////////////////////////////////////////////////////////////////////

	/// 로딩
	cFileLoader mLoader;
	cTerrainNode** mLoadArray;
	unsigned int mLoadCount;
	unsigned int mLoadIndex;
};

inline
cTerrainTexture* cTerrain::GetTexture( unsigned int i )
{
	assert( i < TERRAIN_TEXTURE_COUNT && "index out of range" );

	return mTextures[i];
}

inline
cTerrainBuffer* cTerrain::GetBuffer( unsigned int xi, unsigned int yi )
{
	return &(mBuffer[yi / TERRAIN_BUFF_CELL_COUNT][xi / TERRAIN_BUFF_CELL_COUNT]);
}

inline
void cTerrain::SetHeight( unsigned int xi, unsigned int yi, float height )
{
	mHeights[yi * mLineCount + xi] = height;
}

inline
float cTerrain::GetHeightFast( unsigned int xi, unsigned int yi ) const
{
	return mHeights[yi * mLineCount + xi];
}

inline
const NiPoint3& cTerrain::GetNormalFast( unsigned int xi, unsigned int yi ) const
{
	return mNormals[yi * mLineCount + xi];
}

inline
void cTerrain::SetAlpha( unsigned int xi, unsigned int yi, const NiPoint3& alpha )
{
	mAlphas[yi * mLineCount + xi] = alpha;
}

inline
const NiPoint3& cTerrain::GetAlphaFast( unsigned int xi, unsigned int yi ) const
{
	return mAlphas[yi * mLineCount + xi];
}

inline
void cTerrain::SetColor( unsigned int xi, unsigned int yi, const NiColor& color )
{
	mColors[yi * mLineCount + xi] = color;
}

inline
const NiColor& cTerrain::GetColorFast( unsigned int xi, unsigned int yi ) const
{
	return mColors[yi * mLineCount + xi];
}

inline
float cTerrain::GetUnitsPerVertex() const
{
	return mUnitsPerVertex;
}

inline
float cTerrain::GetUnitsPerLeafNode() const
{
	return mUnitsPerLeafNode;
}

inline
float cTerrain::GetSegmentLength() const
{
	return mSegmentLength;
}

inline
unsigned int cTerrain::GetNumVisibleNodes() const
{
	return mVisibleArray.GetSize();
}

inline
cTerrain* cTerrain::GetSingleton()
{
	return mSingleton;
}

#define TERRAIN	cTerrain::GetSingleton()
