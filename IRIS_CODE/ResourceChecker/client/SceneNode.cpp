#include "stdafx.h"
#include "SceneNode.h"

#include "Camera.h"
#include "SceneManager.h"
//#include "LightAgent.h"

#include "application.h"


bool SimpleBoxPickObjectPolicy::FindIntersections( const NiPoint3& kOrigin, const NiPoint3& kDir, NiPick& kPick, NiRenderObject* pkRenderObj)
{
	NiMesh* pkMesh = NiDynamicCast(NiMesh, pkRenderObj);

	if (!pkMesh)
		return false;

	if( mOnlyBound && pkMesh->GetName() != "PickObj" )
		return false;

	NiDataStreamRef* ref = pkMesh->FindStreamRef( NiCommonSemantics::POSITION() );
	if( ref == 0 )
	{
		if (NiCollisionTraversals::FindIntersectionsSkinnedMeshBounds(
			kOrigin, kDir, kPick, pkMesh))
		{
			return true;
		}
	}
	else
	{
		if (NiCollisionTraversals::FindIntersectionsMeshPosition(
			kOrigin, kDir, kPick, pkMesh))
		{
			return true;
		}

		if (NiCollisionTraversals::FindIntersectionsSkinnedMeshBounds(
			kOrigin, kDir, kPick, pkMesh))
		{
			return true;
		}
	}

	return false;
}

/// 컬링용
NiCamera* cSceneNode::mCamera = 0;
NiVisibleArray* cSceneNode::mSolidArray = 0;
NiVisibleArray* cSceneNode::mAlphaArray = 0;
NiVisibleArray* cSceneNode::mAlphaTestArray = 0;

cAlphaData::cAlphaData( bool blendEnabled, bool testEnabled, NiAlphaProperty::TestFunction testFunc, unsigned char testRef, NiAlphaProperty* prop )
: mBlendEnabled( blendEnabled )
, mTestEnabled( testEnabled )
, mTestFunc( testFunc )
, mTestRef( testRef )
, mProp( prop )
{
	assert( prop );
}

cAlphaData::~cAlphaData()
{
	mProp = 0;
}

cVertexColorData::cVertexColorData( NiVertexColorProperty::SourceVertexMode sourceMode, NiVertexColorProperty::LightingMode lightMode, NiVertexColorProperty* prop )
: mSourceMode( sourceMode )
, mLightMode( lightMode )
, mProp( prop )
{
	assert( prop );
}
cVertexColorData::~cVertexColorData()
{
	mProp = 0;
}

cSceneNodeParam::cSceneNodeParam()
: mTranslate( NiPoint3::ZERO )
, mRotate( NiMatrix3::IDENTITY )
, mScale( 1.0f )
, mQueryType( NiPick::QUERY_ALL )
, mPickSort( NiPick::SORT_ON )
, mPickIntersect( NiPick::INTERSECT_TRIANGLE )
, mPickCoordinate( NiPick::COORDINATES_WORLD )
, mPickFrontOnly( false )
, mPickReturnTexture( false )
, mPickReturnNormal( false )
, mPickReturnColor( false )
{
}


///
cSceneNode::cSceneNode( eType type )
: mType( type )
, mNeedUpdateTransform( false )
, mEnableAlphaProcess( false )
, mTargetAlpha( 1.0f )
, mAlpha( 1.0f )
, mAlphaSpeed( 0.8f )
, mIndexByManager( UINT_MAX )
, mContainer( 0 )
, mIteratorValid( false )
, mIteratorByContainer( 0 )
, mAttachSelectLight( false )
, mUpdateSelectLIght( false )
, mViewNode( true )
, mSceneNiNode(0)
{
	mCheckFrustum = true;

	mPickPos = NiPoint3::ZERO;
	mPickDistance = NI_INFINITY;
}

cSceneNode::~cSceneNode()
{
	ClearCollectInfo();

	mPick.RemoveTarget();

	/// 컨테이너에서 제거
	if( mContainer )
	{
		mContainer->Remove( this );
		mContainer = 0;
	}

	if( GetNiObj() )
	{
		assert(GetNiObj()->GetRefCount() == 1);
		mSceneNiNode = 0;
	}
}

void cSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mNeedUpdateTransform )
	{
		mNeedUpdateTransform = false;

		if( GetNiNode() == 0 )
			GetNiObj()->Update( accumTime * 0.001f );
		else
			GetNiNode()->Update( accumTime * 0.001f );

		/// 경계구를 설정
		mBoundSphere.Set( GetCenter(), GetRadius() );

		/// 장면 트리를 갱신
		if( mContainer )
			mContainer->Update( this );
	}
}

