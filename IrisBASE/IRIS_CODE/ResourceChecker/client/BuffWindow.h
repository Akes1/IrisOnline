/* ==========================================================================
* 파    일 : BuffWindow.h
* 목    적 : 버프 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "UIImage.h"

struct sBuffBaseIcon
{
	cUIImage				mImage;
	unsigned long			mUniqueIdx;
	unsigned long			mInfluenceIdx;
};

struct sBuffIcon : public sBuffBaseIcon
{
	unsigned long			mTotalTime;

	/// 시 분 초 표시
	cStringT				mUnit;
	int			mTextX;
	int			mTextY;

	bool					mStartBlink;
	bool					mShow;
	unsigned long			mBlinkTime;

	bool					mAuraType;
	bool					mIsBuff;

	sBuffIcon()
	{
		mTotalTime = 0;

		mStartBlink = false;
		mShow = true;
		mBlinkTime = 0;
		mAuraType = false;
	}
};

//////////////////////////////////////////////////////////////////////////
class cBuffWindow : public cUIWindow
{
public:
	cBuffWindow();
	virtual ~cBuffWindow();

protected:
	unsigned int	mRow;
	unsigned int	mColumn;

	unsigned int	mRowSpace;
	unsigned int	mColumnSpace;

	unsigned int	mIconSize;
	float			mScale;

	unsigned int	mStartX;
	unsigned int	mStartY;

	/// 아이콘 over 이벤트 처리용
	struct sOverInfo
	{
		unsigned int	mOverIconIdx;
		cUIPos			mPos;
		unsigned int	mRow;		/// 줄정보
	};

	sOverInfo		mOverInfo;
	bool			mOverCheck;
	bool			mChangeOverIcon;

	/// 오른쪽 마우스버튼 이벤트 처리용
	unsigned int	mRButtonClickIconIdx;

	/// 아이콘 출력 영역 변경 설정
	bool			mUpdateList;

	/// 아이콘 연결 정보
	typedef	tPointerList<void*>	cBuffIconList;
	cBuffIconList		mIconList;

	bool			mBuffTipShow;
	bool			mIsHeroBuffPage;
};

//////////////////////////////////////////////////////////////////////////
/// cBuffWindowSkin
class cBuffWindowSkin : public cUIWindowSkin
{
public:
	cBuffWindowSkin( eUINodeType type = eUINODE_WINDOW );
	virtual ~cBuffWindowSkin();

	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 1. 간격
	unsigned int	mRowSpace;
	unsigned int	mColumnSpace;

	/// 2. 표시 갯수(줄, 칸)
	unsigned int	mRow;
	unsigned int	mColumn;

	/// 3. 아이콘 크기
	unsigned int	mIconSize;

	/// 4. 비율
	float			mScale;
};
//////////////////////////////////////////////////////////////////////////
