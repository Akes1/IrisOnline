#include "stdafx.h"
#include "SkillObject.h"

#include "ResourceManager.h"

#include "SkillManager.h"
#include "DramaturgyManager.h"
#include "CameraManager.h"

#include "SceneManager.h"
#include "DynamicSceneNode.h"
#include "EffectSceneNode.h"
#include "SoundSceneNode.h"
#include "SkillScript.h"
#include "BulletObj.h"

#include "Application.h"

#include "ModelView.h"
#include "CharacterForm.h"
#include "MonsterForm.h"

//////////////////////////////////////////////////////////////////////////
cDramaturgyObject::cDramaturgyObject()
: mpDramaInfo(0), mManagedKey(0)
{
	mCurrentState = eDRAMASTATE_NONE;

	mRemoved = false;
}

cDramaturgyObject::~cDramaturgyObject()
{
}

void cDramaturgyObject::UpdateDramaObject( sDramaActionBase* pBase, bool bTarget, const NiPoint3& dir )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_ACTION )
		return;

	sDramaAction* pAct = (sDramaAction*)pBase;
	sDramaObj* pobj = mpDramaInfo->GetDramaObj( pAct->objIdx );
	if( pobj == 0 )
		return;

	switch( pobj->kind )
	{
	case eDRAMAOBJ_EFFECT:
		{
			NiTransform trans;
			trans.m_Translate = pAct->var;
			trans.m_fScale = pobj->scale;
			trans.m_Rotate = pobj->rot;

			if( pAct->bFollow == false )
			{
				NiMatrix3 mat = mViewNode->GetRotate();
				trans.m_Rotate = pobj->rot*mat;
			}

			if( dir != -NiPoint3::UNIT_Y )
			{
				/// user -> target 방향으로 회전해야 하는 객체에 대한 회전
				/// 회전 Matrix 산출
				NiPoint3 userDir = -NiPoint3::UNIT_Y;
				NiMatrix3 matA, matB, matRot;
				matA.SetCol( 0, userDir );
				matA.SetCol( 1, -NiPoint3::UNIT_Z );
				matA.SetCol( 2, userDir.UnitCross( -NiPoint3::UNIT_Z ) );

				matB.SetCol( 0, dir );
				matB.SetCol( 1, -NiPoint3::UNIT_Z );
				matB.SetCol( 2, dir.UnitCross( -NiPoint3::UNIT_Z ) );
				matRot = matB * matA.Transpose();

				trans.m_Rotate = matRot * pobj->rot;
			}

			char pathName[256] = {0,};
			::sprintf_s( pathName, "./Data/Effect/%s", pobj->file.Cstr() );

			cEffectSceneNode* pNode = 0;
			if( mViewNode->GetType() == cSceneNode::eMONSTER )
			{
				unsigned int linkPos = pAct->linkPos;
				if( linkPos >= eLINK_RWEAPON )
				{
					assert(0);
					return;
				}
			}

			unsigned int linkPos = pAct->linkPos;
			if( bTarget == true )
			{
				linkPos = UINT_MAX;
			}



			pNode = mViewNode->LinkEffect( linkPos, pathName, &trans, pAct->bLoop, pAct->bFollow );

			if( pNode && pAct->bLoop == true )
				AddLoopEffect( pNode );

		}
		break;
	case eDRAMAOBJ_SOUND:
		{
			/// 사운드 적용
			cSoundSceneNode* pNode = 0;
			pNode = mViewNode->LinkSound( pobj->soundListIndex, pAct->bLoop );

			if( pNode && pAct->bLoop )
			{
				AddLoopSound( pNode );
			}
		}
		break;
	}
}

void cDramaturgyObject::UpdateTrailObject( sDramaActionBase* pBase )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_TRAIL )
		return;

	sTrailAction* pAct = (sTrailAction*)pBase;

	sDramaObj* pobj = mpDramaInfo->GetDramaObj( pAct->texIdx );
	if( pobj == 0 )
		return;
	if( pobj->kind != eDRAMAOBJ_TRAILTEX )
	{
		assert(0);
		return;
	}

	cString str;
	str.Format( "./Data/2DData/%s", pobj->file.Cstr() );
	sTrailInfo* pInfo = mViewNode->ActiveTrailEffect( str, pAct->lifeTime, pAct->linkStart, pAct->linkEnd );
	if( pInfo )
	{
		if( pAct->lifeTime != 0 )
		{
			/// loop 

		}
	}
}

void cDramaturgyObject::AddLoopEffect( cEffectSceneNode* pNode )
{
	if( pNode == 0 )
	{
		assert( pNode );
		return;
	}

	sLoopEffect* p = new sLoopEffect;
	p->mIdxByManager = pNode->GetIndexByManger();
	p->mEffectNode = pNode;

	mApplyController.mLoopEffect.PushBack( p );
}