void cSceneNode::ProcessAlpha( unsigned long deltaTime )
{
	if( mEnableAlphaProcess == false )
		return;

	float dt = float(deltaTime) * 0.001f;
	float da = dt * mAlphaSpeed;

	if( mAlpha < mTargetAlpha )
	{
		/// 페이드 인
		mAlpha += da;

		if( mAlpha > mTargetAlpha )
			mAlpha = mTargetAlpha;
	}
	else
	{
		/// 페이드 아웃
		mAlpha -= da;

		if( mAlpha < mTargetAlpha )
			mAlpha = mTargetAlpha;
	}

	// 알파를 갱신
	UpdateAlpha();

	if( mAlpha == mTargetAlpha )
		mEnableAlphaProcess = false;
}

bool cSceneNode::OnVisible()
{
	if( mAlpha < 0.000001f )
		return false;

//	SCENEMAN->OnVisibleModifierPocess( this );

	/// 빌보드 노드를 처리
	if( mBillboardList.IsEmpty() == false )
	{
		cBillboardList::cConstIterator i = mBillboardList.Begin();
		cBillboardList::cConstIterator iend = mBillboardList.End();

		for( ; i != iend; ++i )
		{
			NiBillboardNode* node = (NiBillboardNode*)(*i);

			switch( node->GetMode() )
			{
			case NiBillboardNode::ALWAYS_FACE_CAMERA:
				{
					NiTransform worldTM = node->GetWorldTransform();
					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					NiPoint3 camD, camU, camR;

					{
						// billboard coordinates for world axes of camera
						camD = -mCamera->GetWorldDirection(); 
						camU = mCamera->GetWorldUpVector();
						camR = mCamera->GetWorldRightVector();
					}
					camD = camD * worldTM.m_Rotate;
					camU = camU * worldTM.m_Rotate;
					camR = camR * worldTM.m_Rotate;

					// Rotated model up vector is that vector in the plane
					// orthogonal to the camera direction which minimizes the angle
					// between it and the original model up (0,1,0).
					float root = NiSqrt( camR.y*camR.y + camU.y*camU.y );
					if( root > 1e-06f )
					{
						float invRoot = 1.0f / root;
						float cosi = camU.y * invRoot;
						float sine = -camR.y * invRoot;
						faceMat.SetCol( 0,  cosi*camR.x + sine*camU.x,  cosi*camR.y + sine*camU.y,  cosi*camR.z + sine*camU.z );
						faceMat.SetCol( 1, -sine*camR.x + cosi*camU.x, -sine*camR.y + cosi*camU.y, -sine*camR.z + cosi*camU.z );
						faceMat.SetCol( 2, camD);
					}
					else
					{
						faceMat.SetCol( 0, -camR );
						faceMat.SetCol( 1, -camU );
						faceMat.SetCol( 2,  camD );
					}
					worldTM.m_Rotate = worldTM.m_Rotate * faceMat;

					///
					NiAVObject* child = 0;
					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						child = node->GetAt( i );
						if( child )
						{
							NiTransform tm = worldTM * child->GetLocalTransform();

							child->SetWorldScale( tm.m_fScale );
							child->SetWorldRotate( tm.m_Rotate );
							child->SetWorldTranslate( tm.m_Translate );
						}
					}
					break;
				}
			case NiBillboardNode::ROTATE_ABOUT_UP:
				{
					NiTransform worldTM = node->GetWorldTransform();
					NiPoint3 dir = ((mCamera->GetWorldTranslate() - worldTM.m_Translate) * worldTM.m_Rotate)/worldTM.m_fScale;
					dir.y = 0.0f;
					dir.Unitize();

					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					faceMat.SetCol( 0, dir.z, 0.0f, -dir.x );
					faceMat.SetCol( 1,  0.0f, 1.0f, 0.0f );
					faceMat.SetCol( 2, dir.x, 0.0f, dir.z );

					worldTM.m_Rotate = worldTM.m_Rotate * faceMat;

					NiAVObject* child = 0;
					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						child = node->GetAt( i );
						if( child )
						{
							NiTransform tm = worldTM * child->GetLocalTransform();

							child->SetWorldScale( tm.m_fScale );
							child->SetWorldRotate( tm.m_Rotate );
							child->SetWorldTranslate( tm.m_Translate );
						}
					}
					break;
				}
			case NiBillboardNode::RIGID_FACE_CAMERA:
				{
					NiTransform worldTM = node->GetWorldTransform();

					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					NiPoint3 camD, camU, camR;

					{
						// billboard coordinates for world axes of camera
						camD = -mCamera->GetWorldDirection(); 
						camU = mCamera->GetWorldUpVector();
						camR = mCamera->GetWorldRightVector();
					}
					camD = camD * worldTM.m_Rotate;
					camU = camU * worldTM.m_Rotate;
					camR = camR * worldTM.m_Rotate;

					faceMat.SetCol( 0, camR );
					faceMat.SetCol( 1, camU );
					faceMat.SetCol( 2, camD );

					worldTM.m_Rotate = worldTM.m_Rotate * faceMat;

					///
					NiAVObject* child = 0;
					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						child = node->GetAt( i );
						if( child )
						{
							NiTransform tm = worldTM * child->GetLocalTransform();

							child->SetWorldScale( tm.m_fScale );
							child->SetWorldRotate( tm.m_Rotate );
							child->SetWorldTranslate( tm.m_Translate );
						}
					}
				}
				break;
			default:
				assert( 0 && "invalid billboard mode" );
				break;
			}
		}
	}
	return true;
}

