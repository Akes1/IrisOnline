#include "StdAfx.h"
#include "NpcSceneNode.h"

//#include "ObjectNameCard.h"

#include "SceneManager.h"
#include "CameraManager.h"
#include "RenderSystem.h"

//#include "ChatBubble.h"

cNpcSceneNode::cNpcSceneNode( eSceneNodeType type )
: cDynamicSceneNode( type )
{
}

cNpcSceneNode::~cNpcSceneNode()
{
}

bool cNpcSceneNode::Init( const cNpcSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHead = GetPartObject( eLINK_HEAD );
	//if( !mpHead )
	//	assert(0);

	/// 말풍선용 객체 생성
//	mpChatBubble = new cChatBubble;

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cNpcSceneNode::InitLinkInfo()
{
	mLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(2, 1);
	assert(mLinkInfo);
	if( mLinkInfo == 0 ) 
	{
		return false;
	}

	for( unsigned int i=0; i<mLinkInfo->GetAllocatedSize(); ++i )
	{
		mLinkInfo->SetAt( i,0 );
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitPartsName()
* 목    적 :	그래픽팀과 맞춘 객체의 이름을 셋팅한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cNpcSceneNode::InitPartsName()
{
	mPartsNameMap.SetAt( eLINK_HEAD,	"head_dummy" );
	mPartsNameMap.SetAt( eLINK_BODY,	"body_dummy" );
	mPartsNameMap.SetAt( eLINK_FOOT,	"foot_dummy" );

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitializeRandomIdle()
* 목    적 :	동작의 다양성을 제공하기 위해서
* 주의사항 :	
* ------------------------------------------------------------------ */
void cNpcSceneNode::InitializeRandomIdle()
{
	mRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mRandomIdle->SetAt(0, 0);
}

///
//cObjectNameCard* cNpcSceneNode::CreateNameCard( cBaseObject* pObj )
//{
//	if( !pObj )
//	{
//		assert(0);
//		return 0;
//	}
//	return new cNpcNameCard( pObj );
//}

///
//void cNpcSceneNode::AddDepthPlaneObject() const
//{
//	cDynamicSceneNode::AddDepthPlaneObject();
//
//	/// 해당 NPC와 나 사이의 관계에 따른 퀘스트 정보를 가져온다.
//	/// 퀘스트 마크 표시에 대한 제안..
//}
