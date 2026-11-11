#include "stdafx.h"
#include "TerrainNode.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "Ray.h"
#include "LightSceneNode.h"

#include "../TerrainEditor.h"

/// 컬링용
NiFrustumPlanes* cTerrainNode::mFrustum = 0;
tArray<cTerrainLeafNode*>* cTerrainNode::mVisibleArray = 0;

/// 충돌 검사용
const cRay* cTerrainNode::mRay = 0;
float* cTerrainNode::mDistance = 0;
NiPoint3* cTerrainNode::mContact = 0;
const cSphere* cTerrainNode::mSphere = 0;
tArray<cTerrainLeafNode*>* cTerrainNode::mPickedArray = 0;

cTerrainNode::cTerrainNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: mParent( parent )
{
	/// 경계 상자를 설정
	float unitsPerVertex = TERRAIN->GetUnitsPerVertex();
	float minx = float(xi) * unitsPerVertex;
	float miny = float(yi) * unitsPerVertex;
	float maxx = minx + float(cellCount) * unitsPerVertex;
	float maxy = miny + float(cellCount) * unitsPerVertex;

	mBoundBox.Set( minx, miny, -1.0f, maxx, maxy, 1.0f );
	
	mCenter.x = (minx + maxx) * 0.5f;
	mCenter.y = (miny + maxy) * 0.5f;
	mCenter.z = 0.0f;

	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;
}

cTerrainNode::~cTerrainNode()
{
}

cTerrainBranchNode::cTerrainBranchNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: cTerrainNode( parent, xi, yi, cellCount )
{
	/// 자식 노드를 생성
	unsigned int childCount = cellCount / 2;

	if( childCount > cTerrainLeafNode::mCellCount[0] )
	{
		mChild[0] = new cTerrainBranchNode( this, xi, yi, childCount );
		mChild[1] = new cTerrainBranchNode( this, xi + childCount, yi, childCount );
		mChild[2] = new cTerrainBranchNode( this, xi, yi + childCount, childCount );
		mChild[3] = new cTerrainBranchNode( this, xi + childCount, yi + childCount, childCount );
	}
	else if( childCount == cTerrainLeafNode::mCellCount[0] )
	{
		mChild[0] = new cTerrainLeafNode( this, xi, yi, childCount );
		mChild[1] = new cTerrainLeafNode( this, xi + childCount, yi, childCount );
		mChild[2] = new cTerrainLeafNode( this, xi, yi + childCount, childCount );
		mChild[3] = new cTerrainLeafNode( this, xi + childCount, yi + childCount, childCount );
	}
	else
	{
		assert( 0 && "invalid cell count" );
	}

	assert( mChild[0] && mChild[1] && mChild[2] && mChild[3] );
}

cTerrainBranchNode::~cTerrainBranchNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];
}

void cTerrainBranchNode::UpdateBoundUpward()
{
	/// 경계 상자를 갱신
	cBox box;
	box.AddBox( mChild[0]->GetBoundBox() );
	box.AddBox( mChild[1]->GetBoundBox() );
	box.AddBox( mChild[2]->GetBoundBox() );
	box.AddBox( mChild[3]->GetBoundBox() );

	mBoundBox = box;
	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;

	/// 부모의 경계 상자를 갱신
	if( mParent )
		mParent->UpdateBoundUpward();
}

void cTerrainBranchNode::SyncHeight()
{
	/// 자식 노드에 대해 재귀 호출
	mChild[0]->SyncHeight();
	mChild[1]->SyncHeight();
	mChild[2]->SyncHeight();
	mChild[3]->SyncHeight();
}

void cTerrainBranchNode::SyncAlpha()
{
	/// 자식 노드에 대해 재귀 호출
	mChild[0]->SyncAlpha();
	mChild[1]->SyncAlpha();
	mChild[2]->SyncAlpha();
	mChild[3]->SyncAlpha();
}

void cTerrainBranchNode::SyncPaintAlpha()
{
	mChild[0]->SyncPaintAlpha();
	mChild[1]->SyncPaintAlpha();
	mChild[2]->SyncPaintAlpha();
	mChild[3]->SyncPaintAlpha();
}

void cTerrainBranchNode::SyncColor()
{
	/// 자식 노드에 대해 재귀 호출
	mChild[0]->SyncColor();
	mChild[1]->SyncColor();
	mChild[2]->SyncColor();
	mChild[3]->SyncColor();
}

