/* ====================================================================
 * 파    일 :	MouseAgent.h
 * 목    적 :	마우스 관리자
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.01
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.01     이법석	    묵향 갱신
 * 
 * 주의사항 :	
 * =================================================================== */

#pragma once

const int MAX_QUESIZE = 32;

class cMouseAgent
{
public:
	cMouseAgent();
	~cMouseAgent();

	///
	struct sMOUSEINFO
	{
		int X;
		int Y;
		int Event;
		int Key;
	};

	///
	enum eMouseEvent
	{
		eME_LDOWN,
		eME_RDOWN,
		eME_MDOWN,
		eME_LUP,
		eME_RUP,
		eME_MUP,
		eME_LDBL,
		eME_RDBL,
		eME_MDBL,
		eME_COUNT,
	};

	/// 생성및 제거
	bool Create( HWND hWnd );
	void Release();

	/// 윈도우 함수
	bool GetWindowMessage( HWND hWnd, unsigned long msg, WPARAM wParam, LPARAM lParam );
	void GetNextMouseEvent();

	///
	bool IsEmptyMouseEvent() { return (mTail == mHead)? true:false; }


	void ClearAllInfo();

	///
	int	GetMouseX()				{ return mX;					}
	int	GetMouseY()				{ return mY;					}
	int	GetDeltaX()				{ return -mAxisX;					}
	int	GetDeltaY()				{ return -mAxisY;					}
	bool	LButtonDown()			{ return mIsMouseEvent[eME_LDOWN];	}
	bool	RButtonDown()			{ return mIsMouseEvent[eME_RDOWN];	}
	bool	MButtonDown()			{ return mIsMouseEvent[eME_MDOWN];	}
	bool	LButtonUp()				{ return mIsMouseEvent[eME_LUP];	}
	bool	RButtonUp()				{ return mIsMouseEvent[eME_RUP];	}
	bool	MButtonUp()				{ return mIsMouseEvent[eME_MUP];	}
	bool	LButtonDoubleClick()	{ return mIsMouseEvent[eME_LDBL];	}
	bool	RButtonDoubleClick()	{ return mIsMouseEvent[eME_RDBL];	}
	bool	MButtonDoubleClick()	{ return mIsMouseEvent[eME_MDBL];	}
	int	GetMouseEventX()		{ return mMouseEventX;			}
	int	GetMouseEventY()		{ return mMouseEventY;			}
	int	GetMouseAxisX()			{ return mAxisX;				}
	int	GetMouseAxisY()			{ return mAxisY;				}
	bool	LButtonPressed()		{ return mIsLBtnPress;				}
	bool	RButtonPressed()		{ return mIsRBtnPress;				}
	bool	MButtonPressed()		{ return mIsMBtnPress;				}
	bool	IsDrag()				{ return mIsDrag;					}
	bool	LButtonDrag()			{ return mIsLBtnDrag;				}
	bool	RButtonDrag()			{ return mIsRBtnDrag;				}
	bool	MButtonDrag()			{ return mIsMBtnDrag;				}
	int	GetWheel()				{ return mWheel;					}

	///
	bool	ControlPressed()		{ return (mCombineKey & MK_CONTROL)? true:false;}
	bool	ShiftPressed()			{ return (mCombineKey & MK_SHIFT)? true:false;	}
	bool AltPressed()			{ return (mCombineKey & MK_ALT)? true:false; }

protected:
	///
	void RecordMouseEvent( int event, WPARAM wParam, LPARAM lParam );

	///
	void ParseCurMouseEvent( sMOUSEINFO* pMouseInfo );

	///
	bool PushMouseInfo( sMOUSEINFO* pIn );

	///
	bool PopMouseInfo( sMOUSEINFO* pOut );

protected:

	///
	sMOUSEINFO*	mpMouseInfo;

	///
	sMOUSEINFO	mCurMouseInfo;

	///
	int			mHead;
	int			mTail;
	int			mQueSize;

	///
	bool		mIsMouseEvent[eME_COUNT];

	///
	bool		mIsLBtnPress;
	bool		mIsRBtnPress;
	bool		mIsMBtnPress;
	bool		mIsLBtnDrag;
	bool		mIsRBtnDrag;
	bool		mIsMBtnDrag;
	bool		mIsDrag;

	///
	int			mX;
	int			mY;
	int			mOldX;
	int			mOldY;
	int			mMouseEventX;
	int			mMouseEventY;

	int			mAxisX;
	int			mAxisY;

	///
	int			mWheelMsg;
	int			mWheel;

	///
	int			mCombineKey;

	///
	NiWindowRef mhWnd;
};
