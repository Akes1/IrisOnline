#include "stdafx.h"
#include "SkillObject.h"

#include "SkillManager.h"
#include "DramaturgyManager.h"
#include "CameraManager.h"

#include "SceneManager.h"
#include "EffectSceneNode.h"
#include "SoundSceneNode.h"
#include "SkillScript.h"
#include "BulletObj.h"

#include "Application.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Monster.h"

#include "UIManager.h"
#include "CastingBar.h"

#include "WorldManager.h"

#include "DamagePrintManager.h"

//////////////////////////////////////////////////////////////////////////
cDramaturgyObject::cDramaturgyObject()
: mpDramaInfo(0), mManagedKey(0)
{
	mCurrentState = eDRAMASTATE_NONE;

	mRemoved = false;

	mSkillInfo.skillIdx = 0;
	mSkillInfo.skillStep = 0;
}

cDramaturgyObject::~cDramaturgyObject()
{
}

void cDramaturgyObject::UpdateDramaObject( sDramaActionBase* pBase, cBaseObject* pChar, const NiPoint3& dir )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_ACTION )
		return;
	if( pChar == 0 ) 
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
				NiMatrix3 mat = pChar->GetModelRot();
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
			if( pChar->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				if( pAct->linkPos >= eLINK_RWEAPON )
				{
					assert(0);
					return;
				}
			}

			pNode = pChar->SetLinkdEffect( pAct->linkPos, pathName, &trans, pAct->bLoop, pAct->bFollow );

			if( pNode && pAct->bLoop == true )
				AddLoopEffect( pNode );
		}
		break;
	case eDRAMAOBJ_SOUND:
		{
			/// 사운드 적용
			cSoundSceneNode* pNode = 0;
			pNode = pChar->SetLinkdSound( pobj->soundListIndex, pAct->bLoop );

			if( pNode && pAct->bLoop )
			{
				AddLoopSound( pNode );
			}
		}
		break;
	}
}

void cDramaturgyObject::UpdateChannelingDramaObject( sDramaActionBase* pBase, cBaseObject* pChar, const NiPoint3& fieldPos )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_CHANNELING )
		return;
	if( pChar == 0 ) 
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
			NiPoint3 charPos = pChar->GetPos();
			charPos = fieldPos - charPos;

			trans.m_Translate = charPos; ///pAct->var;
			trans.m_fScale = pobj->scale;
			trans.m_Rotate = pobj->rot;

			if( pAct->bFollow == false )
			{
				NiMatrix3 mat = pChar->GetModelRot();
				trans.m_Rotate = pobj->rot*mat;
			}

			char pathName[256] = {0,};
			::sprintf_s( pathName, "./Data/Effect/%s", pobj->file.Cstr() );

			cEffectSceneNode* pNode = 0;
			if( pChar->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				if( pAct->linkPos >= eLINK_RWEAPON )
				{
					assert(0);
					return;
				}
			}

			pNode = pChar->SetLinkdEffect( pAct->linkPos, pathName, &trans, pAct->bLoop, pAct->bFollow );

			if( pNode && pAct->bLoop == true )
				AddLoopEffect( pNode );
		}
		break;
	case eDRAMAOBJ_SOUND:
		{
			/// 사운드 적용
			cSoundSceneNode* pNode = 0;
			pNode = pChar->SetLinkdSound( pobj->soundListIndex, pAct->bLoop );

			if( pNode && pAct->bLoop )
			{
				AddLoopSound( pNode );
			}
		}
		break;
	}
}

void cDramaturgyObject::UpdateTrailObject( sDramaActionBase* pBase, cBaseObject* pChar )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_TRAIL )
		return;
	if( pChar == 0 ) 
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
	sTrailInfo* pInfo = pChar->CreateTrailEffect( str, pAct->lifeTime, pAct->linkStart, pAct->linkEnd, pAct->lengthPer, pAct->speedFactor );
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
	mIsNoneStop = false;

	mPrintType = ePRINT_DAMAGE;

	mFieldPos = NiPoint3::ZERO;

	mSpecialType = eSpecial_None;
	mCastingTime = 0;

	mTransformMonSkill = false;
//	mUseFieldTarget = false;

	mIsChanneling = false;
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
				//node->SetLoopFlag( false );
				node->Remove();
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
				//node->SetLoopFlag( false );
				node->Remove();
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
				//node->SetLoopFlag( false );
				node->Remove();
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
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			tPointerList<void*>::cIterator b = mCastingController.mLoopTrail.Begin();
			tPointerList<void*>::cIterator end = mCastingController.mLoopTrail.End();
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pUser->DeleteTrailEffect( pInfo );
			}
		}
		mCastingController.mLoopTrail.Clear();
	}

	/// activity loop sound fadeout
	if( mActivityController.mLoopTrail.IsEmpty() == false )
	{
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			tPointerList<void*>::cIterator b = mActivityController.mLoopTrail.Begin();
			tPointerList<void*>::cIterator end = mActivityController.mLoopTrail.End();
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pUser->DeleteTrailEffect( pInfo );
			}
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
			for( unsigned int i = 0, iend = mTargetObjArray.GetSize(); i < iend; ++i )
			{
				sDamageInfo* p = (sDamageInfo*)mTargetObjArray[i];
				cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
				if( pTarget )
					pTarget->DeleteTrailEffect( pInfo );
			}
		}
		mApplyController.mLoopTrail.Clear();
	}
}

bool cSkillObject::Init( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, cBaseObject* pTarget, float speedFactor )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSpeedFactor = speedFactor;
	if( mSpeedFactor <= 0.0f || mSpeedFactor > 2.0f )
	{
		assert(0);
		mSpeedFactor = 1.0f;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;
	mUserWeapon = eWEAPON_STATE_NONE;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	if( pTarget )
	{
		mTargetObj.index = pTarget->GetObjectID();
		mTargetObj.type = pTarget->GetObjectType();
	}
	else
	{
		mTargetObj.index = 0;
		mTargetObj.type = eOBJECTTYPE_NONE;
	}

//	mTargetObjArray.Reserve( 10 );

	mVerifyState = eVerify_Wait;

	bool bCasting = false;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			if( pPlayer->IsTransformMonster() == 0 )
			{
				switch( pPlayer->GetWeaponState() )
				{
				case eWEAPON_STATE_SWORD:
				case eWEAPON_STATE_SWORD_SHEILD:
					{
						mUserWeapon = eWEAPON_STATE_SWORD;
					}
					break;
				case eWEAPON_STATE_LONGSWORD:	mUserWeapon = eWEAPON_STATE_LONGSWORD;	break;
				case eWEAPON_STATE_DOUBLESWORD: mUserWeapon = eWEAPON_STATE_DOUBLESWORD;break;
				case eWEAPON_STATE_SHORTSWORD:	mUserWeapon = eWEAPON_STATE_SHORTSWORD; break;
				case eWEAPON_STATE_GUN:			mUserWeapon = eWEAPON_STATE_GUN;		break;
				case eWEAPON_STATE_STAFF:		mUserWeapon = eWEAPON_STATE_STAFF;		break;
				default:
					break;
				}

				mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
					pPlayer->GetRace(), pPlayer->GetGender() );

				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
				if( pInfo == 0 )
					return false;

				mIsNoneStop = (pInfo->mType == eSKILLTYPE_NONSTOP);

				if( pInfo->mShotType == eSHOTTYPE_CASTING )
				{
					bCasting = true;

					if( pPlayer && pPlayer == HERO )
					{
						/// casting ui 등록... 
						sKeepSkill* keep = SKILLMAN->GetKeepInfo( mSkillInfo.skillIdx );
						if( keep )
						{
							mCastingTime = HERO->CalcCastingExt( keep->mCastingTime );
							cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
							if( pWin )
							{
								pWin->SetTime( mSkillInfo.skillIdx, eCasting_Skill, mCastingTime );
							}
						}
						else
						{
							assert(0);
						}
					}
				}

				mTransformMonSkill = false;
			}
			else
			{
				unsigned long classId = pPlayer->GetTransMonsterClassIdx();
				mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( pSkillInfo == 0 )
					return false;

				mIsNoneStop = (pSkillInfo->mType == eMONSTERSKILLTYPE_NONSTOP);


				if( pSkillInfo->mShotType  == eMONSTERSHOTTYPE_CASTING )
				{
					bCasting = true;
					if( pPlayer && pPlayer == HERO )
					{
						mCastingTime = HERO->CalcCastingExt( pSkillInfo->mCastingTime );
						cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
						if( pWin )
						{
							pWin->SetTime( skillIdx, eCasting_Skill, mCastingTime );							
						}
					}

				}

				mTransformMonSkill = true;
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMonster = (cMonster*)pUser;
			unsigned long classId = pMonster->GetMonsterClassIdx();

			mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( pSkillInfo == 0 )
				return false;

			if( pSkillInfo->mShotType  == eMONSTERSHOTTYPE_CASTING )
			{
				bCasting = true;
			}
		}
		break;
	default:
		assert(0);
		return false;
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

