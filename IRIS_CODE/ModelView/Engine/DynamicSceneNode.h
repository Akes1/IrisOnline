/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 동적 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"
#include "ActorManagerForPartition.h"
#include "TrailGeometry.h"
#include "BaseObject_Common.h"

class cShadowGeometry;
class cEffectSceneNode;
class cSoundSceneNode;
class cBubbleBase;
class cBaseObject;

struct sTextItem;




/// 동적 장면 노드 인자
class cDynamicSceneNodeParam : public cSceneNodeParam
{
public:
	cDynamicSceneNodeParam();

public:
	///
	//cString mPathName;// resource

	/// 대상을 소유한 게임객체의 인스턴스
	cBaseObject*	mpObject;


	bool			mCreateNameCard;

	bool			mUseShadow;
};

inline cDynamicSceneNodeParam::cDynamicSceneNodeParam()
: mpObject( 0 )
//, mShadowType( eSHADOW_REAL )
{
	mCreateNameCard = true;
	mUseShadow = true;
}

struct sTrailInfo
{
	cRibbonGeometry*	mRibbonGeom;
	unsigned long		mLifeTime;

	unsigned int		mStartLink;
	unsigned int		mEndLink;

	bool				mLoop;

	sTrailInfo()
	{
		mRibbonGeom = 0;
		mLoop = false;
	}
	~sTrailInfo()
	{
		SAFE_NIDELETE(mRibbonGeom);
	}
};

/// 동적 장면 노드
class cDynamicSceneNode : public cSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual bool OnVisible();

	virtual bool Pick( const cRay& ray );

	/// 이동
	virtual bool Move( float /*x*/, float /*y*/ ) { return true; }

	/// 분할 객체를 바꾼다. (기존 객체를 새로운 객체로 대체한다.)
	virtual bool			ChangeParts( unsigned int /*partIdx*/, const char* /*fileName*/ ) { return true; }
	virtual bool			ChangeParts( unsigned int partsIdx, NiNode* pNode );

	virtual bool			ChangePartsTexture( unsigned int /*partsIdx*/, const char* /*fileName*/ ) { return true; }
	virtual bool			ChangePartsTexture( unsigned int /*partsIdx*/, NiTexture* /*pTex*/ );

	/// 더미에 객체를 링크 언링크 한다. (무기등 추가 모델 정보)
	virtual unsigned int	LinkObject( unsigned int /*dummyIdx*/, const char* /*fileName*/ ) { return UINT_MAX; }
	virtual unsigned int	LinkObject( unsigned int dummyIdx, NiNode* pNode );

	/// 링크 해제
	virtual void			UnLinkObject( unsigned int linkIdx );


	virtual cEffectSceneNode* 	LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow );
	virtual void				UnLinkEffect( cEffectSceneNode* pnode );

	/// 실제로 객체를 링크하는 것이 아니고,
	virtual cSoundSceneNode*			LinkSound( unsigned long soundIdx, bool bLoop );
	virtual void						UnLinkSound( cSoundSceneNode* pnode );

	virtual cEffectSceneNode*	LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse );

	sTrailInfo*					ActiveTrailEffect( cString str, unsigned long lifeTime, unsigned int link1, unsigned int link2 );
	void						DeActiveTrailEffect( sTrailInfo* pInfo );

	virtual NiAVObject*				GetLinkObject( unsigned int linkId );

	/// 
	float	GetAniScaleFactor() { return mScaleFactor; }
	unsigned long	GetScaleAccumTime() { return mScaleAccumTime; }
	unsigned long	GetLastTime() { return mLastTime; }
	virtual void	UpdateAniScaleFactor( float factor );

	/// 중점을 리턴
	/// 주의: 바운딩 박스의 중심이 아니다.
	virtual const NiPoint3& GetObjectFoot() = 0;
	virtual const NiPoint3& GetObjectCenter() = 0;
	virtual const NiPoint3& GetObjectHead() = 0;

	/// 분할 객체
	virtual NiAVObject*		GetPartObject( unsigned int partsIdx );
	virtual NiNode*			GetDummyObject( unsigned int dummyIdx );
	virtual NiAVObject*		GetWeaponDummy( unsigned int /*objectIdx*/, bool /*left*/ ) { return 0; }

	/// 액터 관리자를 리턴
	inline virtual cActorManagerForPartition* GetActorManager() { return mpActorManager; }

	/// 현재 애니메이션 
	inline virtual cActorManagerForPartition::EventCode GetTargetAnimation() const { return mTargetAnimation; }

	inline unsigned long	GetTargetAnimationScaleEndTime() { return mTargetAnimationScaleEndTime; }
	inline unsigned long	GetTargetAnimationEndTime() { return mTargetAnimationEndTime; }

	virtual void SetTargetAnimation( unsigned int code );
	virtual bool UpdateTargetAnimation( unsigned int code );


	/// 객체의 각종 Morpher를 제어하기 위해서..
	//	void StartMorpher();
	//	void StopMorpher();

	/// 
	void GotoLastFrame();
	void AnimationReset();

	/// 조명
	NiAmbientLight* FindAmbientLight( NiAVObject* obj );

	void ClearEffect();
	void ClearSound();

	void ClearAllEffect();

	virtual void UpdateLinkEffect() {}

	inline virtual void HideEar( bool hide ) { mHideEar = hide; }

	inline bool IsEnableVisibleArray() { return mSetVisibleArray; }

	virtual void SetFieldEffRadius( float /*r */) {}
	virtual void UpdateFieldTargetEffect( NiPoint3 /*center*/ ) {}

	inline virtual cActorManagerForPartition* GetVehicleActorManager() { return 0; }
	inline virtual cActorManagerForPartition::EventCode GetVehicleTargetAnimation() const { return UINT_MAX; }

	/// 화면에 보여질때 카메라와의 거리(AddToVisibleArray가 호출안되는 객체인 경우 값이 쓰레기다.)
	float mVisibleCamLength;

