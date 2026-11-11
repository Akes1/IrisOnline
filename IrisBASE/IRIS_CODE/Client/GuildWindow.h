/* ==========================================================================
* 파    일 : GuildWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-09-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cListBox;
class cLabel;
class cEditBox;
class cUIImage;
class cButton;
struct sGuildUserInfo;

class cGuildWindow : public cUIWindow
{
public:
	cGuildWindow();
	virtual ~cGuildWindow();

	virtual void Open();
	virtual void Close();

	void Clear();
	void ClearList();

	void Toggle();

	void HideMark();
	
	/// 길드 유저 추가
	void AddGuildPlayer( sGuildUserInfo* data );

	void SetGuildName( LPCTSTR name );
	void SetMasterName( LPCTSTR name );
	void SetTotalCount( unsigned int count );		/// 총인원 세팅
	void SetConnectCount( unsigned int count );
	void SetPosition( unsigned char position );		/// 직위 세팅

	/// 길드공지 갱신
	void UpdateGuildNotice( LPCTSTR notice );

	/// 
	void SetEnableMarkChange( bool enable );

	///
	cUIRect GetGivePositionBtnRect();

	/// 리스트 선택된 유저 정보
	sGuildUserInfo* GetSelectUserInfo();

	/// 선택 열
	unsigned int GetSelectRow();
	void SetSelectRow( unsigned int row );

	/// 길드마크 변경
	bool ChangeMark( NiTexture* tex );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* , unsigned int );
	void OnListBoxRowHovered( cUINode* , unsigned int , const cUIPos& , void* );
	void OnListBoxRowLeft(cUINode* , unsigned int );
	void UpdateRect();
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cListBox*		mpList;
	cEditBox*		mpNoticeEdit;

	cLabel*			mpName;				/// 길드명
	cLabel*			mpMasterName;		/// 마스터명	
	cLabel*			mpPosition;			/// 길드내 직위
	cLabel*			mpTotal;			/// 총 인원
	cLabel*			mpConnectCount;		/// 접속인원

	///
	cButton*		mpGuildAdd;			/// 길드원초대
	cButton*		mpGuildMemDrop;		/// 제명
	cButton*		mpGuildOut;			/// 탈퇴
	cButton*		mpGuildBreak;		/// 해체
	cButton*		mpGuildGivePosition; /// 직위 부여
	cUINode*		mpNotice;
	cButton*		mpMarkChange;

	/// 길드마크
	cUIImage*	mpMarkImage;
	bool		mShowMark;

	/// 마크 이미지 스킨
	cUINodeSkin* mpMarkSkin;
};

/// 길드 버튼 윈도우 (직위부여)
class cGuildButtonWindow : public cUIWindow
{
public:
	cGuildButtonWindow();
	virtual ~cGuildButtonWindow();

	virtual void Open() {}

	void SetPosition( unsigned char position );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnMouseOtherClick( const cUIPos& pos );
	void UpdateSkin();

private:
	/// 버튼
	cUINode*		mpMaster;
	cUINode*		mpSubMaster;
	cUINode*		mpElder;
	cUINode*		mpUpperHouse;
	cUINode*		mpLowerHouse;
	cUINode*		mpGeneral;
};

/// 길드 마크 등록 UI
class cGuildMarkWindow : public cUIWindow
{
public:
	cGuildMarkWindow();
	virtual ~cGuildMarkWindow();

	virtual void Open() {}

	/// 길드마크 변경
	bool ChangeMarkRegist( NiTexture* tex );

	void HideESC( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnShow();
	void OnHide();

private:
	cUIImage*	mpMarkImage;

	/// 마크 이미지 스킨
	cUINodeSkin* mpMarkSkin;
};