bool cSkillObject::InitField( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSpeedFactor = speedFactor;
	if( mSpeedFactor <= 0.0f || mSpeedFactor > 2.0f )
	{
		assert(0);
		mSpeedFactor = 1.0f;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	mFieldPos = fieldPos;
	mSpecialType = eSpecial_Field;
//	mUseFieldTarget = true;

	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

//	mTargetObjArray.Reserve( 10 );

	mVerifyState = eVerify_Wait;

	bool bCasting = false;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			if( pPlayer->IsTransformMonster() == 0 )
			{
				mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
					pPlayer->GetRace(), pPlayer->GetGender() );

				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
				if( pInfo == 0 )
					return false;

				if( pInfo->mShotType == eSHOTTYPE_CASTING )
				{
					bCasting = true;

					if( pPlayer && pPlayer == HERO )
					{
						/// casting ui 등록... 
						sKeepSkill* keep = SKILLMAN->GetKeepInfo( mSkillInfo.skillIdx );
						if( keep )
						{
							mCastingTime = HERO->CalcCastingExt( keep->mCastingTime );
							cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
							if( pWin )
							{
								pWin->SetTime( mSkillInfo.skillIdx, eCasting_Skill, mCastingTime );
							}
						}
						else
						{
							assert(0);
						}
					}
				}

				mTransformMonSkill = false;
			}
			else
			{
				unsigned long classId = pPlayer->GetTransMonsterClassIdx();
				mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( pSkillInfo == 0 )
					return false;

				if( pSkillInfo->mShotType  == eMONSTERSHOTTYPE_CASTING )
				{
					bCasting = true;

					if( pPlayer && pPlayer == HERO )
					{
						mCastingTime = HERO->CalcCastingExt( pSkillInfo->mCastingTime );
						cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
						if( pWin )
						{
							pWin->SetTime( mSkillInfo.skillIdx, eCasting_Skill, mCastingTime );
						}
					}
				}

				mTransformMonSkill = true;
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMonster = (cMonster*)pUser;
			unsigned long classId = pMonster->GetMonsterClassIdx();

			mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( pSkillInfo == 0 )
				return false;

			if( pSkillInfo->mShotType  == eMONSTERSHOTTYPE_CASTING )
				bCasting = true;
		}
		break;
	default:
		assert(0);
		return false;
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

bool cSkillObject::InitMapChange( unsigned long skillIdx, cBaseObject* pUser )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = 0;
	mRemoved = false;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

	mSpecialType = eSpecial_MapChange;
	mVerifyState = eVerify_Wait;

	bool bCasting  = false;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
					pPlayer->GetRace(), pPlayer->GetGender() );

			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
			if( pInfo == 0 )
				return false;

			if( pInfo->mShotType == eSHOTTYPE_CASTING )
				bCasting = true;
		}
		break;
	default:
		assert(0);
		return false;
	}

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
	if( mCastingController.pAct == 0  && mCastingController.animationIdx == UINT_MAX )
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

		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
			if( info )
			{
				if( info->mpSetpInfoArray[0].mCastingTime > 500 )
					mCastingTime = info->mpSetpInfoArray[0].mCastingTime - 500;
				else
					mCastingTime = info->mpSetpInfoArray[0].mCastingTime;

				if( pUser == HERO )
				{
					cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
					if( pWin )
						pWin->SetTime( skillIdx, eCasting_Skill, mCastingTime );
				}
			}
		}
	}

	return true;
}

bool cSkillObject::InitVehicle( unsigned long skillIdx, cBaseObject* pUser )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = 0;
	mRemoved = false;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

	mSpecialType = eSpecial_Vehicle;
	mVerifyState = eVerify_Wait;

	bool bCasting  = false;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
				pPlayer->GetRace(), pPlayer->GetGender() );

			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
			if( pInfo == 0 )
				return false;

			if( pInfo->mShotType == eSHOTTYPE_CASTING )
				bCasting = true;
		}
		break;
	default:
		assert(0);
		return false;
	}

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
	if( mCastingController.pAct == 0  && mCastingController.animationIdx == UINT_MAX )
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

		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
			if( info )
			{
				if( info->mpSetpInfoArray[0].mCastingTime > 500 )
					mCastingTime = info->mpSetpInfoArray[0].mCastingTime - 500;
				else
					mCastingTime = info->mpSetpInfoArray[0].mCastingTime;

				if( pUser == HERO )
				{
					cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
					if( pWin )
						pWin->SetTime( skillIdx, eCasting_Skill, mCastingTime );
				}
			}
		}
	}

	return true;
}

bool cSkillObject::InitBlink( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

	mSpecialType = eSpecial_Blink;
	mVerifyState = eVerify_Wait;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
				pPlayer->GetRace(), pPlayer->GetGender() );

			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
			if( pInfo == 0 )
				return false;

			if( pInfo->mShotType != eSHOTTYPE_BLINK && pInfo->mShotType != eSHOTTYPE_BLINK_BACK )
				return false;
		}
		break;
	default:
		assert(0);
		return false;
	}

	/// 연출을 위한 정보를 참조
	mCastingController.pAct = 0;
	mCastingController.animationIdx = UINT_MAX;
	mCastingController.startTime = ULONG_MAX;

	mActivityController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_ACTIVITY );
	mActivityController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_ACTIVITY );
	mActivityController.startTime = ULONG_MAX;

	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	/// 예외 체크
	assert(mActivityController.pAct);

	ChangeDramaState( eDRAMASTATE_ACTIVITY );

	return true;
}

bool cSkillObject::RevisionInit( unsigned long skillIdx, unsigned char skillStep, unsigned char userType, unsigned long dramaKey, bool transMon )
{
	if( mSpecialType > eSpecial_Field )
		return false;

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;

	/// 사용자 와 타겟의 정보는 기본값
	mUserObj.index = 0;
	mUserObj.type = userType;
	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

//	mTargetObjArray.Reserve( 10 );

	mVerifyState = eVerify_Success;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			if( transMon == false )
			{
				/// dramaKey = (((race*eGENDER_MAX)+gender)*eWEAPON_STATE_MAX + weapon);
				mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)dramaKey );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
				if( pInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
			else
			{
				/// dramaKey = monster class index
				mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
				if( pSkillInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			/// dramaKey = monster class index
			mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
			if( pSkillInfo == 0 )
			{
				assert(0);
				return false;
			}
		}
		break;
	default:
		assert(0);
		return false;
	}

	/// 연출을 위한 정보를 참조
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	ChangeDramaState( eDRAMASTATE_APPLY );

	return true;
}

cBaseObject* cSkillObject::GetUser()
{
	return OBJECTMAN->GetObject( &mUserObj );
}

void cSkillObject::AddDamageTargetInfo( sTargetDamage* dInfo )
{
	if( dInfo == 0 )
		return;

	sDamageInfo* p = new sDamageInfo;
	p->obj = dInfo->mTarget;
	p->viewCnt = 0;

	for( unsigned int i=0; i<dInfo->mDetailCnt; i++ )
	{
		sTargetDamageDetail* detail = new sTargetDamageDetail;
		detail->mDamage = dInfo->mDetail[i].mDamage;
		detail->mDamageKind = dInfo->mDetail[i].mDamageKind;

		detail->mDamageReflection = dInfo->mDetail[i].mDamageReflection;
		detail->mDamageAbsorbHP = dInfo->mDetail[i].mDamageAbsorbHP;
		detail->mDamageAbsorbMP = dInfo->mDetail[i].mDamageAbsorbMP;

		p->damageArr.PushBack( detail );
	}

	mTargetObjArray.PushBack( p );
	mPrintType = ePRINT_DAMAGE;
}

void cSkillObject::AddHealTargetInfo( sTargetHeal* dInfo )
{
	sDamageInfo* p = new sDamageInfo;
	p->obj = dInfo->mTarget;
	p->viewCnt = 0;

	sTargetDamageDetail* detail = new sTargetDamageDetail;
	detail->mDamage = dInfo->mHeal;
	detail->mDamageKind = eDAMAGE_NORMAL;
	p->damageArr.PushBack( detail );

	mTargetObjArray.PushBack( p );
	mPrintType = ePRINT_HEAL;
}

void cSkillObject::AddTargetInfo( sObject* dInfo, unsigned char type )
{
	sDamageInfo* p = new sDamageInfo;
//	p->damageValue = 0;
//	p->damageType = eDAMAGE_NONE;
	p->obj = *dInfo;
	p->viewCnt = 0;

	mTargetObjArray.PushBack( p );

	mPrintType = type;
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
					//node->SetLoopFlag( false );
					node->Remove();
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
			cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
			if( pUser )
			{
				tPointerList<void*>::cIterator b = mCastingController.mLoopTrail.Begin();
				tPointerList<void*>::cIterator end = mCastingController.mLoopTrail.End();
				for( ; b != end; ++b )
				{
					sTrailInfo* pInfo = (sTrailInfo*)(*b);
					pUser->DeleteTrailEffect( pInfo );
				}
			}
			mCastingController.mLoopTrail.Clear();
		}
	}
	
	if( state == eDRAMASTATE_CANCEL_WAIT && mIsChanneling )
	{
		/// casting loop effect clear
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
					//node->SetLoopFlag( false );
					node->Remove();
				}
				delete p;
			}
			mActivityController.mLoopEffect.Clear();
		}

		/// casting loop sound clear
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

		if( mActivityController.mLoopTrail.IsEmpty() == false )
		{
			cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
			if( pUser )
			{
				tPointerList<void*>::cIterator b = mActivityController.mLoopTrail.Begin();
				tPointerList<void*>::cIterator end = mActivityController.mLoopTrail.End();
				for( ; b != end; ++b )
				{
					sTrailInfo* pInfo = (sTrailInfo*)(*b);
					pUser->DeleteTrailEffect( pInfo );
				}
			}
			mActivityController.mLoopTrail.Clear();
		}
	}

	mCurrentState = state;
}

