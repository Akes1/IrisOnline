#include "stdafx.h"
#include "ShadowGeometry.h"

#include "RenderSystem.h"
#include "SceneManager.h"
#include "DynamicSceneNode.h"
#include "NaviMesh.h"
#include "Ray.h"

cShadowGeometry::cShadowGeometry()
: mMaxRadius( 0.0f )
, mRadius( 0.0f )
, mHeight( 0.0f )
, mLineCount( 0 )
, mMaxVerts( 0 )
, mCenter( NiPoint3::ZERO )
, mOOWidth( 0.0f )
, mAlpha( 0.0f )
, mTris( 0 )
, mTexProp(0)
, mTex( 0 )
{
}

cShadowGeometry::~cShadowGeometry()
{
	Clear();
}

void cShadowGeometry::Clear()
{
	mMaxRadius = 0.0f;
	mRadius = 0.0f;
	mHeight = 0.0f;
	mLineCount = 0;
	mMaxVerts = 0;
	mCenter = NiPoint3::ZERO;
	mOOWidth = 0.0f;
	mAlpha = 0.0f;
	if( mTris )
	{
		assert(mTris->GetRefCount()==1);
		mTris = 0;
	}
	if( mTexProp )
	{
		assert(mTexProp->GetRefCount()==1);
		mTexProp = 0;
	}
	mTex = 0;
}

