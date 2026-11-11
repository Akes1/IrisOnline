#include "StdAfx.h"
#include "Skillsheetwindow.h"

#include "SkillScript.h"
#include "CommunityScript.h"
#include "ResourceManager.h"
#include "SkillExecuter.h"
#include "SkillWindow.h"
#include "TipWindow.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "PartyManager.h"
#include "ItemManager.h"
#include "SkillManager.h"
#include "ObjectManager.h"
#include "TarotManager.h"
#include "Hero.h"
#include "UserSellWindow.h"
#include "SkillIcon.h"
#include "GameUIManager.h"
#include "ItemMixManager.h"
#include "SoundSystem.h"
#include "CamaelWindow.h"

#ifdef _GMTOOL
#include "CheatManager.h"
#endif

cSkillSheetWindow::cSkillSheetWindow( eUINodeType type )
: cUIWindow( type )
, mpTreeInfo(0)
, mBackTex(0)
, mJobIdx(0)
{
}

cSkillSheetWindow::~cSkillSheetWindow()
{
	mManagedIcon.Clear();
	mLevelupButtonMap.Clear();

	mJobIdx = 0;
}

void cSkillSheetWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mBackTex )
	{
		if( mpImage == 0 )
			mpImage = new cUIImage( mBackTex );
		else
			mpImage->SetTexture( mBackTex );

		mpImage->SetTextureRect( 0, 0, mBackTex->GetWidth(), mBackTex->GetHeight() );
		mpImage->SetScreenRect( mAbsoluteRect );
	}

	UpdateTree();
}

void cSkillSheetWindow::InitSkillSheetInfo( unsigned long jobIdx, cSkillTree* pTree )
{
	mJobIdx = jobIdx;

	assert( pTree );

	mpTreeInfo = pTree;

	cString pathName;
	pathName.Format( "./Data/2DData/%s", pTree->mImageName.Cstr() );
	mBackTex = RESOURCEMAN->LoadTexture( pathName, false );
	if( mpImage == 0 )
		mpImage = new cUIImage( mBackTex );
	else
		mpImage->SetTexture( mBackTex );

	mpImage->SetTextureRect( 0, 0, mBackTex->GetWidth(), mBackTex->GetHeight() );
	mpImage->SetScreenRect( mAbsoluteRect );

	for( unsigned int i=0; i<pTree->mIconCount; ++i )
	{
		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( pTree->mIconList[i].skillIdx );
		if( info )
		{
			cIconParam* param = UIMAN->GetIconParam( info->mIconIdx );
			if( param )
			{
				cSkillIcon* pIcon = new cSkillIcon;
				if( pIcon->CreateBySkinName( "SkillIcon", this, i + 1 ) == false )
				{
					assert(0);
					continue;
				}
				pIcon->SetRelativePos( cUIPos(pTree->mIconList[i].uiPosX, pTree->mIconList[i].uiPosY) );
				pIcon->ChangeImage( param );

				/// 스킬 관련 정보 셋팅
				pIcon->SetSkillInfo( pTree->mIconList[i].skillIdx, UCHAR_MAX, mJobIdx, eTIP_SKILL );

				if( pIcon->IsBaseSkill() )
				{
					pIcon->SetUsedFlag( true );
					pIcon->SetColor( NiColorA::WHITE );
				}
				else
				{
					/// 보유 스킬 여부 체크
					if( SKILLMAN->GetKeepInfo( pTree->mIconList[i].skillIdx ) )
					{
						pIcon->SetUsedFlag( true );
						pIcon->SetColor( NiColorA::WHITE );
					}
					else
					{
						pIcon->SetUsedFlag( false );
					}
				}

				if( mManagedIcon.Insert( pTree->mIconList[i].skillIdx ,pIcon ) == false )
				{
					assert(0);
				}

				/// 스킬 래벌업 버튼 [4/9/2009 Jo_2]
				
				/// 기본 제공 스킬이면서 단계가 1인 스킬은 버튼 생성을 제외한다.
				if( info->mStepCount == 1 && info->mpSetpInfoArray[0].mPlayerLevel == 0 )
					continue;
				
				cLevelButton *pLevelupButton = new cLevelButton;
				if( pLevelupButton->CreateBySkinName( "SkillLevelUpButton", this, pTree->mIconList[i].skillIdx ) == false )
				{
					assert(0);
					continue;
				}
				
				cUIRect iconRect = pIcon->GetRelativeRect();
				cUIRect btnRect  = pLevelupButton->GetRelativeRect();

				pLevelupButton->SetRelativePos( cUIPos( iconRect.mRight - btnRect.GetWidth() / 2, iconRect.mTop - btnRect.GetHeight() /2 ) );

				if( mLevelupButtonMap.Insert( pTree->mIconList[i].skillIdx, pLevelupButton  ) == false )
				{
					assert(0);
				}

				/// 모든 버튼은 생성과 동시에 숨긴다
				pLevelupButton->Hide();
				pLevelupButton->SetEnabled( false );
			}
		}
		else
		{
			assert(0);
			return;
		}
	}
	///
}

