// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not
// be copied or disclosed except in accordance with the terms of that
// agreement.
//
//      Copyright (c) 1996-2008 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net

//---------------------------------------------------------------------------
inline void cActorManagerForPartition::SetCallbackObject(
	CallbackObject* pkCallbackObject)
{
	m_pkCallbackObject = pkCallbackObject;
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::CallbackObject* cActorManagerForPartition::GetCallbackObject()
const
{
	return m_pkCallbackObject;
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::SequenceID cActorManagerForPartition::GetTargetAnimation() const
{
	return m_eTargetID;
}
//---------------------------------------------------------------------------
inline NiSequenceData* cActorManagerForPartition::SetTargetAnimation(SequenceID eSequenceID)
{
	NiSequenceData* pkSequence = 0;
	if( eSequenceID != INVALID_SEQUENCE_ID )
	{
		NiSequenceDataPtr spSeqData;
		if( !m_kSeqDataMap.GetAt(eSequenceID, spSeqData) )
			return 0;

		pkSequence = (NiSequenceData*)spSeqData;
	}
	else
	{
		assert(0);
		return 0;
	}

	m_eTargetID = eSequenceID;
	m_bBuildTimeline = true;

	return pkSequence;
}
//---------------------------------------------------------------------------
inline NiSequenceData* cActorManagerForPartition::UpdateTargetAnimation(SequenceID eSequenceID)
{
	NiSequenceData* pSeqData = GetSequenceData( eSequenceID );
	if( pSeqData == 0 )
	{
		assert(0);
		return 0;
	}

	NiControllerSequence* pkSequence = GetExtraSequence( eSequenceID );
	if( pkSequence )
	{
		pkSequence->ResetSequence();
		return pSeqData;
	}
	else
	{
		Reset();
		return SetTargetAnimation( eSequenceID );
	}
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::Reset()
{
	if (m_spManager)
	{
		m_spManager->DeactivateAll();
	}
	m_eTargetID = INVALID_SEQUENCE_ID;
	m_fTime = INVALID_TIME;
	m_bBuildTimeline = false;
	m_fTimelineRefreshTime = NI_INFINITY;
	m_eTransitionState = NO_TRANSITION;
	m_bStartTransition = false;
	m_fTransStartTime = NI_INFINITY;
	m_fTransEndTime = -NI_INFINITY;
	m_fTransStartFrameTime = INVALID_TIME;
	m_pcTargetKey = NULL;
	m_eCurID = INVALID_SEQUENCE_ID;
	m_spCurSequence = NULL;
	m_eNextID = INVALID_SEQUENCE_ID;
	m_spNextSeqData = NULL;
	m_spNextSequence = NULL;
	m_bChainActive = false;
	m_fNextChainTime = INVALID_TIME;
	m_kChainIDs.RemoveAll();
	m_kChainDurations.RemoveAll();
	m_kExtraSequenceMap.RemoveAll();
	m_kTimeline.RemoveAll();
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::SequenceID cActorManagerForPartition::GetCurAnimation() const
{
	return m_eCurID;
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::TransitionState cActorManagerForPartition::GetTransitionState()
const
{
	return m_eTransitionState;
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::SequenceID cActorManagerForPartition::GetNextAnimation() const
{
	return m_eNextID;
}
//---------------------------------------------------------------------------
inline NiNode* cActorManagerForPartition::GetNifRoot() const
{
	return m_spModelRoot;
}
//---------------------------------------------------------------------------
inline NiAVObject* cActorManagerForPartition::GetActorRoot() const
{
	NIASSERT(m_spManager && NiIsKindOf(NiAVObject, m_spManager->GetTarget()));
	return (NiAVObject*) m_spManager->GetTarget();
}
//---------------------------------------------------------------------------
inline NiKFMTool* cActorManagerForPartition::GetKFMTool() const
{
	return m_spKFMTool;
}
//---------------------------------------------------------------------------
inline NiControllerManager* cActorManagerForPartition::GetControllerManager() const
{
	return m_spManager;
}
//---------------------------------------------------------------------------
inline NiSkinningLODController* 
cActorManagerForPartition::GetSkinningLODController() const
{
	if (m_spManager)
	{
		return m_spManager->GetSkinningLODController();
	}
	return NULL;
}
//---------------------------------------------------------------------------
inline NiSequenceData* cActorManagerForPartition::GetSequenceData(
	SequenceID eSequenceID) const
{
	NiSequenceDataPtr spSeqData;
	if (m_kSeqDataMap.GetAt(eSequenceID, spSeqData))
	{
		return spSeqData;
	}

	return NULL;
}
//---------------------------------------------------------------------------
inline NiAVObject* cActorManagerForPartition::GetAccumRoot() const
{
	NIASSERT(m_spManager);
	return m_spManager->GetAccumRoot();
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::ClearAllRegisteredCallbacks()
{
	m_kCallbacks.RemoveAll();
	m_bBuildTimeline = true;
}
//---------------------------------------------------------------------------
inline float cActorManagerForPartition::GetNextEventTime(EventType eEventType,
														 SequenceID eSequenceID, const NiFixedString& kTextKey)
{
	if (eEventType == TEXT_KEY_EVENT && kTextKey.Exists())
	{
		NiTextKeyMatch kMatchObject(kTextKey);
		return GetNextEventTime(eEventType, eSequenceID, &kMatchObject);
	}
	else
	{
		return GetNextEventTime(eEventType, eSequenceID);
	}
}
//---------------------------------------------------------------------------
inline float cActorManagerForPartition::GetNextEventTime(EventType eEventType,
														 SequenceID eSequenceID, NiTextKeyMatch* pkMatchObject)
{
	switch (eEventType)
	{
	case ANIM_ACTIVATED:
		return GetNextAnimActivatedTime(eSequenceID);
	case ANIM_DEACTIVATED:
		return GetNextAnimDeactivatedTime(eSequenceID);
	case TEXT_KEY_EVENT:
		return GetNextTextKeyEventTime(eSequenceID, pkMatchObject );
	case END_OF_SEQUENCE:
		return GetNextEndOfSequenceTime(eSequenceID);
	case ANIM_COMPLETED:
		// ANIM_COMPLETED events are always triggered when they
		// occur, and are never added to the timeline.
		// Fall through to the default case.
	default:
		return INVALID_TIME;
	}
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::CallbackData::CallbackData(EventType eEventType,
															 SequenceID eSequenceID, NiTextKeyMatch * pkMatchObject ) : 
m_eEventType(eEventType), m_eSequenceID(eSequenceID),
m_spMatchObject(NULL)
{
	SetMatchObject( pkMatchObject );
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::CallbackData::~CallbackData()
{
	m_spMatchObject = 0;
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::CallbackData::SetMatchObject( 
	NiTextKeyMatch* pkMatchObject )
{
	m_spMatchObject = pkMatchObject;
}
//---------------------------------------------------------------------------
inline NiTextKeyMatchPtr cActorManagerForPartition::CallbackData::GetMatchObject() const
{
	return m_spMatchObject;
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::TimelineData::~TimelineData()
{
	m_spMatchObject = 0;
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::TimelineData::SetValues(float fEventTime,
															   EventType eEventType, SequenceID eSequenceID, 
															   NiTextKeyMatch* pkMatchObject, const NiFixedString& kTextKeyMatched)
{
	m_fEventTime = fEventTime;
	m_eEventType = eEventType;
	m_eSequenceID = eSequenceID;
	m_spMatchObject = pkMatchObject;
	m_kTextKeyMatched = kTextKeyMatched;
}
//---------------------------------------------------------------------------
inline float cActorManagerForPartition::GetNextEndOfSequenceTime(SequenceID eSequenceID)
{
	// This function returns the next time that the "end" text key will occur
	// in the specified sequence. This is a convenience function for a
	// commonly desired text key. If the time of the next "end" text key
	// cannot be determined, INVALID_TIME is returned.

	NiTextKeyMatch kKeyMatch(NiAnimationConstants::GetEndTextKey());
	return GetNextTextKeyEventTime(eSequenceID, &kKeyMatch );
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::ChainCompletionInfo::ChainCompletionInfo()
{
}
//---------------------------------------------------------------------------
inline cActorManagerForPartition::ChainCompletionInfo::~ChainCompletionInfo()
{
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::ChainCompletionInfo::SetName(
	const NiFixedString& kName)
{
	m_kName = kName;
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::ChainCompletionInfo::SetNextName(
	const NiFixedString& kNextName)
{
	m_kNextName = kNextName;
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::RefreshControllerManager()
{
	m_spManager = NiControllerManager::FindControllerManager(m_spModelRoot);
	NIASSERT(m_spManager);
}
//---------------------------------------------------------------------------
inline void cActorManagerForPartition::RebuildTimeline()
{
	m_bBuildTimeline = true;
}
//---------------------------------------------------------------------------
inline NiControllerSequence* cActorManagerForPartition::GetExtraSequence(
	SequenceID eSequenceID) const
{
	NiControllerSequencePtr spSequence;
	if (m_kExtraSequenceMap.GetAt(eSequenceID, spSequence))
	{
		return spSequence;
	}

	return NULL;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline unsigned int cActorManagerForPartition::GetPartsCount()
{
	return mArrayParts.GetAllocatedSize();
}
//---------------------------------------------------------------------------
inline NiAVObject* cActorManagerForPartition::GetParts( unsigned int partIdx ) const
{
	assert(mArrayParts.GetAllocatedSize() > partIdx);
	return mArrayParts.GetAt( partIdx );
}