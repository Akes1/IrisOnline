#include "StdAfx.h"
#include "StageScript.h"

#include "./util/FileLoader.h"
#include "./util/Tokenizer.h"

//#include "Player.h"

#include "./client/ResourceManager.h"


cStageScript* cStageScript::mpStageScript = NULL;

#ifndef _CLIENT
#define D3DX_PI    ((FLOAT)  3.141592654f)
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#endif


cStageScript::cStageScript()
{
	/// 싱글톤
	if( !mpStageScript )
	{
		mpStageScript = this;
	}
}

cStageScript::~cStageScript()
{
	Release();
}

bool cStageScript::Init()
{
	Release();

	/// 맵이동 스크립트 로드
	if( !StageChangePosScriptLoad() )
	{
		assert(NULL);
		return false;
	}

	LoadManagedDoorInfo();

#ifndef _CLIENT
	if( !LoadMapInfo() )
	{
		assert(0);
		return false;
	}
#endif

	return true;
}


void cStageScript::Release()
{
	mReturnArray.Clear();

	cPointerHashMap::cIterator iter;
	if( mStageChangePosMap.IsEmpty() == false )
	{
		for(iter = mStageChangePosMap.Begin(); iter != mStageChangePosMap.End(); ++iter)
		{
			delete (sStageChangePos*)(*iter).mSecond;
		}
		mStageChangePosMap.Clear();
	}

	if( mStageDoorMap.IsEmpty() == false )
	{
		for(iter = mStageDoorMap.Begin(); iter != mStageDoorMap.End(); ++iter)
		{
			delete (sDoorInfo*)(*iter).mSecond;
		}
		mStageDoorMap.Clear();
	}

#ifndef _CLIENT
	cPointerHashMap::cIterator i = mMapInfoMap.Begin();
	cPointerHashMap::cIterator end = mMapInfoMap.End();
	i = mMapInfoMap.Begin();
	end = mMapInfoMap.End();
	for( ; i != end; ++i )
	{
		cPointerHashMap* pMap = (cPointerHashMap*)(*i).mSecond;
		if( pMap != NULL )
		{
			cPointerHashMap::cIterator d;

			for( d = pMap->Begin() ; d != pMap->End() ; ++d )
			{
				sMapInfo* p = (sMapInfo*)(*d).mSecond;
				SAFE_DELETE(p);
			}
			pMap->Clear();
			SAFE_DELETE( pMap );
		}		
	}
	mMapInfoMap.Clear();
#endif
}

bool cStageScript::StageChangePosScriptLoad()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/MapChangePos.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapChangePos.txt" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();
	sStageChangePos* pChangePos = 0;

	while( tokenizer.IsEnd() == false )
	{
		pChangePos = new sStageChangePos;

		/// 고유번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosIdx = str.ToInt();

		///
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mFileName = str;

		/// 맵번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mMapNumber = (unsigned char)str.ToInt();

		/// 좌표X
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosX = str.ToFloat();

		/// 좌표Y
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mPosY = str.ToFloat();

		/// 범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mRange = str.ToFloat();

		/// 목적지맵idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mTargetPosIdx = str.ToInt();

		/// 회전 각도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mRotAngle = S_ToRadian( str.ToFloat() );

		/// 자동생성 유무
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mFirstRegen = str.ToInt() == 1;

		/// 포탈명 출력 높이값
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pChangePos->mNameHeight = str.ToFloat();

		/// 보관소 저장
		if( mStageChangePosMap.Insert( pChangePos->mPosIdx, pChangePos ) == false )
		{
			assert(0);
			goto ERR;
		}
		else
		{
			// Resource 로딩 추가. [12/21/2009 Jo_Client]
			cString path;
			path.Format("./data/effect/%s", pChangePos->mFileName.Cstr() );
			if( RESOURCEMAN->LoadMapNIF( path.Cstr() ) == false )
			{
				cString msg;
				msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
				MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
			}
		}
	}

	begin = mStageChangePosMap.Begin();
	while( begin != mStageChangePosMap.End() )
	{
		pChangePos = (sStageChangePos*)((*begin).mSecond);
		++begin;

		if( pChangePos == 0 )
		{
			assert(0);
			goto ERR;
		}

		pChangePos->mTargetInfo = (sStageChangePos*)mStageChangePosMap.GetAt( pChangePos->mTargetPosIdx );
		
		if( pChangePos->mTargetInfo != NULL )
		{
			/// 텔레포트포탈로 생성되는데 맵번호가틀리면 이동좌표가 틀어짐
			if( pChangePos->mFirstRegen == false )
			{
				if( pChangePos->mMapNumber != pChangePos->mTargetInfo->mMapNumber )
				{
					assert(0);
					goto ERR;
				}
			}
		}
	}

	return true;

