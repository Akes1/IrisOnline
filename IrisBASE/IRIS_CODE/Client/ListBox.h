/* ==========================================================================
* 파    일 : ListBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ListCtrl.h"
#include "UINodeData.h"

class cScrollBar;
class cPlaneObject;
class cListBox : public cListCtrl
{
public:
	cListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cListBox();

	/// 클리어
	virtual void Clear();

	virtual int AddRow( LPCTSTR str, unsigned long color, void* pExtraData = 0 );		/// 새로운 행을 생성하고 하나의 열을 추가한다.
	virtual bool AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color );	/// 특정 라인에 열을 추가한다.

	///
	void SetExtraData( unsigned int rowIdx, void* pExtraData );

	/// 리스트의 선택인덱스 변경 
	bool ChangeSelectRow( unsigned int rowIdx, bool changeTop = true );

	/// 리스트의 출력 초기 인덱스 변경 
	bool ChangeTopIndex( unsigned int topIdx, bool changeSel = true );

	void ChangeTextColor( unsigned int rowIdx, unsigned long color );

	/// 스크롤바 스킨이름 세팅 
	void SetScrollSkin( const cString& name );

	inline void SetHoverTextColor( unsigned long color ) { mHoverTextColor = color; }

	/// 이미 선택되어잇는 열을 눌러도 이벤트 보내줌
	void SetContinueEvent( bool set ) { mContinueEvent = set; }

	/// 선택된 인덱스 받아오기 ( 아이템의 )
	unsigned int GetSelectIndex() const;
	unsigned int GetCurrentTopIndex() const;

	unsigned int GetRowHeight() { return mRowHeight; }

	unsigned int GetMaxRowNum() const; 
	unsigned int GetMaxRowInPage() { return mMaxRowInPage; }

	LPCTSTR GetText( unsigned int rowIdx = 0, unsigned int colIdx = 0 );

	// 스크립트 있는 해당 컬럼 최대 사이즈
	unsigned int GetColumnMaxWidth( unsigned int colIndex );

protected:
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	virtual void OnMouseMove( const cUIPos& pos );
	virtual	void OnMouseLeft( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnMouseWheel( const cUIPos& pos, int wheel );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnScrollbarMoved( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateSkin();
	virtual void UpdateRect();

	/// 스크롤바의 상태 변경
	virtual void UpdateScrollBar();

	/// 선택 이미지 좌표 업데이트 
	virtual void UpdateSelectImage();

	/// 버튼 상태 체크 
	virtual void CheckButtonEvent( unsigned int id );

	/// 리스트 박스의 아이템을 그려줌.
	virtual void RenderData( cUIFontItemKeeper* pKeeper );

	/// 텍스트 출력위치 계산
	unsigned int UpdateTextPos( eText_Align align, unsigned int strWidth, unsigned int x, unsigned int w );

	virtual void CheckShowImage();

	/// 라인의 특정 열정보를 가져온다.
	sData*			GetColumeData( unsigned int rowIdx, unsigned int columeIdx );


protected:
	/// 리스트박스용 스킨 저장 
	cUINodeSkin*	mpSkin;

	cString		mScrollSkin;

	/// 스크롤바
	cScrollBar* mpScrollBar;

	/// 아이템 선택시 의 이미지 
	cPlaneObject* mpSelectImage;

	/// 선택바 폭
	unsigned int mSelectBarWidth;

	/// 이미지 텍스쳐 렉트
	cUIRect		mImageRc;
	bool		mShowImage;

	/// 표시 시작 행
	unsigned int	mTopRowIdx;

	/// 선택 행
	unsigned int	mSelectedRowIdx;

	/// 아무것도 없을때 -1
	/// -1 > mHoveredRowIdx,  mHoveredRowIdx< mMaxRowInPage
	int				mHoveredRowIdx;
	int				mImageHoveredIdx;

	unsigned long	mHoverTextColor;

	/// 행의 높이
	unsigned int	mRowHeight;

	/// 최대 행수
	unsigned int	mMaxRowInPage;

	/// 리스트가 총 수용하는 라인 수
	unsigned int	mTotalMaxNum;

	/// 동일한 열을선택해도 이벤트 보내주는 플래그
	bool			mContinueEvent;

	/// 리스트 텍스트 출력 공간설정
	cUIPos mPrintPos;
	cUISize mPrintSize;
};

inline unsigned int cListBox::GetSelectIndex() const
{
	 return mSelectedRowIdx;
}

inline unsigned int cListBox::GetCurrentTopIndex() const
{
	return mTopRowIdx;
}

inline unsigned int cListBox::GetMaxRowNum() const
{
	return mTotalMaxNum;
}

inline void cListBox::SetScrollSkin( const cString& name )
{
	mScrollSkin = name;
}

//////////////////////////////////////////////////////////////////////////////
/// 리스트컨트롤 인자 
struct sListCtrlParam
{
	/// 컬럼 시작 위치 
	unsigned int mColumnX;

	/// 컬럼 너비
	unsigned int mColumnW;

	/// 컬럼 내 정렬 타입
	eText_Align mAlign;
};

///
class cListBoxSkin : public cUINodeSkin
{
public:
	cListBoxSkin( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cListBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

	///
	sListCtrlParam* GetCtrlParam( unsigned int index );

public:
	/// 리스트박스의 한줄 높이 
	unsigned int mRowHeight;

	/// 텍스트박스가 담을수 있는 최대 갯수
	int mTotalMaxNum;

public:
	/// 리스트 컨트롤 데이터 배열 
	typedef tPointerArray<void*> cLCtrlParam;
	cLCtrlParam mCtrlParamArr;

	cString		mScrollSkin;

	/// 이미지 삽입 위치 데이터 
	sListCtrlParam* mpImgParam;

	///
	NiTexture* mpSelectBarTexture;

	/// ui image 스킨 정보
	sSkinInfo* mSelectBarSkin;

	/// 선택바 폭 
	unsigned int mSelectBarWidth;

	/// 리스트 텍스트 출력 공간설정
	cUIPos mPrintPos;
	cUISize mPrintSize;
};