/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.17
* 내    용 : tPool를 구현
* 주의사항 : 
*===========================================================================*/

template<class T, class ALLOC> inline
tPoolNode<T, ALLOC>::tPoolNode( unsigned int capacity )
: mCapacity( capacity )
, mpNext( 0 )
{
	if( capacity > 0 )
	{
		mpObjectArray = ALLOC::Allocate( capacity );
		assert( mpObjectArray );
	}
	else
	{
		assert( 0 && "size of pool node is zero" );
		mpObjectArray = 0;
	}
}

template<class T, class ALLOC> inline
tPoolNode<T, ALLOC>::~tPoolNode()
{
	ALLOC::Deallocate( mpObjectArray );
	delete mpNext;
}

template<class T, class ALLOC> inline
T* tPoolNode<T, ALLOC>::GetObject( unsigned int i )
{
	if( i >= mCapacity )
	{
		assert( 0 && "index is invalid" );
		return 0;
	}
	return &mpObjectArray[i];
}

template<class T> inline
tPool<T>::tPool( unsigned int initialCapacity, unsigned int stepCapacity, unsigned int hashHint )
: mFreeObjectSet( hashHint )
, mpRootNode( 0 )
, mSize( 0 )
, mCapacity( 0 )
, mInitialCapacity( initialCapacity )
, mStepCapacity( stepCapacity )
{
	if( initialCapacity > 0 )
	{
		CreateNode( initialCapacity );
	}
}

template<class T> inline
tPool<T>::~tPool()
{
	mFreeObjectSet.Clear();
	delete mpRootNode;
}

template<class T> inline
void tPool<T>::Clear()
{
	mFreeObjectSet.Clear();
	delete mpRootNode;
	mpRootNode = 0;
	mSize = 0;
	mCapacity = 0;
}

template<class T> inline
void tPool<T>::Reserve( unsigned int capacity, unsigned int stepCapacity )
{
	if( stepCapacity )
	{
		mStepCapacity = stepCapacity;
	}

	int c = (int)(capacity - mCapacity);

	if( c > 0 )
	{
		CreateNode( (unsigned int)c >= mStepCapacity ? (unsigned int)c : mStepCapacity );
	}
}

template<class T> inline
T* tPool<T>::Alloc()
{
	if( mFreeObjectSet.GetSize() == 0 )
	{
		if( mStepCapacity == 0 )
		{
			CreateNode( mCapacity );
		}
		else
		{
			CreateNode( mStepCapacity );
		}
	}

	cObjectSet::cIterator i = mFreeObjectSet.Begin();
	T* p = *i;
	mFreeObjectSet.Erase( i );

	/// 할당된 오브젝트 수 증가
	++mSize;
	return p;
}

template<class T> inline
void tPool<T>::Free( T* p )
{
	if( mFreeObjectSet.Insert( p ) == true )
	{
		/// 할당된 오브젝트 수 감소
		--mSize;
	}
}

template<class T> inline
unsigned int tPool<T>::GetSize() const
{
	return mSize;
}

template<class T> inline
unsigned int tPool<T>::GetCapacity() const
{
	return mCapacity;
}

template<class T> inline
bool tPool<T>::CreateNode( unsigned int capacity )
{
	if( capacity == 0 )
	{
		assert( 0 && "zero capacity" );
		return false;
	}
	else
	{
		cNode* n = new cNode( capacity );

		for( unsigned int i = 0; i < capacity; ++i )
		{
			mFreeObjectSet.Insert( n->GetObject(i) );
		}

		/// 노드 연결
		n->mpNext = mpRootNode;
		mpRootNode = n;

		/// 용량 증가
		mCapacity += capacity;
		return true;
	}
}