bool cTerrainBranchNode::CollideRay()
{
	bool ret = false;
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( *mRay, scale ) )
	{
		/// 자식들에 대해 교차 검사
		ret |= mChild[0]->CollideRay();
		ret |= mChild[1]->CollideRay();
		ret |= mChild[2]->CollideRay();
		ret |= mChild[3]->CollideRay();
	}
	return ret;
}

bool cTerrainBranchNode::CollideSphere()
{
	bool ret = false;

	if( mBoundBox.IntersectSphere( *mSphere ) )
	{
		/// 자식들에 대해 교차 검사
		ret |= mChild[0]->CollideSphere();
		ret |= mChild[1]->CollideSphere();
		ret |= mChild[2]->CollideSphere();
		ret |= mChild[3]->CollideSphere();
	}
	return ret;
}


unsigned int cTerrainLeafNode::mVersion = 0;
unsigned int cTerrainLeafNode::mCellCount[TERRAIN_LOD_COUNT] = { 0 };
unsigned int cTerrainLeafNode::mLineCount[TERRAIN_LOD_COUNT] = { 0 };

cTerrainLeafNode::cTerrainLeafNode( cTerrainBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: cTerrainNode( parent, xi, yi, cellCount )
, mXIndex( xi )
, mYIndex( yi )
, mVisible( true )
, mTexture0( 0 )
, mTexture1( 0 )
, mTexture2( 0 )
, mTextureValue0( 0 )
, mTextureValue1( 0 )
, mLight( 0 )
, mLod( UINT_MAX )
, mCrackIndex( UINT_MAX )
, mIndexByLight( 0xFFFFFFFF )
{
	assert( cellCount == mCellCount[0] && "not leaf node!" );

	mSiblingLeafNode[0] = 0;
	mSiblingLeafNode[1] = 0;
	mSiblingLeafNode[2] = 0;
	mSiblingLeafNode[3] = 0;

	/// 지형의 리프 노드 배열에 설정
	TERRAIN->SetLeafNode( xi / mCellCount[0], yi / mCellCount[0], this );

	/// 버퍼를 설정
	mBuffer = TERRAIN->GetBuffer( xi, yi );

	/// 텍스처를 설정
	cTerrainTexture* tex = TERRAIN->GetTexture(0);
	SetTextures( tex, tex, tex );

	/// 위치 배열을 생성
	mHeights = new float[mLineCount[0] * mLineCount[0]];

	/// 알파 배열을 생성
	mAlphas = NiNew NiPoint3[mLineCount[0] * mLineCount[0]];

	mPaintAlphas = NiNew NiPoint3[mLineCount[0] * mLineCount[0]];

	/// 색상 배열을 생성
	mColors = NiNew NiColor[mLineCount[0] * mLineCount[0]];
}

cTerrainLeafNode::~cTerrainLeafNode()
{
	Clear();

	mSiblingLeafNode[0] = 0;
	mSiblingLeafNode[1] = 0;
	mSiblingLeafNode[2] = 0;
	mSiblingLeafNode[3] = 0;

	/// 배열을 제거
	if( mHeights )
	{
		delete [] mHeights;
		mHeights = 0;
	}

	if( mPaintAlphas )
	{
		NiDelete [] mPaintAlphas;
		mPaintAlphas = 0;
	}

	if( mAlphas )
	{
		NiDelete [] mAlphas;
		mAlphas = 0;
	}
	if( mColors )
	{
		NiDelete [] mColors;
		mColors = 0;
	}

	/// 조명에서 탈착
	if( mLight )
	{
		mLight->Detach( this );
		mLight = 0;
	}
}

void cTerrainLeafNode::Clear()
{
	/// 텍스처를 해제
	mTextureValue0 = 0;
	mTextureValue1 = 0;

	if( mTexture0 )
	{
		mTexture0->Drop();
		mTexture0 = 0;
	}
	if( mTexture1 )
	{
		mTexture1->Drop();
		mTexture1 = 0;
	}
	if( mTexture2 )
	{
		mTexture2->Drop();
		mTexture2 = 0;
	}
}

void cTerrainLeafNode::UpdateBoundUpward()
{
	/// 리프 노드 안의 모든 정점으로부터 최소값, 최대값을 계산
	float minz = +NI_INFINITY;
	float maxz = -NI_INFINITY;
	float z = 0.0f;

	for( unsigned int yi = 0; yi < mLineCount[0]; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount[0]; ++xi )
		{
			z = TERRAIN->GetHeightFast( mXIndex + xi, mYIndex + yi );

			if( z < minz )
				minz = z;
			if( z > maxz )
				maxz = z;
		}
	}

	/// 경계 상자를 갱신
	NiPoint3 min = mBoundBox.GetMin();
	NiPoint3 max = mBoundBox.GetMax();
	min.z = minz;
	max.z = maxz;
	mBoundBox.Set( min, max );
	mCenter = (min + max) * 0.5f;
	mRadius = (max - min).Length() * 0.5f;

	/// 부모의 경계 상자를 갱신
	mParent->UpdateBoundUpward();
}

