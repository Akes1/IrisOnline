/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.08.03
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIWindow.h"
#include "Label.h"
#include "Button.h"

class cBaseStage;

/// 메시지 핸들러
class cMsgBoxHandler
{
public:
	virtual void MsgBoxEventParser( unsigned int event, unsigned int result ) = 0;
};

/// 메시지 윈도우
class cMsgWindow : public cUIWindow
{
public:
	cMsgWindow();
	virtual ~cMsgWindow();

	bool Create( unsigned int eventType, cMsgBoxHandler* msgHandler, const cString& skinPrefix, cUIContainer* parent, unsigned int id );
	void ShowModal( unsigned int ansType = MB_OK );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long accumTime );
	virtual void OnOk();
	virtual void OnCancel();

protected:
	/// 메시지박스 이벤트 
	unsigned int mEventType;

	/// 메시지 핸들러
	cMsgBoxHandler* mMsgHandler;

	/// 대답 타입
	unsigned int mAnsType;

	/// 스킨 접두어
	cString mSkinPrefix;

	/// 라벨
	cLabel mCaptionLabel;
	cLabel mTextLabel;

	/// 버튼
	cButton mOk_OkButton;
	cButton mOkCancel_OkButton;
	cButton mOkCancel_CancelButton;
};
