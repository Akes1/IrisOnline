#include "StdAfx.h"
#include "PlayerSceneNode.h"

#include "ResourceManager.h"

//#include "ObjectNameCard.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "RenderSystem.h"

//#include "ChatBubble.h"

cPlayerSceneNode::cPlayerSceneNode( eSceneNodeType type )
: cDynamicSceneNode( type )
{
	//mpTarotBubble = 0;
}

cPlayerSceneNode::~cPlayerSceneNode()
{
	//SAFE_DELETE(mpTarotBubble);
}

bool cPlayerSceneNode::Init( const cPlayerSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHead = GetPartObject( eLINK_HEAD );
	if( !mHead )
		assert(0);

	/// 말풍선용 객체 생성
	//mpChatBubble = new cChatBubble;
	//mpTarotBubble = new cChatBubble;

	return true;
}

bool cPlayerSceneNode::InitLinkInfo()
{
	// 이부분은 적당한 메모리 할당을 한다. (나중에 좀더 정확하게 계산하여 추가하거나 줄일것..)
	// 대략 계산 : 양손 무기 2개 + 방패 1개
	mLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(15, 5);
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
* 주의사항 :	추후에 파일로 빼는것도 고려한다.
* ------------------------------------------------------------------ */
bool cPlayerSceneNode::InitPartsName()
{
	mPartsNameMap.SetAt( ePART_HAIR,	"hair" );
	mPartsNameMap.SetAt( ePART_FACE,	"face" );
	mPartsNameMap.SetAt( ePART_BODY1,	"body1" );
	mPartsNameMap.SetAt( ePART_BODY2,	"body2" );
	mPartsNameMap.SetAt( ePART_HAND,	"hand" );
	mPartsNameMap.SetAt( ePART_FOOT,	"foot" );

	mPartsNameMap.SetAt( eLINK_HEAD,	"head_dummy" );
	mPartsNameMap.SetAt( eLINK_BODY,	"body_dummy" );
	mPartsNameMap.SetAt( eLINK_RHAND,	"Rhand_dummy" );
	mPartsNameMap.SetAt( eLINK_LHAND,	"Lhand_dummy" );
	mPartsNameMap.SetAt( eLINK_FOOT,	"foot_dummy" );
	mPartsNameMap.SetAt( eLINK_LARM,	"Larm_dummy" );

	mPartsNameMap.SetAt( eLINK_RWEAPON,	"weapon_dummy" );
	mPartsNameMap.SetAt( eLINK_LWEAPON,	"WeaponL_dummy" );

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitializeRandomIdle()
* 목    적 :	동작의 다양성을 제공하기 위해서
* 주의사항 :	
* ------------------------------------------------------------------ */
void cPlayerSceneNode::InitializeRandomIdle()
{
	mRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);

	mRandomIdle->SetAt( 0, 0 );
	//	m_pkRandomIdle->SetAt(1, 2);
	//	m_pkRandomIdle->SetAt(1, 3);
}

void cPlayerSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	cDynamicSceneNode::Process( deltaTime, accumTime );

	//if( mpTarotBubble )
	//{
	//	if( mHead == 0 )
	//	{
	//		assert( 0 );
	//		return;
	//	}

	//	NiPoint3 headPos = mHead->GetWorldTranslate();
	//	headPos.z += mHeadDist;

	//	mpTarotBubble->Update( headPos, time );
	//}
}


const NiPoint3& cPlayerSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetPartObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeParts( unsigned int index, const cString& pathName )
* 목    적 :	파트 교체용 인터페이스 제공
* 주의사항 :	
* ------------------------------------------------------------------ */
//bool cPlayerSceneNode::ChangeParts( unsigned int partIdx, const char* fileName )
//{
//	if( fileName == 0 )
//		return false;
//
//	///
//	RESOURCEMAN->LoadNIFFile( fileName, cResourceManager::eCHARACTEREFFECT );
//
//	///
//	NiNode* pNode = RESOURCEMAN->CloneObjectByName( fileName );
//	assert(pNode);
//	if( !pNode ) 
//	{
//		return false;
//	}
//
//	return cDynamicSceneNode::ChangeParts( partIdx, pNode );
//}
//
//bool cPlayerSceneNode::ChangePartsTexture( unsigned int partsIdx, const char* fileName )
//{
//	if( partsIdx != ePART_HAIR )
//		return true;
//
//	NiTexture* pTexture = RESOURCEMAN->LoadTexture( fileName );
//	if( pTexture == 0 )
//	{
//		assert(0);
//		return false;
//	}
//
//	return cDynamicSceneNode::ChangePartsTexture( ePART_HAIR, pTexture );
//}


