/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.03
* 내    용 : 네비필드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Shader.h"

class cRay;
class cNaviFieldBranchNode;
class cNaviFieldLeafNode;
class cNaviFieldPaintDialog;
#ifdef MAP_EDITOR
class cNaviFieldPaintingInfo;
#endif

/// 네비필드 상수
const unsigned int NAVIFIELD_DEFAULT_RESOLUTION = 512;
const unsigned int NAVIFIELD_BUFF_CELL_COUNT = 128;
const unsigned int NAVIFIELD_BUFF_LINE_COUNT = NAVIFIELD_BUFF_CELL_COUNT + 1;
const unsigned int NAVIFIELD_BUFF_VERT_COUNT = NAVIFIELD_BUFF_LINE_COUNT * NAVIFIELD_BUFF_LINE_COUNT;
const unsigned int NAVIFIELD_LEAF_CELL_COUNT = 8;
const unsigned int NAVIFIELD_LEAF_LINE_COUNT = NAVIFIELD_LEAF_CELL_COUNT + 1;
const unsigned int NAVIFIELD_LEAF_LINE_COUNT_X2 = NAVIFIELD_LEAF_LINE_COUNT * 2;
const unsigned int NAVIFIELD_LEAF_VERT_COUNT = NAVIFIELD_LEAF_LINE_COUNT * NAVIFIELD_LEAF_LINE_COUNT;

/// 네비필드 로딩 에러 코드
const int NAVIFIELD_LOAD_ERROR_OPEN = -1;
const int NAVIFIELD_LOAD_ERROR_FILE_HEADER = -2;
const int NAVIFIELD_LOAD_ERROR_FILE_TYPE = -3;
const int NAVIFIELD_LOAD_ERROR_FILE_VERSION = -4;
const int NAVIFIELD_LOAD_ERROR_GRID_SIZE = -5;
const int NAVIFIELD_LOAD_ERROR_TEXTURE_NAME = -6;
const int NAVIFIELD_LOAD_ERROR_TEXTURE = -7;
const int NAVIFIELD_LOAD_ERROR_NODE = -8;

/// 네비필드 파일 헤더
#pragma pack( push, 1 )
class cNaviFieldFileHeader
{
public:
	/// 식별 코드
	char mCode[14];

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

/// 네비필드 버퍼
class cNaviFieldBuffer
{
	friend class cNaviField;
	friend class cNaviFieldLeafNode;

public:
	cNaviFieldBuffer();
	~cNaviFieldBuffer();

	bool Init( cNaviField* naviField, unsigned int xi, unsigned int yi );

protected:
	void UpdatePosCoord( cNaviField* naviField, unsigned int xstart, unsigned ystart, unsigned int count );
	void UpdateColor( cNaviField* naviField, unsigned int xstart, unsigned ystart, unsigned int count );

private:
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// DX9
	LPDIRECT3DVERTEXBUFFER9 mPosCoordBuffer;
	LPDIRECT3DVERTEXBUFFER9 mColorBuffer;
};

/// 네비필드
class cNaviField
{
	static cNaviField* mSingleton;
	friend class cNaviFieldLeafNode;
	friend class cNaviFieldBuffer;
	friend class cNaviFieldPainting;

public:
	cNaviField();
	~cNaviField();

	/// 지움
	void Clear();

	/// 초기화
	void Init( unsigned int cellCount, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );

	/// 로딩
	int Load( const cString& pathName );

	/// 저장
	bool Save( const cString& pathName );

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 버퍼
	cNaviFieldBuffer* GetBuffer( unsigned int xi, unsigned int yi );

	/// 픽킹
	bool Pick( NiPoint3* pos, int mouseX, int mouseY );
	bool Pick( NiPoint3* pos, const cRay& ray );

	/// 네비메시에 맞춤
	void SyncAllToNaviMesh();

	/// 적용 범위 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius );

#ifdef MAP_EDITOR
	/// 색칠
	bool Color( const NiPoint3& pos, float radius, float red, float green, float blue, unsigned char value );
#endif

