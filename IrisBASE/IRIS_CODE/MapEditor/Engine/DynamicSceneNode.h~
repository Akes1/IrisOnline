/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 동적 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"
#include "ActorManagerForPartition.h"

class cRibbonGeometry;
class cShadowGeometry;
//class cEffectSceneNode;
//class cSoundSceneNode;
//class cObjectNameCard;
//class cChatBubble;
//class cBaseObject;

/// 동적 장면 노드 인자
class cDynamicSceneNodeParam : public cSceneNodeParam
{
public:
	cDynamicSceneNodeParam();

public:
	bool mUseShadow;
};

inline cDynamicSceneNodeParam::cDynamicSceneNodeParam()
: mUseShadow( true )
{
}

/// 동적 장면 노드
class cDynamicSceneNode : public cSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );
	virtual bool OnVisible( cSceneCuller& culler );

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

	/// 링크
	void					UnLinkObject( unsigned int linkIdx );
	NiAVObject*				GetLinkObject( unsigned int linkId );

	/// 
	void	UpdateAniScaleFactor( float factor );

	/// 분할 객체
	virtual NiAVObject*		GetPartObject( unsigned int index ) const;
	virtual NiAVObject*		GetPartWeapon( unsigned int /*objectIdx*/, bool /*left*/ ) { return 0; }

	/// 액터 관리자를 리턴
	inline cActorManagerForPartition* GetActorManager() { return mActorManager; }

	/// 현재 애니메이션
	cActorManagerForPartition::EventCode GetTargetAnimation() const { return mTargetAnimation; }
	virtual void SetTargetAnimation( unsigned int code );
	bool UpdateTargetAnimation( unsigned int code );

	/// 객체의 각종 Morpher를 제어하기 위해서..
	void StartMorpher();
	void StopMorpher();

	/// 
	void GotoLastFrame();

	/// 조명
	void FindLight( NiAVObject* obj );
	void SetAmbientLightAmbient( const NiColor& color );
	void SetAmbientLightDiffuse( const NiColor& color );
	void SetAmbientLightDimmer( float dimmer );

	NiAmbientLight* FindAmbientLight( NiAVObject* obj );

	/// 반지름
	//virtual float GetRadius() const;

protected:
	cDynamicSceneNode( eSceneNodeType type = SCENENODE_DYNAMIC );
	virtual ~cDynamicSceneNode();

	/// 초기화
	virtual bool Init( const cDynamicSceneNodeParam& param );

	virtual void RecollectGeoms();

	/// picking 용 박스, 구, 실린더 객체를 모은다.
	void CollectPickData( NiAVObject* pobj );

	NiAVObject* CollectTestData( NiNode* pobj );

	/// 링크 정보를 초기화 한다.
	virtual bool InitLinkInfo() = 0;

	/// 분할 정보를 초기화 한다.
	virtual bool InitPartsName() = 0;

	/// Idle애니가 여러개 있을경우 초기화 한다.
	virtual void InitializeRandomIdle() {}

	/// 바이패드를 별도로 움직이는 경우에 대한처리 (시선 처리 등..)
	virtual void	UpdateBipedRootTransforms( unsigned long /*time*/ ) {}

	/// 기하 오브젝트들을 가시 배열에 추가
	void AddToVisibleArray( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray );

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

		/// 실제 Geom 데이터의 참조
		//		NiGeometry* pObject;
		NiAVObject* pObject;
	};

protected:
	/// 액터 관리자
	cActorManagerForPartition* mActorManager;

	/// 현재 애니 인덱스
	cActorManagerForPartition::EventCode mTargetAnimation;

	/// Idle애니 정보
	NiTPrimitiveArray<cActorManagerForPartition::EventCode>* mRandomIdle;

	/// 각 더미에 Link되어진 객체의 정보
	NiTPrimitiveArray<sLinkInfo*>* mLinkInfo;

	/// 현재 걸려 있는 Effect 정보
	//NiTPointerList<cEffectSceneNode*> mEffectList;
	//NiTPointerList<cSoundSceneNode*> mSoundList;

	/// 각 파트별 Geom객체의 이름 (그래픽 제작시 결정된 이름을 등록한다.)
	NiTPointerMap<unsigned int, const char*> mPartsNameMap;

	/// 객체 상단을 위한 정보
	NiAVObject*		mHead;
	float			mHeadDist;

	/// 객체의 정보를 표시하기 위한 객체
	//cObjectNameCard*		mpNameCard;

	/// 말풍선
	//cChatBubble*			mpChatBubble;

	typedef tList<void*> cPickDataList;
	cPickDataList mPickDataList;

private:
	/// 애니 속도 조절자
	unsigned long	mScaleAccumTime;
	unsigned long	mLastTime;
	float	mScaleFactor;

	/// 애니메이션 관리인덱스를 참조하기 위해 모델 정보를 갖는다.
	cModelInstance* mModel;

	/// 게임 객체에 대한 연결정보(씬정보에서 게임객체를 연결하는 다리역할)
	//cBaseObject*	mpGameObject;

	/// 반지름
	float mRadius;

	/// 조명
	NiAmbientLight* mAmbientLight;

#ifdef MAP_EDITOR
	/// 궤적 기하
	cRibbonGeometry* mRibbonGeom;

	/// 그림자 기하
	cShadowGeometry* mShadowGeom;
	bool mNeedUpdateShadow;
#endif /// MAP_EDITOR
};
