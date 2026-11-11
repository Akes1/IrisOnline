/* ==========================================================================
* 파    일 : StageScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-10-22
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Stage_Common.h"
#include "RangeCheck.h"

class cPlayer;

#ifndef _CLIENT
struct sMapInfo
{
	unsigned long mapNum;
	unsigned long folderIdx;

	unsigned char mapMode;

	unsigned char minLevel;
	unsigned char maxLevel;
	unsigned char maxPlayer;
	unsigned long itemIdx;
};
#endif

class cStageScript
{
public:
	static cStageScript*	 mpStageScript;

public:

	/// 생성 소멸
	cStageScript();
	virtual ~cStageScript();

	///	초기화
	bool					Init();
	/// 전체 사용 메모리 제거
	void					Release();

	/// 맵이동 포인트 로드
	bool					StageChangePosScriptLoad();
	/// 좌표를 입력해 맵이동 포인트가 스크립트에 있는지 체크해 index를 리턴. (0:없음)
	unsigned long			GetStageChangeIndex( float posX, float posY );
	/// 좌표이동 고유 인덱스를 입력해 목적지를 리턴한다.
	sTargetPos				GetStageChangePos( float posX, float posY, unsigned short mapNum, float objectSize, unsigned long posIdx );

	/// 좌표이동 고유 인덱스를 입력해 목적지를 리턴한다.
	sTargetPos				GetStageChangePos( unsigned long posIdx );

	/// 가장 가까운 맵이동 목적지좌표를 찾아냄
	NiPoint2				CalcNearTargetMapPos( unsigned short mapNum, float posX, float posY );
	bool					CalcTargetMapPos( unsigned long posIdx, float *pX, float *pY );

	tPointerArray<void*>*	GetMapChangeStarterArr( unsigned short mapNum );
	tPointerArray<void*>*	GetMapChangeDestArr( unsigned short mapNum );

	sStageChangePos*	GetPosScriptInfo( unsigned long idx );


	bool LoadManagedDoorInfo();

	tPointerArray<void*>*	GetManagedDoorArr( unsigned short mapNum );
	sDoorInfo*				GetManagedDoorInfo( unsigned long index );

	sDoorInfo* CrashDoor( unsigned long doorIdx, NiPoint2 start, NiPoint2 goal );

	bool LineIntersectRect( NiPoint2* rect, NiPoint2 start, NiPoint2 goal );

	bool LineIntersectLine( NiPoint2 start, NiPoint2 goal, NiPoint2 lineStart, NiPoint2 lineEnd );

#ifndef _CLIENT
	bool LoadMapInfo();
	sMapInfo* GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode );
	void GetModeSet( unsigned short mapType, void* pModeSet );

protected:
	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;

	/// <unsigned long modeNum, sMapInfo* mapData>
	/// <unsigned long mapNum, cPointerHashMap* modeMapData>
	cPointerHashMap			mMapInfoMap;
#endif

//protected:
public:
	unsigned int InsidePolygon( NiPoint2 *polygon, unsigned int numVerts, NiPoint2 p );

private:

	/// 
	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;
	typedef tHashSet<unsigned long> cSet;

	cPointerHashMap			mStageChangePosMap;

	/// 특정맵의 모든 이동포인트를 넘겨주기 위한 변수
	tPointerArray<void*>	mReturnArray;

//	cRangeCheck				mRangeCheck;

	///
	cPointerHashMap			mStageDoorMap;
};


#define STAGESCRIPT cStageScript::mpStageScript
