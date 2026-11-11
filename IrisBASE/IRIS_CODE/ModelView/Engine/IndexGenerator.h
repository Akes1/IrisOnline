/* ====================================================================
* 파    일 :	IndexGenerator.h
* 목    적 :	서로 겹치지 않는 인덱스를 제공
* 작 성 자 :	이법석
* 작 성 일 :	2006.01
* 주의사항 :	쓰레드에서의 사용은 일단 고려 안한다.
*				만약, 쓰레드 사용을 포함한다면 동기화를 위한 크리티컬 섹션을 넣어라
* =================================================================== */

#pragma once

template <class T>
class cIndexGenerator
{
public:
	cIndexGenerator( T initilize = 100 );
	~cIndexGenerator();

	/// 인덱스를 제공
	T GenerateIndex();

	/// 인덱스를 반환
	void RestoreIndex( T idx );
	void RestoreAll();

private:
	/// 반환된 인덱스를 보관한다.
	NiTPrimitiveQueue<T>*		mpKeeper;

	/// 지금까지 제공한 인덱스의 최종값
	T mLastIndexValue;

	T mInitValue;
};

template <class T> inline
cIndexGenerator<T>::cIndexGenerator( T initilize )
{
	/// 메모리를 잡는다.
	mpKeeper = NiNew NiTPrimitiveQueue<T>( initilize );

	mInitValue = initilize;
	mLastIndexValue = 0;
}

template <class T> inline
cIndexGenerator<T>::~cIndexGenerator()
{
	SAFE_DELETE(mpKeeper);
}

template <class T> inline
void cIndexGenerator<T>::RestoreAll()
{
	mLastIndexValue = 0;

	while( mpKeeper->IsEmpty() == false )
		mpKeeper->Remove();
}


/* ------------------------------------------------------------------
* 함수이름 :	GenerateIndex()
* 목    적 :	인덱스 제공
* 주의사항 :	
* ------------------------------------------------------------------ */
template <class T> inline
T cIndexGenerator<T>::GenerateIndex()
{
	//	if( mpKeeper->GetSize() < 10000 )
	//	{
	if( mLastIndexValue < mLastIndexValue+1 )
	{
		mLastIndexValue += 1;

		return mLastIndexValue;
	}

	//		if( mpKeeper->IsEmpty() )
	//			assert(0);
	//	}
	return mpKeeper->Remove();
}

/* ------------------------------------------------------------------
* 함수이름 :	RestoreIndex( unsigned int idx )
* 목    적 :	사용된 인덱스의 반환
* 주의사항 :	
* ------------------------------------------------------------------ */
template <class T> inline
void cIndexGenerator<T>::RestoreIndex( T idx )
{
	/// 이 함수는 반환 인덱스가 많을수록 느리다.
	//	mpKeeper->AddUnique( idx );

	/// 만약, 인덱스가 겹쳐서 제공된다고 생각한다면, 검사해봐라!!
	mpKeeper->Add( idx );
}
