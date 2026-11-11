/* ==========================================================================
* 파    일 : ListFolderBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-03-18
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ListCtrl.h"

const unsigned int IMAGE_GAP = 2;
const unsigned int LOW_GAP = 15;

struct sFolderData : sData
{
	long	key;

	/// 이미지 플래그
	bool mCheck;
	cUIImage*	mCheckImg;

	/// 추가 정보
	void* mExtraData; 

	sFolderData()
	{
		key = -1;
		mExtraData = 0;
	}
	~sFolderData(){}
};

/// 리스트 폴더 박스 ui 
class cScrollBar;
class cPlaneObject;
class cListFolderBox : public cListCtrl
{
public:
	cListFolderBox( eUINodeType type = eUINODE_LISTFOLDERBOX );
	virtual ~cListFolderBox();

	virtual void Clear();
	void InitSelectRow();

	/// 상위열 추가 ( key : 상위열의 인덱스 )
	int AddHighData( long key, LPCTSTR str, unsigned long color, void* pExtraData = 0, cUIImage* pImage = 0, bool check = true );

	/// 하위열 추가 ( key : 상위열의 인덱스 )
	bool AddLowData( long key, LPCTSTR str, unsigned long color, void* pExtraData = 0, cUIImage* pImage = 0, bool check = true );

	// 선택열 바꾸기
	void ChangeSelectColumn( unsigned int selectRow, bool sendEvent = false );
	
	/// extraData를 가진 정보 찾기
	sFolderData* GetFolderData( unsigned int row, unsigned int column );
	sFolderData* GetFolderDataByExtra( long key, void* extraData );
	
	/// extraData 리턴
	void* GetColumnData( unsigned int row, unsigned int column );

	/// 선택 이미지 세팅
	void SetHighCheckPos( unsigned int texX, unsigned int texY );
	void SetHighNoCheckPos( unsigned int texX, unsigned int texY );
	void SetLowCheckPos( unsigned int texX, unsigned int texY );
	void SetLowNoCheckPos( unsigned int texX, unsigned int texY );
	
	/// 실제로 시작된 열
	unsigned int GetSelectRow() { return mSelectedRow; }
	unsigned int GetSelectRowColumn() { return mSelectedRowColumn; }

	/// 하위열 출력 시작 x 위치 조정
	void SetLowGap( unsigned int gap ) { mLowGap = gap; }

	/// 선택 이미지 좌표 업데이트 
	virtual void UpdateSelectImage();

	/// 하위열 갯수 출력 여부
	void ShowNumber( bool show ) { mShowNumber = show; }

	/// 접혀져 있는 카테고리 키 값 가져오기
	void GetClosedCategory( tHashSet<unsigned int>& keySet );
	void SetClosedCategory( long key );

protected:
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnProcess(unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnScrollbarMoved( const cUIPos& pos );
	virtual void OnMouseWheel( const cUIPos& pos, int wheel );
	void CheckButtonEvent( unsigned int id );
	virtual void UpdateSkin();
	virtual void UpdateRect();

	/// 스크롤바의 상태 변경
	void UpdateScrollBar();

	/// 컬럼 총 갯수 갱신
	void UpdateColumnCount();

protected:
	/// 스크롤바 관련
	cString mScrollSkin;
	cScrollBar* mpScrollBar;

	/// 아이템 선택시 의 이미지 
	cPlaneObject* mpSelectImage;

	/// 표시 시작 행
	unsigned int	mTopColumnIdx;

	/// 선택 행 ( mTopColumnIdx 을 기준으로 한 화면상에 보이는 컬럼 )
	unsigned int	mSelectedColumnIdx;

	/// 실제 선택된 row. 와 column
	unsigned int	mSelectedRow;
	unsigned int	mSelectedRowColumn; /// row 내의 컬럼

	/// 행의 높이
	unsigned int	mRowHeight;

	/// 최대 행수
	unsigned int	mMaxRowInPage;

	/// 리스트가 총 수용하는 라인 수
	unsigned int	mTotalMaxNum;

	/// 체크, 노체크 텍스쳐 위치
	cUIPos			mHighCheckPos;
	cUIPos			mHighNoCheckPos;
	cUIPos			mLowCheckPos;
	cUIPos			mLowNoCheckPos;

	/// 총 열 갯수
	unsigned int mColumnCount;

	/// 렌더링을 위한 출력 위치 계산용
	unsigned int mRenderRow;
	unsigned int mRenderStartColumn;

	/// 하위열 출력 x 좌표 설정
	unsigned int mLowGap;

	/// 갯수 보여주기
	bool		mShowNumber;

	//
	bool		mShowSelect;
};

inline
void cListFolderBox::SetHighCheckPos( unsigned int texX, unsigned int texY )
{
	mHighCheckPos.mX = texX;
	mHighCheckPos.mY = texY;
}

inline
void cListFolderBox::SetHighNoCheckPos( unsigned int texX, unsigned int texY )
{
	mHighNoCheckPos.mX = texX;
	mHighNoCheckPos.mY = texY;
}

inline
void cListFolderBox::SetLowCheckPos( unsigned int texX, unsigned int texY )
{
	mLowCheckPos.mX = texX;
	mLowCheckPos.mY = texY;
}

inline
void cListFolderBox::SetLowNoCheckPos( unsigned int texX, unsigned int texY )
{
	mLowNoCheckPos.mX = texX;
	mLowNoCheckPos.mY = texY;
}

/////////////////////////////////////////////
/// 리스트 폴더 박스스킨
class cListFolderBoxSkin : public cUINodeSkin
{
public:
	cListFolderBoxSkin( eUINodeType type = eUINODE_LISTFOLDERBOX );
	virtual ~cListFolderBoxSkin();

	virtual bool Load( cParser& parser );

public:
	/// 리스트박스의 한줄 높이 
	unsigned int mRowHeight;

	/// 담을수 있는 최대 갯수
	int mTotalMaxNum;

public:
	cString		mScrollSkin;

	///
	NiTexture* mpSelectBarTexture;

	/// ui image 스킨 정보
	sSkinInfo* mSelectBarSkin;

	/// 선택바 폭 
	unsigned int mSelectBarWidth;
};