void cSkillSheetWindow::InitSkillData()
{
	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator e = mManagedIcon.End();

	for( ; i != e ; ++i )
	{
		unsigned long skillIdx = (*i).mFirst;
		cSkillIcon* pIcon = (cSkillIcon*)(*i).mSecond;
		if( pIcon == NULL )
			continue;

		/// 스킬 관련 정보 초기화
		pIcon->SetSkillInfo( skillIdx, UCHAR_MAX, mJobIdx, eTIP_SKILL );
		pIcon->SetUsedFlag( false );
		pIcon->SetAniParam( 0 );
		pIcon->StopDelay();
	}

	//
	ShowSkillUpButton();
}

void cSkillSheetWindow::OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& /*pos*/, bool shift )
{
	cSkillIcon* icon = (cSkillIcon*)pcaller;
	if( icon->IsActiveSkill() )
		SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, pcaller, id, shift );
}

void cSkillSheetWindow::OnIconDoubleClicked( cUINode* pcaller, unsigned int /*id*/ )
{
	cSkillIcon* p = (cSkillIcon*)pcaller;
	unsigned long skillIdx = p->GetSkillIdx();
	unsigned char skillStep = p->GetSkillStep();

	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );
	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		if( HERO->GetState() != eOBJECT_STATE_ATTACK )
			SKILLEXECUTER->ExecuteAttack( HERO->GetTargetObject() );
	}
	else
	{
		if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == true )
			SKILLEXECUTER->ExecuteSkill( skillIdx, skillStep );
	}
}

void cSkillSheetWindow::OnLButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}

	/// 상대영역을 절대영역으로 바꾼다 
	cUIRect captionRc = GetAbsoluteRect();
	captionRc.mLeft = GetAbsoluteRect().mLeft + mCaptionRect.mLeft;
	captionRc.mRight = captionRc.mLeft + mCaptionRect.GetWidth();
	captionRc.mTop = GetAbsoluteRect().mTop + mCaptionRect.mTop;
	captionRc.mBottom = captionRc.mTop + mCaptionRect.GetHeight();

	/// 캡션영역 안에 포인터가 있어야먄 드래그한다 
	if( mpParent == (cUIContainer*)GetRoot() && captionRc.ContainPoint( pos ) )
	{
		mDragStart.mX = pos.mX;
		mDragStart.mY = pos.mY;

		mDragging = true;
	}
}


void cSkillSheetWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	cSkillWindow* win = (cSkillWindow*)mpParent;
	if( !win )
	{
		assert(0);
		return;
	}

	if( win->IsVisible() == false )
		return;

	/// id 를 skillidx와 동일하게 설정하였음.
	//cButton* pSkillLevelupButton = (cButton*)pcaller;
	unsigned long skillIdx = id;

	sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( info == 0 )
	{
		assert(0);
		return;
	}

	/// 스킬 이름
	LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( info->mNameIdx );

	/// 스킬 래밸
	unsigned char skillStep = 0;
	sKeepSkill* p = SKILLMAN->GetKeepInfo( skillIdx );
	if( p )
	{
		skillStep = p->mStep + 1;
	}

	sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[skillStep];

	/// 소모 sp
	unsigned int requireSP = stepInfo.mRequireSP;

	cStringT form;
	form = GAMERESOURCEMAN->GetGameText( 6216 );

	cStringT str;
	str.Format( form.Cstr(), skillName, skillStep+1, requireSP );

	mExtraData.skillIdx = skillIdx;
	mExtraData.skillStep = skillStep;

	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_SKILL_BUY, eSKIN_YESNO,
		str.Cstr(),	// 
		GAMERESOURCEMAN->GetGameText( 2 ),	// 알림
		&mExtraData	);
}

