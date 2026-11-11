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
class cUserPortalSceneNode;
class cUserPortalSceneNodeParam;
class cCardSceneNode;
class cCardSceneNodeParam;
class cGatheringSceneNode;
class cGatheringSceneNodeParam;
class cDoorSceneNode;
class cDoorSceneNodeParam;

class cDepthPlane;
class cPlane;
class cShadowGeometry;
class cRibbonGeometry;
class cBaseObject;

struct sObject;
struct sDoorInfo;

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

	/// 로딩
//	bool Load( const cString& pathName );
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
	bool LoadAreaGroup( unsigned int mapIndex, unsigned char mapMode );

	/// load process용 함수
	bool LoadSceneHeader( cFileLoader& loader, unsigned int& outNum );
	bool LoadSoundSceneHeader( cFileLoader& loader, unsigned int& outNum );

	/// return error event( -1:오류, 0:로딩완료, 1~:진행수)
	int LoadSceneNode( cFileLoader& loader, unsigned int loadCount=1 );
	int LoadSoundSceneNode( cFileLoader& loader, unsigned int loadCount=1 );

	int LoadSceneNode6( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode7( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode8( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode9( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode10( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode11( cFileLoader& loader, unsigned int loadCount );
	int LoadSceneNode12( cFileLoader& loader, unsigned int loadCount );

	int LoadSoundNode11( cFileLoader& loader, unsigned int loadCount );

	/// 튜토리얼에서만 로드해야 하는 데이터들
	bool LoadTutorialScene();

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 렌더링
	void Render();
	void RenderZFalseDamage();
	void RenderShadow();
	void RenderTrail();

	/// 2D 객체 랜더링 (말풍선 etc.)
	void Render2D();
	void PreRender2D();

	/// 가시 레벨 거리
	void SetLevelDistance( float dist0, float dist1, float dist2 );

	/// 화면 상의 좌표에 대응하는 장면 노드들을 리턴
	/// 삼각형 단위로 검사한다.
	bool PickStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );
	bool PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );
	bool PickManagedStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance = false );

	/// 반직선과 충돌하는 장면 노드들을 수집
	/// 삼각형 단위로 검사한다.
	bool PickStatics( tArray<void*>* pickedArray, const cRay& ray, bool camRay, float maxDistance, bool sortByDistance = false );
	bool PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );
	bool PickManagedStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );
	bool PickMapportal( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance = false );

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool Pick_Kind( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type );
	bool Pick_Type( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type );
	bool PickAreas( tArray<void*>* pickedArray, const cSphere& sphere );
	bool PickLights( tArray<void*>* pickedArray, const cSphere& sphere );
	bool PickSounds( tArray<void*>* pickedArray, const cSphere& sphere );

	cBaseObject* PickPlane( int mouseX, int mouseY );

	/// 영역 장면 노드를 생성
	cAreaSceneNode* CreateArea( cAreaSceneNodeParam& param );

	/// 조명 장면 노드를 생성
	cLightSceneNode* CreateLight( cLightSceneNodeParam& param );

	/// 사운드 장면 노드를 생성
	cSoundSceneNode* CreateEnvSound( cSoundSceneNodeParam& param );
	cSoundSceneNode* CreateEffectSound( cSoundSceneNodeParam& param );

	void UpdateSoundGain();

	void ActiveSoundNodeAll( bool active );

	/// 정적 장면 노드를 생성
	cStaticSceneNode* CreateStatic( cStaticSceneNodeParam& param );
	cDropItemSceneNode* CreateDropItem( cDropItemSceneNodeParam& param );
	cGatheringSceneNode* CreateGathering( cGatheringSceneNodeParam& param );
	cMapPortalSceneNode* CreateMapPortal(  unsigned long scriptIdx, NiPoint3 pos, NiMatrix3 rot );
	cUserPortalSceneNode* CreateUserPortal( unsigned long portalIdx, unsigned long ownerIdx, unsigned long scriptIdx, NiPoint3 pos );
	cCardSceneNode* CreateCard( cCardSceneNodeParam& param );

	/// 관리 객체 생성용 외부 인터페이스
	cPlayerSceneNode* CreatePlayer( cPlayerSceneNodeParam& param );
	cMonsterSceneNode* CreateMonster( cMonsterSceneNodeParam& param );
	cTransMonSceneNode* CreateTransMonster( cMonsterSceneNodeParam& param, bool IsHero );
	cNpcSceneNode* CreateNPC( cNpcSceneNodeParam& param );
	cEffectSceneNode* CreateEffect( cEffectSceneNodeParam& param );
	cEffectSceneNode* CreateSelfEffect( cString pathName, bool distCheck, NiTransform& trans, bool bLoop );
	cEffectSceneNode* CreateDamageEffect( cEffectSceneNodeParam& param, bool IsMiss, bool zFalse );
	cHeroSceneNode* CreateHero( cPlayerSceneNodeParam& param );
	cVehicleSceneNode* CreateVehicle( cVehicleSceneNodeParam& param );
	cRefereeSceneNode* CreateReferee( cSceneNodeParam& param );
	cDoorSceneNode* CreateDoor(  unsigned long index );//cDoorSceneNodeParam& param );

	/// 관리 객체의 제거용 외부 인터페이스
	bool DestroyDynamic( cSceneNode* node );
	bool DestroyDropItemNode( cSceneNode* node );
	bool DestroyMapPortalNode( cSceneNode* node );
	bool DestroyGathringNode( cSceneNode* node );
	bool DestroyStaticNode( cSceneNode* node );
	bool DestroyDoorNode( cSceneNode* node );

	void RemoveAllDoorNode();

	/// Deletelist에서 제거하지 않고 외부에서 임의로 호출하는 경우는 DelListRemove = ture로 설정하시오
	bool DestroyEffectNode( cSceneNode* node );
	bool DestroySoundNode( cSceneNode* node );

	/// 제거 되어야 할 이펙트 노드에 대한 처리
	void AddDeleteEffectNode( cEffectSceneNode* node );
	void AddDeleteGatheringNode( cGatheringSceneNode* node );
	void AddDeleteSoundNode( cSoundSceneNode* node );
	void AddDeleteDoorNode( cDoorSceneNode* node );

	/// 모든 노드들을 제거
	void DestroyAll();

	/// 전체 영역 노드
	cAreaSceneNode* GetGlobalArea();

	void AddVisibleShadowGeom( cShadowGeometry* geom );
	void AddVisibleTrailGeom( cRibbonGeometry* geom );

	void AddPlane( cPlane* pPlane );

	void AddVisibleDynamicNode( cDynamicSceneNode* node );
	///

	inline void SetScreenShotMode( bool enable ) { mScreenShotMode = enable; }
	inline bool IsScreenShotMode() { return mScreenShotMode; }

	inline const tArray<unsigned long>* GetVisibleDynamicSceneArray() { return &mVisibleDynamicNodeArray; }

	unsigned int CalcVisibleLevel( cStaticSceneNode* n );
	bool CalcDynamicVisibleLevel( cSceneNode* n );


	cEffectSceneNode* GetEffectSceneNode( unsigned long effectIdx );
	cSoundSceneNode* GetSoundSceneNode( unsigned long soundIdx );
	cDynamicSceneNode* GetDynamicSceneNode( unsigned long dynIdx );
	cDropItemSceneNode* GetDropItemSceneNode( unsigned long nodeIdx );
	cGatheringSceneNode* GetGatheringSceneNode( unsigned long nodeIdx );
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

	void OnVisibleModifierPocess( cSceneNode* node );

	void ShowDepthPlane( bool show ) { mShowDepthPlane = show; }

	sDoorInfo* DisableDoorMove( NiPoint2 startPos, NiPoint2 goalPos );

