#include "StdAfx.h"
#include "displayobject.h"

#include "PlayerSceneNode.h"
#include "GameResourceManager.h"
#include "ObjectNameCard.h"

#include "ItemManager.h"

cDisplayObject::cDisplayObject()
{
	mStandIdx = 0;
	mMapNum = 0;
	mSetName = false;
	mDeleteWaitTime = LONG_MAX;
	mDeleteLock = false;
}

cDisplayObject::~cDisplayObject()
{
}

bool cDisplayObject::Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, 
						 sPlayerExrInfo* pExrInfo, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	::memcpy( &mPlayerInfo, pBaseInfo, sizeof(sPlayerInfo) );
	::memcpy( &mPlayerWearInfo, pWearInfo, sizeof(sPlayerWearInfo) );
	::memcpy( &mPlayerWeaponInfo, pWeaponInfo, sizeof(sPlayerWeaponInfo) );
	::memset( &mPlayerExrInfo, 0, sizeof(sPlayerExrInfo) );

	const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
	cPlayerSceneNodeParam param;
	param.mpObject = this;
	param.mDisplayObj = true;
	param.mPathName = strFile;
	param.mTranslate = sPos;
	param.mRotate = sRot;
	param.mScale = fScale;
	param.mCreateNameCard = mSetName;
	param.mUseShadow = false;

	mpObjectSceneNode = CreateSceneNode( &param );
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	mpObjectSceneNode->ShowNameGauge( false );

	/// head 정보 셋팅
	ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
	SetBaseFace();

	if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC], true );
	else
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE], true );

	/// 코스튬 우선
	for( unsigned int ui=eWEAR_BODY1; ui<eWEAR_HAT; ui++ )
	{
		if( mPlayerWearInfo.CostumeIdx[ui] == 0 )
		{
			if( mPlayerWearInfo.WearIdx[ui] == 0 )
				ChangeBaseWear( ePART_BODY1 + ui );
			else
				EquipWearItem( mPlayerWearInfo.WearIdx[ui] );
		}
		else
		{
			EquipWearItem( mPlayerWearInfo.CostumeIdx[ui] );

			/// 원피스인 경우 배제..
			if( ui == eWEAR_BODY1 )
			{
				cItemDefine* p = ITEMMAN->GetItemDefine( mPlayerWearInfo.CostumeIdx[ui] );
				if( p && p->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
					break;
			}
		}
	}

	/// eWEAR_HAT
	if( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] == 0 )
	{
		if( mPlayerWearInfo.WearIdx[eWEAR_HAT] != 0 )
			EquipWearItem( mPlayerWearInfo.WearIdx[eWEAR_HAT] );
	}
	else
	{
		EquipWearItem( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] );
	}

	/// 착용 아이템 설정
	/// 초기화 구문이므로 직접 제어한다..
	EquipWeaponItem( &mPlayerWeaponInfo );

	for( unsigned char i=eCOSTUME_HEAD; i<eCOSTUME_MAX; i++ )
		EquipAccessoryItem( i, mPlayerWearInfo.CostumeAccIdx[i] );

	cPlayerNameCard* card = (cPlayerNameCard*)mpObjectSceneNode->GetNameCard();
	if( card )
	{
		TCHAR buf[50];
		::_stprintf_s(buf, _T("Lv %d"), mPlayerInfo.Level );
		card->SetGuildName( buf );
	}

	return true;
}

bool cDisplayObject::ReCreate( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	RemoveObjectSceneNode();

	for( unsigned int i = 0; i < eHAND_ALL; i++ )
	{
		mWeaponObjectIndex[i] = UINT_MAX;
	}

	::memcpy( &mPlayerInfo, pBaseInfo, sizeof(sPlayerInfo) );
	::memcpy( &mPlayerWearInfo, pWearInfo, sizeof(sPlayerWearInfo) );
	::memcpy( &mPlayerWeaponInfo, pWeaponInfo, sizeof(sPlayerWeaponInfo) );
	::memset( &mPlayerExrInfo, 0, sizeof(sPlayerExrInfo) );

	const char* strFile = GAMERESOURCEMAN->GetModelFileName( (mPlayerInfo.Race * eGENDER_MAX) + mPlayerInfo.Gender );
	cPlayerSceneNodeParam param;
	param.mpObject = this;
	param.mDisplayObj = true;
	param.mPathName = strFile;
	param.mTranslate = sPos;
	param.mRotate = sRot;
	param.mScale = fScale;
	param.mCreateNameCard = false;
	param.mUseShadow = false;

	mpObjectSceneNode = CreateSceneNode( &param );
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	mpObjectSceneNode->ShowNameGauge( false );

	/// head 정보 셋팅
	ChangeBaseHair( mPlayerInfo.HeadInfo[eHEAD_HAIR], mPlayerInfo.HeadInfo[eHEAD_HAIRCOLOR] );
	SetBaseFace();

	if( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] != 0 )
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC], true );
	else
		ChangeFaceTexture( mPlayerInfo.HeadInfo[eHEAD_FACE], true );


	/// 코스튬 우선
	for( unsigned int ui=eWEAR_BODY1; ui<eWEAR_HAT; ui++ )
	{
		if( mPlayerWearInfo.CostumeIdx[ui] == 0 )
		{
			if( mPlayerWearInfo.WearIdx[ui] == 0 )
				ChangeBaseWear( ePART_BODY1 + ui );
			else
				EquipWearItem( mPlayerWearInfo.WearIdx[ui] );
		}
		else
		{
			EquipWearItem( mPlayerWearInfo.CostumeIdx[ui] );

			/// 원피스인 경우 배제..
			if( ui == eWEAR_BODY1 )
			{
				cItemDefine* p = ITEMMAN->GetItemDefine( mPlayerWearInfo.CostumeIdx[ui] );
				if( p && p->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
					break;
			}
		}
	}

	/// eWEAR_HAT
	if( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] == 0 )
	{
		if( mPlayerWearInfo.WearIdx[eWEAR_HAT] != 0 )
			EquipWearItem( mPlayerWearInfo.WearIdx[eWEAR_HAT] );
	}
	else
	{
		EquipWearItem( mPlayerWearInfo.CostumeIdx[eWEAR_HAT] );
	}

	/// 착용 아이템 설정
	/// 초기화 구문이므로 직접 제어한다..
	EquipWeaponItem( &mPlayerWeaponInfo );

	for( unsigned char i=eCOSTUME_HEAD; i<eCOSTUME_MAX; i++ )
		EquipAccessoryItem( i, mPlayerWearInfo.CostumeAccIdx[i] );

	return true;
}

