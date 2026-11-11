#include "gamesrv.h"

#include "stdafx.h"

#include "GridManager.h"

#include "BaseObject.h"
#include "StatusCalc_Server.h"

#include "Player.h"
#include "Monster.h"
#include "Npc.h"
#include "Item.h"
#include "totem.h"
#include "Gathering.h"
#include "Party.h"
#include "GuildManager.h"
#include "PartyManager.h"
#include "PartyUnionManager.h"
#include "UserPortal.h"
#include "SkillTargetPool.h"
#include "BaseSkillObject.h"
#include "ObjectManager.h"

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }

// Global data
cGridManager* cGridManager::mpGridManager = NULL;


// GridPool Constructor
GridPool::GridPool(int minGrid, int maxGrid, int matrixSize) : mMinGrid(minGrid), mMaxGrid(maxGrid), mMatrixSize(matrixSize)
{
	// Pool Usage Pointer
	mPagedPoolUsage    = (PerGrid**)GlobalAlloc( GPTR, sizeof(PerGrid*)*mMaxGrid );
	mNonPagedPoolUsage = NULL;

	// BST Root Pointer
	mBstRoot = NULL;

	// Grid Min & Max
	mMinMax.minTrack = mMaxGrid;
	mMinMax.maxTrack = mMinGrid;

	mSkipMax = mMaxGrid / MAP_SIZE;
	mSkipMap = (int*)GlobalAlloc( GPTR, sizeof(int)*mSkipMax );

	// 메모리 카운터.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
}

// ~GridPool Destructor.
GridPool::~GridPool(void)
{
	// GridPool를 해제.
	Shutdown();

	GlobalFree( mSkipMap );
	GlobalFree( mPagedPoolUsage );
	mPagedPoolUsage = NULL;
}

// Shutdown Method
void GridPool::Shutdown()
{
	PerGrid* temp;

	// BST Shutdown
	while ( mBstRoot != NULL )
	{
		DetachBst( mBstRoot );
	}
	// Pool Usage Shutdown - PagedPoolUsage
	for ( int i = mMinGrid; i < mMaxGrid; i++ )
	{
		while ( mPagedPoolUsage[ i ] != NULL )
		{
			temp                 = mPagedPoolUsage[ i ];
			mPagedPoolUsage[ i ] = mPagedPoolUsage[ i ]->next;
			FreeGrid( &temp );
		}
	}
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreeGrid( &temp );
	}
}

// AllocGrid Method
PerGrid* GridPool::AllocGrid()
{
	PerGrid* perGrid = (PerGrid*)GlobalAlloc( GPTR, sizeof(PerGrid) );

	if ( perGrid != NULL )
	{
		mWorkingSetSize++;				// mWorkingSetSize를 증가.

		perGrid->objectId      = 0;     // sObject::Index (include cBaseObject class )
		perGrid->baseObject    = NULL;  // cBaseObject Class Pointer
		perGrid->lastPos       = 0;     // Last Position
		perGrid->currentPos    = 0;     // Current Position
		perGrid->lastTickCount = 0;     // Last Change Tick Count

		perGrid->matrix = (GridMatrix*)GlobalAlloc( GPTR, sizeof(GridMatrix)*mMatrixSize );

		perGrid->prev   = NULL;			// 선형리스트의 포인터 - 이전
		perGrid->next   = NULL;			// 선형리스트의 포인터 - 다음
		perGrid->parent = NULL;			// BST 포인터 - 상위
		perGrid->left   = NULL;			// BST 포인터 - 왼쪽
		perGrid->right  = NULL;			// BST 포인터 - 오른쪽
	}

	return perGrid;
}

// FreeGrid Method - I/O Context를 삭제한다.
void GridPool::FreeGrid(PerGrid** perGrid)
{
	if ( (*perGrid) != NULL )
	{
		if ( (*perGrid)->matrix != NULL )
		{
			GlobalFree( (*perGrid)->matrix );
			(*perGrid)->matrix = NULL;
		}

		GlobalFree( (*perGrid) );
		(*perGrid) = NULL;

		mWorkingSetSize--;				// mWorkingSetSize를 감소.
	}
}

// CompareObjectId Method
int GridPool::CompareObjectId(PerGrid* perGrid1, PerGrid* perGrid2)
{
	return (perGrid1->objectId - perGrid2->objectId);
}
int GridPool::CompareObjectId(long objectId1, long objectId2)
{
	return (objectId1 - objectId2);
}

// AttachPool Method
void GridPool::AttachPool(PerGrid** pool, PerGrid* perGrid)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = perGrid;

		perGrid->prev = NULL;
		perGrid->next = (*pool);
	}
	(*pool) = perGrid;
}

// DetachPool Method
void GridPool::DetachPool(PerGrid** pool, PerGrid* perGrid)
{
	PerGrid* prev = perGrid->prev;
	PerGrid* next = perGrid->next;

	if ( prev == NULL && next == NULL )
	{
		(*pool) = NULL;
	}
	else if ( prev == NULL && next != NULL )
	{
		next->prev = NULL;
		(*pool) = next;
	}
	else if ( prev != NULL && next == NULL )
	{
		prev->next = NULL;
	}
	else if ( prev != NULL && next != NULL )
	{
		prev->next = next;
		next->prev = prev;
	}

	perGrid->prev = NULL;
	perGrid->next = NULL;
}

// AttachBst Method
bool GridPool::AttachBst(PerGrid* perGrid)
{
	PerGrid* parent = NULL;
	PerGrid* child  = mBstRoot;
	int      result;

	while ( child != NULL )
	{
		parent = child;
		result = CompareObjectId( child->objectId, perGrid->objectId );

		if ( result == 0 ) return false;
		else if ( result > 0 ) child = child->left;
		else if ( result < 0 ) child = child->right;
	}

	if ( parent == NULL )
	{
		mBstRoot = perGrid;
	}
	else if ( CompareObjectId( parent, perGrid ) > 0 )
	{
		SetLeft( parent, perGrid );
	}
	else // if ( CompareObjectId( parent, perGrid ) < 0 )
	{
		SetRight( parent, perGrid );
	}
	return true;
}

// DetachBst Method
bool GridPool::DetachBst(PerGrid* perGrid)
{
	PerGrid* parent = perGrid->parent;
	PerGrid* left   = perGrid->left;
	PerGrid* right  = perGrid->right;
	PerGrid* child  = NULL;

	if ( left == NULL )
	{
		child = right;
	}
	else if ( right == NULL )
	{
		child = left;
	}
	else
	{
		child = right;
		while ( child->left != NULL ) child = child->left;

		if ( child->parent != perGrid )
		{
			SetLeft( child->parent, child->right );
			SetRight( child, right );
		}
		child->parent = parent;
		SetLeft( child, left );
	}

	if ( mBstRoot == perGrid )
	{
		mBstRoot = child;
		if ( mBstRoot != NULL ) mBstRoot->parent = NULL;
	}
	else if ( perGrid == parent->left )
	{
		SetLeft( parent, child );
	}
	else
	{
		SetRight( parent, child );
	}

	perGrid->parent = NULL;
	perGrid->left   = NULL;
	perGrid->right  = NULL;

	return true;
}

// GetPool Method
PerGrid* GridPool::GetPool(long objectId, int pos)
{
	PerGrid* perGrid = mNonPagedPoolUsage;

	if ( perGrid != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, perGrid );
		mQuotaNonPagedPoolUsage--;
	}
	else
	{
		// 페이지가 없을 경우 새로 생성한다.
		perGrid = AllocGrid();
	}

	if ( perGrid != NULL )
	{
		perGrid->objectId = objectId;

		// BST - 연결시도.
		if ( AttachBst( perGrid ) == false )
		{
			// BST - 연결실패 / 페이지 안된 풀 사용량을 증감.
			AttachPool( &mNonPagedPoolUsage, perGrid );
			mQuotaNonPagedPoolUsage++;
			return NULL;
		}

		// 페이지 된 풀 사용량을 증감.
		AttachPool( &mPagedPoolUsage[ pos ], perGrid );
		mQuotaPagedPoolUsage++;
	}

	return perGrid;
}

// ReleasePool Method
void GridPool::ReleasePool(PerGrid* perGrid, int g, bool isDelete)
{
	// BST - 연결종료.
	DetachBst( perGrid );

	// 페이지 된 풀 사용량을 차감.
	DetachPool( &mPagedPoolUsage[ g ], perGrid );
	mQuotaPagedPoolUsage--;

	if ( isDelete != true )
	{
		// 페이지 안된 풀 사용량을 증감.
		AttachPool( &mNonPagedPoolUsage, perGrid );
		mQuotaNonPagedPoolUsage++;
	}
	else
	{
		// 페이지 삭제.
		FreeGrid( &perGrid );
	}
}