bool cShadowGeometry::Init( NiTexture* tex, float maxRadius, float height, bool zTest )
{
	assert( tex );
	assert( maxRadius > 0.0f );

	Clear();

	///
	if( maxRadius > 500.0f )
		mRadius = mMaxRadius = 500.0f;
	else
		mRadius = mMaxRadius = maxRadius;
	mHeight = height;

	int i = int((mMaxRadius * 2.0f + 200.0f) / 100.0f);
	unsigned int maxTris = i * i * 2;
	assert( maxTris );

	mMaxVerts = 3 * maxTris;
	mActiveCount = 0;

	/// 엔진 기하를 생성
	mTris = NiNew NiMesh;
	assert(mTris);
	mTris->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_TRIANGLES);

	/// 렌더링 속성을 적용
	mTex = tex;

	NiAlphaProperty* pkAlpha = NiNew NiAlphaProperty;
	pkAlpha->SetAlphaBlending(true);
	mTris->AttachProperty(pkAlpha);

	NiMaterialProperty* pkMat = NiNew NiMaterialProperty;
	pkMat->SetAlpha(0.0f);
	mTris->AttachProperty(pkMat);

	NiTexturingProperty::Map* baseMap = NiNew NiTexturingProperty::Map( tex, 0, NiTexturingProperty::CLAMP_S_CLAMP_T, NiTexturingProperty::FILTER_BILERP );
	mTexProp = NiNew NiTexturingProperty;
	mTexProp->SetApplyMode( NiTexturingProperty::APPLY_REPLACE );
	mTexProp->SetBaseMap( baseMap );
	mTris->AttachProperty( mTexProp );

	NiZBufferProperty* pkZ = NiNew NiZBufferProperty;
	NIASSERT(pkZ);
	pkZ->SetZBufferTest(zTest);
	pkZ->SetZBufferWrite(false);
	mTris->AttachProperty(pkZ);

	mTris->UpdateProperties();

	mTris->AddStream( NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, mMaxVerts, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mTris->AddStream( NiCommonSemantics::TEXCOORD(), 0, NiDataStreamElement::F_FLOAT32_2, mMaxVerts, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mTris->SetSubmeshCount(1);

	SetActiveCount( 0 );

	///
	NiMeshUpdateProcess kUpdateProcess;
	mTris->Update( kUpdateProcess );
	return true;
}

void cShadowGeometry::Draw( NiRenderer* renderer )
{
	if( renderer == 0 )
		return;
	if( mTris == 0 )
		return;

	mTris->RenderImmediate( renderer );
}


void cShadowGeometry::Process( bool needUpdate, const NiPoint3& center, float alpha )
{
	if( mTris == 0 )
		return;

	/// 그림자 삼각형을 갱신
	if( needUpdate )
	{
		mAlpha = alpha;

		mActiveCount = 0;
		SetActiveCount( 0 );

		NiDataStreamElementLock kPositionLock( mTris, NiCommonSemantics::POSITION(), 0,
			NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
		assert( kPositionLock.IsLocked() );

		NiDataStreamElementLock kTexCoordLock( mTris, NiCommonSemantics::TEXCOORD(), 0,
			NiDataStreamElement::F_FLOAT32_2, NiDataStream::LOCK_WRITE );
		assert( kTexCoordLock.IsLocked() );

		const float* heights = NAVIMESH->GetHeights();
		assert( heights );
		mLineCount = (int)NAVIMESH->GetCellCount() + 1;

		float r = mRadius;
		float x = mCenter.x = center.x;
		float y = mCenter.y = center.y;

		/// 그림자가 드리워지는 지형 범위를 계산
		unsigned int xbegin, ybegin, xend, yend;
		CalcRange( &xbegin, &ybegin, &xend, &yend, x, y, r );

		// Calculate an approximate width for the shadow geometry -- used
		// in texture coordinate generation
		mOOWidth = 1.0f / (r * 2.0f);

		///
		NiPoint3 v[3];
		float x0, y0;

		for( unsigned int yi = ybegin; yi < yend; ++yi )
		{
			for( unsigned int xi = xbegin; xi < xend; ++xi )
			{
				x0 = xi * 100.0f;
				y0 = yi * 100.0f;

				/// 왼쪽 아래 삼각형
				v[0].x = x0;
				v[0].y = y0;
				v[0].z = heights[mLineCount * yi + xi];
				v[1].x = x0 + 100.0f;
				v[1].y = y0;
				v[1].z = heights[mLineCount * yi + xi+1];
				v[2].x = x0;
				v[2].y = y0 + 100.0f;
				v[2].z = heights[mLineCount * (yi+1) + xi];

				AddShadowTriangle( v, kPositionLock, kTexCoordLock );

				/// 오른쪽 위 삼각형
				v[0].x = x0 + 100.0f;
				v[0].y = y0 + 100.0f;
				v[0].z = heights[mLineCount * (yi+1) + xi+1];
				v[1].x = x0;
				v[1].y = y0 + 100.0f;
				v[1].z = heights[mLineCount * (yi+1) + xi];
				v[2].x = x0 + 100.0f;
				v[2].y = y0;
				v[2].z = heights[mLineCount * yi + xi+1];

				AddShadowTriangle( v, kPositionLock, kTexCoordLock );
			}
		}

		kTexCoordLock.Unlock();
		kPositionLock.Unlock();

		SetActiveCount( mActiveCount );
	}

	if( mAlpha != alpha )
	{
		mAlpha = alpha;

		NiMaterialProperty* pMat = (NiMaterialProperty*)mTris->GetProperty( NiProperty::MATERIAL );
		if( pMat )
			pMat->SetAlpha( mAlpha );
	}

	/// 장면 관리자의 그림자 가시 배열에 추가
	SCENEMAN->AddVisibleShadowGeom( this );
}

void cShadowGeometry::AddShadowTriangle( NiPoint3* v, NiDataStreamElementLock& kPositionLock, NiDataStreamElementLock& kTexCoordLock )
{
	if( (mActiveCount + 3) >= mMaxVerts )
	{
		//		assert( 0 );
		return;
	}

	NiTStridedRandomAccessIterator<NiPoint2> kTexCoordIter = kTexCoordLock.begin<NiPoint2>();
	for( unsigned int i=0; i<3; i++ )
	{
		NiPoint3 diff = v[i] - mCenter;
		kTexCoordIter[mActiveCount + i].x = (NiPoint3::UNIT_X*diff) * mOOWidth + 0.5f;
		kTexCoordIter[mActiveCount + i].y = (NiPoint3::UNIT_Y*diff) * mOOWidth + 0.5f;
	}

	/// 정점 좌표를 갱신
	v[0].z += mHeight;
	v[1].z += mHeight;
	v[2].z += mHeight;

	NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();
	kPositionIter[mActiveCount]		= v[0];
	kPositionIter[mActiveCount + 1] = v[1];
	kPositionIter[mActiveCount + 2] = v[2];

	mActiveCount += 3;
}

void cShadowGeometry::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, float x, float y, float radius )
{
	int xb = (int)((x - radius) / 100.0f);
	int yb = (int)((y - radius) / 100.0f);
	int xe = (int)((x + radius) / 100.0f) + 1;
	int ye = (int)((y + radius) / 100.0f) + 1;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe >= mLineCount )
		xe = mLineCount - 1;
	if( ye >= mLineCount )
		ye = mLineCount - 1;

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
}

void cShadowGeometry::SetTexture( NiTexture* tex )
{
	assert( tex );

	mTex = tex;

	NiTexturingProperty::Map* baseMap = NiNew NiTexturingProperty::Map( tex, 0, NiTexturingProperty::CLAMP_S_CLAMP_T, NiTexturingProperty::FILTER_BILERP );
	mTexProp->SetBaseMap( baseMap );
}

void cShadowGeometry::SetActiveCount( unsigned int count )
{
	if( mTris == 0 )
		return;

	NiDataStreamRef* pkRef = mTris->FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	pkRef = mTris->FindStreamRef( NiCommonSemantics::TEXCOORD() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	mTris->RecomputeBounds();
}
