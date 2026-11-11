/* ==========================================================================
* 파    일 : ListCtrl.h
* 목    적 : 리스트 박스 실제로 작동 하는 클래스
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-23
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cUIImage;

/// 내부 데이터
struct sData 
{
	cStringT mText;
	unsigned long mTextColor;

	sData()
	{
		mTextColor = 0;
	}
	virtual ~sData(){}
};

/// 세로행 
struct sRowData
{
	tPointerArray<sData*>		mColumes;

	/// 확장 데이터
	void*				mExtraData; 

	sRowData()
	{
		mExtraData = 0;
	}
};

class cListCtrl : public cUIWindow
{
public:
	cListCtrl( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cListCtrl();

 	/// 데이터를 모두 지움 
 	virtual void			Clear();


protected:
	/// 세로행 리스트 
	typedef tPointerArray<void*> cRowDataArray;
	cRowDataArray mRows;
};
