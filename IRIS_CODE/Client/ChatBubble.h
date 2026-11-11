#pragma once

/*
원형틀
131 0 ( 4 * 23 )
135 0 ( 1 * 23 )
136 0 ( 4 * 23 )

143 0 ( 4 * 38 )
147 0 ( 1 * 38 )
148 0 ( 4 * 38 )

155 0 ( 4 * 53 )
159 0 ( 1 * 53 )
160 0 ( 4 * 53 )
*/

#include "DepthPlane.h"

const unsigned int BUBBLE_LINE_MAX = 3;		// 라인 수
const unsigned int BUBBLE_TEXT_LENGTH = 15;	// 문자 수
const unsigned long BUBBLE_ACTIVE_TIME = 5000;		// 5초

struct sTextItem
{
	TCHAR	mText[512];
//	cStringT mText;
	unsigned long mColor;

	unsigned int mWidth;

	sTextItem*	mpNext;

	sTextItem() : mpNext(0) 
	{
		memset( mText, 0, sizeof(TCHAR)*512 );
		mWidth = 0;
	}
};

class cBubbleBase : public cPlane
{
public:
	virtual ~cBubbleBase();

	enum
	{
		LEFT_ELEMENT = 0,
		CENTER_ELEMENT,
		RIGHT_ELEMENT,

		ELEMENT_MAX,
	};

	virtual void Update( unsigned long time );
	virtual void Draw() = 0;

	virtual void Active( LPTSTR msg, unsigned long color, bool loop = false ) = 0;
	virtual void Active( sTextItem* text, bool loop = false ) = 0;

	inline void DeActive() { mActive = false; }
	inline bool IsActive() { return mActive; }

	unsigned int GetBubbleHeight();

	void SetPos( int screenX, int screenY );

protected:
	cBubbleBase();

	struct sBubbleBox
	{
		NiMeshScreenElementsPtr	mpBoxElement;

		unsigned int		mFrameWidth;		/// 문자길이에 따라 유동이다.
		unsigned int		mFrameHeight;		/// 셋팅 이후 변하지 않는다.
	};

protected:
	/// 말풍선 이미지를 위한 정보
	sBubbleBox		mBubbleBox[BUBBLE_LINE_MAX];

	/// 말풍선의 시작위치
	unsigned int	mPosX;
	unsigned int	mPosY;

	/// 표현중인 라인의 크기
	unsigned int	mLineNum;

	/// 라인별 채팅 메세지 길이
	unsigned int	mChatMsgLen[BUBBLE_LINE_MAX];

	/// 채팅 메세지 표현부의 이미지 픽셀
	unsigned int	mChatMsgWidth;

	/// 활성화 여부
	bool			mActive;
	unsigned long 	mActiveTime;

	bool			mLoop;

	int				mFontSpace;	/// 폰트가 적힐 위치의 상하여백

	unsigned long	mAlphaColor;
};

class cChatBubble : public cBubbleBase
{
public:
	cChatBubble();
	virtual ~cChatBubble();

	/// 말풍선을 띄우다
	virtual void Active( LPTSTR msg, unsigned long color, bool loop = false );
	virtual void Active( sTextItem*, bool ){}

//	void Update( NiPoint3& pos, unsigned long time );
	void Draw();

protected:
	/// 라인별 채팅 메세지
	TCHAR			mChatMsg[BUBBLE_LINE_MAX][BUBBLE_TEXT_LENGTH+1];

	unsigned long	mChatColor;
};

class cFuncChatBubble : public cBubbleBase
{
public:
	cFuncChatBubble();
	virtual ~cFuncChatBubble();

	/// 말풍선을 띄우다
	virtual void Active( LPTSTR msg, unsigned long color, bool loop = false );
	virtual void Active( sTextItem*, bool loop = false );

//	void Update( NiPoint3& pos, unsigned long time );
	void Draw();


protected:
	tArray<sTextItem>	mChatMsg[BUBBLE_LINE_MAX];

};