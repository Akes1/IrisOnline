/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.15
* 내    용 : 장면 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneTree.h"
#include "CullingProcess.h"
#include "VisibleArray.h"
#include "VisibleArraySorter.h"
#include "IndexGenerator.h"

class cRay;
class cCamera;
class cTerrain;
class cAreaGroup;
class cAreaSceneNode;
class cAreaSceneNodeParam;
class cLightSceneNode;
class cLightSceneNodeParam;
class cSoundSceneNode;
class cSoundSceneNodeParam;
class cStaticSceneNode;
class cManagedStaticSceneNode;
class cStaticSceneNodeParam;
class cDynamicSceneNode;
class cHeroSceneNode;
class cPlayerSceneNode;
class cPlayerSceneNodeParam;
class cMonsterSceneNode;
class cTransMonSceneNode;
class cMonsterSceneNodeParam;
class cRefereeSceneNode;
class cNpcSceneNode;
class cNpcSceneNodeParam;
class cVehicleSceneNode;
class cVehicleSceneNodeParam;
class cEffectSceneNode;
class cEffectSceneNodeParam;
class cDropItemSceneNode;
class cDropItemSceneNodeParam;
class cMapPortalSceneNode;
class cMapPortalSceneNodeParam;
class cCardSceneNode;
class cCardSceneNodeParam;
class cGatheringSceneNode;
class cGatheringSceneNodeParam;

class cDepthPlane;
class cPlane;
class cShadowGeometry;
class cRibbonGeometry;

enum eNameCardOption;

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

class cSceneCuller : public NiMeshCullingProcess
{
public:
	cSceneCuller( const cCamera* cam, const NiPoint3& heroPos );
public:
	const cCamera* mCamera;
	NiPoint3 mHeroPos;
};


/// 장면 관리자
/// 장면 노드를 생성, 제거하는 관리자이다.
/// 장면을 렌더링하고,
/// 충돌 검사, 가시성 검사, 정렬 등의 작업을 수행한다.
class cSceneManager
{
	friend class cLightAgent;
	static cSceneManager* mSingleton;

public:
	cSceneManager();
	~cSceneManager();

	/// 어플리케이션 시작 및 종료 시점
	bool Init();
	void Exit();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( const NiPoint3& center, float minRadius, float maxRadius );
	inline bool IsInited() const { return mInited; }

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 렌더링
	void Render();
	void RenderZFalseDamage();
	void RenderTrail();

	/// 2D 객체 랜더링 (말풍선 etc.)
	void Render2D();

	/// 가시 레벨 거리
	void SetLevelDistance( float dist0, float dist1, float dist2 );