void cSkillSheetWindow::UpdateTree()
{
	if( mpTreeInfo == 0 )
		return;

	for( unsigned int i=0; i<mpTreeInfo->mIconCount; ++i )
	{
		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( mpTreeInfo->mIconList[i].skillIdx );
		if( info == 0 )
		{
			assert(0);
			return;
		}

		cIconParam* param = UIMAN->GetIconParam( info->mIconIdx );
		if( param == 0 ) continue;

		cSkillIcon* p = (cSkillIcon*)mManagedIcon.GetAt( mpTreeInfo->mIconList[i].skillIdx );
		if( p == 0 ) continue;

		p->SetRelativePos( cUIPos(mpTreeInfo->mIconList[i].uiPosX, mpTreeInfo->mIconList[i].uiPosY) );
		p->ChangeImage( param );

		cLevelButton* pLevelupButton;
		pLevelupButton = (cLevelButton*)mLevelupButtonMap.GetAt( mpTreeInfo->mIconList[i].skillIdx );
		if ( pLevelupButton )
		{
			cUIRect iconRect = p->GetRelativeRect();
			cUIRect btnRect  = pLevelupButton->GetRelativeRect();

			pLevelupButton->SetRelativePos( cUIPos( iconRect.mRight - btnRect.GetWidth() / 2, iconRect.mTop - btnRect.GetHeight() /2 ) );
		}

		if( p->IsBaseSkill() )
		{
			p->SetUsedFlag( true );
			p->SetColor( NiColorA::WHITE );
		}
		else
		{
			/// 보유 스킬 여부 체크
			if( SKILLMAN->GetKeepInfo( mpTreeInfo->mIconList[i].skillIdx ) )
			{
				p->SetUsedFlag( true );
				p->SetColor( NiColorA::WHITE );
			}
			else
			{
				p->SetUsedFlag( false );
			}
		}
	}
}

void cSkillSheetWindow::Clear()
{
	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator iend = mManagedIcon.End();

	for( ; i != iend; ++i )
	{
		cSkillIcon* icon = (cSkillIcon*)(*i).mSecond;
		if( icon == 0 ) continue;

		RemoveChildComplete( icon );
	}
	mManagedIcon.Clear();


	cLevelupButtonMap::cIterator i2 = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator iend2 = mLevelupButtonMap.End();

	for( ; i2 != iend2; ++i2 )
	{
		cLevelButton* pBtn = (cLevelButton*)(*i2).mSecond;
		if( pBtn == 0 ) continue;

		RemoveChildComplete( pBtn );
	}
	mLevelupButtonMap.Clear();

	mJobIdx = 0;
}

void cSkillSheetWindow::HideSkillUpButton()
{
	cLevelupButtonMap::cIterator iBegin = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator iEnd = mLevelupButtonMap.End();

	for( ; iBegin != iEnd ; ++iBegin )
	{
		cLevelButton*  pLevelupButton = (cLevelButton*)(*iBegin).mSecond;
		if( pLevelupButton ) 
		{
			pLevelupButton->Hide();
			pLevelupButton->SetEnabled( false );
		}
	}
}

void cSkillSheetWindow::ShowSkillUpButton()
{
	if( HERO->GetSkillPointRemain() == 0 )
	{
		// 전부 HIDE
		HideSkillUpButton();
		return;
	}

	cLevelupButtonMap::cIterator i = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator end = mLevelupButtonMap.End(); 
	for( ; i != end; ++i )
	{
		unsigned long	skillIdx = (unsigned long)(i->mFirst);
		cLevelButton*	button   = (cLevelButton*)(i->mSecond);
		//bool		  show	   = false;

		if( button )
		{
			button->SetEnabled( false );
			button->Hide();
			
			sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
			if( pInfo == 0 )
			{
				assert(0);
				continue;
			}

			unsigned int nextStep = 0;
			sKeepSkill* p = SKILLMAN->GetKeepInfo( skillIdx );
			if( p )
			{
				nextStep = p->mStep + 1;

				// 마스터 검사	
				if( nextStep == pInfo->mStepCount )
					continue;
			}

			// 단계관련 검사
			sPlayerSkillStepInfo pStepInfo = pInfo->mpSetpInfoArray[nextStep];

			// 요구 sp검사
			if( HERO->GetSkillPointRemain() < pStepInfo.mRequireSP )
				continue;

			// 레벨 검사
			if( pInfo->mpSetpInfoArray[nextStep].mPlayerLevel > HERO->GetLevel() )
				continue;

			// 이전 직업 총 사용 SP검사
			if( pInfo->mJobStep > 0 && pInfo->mJobStep < MAX_JOBSTEP )
			{
				unsigned long point = HERO->GetJobUsedSkillPoint( pInfo->mJobStep-1 );
				if( point < pInfo->mBeforeReqSP )
					continue;
			}

			// 선행 스킬 1
			if( pStepInfo.mLearnSkillIdx1 != 0 )
			{
				sKeepSkill* keep1 = SKILLMAN->GetKeepInfo( pStepInfo.mLearnSkillIdx1 );
				if( keep1 == 0 )
					continue;

				if( keep1->mStep + 1 < pStepInfo.mLearnSkillStep1 )
					continue;
			}

			// 선행 스킬 2
			if( pStepInfo.mLearnSkillIdx2 != 0 )
			{
				sKeepSkill* keep2 = SKILLMAN->GetKeepInfo( pStepInfo.mLearnSkillIdx2 );
				if( keep2 == 0 )
					continue;

				if( keep2->mStep + 1 < pStepInfo.mLearnSkillStep2 )
					continue;
			}

			button->SetEnabled( true );
			button->Show();
		}
		else
		{
			assert(0);
		}
	}
}

