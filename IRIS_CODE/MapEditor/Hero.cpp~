#include "stdafx.h"
#include "Hero.h"

#include "MapEditorApp.h"
#include "NaviFieldEditor.h"

#include "Engine/Box.h"
#include "Engine/LookAtCamera.h"
#include "Engine/NaviMesh.h"
#include "Engine/SoundSystem.h"
#include "Engine/ResourceManager.h"
#include "Engine/SceneManager.h"
#include "Engine/TrailGeometry.h"
#include "Engine/PathFinder.h"

cHero* cHero::mSingleton = 0;

cHero::cHero( cLookAtCamera* cam )
: mPos( NiPoint3::ZERO )
, mLevel0Lines( NiColorA(1.0f, 1.0f, 1.0f), NiColorA::BLACK )
, mLevel1Lines( NiColorA(1.0f, 1.0f, 0.0f), NiColorA::BLACK )
, mLevel2Lines( NiColorA(1.0f, 0.0f, 0.0f), NiColorA::BLACK )
, mCamera( cam )
, mNode( 0 )
, mBoundSphere( NiPoint3::ZERO, 0.0f )
//, mRibbonTexture( 0 )
//, mRibbonGeom( 0 )
, mNeedUpdate( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	assert( cam && "null camera" );

	mPathCount = 0;
	mPathIndex = 0;

	/// 길찾기 이동 경로 선을 생성
	mPathLines = NiNew NiMesh;
	mPathLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mPathLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, MAX_PATH_COUNT, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::BLACK );
	mMatProp->SetDiffuseColor( NiColor::BLACK );
	mMatProp->SetSpecularColor( NiColor::BLACK );
	mMatProp->SetEmittance( NiColor::WHITE );

	mVertColorProp = NiNew NiVertexColorProperty;
	mVertColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
	mVertColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E_A_D );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mPathLines->AttachProperty( mMatProp );
	mPathLines->AttachProperty( mVertColorProp );
	mPathLines->AttachProperty( mWireProp );
	mPathLines->UpdateProperties();
	mPathLines->Update( 0.0f );
}

cHero::~cHero()
{
	Clear();
	mSingleton = 0;
}

void cHero::Clear()
{
	if( mNode )
	{
		delete mNode;
		mNode = 0;
	}

	//if( mRibbonTexture )
	//{
	//	mRibbonTexture->Release();
	//	mRibbonTexture = 0;
	//}
}