// SearchObjectId Method
PerGrid* GridPool::SearchObjectId(long objectId)
{
	PerGrid* perGrid = mBstRoot;
	int      result;
	while ( perGrid != NULL )
	{
		result = CompareObjectId( perGrid->objectId, objectId );
		if ( result == 0 )
			return perGrid;
		else if ( result > 0 )
			perGrid = perGrid->left;
		else if ( result < 0 )
			perGrid = perGrid->right;
	}
	return NULL;
}

// GetGrid Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseGrid와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerGrid* GridPool::GetGrid(cBaseObject* baseObject)
{
	int      pos     = CalcPos( baseObject->GetXPos(), baseObject->GetYPos(), baseObject->GetMapNumber() );
	int      skip    = pos / MAP_SIZE;
	PerGrid* perGrid = NULL;

	if ( pos < mMaxGrid )
	{
		perGrid = GetPool( baseObject->GetObjectID(), pos );

		if ( perGrid != NULL )
		{
			perGrid->baseObject = baseObject;
			perGrid->lastPos    = pos;
			perGrid->currentPos = pos;

			/*-- MinMax Track 재 계산 - 새로운 입력시 마다.
			*/
			mMinMax.minTrack = min( mMinMax.minTrack, perGrid->currentPos );
			mMinMax.maxTrack = max( mMinMax.maxTrack, perGrid->currentPos );

			mSkipMap[ skip ] += 1;

			baseObject->SetGridPos( pos );
		}
	}

	return perGrid;
}

// ReleaseGrid Method - 선형리스트의 HEAD로 회수한다.
// GetGrid와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void GridPool::ReleaseGrid(PerGrid* perGrid, bool isDelete)
{
	int pos  = perGrid->currentPos;
	int skip = pos / MAP_SIZE;

	perGrid->objectId      = 0;
	perGrid->baseObject    = NULL;
	perGrid->lastPos       = 0;
	perGrid->currentPos    = 0;
	perGrid->lastTickCount = 0;

	memset( perGrid->matrix, 0, sizeof(GridMatrix)*mMatrixSize );

	if ( mSkipMap[ skip ] > 0 ) mSkipMap[ skip ] -= 1;

	ReleasePool( perGrid, pos, isDelete );
}

// ChangeGrid Method
bool GridPool::ChangeGrid(PerGrid* perGrid, DWORD tickCount)
{
	if ( tickCount > perGrid->lastTickCount )
	{
		perGrid->lastTickCount = tickCount + DELAY_CHANGE_GRID;

		cBaseObject* baseObject = perGrid->baseObject;
		int          newPos     = CalcPos( baseObject->GetXPos(), baseObject->GetYPos(), baseObject->GetMapNumber() );

		if ( newPos < mMaxGrid )
		{
			int& lastPos    = perGrid->lastPos;
			int& currentPos = perGrid->currentPos;

			if ( newPos != currentPos )
			{
				DetachPool( &mPagedPoolUsage[ currentPos ], perGrid );
				AttachPool( &mPagedPoolUsage[ newPos     ], perGrid );

				lastPos    = currentPos;
				currentPos = newPos;

				mMinMax.minTrack = min( mMinMax.minTrack, currentPos );
				mMinMax.maxTrack = max( mMinMax.maxTrack, currentPos );

				mSkipMap[ lastPos    / MAP_SIZE ] -= 1;
				mSkipMap[ currentPos / MAP_SIZE ] += 1;

				baseObject->SetGridPos( currentPos );
				return true;
			}
		}
	}
	return false;
}

// cGridManager Constructor.
cGridManager::cGridManager(void)
{
	mpGridManager = this;

	mMatrixSize = 0;
	mMinmaxSize = 0;
	mAbsTrack   = 0;
	mMinTrack   = 0;
	mMaxTrack   = 0;

	mMinGrid = 0;
	mMaxGrid = 0;

	mMatrix    = NULL;
	mPlayerMtr = NULL;

	mPlayerPool  = NULL;
	mMonsterPool = NULL;
	mNpcPool     = NULL;
	mItemPool    = NULL;
	mTotemPool   = NULL;
	mGatheringPool = NULL;
	mUserPortalPool = NULL;

	mOldMinMax = NULL;
	mNewMinMax = NULL;
	mOldMatrix = NULL;
	mNewMatrix = NULL;
}

// ~cGridManager Destructor.
cGridManager::~cGridManager(void)
{
	mpGridManager = NULL;
}

// Init Method
bool cGridManager::Init(eGRID_MATRIX_TYPE type, int serverType)
{
	switch ( type )
	{
	case _E_GMT_3X3_:
		mMatrixSize = 9;				// 3 x 3 의 값(행렬)
		mMinmaxSize = 3;				// 3 x 3 의 행
		mAbsTrack   = 1;				// 1 허용범위
		mMinTrack   = (-TRACK_SIZE * mAbsTrack) - mAbsTrack;
		mMaxTrack   = (-TRACK_SIZE * mAbsTrack) + mAbsTrack;
		break;
	case _E_GMT_5X5_:
		mMatrixSize = 25;				// 5 x 5 의 값(행렬)
		mMinmaxSize = 5;				// 5 x 5 의 행
		mAbsTrack   = 2;				// 2 허용범위
		mMinTrack   = (-TRACK_SIZE * mAbsTrack) - mAbsTrack;
		mMaxTrack   = (-TRACK_SIZE * mAbsTrack) + mAbsTrack;
		break;
	case _E_GMT_7X7_:
		mMatrixSize = 49;				// 7 x 7 의 값(행렬)
		mMinmaxSize = 7;				// 7 x 7 의 행
		mAbsTrack   = 3;				// 3 허용범위
		mMinTrack   = (-TRACK_SIZE * mAbsTrack) - mAbsTrack;
		mMaxTrack   = (-TRACK_SIZE * mAbsTrack) + mAbsTrack;
		break;
	default:
		return false;
	}

	mMinGrid = DEF_GRID_POS;
	switch ( serverType )
	{
	case _E_ST_NORMAL_MAP_:
		mMaxGrid = MAP_SIZE*MAP_MAX;	// defined baseobject_common.h file
		break;
	case _E_ST_ID_PVP_:
		mMaxGrid = MAP_SIZE*INDUN_MAP_MAX;			// 테스트 후 상수 적용.
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		mMaxGrid = MAP_SIZE*INDUN_MAP_MAX;			// 테스트 후 상수 적용.
		break;
	default:
		return false;
	}

	mMatrix    = (int*)GlobalAlloc( GPTR, sizeof(int)*mMaxGrid );
	mPlayerMtr = (int*)GlobalAlloc( GPTR, sizeof(int)*mMaxGrid );

	mPlayerPool     = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mMonsterPool    = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mNpcPool        = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mItemPool       = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mTotemPool      = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mGatheringPool  = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );
	mUserPortalPool = new GridPool( mMinGrid, mMaxGrid, mMatrixSize );

	mOldMinMax = (GridMinMax*)GlobalAlloc( GPTR, sizeof(GridMinMax)*mMinmaxSize );
	mNewMinMax = (GridMinMax*)GlobalAlloc( GPTR, sizeof(GridMinMax)*mMinmaxSize );
	mOldMatrix = (GridMatrix*)GlobalAlloc( GPTR, sizeof(GridMatrix)*mMatrixSize );
	mNewMatrix = (GridMatrix*)GlobalAlloc( GPTR, sizeof(GridMatrix)*mMatrixSize );

	memset( &mPlayerData,  0, sizeof(mPlayerData ) );
	memset( &mMonsterData, 0, sizeof(mMonsterData) );
	memset( &mNpcData,     0, sizeof(mNpcData    ) );
	memset( &mItemData,    0, sizeof(mItemData   ) );
	memset( &mTotemData,   0, sizeof(mTotemData  ) );
	memset( &mGatheringData, 0, sizeof(mGatheringData) );
	memset( &mUserPortalData, 0, sizeof(mUserPortalData) );
	return true;
}

// Release Method
void cGridManager::Release()
{
	GlobalFree( mNewMatrix );
	GlobalFree( mOldMatrix );
	GlobalFree( mNewMinMax );
	GlobalFree( mOldMinMax );

	SAFE_DELETE( mUserPortalPool );
	SAFE_DELETE( mGatheringPool  );
	SAFE_DELETE( mTotemPool      );
	SAFE_DELETE( mItemPool       );
	SAFE_DELETE( mNpcPool        );
	SAFE_DELETE( mMonsterPool    );
	SAFE_DELETE( mPlayerPool     );

	GlobalFree( mPlayerMtr );
	GlobalFree( mMatrix    );
}