void cDramaturgyObject::AddLoopSound( cSoundSceneNode* pNode )
{
	if( pNode == 0 )
	{
		assert( pNode );
		return;
	}

	sLoopSound* p = new sLoopSound;
	p->mIdxByManager = pNode->GetIndexByManger();
	p->mSoundNode = pNode;

	mApplyController.mLoopSound.PushBack( p );
}

void cDramaturgyObject::AddLoopTrail( sTrailInfo* pInfo )
{
	mApplyController.mLoopTrail.PushBack( pInfo );
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
cSkillObject::cSkillObject()
: mIsDummy(true)
{
	mCurrentState = eDRAMASTATE_CASTING;
	mSpeedFactor = 1.0f;

	mUsedAnimation = false;

	mPrintType = ePRINT_DAMAGE;

	mFieldPos = NiPoint3::ZERO;

	mSpecialType = eSpecial_None;
	mCastingTime = 0;
}

cSkillObject::~cSkillObject()
{
	FadeOutAllLoopEffect();
	FadeOutAllSoundEffect();
	RemoveAllTrail();

	for( unsigned int i = 0, iend = mTargetObjArray.GetSize(); i < iend; ++i )
	{
		delete (sDamageInfo*)mTargetObjArray[i];
	}
	mTargetObjArray.Clear();


	/// 발사체 제거
	tPointerList<void*>::cIterator k = mBulletList.Begin();
	tPointerList<void*>::cIterator kend = mBulletList.End();
	for( ; k != kend; ++k )
	{
		cBulletObj* pObj = (cBulletObj*)(*k);
		SAFE_DELETE(pObj);
	}
	mBulletList.Clear();
	mDelBulletArray.Clear();
}

void* cSkillObject::operator new(size_t n)
{ 
	n;
	return SKILLMAN->AllocSkillObject();
} 

void cSkillObject::operator delete(void* ptr)
{
	SKILLMAN->FreeSkillObject( (cSkillObject*)ptr );
}

void cSkillObject::FadeOutAllLoopEffect()
{
	/// casting loop effect fadeout
	if( mCastingController.mLoopEffect.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mCastingController.mLoopEffect.Begin();
		tPointerList<void*>::cIterator end = mCastingController.mLoopEffect.End();
		for( ; b != end; ++b )
		{
			sLoopEffect* p = (sLoopEffect*)(*b);
			if( p == 0 )
				continue;

			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mEffectNode );
				node->SetLoopFlag( false );
			}
			delete p;
		}
		mCastingController.mLoopEffect.Clear();
	}

	/// activity loop effect fadeout
	if( mActivityController.mLoopEffect.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mActivityController.mLoopEffect.Begin();
		tPointerList<void*>::cIterator end = mActivityController.mLoopEffect.End();
		for( ; b != end; ++b )
		{
			sLoopEffect* p = (sLoopEffect*)(*b);
			if( p == 0 )
				continue;

			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mEffectNode );
				node->SetLoopFlag( false );
			}
			delete p;
		}
		mActivityController.mLoopEffect.Clear();
	}

	/// apply loop effect fadeout
	if( mApplyController.mLoopEffect.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopEffect.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopEffect.End();
		for( ; b != end; ++b )
		{
			sLoopEffect* p = (sLoopEffect*)(*b);
			if( p == 0 )
				continue;

			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mEffectNode );
				node->SetLoopFlag( false );
			}
			delete p;
		}
		mApplyController.mLoopEffect.Clear();
	}
}

void cSkillObject::FadeOutAllSoundEffect()
{
	/// casting loop sound fadeout
	if( mCastingController.mLoopSound.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mCastingController.mLoopSound.Begin();
		tPointerList<void*>::cIterator end = mCastingController.mLoopSound.End();
		for( ; b != end; ++b )
		{
			sLoopSound* p = (sLoopSound*)(*b);
			if( p == 0 )
				continue;

			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mSoundNode );

				node->FadeOut();
			}
			delete p;
		}
		mCastingController.mLoopSound.Clear();
	}

	/// activity loop sound fadeout
	if( mActivityController.mLoopSound.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mActivityController.mLoopSound.Begin();
		tPointerList<void*>::cIterator end = mActivityController.mLoopSound.End();
		for( ; b != end; ++b )
		{
			sLoopSound* p = (sLoopSound*)(*b);
			if( p == 0 )
				continue;

			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mSoundNode );

				node->FadeOut();
			}
			delete p;
		}
		mActivityController.mLoopSound.Clear();
	}

	/// apply loop sound fadeout
	if( mApplyController.mLoopSound.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopSound.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopSound.End();
		for( ; b != end; ++b )
		{
			sLoopSound* p = (sLoopSound*)(*b);
			if( p == 0 )
				continue;

			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mSoundNode );

				node->FadeOut();
			}
			delete p;
		}
		mApplyController.mLoopSound.Clear();
	}
}

