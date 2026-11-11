#include "StdAfx.h"
#include "nifanimationinfo.h"


//---------------------------------------------------------------------------
cNifTimeManager::cNifTimeManager()
: m_fAccumTime(0.0f),
m_fScaleFactor(1.0f), 
m_fStartTime(0.0f), 
m_fEndTime(0.0f),
m_bTimingEnabled(true), 
m_fLastTime(-NI_INFINITY)
{ 
}
//---------------------------------------------------------------------------
float cNifTimeManager::GetFrameTime( float fTime )
{
	if (!m_bTimingEnabled)
	{
		return m_fAccumTime;
	}

	float fDelta = fTime;
	fDelta *= m_fScaleFactor;

	float accumTime = m_fAccumTime + fDelta;
	if( accumTime > m_fEndTime )
	{
		m_fAccumTime = m_fStartTime + ( accumTime - m_fEndTime );
	}
	else
		m_fAccumTime = accumTime;

	return m_fAccumTime;

	/*
	float fCurrentTime = fTime;
	if (m_fLastTime == -NI_INFINITY)
	{
	m_fLastTime = fCurrentTime;
	}

	float fDelta = fCurrentTime - m_fLastTime;
	fDelta *= m_fScaleFactor;

	m_fAccumTime += fDelta;

	if (m_fAccumTime < m_fStartTime)
	{
	m_fAccumTime = m_fStartTime;
	}
	else if (m_eTimeMode == LOOP && m_fAccumTime > m_fEndTime)
	{
	if (m_fStartTime == m_fEndTime)
	{
	m_fAccumTime = m_fEndTime;
	}
	else
	{
	m_fAccumTime -= m_fEndTime - m_fStartTime;
	}
	}

	m_fLastTime = fCurrentTime;
	return m_fAccumTime;
	*/
}

float cNifTimeManager::GetRewTime( float fTime )
{
	if (!m_bTimingEnabled)
	{
		return m_fAccumTime;
	}

	float fDelta = fTime;
	fDelta *= m_fScaleFactor;

	float accumTime = m_fAccumTime - fDelta;
	if( accumTime < m_fStartTime )
	{
		m_fAccumTime = m_fEndTime - (fDelta - m_fAccumTime);
	}
	else
		m_fAccumTime = accumTime;

	return m_fAccumTime;
}

//---------------------------------------------------------------------------
void  cNifTimeManager::SetFrameTime(float fTime)
{ 
	//	if(m_fAccumTime != fTime)
	//	{
	m_fAccumTime = fTime;
	//	}
}
//--------------------------------------------------------------------------


cNifAnimationInfo::cNifAnimationInfo()
{
	mStartTime = FLT_MAX;
	mMinTime = FLT_MAX;
	mMaxTime = 0.0f;
	mKeyframeCount = 0;
	mControllerCount = 0;

	mLooping = false;
	mEnable = true;

	mStopFlag = true;
}

cNifAnimationInfo::~cNifAnimationInfo()
{
	mControllerList.RemoveAll();
}

/// 주의사항 : 
void cNifAnimationInfo::CollectData( NiNode* pRoot )
{
	mMinTime = FLT_MAX;
	mMaxTime = 0.0f;
	mKeyframeCount = 0;

	FindAndAdd( pRoot );

	float t = NiTimeController::GetMinBeginKeyTime( pRoot );
	if( t != NI_INFINITY )
	{
		float startTime = t;
		if( startTime < mMinTime )
			mMinTime = startTime;
	}
}

/// 주의사항 : 
void cNifAnimationInfo::Add(NiTimeController* pController)
{
	if(pController == NULL)
	{
		return;
	}

	AddInfo(pController);

	float t = pController->GetBeginKeyTime();
	if( t != NI_INFINITY )
	{
		if( t < mMinTime )
			mMinTime = t;
	}
	t = pController->GetEndKeyTime();
	if( t != NI_INFINITY )
	{
		if(t > mMaxTime)
			mMaxTime = t;
	}

	mControllerCount++;

	if(NiIsKindOf(NiInterpController, pController))
		AddInterpInfo((NiInterpController*) pController);
}

