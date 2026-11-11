#include "StdAfx.h"
#include ".\monsterscenenode.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"

#include "SceneManager.h"
#include "cameraManager.h"
#include "RenderSystem.h"

#include "ObjectManager.h"

cMonsterSceneNode::cMonsterSceneNode( eType type )
: cDynamicSceneNode( type )
{
}

cMonsterSceneNode::~cMonsterSceneNode()
{
	ClearCollectInfo();
}

bool cMonsterSceneNode::Init( const cMonsterSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHeadDist = param.mHeadHeight;
	mMonsterIndex = param.mMonsterIndex;

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cMonsterSceneNode::InitLinkInfo()
{
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(10, 2);
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
void cMonsterSceneNode::InitializeRandomIdle()
{
	mpRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mpRandomIdle->SetAt(0, 0);
}

/// 061016 PKH 
/* ------------------------------------------------------------------
* 함수이름 :	GetObjectCenter()
* 목    적 :	객체의 중심좌표를 제공한다.
* 주의사항 :	플레이어는 Spine더미의 좌표를 중심좌표로 사용한다.
* ------------------------------------------------------------------ */
const NiPoint3& cMonsterSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetDummyObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cMonsterSceneNode::GetObjectFoot()
{
	NiAVObject* spine = GetDummyObject( eLINK_FOOT );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cMonsterSceneNode::GetObjectHead()
{
	NiAVObject* spine = GetDummyObject( eLINK_HEAD );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

/// 061013 PKH 
unsigned int cMonsterSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
{
	NiNode* pNode = RESOURCEMAN->GetObjectByName( fileName );
	if( !pNode ) 
	{
		assert(0);
		return UINT_MAX;
	}
	/*
	NiTimeController* pCtrl = GetTimeController( (NiAVObject*)pNode );
	if( pCtrl )
	{
	pCtrl->SetAnimType( NiTimeController::APP_INIT );
	pCtrl->SetCycleType( NiTimeController::LOOP );
	pCtrl->Start();
	}
	*/
	return cDynamicSceneNode::LinkObject( dummyIdx, pNode );
}

cEffectSceneNode* cMonsterSceneNode::LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow )
{
	if( dummyIdx == eLINK_RHAND || dummyIdx == eLINK_LHAND )
	{
		if( GetDummyObject( dummyIdx ) == 0 )
			return 0;
	}

	sMonsterInfo* pInfo = GAMERESOURCEMAN->GetMonsterInfo( mMonsterIndex );
	if( pInfo && trans )
		trans->m_fScale *= pInfo->mEffectScale;

	return cDynamicSceneNode::LinkEffect( dummyIdx, fileName, trans, bLoop, bFollow );
}

cEffectSceneNode* cMonsterSceneNode::LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse )
{
	if( dummyIdx == eLINK_RHAND || dummyIdx == eLINK_LHAND )
	{
		if( GetDummyObject( dummyIdx ) == 0 )
			return 0;
	}
	return cDynamicSceneNode::LinkDamage( dummyIdx, fileName, trans, IsMiss, zFalse );
}