void cSkillObject::RemoveAllTrail()
{
	/// casting loop sound fadeout
	if( mCastingController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mCastingController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mCastingController.mLoopTrail.End();
		for( ; b != end; ++b )
		{
			sTrailInfo* pInfo = (sTrailInfo*)(*b);
			mViewNode->DeActiveTrailEffect( pInfo );
		}
		mCastingController.mLoopTrail.Clear();
	}

	/// activity loop sound fadeout
	if( mActivityController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mActivityController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mActivityController.mLoopTrail.End();
		for( ; b != end; ++b )
		{
			sTrailInfo* pInfo = (sTrailInfo*)(*b);
			mViewNode->DeActiveTrailEffect( pInfo );
		}
		mActivityController.mLoopTrail.Clear();
	}

	/// apply loop sound fadeout
	if( mApplyController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopTrail.End();
		for( ; b != end; ++b )
		{
			sTrailInfo* pInfo = (sTrailInfo*)(*b);
			mViewNode->DeActiveTrailEffect( pInfo );
		}
		mApplyController.mLoopTrail.Clear();
	}
}

bool cSkillObject::Init( unsigned long skillIdx, float speedFactor )
{
	mSpeedFactor = speedFactor;
	if( mSpeedFactor <= 0.0f || mSpeedFactor > 2.0f )
	{
		assert(0);
		mSpeedFactor = 1.0f;
	}

	mSkillIdx = skillIdx;
	mRemoved = false;
	mUserWeapon = eWEAPON_STATE_NONE;

	mVerifyState = eVerify_Wait;

	bool bCasting = false;

	mViewNode = SCENEMAN->GetCurrentSceneNode();
	if( mViewNode->GetType() == cSceneNode::ePLAYER )
	{
		mUserWeapon = CHARACTERFORM->GetCurrentWeaponState();

		unsigned char state = mUserWeapon;
		if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
		{
			state = (unsigned char)((skillIdx%10) - 1);
		}

		mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, state, 
			CHARACTERFORM->GetCurrentRace(), CHARACTERFORM->GetCurrentGender() );
		if( mpDramaInfo == 0 )
		{
			assert(0);
			return false;
		}

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
		if( pInfo == 0 )
			return false;

		if( pInfo->mType == eSKILLTYPE_NONACTIVE )
		{
			unsigned long infIdx = pInfo->mpSetpInfoArray[0].mInfulenceIdx;
			cInfluenceObject* pInfluence = SKILLMAN->CreateInfluenceObject( 0, infIdx );
			if( pInfluence )
				pInfluence->StartProcess( 10 );
		}

		if( pInfo->mShotType == eSHOTTYPE_CASTING )
			bCasting = true;

		if( pInfo->mShotType == eSHOTTYPE_BLINK )
			mSpecialType = eSpecial_Blink;
		
		mCastingTime = pInfo->mpSetpInfoArray[0].mCastingTime;
	}
	else
	{
		unsigned long classId = MONSTERFORM->GetCurrentMonsterIdx();
		mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
		if( mpDramaInfo == 0 )
		{
			assert(0);
			return false;
		}

		sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
		if( pSkillInfo->mType == eSKILLTYPE_NONACTIVE )
		{
			unsigned long infIdx = pSkillInfo->mInfulenceIdx;
			cInfluenceObject* pInfluence = SKILLMAN->CreateInfluenceObject( 0, infIdx );
			if( pInfluence )
				pInfluence->StartProcess( 10 );
		}

		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
			bCasting = true;

		if( pSkillInfo->mShotType == eSHOTTYPE_BLINK )
			mSpecialType = eSpecial_Blink;

		mCastingTime = pSkillInfo->mCastingTime;
	}

	/// data thread loading (hero 데이터는 이미 로딩 되어 있다)

	/// 연출을 위한 정보를 참조
	mCastingController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_CASTING );
	mCastingController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_CASTING );
	mCastingController.startTime = ULONG_MAX;

	mActivityController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_ACTIVITY );
	mActivityController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_ACTIVITY );
	mActivityController.startTime = ULONG_MAX;

	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;


	/// 예외 체크
	assert(mActivityController.pAct);
	if( mCastingController.pAct == 0 && mCastingController.animationIdx == UINT_MAX )
	{
		if( bCasting == true )
		{
			assert(0);
		}

		ChangeDramaState( eDRAMASTATE_ACTIVITY );
	}
	else
	{
		if( bCasting == false )
		{
			assert(0);
		}
	}

	return true;
}

