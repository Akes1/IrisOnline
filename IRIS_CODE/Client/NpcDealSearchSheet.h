/* ==========================================================================
* 파    일 : NpcDealSearchSheet.h
* 목    적 : 판매대행 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2009-04-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "StyleListBox.h"

class cUIImage;
class cEditBox;
class cNumberEditBox;
class cComboBox;
class cListFolderBox;
class cButton;
class cSpinBox;
struct NPC_ITEM_AGENT_SEARCH_RESULT;

const unsigned int BUYBUTTON_ID = 10000;
const unsigned int HIGHLEVEL = 99;
const unsigned int LOWLEVEL = 1;

/// 페이지 번호 구조체
struct sPageNum
{
	cStringT	 mPageNum;
	long		 mColor;
	cUIPos		 mPos;
	unsigned int mTextWidth;
	bool		 mIsShow;
};

class cNpcDealSearchSheet : public cUIWindow
{
public:
	cNpcDealSearchSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cNpcDealSearchSheet();

	void Clear();
	void ClearData();

	bool AddItem( NPC_ITEM_AGENT_SEARCH_RESULT* data );
	void UpdateItemList();	// 아이템 리스트 세팅 - UI 에 갱신
	
	void SavePage( long startPage, long endPage );
	void SaveResult( int result );	// 결과

	void SendDealBuy( long idx, unsigned int count );

	// 목록 재 요청 보내기
	void SendUpdateList( unsigned int pageNum );

	// 롤백
	void RollBackPage();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* node, unsigned int id );
	void OnListBoxImageHorvered( cUINode* , unsigned int , const cUIPos& , void* );
	void OnListBoxImageLeft( cUINode* , unsigned int );
	void OnListBoxChanged( cUINode* , unsigned int );
	void UpdateSkin();
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	void UpdateRect();

	void SetTypeList();
	bool CheckItemName( LPCTSTR word, unsigned int len );

	void UpdatePage();
	void ChangePage( unsigned int page );

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	void SetSelectPage( unsigned int page );

	void ClearPage();

private:
	// 정보 관리및 삭제용 NPC_ITEM_AGENT_SEARCH_RESULT
	typedef tPointerArray<void*> cNpcDealSearchArr;
	cNpcDealSearchArr	mDealSearchArr;

	/// 아이템명
	cEditBox*	mpNameEdit;

	/// 레벨구간
	cNumberEditBox*	mpHighLv;
	cNumberEditBox*	mpLowLv;

	/// 등급
	cComboBox*	mpRare;

	/// 강화단계
	cComboBox* mpEnhance;

	/// 종류
	cListFolderBox*		mpTypeList;
	cUIImage*			mpHighImage;

	/// 검색 버튼
	cUINode*		mpSearch;

	/// 아이템 결과 리스트
	cImageAndFontDataListBox*	mpItemList;

	/// 구매버튼
	cButton*		mpBuyButton[10];

	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} mSort;
		BYTE mSortData;
	};

	// 임시정보
	cStringT mTempName;
	char mTempType;
	short mTempSubType;
	unsigned char mTempLowLv;
	unsigned char mTempHighLv;
	unsigned char mTempEnhance;
	unsigned char mTempRare;

	// 백업정보
	int mTempPage;
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} mTempSort;
		BYTE mTempSortData;
	};

	// 시간 딜레이 - 메세지박스 닫은후 처리하기위함
	unsigned long mDealDelayTime;
	unsigned int  mSaveStartPage;
	unsigned int  mSaveEndPage;
	int mSaveResult;
	unsigned int  mSaveCurrentPage; ///현재 페이지 번호

	/// 판매대행 페이지
	sPageNum mPageNum[10];	
	cButton* mpLeftPageButton;
	cButton* mpRightPageButton;
	cUINodeSkin* mpPageNumPos;
	cUIRect	 mPageTextRect;

};

inline void cNpcDealSearchSheet::SaveResult( int result )
{
	mSaveResult = result;
}