void cSkillObject::WaitCancel()
{
	if( mCurrentState == eDRAMASTATE_CASTING )
		ChangeDramaState( eDRAMASTATE_CANCEL_WAIT );
	else if( mCurrentState == eDRAMASTATE_ACTIVITY && mIsChanneling == true )
		ChangeDramaState( eDRAMASTATE_CANCEL_WAIT );
}

void cSkillObject::AddBulletObject( sDramaActionBase* pBase )
{
	if( pBase == 0 )
		return;
	if( pBase->actionType != eDRAMA_BULLET )
		return;

	/// 시전자..
	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
	if( pUser == 0 )
		return;

	cBulletObj* pObj = 0;
	if( mSpecialType == eSpecial_Field )
	{
		pObj = new cBulletObj;
		if( pObj->Init( mpDramaInfo, (sBulletAction*)pBase, pUser, mFieldPos ) == false )
		{
			delete pObj;
			return;
		}
		mBulletList.PushBack( pObj );
	}
	else
	{
		cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
		if( pTarget == 0 )
			return;

		pObj = new cBulletObj;
		if( pObj->Init( mpDramaInfo, (sBulletAction*)pBase, pUser, pTarget ) == false )
		{
			delete pObj;
			return;
		}
		mBulletList.PushBack( pObj );
	}
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
		/// 스킬 사용시 동작이 없는 객체의 경우 바로 스킬을 사용할수 있도록 하는 보정코드
		if( mUsedAnimation == false )
		{
			cBaseObject* obj = OBJECTMAN->GetObject( &mUserObj );

			if( obj && obj == HERO )
			{
				mUsedAnimation = true;

				if( mIsNoneStop == false )
					HERO->SetReadyUseSkill( true );
			}
		}

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

		if( mSpecialType == eSpecial_Blink )
			return;

		/// 스킬 객체 제거
		if( mVerifyState != eVerify_Wait )
			SKILLMAN->DeleteSkillObject( this );
	}
}

void cSkillObject::ProcessCasting( unsigned long time )
{
	if( mVerifyState == eVerify_Faile )
		return;

	/// 초기 설정
	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
	if( mCastingController.startTime == ULONG_MAX )
	{
		mCastingController.startTime = time;

		/// 애니메이션 적용
		if( pUser )
		{
			if( mCastingController.animationIdx != UINT_MAX )
			{
				if( pUser->GetState() != eOBJECT_STATE_DIE )
				{
//					pUser->UpdateAnimationSpeed( mSpeedFactor );
					if( mSpecialType > eSpecial_Blink )
					{
						if( pUser->GetObjectType() == eOBJECTTYPE_HERO ||
							pUser->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							if( ((cPlayer*)pUser)->IsTransformMonster() == false )
							{
								pUser->UpdateSkillAnimation( mCastingController.animationIdx, true );
								mUsedAnimation = true;
							}
							else
							{
								((cPlayer*)pUser)->ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
							}
						}
					}
					else
					{

						pUser->UpdateSkillAnimation( mCastingController.animationIdx );
						mUsedAnimation = true;
					}
				}
			}
		}
	}

	sDramaActionBase* pAct = mCastingController.pAct;
	if( pAct == 0 )
	{
		if( mSpecialType > eSpecial_Blink )
		{
			unsigned long delta = time - mCastingController.startTime;
			if( delta >= mCastingTime )
				EndCast();
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

			if( pUser )
			{
				if( pAct->actionType == eDRAMA_TRAIL )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
						UpdateTrailObject( pAct, pUser );
				}
				else if( pAct->actionType == eDRAMA_SHAKE )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						if( pUser == HERO )
							CAMERAMAN->Shake();
					}
				}
				else
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						UpdateDramaObject( pAct, pUser );
					}
				}
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
	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );

	/// 초기 설정
	if( mActivityController.startTime == ULONG_MAX )
	{
		mActivityController.startTime = time;

		/// 애니메이션 적용
		if( pUser )
		{
			if( mActivityController.animationIdx != UINT_MAX )
			{
				if( pUser->GetState() != eOBJECT_STATE_DIE )
				{
					if( mSpecialType > eSpecial_Blink )
					{
						if( pUser->GetObjectType() == eOBJECTTYPE_HERO ||
							pUser->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							if( ((cPlayer*)pUser)->IsTransformMonster() == false )
							{
								pUser->UpdateAnimationSpeed( mSpeedFactor );
								pUser->UpdateSkillAnimation( mActivityController.animationIdx, true );
								mUsedAnimation = true;
							}
						}
					}
					else
					{
						pUser->UpdateAnimationSpeed( mSpeedFactor );
						pUser->UpdateSkillAnimation( mActivityController.animationIdx );
						mUsedAnimation = true;
					}
				}
			}
			else
			{
				/// 동작이 없는 스킬을 사용하는 경우 보정
				if( mIsNoneStop == false )
				{
					if( mUserObj.type == eOBJECTTYPE_HERO || mUserObj.type == eOBJECTTYPE_PLAYER )
					{
						((cPlayer*)pUser)->ChangeAnimation( ANITYPE_IDLE );
					}
					else if( mUserObj.type == eOBJECTTYPE_MONSTER )
					{
						pUser->UpdateSkillAnimation( M_ANITYPE_IDLE1 );
					}
				}
			}
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
				if( pUser )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
						UpdateTrailObject( pAct, pUser );
				}
			}
			else if( pAct->actionType == eDRAMA_SHAKE )
			{
				if( pUser->GetState() != eOBJECT_STATE_DIE )
				{
					if( pUser == HERO )
						CAMERAMAN->Shake();
				}
			}
			else if( pAct->actionType == eDRAMA_ACTIVE )
			{
				ChangeDramaState( eDRAMASTATE_APPLY );

				if( pUser && pUser != HERO )
				{
					/// 타유저 및 몬스터에 대한 애니메이션 보정을 위해서
					if( mIsNoneStop == false )
						pUser->SetReadyUseSkill( true );
				}
				break;
			}
			else if( pAct->actionType == eDRAMA_BULLET )
			{
				AddBulletObject( pAct );
				ChangeDramaState( eDRAMASTATE_APPLY );

				if( pUser && pUser != HERO )
				{
					/// 타유저 및 몬스터에 대한 애니메이션 보정을 위해서
					if( mIsNoneStop == false )
						pUser->SetReadyUseSkill( true );
				}
				break;
			}
			else
			{
				if( pUser )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						UpdateDramaObject( pAct, pUser );
					}
				}
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
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( mActivityController.startTime == ULONG_MAX )
		{
			mActivityController.startTime = time;

			/// 애니메이션 적용
			if( pUser )
			{
				if( mActivityController.animationIdx != UINT_MAX )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						pUser->UpdateAnimationSpeed( mSpeedFactor );
						pUser->UpdateSkillAnimation( mActivityController.animationIdx );
						mUsedAnimation = true;
					}
				}
				else
				{
					/// 동작이 없는 스킬을 사용하는 경우 보정
					if( mIsNoneStop == false )
					{
						if( mUserObj.type == eOBJECTTYPE_HERO || mUserObj.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pUser)->ChangeAnimation( ANITYPE_IDLE );
						}
						else if( mUserObj.type == eOBJECTTYPE_MONSTER )
						{
							pUser->UpdateSkillAnimation( M_ANITYPE_IDLE1 );
						}
					}
				}
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
					if( pUser )
					{
						if( pUser->GetState() != eOBJECT_STATE_DIE )
							UpdateTrailObject( pAct, pUser );
					}
				}
				else if( pAct->actionType == eDRAMA_SHAKE )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						if( pUser == HERO )
							CAMERAMAN->Shake();
					}
				}
				else if( pAct->actionType == eDRAMA_ACTION )
				{
					if( pUser )
					{
						if( pUser->GetState() != eOBJECT_STATE_DIE )
						{
							UpdateDramaObject( pAct, pUser );
						}
					}
				}
			}
			else
				break;

			pAct = mActivityController.pAct;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 타겟 행동 처리
	if( mVerifyState == eVerify_Wait )
	{
		/// 데미지 출력부 진입전 사용자가 사라지면...
		/// 객체를 유지하지 않는다.
		/// 후에 데미지만 별도로 온다면..
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser == 0 )
			SKILLMAN->DeleteSkillObject( this );

		return;
	}

	if( mSpecialType == eSpecial_Blink )
	{
		/// move
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			if( WORLDMAN->CalcHeight( &mFieldPos.z, mFieldPos.x, mFieldPos.y ) )
			{
				pUser->SetPos( mFieldPos );
			}

			if( pUser == HERO )
			{
				if( HERO->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->IsTransformMonster() == true )
						HERO->ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
					else
						HERO->ChangeAnimation( ANITYPE_IDLE );
				}
				else
				{
					HERO->SetState( eOBJECT_STATE_IDLE );
				}
			}
			else
			{
				pUser->SetState( eOBJECT_STATE_IDLE, true );
			}
		}

		mSpecialType = eSpecial_None;
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
					sDamageInfo* p = 0;
					for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
					{
						p = (sDamageInfo*)mTargetObjArray[i];
						cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
						if( pTarget == 0 )
							continue;

						cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
						NiPoint3 dir = NiPoint3::UNIT_Y;
						if( pUser )
						{
							NiPoint3 viewDir = pTarget->GetPos() - pUser->GetPos();
							viewDir.z = 0;
							if( viewDir.SqrLength() >= 0.001f )	
							{
								viewDir.Unitize();
								dir = viewDir;
							}
						}
						if( pAct->actionType == eDRAMA_TRAIL )
							UpdateTrailObject( pAct, pTarget );
						else if( pAct->actionType == eDRAMA_SHAKE )
						{
							if( pTarget && pTarget == HERO )
								CAMERAMAN->Shake();
						}
						else
						{
							UpdateDramaObject( pAct, pTarget, -dir );
						}
					}
				}
				///
			}
			else
				break;

			pAct = mApplyController.pAct;
		}
	}
	else
	{
		for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
		{
			delete (sDamageInfo*)mTargetObjArray[i];
		}
		mTargetObjArray.Clear();
	}
}

