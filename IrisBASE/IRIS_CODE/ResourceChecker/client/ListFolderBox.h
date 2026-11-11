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

// 	virtual void Clear();

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