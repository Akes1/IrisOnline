#pragma once
#include "DepthPlane.h"
#include "BaseObject_Common.h"

enum eBoardType
{
	eBOARD_TYPE_NONE,
	eBOARD_TYPE_TAROT,
	eBOARD_TYPE_USER_SELL,
	eBOARD_TYPE_NPC_SIGN,
	eBOARD_TYPE_TUTORIAL_SIGN,
};

class cSignboard : public cPlane
{
public:
	virtual ~cSignboard();
	virtual void Draw();

	inline unsigned int GetBoardHeight() { return mSignHeight; }

	inline virtual void Active( LPTSTR name, LPTSTR title, unsigned long color, bool loop = false ) = 0;
	inline virtual void DeActive() { mActive = false; }

	inline bool IsActive() { return mActive; }

	virtual void SetPos( int screenX, int screenY );

	virtual bool PickPoint( float x, float y );

	virtual int GetType(){ return mType; }

protected:
	cSignboard();

protected:
	bool				mActive;

	NiMeshScreenElementsPtr	mpBoxElement;
	unsigned int		mSignWidth;
	unsigned int		mSignHeight;

	unsigned int		mAllowTextWidth;

	sObject				mOwner;

	struct sSignString
	{
		TCHAR			mString[128];
		int				mStrLen;
		unsigned long	mColor;

		sSignString()
		{
			_stprintf_s( mString, _T("%s"), _T("") );
			mStrLen = ::_tcslen(mString);
			mColor = 0xFFFFFFFF;
		}
	};

	sSignString		mTitle[2];
	sSignString		mName;

	int	mScreenPosX;
	int	mScreenPosY;

	eBoardType mType;
};

class cTarotSignboard : public cSignboard
{
public:
	cTarotSignboard();
	virtual ~cTarotSignboard();

	void Active( LPTSTR name, LPTSTR title, unsigned long color, bool loop = false );
	void DeActive();

	void Draw();
};

class cUseSellSignboard : public cSignboard
{
public:
	cUseSellSignboard();
	virtual ~cUseSellSignboard();

	void Active( LPTSTR name, LPTSTR title, unsigned long color, bool loop = false );
	void DeActive();

	void Draw();

};

class cNpcSignboard : public cSignboard
{
public:
	cNpcSignboard( unsigned long npcIndex );
	virtual ~cNpcSignboard() {}

	inline virtual void Active( LPTSTR, LPTSTR, unsigned long, bool ) {}

	virtual void SetPos( int screenX, int screenY );

protected:
	unsigned long mNpcIndex;
};

// 튜토리얼용 싸인보드
struct sTutorialBaseImage;
class cTutorialSignboard : public cSignboard
{
public:
	cTutorialSignboard( unsigned char type, unsigned long classIndex, unsigned long objectIndex );
	virtual ~cTutorialSignboard();

	void Update( unsigned long accumtime );

	void ClearImage();
	void UpdateImage();

	inline virtual void Active( LPTSTR, LPTSTR, unsigned long, bool ) {}

	inline short GetPosX() { return mPosX; }
	inline short GetPosY() { return mPosY; }

protected:
	//
	NiTexturingProperty*	mpTextureProp;

	//
	unsigned char mObjectType;
	unsigned long mObjectIndex;
	unsigned long mObjectClassIndex;

	sTutorialBaseImage* mpImageData;

	// 애니메이션 관련 정보
	short			mCurFrame;	/// 현재 프레임
	bool			mBeginAnim;	/// 애니사용여부

	/// 타이머
	unsigned long	mLastAccumTime;

	short			mPosX;
	short			mPosY;
};