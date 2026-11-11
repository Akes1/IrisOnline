#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "SoundSystem.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "WorldManager.h"

#include "AreaGroup.h"
#include "AreaSceneNode.h"
#include "LightSceneNode.h"
#include "SoundSceneNode.h"
#include "StaticSceneNode.h"

static const char* gSceneFileCode = "IrisScene";

#ifdef MAP_EDITOR
bool cSceneManager::Load( const cString& pathName,
						 tHashSet<cStaticSceneNode*>& objectSet,
						 tHashSet<cLightSceneNode*>& lightSet,
						 tHashSet<cSoundSceneNode*>& soundSet,
						 tHashSet<cAreaSceneNode*>& areaSet,
						 bool add )
#else
bool cSceneManager::Load( const cString& pathName, bool add )
#endif
{
	if( add == false )
		Clear();

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		AfxMessageBox( "failed to open file to load scene" );
		return false;
	}

	/// 파일 헤더 로딩
	cSceneFileHeader header;

	if( loader.Read( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		AfxMessageBox( "failed to load scene file header" );
		return false;
	}

	/// 헤더 검사
	if( ::memcmp( header.mCode, gSceneFileCode, 10 ) != 0 )
	{
		AfxMessageBox( "invalid file type" );
		return false;
	}

	/// 장면 트리를 초기화
	if( add == false )
	{
		InitTree( header.mCenter, header.mMinRadius, header.mMaxRadius );
	}
	else
	{
		NiPoint3 ctr0 = mNodeTree.GetCenter();
		float mnr0 = mNodeTree.GetMinRadius();
		float mxr0 = mNodeTree.GetMaxRadius();
		NiPoint3 ctr1 = header.mCenter;
		float mnr1 = header.mMinRadius;
		float mxr1 = header.mMaxRadius;

		cSphere sphere0( ctr0, mxr0 );
		cSphere sphere1( ctr1, mxr1 );

		if( sphere0.ContainSphere( sphere1 ) )
		{
			/// DoNothing();
		}
		else if( sphere1.ContainSphere( sphere0 ) )
		{
			InitTree( ctr1, mnr1, mxr1 );
		}
		else
		{
			NiPoint3 ctr = (ctr0 + ctr1) * 0.5f;
			float mnr = mnr0 < mnr1 ? mnr0 : mnr1;
			float mxr = (ctr0 - ctr1).Length() + mxr0 + mxr1;

			InitTree( ctr, mnr, mxr );
		}
	}

	/// 버전별 로딩
	switch( header.mVersion )
	{
	case 6:
		{
			if( LoadEnvVer6( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer6( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}

#ifdef MAP_EDITOR
			return LoadVer6( loader, header, objectSet, lightSet, soundSet, areaSet );
#else
			return LoadVer6( loader, header );
#endif
		}
	case 7:
		{
			if( LoadEnvVer7( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer7( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}

#ifdef MAP_EDITOR
			return LoadVer7( loader, header, objectSet, lightSet, soundSet, areaSet );
#else
			return LoadVer7( loader, header );
#endif
		}
	case 8:
		{
			if( LoadEnvVer8( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer8( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}

#ifdef MAP_EDITOR
			return LoadVer8( loader, header, objectSet, lightSet, soundSet, areaSet );
#else
			return LoadVer8( loader, header );
#endif
		}
	case 9:
		{
			if( LoadEnvVer9( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer9( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}

#ifdef MAP_EDITOR
			return LoadVer9( loader, header, objectSet, lightSet, soundSet, areaSet );
#else
			return LoadVer9( loader, header );
#endif
		}
	case 10:
		{
			if( LoadEnvVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}

#ifdef MAP_EDITOR
			return LoadVer10( loader, header, objectSet, lightSet, soundSet, areaSet );
#else
			return LoadVer10( loader, header );
#endif
		}
	case 11:
		{
			if( LoadEnvVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}
#ifdef MAP_EDITOR
			return LoadVer11( loader, header, objectSet, lightSet, areaSet );
#else
			return LoadVer11( loader, header );
#endif
		}
	case 12:
		{
			if( LoadEnvVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load environment" );
				return false;
			}
			if( LoadGlobalAreaVer10( loader, add ) == false )
			{
				AfxMessageBox( "failed to load global area" );
				return false;
			}
#ifdef MAP_EDITOR
			return LoadVer12( loader, header, objectSet, lightSet, areaSet );
#else
			return LoadVer12( loader, header );
#endif
		}
	}

	AfxMessageBox( "invalid file version" );
	return false;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadSound( const cString& pathName, tHashSet<cSoundSceneNode*>& soundSet )
#else
bool cSceneManager::LoadSound( const cString& pathName )
#endif
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		AfxMessageBox( "failed to open file to load scene" );
		return false;
	}

	/// 파일 헤더 로딩
	cSoundSceneFileHeader header;
	if( loader.Read( &header, sizeof(cSoundSceneFileHeader) ) != sizeof(cSoundSceneFileHeader) )
	{
		AfxMessageBox( "failed to load scene file header" );
		return false;
	}

	/// 헤더 검사
	if( ::memcmp( header.mCode, gSceneFileCode, 10 ) != 0 )
	{
		AfxMessageBox( "invalid file type" );
		return false;
	}

	/// 버전별 로딩
	switch( header.mVersion )
	{
	case 11:
		{
#ifdef MAP_EDITOR
			return LoadSoundVer11( loader, header, soundSet );
#else
			return LoadSoundVer11( loader, header );
#endif
		}
	}

	AfxMessageBox( "invalid file version" );
	return false;

	return true;
}


bool cSceneManager::LoadEnvVer6( cFileLoader& loader, bool add )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	if( add == false )
	{
		CAMERAMAN->GetCamera( 0 )->SetFarDistance( camFarDist );
	}

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyHeight = 0.0f;
	loader.ReadFloat( &skyHeight );
	//WORLDMAN->SetSkyHeight( skyHeight );

#ifdef MAP_EDITOR
	if( add == false )
	{
		WORLDMAN->LoadSkyDome( name, 1.0f, skyEnabled != 0 );
	}
#endif

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 1 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}
	return true;
}

bool cSceneManager::LoadEnvVer7( cFileLoader& loader, bool add )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	if( add == false )
	{
		CAMERAMAN->GetCamera( 0 )->SetFarDistance( camFarDist );
	}

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.0f;
	loader.ReadFloat( &skyScale );

#ifdef MAP_EDITOR
	if( add == false )
	{
		WORLDMAN->LoadSkyDome( name, skyScale, skyEnabled != 0 );
	}
#endif

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 1 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}
	return true;
}

bool cSceneManager::LoadEnvVer8( cFileLoader& loader, bool add )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	if( add == false )
	{
		CAMERAMAN->GetCamera( 0 )->SetFarDistance( camFarDist );
	}

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	if( add == false )
	{
		SetLevelDistance( dist0, dist1, dist2 );
	}

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.0f;
	loader.ReadFloat( &skyScale );

#ifdef MAP_EDITOR
	if( add == false )
	{
		WORLDMAN->LoadSkyDome( name, skyScale, skyEnabled != 0 );
	}
#endif

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 1 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}
	return true;
}

bool cSceneManager::LoadEnvVer9( cFileLoader& loader, bool add )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	if( add == false )
	{
		CAMERAMAN->GetCamera( 0 )->SetFarDistance( camFarDist );
	}

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	if( add == false )
	{
		SetLevelDistance( dist0, dist1, dist2 );
	}

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.0f;
	loader.ReadFloat( &skyScale );

#ifdef MAP_EDITOR
	if( add == false )
	{
		WORLDMAN->LoadSkyDome( name, skyScale, skyEnabled != 0 );
	}
#endif

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 1 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}
	return true;
}

