#include "stdafx.h"
#include "LightSceneNode.h"

#include "Ray.h"
#include "Sphere.h"
#include "TerrainNode.h"

cLightSceneNode::cLightSceneNode( eType type )
: cSceneNode( type )
, mRadius( 0.0f )
, mDiffuse0( NiColor::WHITE )
, mDiffuse1( NiColor::WHITE )
, mDiffuseAnimTime( 0.0f )
, mDiffuseTime( 0.0f )
, mDiffuseDir( false )
, mFlickering( false )
, mConstantAtten0( 0.0f )
, mConstantAtten1( 0.0f )
, mConstantAttenAnimTime( 0.0f )
, mConstantAttenTime( 0.0f )
, mConstantAttenDir( false )
, mAffectedSet( 16 )
, mLight(0)
{
	mTerrainArray.Clear();
}

cLightSceneNode::~cLightSceneNode()
{
	for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = (cTerrainLeafNode*)mTerrainArray[i];

		n->mLight = 0;
		n->mIndexByLight = 0xFFFFFFFF;
	}

	mPick.RemoveTarget();

	if( mLight )
	{
		assert( mLight->GetRefCount() == 1 );
		mLight = 0;
	}
}

void cLightSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	bool needUpdate = mNeedUpdateTransform;
	cSceneNode::OnProcess( deltaTime, accumTime );

	if( needUpdate )
	{
		for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
		{
			cTerrainLeafNode* n = (cTerrainLeafNode*)mTerrainArray[i];

			n->mLight = 0;
			n->mIndexByLight = 0xFFFFFFFF;
		}

		///
		mTerrainArray.Clear();
		mTerrainArray.Reserve( 9 );

		if( TERRAIN->CollideSphere( &mTerrainArray, mBoundSphere ) )
		{
			for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
			{
				cTerrainLeafNode* n = (cTerrainLeafNode*)mTerrainArray[i];
				n->SetLight( this, i );
			}
		}
	}

	float dt = float(deltaTime) * 0.001f;
	if( mDiffuseAnimTime != 0.0f )
	{
		if( mDiffuseDir )
			mDiffuseTime += dt;
		else
			mDiffuseTime -= dt;

		if( mDiffuseTime > mDiffuseAnimTime )
		{
			mDiffuseTime = mDiffuseAnimTime;
			mDiffuseDir = false;
		}
		else if( mDiffuseTime < 0.0f )
		{
			mDiffuseTime = 0.0f;
			mDiffuseDir = true;
		}

		float r = mDiffuseTime / mDiffuseAnimTime;
		mLight->SetDiffuseColor( mDiffuse0 + (mDiffuse1 - mDiffuse0) * r );
	}

	if( mConstantAttenAnimTime != 0.0f )
	{
		if( mConstantAttenDir )
			mConstantAttenTime += dt;
		else
			mConstantAttenTime -= dt;

		if( mConstantAttenTime > mConstantAttenAnimTime )
		{
			mConstantAttenTime = mConstantAttenAnimTime;
			mConstantAttenDir = false;
		}
		else if( mConstantAttenTime < 0.0f )
		{
			mConstantAttenTime = 0.0f;
			mConstantAttenDir = true;
		}

		float r = mConstantAttenTime / mConstantAttenAnimTime;
		mLight->SetConstantAttenuation( mConstantAtten0 + (mConstantAtten1 - mConstantAtten0) * r );
	}
}

bool cLightSceneNode::Init( const cLightSceneNodeParam& param )
{
	/// 엔진 노드를 생성
	NiPointLight* n = NiNew NiPointLight;
	mLight = n;
	mRadius = param.mRadius;
	mDiffuse0 = param.mDiffuse0;
	mDiffuse1 = param.mDiffuse1;
	mConstantAtten0 = param.mConstantAtten0;
	mConstantAtten1 = param.mConstantAtten1;

	if( mDiffuse0 != mDiffuse1 )
		mDiffuseAnimTime = param.mDiffuseAnimTime;
	else
		mDiffuseAnimTime = 0.0f;

	if( mConstantAtten0 != mConstantAtten1 )
		mConstantAttenAnimTime = param.mConstantAttenAnimTime;
	else
		mConstantAttenAnimTime = 0.0f;

	mFlickering = param.mFlickering;

	/// 상위 초기화
	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// 색상을 설정
	n->SetAmbientColor( param.mAmbient );
	n->SetDiffuseColor( param.mDiffuse0 );
	n->SetSpecularColor( param.mSpecular );

	/// 감쇠 계수를 설정
	n->SetConstantAttenuation( param.mConstantAtten0 );
	n->SetLinearAttenuation( param.mLinearAtten );
	n->SetQuadraticAttenuation( param.mQuadricAtten );

	/// 갱신 선택 사항을 설정
	n->SetSelectiveUpdate( true );
	n->SetSelectiveUpdateTransforms( true );
	n->SetSelectiveUpdatePropertyControllers( true );
	n->SetSelectiveUpdateRigid( false );
	return true;
}

bool cLightSceneNode::Pick( const cRay& ray )
{
	return mBoundSphere.IntersectRay( &mPickPos, &mPickDistance, ray );
}

void cLightSceneNode::Attach( cSceneNode* node )
{
	assert( node );

	if( mAffectedSet.Find(node) == mAffectedSet.End() )
	{
		mLight->AttachAffectedNode( node->GetNiNode() );
		mAffectedSet.Insert( node );
	}
}

void cLightSceneNode::Detach( cSceneNode* node )
{
	assert( node );

	if( mAffectedSet.Find(node) != mAffectedSet.End() )
	{
		mLight->DetachAffectedNode( node->GetNiNode() );
		mAffectedSet.Erase( node );
	}
}

void cLightSceneNode::Detach( cTerrainLeafNode* node )
{
	assert( node );

	/// 지형 배열에서 삭제
	unsigned int i = node->mIndexByLight;
	bool orderChanged = false;
	mTerrainArray.PopAt( &orderChanged, i );

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 지형의 인덱스를 재설정
		cTerrainLeafNode* n = (cTerrainLeafNode*)mTerrainArray[i];
		n->mIndexByLight = i;
	}

	node->mLight = 0;
	node->mIndexByLight = 0xFFFFFFFF;
}

void cLightSceneNode::SetAmbient( const NiColor& color )
{
	mLight->SetAmbientColor( color );
}

void cLightSceneNode::SetDiffuse0( const NiColor& color )
{
	mDiffuse0 = color;
	mLight->SetDiffuseColor( color );
}

void cLightSceneNode::SetDiffuse1( const NiColor& color )
{
	mDiffuse1 = color;
}

void cLightSceneNode::SetSpecular( const NiColor& color )
{
	mLight->SetSpecularColor( color );
}

void cLightSceneNode::SetConstantAtten0( float value )
{
	mConstantAtten0 = value;
	mLight->SetConstantAttenuation( value );
}

void cLightSceneNode::SetConstantAtten1( float value )
{
	mConstantAtten1 = value;
}

void cLightSceneNode::SetLinearAtten( float value )
{
	mLight->SetLinearAttenuation( value );
}

void cLightSceneNode::SetQuadricAtten( float value )
{
	mLight->SetQuadraticAttenuation( value );
}