	/// 화면 상의 좌표에 대응하는 장면 노드들을 리턴
	/// 삼각형 단위로 검사한다.
	bool PickStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );
	bool PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );
	bool PickManagedStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );

	/// 반직선과 충돌하는 장면 노드들을 수집
	/// 삼각형 단위로 검사한다.
	bool PickStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );
	bool PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );
	bool PickManagedStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool Pick_Kind( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type );
	bool Pick_Type( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type );
	bool PickAreas( tArray<void*>* pickedArray, const cSphere& sphere );
	bool PickLights( tArray<void*>* pickedArray, const cSphere& sphere );
	bool PickSounds( tArray<void*>* pickedArray, const cSphere& sphere );

	/// 영역 장면 노드를 생성
	cAreaSceneNode* CreateArea( cAreaSceneNodeParam& param );

	/// 조명 장면 노드를 생성
	cLightSceneNode* CreateLight( cLightSceneNodeParam& param );

	/// 사운드 장면 노드를 생성
	cSoundSceneNode* CreateMapSound( cSoundSceneNodeParam& param );
	cSoundSceneNode* CreateEffectSound( cSoundSceneNodeParam& param );

	/// 정적 장면 노드를 생성
	cStaticSceneNode* CreateStatic( cStaticSceneNodeParam& param );
	cDropItemSceneNode* CreateDropItem( cDropItemSceneNodeParam& param );
	cGatheringSceneNode* CreateGathering( cGatheringSceneNodeParam& param );
	cMapPortalSceneNode* CreateMapPortal(  unsigned long scriptIdx, NiPoint3 pos, NiMatrix3 rot );
	cCardSceneNode* CreateCard( cCardSceneNodeParam& param );

	void InterpretPlayer( unsigned long accumtime );
	void InterpretMonster( unsigned long accumtime );
	void InterpretNpc( unsigned long accumtime );

	/// 관리 객체 생성용 외부 인터페이스
	cPlayerSceneNode* CreatePlayer( unsigned long modelIndex );
	void CreateDummy( NiPoint3 pos );

	cMonsterSceneNode* CreateMonster( unsigned long monIdx, unsigned long modelIndex );
	cNpcSceneNode* CreateNPC( unsigned long modelIndex );

	cEffectSceneNode* CreateEffect( cEffectSceneNodeParam& param );
	cEffectSceneNode* CreateSelfEffect( cString pathName, bool distCheck, NiTransform& trans, bool bLoop );
	cEffectSceneNode* CreateDamageEffect( cEffectSceneNodeParam& param, bool IsMiss, bool zFalse );

	/// 관리 객체의 제거용 외부 인터페이스
	bool DestroyDynamic( cSceneNode* node );
	bool DestroyStaticNode( cSceneNode* node );

	/// Deletelist에서 제거하지 않고 외부에서 임의로 호출하는 경우는 DelListRemove = ture로 설정하시오
	bool DestroyEffectNode( cSceneNode* node );
	bool DestroySoundNode( cSceneNode* node );

	/// 제거 되어야 할 이펙트 노드에 대한 처리
	void AddDeleteEffectNode( cEffectSceneNode* node );

	/// 제거 되어야 할 사운드 노드에 대한 처리
	void AddDeleteSoundNode( cSoundSceneNode* node );

	/// 모든 노드들을 제거
	void DestroyAll();

	/// 전체 영역 노드
	cAreaSceneNode* GetGlobalArea();

	/// 그림자 렌더링 속성
	NiZBufferProperty* GetShadowZBuffProperty() const;
	NiAlphaProperty* GetShadowAlphaProperty() const;
	NiTexturingProperty* GetShadowTexProperty() const;

	void AddPlane( cPlane* pPlane );
	///

	///
	void AddVisibleTrailGeom( cRibbonGeometry* geom );

	inline void SetScreenShotMode( bool enable ) { mScreenShotMode = enable; }
	inline bool IsScreenShotMode() { return mScreenShotMode; }

	unsigned int CalcVisibleLevel( cStaticSceneNode* n );
	bool CalcDynamicVisibleLevel( cSceneNode* n );


	cEffectSceneNode* GetEffectSceneNode( unsigned long effectIdx );
	cSoundSceneNode* GetSoundSceneNode( unsigned long soundIdx );
	cDynamicSceneNode* GetDynamicSceneNode( unsigned long dynIdx );
	cStaticSceneNode* GetStaticSceneNode( unsigned long nodeIdx );

	cManagedStaticSceneNode* GetManagedStaticNode( unsigned int type, unsigned long nodeIdx );

	/// 검증코드
	bool IsValidEffectNode( cEffectSceneNode* node );
	bool IsValidEffectNode( unsigned long idx, cEffectSceneNode* node );

	bool IsValidSoundNode( cSoundSceneNode* node );
	bool IsValidSoundNode( unsigned long idx, cSoundSceneNode* node );

	bool IsValidDynamicNode( cDynamicSceneNode* node );
	bool IsValidDynamicNode( unsigned long idx, cDynamicSceneNode* node );

	void SetLoadingFlag( bool set ) { mEnableLoading = set; }


	cDynamicSceneNode* GetCurrentSceneNode() { return mpCurrentSceneNode; }
	NiNode* GetSceneDummy();

	inline void SetDramaProcess( bool isDrama ) { mDramaProcess = isDrama; }

	/// light
	void AddAmbientLight( NiColor ambientColor, NiColor diffuseColor, float dimmer );
	void AddDirLight( NiColor ambientColor, NiColor diffuseColor, float dimmer, NiMatrix3 rot );

	NiAmbientLight* GetAmbientLight() const;
	NiDirectionalLight* GetDirLight() const;

	inline unsigned int GetLightCount() { return mAmbientLightArray.GetSize(); }
	inline unsigned int GetCurrentLightIndex() { return mCurrentLightIndex; }

	void ChangeLight( unsigned int index );

	void OnVisibleModifierPocess( cSceneNode* node );