ERR:
	delete pChangePos;
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}



unsigned long cStageScript::GetStageChangeIndex( float posX, float posY )
{
	NiPoint2 characterPos( posX,  posY );
	NiPoint2 teleportStartPos = NiPoint2::ZERO;

	sStageChangePos* pChangePos = 0;
	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();
	while( begin != mStageChangePosMap.End() )
	{
		pChangePos = (sStageChangePos*)(*begin).mSecond;
		unsigned long posIndex = (*begin).mFirst;
		++begin;

		if( pChangePos == 0 )
			continue;

		/// 검사 기준에서 목적지 제외
		if( pChangePos->mTargetInfo == 0 )
			continue;

		teleportStartPos.x = pChangePos->mPosX;
		teleportStartPos.y = pChangePos->mPosY;

//		mRangeCheck.SetRadius( pChangePos->mRange );

		/// 입력좌표가 스크립트좌표 허용범위 안에 들어온경우 스크립트idx를 리턴
//		if( mRangeCheck.IsRange( teleportStartPos, characterPos ) )
// 		{
// 			return posIndex;
// 		}
	}
	return 0;
}


sTargetPos cStageScript::GetStageChangePos( float posX, float posY, unsigned short mapNum, float objectSize, unsigned long posIdx )
{
	sTargetPos targetPos;
	targetPos.mMapNumber = 0;
	targetPos.mPosX = 0.0f;
	targetPos.mPosY = 0.0f;

	NiPoint2 characterPos( posX, posY );

	/// 스크립트상의 시작좌표
	sStageChangePos* pChangePos = (sStageChangePos*)mStageChangePosMap.GetAt( posIdx );
	if( pChangePos == 0 )
	{
		assert(0);
		return targetPos;
	}

	NiPoint2 teleportStartPos( pChangePos->mPosX, pChangePos->mPosY );

	/// 플레이어 현재 맵번호 확인
	if( pChangePos->mMapNumber != mapNum )
	{
		assert(0);
		return targetPos;
	}

	/// 텔레포트포탈 - 생성시 안보이는 포탈이면 이동불가
	if( pChangePos->mFirstRegen == false )
		return targetPos;

	/// 시작좌표와 플레이어좌표를 확인
	/// 이동시 좌표위치 허용범위 더함
// 	mRangeCheck.SetRadius( pChangePos->mRange + objectSize + SYNC_MOVE_RANGE );
// 	if( mRangeCheck.IsNotRange( teleportStartPos, characterPos ) )
// 	{
// 		return targetPos;
// 	}

	/// 랜덤 위치
	sStageChangePos* pTargetChangePos = pChangePos->mTargetInfo;
	if( pTargetChangePos == 0 )
		return targetPos;

	int range = (int) pTargetChangePos->mRange;
	if( range == 0 ) range = 1;
	/// 좌표가 맞는경우 목적지좌표를 리턴
	targetPos.mMapNumber = pTargetChangePos->mMapNumber;
	targetPos.mPosX = pTargetChangePos->mPosX + ( rand() % range ) - ( rand() % range );
	targetPos.mPosY = pTargetChangePos->mPosY + ( rand() % range ) - ( rand() % range );
	targetPos.mRotAngle = pTargetChangePos->mRotAngle;

	return targetPos;
}


