#include "StdAfx.h"
#include "vehiclescenenode.h"

#include "RenderSystem.h"
#include "PlayerSceneNode.h"
#include "EffectSceneNode.h"
#include "ResourceManager.h"
#include "LightAgent.h"
#include "FogAgent.h"
#include "CameraManager.h"

#include "Player.h"
#include "Vehicle_Common.h"
#include "VehicleScript.h"
#include "ObjectNameCard.h"

#include "Application.h"

cVehicleSceneNode::cVehicleSceneNode( eType type )
: cDynamicSceneNode(type)
, mScriptData(0)
, mPlayerSceneNode(0)
, mVehicleTexture(0)
{
//	mCheckAnimationId = 0;
}

cVehicleSceneNode::~cVehicleSceneNode()
{
	ClearCollectInfo();

	mVehicleTexture = 0;
}

bool cVehicleSceneNode::Init( const cVehicleSceneNodeParam& param )
{
	mPlayerSceneNode = param.mPlayerSceneNode;
	if( mPlayerSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	mScriptData = VEHICLESCRIPT->GetVehicleInfo( param.mVehicleIndex );
	if( mScriptData == 0 )
	{
		assert(0);
		return false;
	}

	cBaseObject* gameObj = mPlayerSceneNode->GetGameObject();
	if( gameObj == 0 )
	{
		assert(0);
		return false;
	}
	mGameObject.index = gameObj->GetObjectID();
	mGameObject.type = gameObj->GetObjectType();

	mPlayerSceneNode->AnimationReset();
	mScaleAccumTime = mPlayerSceneNode->GetScaleAccumTime();
	mLastUpdateTime = mPlayerSceneNode->GetLastUpdateTime();
	UpdateAniScaleFactor( mPlayerSceneNode->GetAniScaleFactor() );

	cString fileName;
	fileName.Format( "./data/monster/%s", mScriptData->mFileName );
	mpModel = RESOURCEMAN->LoadKFM( fileName.Cstr() );
	if( mpModel == 0 )
	{
		assert(0 && "Model file not found");
		return false;
	}

	if( InitLinkInfo() == false )
		return false;

	/// ActorManager 생성..
	mpActorManager = cActorManagerForPartition::Create( mpModel );
	if( mpActorManager == 0 )
	{
		assert( 0 && "null actor manager" );
		return false;
	}

	/// 갱신
	mpActorManager->Update( 0.0f );

	/// 엔진 노드를 설정
	//	NiNode* n = GetNiObj();
	mSceneNiNode = mpActorManager->GetNifRoot();
	if( GetNiObj() == 0 )
	{
		assert( 0 && "null node" );
		return false;
	}

	/// 변환 행렬을 설정
	GetNiObj()->SetTranslate( param.mTranslate );
	GetNiObj()->SetRotate( param.mRotate );
	GetNiObj()->SetScale( param.mScale );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	NiAmbientLight* amb = FindAmbientLight( GetNiObj() );
	if( amb )
		GetNiNode()->DetachChild( amb );

	/// 조명을 부착
	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneAmbientLight( 1 ) );
	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneDirLight( 1 ) );

	/// 안개 속성을 부착
	GetNiNode()->AttachProperty( FOGAGENT->GetFogProperty() );

	{
		/// 기하 오브젝트들을 수집
		cString texPath;
		texPath.Format( "./data/monster/%s", mScriptData->mTextureName.Cstr() );
		mVehicleTexture = RESOURCEMAN->LoadTexture( texPath.Cstr() );


		/// 픽킹 방식을 설정
		mPick.SetQueryType( NiPick::QUERY_FIRST );
		mPick.SetSortType( NiPick::SORT_OFF );
		mPick.SetIntersectType( NiPick::INTERSECT_TRIANGLE );
		mPick.SetCoordinateType( param.mPickCoordinate );
		mPick.SetFrontOnly( false );
		mPick.SetReturnTexture( param.mPickReturnTexture );
		mPick.SetReturnNormal( param.mPickReturnNormal );
		mPick.SetReturnSmoothNormal( false );
		mPick.SetReturnColor( param.mPickReturnColor );
		mPick.SetTarget( GetNiObj() );
		mPick.SetPickObjectPolicy(NiNew SimpleBoxPickObjectPolicy( true ));

		////		/// 경계 구를 설정
		//NiMeshUpdateProcess kUpdateProcess;
		////kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
		//GetNiNode()->Update( kUpdateProcess );
	}

	/// 알파 속성이 없으면 생성
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)GetNiObj()->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		alphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		GetNiObj()->AttachProperty( alphaProp );
	}

	///
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	/// 갱신
	bool selectiveUpdate = true;
	bool rigid = false;
	GetNiNode()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	GetNiNode()->UpdateProperties();
	GetNiNode()->UpdateEffects();

	NiMeshUpdateProcess kUpdateProcess;
