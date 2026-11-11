#pragma once

#include "UIWindow.h"

class cButton;



class cSummonWindow : public cUIWindow
{

public:

	cSummonWindow();
	virtual ~cSummonWindow();

	virtual void Open();
	virtual void Close();

	void Clear();
	void Show( unsigned short slotIdx);
	void StartCoolTime();

protected:

	virtual bool	OnCreate( cUINodeProperty* pproperty );
	virtual void	OnCommand( cUINode* caller, unsigned int id );
	virtual void	OnNetLock( int lockTry );
	virtual void	OnNetUnLock( int lockTry );
	virtual void	OnHide();

	bool			CheckUsableState();

protected:

	cButton*		mpSummonBtn[3];
	unsigned short	mSummonSlotIdx;
};


