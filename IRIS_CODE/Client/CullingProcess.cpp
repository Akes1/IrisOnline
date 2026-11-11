#include "stdafx.h"
#include "CullingProcess.h"
#include "SceneNode.h"


cCullingProcess::cCullingProcess( NiVisibleArray* palphaArray, NiVisibleArray* alphaTestArray, NiVisibleArray* psolidArray )
: NiMeshCullingProcess(psolidArray, NULL, true)
{
	assert( palphaArray && "null alpha node array" );

	mpAlphaArray = palphaArray;
	mpAlphaTestArray = alphaTestArray;
}

cCullingProcess::~cCullingProcess()
{
}

void cCullingProcess::Cull( const NiCamera* pcamera, cSceneNode* pnode)
{
	if( pcamera == 0 || pnode == 0 )
	{
		assert( 0 && "null camera or null node" );
		return;
	}

	if( pnode->GetNiObj() == 0 )
		return;

	NiMeshCullingProcess::Cull( pcamera, pnode->GetNiObj(), NULL );
}

void cCullingProcess::AppendVirtual( NiRenderObject& geom )
{
	if( geom.GetRefCount() == 0 )
	{
		assert(0);
	}

	const NiPropertyState* pkState = geom.GetPropertyState();
	if( pkState )
	{
		const NiAlphaProperty* pkAlpha = pkState->GetAlpha();
		if( pkAlpha && (pkAlpha->GetAlphaBlending() || pkAlpha->GetAlphaTesting()) )// && !(pkAlpha->GetNoSorter()) && geom.GetSortObject() )
		{
//			if( pkAlpha->GetAlphaTesting() )
//				mpAlphaTestArray->Add( geom );
//			else
				mpAlphaArray->Add( geom );
		}
		else
			m_pkVisibleSet->Add( geom );

		return;
	}

	m_pkVisibleSet->Add( geom );
}

void cCullingProcess::Process( NiAVObject* pobj )
{
	if( !m_kPlanes.IsAnyPlaneActive() )
	{
		pobj->OnVisible(*this);
	}
	else
	{
		/// Determine if the object is not visible by comparing its world
		/// bound to each culling plane.
		unsigned int saveActive = m_kPlanes.GetActivePlaneState();

		unsigned int i;
		for( i = 0; i<NiFrustumPlanes::MAX_PLANES; i++ )
		{
			if( m_kPlanes.IsPlaneActive( i ) )
			{
				int iSide = pobj->GetWorldBound().WhichSide( m_kPlanes.GetPlane(i) );

				if( iSide == NiPlane::NEGATIVE_SIDE )
				{
					/// The object is not visible since it is on the negative
					/// side of the plane.
					break;
				}

				if( iSide == NiPlane::POSITIVE_SIDE )
				{
					/// The object is fully on the positive side of the plane,
					/// so there is no need to compare child objects to this
					/// plane.
					m_kPlanes.DisablePlane( i );
				}
			}
		}

		if( i == NiFrustumPlanes::MAX_PLANES )
		{
			pobj->OnVisible( *this );
		}

		m_kPlanes.SetActivePlaneState( saveActive );
	}
}
//////////////////////////////////////////////////////////////////////////

cModifierProcess::cModifierProcess()
: NiMeshCullingProcess(NULL, NULL, true)
{
}

cModifierProcess::~cModifierProcess()
{

}

void cModifierProcess::Cull( const NiCamera* pcamera, cSceneNode* pnode )
{
	if( pcamera == 0 || pnode == 0 )
	{
		assert( 0 && "null camera or null node" );
		return;
	}

	if( pnode->GetNiObj() == 0 )
		return;

	NiVisibleArray temp;
	NiMeshCullingProcess::Cull( pcamera, pnode->GetNiObj(), &temp );
}

void cModifierProcess::Process( NiAVObject* pobj )
{
	pobj->OnVisible(*this);
}

void cModifierProcess::Process(const NiCamera* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet)
{
	NIASSERT(pkCamera && pkScene);
	if (!pkCamera || !pkScene)
		return;

	m_pkCamera = pkCamera;
	SetFrustum(m_pkCamera->GetViewFrustum());

	m_pkVisibleSet = pkVisibleSet;

	if( pkScene )
		pkScene->Cull(*this);

	m_pkCamera = 0;
}


