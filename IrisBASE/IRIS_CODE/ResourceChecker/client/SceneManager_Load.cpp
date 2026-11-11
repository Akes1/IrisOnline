#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "SoundSystem.h"
#include "ResourceManager.h"
// #include "OptionManager.h"
// 
// #include "LightAgent.h"
#include "WorldManager.h"
// #include "CameraManager.h"
#include "GameResourceManager.h"
 
#include "AreaGroup.h"
#include "AreaSceneNode.h"
#include "LightSceneNode.h"
#include "SoundSceneNode.h"
#include "StaticSceneNode.h"
#include "Application.h"
// 
// #include "ObjectManager.h"
// #include "Hero.h"

#include "FileSystem.h"

static const char* gSceneFileCode = "IrisScene";

static unsigned int gNodeNum = 0;
static unsigned int gLoadNodeNum = 0;

static unsigned int gSoundNodeNum = 0;
static unsigned int gLoadSoundNodeNum = 0;


bool cSceneManager::LoadEnv6( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );
//	CAMERAMAN->GetCurrent()->SetFarDistance( camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;

	cString pathName;
	pathName.Format( "./data/map/%s", name );
	WORLDMAN->LoadSkydome( pathName, 1.0f );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyHeight = 0.f;
	loader.ReadFloat( &skyHeight );
	//WORLDMAN->SetSkyHeight( skyHeight );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 0 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 
// 	NiMeshUpdateProcess kUpdateProcess;
// 	light->Update( kUpdateProcess );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	light = LIGHTAGENT->GetSceneDirLight( 1 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );
	return true;
}

bool cSceneManager::LoadGlobalArea6( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetSkyAmbient( ambient );
	mGlobalArea->SetSkyDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetTerrainAmbient( ambient );
	mGlobalArea->SetTerrainDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetStaticObjectAmbient( ambient );
	mGlobalArea->SetStaticObjectDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetDynamicObjectAmbient( ambient );
	mGlobalArea->SetDynamicObjectDimmer( dimmer );

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );
	mGlobalArea->SetFogColor( fogColor );
	mGlobalArea->SetFogDepth( fogDepth );

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadEnv7( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );
//	CAMERAMAN->GetCurrent()->SetFarDistance( camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;
	pathName.Format( "./data/map/%s", name );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );
	WORLDMAN->LoadSkydome( pathName, 0.4f );//skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	NiMeshUpdateProcess kUpdateProcess;
// 	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 0 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	light = LIGHTAGENT->GetSceneDirLight( 1 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );
	return true;
}

bool cSceneManager::LoadGlobalArea7( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetSkyAmbient( ambient );
	mGlobalArea->SetSkyDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetTerrainAmbient( ambient );
	mGlobalArea->SetTerrainDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetStaticObjectAmbient( ambient );
	mGlobalArea->SetStaticObjectDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetDynamicObjectAmbient( ambient );
	mGlobalArea->SetDynamicObjectDimmer( dimmer );

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );
	mGlobalArea->SetFogColor( fogColor );
	mGlobalArea->SetFogDepth( fogDepth );

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadEnv8( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );
//	CAMERAMAN->GetCurrent()->SetFarDistance( camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );
//	SetLevelDistance( dist0, dist1, dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;
	pathName.Format( "./data/map/%s", name );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );
	WORLDMAN->LoadSkydome( pathName, skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	NiMeshUpdateProcess kUpdateProcess;
// 	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 0 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	light = LIGHTAGENT->GetSceneDirLight( 1 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );
	return true;
}

