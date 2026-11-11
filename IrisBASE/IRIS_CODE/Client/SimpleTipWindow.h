#pragma once
#include "tipwindow.h"

class cSimpleTipWindow :public cTipWindow
{
public:
	cSimpleTipWindow();
	virtual ~cSimpleTipWindow();

	virtual void Open() {}

	bool SetSimpleTip( const cUIPos& pos, cStringT str );

protected:
	void OnRender( cUIFontItemKeeper* fontKeeper );
};

// Äù½ºÆ® Á¤º¸ ÅøÆÁ 
class cQuestTipWindow : public cTipWindow
{
public:
	cQuestTipWindow();
	virtual ~cQuestTipWindow();

	virtual void Open() {}

	bool AddQuestInfo( LPCTSTR str, unsigned long color, bool addgap = false );
	bool ShowQuestInfo( const cUIPos& pos );

protected:
	void OnRender( cUIFontItemKeeper* fontKeeper );

	void OnHide();
};