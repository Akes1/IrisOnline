#include "StdAfx.h"
#include "PathFinder.h"

#include "NaviField.h"
#include <math.h>


#pragma warning( disable: 4258 )
#pragma warning( disable: 4288 )

static
const unsigned short gCostTable[8] =
{
	10, 10, 10, 10, 14, 14, 14, 14
};

static inline
float GetPosOnCatmullRomSpline( float* cp, float t )
{
	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * (
		( 2.0f*cp[1] ) +
		( -cp[0] + cp[2] ) * t +
		( 2.0f*cp[0] - 5.0f*cp[1] + 4.0f*cp[2] - cp[3] ) * t2 +
		( -cp[0] + 3.0f*cp[1]- 3.0f*cp[2] + cp[3] ) * t3 );
}

//////////////////////////////////////////////////////////////////////////
cPathNode::cPathNode()
{
	/// 부모
	mParent = 0;

	for( unsigned int i=0; i<8; i++ )
		mChild[i] = 0;

	mNext = 0;
}

//////////////////////////////////////////////////////////////////////////
cPathFinder::cPathFinder( unsigned int capacity )
: mCellCount( 0 )
, mLineCount( 0 )
, mCells( 0 )
, mNodes( 0 )
, mOpenQueue( capacity )
, mGoalX( 0 )
, mGoalY( 0 )
{
}

cPathFinder::~cPathFinder()
{
	Clear();
}

void cPathFinder::Clear()
{
	if( mCells )
	{
		delete [] mCells;
		mCells = 0;
	}

	if( mNodes )
	{
		for( unsigned int i = 0, iend = mLineCount * mLineCount; i < iend; ++i )
		{
			delete mNodes[i];
		}
		delete [] mNodes;
		mNodes = 0;
	}
}

void cPathFinder::Init( const cNaviField* naviField )
{
	assert( naviField );

	Clear();

	/// 그리드 크기를 검사
	unsigned int cellCount = naviField->GetCellCount();

	if( CheckCellCount( cellCount ) == false )
	{
		assert( 0 && "invalid navifield grid size" );
		return;
	}

	/// 그리드 크기 및 단위 설정
	mCellCount = cellCount;
	mLineCount = mCellCount + 1;

	/// 노드 배열 생성 및 초기화
	unsigned int numVerts = mLineCount * mLineCount;
	mNodes = new cPathNode*[numVerts];
	mCells = new cPathNode**[mLineCount];
	::memset( mNodes, 0, sizeof(cPathNode*) * numVerts );

	for( unsigned int xi = 1; xi < mCellCount; ++xi )
	{
		for( unsigned int yi = 1; yi < mCellCount; ++yi )
		{
			if( naviField->GetValue( xi, yi ) == 0 )
			{
				mNodes[yi * mLineCount + xi] = new cPathNode;
			}
		}
	}
	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mCells[i] = &mNodes[i * mLineCount];
	}
	for( unsigned int xi = 1; xi < mCellCount; ++xi )
	{
		for( unsigned int yi = 1; yi < mCellCount; ++yi )
		{
			cPathNode* n = mCells[yi][xi];

			if( n )
			{
				n->mXIndex = (unsigned short)xi;
				n->mYIndex = (unsigned short)yi;
				n->mParent = 0;
				n->mTotalCost = 0;
				n->mCostFromStart = 0;
				n->mCostToGoal = 0;
				n->mClosed = false;

				n->mChild[0] = mCells[yi-1][xi  ];
				n->mChild[1] = mCells[yi  ][xi+1];
				n->mChild[2] = mCells[yi+1][xi  ];
				n->mChild[3] = mCells[yi  ][xi-1];
				n->mChild[4] = mCells[yi-1][xi+1];
				n->mChild[5] = mCells[yi+1][xi+1];
				n->mChild[6] = mCells[yi+1][xi-1];
				n->mChild[7] = mCells[yi-1][xi-1];
			}
		}
	}
}

