/* ====================================================================
 * 파    일 :	Npc.h
 * 목    적 :	Npc 객체
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12.20
 * 주의사항 :	
 * =================================================================== */
#pragma once
#include "BaseObject.h"
#include "Npc_Common.h"

class cUIWindow;

enum eNpcAniType
{
	NPC_ANI_IDLE1 = 1,
	NPC_ANI_IDLE2,
	NPC_ANI_EVENT,
	NPC_ANI_ENDSPEECH,
	NPC_ANI_ETC,
};

class cNpc : public cBaseObject
{
public:
	cNpc( unsigned char type = eOBJECTTYPE_NPC );
	virtual ~cNpc();

	inline virtual unsigned long	GetObjectID() { return mObjectId; }

	virtual bool Create( unsigned long objId, unsigned long npcClassIdx, NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );

	virtual void Update( unsigned long deltaTime, unsigned long accumTime );

	virtual LPTSTR	GetName();

	LPTSTR	GetJobName();
	
	unsigned long GetJobIconIndex();

	void SetQuestEffect( eNpcQuestEffect type );
	eNpcQuestEffect GetQuestEffect() { return mQuestEffectType; }

	void StartSpeech();
	void EndSpeech();

	unsigned long GetClassId() { return mpNpcClassInfo->mNpcClassIdx; }

	void FadeOut();

	void UpdateTutorialBoard();
	void ClearTutorialBoard();

	void CreateButton();

	void ShowTalkWindow( unsigned long x, unsigned long y );
	void HideTalkWindow();

protected:
	/// 애니메이션 관계 설정
	virtual void Interpret( unsigned long time );

protected:
	/// npc 인덱스
	unsigned long		mObjectId;

	/// npc 종류 구분자..
	sNPCList*			mpNpcClassInfo;

	/// 이동시 방향 및 
	//NiPoint3			mDesiredDir;

	eNpcQuestEffect		mQuestEffectType;

	unsigned long		mEffectIdx;
	cEffectSceneNode*	mQuestEffect;

	bool				mIsSpeeching;

	// 대화버튼윈도우
	cUIWindow*			mpButtonWindow;
};

inline LPTSTR cNpc::GetName()
{
	assert(mpNpcClassInfo);
	return mpNpcClassInfo->mName;
}

inline LPTSTR cNpc::GetJobName()
{
	assert( mpNpcClassInfo );
	return mpNpcClassInfo->mJobName;
}

inline unsigned long cNpc::GetJobIconIndex()
{
	assert( mpNpcClassInfo );
	return mpNpcClassInfo->mJobIconIndexTarget;
}
