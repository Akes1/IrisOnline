#include "stdafx.h"
#include "VisibleArraySorter.h"

#include "VisibleArray.h"
#include "Camera.h"

cVisibleArraySorter::cVisibleArraySorter( cVisibleArray* array )
: mArray( array )
, mNumGeoms( 0 )
, mMaxGeoms( 0 )
, mGeoms( 0 )
, mDepths( 0 )
{
	assert( array && "null visible array" );
}

cVisibleArraySorter::~cVisibleArraySorter()
{
	NiFree( mDepths );
}


//////////////////////////////////////////////////////////////////////////

cAlphaArraySorter::cAlphaArraySorter( cVisibleArray* array, cVisibleArray* testArray )
: cVisibleArraySorter( array )
, mTestArray( testArray )
, mNumTestGeoms( 0 )
, mMaxTestGeoms( 0 )
, mTestGeoms( 0 )
, mTestDepths( 0 )
{
	mSortGeoms = 0;
	mSortDepths = 0;
	mSortMaxGeoms = 0;
}

cAlphaArraySorter::~cAlphaArraySorter()
{
	NiFree( mTestDepths );
}

void cAlphaArraySorter::Sort( const cCamera& cam )
{
	SortTestGeom( cam );

	/// 기하 오브젝트 수를 검사
	mNumGeoms = mArray->GetCount();
	if( mNumGeoms == 0 )
		return;

	if( mNumGeoms > mMaxGeoms )
	{
		mMaxGeoms = mNumGeoms;

		NiFree( mDepths );
		mDepths = NiAlloc( float, mMaxGeoms );
		assert( mDepths );
	}

	/// 기하 오브젝트들의 깊이값을 계산
	mGeoms = mArray->GetGeomArray();

	NiPoint3 viewPos = cam.GetWorldTranslate();
	NiPoint3 viewDir = cam.GetWorldDirection();

	float r,length;
	for( int i = 0; i < mNumGeoms; ++i )
	{
		r = mGeoms[i]->GetWorldBound().GetRadius();
		length = (mGeoms[i]->GetWorldBound().GetCenter() - viewPos).SqrLength();

		mDepths[i] = (mGeoms[i]->GetWorldBound().GetCenter() - mGeoms[i]->GetWorldBound().GetRadius()*viewDir - viewPos).SqrLength();

		if( length < r*r )
			mDepths[i] = -mDepths[i];
	}

	mSortGeoms = mGeoms;
	mSortDepths = mDepths;
	mSortMaxGeoms = mNumGeoms;
	SortObjectsByDepth( 0, mNumGeoms - 1 );
}

void cAlphaArraySorter::SortTestGeom( const cCamera& cam )
{
	if( mTestArray == 0 )
		return;

	/// alpha test count
	mNumTestGeoms = mTestArray->GetCount();
	if( mNumTestGeoms == 0 )
		return;

	if( mNumTestGeoms > mMaxTestGeoms )
	{
		mMaxTestGeoms = mNumTestGeoms;

		NiFree( mTestDepths );
		mTestDepths = NiAlloc( float, mMaxTestGeoms );
		assert( mTestDepths );
	}

	mTestGeoms = mTestArray->GetGeomArray();

	NiPoint3 viewPos = cam.GetWorldTranslate();
	NiPoint3 viewDir = cam.GetWorldDirection();

	float r,length;
	for( int i = 0; i < mNumTestGeoms; ++i )
	{
		r = mTestGeoms[i]->GetWorldBound().GetRadius();
		length = (mTestGeoms[i]->GetWorldBound().GetCenter() - viewPos).SqrLength();

		mTestDepths[i] = (mTestGeoms[i]->GetWorldBound().GetCenter() - mTestGeoms[i]->GetWorldBound().GetRadius()*viewDir - viewPos).SqrLength();

		if( length < r*r )
			mTestDepths[i] = -mTestDepths[i];
	}

	mSortGeoms = mTestGeoms;
	mSortDepths = mTestDepths;
	mSortMaxGeoms = mNumTestGeoms;
	SortObjectsByDepth( 0, mNumTestGeoms - 1 );
}

void cAlphaArraySorter::Render()
{
	NiRenderer* renderer = NiRenderer::GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 );
		return;
	}

	if( mNumGeoms == 0 && mNumTestGeoms == 0 )
		return;

	for( int i = mNumGeoms - 1; i >= 0; --i )
	{
		mGeoms[i]->RenderImmediate( renderer );
	}

	if( mTestArray )
	{
		for( int i = mNumTestGeoms-1; i >= 0; --i  )
			mTestGeoms[i]->RenderImmediate( renderer );
	}

	/// rendering후 초기화
	mNumGeoms = 0;
	mNumTestGeoms = 0;
}

