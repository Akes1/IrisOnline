#pragma once
#include "DepthPlane.h"

const unsigned int MSG_ACTIVE_TIME = 2000;
const unsigned int MSG_TEXT_LENGTH = 40;

const unsigned int MAX_NAME_SIZE = 16;

/// Quest Msg »Æ¿Â
class cCenterMsgPlane : public cPlane
{
public:
	cCenterMsgPlane();
	virtual ~cCenterMsgPlane();

	void Init( NiPoint3 pos );
	void Update( unsigned long time );

	void ActiveMsg( LPCTSTR msg, NiPoint3 pos, unsigned long color );
	void Clear();

	bool IsActive() { return (mMsgList.IsEmpty() == false); }

protected:
	void Draw();

protected:
	struct sMsgItem
	{
		TCHAR			Messge[MSG_TEXT_LENGTH];
		unsigned int	MsgLen;
		unsigned int	MsgWidth;
		unsigned long	MsgColor;

		int PosX;
		float PosY;

		float	valY;
		float	checkY;
		float	valAlpha;

		bool PrevCheck;

		sMsgItem()
		{
			memset(Messge, 0, sizeof(TCHAR)*MSG_TEXT_LENGTH);
			MsgLen = 0;
			MsgWidth = 0;
			MsgColor = 0xFFFFFFFF;
			PosX = 0;
			PosY = 0.0f;

			PrevCheck = false;
		}
	};

	typedef tPointerList<void*> cMsgItemList;
	cMsgItemList mMsgList;
	cMsgItemList mDeleteMsgList;

	unsigned long mBackColor;
};
