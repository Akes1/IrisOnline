/* =======================================================================================================
* 파    일 :	PathObserver.h , PathObserver.cpp
* 목    적 :	객체들마다 자신만의 경로를 갖도록 해준다.
* 작 성 자 :	정원주
* 작 성 일 :    2006년 9월 26일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* ======================================================================================================== */

#pragma once

class cPathfinder;

class cPathNodeSet
{

private:
	
	void			MemoryDestory();
	
	///index를 실제 좌표로 변환해주는 함수
	void			MakeRealPosition();
public:

	bool			Create(cPathfinder* pathfinder);

	void			ResetPath(float startx,float starty,float destx,float desty);
	void			RayPath(float destx,float desty);
	
	NiPoint2		GetNextPath();
	NiPoint2		GetCurrentPathNode(const unsigned int& curnode);
	
	bool			IsFinalPath();

	unsigned int	GetTotalPathCount() { return mTotalPathCount; }
	unsigned int	GetCurrentPathCount() { return mCurrentUseIndex; }
	
	void			GetPathArray(POINT* array , const unsigned int maxcopycount, unsigned int& copycount);
	void            SetPathArray(POINT* array, const unsigned int count);

	cPathNodeSet();
	~cPathNodeSet();

private:

	cPathfinder*    mpPathfinder;

	POINT*			mPointArray;

	struct Point2
	{
		float x, y;
	};

	
	///실제 좌표의 중간 값이 들어간 리스트
	tArray<Point2>		mPosvectorlist;

	///mPosvectorlist에 최종적으로 들어가는 index수
	unsigned int    mTotalPathCount;
	///현재 유저가 사용하고 있는 Index
	unsigned int	mCurrentUseIndex;
};