protected:
	/// 노드를 추가
	bool AddAreaNode( cSceneNode* node );
	bool AddLightNode( cSceneNode* node );
	bool AddSoundNode( cSceneNode* node );
	bool AddStaticNode( cSceneNode* node );
	bool AddDynamicNode( cSceneNode* node );
	bool AddEffectNode( cEffectSceneNode* node );

	void LoadMapLightSetting( const char* fullPath );
	void ScanDir( LPCTSTR pathname );

	bool LoadEnv6( cFileLoader& loader );
	bool LoadEnv7( cFileLoader& loader );
	bool LoadEnv8( cFileLoader& loader );
	bool LoadEnv9( cFileLoader& loader );
	bool LoadEnv10( cFileLoader& loader );
	bool LoadGlobalArea6( cFileLoader& loader );
	bool LoadGlobalArea7( cFileLoader& loader );
	bool LoadGlobalArea8( cFileLoader& loader );
	bool LoadGlobalArea9( cFileLoader& loader );
	bool LoadGlobalArea10( cFileLoader& loader );


public:
	/// 단일체를 리턴
	static cSceneManager* GetSingleton();

protected:
	/// 초기화 여부
	bool mInited;

	//// 보여질 객체 자체..
	cDynamicSceneNode*			mpCurrentSceneNode;

	cSceneNode* mpDummyNode;
	NiNode* mpDummy;

	bool mDramaProcess;


	bool mEnableLoading;

	/// 버전
	unsigned int mVersion;

	cIndexGenerator<unsigned long> mSoundIndexGen;
	cIndexGenerator<unsigned long> mStaticNodeIndexGen;
	cIndexGenerator<unsigned long> mDynamicNodeIndexGen;
	cIndexGenerator<unsigned long> mEffectIndexGen;

	typedef tPointerHashMap<unsigned long, void*> cSoundMap;
	typedef tPointerHashMap<unsigned long, void*> cNodeMap;


	/// 트리
	/// 공간을 분할하여 노드들을 계층적으로 관리한다.
	/// 충돌 검사 및 컬링을 효율적으로 수행하기위해 사용한다.
	cSceneTree mNodeTree;

	cSoundMap mSoundArray;

	/// 캐릭터 (PC, NPC, Monster)
	cNodeMap mDynamicObjectArray;

	/// 환경 객체 노드 배열
	cNodeMap mStaticObjectArray;

	/// 이펙트 (트리로 관리할 필요 없음)
	cNodeMap mEffectNodeArray;

	/// 이펙트 삭제용
	//	typedef tPointerList<cSceneNode*> cDelEffectList;
	typedef tPointerList<void*> cDelEffectList;
	cDelEffectList mDelEffectList;

	/// 사운드 삭제용
	typedef tPointerList<void*> cDelSoundList;
	cDelSoundList mDelSoundList;


	/// 가시 레벨 거리
	float mSqaredLevelDistance0;
	float mSqaredLevelDistance1;
	float mSqaredLevelDistance2;

	/// 컬링
	cCullingProcess mCuller;
	cCullingProcess mDamageCuller;

	cModifierProcess mModifierProcess;

	/// 반투명 노드 가시 배열
	cVisibleArray mAlphaArray;
	cVisibleArray mAlphaTestArray;
	cVisibleArray mDamageArray;

	/// 반투명 노드 가시 배열 정렬자
	cAlphaArraySorter mAlphaArraySorter;
	cAlphaArraySorter mDamageSorter;

	/// 불투명 노드 가시 배열
	cVisibleArray mSolidArray;

	/// 불투명 노드 가시 배열 정렬자
	cSolidArraySorter mSolidArraySorter;

	/// 픽킹된 장면 노드 배열
	typedef tArray<void*> cPickedArray;
	cPickedArray mPickedArray;

	/// 텍스처 맵
	typedef tHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mTextureMap;

	/// 
	typedef tPointerArray<cRibbonGeometry*> cTrailGeomArray;
	cTrailGeomArray mTrailGeomArray;

	///
	bool mScreenShotMode;

	NiPoint3 mHeroPos;

	/// light
	NiTObjectArray<NiAmbientLightPtr>		mAmbientLightArray;
	NiTObjectArray<NiDirectionalLightPtr>	mDirLightArray;
	unsigned int mCurrentLightIndex;
};