void cAlphaArraySorter::SortObjectsByDepth( int l, int r )
{
	if( r > l)
	{
		int i, j;

		i = l - 1;
		j = r + 1;
		float fPivot = ChoosePivot(l, r);

		for( ;;)
		{
			do 
			{
				j--;

			} while (fPivot < mSortDepths[j]);

			do
			{
				i++;
				assert( i<mSortMaxGeoms );

			} while (mSortDepths[i] < fPivot);

			if( i < j )
			{
				assert( i<mSortMaxGeoms );
				assert( j<mSortMaxGeoms );

				NiRenderObject* pkObjTemp = mSortGeoms[i];
				mSortGeoms[i] = mSortGeoms[j];
				mSortGeoms[j] = pkObjTemp;

				float fTemp = mSortDepths[i];
				mSortDepths[i] = mSortDepths[j];
				mSortDepths[j] = fTemp;
			}
			else
			{
				break;
			}
		}

		if( j == r)
		{
			SortObjectsByDepth(l, j - 1);
		}
		else
		{
			SortObjectsByDepth(l, j);
			SortObjectsByDepth(j + 1, r);
		}
	}
}

float cAlphaArraySorter::ChoosePivot( int l, int r ) const
{
	// Check the first, middle, and last element. Choose the one which falls
	// between the other two. This has a good chance of discouraging 
	// quadratic behavior from qsort.
	// In the case when all three are equal, this code chooses the middle
	// element, which will prevent quadratic behavior for a list with 
	// all elements equal.

	int m = (l + r) >> 1;

	assert( l < mSortMaxGeoms );
	assert( m < mSortMaxGeoms );
	assert( r < mSortMaxGeoms );

	if( mSortDepths[l] < mSortDepths[m])
	{
		if( mSortDepths[m] < mSortDepths[r])
		{
			return mSortDepths[m];
		}
		else
		{
			if( mSortDepths[l] < mSortDepths[r])
				return mSortDepths[r];
			else
				return mSortDepths[l];
		}
	}
	else
	{
		if( mSortDepths[l] < mSortDepths[r])
		{
			return mSortDepths[l];
		}
		else
		{
			if( mSortDepths[m] < mSortDepths[r])
				return mSortDepths[r];
			else
				return mSortDepths[m];
		}
	}
}

//////////////////////////////////////////////////////////////////////////

cSolidArraySorter::cSolidArraySorter( cVisibleArray* array )
: cVisibleArraySorter( array )
{
}

cSolidArraySorter::~cSolidArraySorter()
{
}

void cSolidArraySorter::Sort( const cCamera& cam )
{
	/// 기하 오브젝트 수를 검사
	mNumGeoms = mArray->GetCount();
	if( mNumGeoms == 0 )
		return;

	if( mNumGeoms > mMaxGeoms )
	{
		mMaxGeoms = mNumGeoms;

		NiFree( mDepths );
		mDepths = NiAlloc( float, mMaxGeoms );
		assert( mDepths );
	}

	/// 기하 오브젝트들의 깊이값을 계산
	mGeoms = mArray->GetGeomArray();
	NiPoint3 viewDir = cam.GetWorldDirection();

	for( int i = 0; i < mNumGeoms; ++i )
	{
		mDepths[i] = mGeoms[i]->GetWorldBound().GetCenter() * viewDir;
	}

	SortObjectsByDepth( 0, mNumGeoms - 1 );
}

void cSolidArraySorter::Render()
{
	NiRenderer* renderer = NiRenderer::GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 );
		return;
	}
	if( mNumGeoms == 0 )
		return;

	for( int i = 0; i < mNumGeoms; ++i )
	{
		mGeoms[i]->RenderImmediate( renderer );
	}

	/// rendering후 초기화
	mNumGeoms = 0;
}

void cSolidArraySorter::SortObjectsByDepth(int l, int r)
{
	if (r > l)
	{
		int i, j;

		i = l - 1;
		j = r + 1;
		float fPivot = ChoosePivot(l, r);

		for (;;)
		{
			do 
			{
				j--;
			} while (fPivot > mDepths[j]);

			do
			{
				i++;
			} while (mDepths[i] > fPivot);

			if (i < j)
			{
				NiRenderObject* pkObjTemp = mGeoms[i];
				mGeoms[i] = mGeoms[j];
				mGeoms[j] = pkObjTemp;
				float fTemp = mDepths[i];
				mDepths[i] = mDepths[j];
				mDepths[j] = fTemp;
			}
			else
			{
				break;
			}
		}

		if (j == r)
		{
			SortObjectsByDepth(l, j - 1);
		}
		else
		{
			SortObjectsByDepth(l, j);
			SortObjectsByDepth(j + 1, r);
		}
	}
}

float cSolidArraySorter::ChoosePivot(int l, int r) const
{
	// Check the first, middle, and last element. Choose the one which falls
	// between the other two. This has a good chance of discouraging 
	// quadratic behavior from qsort.
	// In the case when all three are equal, this code chooses the middle
	// element, which will prevent quadratic behavior for a list with 
	// all elements equal.

	int m = (l + r) >> 1;

	if (mDepths[l] > mDepths[m])
	{
		if (mDepths[m] > mDepths[r])
		{
			return mDepths[m];
		}
		else
		{
			if (mDepths[l] > mDepths[r])
				return mDepths[r];
			else
				return mDepths[l];
		}
	}
	else
	{
		if (mDepths[l] > mDepths[r])
		{
			return mDepths[l];
		}
		else
		{
			if (mDepths[m] > mDepths[r])
				return mDepths[r];
			else
				return mDepths[m];
		}
	}
}
