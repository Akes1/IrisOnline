#include "stdafx.h"
#include "Object.h"

#include "GameResourceManager.h"

cPlayer::cPlayer( cObjectManager* manager )
: cObject( eOBJECTTYPE_PLAYER, manager)
{

	mFixedObjectSize = 0.f;
}

cPlayer::~cPlayer()
{

}

bool cPlayer::Init( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, sPlayerExrInfo* pExrInfo, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	::memcpy( &mPlayerInfo, pBaseInfo, sizeof(sPlayerInfo) );
	::memcpy( &mPlayerExrInfo, pExrInfo, sizeof(sPlayerExrInfo) );
	::memcpy( &mPlayerWearInfo, pWearInfo, sizeof(sPlayerWearInfo) );
	::memcpy( &mPlayerWeaponInfo, pWeaponInfo, sizeof(sPlayerWeaponInfo) );


	SetFixedObjectSizeScale( mPlayerExrInfo.mFixedObjectSizePer, true );

	return true;
}


void cPlayer::Process( unsigned long deltaTime, unsigned long accumTime )
{
	deltaTime;
	accumTime;

}

float cPlayer::GetFixedObjectSize()
{
	if( mFixedObjectSize <= 0.0f )
	{
		assert(NULL);
		return 1.0f;
	}

	return mFixedObjectSize;
}

void cPlayer::SetFixedObjectSizeScale( unsigned short scalePer, bool init )
{
	mPlayerExrInfo.mFixedObjectSizePer = scalePer;

	if( mPlayerExrInfo.mChgMonsterIdx == 0 )
	{
		mFixedObjectSize = mPlayerExrInfo.mBaseFixedSize * (float)scalePer * 0.01f;
	}
	else
	{
		sMonsterScript* pMonsterInfo = GAMERESOURCEMAN->GetMonsterListInfo( mPlayerExrInfo.mChgMonsterIdx );
		if( pMonsterInfo == NULL )
		{
			assert(0);
			return ;
		}

		float scale = 1.0f;
		if( pMonsterInfo->mMonsterScale > 0.0f )
		{
			scale = pMonsterInfo->mMonsterScale;
		}
		else
		{
			assert(0);
		}

		mFixedObjectSize = ( pMonsterInfo->mMonsterFixSize * scale ) * (float)scalePer * 0.01f;
	}
}