void cSkillObject::AddLoopEffect( cEffectSceneNode* pNode )
{
	if( pNode == 0 )
	{
		assert( pNode );
		return;
	}

	sLoopEffect* p = new sLoopEffect;
	p->mIdxByManager = pNode->GetIndexByManger();
	p->mEffectNode = pNode;

	switch( mCurrentState )
	{
	case eDRAMASTATE_CASTING:
		mCastingController.mLoopEffect.PushBack( p );
		break;
	case eDRAMASTATE_ACTIVITY:
		mActivityController.mLoopEffect.PushBack( p );
		break;
	case eDRAMASTATE_APPLY:
		mApplyController.mLoopEffect.PushBack( p );
		break;
	default:
		assert(0);
		return;
	}
}

void cSkillObject::AddLoopSound( cSoundSceneNode* pNode )
{
	if( pNode == 0 )
	{
		assert( pNode );
		return;
	}

	sLoopSound* p = new sLoopSound;
	p->mIdxByManager = pNode->GetIndexByManger();
	p->mSoundNode = pNode;

	switch( mCurrentState )
	{
	case eDRAMASTATE_CASTING:
		mCastingController.mLoopSound.PushBack( p );
		break;
	case eDRAMASTATE_ACTIVITY:
		mActivityController.mLoopSound.PushBack( p );
		break;
	case eDRAMASTATE_APPLY:
		mApplyController.mLoopSound.PushBack( p );
		break;
	default:
		assert(0);
		return;
	}
}

void cSkillObject::AddLoopTrail( sTrailInfo* pInfo )
{
	switch( mCurrentState )
	{
	case eDRAMASTATE_CASTING:
		mCastingController.mLoopTrail.PushBack( pInfo );
		break;
	case eDRAMASTATE_ACTIVITY:
		mActivityController.mLoopTrail.PushBack( pInfo );
		break;
	case eDRAMASTATE_APPLY:
		mApplyController.mLoopTrail.PushBack( pInfo );
		break;
	default:
		assert(0);
		return;
	}

}

void cSkillObject::ChangeDramaState( eDRAMASTATE state )
{
	if( state == eDRAMASTATE_ACTIVITY || state == eDRAMASTATE_CANCEL_WAIT )
	{
		/// casting loop effect clear
		if( mCastingController.mLoopEffect.IsEmpty() == false )
		{
			tPointerList<void*>::cIterator b = mCastingController.mLoopEffect.Begin();
			tPointerList<void*>::cIterator end = mCastingController.mLoopEffect.End();
			for( ; b != end; ++b )
			{
				sLoopEffect* p = (sLoopEffect*)(*b);
				if( p == 0 )
					continue;

				cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
				if( node )
				{
					assert( node == p->mEffectNode );
					node->SetLoopFlag( false );
				}
				delete p;
			}
			mCastingController.mLoopEffect.Clear();
		}

		/// casting loop sound clear
		if( mCastingController.mLoopSound.IsEmpty() == false )
		{
			tPointerList<void*>::cIterator b = mCastingController.mLoopSound.Begin();
			tPointerList<void*>::cIterator end = mCastingController.mLoopSound.End();
			for( ; b != end; ++b )
			{
				sLoopSound* p = (sLoopSound*)(*b);
				if( p == 0 )
					continue;

				cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( p->mIdxByManager );
				if( node )
				{
					assert( node == p->mSoundNode );
					node->FadeOut();
				}
				delete p;
			}
			mCastingController.mLoopSound.Clear();
		}

		if( mCastingController.mLoopTrail.IsEmpty() == false )
		{
			tPointerList<void*>::cIterator b = mCastingController.mLoopTrail.Begin();
			tPointerList<void*>::cIterator end = mCastingController.mLoopTrail.End();
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				mViewNode->DeActiveTrailEffect( pInfo );
			}
			mCastingController.mLoopTrail.Clear();
		}
	}
	mCurrentState = state;
}

void cSkillObject::WaitCancel()
{
	if( mCurrentState == eDRAMASTATE_CASTING )
		ChangeDramaState( eDRAMASTATE_CANCEL_WAIT );
}

void cSkillObject::AddBulletObject( sDramaActionBase* pBase )
{
	if( pBase == 0 )
		return;
	if( pBase->actionType != eDRAMA_BULLET )
		return;

	cBulletObj* pObj = new cBulletObj;
	if( pObj->Init( mpDramaInfo, (sBulletAction*)pBase, mViewNode ) == false )
	{
		delete pObj;
		return;
	}
	mBulletList.PushBack( pObj );
}

