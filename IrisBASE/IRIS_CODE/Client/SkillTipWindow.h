/* ==========================================================================
* 파    일 : SkillTipWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TipWindow.h"

const int MAX_RIGHT_TIP = 20;

struct sPlayerSkillBaseInfo;
struct sPlayerSkillStepInfo;

/// 아이템용 팁 윈도우
class cSkillTipWindow : public cTipWindow
{
public:
	cSkillTipWindow();
	virtual ~cSkillTipWindow();

	virtual void Open();
	virtual void Close();

	///
	bool SetSkill( eTip_Kind tipkind, const cUIPos& pos, unsigned long skillIndex, unsigned char skillStep, unsigned long jobIdx );

protected:
	void OnRender( cUIFontItemKeeper* fontKeeper );
	void Clear();
	void OnTimer();
	void OnHide();

	void CalcRightMaxWidth( const cStringT& str );

	////
	bool SetNormalTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx );
	bool SetQuickTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx );
	bool SetSkillPreviewTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx );

	///
	void SetName( sPlayerSkillBaseInfo* pInfo, unsigned long color );
	void SetShotType( sPlayerSkillBaseInfo* pInfo, unsigned long color );
	void SetEquip( sPlayerSkillBaseInfo* pInfo, unsigned long color );
	void SetCastingTime( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color );
	void SetCoolTime( sPlayerSkillStepInfo* pStepInfo, unsigned long color, unsigned long skillIdx );
	void SetCoolTimeShareList( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long jobIdx, unsigned long color );
	void SetUsedValue( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color );
	bool SetInfluence( unsigned long influenceIdx );
	void SetNoOverLapInfluenceList( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long jobIdx, unsigned long color);	/// 중첩 불가 스킬 목록
	bool SetActiveEffect( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color);			/// 발동 효과
	void SetRangeType( sPlayerSkillBaseInfo* pInfo, unsigned long color);												/// 범위 방식

	/// 
	void SetRightAttribute( sPlayerSkillBaseInfo* pInfo, unsigned long color ); 
	void SetRightStepInfo( sPlayerSkillBaseInfo* pInfo, unsigned int curStep, unsigned long color ); 
	void SetRightTargetDist( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color, unsigned long skillIdx ); 
	void SetRightBound( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color ); 
	void SetRightApplyCnt( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color );
	void SetRightInfluence( sPlayerSkillBaseInfo* pInfo,  sPlayerSkillStepInfo* pStepInfo, unsigned int step, unsigned long color );
	void SetRightActiveEffect( sPlayerSkillStepInfo* pStepInfo,  unsigned long color ); 	

	void AddRightGap( unsigned int count );

	virtual void SetBackImg( const cUIPos& pos );

private:
	/// 오른쪽 정렬로 붙어있는 추가 팁 
	cTipParam	mRightTip[MAX_RIGHT_TIP];

	unsigned int mRightMaxWidth;
	unsigned int mRightIndex;

	/// 상점 처럼 나누어서 찍는 루틴을 표현하기 위한 플래그
	unsigned int mBottomIndex;
	unsigned int mBottomRightIndex;

	cTipParam		mRequireValue[MAX_RIGHT_TIP][4];
	unsigned int	mRequireIndex;

	unsigned long	mSaveSkillIndex;

	unsigned int	mRightGapCount;
};