// Process Method - 주의: Player < Monster < NPC 순서 (Player를 중심으로 Monster와 NPC는 추가정보).
void cGridManager::Process(DWORD tickCount)
{
	GridMinMax gridMinMax;
	PerGrid*   temp;
	PerGrid*   next;

	/*-- PlayerPool 처리. --*/
	gridMinMax.minTrack = mMaxGrid;
	gridMinMax.maxTrack = mMinGrid;
	for ( int i = mPlayerPool->mMinMax.minTrack; i <= mPlayerPool->mMinMax.maxTrack; )
	{
		temp = mPlayerPool->mPagedPoolUsage[ i ];
		while ( temp != NULL )
		{
			next = temp->next;

			if ( mPlayerPool->ChangeGrid( temp, tickCount ) )
			{
				/*-- 그리드 매트릭스 계산. --*/
				Matrix( temp );

				/*-- 그리드 매트릭스 적용. --*/
				GridMatrix* oldMatrix = mOldMatrix;
				GridMatrix* newMatrix = mNewMatrix;

				for ( int i = 0; i < mMatrixSize; i++, oldMatrix++, newMatrix++ )
				{
					// 이전 그리드 중 삭제된 그리드.
					if ( oldMatrix->status < 0 )
					{
						SendSightOutPlayer    ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutMonster   ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutNpc       ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutItem      ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutTotem     ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutGathering ( (cPlayer*)temp->baseObject, oldMatrix->pos );
						SendSightOutUserPortal( (cPlayer*)temp->baseObject, oldMatrix->pos );
					}
					// 현재 그리드 중 추가된 그리드.
					if ( newMatrix->status > 0 )
					{
						SendSightInPlayer    ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInMonster   ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInNpc       ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInItem      ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInTotem     ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInGathering ( (cPlayer*)temp->baseObject, newMatrix->pos );
						SendSightInUserPortal( (cPlayer*)temp->baseObject, newMatrix->pos );
					}
				}
			}

			/*-- MinMaxTrack 재 계산. --*/
			gridMinMax.minTrack = min( gridMinMax.minTrack, temp->currentPos );
			gridMinMax.maxTrack = max( gridMinMax.maxTrack, temp->currentPos );

			temp = next;
		}

		i += 1;
		while ( i <= mPlayerPool->mMinMax.maxTrack )
		{
			int skip = i / MAP_SIZE;
			if ( mPlayerPool->mSkipMap[ skip ] == 0 )
			{
				i = (skip+1) * MAP_SIZE;
				continue;
			}
			else if ( mPlayerPool->mSkipMap[ skip ] < 0 )
			{
				NETWORK2->PostServerEvent( "PlayerPool::mSkipMap[%d]=%d", skip, mPlayerPool->mSkipMap[ skip ] );
				mPlayerPool->mSkipMap[ skip ] = 0;
			}
			break;
		}
	}
	/*-- Player Pool의 MinMaxTrack 갱신. --*/
	mPlayerPool->mMinMax = gridMinMax;


	/*-- MonsterPool 처리. --*/
	gridMinMax.minTrack = mMaxGrid;
	gridMinMax.maxTrack = mMinGrid;
	for ( int i = mMonsterPool->mMinMax.minTrack; i <= mMonsterPool->mMinMax.maxTrack; )
	{
		temp = mMonsterPool->mPagedPoolUsage[ i ];
		while ( temp != NULL )
		{
			next = temp->next;

			if ( mMonsterPool->ChangeGrid( temp, tickCount ) )
			{
				/*-- 그리드 매트릭스 계산. --*/
				Matrix( temp );

				/*-- 그리드 매트릭스 적용. --*/
				GridMatrix* oldMatrix = mOldMatrix;
				GridMatrix* newMatrix = mNewMatrix;

				for ( int i = 0; i < mMatrixSize; i++, oldMatrix++, newMatrix++ )
				{
					// 이전 그리드 중 삭제된 그리드.
					if ( oldMatrix->status < 0 )
					{
						SendMonsterSightOut( (cMonster*)temp->baseObject, oldMatrix->pos );
					}
					// 현재 그리드 중 추가된 그리드.
					if ( newMatrix->status > 0 )
					{
						SendMonsterSightIn( (cMonster*)temp->baseObject, newMatrix->pos );
					}
				}
			}

			/*-- MinMaxTrack 재 계산. --*/
			gridMinMax.minTrack = min( gridMinMax.minTrack, temp->currentPos );
			gridMinMax.maxTrack = max( gridMinMax.maxTrack, temp->currentPos );

			temp = next;
		}

		i += 1;
		while ( i <= mMonsterPool->mMinMax.maxTrack )
		{
			int skip = i / MAP_SIZE;
			if ( mMonsterPool->mSkipMap[ skip ] == 0 )
			{
				i = (skip+1) * MAP_SIZE;
				continue;
			}
			else if ( mMonsterPool->mSkipMap[ skip ] < 0 )
			{
				NETWORK2->PostServerEvent( "MonsterPool::mSkipMap[%d]=%d", skip, mMonsterPool->mSkipMap[ skip ] );
				mMonsterPool->mSkipMap[ skip ] = 0;
			}
			break;
		}
	}
	/*-- Monster Pool의 MinMaxTrack 갱신. --*/
	mMonsterPool->mMinMax = gridMinMax;
}

// FindPlayers Method
void cGridManager::FindPlayers(float x, float y, short map, float dst, void* stlAry, bool applyCheatHideMode)
{
	if ( stlAry == NULL ) return;
	tArray<unsigned long>* pArray = (tArray<unsigned long>*)stlAry;

	int p0 = mPlayerPool->CalcPos( x-dst, y-dst, map );
	int px = mPlayerPool->CalcPos( x+dst, y-dst, map );
	int py = mPlayerPool->CalcPos( x-dst, y+dst, map );

	int ylen = py - p0;
	int xlen = px - p0;
	int pos  = 0;

	PerGrid* grid   = NULL;
	cPlayer* player = NULL;
	NiPoint2 point;

	point.x = x;
	point.y = y;

	for ( int iy = 0; iy <= ylen; iy += (1 << TRACK_SIZE_SHIFT) )
	{
		for ( int ix = 0; ix <= xlen; ix++ )
		{
			pos  = p0 + ix + iy;
			if ( pos >= mPlayerPool->mMinGrid && pos < mPlayerPool->mMaxGrid )
			{
				grid = mPlayerPool->mPagedPoolUsage[ pos ];
				while ( grid != NULL )
				{
					player = (cPlayer*)grid->baseObject;

					if ( applyCheatHideMode == false )
					{
						if ( !player->GetCheatHideMode( ) )
						{
							if ( (point - player->GetPos( )).Length( ) <= dst )
								pArray->PushBack( player->GetObjectID() );
						}
					}
					else if ( (point - player->GetPos( )).Length( ) <= dst )
					{
						pArray->PushBack( player->GetObjectID() );
					}

					grid = grid->next;
				}
			}
		}
	}
}

// FindPlayers Method
void cGridManager::FindPlayers(cBaseObject* object, void* stlAry, bool applyCheatHideMode)
{
	if( stlAry == NULL )
		return;

	tArray<unsigned long>* pArray = (tArray<unsigned long>*)stlAry;

	PerGrid* seed  = NULL;
	PerGrid* grid  = NULL;
	int      index = 0;

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	default:
		return;
	}

	while ( index < mMatrixSize )
	{
		GridMatrix* matrix = &seed->matrix[ index ];
		cPlayer*    player = NULL;
		if ( matrix->status > 0 )
		{
			if ( grid == NULL )
			{
				grid = mPlayerPool->mPagedPoolUsage[ matrix->pos ];
			}
			if ( grid != NULL )
			{
				player  = (cPlayer*)grid->baseObject;
				grid = grid->next;
			}
		}
		if ( grid == NULL )
			index++;

		if ( player != NULL )
		{
			if ( applyCheatHideMode == false )
			{
				if ( !player->GetCheatHideMode( ) )
				{
					pArray->PushBack( player->GetObjectID() );
				}
			}
			else
			{
				pArray->PushBack( player->GetObjectID() );
			}
		}
	}
}

void cGridManager::FindSkillSendPlayer ( cBaseSkillObject* pSkill, bool applyCheatHideMode )
{
	if( pSkill == NULL )
		return;

	sSkillTargetRoot* pSkillRoot = pSkill->GetTargetRoot();
	if( pSkillRoot == NULL )
		return;

	PerNode* node = pSkillRoot->pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
		if( pSkillTarget == NULL )
		{
			node = node->next;
			continue;
		}

		cBaseObject* pObject = OBJECTMANAGER->GetObject( pSkillTarget->mTarget.type, pSkillTarget->mTarget.index );
		if( pObject == NULL )
		{
			node = node->next;
			continue;
		}

		PerGrid* seed  = NULL;
		PerGrid* grid  = NULL;
		int      index = 0;

		switch ( pObject->GetObjectType() )
		{
		case eOBJECTTYPE_HERO:
		case eOBJECTTYPE_PLAYER:
			seed = mPlayerPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_MONSTER:
			seed = mMonsterPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_NPC:
			seed = mNpcPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_ITEM:
			seed = mItemPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_TOTEM:
			seed = mTotemPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_GATHERING:
			seed = mGatheringPool->SearchObjectId( pObject->GetObjectID() );
			break;
		case eOBJECTTYPE_USERPORTAL:
			seed = mUserPortalPool->SearchObjectId( pObject->GetObjectID() );
			break;
		default:
			return;
		}

		if( seed != NULL )
		{
			while ( index < mMatrixSize )
			{
				GridMatrix* matrix = &seed->matrix[ index ];
				cPlayer*    player = NULL;
				if ( matrix->status > 0 )
				{
					if ( grid == NULL )
					{
						grid = mPlayerPool->mPagedPoolUsage[ matrix->pos ];
					}
					if ( grid != NULL )
					{
						player  = (cPlayer*)grid->baseObject;
						grid = grid->next;
					}
				}
				if ( grid == NULL )
					index++;

				if ( player != NULL )
				{
					if ( applyCheatHideMode == false )
					{
						if ( !player->GetCheatHideMode( ) )
						{
							/// send 타겟 플레이어
							pSkill->AddSendTarget( player->GetObjectID() );
						}
					}
					else
					{
						/// send 타겟 플레이어
						pSkill->AddSendTarget( player->GetObjectID() );
					}
				}
			}
		}

		node = node->next;
	}
}

