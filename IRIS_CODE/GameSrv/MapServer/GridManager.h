/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

#include "BaseObject_Common.h"

enum eGRID_MATRIX_TYPE
{
	_E_GMT_NONE_ = 0,							// 사용안함
	_E_GMT_3X3_,								// 3x3 행렬
	_E_GMT_5X5_,								// 5x5 행렬
	_E_GMT_7X7_,								// 7x7 행렬
};

#define DEF_GRID_POS        0					// default grid pos

// 16 x 16 Grid = 시야 넓이
#define MAP_SIZE            1024				// 1024 = 32 * 32
#define MAP_SIZE_SHIFT      10                  // 1024 = 1 << 10

#define TRACK_SIZE          (+32)               // TRACK_SIZE = MAP_SIZE / 16
#define TRACK_SIZE_SHIFT    5                   // 32 = 1 << 5
#define TRACK_POS_SHIFT     4                   // 1/16 = 1 >> 4

// 32 x 32 Grid = 시야 넓이
//#define MAP_SIZE            256                 // 256 = 16 * 16
//#define MAP_SIZE_SHIFT      8                   // 512 = 1 << 9
//
//#define TRACK_SIZE          (+16)               // TRACK_SIZE = MAP_SIZE / 32
//#define TRACK_SIZE_SHIFT    4                   // 16 = 1 << 4
//#define TRACK_YPOS_SHIFT    5                   // 1/32 = 1 >> 5

#define DELAY_CHANGE_GRID   100                 // 1000 / DELAY_CHANGE_GRID
#define INDUN_MAP_MAX		4096				/// 인던 맵(방) 최대 설정수

class cGridManager;
class cBaseObject;								// defined baseobject.h file
class cPlayer;									// "       player.h     "
class cMonster;									// "       monster.h    "
class cNpc;										// "       npc.h        "
class cItem;									// "       item.h       "
class cTotem;									// "       totem.h      "
class cGathering;								// "       gathering.h  "
class cUserPortal;								// "       UserPortal.h "
class cBaseSkillObject;


struct GridMinMax
{
	int minTrack;
	int maxTrack;
};

struct GridMatrix
{
	int status;
	int pos;
};

struct PerGrid
{
	long            objectId;           // sObject::Index
	cBaseObject*    baseObject;         // cBaseObject Class
	int             lastPos;            // Last Position
	int             currentPos;         // Current Position
	DWORD           lastTickCount;      // Last Tick Count

	GridMatrix*     matrix;

	struct PerGrid* next;
	struct PerGrid* prev;
	struct PerGrid* parent;
	struct PerGrid* left;
	struct PerGrid* right;
};

class GridPool
{
friend class cGridManager;

private:
	PerGrid**    mPagedPoolUsage;           // 페이지 된 풀.
	PerGrid*     mNonPagedPoolUsage;        // 페이지 안된 풀.
	PerGrid*     mBstRoot;                  // BST 루트.

	int          mMinGrid;
	int          mMaxGrid;
	int          mMatrixSize;
	GridMinMax   mMinMax;                   // 그리드 최소 & 최대 사용영역.

	int          mSkipMax;
	int*         mSkipMap;

	SIZE_T       mQuotaPagedPoolUsage;      // 현재 페이지 된 풀   - 사용량.
	SIZE_T       mQuotaNonPagedPoolUsage;   // 현재 페이지 안된 풀 - 사용량.
	SIZE_T       mWorkingSetSize;           // 현재 작업 설정크기.

	int          CalcPos         ( float fx, float fy, unsigned int map );
	int          CalcPos         ( unsigned int map );

	PerGrid*     AllocGrid       ( );
	void         FreeGrid        ( PerGrid** perGrid );

	int          CompareObjectId ( PerGrid* perGrid1, PerGrid* perGrid2);
	int          CompareObjectId ( long objectId1, long objectId2 );

