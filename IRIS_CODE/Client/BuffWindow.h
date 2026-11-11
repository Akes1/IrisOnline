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

	unsigned long			mGaurdCnt;

	sBuffIcon()
	{
		mTotalTime = 0;

		mStartBlink = false;
		mShow = true;
		mBlinkTime = 0;
		mAuraType = false;

		mGaurdCnt = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
class cBuffWindow : public cUIWindow
{
public:
	cBuffWindow();
	virtual ~cBuffWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	virtual void AddIcon( unsigned long uniqueIdx, unsigned long influenceIdx );
	virtual void DelIcon( unsigned long uniqueIdx );
	void BlinkIcon( unsigned long uniqueIdx, unsigned long influenceIdx );

	///
	inline void SetRowSpace( unsigned int space ) { mRowSpace = space; }
	inline void SetColumnSpace( unsigned int space ) { mColumnSpace = space; }
	inline void SetHeroBuff( bool value ) { mIsHeroBuffPage = value; }
	inline void ClearMouseOverInfo(){ mOverInfo.mOverIconIdx = (unsigned int)-1; mOverInfo.mRow = (unsigned int)-1;}

protected:
	/// 처리
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnShow();
	virtual void OnHide();
	virtual void HideTip();
	virtual void OnTimer();
	virtual void EventOverIcon( sBuffBaseIcon* p, const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateSkin();
	
	virtual void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

	virtual void OnNetLock(int lockTry );
	virtual void OnNetUnLock(int lockTry );

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

	cUIImageFont* mImageFont;
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