bool cHero::Init()
{
	Clear();

	/// 현재 경로를 설정
	cString path = theApp.GetBaseDir();
	path += "MapData";
	SetCurrentDirectory( path.Cstr() );

	/// 노드를 생성
	mNode = new cPlayerSceneNode;

	/// 노드를 초기화
	cPlayerSceneNodeParam param;
	param.mPathName.Format( "%s/%s", path.Cstr(), "h_m.kfm");

	if( mNode->Init( param ) == false )
	{
		delete mNode;
		mNode = 0;

		cString msg;
		msg.Format( "Failed to init hero scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return false;
	}

	mBoundSphere = mNode->GetBoundSphere();

	return true;
}

void cHero::Reset()
{
	mNeedUpdate = true;

	NiPoint3 min = NAVIMESH->GetBoundBox().GetMin();
	NiPoint3 max = NAVIMESH->GetBoundBox().GetMax();
	mPos = (min + max) * 0.5f;

	/// 높이값을 계산
	NAVIMESH->CalcHeight( &mPos.z, mPos.x, mPos.y );

	/// 카메라의 시점을 설정
	mCamera->SetLookAt( mPos + NiPoint3(0.0f, 0.0f, 124.0f) );

	/// 노드를 갱신
	if( mNode )
	{
		mNode->SetTranslate( mPos );
		mNode->Update();

		mBoundSphere = mNode->GetBoundSphere();
	}
}

void cHero::Process( unsigned long deltaTime )
{
	if( mPathCount == 0 )
	{
		if( mNeedUpdate )
		{
			mNeedUpdate = false;

			/// 이동
			mNode->SetTranslate( mPos );

			mBoundSphere = mNode->GetBoundSphere();
		}
		return;
	}

	NiPoint2 nextPos = mPathArray[mPathIndex];

	/// 방향
	NiPoint2 dir1, dir2;
	dir1.x = nextPos.x - mPos.x;
	dir1.y = nextPos.y - mPos.y;
	dir1.Unitize();

	/// 이동
	float speed = 500.0f;
	mPos.x += dir1.x * (deltaTime*0.001f) * speed;
	mPos.y += dir1.y * (deltaTime*0.001f) * speed;
	mPos.z = 0.0f;

	/// 보정을 위해서 현재계산된 위치로의 거리를 계산한다.
	dir2.x = nextPos.x - mPos.x;
	dir2.y = nextPos.y - mPos.y;
	dir2.Unitize();

	if( dir2.Dot(dir1) <= 0.0f )
	{
		//mPos.x = nextPos.x;
		//mPos.y = nextPos.y;

		if( mPathIndex < mPathCount-1 )
		{
			mNeedUpdate = true;
			nextPos = mPathArray[++mPathIndex];
		}
		else
		{
			mNeedUpdate = false;
			mPathCount = 0;
			mPathIndex = 0;

			SetActiveLines(0);
		}
	}
	else
	{
		mNeedUpdate = true;
	}

	if( mNeedUpdate )
	{
		mNeedUpdate = false;

		/// 높이값을 계산
		NAVIMESH->CalcHeight( &mPos.z, mPos.x, mPos.y );

		/// 카메라의 시점을 설정
		mCamera->SetLookAt( mPos + NiPoint3(0.0f, 0.0f, 124.0f) );

		/// 노드를 갱신
		if( mNode )
		{
			/// 회전
			float dot = dir1.Dot( -NiPoint2::UNIT_Y );
			float ang = NiACos( dot );
			if( dir1.x >= 0.0f )
				ang = -ang;
			mNode->SetRotate( NiPoint3::UNIT_Z, ang );

			/// 이동
			mNode->SetTranslate( mPos );

			mBoundSphere = mNode->GetBoundSphere();
		}

		/// 궤적
		//if( mRibbonGeom )
		//{
		//	mRibbonGeom->SetPushPoint( mPos, mPos + NiPoint3(0.0f, 0.0f, 124.0f) );
		//}
	}
}

void cHero::RenderLines()
{
	///
	mLevel0Lines.Process( mPos, SCENEMAN->GetLevelDistance0(), NAVIMESH );
	mLevel1Lines.Process( mPos, SCENEMAN->GetLevelDistance1(), NAVIMESH );
	mLevel2Lines.Process( mPos, SCENEMAN->GetLevelDistance2(), NAVIMESH );
	mLevel0Lines.Render();
	mLevel1Lines.Render();
	mLevel2Lines.Render();

	if( mPathCount != 0 )
		mPathLines->RenderImmediate( NiRenderer::GetRenderer() );
}

void cHero::SetTranslate( const NiPoint3& pos )
{
	mPos = pos;
	mPathCount = 0;

	/// 카메라의 시점을 설정
	mCamera->SetLookAt( mPos + NiPoint3(0.0f, 0.0f, 124.0f) );

	/// 노드를 갱신
	mNeedUpdate = true;
}

void cHero::SetTargetPos( const NiPoint3& pos )
{
	/// 거리 제한
	//if( (mPos - pos).Length() > 30000.0f )
	//	return;

	/// 길찾기
	if( PATHFINDER->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, NiPoint2(mPos.x, mPos.y), NiPoint2(pos.x, pos.y) ) )
	{
		mPathIndex = 0;
		SetActiveLines(0);

		NiDataStreamElementLock kPositionLock( mPathLines, NiCommonSemantics::POSITION(), 0,
			NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
		assert( kPositionLock.IsLocked() );

		NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

		///
		NAVIMESH->CalcHeight( &(mPos.z), mPos.x, mPos.y );
		mPos.z += 10.0f;
		kPositionIter[0] = mPos;

		NiPoint3 p;
		unsigned int i = 0;
		unsigned int iend = mPathCount < MAX_PATH_COUNT - 1 ? mPathCount : MAX_PATH_COUNT - 1;

		for( ; i < iend; ++i )
		{
			p.x = mPathArray[i].x;
			p.y = mPathArray[i].y;

			NAVIMESH->CalcHeight( &(p.z), p.x, p.y );
			p.z += 100.0f;
			kPositionIter[2*i + 1] = kPositionIter[2*i + 2] = p;
		}

		kPositionLock.Unlock();

		SetActiveLines(2*iend + 1);
	}
}

void cHero::SetActiveLines( unsigned int count )
{
	if( mPathLines == 0 )
		return;

	NiDataStreamRef* pkRef = mPathLines->FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);
}
