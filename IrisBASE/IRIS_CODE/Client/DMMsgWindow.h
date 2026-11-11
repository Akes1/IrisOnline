/* ==========================================================================
* 파    일 : DMMsgWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-09-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cDMMsgWindow : public cUIWindow
{
public:
	cDMMsgWindow();
	virtual ~cDMMsgWindow();

	virtual void Open() {}

	void Clear();
	void Show( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void UpdateSkin();

protected:
	cUINode*	mpJoin;
	cUINode*	mpCancel;
	cUINode*	mpCountCancel;

	cLabel*		mpCaption;
	cLabel*		mpText;
};
