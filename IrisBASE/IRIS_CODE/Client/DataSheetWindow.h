/* ==========================================================================
* 파    일 : DataSheetWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-06-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cDataSheetWindow : public cUIWindow
{
public:
	cDataSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cDataSheetWindow();
};
