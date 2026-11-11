/* ==========================================================================
* 파    일 : TipWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

enum eTip_Kind
{
	eTIP_ITEM,
	eTIP_SKILL,
	eTIP_QUICK,
	eTIP_SKILLPREVIEW,
};

class cTipParam
{
public:
	/// 텍스트
	cStringT mString;

	/// 텍스트 컬러
	unsigned long mColor;

	unsigned int mGapCount;

	cTipParam() : mGapCount(0), mColor(0) {}
};

enum
{
	eTIMELINE_COOLTIME = 0,	// 쿨타임
	eTIMELINE_LIMITDATE,	// 시간제

	eTIMELINE_MAX,
};

struct sTimeData
{
	unsigned int timeTipIdx;
	cStringT	 saveCoolTime;

	sTimeData()
	{
		timeTipIdx = 0;
	}
};

const unsigned int GAP_SIZE = 6;
const unsigned int LINEGAP_SIZE = 3;
const unsigned int MAX_TIP = 50;

class cPlaneObject;
class cTipWindow : public cUIWindow
{
public:
	cTipWindow();
	virtual ~cTipWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnRender( cUIFontItemKeeper* fontKeeper ) = 0;
	bool SetSkin( const cUINodeSkin* skin );
	void OnHide();
	virtual void Clear();

	/// 텍스트 길이검사 ( 여러줄 )
	bool CheckText( LPCTSTR str, unsigned long color );

	/// 텍스트 세팅 
	void AppendText( LPCTSTR str1, LPCTSTR str2, unsigned long color );
	void SetText( LPCTSTR str1, LPCTSTR str2, unsigned long color );
	void SetText( LPCTSTR str, unsigned long color );
	void SetText( const LPCTSTR str, unsigned long value, unsigned long color );
	void SetText( const LPCTSTR str, float value, unsigned long color );
	void SetText( const LPCTSTR str, short value, unsigned long color );

	/// 스트링 너비 계산 
	void CalcMaxWidth( const cStringT& str );

	void AddGap( unsigned int count );

	void UpdateEdge();

protected:
	enum
	{
		eBACKIMAGE_TOP = 0,
		eBACKIMAGE_BOTTOM,
		eBACKIMAGE_LEFT,
		eBACKIMAGE_RIGHT,

		eBACKIMAGE_MAX
	};

	/// 배경
	unsigned int mX;
	unsigned int mY;
	unsigned int mWidth;
	unsigned int mHeight;

	unsigned int mGapCount;

	/// 투명 배경 이미지
	cPlaneObject* mMiddleBack;

	/// 툴팁 데이터 
	cTipParam mToolTip[MAX_TIP];

	/// 데이터 인덱스 
	unsigned int mIndex;

	/// 스트링 너비
	unsigned int mMaxWidth;

	/// 한계 너비
	unsigned int mLimitMinWidth;
	unsigned int mLimitMaxWidth;

	/// 라인 높이 
	unsigned int mLineHeight;

	/// 배경 이미지
	cPlaneObject*	mpBackImage[eBACKIMAGE_MAX];

	/// 재사용시간, 사용대기시간 변경시 저장용
	sTimeData	mTimeLine[eTIMELINE_MAX];
	
	bool		mUpdateSizebyTime;	/// 재사용시간 출력시 바탕이미지 재계산
};
