#include "stdafx.h"
#include "BaseObject.h"
#include "ObjectManager.h"
#include "DynamicSceneNode.h"
#include "SceneManager.h"

#include "WorldManager.h"
#include "CameraManager.h"
#include "DamagePrintManager.h"

#include "SkillManager.h"
#include "Hero.h"
#include "GameUIManager.h"
#include "TargetWindow.h"
#include "SkillScript.h"

cBaseObject::cBaseObject( unsigned char type )
: mpObjectSceneNode(0)
, mObjectType(type)
{
	mTarget.type = eOBJECTTYPE_NONE;
	mTarget.index = 0;

	mReadyUseSkill = true;

//	mStatureValue = 0.0f;

	mIdleTime = 0;

	for( unsigned int i=0;i<eANIEVENT_MAX;i++ )
		mRevisionEventTime[i] = 0.0f;
}

cBaseObject::~cBaseObject()
{
	RemoveObjectSceneNode();

	ClearAllBuff( true );
	ClearAllDeBuff( true );
}

cBaseObject* cBaseObject::GetTargetObject()
{
	return OBJECTMAN->GetObject( mTarget.type, mTarget.index );
}

void cBaseObject::SetTargetObject( unsigned char type, unsigned long idx )
{
	mTarget.type = type;
	mTarget.index = idx;
}


/// buff 처리
void cBaseObject::AddBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	if( mBuffMap.Insert( uniqueIdx, p ) == false )
	{
		assert(0);
		return;
	}

	if( HERO->GetTargetObject() == this )
	{
		cTargetWindow* targetWindow = GAMEUI->GetTargetWindow();
		if( targetWindow )
		{
			/// 모든 버프류 삽입( 버프, 디버프 내부 구별 )
			targetWindow->AddBuff( uniqueIdx, p->GetInfluenceIdx() );
		}
	}
}

void cBaseObject::DelBuff( unsigned long uniqueIdx )
{
	mBuffMap.Erase( uniqueIdx );

	/// 대상타겟이 동일하면 버프아이콘 삭제
	if( HERO->GetTargetObject() == this )
	{
		cTargetWindow* targetWindow = GAMEUI->GetTargetWindow();
		if( targetWindow )
		{
			targetWindow->DelBuff( uniqueIdx );
		}
	}
}

void cBaseObject::AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p )
{
	if( p == 0 )
		return;

	if( mDeBuffMap.Insert( uniqueIdx, p ) == false )
	{
		assert(0);
		return;
	}

	if( HERO->GetTargetObject() == this )
	{
		cTargetWindow* targetWindow = GAMEUI->GetTargetWindow();
		if( targetWindow )
		{
			/// 모든 버프류 삽입( 버프, 디버프 내부 구별 )
			targetWindow->AddBuff( uniqueIdx, p->GetInfluenceIdx() );
		}
	}
}

void cBaseObject::DelDeBuff( unsigned long uniqueIdx )
{
	mDeBuffMap.Erase( uniqueIdx );

	/// 대상타겟이 동일하면 버프아이콘 삭제
	if( HERO->GetTargetObject() == this )
	{
		cTargetWindow* targetWindow = GAMEUI->GetTargetWindow();
		if( targetWindow )
		{
			targetWindow->DelDeBuff( uniqueIdx );
		}
	}
}

void cBaseObject::RemoveObjectSceneNode()
{
	try {
		if( mpObjectSceneNode )
		{
			/// 등록된 그래픽 객체 삭제
			SCENEMAN->DestroyDynamic( mpObjectSceneNode );
			mpObjectSceneNode = NULL;
		}
	} catch(...){
		GameErrorLog( "cBaseObject::RemoveObjectSceneNode %d, %d", GetObjectID(), GetObjectType() );
		throw;
	}
}

cEffectSceneNode* cBaseObject::SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop, bool bFollow ) 
{ 
	if( ::strlen(nif) == 0 )
		return 0;

	return mpObjectSceneNode->LinkEffect( num, nif, trans, bLoop, bFollow ); 
}

