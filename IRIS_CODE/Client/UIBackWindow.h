/* ==========================================================================
* 파    일 : UIBackWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-13
* 주의사항 : 배경윈도우는 이것으로 생성한다
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cPlaneObject;
/// UI back 윈도우
class cUIBackWindow : public cUIWindow
{
public:
	cUIBackWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cUIBackWindow();

	virtual void Open();
	virtual void Close();

	//void SetAlphaEnabled( bool enabled, NiAlphaProperty::AlphaFunction srcBlend, NiAlphaProperty::AlphaFunction destBlend );

	void SetBackTexture( NiTexture* tex );
	virtual void SetAlpha( float alpha );

protected:
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateSkin();

protected:
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool /*ctrlPress*/, bool /*AltPress*/, bool /*shiftPress*/ );

protected:
	/// 
	cPlaneObject*	mpPlane;
};

// 로드 백 윈도우
class cLabel;
class cLoadBackWindow : public cUIBackWindow
{
public:
	cLoadBackWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cLoadBackWindow();

	virtual void Open();
	virtual void Close();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateSkin();

private:
//	cLabel*		mpGrade;
	cLabel*		mpCopyRight;
	cLabel*		mpAddress;
};

// npc 백 윈도우
class cNpcBackWindow : public cUIBackWindow
{
public:
	cNpcBackWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cNpcBackWindow();

	virtual void Open();
	virtual void Close();
};