bool cSceneManager::LoadEnvVer10( cFileLoader& loader, bool add )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	if( add == false )
	{
		CAMERAMAN->GetCamera( 0 )->SetFarDistance( camFarDist );
	}

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	if( add == false )
	{
		SetLevelDistance( dist0, dist1, dist2 );
	}

	/// 하늘
	char name[64];
	loader.Read( name, 64 );
	name[63] = 0;
	cString pathName;

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.0f;
	loader.ReadFloat( &skyScale );

#ifdef MAP_EDITOR
	if( add == false )
	{
		WORLDMAN->LoadSkyDome( name, skyScale, skyEnabled != 0 );
	}
#endif

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	if( add == false )
	{
		NiDirectionalLight* light = SCENEMAN->GetDirLight( 1 );
		light->SetAmbientColor( ambient );
		light->SetDiffuseColor( diffuse );
		light->SetDimmer( dimmer );
		light->SetRotate( r );
		light->Update( 0.0f );
	}
	return true;
}

bool cSceneManager::LoadGlobalAreaVer6( cFileLoader& loader, bool add )
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
	
	if( add == false )
	{
		mGlobalArea->SetSkyAmbient( ambient );
		mGlobalArea->SetSkyDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetTerrainAmbient( ambient );
		mGlobalArea->SetTerrainDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetStaticObjectAmbient( ambient );
		mGlobalArea->SetStaticObjectDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetDynamicObjectAmbient( ambient );
		mGlobalArea->SetDynamicObjectDimmer( dimmer );
	}

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	bool fogEnabled;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	fogEnabled = i != 0;
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );

	if( add == false )
	{
		mGlobalArea->SetFogEnabled( fogEnabled );
		mGlobalArea->SetFogColor( fogColor );
		mGlobalArea->SetFogDepth( fogDepth );
	}

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadGlobalAreaVer7( cFileLoader& loader, bool add )
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

	if( add == false )
	{
		mGlobalArea->SetSkyAmbient( ambient );
		mGlobalArea->SetSkyDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetTerrainAmbient( ambient );
		mGlobalArea->SetTerrainDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetStaticObjectAmbient( ambient );
		mGlobalArea->SetStaticObjectDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetDynamicObjectAmbient( ambient );
		mGlobalArea->SetDynamicObjectDimmer( dimmer );
	}

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	bool fogEnabled;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	fogEnabled = i != 0;
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );

	if( add == false )
	{
		mGlobalArea->SetFogEnabled( fogEnabled );
		mGlobalArea->SetFogColor( fogColor );
		mGlobalArea->SetFogDepth( fogDepth );
	}

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadGlobalAreaVer8( cFileLoader& loader, bool add )
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

	if( add == false )
	{
		mGlobalArea->SetSkyAmbient( ambient );
		mGlobalArea->SetSkyDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetTerrainAmbient( ambient );
		mGlobalArea->SetTerrainDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetStaticObjectAmbient( ambient );
		mGlobalArea->SetStaticObjectDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetDynamicObjectAmbient( ambient );
		mGlobalArea->SetDynamicObjectDimmer( dimmer );
	}

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	bool fogEnabled;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	fogEnabled = i != 0;
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );

	if( add == false )
	{
		mGlobalArea->SetFogEnabled( fogEnabled );
		mGlobalArea->SetFogColor( fogColor );
		mGlobalArea->SetFogDepth( fogDepth );
	}

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadGlobalAreaVer9( cFileLoader& loader, bool add )
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

	if( add == false )
	{
		mGlobalArea->SetSkyAmbient( ambient );
		mGlobalArea->SetSkyDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetTerrainAmbient( ambient );
		mGlobalArea->SetTerrainDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetStaticObjectAmbient( ambient );
		mGlobalArea->SetStaticObjectDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetDynamicObjectAmbient( ambient );
		mGlobalArea->SetDynamicObjectDimmer( dimmer );
	}

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	bool fogEnabled;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	fogEnabled = i != 0;
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );

	if( add == false )
	{
		mGlobalArea->SetFogEnabled( fogEnabled );
		mGlobalArea->SetFogColor( fogColor );
		mGlobalArea->SetFogDepth( fogDepth );
	}

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

