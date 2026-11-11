/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 네비메시
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Shader.h"

class cRay;
class cBox;
class cSceneNode;
class cNaviMeshBranchNode;
class cNaviMeshLeafNode;
#ifdef MAP_EDITOR
class cNaviMeshBuildingInfo;
#endif

/// 네비메시 상수
const unsigned int NAVIMESH_DEFAULT_RESOLUTION = 512;
const unsigned int NAVIMESH_BUFF_CELL_COUNT = 128;
const unsigned int NAVIMESH_BUFF_LINE_COUNT = NAVIMESH_BUFF_CELL_COUNT + 1;
const unsigned int NAVIMESH_BUFF_VERT_COUNT = NAVIMESH_BUFF_LINE_COUNT * NAVIMESH_BUFF_LINE_COUNT;
const unsigned int NAVIMESH_LEAF_CELL_COUNT = 8;
const unsigned int NAVIMESH_LEAF_LINE_COUNT = NAVIMESH_LEAF_CELL_COUNT + 1;
const unsigned int NAVIMESH_LEAF_LINE_COUNT_X2 = NAVIMESH_LEAF_LINE_COUNT * 2;
const unsigned int NAVIMESH_LEAF_VERT_COUNT = NAVIMESH_LEAF_LINE_COUNT * NAVIMESH_LEAF_LINE_COUNT;

/// 네비메시 로딩 에러 코드
const int NAVIMESH_LOAD_ERROR_OPEN = -1;
const int NAVIMESH_LOAD_ERROR_FILE_HEADER = -2;
const int NAVIMESH_LOAD_ERROR_FILE_TYPE = -3;
const int NAVIMESH_LOAD_ERROR_FILE_VERSION = -4;
const int NAVIMESH_LOAD_ERROR_GRID_SIZE = -5;
const int NAVIMESH_LOAD_ERROR_TEXTURE_NAME = -6;
const int NAVIMESH_LOAD_ERROR_TEXTURE = -7;
const int NAVIMESH_LOAD_ERROR_NODE = -8;

/// 네비메시 파일 헤더
#pragma pack( push, 1 )
class cNaviMeshFileHeader
{
public:
	/// 식별 코드
	char mCode[13];

	/// 버전
	unsigned int mVersion;

	/// 그리드 크기 (셀 수)
	unsigned int mCellCount;

	/// 정점당 미터 수
	float mMetersPerVertex;

	/// 미터당 단위 수 ( 100 )
	unsigned int mUnitsPerMeter;
};
#pragma pack( pop )

/// 네비메시 버퍼
class cNaviMeshBuffer
{
	friend class cNaviMesh;
	friend class cNaviMeshLeafNode;

public:
	cNaviMeshBuffer();
	~cNaviMeshBuffer();

	bool Init( cNaviMesh* naviMesh, unsigned int xi, unsigned int yi );

protected:
	void UpdatePosCoord( cNaviMesh* naviMesh, unsigned int xstart, unsigned ystart, unsigned int count );

private:
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// DX9
	LPDIRECT3DVERTEXBUFFER9 mPosCoordBuffer;
};

/// 네비메시
class cNaviMesh
{
	static cNaviMesh* mSingleton;
	friend class cNaviMeshLeafNode;
	friend class cNaviMeshBuffer;
	friend class cNaviMeshBuilding;

public:
	cNaviMesh();
	~cNaviMesh();

	/// 지움
	void Clear();

	/// 초기화
	void Init( unsigned int cellCount, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );
	bool IsInited() const;

	/// 로딩
	int Load( const cString& pathName );

	/// 저장
	bool Save( const cString& pathName );

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 버퍼
	cNaviMeshBuffer* GetBuffer( unsigned int xi, unsigned int yi );

	/// 픽킹
	bool Pick( NiPoint3* pos, int mouseX, int mouseY );
	bool Pick( NiPoint3* pos, const cRay& ray );
	bool Pick( NiPoint3* pos, float* dist, const cRay& ray, float maxDistance );

	/// 높이값
	bool CalcHeight( float* height, float x, float y ) const;
	bool GetHeight( float* height, unsigned int xi, unsigned int yi ) const;
	const float* GetHeights() const;