void cTerrainLeafNode::UpdatePosCoords()
{
	assert( mBuffer );

	/// 법선 배열을 갱신
	TERRAIN->ComputeNormals( mXIndex, mYIndex, mXIndex + mLineCount[0], mYIndex + mLineCount[0] );

	/// 버퍼를 갱신
	mBuffer->UpdatePosCoord( TERRAIN, mXIndex, mYIndex, mLineCount[0] );


	/*// 리스트를 갱신
	for( unsigned int i = 0; i < TERRAIN_LOD_COUNT; ++i )
	{
		unsigned int gridSize = mGridSize[i];
		NiPoint3* posCoords = mPosCoords[i];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackPosCoords[i][0][j] = posCoords[0 + j];
			mCrackPosCoords[i][1][j] = posCoords[cellCount * gridSize + j];
			mCrackPosCoords[i][2][j] = posCoords[j * gridSize + 0];
			mCrackPosCoords[i][3][j] = posCoords[j * gridSize + cellCount];
		}

		for( unsigned int j = 0; j < 4; ++j )
		{
			NiGeometryData* geomData = mTriCracks[i][j]->GetModelData();

			//geomData->Replace(
			//	unsigned short(gridSize),
			//	mCrackPosCoords[i][j],
			//	mCrackAlphas[i][j],
			//	mCrackColors[i][j],
			//	mCrackTexCoords[i][j],
			//	1,
			//	NiGeometryData::NBT_METHOD_NONE );

			geomData->MarkAsChanged( NiGeometryData::VERTEX_MASK );
			//mTriCracks[i][j]->Update( 0.0f );
		}
	}
	*/
}

void cTerrainLeafNode::UpdateColors()
{
	assert( mBuffer );

	mBuffer->UpdateColor( TERRAIN, mXIndex, mYIndex, mLineCount[0] );

	/*// 리스트를 갱신
	for( unsigned int i = 0; i < TERRAIN_LOD_COUNT; ++i )
	{
		unsigned int gridSize = mGridSize[i];
		NiColorA* colors = mColors[i];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackColors[i][0][j] = colors[0 + j];
			mCrackColors[i][1][j] = colors[cellCount * gridSize + j];
			mCrackColors[i][2][j] = colors[j * gridSize + 0];
			mCrackColors[i][3][j] = colors[j * gridSize + cellCount];
		}

		for( unsigned int j = 0; j < 4; ++j )
		{
			NiGeometryData* geomData = mTriCracks[i][j]->GetModelData();

			//geomData->Replace(
			//	unsigned short(gridSize),
			//	mCrackPosCoords[i][j],
			//	mCrackAlphas[i][j],
			//	mCrackColors[i][j],
			//	mCrackTexCoords[i][j],
			//	1,
			//	NiGeometryData::NBT_METHOD_NONE );

			geomData->MarkAsChanged( NiGeometryData::COLOR_MASK );
		}
	}
	*/
}

void cTerrainLeafNode::UpdateAlphas()
{
	assert( mBuffer );

	mBuffer->UpdateAlpha( TERRAIN, mXIndex, mYIndex, mLineCount[0] );

	/*// 리스트를 갱신
	for( unsigned int i = 0; i < TERRAIN_LOD_COUNT; ++i )
	{
		unsigned int gridSize = mGridSize[i];
		NiPoint3* alphas = mAlphas[i];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackAlphas[i][0][j] = alphas[0 + j];
			mCrackAlphas[i][1][j] = alphas[cellCount * gridSize + j];
			mCrackAlphas[i][2][j] = alphas[j * gridSize + 0];
			mCrackAlphas[i][3][j] = alphas[j * gridSize + cellCount];
		}

		for( unsigned int j = 0; j < 4; ++j )
		{
			NiGeometryData* geomData = mTriCracks[i][j]->GetModelData();

			//geomData->Replace(
			//	unsigned short(gridSize),
			//	mCrackPosCoords[i][j],
			//	mCrackAlphas[i][j],
			//	mCrackColors[i][j],
			//	mCrackTexCoords[i][j],
			//	1,
			//	NiGeometryData::NBT_METHOD_NONE );

			geomData->MarkAsChanged( NiGeometryData::NORMAL_MASK );
		}
	}
	*/
}