bool cSceneManager::LoadGlobalArea8( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetSkyAmbient( ambient );
	mGlobalArea->SetSkyDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetTerrainAmbient( ambient );
	mGlobalArea->SetTerrainDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetStaticObjectAmbient( ambient );
	mGlobalArea->SetStaticObjectDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetDynamicObjectAmbient( ambient );
	mGlobalArea->SetDynamicObjectDimmer( dimmer );

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );
	mGlobalArea->SetFogColor( fogColor );
	mGlobalArea->SetFogDepth( fogDepth );

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadEnv9( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );
//	CAMERAMAN->GetCurrent()->SetFarDistance( camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );
//	SetLevelDistance( dist0, dist1, dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;
	pathName.Format( "./data/map/%s", name );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );
	WORLDMAN->LoadSkydome( pathName, skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	NiMeshUpdateProcess kUpdateProcess;
// 	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 0 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	light = LIGHTAGENT->GetSceneDirLight( 1 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );
	return true;
}

bool cSceneManager::LoadGlobalArea9( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetSkyAmbient( ambient );
	mGlobalArea->SetSkyDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetTerrainAmbient( ambient );
	mGlobalArea->SetTerrainDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetStaticObjectAmbient( ambient );
	mGlobalArea->SetStaticObjectDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetDynamicObjectAmbient( ambient );
	mGlobalArea->SetDynamicObjectDimmer( dimmer );

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );
	mGlobalArea->SetFogColor( fogColor );
	mGlobalArea->SetFogDepth( fogDepth );

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadEnv10( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );
//	CAMERAMAN->GetCurrent()->SetFarDistance( camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );
//	SetLevelDistance( dist0, dist1, dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;
	pathName.Format( "./data/map/%s", name );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );
	WORLDMAN->LoadSkydome( pathName, skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	NiMeshUpdateProcess kUpdateProcess;
// 	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 0 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

// 	light = LIGHTAGENT->GetSceneDirLight( 1 );
// 	light->SetAmbientColor( ambient );
// 	light->SetDiffuseColor( diffuse );
// 	light->SetDimmer( dimmer );
// 	light->SetRotate( r );
// 	light->Update( kUpdateProcess );
	return true;
}

