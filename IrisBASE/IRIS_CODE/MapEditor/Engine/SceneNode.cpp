#include "stdafx.h"
#include "SceneNode.h"

#include "Ray.h"
#include "Camera.h"
#include "SceneManager.h"
#include "LightSceneNode.h"

#include "Application.h"

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
, mPickFrontOnly( true )
, mPickReturnTexture( false )
, mPickReturnNormal( false )
, mPickReturnColor( false )
{
	if( THEONE )
		mWoldAccumTime = THEONE->GetAccumTime();
	else
		mWoldAccumTime = ULONG_MAX;
}

//////////////////////////////////////////////////////////////////////////
cSceneNode::cSceneNode( eSceneNodeType type )
: mType( type )
, mUserData0( 0 )
, mUserData1( 0 )
, mUserData2( 0 )
, mUserData3( 0 )
, mUserData4( 0 )
, mUserData5( 0 )
, mUserData( 0 )
, mPickBound( false )
, mNeedUpdateProperties( false )
, mNeedUpdateTransform( false )
, mNeedUpdateEffects( false )
, mTargetAlpha( 1.0f )
, mAlpha( 1.0f )
, mAlphaSpeed( 0.8f )
, mEnableAlphaProcess( false )
, mIndexByManager( 0xFFFFFFFF )
, mContainer( 0 )
, mIteratorByContainer( 0 )
, mCheckFrustum(true)
, mViewNode( true )
, mSceneNiNode(0)
#ifdef REGEN_TOOL
, mCulled( false )
#endif
{
	

	mPickPos = NiPoint3::ZERO;
	mPickDistance = 0.0f; 
}

cSceneNode::~cSceneNode()
{
	ClearCollectInfo();

	mPick.RemoveTarget();

	/// 컨테이너에서 제거
	if( mContainer )
	{
		mContainer->Remove( this );
	}
}

