/* ==========================================================================
* 파    일 : DMScoreWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-09-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ListBox.h"

class cLabel;
class cUIImageFont;
class cGauge;

class cDMScoreWindow : public cUIWindow
{
public:
	cDMScoreWindow();
	virtual ~cDMScoreWindow();

	virtual void Open() {}
	void Clear();

	void UpdatePoint( unsigned int pointA, unsigned int pointB );

	void SetTime( long time );
	void SetTeamImage( bool fire, bool water, bool wind, bool earth );

	void ActiveTime();
	void DeActiveTime();
	
	void SetStoneImage( unsigned char pvpTeamType );
	void SetStoneMaxHP( unsigned int maxHP );
	void SetStoneHP( unsigned int hp );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();

private:
	/// 아군 포인트
	cString mTeamAPointStr;
	cString mTeamBPointStr;
	cString mGoalPointStr;

	/// 제한 시간
	long mPlayTime;
	long mStartTime;
	cString mTimeStr;

	/// 폰트
	cUIImageFont* mTeamAFont;
	cUIImageFont* mTeamBFont;
	cUIImageFont* mTimeFont;
	cUIImageFont* mGoalPointFont;
	
	/// 각 스킨
	cUIPos			mTimePos;
	cUIPos			mGoalPointPos;

	cUIRect			mTeamAPointSkinRect;
	cUIRect			mTeamBPointSkinRect;

	/// 각 팀 이미지 표시 위치
	cUIPos			mTeamAPointPos;
	cUIPos			mTeamBPointPos;

	/// 각 세력 이미지 표시 위치
	cLabel*			mTeamAMark[3];
	cLabel*			mTeamBMark[3];

	cUINodeSkin*	mpFireSkin;
	cUINodeSkin*	mpWaterSkin;
	cUINodeSkin*	mpWindSkin;
	cUINodeSkin*	mpEarthSkin;

	/// 진영별 석상 분류 표시 스킨
	cUINodeSkin*	mTeamAStoneMarkSkin;
	cUINodeSkin*	mTeamBStoneMarkSkin;

	/// 석상 HP 게이지 표시
	cGauge*			mStoneGauge;
	
	/// 진영별 석상 분류 표시 이미지
	cUIImage*		mStoneImage;
};


//////////////////////////////////////////////////////////////////////////
/// 랭킹
struct sMyTeamInfo;

class cDMRankingListWindow : public cUIWindow
{
public:
	cDMRankingListWindow();
	virtual ~cDMRankingListWindow();

	virtual void Open() {}

	void Clear();
	void AddPlayerRankInfo( sMyTeamInfo* info, bool leader );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnShow();
	void UpdateSkin();

protected:
	cListBox*	mpRankList;
};



//////////////////////////////////////////////////////////////////////////
/// 결과
const int MAX_ROW = 9;

struct sMyTeamInfo;
class cDMResultWindow : public cUIWindow
{
public:
	cDMResultWindow();
	virtual ~cDMResultWindow();

	virtual void Open() {}

	void Clear();
	void InsertRank( sMyTeamInfo* data, int ranki );
	void SetReward( unsigned long money, unsigned long exp, unsigned long sxp );
	void SetRewardForce( unsigned long point, unsigned long friendly );
	void SetResult( bool win );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnShow();
	void UpdateSkin();


private:
	/// 결과 리스트박스
	cListBox*	mpReultList;

	/// 포인트 점수 리스트박스
	cListBox*	mpPointList;

	///
	cLabel*		mpMoney;
	cLabel*		mpEXP;
	cLabel*		mpSXP;

	/// 결과
	cUINode*	mpWin;
	cUINode*	mpLose;
};
