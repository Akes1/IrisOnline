#pragma once
#include "UIWindow.h"
#include "Tarot_Common.h"

struct sNpcTarotDeck;

/// child window
class cTarotOpenWindow;
class cTarotCheckWindow;
class cTarotSelectWindow;
class cTarotResultWindow;
class cNpcTarotCheckWindow;
class cNpcTarotSelectWindow;
class cNpcTarotResultWindow;

class cLabel;
class cButton;
class cItemIcon;
class cCardImage;
class cItemSpread;
class cPlaneObject;
class cUIImage;

const int SPREAD_GAPY = 9;
const int TAROT_GAPX = 7;
const int TAROT_GAPY = 4;
const int TAROTNUMBER_TEXTSTART = 1000001;

struct sStringData
{
	cStringT spreadName;
	unsigned long color;

	sStringData()
	{
		color = 0;
	}
};

class cTarotWindow : public cUIWindow
{
public:
	cTarotWindow();
	virtual ~cTarotWindow();

	enum 
	{
		eCardDrama_Appear1 = 0,
		eCardDrama_Appear2,

		eCardDrama_Max,
	};

	virtual void Open();
	virtual void Close();

	void ChangeTarotPage( unsigned char page );
	bool OpenSelectCard( cCardImage* image = 0 );
	bool OpenResultCard();

	bool OpenNpcResultCard();								/// 결과 카드 & 해석 오픈
	bool OpenNpcResultReview( unsigned int selectIndex );	/// 결과 해석 리뷰 오픈

	void SetFortuneData( unsigned long data );				/// 서버로부터 점보기 가능한 npc 운세를 받는다
	inline unsigned long GetFortuneData() { return mFortuneData; }

	// 연출중인지 여부
	bool IsEnalbeOpenDrama() { return mEnableOpenDrama; }

	void SetPrice( unsigned long price );
	void SetTitle( LPCTSTR str );
	void SetCardInfo( unsigned int index, unsigned int itemIndx, bool InitPoint = false );
	void SetSpreadInfo( unsigned int index, unsigned int itemIndex );
	void SetSelectedSpread( unsigned int index );

	inline cItemSpread* GetCurrentSpreadInfo() { return mpSpreadInfo;} 
	inline unsigned int GetSelectedSpreadIndex() { return mSelectSpreadIndex; }
	inline unsigned int GetSelectedCount() { return mSelectCardCount; }
	inline unsigned int GetShowReviewIndex() { return mShowReviewIndex; }
	inline unsigned int GetMaxCount() { return mMaxSelect; }
	inline unsigned long GetPrice() { return mPrice; }
	inline unsigned long GetCardTotalPoint() { return mCardTotalPoint; }
	inline unsigned long GetCurrentResult() { return mCurrentResult; }
	inline unsigned int GetCardHalfWidth() { return mCardHalfWidth; }
	inline unsigned int GetCardHalfHeight() { return mCardHalfHeight; }
	inline LPCTSTR GetTitle() { return mTitle.Cstr(); }

public:
	/// NPC 점집 정보 세팅
	void SetNpcTarotDeck( unsigned long* tarotArr );
	void SetNpcSpread( unsigned long* spreadArr, short size );

	bool OpenSelectNpcCard( cCardImage* image = 0 );
	void SetSelectedNpcSpread( unsigned int index );

	/// NPC 점집 결과 생성
	void MakeNpcTarotResult();
	unsigned int GetNpcHaveSpreadCount() { return mNpcSpreadCount; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnImageHovered( cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void OnLButtonDoubleClick( const cUIPos& pos );
	
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );

	void OnIconClicked( cUINode* pcaller, unsigned int id );
	void OnImageClicked( cUINode* pcaller, unsigned int id );

	void OnHide();
	void OnShow();

	void UpdateSkin();
	void UpdateRect();

	void ShowHeroTarotCard();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

protected:
	cTarotOpenWindow*		mpOpenWindow;
	cTarotCheckWindow*		mpCheckWindow;
	cTarotSelectWindow*		mpSelectWindow;
	cTarotResultWindow*		mpResultWindow;
	cNpcTarotCheckWindow*	mpNpcCheckWindow;
	cNpcTarotSelectWindow*	mpNpcSelectWindow;
	cNpcTarotResultWindow*	mpNpcResultWindow;

	///
	cUINodeSkin* mpTarotSkin;
	unsigned int mCardHalfWidth;
	unsigned int mCardHalfHeight;

	/// 스프레드 배경이미지와 스킨
	cUIImage*		mpSpreadBackImg;
	cUINodeSkin*	mpSpreadBackSkin;
	bool			mShowSpreadBack;

	/// 선택 이미지
	cPlaneObject*	mpSelectImage;
	int				mSelectArrIndex;	/// 선택된 스프레드 인덱스
	cUIRect			mSelectRect[MAX_SPREAD];		/// 선택 영역

	/// 타로 관리 정보
	unsigned long mPrice;					/// 이용 금액
	unsigned int mMaxSelect;				/// 최대로 선택할 수 있는 갯수
	cStringT mTitle;

	unsigned long mCardTotalPoint;
	unsigned long mCurrentResult;

	unsigned int mSelectCardCount;
	
	unsigned int mSelectSpreadIndex;
	cItemSpread* mpSpreadInfo;

	/// 카드
	typedef tPointerArray<void*> cTarotCardArr;
	cTarotCardArr	mCardArr;
	cUIRect mDefaultCardRect;

	/// 스프레드 아이콘
	sStringData		mStringData[MAX_SPREAD];
	cItemIcon*		mpIcon[MAX_SPREAD];

	struct sDramaInfo
	{
		unsigned int mIndex;
		unsigned long mStartTime;
	};

	bool mEnableOpenDrama;
	unsigned long mStartTime;
	unsigned long* mCurrentPattern;
	bool mTarotFront;
	unsigned char mTarotFadeState;

protected:
	/// NPC 타로 관리 정보
	unsigned int mNpcSpreadCount;
	unsigned long mNpcSpreadIndex[MAX_SPREAD];
	unsigned long mNpcTarotIndex[MAX_TAROT_CARDS];

	/// NPC 타로 결과 정보
	sTarotResult	mNpcTarotResult[13];

	/// 보고있는 review 인덱스 
	unsigned int mShowReviewIndex;

	/// 이용가능한 운세 정보
	unsigned long mFortuneData;
};

inline void cTarotWindow::SetPrice( unsigned long price )
{
	mPrice = price;
}

inline void cTarotWindow::SetTitle( LPCTSTR str )
{
	mTitle = str;
}

inline void cTarotWindow::SetNpcTarotDeck( unsigned long* tarotArr )
{
	for( int i = 0; i < MAX_TAROT_CARDS; ++i, ++tarotArr )
		mNpcTarotIndex[i] = *tarotArr;
}

inline void cTarotWindow::SetNpcSpread( unsigned long* spreadArr, short size )
{
	mNpcSpreadCount = size;

	for( int i = 0; i < size; ++i, ++spreadArr )
		mNpcSpreadIndex[i] = *spreadArr;
}

inline void cTarotWindow::SetFortuneData( unsigned long data )
{
	mFortuneData = data;
}

