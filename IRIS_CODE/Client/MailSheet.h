/* ==========================================================================
* 파    일 : MailListSheet.h
* 목    적 : 우편시스템 윈도우 
* 작 성 자 : 
* 작 성 일 : 2009-05-25
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "StyleListBox.h"

struct sImageData
{
	cUIImage*	mpItemAdd;
	cUIImage*	mpNewMail;

	bool	mShowItem;
	bool	mShowMail;

	sImageData()
	{
		mpItemAdd = 0;
		mpNewMail = 0;
		mShowItem = false;
		mShowMail = false;
	}
};

struct POST_OPEN_RESULT;
class cSpinBox;
class cButton;
class cMailListSheet : public cUIWindow
{
public:
	cMailListSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cMailListSheet();

	void Clear();

	bool AddMail( POST_OPEN_RESULT* post );
	void SetTotalPage( unsigned int total );
	void ChangePage( unsigned int page );
	void SendSelectDelete();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnListBoxSelected( cUINode* , unsigned int );
	void UpdateRect();
	void OnShow();

	void UpdateImage();
	void UpdatePage();

private:
	// 선택 삭제
	cButton*	mpDelete;

	// 전체 선택
	cButton*	mpAllSelect;

	// 답장
	cUINode*	mpSend;

	// 목록
	cCheckBoxListBox*	mpListBox;

	/// 페이지 스핀
	cSpinBox*		mpPageSpin;

	// 이미지
	sImageData mpImageData[10];

	unsigned int mImageX1;
	unsigned int mImageHeight1;	// 아이템

	unsigned int mImageX2;
	unsigned int mImageHeight2; // 메일
};


//--------------------------------------------------------
// 읽기 페이지
class cLabel;
class cItemIcon;
class cTextBox;
struct MSG_RES_CHAT_POST_DOWNLOAD;
class cMailReadSheet : public cUIWindow
{
public:
	cMailReadSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cMailReadSheet();

	void Clear();

	//
	void SetData( POST_OPEN_RESULT* data );
	void DownLoadData( MSG_RES_CHAT_POST_DOWNLOAD* msg );

	// 서버에 요청
	void SendItemDownload( unsigned int iconid );
	void SendMoneyDownload();
	void SendDelete();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnIconHovered( cUINode* , unsigned int , const cUIPos& );
	void OnIconRightClicked( cUINode* , unsigned int , bool );
	void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );

	void SetTime( LPCTSTR time );
	void SetMoney( unsigned long money );
	void SetItem( sInventory& inven, unsigned short slot );

private:
	// 이름
	cLabel*	mpName;

	// 시간
	cLabel*	mpTime;

	// 제목
	cLabel* mpTitle;

	// 내용
	cTextBox* mpText;

	// 금액
	cLabel* mpMoney;

	// 돈 찾기 버튼
	cUINode* mpGetMoney;

	// 삭제 버튼
	cUINode* mpDelete;

	// 답장 버튼
	cUINode* mpSend;

	// 아이템 슬롯
	cItemIcon* mpItemSlot[3];

	// 우편 고유 인덱스
	unsigned long mIndex;

	// 임시 슬롯
	unsigned int mTempArrIndex;
};


//--------------------------------------------------------------------------
// 보내기 페이지
class cEditBox;
class cMultiEditBox;
class cMailSendSheet : public cUIWindow
{
public:
	cMailSendSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cMailSendSheet();

	void Clear();

	bool AddItem( unsigned short slot );
	void DeleteItem( unsigned int i );
	void DeleteItemBySend();			// 전송후 인벤토리 삭제

	void SetMoney( unsigned long money );
	void SetName( LPCTSTR name );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* node, unsigned int id );
	void OnIconDragged(cUINode* , unsigned int , const cUIPos& , bool );
	void OnIconRightClicked( cUINode* , unsigned int , bool );

	void UpdateItem( unsigned int index );
	void SetCharge( unsigned long charge );

private:
	// 이름
	cEditBox*	mpName;

	// 제목
	cEditBox*	mpTitle;

	// 내용
	cMultiEditBox*	mpText;

	// 금액
	unsigned long mMoney;
	cLabel*		mpMoney;

	// 수수료
	cLabel*		mpCharge;

	// 보내기
	cUINode*	mpSend;

	// 아이템 슬롯
	cItemIcon* mpItemSlot[3];
	BYTE mLastSlot;		// 마지막으로 채워진 슬롯 위치
};