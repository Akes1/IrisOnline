/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 지형
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Shader.h"

class cRay;
class cSphere;
class cBox;
class cSceneNode;
class cTerrainBranchNode;
class cTerrainLeafNode;
#ifdef MAP_EDITOR
class cTerrainBuildingInfo;
class cTerrainPaintingInfo;
class cTerrainDetailingInfo;
#endif /// MAP_EDITOR

enum eTerrainViewMode;

/// 지형 상수
const unsigned int TERRAIN_DEFAULT_RESOLUTION = 512;
const unsigned int TERRAIN_BUFF_CELL_COUNT = 128;
const unsigned int TERRAIN_BUFF_LINE_COUNT = TERRAIN_BUFF_CELL_COUNT + 1;
const unsigned int TERRAIN_BUFF_VERT_COUNT = TERRAIN_BUFF_LINE_COUNT * TERRAIN_BUFF_LINE_COUNT;
const unsigned int TERRAIN_LEAF_CELL_COUNT = 8;
const unsigned int TERRAIN_LEAF_LINE_COUNT = TERRAIN_LEAF_CELL_COUNT + 1;
const unsigned int TERRAIN_LEAF_LINE_COUNT_X2 = TERRAIN_LEAF_LINE_COUNT * 2;
const unsigned int TERRAIN_LEAF_VERT_COUNT = TERRAIN_LEAF_LINE_COUNT * TERRAIN_LEAF_LINE_COUNT;

/// 지형 텍스처 상수
const unsigned int TERRAIN_TEXTURE_COUNT = 21;
const unsigned int TERRAIN_TEXTURE_COUNT_VER2 = 16;

/// 지형 로딩 에러 코드
const int TERRAIN_LOAD_ERROR_OPEN = -1;
const int TERRAIN_LOAD_ERROR_FILE_HEADER = -2;
const int TERRAIN_LOAD_ERROR_FILE_TYPE = -3;
const int TERRAIN_LOAD_ERROR_FILE_VERSION = -4;
const int TERRAIN_LOAD_ERROR_GRID_SIZE = -5;
const int TERRAIN_LOAD_ERROR_TEXTURE_NAME = -6;
const int TERRAIN_LOAD_ERROR_TEXTURE = -7;
const int TERRAIN_LOAD_ERROR_NODE = -8;

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
	cTerrainTexture( unsigned int index, const cString& pathName, LPDIRECT3DTEXTURE9 tex );
	~cTerrainTexture();

	void Grab();
	void Drop();

	const char* GetFileName() const;
	CBitmap* GetBitmap();
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetIndexByTerrain() const;

	/// 색상값을 리턴
	/// 주의: x와 y는 [0, 1]사이의 값을 가져야한다.
	bool GetColor( float* red, float* green, float* blue, float x, float y );

private:
	/// 파일 이름
	cString mFileName;

	/// 이미지 데이타
	cTargaImage mImageData;

	/// 비트맵
	CBitmap mBitmap;

	/// 지형에 의해 설정된 내부 인덱스
	unsigned int mIndexByTerrain;

	/// 참조 카운트
	unsigned int mRefCount;

private:
	/// DX9
	LPDIRECT3DTEXTURE9 mTexture;
};

inline
const char* cTerrainTexture::GetFileName() const
{
	return mFileName.Cstr();
}

inline
CBitmap* cTerrainTexture::GetBitmap()
{
	return &mBitmap;
}

inline
unsigned int cTerrainTexture::GetWidth() const
{
	return mImageData.GetWidth();
}

inline
unsigned int cTerrainTexture::GetHeight() const
{
	return mImageData.GetHeight();
}

inline
unsigned int cTerrainTexture::GetIndexByTerrain() const
{
	return mIndexByTerrain;
}

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
#ifdef MAP_EDITOR
	friend class cTerrainBuilding;
#endif

public:
	cTerrain();
	~cTerrain();

	/// 지움
	void Clear();

	/// 초기화
	void Init( unsigned int cellCount, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );
	bool IsInited() const;

	/// 로딩
	int Load( const cString& pathName );

#ifdef MAP_EDITOR
	bool LoadPaintAlpha( const cString& pathName, unsigned int numVerts );
#endif

	/// 처리
	void Process( bool testing );

	/// 렌더링
	void Render();

	/// 텍스처
	bool LoadTexture( const cString& pathName );
	bool LoadTexture( unsigned int index, const cString& pathName );
	void UnLoadTexture( unsigned int index );
	cTerrainTexture* GetTexture( int index );

	/// 픽킹
	bool Pick( NiPoint3* pos, int mouseX, int mouseY );
	bool Pick( NiPoint3* pos, const cRay& ray );

	/// 구와 충돌하는 노드들을 수집
	/// 바운딩 박스로 검사한다.
	bool CollideSphere( tArray<cTerrainLeafNode*>* pickedArray, const cSphere& sphere );