bool cPathFinder::CheckCellCount( unsigned int cellCount )
{
	return ( cellCount % 512 == 0 );
}

unsigned short cPathFinder::CalcHeuristic( unsigned short x, unsigned short y )
{
	int dx = x - mGoalX;
	int dy = y - mGoalY;

	return (unsigned short)(::sqrtf(float(dx*dx + dy*dy)) * 5.0f);
}

bool cPathFinder::FindPath( NiPoint2* pathArray, unsigned int* pathCount, unsigned int maxCount, const NiPoint2& s, const NiPoint2& g )
{
	unsigned short sx = (unsigned short)((s.x + 50.0f) / 100.0f);
	unsigned short sy = (unsigned short)((s.y + 50.0f) / 100.0f);
	unsigned short gx = (unsigned short)((g.x + 50.0f) / 100.0f);
	unsigned short gy = (unsigned short)((g.y + 50.0f) / 100.0f);

	return FindPath( pathArray, pathCount, maxCount, s, g, sx, sy, gx, gy );
}

bool cPathFinder::FindPath( NiPoint2* pathArray, unsigned int* pathCount, unsigned int maxCount, const NiPoint2& s, const NiPoint2& goal, unsigned short sx, unsigned short sy, unsigned short gx, unsigned short gy )
{
	assert( pathArray );
	assert( pathCount );
	assert( maxCount >= 4 && maxCount <= MAX_PATH_COUNT );
	assert( mNodes );

	if( sx == 0 || sx >= mLineCount || sy == 0 || sy >= mLineCount )
		return false;
	if( gx == 0 || gx >= mLineCount || gy == 0 || gy >= mLineCount )
		return false;

	/// 목표점이 이동 불가인지 검사
	if( mCells[gy][gx] == 0 )
		return false;

	/// 일직선 검사
	{
		unsigned int i = 1;
		unsigned int iend = (unsigned int)((goal-s).Length() / 50.0f);
		unsigned int x;
		unsigned int y;
		float t;
		NiPoint2 pos;

		if( iend == 0 )
		{
			x = (unsigned int)((goal.x + 50.0f) / 100.0f);
			y = (unsigned int)((goal.y + 50.0f) / 100.0f);

			if( mCells[y][x] )
			{
				pathArray[0].x = goal.x;
				pathArray[0].y = goal.y;
				*pathCount = 1;
				return true;
			}
		}
		else
		{
			for( ; i <= iend; ++i )
			{
				t = float(i) / float(iend);
				pos = (1.0f-t)*s + t*goal;
				x = (unsigned int)((pos.x + 50.0f) / 100.0f);
				y = (unsigned int)((pos.y + 50.0f) / 100.0f);

				if( mCells[y][x] == 0 )
				break;
			}

			if( i > iend )
			{
				pathArray[0].x = goal.x;
				pathArray[0].y = goal.y;
				*pathCount = 1;
				return true;
			}
		}
	}

	/// 길찾기 준비
	mGoalX = gx;
	mGoalY = gy;
	mOpenQueue.Clear();

	for( unsigned int x = 1; x < mCellCount; ++x )
	{
		for( unsigned int y = 1; y < mCellCount; ++y )
		{
			cPathNode* n = mCells[y][x];

			if( n )
			{
				n->mParent = 0;
				n->mClosed = false;
			}
		}
	}

	/// 열린 큐에 시작 노드를 추가
	bool failed = false;
	cPathNode* node = 0;
	cPathNode* start = mCells[sy][sx];

	if( start == 0 )
	{
		cPathNode* n = start = &mTempNode;
		n->mXIndex = (unsigned short)sx;
		n->mYIndex = (unsigned short)sy;
		n->mParent = 0;
		n->mCostFromStart = 0;
		n->mTotalCost = n->mCostToGoal = CalcHeuristic( sx, sy );
		n->mClosed = false;

		n->mChild[0] = mCells[sy-1][sx  ];
		n->mChild[1] = mCells[sy  ][sx+1];
		n->mChild[2] = mCells[sy+1][sx  ];
		n->mChild[3] = mCells[sy  ][sx-1];
		n->mChild[4] = mCells[sy-1][sx+1];
		n->mChild[5] = mCells[sy+1][sx+1];
		n->mChild[6] = mCells[sy+1][sx-1];
		n->mChild[7] = mCells[sy-1][sx-1];
	}
	else
	{
		cPathNode* n = start;
		n->mParent = 0;
		n->mCostFromStart = 0;
		n->mTotalCost = n->mCostToGoal = CalcHeuristic( sx, sy );
		n->mClosed = false;
	}

	mOpenQueue.Push( start );

	while( !failed )
	{
		if( mOpenQueue.GetCount() == 0 )
		{
			failed = true;
			break;
		}

		/// 최우선 순위 노드를 가져옴
		node = (cPathNode*)mOpenQueue.Front();
		mOpenQueue.PopFront();

		if( node->mClosed )
			continue;
		else
			node->mClosed = true;

		/// 현재 노드가 목표라면 길찾기 성공!
		if( node->mXIndex == gx && node->mYIndex == gy )
			break;

		/// 인접 노드들(자식들)에 대해
		for( unsigned int i = 0; i < 8; ++i )
		{
			/// 자식이 이미 닫혀있거나 지나갈 수 없는지 검사
			cPathNode* child = node->mChild[i];

			if( child == 0 || child->mClosed )
				continue;

			/// 비용을 계산
			unsigned short g = node->mCostFromStart + gCostTable[i];

			if( child->mParent == 0 )
			{
				/// 열린 목록에 없으면
				unsigned short h = CalcHeuristic( child->mXIndex, child->mYIndex );

				child->mParent = node;
				child->mTotalCost = g + h;
				child->mCostFromStart = g;
				child->mCostToGoal = h;

				/// 열린 목록에 추가
				if( mOpenQueue.Push( child ) == false )
				{
					failed = true;
					break;
				}
			}
			else if( g < child->mCostFromStart )
			{
				/// 열린 목록에 이미 있고, 비용이 이전보다 더 싸면
				child->mParent = node;
				child->mTotalCost = g + child->mCostToGoal;
				child->mCostFromStart = g;

				/// 열린 목록에 추가
				if( mOpenQueue.Push( child ) == false )
				{
					failed = true;
					break;
				}
			}
		}
	}

	if( failed )
	{
		return false;
	}
	if( node == 0 || node == start )
	{
		return false;
	}

	/// 길찾기 완료 작업
	node->mNext = 0;
	cPathNode* temp = node;
	cPathNode* parent = temp->mParent;

	while( parent )
	{
		parent->mNext = temp;
		temp = parent;
		parent = temp->mParent;
	}

	/// 길찾기 이동 경로를 보간 (직선경로 사이값들 제거)
	static NiPoint2 posArray0[MAX_PATH_COUNT];
	static NiPoint2 posArray1[MAX_PATH_COUNT];
	static float valArray[MAX_PATH_COUNT];
	unsigned int c = 0;

	cPathNode* checkPoint = start;
	cPathNode* smoothTemp = 0;
	for( temp = start; c < MAX_PATH_COUNT; )
	{
//*
		if( smoothTemp )
		{
			NiPoint2 sPos(checkPoint->mXIndex * 100.0f, checkPoint->mYIndex * 100.0f);
			NiPoint2 ePos(smoothTemp->mXIndex * 100.0f, smoothTemp->mYIndex * 100.0f);

			unsigned int i = 1;
			unsigned int iend = (unsigned int)((ePos-sPos).Length() / 50.0f);
			unsigned int x;
			unsigned int y;
			float t;
			NiPoint2 pos;

			if( iend == 0 )
			{
				x = (unsigned int)((ePos.x + 50.0f) / 100.0f);
				y = (unsigned int)((ePos.y + 50.0f) / 100.0f);

				if( mCells[y][x] )
				{
					temp = temp->mNext;
					if( temp == 0 )
						break;

					smoothTemp = temp->mNext;
					continue;
				}
			}
			else
			{
				for( ; i <= iend; ++i )
				{
					t = float(i) / float(iend);
					pos = (1.0f-t)*sPos + t*ePos;
					x = (unsigned int)((pos.x + 50.0f) / 100.0f);
					y = (unsigned int)((pos.y + 50.0f) / 100.0f);

					if( mCells[y][x] == 0 )
					break;
				}

				///
				if( i > iend )
				{
					temp = temp->mNext;
					if( temp == 0 )
						break;

					smoothTemp = temp->mNext;
					continue;
				}
				else
				{
					checkPoint = temp;
				}
			}
		}

//*/
		posArray0[c].x = temp->mXIndex * 100.0f;
		posArray0[c].y = temp->mYIndex * 100.0f;
		++c;

		temp = temp->mNext;
		if( temp == 0 )
			break;

		smoothTemp = temp->mNext;
	}
	{
		float x = node->mXIndex * 100.0f;
		float y = node->mYIndex * 100.0f;

		if( posArray0[c-1].x != x || posArray0[c-1].y != y )
		{
			posArray0[c].x = x;
			posArray0[c].y = y;
			++c;
		}
	}


	{
		unsigned int iend = c;

		/// 배열사이즈 검사
		if( iend > maxCount )
		{
			iend = maxCount;
			failed = true;
		}

		unsigned int c = 0;
		unsigned int i = 1;

		for( ; i < iend; ++i, ++c )
		{
			pathArray[c] = posArray0[i];
		}

		if( failed == false && c > 0 && (pathArray[c-1].x != goal.x || pathArray[c-1].y != goal.y) )
		{
			pathArray[c-1].x = goal.x;
			pathArray[c-1].y = goal.y;
		}

		*pathCount = c;
		assert( c <= maxCount );
	}

	return true;
}

