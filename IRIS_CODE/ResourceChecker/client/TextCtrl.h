/* ==========================================================================
* 파    일 : TextCtrl.h
* 목    적 : 텍스트박스용 전용 자료 구조 클래스 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "PointerQueue.h"

/// 가로행 데이터
struct sTextData
{
	cStringT mText;
	unsigned long mColor;

	bool mEnter;	/// 엔터키 (현재 라인이 엔터라인인지 여부)
	bool mUseInven;

	sTextData()
	{
		mColor = 0;
		mUseInven = false;
		mEnter = false;
	}
};

struct sInvenData : sTextData
{
	sInventory	mInventory;
	sInvenData()
	{
		::memset( &mInventory, 0, sizeof(mInventory) );
	}
};


/// 세로행 데이터
struct sRowTextData
{
	/// sTextData
	tPointerArray<sTextData*>		mColumes;
};

class cTextCtrl : public cUIWindow
{
public:
	cTextCtrl( eUINodeType type = eUINODE_TEXTBOX );
	virtual ~cTextCtrl();

protected:
 	/// 데이터를 모두 지움 
 	void Clear();

protected:
	/// 문자열 리스트
	typedef tPointerQueue<void*> cTextQueue;
	cTextQueue	mTextQueue;

	/// 한줄에수용하는 문자 폭 
	unsigned int mMaxTextWidth;

	/// 리스트가 총 수용하는 라인 수
	unsigned int mTotalMaxNum;
};
