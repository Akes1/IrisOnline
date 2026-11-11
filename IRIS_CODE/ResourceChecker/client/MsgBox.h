/* ==========================================================================
* 파    일 : MsgBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-04-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
//#include "UIMsgboxEvent.h"
#include "UINodeData.h"

class cBaseStage;
class cButton;
class cLabel;
class cEditBox;

class cMsgBox : public cUIWindow
{
	friend class cUIManager;
public:
	virtual ~cMsgBox();
	/// Msg박스의 생성은 UIMANAGER로 한정한다.
	cMsgBox( eUINodeType type = eUINODE_MESSAGEBOX );

protected:
	/// 자식 컨트롤 스킨 
	cString mOkButtonSkin;
	cString mYesButtonSkin;
	cString mNoButtonSkin;
	cString mEditSkin;

	cButton*	mpYesButton;
	cButton*	mpNoButton;
	cEditBox*	mpEdit;		
	cLabel*		mpLabel;		/// 텍스트 출력 라벨

	eBoxSkin_Type mSkinType;
		
	// 데이터
	cUIPos		mCaptionPos;
	cStringT	mCaptionText;
	cStringT	mText;			// 처음 세팅된 텍스트

	void*		mExtraData;

	/// 시간 제한 넣기
	unsigned long	mStartTime;
	unsigned long	mTotalTime;
	bool			mTimePrint;			/// 시간정보를 출력할 것인지 여부
	bool			mTimeResult;		/// 성공 or 실패 메세지
	
	//
	unsigned int mEnableLength;			/// yes 버튼이 활성화 될 최소 글자 조건.
};


//////////////////////////////////////////////////////////////////////////
class cMessageBoxSkin : public cUINodeSkin
{
public:
	cMessageBoxSkin( eUINodeType type = eUINODE_MESSAGEBOX );
	virtual ~cMessageBoxSkin();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 이동가능 여부
	bool mMoveable;

	/// 캡션영역 
	cUIRect mCaptionRect;

	/// 버튼 박스 스킨 네임
	cString mOkButtonSkin;
	cString mYesButtonSkin;
	cString mNoButtonSkin;
	cString mEditSkin;
};