void cSkillSheetWindow::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cSkillSheetWindow::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}

void cSkillSheetWindow::ClearSkillCoolTime( unsigned long exceptSkillIdx )
{
	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator iend = mManagedIcon.End();
	for( ; i != iend; ++i )
	{
		cSkillIcon* icon = (cSkillIcon*)(*i).mSecond;
		if( icon == 0 ) 
			continue;

		if( icon->IsActive() == false || icon->GetSkillIdx() == exceptSkillIdx )
			continue;

		icon->ClearCooltime( );
	}
}

////////////////////////////////////////////////////////////////////////
/// 생활관련 시트
cCommuSheetWindow::cCommuSheetWindow( eUINodeType type )
: cUIWindow( type )
{

}

cCommuSheetWindow::~cCommuSheetWindow()
{
	mManagedIcon.Clear();
}

bool cCommuSheetWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	/// apply community script 
	cCommunityScript::cComHashMap* pCom = COMMUNITYSCRIPT->GetComSkillMap();
	if( pCom == 0 )
		return false;

	cCommunityScript::cComHashMap::cIterator iter = pCom->Begin();
	unsigned int i=0;
	for( ;iter != pCom->End() ; ++iter )
	{
		sCommunitySkillInfo* info = (sCommunitySkillInfo*)(*iter).mSecond;
		if( !info )
		{
			assert(0);
			return false;
		}

		cIconParam* param = UIMAN->GetIconParam( info->mIconIndex );
		if( param == 0 )
			continue;

		cCommunityIcon* pIcon = new cCommunityIcon;
		if( pIcon->CreateBySkinName( "SkillIcon", this, i + 100 ) == false )
		{
			assert(0);
			continue;
		}	
		pIcon->SetRelativePos( cUIPos(info->mPosX, info->mPosY) );
		pIcon->ChangeImage( param );
		pIcon->SetUsedFlag( true );
		
		pIcon->InitInfo( info->mIndex, (info->mType == 0)? false:true );

		if( mManagedIcon.Insert( info->mIndex ,pIcon ) == false )
		{
			assert(0);
		}

		i++;
	}

	return true;
}

void cCommuSheetWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 세팅
	cCommunityScript::cComHashMap* pCom = COMMUNITYSCRIPT->GetComSkillMap();
	if( pCom )
	{
		cCommunityScript::cComHashMap::cIterator iter = pCom->Begin();
		for( ;iter != pCom->End() ; ++iter )
		{
			sCommunitySkillInfo* info = (sCommunitySkillInfo*)(*iter).mSecond;
			if( info )
			{
				cCommunityIcon* pIcon = (cCommunityIcon*)mManagedIcon.GetAt( info->mIndex );
				if( pIcon )
					pIcon->SetRelativePos( cUIPos(info->mPosX, info->mPosY) );
			}
		}
	}
}