/*
cCullingProcess::cCullingProcess( NiVisibleArray* palphaArray, NiVisibleArray* psolidArray )
: NiCullingProcess( psolidArray, true )
{
	mCurrentObjectAlphaBlendOption = false;
	mCullingAll = true;

	mpAlphaArray = palphaArray;
	mAlphaCheckMaxDistance = 0.0f;
	mAlphaCollisionCheck = false;

	/// Alpha Picking 객체에 대한 초기화
	mAlphaCheckPick.SetPickType( NiPick::FIND_ALL );
	mAlphaCheckPick.SetSortType( NiPick::NO_SORT );
	mAlphaCheckPick.SetIntersectType( NiPick::TRIANGLE_INTERSECT );
	mAlphaCheckPick.SetCoordinateType( NiPick::WORLD_COORDINATES );
	mAlphaCheckPick.SetFrontOnly( false );
	mAlphaCheckPick.SetObserveAppCullFlag( false );
	mAlphaCheckPick.SetReturnTexture( false );
	mAlphaCheckPick.SetReturnNormal( false );
	mAlphaCheckPick.SetReturnSmoothNormal( false );
	mAlphaCheckPick.SetReturnColor( false );
}

cCullingProcess::~cCullingProcess()
{
}

void cCullingProcess::Process( const NiCamera* pcamera, cSceneNode* pnode )
{
	if( pcamera == 0 || pnode == 0 )
	{
		assert( 0 && "null camera or null node" );
		return;
	}

	if( mpAlphaArray == 0 )
	{
		assert( 0 && "null alpha node array" );
		return;
	}

	NiAVObject* pobj = (NiAVObject*)pnode->GetNiObj();
	if( !pobj )
	{
		return;
	}

	if( mAlphaCollisionCheck )
	{
		float rDist = 0.0f;
		if( InterruptTheView( pcamera->GetWorldTranslate(), pcamera->GetWorldDirection(), pobj, rDist ) )
		{
			/// 객체가 시야를 가린다면...
			if( mAlphaCheckMaxDistance && mAlphaCheckMaxDistance > rDist )
				pnode->SetAlphaEnabled( true, 0.2f );
			else
				pnode->SetAlphaEnabled( false, 1.0f );
		}
		else
		{
			/// 객체가 시야를 가리지 않는 경우..
			pnode->SetAlphaEnabled( false, 1.0f );
		}
	}

	mCurrentObjectAlphaBlendOption = pnode->IsAlphaEnabled();
	mCullingAll = true;
	NiCullingProcess::Process( pcamera, pobj, NULL );
}

void cCullingProcess::AppendVirtual( NiGeometry& visible )
{
	assert(mpAlphaArray);
	NiPropertyState* pState = visible.GetPropertyState();
	assert(pState);
	NiAlphaProperty* pAlpha = pState->GetAlpha();

	if( pAlpha && pAlpha->GetAlphaBlending() )
	{
		mpAlphaArray->Add( visible );
	}
	else
	{
		if( mCurrentObjectAlphaBlendOption )
		{
			/// 게임상에서 임의로 알파 처리된 경우
			mpAlphaArray->Add(visible);
		}
		else
		{
			m_pkVisibleSet->Add( visible );
		}
	}
}

void cCullingProcess::Process( NiAVObject* pobj )
{
	if( !m_kPlanes.IsAnyPlaneActive() )
	{
		pobj->OnVisible(*this);
		mCullingAll = false;
	}
	else
	{
		/// Determine if the object is not visible by comparing its world
		/// bound to each culling plane.
		unsigned int saveActive = m_kPlanes.GetActivePlaneState();

		int iSide;
		unsigned int i;
		for( i = 0; i<NiFrustumPlanes::MAX_PLANES; i++ )
		{
			if( m_kPlanes.IsPlaneActive( i ) )
			{
				iSide = pobj->GetWorldBound().WhichSide( m_kPlanes.GetPlane(i) );

				if( iSide == NiPlane::NEGATIVE_SIDE )
				{
					/// The object is not visible since it is on the negative
					/// side of the plane.
					break;
				}

				if( iSide == NiPlane::POSITIVE_SIDE )
				{
					/// The object is fully on the positive side of the plane,
					/// so there is no need to compare child objects to this
					/// plane.
					m_kPlanes.DisablePlane( i );
				}
			}
		}

		if( i == NiFrustumPlanes::MAX_PLANES )
		{
			pobj->OnVisible( *this );
			mCullingAll = false;
		}

		m_kPlanes.SetActivePlaneState( saveActive );
	}
}

bool cCullingProcess::InterruptTheView( NiPoint3 camPos, NiPoint3 camDir, NiAVObject* pObject, float& rDist )
{
	if( !pObject ) 
	{
		return false;
	}

	mAlphaCheckPick.SetTarget( pObject );
	if( mAlphaCheckPick.PickObjects(camPos, camDir) )
	{
		NiPick::Record* pkRecord = mAlphaCheckPick.GetResults().GetAt(0);

		if( pkRecord )
		{
			rDist = pkRecord->GetDistance();
			
			mAlphaCheckPick.RemoveTarget();
			return true;
		}
	}
	mAlphaCheckPick.RemoveTarget();
	
	return false;
}
*/
