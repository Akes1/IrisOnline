#include "StdAfx.h"
#include "CardDramaobject.h"

#include "SceneManager.h"

#include "SelectStage.h"
#include "MakeStage.h"

#include "Player_Common.h"


NiRect<float> cCardDramaObject::mUVPoint[eUV_MAX] = { NiRect<float>( 0.0f, 0.0f, 0.0f, 0.0f), };

cCardDramaObject::cCardDramaObject()
: mSceneObject(0) 
{
	/// uvPoint 셋팅

	for( unsigned int i=0; i<eUV_MAX; i++ )
	{
		unsigned int col = i%9;
		unsigned int row = i/9;

		mUVPoint[i].m_left = 225.0f * (float)col / CARDDATA_WIDTH;
		mUVPoint[i].m_right = 225.0f * (float)(col+1) / CARDDATA_WIDTH;

		mUVPoint[i].m_top = 338.0f * (float)row / CARDDATA_HEIGHT;
		mUVPoint[i].m_bottom = 338.0f * (float)(row+1) / CARDDATA_HEIGHT;
	}
}

cCardDramaObject::~cCardDramaObject()
{
	if( mSceneObject )
	{
		/// 등록된 그래픽 객체 삭제
		SCENEMAN->DestroyDynamic( mSceneObject );
		mSceneObject = NULL;
	}
}

void cCardDramaObject::Update( unsigned long time )
{
	Interpret( time );
}

void cCardDramaObject::SetTargetAnimation( unsigned int aniIdx )
{
	if( mSceneObject )
		mSceneObject->SetTargetAnimation( aniIdx );
}

void cCardDramaObject::SetCardImage( unsigned int cardIdx, unsigned char race, unsigned char gender )
{
	unsigned int idx = race * eGENDER_MAX + gender;
	idx += 1;

	mSceneObject->ChangeTextureInfo( cardIdx, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
}

//////////////////////////////////////////////////////////////////////////
bool cSelectCardDramaObject::Init( const char* pathname )
{
	cCardSceneNodeParam param;
	param.mPathName = pathname;
	param.mStage = eStage_Select;

	mSceneObject = SCENEMAN->CreateCard( param );
	if( mSceneObject == 0 )
		return false;

	return true;
}

void cSelectCardDramaObject::Interpret( unsigned long time )
{
	if( mSceneObject == 0 )
	{
		assert( 0 );
		return;
	}

	NiActorManager*	pActor = mSceneObject->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mSceneObject->GetTargetAnimation();

	/// 현재 애니메이션 종료 시점 획득

	float endTime = pActor->GetNextEventTime( NiActorManager::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;

	float checkTime = time * 0.001f;
	if( endTime > 0 && endTime <= checkTime )
	{
		if( id == cSelectStageCardSceneNode::eANI_SELECT_CREATE )
		{
			mpStage->SetViewState( cSelectStage::eVIEW_LIST );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cSelectStageCardSceneNode::eANI_SELECT_LIST_STAND );
		}
		else if( id >= cSelectStageCardSceneNode::eANI_SELECT_GATHER && id < cSelectStageCardSceneNode::eANI_SELECT_GATHER_END )
		{
			mpStage->CardSelect();
			//mpStage->SetViewState( cSelectStage::eVIEW_CHANNEL );
			mSceneObject->ClearOverIndex();
		}
		else if( id >= cSelectStageCardSceneNode::eANI_SELECT_RETURN && id < cSelectStageCardSceneNode::eANI_SELECT_RETURN_END )
		{
			mpStage->SetViewState( cSelectStage::eVIEW_LIST );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cSelectStageCardSceneNode::eANI_SELECT_LIST_STAND );
		}
	}
	else if( endTime == NiActorManager::INVALID_TIME )
	{
	}
}

//////////////////////////////////////////////////////////////////////////
bool cMakeCardDramaObject::Init( const char* pathname )
{
	cCardSceneNodeParam param;
	param.mPathName = pathname;
	param.mStage = eStage_Make;

	mSceneObject = SCENEMAN->CreateCard( param );
	if( mSceneObject == 0 )
		return false;

	return true;
}

void cMakeCardDramaObject::SetMakeCardImage( unsigned char race )
{
	unsigned int idx = 0;
	switch( race )
	{
	case eRACE_HUMAN:
		{
			idx = 16;
			mSceneObject->ChangeTextureInfo( 0, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 17;
			mSceneObject->ChangeTextureInfo( 1, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );

			idx = 10;
			mSceneObject->ChangeTextureInfo( 2, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 11;
			mSceneObject->ChangeTextureInfo( 3, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
		}
		break;
	case eRACE_BEAST:
		{
			idx = 18;
			mSceneObject->ChangeTextureInfo( 0, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 19;
			mSceneObject->ChangeTextureInfo( 1, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );

			idx = 12;
			mSceneObject->ChangeTextureInfo( 2, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 13;
			mSceneObject->ChangeTextureInfo( 3, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
		}
		break;
	case eRACE_ELF:
		{
			idx = 20;
			mSceneObject->ChangeTextureInfo( 0, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 21;
			mSceneObject->ChangeTextureInfo( 1, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );

			idx = 14;
			mSceneObject->ChangeTextureInfo( 2, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
			idx = 15;
			mSceneObject->ChangeTextureInfo( 3, mUVPoint[idx].m_left, mUVPoint[idx].m_top, mUVPoint[idx].m_right, mUVPoint[idx].m_bottom );
		}
		break;
	}
}

void cMakeCardDramaObject::Interpret( unsigned long time )
{
	if( mSceneObject == 0 )
	{
		assert( 0 );
		return;
	}

	NiActorManager*	pActor = mSceneObject->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mSceneObject->GetTargetAnimation();

	/// 현재 애니메이션 종료 시점 획득

	float endTime = pActor->GetNextEventTime( NiActorManager::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;

	float checkTime = time * 0.001f;
	if( endTime > 0 && endTime <= checkTime )
	{
		if( id == cMakeStageCardSceneNode::eANI_MAKE_RACE_CREATE )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_RACE );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_RACE );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_RACE_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_GENDER_CREATE )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_GENDER );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_GENDER );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_GENDER_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_JOB_CREATE )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_JOB );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_JOB );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_JOB_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_DETAIL_CREATE )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_DETAIL );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_DETAIL );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_DETAIL_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_RACE_RETURN )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_RACE );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_RACE );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_RACE_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_GENDER_RETURN )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_GENDER );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_GENDER );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_GENDER_STAND );
		}
		else if( id == cMakeStageCardSceneNode::eANI_MAKE_JOB_RETURN )
		{
			mpStage->SetViewState( cMakeStage::eVIEW_JOB );

			mSceneObject->SetOverStage( cMakeStageCardSceneNode::eOVER_JOB );
			mSceneObject->ClearOverIndex();

			SetTargetAnimation( cMakeStageCardSceneNode::eANI_MAKE_JOB_STAND );
		}
	}
	else if( endTime == NiActorManager::INVALID_TIME )
	{
	}
}