sTargetPos cStageScript::GetStageChangePos( unsigned long posIdx )
{
	sTargetPos targetPos;
	targetPos.mMapNumber = 0;
	targetPos.mPosX = 0.0f;
	targetPos.mPosY = 0.0f;

	/// 스크립트상의 시작좌표
	sStageChangePos* pChangePos = (sStageChangePos*)mStageChangePosMap.GetAt( posIdx );
	if( pChangePos == NULL )
	{
		assert(NULL);
		return targetPos;
	}

	NiPoint2 teleportStartPos( pChangePos->mPosX, pChangePos->mPosY );

	/// 랜덤 위치
	int range = (int)pChangePos->mRange;
	if( range == 0 ) range = 1;
	/// 좌표가 맞는경우 목적지좌표를 리턴
	targetPos.mMapNumber = pChangePos->mMapNumber;
	targetPos.mPosX = pChangePos->mPosX + ( rand() % range ) - ( rand() % range );
	targetPos.mPosY = pChangePos->mPosY + ( rand() % range ) - ( rand() % range );
	targetPos.mRotAngle = pChangePos->mRotAngle;

	return targetPos;
}


NiPoint2 cStageScript::CalcNearTargetMapPos( unsigned short mapNum, float posX, float posY )
{
	NiPoint2 heroPos( posX, posY );
	NiPoint2 nearPos( posX, posY );
	unsigned long nearRange = ULONG_MAX;

	unsigned long range;
	NiPoint2 targetPos;
	unsigned long randRange = 0;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();
	while( begin != mStageChangePosMap.End() )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*begin++).mSecond;
		if( pChangePos == 0 )
		{
			assert(0);
			continue;
		}
		if( pChangePos->mMapNumber != mapNum )
			continue;

		/// 검사 기준에서 출발지 제외
		if( pChangePos->mTargetInfo != 0 )
			continue;

		targetPos.x = pChangePos->mPosX;
		targetPos.y = pChangePos->mPosY;
		randRange = (unsigned long)pChangePos->mRange;

		range = (unsigned long)(( heroPos - targetPos ).Length());
		if( range < nearRange )
		{
			nearRange = range;
			nearPos = targetPos;     
		}
	}

	if( randRange != 0 )
	{
		nearPos.x = nearPos.x + ( rand() % randRange ) - ( rand() % randRange );
		nearPos.y = nearPos.y + ( rand() % randRange ) - ( rand() % randRange );
	}

	return nearPos;
}


bool cStageScript::CalcTargetMapPos( unsigned long posIdx, float *pX, float *pY )
{
	/// 스크립트상의 시작좌표
	sStageChangePos* pChagePos = (sStageChangePos*)mStageChangePosMap.GetAt( posIdx );
	if( pChagePos == 0 )
		return false;

	if( pX == 0 || pY == 0 )
		return false;

	unsigned long range = (unsigned long)pChagePos->mRange;
    if( range == 0 ) range = 1;

	*pX = pChagePos->mPosX + ( rand() % range ) - ( rand() % range );
	*pY = pChagePos->mPosY + ( rand() % range ) - ( rand() % range );

	return true;
}

///
tPointerArray<void*>* cStageScript::GetMapChangeStarterArr( unsigned short mapNum )
{
	mReturnArray.Clear();

	if( mStageChangePosMap.IsEmpty() )
		return &mReturnArray;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();

	while( begin != mStageChangePosMap.End() )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*begin++).mSecond;
		if( pChangePos == 0 )
		{
			assert(0);
			continue;
		}

		if( pChangePos->mMapNumber != mapNum )
			continue;

		/// 검사 기준에서 목적지 제외
		if( pChangePos->mTargetInfo == 0 )
			continue;

		mReturnArray.PushBack( pChangePos );
	}

	return &mReturnArray;
}

///
tPointerArray<void*>* cStageScript::GetMapChangeDestArr( unsigned short mapNum )
{
	mReturnArray.Clear();

	if( mStageChangePosMap.IsEmpty() )
		return &mReturnArray;

	cPointerHashMap::cIterator begin = mStageChangePosMap.Begin();

	while( begin != mStageChangePosMap.End() )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*begin++).mSecond;
		if( pChangePos == 0 )
		{
			assert(0);
			continue;
		}

		if( pChangePos->mMapNumber != mapNum )
			continue;

		/// 검사 기준에서 출발지 제외
		if( pChangePos->mTargetInfo != 0 )
			continue;

		mReturnArray.PushBack( pChangePos );
	}

	return &mReturnArray;
}

sStageChangePos* cStageScript::GetPosScriptInfo( unsigned long idx )
{
	return (sStageChangePos*)mStageChangePosMap.GetAt( idx );
}

