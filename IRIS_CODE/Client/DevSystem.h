/* ====================================================================
* 파    일 :	DevSystem.h
* 목    적 :	개발시에 필요한 정보들을 셋팅하는 클래스
* 작 성 자 :	이법석
* 작 성 일 :	2006.01
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		2006.01     이법석	    생성
* 
* 주의사항 :	릴리즈 버젼에서 완전히 제가 될 수 있도록 만드세요
* =================================================================== */
#pragma once

class cDevSystem
{
public:
	static cDevSystem* mpDevSystem;

	cDevSystem();
	~cDevSystem();


	bool Init();
	void Exit();

	void Open() {}
	void Close() {}

	void Process( unsigned long accumTime );

	void Invalidate() {}
	void Restore() {}

	void IncreaseNode()		{	++mNode;	}
	void ResetNodeCount()	{	mNode = 0;	}

	void DrawDevInfo();

	int mSolidRenderCount;
	int mDamageRenderCount;
	int mAlphaRenderCount;
	int mAlphaTestRenderCount;

	int mTerrainRenderCount;

	unsigned long	mScaleAccumTime[2];
	unsigned long	mAniLastTime[2];
	float	mScaleFactor[2];
	

private:

	/// 글자 입력용 2D화면 및 정보
	NiColorA		mTxtColor;

	/// FPS 계산을 위해서
	unsigned long		mLastTime;
	long double			mFrames;
	float				mFPS;
	///

	/// 화면 객체 정보
	int				mNode;
	int				mTriangle;
};

#define DEVSYSTEM	cDevSystem::mpDevSystem