void cCommuSheetWindow::OnIconDoubleClicked( cUINode* caller, unsigned int /*id*/ )
{
	cCommunityIcon* p = (cCommunityIcon*)caller;

	unsigned long comIdx = p->GetCommunityIndex();

	if( p->IsEmoting() )
	{
		///
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( comIdx );
		if( pInfo == 0 )
			return;

		if( HERO->GetState() == eOBJECT_STATE_IDLE )
		{
			RequestCommuSkill( comIdx );
		}
	}
	else
	{
		RequestLifeCommand( comIdx, 0, 0 );
	}
}

void cCommuSheetWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& /*pos*/, bool shift )
{
	SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, caller, id, shift );
}

void cCommuSheetWindow::OnLButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}

	/// 상대영역을 절대영역으로 바꾼다 
	cUIRect captionRc = GetAbsoluteRect();
	captionRc.mLeft = GetAbsoluteRect().mLeft + mCaptionRect.mLeft;
	captionRc.mRight = captionRc.mLeft + mCaptionRect.GetWidth();
	captionRc.mTop = GetAbsoluteRect().mTop + mCaptionRect.mTop;
	captionRc.mBottom = captionRc.mTop + mCaptionRect.GetHeight();

	/// 캡션영역 안에 포인터가 있어야먄 드래그한다 
	if( mpParent == (cUIContainer*)GetRoot() && captionRc.ContainPoint( pos ) )
	{
		mDragStart.mX = pos.mX;
		mDragStart.mY = pos.mY;

		mDragging = true;
	}
}

void cCommuSheetWindow::RequestCommuSkill( unsigned long skillIndex )
{
	if( IsNetLock() == true )
		return;

	if( HERO->IsRideVehicle() == true )
		return;

	if( HERO->IsJumping() == true )
		return;

	/// network msg
	MSG_REQ_PLAYER_COMMUNITYSKILL Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_COMMUNITYSKILL_REQ;
	Msg.mCommunitySkillIdx = skillIndex;
	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	/// 
	NetLock();
}