inline 
void cSceneManager::AddVisibleTrailGeom( cRibbonGeometry* geom )
{
	assert( geom );

	mTrailGeomArray.PushBack( geom );
}

inline
NiAmbientLight* cSceneManager::GetAmbientLight() const
{
	return mAmbientLightArray.GetAt( 0 );
}

inline
NiDirectionalLight* cSceneManager::GetDirLight() const
{
	return mDirLightArray.GetAt( 0 );
}

inline
void cSceneManager::SetLevelDistance( float dist0, float dist1, float dist2 )
{
	mSqaredLevelDistance0 = dist0 * dist0;
	mSqaredLevelDistance1 = dist1 * dist1;
	mSqaredLevelDistance2 = dist2 * dist2;
}

inline 
cEffectSceneNode* cSceneManager::GetEffectSceneNode( unsigned long effectIdx ) 
{ 
	return (cEffectSceneNode*)mEffectNodeArray.GetAt( effectIdx ); 
}

inline 
cSoundSceneNode* cSceneManager::GetSoundSceneNode( unsigned long soundIdx ) 
{ 
	return (cSoundSceneNode*)mSoundArray.GetAt( soundIdx ); 
}

inline 
cDynamicSceneNode* cSceneManager::GetDynamicSceneNode( unsigned long dynIdx ) 
{ 
	return (cDynamicSceneNode*)mDynamicObjectArray.GetAt( dynIdx ); 
}

inline 
cStaticSceneNode* cSceneManager::GetStaticSceneNode( unsigned long nodeIdx ) 
{ 
	return (cStaticSceneNode*) mStaticObjectArray.GetAt( nodeIdx ); 
}

inline 
NiNode* cSceneManager::GetSceneDummy()
{
	if( mpDummyNode == 0 )
		return 0;

	return mpDummyNode->GetNiNode();
}

inline
cSceneManager* cSceneManager::GetSingleton()
{
	return mSingleton;
}

#define SCENEMAN cSceneManager::GetSingleton()

