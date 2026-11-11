#pragma once
#include "tipwindow.h"

const unsigned int MAX_RIGHT_BUFF_TIP = 2;

class cBuffToolTipWindow : public cTipWindow
{
public:
	cBuffToolTipWindow();
	virtual ~cBuffToolTipWindow();
	
	virtual void Open();
	virtual void Close();

	bool SetBuff( const cUIPos& pos, unsigned long uniqueIdx, unsigned long influenceIdx, bool showTime );

	virtual void Show( bool onsound = true );
	virtual void Hide( bool onsound = true );

protected:
	
	void OnTimer();
	void OnRender( cUIFontItemKeeper* fontKeeper );
	void Clear();

	bool SetBuffTip( unsigned long uniqueIdx, unsigned long influenceIdx );

protected:
	cTipParam		mRightTip[MAX_RIGHT_BUFF_TIP];
	unsigned int	mRightIndex;

	bool			mShowTime;
	unsigned long	mUniqueIdx;

	unsigned int	mRightWidth;

	int	mGaurdTipIdx[2];
};
