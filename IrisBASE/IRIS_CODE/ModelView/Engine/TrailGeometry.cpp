#include "stdafx.h"
#include "TrailGeometry.h"

#include "RenderSystem.h"
#include "CameraManager.h"

cRibbonGeometry::cRibbonGeometry()
: mMaxVerts( 0 )
, mActiveVerts( 0 )
, mPushInterval( 0 )
, mPushEnabled( true )
, mUpdatePoint( false )
, mPushChekTime(0)
{
	mCheckAddPoint = 0;
}

cRibbonGeometry::~cRibbonGeometry()
{
	Clear();
}

void cRibbonGeometry::Clear()
{
	NiTListIterator pos = mTrailPointList.GetHeadPos();
	while( pos )
	{
		sPointInfo* p = mTrailPointList.GetNext(pos);
		SAFE_DELETE( p );
	}
	mTrailPointList.RemoveAll();

	///
	mMaxVerts = 0;
	mActiveVerts = 0;
	mPushInterval = 0;

	mUpdatePoint = false;
}

//
bool cRibbonGeometry::Init( unsigned long accumTime, NiTexture* texture, unsigned int maxVerts, unsigned long pushInterval )
{
	assert( maxVerts >= 4 );

	mMaxVerts = maxVerts;
	mPushInterval = pushInterval;
	mPushChekTime = accumTime + mPushInterval;

	/// 메쉬 데이터를 생성
	SetPrimitiveType(NiPrimitiveType::PRIMITIVE_TRISTRIPS);

	/// 프로퍼티 정보 생성
	NiAlphaProperty* pkAlpha = NiNew NiAlphaProperty;
	pkAlpha->SetAlphaBlending(true);
	pkAlpha->SetAlphaTesting(false);
	pkAlpha->SetSrcBlendMode( NiAlphaProperty::ALPHA_ONE );
	pkAlpha->SetDestBlendMode( NiAlphaProperty::ALPHA_ONE );
	AttachProperty(pkAlpha);

	//	NiMaterialProperty* pkMat = NiNew NiMaterialProperty;
	//	pkMat->SetAlpha(1.0f);
	//	AttachProperty(pkMat);

	NiStencilProperty* pkStencil = NiNew NiStencilProperty;
	pkStencil->SetDrawMode( NiStencilProperty::DRAW_BOTH );
	AttachProperty(pkStencil);

	NiFogProperty* pkFog = NiNew NiFogProperty;
	pkFog->SetFog( false );
	AttachProperty(pkFog);

	NiTexturingProperty* texProp = NiNew NiTexturingProperty;
	texProp->SetBaseTexture(texture);
	texProp->SetBaseFilterMode(NiTexturingProperty::FILTER_TRILERP);
	texProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
	texProp->SetBaseClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T);
	AttachProperty( texProp );

	NiVertexColorProperty *pkVCProp = NiNew NiVertexColorProperty(); 
	pkVCProp->SetSourceMode(NiVertexColorProperty::SOURCE_EMISSIVE);
	pkVCProp->SetLightingMode(NiVertexColorProperty::LIGHTING_E);
	AttachProperty( pkVCProp );

	UpdateProperties();

	AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, mMaxVerts, 
		NiDataStream::ACCESS_CPU_READ | NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	AddStream( NiCommonSemantics::TEXCOORD(), 0, NiDataStreamElement::F_FLOAT32_2, mMaxVerts, 
		NiDataStream::ACCESS_CPU_READ | NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	AddStream( NiCommonSemantics::COLOR(), 0, NiDataStreamElement::F_FLOAT32_4, mMaxVerts, 
		NiDataStream::ACCESS_CPU_READ | NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	/// 초기화
	SetSubmeshCount( 1 );
	SetActiveCount( 0 );

	NiMeshUpdateProcess kUpdateProcess;
	Update( kUpdateProcess );

	return true;
}

void cRibbonGeometry::SetActiveCount( unsigned int count )
{
	NiDataStreamRef* pkRef = FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	pkRef = FindStreamRef( NiCommonSemantics::TEXCOORD() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	pkRef = FindStreamRef( NiCommonSemantics::COLOR() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	mActiveVerts = count;

	//	RecomputeBounds();
}


void cRibbonGeometry::PushTrailPoint( unsigned long pushTime, const NiPoint3& v0, const NiPoint3& v1 )
{
	/// 정점 범위 체크
	if( mTrailPointList.GetSize() * 4 > mMaxVerts )
		return;

	if( mPushEnabled == false )
	{
		if( mCheckAddPoint > MIN_ADDPOINT_CNT )
			return;
	}

	if( mPushChekTime > pushTime )
		return;

	mPushChekTime = pushTime + mPushInterval;

	if( mLastPos[0] == v0 && mLastPos[1] == v1 )
		return;

	float addAlpha = 1.0f;
	unsigned int cnt = mTrailPointList.GetSize();
	if( cnt == 0 )
	{
		sPointInfo* info = new sPointInfo;
		info->mPoint[0] = v0;
		info->mPoint[1] = v1;

		info->mDir21 = info->mPoint[0]-info->mPoint[1];
		info->mDirLen = info->mDir21.Unitize();

		info->mAlpha = addAlpha+0.05f;

		/// insert
		mTrailPointList.AddHead( info );
		mCheckAddPoint++;
	}
	else if( cnt == 1 )
	{
		sPointInfo* info = new sPointInfo;
		info->mPoint[0] = v0;
		info->mPoint[1] = v1;

		info->mDir21 = info->mPoint[0]-info->mPoint[1];
		info->mDirLen = info->mDir21.Unitize();

		info->mAlpha = addAlpha+0.05f;

		/// insert
		mTrailPointList.AddHead( info );
		mCheckAddPoint++;

		mDirection[0] = v0 - mLastPos[0];
		mDirection[0].Unitize();

		mDirection[1] = v1 - mLastPos[1];
		mDirection[1].Unitize();
	}
	else
	{
		NiPoint3 dirS = v0 - mLastPos[0];
		NiPoint3 dirE = v1 - mLastPos[1];
		float lenS = dirS.Unitize();
		float lenE = dirE.Unitize();

		NiPoint3 oldPos[2];
		NiPoint3 addPos[2];
		float ds = 10.0f;
		for( float i = 0; i<=ds; i++ )
		{
			float di = i/ds;

			oldPos[0] = addPos[0];
			oldPos[1] = addPos[1];

			NiPoint3 dirU = (1.0f-di)*mDirection[0] + di*dirS;
			dirU.Unitize();
			addPos[0] = mLastPos[0] + (lenS*di)*dirU;

			dirU = (1.0f-di)*mDirection[1] + di*dirE;
			dirU.Unitize();
			addPos[1] = mLastPos[1] + (lenE*di)*dirU;

			sPointInfo* info = new sPointInfo;
			info->mPoint[0] = addPos[0];
			info->mPoint[1] = addPos[1];

			info->mDir21 = info->mPoint[0]-info->mPoint[1];
			info->mDirLen = info->mDir21.Unitize();

			info->mAlpha = addAlpha+0.05f;

			mTrailPointList.AddHead( info );
			mCheckAddPoint++;
		}

		mDirection[0] = addPos[0] - oldPos[0];
		mDirection[0].Unitize();

		mDirection[1] = addPos[1] - oldPos[1];
		mDirection[1].Unitize();
	}

	mLastPos[0] = v0;
	mLastPos[1] = v1;

	if( mTrailPointList.GetSize() > 1 )
		mUpdatePoint = true;
}

void cRibbonGeometry::Process( unsigned long deltaTime )
{
	/// delete Point check
	float da = 0.005f*(float)deltaTime;

	NiTListIterator kIter = mTrailPointList.GetHeadPos();
	while( kIter )
	{
		NiTListIterator kOldIter = kIter;
		sPointInfo* p = mTrailPointList.GetNext(kIter);
		p->UpdateAlpha( da );

		if( p->mAlpha <= 0.0f )
		{
			SAFE_DELETE( p );
			mTrailPointList.RemovePos(kOldIter);

			mUpdatePoint = true;
		}
	}

	/// current point count check
	unsigned int pointCnt = mTrailPointList.GetSize();
	if( pointCnt <= 1)
	{
		SetActiveCount( 0 );
		mUpdatePoint = false;
		return;
	}

	/// trail update
	if( mUpdatePoint == true )
	{
		NiDataStreamElementLock kPositionLock( this, NiCommonSemantics::POSITION(), 0,
			NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
		assert( kPositionLock.IsLocked() );

		NiDataStreamElementLock kTexCoordLock( this, NiCommonSemantics::TEXCOORD(), 0,
			NiDataStreamElement::F_FLOAT32_2, NiDataStream::LOCK_WRITE );
		assert( kTexCoordLock.IsLocked() );

		NiDataStreamElementLock kColorLock( this, NiCommonSemantics::COLOR(), 0,
			NiDataStreamElement::F_FLOAT32_4, NiDataStream::LOCK_WRITE );
		assert( kColorLock.IsLocked() );

		SetActiveCount( 0 );

		NiTStridedRandomAccessIterator<NiPoint3> kPositionIter = kPositionLock.begin<NiPoint3>();
		NiTStridedRandomAccessIterator<NiPoint2> kTexCoordIter = kTexCoordLock.begin<NiPoint2>();
		NiTStridedRandomAccessIterator<NiColorA> kColorIter = kColorLock.begin<NiColorA>();

		NiTListIterator pos = mTrailPointList.GetHeadPos();

		sPointInfo* prePointInfo = mTrailPointList.GetNext(pos);

		if( prePointInfo == 0 )
			return;

		float alpha =prePointInfo->mAlpha;
		if( alpha > 1.0f )
			alpha = 1.0f;

		kPositionIter[0] = prePointInfo->mPoint[0];
		kPositionIter[1] = prePointInfo->mPoint[1];

		kTexCoordIter[0] = NiPoint2( 0.0f, 0.0f );
		kTexCoordIter[1] = NiPoint2( 0.0f, 1.0f );

		kColorIter[0] = kColorIter[1] = NiColorA(1.0f,1.0f, 1.0f, prePointInfo->mAlpha);

		unsigned int i = 0;
		unsigned int uvIdx = 0;
		while( pos )
		{
			sPointInfo* p = mTrailPointList.GetNext(pos);
			float uv1 = (float)(uvIdx+1) / (float)(pointCnt-1);

			/// 면 
			kPositionIter[i+2] = p->mPoint[0];
			kPositionIter[i+3] = p->mPoint[1];

			kTexCoordIter[i+2] = NiPoint2( uv1, 0.0f );
			kTexCoordIter[i+3] = NiPoint2( uv1, 1.0f );

			alpha = p->mAlpha;
			if( alpha > 1.0f )
				alpha = 1.0f;

			kColorIter[i+2] = kColorIter[i+3] = NiColorA(1.0f,1.0f, 1.0f, p->mAlpha);
			i+=2;

			uvIdx++;
		}

		SetActiveCount( pointCnt*2 );

		kColorLock.Unlock();
		kTexCoordLock.Unlock();
		kPositionLock.Unlock();

		UpdateBound();
	}
	else
	{
		/// 정점추가가 없는경우 - 정점 알파 갱신
		NiDataStreamElementLock kColorLock( this, NiCommonSemantics::COLOR(), 0,
			NiDataStreamElement::F_FLOAT32_4, NiDataStream::LOCK_WRITE );
		assert( kColorLock.IsLocked() );

		NiTStridedRandomAccessIterator<NiColorA> kColorIter = kColorLock.begin<NiColorA>();

		NiTListIterator pos = mTrailPointList.GetHeadPos();
		sPointInfo* prePointInfo = mTrailPointList.GetNext(pos);

		if( prePointInfo == 0 )
			return;

		float alpha =prePointInfo->mAlpha;
		if( alpha > 1.0f )
			alpha = 1.0f;

		kColorIter[0] = kColorIter[1] = NiColorA(1.0f,1.0f, 1.0f, alpha);

		unsigned int i = 0;
		while( pos )
		{
			sPointInfo* p = mTrailPointList.GetNext(pos);

			alpha = p->mAlpha;
			if( alpha > 1.0f )
				alpha = 1.0f;

			kColorIter[i+2] = kColorIter[i+3] = NiColorA(1.0f,1.0f, 1.0f, alpha);
			i+=2;
		}

		kColorLock.Unlock();
	}

	mUpdatePoint = false;
}

void cRibbonGeometry::UpdateBound()
{
	if( mActiveVerts == 0 )
		return;

	const NiDataStream* pkVertexDS = GetVertexDataStream();
	const NiPoint3* pkVertex = (const NiPoint3*)
		((NiDataStream*)pkVertexDS)->Lock(NiDataStream::LOCK_READ);
	m_kBound.ComputeFromData(mActiveVerts, pkVertex);
	((NiDataStream*)pkVertexDS)->Unlock(NiDataStream::LOCK_READ);
}

const NiDataStream* cRibbonGeometry::GetVertexDataStream()
{
	const NiDataStreamRef* pkRef;
	NiDataStreamElement kElement;
	NIVERIFY(FindStreamRefAndElementBySemantic(NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3,pkRef, kElement));
	NIASSERT(GetSubmeshCount() == 1);
	NIASSERT(pkRef->GetRegionIndexForSubmesh(0) == 0);
	return pkRef->GetDataStream();
}


void cRibbonGeometry::RenderImmediate( NiRenderer* renderer )
{
	if( mActiveVerts < 4 )
		return;

	NiMesh::RenderImmediate( renderer );
}