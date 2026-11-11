#pragma once

#include "BaseObject.h"
#include "RangeCheck.h"
#include "UserPortal_Common.h"

class cUserPortal : public cBaseObject
{

public:
	/// 생성소멸
	cUserPortal( void );
	~cUserPortal( void );

	/// 메모리 할당 제거 pool 사용
	void* operator new ( size_t n ); 
	void operator delete ( void* ptr, size_t n );

	bool			InitUserPortal( unsigned long objectIdx, unsigned long creatorIdx, unsigned long mapChangePosIdx,
		unsigned short mapNumber, float posX, float posY );
	
	void			ProcessUserPortal( unsigned long elapsedTime, unsigned long accumTime );

	bool			IsMapChange( unsigned long playerIdx );	

	unsigned long	GetMapChangePosIdx()	{ return mMapChangePosIdx; }
	unsigned long	GetCreator()			{ return mCreator; }

private:

	unsigned long	mMapChangePosIdx;		/// 맵이동 좌표 스크립트 인덱스
	unsigned long	mCreator;				/// 포탈 생성자

	unsigned long	mEndTime;				/// 객제 삭제 시간

	cRangeCheck		mRangeCheck;

public:
	bool           SendSightIn        ( char category, char protocol, unsigned long connectionIdx );
	bool           SendSightOut       ( char category, char protocol, unsigned long connectionIdx );
};

