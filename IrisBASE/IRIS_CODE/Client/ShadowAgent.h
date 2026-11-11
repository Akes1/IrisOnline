/* ====================================================================
 * 파    일 :	ShadowAgent.h
 * 목    적 :	객첵의 그림자를 관리한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.06
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.06     이법석	    생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

#include "ShadowGeometry.h"

NiSmartPointer(NiTexture);
NiSmartPointer(NiPropertyState);
NiSmartPointer(NiRenderTargetGroup);


class cShadowAgent
{
public:
	static cShadowAgent*	mpShadowAgent;

	cShadowAgent();
	~cShadowAgent();

	/// 그리는 처리가 포함되어 있으므로 전체 프로세스중 Render직전에 호출한다.
	void Process( float time );	

	/// 실제 랜더링
	void Render( NiRenderer* pRenderer );

	/// 그림자 객체 생성
	cShadowGeometry* CreateShadow( unsigned int detail = 6, unsigned int maxTriangleCount = 0 );

	/// 현 프레임에 그려질 그림자 추가
	void AddDrawingShadow( cShadowGeometry* pShadow );

	/// Terrain 객체에 대한 정보 참조
	inline NiAVObject*	GetGroundSceneGraph() { return mpGround; }
	inline void			SetGroundSceneGraph( NiAVObject* pGround ) { mpGround = pGround; }

protected:

	///
	bool UpdateShadowPosition( float time );

	///
	void DrawRenderTaget();

	///
	void UpdateShadowGeometry();

protected:
	/// 그려야하는 객체 매 프레임 갱신
	NiTPrimitiveSet<cShadowGeometry*>		mDrawingShadow; 

	/// Root of the scene upon which shadow will be cast (the "recipient")
	NiAVObject*		mpGround;

	/// 랜더링 스테이트
	NiPropertyStatePtr mpPropertyState;
};

#define SHADOWAGENT			cShadowAgent::mpShadowAgent
