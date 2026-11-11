#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "Cheat_Common.h"
#include "Protocol_Cheat.h"

#include "UIManager.h"
#include "GameResourceManager.h"
#include "SkillScript.h"

#include "ObjectManager.h"
#include "WorldManager.h"
#include "Hero.h"
#include "ChatManager.h"

/// 스킬 관련
void cGMTool::OnShowSkill()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_SKILL];

	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_SKILL].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_SKILL].y = GetSystemMetrics(SM_CYCAPTION);

	/// 버튼 체크 상태 해제.
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_CHECK_SKILL_USE_AREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );
}

void cGMTool::OnCommandSkill( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_SKILL_USE_ONE:
		/// 지형 위치, 스킬이름 얻어와서 쏘기
		{
			if( msg == BN_CLICKED )
			{
				/// 타켓 오브젝트가 선택되어있는지 체크
				cBaseObject* pObj = HERO->GetTargetObject();
				if( pObj && ( pObj->GetObjectType() == eOBJECTTYPE_PLAYER || pObj->GetObjectType() == eOBJECTTYPE_HERO ) )
				{
					HWND skillNameCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_NAME );
					int i = SendMessage( skillNameCombo, CB_GETCURSEL, 0, 0 );
					int skillIndex = SendMessage( skillNameCombo, CB_GETITEMDATA, i, 0 );
					sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
					if( pSkillInfo )
					{
						sPlayerSkillStepInfo* pSkillStepInfo = &pSkillInfo->mpSetpInfoArray[pSkillInfo->mStepCount-1];
						if( pSkillStepInfo )
						{
							if( NETWORK->IsConnected() )
							{
								/// 플레이어 검사 
								cPlayer* pPlayer = (cPlayer*)pObj;
								/// 해당 유저 정보 서버로 보내기
								MSG_REQ_CHEAT_GM_SKILL_USE_PLAYER msg;
								msg.Category = NM_CHEAT;
								msg.Protocol = NM_CHEAT_GM_SKILL_USE_PLAYER_REQ;
								msg.mCharacterIdx = pPlayer->GetObjectID();
								msg.mInfluenceIndex = pSkillStepInfo->mInfulenceIdx;

								NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_SKILL_USE_PLAYER ) );
								return;
							}
						}
					}
				}
				else
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8081 ) );
				}
			}
		}
		break;
	case IDC_BUTTON_SKILL_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_SKILL], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_COMBO_SKILL_TYPE:
		if( msg == CBN_SELCHANGE )
		{
			EventSkillType();
		}
		break;
	case IDC_COMBO_SKILL_NAME:
		if( msg == CBN_SELCHANGE )
		{
			EventSkillName();	
		}
		break;
	}
}

bool cGMTool::LoadSkillData()
{

	HWND typeCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_TYPE );
	//HWND skillNameCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_NAME );

	/// 스킬 종류 셋팅
	for( int index = eGM_SKILL_TYPE_BUILD; index < eGM_SKILL_TYPE_MAX; index++ )
	{
		LPCTSTR typeName = UIMAN->GetUIText( index );
		int key = SendMessage( typeCombo, CB_ADDSTRING, 0, (LPARAM)(typeName));
		SendMessage( typeCombo, CB_SETITEMDATA, key, (LPARAM)index );
	}

	/// 스킬리스트 pc -> 스킬리스트 서브 -> 스킬리스트 이펙트
	typedef tPointerHashMap<unsigned long, void*> cSkillMap;
	cSkillMap* skillListMap = SKILLSCRIPT->GetPlayerSkillInfoMap();
	if( skillListMap && skillListMap->IsEmpty() == false )
	{
		cSkillMap::cConstIterator pos = skillListMap->Begin();
		cSkillMap::cConstIterator end = skillListMap->End();

		for( ; pos != end; ++pos )
		{
			//unsigned long skillIndex = (*pos).mFirst;
			sPlayerSkillBaseInfo* pSkillInfo = (sPlayerSkillBaseInfo*)(*pos).mSecond;
			if( pSkillInfo )
			{
				/// 스킬 대상이 아군, 파티원인경우만 추가.
				if( pSkillInfo->mApplyType == eAPPLYTYPE_BUDDY      || pSkillInfo->mApplyType == eAPPLYTYPE_PARTY || 
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_PARTY || pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_PARTY )
				{
					if( pSkillInfo->mType == eSKILLTYPE_TOTEM )
						continue;

					sPlayerSkillStepInfo* pSkillStepInfo = &pSkillInfo->mpSetpInfoArray[pSkillInfo->mStepCount-1];
					if( pSkillStepInfo && pSkillStepInfo->mInfulenceIdx != 0 )
					{
						sInfluenceScript* pInfluence = SKILLSCRIPT->GetInfluenceInfo( pSkillStepInfo->mInfulenceIdx );
						if( pInfluence )
						{
							/// 속성이 버프면서 분류가 강화 or 마법이면 추가
							if(  pInfluence->mType == eINFLUENCETYPE_BUF )
							{
								int gmSkillType = -1;

								///강화
								if( pInfluence->mTypeDetail == eINFLUENCETYPEDETAIL_BUILD )
									gmSkillType = eGM_SKILL_TYPE_BUILD;
								/// 마법
								else if( pInfluence->mTypeDetail == eINFLUENCETYPEDETAIL_MAGIC )
									gmSkillType = eGM_SKILL_TYPE_MAGIC;
								

								if( gmSkillType != -1 )
								{
									tSkillTypeMap::cConstIterator find = mSkillTypeIndexMap.Find( gmSkillType );

									if( find == mSkillTypeIndexMap.End() )
									{
										tSkillSubArr* skillArr = new tSkillSubArr;
										if( skillArr )
											skillArr->PushBack( (*pos).mFirst );
										mSkillTypeIndexMap.Insert( gmSkillType, skillArr );
									}
									else
									{
										tSkillSubArr* skillArr = (*find).mSecond;
										if( skillArr )
											skillArr->PushBack( (*pos).mFirst );
									}
								}
							}
						}
					}
				}
			}
		}
	}

	SendMessage( typeCombo, CB_SETCURSEL, 0, 0 );
	
	EventSkillType();

	return true;
}