void cSkillObject::ApplyDamage( const sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;

	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
	sDamageInfo* p = 0;
	sTargetDamageDetail* damageDetail = 0;
	for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
	{
		p = (sDamageInfo*)mTargetObjArray[i];
		if( p->damageArr.GetSize() == 0 )
		{
			///
			cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
			if( pTarget )
			{
				pTarget->ApplyDamageDrama( -1, 0, pUser );
				SKILLMAN->CreateApplyObject( pUser, pTarget, mUserWeapon, eAPPLYDRAMA_DAMAGE );
			}
			continue;
		}

		if( p->viewCnt >= p->damageArr.GetSize() )
		{
			assert(0);
			continue;
		}

		damageDetail = (sTargetDamageDetail*)p->damageArr[p->viewCnt];
		p->viewCnt++;


		cBaseObject* pAttacker = OBJECTMAN->GetObject( &mUserObj );
		cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
		if( pAttacker )
		{
			/// 적/아군 구분
			bool enemy = false;
			switch( pAttacker->GetObjectType() )
			{
			case eOBJECTTYPE_MONSTER:
				enemy = true;
				break;
			case eOBJECTTYPE_PLAYER:
				{
					if( HERO &&	HERO->IsEnemy( (cPlayer*)pAttacker ) == true )
						enemy = true;
				}
				break;
			}

			/// 데미지 반사 처리
			if( damageDetail->mDamageReflection != 0 )
			{
				if( enemy == false )
				{
					if( damageDetail->mDamageReflection > 0 )
						pAttacker->ApplyDamageDrama( damageDetail->mDamageReflection, eDAMAGEPRINT_NORMAL_ENEMY, pTarget );
					else if( damageDetail->mDamageReflection < 0 )
						pAttacker->ApplyDamageDrama( abs(damageDetail->mDamageReflection), eDAMAGEPRINT_HEAL_ENEMY, pTarget );
				}
				else
				{
					if( damageDetail->mDamageReflection > 0 )
						pAttacker->ApplyDamageDrama( damageDetail->mDamageReflection, eDAMAGEPRINT_NORMAL_OUR, pTarget );
					else if( damageDetail->mDamageReflection < 0 )
						pAttacker->ApplyDamageDrama( abs(damageDetail->mDamageReflection), eDAMAGEPRINT_HEAL_OUR, pTarget );
				}
			}

			/// hp흡수
			if( damageDetail->mDamageAbsorbHP != 0 )
			{
				if( enemy == false )
				{
					if( damageDetail->mDamageAbsorbHP > 0 )
						pAttacker->ApplyDamageDrama( damageDetail->mDamageAbsorbHP, eDAMAGEPRINT_HEAL_OUR, pTarget );
					else if( damageDetail->mDamageAbsorbHP < 0 )
						pAttacker->ApplyDamageDrama( abs(damageDetail->mDamageAbsorbHP), eDAMAGEPRINT_NORMAL_OUR, pTarget );
				}
				else
				{
					if( damageDetail->mDamageAbsorbHP > 0 )
						pAttacker->ApplyDamageDrama( damageDetail->mDamageAbsorbHP, eDAMAGEPRINT_HEAL_ENEMY, pTarget );
					else if( damageDetail->mDamageAbsorbHP < 0 )
						pAttacker->ApplyDamageDrama( abs(damageDetail->mDamageAbsorbHP), eDAMAGEPRINT_NORMAL_ENEMY, pTarget );
				}
			}

			/// mp흡수
			if( damageDetail->mDamageAbsorbMP != 0 )
			{
				if( damageDetail->mDamageAbsorbMP > 0 )
					pAttacker->ApplyDamageDrama( damageDetail->mDamageAbsorbMP, eDAMAGEPRINT_MP_HEAL, pTarget );
				else if( damageDetail->mDamageAbsorbMP < 0 )
					pAttacker->ApplyDamageDrama( abs(damageDetail->mDamageAbsorbMP), eDAMAGEPRINT_MP_NORMAL, pTarget );
			}
		}

		///
		if( pTarget == 0 )
			continue;

		eDAMAGEPRINT_TYPE type = eDAMAGEPRINT_MISS;
		/// 2발스킬에 첫방에 안죽고 두번째방에 죽는경우 미스뜨면서 죽는경우때문에 주석처리함
		/// 대신 이곳이 주석처리되면 첫발에 죽는경우 두번째공격에 데미지가 들어갈수있으나
		/// 첫번째 경우는 버그로 보여서 두번째경우가 덜어색해 보여 두번째로 선택함
//		if( pTarget->IsDie() == false || p->viewCnt == 1 )
		{
			/// 적/아군 구분
			bool enemy = false;
			switch( pTarget->GetObjectType() )
			{
			case eOBJECTTYPE_MONSTER:
				enemy = true;
				break;
			case eOBJECTTYPE_PLAYER:
				{
					if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
						enemy = true;
				}
				break;
			}

			if( enemy == true )
			{
				if( damageDetail->mDamageKind == eDAMAGE_NORMAL )
					type = eDAMAGEPRINT_NORMAL_ENEMY;
				else if( damageDetail->mDamageKind == eDAMAGE_CRITICAL )
					type = eDAMAGEPRINT_CRITICAL_ENEMY;
				else if( damageDetail->mDamageKind == eDAMAGE_NORMAL_MP )
					type = eDAMAGEPRINT_MP_NORMAL;
				else if( damageDetail->mDamageKind == eDAMAGE_CRITICAL_MP )
					type = eDAMAGEPRINT_MP_CRITICAL;
			}
			else
			{
				if( damageDetail->mDamageKind == eDAMAGE_NORMAL )
					type = eDAMAGEPRINT_NORMAL_OUR;
				else if( damageDetail->mDamageKind == eDAMAGE_CRITICAL )
					type = eDAMAGEPRINT_CRITICAL_OUR;
				else if( damageDetail->mDamageKind == eDAMAGE_NORMAL_MP )
					type = eDAMAGEPRINT_MP_NORMAL;
				else if( damageDetail->mDamageKind == eDAMAGE_CRITICAL_MP )
					type = eDAMAGEPRINT_MP_CRITICAL;
			}
		}

		unsigned int viewDamage = (unsigned int)damageDetail->mDamage;

		/// damage출력 및 damage동작
		pTarget->ApplyDamageDrama( viewDamage, type, pUser );

		/// 객체별 damage 연출을 호출한다.
		if( type == eDAMAGEPRINT_CRITICAL_OUR || type == eDAMAGEPRINT_CRITICAL_ENEMY )
			SKILLMAN->CreateApplyObject( pUser, pTarget, mUserWeapon, eAPPLYDRAMA_CRITICAL );
		else
			SKILLMAN->CreateApplyObject( pUser, pTarget, mUserWeapon, eAPPLYDRAMA_DAMAGE );
	}
}

void cSkillObject::ApplyHeal( const sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;

	sDamageInfo* p = 0;
	sTargetDamageDetail* damageDetail = 0;
	for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
	{
		p = (sDamageInfo*)mTargetObjArray[i];
		if( p->damageArr.GetSize() == 0 )
		{
			///
			cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
			if( pTarget )
				pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_HEAL_OUR );

			continue;
		}

		if( p->viewCnt >= p->damageArr.GetSize() )
		{
			assert(0);
			continue;
		}

		damageDetail = (sTargetDamageDetail*)p->damageArr[p->viewCnt];
		p->viewCnt++;

		cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
		if( pTarget == 0 )
			continue;

		eDAMAGEPRINT_TYPE type = eDAMAGEPRINT_MISS;
		if( pTarget->IsDie() == false || p->viewCnt == 1 )
		{
			/// 적/아군 구분
			bool enemy = false;
			switch( pTarget->GetObjectType() )
			{
			case eOBJECTTYPE_MONSTER:
				enemy = true;
				break;
			case eOBJECTTYPE_PLAYER:
				{
					if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
						enemy = true;
				}
				break;
			}

			if( enemy == false )
				type = eDAMAGEPRINT_HEAL_OUR;
			else
				type = eDAMAGEPRINT_HEAL_ENEMY;
		}

		unsigned int viewDamage = (unsigned int)damageDetail->mDamage;

		/// damage출력 및 damage동작
		pTarget->ApplyDamageDrama( viewDamage, type );
	}
}

