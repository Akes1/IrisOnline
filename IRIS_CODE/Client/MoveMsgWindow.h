/* ==========================================================================
* 파    일 : MoveMsgWindow.h
* 목    적 : 
* 작 성 자 : 조영인
* 작 성 일 : 2009-11-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

struct sMoveMsgInfo
{
	cStringT mMoveMsg;
	long	 mMoveMsgColor;
	int      mTextPosX;

	sMoveMsgInfo()
	{
		mMoveMsgColor = 0;
		mTextPosX     = 0;
	}
};

class cMoveMsgWindow :public cUIWindow
{
public:
	cMoveMsgWindow();
	virtual ~cMoveMsgWindow();

	void AddMoveMsg( LPCTSTR msg1, unsigned long color1, LPCTSTR msg2, unsigned long color2 );

protected:
	void Open(){};
	void Close();
	void OnRender( cUIFontItemKeeper* pKeeper );

private:
	sMoveMsgInfo mMoveMsg[2];
	int			 mTextPosY;
};