//	kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	GetNiNode()->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	GetNiNode()->UpdateNodeBound();

	mTargetScale = GetNiNode()->GetWorldScale();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	NiPoint3 camPos = CAMERAMAN->GetCurrentCameraPos();
	if( (camPos - GetWorldTranslate()).Length() > 4000.0f )
	{
		mAlpha = 0.0f;
		SetAlphaBlended( 0.0f );
	}
	else
	{
		mAlpha = 1.0f;
		SetAlphaBlended( 1.0f );
	}

	mpHead = mPlayerSceneNode->GetDummyObject( eLINK_HEAD );
	if( !mpHead )
	{
		assert(0);
		return false;
	}
	mpFoot = GetDummyObject( eLINK_FOOT );
	if( mpFoot == 0 )
	{
		assert(0);
		return false;
	}

	return true;
}

const NiPoint3& cVehicleSceneNode::GetObjectHead()
{
	if( mPlayerSceneNode == 0 )
		return NiPoint3::ZERO;

	NiNode* spine = mPlayerSceneNode->GetDummyObject( eLINK_HEAD );
	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cVehicleSceneNode::GetObjectFoot()
{
	if( mPlayerSceneNode == 0 )
		return NiPoint3::ZERO;

	NiNode* spine = mPlayerSceneNode->GetDummyObject( eLINK_FOOT );
	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cVehicleSceneNode::GetObjectCenter()
{
	NiNode* spine = GetDummyObject( eLINK_FOOT );
	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

void cVehicleSceneNode::UpdateNameCardTextValue()
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->UpdateNameCardTextValue();
}

void cVehicleSceneNode::UpdateNameCardGaugeValue()
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->UpdateNameCardGaugeValue();
}

void cVehicleSceneNode::UpdateGuildName()
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->UpdateGuildName();
}

void cVehicleSceneNode::UpdateGuildMark()
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->UpdateGuildMark();
}

void cVehicleSceneNode::PreviewGuildMark( NiTexture* tex )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->PreviewGuildMark( tex );
}

void cVehicleSceneNode::PreviewTodayWord( LPCTSTR word, unsigned long color )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->PreviewTodayWord( (LPTSTR)word, color );
}

void cVehicleSceneNode::UpdateAniScaleFactor( float factor )
{
	cDynamicSceneNode::UpdateAniScaleFactor( factor );

	if( mPlayerSceneNode )
		mPlayerSceneNode->UpdateAniScaleFactor( factor );
}

void cVehicleSceneNode::ActiveChatBubble( LPTSTR msg, unsigned long color )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->ActiveChatBubble( msg, color );
}

void cVehicleSceneNode::ActiveChatBubble( sTextItem* text )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->ActiveChatBubble( text );
}

NiAVObject* cVehicleSceneNode::GetPartObject( unsigned int partsIdx )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->GetPartObject( partsIdx );
	}
	return 0;
}

NiNode* cVehicleSceneNode::GetDummyObject( unsigned int dummyIdx )
{
	if( dummyIdx == eLINK_FOOT )
	{
		return cDynamicSceneNode::GetDummyObject( dummyIdx );
	}
	else
	{
		if( mPlayerSceneNode )
			return mPlayerSceneNode->GetDummyObject( dummyIdx );
	}
	return 0;
}

NiAVObject* cVehicleSceneNode::GetWeaponDummy( unsigned int objectIdx, bool left )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->GetWeaponDummy( objectIdx, left );
	}
	return 0;
}

NiAVObject* cVehicleSceneNode::GetLinkObject( unsigned int linkId )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->GetLinkObject( linkId );
	}
	return 0;
}

