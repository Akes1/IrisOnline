/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.15
* 내    용 : 장면 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneTree.h"
#include "VisibleArray.h"
#include "VisibleArraySorter.h"

class cRay;
class cCamera;
class cTerrain;
class cAreaGroup;
class cAreaSceneNode;
class cAreaSceneNodeParam;
class cLightSceneNode;
class cLightSceneNodeParam;
#ifdef MAP_EDITOR
class cSoundSceneNode;
#endif
class cSoundSceneNodeParam;
class cStaticSceneNode;
class cStaticSceneNodeParam;
class cDynamicSceneNode;
class cNpcSceneNode;
class cNpcSceneNodeParam;
class cMonsterSceneNode;
class cMonsterSceneNodeParam;
class cPortalSceneNode;
class cPortalSceneNodeParam;
class cGatheringSceneNode;
class cGatheringSceneNodeParam;
class cPatternSceneNode;
class cPatternSceneNodeParam;
class cDoorSceneNode;
class cDoorSceneNodeParam;
class cShadowGeometry;


/// 장면 파일 헤더
#pragma pack( push, 1 )
class cSceneFileHeader
{
public:
	/// 식별 코드
	char mCode[10];

	/// 버전
	unsigned int mVersion;

	/// 장면 트리 정보
	NiPoint3 mCenter;
	float mMinRadius;
	float mMaxRadius;

	/// 장면 노드 수
	unsigned int mNumSceneNodes;
};

class cSoundSceneFileHeader
{
public:
	/// 식별 코드
	char mCode[10];

	/// 버전
	unsigned int mVersion;

	/// 장면 노드 수
	unsigned int mNumSceneNodes;
};
#pragma pack( pop )

/// 장면 컬러
class cSceneCuller : public NiMeshCullingProcess
{
public:
	cSceneCuller( const cCamera* cam, const NiPoint3& heroPos, bool testing );

#ifdef MAP_EDITOR
	/// 가시 레벨
	unsigned int GetVisibleLevel( const NiPoint3& pos );
#endif

public:
	const cCamera* mCamera;
	NiPoint3 mHeroPos;
	bool mPlaying;

#ifdef MAP_EDITOR
	float mSqaredLevelDistance0;
	float mSqaredLevelDistance1;
	float mSqaredLevelDistance2;
#endif
};

class cModifierProcess : public NiMeshCullingProcess
{
public:
	cModifierProcess();
	virtual ~cModifierProcess();

	void Cull( const NiCamera* pcamera, cSceneNode* pnode );

protected:
	virtual void AppendVirtual( NiRenderObject& /*geom*/ ){}
	virtual void Process( NiAVObject* pobj );
	virtual void Process(const NiCamera* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet);
};



/// 장면 관리자
/// 장면 노드를 생성, 제거하는 관리자이다.
/// 장면을 렌더링하고,
/// 충돌 검사, 가시성 검사, 정렬 등의 작업을 수행한다.
class cSceneManager
{
	static cSceneManager* mSingleton;
	friend class cObjectEditor;
	friend class cLightEditor;

public:
	cSceneManager();
	~cSceneManager();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( const cTerrain& terrain, float minRadius = 1000 );
	bool Init( const NiPoint3& center, float minRadius, float maxRadius );
	bool InitTree( const cTerrain& terrain, float minRadius = 1000 );
	bool InitTree( const NiPoint3& center, float minRadius, float maxRadius );
	bool IsInited() const;

	/// 로딩
#ifdef MAP_EDITOR
	bool Load( const cString& pathName, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet, bool add = false );
	bool LoadSound( const cString& pathName, tHashSet<cSoundSceneNode*>& soundSet );
#else
	bool Load( const cString& pathName, bool add = false );
	bool LoadSound( const cString& pathName );
#endif

