/* ====================================================================
 * 파    일 :	WorldManager.h
 * 목    적 :	환경에 관련된 모든 객체및 정보를 관리한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.01
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.01     이법석	    생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

class cPathFinder;
class cTerrain;
class cNaviMesh;
class cNaviField;

class cMaterialData;
class cSceneNode;
class cAreaSceneNode;

/// 월드 관리자
class cWorldManager
{
	static cWorldManager* mSingleton;

public:
	cWorldManager();
	~cWorldManager();

	/// 초기화 및 종료
	bool Init();
	void Exit();

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );
	void ProcessAmbient( unsigned long deltaTime );
	void ProcessFog( unsigned long deltaTime );
	void ProcessCamera( unsigned long deltaTime );

	/// 렌더링
	void Render();
	void RenderSkyDome();

	/// 현재 월드 정보를 생성 및 파괴
	bool Open();
	void Close();
	void ClearSkyDome();

	/// 하늘
	bool LoadSkydome( const cString& pathName, float scale );
	void MoveSkyDome( const NiPoint3& pos );

	/// 영역
	void SetArea( const cAreaSceneNode& n );
	void BlendArea( const cAreaSceneNode& n0, const cAreaSceneNode& n1, float ratio );

	/// 픽킹
	bool Pick( NiPoint3* pos, int mouseX, int mouseY );
	
	/// 높이값
	bool CalcHeight( float* height, float posX, float posY );

	bool EnableMove( int mouseX, int mouseY );

	bool EnableRayCheck( NiPoint2 start, NiPoint2 goal );
	bool EnableRayGet( NiPoint3 start, NiPoint3 &goal );
	bool EnableRayGet(  NiPoint2 start, NiPoint2 &goal );

	/// Map Index정보
	inline unsigned int			GetCurrentMapIndex()						{ return mCurrentMapIndex; }
	inline void					SetCurrentMapIndex( unsigned int mapIdx )	{ mCurrentMapIndex = mapIdx; }

	/// 지형을 리턴
	cTerrain* GetTerrain() const;
	cNaviField* GetNaviField() const;
	cNaviMesh* GetNaviMesh() const;
    cPathFinder* GetPathFinder() const;

	cNaviField* GetNaviField( int mapIndex, unsigned char mapMode  ) const;

	/// load process를 위한 함수
	bool LoadTerrainHeader( unsigned int& loadCount, unsigned int mapIdx, unsigned char mapMode );
	int LoadTerrainNodes( unsigned int count );
	bool LoadNaviMeshHeader( unsigned int& loadCount, unsigned int mapIdx, unsigned char mapMode );
	int LoadNaviMeshNodes( unsigned int count );
	bool LoadNaviField( unsigned int mapIdx, unsigned char mapMode );

protected:
	/// 기하 노드들을 수집
	void CollectGeometries( tList<void*>* pgeomList, NiAVObject* pobj );
	void CollectProperties( tList<void*>* ppropList, NiAVObject* pobj );
	void SetMaterial( tList<void*>& propList, const cMaterialData& material );

	/// 객체를 변형한다.
	void DisableZBuffer( NiAVObject* pObj );
//	void RecursivePrepack( NiAVObject* pObj );

public:
	/// 단일체를 리턴
	static cWorldManager* GetSingleton();

private:
	/// 맵 인덱스
	unsigned int mCurrentMapIndex;

	/// 길찾기
	cPathFinder* mPathFinder;

	/// 지형
	cTerrain* mTerrain;

	/// 네비메시
	cNaviMesh* mNaviMesh;

	/// 네비필드
	typedef tHashMap< unsigned long, void* > cNaviFieldMap;
	cNaviFieldMap mNaviFieldMap;

	cNaviField* mpCurrentNaviField;
	bool mNaviFieldLoadEnd;

	/// 하늘
	NiNodePtr mSkyDome;
	cString mSkyPathName;

	typedef tList<void*> cRenderObjList;
	cRenderObjList mSkyGeomList;

	typedef tList<void*> cMatPropList;
	cMatPropList mSkyMatPropList;

	float mSkyHeight;
	bool mSkyNeedUpdate;

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
	typedef tArray<void*> cCollidableArray;
	cCollidableArray mCollidableArray;

	cAreaSceneNode* mArea1;
	cAreaSceneNode* mArea2;
};

inline
cTerrain* cWorldManager::GetTerrain() const
{
	return mTerrain;
}

inline
cNaviMesh* cWorldManager::GetNaviMesh() const
{
	return mNaviMesh;
}

inline
cNaviField* cWorldManager::GetNaviField() const
{
	return mpCurrentNaviField;
}

inline
cPathFinder* cWorldManager::GetPathFinder() const
{
	return mPathFinder;
}

inline
cWorldManager* cWorldManager::GetSingleton()
{
	return mSingleton;
}

#define WORLDMAN cWorldManager::GetSingleton()