cActorManagerForPartition* cVehicleSceneNode::GetActorManager()
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->GetActorManager();
	}
	return 0;
}

cActorManagerForPartition::EventCode cVehicleSceneNode::GetTargetAnimation() const
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->GetTargetAnimation();
	}
	return 0;
}

void cVehicleSceneNode::SetTargetAnimation( unsigned int code )
{
	unsigned int aniIdx = 0;
	unsigned int vAniIdx = 0;
	if( code >= ANITYPE_IDLE && code < ANITYPE_IDLE2 )
	{
		aniIdx = mScriptData->mPlayerIdle1;
		vAniIdx = V_ANITYPE_IDLE1;
	}
	else if( code>= ANITYPE_IDLE2 && code < ANITYPE_RUN )
	{
		aniIdx = mScriptData->mPlayerIdle2;
		vAniIdx = V_ANITYPE_IDLE2;
	}
	else if( code>= ANITYPE_RUN && code < ANITYPE_ATTACK1 )
	{
		aniIdx = mScriptData->mPlayerRun;
		vAniIdx = V_ANITYPE_RUN;
	}
	else if( code>=ANITYPE_JUMP&& code < ANITYPE_JUMP_END )
	{
		aniIdx = mScriptData->mPlayerJump;
		vAniIdx = V_ANITYPE_JUMP;
	}
	else
	{
		return;
	}

	if( mTargetAnimation != vAniIdx )
	{
		AnimationReset();

		if( mPlayerSceneNode )
			mPlayerSceneNode->AnimationReset();
	}

	cDynamicSceneNode::SetTargetAnimation( vAniIdx );

//	mCheckAnimationId = aniIdx;

	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->SetTargetAnimation( aniIdx );
	}
}

bool cVehicleSceneNode::UpdateTargetAnimation( unsigned int code )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->UpdateTargetAnimation( code );
	}
	return false;
}

void cVehicleSceneNode::ActiveQuestText( LPCTSTR msg, unsigned long color )
{
	if( mPlayerSceneNode && mPlayerSceneNode->GetType() == cSceneNode::eHERO )
	{
		mPlayerSceneNode->ActiveQuestText( msg, color );
	}
}

void cVehicleSceneNode::AddToVisibleArray()
{
	if( IsViewNode() == false )
		return;

	cSceneNode::AddToVisibleArray();

	//{
	//	/// 카메라와의 거리 등록
	//	NiPoint3 vec = GetWorldTranslate() - CAMERAMAN->GetCurrentCameraPos();
	//	mVisibleCamLength = vec.Length();

	//	/// visible dynamic array에 등록
	//	SCENEMAN->AddVisibleDynamicNode( this );
	//}

	/// 
	if( mpFoot == 0 )
	{
		assert(0);
		return;
	}

	///
	float x,y;
	NiPoint3 posF = mpFoot->GetWorldTranslate();
	cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), posF, x, y, false );
	mFootScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
	mFootScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );
/*
	///
	NiPoint3 posH = mpHead->GetWorldTranslate();
	posF.z = posH.z + mHeadDist;

	cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), posF, x, y, false );
	mHeadScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
	mHeadScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );
*/
	if( mPlayerSceneNode )
		mPlayerSceneNode->CalcScreenPos();
}

///
void cVehicleSceneNode::AddDepthPlaneObject()
{
	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->AddDepthPlaneObject();
		cObjectNameCard* card = mPlayerSceneNode->GetNameCard();
		if( card )
			card->Set2DFootPos( mFootScreenPosX, mFootScreenPosY );
	}
}

unsigned int cVehicleSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
{
	if( mPlayerSceneNode )
	{
		unsigned int rValue = mPlayerSceneNode->LinkObject( dummyIdx, fileName );

		mUpdateGeomInfo = true;

		return rValue;
	}
	return UINT_MAX;
}

void cVehicleSceneNode::UnLinkObject( unsigned int linkIdx )
{
	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->UnLinkObject( linkIdx );

		mUpdateGeomInfo = true;
	}
}