void cSkillObject::BulletProcess( unsigned long time )
{
	tPointerList<void*>::cIterator i = mBulletList.Begin();
	tPointerList<void*>::cIterator end = mBulletList.End();
	for( ; i != end; ++i )
	{
		cBulletObj* pObj = (cBulletObj*)(*i);
		if( pObj->Update( time ) == false )
		{
			/// 삭제 등록
			mDelBulletArray.PushBack( pObj );
		}
	}

	/// 삭제 처리
	if( mDelBulletArray.IsEmpty() )
		return;

	for( unsigned int i=0; i<mDelBulletArray.GetSize(); ++i )
	{
		cBulletObj* pObj = (cBulletObj*)mDelBulletArray[i];

		delete pObj;
		mBulletList.Remove( pObj );
	}
	mDelBulletArray.Clear();
}

void cSkillObject::Process( unsigned long time )
{
	/// 발사체 Process
	BulletProcess( time );

	/// 상태에 따른 연출 처리
	switch( mCurrentState )
	{
	case eDRAMASTATE_CASTING:
		ProcessCasting( time );
		break;
	case eDRAMASTATE_ACTIVITY:
		ProcessActivity( time );
		break;
	case eDRAMASTATE_APPLY:
		ProcessApply( time );
		/// apply처리
		break;
	case eDRAMASTATE_CANCEL_WAIT:
		break;
	default:
		assert(0);
	}

	if( mActivityController.pAct == 0 && mApplyController.pAct == 0 )
	{
		/// 만약 발사체 처리가 끝나지 않았다면..
		/// 발사체 속도를 증가시켜서 빠르게 종료 시킬것..
		if( mBulletList.IsEmpty() == false )
		{
			tPointerList<void*>::cIterator i = mBulletList.Begin();
			tPointerList<void*>::cIterator end = mBulletList.End();
			for( ; i != end; ++i )
			{
				cBulletObj* pObj = (cBulletObj*)(*i);
				pObj->SpeedUp( 20.0f );
			}
			return;
		}

		/// 스킬 객체 제거
		SKILLMAN->DeleteSkillObject( this );
	}
}

void cSkillObject::ProcessCasting( unsigned long time )
{
	if( mVerifyState == eVerify_Faile )
		return;

	/// 초기 설정
	if( mCastingController.startTime == ULONG_MAX )
	{
		mCastingController.startTime = time;

		/// 애니메이션 적용
		if( mCastingController.animationIdx != UINT_MAX )
		{
			mViewNode->UpdateTargetAnimation( mCastingController.animationIdx );
			mUsedAnimation = true;
		}
	}

	sDramaActionBase* pAct = mCastingController.pAct;
	if( pAct == 0 )
	{
		unsigned long delta = time - mCastingController.startTime;
		if( mSpecialType > eSpecial_Field )
		{
			if( delta >= mCastingTime )
				EndCast();
		}
		else
		{
			if( mCastingController.startTime == FLT_MAX )
			{
				EndCast();
			}
			else
			{
				if( delta >= mCastingTime )
				{
					EndCast();
				}
			}
		}
		return;
	}

	unsigned long delta = time - mCastingController.startTime;
	while( pAct )
	{
		unsigned long checkTime = pAct->time;
		if( delta >= checkTime )
		{
			mCastingController.pAct = pAct->pNext;

			if( pAct->actionType == eDRAMA_TRAIL )
			{
				UpdateTrailObject( pAct );
			}
			else
			{
				UpdateDramaObject( pAct );
			}
		}
		else
			break;

		pAct = mCastingController.pAct;
	}
}

void cSkillObject::ProcessActivity( unsigned long time )
{
	sDramaActionBase* pAct = mActivityController.pAct;

	/// 초기 설정
	if( mActivityController.startTime == ULONG_MAX )
	{
		mActivityController.startTime = time;

		/// 애니메이션 적용
		if( mActivityController.animationIdx != UINT_MAX )
		{
			mViewNode->UpdateTargetAnimation( mActivityController.animationIdx );
			mUsedAnimation = true;
		}
	}

	unsigned long delta = (unsigned long)((time - mActivityController.startTime) * mSpeedFactor);
	while( pAct )
	{
		unsigned long checkTime = pAct->time;
		if( delta >= checkTime )
		{
			mActivityController.pAct = pAct->pNext;

			/// 처리부 
			if( pAct->actionType == eDRAMA_TRAIL )
			{
				UpdateTrailObject( pAct );
			}
			else if( pAct->actionType == eDRAMA_ACTIVE )
			{
				ChangeDramaState( eDRAMASTATE_APPLY );
				break;
			}
			else if( pAct->actionType == eDRAMA_BULLET )
			{
				AddBulletObject( pAct );
				ChangeDramaState( eDRAMASTATE_APPLY );
				break;
			}
			else
			{
				UpdateDramaObject( pAct );
			}
			///

			/// 예외 처리
			if( mActivityController.pAct == 0 )
			{
				/// 발동 -> 적용으로 못넘아 갔다면 체크 하시오.
				assert(0);
				break;
			}
		}
		else
			break;

		pAct = mActivityController.pAct;
	}
}

