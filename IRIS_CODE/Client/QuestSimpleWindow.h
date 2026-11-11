/* ==========================================================================
* 파    일 : QuestSimpleWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "QuestWindow.h"

const int START_GAPX = 7;
const int START_GAPY = 9;

struct sSimpleData
{
	unsigned long	mTimeLine;	 /// 출력해야할 라인
	unsigned long	mQuestIndex; /// 퀘스트 인덱스
};

/// 
struct sQuestHaveData
{
	tPointerList<void*> mQuestProgressList;	/// 퀘스트 상세정보리스트
	bool				mClose;				/// close 정보 (퀘스트 리스트 보기 여부)
	cUIImage*			mCloseImage;		/// close 정보의 이미지
	cStringT			mMapName;			/// 맵이름
	unsigned int		mTotalCount;		/// 보유한 퀘스트 갯수

	sQuestHaveData()
	{
		mClose = false;
		mCloseImage = 0;
		mTotalCount = 0;
	}
};

/// 퀘스트 진행정보
struct sQuestProgress
{
	unsigned long	mIndex;		/// 퀘스트 인덱스
	cQuestDefine*	mDefine;	/// 퀘스트 정의
	cStringT		mQuestName;	/// 퀘스트명
	__int64*		mCheck;		/// 조건 진행정보
	unsigned long	mTime;		/// 시간정보
	cUIImage*		mCloseImage;/// close 정보의 이미지

	sQuestProgress()
	{
		mIndex = 0;
		mDefine = 0;
		mCheck = 0;
		mTime = 0;
		mCloseImage = 0;
	}
};

class cPlaneObject;
class cQuestSimpleWindow : public cUIWindow
{
public:
	cQuestSimpleWindow();
	virtual ~cQuestSimpleWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	void AddQuestInfo( unsigned long questIndex, __int64* check );
	void DeleteQuestInfo( unsigned long questIndex );

	//
	void UpdateSelect();

	void Show( bool onsound = true );

	void GetHideMapCategorySet( tHashSet<unsigned int>& mapSet );
	void SetHideMapCategory( unsigned long mapIndex );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void OnLButtonDoubleClick( const cUIPos& pos );
	void UpdateRect();
	void UpdateSkin();

	/// 윈도우 사이즈 갱신 
	void UpdateSize();

	// 해당 퀘스트의 시작 줄 위치 구하기
	int FindStartRow( unsigned long questIndex );

private:
	enum
	{
		eBACKIMAGE_TOP = 0,
		eBACKIMAGE_CENTER,
		eBACKIMAGE_BOTTOM,
		eBACKIMAGE_LEFT,
		eBACKIMAGE_RIGHT,

		eBACKIMAGE_MAX
	};

	/// 맵 단위 퀘스트 정보 ( 맵인덱스, 퀘스트 리스트 sQuestHaveData )
	typedef tPointerHashMap<unsigned int, void*> cQuestMap;
	cQuestMap	mQuestMap;

	cUIImage*	mpHighImage;
	cUIImage*	mpLowImage;

	/// 체크, 노체크 텍스쳐 위치
	cUIPos			mHighCheckPos;
	cUIPos			mHighNoCheckPos;
	cUIPos			mLowCheckPos;

	cUISize			mImageSize;

	/// 배경 이미지
	cUINodeSkin*	mpBackSkin[eBACKIMAGE_MAX];
	cPlaneObject*	mpBackImage[eBACKIMAGE_MAX];

	/// 기본 넓이
	unsigned int	mInitWidth;
	unsigned int	mInitX;

	/// 선택이미지
	cPlaneObject*	mpSelectImage;
	bool			mShowSelect;

	/// 추적이미지
	cUINodeSkin*	mpOnSkin;
	cPlaneObject*	mpOnImage;
	bool			mShowOn;
};