bool cVehicleSceneNode::ChangeParts( unsigned int partIdx, const char* fileName )
{
	if( mPlayerSceneNode )
	{
		bool rValue = mPlayerSceneNode->ChangeParts( partIdx, fileName );

		if( rValue )
			mUpdateGeomInfo = true;

		return rValue;
	}
	return false;
}

bool cVehicleSceneNode::ChangePartsTexture( unsigned int partsIdx, const char* fileName )
{
	if( mPlayerSceneNode )
	{
		bool rValue = mPlayerSceneNode->ChangePartsTexture( partsIdx, fileName );

		if( rValue )
			mUpdateGeomInfo = true;

		return rValue;
	}
	return false;
}

cEffectSceneNode* cVehicleSceneNode::LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow )
{
	if( dummyIdx == eLINK_FOOT )
	{
		return cDynamicSceneNode::LinkEffect( dummyIdx, fileName, trans, bLoop, bFollow );
	}
	else
	{
		if( mPlayerSceneNode )
		{
			return mPlayerSceneNode->LinkEffect( dummyIdx, fileName, trans, bLoop, bFollow );
		}
	}
	return 0;
}

void cVehicleSceneNode::UnLinkEffect( cEffectSceneNode* pnode )
{
	if( pnode->GetDummyIdx() == eLINK_FOOT )
	{
		cDynamicSceneNode::UnLinkEffect( pnode );
	}
	else
	{
		if( mPlayerSceneNode )
			mPlayerSceneNode->UnLinkEffect( pnode );
	}
}

cSoundSceneNode* cVehicleSceneNode::LinkSound( unsigned char type, unsigned long soundIdx, bool bLoop )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->LinkSound( type, soundIdx, bLoop );
	}
	return 0;
}

void cVehicleSceneNode::UnLinkSound( cSoundSceneNode* pnode )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->UnLinkSound( pnode );
}

cEffectSceneNode* cVehicleSceneNode::LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse )
{
	if( mPlayerSceneNode )
	{
		return mPlayerSceneNode->LinkDamage( dummyIdx, fileName, trans, IsMiss, zFalse );
	}
	return 0;
}

bool cVehicleSceneNode::OnVisible()
{
	mSetVisibleArray = true;

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->OnVisibleVehicle( mNeedUpdateShadow, GetWorldTranslate() );
	}
	return cDynamicSceneNode::OnVisible();
}

void cVehicleSceneNode::HideEar( bool hide )
{
	if( mPlayerSceneNode )
		mPlayerSceneNode->HideEar( hide );
}

void cVehicleSceneNode::SetAlphaBlended( float targetAlpha )
{
	cDynamicSceneNode::SetAlphaBlended( targetAlpha );

	if( mPlayerSceneNode )
		mPlayerSceneNode->SetAlphaBlended( targetAlpha );
}

void cVehicleSceneNode::SetSelectLightFlag( bool attach )
{
	cDynamicSceneNode::SetSelectLightFlag( attach );

	if( mPlayerSceneNode )
		mPlayerSceneNode->SetSelectLightFlag( attach );
}