protected:
	/// 노드를 추가
	bool AddAreaNode( cSceneNode* node );
	bool AddLightNode( cSceneNode* node );
	bool AddEffectSoundNode( cSceneNode* node );
	bool AddEnvSoundNode( cSceneNode* node );
	bool AddStaticNode( cSceneNode* node );
	bool AddDropItemNode( cSceneNode* node );
	bool AddGatheringNode( cSceneNode* node );
	bool AddMapPotalNode( cSceneNode* node );
	bool AddDynamicNode( cSceneNode* node );
	bool AddEffectNode( cEffectSceneNode* node );
	bool AddDoorNode( unsigned long index, cSceneNode* node );

	void DelNodeProcess();

public:
	/// 단일체를 리턴
	static cSceneManager* GetSingleton();

protected:

	bool mEnableLoading;

	/// 버전
	unsigned int mVersion;

	cIndexGenerator<unsigned long> mAreaNodeIndexGen;
	cIndexGenerator<unsigned long> mLightNodeIndexGen;
	cIndexGenerator<unsigned long> mSoundIndexGen;
	cIndexGenerator<unsigned long> mStaticNodeIndexGen;
	cIndexGenerator<unsigned long> mDropItemNodeIndexGen;
	cIndexGenerator<unsigned long> mGatheringNodeIndexGen;
	cIndexGenerator<unsigned long> mMapPortalNodeIndexGen;
	cIndexGenerator<unsigned long> mDynamicNodeIndexGen;
	cIndexGenerator<unsigned long> mEffectIndexGen;

	typedef tPointerHashMap<unsigned long, void*> cAreaMap;
	typedef tPointerHashMap<unsigned long, void*> cLightMap;
	typedef tPointerHashMap<unsigned long, void*> cSoundMap;
	typedef tPointerHashMap<unsigned long, void*> cNodeMap;

	/// 트리
	/// 공간을 분할하여 노드들을 계층적으로 관리한다.
	/// 충돌 검사 및 컬링을 효율적으로 수행하기위해 사용한다.
	cSceneTree mNodeTree;

	cAreaMap mAreaArray;
	cLightMap mLightArray;
	cSoundMap mSoundArray;

	/// 캐릭터 (PC, NPC, Monster)
	cNodeMap mDynamicObjectArray;

	/// 아이템
	cNodeMap mDropItemArray;

	/// 채집 객체
	cNodeMap mGatheringArray;

	/// 맵이동 Potal객체
	cNodeMap mMapPortalArray;

	/// 관리 게이트 객체
	cNodeMap mDoorArray;

	/// 환경 객체 노드 배열
	cNodeMap mStaticObjectArray;

	/// 이펙트 (트리로 관리할 필요 없음)
	cNodeMap mEffectNodeArray;

	/// 노드 스스로 삭제용
	typedef tPointerList<void*> cDelNodeList;
	cDelNodeList mDelEffectList;
	cDelNodeList mDelGatheringList;
	cDelNodeList mDelSoundList;
	cDelNodeList mDelDoorList;

	/// 2D 평면 객체 관리
	typedef tPointerArray<void*> cPlaneArray;
	cPlaneArray	mPlaneArray;

	/// 게임 BaseObj에 대응한 SceneNode를 거리별 Sorting하기위해 사용한다.
	tArray<unsigned long> mVisibleDynamicNodeArray;

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

	/// 전체 영역 노드
	cAreaSceneNode* mGlobalArea;

	/// 영역 그룹
	typedef tHashMap<unsigned int, void*> cAreaGroupMap;
	cAreaGroupMap mAreaGroupMap;

	/// 텍스처 맵
	typedef tHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mTextureMap;

	/// 그림자 가시 배열
	typedef tPointerArray<cShadowGeometry*> cShadowGeomArray;
	cShadowGeomArray mShadowGeomArray;

	/// 
	typedef tPointerArray<cRibbonGeometry*> cTrailGeomArray;
	cTrailGeomArray mTrailGeomArray;

	///
	bool mScreenShotMode;
	bool mShowDepthPlane;

	NiPoint3 mHeroPos;
};

