#include "stdafx.h"
#include "shadowAgent.h"
#include "CameraManager.h"

cShadowAgent*	cShadowAgent::mpShadowAgent = NULL;

cShadowAgent::cShadowAgent() 
: mDrawingShadow(100)
{
	mpShadowAgent = this;
}

cShadowAgent::~cShadowAgent()
{
	mDrawingShadow.RemoveAll();
}

/* ------------------------------------------------------------------
 * 함수이름 :	Process( float time )
 * 목    적 :	처리부
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cShadowAgent::Process( float /*time*/ )
{
	NiCamera* pCamera = CAMERAMAN->GetCurrentNi();
	if( !pCamera )
	{
		return;
	}

	unsigned int shadowNum = mDrawingShadow.GetSize();
	cShadowGeometry* pShadow = NULL;
	for( unsigned int i=0; i<shadowNum; ++i )
	{
		pShadow = mDrawingShadow.GetAt( i );
		if( pShadow )
			pShadow->Process( 0.0f, pCamera );
		else
		{
			int a=0;
			a=1;
		}
	}
}

/* ------------------------------------------------------------------
 * 함수이름 :	Render( NiRenderer* pRenderer )
 * 목    적 :	출력부
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cShadowAgent::Render( NiRenderer* pRenderer )
{
	NiPropertyState* pState = pRenderer->GetPropertyState();
   	pRenderer->SetPropertyState( mpPropertyState );

	unsigned int shadowNum = mDrawingShadow.GetSize();
	cShadowGeometry* pShadow = NULL;
	for( unsigned int i=0; i<shadowNum; ++i )
	{
		pShadow = mDrawingShadow.GetAt( i );
		if( pShadow ) 
			pShadow->GetShadowGeometry()->RenderImmediate( pRenderer );
	}

	pRenderer->SetPropertyState( pState );

	/// 모두 처리가 되면 갱신하다.
	mDrawingShadow.RemoveAll();
}

/* ------------------------------------------------------------------
 * 함수이름 :	AddDrawingShadow( cShadowGeometry* pShadow )
 * 목    적 :	현재 프레임에 그려져야 할 그림자 객체 등록
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cShadowAgent::AddDrawingShadow( cShadowGeometry* pShadow )
{
	mDrawingShadow.Add( pShadow );
}

/* ------------------------------------------------------------------
 * 함수이름 :	CreateShadow( unsigned int detail, unsigned int maxTriangleCount )
 * 목    적 :	그림자 객체 생성용 인터페이스 제공
 * 주의사항 :	
 * ------------------------------------------------------------------ */
cShadowGeometry* cShadowAgent::CreateShadow( unsigned int detail, unsigned int maxTriangleCount )
{
	cShadowGeometry* pGeom = cShadowGeometry::Create( this, detail, maxTriangleCount );

	return pGeom;
}