/* ------------------------------------------------------------------
* 함수이름 :	LinkToDummy( unsigned int index, const cString& pathName )
* 목    적 :	객체 링크용 인터페이스
* 주의사항 :	
* ------------------------------------------------------------------ */
//unsigned int cPlayerSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
//{
//	if( fileName == 0 )
//		return false;
//
//	///
//	RESOURCEMAN->LoadNIFFile( fileName, cResourceManager::eCHARACTEREFFECT );
//
//	///
//	NiNode* pNode = RESOURCEMAN->GetObjectByName( fileName );
//	if( !pNode ) 
//	{
//		assert(0);
//		return UINT_MAX;
//	}
//
//	return cDynamicSceneNode::LinkObject( dummyIdx, pNode );
//}
//
//NiAVObject*	 cPlayerSceneNode::GetPartWeapon( unsigned int objectIdx, bool left )
//{
//	NiAVObject* pObj = GetLinkObject( objectIdx );
//	if( pObj == 0 )
//		return 0;
//
//	unsigned int partIdx = (left == true)? eLINK_LWEAPON:eLINK_RWEAPON;
//	const char* partsName = NULL;
//	if( mPartsNameMap.GetAt( partIdx, partsName ) == false )
//	{
//		return NULL;
//	}
//	return pObj->GetObjectByName( partsName );
//}
//
//cObjectNameCard* cPlayerSceneNode::CreateNameCard( cBaseObject* pObj )
//{
//	if( !pObj )
//	{
//		assert(0);
//		return 0;
//	}
//	return new cPlayerNameCard( pObj );
//}
//
//void cPlayerSceneNode::AddDepthPlaneObject() const
//{
//	if( mpTarotBubble && mpTarotBubble->IsActive() )
//	{
//		/// 말풍선 등록
//		SCENEMAN->AddPlane( mpTarotBubble );
//		return;
//	}
//
//	cDynamicSceneNode::AddDepthPlaneObject();
//
//	/// 타유저 이름 보기 옵션 체크
//	/// 파티원인지 여부를 체크한다.
//	/// 파티원? type2 : (이름보기0? type1 : 0);
//
//	if( mpNameCard )
//		mpNameCard->SetCardType( eNameCard_Type_Extra );
//}
//
//void cPlayerSceneNode::ActiveTarotBubble( LPTSTR msg, unsigned long color )
//{
//	if( mpTarotBubble )
//		mpTarotBubble->Active( msg, color, true );
//}
//
//void cPlayerSceneNode::DeActiveTarotBubble()
//{
//	if( mpTarotBubble )
//		mpTarotBubble->DeActive();
//}

//////////////////////////////////////////////////////////////////////////
/// Hero SceneNode

//
//cHeroSceneNode::cHeroSceneNode( eType type )
//: cPlayerSceneNode( type )
//{
//}
//
//cHeroSceneNode::~cHeroSceneNode()
//{
//}
//
//void cHeroSceneNode::AddDepthPlaneObject() const
//{
//	if( mpTarotBubble && mpTarotBubble->IsActive() )
//	{
//		/// 말풍선 등록
//		SCENEMAN->AddPlane( mpTarotBubble );
//		return;
//	}
//
//	cDynamicSceneNode::AddDepthPlaneObject();
//
//	/// 내이름 보기 옵션 체크
//	/// 옵션 type2 : type1
//	if( mpNameCard )
//		mpNameCard->SetCardType( eNameCard_Type_Extra );
//}