cEffectSceneNode* cBaseObject::SetLinkdDamage( unsigned int num, const char* nif, NiTransform* trans, bool IsMiss, bool zFalse )
{
	if( mpObjectSceneNode && mpObjectSceneNode->IsViewNode() == false )
		return 0;

	if( ::strlen(nif) == 0 )
		return 0;

	return mpObjectSceneNode->LinkDamage( num, nif, trans, IsMiss, zFalse ); 
}

/*
bool cBaseObject::IsLinkedEffect( cEffectSceneNode* pNode )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	return mpObjectSceneNode->IsLinkedEffect( pNode );
}
*/

cSoundSceneNode* cBaseObject::SetLinkdSound( unsigned long soundIdx, bool bLoop )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	return mpObjectSceneNode->LinkSound( 0, soundIdx, bLoop ); //eSOUNDTYPE_EFFECT
}

cSoundSceneNode* cBaseObject::SetStepSound( unsigned long soundIdx )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	return mpObjectSceneNode->LinkSound( 1, soundIdx, false ); //eSOUNDTYPE_NOFADE_EFFECT
}


/*
bool cBaseObject::IsLinkedSound( cSoundSceneNode* pNode )
{
	if( mpObjectSceneNode == 0 )
		return false;

	return mpObjectSceneNode->IsLinkedSound( pNode );
}
*/

float cBaseObject::GetXPos()
{
	assert(mpObjectSceneNode);
	return mpObjectSceneNode->GetTranslate().x;
}

float cBaseObject::GetYPos()
{
	assert(mpObjectSceneNode);
	return mpObjectSceneNode->GetTranslate().y;
}

float cBaseObject::GetZPos()
{
	assert(mpObjectSceneNode);
	return mpObjectSceneNode->GetTranslate().z;
}

NiPoint3 cBaseObject::GetPos()
{
	assert(mpObjectSceneNode);
	return mpObjectSceneNode->GetTranslate();
}

void cBaseObject::SetPos( const NiPoint3& pos )
{
	assert(mpObjectSceneNode);
	mpObjectSceneNode->SetTranslate( pos );
}

void cBaseObject::SetWorldPos( const NiPoint3& pos )
{
	assert(mpObjectSceneNode);
	mpObjectSceneNode->SetWorldTranslate( pos );
}

void cBaseObject::FixPos( float x, float y, bool resetCamera )
{
	NiPoint3 oldPos;
	oldPos = GetPos();
	oldPos.z = 0.0f;

	NiPoint3 newPos;
	newPos.x = x;
	newPos.y = y;
	newPos.z = 0.0f;

	if( (oldPos - newPos).Length() > SYNC_MOVE_RANGE )
	{
		if( WORLDMAN->CalcHeight( &newPos.z, newPos.x, newPos.y ) )
		{		
			SetPos( newPos );
			cCamera* cam = CAMERAMAN->GetCurrent();

			if( resetCamera && cam )
			{
				cam->ResetLookAt( newPos );
			}

			//TCHAR tempmsg[255] = {0,};
			//if( mObjectType == eOBJECTTYPE_MONSTER )
			//	_stprintf( tempmsg, _T("\n몬스터 좌표보간[%.0f,%.0f] => [%.0f,%.0f]"), oldPos.x, oldPos.y, newPos.x, newPos.y );
			//else if( mObjectType == eOBJECTTYPE_PLAYER )
			//	_stprintf( tempmsg, _T("\n플레이어 좌표보간[%.0f,%.0f] => [%.0f,%.0f]"), oldPos.x, oldPos.y, newPos.x, newPos.y );
			//else if( mObjectType == eOBJECTTYPE_HERO )
			//	_stprintf( tempmsg, _T("\n히어로 좌표보간[%.0f,%.0f] => [%.0f,%.0f]"), oldPos.x, oldPos.y, newPos.x, newPos.y );
			//OutputDebugString( tempmsg );
		}
		else
		{
			assert(NULL);
			return;
		}
	}

}