	void         AttachPool      ( PerGrid** pool, PerGrid* perGrid );
	void         DetachPool      ( PerGrid** pool, PerGrid* perGrid );

	bool         AttachBst       ( PerGrid* perGrid );
	bool         DetachBst       ( PerGrid* perGrid );

	PerGrid*     GetPool         ( long objectId, int pos );
	void         ReleasePool     ( PerGrid* perGrid, int pos, bool isDelete=false );

public:
	GridPool(int minGrid, int maxGrid, int matrixSize);

	void         Shutdown        ( );

	PerGrid*     SearchObjectId  ( long objectId );

	PerGrid*     GetGrid         ( cBaseObject* baseObject );
	void         ReleaseGrid     ( PerGrid* perGrid, bool isDelete=false );
	bool         ChangeGrid      ( PerGrid* perGrid, DWORD tickCount );

public:
	virtual ~GridPool(void);
};

inline int GridPool::CalcPos(float fx, float fy, unsigned int map)
{
	int ix  = (int)(fx * 0.01f) >> TRACK_POS_SHIFT;
	int iy  = (int)(fy * 0.01f) >> TRACK_POS_SHIFT;
	return (unsigned int)((iy<<TRACK_SIZE_SHIFT) + ix) + ((map-MAP_MIN)<<MAP_SIZE_SHIFT);
}
inline int GridPool::CalcPos(unsigned int map)
{
	return (unsigned int)((map-MAP_MIN)<<MAP_SIZE_SHIFT);
}


struct GridData
{
	PerGrid* seed;
	int      matrixCount;
	PerGrid* tree;
};


class cGridManager
{
private:
	int          mMatrixSize;	// 3 x 3 의 값
	int          mMinmaxSize;	// 3 x 3 의 행.
	int          mAbsTrack;		// 허용범위
	int          mMinTrack;		// (-TRACK_SIZE * mAbsTrack) - mAbsTrack
	int          mMaxTrack;		// (-TRACK_SIZE * mAbsTrack) + mAbsTrack

	int          mMinGrid;
	int          mMaxGrid;

	int*         mMatrix;
	int*         mPlayerMtr;

	GridPool*    mPlayerPool;
	GridPool*    mMonsterPool;
	GridPool*    mNpcPool;
	GridPool*    mItemPool;
	GridPool*    mTotemPool;
	GridPool*    mGatheringPool;
	GridPool*    mUserPortalPool;

	GridMinMax*  mOldMinMax;
	GridMinMax*  mNewMinMax;
	GridMatrix*  mOldMatrix;
	GridMatrix*  mNewMatrix;

	GridData     mPlayerData;
	GridData     mMonsterData;
	GridData     mNpcData;
	GridData     mItemData;
	GridData     mTotemData;
	GridData     mGatheringData;
	GridData     mUserPortalData;

private:
	void         Matrix              ( PerGrid* perGrid );

	void         SendPlayerGameIn    ( cPlayer* player, int gridPos, bool mtrApply=true );
	void         SendPlayerGameOut   ( cPlayer* player, int gridPos, bool mtrApply=true );

	void         SendSightInPlayer   ( cPlayer* player, int gridPos );
	void         SendSightOutPlayer  ( cPlayer* player, int gridPos );

	void         SendSightInMonster  ( cPlayer* player, int gridPos );
	void         SendSightOutMonster ( cPlayer* player, int gridPos );

	void         SendSightInNpc      ( cPlayer* player, int gridPos );
	void         SendSightOutNpc     ( cPlayer* player, int gridPos );

	void         SendSightInItem     ( cPlayer* player, int gridPos );
	void         SendSightOutItem    ( cPlayer* player, int gridPos );

	void         SendSightInTotem    ( cPlayer* player, int gridPos );
	void         SendSightOutTotem   ( cPlayer* player, int gridPos );

	void         SendMonsterSightIn  ( cMonster* monster, int gridPos );
	void         SendMonsterSightOut ( cMonster* monster, int gridPos );