bool cStageScript::LoadManagedDoorInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/doorposlist.txt";

	if( loader.Open( pathName, true ) == false )
	{
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sDoorInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sDoorInfo;

		/// index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mIndex = str.ToInt();

		/// map number
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mMapNumber = (unsigned short)str.ToInt();

		/// file name
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mFileName = str;

		/// door pos x
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mPosX = str.ToFloat();

		/// door pos y
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mPosY = str.ToFloat();

		/// door pos appendZ
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mAppenZ = str.ToFloat();

		/// door rot
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mRotAngle = S_ToRadian(str.ToFloat());

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// rect append skip

		/// rectInfo
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[1].x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[1].y = str.ToFloat();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[2].x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[2].y = str.ToFloat();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[3].x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[3].y = str.ToFloat();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[0].x = str.ToFloat();
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mRectPos[0].y = str.ToFloat();

		if( mStageDoorMap.Insert( pInfo->mIndex, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pInfo;
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}

tPointerArray<void*>* cStageScript::GetManagedDoorArr( unsigned short mapNum )
{
	mReturnArray.Clear();

	if( mStageDoorMap.IsEmpty() )
		return &mReturnArray;

	cPointerHashMap::cIterator begin = mStageDoorMap.Begin();

	while( begin != mStageDoorMap.End() )
	{
		sDoorInfo* pInfo = (sDoorInfo*)(*begin++).mSecond;
		if( pInfo == 0 )
		{
			assert(0);
			continue;
		}

		if( pInfo->mMapNumber != mapNum )
			continue;

		mReturnArray.PushBack( pInfo );
	}

	return &mReturnArray;
}

sDoorInfo* cStageScript::GetManagedDoorInfo( unsigned long index )
{
	return (sDoorInfo*)mStageDoorMap.GetAt( index );
}

#ifndef _CLIENT
bool cStageScript::LoadMapInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/MapData.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapData.txt" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMapInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sMapInfo;

		// Map number
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapNum = str.ToInt();

		if( pInfo->mapNum > 999 )	{ goto ERR; }

		cPointerHashMap* pModeMap = (cPointerHashMap*)mMapInfoMap.GetAt( pInfo->mapNum );
		if( pModeMap == NULL )
		{
			pModeMap = new cPointerHashMap;
			mMapInfoMap.Insert( pInfo->mapNum, pModeMap );
		}

		/// Map 폴더번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->folderIdx = str.ToInt();

		/// 한맵에 여러개의 폴더 인덱스를 가질수 없다.
		if( pModeMap->GetSize() != 0 )
		{
			cPointerHashMap::cIterator b = pModeMap->Begin();
			sMapInfo* pMapInfo = (sMapInfo*)(*b).mSecond;
			if( pMapInfo == NULL )
				goto ERR;

			if( pMapInfo->folderIdx != pInfo->folderIdx )
				goto ERR;			
		}

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 난이도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapMode = (unsigned char)str.ToInt();

		/// 일반맵에 다른 모드를 사용할 수 없다.
		if( MAP_MIN <= pInfo->mapNum && pInfo->mapNum <= MAP_MAX && pInfo->mapMode != DEFAULT_MAPMODE )
			goto ERR;

		/// 최소레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->minLevel = (unsigned char)str.ToInt();
		/// 최대레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxLevel = (unsigned char)str.ToInt();
		/// 최대인원
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxPlayer = (unsigned char)str.ToInt();
		/// 필요아이템
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->itemIdx = str.ToInt();
		
		if( pModeMap->Insert( pInfo->mapMode, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pInfo;
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}

sMapInfo* cStageScript::GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode )
{
	cPointerHashMap* pModeMap = (cPointerHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pModeMap == NULL )
		return NULL;

	sMapInfo* pMapData = (sMapInfo*)pModeMap->GetAt( mapMode );
	if( pMapData == NULL )
		return NULL;

	return pMapData;
}

void cStageScript::GetModeSet( unsigned short mapType, void* pModeSet )
{
	if( pModeSet == NULL )
		return;

	cSet* pSet = (cSet*)pModeSet;

	if( mapType == MAP_NONE )
	{

		cPointerHashMap::cIterator hB = mMapInfoMap.Begin();
		cPointerHashMap::cIterator hE = mMapInfoMap.End();

		for( ; hB != hE ; ++hB )
		{
			cPointerHashMap* pModeMap = (cPointerHashMap*)( (*hB).mSecond );
			if( pModeMap == NULL )
				return;

			cPointerHashMap::cIterator b = pModeMap->Begin();
			cPointerHashMap::cIterator e = pModeMap->End();
			for( ; b != e ; ++b )
			{
				sMapInfo* pMapData = (sMapInfo*)(*b).mSecond;
				if( pMapData == NULL )
					continue;

				if( MAP_MIN <= pMapData->mapNum && pMapData->mapNum <= MAP_MAX )
					pSet->Insert( pMapData->mapMode );
			}
		}
	}
	else
	{
		cPointerHashMap* pModeMap = (cPointerHashMap*)mMapInfoMap.GetAt( mapType );
		if( pModeMap == NULL )
			return;

		cPointerHashMap::cIterator b = pModeMap->Begin();
		cPointerHashMap::cIterator e = pModeMap->End();
		for( ; b != e ; ++b )
		{
			sMapInfo* pMapData = (sMapInfo*)(*b).mSecond;
			if( pMapData == NULL )
				continue;

			pSet->Insert( pMapData->mapMode );
		}
	}
}

#endif

unsigned int cStageScript::InsidePolygon( NiPoint2 *polygon, unsigned int numVerts, NiPoint2 p )
{
	/// 주의 : 경계선 미포함
	/// 폴리곤은 시계방향 순으로 넘어와야 한다.

	unsigned int N = numVerts;
	unsigned int counter = 0;
	float xinters;
	NiPoint2 p1,p2;

	p1 = polygon[0];
	for( unsigned int i=1; i<=N; i++ ) 
	{
		p2 = polygon[i % N];
		if( p.y > min(p1.y,p2.y) )  
		{
			if( p.y <= max(p1.y,p2.y) ) 
			{
				if( p.x <= max(p1.x,p2.x) ) 
				{
					if( p1.y != p2.y )
					{
						xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
						if( p1.x == p2.x || p.x <= xinters )
							counter++;
					}
				}
			}
		}
		p1 = p2;
	}

	if( counter % 2 == 0 )
		return(1);	/// outside
	else
		return(0);	/// inside
}

bool cStageScript::LineIntersectLine( NiPoint2 start, NiPoint2 goal, NiPoint2 lineStart, NiPoint2 lineEnd )
{
	float denom = ((lineEnd.y-lineStart.y) * (goal.x - start.x)) - ((lineEnd.x-lineStart.x) * (goal.y - start.y));

	float numerator1 = ((lineEnd.x-lineStart.x) * (start.y-lineStart.y)) - ((lineEnd.y-lineStart.y) * (start.x-lineStart.x));
	float numerator2 = ((goal.x-start.x) * (start.y-lineStart.y)) - ((goal.y-start.y) * (start.x-lineStart.x));

	if( denom == 0.0f )
	{
		if( numerator1 == 0.0f && numerator2 == 0.0f )
			return false;
		return false;
	}

	float ua = numerator1 / denom;
	float ub = numerator2 / denom;

	return ( ua >= 0.0f && ua <= 1.0f && ub >=0.0f && ub <=1.0f );
}

bool cStageScript::LineIntersectRect( NiPoint2* rect, NiPoint2 start, NiPoint2 goal )
{
	//	사각형 rect는 아래순으로 포함해야 한다.
	//	0------1
	//  |	   |
	//	|	   |
	//	3------2	
	if( InsidePolygon( rect, 4, start ) == 0 )
		return true;
	if( InsidePolygon( rect, 4, goal ) == 0 ) 
		return true;

	/// bottom
	if( LineIntersectLine( start, goal, rect[3], rect[2] ) ) return true;
	/// top
	if( LineIntersectLine( start, goal, rect[0], rect[1] ) ) return true;
	/// right
	if( LineIntersectLine( start, goal, rect[1], rect[2] ) ) return true;
	/// left
	if( LineIntersectLine( start, goal, rect[0], rect[3] ) ) return true;

	return false;
}

sDoorInfo* cStageScript::CrashDoor( unsigned long doorIdx, NiPoint2 start, NiPoint2 goal )
{
	sDoorInfo* pInfo = GetManagedDoorInfo( doorIdx );
	if( pInfo == 0 )
		return 0;

	if( LineIntersectRect( pInfo->mRectPos, start, goal ) == true )
		return pInfo;

	return 0;
}