bool cSceneNode::Init( const cSceneNodeParam& param )
{
	/// 변환 행렬을 설정
	GetNiObj()->SetTranslate( param.mTranslate );
	GetNiObj()->SetRotate( param.mRotate );
	GetNiObj()->SetScale( param.mScale );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	/// 픽킹 방식을 설정
	mPick.SetQueryType( param.mQueryType );
	mPick.SetSortType( param.mPickSort );
	mPick.SetIntersectType( param.mPickIntersect );
	mPick.SetCoordinateType( param.mPickCoordinate );
	mPick.SetFrontOnly( param.mPickFrontOnly );
	mPick.SetReturnTexture( param.mPickReturnTexture );
	mPick.SetReturnNormal( param.mPickReturnNormal );
	mPick.SetReturnSmoothNormal( false );
	mPick.SetReturnColor( param.mPickReturnColor );
	mPick.SetTarget( GetNiObj() );
	mPick.SetPickObjectPolicy(NiNew SimpleBoxPickObjectPolicy(false));

	/// 기하 오브젝트들을 수집
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	bool selectiveUpdate = true;
	bool rigid = false;

	NiMeshUpdateProcess kUpdateProcess;
	//kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	if( GetNiNode() == 0 )
	{
		GetNiObj()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

		GetNiObj()->UpdateProperties();
		GetNiObj()->UpdateEffects();
		GetNiObj()->Update( kUpdateProcess );
		NiMesh::CompleteSceneModifiers( GetNiObj() );
		GetNiObj()->UpdateNodeBound();

	}
	else
	{
		GetNiNode()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

		GetNiNode()->UpdateProperties();
		GetNiNode()->UpdateEffects();
		GetNiNode()->Update( kUpdateProcess );
		NiMesh::CompleteSceneModifiers( GetNiObj() );
		GetNiNode()->UpdateNodeBound();
	}

	/// 경계 구를 설정
	mBoundSphere.Set( GetCenter(), GetRadius() );

	return true;
}

void cSceneNode::UpdateAlpha()
{
	{
		cMatPropList::cIterator i = mMatPropList.Begin();
		cMatPropList::cIterator iend = mMatPropList.End();

		for( ; i != iend; ++i )
		{
			((NiMaterialProperty*)(*i))->SetAlpha( mAlpha );
		}
	}
/*
	if( mType != eSTATIC )
	{
		cVertexColorList::cIterator i = mVertexColorDataList.Begin();
		cVertexColorList::cIterator iend = mVertexColorDataList.End();
		for( ; i != iend; ++i )
		{
			cVertexColorData* vertData = (cVertexColorData*)(*i);
			NiVertexColorProperty* vertProp = vertData->mProp;
			assert( vertProp );

			if( mAlpha < 1.0f - FLT_EPSILON )
			{
				vertProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
				vertProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E_A_D );
			}
			else
			{
				vertProp->SetSourceMode( vertData->mSourceMode );
				vertProp->SetLightingMode( vertData->mLightMode );
			}
		}
	}
*/
	{
		cAlphaDataList::cIterator i = mAlphaDataList.Begin();
		cAlphaDataList::cIterator iend = mAlphaDataList.End();

		for( ; i != iend; ++i )
		{
			cAlphaData* alphaData = (cAlphaData*)(*i);
			NiAlphaProperty* alphaProp = alphaData->mProp;
			assert( alphaProp );

			if( mAlpha < 1.0f - FLT_EPSILON )
			{
				alphaProp->SetAlphaBlending( true );
				if( alphaData->mTestEnabled )
				{
					unsigned char ref = (unsigned char)(alphaData->mTestRef * mAlpha);
					if( ref > alphaData->mTestRef ) 
						ref = alphaData->mTestRef;

					alphaProp->SetTestMode( NiAlphaProperty::TEST_GREATEREQUAL );
					alphaProp->SetTestRef( ref );
				}
			}
			else
			{
				alphaProp->SetAlphaBlending( alphaData->mBlendEnabled );
				if( alphaData->mTestEnabled )
				{
					alphaProp->SetTestMode( alphaData->mTestFunc );
					alphaProp->SetTestRef( alphaData->mTestRef );
				}
			}
		}
	}
}

