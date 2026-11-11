/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.09
* 내    용 : 환경 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cAreaSceneNode;
class cEnvCameraDialog;
class cEnvLevelDistanceDialog;
class cEnvSkyDialog;
class cEnvDirLightDialog;

/// 환경 편집자
class cEnvEditor
{
	static cEnvEditor* mSingleton;

public:
	cEnvEditor();
	~cEnvEditor();

	/// 저장
	bool Save( cFileSaver& saver );

	/// 카메라
	void SetCamFarDistance( float dist );

	/// 가시 레벨 거리
	void SetLevelDistance( unsigned int dist0, unsigned int dist1, unsigned int dist2 );

	/// 하늘
	bool LoadSky( const cString& pathName );
	void Update();
	void SetSkyEnabled( bool enabled );
	void SetSkyScale( float scale );

	/// 방향광
	void SetDirLightAmbient( unsigned int i, const NiColor& color );
	void SetDirLightDiffuse( unsigned int i, const NiColor& color );
	void SetDirLightDimmer( unsigned int i, float dimmer );
	void SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle );

	/// 다이얼로그
	void SetCameraDialog( cEnvCameraDialog* dlg );
	void SetLevelDistanceDialog( cEnvLevelDistanceDialog* dlg );
	void SetSkyDialog( cEnvSkyDialog* dlg );
	void SetDirLightDialog( cEnvDirLightDialog* dlg );

public:
	/// 단일체를 리턴
	static cEnvEditor* GetSingleton();

protected:
	/// 다이얼로그
	cEnvCameraDialog* mCameraDialog;
	cEnvLevelDistanceDialog* mLevelDistanceDialog;
	cEnvSkyDialog* mSkyDialog;
	cEnvDirLightDialog* mDirLightDialog;

	/// 전체 영역 노드
	cAreaSceneNode* mGlobalArea;
};

inline
void cEnvEditor::SetCameraDialog( cEnvCameraDialog* dlg )
{
	assert( dlg );

	mCameraDialog = dlg;
}

inline
void cEnvEditor::SetLevelDistanceDialog( cEnvLevelDistanceDialog* dlg )
{
	assert( dlg );

	mLevelDistanceDialog = dlg;
}

inline
void cEnvEditor::SetSkyDialog( cEnvSkyDialog* dlg )
{
	assert( dlg );

	mSkyDialog = dlg;
}

inline
void cEnvEditor::SetDirLightDialog( cEnvDirLightDialog* dlg )
{
	assert( dlg );

	mDirLightDialog = dlg;
}

inline
cEnvEditor* cEnvEditor::GetSingleton()
{
	return mSingleton;
}

#define ENVEDIT cEnvEditor::GetSingleton()
