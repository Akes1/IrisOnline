#pragma once

#include "DynamicSceneNode.h"

class cNpc;
class cSignboard;
class cTutorialSignboard;

/// NPC 장면 노드 인자
class cNpcSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cNpcSceneNodeParam();

public:
	unsigned long mNpcClassIndex;
};

inline cNpcSceneNodeParam::cNpcSceneNodeParam()
{
	mNpcClassIndex = 0;
}

class cNpcSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cNpcSceneNode();

	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	/// 초기화
	virtual bool Init( const cNpcSceneNodeParam& param );

	virtual const NiPoint3& GetObjectCenter();
	virtual const NiPoint3& GetObjectFoot();
	virtual const NiPoint3& GetObjectHead();

	virtual void SetAlphaBlended( float targetAlpha );
	virtual void AddDepthPlaneObject();

	virtual bool Pick( const cRay& ray );

	void FadeOut();

	bool IsFadeOutEnd();

	//
	void UpdateTutorialBoard();
	void ClearTutorialBoard();

protected:
	cNpcSceneNode( eType type = eNPC );

	/// 링크공간을 정의한다.
	virtual bool	InitLinkInfo();

	/// 기본 애니메이션을 정의한다.
	virtual void	InitializeRandomIdle();

	///
	virtual cObjectNameCard* CreateNameCard( cBaseObject* pObj );


protected:
	cSignboard*				mpSignBoard;

	// 튜토리얼용
	cTutorialSignboard*		mpTutorialBoard;

	bool mIsFadeOutProcess;
};


//////////////////////////////////////////////////////////////////////////
class cRefereeSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cRefereeSceneNode();

	/// 초기화
	virtual bool Init( const cSceneNodeParam& param );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	virtual const NiPoint3& GetObjectFoot() { return NiPoint3::ZERO; }
	virtual const NiPoint3& GetObjectCenter() { return NiPoint3::ZERO; }
	virtual const NiPoint3& GetObjectHead() { return NiPoint3::ZERO; }

	virtual cBaseObject* GetGameObject() { return 0; }

protected:
	cRefereeSceneNode( eType type = eNPC );

	/// 링크공간을 정의한다.
	virtual bool	InitLinkInfo();

	void Interpret( unsigned long accumTime );

	cObjectNameCard* CreateNameCard( cBaseObject* pObj ) { return 0; }

	void AddToVisibleArray();
	void AddDepthPlaneObject() {}
};
