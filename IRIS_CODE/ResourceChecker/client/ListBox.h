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