void cCommuSheetWindow::RequestLifeCommand( unsigned long comIdx, LPCTSTR value1, LPCSTR value2 )
{
	sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( comIdx );
	if( pInfo == 0 )
		return;

	switch( pInfo->mRoleIndex )
	{
	case eCOMMU_LIFE_REST:
		{
			HERO->ToggleSitDown();
		}
		break;
	case eCOMMU_LIFE_PARTY:
		{
			LPCTSTR targetName = value1;
			if( targetName == 0 )
			{
				cBaseObject* pTarget = HERO->GetTargetObject();
				if( pTarget == 0 )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
					return;
				}
				
				// 타겟이 Player가 아닌 다른 Object인 경우 취소
				if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
					return;
				}

				targetName = pTarget->GetName();
			}
			PARTYMAN->RequestPartyAdd( targetName );
		}
		break;
	case eCOMMU_LIFE_TRADE:
		{
			LPCTSTR targetName = value1;
			if( targetName == 0 )
			{
				/// /거래 + 타겟팅 인지 검사
				cBaseObject* obj = HERO->GetTargetObject();
				if( obj )
				{
					if( obj->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						#ifdef _GMTOOL
						if( CHEATMAN->GetHideMode() == true )
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
							break;
						}
						#endif

						/// 따라가기 종료
						if( HERO->IsFollowing() )
							HERO->ReleaseFollowPlayer();

						ITEMMAN->TryToTradeAsk( (cPlayer*)obj );
					}
					else
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
				}
				else
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
				}
			}
			else
			{
				cPlayer* player = OBJECTMAN->GetPlayerByName( targetName );
				if( player )
				{
					#ifdef _GMTOOL
					if( CHEATMAN->GetHideMode() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
						break;
					}
					#endif

					/// 따라가기 종료
					if( HERO->IsFollowing() )
						HERO->ReleaseFollowPlayer();

					ITEMMAN->TryToTradeAsk( player );
				}
				else
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
			}
		}
		break;
	case eCOMMU_LIFE_TITLE:
		{
			GAMEUI->ToggleTitleWindow();
		}
		break;
	case eCOMMU_LIFE_ITEMMIX:
		{
			cUIWindow* pMix = (cUIWindow*)GAMEUI->GetItemMixWindow();
			if( pMix )
			{
				if( pMix->IsVisible() == false )
				{
					ITEMMIXMAN->ItemMixOpen();
				}
				else
				{
					ITEMMIXMAN->ItemMixCancel();
				}
			}
		}
		break;
	case eCOMMU_LIFE_STREETSTALL:
		{
			cUserSellWindow* pUserSell = GAMEUI->GetUserSellWindow();
			if( pUserSell && pUserSell->IsVisible() == false )
				ITEMMAN->TryToUserSellOpen();
		}
		break;
	case eCOMMU_LIFE_DUEL:
		{
			HERO->DuelRequest();
		}
		break;
	case eCOMMU_LIFE_ENHANCED:
		{
			/// 아이템 강화창 열기
			cUIWindow* pEnhan = (cUIWindow*)GAMEUI->GetEnhancedWindow();
			if( pEnhan )
			{
				if( pEnhan->IsVisible() == false )
					pEnhan->Show();
				else
					pEnhan->Hide();
			}

			cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
			if( pEnhan->IsVisible() == true )
			{
				if( inven && inven->IsVisible() == false )
					inven->Show(false);
			}
		}
		break;
	case eCOMMU_LIFE_PUTCARD:
		{
			cUIWindow* putcard = (cUIWindow*)GAMEUI->GetPutCardWindow();
			if( putcard )
			{
				if( putcard->IsVisible() == false )
					putcard->Show();
				else
					putcard->Hide();
			}

			cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
			if( putcard->IsVisible() == true )
			{
				if( inven && inven->IsVisible() == false )
					inven->Show(false);
			}
		}
		break;
	case eCOMMU_LIFE_TAROT:
		TAROTMAN->TarotSelfOpen();
		break;
	case eCOMMU_LIFE_FOLLOW:
		{
			/// 파티원초대
			/// /파티 + 타겟팅 인지 검사
			cBaseObject* pTarget = HERO->GetTargetObject();
			if( pTarget )
			{
				/// 타겟 있음
				HERO->RequestFollowPlayer( pTarget->GetObjectID() );
			}
			else
			{
				/// 타겟 없음.
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7002 ) );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_ME:
		{
			/// 장전된 스킬이 있는 경우 - 타겟 변경 여부 확인..
			if( SKILLEXECUTER->IsSkillCharge() == true )
			{
				cBaseObject* curTarget = HERO->GetTargetObject();
				if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
				{
					/// 신규 타겟 등록후 발동
					HERO->SetTargetObject( eOBJECTTYPE_HERO, HERO->GetObjectID() );
				}
				else
				{
					/// 타겟 변경 없이 발동
					sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
					SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, HERO );
				}
			}
			else
			{
				HERO->SetTargetObject( eOBJECTTYPE_HERO, HERO->GetObjectID() );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_PARTY_1:
		{
			if( HERO->GetPartyIndex() > 0 )
			{
				PARTYMAN->SetTargetByArrIndex( 0 );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_PARTY_2:
		{
			if( HERO->GetPartyIndex() > 0 )
			{
				PARTYMAN->SetTargetByArrIndex( 1 );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_PARTY_3:
		{
			if( HERO->GetPartyIndex() > 0 )
			{
				PARTYMAN->SetTargetByArrIndex( 2 );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_PARTY_4:
		{
			if( HERO->GetPartyIndex() > 0 )
			{
				PARTYMAN->SetTargetByArrIndex( 3 );
			}
		}
		break;
	case eCOMMU_LIFE_TARGET_PARTY_5:
		{
			if( HERO->GetPartyIndex() > 0 )
			{
				PARTYMAN->SetTargetByArrIndex( 4 );
			}
		}
		break;
	case eCOMMU_LIFE_REPORT_AUTO:
		{
 			/// 이미 심판 중인지 체크
 			cCamaelWindow* camaelWin = (cCamaelWindow*)GAMEUI->GetCamaelWindow();
 			if( camaelWin )
 			{
 				camaelWin->TryToReport();
 			}
 			else
 				assert(0);
		}
		break;
	}
}

void cCommuSheetWindow::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cCommuSheetWindow::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}

//////////////////////////////////////////////////////////////////////////
/// 주의사항 : 
void cLevelButton::OnLButtonDoubleClick( const cUIPos& pos )
{
	cLevelButton::OnLButtonDown( pos, false, false, false );
}

void cLevelButton::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	///cUINode::OnLButtonDown( pos, cUINode, alt, shift );
	
	///
	SOUNDSYS->Play2DSound( mDownSoundIdx );

	/// 캡쳐가 없으면 캡쳐를 갖는다
	if( GetCapture() != this )
	{
		SetCapture();
	}

	// 소스 이미지 바꾸기
	mpImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
		(unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );
	mPressed = true;
	mTimeFlag = true;	
}
