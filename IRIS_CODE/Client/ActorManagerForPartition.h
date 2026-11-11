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

/* ====================================================================
* 파    일 :	ActorManagerForPartition.h
* 목    적 :	게임브리오 ActorManager의 Wrapping이면서, 캐릭터 분할 관리
*				를 추가하기 위해서 상속을 이용하지 않은 별도의 클래스이다.
* 작 성 자 :	이법석
* 작 성 일 :	2006.01.19
* 주의사항 :	엔진에 포함되어진 cActorManagerForPartition가 상속을 지원하지 않은 상황
*				좋은 의견 개시 바랍니다!!
*				게임브리오 기본코드에서 가져온 부분은 규칙을 적용하지 않았다.
*				Root Node의 Update를 최소화 하려고 노력하자!!
* =================================================================== */

#pragma once

class cModelInstance;
class NiPoseBinding;
class NiSkinningLODController;

NiSmartPointer(NiTextKeyMatch);

class NIANIMATION_ENTRY cActorManagerForPartition 
	: public NiRefObject, public NiSequenceData::ActivationCallback
{
public:
	typedef unsigned int SequenceID;

	// Deprecated typedef.
	typedef SequenceID EventCode;

	class NIANIMATION_ENTRY CallbackObject
	{
	public:
		// Virtual destructor defined in case derived classes need virtual
		// destruction.
		virtual ~CallbackObject() {};
		virtual void AnimActivated(cActorManagerForPartition* pkManager,
			SequenceID eSequenceID, float fCurrentTime, float fEventTime) = 0;
		virtual void AnimDeactivated(cActorManagerForPartition* pkManager,
			SequenceID eSequenceID, float fCurrentTime, float fEventTime) = 0;
		virtual void AnimCompleted(cActorManagerForPartition* pkManager,
			SequenceID eSequenceID, float fCurrentTime, float fEventTime) = 0;
		virtual void TextKeyEvent(cActorManagerForPartition* pkManager,
			SequenceID eSequenceID, const NiFixedString& kTextKey,
			const NiTextKeyMatch* pkMatchObject,
			float fCurrentTime, float fEventTime) = 0;
		virtual void EndOfSequence(cActorManagerForPartition* pkManager,
			SequenceID eSequenceID, float fCurrentTime, float fEventTime) = 0;
	};

	enum EventType
	{
		ANIM_ACTIVATED,
		ANIM_DEACTIVATED,
		ANIM_COMPLETED,
		TEXT_KEY_EVENT,
		END_OF_SEQUENCE
	};

	enum TransitionState
	{
		NO_TRANSITION,
		BLENDING,
		MORPHING,
		CROSSFADING
	};

	// Static constants.
	static const SequenceID ANY_SEQUENCE_ID;
	static const SequenceID INVALID_SEQUENCE_ID;
	static const float INVALID_TIME;
	static const float SEQUENCE_DATA_FREQUENCY;

	// Deprecated static constants.
	static const SequenceID ANY_EVENT_CODE;
	static const SequenceID INVALID_EVENT_CODE;

	// Static creation functions.
	//static cActorManagerForPartition* Create(const char* pcKFMFilename,
	//	bool bCumulativeAnimations = false, bool bLoadFilesFromDisk = true,
	//	NiStream* pkStream = NULL, NiPoseBinding* pkPoseBinding = NULL);
	//static cActorManagerForPartition* Create(NiKFMTool* pkKFMTool,
	//	const char* pcKFMFilePath, bool bCumulativeAnimations = false,
	//	bool bLoadFilesFromDisk = true, NiStream* pkStream = NULL,
	//	NiPoseBinding* pkPoseBinding = NULL);

	// Destructor.
	~cActorManagerForPartition();

	// Cloning function. Creates a clone of the cActorManagerForPartition and all of
	// its contents.
//	cActorManagerForPartition* Clone(NiCloningProcess* pkCloningProcess = NULL);

	// Reloads the NIF file using the info in the NiKFMTool class.
	// If bLoadNIFFile is false, the NIF root will be loaded directly from
	// pkStream without loading a NIF file. pkStream cannot be NULL if
	// bLoadNIFFile is false.
//	bool ReloadNIFFile(NiStream* pkStream = NULL, bool bLoadNIFFile = true,
//		NiPoseBinding* pkPoseBinding = NULL);

	// Changes the NIF root and actor root that are being used for the
	// cActorManagerForPartition. This function does the same thing as ReloadNIFFile but
	// does not require the root objects to reside in an NiStream object or
	// to be loaded from disk.
//	bool ChangeNIFRoot(NiAVObject* pkNIFRoot, 
//		NiPoseBinding* pkPoseBinding = NULL);

	// Loads the specified sequence data using the info in the 
	// NiKFMTool class. If bLoadKFFile is false, the sequence data 
	// will be loaded directly from pkStream without loading a KF file. 
	// pkStream cannot be NULL if bLoadKFFile is false.
//	bool LoadSequenceData(SequenceID eSequenceID, bool bLoadKFFile = true,
//		NiStream* pkStream = NULL);

	// Loads all the sequence datas in the NiKFMTool class.
	// If pkStream is NULL this function creates and deletes its own
	// stream, otherwise it uses the given stream.
//	bool LoadAllSequenceData(NiStream* pkStream = NULL);

	// Changes the NiSequenceData object that is used for the specified
	// SequenceID. This function does the same thing as LoadSequenceData
	// but does not require the NiSequenceData object to reside in 
	// an NiStream object or to be loaded from disk.
	bool ChangeSequenceData(SequenceID eSequenceID,
		NiSequenceData* pkSeqData);

	// Unloads the specified sequence data from the actor manager, removing 
	// it from the NiControllerManager in the process. If there is no other
	// smart pointer referencing this sequence data, it will be deleted.
	void UnloadSequenceData(SequenceID eSequenceID);

	// Update function. Must be called with the current time periodically to
	// update the state machine.
	void Update(float fTime);

	// Functions for setting/getting the next target animation.
	SequenceID GetTargetAnimation() const;
	inline NiSequenceData* SetTargetAnimation(SequenceID eSequenceID);
//	inline bool SetTargetAnimation(SequenceID eSequenceID);
	inline void Reset();

	/// LBS
	NiSequenceData* UpdateTargetAnimation(SequenceID eSequenceID);

	// Functions for getting the current state of the actor manager.
	SequenceID GetCurAnimation() const;
	TransitionState GetTransitionState() const;
	SequenceID GetNextAnimation() const;

	// Functions for setting/getting the callback object.
	inline void SetCallbackObject(CallbackObject* pkCallbackObject);
	CallbackObject* GetCallbackObject() const;

	// Gets the root of the NIF file that was loaded.
//	inline NiAVObject* GetNIFRoot() const;

	// Gets the actor root (the target of the NiControllerManager).
//	inline NiAVObject* GetActorRoot() const;

	// Accessor functions.
	inline NiKFMTool* GetKFMTool() const;
	inline NiControllerManager* GetControllerManager() const;
	inline NiSkinningLODController* GetSkinningLODController() const;
	inline NiSequenceData* GetSequenceData(SequenceID eSequenceID) const;

	// Returns the first active sequence assigned to eSequenceID.
	NiControllerSequence* GetActiveSequence(SequenceID eSequenceID, 
		bool bCheckExtraSequences, bool bCheckStateSequences) const;

	// Returns the ID of the specified sequence.
	SequenceID GetSequenceID(NiControllerSequence* pkSequence,
		bool bCheckExtraSequences, bool bCheckStateSequences) const;

	// Gets the accumulation root of the actor, if there is one.
	inline NiAVObject* GetAccumRoot() const;

	// Finds the SequenceID for the first found sequence data with a given
	// name.  Returns INVALID_SEQUENCE_ID if the name is not found.
	// pcName must be non-NULL.
	SequenceID FindSequenceID(const char* pcName) const;

	// Functions for registering and unregistering callback events to occur
	// within some advance notification time.
	bool RegisterCallback(EventType eEventType, SequenceID eSequenceID,
		const NiFixedString& kTextKey);
	void UnregisterCallback(EventType eEventType, SequenceID eSequenceID,
		const NiFixedString& kTextKey);
	bool RegisterCallback(EventType eEventType, SequenceID eSequenceID,
		NiTextKeyMatch* pkMatchObject = NULL );
	void UnregisterCallback(EventType eEventType, SequenceID eSequenceID,
		NiTextKeyMatch* pkMatchObject = NULL );
	inline void ClearAllRegisteredCallbacks();

	// Gets the time of the next event. If the next time cannot be predicted,
	// INVALID_TIME is returned.
	inline float GetNextEventTime(EventType eEventType,
		SequenceID eSequenceID, const NiFixedString& kTextKey);
	inline float GetNextEventTime(EventType eEventType, 
		SequenceID eSequenceID, NiTextKeyMatch* pkMatchObject = NULL);

	// Activate a sequence separately from the internal state machine. This
	// is useful for layering animations on top of a base animation that is
	// controlled by the state machine. The newly activated sequence is 
	// returned. Callbacks are available for these animations.
	NiControllerSequence* ActivateSequence(SequenceID eSequenceID, 
		int iPriority = 0, float fWeight = 1.0f, float fEaseInTime = 0.0f,
		SequenceID eTimeSyncSeqID = NiKFMTool::SYNC_SEQUENCE_ID_NONE, 
		float fFrequency = SEQUENCE_DATA_FREQUENCY, 
		float fStartFrame = 0.0f);
	bool DeactivateSequence(SequenceID eSequenceID,
		float fEaseOutTime = 0.0f);

	// Refreshes the cActorManagerForPartition's internal pointer to its
	// NiControllerManager. This function is useful if you have
	// programmatically changed the NiControllerManager that the actor 
	// manager is using and need the cActorManagerForPartition to reflect that change.
	inline void RefreshControllerManager();

	// Causes the timeline of callback events to be rebuilt during the next
	// update.
	inline void RebuildTimeline();

	// *** begin Emergent internal use only ***
	// Override for NiSequenceData::ActivationCallback
	virtual void ActivationChanged(NiControllerSequence* pkSequence, 
		NiAnimState eState);

	NiKFMTool::KFM_RC ChangeSequenceID(SequenceID eOldID, SequenceID eNewID);

	class ChainCompletionInfo : public NiRefObject
	{
	public:
		ChainCompletionInfo();
		~ChainCompletionInfo();

		inline void SetName(const NiFixedString& kName);
		inline void SetNextName(const NiFixedString& kNextName);

		float m_fSeqStart;
		float m_fTransStart;
		float m_fTransEnd;
		float m_fInSeqBeginFrame;
		NiFixedString m_kName;
		NiFixedString m_kNextName;
		NiKFMTool::Transition* m_pkTransition;
		unsigned int m_uiSrcID;
		unsigned int m_uiDesID;
	};
	typedef NiPointer<ChainCompletionInfo> ChainCompletionInfoPtr;
	typedef NiTObjectSet<ChainCompletionInfoPtr> ChainCompletionInfoPtrSet;

	class CompletionInfo : public NiRefObject
	{
	public:
		float m_fFrameInDestWhenTransitionCompletes;
		float m_fTimeToCompleteTransition;
		float m_fFrameTransitionOccursInSrc;
		float m_fTimeForChainToComplete;
		NiKFMTool::Transition::BlendPair* m_pkBlendPair;
		ChainCompletionInfoPtrSet m_kChainCompletionInfoSet;
	};
	typedef NiPointer<CompletionInfo> CompletionInfoPtr;

	bool FindTimeForAnimationToCompleteTransition(
		unsigned int uiTransSrcID, unsigned int uiTransDesID,
		NiKFMTool::Transition* pkTransition, float fBeginSQTime,
		float fDesiredTransitionSQTime, CompletionInfo* pkInfoOut);

protected:
	ChainCompletionInfo* FillChainComplete( unsigned int uiSrcID,
		unsigned int uiDesID, float fDuration, 
		float& fSrcBeginFrame, float& fTotalTime);

	// *** end Emergent internal use only ***

	//////////////////////////////////////////////////////////////////////////
	// 추가 코드 LBS

	/// 초기화 
	bool Initialize( NiNode* pModelRoot, cModelInstance* pModel );

	void CollectEffect( tArray<NiAVObject*>* pArr, NiAVObject* obj );

public:

	/// 객체 생성시 추가 함수
	void AddPartsObject( unsigned int partIdx, const char* partName );

	/// Gets the root of the NIF file that was loaded.
	unsigned int	GetPartsCount();
	NiAVObject*		GetParts( unsigned int partIdx ) const;

	/// Gets the actor root (the target of the NiControllerManager).
	NiAVObject*			GetActorRoot() const;

	NiNode*				GetNifRoot() const;

	/// Static creation functions.
	/// 애니메이션 정보, NIF파일 정보 생성
	static cActorManagerForPartition*	Create( cModelInstance* pModel );

	/// 파트를 교체한다. (기존 객체도 Detach된다.)
	void								ChangePartsObject( unsigned int partIdx, NiAVObject* pObj );

	/// 배열을 비운다.
	void								RemoveAllPartsArray();

	//////////////////////////////////////////////////////////////////////////

protected:
	class CallbackData : public NiRefObject
	{
	public:
		CallbackData(EventType eEventType, SequenceID eSequenceID,
			NiTextKeyMatch * pkMatchObject = NULL);
		~CallbackData();

		EventType m_eEventType;
		SequenceID m_eSequenceID;

		NiTextKeyMatchPtr GetMatchObject( ) const;
		inline void SetMatchObject( NiTextKeyMatch * pkMatchObject );

	protected:
		NiTextKeyMatchPtr m_spMatchObject;
	};
	typedef NiPointer<CallbackData> CallbackDataPtr;

public:
	class TimelineData : public NiMemObject
	{
	public:
		TimelineData(){};
		~TimelineData();

		inline void SetValues(float fEventTime, EventType eEventType,
			SequenceID eSequenceID, NiTextKeyMatch* pkMatchObject,
			const NiFixedString& kTextKeyMatched);

		float m_fEventTime;
		EventType m_eEventType;
		SequenceID m_eSequenceID;
		NiTextKeyMatchPtr m_spMatchObject;
		NiFixedString m_kTextKeyMatched;
	};

protected:
	cActorManagerForPartition(NiKFMTool* pkKFMTool, bool bCumulativeAnimations);

//	bool Initialize(NiStream& kStream, NiPoseBinding* pkPoseBinding);
//	bool LoadNIFFile(NiStream& kStream, bool bLoadNIFFile, 
//		NiPoseBinding* pkPoseBinding);
	bool AddSequenceData(SequenceID eSequenceID, NiStream& kStream,
		bool bLoadKFFile);

	float GetNextAnimActivatedTime(SequenceID eSequenceID);
	float GetNextAnimDeactivatedTime(SequenceID eSequenceID);
	float GetNextTextKeyEventTime(SequenceID eSequenceID,
		NiTextKeyMatch* pkMatchObject);
	float GetNextTextKeyEventTime(SequenceID eSequenceID,
		const NiFixedString& kTextKey);

	inline float GetNextEndOfSequenceTime(SequenceID eSequenceID);

	void BuildTimeline();
	void AddTimelineDataObject(EventType eEventType, SequenceID eSequenceID,
		NiTextKeyMatch* pkMatchObject);
	void RemoveTimelineDataObjects(EventType eEventType,
		SequenceID eSequenceID, NiTextKeyMatch* pkMatchObject);
	void ProcessCallbacks();
	void RaiseAnimActivatedEvents(SequenceID eEventSeqID);
	void RaiseAnimDeactivatedEvents(SequenceID eEventSeqID);
	void RaiseAnimCompletedEvents(SequenceID eEventSeqID);

	NiKFMTool::Transition::BlendPair* GetNextBlendPair(
		unsigned int uiTransSrcID, unsigned int uiTransDesID,
		NiKFMTool::Transition* pkTransition, float fFrameTime,
		float& fActualUnboundedFrame);

	NiControllerSequence* GetExtraSequence(SequenceID eSequenceID) const;
	void RemoveInactiveExtraSequences();

	//////////////////////////////////////////////////////////////////////////
	// 추가 변수 LBS (분할 정보들)

	/// 분할 객체 참조 정보를 갖는다.
	NiTPrimitiveArray<NiAVObject*> mArrayParts;

	typedef tPointerHashMap<unsigned long, tArray<NiAVObject*>*> cPartsEffectMap;
	cPartsEffectMap mPartsEffect;

	/// Actor Main Node
	NiNodePtr		m_spModelRoot;

	//////////////////////////////////////////////////////////////////////////

	NiKFMTool* m_spKFMTool;
	NiControllerManagerPtr m_spManager;

	CallbackObject* m_pkCallbackObject;

	SequenceID m_eTargetID;
	float m_fTime;
	float m_fTimelineRefreshTime;
	bool m_bBuildTimeline;

	bool m_bStartTransition;
	TransitionState m_eTransitionState;
	float m_fTransStartTime;
	float m_fTransEndTime;
	float m_fTransStartFrameTime;
	const char* m_pcTargetKey;
	SequenceID m_eCurID;
	NiControllerSequencePtr m_spCurSequence;
	SequenceID m_eNextID;
	NiSequenceDataPtr m_spNextSeqData;
	NiControllerSequencePtr m_spNextSequence;
	float m_fNextChainTime;
	NiTPointerList<unsigned int> m_kChainIDs;
	NiTPointerList<float> m_kChainDurations;
	bool m_bChainActive;

	bool m_bCumulativeAnimationsRequested;

	NiTPointerMap<SequenceID, NiSequenceDataPtr> m_kSeqDataMap;

	NiTObjectArray<CallbackDataPtr> m_kCallbacks;
	NiTObjectList<TimelineData> m_kTimeline;

	// Allow just one active extra sequence per ID.
	NiTPointerMap<SequenceID, NiControllerSequencePtr> m_kExtraSequenceMap;
};

NiSmartPointer(cActorManagerForPartition);

#include "ActorManagerForPartition.inl"
