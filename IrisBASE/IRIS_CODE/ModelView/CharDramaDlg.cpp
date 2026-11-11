// CharDramaDlg.cpp : 구현 파일입니다.
//
#include "stdafx.h"
#include "CharDramaDlg.h"

#include "resource.h"

#include "CharacterForm.h"
#include "SkillScript.h"
#include "Item_Common.h"
#include "GameResourceManager.h"

#include "SkillManager.h"
#include "SceneManager.h"
#include "DynamicSceneNode.h"

#include "ModelView.h"


// cCharDramaDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cCharDramaDlg, CDialog)

BEGIN_MESSAGE_MAP(cCharDramaDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_CHAR_DRAMA1, OnCbnSelchangeCombo)
	ON_BN_CLICKED(IDC_DRAMA_REPLAY, OnBnClickedDramaReplay)
END_MESSAGE_MAP()

cCharDramaDlg::cCharDramaDlg(CWnd* /*pParent=NULL*/)
{
}

cCharDramaDlg::~cCharDramaDlg()
{
}

void cCharDramaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_CHAR_DRAMA1, mDramaCombo );
}


void cCharDramaDlg::OpenData()
{
	mDramaCombo.ResetContent();

	mDramaCombo.AddString( "---!" );

	cArrayHashMap* pMap = (cArrayHashMap*)SKILLSCRIPT->GetPlayerSkillInfoMap();

	cArrayHashMap::cIterator i = pMap->Begin();
	cArrayHashMap::cIterator iend = pMap->End();
	for( ; i != iend; ++i )
	{
		sPlayerSkillBaseInfo* pInfo = (sPlayerSkillBaseInfo*)(i->mSecond);
		if( pInfo->mSkillIdx >= NORMAL_ATTACK_SKILL_MAX )
		{
			switch( CHARACTERFORM->GetCurrentRace() )
			{
			case eRACE_HUMAN:
				if( pInfo->mRace != 0 && pInfo->mRace != 1 && pInfo->mRace != 4 && pInfo->mRace != 5 )
					continue;
				break;
			case eRACE_BEAST:
				if( pInfo->mRace != 0 && pInfo->mRace != 2 && pInfo->mRace != 4 && pInfo->mRace != 6 )
					continue;
				break;
			case eRACE_ELF:
				if( pInfo->mRace != 0 && pInfo->mRace != 3 && pInfo->mRace != 5 && pInfo->mRace != 6 )
					continue;
				break;
			default:
				continue;
			}
		}

		if( pInfo->mType == eSKILLTYPE_PASSIVE )
			continue;

		unsigned char weaponState = CHARACTERFORM->GetCurrentWeaponState();
		if( pInfo->mUseEquipment )
		{
			switch( pInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SWORD )
					continue;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					continue;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					continue;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					continue;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					continue;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					continue;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					continue;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					continue;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					continue;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					continue;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					continue;
				break;
			}
		}
		else
		{
			if( weaponState != eWEAPON_STATE_NONE )
				continue;
		}

		LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameIdx );
		char str[256]={0,};
		::sprintf_s(str, "%s(%.2d)", name, pInfo->mSkillIdx );
		
		int idx = mDramaCombo.AddString( str );
		mDramaCombo.SetItemDataPtr( idx, pInfo );
	}

	mDramaCombo.SetCurSel(0);
}

void cCharDramaDlg::OnCbnSelchangeCombo()
{
	int sel = mDramaCombo.GetCurSel();
	if( sel < 0 )
		return;

	SCENEMAN->SetDramaProcess( true );
	SCENEMAN->GetCurrentSceneNode()->ClearAllEffect();
	SKILLMAN->DeleteAll();

	CHARACTERFORM->UpdateAnimationList( CHARACTERFORM->GetCurrentRace(), CHARACTERFORM->GetCurrentGender() );

	if( sel == 0 )
		return;

	sPlayerSkillBaseInfo* seq = (sPlayerSkillBaseInfo*)mDramaCombo.GetItemDataPtr( sel );

	if( seq->mSkillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		unsigned int state = (unsigned int)CHARACTERFORM->GetCurrentWeaponState();
		if( state > eWEAPON_STATE_MAX )
			state -= eWEAPON_STATE_SHEILD;

		unsigned int idx = (seq->mSkillIdx/10)*10 + state + 1;
		SKILLMAN->CreateHeroSkillObject( idx );
	}
	else
		SKILLMAN->CreateHeroSkillObject( seq->mSkillIdx );
}

// cCharDramaDlg 메시지 처리기입니다.

void cCharDramaDlg::OnBnClickedDramaReplay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	///
	int sel = mDramaCombo.GetCurSel();
	if( sel < 0 )
		return;
	if( sel == 0 )
		return;

	SCENEMAN->SetDramaProcess( true );
	SKILLMAN->DeleteAll();

	cDynamicSceneNode* curNode = SCENEMAN->GetCurrentSceneNode();
	if( curNode )
	{
		curNode->ClearAllEffect();

		unsigned char state = CHARACTERFORM->GetCurrentWeaponState();
		curNode->UpdateTargetAnimation( state );
	}

	CHARACTERFORM->UpdateAnimationList( CHARACTERFORM->GetCurrentRace(), CHARACTERFORM->GetCurrentGender() );

	sPlayerSkillBaseInfo* seq = (sPlayerSkillBaseInfo*)mDramaCombo.GetItemDataPtr( sel );
	if( seq == 0 )
		return;

	unsigned int idx = seq->mSkillIdx;
	if( seq->mSkillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		unsigned int state = (unsigned int)CHARACTERFORM->GetCurrentWeaponState();
		if( state > eWEAPON_STATE_MAX )
			state -= eWEAPON_STATE_SHEILD;

		idx = (seq->mSkillIdx/10)*10 + state + 1;
	}

	if( SKILLMAN->CreateHeroSkillObject( idx ) == 0 )
		return;
}
