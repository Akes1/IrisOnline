//
//
//

#include "stdafx.h"
#include "Pathfinder.h"
#include "PathNodeSet.h"


cPathNodeSet::cPathNodeSet()
{
	mPointArray = NULL;
	mCurrentUseIndex = 0;
	mpPathfinder = NULL;
	mTotalPathCount = 0;
	
}

cPathNodeSet::~cPathNodeSet()
{
	MemoryDestory();
	
}

void cPathNodeSet::MemoryDestory()
{
	mPosvectorlist.Clear();

	if(mPointArray)
	{
		delete[] mPointArray; mPointArray = NULL;
	}
}

bool cPathNodeSet::Create(cPathfinder* pathfinder)
{
	MemoryDestory();
	
	mpPathfinder = pathfinder;
	
	
	mPosvectorlist.Clear();
	mPointArray = new POINT[31];

	return true;
}


void cPathNodeSet::RayPath( float destx, float desty )
{

	mCurrentUseIndex = 0;
	mTotalPathCount = 1;

	mPosvectorlist.Clear();

	Point2 pos = {destx,desty};
	mPosvectorlist.PushBack(pos);
	//mPosvectorlist[0].x = destx;
	//mPosvectorlist[0].y = desty;
	//mPosvectorlist[1] = NiPoint2(destx,desty);

}

/* ------------------------------------------------------------------
* 함수이름 :	ResetPath
* 목    적 :    경로를 다시 얻어내는 함수(전체경로)
* 주의사항 :    
* ------------------------------------------------------------------ */

void cPathNodeSet::ResetPath(float startx,float starty,float destx,float desty)
{
	mCurrentUseIndex = 0;
	mTotalPathCount = 0;

	float onetilesize = mpPathfinder->GetOneTileSize();

	startx	= startx/onetilesize;
	starty	= starty/onetilesize;
	destx	= destx/onetilesize;
	desty	= desty/onetilesize;

	if(startx < 0)
	{
		assert(0);
		return;
	}
	if(starty < 0)
	{
		assert(0);
		return;
	}
	if(destx < 0)
	{
		assert(0);
		return;
	}
	if(desty < 0)
	{
		assert(0);
		return;
	}

	mpPathfinder->DoRefind((unsigned int)startx,(unsigned int)starty,(unsigned int)destx,(unsigned int)desty);
	
	if(mpPathfinder->GetLastResult() == PATH_BLOCK)
	{
		return;
	}
	
	mTotalPathCount = mpPathfinder->FindUpdate(0);

	if(mTotalPathCount > 0)
	{
		mpPathfinder->CopyPathArray(mPointArray,31,mTotalPathCount,false);

		MakeRealPosition();
	}
}

/* ------------------------------------------------------------------
* 함수이름 :	GetPathArray
* 목    적 :    경로의 배열인덱스을 얻어내는 함수
* 주의사항 :    
* ------------------------------------------------------------------ */

void cPathNodeSet::GetPathArray(POINT* array , const unsigned int maxcopycount, unsigned int& copycount)
{
	mpPathfinder->CopyPathArray(array, maxcopycount, copycount, true);

	for(unsigned int i = 0; i < copycount; ++i)
	{
		mPointArray[i] = array[i];
	}

	MakeRealPosition();
}

/* ------------------------------------------------------------------
* 함수이름 :	InsertPathArray
* 목    적 :    경로의 배열인덱스를 셋팅해주는 함수
* 주의사항 :    
* ------------------------------------------------------------------ */

void cPathNodeSet::SetPathArray(POINT* array, const unsigned int count)
{
	mCurrentUseIndex = 0;

	mTotalPathCount = count;

	for(unsigned int i = 0; i < count; ++i)
	{
		mPointArray[i] = array[i];
	}

	MakeRealPosition();
}

/* ------------------------------------------------------------------
* 함수이름 :	MakeRealPosition
* 목    적 :    사실 이 함수는 최종 결정된 인덱스를 실제 맵에서 사용하는 
				위치로 변형하여 스텍에 담는 역할을 한다.
* 주의사항 :    
* ------------------------------------------------------------------ */

void cPathNodeSet::MakeRealPosition()
{
	float onetilesize = mpPathfinder->GetOneTileSize();
	float halfsize = onetilesize/2;

	Point2 element;

	for(unsigned int i = 0; i < mTotalPathCount; ++i)
	{
		element.x = (float)(mPointArray[i].x*onetilesize + halfsize);
		element.y = (float)(mPointArray[i].y*onetilesize + halfsize);

		mPosvectorlist.PushBack(element);
	}
}