	bool LoadEnvVer6( cFileLoader& loader, bool add );
	bool LoadEnvVer7( cFileLoader& loader, bool add );
	bool LoadEnvVer8( cFileLoader& loader, bool add );
	bool LoadEnvVer9( cFileLoader& loader , bool add);
	bool LoadEnvVer10( cFileLoader& loader, bool add );
	bool LoadGlobalAreaVer6( cFileLoader& loader, bool add );
	bool LoadGlobalAreaVer7( cFileLoader& loader, bool add );
	bool LoadGlobalAreaVer8( cFileLoader& loader, bool add );
	bool LoadGlobalAreaVer9( cFileLoader& loader, bool add );
	bool LoadGlobalAreaVer10( cFileLoader& loader, bool add );
#ifdef MAP_EDITOR
	bool LoadVer6( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer7( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer8( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer9( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer10( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cSoundSceneNode*>& soundSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer11( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadVer12( cFileLoader& loader, cSceneFileHeader& header, tHashSet<cStaticSceneNode*>& objectSet, tHashSet<cLightSceneNode*>& lightSet, tHashSet<cAreaSceneNode*>& areaSet );
	bool LoadAreaGroup( const cString& pathName );
	bool LoadObjectTransformScript( const cString& pathName, tHashSet<cStaticSceneNode*>& objectSet );

	bool LoadSoundVer11( cFileLoader& loader, cSoundSceneFileHeader& header, tHashSet<cSoundSceneNode*>& soundSet );
#else
	bool LoadVer6( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer7( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer8( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer9( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer10( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer11( cFileLoader& loader, cSceneFileHeader& header );
	bool LoadVer12( cFileLoader& loader, cSceneFileHeader& header );

	bool LoadSoundVer11( cFileLoader& loader, cSoundSceneFileHeader& header );
#endif

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime, bool testing );

	/// 렌더링
	void Render();
	void RenderShadow();

	/// 오브젝트 위치, 크기 조절
	void AdjustAllPos( float scale );
	void AdjustAllSize( float scale );

	/// 화면 상의 좌표에 대응하는 장면 노드들을 리턴
	/// 삼각형 단위로 검사한다.
	bool Pick( tArray<cSceneNode*>* pickedArray, int mouseX, int mouseY, bool sortByDistance, eSceneNodeType type );

	/// 충돌하는 장면 노드들을 수집
	bool Pick( tArray<cSceneNode*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance, eSceneNodeType type );
	bool Pick( tArray<cSceneNode*>* pickedArray, const cSphere& sphere, eSceneNodeType type );

	/// 노드를 제거
	void DestroyAll();
	bool DestroyNode( cSceneNode* node );
	bool DestroyNode( eSceneNodeType type, unsigned int index );

	/// 노드를 생성
	cAreaSceneNode* CreateArea( const cAreaSceneNodeParam& param );
	cLightSceneNode* CreateLight( const cLightSceneNodeParam& param );
	cStaticSceneNode* CreateStatic( cStaticSceneNodeParam& param );
	cNpcSceneNode* CreateNPC( const cNpcSceneNodeParam& param );
	cMonsterSceneNode* CreateMonster( const cMonsterSceneNodeParam& param );
	cGatheringSceneNode* CreateGathering( cGatheringSceneNodeParam& param );
	cPortalSceneNode* CreatePortal( cPortalSceneNodeParam& param );
	cPatternSceneNode* CreatePattern( cPatternSceneNodeParam& param );
	cDoorSceneNode* CreateDoor( cDoorSceneNodeParam& param );

#ifdef MAP_EDITOR
	cSoundSceneNode* CreateSound( const cSoundSceneNodeParam& param );

	void SetStaticeObjectWireframe( bool view ) { mStaticObjectWireframe = view; }
#endif

	/// 주변광
	void SetAmbientLightAmbient( unsigned int i, const NiColor& color );
	void SetAmbientLightDiffuse( unsigned int i, const NiColor& color );
	void SetAmbientLightDimmer( unsigned int i, float dimmer );
	NiAmbientLight* GetAmbientLight( unsigned int i ) const;

	/// 방향광
	void SetDirLightAmbient( unsigned int i, const NiColor& color );
	void SetDirLightDiffuse( unsigned int i, const NiColor& color );
	void SetDirLightDimmer( unsigned int i, float dimmer );
	void SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle );
	NiDirectionalLight* GetDirLight( unsigned int i ) const;

	/// 안개
	void SetFog( bool enabled, const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );
	NiFogProperty* GetFogProperty() const;

	/// 가시 레벨 거리
	void SetLevelDistance( float dist0, float dist1, float dist2 );
	float GetLevelDistance0() const;
	float GetLevelDistance1() const;
	float GetLevelDistance2() const;

	unsigned int CalcVisibleLevel( cStaticSceneNode* n );

	/// 장면 트리의 정보
	const NiPoint3& GetCenter() const;
	float GetMinRadius() const;
	float GetMaxRadius() const;

	/// 보이는 장면 노드 수
	unsigned int GetNumVisibleAlphaNodes() const;
	unsigned int GetNumVisibleSolidNodes() const;

	/// 전체 영역 노드
	cAreaSceneNode* GetGlobalArea();

	/// 그림자
	void AddVisibleShadowGeom( cShadowGeometry* geom );

	void OnVisibleModifierPocess( cSceneNode* node );

protected:
	/// 노드를 추가
	void AddNode( cSceneNode* node );

public:
	/// 단일체를 리턴
	static cSceneManager* GetSingleton();

protected:
	/// 초기화 여부
	bool mInited;

	/// 텍스처 맵
	typedef tHashMap<unsigned int, NiTexture*> cTextureMap;
	cTextureMap mTextureMap;

	/// 트리
	/// 공간을 분할하여 노드들을 계층적으로 관리한다.
	/// 충돌 검사 및 컬링을 효율적으로 수행하기위해 사용한다.
	NiPoint3 mCenter;
	float mMinRadius;
	float mMaxRadius;
	cSceneTree mNodeTree;

	cModifierProcess mModifierProcess;

	/// 영역 배열
	typedef tArray<cAreaSceneNode*> cAreaArray;
	cAreaArray mAreaArray;

	/// 조명 배열
	typedef tArray<cLightSceneNode*> cLightArray;
	cLightArray mLightArray;

	/// 정적 노드 배열
	typedef tArray<cStaticSceneNode*> cStaticArray;
	cStaticArray mStaticArray;

#ifdef MAP_EDITOR
	bool mStaticObjectWireframe;
#endif

	/// 동적 노드 배열
	typedef tArray<cDynamicSceneNode*> cDynamicArray;
	cDynamicArray mDynamicArray;

#ifdef MAP_EDITOR
	/// 사운드 배열
	typedef tArray<cSoundSceneNode*> cSoundArray;
	cSoundArray mSoundArray;
#endif

	/// 가시 레벨 거리
	float mLevelDistance0;
	float mLevelDistance1;
	float mLevelDistance2;
	float mSqaredLevelDistance0;
	float mSqaredLevelDistance1;
	float mSqaredLevelDistance2;

	/// 반투명 노드 가시 배열
	cVisibleArray mAlphaArray;
	cVisibleArray mAlphaTestArray;

	/// 불투명 노드 가시 배열
	cVisibleArray mSolidArray;

	/// 반투명 노드 가시 배열 정렬자
	cAlphaArraySorter mAlphaArraySorter;

	/// 불투명 노드 가시 배열 정렬자
	cSolidArraySorter mSolidArraySorter;

	/// 안개 속성
	NiFogPropertyPtr mFogProp;

	/// 조명
	NiAmbientLightPtr mAmbientLight[2];
	NiDirectionalLightPtr mDirLight[2];

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 전체 영역 노드
	cAreaSceneNode* mGlobalArea;

#ifdef MAP_EDITOR
	/// 영역 그룹
	typedef tHashMap<unsigned int, cAreaGroup*> cAreaGroupMap;
	cAreaGroupMap mAreaGroupMap;
#endif

	/// 그림자 가시 배열
	typedef tArray<cShadowGeometry*> cShadowGeomArray;
	cShadowGeomArray mShadowGeomArray;

	NiPoint3 mHeroPos;
};

inline
bool cSceneManager::IsInited() const
{
	return mInited;
}

inline
NiFogProperty* cSceneManager::GetFogProperty() const
{
	return mFogProp;
}

inline
NiAmbientLight* cSceneManager::GetAmbientLight( unsigned int i ) const
{
	return mAmbientLight[i];
}

inline
NiDirectionalLight* cSceneManager::GetDirLight( unsigned int i ) const
{
	return mDirLight[i];
}

inline
const NiPoint3& cSceneManager::GetCenter() const
{
	return mCenter;
}

inline
float cSceneManager::GetMinRadius() const
{
	return mMinRadius;
}

inline
float cSceneManager::GetMaxRadius() const
{
	return mMaxRadius;
}

inline
unsigned int cSceneManager::GetNumVisibleAlphaNodes() const
{
	return mAlphaArray.GetCount();
}

inline
unsigned int cSceneManager::GetNumVisibleSolidNodes() const
{
	return mSolidArray.GetCount();
}

inline
cAreaSceneNode* cSceneManager::GetGlobalArea()
{
	return mGlobalArea;
}

inline
void cSceneManager::SetLevelDistance( float dist0, float dist1, float dist2 )
{
	mLevelDistance0 = dist0;
	mLevelDistance1 = dist1;
	mLevelDistance2 = dist2;
	mSqaredLevelDistance0 = mLevelDistance0 * mLevelDistance0;//dist0 * dist0;
	mSqaredLevelDistance1 = mLevelDistance1 * mLevelDistance1;//dist1 * dist1;
	mSqaredLevelDistance2 = mLevelDistance2 * mLevelDistance2;//dist2 * dist2;
}

inline
float cSceneManager::GetLevelDistance0() const
{
	return mLevelDistance0;
}

inline
float cSceneManager::GetLevelDistance1() const
{
	return mLevelDistance1;
}

inline
float cSceneManager::GetLevelDistance2() const
{
	return mLevelDistance2;
}

inline
void cSceneManager::AddVisibleShadowGeom( cShadowGeometry* geom )
{
	assert( geom );

	mShadowGeomArray.PushBack( geom );
}

inline
cSceneManager* cSceneManager::GetSingleton()
{
	return mSingleton;
}

#define SCENEMAN cSceneManager::GetSingleton()