// FindFirstPlayer Method
cPlayer* cGridManager::FindFirstPlayer(cBaseObject* object, bool applyCheatHideMode)
{
	memset( &mPlayerData, 0, sizeof(mPlayerData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mPlayerData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mPlayerData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mPlayerData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mPlayerData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mPlayerData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mPlayerData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mPlayerData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextPlayer( applyCheatHideMode );
}

// FindNextPlayer Method
cPlayer* cGridManager::FindNextPlayer(bool applyCheatHideMode)
{
	if ( mPlayerData.seed != NULL )
	{
		int&      matrixCount = mPlayerData.matrixCount;
		PerGrid** tree        = &mPlayerData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix = &mPlayerData.seed->matrix[ matrixCount ];
			cPlayer*    player = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mPlayerPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					player  = (cPlayer*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( player != NULL )
			{
				if ( applyCheatHideMode == false )
				{
					if ( !player->GetCheatHideMode( ) )
						return player;
				}
				else
					return player;
			}
		}
	}
	return NULL;
}

// FindMonsters Method
void cGridManager::FindMonsters(float x, float y, short map, float dst, void* stlAry)
{
	if ( stlAry == NULL ) return;
	tArray<unsigned long>* pArray = (tArray<unsigned long>*)stlAry;

	int p0 = mMonsterPool->CalcPos( x-dst, y-dst, map );
	int px = mMonsterPool->CalcPos( x+dst, y-dst, map );
	int py = mMonsterPool->CalcPos( x-dst, y+dst, map );

	int ylen = py - p0;
	int xlen = px - p0;
	int pos  = 0;

	PerGrid*  grid    = NULL;
	cMonster* monster = NULL;
	NiPoint2  point;

	point.x = x;
	point.y = y;

	for ( int iy = 0; iy <= ylen; iy += (1 << TRACK_SIZE_SHIFT) )
	{
		for ( int ix = 0; ix <= xlen; ix++ )
		{
			pos  = p0 + ix + iy;
			if ( pos >= mMonsterPool->mMinGrid && pos < mMonsterPool->mMaxGrid )
			{
				grid = mMonsterPool->mPagedPoolUsage[ pos ];
				while ( grid != NULL )
				{
					monster = (cMonster*)grid->baseObject;

					if ( (point - monster->GetPos( )).Length( ) <= dst )
					{
						pArray->PushBack( monster->GetObjectID() );
					}

					grid = grid->next;
				}
			}
		}
	}
}

// FindMonsters Method
void cGridManager::FindMonsters(cBaseObject* object, void* stlAry)
{
	if( stlAry == NULL )
		return;

	tArray<unsigned long>* pArray = (tArray<unsigned long>*)stlAry;

	PerGrid* seed  = NULL;
	PerGrid* grid  = NULL;
	int      index = 0;

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	default:
		return;
	}

	while ( index < mMatrixSize )
	{
		GridMatrix* matrix  = &seed->matrix[ index ];
		cMonster*   monster = NULL;
		if ( matrix->status > 0 )
		{
			if ( grid == NULL )
			{
				grid = mMonsterPool->mPagedPoolUsage[ matrix->pos ];
			}
			if ( grid != NULL )
			{
				monster = (cMonster*)grid->baseObject;
				grid    = grid->next;
			}
		}
		if ( grid == NULL )
			index++;

		if ( monster != NULL )
		{
			pArray->PushBack( monster->GetObjectID() );
		}
	}
}

// FindFirstMonster Method
cMonster* cGridManager::FindFirstMonster(cBaseObject* object)
{
	memset( &mMonsterData, 0, sizeof(mMonsterData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mMonsterData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mMonsterData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mMonsterData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mMonsterData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mMonsterData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mMonsterData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mMonsterData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextMonster();
}

// FindNextMonster Method
cMonster* cGridManager::FindNextMonster()
{
	if ( mMonsterData.seed != NULL )
	{
		int&      matrixCount = mMonsterData.matrixCount;
		PerGrid** tree        = &mMonsterData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix  = &mMonsterData.seed->matrix[ matrixCount ];
			cMonster*   monster = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mMonsterPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					monster  = (cMonster*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( monster != NULL )
				return monster;
		}
	}
	return NULL;
}

// FindFirstNpc Method
cNpc* cGridManager::FindFirstNpc(cBaseObject* object)
{
	memset( &mNpcData, 0, sizeof(mNpcData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mNpcData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mNpcData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mNpcData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mNpcData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mNpcData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mNpcData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mNpcData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextNpc();
}

// FindNextNpc Method
cNpc* cGridManager::FindNextNpc()
{
	if ( mNpcData.seed != NULL )
	{
		int&      matrixCount = mNpcData.matrixCount;
		PerGrid** tree        = &mNpcData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix = &mNpcData.seed->matrix[ matrixCount ];
			cNpc*       npc    = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mNpcPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					npc  = (cNpc*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( npc != NULL )
				return npc;
		}
	}
	return NULL;
}

// FindFirstItem Method
cItem* cGridManager::FindFirstItem(cBaseObject* object)
{
	memset( &mItemData, 0, sizeof(mItemData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mItemData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mItemData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mItemData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mItemData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mItemData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mItemData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mItemData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextItem();
}

// FindNextItem Method
cItem* cGridManager::FindNextItem()
{
	if ( mItemData.seed != NULL )
	{
		int&      matrixCount = mItemData.matrixCount;
		PerGrid** tree        = &mItemData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix = &mItemData.seed->matrix[ matrixCount ];
			cItem*      item   = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mItemPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					item  = (cItem*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( item != NULL )
				return item;
		}
	}
	return NULL;
}

//
// FindFirstItem Method
cTotem* cGridManager::FindFirstTotem(cBaseObject* object)
{
	memset( &mTotemData, 0, sizeof(mTotemData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mTotemData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mTotemData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mTotemData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mTotemData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mTotemData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mTotemData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mTotemData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextTotem();
}

// FindNextItem Method
cTotem* cGridManager::FindNextTotem()
{
	if ( mTotemData.seed != NULL )
	{
		int&      matrixCount = mTotemData.matrixCount;
		PerGrid** tree        = &mTotemData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix = &mTotemData.seed->matrix[ matrixCount ];
			cTotem*     totem  = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mTotemPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					totem  = (cTotem*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( totem != NULL )
				return totem;
		}
	}
	return NULL;
}

// FindFirstGathering Method
cGathering* cGridManager::FindFirstGathering(cBaseObject* object)
{
	memset( &mGatheringData, 0, sizeof(mGatheringData) );

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		mGatheringData.seed = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		mGatheringData.seed = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		mGatheringData.seed = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		mGatheringData.seed = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		mGatheringData.seed = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		mGatheringData.seed = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		mGatheringData.seed = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	return FindNextGathering();
}

// FindNextItem Method
cGathering* cGridManager::FindNextGathering()
{
	if ( mGatheringData.seed != NULL )
	{
		int&      matrixCount = mGatheringData.matrixCount;
		PerGrid** tree        = &mGatheringData.tree;

		while ( matrixCount < mMatrixSize )
		{
			GridMatrix* matrix = &mGatheringData.seed->matrix[ matrixCount ];
			cGathering*     gathering = NULL;
			if ( matrix->status > 0 )
			{
				if ( (*tree) == NULL )
				{
					(*tree) = mGatheringPool->mPagedPoolUsage[ matrix->pos ];
				}
				if ( (*tree) != NULL )
				{
					gathering  = (cGathering*)(*tree)->baseObject;
					(*tree) = (*tree)->next;
				}
			}
			if ( (*tree) == NULL )
				matrixCount++;

			if ( gathering != NULL )
				return gathering;
		}
	}
	return NULL;
}

// IsSight Method
bool cGridManager::IsSight(cBaseObject* object, long objectId)
{
	PerGrid* perGrid = mPlayerPool->SearchObjectId( objectId );
	return (perGrid != NULL) ? IsSight( object, perGrid->baseObject ) : false;
}

// IsPlayer Method
bool cGridManager::IsSight(cBaseObject* object1, cBaseObject* object2)
{
	int gridPos1 = object1->GetGridPos( );
	int gridPos2 = object2->GetGridPos( ) - (TRACK_SIZE * mAbsTrack);
	int result;
	for ( int i = 0; i < mMinmaxSize; i++ )
	{
		result = gridPos1 - gridPos2;
		result = abs( result );
		if ( result <= mAbsTrack )
			return true;
		gridPos2 += TRACK_SIZE;
	}
	return false;
}

// IsPlayer Method
int cGridManager::IsPlayer(cBaseObject* object)
{
	int gridPos = object->GetGridPos( );
	return mPlayerMtr[ gridPos ];
}

// GetPlayer Method
cPlayer* cGridManager::GetPlayer(long objectId)
{
	PerGrid* perGrid = mPlayerPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cPlayer*)perGrid->baseObject : NULL);
}

// AddPlayer Method
bool cGridManager::AddPlayer(cPlayer* player)
{
	unsigned long objectId = player->GetObjectID();
	PerGrid*      perGrid  = mPlayerPool->SearchObjectId( objectId );;

	if ( perGrid == NULL )
	{
		perGrid = mPlayerPool->GetGrid( player );
		if ( perGrid != NULL )
		{
			// 070123 PKH 스텟 계산.
			STATUSCALC->CalcPlayerInit( player->GetObject( ) );

			// 게임-인.
			player->SetGameIn( );
			player->StartItemBill( );

			// NPC 리스트 내보내기(MAP 단위).
			SendNpcList( player );

			// 효과 프로세스 시작.
			player->InfluenceProcessStart( );

			/// 사용 sp 정보 갱신
			player->UpdateJobUsedSP();
			player->SendSkillPoint();

			//
			int modeIndex = player->GetTutorialMode();
			if( modeIndex > -1 )
				player->SetTutorialMode( modeIndex );

			/// 오늘의 한마디 정보 보내기
			player->SendInitTodayWord();

			// 파티 정보 복구
			PARTYMAN->SetRestore( player );
			// 파티정보 내보내기.
			PARTYMAN->PartyMapIn( player );

			// 파티 연합 정보 보내기
			PARTYUNIONMAN->MapIn( player );

			// 길드정보 내보내기
			GUILDMAN->PlayerMapIn( player );

			// 신규 퀘스트 내보내기 - MAP 단위.
			player->SendNewQuestList();
			
			player->SendCoolTimeChgMon();

			// 그리드 매트릭스 계산.
			Matrix( perGrid );

			// 그리드 매트릭스 정보 내보내기 - 시작.
			GridMatrix* matrix = perGrid->matrix;
			for ( int i = 0; i < mMatrixSize; i++, matrix++ )
			{
				if ( matrix->status > 0 )
				{
					// 이미 접속해 있는 캐릭터에게 새로운 캐릭터 정보 내보내기.
					// 이미 접속해 있는 캐릭터 정보를 새로운 캐릭터에 내보내기.
					SendPlayerGameIn( player, matrix->pos );

					// 새로운 캐릭터에게 몬스터 정보 내보내기.
					SendSightInMonster( player, matrix->pos );

					// 새로운 캐릭터에게 NPC 정보 내보내기.
					SendSightInNpc( player, matrix->pos );

					// 새로운 캐릭터에게 ITEM 정보 내보내기.
					SendSightInItem( player, matrix->pos );

					// 새로운 캐릭터에게 TOTEM 정보 내보내기.
					SendSightInTotem( player, matrix->pos );

					// 새로운 캐릭터에게 Gathering 정보 내보내기.
					SendSightInGathering( player, matrix->pos );

					// 새로운 캐릭터에게 UserPortal 정보 내보내기.
					SendSightInUserPortal( player, matrix->pos );
				}
			}
			// 그리드 매트릭스 정보 내보내기 - 완료.
			return true;
		}
	}
	assert( NULL );
	return false;
}

// RemovePlayer Method
bool cGridManager::RemovePlayer(cPlayer* player)
{
	unsigned long objectId = player->GetObjectID();
	PerGrid*      perGrid  = mPlayerPool->SearchObjectId( objectId );

	if ( perGrid != NULL )
	{
		if ( !player->GetCheatHideMode( ) )
		{
			// 그리드 매트릭스 정보 내보내기 - 시작.
			GridMatrix* matrix = perGrid->matrix;
			for ( int i = 0; i < mMatrixSize; i++, matrix++ )
			{
				if ( matrix->status > 0 )
				{
					// 이미 접속해 있는 캐릭터에게 종료 캐릭터 정보 내보내기.
					SendPlayerGameOut( player, matrix->pos );
				}
			}
			// 그리드 매트릭스 정보 내보내기 - 완료.
		}

		// 거래상태 취소
		player->ExchangeCancel( false );

		// 신청상태 전부 취소 처리
		if( player->GetRequestRejection() != eREQREJCT_NONE )
			player->EndRequestRejectionAll();

		// 길드 신청 받는 유저의 신청정보 취소 처리
		GUILDMAN->ClearRequest( player );

		player->StopItemBill( );

		// 게임-아웃.
		player->SetGameOut();

		player->ClearTargetingMonster();

		mPlayerPool->ReleaseGrid( perGrid );
		return true;
	}
	return false;
}

// SetHidePlayer Method
void cGridManager::SetHidePlayer(cPlayer* player, bool apply)
{
	unsigned long objectId = player->GetObjectID();
	PerGrid*      perGrid  = mPlayerPool->SearchObjectId( objectId );

	if ( perGrid != NULL )
	{
		// 그리드 매트릭스 정보 내보내기 - 시작.
		GridMatrix* matrix = perGrid->matrix;
		for ( int i = 0; i < mMatrixSize; i++, matrix++ )
		{
			if ( matrix->status > 0 )
			{
				apply == true ? SendPlayerGameOut( player, matrix->pos, false ) : SendPlayerGameIn( player, matrix->pos, false );
			}
		}
	}
}

// GetMonster Method
cMonster* cGridManager::GetMonster(long objectId)
{
	PerGrid* perGrid = mMonsterPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cMonster*)perGrid->baseObject : NULL);
}

// AddMonster Method - 몬스터 추가
bool cGridManager::AddMonster( cMonster* monster )
{
	if( monster == NULL )
		return false;

	unsigned long objectIdx = monster->GetObjectID();
	PerGrid*      perGrid   = mMonsterPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mMonsterPool->GetGrid( monster );
		if ( perGrid != NULL )
		{
			Matrix( perGrid );

			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveMonster Method - 몬스터 삭제
bool cGridManager::RemoveMonster( cMonster* monster )
{
	if( monster == NULL )
		return false;

	unsigned long objectIdx = monster->GetObjectID();
	PerGrid*      perGrid   = mMonsterPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		mMonsterPool->ReleaseGrid( perGrid );
		return true;
	}

	assert( NULL );
	return false;
}

// GetNpc Method
cNpc* cGridManager::GetNpc(long objectId)
{
	PerGrid* perGrid = mNpcPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cNpc*)perGrid->baseObject : NULL);
}

// AddNpc Method - NPC 추가
bool cGridManager::AddNpc(cNpc* npc)
{
	unsigned long objectIdx = npc->GetObjectID();
	PerGrid*      perGrid   = mNpcPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mNpcPool->GetGrid( npc );
		if (perGrid != NULL )
		{
			Matrix( perGrid );
			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveNpc Method - NPC 삭제
bool cGridManager::RemoveNpc(cNpc* npc)
{
	unsigned long objectIdx = npc->GetObjectID();
	PerGrid*      perGrid   = mNpcPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		mNpcPool->ReleaseGrid( perGrid );
		return true;
	}

	NETWORK2->PostServerEvent( "cGridManager::RemoveNpc[%d] return false", objectIdx );
	return false;
}

// GetItem Method
cItem* cGridManager::GetItem(long objectId)
{
	PerGrid* perGrid = mItemPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cItem*)perGrid->baseObject : NULL);
}

// AddItem Method
bool cGridManager::AddItem(cItem* item)
{
	unsigned long objectIdx = item->GetObjectID();
	PerGrid*      perGrid   = mItemPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mItemPool->GetGrid( item );
		if (perGrid != NULL )
		{
			Matrix( perGrid );

			MSG_ITEM_INFO msg;
			sItemDrop*    itemDrop = &msg.itemDrop;

			memset( &msg, 0, sizeof(MSG_ITEM_INFO) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_DROP_IN_SYN;

			item->GetOwnerInfo( msg.owner, sizeof(msg.owner), msg.timeToOwner );
			item->GetPartyInfo( msg.party, sizeof(msg.party), msg.timeToParty );
			msg.rare = item->GetItemRare();

			itemDrop->idx  = item->GetObjectID();
			itemDrop->xPos = item->GetXPos();
			itemDrop->yPos = item->GetYPos();

			NETWORK2->QuickSend( item, (char*)&msg, sizeof(MSG_ITEM_INFO) );
			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveItem Method
bool cGridManager::RemoveItem(cItem* item)
{
	unsigned long objectIdx = item->GetObjectID();
	PerGrid*      perGrid   = mItemPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		MSG_ITEMIDX msg;

		msg.Category = NM_ITEM;
		msg.Protocol = NM_ITEM_DROP_OUT_SYN;
		msg.idx      = objectIdx;

		NETWORK2->QuickSend( item, (char*)&msg, sizeof(MSG_ITEMIDX) );

		mItemPool->ReleaseGrid( perGrid );
		return true;
	}

	assert( NULL );
	return false;
}

// GetTotem Method
cTotem* cGridManager::GetTotem(long objectId)
{
	PerGrid* perGrid = mTotemPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cTotem*)perGrid->baseObject : NULL);
}

// AddTotem Method
bool cGridManager::AddTotem(cTotem* totem)
{
	unsigned long objectIdx = totem->GetObjectID();
	PerGrid*      perGrid   = mTotemPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mTotemPool->GetGrid( totem );
		if (perGrid != NULL )
		{
			Matrix( perGrid );

			MSG_TOTEM_INFO msg;

			memset( &msg, 0, sizeof(MSG_TOTEM_INFO) );

			msg.Category   = NM_TOTEM;
			msg.Protocol   = NM_TOTEM_SIGHT_IN_RES;
			msg.mObjectIdx = objectIdx;
			msg.mPosX      = totem->GetXPos( );
			msg.mPosY      = totem->GetYPos( );
			msg.mDirection = totem->GetDirection();
			msg.mCreateInfClassIdx = totem->GetClassIdx( );

			NETWORK2->QuickSend( totem, (char*)&msg, sizeof(MSG_TOTEM_INFO) );
			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveTotem Method
bool cGridManager::RemoveTotem(cTotem* totem)
{
	unsigned long objectIdx = totem->GetObjectID();
	PerGrid*      perGrid   = mTotemPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		MSG_TOTEMIDX msg;

		msg.Category   = NM_TOTEM;
		msg.Protocol   = NM_TOTEM_SIGHT_OUT_RES;
		msg.mObjectIdx = objectIdx;

		NETWORK2->QuickSend( totem, (char*)&msg, sizeof(MSG_ITEMIDX) );

		mTotemPool->ReleaseGrid( perGrid );
		return true;
	}

	assert( NULL );
	return false;
}

// GetGathering Method
cGathering* cGridManager::GetGathering(long objectId)
{
	PerGrid* perGrid = mGatheringPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cGathering*)perGrid->baseObject : NULL);
}


// AddGathering Method
bool cGridManager::AddGathering(cGathering* gathering)
{
	unsigned long objectIdx = gathering->GetObjectID();
	PerGrid*      perGrid   = mGatheringPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mGatheringPool->GetGrid( gathering );
		if (perGrid != NULL )
		{
			Matrix( perGrid );
			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveGathering Method
bool cGridManager::RemoveGathering(cGathering* pGathering)
{
	unsigned long objectIdx = pGathering->GetObjectID();
	PerGrid*      perGrid   = mGatheringPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		mGatheringPool->ReleaseGrid( perGrid );
		return true;
	}

	assert( NULL );
	return false;
}

// GetUserPortal Method
cUserPortal* cGridManager::GetUserPortal(long objectId)
{
	PerGrid* perGrid = mUserPortalPool->SearchObjectId( objectId );
	return (perGrid != NULL ? (cUserPortal*)perGrid->baseObject : NULL);
}

// AddUserPortal Method
bool cGridManager::AddUserPortal(cUserPortal* pUserPortal)
{
	unsigned long objectIdx = pUserPortal->GetObjectID();
	PerGrid*      perGrid   = mUserPortalPool->SearchObjectId( objectIdx );

	if ( perGrid == NULL )
	{
		perGrid = mUserPortalPool->GetGrid( pUserPortal );
		if (perGrid != NULL )
		{
			Matrix( perGrid );

			MSG_USERPORTAL_CREATE msg;

			memset( &msg, 0, sizeof(MSG_USERPORTAL_CREATE) );

			msg.Category   = NM_USERPORTAL;
			msg.Protocol   = NM_USERPORTAL_REGEN;
			msg.mObjectIdx = objectIdx;
			msg.mMapChangePosIdx = pUserPortal->GetMapChangePosIdx();
			msg.mPlayerIdx = pUserPortal->GetCreator();
			msg.mPosX      = pUserPortal->GetXPos( );
			msg.mPosY      = pUserPortal->GetYPos( );

			NETWORK2->QuickSend( pUserPortal, (char*)&msg, sizeof(MSG_USERPORTAL_CREATE) );
			return true;
		}
	}

	assert( NULL );
	return false;
}

// RemoveUserPortal Method
bool cGridManager::RemoveUserPortal(cUserPortal* pUserPortal)
{
	unsigned long objectIdx = pUserPortal->GetObjectID();
	PerGrid*      perGrid   = mUserPortalPool->SearchObjectId( objectIdx );

	if ( perGrid != NULL )
	{
		MSG_USERPORTAL_DELETE msg;

		msg.Category   = NM_USERPORTAL;
		msg.Protocol   = NM_USERPORTAL_DELETE;
		msg.mObjectIdx = objectIdx;

		NETWORK2->QuickSend( pUserPortal, (char*)&msg, sizeof(MSG_USERPORTAL_DELETE) );

		mUserPortalPool->ReleaseGrid( perGrid );
		return true;
	}

	assert( NULL );
	return false;
}

// MakeCID Method
long cGridManager::MakeCID(cBaseObject* object,
						   u_long       cidSize,
						   char*        buffer,
						   u_long       bufferLength,
						   u_long&      indPtr)
{
	PerGrid* perGrid = NULL;

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		perGrid = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		perGrid = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		perGrid = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		perGrid = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		perGrid = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		perGrid = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		perGrid = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	if ( perGrid != NULL )
	{
		GridMatrix* matrix = perGrid->matrix;
		u_long      length = bufferLength - cidSize;

		cPlayer*    player;
		DWORD       cid;

		for ( int i = 0; i < mMatrixSize; i++, matrix++ )
		{
			if ( matrix->status > 0 )
			{
				PerGrid* temp = mPlayerPool->mPagedPoolUsage[ matrix->pos ];
				while ( temp != NULL && indPtr <= length )
				{
					player = (cPlayer*)temp->baseObject;
					cid    = player->GetConnectionIdx();

					(*(DWORD*)buffer) = cid;

					buffer += cidSize;
					indPtr += cidSize;

					temp = temp->next;
				}
			}
		}
	}
	return indPtr;
}

// MakeCIDExcept Method
long cGridManager::MakeCIDExcept(cBaseObject* object,
								 u_long       cidSize,
								 char*        buffer,
								 u_long       bufferLength,
								 u_long&      indPtr)
{
	PerGrid* perGrid = NULL;

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		perGrid = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		perGrid = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		perGrid = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		perGrid = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		perGrid = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		perGrid = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		perGrid = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	if ( perGrid != NULL )
	{
		GridMatrix* matrix = perGrid->matrix;
		u_long      length = bufferLength - cidSize;

		cPlayer*    player;
		DWORD       cid;

		for ( int i = 0; i < mMatrixSize; i++, matrix++ )
		{
			if ( matrix->status > 0 )
			{
				PerGrid* temp = mPlayerPool->mPagedPoolUsage[ matrix->pos ];
				while ( temp != NULL && indPtr <= length )
				{
					if ( temp->baseObject != object )
					{
						player = (cPlayer*)temp->baseObject;
						cid    = player->GetConnectionIdx();

						(*(DWORD*)buffer) = cid;

						buffer += cidSize;
						indPtr += cidSize;
					}
					temp = temp->next;
				}
			}
		}
	}
	return indPtr;
}

// MakeCIDMap Method
long cGridManager::MakeCIDMap(cBaseObject* object,
							  u_long       cidSize,
							  char*        buffer,
							  u_long       bufferLength,
							  u_long&      indPtr)
{
	PerGrid* perGrid = NULL;

	switch ( object->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		perGrid = mPlayerPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		perGrid = mMonsterPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		perGrid = mNpcPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		perGrid = mItemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		perGrid = mTotemPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		perGrid = mGatheringPool->SearchObjectId( object->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		perGrid = mUserPortalPool->SearchObjectId( object->GetObjectID() );
		break;
	}

	if ( perGrid != NULL )
	{
		int      beginPos = mPlayerPool->CalcPos( object->GetMapNumber() );
		int      endPos   = beginPos + MAP_SIZE;

		u_long   length   = bufferLength - cidSize;
		cPlayer* player;
		DWORD    cid;

		while ( beginPos < endPos && beginPos < mMaxGrid )
		{
			PerGrid* temp = mPlayerPool->mPagedPoolUsage[ beginPos ];
			while ( temp != NULL && indPtr <= length )
			{
				player = (cPlayer*)temp->baseObject;
				cid    = player->GetConnectionIdx();

				(*(DWORD*)buffer) = cid;

				buffer += cidSize;
				indPtr += cidSize;

				temp = temp->next;
			}
			beginPos++;
		}
	}
	return indPtr;
}

// MakeCIDMap Method
long cGridManager::MakeCIDMap(unsigned short mapNumber, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr)
{
	int      beginPos = mPlayerPool->CalcPos( mapNumber );
	int      endPos   = beginPos + MAP_SIZE;

	u_long   length   = bufferLength - cidSize;
	cPlayer* player;
	DWORD    cid;

	while ( beginPos < endPos && beginPos < mMaxGrid )
	{
		PerGrid* temp = mPlayerPool->mPagedPoolUsage[ beginPos ];
		while ( temp != NULL && indPtr <= length )
		{
			player = (cPlayer*)temp->baseObject;
			cid    = player->GetConnectionIdx();

			(*(DWORD*)buffer) = cid;

			buffer += cidSize;
			indPtr += cidSize;

			temp = temp->next;
		}
		beginPos++;
	}

	return indPtr;
}

// MakeCIDExceptMap Method
long cGridManager::MakeCIDExceptMap(cBaseObject* object1, cBaseObject* object2, u_long cidSize, char* buffer, u_long bufferLength, u_long& indPtr)
{
	PerGrid* perGrid = NULL;

	if ( object1->GetMapNumber( ) != object2->GetMapNumber( ) )
		return indPtr;

	switch ( object1->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		perGrid = mPlayerPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_MONSTER:
		perGrid = mMonsterPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_NPC:
		perGrid = mNpcPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_ITEM:
		perGrid = mItemPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_TOTEM:
		perGrid = mTotemPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_GATHERING:
		perGrid = mGatheringPool->SearchObjectId( object1->GetObjectID() );
		break;
	case eOBJECTTYPE_USERPORTAL:
		perGrid = mUserPortalPool->SearchObjectId( object1->GetObjectID() );
		break;
	}

	if ( perGrid != NULL )
	{
		int      beginPos = mPlayerPool->CalcPos( object1->GetMapNumber() );
		int      endPos   = beginPos + MAP_SIZE;

		u_long   length   = bufferLength - cidSize;
		cPlayer* player;
		DWORD    cid;

		while ( beginPos < endPos && beginPos < mMaxGrid )
		{
			PerGrid* temp = mPlayerPool->mPagedPoolUsage[ beginPos ];
			while ( temp != NULL && indPtr <= length )
			{
				if ( !(temp->baseObject == object1 || temp->baseObject == object2) )
				{
					player = (cPlayer*)temp->baseObject;
					cid    = player->GetConnectionIdx();

					(*(DWORD*)buffer) = cid;

					buffer += cidSize;
					indPtr += cidSize;
				}
				temp = temp->next;
			}
			beginPos++;
		}
	}
	return indPtr;
}

// MakePIDMap Method
long cGridManager::MakePIDMap(unsigned short mapNumber, u_long pidSize, char* buffer, u_long bufferLength, u_long& indPtr)
{
	int      beginPos = mPlayerPool->CalcPos( mapNumber );
	int      endPos   = beginPos + MAP_SIZE;

	u_long   length   = bufferLength - pidSize;
	cPlayer* player;
	u_long   pid;

	while ( beginPos < endPos && beginPos < mMaxGrid )
	{
		PerGrid* temp = mPlayerPool->mPagedPoolUsage[ beginPos ];
		while ( temp != NULL && indPtr <= length )
		{
			player = (cPlayer*)temp->baseObject;
			pid    = player->GetObjectID( );

			(*(DWORD*)buffer) = pid;

			buffer += pidSize;
			indPtr += pidSize;

			temp = temp->next;
		}
		beginPos++;
	}

	return indPtr;
}

// Matrix Method
void cGridManager::Matrix(PerGrid* perGrid)
{
	/*-- 그리드의 이전과 현재 위치 참조.
	*/
	int lastPos    = perGrid->lastPos;
	int currentPos = perGrid->currentPos;

	GridMinMax* oldMinMax;
	GridMinMax* newMinMax;
	GridMatrix* matrix;

	/*-- Sight - In / Out Track & Matrix
	*/
	int lastMinTrack    = lastPos    + mMinTrack;
	int lastMaxTrack    = lastPos    + mMaxTrack;
	int currentMinTrack = currentPos + mMinTrack;
	int currentMaxTrack = currentPos + mMaxTrack;

	oldMinMax = mOldMinMax;
	newMinMax = mNewMinMax;
	for ( int i0 = 0; i0 < mMinmaxSize; i0++ )
	{
		// 이전 트랙과 매트릭스
		oldMinMax->minTrack = lastMinTrack < 0 ? max( lastMinTrack, mMinGrid ) : min( lastMinTrack, mMaxGrid-1 );
		oldMinMax->maxTrack = lastMaxTrack < 0 ? max( lastMaxTrack, mMinGrid ) : min( lastMaxTrack, mMaxGrid-1 );

		if ( oldMinMax->minTrack != oldMinMax->maxTrack )
		{
			int* ptr = mMatrix + oldMinMax->minTrack;
			for ( int i1 = oldMinMax->minTrack; i1 <= oldMinMax->maxTrack; i1++, ptr++ )
				(*ptr) -= 1;
		}

		lastMinTrack += TRACK_SIZE;
		lastMaxTrack += TRACK_SIZE;
		oldMinMax++;

		// 현재 트랙과 매트릭스
		newMinMax->minTrack = currentMinTrack < 0 ? max( currentMinTrack, mMinGrid ) : min( currentMinTrack, mMaxGrid-1 );
		newMinMax->maxTrack = currentMaxTrack < 0 ? max( currentMaxTrack, mMinGrid ) : min( currentMaxTrack, mMaxGrid-1 );

		if ( newMinMax->minTrack != newMinMax->maxTrack )
		{
			int* ptr = mMatrix + newMinMax->minTrack;
			for ( int i1 = newMinMax->minTrack; i1 <= newMinMax->maxTrack; i1++, ptr++ )
				(*ptr) += 1;
		}

		currentMinTrack += TRACK_SIZE;
		currentMaxTrack += TRACK_SIZE;
		newMinMax++;
	}


	/*-- 단위 그리드의 매트릭스.
	*/
	memset( perGrid->matrix, 0, sizeof(GridMatrix)*mMatrixSize );

	matrix    = perGrid->matrix;
	newMinMax = mNewMinMax;
	for ( int i0 = 0; i0 < mMinmaxSize; i0++, newMinMax++ )
	{
		if ( newMinMax->minTrack != newMinMax->maxTrack )
		{
			for ( int i1 = newMinMax->minTrack; i1 <= newMinMax->maxTrack; i1++ )
			{
				matrix->status = 1;
				matrix->pos    = i1;
				matrix++;
			}
		}
	}


	/*-- Sight - In / Out Track & Matrix
	*/
	memset( mOldMatrix, 0, sizeof(GridMatrix)*mMatrixSize );
	memset( mNewMatrix, 0, sizeof(GridMatrix)*mMatrixSize );

	GridMatrix* oldMatrix = mOldMatrix;
	GridMatrix* newMatrix = mNewMatrix;

	oldMinMax = mOldMinMax;
	newMinMax = mNewMinMax;
	for ( int i0 = 0; i0 < mMinmaxSize; i0++, oldMinMax++, newMinMax++ )
	{
		if ( oldMinMax->minTrack != oldMinMax->maxTrack )
		{
			for ( int i1 = oldMinMax->minTrack; i1 <= oldMinMax->maxTrack; i1++ )
			{
				if ( mMatrix[ i1 ] < 0 )
				{
					oldMatrix->status = mMatrix[ i1 ];
					oldMatrix->pos    = i1;
					oldMatrix++;

					mMatrix[ i1 ] = 0;
				}
			}
		}
		if ( newMinMax->minTrack != newMinMax->maxTrack )
		{
			for ( int i1 = newMinMax->minTrack; i1 <= newMinMax->maxTrack; i1++ )
			{
				if ( mMatrix[ i1 ] > 0 )
				{
					newMatrix->status = mMatrix[ i1 ];
					newMatrix->pos    = i1;
					newMatrix++;

					mMatrix[ i1 ] = 0;
				}
			}
		}
	}
}

// SendPlayerGameIn Method - 플레이어 접속완료.
void cGridManager::SendPlayerGameIn(cPlayer* player, int gridPos, bool mtrApply)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		if ( perGrid->baseObject != player )
		{
			cPlayer* other = (cPlayer*)perGrid->baseObject;

			// FROM PLAYER TO 그리드(OTHER PLAYER).
			if ( !player->GetCheatHideMode( ) )
				player->SendSightIn( NM_PLAYER, NM_PLAYER_GAMEIN_SYN, other->GetConnectionIdx() );

			// FROM 그리드(OTHER PLAYER) TO PLAYER.
			if ( !other->GetCheatHideMode( ) && mtrApply == true )
				other->SendSightIn( NM_PLAYER, NM_PLAYER_LIST_RES, player->GetConnectionIdx() );
		}

		perGrid = perGrid->next;
	}

	if ( mtrApply == true )
		mPlayerMtr[ gridPos ] += 1;
}

// SendPlayerGameOut Method - 플레이어 접속종료.
void cGridManager::SendPlayerGameOut(cPlayer* player, int gridPos, bool mtrApply)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		if ( perGrid->baseObject != player )
		{
			cPlayer* other = (cPlayer*)perGrid->baseObject;

			// FROM PLAYER TO 그리드(OTHER PLAYER).
			if ( !player->GetCheatHideMode( ) )
				player->SendSightOut( NM_PLAYER, NM_PLAYER_DISCONNECT_SYN, other->GetConnectionIdx() );
		}
		perGrid = perGrid->next;
	}

	if ( mtrApply == true )
	{
		if ( mPlayerMtr[ gridPos ] > 0 )
			mPlayerMtr[ gridPos ] -= 1;
		else
			NETWORK2->PostServerEvent( "cGridManager::SendPlayerGameOut(=%d -1)", mPlayerMtr[ gridPos ] );
	}
}

