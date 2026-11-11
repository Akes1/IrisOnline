/* ==========================================================================
* 파    일 : SkillPreviewWindow.h
* 목    적 : 스킬 찍어보기 윈도우
* 작 성 자 : 김가람
* 작 성 일 : 2009-08-27
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"

const unsigned int MAX_JOBCOMBO = 2;

class cLabel;
class cComboBox;

struct sSkillData
{
	// 현재 스텝
	unsigned char step;

	// 자신을 선행스킬로 가지는 스킬 리스트
	typedef tHashSet<unsigned long> cHashSet;
	cHashSet skillSet;

	sSkillData()
	{
		step = UCHAR_MAX;
	}
};

struct sPreviewData
{
	/// 찍은 포인트 저장
	long jobPoint;
	
	// 최고 레벨
	char highLevel;

	sPreviewData()
	{
		jobPoint = 0;
		highLevel = 0;
	}
};

//
class cSkillPreviewWindow : public cTabWindow
{
public:
	cSkillPreviewWindow();
	virtual ~cSkillPreviewWindow();

	virtual void Open() {}
	virtual void Close();

	void Clear();

	virtual void Show( unsigned long jobindex1, unsigned long jobindex2 );

	// 스킬정보 삽입
	void InsertKeepSkill( unsigned long skillindex, unsigned char step );
	void UpdateSkillStep( unsigned long skillIndex, unsigned char step );

	// 해당 직업과 관련된 레벨, 포인트 계산
	void CheckInfo( unsigned long jobIndex );
	void UpdateInfoUI( unsigned long jobIndex );
	
	// 직업 관련 정보 삽입
	bool UpdatePreviewData( unsigned long jobIndex, long jobPoint, char highLevel );

	sSkillData* GetSkillData( unsigned long skillIndex );
	sPreviewData* GetPreviewData( unsigned long jobIndex );

	unsigned long GetCurrentJobIndex();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnComboSelected( cUINode* , unsigned int id );
	void OnHide();

	// 
	void UpdateJobCombo( int step );

	// hero의 스킬과 동기화
	void SyncHeroSkill();

	void ShowJobTab( unsigned int jobStep, unsigned long jobIndex );
	void HideJobTab( unsigned int jobStep );

	void SetLevel( char level );
	void SetUsePoint( long point );

private:
	// 각 직업별 스킬 관리자 ( sPreviewData )
	typedef tPointerHashMap<unsigned long, void*> cJobSkillMap;
	cJobSkillMap	mJobSkillMap;

	// 데이터 ( 직업인덱스 )
	typedef tList<unsigned long> cJobIndexList;
	cJobIndexList mJobIndexList;

	// 찍은 스킬 정보 관리자( 스킬인덱스, sSkillData )
	typedef tPointerHashMap<unsigned long, void*> cKeepSkillMap;
	cKeepSkillMap mKeepSkillMap;

	// 총 사용한 sp
	cLabel*	mpUsePoint;

	// 필요레벨
	cLabel*	mpLimitLevel;

	// 직업콤보
	cComboBox*	mpJobCombo[MAX_JOBCOMBO];

	// 스킬 동기화 버튼
	cUINode*	mpSyncSkill;
	
	// 기본 사이즈
	cUIRect mTabBaseRect;
};