void cSceneNode::Process( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( mNeedUpdateProperties )
	{
		GetNiObj()->UpdateProperties();
	}

	if( mNeedUpdateTransform )
	{
		mNeedUpdateTransform = false;

		GetNiObj()->Update( accumTime * 0.001f );

		/// 경계구를 설정
		mBoundSphere.Set( GetCenter(), GetRadius() );

		/// 조명을 처리
		if( mType != SCENENODE_LIGHT && mType != SCENENODE_SOUND && mType != SCENENODE_AREA )
		{
			for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
			{
				cLightSceneNode* n = (cLightSceneNode*)mLightArray[i];

				if( mBoundSphere.IntersectSphere( n->GetBoundSphere() ) == false )
				{
					mNeedUpdateEffects = true;
					n->Detach( this );
				}
			}

			mLightArray.Clear();
			mLightArray.Reserve( 8 );

			if( SCENEMAN->Pick( &mLightArray, mBoundSphere, SCENENODE_LIGHT ) )
			{
				mNeedUpdateEffects = true;

				for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
				{
					cLightSceneNode* n = (cLightSceneNode*)mLightArray[i];
					n->Attach( this );
				}
			}
		}

		/// 장면 트리를 갱신
		if( mContainer )
			mContainer->Update( this );
	}

	if( mNeedUpdateEffects )
	{
		mNeedUpdateEffects = false;

		GetNiObj()->UpdateEffects();
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

bool cSceneNode::OnVisible( cSceneCuller& culler )
{
#ifdef REGEN_TOOL
	if( mCulled )
		return false;
#endif

	if( mAlpha < 0.000001f )
		return false;

	SCENEMAN->OnVisibleModifierPocess( this );

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
					NiCamera* camera = (NiCamera*)*culler.mCamera;
					NiTransform worldTM = node->GetWorldTransform();
					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					NiPoint3 camD, camU, camR;
					{
						// billboard coordinates for world axes of camera
						camD = -camera->GetWorldDirection(); 
						camU = camera->GetWorldUpVector();
						camR = camera->GetWorldRightVector();
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
					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						NiAVObject* child = node->GetAt( i );
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
					NiPoint3 dir = ((culler.mCamera->GetWorldTranslate() - worldTM.m_Translate) * worldTM.m_Rotate)/worldTM.m_fScale;
					dir.y = 0.0f;
					dir.Unitize();

					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					faceMat.SetCol( 0, dir.z, 0.0f, -dir.x );
					faceMat.SetCol( 1,  0.0f, 1.0f, 0.0f );
					faceMat.SetCol( 2, dir.x, 0.0f, dir.z );

					worldTM.m_Rotate = worldTM.m_Rotate * faceMat;

					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						NiAVObject* child = node->GetAt( i );
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
					NiCamera* camera = (NiCamera*)*culler.mCamera;
					NiTransform worldTM = node->GetWorldTransform();

					NiMatrix3 faceMat = NiMatrix3::IDENTITY;
					NiPoint3 camD, camU, camR;

					{
						// billboard coordinates for world axes of camera
						camD = -camera->GetWorldDirection(); 
						camU = camera->GetWorldUpVector();
						camR = camera->GetWorldRightVector();
					}
					camD = camD * worldTM.m_Rotate;
					camU = camU * worldTM.m_Rotate;
					camR = camR * worldTM.m_Rotate;

					faceMat.SetCol( 0, camR );
					faceMat.SetCol( 1, camU );
					faceMat.SetCol( 2, camD );

					worldTM.m_Rotate = worldTM.m_Rotate * faceMat;

					///
					for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
					{
						NiAVObject* child = node->GetAt( i );
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

bool cSceneNode::Save( cFileSaver& saver )
{
	/// 타입을 저장
	saver.WriteUnsignedInt( mType );

	/// 경로를 저장
	char cpath[64];
	::memset( cpath, 0, 64 );
	::strncpy_s( cpath, mFilePath.Cstr(), 63 );
	saver.Write( cpath, 64 );

	/// 이름을 저장
	char cname[64];
	::memset( cname, 0, 64 );
	::strncpy_s( cname, mFileName.Cstr(), 63 );
	saver.Write( cname, 64 );

	/// 위치를 저장
	saver.Write( &(GetWorldTranslate()), sizeof(NiPoint3) );

	/// 회전을 저장
	saver.Write( &(GetWorldRotate()), sizeof(NiMatrix3) );

	/// 배율을 저장
	saver.WriteFloat( GetWorldScale() );
	return true;
}

bool cSceneNode::Init( const cSceneNodeParam& param )
{
	/// 이름을 설정
	::GetFilePath( &mFilePath, param.mPathName );
	::GetFileName( &mFileName, param.mPathName );
	GetNiObj()->SetName( mFileName.Cstr() );

	/// 변환 행렬을 설정
	GetNiObj()->SetTranslate( param.mTranslate );
	GetNiObj()->SetRotate( param.mRotate );
	GetNiObj()->SetScale( param.mScale );

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
	mPick.SetPickObjectPolicy(NiNew SimpleSkinPickObjectPolicy());

	/// 기하 오브젝트들을 수집
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	/// 경계 구를 설정
	GetNiObj()->Update( 0.0f );
	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;
	return true;
}


void cSceneNode::Update()
{
	Process( 0, 0 );
}

void cSceneNode::UpdateAlpha()
{
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();

		for( ; i != iend; ++i )
		{
			cMaterialData* data = (cMaterialData*)(*i);
			NiMaterialProperty* prop = data->mProp;
			assert( prop );

			prop->SetAlpha( mAlpha );
		}
	}

	if( mType != SCENENODE_STATIC )
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

					alphaProp->SetTestMode( NiAlphaProperty::TEST_GREATER );
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

void cSceneNode::UpdateTransform()
{
	mNeedUpdateTransform = true;

	/// 경계 구를 설정
	GetNiObj()->Update( 0.0f );
	mBoundSphere.Set( GetCenter(), GetRadius() );
}

bool cSceneNode::Pick( const cRay& ray )
{
	mPickDistance = NI_INFINITY;

	if( mBoundSphere.IntersectRay( &mPickPos, &mPickDistance, ray ) )
	{
		if( mPickBound )
			return true;

		if( mPick.PickObjects( ray.GetOrigin(), ray.GetDirection() ) )
		{
			NiPick::Record* record = mPick.GetResults().GetAt(0);
			if( record )
			{
				mPickPos = record->GetIntersection();
				mPickDistance = record->GetDistance();
			}
			return true;
		}
	}
	return false;
}

void cSceneNode::RemoveLight( cLightSceneNode* node )
{
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		if( node == mLightArray[i] )
		{
			mLightArray.PopAt( i );
			break;
		}
	}
}

void cSceneNode::Translate( const NiPoint3& move )
{
	GetNiObj()->SetTranslate( GetNiNode()->GetTranslate() + move );
	mNeedUpdateTransform = true;
}

void cSceneNode::SetTranslate( const NiPoint3& trans )
{
	GetNiObj()->SetTranslate( trans );
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
	{
		return (NiMesh*)obj;
	}
	else if( NiIsKindOf(NiNode, obj) )
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

void cSceneNode::AddToVisibleArray( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray )
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
				const NiAlphaProperty* pkAlpha = pkState->GetAlpha();
				if( pkAlpha )// && !(pkAlpha->GetNoSorter()) && geom.GetSortObject() )
				{
					if( pkAlpha->GetAlphaTesting() )
						alphaTestArray->Add( *geom );
					else
					{
						if( pkAlpha->GetAlphaBlending() )
							alphaArray->Add( *geom );
						else
							solidArray->Add( *geom );
					}
				}
				else
				{
					solidArray->Add( *geom );
				}
			}
			else
			{
				assert(0);
				solidArray->Add( *geom );
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

			if( i->mSecond == 1 )
				alphaTestArray->Add( *geom );
			else if( i->mSecond == 2 )
				alphaArray->Add( *geom );
			else
				solidArray->Add( *geom );
		}
	}
}


void cSceneNode::SetViewNode( bool view )
{
	mViewNode = view;
}

void cSceneNode::RecursiveNodeInfo( NiAVObject* obj, bool geom, bool property )
{
	bool checkGeom = geom;
	bool checkProperty = property;

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
		if( obj->GetName() == "PickObj" )
			collectGeom = false;

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

						if( alphaProp )
						{
							if( alphaProp->GetAlphaTesting() )
								mRenderObjList.PushBack( cRenderObjPair( mesh, 1 ) );
							else
							{
								if( alphaProp->GetAlphaBlending() )
									mRenderObjList.PushBack( cRenderObjPair( mesh, 2 ) );
								else
									mRenderObjList.PushBack( cRenderObjPair( mesh, 0 ) );
							}
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

			if( NiIsKindOf(NiPSParticleSystem, obj) )
			{
				mPickBound = true;
			}
		}
	}

	/// property
	{
		bool fogdisable = false;
		if( mType & SCENENODE_DYNAMIC )
		{
			if( obj->GetName() == "PickObj" )
				collectProperty = false;

			fogdisable = true;
		}
		else if( mType & SCENENODE_STATIC )
		{
			fogdisable = true;
		}

		if( collectProperty == true )
		{
			NiMaterialProperty* matProp = (NiMaterialProperty*)obj->GetProperty( NiProperty::MATERIAL );
			if( matProp )
			{

				const NiColor& ambient = matProp->GetAmbientColor();
				const NiColor& diffuse = matProp->GetDiffuseColor();
				const NiColor& specular = matProp->GetSpecularColor();
				const NiColor& emissive = matProp->GetEmittance();
				float shininess = matProp->GetShineness();
				float alpha = matProp->GetAlpha();

				mMatDataList.PushBack( new cMaterialData( ambient, diffuse, specular, emissive, shininess, alpha, matProp ) );
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
}

void cSceneNode::ClearCollectInfo()
{
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();
		for( ; i != iend; ++i )
		{
			cMaterialData* matData = (cMaterialData*)(*i);
			delete matData;
		}
	}

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
	mMatDataList.Clear();

	mBillboardList.Clear();
	mRenderObjList.Clear();
}