// SendSightInPlayer Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInPlayer(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		if ( perGrid->baseObject != player )
		{
			cPlayer* other = (cPlayer*)perGrid->baseObject;

			// FROM PLAYER TO 그리드(OTHER PLAYER).
			if ( !player->GetCheatHideMode( ) )
				player->SendSightIn( NM_PLAYER, NM_PLAYER_LIST_RES, other->GetConnectionIdx() );

			// FROM 그리드(OTHER PLAYER) TO PLAYER.
			if ( !other->GetCheatHideMode( ) )
				other->SendSightIn( NM_PLAYER, NM_PLAYER_LIST_RES, player->GetConnectionIdx() );
		}

		perGrid = perGrid->next;
	}

	mPlayerMtr[ gridPos ] += 1;
}

// SendSightOutPlayer Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutPlayer(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		cPlayer* other = (cPlayer*)perGrid->baseObject;

		// FROM PLAYER TO 그리드(OTHER PLAYER).
		if ( !player->GetCheatHideMode( ) )
			player->SendSightOut( NM_PLAYER, NM_PLAYER_DISCONNECT_SYN, other->GetConnectionIdx() );

		// FROM 그리드(OTHER PLAYER) TO PLAYER.
		if ( !other->GetCheatHideMode( ) )
			other->SendSightOut( NM_PLAYER, NM_PLAYER_DISCONNECT_SYN, player->GetConnectionIdx() );

		perGrid = perGrid->next;
	}

	if ( mPlayerMtr[ gridPos ] > 0 )
		mPlayerMtr[ gridPos ] -= 1;
	else
		NETWORK2->PostServerEvent( "cGridManager::SendPlayerGameOut(=%d -1)", mPlayerMtr[ gridPos ] );
}

