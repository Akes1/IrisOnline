/* ==========================================================================
* 파    일 : NumPassWindow.h
* 목    적 : 
* 작 성 자 : 조영인
* 작 성 일 : 2009-04-28
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cEditBox;
class cButton;

class cNumPassWindow : public cUIWindow
{
public:
	cNumPassWindow();
	virtual ~cNumPassWindow();

	void Open();

	///
	void ClearPass( bool focus = false );
	
	//
	void SetSendLoginFlag( bool set ) { mSendLogin = set; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void UpdateSkin();

	void MixNumberPos();
	void SetAllNumEnable( bool enable );

	/// 로그인 패킷 발송
	void SendLoginMsg();

protected:
	cEditBox*			mpPass;
	cButton*			mpNumBtn[10];

	cButton*			mpConnect;

	/// 로그인 했는지 여부. 
	bool		mSendLogin;
};
