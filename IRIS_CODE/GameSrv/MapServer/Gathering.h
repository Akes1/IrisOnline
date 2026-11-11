/* ==========================================================================
* 파    일 : Gathering.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-12-11
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Gathering_Common.h"
#include "BaseObject.h"
#include "RangeCheck.h"

enum eACTION_GAT_ID;
enum eACTION_GAT_CHANGE;

class cGathering : public cBaseObject
{

public:
	/// 생성소멸
	cGathering( void );
	virtual ~cGathering( void );

	/// 메모리 할당 제거 pool 사용
	void* operator new ( size_t n ); 
	void operator delete ( void* ptr, size_t n );

	bool			GatheringInit( unsigned long objectIdx, unsigned long regenIdx, unsigned long gatheringIdx, unsigned short mapNumber,
		float posX, float posY, float direction, float height );
	void			SetFlagTeamType( unsigned char teamType )	{ mFlagTeamType = teamType; }

	void			Update( unsigned long elapsedTime, unsigned long accumTime );

	float			GetAppendZ() { return mHeight; }
	float			GetDirection() { return mDirection; }

	bool			SelectRegenPos();

	unsigned long	GetClassIdx();

	void			SendRegenSync();
	void			SendRegenSync( unsigned char teamType );
	
	/// 채집하는 유저 추가
	void			AddGatherPlayer( cPlayer* pPlayer );

	/// 유저 취소 신청
	void			ReqEraseGatherPlayer( cPlayer* pPlayer );

	/// 채집 취소 (상태이상 )
	void			CancelGatherByOddity( cPlayer* pPlayer );

	void			EraseGatherPlayer( cPlayer* pPlayer );

	/// 채집 종료 - 드롭처리
	void			GatheringDie( unsigned long playerIdx );
	void			GatheringDie_DM( unsigned long playerIdx );

	void			SetDMIndex( unsigned long dmIdx ) { mDMIdx = dmIdx; }
	
	unsigned long	GetInfluence();

private:

	/// 스크립트 정보
	sGatheringList*	mpGatheringInfo;

	/// 리젠정보
	unsigned long	mGatheringIdx;		/// 채집IDX
	float			mPosX;				/// 좌표
	float			mPosY;
	float			mDirection;			/// 회전값
	float			mHeight;			/// 높이 확장값

	typedef tHashMap<unsigned long /*playerIdx*/, unsigned long /*startTime*/> cHashMap;
	cHashMap		mGatherCharactorMap;

	unsigned long	mRegenIndex;

	cRangeCheck		mRangeCheck;

	/// 데스매치용 채집물 연결
	unsigned long	mDMIdx;

	unsigned char	mFlagTeamType;

public:
	bool           SendSightIn        ( char category, char protocol, unsigned long connectionIdx );
	bool           SendSightOut       ( char category, char protocol, unsigned long connectionIdx );
};