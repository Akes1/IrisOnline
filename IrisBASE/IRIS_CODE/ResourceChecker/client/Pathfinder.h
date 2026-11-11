/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.10.08
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cNaviField;

/// 길찾기 노드
class cPathNode
{
	friend class cPathFinder;
	friend class cPathNodeCompare;
public:
	cPathNode();

private:
	/// 좌표
	unsigned short mXIndex;
	unsigned short mYIndex;

	/// 부모
	cPathNode* mParent;

	/// 자식
	cPathNode* mChild[8];

	/// 다음 (길찾기 성공후에 경로가 되는 자식)
	cPathNode* mNext;

	/// 총비용 (f = g + h)
	unsigned short mTotalCost;

	/// 시작점에서 현재 노드까지의 비용 (g)
	unsigned short mCostFromStart;

	/// 현재 노드에서 목표점까지의 비용 (h)
	unsigned short mCostToGoal;

	/// 닫힘 여부
	bool mClosed;
};

class cPathNodeCompare
{
public:
	bool operator () ( void* x, void* y ) const
	{
		return ((cPathNode*)x)->mTotalCost < ((cPathNode*)y)->mTotalCost;
	}
};

/// 길찾기
class cPathFinder
{
	static const unsigned int MAX_PATH_COUNT = 2048;

public:
	cPathFinder( unsigned int capacity );
	~cPathFinder();

	/// 지움
	void Clear();

	/// 초기화
	void Init( const cNaviField* naviField );

	/// 길찾기
	bool FindPath( NiPoint2* pathArray, unsigned int* pathCount, unsigned int maxCount, const NiPoint2& start, const NiPoint2& goal );
	bool FindPath( NiPoint2* pathArray, unsigned int* pathCount, unsigned int maxCount, const NiPoint2& start, const NiPoint2& goal, unsigned short sx, unsigned short sy, unsigned short gx, unsigned short gy );

	bool GetPossibleGoal( NiPoint2 start, NiPoint2 &goal );

	bool IsPossible( NiPoint2 start, NiPoint2 goal );

protected:
	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

	/// 발견적 비용을 계산
	unsigned short CalcHeuristic( unsigned short x, unsigned short y );

protected:
	/// 그리드 크기 및 단위
	unsigned int mCellCount;
	unsigned int mLineCount;

	/// 노드 배열
	cPathNode** mNodes;
	cPathNode*** mCells;
	cPathNode mTempNode;

	/// 열린 큐
	typedef tPriorityQueue<void*, cPathNodeCompare> cOpenQueue;
	cOpenQueue mOpenQueue;

	/// 목표점
	unsigned short mGoalX;
	unsigned short mGoalY;
};