void cBaseObject::SetPos( float x, float y, float z )
{
	NiPoint3 pos = NiPoint3(x, y, z);
	SetPos( pos );
}

NiMatrix3 cBaseObject::GetModelRot() 
{ 
	if( mpObjectSceneNode )
		return mpObjectSceneNode->GetRotate();
	else
		return NiMatrix3::IDENTITY;
}

float cBaseObject::GetModelScale()
{
	if( mpObjectSceneNode )
		return mpObjectSceneNode->GetScale();
	else
		return 0.0f;
}

bool cBaseObject::GetDummyPos( unsigned int dummyId, NiPoint3& pos )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	NiAVObject* pObj = NiDynamicCast( NiAVObject, mpObjectSceneNode->GetDummyObject( dummyId ) );
	if( pObj )
	{
		pos = pObj->GetWorldTransform().m_Translate;
		return true;
	}
	return false;
}

void cBaseObject::UpdateAnimationSpeed(float val)
{
	assert(mpObjectSceneNode);
	mpObjectSceneNode->UpdateAniScaleFactor( val );
}

float cBaseObject::GetAnimationSpeed()
{
	if( mpObjectSceneNode )
		return mpObjectSceneNode->GetAniScaleFactor();

	return 1.0f;
}


/// 이 함수는 skillobject에서만 호출 된다.
void cBaseObject::UpdateSkillAnimation( unsigned int aniIdx, bool /*checkWeapon*/ )
{
	if( aniIdx == UINT_MAX )
		return;

	assert(mpObjectSceneNode);
	if( mpObjectSceneNode->UpdateTargetAnimation( aniIdx ) == false )
	{
		//assert(0 && "Not have animation");
		if( IsReadyUseSkill() == false )
		{
			SetReadyUseSkill( true );
			ResetState();
		}
	}
}

void cBaseObject::ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker )
{
	if( damage < 0 )
		return;

	bool heroOwner = false;
	if( GetObjectType() == eOBJECTTYPE_HERO )
	{
		heroOwner = true;
	}
	else
	{
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_HERO )
		{
			heroOwner = true;
		}
	}
	DAMAGEPRINTMAN->PrintDamage( this, damage, (eDAMAGEPRINT_TYPE)damageType, heroOwner );
}

void cBaseObject::SetOverFlag( bool over ) 
{ 
	mpObjectSceneNode->SetSelectLightFlag( over ); 
}

//void cBaseObject::ClearAllBuff()
//{
//	cInfluenceMap::cIterator i = mBuffMap.Begin();
//	cInfluenceMap::cIterator end = mBuffMap.End();
//
//	for( ; i != end; ++i )
//	{
//		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
//		if( p )
//			SKILLMAN->DeleteInfluenceObject( p );
//	}
//	mBuffMap.Clear();
//}
void cBaseObject::ClearAllBuff( bool delDieInf )
{
	cInfluenceMap::cIterator i = mBuffMap.Begin();
	cInfluenceMap::cIterator end = mBuffMap.End();

	while( i != end )
	{
		unsigned long idx = (*i).mFirst;
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		++i;
		if( p )
		{
			if( delDieInf == true ) 
			{
				SKILLMAN->DeleteInfluenceObject( idx );
				mBuffMap.Erase( idx );
			}
			else
			{
				/// 090302 PKH 죽었을때 지워지지 않는 효과 확인후 삭제
				if( p->GetDeleteType() != eDELETETYPE_NODELETE_PLAYER_DIE )
				{
					SKILLMAN->DeleteInfluenceObject( idx );
					mBuffMap.Erase( idx );
				}
			}
		}
	}
}