void cVehicleSceneNode::CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty )
{
	if( obj == 0 )
	{
		assert( obj );
		return;
	}

	/// geom info
	{
		if( mType & eDYNAMIC || mType == eDROPITEM )
		{
			if( obj->GetName() == "EffectSceneNode" )
				collectGeom = false;
			if( obj->GetName() == "PickObj" )
				collectGeom = false;
		}

		if( collectGeom == true )
		{
			NiMesh* geom = 0;
			if( NiIsKindOf(NiMesh, obj) )
			{
				geom = (NiMesh*)obj;
				assert( geom );

				if( geom->GetVertexCount() >= 3 )
				{
					NiAlphaProperty* alphaProp = (NiAlphaProperty*)obj->GetProperty(NiProperty::ALPHA);

					if( alphaProp )
					{
						if( alphaProp->GetAlphaTesting() )
							mRenderObjList.PushBack( cRenderObjPair( geom, 1 ) );
						else
						{
							if( alphaProp->GetAlphaBlending() )
								mRenderObjList.PushBack( cRenderObjPair( geom, 2 ) );
							else
								mRenderObjList.PushBack( cRenderObjPair( geom, 0 ) );
						}
					}
					else
						mRenderObjList.PushBack( cRenderObjPair(geom, 0) );
				}

				if( mVehicleTexture )
				{
					NiTexturingProperty* texProp = (NiTexturingProperty*)obj->GetProperty(NiProperty::TEXTURING);

					if( texProp && obj->GetName() == "Vehicle" )
						texProp->SetBaseTexture( mVehicleTexture );
				}
			}

			/// billboard Info
			if( NiIsKindOf(NiBillboardNode, obj) )
			{
				mBillboardList.PushBack( (NiBillboardNode*)obj );
			}
		}
	}

	/// property
	{
		bool fogdisable = false;
		if( mType & eDYNAMIC )
		{
			if( obj->GetName() == "EffectSceneNode" )
				collectProperty = false;
			if( obj->GetName() == "PickObj" )
				collectProperty = false;

			fogdisable = true;
		}
		else if( mType & eSTATIC )
		{
			fogdisable = true;
		}

		if( collectProperty == true )
		{
			NiMaterialProperty* matProp = (NiMaterialProperty*)obj->GetProperty( NiProperty::MATERIAL );
			if( matProp )
			{
				mMatPropList.PushBack( matProp );
			}

			NiVertexColorProperty* vertProp = (NiVertexColorProperty*)obj->GetProperty( NiProperty::VERTEX_COLOR );
			if( vertProp )
			{
				if( fogdisable == true )
				{
					if( vertProp->GetSourceMode() == NiVertexColorProperty::SOURCE_EMISSIVE )
					{
						NiFogProperty* fog = NiNew NiFogProperty;
						fog->SetFog( false );
						obj->AttachProperty( fog );
					}
				}

				NiVertexColorProperty::SourceVertexMode sourceMode = vertProp->GetSourceMode();
				NiVertexColorProperty::LightingMode lightMode = vertProp->GetLightingMode();

				mVertexColorDataList.PushBack( new cVertexColorData( sourceMode, lightMode, vertProp ) );
			}

			NiAlphaProperty* alphaProp = (NiAlphaProperty*)obj->GetProperty( NiProperty::ALPHA );
			if( alphaProp )
			{
				bool blendEnabled = alphaProp->GetAlphaBlending();
				bool testEnabled = alphaProp->GetAlphaTesting();
				NiAlphaProperty::TestFunction testFunc = alphaProp->GetTestMode();
				unsigned char testRef = alphaProp->GetTestRef();

				mAlphaDataList.PushBack( new cAlphaData( blendEnabled, testEnabled, testFunc, testRef, alphaProp ) );
			}
		}
	}

	/// 픽킹박스 추출
	if( obj->GetName() == "PickObj" )
	{
		mPickDataList.PushBack( obj );
	}
}

void cVehicleSceneNode::Translate( const NiPoint3& move )
{
	cDynamicSceneNode::Translate( move );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->Translate( move );
	}
}

void cVehicleSceneNode::SetTranslate( const NiPoint3& trans )
{
	cDynamicSceneNode::SetTranslate( trans );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetTranslate( trans );
	}
}

void cVehicleSceneNode::SetWorldTranslate( const NiPoint3& trans )
{
	cDynamicSceneNode::SetWorldTranslate( trans );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetWorldTranslate( trans );
	}
}

void cVehicleSceneNode::SetScale( float scale )
{
	cDynamicSceneNode::SetScale( scale );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetScale( scale );
	}
}

void cVehicleSceneNode::SetRotate( const NiPoint3& axis, float angle )
{
	cDynamicSceneNode::SetRotate( axis, angle );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetRotate( axis, angle );
	}
}

void cVehicleSceneNode::SetRotate( float xangle, float yangle, float zangle )
{
	cDynamicSceneNode::SetRotate( xangle, yangle, zangle );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetRotate( xangle, yangle, zangle );
	}
}

void cVehicleSceneNode::SetRotate( const NiMatrix3& rot )
{
	cDynamicSceneNode::SetRotate( rot );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetRotate( rot );
	}
}

void cVehicleSceneNode::SetTargetScale( float scale, bool init )
{
	cDynamicSceneNode::SetTargetScale( scale, init );

	if( mPlayerSceneNode )
	{
		mPlayerSceneNode->SetTargetScale( scale, init );
	}
}
