#include "stdafx.h"
#include "cullingprocess.h"

#include "SceneNode.h"
#include "objectmanager.h"


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
		if( pkAlpha && pkAlpha->GetAlphaBlending() )// && !(pkAlpha->GetNoSorter()) && geom.GetSortObject() )
		{
			if( pkAlpha->GetAlphaTesting() )
				mpAlphaTestArray->Add( geom );
			else
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