bool cSceneManager::LoadGlobalArea10( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetSkyAmbient( ambient );
	mGlobalArea->SetSkyDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetTerrainAmbient( ambient );
	mGlobalArea->SetTerrainDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetStaticObjectAmbient( ambient );
	mGlobalArea->SetStaticObjectDimmer( dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	mGlobalArea->SetDynamicObjectAmbient( ambient );
	mGlobalArea->SetDynamicObjectDimmer( dimmer );

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );
	mGlobalArea->SetFogColor( fogColor );
	mGlobalArea->SetFogDepth( fogDepth );

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadSceneHeader( cFileLoader& loader, unsigned int& outNum )
{
	/// 파일 헤더 로딩
	cSceneFileHeader header;

	if( loader.Read( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		assert( 0 && "failed to load scene file header" );
		return false;
	}

	/// 헤더 검사
	if( memcmp( header.mCode, gSceneFileCode, 10 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		return false;
	}

	/// 장면 트리를 초기화
	Init( header.mCenter, header.mMinRadius, header.mMaxRadius );

	/// 전체 영역 정보를 로딩
	/// 버전별 로딩
	mVersion = header.mVersion;
	switch( mVersion )
	{
	case 6:
		if( LoadEnv6( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		if( LoadGlobalArea6( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		break;
	case 7:
		if( LoadEnv7( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		if( LoadGlobalArea7( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		break;
	case 8:
		if( LoadEnv8( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		if( LoadGlobalArea8( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		break;
	case 9:
		if( LoadEnv9( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		if( LoadGlobalArea9( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		break;
	case 10:
	case 11:
		if( LoadEnv10( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		if( LoadGlobalArea10( loader ) == false )
		{
			assert( 0 );
			return false;
		}
		break;
	default:
		assert( 0 && "invalid file version" );
		return false;
	}

	gNodeNum = header.mNumSceneNodes;
	outNum = gNodeNum;
	gLoadNodeNum = 0;

	return true;
}

bool cSceneManager::LoadSoundSceneHeader( cFileLoader& loader, unsigned int& outNum )
{
	/// 파일 헤더 로딩
	cSoundSceneFileHeader header;

	if( loader.Read( &header, sizeof(cSoundSceneFileHeader) ) != sizeof(cSoundSceneFileHeader) )
	{
		assert( 0 && "failed to load sound scene file header" );
		return false;
	}

	/// 헤더 검사
	if( memcmp( header.mCode, gSceneFileCode, 10 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		return false;
	}

	/// 전체 영역 정보를 로딩
	/// 버전별 로딩
	mVersion = header.mVersion;
	switch( mVersion )
	{
	case 11:
		break;
	default:
		assert( 0 && "invalid file version" );
		return false;
	}

	gSoundNodeNum = header.mNumSceneNodes;
	outNum = gSoundNodeNum;
	gLoadSoundNodeNum = 0;

	return true;
}


int cSceneManager::LoadSceneNode( cFileLoader& loader, unsigned int loadCount )
{
	switch( mVersion )
	{
	case 6:
		return LoadSceneNode6( loader, loadCount );
	case 7:
		return LoadSceneNode7( loader, loadCount );
	case 8:
		return LoadSceneNode8( loader, loadCount );
	case 9:
		return LoadSceneNode9( loader, loadCount );
	case 10:
		return LoadSceneNode10( loader, loadCount );
	case 11:
		return LoadSceneNode11( loader, loadCount );
	default:
		assert( 0 && "invalid file version" );
	}
	return -1;
}

int cSceneManager::LoadSoundSceneNode( cFileLoader& loader, unsigned int loadCount )
{
	switch( mVersion )
	{
	case 11:
		return LoadSoundNode11( loader, loadCount );
	default:
		assert( 0 && "invalid file version" );
	}
	return -1;
}


int cSceneManager::LoadSceneNode6( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = loadCount; i < iend; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA_VER8:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT_VER8:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum == gLoadNodeNum )
			return 0;
	}
	return 1;
}

int cSceneManager::LoadSceneNode7( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = loadCount; i < iend; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA_VER8:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT_VER8:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./Sound/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum == gLoadNodeNum )
			return 0;
	}
	return 1;
}

int cSceneManager::LoadSceneNode8( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = loadCount; i < iend; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA_VER8:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT_VER8:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./Sound/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum == gLoadNodeNum )
			return 0;
	}
	return 1;
}

int cSceneManager::LoadSceneNode9( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = loadCount; i < iend; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./Sound/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum == gLoadNodeNum )
			return 0;
	}
	return 1;
}

int cSceneManager::LoadSceneNode10( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = loadCount; i < iend; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./Sound/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC:
			{
				cStaticSceneNodeParam param;

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 재질 적용 여부를 로딩
				i = 0;
				loader.ReadUnsignedInt( &i );
				param.mMaterialApplied = (i != 0);

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum == gLoadNodeNum )
			return 0;
	}
	return 1;
}

int cSceneManager::LoadSceneNode11( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	unsigned int i = 0;
	for( ; i < loadCount; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
//				cLightSceneNode* n = CreateLight( param );
//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC:
			{
				cStaticSceneNodeParam param;

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 재질 적용 여부를 로딩
				i = 0;
				loader.ReadUnsignedInt( &i );
				param.mMaterialApplied = (i != 0);

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				assert( n && "failed to create static scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum <= gLoadNodeNum )
			return -2;
	}
	return i;
}

int cSceneManager::LoadSceneNode12( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	char lightMapName[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	unsigned int i = 0;
	for( ; i < loadCount; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eAREA:
			{
				cAreaSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변광을 로딩
				loader.Read( &(param.mSkyAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mSkyDimmer) );
				loader.Read( &(param.mTerrainAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mTerrainDimmer) );
				loader.Read( &(param.mStaticObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mStaticObjectDimmer) );
				loader.Read( &(param.mDynamicObjectAmbient), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDynamicObjectDimmer) );

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 그룹 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cAreaSceneNode* n = CreateArea( param );
				assert( n && "failed to create area scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eLIGHT:
			{
				cLightSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 주변색을 로딩
				loader.Read( &(param.mAmbient), sizeof(NiColor) );

				/// 산란색을 로딩
				loader.Read( &(param.mDiffuse0), sizeof(NiColor) );
				loader.Read( &(param.mDiffuse1), sizeof(NiColor) );
				loader.ReadFloat( &(param.mDiffuseAnimTime) );
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFlickering = i != 0;

				/// 반사색을 로딩
				loader.Read( &(param.mSpecular), sizeof(NiColor) );

				/// 감쇠 계수들을 로딩
				loader.ReadFloat( &(param.mConstantAtten0) );
				loader.ReadFloat( &(param.mConstantAtten1) );
				loader.ReadFloat( &(param.mConstantAttenAnimTime) );
				loader.ReadFloat( &(param.mLinearAtten) );
				loader.ReadFloat( &(param.mQuadricAtten) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				//				cLightSceneNode* n = CreateLight( param );
				//				assert( n && "failed to create light scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		case cSceneNode::eSTATIC:
			{
				cStaticSceneNodeParam param;

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 재질 적용 여부를 로딩
				i = 0;
				loader.ReadUnsignedInt( &i );
				param.mMaterialApplied = (i != 0);

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 라이트맵 로딩
				loader.Read( lightMapName, 64 );
				lightMapName[63] = 0;

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./data/map/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
				{
					if( ::strlen(lightMapName) != 0 )
					{
						NiTexture* tex = RESOURCEMAN->LoadMapTexture( lightMapName, false );
						if( tex )
						{
							/// change detail map
							int a = 0;
							a = 1;
//							n->SetLightMapTexture( tex, n->GetNiNode() );
						}
						else
						{
							cString msg;
							msg.Format( "Failed to load Light Map Texture : %s", lightMapName );
							NiMessageBox( msg.Cstr(), "Load Error" );
						}
					}
				}
				else
				{
					assert( n && "failed to create static scene node" );
				}
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadNodeNum++;
		if( gNodeNum <= gLoadNodeNum )
			return -2;
	}
	return i;
}

int cSceneManager::LoadSoundNode11( cFileLoader& loader, unsigned int loadCount )
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	unsigned int i = 0;
	for( ; i < loadCount; ++i )
	{
		/// 타입을 로딩
		loader.ReadUnsignedInt( &type );

		/// 경로를 로딩
		loader.Read( path, 64 );
		path[63] = 0;

		/// 이름을 로딩
		loader.Read( name, 64 );
		name[63] = 0;

		/// 위치를 로딩
		loader.Read( &translate, sizeof(NiPoint3) );

		/// 회전을 로딩
		loader.Read( &rotate, sizeof(NiMatrix3) );

		/// 배율을 로딩
		loader.ReadFloat( &scale );

		/// 노드 타입에 따른 로딩 및 생성
		switch( type )
		{
		case cSceneNode::eSOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "./Sound/%s", name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;
				param.mManaged = false;
//				param.mVolumeRatio *= OPTIONMAN->GetEnvVolume();
				param.mEffectSound = false;

				/// 노드를 생성
				cSoundSceneNode* n = CreateMapSound( param );
				assert( n && "failed to create sound scene node" );
				//if( n == 0 )
				//	return -1;
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return -1;
		}

		gLoadSoundNodeNum++;
		if( gSoundNodeNum == gLoadSoundNodeNum )
			return -2;
	}
	return i;
}


bool cSceneManager::LoadAreaGroup( unsigned int mapIndex, unsigned char mapMode )
{
	mTextureMap.Clear();

	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIndex, mapMode );

	/// 영역 그룹들을 제거
	cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
	cAreaGroupMap::cIterator iend = mAreaGroupMap.End();

	for( ; i != iend; ++i )
	{
		delete (cAreaGroup*)((*i).mSecond);
	}
	mAreaGroupMap.Clear();

	/// 파일 열기
	cString pathName;
	pathName.Format( "./Script/Resource/%03d_areagroup.txt", folderIdx );

	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	/// 사운드 파일을 로딩
	tok.GetNext( &str );

	if( str == "sounds" )
	{
		/// "{"
		tok.GetNext( &str );

		while( tok.IsEnd() == false )
		{
			/// 사운드 인덱스
			tok.GetNext( &str );
			if( str == "}" )
				break;

			unsigned int sndIndex = str.ToInt();

			/// 사운드 파일명
			tok.GetNext( &str );

			/// 로딩
			pathName.Format( "./Sound/%s", str.Cstr() );
			NiAudioSource* snd = SOUNDSYS->LoadAreaSound( sndIndex, pathName.Cstr() );

			// sound 파일이 있는지 체크하는 루틴 추가 [12/28/2009 Jo_Client]
			if( FILESYSTEM->FileExist( pathName.Cstr() ) == true )
				FILESYSTEM->WriteLoadLog( pathName.Cstr() );
			else
				assert(0);

			if( snd == 0 )
				continue;
		}
	}

	/// 간판 이미지 파일을 로딩
	tok.GetNext( &str );

	if( str == "images" )
	{
		/// "{"
		tok.GetNext( &str );

		while( tok.IsEnd() == false )
		{
			/// 이미지 인덱스
			tok.GetNext( &str );
			if( str == "}" )
				break;

			/// 이미지 파일명
			unsigned int texIndex = str.ToInt();
			tok.GetNext( &str );
			pathName.Format( "./language/%s/%s", cApplication::mLangaugeFolder.Cstr(), str.Cstr() );

			/// 로딩
			NiTexture* tex = RESOURCEMAN->LoadTexture( pathName, false );
			
			if( tex == 0 )
				continue;

			/// 텍스처 맵에 추가
			mTextureMap.Insert( texIndex, tex );
		}
	}

	///
	tok.GetNext( &str );

	if( str == "areaGroups" )
	{
		/// "{"
		tok.GetNext( &str );

		while( tok.IsEnd() == false )
		{
			/// 영역 그룹 인덱스
			tok.GetNext( &str );
			unsigned int groupIndex = str.ToInt();

			/// 사운드 인덱스
			tok.GetNext( &str );
			unsigned int sndIndex = str.ToInt();
			NiAudioSource* snd = SOUNDSYS->GetAreaSound( sndIndex );

			/// 간판 이미지 인덱스
			tok.GetNext( &str );
			unsigned int texIndex = str.ToInt();
			NiTexture* tex = 0;

			cTextureMap::cIterator ti = mTextureMap.Find( texIndex );
			if( ti != mTextureMap.End() )
				tex = (NiTexture*)(*ti).mSecond;

			/// 간판 이미지 위치, 크기
			tok.GetNext( &str );
			unsigned int tx = str.ToInt();

			tok.GetNext( &str );
			unsigned int ty = str.ToInt();

			tok.GetNext( &str );
			unsigned int tw = str.ToInt();

			tok.GetNext( &str );
			unsigned int th = str.ToInt();

			/// 이름 인덱스
			tok.GetNext( &str );
			unsigned int nameIndex = str.ToInt();

			/// 영역 그룹을 생성
			cAreaGroupMap::cIterator ai = mAreaGroupMap.Find( groupIndex );

			if( ai == mAreaGroupMap.End() )
			{
				unsigned int x = (RENDERSYS->GetScreenWidth() - tw) / 2;
				unsigned int y = 60;

				cAreaGroup* areaGroup = new cAreaGroup( snd, tex, x, y, tx, ty, tw, th, nameIndex );
				mAreaGroupMap.Insert( groupIndex, areaGroup );

				/// 영역 그룹에 영역 노드들을 추가

				cAreaMap::cIterator i, iend;
				i = mAreaArray.Begin();
				iend = mAreaArray.End();
				for( ; i != iend; ++i )
				{
					cAreaSceneNode* area = (cAreaSceneNode*)(*i).mSecond;

					if( area->GetGroupIndex() == groupIndex )
						areaGroup->AddArea( area );
				}
			}
		}
	}
	return true;
}