bool cSceneManager::LoadGlobalAreaVer10( cFileLoader& loader, bool add )
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

	if( add == false )
	{
		mGlobalArea->SetSkyAmbient( ambient );
		mGlobalArea->SetSkyDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	if( add == false )
	{
		mGlobalArea->SetTerrainAmbient( ambient );
		mGlobalArea->SetTerrainDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	if( add == false )
	{
		mGlobalArea->SetStaticObjectAmbient( ambient );
		mGlobalArea->SetStaticObjectDimmer( dimmer );
	}

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	if( add == false )
	{
		mGlobalArea->SetDynamicObjectAmbient( ambient );
		mGlobalArea->SetDynamicObjectDimmer( dimmer );
	}

	/// 전체 영역의 안개 속성을 로딩
	unsigned int i;
	bool fogEnabled;
	NiColor fogColor;
	float fogDepth;
	loader.ReadUnsignedInt( &i );
	fogEnabled = i != 0;
	loader.Read( &fogColor, sizeof(NiColor) );
	loader.ReadFloat( &fogDepth );

	if( add == false )
	{
		mGlobalArea->SetFogEnabled( fogEnabled );
		mGlobalArea->SetFogColor( fogColor );
		mGlobalArea->SetFogDepth( fogDepth );
	}

	/// 영역 그룹 인덱스를 로딩
	unsigned int groupIndex;
	loader.ReadUnsignedInt( &groupIndex );
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer6( cFileLoader& loader, cSceneFileHeader& header,
							 tHashSet<cStaticSceneNode*>& objectSet,
							 tHashSet<cLightSceneNode*>& lightSet,
							 tHashSet<cSoundSceneNode*>& soundSet,
							 tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer6( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA_VER8:
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
				param.mFogEnabled = i != 0;
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 사용자 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT_VER8:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_SOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		case SCENENODE_STATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer7( cFileLoader& loader, cSceneFileHeader& header,
							 tHashSet<cStaticSceneNode*>& objectSet,
							 tHashSet<cLightSceneNode*>& lightSet,
							 tHashSet<cSoundSceneNode*>& soundSet,
							 tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer7( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA_VER8:
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
				param.mFogEnabled = i != 0;
				loader.Read( &(param.mFogColor), sizeof(NiColor) );
				loader.ReadFloat( &(param.mFogDepth) );

				/// 사용자 인덱스를 로딩
				loader.ReadUnsignedInt( &(param.mGroupIndex) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT_VER8:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_SOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		case SCENENODE_STATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer8( cFileLoader& loader, cSceneFileHeader& header,
							 tHashSet<cStaticSceneNode*>& objectSet,
							 tHashSet<cLightSceneNode*>& lightSet,
							 tHashSet<cSoundSceneNode*>& soundSet,
							 tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer8( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA_VER8:
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
				param.mFogEnabled = i != 0;
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
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT_VER8:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_SOUND_VER8:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		case SCENENODE_STATIC_VER8:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer9( cFileLoader& loader, cSceneFileHeader& header,
							 tHashSet<cStaticSceneNode*>& objectSet,
							 tHashSet<cLightSceneNode*>& lightSet,
							 tHashSet<cSoundSceneNode*>& soundSet,
							 tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer9( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA:
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
				param.mFogEnabled = i != 0;
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
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_SOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		case SCENENODE_STATIC:
			{
				cStaticSceneNodeParam param;
				param.mMaterialApplied = true;

				/// 재질 속성을 로딩
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 안개 적용 여부를 로딩
				unsigned int i = 0;
				loader.ReadUnsignedInt( &i );
				param.mFogApplied = (i != 0);

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer10( cFileLoader& loader, cSceneFileHeader& header,
							 tHashSet<cStaticSceneNode*>& objectSet,
							 tHashSet<cLightSceneNode*>& lightSet,
							 tHashSet<cSoundSceneNode*>& soundSet,
							 tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer10( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA:
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

//				char test[256];
//				::sprintf(test, "check %f", param.mDynamicObjectDimmer);
//				NiMessageBox(test, "check");

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFogEnabled = i != 0;
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
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_SOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		case SCENENODE_STATIC:
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
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				param.mPathName.Format( "./data/map/%s", name );
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer11( cFileLoader& loader, cSceneFileHeader& header,
							  tHashSet<cStaticSceneNode*>& objectSet,
							  tHashSet<cLightSceneNode*>& lightSet,
							  tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer11( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA:
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

				//				char test[256];
				//				::sprintf(test, "check %f", param.mDynamicObjectDimmer);
				//				NiMessageBox(test, "check");

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFogEnabled = i != 0;
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
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_STATIC:
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
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
					objectSet.Insert( n );
#else
				param.mPathName.Format( "./data/map/%s", name );
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadVer12( cFileLoader& loader, cSceneFileHeader& header,
							  tHashSet<cStaticSceneNode*>& objectSet,
							  tHashSet<cLightSceneNode*>& lightSet,
							  tHashSet<cAreaSceneNode*>& areaSet )
#else
bool cSceneManager::LoadVer12( cFileLoader& loader, cSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	char lightMapName[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;


	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_AREA:
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

				//				char test[256];
				//				::sprintf(test, "check %f", param.mDynamicObjectDimmer);
				//				NiMessageBox(test, "check");

				/// 안개 속성을 로딩
				unsigned int i;
				loader.ReadUnsignedInt( &i );
				param.mFogEnabled = i != 0;
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
#ifdef MAP_EDITOR
				cAreaSceneNode* n = CreateArea( param );
				if( n )
					areaSet.Insert( n );
#else
				CreateArea( param );
#endif
			}
			break;
		case SCENENODE_LIGHT:
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
#ifdef MAP_EDITOR
				cLightSceneNode* n = CreateLight( param );
				if( n )
					lightSet.Insert( n );
#else
				CreateLight( param );
#endif
			}
			break;
		case SCENENODE_STATIC:
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
				loader.Read( &param.mMaterial, sizeof(cMaterialData_Ver9) );

				/// 가시 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mVisibleLevel) );

				/// 차폐 레벨을 로딩
				loader.ReadUnsignedInt( &(param.mOccludeLevel) );

				/// 라이트맵 로딩
				loader.Read( lightMapName, 64 );
				lightMapName[63] = 0;

				/// 기본 속성 인자들을 설정
				param.mPathName = name;
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cStaticSceneNode* n = CreateStatic( param );
				if( n )
				{
					objectSet.Insert( n );
				}
				else
				{
					assert(0);
					cString msg;
					msg.Format( "Failed to load file : %s", name );
					AfxMessageBox( msg.Cstr() );
				}
#else
				param.mPathName.Format( "./data/map/%s", name );
				CreateStatic( param );
#endif
			}
			break;
		default:
			assert( 0 && "invalid scene node type" );
			return false;
		}
	}
	mInited = true;
	return true;
}

#ifdef MAP_EDITOR
bool cSceneManager::LoadSoundVer11( cFileLoader& loader, cSoundSceneFileHeader& header, tHashSet<cSoundSceneNode*>& soundSet )
#else
bool cSceneManager::LoadSoundVer11( cFileLoader& loader, cSoundSceneFileHeader& header )
#endif
{
	/// 장면 노드들을 생성 및 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	for( unsigned int i = 0, iend = header.mNumSceneNodes; i < iend; ++i )
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
		case SCENENODE_SOUND:
			{
				cSoundSceneNodeParam param;

				/// 반지름을 로딩
				loader.ReadFloat( &(param.mOuterRadius) );

				/// 감쇠 거리를 로딩
				loader.ReadFloat( &(param.mAttenDistance) );

				/// 반복 횟수를 로딩
				loader.ReadUnsignedInt( &(param.mLoopCount) );

				/// 재생 간격을 로딩
				loader.ReadFloat( &(param.mLoopInterval) );

				/// 음량 비율을 로딩
				loader.ReadFloat( &(param.mVolumeRatio) );

				/// 기본 속성 인자들을 설정
				param.mPathName.Format( "%s%s", path, name );
				param.mTranslate = translate;
				param.mRotate = rotate;
				param.mScale = scale;

				/// 노드를 생성
#ifdef MAP_EDITOR
				cSoundSceneNode* n = CreateSound( param );
				if( n )
					soundSet.Insert( n );
#endif
			}
			break;
		default:
			assert( 0 && "invalid sound scene node type" );
			return false;
		}
	}

	return true;
}


#ifdef MAP_EDITOR
bool cSceneManager::LoadAreaGroup( const cString& pathName )
{
	mTextureMap.Clear();

	/// 영역 그룹들을 제거
	cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
	cAreaGroupMap::cIterator iend = mAreaGroupMap.End();

	for( ; i != iend; ++i )
	{
		delete (*i).mSecond;
	}
	mAreaGroupMap.Clear();

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		AfxMessageBox( "failed to open area info file" );
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
			NiAudioSource* snd = SOUNDSYS->Load2DSound( sndIndex, str.Cstr() );

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

			unsigned int texIndex = str.ToInt();

			/// 이미지 파일명
			tok.GetNext( &str );

			/// 로딩
			NiTexture* tex = RESOURCEMAN->LoadTexture( str.Cstr(), false );

			if( tex == 0 )
			{
				//assert( 0 && "failed to load texture" );
				//return false;
				continue;
			}

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
			NiAudioSource* snd = 0;

			snd = SOUNDSYS->Get2DSound( sndIndex );

			/// 간판 이미지 인덱스
			tok.GetNext( &str );
			unsigned int texIndex = str.ToInt();
			NiTexture* tex = 0;

			cTextureMap::cIterator ti = mTextureMap.Find( texIndex );
			if( ti != mTextureMap.End() )
				tex = (*ti).mSecond;

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

			/// 영역 그룹을 생성
			cAreaGroupMap::cIterator ai = mAreaGroupMap.Find( groupIndex );

			if( ai == mAreaGroupMap.End() )
			{
				unsigned int x = (RENDERSYS->GetScreenWidth() - tw) / 2;
				unsigned int y = 60;

				cAreaGroup* areaGroup = new cAreaGroup( snd, tex, x, y, tx, ty, tw, th );
				mAreaGroupMap.Insert( groupIndex, areaGroup );

				/// 영역 그룹에 영역 노드들을 추가
				unsigned int i = 0;
				unsigned int iend = mAreaArray.GetSize();

				for( ; i < iend; ++i )
				{
					if( mAreaArray[i]->GetGroupIndex() == groupIndex )
						areaGroup->AddArea( mAreaArray[i] );
				}
			}
		}
	}
	return true;
}

bool cSceneManager::LoadObjectTransformScript( const cString& pathName, tHashSet<cStaticSceneNode*>& objectSet )
{
	cString path;
	::GetFilePath( &path, pathName );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		AfxMessageBox( "failed to open object transform script" );
		return false;
	}

	/// 한줄씩 처리
	char buffer[256];

	while( loader.IsEnd() == false && loader.ReadLine( buffer, 256 ) > 0 )
	{
		/// 토큰 파싱
		cTokenizer tok( buffer, 256, " ,[]\t\r\n\"" );
		cString str, nifName;
		NiQuaternion q;
		NiMatrix3 r;
		NiPoint3 t;
		float s;

		/// 인덱스
		tok.GetNext( &str );

		/// 파일명
		tok.GetNext( &str );
		nifName.Format( "%s%s", path.Cstr(), str.Cstr() );

		/// 회전
		tok.GetNext( &str );
		q.m_fX = str.ToFloat();

		tok.GetNext( &str );
		q.m_fY = str.ToFloat();

		tok.GetNext( &str );
		q.m_fZ = str.ToFloat();

		tok.GetNext( &str );
		q.m_fW = str.ToFloat();

		q.ToRotation( r );

		/// 이동
		tok.GetNext( &str );
		t.x = str.ToFloat();

		tok.GetNext( &str );
		t.y = str.ToFloat();

		tok.GetNext( &str );
		t.z = str.ToFloat();

		/// 배율
		tok.GetNext( &str );
		s = str.ToFloat();

		/// 기본 속성 인자들을 설정
		cStaticSceneNodeParam param;
		param.mPathName = nifName;
		param.mRotate = r;
		param.mTranslate = t;
		param.mScale = s;

		/// 노드를 생성
		cStaticSceneNode* n = CreateStatic( param );
		if( n )
			objectSet.Insert( n );
	}

	return true;
}
#endif