/* ------------------------------------------------------------------
* 함수이름 :	DetailPathMake
* 목    적 :    스텍에 담긴 위치값을 이용하여 방향이 심하게 틀어질 경우 그 중간에 
노드를 삽입하여 좀 더 부드럽게 보이도록 하기 위해

* 주의사항 :    
* ------------------------------------------------------------------ */
/*
void cPathObserver::DetailPathMake(cObstacle* pObstacle)
{
///저장된 목록에서 새로운 목록을 추가하며 옮겨 닮는다.
int count = mStack.GetCount();

D3DXVECTOR3 FirstVec;
D3DXVECTOR3 NextVec;

///추후에 이 노드의 개수가 일정수 이상이 넘어가면
///더이상 연산을 하지 않는 코드가 추가되야 한다.
int finallistcount = 0;

///처음점 기본 삽입
mPosvectorlist[0] = mTempvectorlist[0];
finallistcount++;

for(int i = 1; i < (count-1); ++i)
{
if(mPosvectorlist[finallistcount-1] == mPosvectorlist[finallistcount-2] && finallistcount > 1)
{
finallistcount --;
}

FirstVec = mTempvectorlist[i] - mTempvectorlist[i-1];
NextVec = mTempvectorlist[i+1] - mTempvectorlist[i];

///앞뒤 벡터의 각이 45도보다 크면 중간에 값을 삽입한다.

float f = D3DXVec3Dot(D3DXVec3Normalize(&FirstVec,&FirstVec),
D3DXVec3Normalize(&NextVec,&NextVec));

if( f >= 0.25f && f <= 0.75f)
{
mPosvectorlist[finallistcount] = mTempvectorlist[i];
finallistcount++;

if(mPosvectorlist[finallistcount-1] == mPosvectorlist[finallistcount-2] && finallistcount > 1)
{
finallistcount --;
}

sElement element1 = mStack.GetElementArray()[i];

sElement element2 = mStack.GetElementArray()[i+1];

///두지점이 교차하는 지점의로의 벡터

int x = element2.Xindex;
int y = element1.Yindex;

if(this->IsObstacle(x,y) == 1)
{
x = element1.Xindex;
y = element2.Yindex;
}

D3DXVECTOR3 dir = pObstacle->GetIndexCenterPos(x,y) - mTempvectorlist[i];

mPosvectorlist[finallistcount] = mTempvectorlist[i] + dir*0.5f;
finallistcount++;

if(mPosvectorlist[finallistcount-1] == mPosvectorlist[finallistcount-2] && finallistcount > 1)
{
finallistcount --;
}

dir = pObstacle->GetIndexCenterPos(x,y) - mTempvectorlist[i+1];

mPosvectorlist[finallistcount] = mTempvectorlist[i+1] + dir*0.5f;
finallistcount++;

if(mPosvectorlist[finallistcount-1] == mPosvectorlist[finallistcount-2] && finallistcount > 1)
{
finallistcount --;
}
}
else
{
mPosvectorlist[finallistcount] = mTempvectorlist[i];

finallistcount++;

if(mPosvectorlist[finallistcount-1] == mPosvectorlist[finallistcount-2] && finallistcount > 1)
{
finallistcount --;
}
}
}

mPosvectorlist[finallistcount] = mTempvectorlist[count-1];
finallistcount++;

mFinalCountIndex = finallistcount;

}
*/

NiPoint2 cPathNodeSet::GetCurrentPathNode(const unsigned int& curnode)
{
	NiPoint2 pos;

	pos.x = mPosvectorlist[curnode].x;
	pos.y = mPosvectorlist[curnode].y;

	return pos;
}

/* ------------------------------------------------------------------
* 함수이름 :	GetNext
* 목    적 :    외부로 노출되는 인테페이스 중에 가장 빈도수가 높은 함수
최종적으로 스텍에 담겨진 내용을 밖으로 하나하나 내보낸다.

* 주의사항 :    
* ------------------------------------------------------------------ */
NiPoint2 cPathNodeSet::GetNextPath()
{
	NiPoint2 p; p.x = mPosvectorlist[mCurrentUseIndex].x; p.y = mPosvectorlist[mCurrentUseIndex].y;

	mCurrentUseIndex++;

	return p;
}

/* ------------------------------------------------------------------
* 함수이름 :	IsFinalPath
* 목    적 :    마지막 경로가 밖으로 나갔는지 체크하는 함수

* 주의사항 :    
* ------------------------------------------------------------------ */

bool cPathNodeSet::IsFinalPath()
{
	if(mTotalPathCount == 0) return true;
	
	else if(mCurrentUseIndex >= mTotalPathCount) return true;

	return false;
}
