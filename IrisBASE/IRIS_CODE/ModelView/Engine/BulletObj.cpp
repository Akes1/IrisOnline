#include "StdAfx.h"
#include "bulletobj.h"

#include "ResourceManager.h"

#include "DramaturgyInfo.h"
#include "EffectSceneNode.h"
#include "DynamicSceneNode.h"
#include "RangeCheck.h"

#include "Application.h"
#include "SceneManager.h"
#include "ObjectManager.h"

cBulletObj::cBulletObj()
{
	mpDramaInfo = 0;
	mBullet = 0;
	mBulletEffectIdx = (unsigned long)-1;
	mDesiredDir = -NiPoint3::UNIT_Y;

	mAccumTime = 0;

	mExistCrashEffect = false;
	mCrashCount = 0;

	mUseFieldTarget = false;
	mLastTargetPos = NiPoint3::ZERO;
}

cBulletObj::~cBulletObj()
{
	mpDramaInfo = 0;

	if( mBulletEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mBulletEffectIdx );
		if( node )
		{
			assert( node == mBullet );
			node->Remove();
		}
	}

	mBullet = 0;
	mBulletEffectIdx = (unsigned long)-1;
}

bool cBulletObj::Init( cDramaturgyInfo* pInfo, sBulletAction* pAct, cDynamicSceneNode* pNode )
{
	/// 에러 처리
	if( pInfo == 0 )
		return false;
	if( pAct == 0 )
		return false;

	mpDramaInfo = pInfo;

	/// 충돌 효과
	mExistCrashEffect = pAct->existCrash;

	mCrashCount = pAct->crashCount;
	if( mCrashCount > 5 )
	{
		assert(0);
		mCrashCount = 5;
	}
	::memcpy( mCrashInfo, pAct->crashInfo, sizeof(sCrashInfo)*mCrashCount );

	mSpeed = pAct->speed;

	/// 발사체 시작 / 종료 위치 획득
	NiPoint3 startPos;
	if( pNode->GetType() == cSceneNode::eMONSTER )
	{
		if( pAct->linkPos >= eLINK_RWEAPON )
		{
			assert(0);
			return false;
		}

		NiAVObject* obj = pNode->GetPartObject( pAct->linkPos );
		if( obj == 0 )
			return false;

		startPos = obj->GetWorldTransform().m_Translate;
	}
	else
	{
		NiAVObject* obj = pNode->GetPartObject( ePART_MAX + pAct->linkPos );
		if( obj == 0 )
			return false;

		startPos = obj->GetWorldTransform().m_Translate;
	}

	sDramaObj* pobj = mpDramaInfo->GetDramaObj( pAct->bulletEffectIdx );
	if( pobj == 0 )
		return false;

	NiTransform trans;
	trans.m_Translate = startPos + pAct->bulletVar;
	trans.m_Rotate = pobj->rot;
	trans.m_fScale = pobj->scale;

	char pathName[256] = {0,};
	::sprintf_s( pathName, "./Data/Effect/%s", pobj->file.Cstr() );

	/// 객체 생성
	mBullet = SCENEMAN->CreateSelfEffect( pathName, true, trans, true );
	if( mBullet == 0 )
		return false;

	mBulletEffectIdx = mBullet->GetIndexByManger();

	mAccumTime = THEAPP->GetAccumTime();

	mLastTargetPos = SCENEMAN->GetSceneDummy()->GetWorldTranslate();

	return true;
}

bool cBulletObj::Update( unsigned long time )
{
	/// 객체를 제거 하기 위해서 false를 보낸다.
	if( mBullet == 0 )
		return false;

	/// 진행 방향 산출
	NiPoint3 pos = mBullet->GetWorldTranslate();

	/// 충돌객체 연출 방향
	NiPoint3 crashDir = mLastTargetPos - pos;
	crashDir.z = 0.0f;
	crashDir.Unitize();
	NiPoint3 dir = mLastTargetPos - pos;
	if( dir == NiPoint3::ZERO )
	{
		Crash( -crashDir );
		return false;
	}
	if( dir.SqrLength() < 0.001f )	
	{
		Crash( -crashDir );
		return false;
	}
	dir.Unitize();
	/// 진행 방향 산출 종료

	/// 회전값 계산 & 적용
	BulletRotate( mDesiredDir, dir );
	mDesiredDir = dir;
	/// 회전값 계산 & 적용 종료

	/// 이동값 계산 & 적용
	float deltaTime = (float)(time - mAccumTime) * 0.001f;
	mAccumTime = time;

	NiPoint3 calcPos = pos + (mSpeed*deltaTime*dir);
	mBullet->SetTranslate( calcPos );
	/// 이동값 계산 & 적용 종료

	if( (calcPos - pos).Length()  > (mLastTargetPos - pos).Length() )
	{
		/// 충돌 이펙트 출력후 제거
		Crash( -crashDir );
		return false;
	}

	/// 충돌 처리
	cRangeCheck rangecheck( 40.0f );
	if( rangecheck.IsRange( calcPos, mLastTargetPos ) )
	{
		/// 충돌 이펙트 출력후 제거
		Crash( -crashDir );
		return false;
	}
	///

	return true;
}