	/// 그리드 크기
	unsigned int GetCellCount() const;

	/// 정점당 미터
	float GetMetersPerVertex() const;

	/// 미터당 단위
	unsigned int GetUnitsPerMeter() const;

	/// 정점 사이의 단위
	float GetUnitsPerVertex() const;

	/// 필드값
	unsigned char GetValue( unsigned int xi, unsigned int yi ) const;
	bool GetValue( unsigned char* value, unsigned int xi, unsigned int yi ) const;
	bool GetValue( unsigned char* value, const NiPoint3& pos ) const;

	/// 높이값
	bool GetHeight( float* height, unsigned int xi, unsigned int yi ) const;
	bool GetHeight( float* height, const NiPoint3& pos ) const;

	/// 색상값
	bool GetColor( NiColor* color, const NiPoint3& pos ) const;

	/// 그리드 크기
	unsigned int GetLeafGridSize() const;

	/// 변경 여부
	bool IsModified() const;

private:
#ifdef MAP_EDITOR
	/// 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupPainting( cNaviFieldPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float radius );

	/// 갱신
	void UpdatePainting( cNaviFieldPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float radius );
#endif

	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 해당 위치에 리프 노드를 설정
	void SetLeafNode( unsigned int xi, unsigned int yi, cNaviFieldLeafNode* node );

	/// 필드값
	void SetValue( unsigned int xi, unsigned int yi, unsigned char value );

	/// 높이값
	float GetHeightFast( unsigned int xi, unsigned int yi ) const;

	/// 색상값
	void SetColor( unsigned int xi, unsigned int yi, const NiColor& color );
	const NiColor& GetColorFast( unsigned int xi, unsigned int yi ) const;

	/// 인덱스가 범위를 벗어난 경우 false를 리턴한다.
	bool GetColor( NiColor* color, unsigned int xi, unsigned int yi );

public:
	/// 단일체를 리턴
	static cNaviField* GetSingleton();

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
	cNaviFieldBranchNode* mRootNode;

	/// 리프 노드 배열
	typedef tArray<cNaviFieldLeafNode*> cLeafNodeArray;
	cLeafNodeArray mNodeArray;

	/// 필드값 배열
	unsigned char** mpValues;
	unsigned char* mValues;

	/// 높이 배열
	float** mpHeights;
	float* mHeights;

	/// 색상 배열
	NiColor* mColors;

	/// 가시 배열
	cLeafNodeArray mVisibleArray;

	/// 셰이더
	cTerrainShader mShader;

	/// 버퍼
	cNaviFieldBuffer** mBuffer;

	/// DX9
	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration;
	LPDIRECT3DINDEXBUFFER9 mIndexBuffer;

	/// 변경 여부
	bool mModified;
};

inline
cNaviFieldBuffer* cNaviField::GetBuffer( unsigned int xi, unsigned int yi )
{
	return &(mBuffer[yi / NAVIFIELD_BUFF_CELL_COUNT][xi / NAVIFIELD_BUFF_CELL_COUNT]);
}

inline
unsigned int cNaviField::GetCellCount() const
{
	return mCellCount;
}

inline
float cNaviField::GetMetersPerVertex() const
{
	return mMetersPerVertex;
}

inline
unsigned int cNaviField::GetUnitsPerMeter() const
{
	return mUnitsPerMeter;
}

inline
float cNaviField::GetUnitsPerVertex() const
{
	return mUnitsPerVertex;
}

inline
float cNaviField::GetHeightFast( unsigned int xi, unsigned int yi ) const
{
	return mHeights[yi * mLineCount + xi];
}

inline
const NiColor& cNaviField::GetColorFast( unsigned int xi, unsigned int yi ) const
{
	return mColors[yi * mLineCount + xi];
}

inline
bool cNaviField::IsModified() const
{
	return mModified;
}

inline
cNaviField* cNaviField::GetSingleton()
{
	return mSingleton;
}

#define NAVIFIELD cNaviField::GetSingleton()
