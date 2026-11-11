/* ==========================================================================
* 작 성 자 : 
* 작 성 일 : 2009.09.11
* 내    용 : 화면 출력용 이미지 or 텍스트
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 중앙 공지 출력 
const unsigned int MSG_PRINT_LENGTH = 50;
const unsigned int MSG_PRINT_BUFFER_LENGTH = MSG_PRINT_LENGTH + 21;
const unsigned int PRINT_POSY = 100;
const unsigned int MAX_NOTICCOUNT = 3;
const unsigned int NOTICEMSG_ACTIVE_TIME = 15000;
const unsigned int NOTICEMSG_DUMMYACTIVE_TIME = 3000;

class cCenterNoticeText
{
public:
	cCenterNoticeText();
	virtual ~cCenterNoticeText();

	virtual void AddText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height, 
						  bool autoAddHeight = true, unsigned long activeTime = NOTICEMSG_ACTIVE_TIME );
	virtual void DeleteText();

	virtual void Update( unsigned long deltaTime, unsigned long accumTime );
	virtual void Draw();

	virtual void UpdateScreen();

protected:
	struct sNoticeMsgItem
	{
		unsigned short	fontIdx;
		TCHAR			msg[MSG_PRINT_BUFFER_LENGTH];
		unsigned int	length;
		unsigned long	color;
		unsigned int	width;
		int		posX;
		int		posY;
		unsigned long	startTime;

		sNoticeMsgItem()
		{
			::memset( msg, 0, sizeof(msg) );
			fontIdx = 0;
			length = 0;
			color = 0xFFFFFFFF;
			width = 0;
			posX = 0;
			posY = 0;

			startTime = 0;
		}
	};

protected:
	typedef tPointerList<void*> cMsgItemList;
	cMsgItemList mMsgDelList;

	typedef tPointerList<void*> cMsgItemList;
	cMsgItemList mMsgItemList;

	unsigned int mPosY;
	bool mAutoAddHeight;
	unsigned long mActiveTime;
};