/// 시전자와 적용자의 처리를 동시에 진행
void cSkillObject::ProcessApply( unsigned long time )
{
	//////////////////////////////////////////////////////////////////////////
	/// 시전자 행동 처리
	sDramaActionBase* pAct = mActivityController.pAct;
	if( pAct )
	{
		if( mActivityController.startTime == ULONG_MAX )
		{
			mActivityController.startTime = time;

			/// 애니메이션 적용
			if( mActivityController.animationIdx != UINT_MAX )
			{
				mViewNode->UpdateTargetAnimation( mActivityController.animationIdx );
				mUsedAnimation = true;
			}
		}

		unsigned long delta = (unsigned long)((time - mActivityController.startTime) * mSpeedFactor);
		while( pAct )
		{
			unsigned long checkTime = pAct->time;
			if( delta >= checkTime )
			{
				mActivityController.pAct = pAct->pNext;

				/// 처리부
				if( pAct->actionType == eDRAMA_TRAIL )
				{
					UpdateTrailObject( pAct );
				}
				else if( pAct->actionType == eDRAMA_ACTION )
				{
					UpdateDramaObject( pAct );
				}
			}
			else
				break;

			pAct = mActivityController.pAct;
		}
	}

	pAct = mApplyController.pAct;
	if( pAct )
	{
		if( mApplyController.startTime == ULONG_MAX )
		{
			mApplyController.startTime = time;
		}

		unsigned long delta = (unsigned long)((time - mApplyController.startTime) * mSpeedFactor);
		while( pAct )
		{
			unsigned long checkTime = pAct->time;
			if( delta >= checkTime )
			{
				mApplyController.pAct = pAct->pNext;

				/// 처리부
				if( pAct->actionType == eDRAMA_DAMAGE )
				{
					/// print type에 따라 별도의 처리가 필요..
					if( mPrintType == ePRINT_DAMAGE )
					{
						ApplyDamage( pAct );
					}
					else if( mPrintType == ePRINT_HEAL )
					{
						ApplyHeal( pAct );
					}
					else
					{
						/// 별도의 처리가 필요하다면 체크하자..
					}
				}
				else
				{
					if( pAct->actionType == eDRAMA_TRAIL )
						UpdateTrailObject( pAct );
					else
					{
						UpdateDramaObject( pAct, true );
					}
				}
				///
			}
			else
				break;

			pAct = mApplyController.pAct;
		}
	}
}

void cSkillObject::ApplyDamage( const sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;

	SKILLMAN->CreateApplyObject( mUserWeapon, eAPPLYDRAMA_DAMAGE );

	if( mViewNode->GetType() == cSceneNode::ePLAYER )
	{
		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
		if( pInfo == 0 )
			return;

		unsigned long infIdx = pInfo->mpSetpInfoArray[0].mInfulenceIdx;
		cInfluenceObject* pInfluence = SKILLMAN->CreateInfluenceObject( 0, infIdx );
		if( pInfluence )
			pInfluence->StartProcess( 5 );
	}
	else
	{

		unsigned long classId =MONSTERFORM->GetCurrentMonsterIdx();
		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)mSkillIdx );
		if( pInfo == 0 )
			return;

		unsigned long infIdx = pInfo->mInfulenceIdx;
		cInfluenceObject* pInfluence = SKILLMAN->CreateInfluenceObject( 0, infIdx );
		if( pInfluence )
			pInfluence->StartProcess( 5 );


	}
}

void cSkillObject::ApplyHeal( const sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;
}