void cSkillObject::EndCast()
{
	if( mCurrentState != eDRAMASTATE_CASTING && mCurrentState != eDRAMASTATE_CANCEL_WAIT )
	{
		assert(0);
		return;
	}

	cBaseObject* obj = OBJECTMAN->GetObject( &mUserObj );
	if( obj == HERO )
	{
		cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
		if( pWin )
			pWin->Clear( eCasting_Skill );

		if( mSpecialType < eSpecial_MapChange )
		{
			if( mTransformMonSkill == true )
				SKILLMAN->StartCoolTimeMon( mSkillInfo.skillIdx );
			else
				SKILLMAN->StartCoolTime( mSkillInfo.skillIdx );

			if( HERO->GetState() != eOBJECT_STATE_ATTACK )
				HERO->SetState( eOBJECT_STATE_ATTACK );
		}
	}

	ChangeDramaState( eDRAMASTATE_ACTIVITY );
}

void cSkillObject::VerifyFaile( bool setIdle )
{
	mVerifyState = eVerify_Faile;

	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );

	if( mCurrentState == eDRAMASTATE_CASTING || mCurrentState == eDRAMASTATE_CANCEL_WAIT )
	{
		if( pUser && pUser->GetObjectType() == eOBJECTTYPE_HERO )
		{
			cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
			if( pWin )
				pWin->Clear( eCasting_Skill );
		}

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
					//node->SetLoopFlag( false );
					node->Remove();
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
					node->Remove();
				}
				delete p;
			}
			mCastingController.mLoopSound.Clear();
		}

		if( mCastingController.mLoopTrail.IsEmpty() == false )
		{
			if( pUser )
			{
				tPointerList<void*>::cIterator b = mCastingController.mLoopTrail.Begin();
				tPointerList<void*>::cIterator end = mCastingController.mLoopTrail.End();
				for( ; b != end; ++b )
				{
					sTrailInfo* pInfo = (sTrailInfo*)(*b);
					pUser->DeleteTrailEffect( pInfo );
				}
			}
			mCastingController.mLoopTrail.Clear();
		}

		SKILLMAN->DeleteSkillObject( this );
	}

	if( setIdle )
	{
		if( pUser && pUser->GetState() != eOBJECT_STATE_DIE )
			pUser->SetState( eOBJECT_STATE_IDLE, setIdle );
	}
}

void cSkillObject::VerifySuccess()
{
	/// 최종 Damage까지 받으면 호출된다.
	mVerifyState = eVerify_Success;
}

void cSkillObject::SetBlinkPos( NiPoint3 pos )
{
	if( mSpecialType != eSpecial_Blink )
	{
		mActivityController.pAct = 0;
		mApplyController.pAct = 0;
		mVerifyState = eVerify_Faile;

		return;
	}

	mFieldPos = pos;
	mVerifyState = eVerify_Success;

	sDamageInfo* p = new sDamageInfo;
	p->obj = mUserObj;
	p->viewCnt = 0;
	mTargetObjArray.PushBack( p );
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

	mGuardCnt = 0;
	mGuardHP = 0;

	mIsRealTime = false;
}

cInfluenceObject::~cInfluenceObject()
{
	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
	if( pTarget )
	{
		if( mIsBuff )
			pTarget->DelBuff( mManagedKey );
		else
			pTarget->DelDeBuff( mManagedKey );
	}

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
				//node->SetLoopFlag( false );
				node->Remove();
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

	/// Apply loop Trail clear
	if( mApplyController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopTrail.End();
		if( pTarget )
		{
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pTarget->DeleteTrailEffect( pInfo );
			}
		}
		mApplyController.mLoopTrail.Clear();
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

bool cInfluenceObject::Init( unsigned long uniqueIdx, unsigned long influenceIdx, unsigned long skillIdx, bool isbuff, cBaseObject* pTarget, unsigned char deleteType )
{
	if( pTarget == 0 )
	{
		assert(0);
		return false;
	}

	/// 081015 PKH 원본 보관
	//mInfluenceIdx = 
	//mManagedKey = uniqueIdx;
	//mSkillIdx = skillIdx;//influenceIdx;
	/// 081015 PKH 수정
	mInfluenceIdx = influenceIdx;
	mManagedKey = uniqueIdx;
	mSkillInfo.skillIdx = skillIdx;
	mOnlyShowLoopEffect = false;

	mInfluenceIdx = influenceIdx;
	mIsBuff = isbuff;
	mDeleteType = deleteType;
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
		//오라
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

	if( pTarget->GetObjectType() == eOBJECTTYPE_HERO )
	{
		mIsHeroInfluce = true;
		mCheckBlink = true;

		if( mLifeTime <= 10000 )
			mBlinkTime = 0;
		else
			mBlinkTime = mLifeTime - 10000;
	}

	mTargetObj.index = pTarget->GetObjectID();
	mTargetObj.type = pTarget->GetObjectType();

	/// 연출 획득
	mpDramaInfo =DRAMATURGYMAN->GetInfluenceDramaInfo( influenceIdx );
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

	/// 가드 횟수/수치 설정
	unsigned short statusIdx[3] = { p->mStatusPlusIdx1, p->mStatusPlusIdx2, p->mStatusPlusIdx3 };
	unsigned long statusValue[3] = { p->mValue1, p->mValue2, p->mValue3 };
	for( int i = 0 ; i < 3 ; ++i )
	{
		switch( statusIdx[i] )
		{
		case eSTATUSPLUS_GUARDCNT_PHY:
		case eSTATUSPLUS_GUARDCNT_MAG:
		case eSTATUSPLUS_GUARDCNT_ALL:
		case eSTATUSPLUS_CASHEXPGUARDCNT:
			mGuardCnt = statusValue[i];
			break;
		case eSTATUSPLUS_GUARDVALUE_PHY:
		case eSTATUSPLUS_GUARDVALUE_MAG:
		case eSTATUSPLUS_GUARDVALUE_ALL:
			mGuardHP = statusValue[i];
			break;
		}
	}

	return true;
}

void  cInfluenceObject::Init_TargetUpdate()
{
	mTickInit = false;
	mOnlyShowLoopEffect = true;

	if( mpDramaInfo )
	{
		mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
		mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
		mApplyController.startTime = ULONG_MAX;
	}

	mStartTime = THEAPP->GetWorldAccumTime();
	mLifeTime = mRestTime;

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );

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
				node->Remove();
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
				node->Remove();
			}
			delete p;
		}
		mApplyController.mLoopSound.Clear();
	}

	/// Apply loop Trail clear
	if( mApplyController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopTrail.End();
		if( pTarget )
		{
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pTarget->DeleteTrailEffect( pInfo );
			}
		}
		mApplyController.mLoopTrail.Clear();
	}
}

void cInfluenceObject::StartProcess( long restTime, bool onlyLoopEffect )
{
	mOnlyShowLoopEffect = onlyLoopEffect;

	if( mStartTimeCheck == true )
		return;

	mStartTimeCheck = true;

	mStartTime = THEAPP->GetWorldAccumTime();

	if( restTime > 0 )
	{
		mRestTime = restTime;
		mLifeTime = restTime;
	}

	mApplyController.startTime = THEAPP->GetWorldAccumTime();

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
	if( pTarget )
	{
		/// 객체의 버프 or 디버프 (객체 참조)
		if( mIsBuff )
			pTarget->AddBuff( mManagedKey, this );
		else
			pTarget->AddDeBuff( mManagedKey, this );
	}
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
						switch( mTickType )
						{
						case eTick_HP:
						case eTick_HP_NOPRINT:
							ApplyHP( pAct );
							break;
						case eTick_MP:
						case eTick_MP_NOPRINT:
							ApplyMP( pAct );
							break;
						default:
							assert(0);
						}
					}
				}
				else if( pAct->actionType == eDRAMA_TRAIL ) 
				{
					cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
					if( pTarget )
					{
						UpdateTrailObject( pAct, pTarget );
					}
				}
				else if( pAct->actionType == eDRAMA_SHAKE )
				{
					cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
					if( pTarget && pTarget == HERO )
						CAMERAMAN->Shake();
				}
				else if( pAct->actionType == eDRAMA_ACTION )
				{
					cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
					if( pTarget )
					{
						sDramaAction* p = (sDramaAction*)pAct;

						if( mTickInit )
						{
							if( p->bLoop == false )
								UpdateDramaObject( pAct, pTarget );
						}
						else
						{
							if( mOnlyShowLoopEffect == true )
							{
								/// 진행형을 등록하는 경우는 반복효과만 연출한다.
								if( p->bLoop == true )
									UpdateDramaObject( pAct, pTarget );
							}
							else
							{
								UpdateDramaObject( pAct, pTarget );
							}
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
				mOnlyShowLoopEffect = true;
			}
		}
	}

	if( mStaticInfluence == false )
	{
		if( mCheckBlink == true )
		{
			if( mBlinkTime <= time - mStartTime )
			{
				cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
				if( pTarget )
				{
					if( mIsBuff )
						pTarget->BlinkBuffIcon( mManagedKey, mInfluenceIdx );
					else
						pTarget->BlinkDeBuffIcon( mManagedKey, mInfluenceIdx );
				}
				mCheckBlink = false;
			}
		}

		if( mIsRealTime == false )
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
}