//void cBaseObject::ClearAllDeBuff()
//{
//	cInfluenceMap::cIterator i = mDeBuffMap.Begin();
//	cInfluenceMap::cIterator end = mDeBuffMap.End();
//
//	for( ; i != end; ++i )
//	{
//		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
//		if( p )
//			SKILLMAN->DeleteInfluenceObject( p );
//	}
//	mDeBuffMap.Clear();
//}
void cBaseObject::ClearAllDeBuff( bool delDieInf )
{
	cInfluenceMap::cIterator i = mDeBuffMap.Begin();
	cInfluenceMap::cIterator end = mDeBuffMap.End();

	while( i != end )
	{
		unsigned long idx = (*i).mFirst;
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		++i;
		if( p )
		{
			if( delDieInf == true ) 
			{
				SKILLMAN->DeleteInfluenceObject( idx );
				mBuffMap.Erase( idx );
			}
			else
			{
				/// 090302 PKH 죽었을때 지워지지 않는 효과 확인후 삭제
				if( p->GetDeleteType() != eDELETETYPE_NODELETE_PLAYER_DIE )
				{
					SKILLMAN->DeleteInfluenceObject( idx );
					mBuffMap.Erase( idx );
				}
			}
		}
	}
}

sTrailInfo* cBaseObject::CreateTrailEffect( cString str, unsigned long lifeTime, unsigned int link1, unsigned int link2, float lenPer, float factor )
{
	if( mpObjectSceneNode == 0 )
	{
		assert(0);
		return 0;
	}

	return mpObjectSceneNode->ActiveTrailEffect( str, lifeTime, link1, link2, lenPer, factor );
}

void cBaseObject::DeleteTrailEffect( sTrailInfo* pInfo )
{
	mpObjectSceneNode->DeActiveTrailEffect( pInfo );
}

float cBaseObject::GetStatureValue()
{
	if( mpObjectSceneNode == 0 )
		return 0.0f;

	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();

    return sHead.z - sFoot.z;

//	return mStatureValue;
//	return mStatureValue * mpObjectSceneNode->GetWorldScale();
}

void cBaseObject::FadeOutObject()
{
	if( mpObjectSceneNode )
		mpObjectSceneNode->SetAlphaBlended( 0.01f );
}

bool cBaseObject::IsBuffHave( unsigned long influenceIdx )
{
	cInfluenceMap::cIterator i = mBuffMap.Begin();
	cInfluenceMap::cIterator end = mBuffMap.End();
	for( ; i != end; ++i )
	{
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		if( p )
		{
			if( p->GetInfluenceIdx() == influenceIdx )
				return true;
		}
	}

	return false;
}

bool cBaseObject::IsHaveApplyBuffType( unsigned short applyValueType )
{
	cInfluenceMap::cIterator i = mBuffMap.Begin();
	cInfluenceMap::cIterator end = mBuffMap.End();
	for( ; i != end; ++i )
	{
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		if( p )
		{
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( p->GetInfluenceIdx() );
			if( pScript )
			{
				if( pScript->mType == eINFLUENCETYPE_BUF )
				{
					if( pScript->mStatusPlusIdx1 == applyValueType ||
						pScript->mStatusPlusIdx2 == applyValueType ||
						pScript->mStatusPlusIdx3 == applyValueType )
						return true;
				}
// 				else	/// PC, CASH, GMEVENT 종류가 여기 추가됨 
// 					assert(0);
			}
		}
	}

	return false;
}

bool cBaseObject::IsHaveApplyDeBuffType( unsigned short applyValueType )
{
	cInfluenceMap::cIterator i = mDeBuffMap.Begin();
	cInfluenceMap::cIterator end = mDeBuffMap.End();
	for( ; i != end; ++i )
	{
		cInfluenceObject* p = (cInfluenceObject*)(*i).mSecond;
		if( p )
		{
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( p->GetInfluenceIdx() );
			if( pScript )
			{
				if( pScript->mType == eINFLUENCETYPE_DEBUF )
				{
					if( pScript->mStatusPlusIdx1 == applyValueType ||
						pScript->mStatusPlusIdx2 == applyValueType ||
						pScript->mStatusPlusIdx3 == applyValueType )
						return true;
				}
				else
					assert(0);
			}
		}
	}

	return false;
}