void cSkillObject::EndCast()
{
	if( mCurrentState != eDRAMASTATE_CASTING && mCurrentState != eDRAMASTATE_CANCEL_WAIT )
	{
		assert(0);
		return;
	}

	mViewNode->ClearAllEffect();
	ChangeDramaState( eDRAMASTATE_ACTIVITY );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


cInfluenceObject::cInfluenceObject()
{
	mCurrentState = eDRAMASTATE_APPLY;

	mCheckBlink = false;
	mBlinkTime = 0;

	mStartTimeCheck = false;

	mIsHeroInfluce = false;
	mOnlyShowLoopEffect = false;

	mTickInit = false;
	mTickInfluence = false;

	mStaticInfluence = false;
}

cInfluenceObject::~cInfluenceObject()
{
	/// Apply loop effect clear
	if( mApplyController.mLoopEffect.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopEffect.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopEffect.End();
		for( ; b != end; ++b )
		{
			sLoopEffect* p = (sLoopEffect*)(*b);
			if( p == 0 )
				continue;

			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mEffectNode );
				node->SetLoopFlag( false );
			}
			delete p;
		}
		mApplyController.mLoopEffect.Clear();
	}

	/// Apply loop sound clear
	if( mApplyController.mLoopSound.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopSound.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopSound.End();
		for( ; b != end; ++b )
		{
			sLoopSound* p = (sLoopSound*)(*b);
			if( p == 0 )
				continue;

			cSoundSceneNode* node = SCENEMAN->GetSoundSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mSoundNode );
				node->FadeOut();
			}
			delete p;
		}
		mApplyController.mLoopSound.Clear();
	}
}

void* cInfluenceObject::operator new(size_t n)
{ 
	n;
	return SKILLMAN->AllocInfluenceObject();
} 

void cInfluenceObject::operator delete(void* ptr)
{
	SKILLMAN->FreeInfluenceObject( (cInfluenceObject*)ptr );
}

bool cInfluenceObject::Init( unsigned long uniqueIdx, unsigned long influenceIdx, bool isbuff )
{
	/// 081015 PKH 수정
	mManagedKey = uniqueIdx;
	mInfluenceIdx = influenceIdx;
	mIsBuff = isbuff;
	sInfluenceScript* p = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( p == 0 )
	{
		assert(0);
		return false;
	}

	/// 효과 타입을 저장한다.
	mInfluenceType = p->mTypeDetail;

	if( p->mContinuanceTime < 0 )
	{
		mStaticInfluence = true;

		mRestTime = 0;
		mLifeTime = 0;
	}
	else
	{
		mStaticInfluence = false;

		mRestTime = (unsigned long)p->mContinuanceTime;
		mLifeTime = (unsigned long)p->mContinuanceTime;
	}

	mRemoved = false;

	mViewNode = SCENEMAN->GetCurrentSceneNode();
	
	/// 연출 획득
	mpDramaInfo =DRAMATURGYMAN->GetInfluenceDramaturgyInfo( influenceIdx );
	if( mpDramaInfo == 0 )
	{
		/// 연출이 없어도 아이콘등 진행은 하자!!
		assert(0);
		return true;
	}

	/// 연출 정보 셋팅
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	return true;
}

void cInfluenceObject::StartProcess( long restTime, bool onlyLoopEffect )
{
	mOnlyShowLoopEffect = onlyLoopEffect;

	if( mStartTimeCheck == true )
		return;

	mStartTimeCheck = true;

	mStartTime = THEAPP->GetAccumTime();

	if( restTime > 0 )
	{
		mRestTime = restTime;
		mLifeTime = restTime;
	}

	mApplyController.startTime = THEAPP->GetAccumTime();
}

void cInfluenceObject::SetTickInfo( unsigned char tickType, long value )
{
	mTickType = tickType;
	mTickValue = (float)value;
	mTickInfluence = true;

	/// 
	if( mpDramaInfo == 0 )
		return;

	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;
}

void cInfluenceObject::Process( unsigned long time )
{
	if( mStartTimeCheck == false )
		return;

	sDramaActionBase* pAct = mApplyController.pAct;
	if( pAct )
	{
		if( mApplyController.startTime == ULONG_MAX )
		{
			mApplyController.startTime = time;
		}

		unsigned long delta = time - mApplyController.startTime;
		while( pAct )
		{
			if( delta >= pAct->time )
			{
				mApplyController.pAct = pAct->pNext;

				if( pAct->actionType == eDRAMA_DAMAGE )
				{
					if( mTickInit )
					{
						/// heal or damage 처리
						if( mTickType == eTick_HP )
						{
							ApplyHP( pAct );
						}
						else if( mTickType == eTick_MP )
						{
							ApplyMP( pAct );
						}
						else
						{
							assert(0);
						}
					}
				}
				else if( pAct->actionType == eDRAMA_TRAIL ) 
				{
					UpdateTrailObject( pAct );
				}
				else if( pAct->actionType == eDRAMA_ACTION )
				{
					sDramaAction* p = (sDramaAction*)pAct;

					if( mTickInit )
					{
						if( p->bLoop == false )
							UpdateDramaObject( pAct, true );
					}
					else
					{
						if( mOnlyShowLoopEffect == true )
						{
							/// 진행형을 등록하는 경우는 반복효과만 연출한다.
							if( p->bLoop == true )
								UpdateDramaObject( pAct, true );
						}
						else
						{
							UpdateDramaObject( pAct, true );
						}
					}
				}
			}
			else
				break;

			pAct = mApplyController.pAct;
			if( pAct == 0 )
			{
				mTickInit = true;
			}
		}
	}

	if( mStaticInfluence == false )
	{
		/// 효과 객체의 삭제 주기를 계산한다.
		if( mLifeTime < (time - mStartTime) )
			mRestTime = 0;
		else
			mRestTime = mLifeTime - (time - mStartTime);

		if( mTickInfluence == false )
		{
			if( mRestTime <= 0 )
			{
				mRestTime = 0;
				/// 스킬 객체 제거
				SKILLMAN->DeleteInfluenceObject( this );
			}
		}
		else
		{
			if( time - mStartTime > 5000 * mLifeTime )
			{
				assert(0);
			}
		}
	}
}