void cTerrainLeafNode::SyncHeight()
{
	/// 위치 배열을 갱신
	unsigned int lineCount = mLineCount[0];

	for( unsigned int yi = 0, i = 0; yi < lineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < lineCount; ++xi, ++i )
		{
			mHeights[i] = TERRAIN->GetHeightFast( mXIndex + xi, mYIndex + yi );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		gridSize = mGridSize[i];

		for( unsigned int j = 0, y = 0; y < gridSize; ++y )
		{
			for( unsigned int x = 0; x < gridSize; ++x, ++j )
			{
				mPosCoords[i][j].z = mPosCoords[0][y * step * mLineCount[0] + x * step].z;
			}
		}
	}
	*/

	/// 경계 상자를 갱신
	UpdateBoundUpward();
}

void cTerrainLeafNode::SyncAlpha()
{
	/// 색상 배열을 갱신
	unsigned int lineCount = mLineCount[0];

	for( unsigned int yi = 0, i = 0; yi < lineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < lineCount; ++xi, ++i )
		{
			mAlphas[i] = TERRAIN->GetAlphaFast( mXIndex + xi, mYIndex + yi );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		gridSize = mGridSize[i];

		for( unsigned int j = 0, y = 0; y < gridSize; ++y )
		{
			for( unsigned int x = 0; x < gridSize; ++x, ++j )
			{
				mAlphas[i][j] = mAlphas[0][y * step * mLineCount[0] + x * step];
			}
		}
	}
	*/
}

void cTerrainLeafNode::SyncPaintAlpha()
{
	unsigned int lineCount = mLineCount[0];

	for( unsigned int yi = 0, i = 0; yi < lineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < lineCount; ++xi, ++i )
		{
			mPaintAlphas[i] = TERRAIN->GetPaintAlphaFast( mXIndex + xi, mYIndex + yi );
		}
	}
}


void cTerrainLeafNode::SyncColor()
{
	/// 색상 배열을 갱신
	unsigned int lineCount = mLineCount[0];

	for( unsigned int yi = 0, i = 0; yi < lineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < lineCount; ++xi, ++i )
		{
			mColors[i] = TERRAIN->GetColorFast( mXIndex + xi, mYIndex + yi );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		gridSize = mGridSize[i];

		for( unsigned int j = 0, y = 0; y < gridSize; ++y )
		{
			for( unsigned int x = 0; x < gridSize; ++x, ++j )
			{
				mColors[i][j] = mColors[0][y * step * mLineCount[0] + x * step];
			}
		}
	}
	*/
}

bool cTerrainLeafNode::CollideRay()
{
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( *mRay, scale ) == false )
		return false;

	/// 삼각형들에 대해 교차 검사
	bool ret = false;
	NiPoint3 p0, p1, p2, p3, out;

	for( unsigned int yi = mYIndex, yend = mYIndex + mCellCount[0]; yi < yend; ++yi )
	{
		for( unsigned int xi = mXIndex, xend = mXIndex + mCellCount[0]; xi < xend; ++xi )
		{
			float x0 = xi * 100.0f;
			float y0 = yi * 100.0f;
			p0.x = x0;
			p0.y = y0;
			p0.z = TERRAIN->GetHeightFast( xi, yi );
			p1.x = x0 + 100.0f;
			p1.y = y0 + 100.0f;
			p1.z = TERRAIN->GetHeightFast( xi + 1, yi + 1 );
			p2.x = x0;
			p2.y = y0 + 100.0f;
			p2.z = TERRAIN->GetHeightFast( xi, yi + 1 );
			p3.x = x0 + 100.0f;
			p3.y = y0;
			p3.z = TERRAIN->GetHeightFast( xi + 1, yi );

			/// 왼쪽 위 삼각형
			if( mRay->IntersectTri( &out, p0, p1, p2 ) == true )
			{
				ret = true;
				float d = (out - mRay->GetOrigin()).Length();
				if( d < *mDistance )
				{
					*mDistance = d;
					*mContact = out;
				}
			}

			/// 오른쪽 아래 삼각형
			if( mRay->IntersectTri( &out, p0, p3, p1 ) == true )
			{
				ret = true;
				float d = (out - mRay->GetOrigin()).Length();
				if( d < *mDistance )
				{
					*mDistance = d;
					*mContact = out;
				}
			}
		}
	}
	return ret;
}