void cGMTool::EventSkillType()
{
	/// 스킬 이름 셋팅하기
	HWND typeCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_TYPE );
	int i = SendMessage( typeCombo, CB_GETCURSEL, 0, 0 );
	int typeIndex = SendMessage( typeCombo, CB_GETITEMDATA, i, 0 );

	HWND skillNameCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_NAME );
	SendMessage( skillNameCombo, CB_RESETCONTENT, 0, 0 );

	tSkillTypeMap::cConstIterator pos = mSkillTypeIndexMap.Find( typeIndex );
	if( pos != mSkillTypeIndexMap.End() )
	{
		tSkillSubArr* skillArr = (*pos).mSecond;
		if( skillArr )
		{
			tSkillSubArr::cConstIterator pos2 = skillArr->Begin();
			tSkillSubArr::cConstIterator end2 = skillArr->End();

			for( ; pos2 != end2; ++pos2 )
			{
				LPCTSTR typeName = GAMERESOURCEMAN->GetSkillNameText( (*pos2) );
				int key = SendMessage( skillNameCombo, CB_ADDSTRING, 0, (LPARAM)(typeName));
				SendMessage( skillNameCombo, CB_SETITEMDATA, key, (LPARAM)(*pos2) );
			}
			SendMessage( skillNameCombo, CB_SETCURSEL, 0, 0 );
		}
	}

	EventSkillName();
}

void cGMTool::EventSkillName()
{
	/// 스킬 이름 셋팅하기
	SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_EDIT_SKILL_TIP, "" );
	HWND skillNameCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_NAME );
	int i = SendMessage( skillNameCombo, CB_GETCURSEL, 0, 0 );
	int skillIndex = SendMessage( skillNameCombo, CB_GETITEMDATA, i, 0 );

	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
	if( pSkillInfo )
	{
		sPlayerSkillStepInfo* pSkillStepInfo = &pSkillInfo->mpSetpInfoArray[pSkillInfo->mStepCount-1];
		if( pSkillStepInfo )
		{
			/// 설명
			cStringT temp;
			sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( pSkillStepInfo->mInfulenceIdx );
			if( pInfo )
			{

				/// 효과값이 있으면 효과출력
				if( pInfo->mToolTipIdx!= 0 && ( pInfo->mValue1 != 0 || pInfo->mValue2 != 0 || pInfo->mValue3 != 0 ) )
				{
					if( pInfo->mValue1 && pInfo->mValue2 && pInfo->mValue3 )
					{
						temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2, pInfo->mValue3 );
					}
					else if( pInfo->mValue1 && pInfo->mValue2 )
					{
						temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2 );
					}
					else if( pInfo->mValue1 )
					{
						temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1 );
					}
				}
				else if( pInfo->mToolTipIdx!= 0 )
				{
					temp = GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx );
				}
			}

			temp.Replace( _T("\\n"), _T("\r\n") );
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_EDIT_SKILL_TIP, temp.Cstr() );
		}
	}

	/// 버튼 체크 상태 해제.
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_CHECK_SKILL_USE_AREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );
}

bool cGMTool::ProcessGMSkill( int mouseX, int mouseY )
{
	/// 스킬이 사용중인지 체크
	if( SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_CHECK_SKILL_USE_AREA, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
	{
		/// 버튼 상태 체크 초기화
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_CHECK_SKILL_USE_AREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );

		/// 지형 좌표값 얻어오기
		NiPoint3 pos;
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
			return false;

		if( pos.x < 0 || pos.y < 0 )
		{
			assert(0);
			return false;
		}

		HWND skillNameCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_SKILL], IDC_COMBO_SKILL_NAME );
		int i = SendMessage( skillNameCombo, CB_GETCURSEL, 0, 0 );
		int skillIndex = SendMessage( skillNameCombo, CB_GETITEMDATA, i, 0 );
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
		if( pSkillInfo )
		{
			sPlayerSkillStepInfo* pSkillStepInfo = &pSkillInfo->mpSetpInfoArray[pSkillInfo->mStepCount-1];
			if( pSkillStepInfo )
			{
				if( NETWORK->IsConnected() )
				{
					///좌표값 서버로 보내기
					MSG_REQ_CHEAT_GM_SKILL_USE_AREA msg;
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GM_SKILL_USE_AREA_REQ;
					msg.mPosX     = pos.x;
					msg.mPosY     = pos.y;
					msg.mInfluenceIndex = pSkillStepInfo->mInfulenceIdx;
					
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_SKILL_USE_AREA ) );
					return true;
				}
			}
		}
		return false;
	}
	return false;
	
}
