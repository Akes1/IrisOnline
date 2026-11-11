#include "StdAfx.h"
#include ".\npcscenenode.h"

#include "ObjectManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "cameraManager.h"
#include "RenderSystem.h"

cNpcSceneNode::cNpcSceneNode( eType type )
: cDynamicSceneNode( type )
{
}

cNpcSceneNode::~cNpcSceneNode()
{
	ClearCollectInfo();
}

bool cNpcSceneNode::Init( const cNpcSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cNpcSceneNode::InitLinkInfo()
{
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(2, 1);
	assert(mpLinkInfo);
	if( mpLinkInfo == NULL ) 
	{
		return false;
	}

	for( unsigned int i=0; i<mpLinkInfo->GetAllocatedSize(); ++i )
	{
		mpLinkInfo->SetAt( i,0 );
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitializeRandomIdle()
* 목    적 :	동작의 다양성을 제공하기 위해서
* 주의사항 :	
* ------------------------------------------------------------------ */
void cNpcSceneNode::InitializeRandomIdle()
{
	mpRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mpRandomIdle->SetAt(0, 0);
}

const NiPoint3& cNpcSceneNode::GetObjectHead()
{
	NiAVObject* spine = GetDummyObject( eLINK_HEAD );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cNpcSceneNode::GetObjectFoot()
{
	NiAVObject* spine = GetDummyObject( eLINK_FOOT );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cNpcSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetDummyObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}