/// 주의사항 : 
void cNifAnimationInfo::FindAndAdd(NiObjectNET* pRoot)
{
	if(pRoot == NULL)
		return;

	NiTimeController* pController = pRoot->GetControllers();

	// Gather all time controllers from this object
	while(pController != NULL)
	{
		Add(pController);
		pController = pController->GetNext();
	}

	if(NiIsKindOf(NiAVObject, pRoot))
	{
		// NiProperties can have time controllers, so search them too
		NiAVObject* pObj = (NiAVObject*)pRoot;
		NiPropertyList* pPropList = &(pObj->GetPropertyList());
		NiTListIterator iter = pPropList->GetHeadPos();
		while(pPropList != NULL && !pPropList->IsEmpty()&& iter)
		{
			NiProperty* pProperty = pPropList->GetNext(iter);
			if(pProperty)
				FindAndAdd(pProperty);
		}
	}
	if(NiIsKindOf(NiNode, pRoot))
	{
		NiAVObject* pObj = 0;
		NiNode* pNode = (NiNode*)pRoot;
		// Search all of the children
		for(unsigned int ui = 0; ui < pNode->GetArrayCount(); ui++)
		{
			pObj = pNode->GetAt(ui);
			if( pObj )
				FindAndAdd(pObj);
		}
		// NiDynamicEffects can have time controllers, so search them too
		const NiDynamicEffectList* pEffectList= &(pNode->GetEffectList());

		NiTListIterator iter = pEffectList->GetHeadPos();
		while(pEffectList != NULL && !pEffectList->IsEmpty() && iter)
		{
			NiDynamicEffect* pEffect = pEffectList->GetNext(iter);
			if( pEffect )
				FindAndAdd( pEffect );
		}
	}
}

/// 주의사항 : 
void cNifAnimationInfo::AddInfo( NiTimeController* pController )
{
	/// 만약 컨트롤러들에 대한 참조를 갖으려면 이곳에..
	mControllerList.AddTail( pController );

	if( pController->GetCycleType() == NiTimeController::LOOP )
		mLooping = true;
}

/// 주의사항 : 
void cNifAnimationInfo::AddInterpInfo( NiInterpController* pController )
{
	for( unsigned short us = 0; us <  pController->GetInterpolatorCount(); us++ )
	{
		NiInterpolator* pInterp = pController->GetInterpolator( us );
		if( NiIsKindOf(NiKeyBasedInterpolator, pInterp) )
		{
			NiKeyBasedInterpolator* pKeyInterp = (NiKeyBasedInterpolator*)pInterp;
			for( unsigned short usc = 0; usc < pKeyInterp->GetKeyChannelCount(); usc++ )
			{
				mKeyframeCount += pKeyInterp->GetKeyCount( usc );
			}
		}
	}
}

/// 주의사항 : 
void cNifAnimationInfo::SetAnimType( NiTimeController::AnimType type )
{
	NiTimeController* pcon = 0;
	NiTListIterator pos = mControllerList.GetHeadPos();
	while( pos )
	{
		pcon = mControllerList.GetNext(pos);
		if( pcon )
			pcon->SetAnimType( type );
	}
}

void cNifAnimationInfo::SetCycleType( NiTimeController::CycleType type )
{
	NiTimeController* pcon = 0;
	NiTListIterator pos = mControllerList.GetHeadPos();
	while( pos )
	{
		pcon = mControllerList.GetNext(pos);
		if( pcon )
			pcon->SetCycleType( type );
	}
}

void cNifAnimationInfo::SetTarget( NiObjectNET* obj )
{
	NiTimeController* pcon = 0;
	NiTListIterator pos = mControllerList.GetHeadPos();
	while( pos )
	{
		pcon = mControllerList.GetNext(pos);
		if( pcon )
			pcon->SetTarget( obj );
	}
}

/// 주의사항 : 
unsigned char cNifAnimationInfo::Update( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	unsigned char ev = eEvent_None;
	if( mEnable == false ) 
		return ev;

	float checkTime = (float)accumTime * 0.001f;

	/// 
	if( mStartTime == FLT_MAX )
		Start( checkTime, true );

	/// 전체 종료 여부 확인..
	if( mMaxTime - mMinTime <= checkTime - mStartTime )
//	if( mMaxTime <= checkTime - mStartTime )
	{
		if( mLooping )
			return eEvent_Ing;

		Stop();
//		mEnable = false;
		ev = eEvent_End;
	}

	return ev;
}

/// 주의사항 : 
void cNifAnimationInfo::Start( float time, bool begin )
{
	mStartTime = time;

	NiTimeController* pcon = 0;
	NiTListIterator pos = mControllerList.GetHeadPos();
	while( pos )
	{
		pcon = mControllerList.GetNext(pos);
		if( pcon )
		{
			if( begin || mStopFlag == true )
				pcon->Start( time );

			//if( NiIsKindOf(NiMorphWeightsController, pcon) )
			//{
			//	NiMorphWeightsController* con = (NiMorphWeightsController*)pcon;
			//	con->SetManagerControlled( false );
			//}
		}
	}

	mEnable = true;
}

/// 주의사항 : 
void cNifAnimationInfo::Stop()
{
	NiTimeController* pcon = 0;
	NiTListIterator pos = mControllerList.GetHeadPos();
	while( pos )
	{
		pcon = mControllerList.GetNext(pos);
		if( pcon )
		{
			if( mStopFlag == true )
				pcon->Stop();

			//if( NiIsKindOf(NiMorphWeightsController, pcon) )
			//{
			//	NiMorphWeightsController* con = (NiMorphWeightsController*)pcon;
			//	con->SetManagerControlled( true );
			//}
		}
	}

	mEnable = false;
}
