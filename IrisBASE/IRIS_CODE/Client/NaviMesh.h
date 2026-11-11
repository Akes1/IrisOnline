/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 네비메시
* 주의사항 : 
*===========================================================================*/
#pragma once

//#include "Shader.h"

class cRay;
class cBox;
class cNaviMeshNode;
class cNaviMeshBranchNode;
class cNaviMeshLeafNode;

/// 네비메시 상수
const unsigned int NAVIMESH_DEFAULT_RESOLUTION = 512;
const unsigned int NAVIMESH_BUFF_CELL_COUNT = 128;
const unsigned int NAVIMESH_BUFF_LINE_COUNT = NAVIMESH_BUFF_CELL_COUNT + 1;
const unsigned int NAVIMESH_BUFF_VERT_COUNT = NAVIMESH_BUFF_LINE_COUNT * NAVIMESH_BUFF_LINE_COUNT;
const unsigned int NAVIMESH_LEAF_CELL_COUNT = 8;
const unsigned int NAVIMESH_LEAF_LINE_COUNT = NAVIMESH_LEAF_CELL_COUNT + 1;
const unsigned int NAVIMESH_LEAF_LINE_COUNT_X2 = NAVIMESH_LEAF_LINE_COUNT * 2;
const unsigned int NAVIMESH_LEAF_VERT_COUNT = NAVIMESH_LEAF_LINE_COUNT * NAVIMESH_LEAF_LINE_COUNT;

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

/// 네비메시
class cNaviMesh
{
	static cNaviMesh* mSingleton;
	friend class cNaviMeshLeafNode;

public:
	static const unsigned int MAX_NODES = 21845;

public:
	cNaviMesh();
	~cNaviMesh();

	/// 지움
	void Clear();

	/// 로딩
	bool LoadHeader( unsigned int& count, const cString& pathName );
	int LoadNodes( unsigned int count );

	/// 픽킹
	bool Pick( NiPoint3* pos, int mouseX, int mouseY );
	bool Pick( NiPoint3* pos, const cRay& ray );
	bool Pick( NiPoint3* pos, float* dist, const cRay& ray, float maxDistance );

	/// 높이값
	bool CalcHeight( float* height, float x, float y ) const;
	const float* GetHeights() const;

	/// 그리드 사이즈를 리턴
	unsigned int GetCellCount() const;

	/// 정점당 미터수를 리턴
	float GetMetersPerVertex() const;

	/// 미터당 단위수를 리턴
	unsigned int GetUnitsPerMeter() const;

	/// 정점 사이의 단위를 리턴
	float GetUnitsPerVertex() const;

	/// 리프 노드 사이의 단위를 리턴
	float GetUnitsPerLeafNode() const;

	/// 해당 위치의 리프 노드를 리턴
	cNaviMeshLeafNode* GetLeafNode( float x, float y ) const;

	/// 네비메시 전체를 감싸는 경계 상자를 리턴
	const cBox& GetBoundBox() const;

	/// 한 변의 길이
	float GetSegmentLength() const;

protected:
	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 해당 위치에 리프 노드를 설정
	void SetLeafNode( unsigned int xi, unsigned int yi, cNaviMeshLeafNode* node );

	/// 높이값
	void SetHeight( unsigned int xi, unsigned int yi, float height );
	float GetHeightFast( unsigned int xi, unsigned int yi ) const;

	/// 텍스처 좌표 배열
	NiPoint2* GetTexCoords() const;

	/// 공통 스트립 배열
	unsigned short* GetStripLength();
	unsigned short* GetStripIndex();

public:
	/// 단일체를 리턴
	static cNaviMesh* GetSingleton();

private:
	/// 그리드 크기
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
	cNaviMeshBranchNode* mRootNode;

	/// 리프 노드 배열
	typedef tPointerArray<void*> cLeafNodeArray;
	cLeafNodeArray mNodeArray;

	/// 높이 배열
	float* mHeights;

	/// 로딩
	cFileLoader mLoader;
	cNaviMeshNode** mLoadArray;
	unsigned int mLoadCount;
	unsigned int mLoadIndex;
};

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
float cNaviMesh::GetSegmentLength() const
{
	return mSegmentLength;
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
cNaviMesh* cNaviMesh::GetSingleton()
{
	return mSingleton;
}

#define NAVIMESH cNaviMesh::GetSingleton()
