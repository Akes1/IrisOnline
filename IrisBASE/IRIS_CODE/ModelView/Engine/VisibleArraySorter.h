/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.07
* 내    용 : 가시 배열 정렬자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cVisibleArray;
class cCamera;

/// 가시 배열 정렬자
class cVisibleArraySorter
{
public:
	cVisibleArraySorter( cVisibleArray* array );
	virtual ~cVisibleArraySorter();

	/// 정렬
	virtual void Sort( const cCamera& cam )=0;

	/// 렌더링
	virtual void Render() = 0;

protected:
	/// 가시 배열
	cVisibleArray* mArray;

	int mNumGeoms;
	int mMaxGeoms;

	NiRenderObject** mGeoms;
	float* mDepths;
};

/// 반투명 가시 배열 정렬자
class cAlphaArraySorter : public cVisibleArraySorter
{
public:
	cAlphaArraySorter( cVisibleArray* array, cVisibleArray* testArray );
	virtual ~cAlphaArraySorter();

	/// 정렬
	virtual void Sort( const cCamera& cam );

	/// 렌더링
	virtual void Render();


private:
	void SortObjectsByDepth( int l, int r );
	float ChoosePivot( int l, int r ) const;

	NiRenderObject** mSortGeoms;
	float* mSortDepths;
	int mSortMaxGeoms;

protected:
	void SortTestGeom( const cCamera& cam );

protected:
	cVisibleArray* mTestArray;

	int mNumTestGeoms;
	int mMaxTestGeoms;

	NiRenderObject** mTestGeoms;
	float* mTestDepths;
};

/// 불투명 가시 배열 정렬자
class cSolidArraySorter : public cVisibleArraySorter
{
public:
	cSolidArraySorter( cVisibleArray* array );
	virtual ~cSolidArraySorter();

	/// 정렬
	virtual void Sort( const cCamera& cam );

	/// 렌더링
	virtual void Render();
protected:
	void SortObjectsByDepth(int l, int r);
	float ChoosePivot(int l, int r) const;
};