// SendSightInMonster Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInMonster(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mMonsterPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cMonster* monster = (cMonster*)perGrid->baseObject;

		// FROM 그리드(MONSTER) TO PLAYER.
		monster->SendSightIn( NM_MONSTER, NM_MONSTER_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightOutMonster Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutMonster(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mMonsterPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cMonster* monster = (cMonster*)perGrid->baseObject;

		// FROM 그리드(MONSTER) TO PLAYER.
		monster->SendSightOut( NM_MONSTER, NM_MONSTER_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightInNpc Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInNpc(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mNpcPool->mPagedPoolUsage[ gridPos ];
	while ( perGrid )
	{
		cNpc* npc = (cNpc*)perGrid->baseObject;

		// FROM 그리드(NPC) TO PLAYER.
		npc->SendSightIn( NM_NPC, NM_NPC_SIGHT_IN_RES, player );

		perGrid = perGrid->next;
	}
}

// SendSightOutNpc Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutNpc(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mNpcPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cNpc* npc = (cNpc*)perGrid->baseObject;

		// FROM 그리드(NPC) TO PLAYER.
		npc->SendSightOut( NM_NPC, NM_NPC_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightInItem Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInItem(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mItemPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cItem* item = (cItem*)perGrid->baseObject;

		// FROM 그리드(ITEM) TO PLAYER.
		item->SendSightIn( NM_ITEM, NM_ITEM_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightOutItem Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutItem(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mItemPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cItem* item = (cItem*)perGrid->baseObject;

		// FROM 그리드(ITEM) TO PLAYER.
		item->SendSightOut( NM_ITEM, NM_ITEM_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightInTotem Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInTotem(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mTotemPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cTotem* totem = (cTotem*)perGrid->baseObject;

		// FROM 그리드(TOTEM) TO PLAYER.
		totem->SendSightIn( NM_TOTEM, NM_TOTEM_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightOutTotem Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutTotem(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mTotemPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cTotem* totem = (cTotem*)perGrid->baseObject;

		// FROM 그리드(TOTEM) TO PLAYER.
		totem->SendSightOut( NM_TOTEM, NM_TOTEM_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendMonsterSightIn Method - 몬스터 이동에 따른 그리드 변경내용.
void cGridManager::SendMonsterSightIn(cMonster* monster, int gridPos)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		// FROM 몬스터 TO 그리드(PLAYER).
		cPlayer* player = (cPlayer*)perGrid->baseObject;
		long     cid    = player->GetConnectionIdx();

		monster->SendSightIn( NM_MONSTER, NM_MONSTER_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendMonsterSightOut Method - 몬스터 이동에 따른 그리드 변경내용.
void cGridManager::SendMonsterSightOut(cMonster* monster, int gridPos)
{
	PerGrid* perGrid = mPlayerPool->mPagedPoolUsage[ gridPos ];

	while ( perGrid )
	{
		// FROM 몬스터 TO 그리드(PLAYER).
		cPlayer* player = (cPlayer*)perGrid->baseObject;

		monster->SendSightOut( NM_MONSTER, NM_MONSTER_SIGHT_OUT_RES, player->GetConnectionIdx() );

		perGrid = perGrid->next;
	}
}

// SendSightInGathering Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInGathering(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mGatheringPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cGathering* gathering = (cGathering*)perGrid->baseObject;

		// FROM 그리드(Gathering) TO PLAYER.
		gathering->SendSightIn( NM_GATHERING, NM_GATHERING_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightOutGathering Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutGathering(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mGatheringPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cGathering* gathering = (cGathering*)perGrid->baseObject;

		// FROM 그리드(Gathering) TO PLAYER.
		gathering->SendSightOut( NM_GATHERING, NM_GATHERING_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightInUserPortal Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightInUserPortal(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mUserPortalPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cUserPortal* pUserPortal = (cUserPortal*)perGrid->baseObject;

		// FROM 그리드(UserPortal) TO PLAYER.
		pUserPortal->SendSightIn( NM_USERPORTAL, NM_USERPORTAL_SIGHT_IN_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendSightOutGathering Method - 플레이어 이동에 따른 그리드 변경내용.
void cGridManager::SendSightOutUserPortal(cPlayer* player, int gridPos)
{
	PerGrid* perGrid = mUserPortalPool->mPagedPoolUsage[ gridPos ];
	long     cid     = player->GetConnectionIdx( );
	while ( perGrid )
	{
		cUserPortal* pUserPortal = (cUserPortal*)perGrid->baseObject;

		// FROM 그리드(UserPortal) TO PLAYER.
		pUserPortal->SendSightOut( NM_USERPORTAL, NM_USERPORTAL_SIGHT_OUT_RES, cid );

		perGrid = perGrid->next;
	}
}

// SendNpcList Method.
bool cGridManager::SendNpcList(cPlayer* player)
{
	HANDLE        handle  = NULL;
	MSG_NPC_INFO* sendMsg = (MSG_NPC_INFO*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ) );
	if ( sendMsg != NULL )
	{
		unsigned long& count   = sendMsg->mCount;
		sNpcData*      npcData = sendMsg->mNpcInfo;

		sendMsg->Category = NM_NPC;
		sendMsg->Protocol = NM_NPC_LIST_RES;
		sendMsg->mCount   = 0;

		int beginPos = mNpcPool->CalcPos( player->GetMapNumber( ) );
		int endPos   = beginPos + MAP_SIZE;

		while ( beginPos < endPos && beginPos < mMaxGrid )
		{
			PerGrid* temp = mNpcPool->mPagedPoolUsage[ beginPos ];
			cNpc*    npc;
			while ( temp != NULL )
			{
				npc = (cNpc*)temp->baseObject;

					npc->GetNpcData( player, npcData );
					npcData++;
					count++;

				temp = temp->next;
			}
			beginPos++;
		}

		return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	return false;
}