// [int2000 - add get possible goal]
bool cPathFinder::GetPossibleGoal( NiPoint2 start, NiPoint2 &goal )
{
	unsigned int i = 1;
	unsigned int iend = (unsigned int)((goal-start).Length() / 100.0f );
	unsigned int x, y;
	float t;
	NiPoint2 pos, sPos;

	if( iend == 0 )
	{
		x = (unsigned int)((goal.x + 50.0f) / 100.0f);
		y = (unsigned int)((goal.y + 50.0f) / 100.0f);

		if( mCells[y][x] )
			return true;
	}
	else
	{
		if( iend < 5 )
			iend = 5;

		pos.x = start.x;
		pos.y = start.y;

		for( ; i <= iend; ++i )
		{
			sPos = pos;

			t = float(i) / float(iend);
			pos = (1.0f-t)*start + t*goal;

			x = (unsigned int)((pos.x + 50.0f) / 100.0f);
			y = (unsigned int)((pos.y + 50.0f) / 100.0f);

			if( mCells[y][x] == 0 )
			{
				goal = sPos;
				break;
			}
		}

		///
		if( i > iend )
		{
			return true;
		}
	}

	return false;
}

bool cPathFinder::IsPossible( NiPoint2 start, NiPoint2 goal )
{
	unsigned int i = 1;
	unsigned int iend = (unsigned int)((goal-start).Length() / 50.0f );
	unsigned int x, y;
	float t;
	NiPoint2 pos;

	if( iend == 0 )
	{
		x = (unsigned int)((goal.x + 50.0f) / 100.0f);
		y = (unsigned int)((goal.y + 50.0f) / 100.0f);

		if( mCells[y][x] )
			return true;
	}
	else
	{
		for( ; i <= iend; ++i )
		{
			t = float(i) / float(iend);
			pos = (1.0f-t)*start + t*goal;
			x = (unsigned int)((pos.x + 50.0f) / 100.0f);
			y = (unsigned int)((pos.y + 50.0f) / 100.0f);

			if( mCells[y][x] == 0 )
				break;
		}

		///
		if( i > iend )
			return true;
	}

	return false;
}
