#pragma once
#include "MonsterSceneNode.h"

class cSignboard;
class cCenterMsgPlane;

class cTransMonSceneNode : public cMonsterSceneNode
{
	friend class cSceneManager;
public:
	virtual ~cTransMonSceneNode();

	virtual bool Init( const cMonsterSceneNodeParam& param, bool hero = false );

	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual bool OnVisible();

	void ActiveQuestText( LPCTSTR msg, unsigned long color );
	void ActiveTarotBubble( LPTSTR title, unsigned long color );
	void DeActiveTarotBubble();
	void ActiveUserSellBubble( LPTSTR title, unsigned long color );
	void DeActiveUserSellBubble();

	void SetFieldEffRadius( float r );
	void UpdateFieldTargetEffect( NiPoint3 center );

protected:
	cTransMonSceneNode( eType type = eMONSTER );

	virtual void AddToVisibleArray();
	virtual void AddDepthPlaneObject();
	virtual bool PickManagedPlane( float mouseX, float mouseY );

	virtual cObjectNameCard* CreateNameCard( cBaseObject* pObj );

protected:
	cSignboard*				mpTarotBoard;
	cSignboard*				mpUserSellBoard;

	cCenterMsgPlane*		mpQuestPlane;

	cShadowGeometry*		mpFieldTargetEffect;
	bool					mFieldEffectOn;
	NiPoint3				mFieldEffectCenter;

	bool					mIsHero;
};