void cBulletObj::Crash( NiPoint3 dir )
{
	if( mExistCrashEffect == false )
		return;

	NiMatrix3 matRot = NiMatrix3::IDENTITY;
	if( dir != -NiPoint3::UNIT_Y )
	{
		/// user -> target 방향으로 회전해야 하는 객체에 대한 회전
		/// 회전 Matrix 산출
		NiPoint3 userDir = -NiPoint3::UNIT_Y;
		NiMatrix3 matA, matB;
		matA.SetCol( 0, userDir );
		matA.SetCol( 1, -NiPoint3::UNIT_Z );
		matA.SetCol( 2, userDir.UnitCross( -NiPoint3::UNIT_Z ) );

		matB.SetCol( 0, dir );
		matB.SetCol( 1, -NiPoint3::UNIT_Z );
		matB.SetCol( 2, dir.UnitCross( -NiPoint3::UNIT_Z ) );
		matRot = matB * matA.Transpose();
	}


	assert(mCrashCount<=5);
	for( unsigned int i=0; i<mCrashCount; i++ )
	{
		sDramaObj* pobj = mpDramaInfo->GetDramaObj( mCrashInfo[i].crashEffectIdx );
		if( pobj )
		{
			NiTransform trans;
			trans.m_Translate = mCrashInfo[i].crashVar;
			trans.m_Rotate = matRot*pobj->rot;
			trans.m_fScale = pobj->scale;

			char pathName[256] = {0,};
			::sprintf_s( pathName, "./Data/Effect/%s", pobj->file.Cstr() );

			SCENEMAN->GetCurrentSceneNode()->LinkEffect( UINT_MAX, pathName, &trans, false, false );
		}
	}
}

void cBulletObj::BulletRotate( NiPoint3 beforeDir, NiPoint3 afterDir )
{
	NiMatrix3 matA, matB, matRot;
	NiPoint3 kUp, kRight;
	float fDot;
	if (((beforeDir.z < 1.001f) && (beforeDir.z > 0.999f)) ||
		((beforeDir.z > -1.001f) && (beforeDir.z < -0.999f)))
	{
		// kUp & kAt are too close - use the Y axis as an alternate kUp
		kUp = NiPoint3::UNIT_Y;
		fDot = beforeDir.y;
	}
	else
	{
		kUp = NiPoint3::UNIT_Z;
		fDot = beforeDir.z;
	}

	kUp -= fDot*beforeDir;
	kUp.Unitize();

	kRight = kUp.Cross(beforeDir);

	matA.SetCol( 0, beforeDir );
	matA.SetCol( 1, kUp);
	matA.SetCol( 2, -kRight );

	if (((afterDir.z < 1.001f) && (afterDir.z > 0.999f)) ||
		((afterDir.z > -1.001f) && (afterDir.z < -0.999f)))
	{
		// kUp & kAt are too close - use the Y axis as an alternate kUp
		kUp = NiPoint3::UNIT_Y;
		fDot = afterDir.y;
	}
	else
	{
		kUp = NiPoint3::UNIT_Z;
		fDot = afterDir.z;
	}

	kUp -= fDot*afterDir;
	kUp.Unitize();

	kRight = kUp.Cross(afterDir);

	matB.SetCol( 0, afterDir );
	matB.SetCol( 1, kUp);
	matB.SetCol( 2, -kRight );

	matRot = matB * matA.Transpose();
	mBullet->SetRotate( mBullet->GetRotate()*matRot );
}