	void         SendSightInGathering( cPlayer* player, int gridPos );
	void         SendSightOutGathering( cPlayer* player, int gridPos );

	void         SendSightInUserPortal( cPlayer* player, int gridPos );
	void         SendSightOutUserPortal( cPlayer* player, int gridPos );


	bool         SendNpcList         ( cPlayer* player );

public:
	cGridManager(void);

	bool         Init                ( eGRID_MATRIX_TYPE type, int serverType );
	void         Release             ( );
	void         Process             ( DWORD tickCount );

	void         FindPlayers         ( float x, float y, short map, float dst, void* stlAry, bool applyCheatHideMode=false );
	void         FindPlayers         ( cBaseObject* object, void* stlAry, bool applyCheatHideMode=false );
	void         FindSkillSendPlayer ( cBaseSkillObject* pSkill, bool applyCheatHideMode=false );
	cPlayer*     FindFirstPlayer     ( cBaseObject* object, bool applyCheatHideMode=false );
	cPlayer*     FindNextPlayer      ( bool applyCheatHideMode=false );

	void         FindMonsters        ( float x, float y, short map, float dst, void* stlAry );
	void         FindMonsters        ( cBaseObject* object, void* stlAry );
	cMonster*    FindFirstMonster    ( cBaseObject* object );
	cMonster*    FindNextMonster     ( );

	cNpc*        FindFirstNpc        ( cBaseObject* object );
	cNpc*        FindNextNpc         ( );

	cItem*       FindFirstItem       ( cBaseObject* object );
	cItem*       FindNextItem        ( );

	cTotem*      FindFirstTotem      ( cBaseObject* object );
	cTotem*      FindNextTotem       ( );

	cGathering*  FindFirstGathering  ( cBaseObject* object );
	cGathering*  FindNextGathering   ( );

	bool         IsSight             ( cBaseObject* object, long objectId );
	bool         IsSight             ( cBaseObject* object1, cBaseObject* object2 );
	int          IsPlayer            ( cBaseObject* object );
	cPlayer*     GetPlayer           ( long objectId );
	bool         AddPlayer           ( cPlayer* player );
	bool         RemovePlayer        ( cPlayer* player );

	void         SetHidePlayer       ( cPlayer* player, bool apply );

	cMonster*    GetMonster          ( long objectId );
	bool         AddMonster          ( cMonster* monster );
	bool         RemoveMonster       ( cMonster* monster );

	cNpc*        GetNpc              ( long objectId );
	bool         AddNpc              ( cNpc* npc );
	bool         RemoveNpc           ( cNpc* npc );

	cItem*       GetItem             ( long objectId );
	bool         AddItem             ( cItem* item );
	bool         RemoveItem          ( cItem* item );

	cTotem*      GetTotem            ( long objectId );
	bool         AddTotem            ( cTotem* totem );
	bool         RemoveTotem         ( cTotem* totem );

	cGathering*  GetGathering        ( long objectId );
	bool         AddGathering        ( cGathering* gathering );
	bool         RemoveGathering     ( cGathering* gathering );

	cUserPortal* GetUserPortal       ( long objectId );
	bool         AddUserPortal       ( cUserPortal* pUserPortal );
	bool         RemoveUserPortal    ( cUserPortal* pUserPortal );

	long         MakeCID             ( cBaseObject* object, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr );
	long         MakeCIDExcept       ( cBaseObject* object, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr );
	long         MakeCIDMap          ( cBaseObject* object, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr );
	long         MakeCIDMap          ( unsigned short mapNumber, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr );
	long         MakeCIDExceptMap    ( cBaseObject* object1, cBaseObject* object2, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr );

	long         MakePIDMap          ( unsigned short mapNumber, u_long pidSize, char* buffer, u_long bufferLength, u_long& indPtr );

public:
	virtual ~cGridManager(void);

public:
	static cGridManager* mpGridManager;
};

#define GRIDMANAGER	cGridManager::mpGridManager