inline
cAreaSceneNode* cSceneManager::GetGlobalArea()
{
	return mGlobalArea;
}

inline
void cSceneManager::SetLevelDistance( float dist0, float dist1, float dist2 )
{
	mSqaredLevelDistance0 = dist0 * dist0;
	mSqaredLevelDistance1 = dist1 * dist1;
	mSqaredLevelDistance2 = dist2 * dist2;
}

inline
void cSceneManager::AddVisibleShadowGeom( cShadowGeometry* geom )
{
	assert( geom );

	mShadowGeomArray.PushBack( geom );
}

inline 
void cSceneManager::AddVisibleTrailGeom( cRibbonGeometry* geom )
{
	assert( geom );

	mTrailGeomArray.PushBack( geom );
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
cDropItemSceneNode* cSceneManager::GetDropItemSceneNode( unsigned long nodeIdx ) 
{ 
	return (cDropItemSceneNode*)mDropItemArray.GetAt( nodeIdx ); 
}

inline 
cGatheringSceneNode* cSceneManager::GetGatheringSceneNode( unsigned long nodeIdx ) 
{ 
	return (cGatheringSceneNode*)mGatheringArray.GetAt( nodeIdx ); 
}

inline 
cStaticSceneNode* cSceneManager::GetStaticSceneNode( unsigned long nodeIdx ) 
{ 
	return (cStaticSceneNode*) mStaticObjectArray.GetAt( nodeIdx ); 
}


inline
cSceneManager* cSceneManager::GetSingleton()
{
	return mSingleton;
}

#define SCENEMAN cSceneManager::GetSingleton()
