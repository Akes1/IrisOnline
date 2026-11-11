/* ====================================================================
* 파    일 :	DisplayObject.h
* 목    적 :	캐릭터 선택 및 생성창을 위한 전시용 객체
* 작 성 자 :	이법석
* 작 성 일 :	2007.4.17
* 주의사항 :	
* =================================================================== */

#pragma once
#include "Player.h"

class cDisplayObject : public cPlayer
{
public:
	cDisplayObject();
	virtual ~cDisplayObject();

	virtual bool Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, sPlayerExrInfo* pExrInfo,
						NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );

	virtual bool ReCreate( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, 
							NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );


	/// 갱신부
	virtual void Update( unsigned long deltaTime, unsigned long accumTime );


	inline char				GetStandIdx() { return mStandIdx; }
	inline void				SetStandIdx( char standIdx ) { mStandIdx = standIdx; }

	inline unsigned short	GetMapNum() { return mMapNum; }
	inline void				SetMapNum( unsigned short mapIdx ) { mMapNum = mapIdx; }

	const NiPoint3& GetCenterPos();

	void SetRot( NiMatrix3 mat );
	void Rotate( float angle );

	/// 객체의 외형정보 변형
	void UpdateDisplayForm( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo );

	void ClearEffect();
	void EnableNameCard( bool enable ) { mSetName = enable; }

	void SetDeleteWaitTime( long time );
	long GetDeleteWaitTime() { return mDeleteWaitTime; }

	void SetDeleteLock( bool lock ) { mDeleteLock = lock; }
	bool IsDeleteLock() { return mDeleteLock; }

protected:
	char				mStandIdx;					/// 캐릭터 순서
	unsigned short		mMapNum;					/// 맵     번호

	bool				mSetName;
	bool				mInited;

	long				mDeleteWaitTime;			/// 삭제 대기 시간
	bool				mDeleteLock;				/// delete 락
};
