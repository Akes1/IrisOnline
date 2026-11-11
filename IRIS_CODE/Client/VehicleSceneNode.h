#pragma once
#include "dynamicscenenode.h"

class cPlayerSceneNode;
struct sVehicleList;

///
class cVehicleSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cVehicleSceneNodeParam();

public:
	unsigned long		mVehicleIndex;
	cPlayerSceneNode*	mPlayerSceneNode;
};

inline cVehicleSceneNodeParam::cVehicleSceneNodeParam()
{
	mVehicleIndex = 0;
	mPlayerSceneNode = 0;
}

class cVehicleSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;
public:
	virtual ~cVehicleSceneNode();

	inline cPlayerSceneNode* GetPlayerNode() { return mPlayerSceneNode; }

	virtual const NiPoint3& GetObjectHead();
	virtual const NiPoint3& GetObjectFoot();
	virtual const NiPoint3& GetObjectCenter();

	virtual bool ChangeParts( unsigned int partIdx, const char* fileName );

	virtual bool ChangePartsTexture( unsigned int partsIdx, const char* fileName );


	/// 더미에 객체를 링크 언링크 한다. (무기등 추가 모델 정보)
	virtual unsigned int LinkObject( unsigned int dummyIdx, const char* fileName );

	/// 링크 해제
	virtual void UnLinkObject( unsigned int linkIdx );


	virtual cEffectSceneNode* LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow );
	virtual void UnLinkEffect( cEffectSceneNode* pnode );

	/// 실제로 객체를 링크하는 것이 아니고,
	virtual cSoundSceneNode* LinkSound( unsigned char type, unsigned long soundIdx, bool bLoop );
	virtual void UnLinkSound( cSoundSceneNode* pnode );

	virtual cEffectSceneNode* LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse );


	virtual void	UpdateAniScaleFactor( float factor );

	///
	virtual void UpdateNameCardTextValue();
	virtual void UpdateNameCardGaugeValue();
	virtual void UpdateGuildName();
	virtual void UpdateGuildMark();

	virtual void PreviewGuildMark( NiTexture* tex );
	virtual void PreviewTodayWord( LPCTSTR word, unsigned long color );

	virtual void ActiveChatBubble( LPTSTR msg, unsigned long color );
	virtual void ActiveChatBubble( sTextItem* text );

	virtual NiAVObject*		GetPartObject( unsigned int partsIdx );
	virtual NiNode*			GetDummyObject( unsigned int dummyIdx );
	virtual NiAVObject*		GetWeaponDummy( unsigned int objectIdx, bool left );

	virtual NiAVObject*		GetLinkObject( unsigned int linkId );

	/// 액터 관리자를 리턴
	virtual cActorManagerForPartition* GetActorManager();
	inline cActorManagerForPartition* GetVehicleActorManager() { return mpActorManager; }

	/// 현재 애니메이션 
	virtual cActorManagerForPartition::EventCode GetTargetAnimation() const;
	inline cActorManagerForPartition::EventCode GetVehicleTargetAnimation() const { return mTargetAnimation; }

	virtual void SetTargetAnimation( unsigned int code );
	virtual bool UpdateTargetAnimation( unsigned int code );

	void ActiveQuestText( LPCTSTR msg, unsigned long color );

	virtual void SetSelectLightFlag( bool attach );

	void SetAlphaBlended( float targetAlpha );

	virtual void Translate( const NiPoint3& move );
	virtual void SetTranslate( const NiPoint3& trans );

	virtual void SetWorldTranslate( const NiPoint3& trans );

	virtual void SetScale( float scale );

	virtual void SetTargetScale( float scale, bool Init );

	virtual void SetRotate( const NiPoint3& axis, float angle );
	virtual void SetRotate( float xangle, float yangle, float zangle );
	virtual void SetRotate( const NiMatrix3& rot );

	virtual void HideEar( bool hide );

protected:
	cVehicleSceneNode( eType type = eVEHICLE );

	virtual bool Init( const cVehicleSceneNodeParam& param );

	virtual bool OnVisible();

	virtual cObjectNameCard* CreateNameCard( cBaseObject* /*pObj*/ ) { return 0; }

	virtual bool InitLinkInfo() { return true; }

	/// 가시에 들어왔을때 별도 처리가 필요
	virtual void AddToVisibleArray();
	virtual void AddDepthPlaneObject();

	virtual void CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty );

//	void CollectGeomsEx( NiAVObject* obj, NiTexture* tex );

protected:

	sVehicleList*		mScriptData;
	cPlayerSceneNode*	mPlayerSceneNode;

//	unsigned long		mCheckAnimationId;
	NiTexture*			mVehicleTexture;
};
