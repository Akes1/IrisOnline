#pragma once
#include "PlaneObject.h"

// 일반 애니메이션 2D
class cBaseAniPlane : public cPlaneObject
{
public:
	cBaseAniPlane();
	~cBaseAniPlane();

	/// 생성부
	virtual bool Create( NiTexture* pTexture, 
						int screenX, int screenY, unsigned int screenW, unsigned int screenH,
						unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom );

	virtual void Process( unsigned long accumTime );

	void BeginAnimation();
	void EndAnimation();

	void SetAnimationInfo( short totalFrame, unsigned long timeOut, bool loop );

protected:
	short			mFrames;	/// 프레임수

	short			mCurFrame;	/// 현재 프레임

	bool			mLoop;		/// 애니메이션 한바퀴 돌리기

	bool			mBeginAnim;	/// 애니사용여부

	/// frame texture size
	unsigned int	mTexX;
	unsigned int	mTexY;

	unsigned int	mTexW;
	unsigned int	mTexH;

	/// 타이머
	unsigned long	mTimeOut;	/// 애니메이션 갱신 시간
	unsigned long	mLastAccumTime;
};

// 효과만 연출하고 사라지는 애니메이션
class cEffetAniPlane : public cBaseAniPlane
{
public:
	cEffetAniPlane();
	~cEffetAniPlane();

	/// 생성부
	virtual bool Create( NiTexture* pTexture, 
						int screenX, int screenY, unsigned int screenW, unsigned int screenH,
						unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom );
	
	virtual void Process( unsigned long accumTime );
	virtual void Draw();

protected:
	unsigned int	mColumCount;
};