void cInfluenceObject::ApplyHP( sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;
}

void cInfluenceObject::ApplyMP( sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

cApplyObject::cApplyObject()
{
	mCurrentState = eDRAMASTATE_APPLY;
	mUserWeapon = 0;
}

cApplyObject::~cApplyObject()
{
	/// Apply loop Trail clear
	if( mApplyController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopTrail.End();
		for( ; b != end; ++b )
		{
			sTrailInfo* pInfo = (sTrailInfo*)(*b);
			mViewNode->DeActiveTrailEffect( pInfo );
		}
		mApplyController.mLoopTrail.Clear();
	}
}

void* cApplyObject::operator new( size_t n )
{
	n;
	return SKILLMAN->AllocApplyObject();
}

void cApplyObject::operator delete( void* ptr )
{
	SKILLMAN->FreeApplyObject( (cApplyObject*)ptr );
}

bool cApplyObject::Init( unsigned long managedKey, unsigned char weaponType, eAPPLYDRAMA_TYPE type )
{
	if( type >= eAPPLYDRAMA_MAX )
		return false;

	mUserWeapon = weaponType;
	mIndexType = eINDEX_DRAMA;
	mApplyIndex = type;
	mRemoved = false;
	mManagedKey = managedKey;

	mViewNode = SCENEMAN->GetCurrentSceneNode();

	if( VIEW->GetCurStage() == CModelView::eSTAGE_CHARACTER )
	{
		mpDramaInfo = DRAMATURGYMAN->GetPlayerApplyDramaInfo( CHARACTERFORM->GetCurrentRace(), CHARACTERFORM->GetCurrentGender(), type );
		if( mpDramaInfo == 0 )
		{
			assert(0);
			return false;
		}
	}
	else
	{
		mpDramaInfo = DRAMATURGYMAN->GetMonsterApplyDramaInfo( MONSTERFORM->GetCurrentMonsterIdx(), type );
		if( mpDramaInfo == 0 )
		{
			assert(0);
			return false;
		}
	}

	/// 연출을 위한 정보를 참조
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	return true;
}

bool cApplyObject::Init( unsigned long managedKey, unsigned long influenceIdx )
{

	mManagedKey = managedKey;

	mIndexType = eINDEX_INFLUENCE;
	mApplyIndex = influenceIdx;

	mRemoved = false;

	mViewNode = SCENEMAN->GetCurrentSceneNode();

	/// 연출 획득
	mpDramaInfo =DRAMATURGYMAN->GetInfluenceDramaturgyInfo( influenceIdx );
	if( mpDramaInfo == 0 )
	{
		assert(0);
		return false;
	}

	/// 연출 정보 셋팅
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	return true;
}

void cApplyObject::Process( unsigned long time )
{
	sDramaActionBase* pAct = mApplyController.pAct;
	if( pAct )
	{
		if( mApplyController.startTime == ULONG_MAX )
		{
			mApplyController.startTime = time;
		}

		unsigned long delta = time - mApplyController.startTime;
		while( pAct )
		{
			unsigned long checkTime = pAct->time;
			if( delta >= checkTime )
			{
				mApplyController.pAct = pAct->pNext;

				if( pAct->actionType == eDRAMA_TRAIL )
					UpdateTrailObject( pAct );
				else
				{
					if( ((sDramaAction*)pAct)->weaponType == mUserWeapon )
						UpdateDramaObject( pAct, true );
				}
				///
			}
			else
				break;

			pAct = mApplyController.pAct;
		}
	}

	if( mApplyController.pAct == 0 )
	{
		/// 스킬 객체 제거
		SKILLMAN->DeleteApplyObject( this );
	}
}
//////////////////////////////////////////////////////////////////////////
