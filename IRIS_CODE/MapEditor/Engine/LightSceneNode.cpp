#include "stdafx.h"
#include "LightSceneNode.h"

#include "Ray.h"
#include "Sphere.h"
#include "TerrainNode.h"

cLightSceneNode::cLightSceneNode( eSceneNodeType type )
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
{
}

cLightSceneNode::~cLightSceneNode()
{
	for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = mTerrainArray[i];

		n->mLight = 0;
		n->mIndexByLight = 0xFFFFFFFF;
	}
}

void cLightSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	bool needUpdate = mNeedUpdateTransform;
	cSceneNode::Process( deltaTime, accumTime );

	if( needUpdate )
	{
		for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
		{
			cTerrainLeafNode* n = mTerrainArray[i];

			n->mLight = 0;
			n->mIndexByLight = 0xFFFFFFFF;
		}

		///
		mTerrainArray.Clear();
		mTerrainArray.Reserve(9);

		if( TERRAIN->CollideSphere( &mTerrainArray, mBoundSphere ) )
		{
			for( unsigned int i = 0, iend = mTerrainArray.GetSize(); i < iend; ++i )
			{
				cTerrainLeafNode* n = mTerrainArray[i];
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

bool cLightSceneNode::Save( cFileSaver& saver )
{
	/// 상위 정보(이름, 변환 행렬)를 저장
	if( cSceneNode::Save( saver ) == false )
		return false;

	/// 반지름을 저장
	saver.WriteFloat( mRadius );

	/// 주변색을 저장
	saver.Write( &(mLight->GetAmbientColor()), sizeof(NiColor) );

	/// 산란색을 저장
	saver.Write( &mDiffuse0, sizeof(NiColor) );
	saver.Write( &mDiffuse1, sizeof(NiColor) );
	saver.WriteFloat( mDiffuseAnimTime );
	saver.WriteUnsignedInt( mFlickering );

	/// 반사색을 저장
	saver.Write( &(mLight->GetSpecularColor()), sizeof(NiColor) );

	/// 감쇠 계수들을 저장
	saver.WriteFloat( mConstantAtten0 );
	saver.WriteFloat( mConstantAtten1 );
	saver.WriteFloat( mConstantAttenAnimTime );
	saver.WriteFloat( mLight->GetLinearAttenuation() );
	saver.WriteFloat( mLight->GetQuadraticAttenuation() );
	return true;
}

bool cLightSceneNode::Init( const cLightSceneNodeParam& param )
{
	/// 엔진 노드를 생성
	NiPointLight* o = NiNew NiPointLight;
	mLight = o;
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
	o->SetAmbientColor( param.mAmbient );
	o->SetDiffuseColor( param.mDiffuse0 );
	o->SetSpecularColor( param.mSpecular );

	/// 감쇠 계수를 설정
	o->SetConstantAttenuation( param.mConstantAtten0 );
	o->SetLinearAttenuation( param.mLinearAtten );
	o->SetQuadraticAttenuation( param.mQuadricAtten );

	/// 갱신 선택 사항을 설정
	o->SetSelectiveUpdate( true );
	o->SetSelectiveUpdateTransforms( true );
	o->SetSelectiveUpdatePropertyControllers( true );
	o->SetSelectiveUpdateRigid( false );
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
		mTerrainArray[i]->mIndexByLight = i;
	}

	node->mLight = 0;
	node->mIndexByLight = 0xFFFFFFFF;
}

void cLightSceneNode::DetachAll()
{
	mLight->DetachAllAffectedNodes();
	mLight->DetachAllUnaffectedNodes();

	cAffectedSet::cIterator i = mAffectedSet.Begin();
	cAffectedSet::cIterator end = mAffectedSet.End();

	for( ; i != end; ++i )
	{
		cSceneNode* n = *i;
		n->RemoveLight( this );
		n->SetNeedUpdateTransform( false );
		n->SetNeedUpdateEffects( true );
		n->Update();
	}
	mAffectedSet.Clear();
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

void cLightSceneNode::SetDiffuseAnimTime( float time )
{
	mDiffuseAnimTime = time;

	if( time == 0.0f )
		mLight->SetDiffuseColor( mDiffuse0 );
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

void cLightSceneNode::SetConstantAttenAnimTime( float time )
{
	mConstantAttenAnimTime = time;

	if( time == 0.0f )
		mLight->SetConstantAttenuation( mConstantAtten0 );
}

void cLightSceneNode::SetLinearAtten( float value )
{
	mLight->SetLinearAttenuation( value );
}

void cLightSceneNode::SetQuadricAtten( float value )
{
	mLight->SetQuadraticAttenuation( value );
}