void cInfluenceObject::ApplyHP( sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
	if( pTarget == 0 )
		return;

	float val;
	eDAMAGEPRINT_TYPE type = eDAMAGEPRINT_MISS;

	/// 적/아군 구분
	bool enemy = false;
	switch( pTarget->GetObjectType() )
	{
	case eOBJECTTYPE_MONSTER:
		enemy = true;
		break;
	case eOBJECTTYPE_PLAYER:
		{
			if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
				enemy = true;
		}
		break;
	}

	if( mTickValue > 0.0f )
	{
		val = mTickValue;

		if( enemy == false )
			type = eDAMAGEPRINT_HEAL_OUR;
		else
			type = eDAMAGEPRINT_HEAL_ENEMY;
	}
	else
	{
		val = mTickValue * -1.0f;
		if( enemy == false )
			type = eDAMAGEPRINT_NORMAL_OUR;
		else
			type = eDAMAGEPRINT_NORMAL_ENEMY;
	}

	sDamageAction* pDact = (sDamageAction*)pAct;
	unsigned int viewDamage = (unsigned int)(val * pDact->percent);

	/// damage출력 및 damage동작

	if( mTickType == eTick_HP_NOPRINT )
		pTarget->ApplyDamageDrama( -1, type );
	else
		pTarget->ApplyDamageDrama( viewDamage, type );
}

void cInfluenceObject::ApplyMP( sDramaActionBase* pAct )
{
	if( pAct == 0 )
		return;

	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
	if( pTarget == 0 )
		return;

	float val;
	eDAMAGEPRINT_TYPE type = eDAMAGEPRINT_MISS;
	if(mTickValue > 0.0f)
	{
		val = mTickValue;
		type = eDAMAGEPRINT_MP_HEAL;
	}
	else
	{
		val = mTickValue * -1.0f;
		type = eDAMAGEPRINT_MP_NORMAL;
	}

	sDamageAction* pDact = (sDamageAction*)pAct;
	unsigned int viewDamage = (unsigned int)(val * pDact->percent);

	/// damage출력 및 damage동작
	if( mTickType == eTick_MP_NOPRINT )
		pTarget->ApplyDamageDrama( -1, type );
	else
		pTarget->ApplyDamageDrama( viewDamage, type );
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
	cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );

	/// Apply loop Trail clear
	if( mApplyController.mLoopTrail.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mApplyController.mLoopTrail.Begin();
		tPointerList<void*>::cIterator end = mApplyController.mLoopTrail.End();
		if( pTarget )
		{
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pTarget->DeleteTrailEffect( pInfo );
			}
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

void cApplyObject::UpdateDramaObject( sDramaActionBase* pBase, cBaseObject* pChar, const NiPoint3& dir )
{
	if( pBase == 0 ) 
		return;
	if( pBase->actionType != eDRAMA_ACTION )
		return;
	if( pChar == 0 ) 
		return;

	sDramaAction* pAct = (sDramaAction*)pBase;
	sDramaObj* pobj = mpDramaInfo->GetDramaObj( pAct->objIdx );
	if( pobj == 0 )
		return;

//	if( pAct->bLoop == true )
//	{
//		assert(0);
//	}

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
				NiMatrix3 mat = pChar->GetModelRot();
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
			if( pChar->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				if( pAct->linkPos >= eLINK_RWEAPON )
				{
					assert(0);
					return;
				}
			}

			pNode = pChar->SetLinkdEffect( pAct->linkPos, pathName, &trans, false, pAct->bFollow );
		}
		break;
	case eDRAMAOBJ_SOUND:
		{
			/// 사운드 적용
			cSoundSceneNode* pNode = 0;
			pNode = pChar->SetLinkdSound( pobj->soundListIndex, false );
		}
		break;
	}
}


bool cApplyObject::Init( unsigned long managedKey, sObject* user, sObject* target, unsigned char weaponType, eAPPLYDRAMA_TYPE type )
{
	if( target == 0 )
		return false;
	if( type >= eAPPLYDRAMA_MAX )
		return false;

	cBaseObject* pTarget = OBJECTMAN->GetObject( target );
	if( pTarget == 0 )
		return true;

	mUserWeapon = weaponType;

	mTargetObj.index = pTarget->GetObjectID();
	mTargetObj.type = pTarget->GetObjectType();

	mIndexType = eINDEX_DRAMA;
	mApplyIndex = type;

	mRemoved = false;

	if( user )
	{
		mUserObj.index = user->index;
		mUserObj.type = user->type;
	}
	else
	{
		mUserObj.index = 0;
		mUserObj.type = eOBJECTTYPE_NONE;
	}

	mManagedKey = managedKey;

	switch( target->type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->IsTransformMonster() == 0 )
			{
				mpDramaInfo = DRAMATURGYMAN->GetPlayerApplyDramaInfo( pPlayer->GetRace(), pPlayer->GetGender(), type );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
			else
			{
				unsigned long classId = pPlayer->GetTransMonsterClassIdx();
				mpDramaInfo = DRAMATURGYMAN->GetMonsterApplyDramaInfo( classId, type );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMonster = (cMonster*)pTarget;
			mpDramaInfo = DRAMATURGYMAN->GetMonsterApplyDramaInfo( pMonster->GetMonsterClassIdx(), type );
			if( mpDramaInfo == 0 )
			{
//				assert(0);
				return false;
			}
		}
		break;
	default:
		return false;
	}

	/// 연출을 위한 정보를 참조
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	return true;
}

