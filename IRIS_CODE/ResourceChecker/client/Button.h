/* ==========================================================================
* 파    일 : Button.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006.09.13
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

/// 처음 버튼 눌림을 체크하는 시간
const unsigned long gOneMoveTime = 500;
/// 연속 버튼 눌림을 체크하는 시간
const unsigned long gContinueMoveTime = 80;

/// 버튼
class cButton : public cUINode
{
public:
	cButton( eUINodeType type = eUINODE_BUTTON );
	virtual ~cButton();

protected:
	/// 텍스트
	cStringT mText;
	/// 버튼의 이미지 변환좌표 저장 
	/// 업 상태 이미지 
	cUIPos mUpPos;
	/// 업 상태 오버 이미지 
	cUIPos mUpOverPos;
	/// 다운 상태 이미지 
	cUIPos mDownPos;
	/// 다운 상태 오버 이미지 
	cUIPos mDownOverPos;
	/// 비활성화 이미지 
	cUIPos mEnablePos;

	/// 각 버튼상태에 따른 텍스트 컬러 
	unsigned long mUpColor;
	unsigned long mDownColor;
	unsigned long mOverColor;

	/// 비활성화 이미지 사용 여부 
	bool mUseEnable;

	/// 텍스처 너비
	unsigned int mOriginalTexWidth, mOriginalTexHeight;
	/// 텍스트 출력위치 
	int mTextPosX, mTextPosY;

	/// 버튼 시간 플래그 
	bool mTimeFlag;
	/// 버튼 시간용
	unsigned long mLastTick, mUpdateTime;

	/// press 이벤트를 계속보낼건지 여부 설정
	bool mContinuEvent;

	unsigned long mDownSoundIdx;
	unsigned long mUpSoundIdx;
};

////////////////////////////////////////////////////////////////////////////////////////
/// 버튼 스킨
class cButtonSkin : public cUINodeSkin
{
public:
	cButtonSkin( eUINodeType type = eUINODE_BUTTON );
	virtual ~cButtonSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트
	cStringT mText;
	/// 버튼의 이미지 변환좌표 저장 
	cUIPos mUpPos;
	cUIPos mUpOverPos;
	cUIPos mDownPos;
	cUIPos mDownOverPos;

	/// 버튼의 사이즈 변환시 사용 
	cUISize mUpSize;
	cUISize mDownSize;

	/// 버튼의 비활성화 이미지 
	cUIPos mEnablePos;

	/// 비활성화 이미지 사용 여부 
	bool mUseEnable;

	/// 각 버튼상태에 따른 텍스트 컬러 
	unsigned long mUpColor;
	unsigned long mDownColor;

	unsigned long mDownSoundIdx;
	unsigned long mUpSoundIdx;
};