/*
#pragma once

#include "SceneTree.h"
#include "VisibleArray.h"
#include "VisibleArraySorter.h"
#include "CullingProcess.h"

class cRay;
class cCamera;
class cDynamicSceneNode;
class cNpcSceneNode;
class cNpcSceneNodeParam;
class cMonsterSceneNode;
class cMonsterSceneNodeParam;
class cPlayerSceneNode;
class cPlayerSceneNodeParam;
class cEffectSceneNode;
class cEffectSceneNodeParam;
class cSoundSceneNode;
class cSoundSceneNodeParam;
class cStaticSceneNode;

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
#pragma pack( pop )

/// 장면 컬러
class cSceneCuller : public NiCullingProcess
{
public:
	cSceneCuller( const cCamera* cam, const NiPoint3& heroPos );

public:
	const cCamera* mCamera;
	NiPoint3 mHeroPos;
};

/// 장면 관리자
/// 장면 노드를 생성, 제거하는 관리자이다.
/// 장면을 렌더링하고,
/// 충돌 검사, 가시성 검사, 정렬 등의 작업을 수행한다.
class cSceneManager
{
	static cSceneManager* mSingleton;
public:
	cSceneManager();
	~cSceneManager();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( const NiPoint3& center, float minRadius, float maxRadius );
	bool InitTree( const NiPoint3& center, float minRadius, float maxRadius );
	bool IsInited() const;

	/// 처리
	void Process( float deltaTime, float accumTime );

	/// 렌더링
	void Render();

	/// 화면 상의 좌표에 대응하는 장면 노드들을 리턴
	/// 삼각형 단위로 검사한다.
	bool PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );

	/// 충돌하는 장면 노드들을 수집
	bool PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );
	bool PickDynamics( tArray<void*>* pickedArray, const cSphere& sphere );
	bool PickSounds( tArray<void*>* pickedArray, const cSphere& sphere );

	/// 노드를 제거
	void DestroyAll();
	bool DestroyDynamic( cSceneNode* node );
	bool DestroyDynamic( unsigned int index );
	bool DestroyEffectNode( cSceneNode* node, bool DelListRemove = false );
	bool DestroySoundNode( cSceneNode* node, bool DelListRemove = false );

	/// 제거 되어야 할 이펙트 노드에 대한 처리
	void AddDeleteEffectNode( cEffectSceneNode* node );
	void AddDeleteSoundNode( cSoundSceneNode* node );

	/// 노드를 생성
	cSoundSceneNode* CreateEffectSound( const cSoundSceneNodeParam& param );

	void ActiveSoundNodeAll( bool active );

	cNpcSceneNode* CreateNPC( unsigned long modelIndex );
	cMonsterSceneNode* CreateMonster( unsigned long modelIndex );
	cPlayerSceneNode* CreatePlayer( unsigned long modelIndex );

	cEffectSceneNode* CreateEffect( const cEffectSceneNodeParam& param );
	cEffectSceneNode* CreateSelfEffect( cString pathName, NiTransform& trans, bool bLoop );

	void AddAmbientLight( NiColor ambientColor, NiColor diffuseColor, float dimmer );
	void AddDirLight( NiColor ambientColor, NiColor diffuseColor, float dimmer, NiMatrix3 rot );

	NiAmbientLight* GetAmbientLight() const;
	NiDirectionalLight* GetDirLight() const;

	inline unsigned int GetLightCount() { return mAmbientLightArray.GetSize(); }
	inline unsigned int GetCurrentLightIndex() { return mCurrentLightIndex; }

	void ChangeLight( unsigned int index );

	/// 주변광
	//void SetAmbientLightAmbient( unsigned int i, const NiColor& color );
	//void SetAmbientLightDiffuse( unsigned int i, const NiColor& color );
	//void SetAmbientLightDimmer( unsigned int i, float dimmer );

	///// 방향광
	//void SetDirLightAmbient( unsigned int i, const NiColor& color );
	//void SetDirLightDiffuse( unsigned int i, const NiColor& color );
	//void SetDirLightDimmer( unsigned int i, float dimmer );
	//void SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle );



	/// 장면 트리의 정보
	const NiPoint3& GetCenter() const;
	float GetMinRadius() const;
	float GetMaxRadius() const;

	/// 보이는 장면 노드 수
	unsigned int GetNumVisibleAlphaNodes() const;
	unsigned int GetNumVisibleSolidNodes() const;

	cDynamicSceneNode* GetCurrentSceneNode() { return mpCurrentSceneNode; }

	inline void SetDramaProcess( bool isDrama ) { mDramaProcess = isDrama; }

	inline NiNode* GetSceneDummy() { return mpDummy; }

protected:
	/// 노드를 추가
	unsigned int AddDynamic( cDynamicSceneNode* node );
	unsigned int AddStatic( cSceneNode* node );
	unsigned int AddEffectNode( cSceneNode* node );
	unsigned int AddSound( cSoundSceneNode* node );

	void LoadMapLightSetting( const char* fullPath );
	void ScanDir( LPCTSTR pathname, bool filescan = false );

	bool LoadEnv6( cFileLoader& loader );
	bool LoadEnv7( cFileLoader& loader );
	bool LoadEnv8( cFileLoader& loader );
	bool LoadEnv9( cFileLoader& loader );
	bool LoadEnv10( cFileLoader& loader );
	bool LoadGlobalArea6( cFileLoader& loader );
	bool LoadGlobalArea7( cFileLoader& loader );
	bool LoadGlobalArea8( cFileLoader& loader );
	bool LoadGlobalArea9( cFileLoader& loader );
	bool LoadGlobalArea10( cFileLoader& loader );

	void InterpretPlayer( float accumtime );
	void InterpretMonster( float accumtime );
	void InterpretNpc( float accumtime );

	void CreateDummy( NiPoint3 pos );

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

	/// 컬링
	cCullingProcess mCuller;

	typedef tArray<void*> cNodeArray;

	//// 보여질 객체 자체..
	cDynamicSceneNode*			mpCurrentSceneNode;

	unsigned int mDummyIdx;
	NiNode* mpDummy;

	cSceneTree mSoundTree;
	cNodeArray mSoundArray;


	cSceneTree mDynamicTree;
	cNodeArray mDynamicArray;

	cSceneTree mStaticObjectTree;
	cNodeArray mStaticObjectArray;

	/// 이펙트 (트리로 관리할 필요 없음)
	cNodeArray mEffectNodeArray;

	/// 이펙트 삭제용
	//	typedef tPointerList<cSceneNode*> cDelEffectList;
	typedef tPointerList<void*> cDelEffectList;
	cDelEffectList mDelEffectList;

	typedef tPointerList<void*> cDelSoundList;
	cDelSoundList mDelSoundList;


	/// 반투명 노드 가시 배열
	cVisibleArray mAlphaArray;

	/// 불투명 노드 가시 배열
	cVisibleArray mSolidArray;
	cVisibleArray mSkinedArray;

	/// 반투명 노드 가시 배열 정렬자
	cAlphaArraySorter mAlphaArraySorter;

	/// 불투명 노드 가시 배열 정렬자
	cSolidArraySorter mSolidArraySorter;
	cSolidArraySorter mSkinedArraySorter;

	unsigned int mCurrentLightIndex;

	bool mDramaProcess;

	/// light
	NiTObjectArray<NiAmbientLightPtr>		mAmbientLightArray;
	NiTObjectArray<NiDirectionalLightPtr>	mDirLightArray;

	/// 조명
//	NiAmbientLightPtr mAmbientLight[2];
//	NiDirectionalLightPtr mDirLight[2];

	/// 픽킹된 장면 노드 배열
	typedef tArray<void*> cPickedArray;
	cPickedArray mPickedArray;
};

inline
bool cSceneManager::IsInited() const
{
	return mInited;
}


//inline
//NiFogProperty* cSceneManager::GetFogProperty() const
//{
//	return mFogProp;
//}

inline
NiAmbientLight* cSceneManager::GetAmbientLight() const
{
	return mAmbientLightArray.GetAt( 0 );//mCurrentLightIndex );
}

inline
NiDirectionalLight* cSceneManager::GetDirLight() const
{
	return mDirLightArray.GetAt( 0 );//mCurrentLightIndex);
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
cSceneManager* cSceneManager::GetSingleton()
{
	return mSingleton;
}

#define SCENEMAN cSceneManager::GetSingleton()
*/