#ifdef MAP_EDITOR
	/// 뷰 모드
	void SetViewMode( eTerrainViewMode viewMode );

	/// 저장
	bool Save( const cString& pathName );
	bool SavePaintAlpha( const cString& pathName );

	/// 높이맵
	void ClearHeightMap();
	bool ImportHeightMap( const cString& pathName, float scale );
	bool ImportHeightMap( const cString& pathName );
	bool ExportHeightMap( const cString& pathName );

	/// 색상맵
	void ClearColorMap();
	bool ImportColorMap( const cString& pathName );
	bool ExportColorMap( const cString& pathName );

	/// 알파맵
	void ClearAlphaMap();
	bool ImportAlphaMap( const cString& pathName );
	bool ExportAlphaMap( const cString& pathName );

	/// 스케일
	void AdjustHeight( float scale );

	/// 맞춤
	void SyncAllToNaviMesh();

	/// 편집
	bool Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float strength );
	bool Smooth( const NiPoint3& pos, float outerRadius, float ratio );
	bool SyncToNaviMesh( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float z );

	/// 색칠
	bool Color( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color );
	bool Brush( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color, float opacity );
	bool Lighter( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool Darker( const NiPoint3& pos, float innerRadius, float outerRadius );
	bool BlurColor( const NiPoint3& pos, float outerRadius, float ratio );

	/// 디테일
	void SetDetailTextures( cTerrainLeafNode* node, unsigned int index0, unsigned int index1, unsigned int index2 );
	void SetDetailTexturesToAll( unsigned int index0, unsigned int index1, unsigned int index2 );
	bool PaintAlpha( const NiPoint3& pos, float innerRadius, float outerRadius, const NiPoint3& alpha, bool applyDetailTex, unsigned int texIndex0, unsigned int texIndex1, unsigned int texIndex2 );
	bool BlurAlpha( const NiPoint3& pos, float outerRadius, float ratio );
	bool SharpenAlpha( const NiPoint3& pos, float outerRadius, float ratio );

	/// 변경 여부
	bool IsModified() const;
#endif /// MAP_EDITOR

	/// Level of Detail
	void SetLodEnabled( bool enabled );

	/// 주변광
	void SetAmbientLightAmbient( const NiColor& color );
	void SetAmbientLightDiffuse( const NiColor& color );
	void SetAmbientLightDimmer( float dimmer );

	/// 안개
	void SetFog( bool enabled, const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );

	/// 주변광
	NiAmbientLight* GetAmbientLight() const;

	/// 높이값
	float GetHeight( float x, float y ) const;
	const float* GetHeights() const;

	/// 그리드 크기
	unsigned int GetCellCount() const;
	unsigned int GetLeafCellCount() const;

	/// 단위
	float GetMetersPerVertex() const;
	unsigned int GetUnitsPerMeter() const;
	float GetUnitsPerVertex() const;
	float GetUnitsPerLeafNode() const;

	/// 해당 위치의 리프 노드
	cTerrainLeafNode* GetLeafNode( float x, float y ) const;

	/// 해당 위치의 색상값
	bool GetColor( NiColor* color, const NiPoint3& pos ) const;

	/// 리프 노드 배열
	tArray<cTerrainLeafNode*>& GetLeafNodeArray();

	/// 보이는 리프 노드 수
	unsigned int GetNumVisibleNodes() const;

	/// 지형 전체를 감싸는 경계 상자
	const cBox& GetBoundBox() const;

	/// 지형의 높이값
	/// 인덱스가 범위를 벗어난 경우 false를 리턴한다.
	bool GetHeight( float* height, unsigned int xi, unsigned int yi );

	///
	NiFogProperty* GetFogProperty() const;

protected:
#ifdef MAP_EDITOR
	/// 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupBuilding( cTerrainBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
	bool BackupPainting( cTerrainPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
	bool BackupDetailing( cTerrainDetailingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );

	/// 갱신
	void UpdateBuilding( cTerrainBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
	void UpdatePainting( cTerrainPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
	void UpdateDetailing( cTerrainDetailingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius );
#endif /// MAP_EDITOR

	void CreateIndexedBuffer();

	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 렌더링
	void Render0( LPDIRECT3DDEVICE9 device, const D3DXMATRIX& worldViewProj );
	void Render1( LPDIRECT3DDEVICE9 device, const D3DXMATRIX& worldViewProj );

	/// 텍스처
	void AddTexture( unsigned int index, cTerrainTexture* tex );
	void RemoveTexture( unsigned int index );

	/// 버퍼
	cTerrainBuffer* GetBuffer( unsigned int xi, unsigned int yi );

	/// 적용 범위 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius );
	bool CalcRangeTight( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius );

	/// 해당 위치에 리프 노드를 설정
	void SetLeafNode( unsigned int xi, unsigned int yi, cTerrainLeafNode* node );

	/// 높이값
	void SetHeight( unsigned int xi, unsigned int yi, float height );
	float GetHeightFast( unsigned int xi, unsigned int yi ) const;

	/// 법선값
	void ComputeNormals( unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend );
	const NiPoint3& GetNormalFast( unsigned int xi, unsigned int yi ) const;

	/// 색상값
	void SetColor( unsigned int xi, unsigned int yi, const NiColor& color );
	bool GetColor( NiColor* color, unsigned int xi, unsigned int yi );
	const NiColor& GetColorFast( unsigned int xi, unsigned int yi ) const;

	/// 알파값
	void SetAlpha( unsigned int xi, unsigned int yi, const NiPoint3& alpha );
	bool GetAlpha( NiPoint3* alpha, unsigned int xi, unsigned int yi );
	const NiPoint3& GetAlphaFast( unsigned int xi, unsigned int yi ) const;

	void SetPaintAlpha( unsigned int xi, unsigned int yi, const NiPoint3& alpha );
	bool GetPaintAlpha( NiPoint3* alpha, unsigned int xi, unsigned int yi );
	const NiPoint3& GetPaintAlphaFast( unsigned int xi, unsigned int yi ) const;

	unsigned int CalcLOD( float sqrDist );

public:
	/// 단일체를 리턴
	static cTerrain* GetSingleton();

protected:
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
	cTerrainBranchNode* mRootNode;

	/// 리프 노드 배열
	typedef tArray<cTerrainLeafNode*> cLeafNodeArray;
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
	NiPoint3* mPaintAlphas;

	/// 주변광
	NiAmbientLightPtr mAmbientLight;

	/// 공통 렌더링 속성
	NiFogPropertyPtr mFogProp;

	/// Level of detail
	bool mLodEnabled;
	float mSquaredDistanceLod[TERRAIN_LOD_COUNT];

	/// 텍스처 배열
	cTerrainTexture* mTextures[TERRAIN_TEXTURE_COUNT];
	unsigned int mNumTextures;

	/// 텍스처 맵
	typedef tHashMap<cString, LPDIRECT3DTEXTURE9> cTextureMap;
	cTextureMap mTextureMap;

	/// 셰이더
	cTerrainShader mShader0;
	cTerrainShader mShader1;

	/// 버퍼
	cTerrainBuffer** mBuffer;

	/// DX9
	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration0;
	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration1;

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

#ifdef MAP_EDITOR
	/// 뷰 모드
	eTerrainViewMode mViewMode;

	/// 변경 여부
	bool mModified;
#endif
};

inline
bool cTerrain::IsInited() const
{
	return mInited;
}

inline
void cTerrain::SetLodEnabled( bool enabled )
{
	mLodEnabled = enabled;
}

inline
NiAmbientLight* cTerrain::GetAmbientLight() const
{
	return mAmbientLight;
}

inline
cTerrainTexture* cTerrain::GetTexture( int i )
{
	if( i < 0 || i >= TERRAIN_TEXTURE_COUNT )
	{
		assert( 0 && "index out of range" );
		return 0;
	}
	else
		return mTextures[i];
}

inline
cTerrainBuffer* cTerrain::GetBuffer( unsigned int xi, unsigned int yi )
{
	return &(mBuffer[yi / TERRAIN_BUFF_CELL_COUNT][xi / TERRAIN_BUFF_CELL_COUNT]);
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
const NiPoint3& cTerrain::GetPaintAlphaFast( unsigned int xi, unsigned int yi ) const
{
	return mPaintAlphas[yi * mLineCount + xi];
}

inline 
void cTerrain::SetPaintAlpha( unsigned int xi, unsigned int yi, const NiPoint3& alpha )
{
	mPaintAlphas[yi * mLineCount + xi] = alpha;
}

inline
const float* cTerrain::GetHeights() const
{
	return mHeights;
}

inline
unsigned int cTerrain::GetCellCount() const
{
	return mCellCount;
}

inline
float cTerrain::GetMetersPerVertex() const
{
	return mMetersPerVertex;
}

inline
unsigned int cTerrain::GetUnitsPerMeter() const
{
	return mUnitsPerMeter;
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
tArray<cTerrainLeafNode*>& cTerrain::GetLeafNodeArray()
{
	return mNodeArray;
}

inline
unsigned int cTerrain::GetNumVisibleNodes() const
{
	return mVisibleArray.GetSize();
}

inline
NiFogProperty* cTerrain::GetFogProperty() const
{
	return mFogProp;
}

#ifdef MAP_EDITOR
inline
bool cTerrain::IsModified() const
{
	return mModified;
}
#endif

inline
cTerrain* cTerrain::GetSingleton()
{
	return mSingleton;
}

#define TERRAIN	cTerrain::GetSingleton()