bool cApplyObject::Init( unsigned long managedKey, cBaseObject* pTarget, unsigned long influenceIdx )
{
	if( pTarget == 0 )
		return false;

	mManagedKey = managedKey;

	mIndexType = eINDEX_INFLUENCE;
	mApplyIndex = influenceIdx;

	mRemoved = false;

	mTargetObj.index = pTarget->GetObjectID();
	mTargetObj.type = pTarget->GetObjectType();

	/// 연출 획득
	mpDramaInfo =DRAMATURGYMAN->GetInfluenceDramaInfo( influenceIdx );
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

bool cApplyObject::InitAct( unsigned long managedKey, cBaseObject* pTarget, unsigned long actIdx )
{
	if( pTarget == 0 )
		return false;

	mManagedKey = managedKey;

	mIndexType = eINDEX_ACTINF;
	mApplyIndex = actIdx;

	mRemoved = false;

	mTargetObj.index = pTarget->GetObjectID();
	mTargetObj.type = pTarget->GetObjectType();

	/// 연출 획득
	mpDramaInfo =DRAMATURGYMAN->GetActInfluenceDramaInfo( actIdx );
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

				/// 처리부
				cBaseObject* pTarget = OBJECTMAN->GetObject( &mTargetObj );
				if( pTarget == 0 )
				{
					return;
				}

				cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
				NiPoint3 dir = NiPoint3::UNIT_Y;
				if( pUser )
				{
					NiPoint3 viewDir = pTarget->GetPos() - pUser->GetPos();
					viewDir.z = 0;
					if( viewDir.SqrLength() >= 0.001f )	
					{
						viewDir.Unitize();
						dir = viewDir;
					}
				}

				if( pAct->actionType == eDRAMA_TRAIL )
					UpdateTrailObject( pAct, pTarget );
				else if( pAct->actionType == eDRAMA_SHAKE )
				{
					if( pTarget && pTarget == HERO )
						CAMERAMAN->Shake();
				}
				else if( pAct->actionType == eDRAMA_FADEOUT )
				{
					if( pTarget )
						pTarget->FadeOutObject();
				}
				else
				{
					if( ((sDramaAction*)pAct)->weaponType == mUserWeapon )
						UpdateDramaObject( pAct, pTarget, -dir );
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
cChannelingSkillObject::cChannelingSkillObject()
{
	mIsChanneling = true;
	mChannelingActProcessed = false;	
}

cChannelingSkillObject::~cChannelingSkillObject()
{
}

void* cChannelingSkillObject::operator new(size_t n)
{ 
	return SKILLMAN->AllocChannelingSkillObject();
} 

void cChannelingSkillObject::operator delete(void* ptr)
{
	SKILLMAN->FreeChannelingSkillObject( (cChannelingSkillObject*)ptr );
}


void cChannelingSkillObject::Process( unsigned long time )
{
	if( mCurrentState == eDRAMASTATE_ACTIVITY )
		ProcessActivity( time );

	if( mActivityController.pAct == 0 && mApplyController.pAct == 0 )
	{
		/// 스킬 사용시 동작이 없는 객체의 경우 바로 스킬을 사용할수 있도록 하는 보정코드
		if( mUsedAnimation == false )
		{
			cBaseObject* obj = OBJECTMAN->GetObject( &mUserObj );

			if( obj && obj == HERO )
			{
				mUsedAnimation = true;

				if( mIsNoneStop == false )
					HERO->SetReadyUseSkill( true );
			}
		}

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

		if( mSpecialType == eSpecial_Blink )
			return;

		/// 스킬 객체 제거 안함.
	}
}


void cChannelingSkillObject::ProcessActivity( unsigned long time )
{
	sDramaActionBase* pAct = mActivityController.pAct;
	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );

	/// 초기 설정
	if( mActivityController.startTime == ULONG_MAX )
	{
		mActivityController.startTime = time;

		if( mChannelingActProcessed == false )
		{
			/// 애니메이션 적용
			if( pUser )
			{
				if( mActivityController.animationIdx != UINT_MAX )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						if( mSpecialType > eSpecial_Blink )
						{
							if( pUser->GetObjectType() == eOBJECTTYPE_HERO ||
								pUser->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( ((cPlayer*)pUser)->IsTransformMonster() == false )
								{
									pUser->UpdateAnimationSpeed( mSpeedFactor );
									pUser->UpdateSkillAnimation( mActivityController.animationIdx, true );
									mUsedAnimation = true;
								}
							}
						}
						else
						{
							pUser->UpdateAnimationSpeed( mSpeedFactor );
							pUser->UpdateSkillAnimation( mActivityController.animationIdx );
							mUsedAnimation = true;
						}
					}
				}
				else
				{
					/// 동작이 없는 스킬을 사용하는 경우 보정
					if( mIsNoneStop == false )
					{
						if( mUserObj.type == eOBJECTTYPE_HERO || mUserObj.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pUser)->ChangeAnimation( ANITYPE_IDLE );
						}
						else if( mUserObj.type == eOBJECTTYPE_MONSTER )
						{
							pUser->UpdateSkillAnimation( M_ANITYPE_IDLE1 );
						}
					}
				}
			}
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
				if( pUser )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
						UpdateTrailObject( pAct, pUser );
				}
			}
			else if( pAct->actionType == eDRAMA_SHAKE )
			{
				if( pUser->GetState() != eOBJECT_STATE_DIE )
				{
					if( pUser == HERO )
						CAMERAMAN->Shake();
				}
			}
			else if( pAct->actionType == eDRAMA_ACTIVE )
			{
				/// 채널링은 어플라이 정보가 왔을 때 표현할뿐 Active가 끝났다고 ChangeDramaState( eDRAMASTATE_APPLY ) 를 호출 하지 않음.

				if( pUser && pUser != HERO )
				{
					/// 타유저 및 몬스터에 대한 애니메이션 보정을 위해서
					if( mIsNoneStop == false )
						pUser->SetReadyUseSkill( true );
				}

				break;
			}
			else if( pAct->actionType == eDRAMA_BULLET )
			{
				/// 채널링에서는 발사체 효과 안쓰기로 함.
				assert(0);
				break;
			}
			else if( pAct->actionType == eDRAMA_CHANNELING )
			{
				if( pUser )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						/// 채널링에서는 ProcessActivity가 계속 호출 되므로 반복으로 설정되어 있는 연출은 이미 연출을 한번했을 경우 처리를 하지 않는다.
						if( mChannelingActProcessed )
						{
							if( ((sDramaAction*)pAct)->bLoop == false )
								UpdateChannelingDramaObject( pAct, pUser, mFieldPos );
						}
						else
						{
							UpdateChannelingDramaObject( pAct, pUser, mFieldPos );
							mChannelingActProcessed = true;
						}
					}
				}
			}
			else
			{
				if( pUser )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						UpdateDramaObject( pAct, pUser );
					}
				}
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
void cChannelingSkillObject::ProcessApply( unsigned long time )
{
	//////////////////////////////////////////////////////////////////////////
	/// 시전자 행동 처리
	sDramaActionBase* pAct = mActivityController.pAct;
	if( pAct )
	{
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( mActivityController.startTime == ULONG_MAX )
		{
			mActivityController.startTime = time;

			/// 애니메이션 적용
			if( pUser )
			{
				if( mActivityController.animationIdx != UINT_MAX )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						pUser->UpdateAnimationSpeed( mSpeedFactor );
						pUser->UpdateSkillAnimation( mActivityController.animationIdx );
						mUsedAnimation = true;
					}
				}
				else
				{
					/// 동작이 없는 스킬을 사용하는 경우 보정
					if( mIsNoneStop == false )
					{
						if( mUserObj.type == eOBJECTTYPE_HERO || mUserObj.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pUser)->ChangeAnimation( ANITYPE_IDLE );
						}
						else if( mUserObj.type == eOBJECTTYPE_MONSTER )
						{
							pUser->UpdateSkillAnimation( M_ANITYPE_IDLE1 );
						}
					}
				}
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
					if( pUser )
					{
						if( pUser->GetState() != eOBJECT_STATE_DIE )
							UpdateTrailObject( pAct, pUser );
					}
				}
				else if( pAct->actionType == eDRAMA_SHAKE )
				{
					if( pUser->GetState() != eOBJECT_STATE_DIE )
					{
						if( pUser == HERO )
							CAMERAMAN->Shake();
					}
				}
				else if( pAct->actionType == eDRAMA_ACTION )
				{
					if( pUser )
					{
						if( pUser->GetState() != eOBJECT_STATE_DIE )
						{
							UpdateDramaObject( pAct, pUser );
						}
					}
				}
				else if( pAct->actionType == eDRAMA_CHANNELING )
				{
					if( pUser )
					{
						if( pUser->GetState() != eOBJECT_STATE_DIE )
						{
							UpdateChannelingDramaObject( pAct, pUser, mFieldPos );
						}
					}
				}
			}
			else
				break;

			pAct = mActivityController.pAct;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 타겟 행동 처리
	if( mVerifyState == eVerify_Wait )
	{
		/// 데미지 출력부 진입전 사용자가 사라지면...
		/// 객체를 유지하지 않는다.
		/// 후에 데미지만 별도로 온다면..
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser == 0 )
		{
			SKILLMAN->DeleteSkillObject( this );
			return;
		}
	}

	if( mSpecialType == eSpecial_Blink )
	{
		/// move
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
		if( pUser )
		{
			if( WORLDMAN->CalcHeight( &mFieldPos.z, mFieldPos.x, mFieldPos.y ) )
			{
				pUser->SetPos( mFieldPos );
			}

			if( pUser == HERO )
			{
				if( HERO->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->IsTransformMonster() == true )
						HERO->ChangeAnimation_TransMon( M_ANITYPE_IDLE1 );
					else
						HERO->ChangeAnimation( ANITYPE_IDLE );
				}
				else
				{
					HERO->SetState( eOBJECT_STATE_IDLE );
				}
			}
			else
			{
				pUser->SetState( eOBJECT_STATE_IDLE, true );
			}
		}

		mSpecialType = eSpecial_None;
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
					sDamageInfo* p = 0;
					for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
					{
						p = (sDamageInfo*)mTargetObjArray[i];
						cBaseObject* pTarget = OBJECTMAN->GetObject( &p->obj );
						if( pTarget == 0 )
							continue;

						cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
						NiPoint3 dir = NiPoint3::UNIT_Y;
						if( pUser )
						{
							NiPoint3 viewDir = pTarget->GetPos() - pUser->GetPos();
							viewDir.z = 0;
							if( viewDir.SqrLength() >= 0.001f )	
							{
								viewDir.Unitize();
								dir = viewDir;
							}
						}
						if( pAct->actionType == eDRAMA_TRAIL )
							UpdateTrailObject( pAct, pTarget );
						else if( pAct->actionType == eDRAMA_SHAKE )
						{
							if( pTarget && pTarget == HERO )
								CAMERAMAN->Shake();
						}
						else
						{
							UpdateDramaObject( pAct, pTarget, -dir );
						}
					}
				}
				///
			}
			else
				break;

			pAct = mApplyController.pAct;
		}
	}
	else
	{
		for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
		{
			delete (sDamageInfo*)mTargetObjArray[i];
		}
		mTargetObjArray.Clear();
	}
}

void cChannelingSkillObject::VerifySuccess()
{
	/// 최종 Damage까지 받으면 호출된다.
	mVerifyState = eVerify_Success;

	if( mCurrentState == eDRAMASTATE_ACTIVITY )
	{
		ChangeDramaState( eDRAMASTATE_APPLY );
		ProcessApply( THEAPP->GetWorldAccumTime() );
	}

	{
		ChangeDramaState( eDRAMASTATE_ACTIVITY );

		mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
		mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
		mApplyController.startTime = ULONG_MAX;

		for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
		{
			delete (sDamageInfo*)mTargetObjArray[i];
		}
		mTargetObjArray.Clear();

		mVerifyState = eVerify_Wait;

		/// 기존 이펙트/사운드 다시 세팅
		mActivityController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_ACTIVITY );
		mActivityController.startTime = ULONG_MAX;
	}

}

