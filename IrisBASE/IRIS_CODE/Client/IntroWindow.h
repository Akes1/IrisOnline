/* ==========================================================================
* 颇    老 : IntroWindow.h
* 格    利 : 
* 累 己 磊 :
* 累 己 老 :
* 林狼荤亲 :
*===========================================================================*/
#pragma once
#include "UIBackWindow.h"

class cIntroWindow : public cUIBackWindow
{
public:
	cIntroWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cIntroWindow();

	void SetAlphaGrade( float alpha );

	void SetGradeTexture( NiTexture* tex );

protected:
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	
private:
	cPlaneObject* mPlaneGrade;
};