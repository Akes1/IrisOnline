/* ==========================================================================
* 파    일 : LoginWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cEditBox;
class cButton;
class cUIImage;

class cLoginWindow : public cUIWindow
{
public:
	cLoginWindow();
	virtual ~cLoginWindow();

	void Open();

	void ClearID( bool focus = false );
	void ClearPass( bool focus = false );

	bool IsExistID();
	bool IsExistPass();

	cEditBox* GetIDEdit() { return mpID; }
	cEditBox* GetPassEdit() { return mpPass; }

	void SetSendLoginFlag( bool set ) { mSendLogin = set; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* , unsigned int );
	void OnEditBoxEntered( cUINode* pcaller, unsigned int id );
	void OnShow();
	void UpdateRect();
	void UpdateSkin();

private:
	void SendLoginMsg();

protected:
	cEditBox*			mpID;
	cEditBox*			mpPass;

	cUIImage*			mpLogoImage;
	cUINodeSkin*		mpLogoSkin;

	cButton*			mpConnect;

	/// 로그인 했는지 여부. 
	bool				mSendLogin;
};

//
class cLabel;
class cVersionWindow : public cUIWindow
{
public:
	cVersionWindow();
	virtual ~cVersionWindow();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateSkin();

protected:
	cLabel*	mpVersion;
};