protected:
	cDynamicSceneNode( eType type = eDYNAMIC );
	virtual ~cDynamicSceneNode();

	/// 초기화
	virtual bool Init( const cDynamicSceneNodeParam& param );

	virtual void CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty );

	/// picking 용 박스, 구, 실린더 객체를 모은다.
	//	void CollectPickData( NiAVObject* pobj );

	void LinkDataInit( NiAVObject* pobj );

	///->
	/// 링크 정보를 초기화 한다.
	virtual bool InitLinkInfo() = 0;

	/// Idle애니가 여러개 있을경우 초기화 한다.
	virtual void InitializeRandomIdle() {}

	/// 바이패드를 별도로 움직이는 경우에 대한처리 (시선 처리 등..)
	virtual void	UpdateBipedRootTransforms( unsigned long /*time*/ ) {}

	/// 가시에 들어왔을때 별도 처리가 필요
	virtual void AddToVisibleArray();

	virtual void ProcessTrail( unsigned long /*deltatime*/, unsigned long /*accumtime*/ ) {}

	NiMesh* GetGeom_Parts( NiAVObject* obj );

protected:
	struct sBoneDataObj : public NiMemObject
	{
		/// 뼈대 정보
		NiSkinData::BoneData* pBoneData;

		/// 뼈의 정점 정보
		NiTPrimitiveSet<NiSkinData::BoneVertData*> BoneVertData;
	};

	struct sLinkInfo : public NiMemObject
	{
		/// 더미 인덱스
		unsigned int DummyIndex;

		/// 데이터의 참조
		NiAVObject* pObject;
	};

protected:
	/// 액터 관리자
	cActorManagerForPartition* mpActorManager;

	/// 현재 애니 인덱스
	cActorManagerForPartition::EventCode mTargetAnimation;

	unsigned long mTargetAnimationEndTime;
	unsigned long mTargetAnimationScaleEndTime;


	/// 눈 깜빡임 모핑 제어
	//	NiMorphWeightsController* mpMorpherController;
	//	NiGeomMorpherController* mpMorpherController;
	//	bool mStopMorpherProcess;

	/// Idle애니 정보
	NiTPrimitiveArray<cActorManagerForPartition::EventCode>* mpRandomIdle;

	/// 각 더미에 Link되어진 객체의 정보
	NiTPrimitiveArray<sLinkInfo*>* mpLinkInfo;

	/// 현재 걸려 있는 Effect 정보
	typedef tList<unsigned long> cLinkIndexList;
	cLinkIndexList mEffectList;
	cLinkIndexList mSoundList;

	/// 객체 상단을 위한 정보
	NiNode*		mpHead;
	NiNode*		mpFoot;
	float		mHeadDist;	/// default 30.0f;

	typedef tList<void*> cPickDataList;
	cPickDataList mPickDataList;

	NiTPointerList<sTrailInfo*> mRibbonList;

protected:
	/// 애니 속도 조절자
	unsigned long	mScaleAccumTime;
	unsigned long	mLastTime;
	float	mScaleFactor;

	/// 애니메이션 관리인덱스를 참조하기 위해 모델 정보를 갖는다.
	cModelInstance* mpModel;

	/// 그림자 기하
	cShadowGeometry* mShadowGeom;
	bool mNeedUpdateShadow;


	bool mUpdateGeomInfo;

	bool mHideEar;

	bool mSetVisibleArray;

	/// Only Use Select Stage!!
	cBaseObject*	mDisplayObj;
};
