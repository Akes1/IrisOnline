#pragma once
#include "Partybuff.h"

class cDuelBuff : public cBuffWindow
{
public:
	cDuelBuff();
	virtual ~cDuelBuff();

	virtual void AddIcon( unsigned long uniqueIdx, unsigned long influenceIdx );

	void SetUpdateList( bool enable ) { mUpdateList = enable; }
	void SetLeftAlign( bool enable ) { mLeftAlign = enable; }

protected:
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender(cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void EventOverIcon( sBuffBaseIcon* p, const cUIPos& pos );

	virtual void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

protected:
	bool	mLeftAlign;
};
