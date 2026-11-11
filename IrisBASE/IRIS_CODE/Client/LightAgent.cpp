#include "stdafx.h"
#include "LightAgent.h"

cLightAgent* cLightAgent::mpLightAgent = 0;

cLightAgent::cLightAgent()
{
	mpLightAgent = this;

	mTerrainAmbientLight = 0;

	mSkyAmbientLight = 0;

	mSceneAmbientLight[0] = mSceneAmbientLight[1] = 0;
	mSceneDirLight[0] = mSceneDirLight[1] = 0;

	mSelectAmbientLight = 0;
}

cLightAgent::~cLightAgent()
{
	assert( mSkyAmbientLight->GetRefCount() == 1 );
	mSkyAmbientLight = 0;

	assert( mTerrainAmbientLight->GetRefCount() == 1 );
	mTerrainAmbientLight = 0;

	assert( mSceneAmbientLight[0]->GetRefCount() == 1 );
	mSceneAmbientLight[0] = 0;

	assert( mSceneAmbientLight[1]->GetRefCount() == 1 );
	mSceneAmbientLight[1] = 0;

	assert( mSceneDirLight[0]->GetRefCount() == 1 );
	mSceneDirLight[0] = 0;

	assert( mSceneDirLight[1]->GetRefCount() == 1 );
	mSceneDirLight[1] = 0;

	assert( mSelectAmbientLight->GetRefCount() == 1 );
	mSelectAmbientLight = 0;
}

void cLightAgent::Close()
{
	/// 지형 주변광을 생성
	mTerrainAmbientLight->SetAmbientColor( NiColor(0.8549f, 0.8549f, 0.8549f ) );//NiColor::WHITE );
	mTerrainAmbientLight->SetDiffuseColor( NiColor::BLACK );//NiColor(0.90f, 0.90f, 0.90f ) );//NiColor::WHITE );
	mTerrainAmbientLight->SetDimmer( 1.0f );

	mSkyAmbientLight->SetAmbientColor( NiColor::WHITE );
	mSkyAmbientLight->SetDimmer( 1.0f );

	/// 장면 주변광을 생성
	for( unsigned int i = 0; i < 2; ++i )
	{
		NiAmbientLight* p = mSceneAmbientLight[i];
		if( p )
		{
			p->SetAmbientColor( NiColor(0.8f, 0.8f, 0.8f ) );
			p->SetDiffuseColor( NiColor::WHITE );
			p->SetSpecularColor( NiColor::BLACK );
			p->SetDimmer( 1.0f );
		}
	}

	/// 장면 방향광을 생성
	NiMatrix3 rot;
	rot.MakeIdentity();
	rot.SetCol( 0, 0.0f, 0.0f, -1.0f );

	for( unsigned int i = 0; i < 2; ++i )
	{
		NiDirectionalLight* p = mSceneDirLight[i];
		if( p )
		{
			p->SetAmbientColor( NiColor(0.8f, 0.8f, 0.8f ) );
			p->SetDiffuseColor( NiColor::WHITE );
			p->SetSpecularColor( NiColor::WHITE );
			p->SetSpecularColor( NiColor::BLACK );
			p->SetDimmer( 1.0f );

			p->SetRotate( rot );
			p->Update(0.0f);
		}
	}
}

bool cLightAgent::Init()
{
	/// 지형 주변광을 생성
	mTerrainAmbientLight = NiNew NiAmbientLight;
	mTerrainAmbientLight->SetAmbientColor( NiColor(0.8549f, 0.8549f, 0.8549f ) );//NiColor::WHITE );
	mTerrainAmbientLight->SetDiffuseColor( NiColor::BLACK );//NiColor(0.90f, 0.90f, 0.90f ) );//NiColor::WHITE );
	mTerrainAmbientLight->SetDimmer( 1.0f );

	/// 하늘 주변광을 생성
	mSkyAmbientLight = NiNew NiAmbientLight;
	mSkyAmbientLight->SetAmbientColor( NiColor::WHITE );
	mSkyAmbientLight->SetDimmer( 1.0f );

	/// 장면 주변광을 생성
	mSceneAmbientLight[0] = NiNew NiAmbientLight;
	mSceneAmbientLight[0]->SetAmbientColor( NiColor::WHITE );
	mSceneAmbientLight[0]->SetDiffuseColor( NiColor::WHITE );
	mSceneAmbientLight[0]->SetSpecularColor( NiColor::BLACK );

	mSceneAmbientLight[1] = NiNew NiAmbientLight;
	mSceneAmbientLight[1]->SetAmbientColor( NiColor::WHITE );
	mSceneAmbientLight[1]->SetDiffuseColor( NiColor::WHITE );
	mSceneAmbientLight[1]->SetSpecularColor( NiColor::BLACK );

	/// 장면 방향광을 생성
	NiMatrix3 rot;
	rot.MakeIdentity();
	rot.SetCol( 0, 0.0f, 0.0f, -1.0f );
	for( unsigned int i = 0; i < 2; ++i )
	{
		mSceneDirLight[i] = NiNew NiDirectionalLight;
		mSceneDirLight[i]->SetDimmer( 0.5f );
		mSceneDirLight[i]->SetAmbientColor( NiColor::WHITE );
		mSceneDirLight[i]->SetDiffuseColor( NiColor::WHITE );
		mSceneDirLight[i]->SetSpecularColor( NiColor::BLACK );
		mSceneDirLight[i]->SetRotate( rot );
		mSceneDirLight[i]->Update(0.0f);
	}

	/// attach mouse over scene node & else detach
	mSelectAmbientLight = NiNew NiAmbientLight;
	mSelectAmbientLight->SetAmbientColor( NiColor(0.2f, 0.2f, 0.2f) );
	mSelectAmbientLight->SetDiffuseColor( NiColor(0.2f, 0.2f, 0.2f) );
	mSelectAmbientLight->SetSpecularColor( NiColor::WHITE );

	return true;
}

void cLightAgent::AttachSkyLight( NiNode* pnode )
{
	if( pnode )
	{
		pnode->AttachEffect( mSkyAmbientLight );
	}
}

void cLightAgent::SetSkyAmbientLight( const NiColor& ambient, float dimmer )
{
	mSkyAmbientLight->SetAmbientColor( ambient );
	mSkyAmbientLight->SetDimmer( dimmer );
}

void cLightAgent::SetSkyAmbient( const NiColor& ambient )
{
	mSkyAmbientLight->SetAmbientColor( ambient );
}

void cLightAgent::SetSkyDimmer( float dimmer )
{
	mSkyAmbientLight->SetDimmer( dimmer );
}

void cLightAgent::SetTerrainAmbientLight( const NiColor& ambient, float dimmer )
{
	mTerrainAmbientLight->SetAmbientColor( ambient );
	mTerrainAmbientLight->SetDimmer( dimmer );
}

void cLightAgent::SetTerrainAmbient( const NiColor& ambient )
{
	mTerrainAmbientLight->SetAmbientColor( ambient );
}

void cLightAgent::SetTerrainDimmer( float dimmer )
{
	mTerrainAmbientLight->SetDimmer( dimmer );
}

void cLightAgent::SetSceneAmbient( unsigned int i, const NiColor& ambient )
{
	assert( i == 0 || i == 1 );

	mSceneAmbientLight[i]->SetAmbientColor( ambient );
	mSceneAmbientLight[i]->Update(0.0f);
}

void cLightAgent::SetSceneDimmer( unsigned int i, float dimmer )
{
	assert( i == 0 || i == 1 );

	mSceneAmbientLight[i]->SetDimmer( dimmer );
	mSceneAmbientLight[i]->Update(0.0f);
}