bool cTerrainLeafNode::CollideSphere()
{
	if( mBoundBox.IntersectSphere( *mSphere ) )
	{
		mPickedArray->PushBack( this );
		return true;
	}
	return false;
}

void cTerrainLeafNode::SetLight( cLightSceneNode* light, unsigned int indexByLight )
{
	if( mLight == light )
		return;

	if( mLight )
		mLight->Detach( this );

	mLight = light;
	mIndexByLight = indexByLight;
}

bool cTerrainLeafNode::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius )
{
	int lineCount = (int)mLineCount[0];
	float unitsPerVertex = TERRAIN->GetUnitsPerVertex();
	float x = pos.x;
	float y = pos.y;

	int xb = (int)((x - outerRadius) / unitsPerVertex) - mXIndex;
	int yb = (int)((y - outerRadius) / unitsPerVertex) - mYIndex;
	int xe = (int)((x + outerRadius) / unitsPerVertex) - mXIndex;
	int ye = (int)((y + outerRadius) / unitsPerVertex) - mYIndex;

	--xb;
	--yb;
	++xe;
	++ye;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe > lineCount )
		xe = lineCount;
	if( ye > lineCount )
		ye = lineCount;

	if( xb >= xe || yb >= ye )
	{
		return false;
	}

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
	return true;
}

void cTerrainLeafNode::SetTextures( cTerrainTexture* tex0, cTerrainTexture* tex1, cTerrainTexture* tex2 )
{
	if( tex0 == 0 || tex1 == 0 || tex2 == 0 )
	{
		assert( 0 );
		return;
	}

	if( tex0 != mTexture0 )
	{
		tex0->Grab();

		if( mTexture0 )
			mTexture0->Drop();
		mTexture0 = 0;
		mTexture0 = tex0;
	}
	if( tex1 != mTexture1 )
	{
		tex1->Grab();

		if( mTexture1 )
			mTexture1->Drop();
		mTexture1 = 0;
		mTexture1 = tex1;
	}
	if( tex2 != mTexture2 )
	{
		tex2->Grab();

		if( mTexture2 )
			mTexture2->Drop();
		mTexture2 = 0;
		mTexture2 = tex2;
	}

	CalcTextureValue();
}

void cTerrainLeafNode::SetTexture0( cTerrainTexture* tex )
{
	if( tex == 0 )
	{
		assert( 0 );
		return;
	}
	if( tex != mTexture0 )
	{
		tex->Grab();

		if( mTexture0 )
			mTexture0->Drop();
		mTexture0 = 0;
		mTexture0 = tex;

		CalcTextureValue();
	}
}

void cTerrainLeafNode::SetTexture1( cTerrainTexture* tex )
{
	if( tex == 0 )
	{
		assert( 0 );
		return;
	}
	if( tex != mTexture1 )
	{
		tex->Grab();

		if( mTexture1 )
			mTexture1->Drop();
		mTexture1 = 0;
		mTexture1 = tex;

		CalcTextureValue();
	}
}

void cTerrainLeafNode::SetTexture2( cTerrainTexture* tex )
{
	if( tex == 0 )
	{
		assert( 0 );
		return;
	}
	if( tex != mTexture2 )
	{
		tex->Grab();

		if( mTexture2 )
			mTexture2->Drop();
		mTexture2 = 0;
		mTexture2 = tex;

		CalcTextureValue();
	}
}

void cTerrainLeafNode::CalcTextureValue()
{
	assert( mTexture0 );
	assert( mTexture1 );
	assert( mTexture2 );

	unsigned int val[3] =
	{
		0, 0, 0
	};
	unsigned int tex[3] =
	{
		mTexture0->mIndexByTerrain, mTexture1->mIndexByTerrain, mTexture2->mIndexByTerrain
	};

	for( int i = 0; i < 3; ++i )
	{
		for( int j = 2; j >= 0; --j )
		{
			if( tex[i] > val[j] )
			{
				for( int k = 1; k <= j; ++k )
				{
					val[k-1] = val[k];
				}

				val[j] = tex[i];
				break;
			}
		}
	}

	mTextureValue0 = val[0] * 10000 + val[1] * 100 + val[2];
	mTextureValue1 = tex[0] * 10000 + tex[1] * 100 + tex[2];
}