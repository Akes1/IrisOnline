#include "StdAfx.h"
#include "DropItemSceneNode.h"
#include "NifAnimationInfo.h"

#include "NaviMesh.h"
#include "EffectSceneNode.h"

#include "SceneManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Signboard.h"
#include "CameraManager.h"
#include "RenderSystem.h"

cDropItemSceneNode::cDropItemSceneNode(eType type)
: cManagedStaticSceneNode(type)
, mDropIndex(UINT_MAX)
, mDropStartPos(0, 0)
, mDropEndPos(0, 0)
, mIsRareItem( false )
, mpTutorialBoard(0)
{
	memset( mOwner, 0, sizeof(unsigned long)*MAX_ITEM_OWNER );
	mTimeToOwn = 0;

	memset( mParty, 0, sizeof(unsigned long)*MAX_ITEM_PARTY );
	mTimeToParty = 0;

	mOwnerEffect = 0;
	mEffectIdxByManager = (unsigned long)-1;
}

cDropItemSceneNode::~cDropItemSceneNode()
{
	SAFE_DELETE( mpTutorialBoard );

	ClearCollectInfo();

	ClearEffect();
}

void cDropItemSceneNode::ClearEffect()
{
	if( mEffectIdxByManager != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdxByManager );
		if( node )
		{
			assert( node == mOwnerEffect );
			node->Remove();
		}
		mOwnerEffect = 0;
		mEffectIdxByManager = (unsigned long)-1;
	}
}

void cDropItemSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mOwnerEffect )
	{
		if( SCENEMAN->IsValidEffectNode( mEffectIdxByManager, mOwnerEffect ) == false )
		{
			assert(0);
			GameErrorLog( "cDropItemSceneNode::OnProcess[%d,%d]", mDropIndex, mEffectIdxByManager );
		}
	}

	cManagedStaticSceneNode::OnProcess( deltaTime, accumTime );

	///
	if( mOwnerEffect == 0 )
	{
		bool createEff = false;

		// 소유자 시간 체크
		if( mTimeToOwn != 0 )
		{
			if( mTimeToOwn <= deltaTime )
			{
				mTimeToOwn = 0;

				if( mTimeToParty > 0 )
				{
					unsigned long heroId = HERO->GetObjectID();
					for( unsigned int i=0; i<MAX_ITEM_PARTY; i++ )
					{
						if( heroId == mParty[i] )
						{
							createEff = true;
							break;
						}
					}
				}
				else
				{
					createEff = true;
				}
			}
			else
			{
				mTimeToOwn -= deltaTime;
			}
		}

		// 파티 시간 체크
		if( mTimeToParty != 0 )
		{
			if( mTimeToParty <= deltaTime )
			{
				mTimeToParty = 0;
				createEff = true;
			}
			else
			{
				mTimeToParty -= deltaTime;
			}
		}

		if( createEff == true )
		{
			mTimeToOwn = 0;
			mTimeToParty = 0;

			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = GetWorldTranslate();

			if( mIsRareItem )
				mOwnerEffect = SCENEMAN->CreateSelfEffect( "./Data/Effect/eff_pc_itemdrop_02.nif", true, trans, true );
			else
				mOwnerEffect = SCENEMAN->CreateSelfEffect( "./Data/Effect/eff_pc_itemdrop_01.nif", true, trans, true );

			if( mOwnerEffect )
				mEffectIdxByManager = mOwnerEffect->GetIndexByManger();
		}
	}

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	if( mpTutorialBoard )
		mpTutorialBoard->Update( accumTime );
}

bool cDropItemSceneNode::Init( cDropItemSceneNodeParam& param )
{
	if( param.mDropIndex == UINT_MAX )
	{
		assert(0);
		return false;
	}

	/// 아이템 정보들을 받는다.
	mDropIndex = param.mDropIndex;
	mDropStartPos = param.mDropStartPos;
	mDropEndPos = param.mDropEndPos;
	mIsRareItem = param.mIsRareItem;

	bool createEff = false;

	mTimeToOwn = param.mTimeToOwn;
	mTimeToParty = param.mTimeToParty;

	unsigned long heroId = HERO->GetObjectID();
	if( mTimeToOwn != 0 )
	{
		for( unsigned int i=0; i<MAX_ITEM_OWNER; i++ )
		{
			mOwner[i] = param.mOwner[i];

			if( heroId == mOwner[i])
				createEff = true;
		}

		memcpy( mParty, param.mParty, sizeof(mParty) );
	}

	if( mTimeToParty != 0 )
	{
		for( unsigned int i=0; i<MAX_ITEM_PARTY; i++ )
		{
			mParty[i] = param.mParty[i];

			// 소유자가 없을시에만 파티 체크
			if( mTimeToOwn == 0 )
			{
				if( heroId == mParty[i])
					createEff = true;
			}
		}
	}
	
	if( mTimeToOwn == 0 && mTimeToParty == 0 )
	{
		createEff = true;
	}

	param.mTranslate.x = mDropEndPos.x;
	param.mTranslate.y = mDropEndPos.y;

	NAVIMESH->CalcHeight( &param.mTranslate.z, param.mTranslate.x, param.mTranslate.y );

	if( cManagedStaticSceneNode::Init( param ) == false )
	{
		return false;
	}

	if( createEff )
	{
		mTimeToOwn = 0;
		mTimeToParty = 0;

		NiTransform trans;
		trans.MakeIdentity();
		trans.m_Translate = GetWorldTranslate();

		if( param.mIsRareItem )
		{
			mOwnerEffect = SCENEMAN->CreateSelfEffect( "./Data/Effect/eff_pc_itemdrop_02.nif", true, trans, true );
		}
		else
			mOwnerEffect = SCENEMAN->CreateSelfEffect( "./Data/Effect/eff_pc_itemdrop_01.nif", true, trans, true );

		if( mOwnerEffect )
			mEffectIdxByManager = mOwnerEffect->GetIndexByManger();
	}

	mpTutorialBoard = new cTutorialSignboard( eOBJECTTYPE_ITEM, param.mDropIndex, 0 );
	return true;
}

bool cDropItemSceneNode::IsHeroOwner()
{
	return (mEffectIdxByManager != (unsigned long)-1);
}

void cDropItemSceneNode::AddToVisibleArray()
{
	cSceneNode::AddToVisibleArray();

	if( IsHeroOwner() == false )
		return;

	if( mpTutorialBoard && mpTutorialBoard->IsActive() )
	{
		NiPoint3 pos = this->GetWorldTranslate();

		//pos.z += 70;

		float x,y;
		cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y, false );
		int ScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
		int ScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );

		SCENEMAN->AddPlane( mpTutorialBoard );
		mpTutorialBoard->SetPos( ScreenPosX + mpTutorialBoard->GetPosX(), ScreenPosY + mpTutorialBoard->GetPosY() );
	}
}

void cDropItemSceneNode::UpdateTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->UpdateImage();
}

void cDropItemSceneNode::ClearTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->ClearImage();
}
