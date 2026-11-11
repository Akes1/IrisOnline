#include "StdAfx.h"
#include "MonsterSceneNode.h"
#include "ResourceManager.h"

//#include "ObjectNameCard.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "RenderSystem.h"

//#include "ChatBubble.h"

cMonsterSceneNode::cMonsterSceneNode( eSceneNodeType type )
: cDynamicSceneNode( type )
{
}

cMonsterSceneNode::~cMonsterSceneNode()
{
}

bool cMonsterSceneNode::Init( const cMonsterSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHead = GetPartObject( eLINK_HEAD );
	//if( !mHead )
	//	assert(0);

	//if( param.mMakeChatBubble == true )
	//{
	//	/// 말풍선용 객체 생성
	//	mpChatBubble = new cChatBubble;
	//}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cMonsterSceneNode::InitLinkInfo()
{
	mLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(10, 2);
	assert(mLinkInfo);
	if( mLinkInfo == NULL ) 
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
bool cMonsterSceneNode::InitPartsName()
{
	mPartsNameMap.SetAt( eLINK_HEAD,	"head_dummy" );
	mPartsNameMap.SetAt( eLINK_BODY,	"body_dummy" );
	mPartsNameMap.SetAt( eLINK_RHAND,	"Rhand_dummy" );
	mPartsNameMap.SetAt( eLINK_LHAND,	"Lhand_dummy" );
	mPartsNameMap.SetAt( eLINK_FOOT,	"foot_dummy" );

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitializeRandomIdle()
* 목    적 :	동작의 다양성을 제공하기 위해서
* 주의사항 :	
* ------------------------------------------------------------------ */
void cMonsterSceneNode::InitializeRandomIdle()
{
	mRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mRandomIdle->SetAt(0, 0);
}

/// 061016 PKH 
/* ------------------------------------------------------------------
* 함수이름 :	GetObjectCenter()
* 목    적 :	객체의 중심좌표를 제공한다.
* 주의사항 :	플레이어는 Spine더미의 좌표를 중심좌표로 사용한다.
* ------------------------------------------------------------------ */
const NiPoint3& cMonsterSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetPartObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

///// 061013 PKH 
//unsigned int cMonsterSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
//{
//	NiNode* pNode = RESOURCEMAN->GetNiNode( fileName );
//	if( !pNode ) 
//	{
//		assert(0);
//		return UINT_MAX;
//	}
//
//	return cDynamicSceneNode::LinkObject( dummyIdx, pNode );
//}

///
//cObjectNameCard* cMonsterSceneNode::CreateNameCard( cBaseObject* pObj )
//{
//	if( !pObj )
//	{
//		assert(0);
//		return 0;
//	}
//	return new cMonsterNameCard( pObj );
//}

///
//void cMonsterSceneNode::AddDepthPlaneObject() const
//{
//	cDynamicSceneNode::AddDepthPlaneObject();
//
//	/// 이름 표시 옵션을 체크
//
//	/// v키에 따른 toggle type1 or type2
//	if( mpNameCard )
//		mpNameCard->SetCardType( eNameCard_Type_Default );
//}