// MonDramaDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MonDramaDlg.h"

#include "resource.h"

#include "MonsterForm.h"
#include "GameResourceManager.h"
#include "SkillScript.h"
#include "SkillManager.h"
#include "SceneManager.h"
#include "DynamicSceneNode.h"


// cMonDramaDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cMonDramaDlg, CDialog)

BEGIN_MESSAGE_MAP(cMonDramaDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_MON_DRAMA1, OnCbnSelchangeCombo )
	ON_BN_CLICKED( IDC_DRAMA_MON_REPLAY, OnBnClickedDramaReplay )
END_MESSAGE_MAP()

cMonDramaDlg::cMonDramaDlg(CWnd* /*pParent=NULL*/)
{
}

cMonDramaDlg::~cMonDramaDlg()
{
}

void cMonDramaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_MON_DRAMA1, mDramaCombo );
}

void cMonDramaDlg::OpenData( unsigned long monsterIdx )
{
	mDramaCombo.ResetContent();
	mDramaCombo.AddString( "---!" );

	char str[256] = {0,};

	/// 기본 공격
	cArray* baseArr = (cArray*)SKILLSCRIPT->GetMonsterBaseSkillArr( monsterIdx );
	if( baseArr )
	{
		for( unsigned int i = 0; i<baseArr->GetSize(); i++ )
		{
			sMonsterSkillScript* pInfo = (sMonsterSkillScript*)((*baseArr)[i]);
			if( pInfo == 0 )
				continue;

			switch( pInfo->mAttackType )
			{
			case eMONSTERATTACK_NORMAL1:
				::sprintf_s(str, "%s", "Normal_1" );
				break;
			case eMONSTERATTACK_NORMAL2:
				::sprintf_s(str, "%s", "Normal_2" );
				break;
			case eMONSTERATTACK_SKILL1:
				::sprintf_s(str, "%s", "Skill_1" );
				break;
			case eMONSTERATTACK_SKILL2:
				::sprintf_s(str, "%s", "Skill_2" );
				break;
			case eMONSTERATTACK_SKILL3:
				::sprintf_s(str, "%s", "Skill_3" );
				break;
			default:
				continue;
			}

			int idx = mDramaCombo.AddString( str );
			mDramaCombo.SetItemDataPtr( idx, pInfo );
		}
	}

	/// 모드 스킬
	cArray* modeArr = (cArray*)SKILLSCRIPT->GetMonsterEliteSkillArr( monsterIdx );
	if( modeArr )
	{
		for( unsigned int i = 0; i<modeArr->GetSize(); i++ )
		{
			sMonsterSkillScript* pInfo = (sMonsterSkillScript*)((*modeArr)[i]);
			if( pInfo == 0 )
				continue;

			::sprintf_s(str, "%s%d", "ModeSkill_", pInfo->mAttackType );

			int idx = mDramaCombo.AddString( str );
			mDramaCombo.SetItemDataPtr( idx, pInfo );
		}
	}

/*
	for( unsigned int i=0; i<eMONSTERATTACK_MAX; i++ )
	{
		sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterIdx, (eMONSTERATTACK_TYPE)i );
		if( pInfo == 0 )
			continue;

		switch( i )
		{
		case eMONSTERATTACK_NORMAL1:
			::sprintf(str, "%s", "Normal_1" );
			break;
		case eMONSTERATTACK_NORMAL2:
			::sprintf(str, "%s", "Normal_2" );
			break;
		case eMONSTERATTACK_SKILL1:
			::sprintf(str, "%s", "Skill_1" );
			break;
		case eMONSTERATTACK_SKILL2:
			::sprintf(str, "%s", "Skill_2" );
			break;
		case eMONSTERATTACK_SKILL3:
			::sprintf(str, "%s", "Skill_3" );
			break;
		default:
			continue;
		}

		int idx = mDramaCombo.AddString( str );
		mDramaCombo.SetItemDataPtr( idx, pInfo );
	}
*/
	mDramaCombo.SetCurSel(0);
}
// cMonDramaDlg 메시지 처리기입니다.

void cMonDramaDlg::OnCbnSelchangeCombo()
{
	int sel = mDramaCombo.GetCurSel();
	if( sel < 0 )
		return;

	SCENEMAN->SetDramaProcess( true );
	SCENEMAN->GetCurrentSceneNode()->ClearAllEffect();
	SKILLMAN->DeleteAll();

	MONSTERFORM->UpdateAnimationList( MONSTERFORM->GetBaseModelIndex() );

	if( sel == 0 )
		return;

	sMonsterSkillScript* info = (sMonsterSkillScript*)mDramaCombo.GetItemDataPtr( sel );
	if( info )
		SKILLMAN->CreateHeroSkillObject( info->mAttackType );
}

void cMonDramaDlg::OnBnClickedDramaReplay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	///
	int sel = mDramaCombo.GetCurSel();
	if( sel < 0 )
		return;
	if( sel == 0 )
		return;

	SCENEMAN->SetDramaProcess( true );
	SCENEMAN->GetCurrentSceneNode()->ClearAllEffect();
	SKILLMAN->DeleteAll();

	MONSTERFORM->UpdateAnimationList( MONSTERFORM->GetBaseModelIndex() );

	sMonsterSkillScript* info = (sMonsterSkillScript*)mDramaCombo.GetItemDataPtr( sel );
	if( info )
		SKILLMAN->CreateHeroSkillObject( info->mAttackType );
}