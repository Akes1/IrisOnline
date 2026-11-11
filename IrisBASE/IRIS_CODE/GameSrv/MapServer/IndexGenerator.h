/* ==========================================================================
* 파    일 : IndexGenerator.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-07-31
* 주의사항 :
*===========================================================================*/
#pragma once

class cIndexGenerator
{
public:
	cIndexGenerator(void);	
	~cIndexGenerator(void);

	unsigned long		GeneratIdx();
	void				DelIdx( unsigned long idx );

	void				SetQueue( bool useQueue, unsigned long useCount )	{ mUseQueue = useQueue; mUseCount = useCount; }

private:

	tQueue<unsigned long>	mEmptyIdxQueue;

	unsigned long		mIdxMax;		/// 현재 발급된 최대 인덱스

	bool				mUseQueue;		/// 큐를 사용해 사용하다 반납한 인덱스도 다시사용인지 여부
	unsigned long		mUseCount;		/// 큐에 일정량이 차면 사용했던 인덱스를 반환

};
