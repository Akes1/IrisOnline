/* ==========================================================================
* 颇    老 : DealWindow.h
* 格    利 : 快祈 扩档快 
* 累 己 磊 : 
* 累 己 老 : 2009-05-25
* 林狼荤亲 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"

class cMailListSheet;
class cMailReadSheet;
class cMailSendSheet;

class cMailWindow : public cTabWindow
{
public:
	cMailWindow();
	virtual ~cMailWindow();

	virtual void Open();
	virtual void Close();

	virtual void Show( unsigned long npcIndex );

	cMailListSheet* GetListSheet() { return mpListSheet; }
	cMailReadSheet* GetReadSheet() { return mpReadSheet; }
	cMailSendSheet* GetSendSheet() { return mpSendSheet; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* node, unsigned int id );
	void OnTabButtonPressed( cUINode* caller, unsigned int id );
	void OnShow();
	void OnHide();
	void Clear();

private:
	cMailListSheet* mpListSheet;
	cMailReadSheet* mpReadSheet;
	cMailSendSheet* mpSendSheet;
};