/* ==========================================================================
* 파    일 : SendNoteWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-06-29
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cEditBox;
class cMultiEditBox;
class cButton;
class cSendNoteWindow : public cUIWindow
{
public:
	cSendNoteWindow();
	virtual ~cSendNoteWindow();
	
	///
	virtual void SetName( LPCTSTR name );

protected:
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnCommand( cUINode* pcaller, unsigned int id );
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnNetLock( int lockTry );
	virtual void OnNetUnLock( int lockTry );

	virtual bool CheckNote();
	virtual void SendNote();

protected:
	/// 받는 이
	cEditBox*	mpNameEdit;

	/// 내용
	cMultiEditBox*	mpMultiEdit;

	/// 보내기 버튼
	cButton*	mpSendButton;
};
