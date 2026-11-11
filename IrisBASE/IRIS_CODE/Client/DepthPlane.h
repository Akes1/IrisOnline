/* ====================================================================
* 파    일 :	DepthPlane.h
* 목    적 :	2D, 2.5D 랜더링 기본 객체
* 작 성 자 :	이법석
* 작 성 일 :	2007.3
* 주의사항 :	2.5D 객체를 멤버로 가지고 관리하려는 클래스가 꼭 상속받아야 할 객체
* =================================================================== */

#pragma once

struct sObject;

///
class cPlane
{
public:
	virtual ~cPlane() {}

//	virtual void Update() = 0;
	virtual void Draw() = 0;

	/// 좌표 변환 ( world pos -> Screen pos )
	static void WorldPtToScreenPt( NiCamera* pCam, const NiPoint3& pt, float& posX, float& posY, bool InScreen = true );

	virtual bool PickPoint( float x, float y ) { return false; }
	virtual int  GetType(){ return 0; }

	virtual void PreRender() {}
protected:
	cPlane() {}
};