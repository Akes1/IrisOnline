/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.09
* 내    용 : 월드 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cMaterialData;
class cTerrain;
class cSceneNode;
class cAreaSceneNode;

/// 월드 관리자
class cWorldManager
{
	static cWorldManager* mSingleton;

public:
	cWorldManager();
	~cWorldManager();

	/// 지움
	void Clear();

	/// 초기화
	void Init();

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime, bool testing );
	void ProcessAmbient( unsigned long deltaTime );
	void ProcessFog( unsigned long deltaTime );

	/// 렌더링
	void Render();
	void RenderSkyDome();

	/// 하늘
	bool LoadSkyDome( const cString& pathName );
	bool LoadSkyDome( const cString& pathName, float scale, bool enabled );
	void SetSkyScale( float scale );
	void SetSkyEnabled( bool enabled );
	void SetSkyAmbient( const NiColor& color );
	void SetSkyDimmer( float dimmer );
	const char* GetSkyName() const;
	float GetSkyScale() const;
	bool IsSkyEnabled() const;

	/// 영역
	void SetArea( const cAreaSceneNode& n );
	void BlendArea( const cAreaSceneNode& n0, const cAreaSceneNode& n1, float ratio );

protected:
	/// 기하 노드들을 수집
	void CollectGeometries( tList<NiRenderObject*>* pgeomList, NiAVObject* obj );
	void CollectProperties( tList<NiMaterialProperty*>* ppropList, NiAVObject* obj );
	void SetMaterial( tList<NiMaterialProperty*>& propList, const cMaterialData& material );

	/// 해당 오브젝트의 깊이 버퍼를 비활성화
	void DisableZBuffer( NiAVObject* obj );

public:
	/// 단일체를 리턴
	static cWorldManager* GetSingleton();

protected:
	/// 지형
	cTerrain* mTerrain;

	/// 하늘
	//NiFogPropertyPtr mSkyFogProp;
	NiAmbientLightPtr mSkyLight;
	NiNodePtr mSkyDome;
	
	typedef tList<NiRenderObject*> cRenderObjList;
	cRenderObjList mSkyGeomList;
	
	typedef tList<NiMaterialProperty*> cMatPropList;
	cMatPropList mSkyMatPropList;

	bool mSkyEnabled;
	cString mSkyName;

	/// 주변광
	NiColor mTargetSkyAmbient;
	NiColor mSkyAmbient;
	float mTargetSkyDimmer;
	float mSkyDimmer;
	NiColor mTargetTerrainAmbient;
	NiColor mTerrainAmbient;
	float mTargetTerrainDimmer;
	float mTerrainDimmer;
	NiColor mTargetStaticObjectAmbient;
	NiColor mStaticObjectAmbient;
	float mTargetStaticObjectDimmer;
	float mStaticObjectDimmer;
	NiColor mTargetDynamicObjectAmbient;
	NiColor mDynamicObjectAmbient;
	float mTargetDynamicObjectDimmer;
	float mDynamicObjectDimmer;

	/// 안개
	NiColor mTargetFogColor;
	NiColor mFogColor;
	float mTargetFogDepth;
	float mFogDepth;

	/// 충돌 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;
};

inline
void cWorldManager::SetSkyAmbient( const NiColor& color )
{
	mSkyLight->SetAmbientColor( color );
}

inline
void cWorldManager::SetSkyDimmer( float dimmer )
{
	mSkyLight->SetDimmer( dimmer );
}

inline
void cWorldManager::SetSkyScale( float scale )
{
	if( mSkyDome )
	{
		mSkyDome->SetScale( scale );
		mSkyDome->Update( 0.0f );
	}
}

inline
void cWorldManager::SetSkyEnabled( bool enabled )
{
	mSkyEnabled = enabled;
}

inline
const char* cWorldManager::GetSkyName() const
{
	return mSkyName.Cstr();
}

inline
float cWorldManager::GetSkyScale() const
{
	if( mSkyDome )
		return mSkyDome->GetWorldScale();
	else
		return 1.0f;
}

inline
bool cWorldManager::IsSkyEnabled() const
{
	return mSkyEnabled;
}

inline
cWorldManager* cWorldManager::GetSingleton()
{
	return mSingleton;
}

#define WORLDMAN cWorldManager::GetSingleton()