void cDisplayObject::Update( unsigned long deltaTime, unsigned long accumTime )
{
	FaceProcess( deltaTime );

	/// 애니메이션 연결에 대한 처리
	Interpret( accumTime );

	// 삭제 대기 시간 처리 
	if( mDeleteWaitTime != LONG_MAX )
	{
		if( mDeleteWaitTime < (long)deltaTime )
			mDeleteWaitTime = 0;
		else
			mDeleteWaitTime -= deltaTime;

		cStringT str;
		unsigned int restTime = mDeleteWaitTime/1000;
		
		unsigned int d = (restTime / 60) / 60 / 24;
		unsigned int h = (restTime / 60) / 60 % 24;
		unsigned int m = (restTime / 60) % 60;
		unsigned int s = (restTime % 60) % 60 + 1;

		/// 반올림해서 출력
		if( d != 0 )
			d++;
		if( h != 0 )
			h++;
		if( m != 0 )
			m++;

		if( d != 0 )
			str.Format( GAMERESOURCEMAN->GetGameText( 74 ), d );
		else if( h != 0 )
			str.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );
		else if( m != 0 )
			str.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
		else
			str.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );

		// 출력 갱신
		cPlayerNameCard* card = (cPlayerNameCard*)mpObjectSceneNode->GetNameCard();
		if( card )
		{
			TCHAR buf[128]={0,};
			::_stprintf_s(buf, _T("%s%s"), GAMERESOURCEMAN->GetGameText(34), (LPCTSTR)str.Cstr() );
			card->SetDeleteWaitWord( buf, COLOR_RED );
		}
	}
}

void cDisplayObject::SetDeleteWaitTime( long time )
{ 
	mDeleteWaitTime = time;

	// 클리어
	if( mDeleteWaitTime == LONG_MAX )
	{
		cPlayerNameCard* card = (cPlayerNameCard*)mpObjectSceneNode->GetNameCard();
		if( card )
			card->SetDeleteWaitWord( _T("") );
	}
}

const NiPoint3& cDisplayObject::GetCenterPos()
{
	return mpObjectSceneNode->GetObjectCenter();
}

void cDisplayObject::UpdateDisplayForm( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo )
{
	assert(pBaseInfo);
	assert(pWearInfo);
	assert(pWeaponInfo);

	/// update head info
	ChangeBaseHair( pBaseInfo->HeadInfo[eHEAD_HAIR], pBaseInfo->HeadInfo[eHEAD_HAIRCOLOR] );
	ChangeFaceTexture( pBaseInfo->HeadInfo[eHEAD_FACE], true );
	mPlayerInfo.HeadInfo[eHEAD_FACE] = pBaseInfo->HeadInfo[eHEAD_FACE];

	/// 코스튬 우선
	for( unsigned int ui=eWEAR_BODY1; ui<eWEAR_HAT; ui++ )
	{
		if( pWearInfo->CostumeIdx[ui] == 0 )
		{
			if( pWearInfo->WearIdx[ui] == 0 )
				ChangeBaseWear( ePART_BODY1 + ui );
			else
				EquipWearItem( pWearInfo->WearIdx[ui] );
		}
		else
		{
			EquipWearItem( pWearInfo->CostumeIdx[ui] );

			/// 원피스인 경우 배제..
			if( ui == eWEAR_BODY1 )
			{
				cItemDefine* p = ITEMMAN->GetItemDefine( pWearInfo->CostumeIdx[ui] );
				if( p && p->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
					break;
			}
		}
	}

	/// eWEAR_HAT
	if( pWearInfo->CostumeIdx[eWEAR_HAT] == 0 )
	{
		if( pWearInfo->WearIdx[eWEAR_HAT] != 0 )
			EquipWearItem( pWearInfo->WearIdx[eWEAR_HAT] );
	}
	else
	{
		EquipWearItem( pWearInfo->CostumeIdx[eWEAR_HAT] );
	}

	/// 착용 아이템 설정
	/// 초기화 구문이므로 직접 제어한다..
	EquipWeaponItem( pWeaponInfo );

	for( unsigned char i=eCOSTUME_HEAD; i<eCOSTUME_MAX; i++ )
		EquipAccessoryItem( i, pWearInfo->CostumeAccIdx[i] );
}

void cDisplayObject::Rotate( float angle )
{
	if( mpObjectSceneNode == 0 )
		return;

	NiMatrix3 rot;
	rot.MakeZRotation( angle );

	rot = rot * mpObjectSceneNode->GetRotate();
	mpObjectSceneNode->SetRotate( rot );
}

void cDisplayObject::SetRot( NiMatrix3 mat )
{
	assert(mpObjectSceneNode);
	mpObjectSceneNode->SetRotate( mat );
}


void cDisplayObject::ClearEffect()
{
//	assert(mpObjectSceneNode);
//	mpObjectSceneNode->ClearEffect();
}