void cChannelingSkillObject::VerifyFaile( bool setIdle )
{
	mVerifyState = eVerify_Faile;

	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );
	if( pUser && pUser->GetObjectType() == eOBJECTTYPE_HERO )
	{
		cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
		if( pWin )
			pWin->Clear( eCasting_Skill );
	}
	
	ChangeDramaState( eDRAMASTATE_CANCEL_WAIT );

	ClearActivityLoop();
	
	SKILLMAN->DeleteSkillObject( this );

	if( setIdle )
	{
		if( pUser && pUser->GetState() != eOBJECT_STATE_DIE )
			pUser->SetState( eOBJECT_STATE_IDLE, setIdle );
	}
}

void cChannelingSkillObject::ClearActivityLoop()
{
	/// activity loop effect clear
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
				//node->SetLoopFlag( false );
				node->Remove();
			}
			delete p;
		}
		mActivityController.mLoopEffect.Clear();
	}

	/// activity loop sound clear
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
				node->Remove();
			}
			delete p;
		}
		mActivityController.mLoopSound.Clear();
	}

	if( mActivityController.mLoopTrail.IsEmpty() == false )
	{
		cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );

		if( pUser )
		{
			tPointerList<void*>::cIterator b = mActivityController.mLoopTrail.Begin();
			tPointerList<void*>::cIterator end = mActivityController.mLoopTrail.End();
			for( ; b != end; ++b )
			{
				sTrailInfo* pInfo = (sTrailInfo*)(*b);
				pUser->DeleteTrailEffect( pInfo );
			}
		}
		mActivityController.mLoopTrail.Clear();
	}
}

bool cChannelingSkillObject::InitField( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor )
{
	if( pUser == 0 )
	{
		assert(0);
		return false;
	}

	mSpeedFactor = speedFactor;
	if( mSpeedFactor <= 0.0f || mSpeedFactor > 2.0f )
	{
		assert(0);
		mSpeedFactor = 1.0f;
	}

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;

	/// 사용자 와 타겟의 정보를 셋팅한다.
	mUserObj.index = pUser->GetObjectID();
	mUserObj.type = pUser->GetObjectType();

	mFieldPos = fieldPos;
	mSpecialType = eSpecial_Field;
	//	mUseFieldTarget = true;

	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

	//	mTargetObjArray.Reserve( 10 );

	mVerifyState = eVerify_Wait;

	bool bCasting = false;

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 연출 정보 획득
			cPlayer* pPlayer = (cPlayer*)pUser;
			if( pPlayer->IsTransformMonster() == 0 )
			{
				mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
					pPlayer->GetRace(), pPlayer->GetGender() );

				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
				if( pInfo == 0 )
					return false;

				/// 캐릭터 채널링에서는 캐스팅이 없다.
				if( pInfo->mShotType == eSHOTTYPE_CASTING )
					return false;

				if( pInfo->mShotType == eSHOTTYPE_CHANNELING )
				{
					if( pPlayer && pPlayer == HERO )
					{
						/// casting ui 등록... 
						sKeepSkill* keep = SKILLMAN->GetKeepInfo( mSkillInfo.skillIdx );
						if( keep )
						{
							mCastingTime = HERO->CalcCastingExt( keep->mCastingTime );
							cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
							if( pWin )
							{
								pWin->SetTime( mSkillInfo.skillIdx, eCasting_Skill, mCastingTime );
							}
						}
						else
						{
							assert(0);
						}
					}
				}

				mTransformMonSkill = false;
			}
			else
			{
				unsigned long classId = pPlayer->GetTransMonsterClassIdx();
				mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
				if( pSkillInfo == 0 )
					return false;

				if( pSkillInfo->mShotType == eMONSTERSHOTTYPE_CASTING )
				{
					bCasting = true;

					if( pPlayer && pPlayer == HERO )
					{
						mCastingTime = HERO->CalcCastingExt( pSkillInfo->mCastingTime );
						cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
						if( pWin )
						{
							pWin->SetTime( mSkillInfo.skillIdx, eCasting_Skill, mCastingTime );
						}
					}
				}

				mTransformMonSkill = true;
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMonster = (cMonster*)pUser;
			unsigned long classId = pMonster->GetMonsterClassIdx();

			mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( classId, (eMONSTERATTACK_TYPE)skillIdx );
			if( pSkillInfo == 0 )
				return false;

			if( pSkillInfo->mShotType == eMONSTERSHOTTYPE_CASTING )
				bCasting = true;
		}
		break;
	default:
		assert(0);
		return false;
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

bool cChannelingSkillObject::RevisionInit( unsigned long skillIdx, unsigned char skillStep, unsigned char userType, unsigned long dramaKey, bool transMon, float fieldX, float fieldY, unsigned long playerIdx )
{
	if( mSpecialType > eSpecial_Field )
		return false;

	mSkillInfo.skillIdx = skillIdx;
	mSkillInfo.skillStep = skillStep;
	mRemoved = false;

	/// 사용자 와 타겟의 정보는 기본값
	mUserObj.index = playerIdx;
	mUserObj.type = userType;
	mTargetObj.index = 0;
	mTargetObj.type = eOBJECTTYPE_NONE;

	//	mTargetObjArray.Reserve( 10 );

	mVerifyState = eVerify_Success;

	mFieldPos.x = fieldX;
	mFieldPos.y = fieldY;

	float height = 0.0f;
	if( WORLDMAN && WORLDMAN->CalcHeight( &height, fieldX, fieldY ) )
	{
		mFieldPos.z += height;
	}
	else
	{
		assert( 0 );
		return false;
	}

	/// player skill || monster skill
	switch( mUserObj.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			if( transMon == false )
			{
				/// dramaKey = (((race*eGENDER_MAX)+gender)*eWEAPON_STATE_MAX + weapon);
				mpDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( mSkillInfo.skillIdx, (unsigned short)dramaKey );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillInfo.skillIdx );
				if( pInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
			else
			{
				/// dramaKey = monster class index
				mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
				if( mpDramaInfo == 0 )
				{
					assert(0);
					return false;
				}

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
				if( pSkillInfo == 0 )
				{
					assert(0);
					return false;
				}
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			/// dramaKey = monster class index
			mpDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
			if( mpDramaInfo == 0 )
			{
				assert(0);
				return false;
			}

			sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( dramaKey, (eMONSTERATTACK_TYPE)mSkillInfo.skillIdx );
			if( pSkillInfo == 0 )
			{
				assert(0);
				return false;
			}
		}
		break;
	default:
		assert(0);
		return false;
	}

	/// 연출을 위한 정보를 참조
	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	ChangeDramaState( eDRAMASTATE_APPLY );

	// apply 연출을 한다.
	ProcessApply( THEAPP->GetWorldAccumTime() );

	ChangeDramaState( eDRAMASTATE_ACTIVITY );
		
	/// 지면에 채널링 연출을 한다.
	sDramaActionBase* pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_ACTIVITY );
	mActivityController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_ACTIVITY );
	cBaseObject* pUser = OBJECTMAN->GetObject( &mUserObj );

	while( pAct )
	{
		if( pAct->actionType != eDRAMA_CHANNELING )
		{
			pAct = pAct->pNext;
			continue;
		}
		else
		{
			if( pUser )
			{
				if( pUser->GetState() != eOBJECT_STATE_DIE )
				{
					UpdateChannelingDramaObject( pAct, pUser, mFieldPos );
					mChannelingActProcessed = true;
				}
			}

			pAct = pAct->pNext;
			continue;
		}
	}

	/// 채널링 애니메이션 적용
	if( pUser )
	{
		if( mActivityController.animationIdx != UINT_MAX )
		{
			if( pUser->GetState() != eOBJECT_STATE_DIE )
			{
				pUser->UpdateAnimationSpeed( mSpeedFactor );
				pUser->UpdateSkillAnimation( mActivityController.animationIdx );
				mUsedAnimation = true;
			}
		}
		else
		{
			/// 동작이 없는 스킬을 사용하는 경우 보정
			if( mIsNoneStop == false )
			{
				if( mUserObj.type == eOBJECTTYPE_HERO || mUserObj.type == eOBJECTTYPE_PLAYER )
				{
					((cPlayer*)pUser)->ChangeAnimation( ANITYPE_IDLE );
				}
				else if( mUserObj.type == eOBJECTTYPE_MONSTER )
				{
					pUser->UpdateSkillAnimation( M_ANITYPE_IDLE1 );
				}
			}
		}
	}

	mApplyController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_APPLY );
	mApplyController.animationIdx = mpDramaInfo->GetAnimation( eDRAMASTATE_APPLY );
	mApplyController.startTime = ULONG_MAX;

	for( unsigned int i = 0, end = mTargetObjArray.GetSize(); i < end; ++i )
	{
		delete (sDamageInfo*)mTargetObjArray[i];
	}
	mTargetObjArray.Clear();

	mVerifyState = eVerify_Wait;

	/// 기존 이펙트/사운드 다시 세팅
	mActivityController.pAct = mpDramaInfo->GetStartAction( eDRAMASTATE_ACTIVITY );
	mActivityController.startTime = THEAPP->GetWorldAccumTime();

	return true;
}