bool cSceneNode::Pick( const cRay& ray )
{
	mPickDistance = NI_INFINITY;
	if( mBoundSphere.IntersectRay( ray ) && mPick.PickObjects( ray.GetOrigin(), ray.GetDirection() ) )
	{
		NiPick::Record* record = mPick.GetResults().GetAt(0);
		if( record )
		{
			mPickPos = record->GetIntersection();
			mPickDistance = record->GetDistance();

			mPick.ClearResultsArray();
			return true;
		}
	}
	mPick.ClearResultsArray();
	return false;
}

void cSceneNode::Translate( const NiPoint3& move )
{
	GetNiObj()->SetTranslate( GetNiObj()->GetTranslate() + move );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetTranslate( const NiPoint3& trans )
{
	GetNiObj()->SetTranslate( trans );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetWorldTranslate( const NiPoint3& trans )
{
	GetNiObj()->SetWorldTranslate( trans );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetRotate( const NiPoint3& axis, float angle )
{
	GetNiObj()->SetRotate( angle, axis.x, axis.y, axis.z );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetRotate( float xangle, float yangle, float zangle )
{
	NiMatrix3 r;
	r.FromEulerAnglesXYZ( xangle, yangle, zangle );
	GetNiObj()->SetRotate( r );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetRotate( const NiMatrix3& rot )
{
	GetNiObj()->SetRotate( rot );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetScale( float scale )
{
	GetNiObj()->SetScale( scale );
	mNeedUpdateTransform = true;
}

const NiPoint3& cSceneNode::GetCenter()
{
	return GetNiObj()->GetWorldBound().GetCenter();
}

float cSceneNode::GetRadius()
{
	return GetNiObj()->GetWorldBound().GetRadius();
}

NiMesh* cSceneNode::GetGeom( NiAVObject* obj )
{
	if( obj == 0 )
	{
		assert( 0 && "null node" );
		return 0;
	}

	if( NiIsKindOf(NiMesh, obj) )
		return (NiMesh*)obj;

	if( NiIsKindOf(NiNode, obj) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt( i );
			if( child )
			{
				NiMesh* next = GetGeom( child );
				if( next )
					return next;
			}
		}
	}
	return 0;
}

void cSceneNode::SetAppCulled( NiAVObject* obj, bool cull )
{
	assert( obj );

	NiMesh* geom = 0;
	if( NiIsKindOf(NiMesh, obj) )
	{
		geom = (NiMesh*)obj;
		if( geom )
			geom->SetAppCulled( cull );
	}
	else if( NiIsKindOf(NiNode, obj) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt(i);
			if( child )
				SetAppCulled( child, cull );
		}
	}
}

void cSceneNode::AddToVisibleArray()
{
	if( IsViewNode() == false )
		return;

	/// 기하 오브젝트들을 가시 배열에 추가
	cRenderObjList::cConstIterator i = mRenderObjList.Begin();
	cRenderObjList::cConstIterator end = mRenderObjList.End();

	if( IsAlphaBlended() )
	{
		NiRenderObject* geom = 0;
		for( ; i != end; ++i )
		{
			geom = i->mFirst;
			if( geom->GetAppCulled() == true )
				continue;

			assert( geom->GetRefCount() );

			const NiPropertyState* pkState = geom->GetPropertyState();
			if( pkState )
			{
				NiAlphaProperty* pkAlpha = pkState->GetAlpha();
				if( pkAlpha )
				{
					if( pkAlpha->GetAlphaBlending() || pkAlpha->GetAlphaTesting() )
					{
//						if( pkAlpha->GetAlphaTesting() )
//							mAlphaArray->Add( *geom );
//						else
						mAlphaArray->Add( *geom );
					}
					else
					{
						mSolidArray->Add( *geom );
					}
				}
				else
				{
					mSolidArray->Add( *geom );
				}
			}
			else
			{
				assert(0);
				mSolidArray->Add( *geom );
			}
		}
	}
	else
	{
		NiRenderObject* geom = 0;
		for( ; i != end; ++i )
		{
			geom = i->mFirst;
			if( geom->GetAppCulled() == true )
				continue;
//*
			const NiPropertyState* pkState = geom->GetPropertyState();
			if( pkState )
			{
				NiAlphaProperty* pkAlpha = pkState->GetAlpha();
				if( pkAlpha )
				{
					if( pkAlpha->GetAlphaBlending() || pkAlpha->GetAlphaTesting() )
					{
//						if( pkAlpha->GetAlphaTesting() )
//							mAlphaArray->Add( *geom );
//						else
						mAlphaArray->Add( *geom );
					}
					else
					{
						mSolidArray->Add( *geom );
					}
				}
				else
				{
					mSolidArray->Add( *geom );
				}
			}
			else
			{
				assert(0);
				mSolidArray->Add( *geom );
			}
//*/
/*
			if( i->mSecond == 1 )
			{
//				mAlphaTestArray->Add( *geom );
				mAlphaArray->Add( *geom );
			}
			else if( i->mSecond == 2 )
			{
				mAlphaArray->Add( *geom );
			}
			else
				mSolidArray->Add( *geom );
//*/
		}
	}
}

void cSceneNode::SetViewNode( bool view )
{
	mViewNode = view;
}

void cSceneNode::LogNiNodeInfo( NiAVObject* obj )
{
	if( obj == 0 )
	{
		GameErrorLog( "LogNiNodeInfo : NULL Object" );
		return;
	}

	GameErrorLog( "LogNiNodeInfo (%s) : %d", obj->GetName(), obj->GetRefCount() );
	if( NiIsKindOf(NiNode, obj) )
	{
		NiNode* node = (NiNode*)obj;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			NiAVObject* child = node->GetAt(i);
			if( child )
				LogNiNodeInfo( child );
		}
	}
}

void cSceneNode::RecursiveNodeInfo( NiAVObject* obj, bool geom, bool prop )
{
	bool checkGeom = geom;
	bool checkProperty = prop;

	if( obj == 0 )
	{
		assert(obj);
		return;
	}

	/// 수집 함수
	CollectNodeInfo( obj, checkGeom, checkProperty );

	/// collect child info
	if( NiIsKindOf(NiNode, obj) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt(i);
			if( child )
				RecursiveNodeInfo( child, checkGeom, checkProperty );
		}
	}
}

void cSceneNode::CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty )
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
			NiRenderObject* geom = 0;

			if( NiDynamicCast(NiMesh, obj) )
			{
				NiMesh* mesh = (NiMesh*)obj;
				assert(mesh);

				if( mesh->GetPrimitiveType() == NiPrimitiveType::PRIMITIVE_TRIANGLES ||
					mesh->GetPrimitiveType() == NiPrimitiveType::PRIMITIVE_TRISTRIPS )
				{
					geom = (NiRenderObject*)obj;
					assert( geom );

					if( mesh->GetVertexCount() >= 3 )
					{
						NiAlphaProperty* alphaProp = (NiAlphaProperty*)obj->GetProperty(NiProperty::ALPHA);

						if( alphaProp && alphaProp->GetAlphaBlending() )
						{
//							if( alphaProp->GetAlphaTesting() )
//							{
//								mRenderObjList.PushBack( cRenderObjPair( mesh, 1 ) );
//							}
//							else
//							{
								mRenderObjList.PushBack( cRenderObjPair( mesh, 2 ) );
//							}
						}
						else
							mRenderObjList.PushBack( cRenderObjPair(mesh, 0) );
					}
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
						NiFogProperty* fog = (NiFogProperty*)obj->GetProperty( NiProperty::FOG );
						if( fog == 0 )
						{
							fog = NiNew NiFogProperty;
							obj->AttachProperty( fog );
						}
						fog->SetFog( false );
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
}

void cSceneNode::ClearCollectInfo()
{
	{
		cAlphaDataList::cIterator i = mAlphaDataList.Begin();
		cAlphaDataList::cIterator iend = mAlphaDataList.End();
		for( ; i != iend; ++i )
		{
			cAlphaData* alphaData = (cAlphaData*)(*i);
			delete alphaData;
		}
	}

	{
		cVertexColorList::cIterator i = mVertexColorDataList.Begin();
		cVertexColorList::cIterator iend = mVertexColorDataList.End();
		for( ; i != iend; ++i )
		{
			cVertexColorData* vertData = (cVertexColorData*)(*i);
			delete vertData;
		}
	}

	mVertexColorDataList.Clear();
	mAlphaDataList.Clear();
	mMatPropList.Clear();

	mBillboardList.Clear();
	mRenderObjList.Clear();
}