#include "stdafx.h"
#include "EnvEditor.h"

#include "MainFrame.h"
#include "TabWindow.h"
#include "MapEditorView.h"
#include "EnvCameraDialog.h"
#include "EnvLevelDistanceDialog.h"
#include "EnvSkyDialog.h"
#include "EnvDirLightDialog.h"

#include "Engine/CameraManager.h"
#include "Engine/FreeCamera.h"
#include "Engine/WorldManager.h"
#include "Engine/SceneManager.h"

cEnvEditor* cEnvEditor::mSingleton = 0;

cEnvEditor::cEnvEditor()
: mCameraDialog( 0 )
, mLevelDistanceDialog( 0 )
, mSkyDialog( 0 )
, mDirLightDialog( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mGlobalArea = SCENEMAN->GetGlobalArea();
}

cEnvEditor::~cEnvEditor()
{
	mSingleton = 0;
}

bool cEnvEditor::Save( cFileSaver& saver )
{
	/// 카메라
	cFreeCamera* cam = VIEW->GetDefaultCamera();
	saver.WriteFloat( cam->GetFarDistance() );

	/// 가시 레벨 거리
	saver.WriteFloat( SCENEMAN->GetLevelDistance0() );
	saver.WriteFloat( SCENEMAN->GetLevelDistance1() );
	saver.WriteFloat( SCENEMAN->GetLevelDistance2() );

	/// 하늘
	char name[64];
	memset( name, 0, 64 );
	const char* skyName = WORLDMAN->GetSkyName();
	if( skyName )
		NiStrcpy( name, 64, skyName );

	saver.Write( name, 64 );
	saver.WriteUnsignedInt( WORLDMAN->IsSkyEnabled() );
	saver.WriteFloat( WORLDMAN->GetSkyScale() );

	/// 방향광
	NiDirectionalLight* light = SCENEMAN->GetDirLight( 0 );
	saver.Write( &(light->GetAmbientColor()), sizeof( NiColor) );
	saver.Write( &(light->GetDiffuseColor()), sizeof(NiColor) );
	saver.WriteFloat( light->GetDimmer() );
	saver.Write( &(light->GetWorldRotate()), sizeof(NiMatrix3) );

	light = SCENEMAN->GetDirLight( 1 );
	saver.Write( &(light->GetAmbientColor()), sizeof(NiColor) );
	saver.Write( &(light->GetDiffuseColor()), sizeof(NiColor) );
	saver.WriteFloat( light->GetDimmer() );
	saver.Write( &(light->GetWorldRotate()), sizeof(NiMatrix3) );
	return true;
}

bool cEnvEditor::LoadSky( const cString& pathName )
{
	if( WORLDMAN->LoadSkyDome( pathName ) )
	{
		cString fileName;
		::GetFileName( &fileName, pathName );
		mSkyDialog->Update( fileName );
		return true;
	}
	else
	{
		return false;
	}
}

void cEnvEditor::Update()
{
	/// 카메라
	mCameraDialog->Update( CAMERAMAN->GetCamera( 0 )->GetFarDistance() );

	/// 가시 레벨 거리
	unsigned int dist0 = (unsigned int)(SCENEMAN->GetLevelDistance0() / 100.0f);
	unsigned int dist1 = (unsigned int)(SCENEMAN->GetLevelDistance1() / 100.0f);
	unsigned int dist2 = (unsigned int)(SCENEMAN->GetLevelDistance2() / 100.0f);
	mLevelDistanceDialog->Update( dist0, dist1, dist2 );

	/// 하늘
	mSkyDialog->Update( WORLDMAN->GetSkyName(), WORLDMAN->IsSkyEnabled(), WORLDMAN->GetSkyScale() ); //, WORLDMAN->GetSkyHeight() );

	/// 방향광
	mDirLightDialog->Update( SCENEMAN->GetDirLight( 0 ), SCENEMAN->GetDirLight( 1 ) );
}

void cEnvEditor::SetSkyEnabled( bool enabled )
{
	WORLDMAN->SetSkyEnabled( enabled );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cEnvEditor::SetSkyScale( float scale )
{
	WORLDMAN->SetSkyScale( scale );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cEnvEditor::SetCamFarDistance( float dist )
{
	if( VIEW == 0 )
		return;

	VIEW->SetDefaultCameraFarDistance( dist );
	VIEW->SetLookAtCameraFarDistance( dist );
	TABWIN->SetDefaultCameraFarDist( dist );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cEnvEditor::SetLevelDistance( unsigned int dist0, unsigned int dist1, unsigned int dist2 )
{
	SCENEMAN->SetLevelDistance( dist0 * 100.0f, dist1 * 100.0f, dist2 * 100.0f );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cEnvEditor::SetDirLightAmbient( unsigned int i, const NiColor& color )
{
	SCENEMAN->SetDirLightAmbient( i, color );
}

void cEnvEditor::SetDirLightDiffuse( unsigned int i, const NiColor& color )
{
	SCENEMAN->SetDirLightDiffuse( i, color );
}

void cEnvEditor::SetDirLightDimmer( unsigned int i, float dimmer )
{
	SCENEMAN->SetDirLightDimmer( i, dimmer );
}

void cEnvEditor::SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle )
{
	SCENEMAN->SetDirLightRotate( i, xangle, yangle, zangle );

	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}