	/// 지형에 맞춤
	void SyncAllToTerrain();

#ifdef MAP_EDITOR
	/// 편집
	bool Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Smooth( const NiPoint3& pos, float outerRadius, float ratio );
#endif
	bool SyncToTerrain( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool SyncToObject( const NiPoint3& pos, float outerRadius );
	bool SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float z );

	/// 그리드 크기
	unsigned int GetCellCount() const;

	/// 정점당 미터수
	float GetMetersPerVertex() const;

	/// 미터당 단위수
	unsigned int GetUnitsPerMeter() const;

	/// 정점 사이의 단위
	float GetUnitsPerVertex() const;

	/// 리프 노드 사이의 단위
	float GetUnitsPerLeafNode() const;

	/// 해당 위치의 리프 노드
	cNaviMeshLeafNode* GetLeafNode( float x, float y ) const;

	/// 보이는 리프 노드 수
	unsigned int GetNumVisibleNodes() const;

	/// 경계 상자
	const cBox& GetBoundBox() const;

	/// 변경 여부
	bool IsModified() const;

private:
#ifdef MAP_EDITOR
	/// 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupBuilding( cNaviMeshBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );

	/// 갱신
	void UpdateBuilding( cNaviMeshBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
#endif

	/// 적용 범위 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius );

	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 해당 위치에 리프 노드를 설정
	void SetLeafNode( unsigned int xi, unsigned int yi, cNaviMeshLeafNode* node );

	/// 높이값
	void SetHeight( unsigned int xi, unsigned int yi, float height );
	float GetHeightFast( unsigned int xi, unsigned int yi ) const;

public:
	/// 단일체를 리턴
	static cNaviMesh* GetSingleton();

private:
	/// 초기화 여부
	bool mInited;

	/// 그리드 크기
	unsigned int mCellCount;
	unsigned int mLineCount;

	/// 정점당 미터 ( 1 )
	float mMetersPerVertex;

	/// 미터당 단위 ( 100 )
	unsigned int mUnitsPerMeter;

	/// 정점당 단위
	float mUnitsPerVertex;

	/// 리프 노드당 단위
	float mUnitsPerLeafNode;

	/// 루트 노드
	cNaviMeshBranchNode* mRootNode;

	/// 리프 노드 배열
	typedef tArray<cNaviMeshLeafNode*> cLeafNodeArray;
	cLeafNodeArray mNodeArray;

	/// 높이 배열
	float* mHeights;

	/// 가시 배열
	cLeafNodeArray mVisibleArray;

	/// 셰이더
	cTerrainShader mShader;

	/// 버퍼
	cNaviMeshBuffer** mBuffer;

	/// DX9
	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer;

	/// 변경 여부
	bool mModified;
};

inline
bool cNaviMesh::IsInited() const
{
	return mInited;
}

inline
cNaviMeshBuffer* cNaviMesh::GetBuffer( unsigned int xi, unsigned int yi )
{
	return &(mBuffer[yi / NAVIMESH_BUFF_CELL_COUNT][xi / NAVIMESH_BUFF_CELL_COUNT]);
}

inline
unsigned int cNaviMesh::GetCellCount() const
{
	return mCellCount;
}

inline
float cNaviMesh::GetMetersPerVertex() const
{
	return mMetersPerVertex;
}

inline
unsigned int cNaviMesh::GetUnitsPerMeter() const
{
	return mUnitsPerMeter;
}

inline
float cNaviMesh::GetUnitsPerVertex() const
{
	return mUnitsPerVertex;
}

inline
float cNaviMesh::GetUnitsPerLeafNode() const
{
	return mUnitsPerLeafNode;
}

inline
unsigned int cNaviMesh::GetNumVisibleNodes() const
{
	return mVisibleArray.GetSize();
}

inline
void cNaviMesh::SetHeight( unsigned int xi, unsigned int yi, float height )
{
	mHeights[yi * mLineCount + xi] = height;
}

inline
float cNaviMesh::GetHeightFast( unsigned int xi, unsigned int yi ) const
{
	return mHeights[yi * mLineCount + xi];
}

inline
const float* cNaviMesh::GetHeights() const
{
	return mHeights;
}

inline
bool cNaviMesh::IsModified() const
{
	return mModified;
}

inline
cNaviMesh* cNaviMesh::GetSingleton()
{
	return mSingleton;
}

#define NAVIMESH cNaviMesh::GetSingleton()
