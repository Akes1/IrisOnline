/* ==========================================================================
* 파    일 : ReceiveNoteWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-06-29
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cEditBox;
class cLabel;
class cReceiveNoteWindow : public cUIWindow
{
public:
	cReceiveNoteWindow();
	virtual ~cReceiveNoteWindow();

	///
	void SetName( LPCTSTR name );
	
	///
    void SetText( LPCTSTR text );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );

private:
	/// 보낸 이
	cEditBox*	mpNameEdit;

	/// 보낸내용
	cLabel*		mpLabel;
};

