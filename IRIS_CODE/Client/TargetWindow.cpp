#include "StdAfx.h"
#include "TargetWindow.h"

#include "Label.h"
#include "Gauge.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PVPManager.h"
#include "SceneManager.h"
#include "EffectSceneNode.h"
#include "GameUIManager.h"
#include "DuelWindow.h"
#include "RButtonMenuWindow.h"
#include "PartyManager.h"
#include "GameResourceManager.h"
#include "Icon.h"
#include "Npc.h"
#include "Monster.h"
#include "SimpleTipWindow.h"

cTargetWindow::cTargetWindow()
: mpTargetNameLabel(0)
, mpTargetLevelLabel(0)
, mpHPGauge(0)
, mpTargetBuff(0)
, mpNodeSkin(0)
, mShowHPImage(true)
, mpTargetJobLabel(0)
, mpTargetNpcJobLabel(0)
, mpTargetNpcJobLabelText(0)
, mIsShowToolTip( false )
, mpIsParty(0)
, mpIsOtherParty(0)
{
	for( unsigned int i=0;i<2;++i )
	{
		mTargetEffectIdx[i] = (unsigned long)-1;
		mpTargetEffect[i] = 0;
	}

	::ZeroMemory( mTargetTexPos, sizeof(mTargetTexPos) );
}

cTargetWindow::~cTargetWindow()
{
}

void cTargetWindow::Open()
{
	for( unsigned int i=0;i<2; ++i )
	{
		if( mTargetEffectIdx[i] != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mTargetEffectIdx[i] );
			if( node )
			{
				assert( node == mpTargetEffect[i] );
				node->Remove();
			}
			mpTargetEffect[i] = 0;
			mTargetEffectIdx[i] = (unsigned long)-1;
		}
	}
}

void cTargetWindow::Close()
{
	cUIWindow::Close();

	for( unsigned int i=0;i<2; ++i )
	{
		if( mTargetEffectIdx[i] != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mTargetEffectIdx[i] );
			if( node )
			{
				assert( node == mpTargetEffect[i] );
				node->Remove();
			}
			mpTargetEffect[i] = 0;
			mTargetEffectIdx[i] = (unsigned long)-1;
		}
	}

	if( mpTargetBuff )
		mpTargetBuff->Hide();

	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip && mIsShowToolTip )
	{	
		pTip->Hide();
		mIsShowToolTip = false;
	}

	HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
}

void cTargetWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpTargetBuff )
		mpTargetBuff->Show();
}

void cTargetWindow::OnHide()
{
	cUIWindow::OnHide();

	for( unsigned int i=0;i<2; ++i )
	{
		if( mTargetEffectIdx[i] != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mTargetEffectIdx[i] );
			if( node )
			{
				assert( node == mpTargetEffect[i] );
				node->Remove();
			}
			mpTargetEffect[i] = 0;
			mTargetEffectIdx[i] = (unsigned long)-1;
		}
	}

	if( mpTargetBuff )
		mpTargetBuff->Hide();

	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip && mIsShowToolTip )
	{	
		pTip->Hide();
		mIsShowToolTip = false;
	}

	HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
}

//
bool cTargetWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mHoldView = true;

	mpHPGauge = (cGauge*)GetChild( eUIID_GAME_TARGET_HP_GAUGE_BAR );
	mpTargetNameLabel = (cLabel*)GetChild( eUIID_GAME_TARGET_NAME_LABEL );
	mpTargetLevelLabel = (cLabel*)GetChild( eUIID_GAME_TARGET_LEVEL_LABEL );
	mpTargetBuff = (cTargetBuff*)GetChild( eUIID_GAME_TARGET_BUFFWINDOW );
	mpTargetJobLabel = (cLabel*)GetChild( eUIID_GAME_TARGET_JOB_LABEL );
	mpTargetNpcJobLabel = (cLabel*)GetChild( eUIID_GAME_TARGET_NPC_JOB_LABEL );
	mpTargetNpcJobLabelText = (cLabel*)GetChild( eUIID_GAME_TARGET_NPC_JOB_TEXT_LABEL );
	mpIsParty = (cLabel*)GetChild( eUIID_GAME_TARGET_PARTY );
	mpIsOtherParty = (cLabel*)GetChild( eUIID_GAME_TARGET_OTHERPARTY );

	ePLAYER_JOB job = HERO->GetJob();
	sJobInfo* pInfo = GAMERESOURCEMAN->GetJobInfo( job );
	if( pInfo )
	{

		cIconParam* pParam = UIMAN->GetIconParam( pInfo->jobTargetIconIndex );
		if( pParam == 0 )
		{
			assert(0);
			return false;
		}

		/// 타겟 직업 이미지와 npc 직업 표시 이미지는 동일한 텍스쳐, 크기를 가진다.
 		mpTargetJobLabel->SetTexture( pParam->mpTexture, pParam->mTexPos.mX, pParam->mTexPos.mY );
 		mpTargetNpcJobLabel->SetTexture( pParam->mpTexture, pParam->mTexPos.mX, pParam->mTexPos.mY );
	}
	

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

 	mpNodeSkin = pSkin->GetNodeSkin( mSkinName );
 	if( !mpNodeSkin )
 		return false;

	cUINodeSkin* pGaugeSkin = pSkin->GetNodeSkin( "Target_HP_Hero" );
	if( !pGaugeSkin )
		return false;

	mTargetTexPos[eGauge_Hero].mX = pGaugeSkin->mSkinInfo->mX;
	mTargetTexPos[eGauge_Hero].mY = pGaugeSkin->mSkinInfo->mY;

	pGaugeSkin = pSkin->GetNodeSkin( "Target_HP_Monster" );
	if( !pGaugeSkin )
		return false;

	mTargetTexPos[eGauge_Monster].mX = pGaugeSkin->mSkinInfo->mX;
	mTargetTexPos[eGauge_Monster].mY = pGaugeSkin->mSkinInfo->mY;


	
	return true;
}

void cTargetWindow::UpdateRect()
{

	cUIWindow::UpdateRect();

	/// 이미지 영역을 갱신
	if( mpImage )
	{
		if( mpNodeSkin )
		{
			cUIRect imgRect( mAbsoluteRect.mLeft, 
				mAbsoluteRect.mTop, 
				mAbsoluteRect.mLeft + mpNodeSkin->mSkinInfo->mWidth, 
				mAbsoluteRect.mTop  + mpNodeSkin->mSkinInfo->mHeight );
			mpImage->SetScreenRect( imgRect );
		}
		else
			mpImage->SetScreenRect( mAbsoluteRect );
	}
}

void cTargetWindow::UpdateSkin()
{
	if( GAMEUI->IsDuelMode() == false )
	{
		cUIWindow::UpdateSkin();
	}
}

///
void cTargetWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	assert( HERO );
	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget )
	{
		if( mpHPGauge )
		{
			mpHPGauge->SetMaxValue( (float)pTarget->GetMaxHP() );
			mpHPGauge->SetGaugeNoAni( (float)pTarget->GetHP() );
		}

		if( pTarget->IsDie() )
		{
			for( unsigned int i=0;i<2; ++i )
			{
				if( mTargetEffectIdx[i] != (unsigned long)-1 )
				{
					cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mTargetEffectIdx[i] );
					if( node )
					{
						assert( node == mpTargetEffect[i] );
						node->Remove();
					}
					mpTargetEffect[i] = 0;
					mTargetEffectIdx[i] = (unsigned long)-1;
				}
			}
		}
	}

	cUIWindow::OnProcess( deltaTime, accumTime );
}

void cTargetWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	cUIWindow::OnRender( pKeeper );
}

void cTargetWindow::UpdateWindow()
{
	assert( HERO );
	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget == 0 )
	{
		Hide();
		return;
	}

	/// 상황에 따라 게이지를 보인다
	if( mpHPGauge )
		mpHPGauge->Hide();

	if( mpTargetNpcJobLabelText )
		mpTargetNpcJobLabelText->Hide();

	mShowHPImage = false;
	///////////////////////////////////
	switch( pTarget->GetObjectType() )
	{
	case eOBJECTTYPE_PLAYER:
		{
			if( PVPMAN->IsEnableBattle() && PVPMAN->IsMyTeam( (cPlayer*)pTarget) == false ) 
			{
				int textIdx = 0;
				switch( ((cPlayer*)pTarget)->GetForceType() )
				{
				case eFORCETYPE_NONE:	textIdx = 119;	break;
				case eFORCETYPE_FIRE:	textIdx = 120;	break;
				case eFORCETYPE_WATER:  textIdx = 121;	break;
				case eFORCETYPE_WIND:	textIdx = 122;	break;
				case eFORCETYPE_EARTH:	textIdx = 123;	break;
				}
				SetTargetInfo( pTarget->GetLevel(), (LPTSTR)UIMAN->GetUIText( textIdx ) );
			}
			else
			{
				SetTargetInfo( pTarget->GetLevel(), pTarget->GetName() );
			}

			if( mpHPGauge )
			{
				mpHPGauge->ChangeTexturePos( mTargetTexPos[eGauge_Hero] );
				mpHPGauge->Show();
			}

			ePLAYER_JOB job = ((cPlayer*)pTarget)->GetJob();
			sJobInfo* pInfo = GAMERESOURCEMAN->GetJobInfo( job );
			if( pInfo )
			{

				cIconParam* pParam = UIMAN->GetIconParam( pInfo->jobTargetIconIndex );
				if( pParam == 0 )
				{
					assert(0);
					return;
				}
				mpTargetJobLabel->ChangeImage( pParam->mTexPos.mX, pParam->mTexPos.mY );
				mpTargetJobLabel->Show();
				mpTargetNpcJobLabel->Hide();
			}

			mShowHPImage = true;
		}
		break;
	case eOBJECTTYPE_HERO:
		{
 			ePLAYER_JOB job = HERO->GetJob();
 			sJobInfo* pInfo = GAMERESOURCEMAN->GetJobInfo( job );
 			if( pInfo )
 			{

				cIconParam* pParam = UIMAN->GetIconParam( pInfo->jobTargetIconIndex );
				if( pParam == 0 )
				{
					assert(0);
					return;
				}

				mpTargetJobLabel->ChangeImage( pParam->mTexPos.mX, pParam->mTexPos.mY );
				mpTargetJobLabel->Show();
				mpTargetNpcJobLabel->Hide();
			}

			SetTargetInfo( pTarget->GetLevel(), pTarget->GetName() );

			if( mpHPGauge )
			{
				mpHPGauge->ChangeTexturePos( mTargetTexPos[eGauge_Hero] );
				mpHPGauge->Show();
			}

			mShowHPImage = true;
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* mon = (cMonster*)pTarget;
			eMONSTER_KIND_TYPE monType = (eMONSTER_KIND_TYPE)(mon->GetKind());
			
			cIconParam* pParam = NULL;

			if( monType == eMONSTER_KIND_NORMAL )
				pParam = UIMAN->GetIconParam( 12300 );
			else if( monType == eMONSTER_KIND_VETERAN )
				pParam = UIMAN->GetIconParam( 12301 );
			else if( monType == eMONSTER_KIND_ELITE )
				pParam = UIMAN->GetIconParam( 12302 );
			else if( monType == eMONSTER_KIND_BOSS )
				pParam = UIMAN->GetIconParam( 12303 );
			else
			{
				assert( 0 && "Unknown monster kind" );
				return;
			}

			if( pParam == 0 )
			{
				assert(0);
				return;
			}

			mpTargetJobLabel->ChangeImage( pParam->mTexPos.mX, pParam->mTexPos.mY );
			mpTargetJobLabel->Show();
			mpTargetNpcJobLabel->Hide();

			// 이름세팅
			SetTargetInfo( pTarget->GetLevel(), pTarget->GetName() );

			// 오토더미
			if( mon->IsAutoDummy() == true )
			{
				// 색 랜덤
				unsigned long color = 0xFFFFFFFF;

				unsigned long colorR = rand() % 255;
				unsigned long colorG = rand() % 255;
				unsigned long colorB = rand() % 255;

				color = (((BYTE)(colorR)|((WORD)((BYTE)(colorG))<<8))|(((DWORD)(BYTE)(colorB))<<16)) | 0xFF000000;

				if( mpTargetNameLabel )
				{
					mpTargetNameLabel->SetTextColor( color );
					mpTargetNameLabel->SetText( GAMERESOURCEMAN->GetGameText(14000) );
				}
			}

			if( mpHPGauge )
			{
				mpHPGauge->ChangeTexturePos( mTargetTexPos[eGauge_Monster] );
				mpHPGauge->Show();
			}

			mShowHPImage = true;
		}
		break;
	case eOBJECTTYPE_NPC:
		{
			cIconParam* pParam = UIMAN->GetIconParam( ((cNpc*)pTarget)->GetJobIconIndex() );
			if( pParam == 0 )
			{
				assert(0);
				return;
			}

			mpTargetNpcJobLabel->ChangeImage( pParam->mTexPos.mX, pParam->mTexPos.mY );
			mpTargetNpcJobLabel->Show();
			mpTargetJobLabel->Hide();

			if( mpTargetLevelLabel )
			{
				mpTargetLevelLabel->Clear();
				mpTargetLevelLabel->Hide();
			}

			if( mpTargetNameLabel )
			{
				mpTargetNameLabel->SetTextColor( COLOR_WHITE );
				mpTargetNameLabel->SetText( pTarget->GetName() );
			}

			if( mpTargetNpcJobLabelText )
			{
				mpTargetNpcJobLabelText->SetTextColor( COLOR_WHITE );
				mpTargetNpcJobLabelText->SetText( ((cNpc*)pTarget)->GetJobName() );
				mpTargetNpcJobLabelText->Show();
			}

		}
		break;
	default:
		Hide();
		return;
	}

	// 파티관련 갱신
	UpdateParty();

	/// 위치 갱신
	if( GAMEUI->IsDuelMode() == true )
	{
		cDuelWindow* win = GAMEUI->GetDuelWindow();
		if( win )
		{
			cUIPos pos = win->GetAbsoluteRect().GetCenter();
			SetRelativePos( cUIPos( pos.mX - GetAbsoluteRect().GetWidth()/2, win->GetAbsoluteRect().mBottom + 5 ) );
		}
	}
//	else
//	{
//		int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
//		SetRelativePos( cUIPos( x, 0 ) );
//	}

	if( HERO->IsOpenNpcTalk() == false )
		Show();

	SetTargetEffect( pTarget );
	UpdateSize();
}

void cTargetWindow::UpdateParty()
{
	assert( HERO );
	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget == 0 )
		return;

	if( mpIsParty )
		mpIsParty->Hide();

	if( mpIsOtherParty )
		mpIsOtherParty->Hide();

	switch( pTarget->GetObjectType() )
	{
	case eOBJECTTYPE_PLAYER:
		{
			// 파티 표시
			if( ((cPlayer*)pTarget)->GetPartyIndex() > 0 )
			{
				if( PARTYMAN->IsHeroParty( pTarget->GetObjectID() ) == true )
				{
					if( mpIsParty )
						mpIsParty->Show();
				}
				else
				{
					if( mpIsOtherParty )
						mpIsOtherParty->Show();
				}
			}
		}
		break;
	case eOBJECTTYPE_HERO:
		{
			// 파티 표시
			if( HERO->GetPartyIndex() > 0 )
			{
				if( mpIsParty )
					mpIsParty->Show();
			}
		}
		break;
	}
}

void cTargetWindow::SetTargetEffect( cBaseObject* pObj )
{
	if( !pObj )
	{
		assert(0);
		return;
	}

	for( unsigned int i=0;i<2; ++i )
	{
		if( mTargetEffectIdx[i] != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mTargetEffectIdx[i] );
			if( node )
			{
				assert( node == mpTargetEffect[i] );
				node->Remove();
			}
			mpTargetEffect[i] = 0;
			mTargetEffectIdx[i] = (unsigned long)-1;
		}
	}

	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 10.0f );

	switch( pObj->GetObjectType() )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			mpTargetEffect[0] = pObj->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_Target_01.nif", &trans, true, true );

			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 35.0f );
			mpTargetEffect[1] = pObj->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/Eff_PC_Target_11.nif", &trans, true, true );
		}
		break;
	case eOBJECTTYPE_NPC:
		{
			mpTargetEffect[0] = pObj->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_Target_01.nif", &trans, true, true );

			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 55.0f );
			mpTargetEffect[1] = pObj->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/Eff_PC_Target_11.nif", &trans, true, true );
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			mpTargetEffect[0] = pObj->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_Target_02.nif", &trans, true, true );

			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 45.0f );
			mpTargetEffect[1] = pObj->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/Eff_PC_Target_12.nif", &trans, true, true );
		}
		break;
	default:
		break;
	}

	for( unsigned int i=0; i<2; ++i )
	{
		if( mpTargetEffect[i] )
			mTargetEffectIdx[i] = mpTargetEffect[i]->GetIndexByManger();
	}
}

void cTargetWindow::ClearBuff()
{
	if( mpTargetBuff )
		mpTargetBuff->Clear();
}

void cTargetWindow::AddBuff(unsigned long uniqueIdx, unsigned long influenceIdx )
{
	if( mpTargetBuff )
	{
		mpTargetBuff->AddBuff( uniqueIdx, influenceIdx );

		if( IsVisible() == true )
			mpTargetBuff->Show();
	}
}

void cTargetWindow::DelBuff( unsigned long uniqueIdx )
{
	if( mpTargetBuff )
		mpTargetBuff->DelBuff( uniqueIdx );
}

void cTargetWindow::DelDeBuff( unsigned long uniqueIdx )
{
	if( mpTargetBuff )
		mpTargetBuff->DelDeBuff( uniqueIdx );
}

///
void cTargetWindow::SetHPGaugeSpeed( float s )
{
	if( mpHPGauge )
		mpHPGauge->SetGaugeSpeed( s );
}

///
void cTargetWindow::SetTargetInfo( char level, LPCTSTR name )
{
	char heroLv = HERO->GetLevel();
	char gap = level - heroLv;

	unsigned long color = COLOR_WHITE;

	/// 몬스터가 HERO보다 6레벨 이상일떄,
	if( gap >= 6 )
		color = COLOR_RED;
	/// 몬스터가 HERO보다 3 ~ 5레벨 이상일떄,
	else if( gap >= 3 )
		color = COLOR_PINK;
	else if( gap >= -2 )
		color = COLOR_GREEN;
	else if( gap >= -5 )
		color = COLOR_YELLOW;
	else
		color = COLOR_WHITE;

	if( mpTargetLevelLabel )
	{
		mpTargetLevelLabel->SetValue( level );
		mpTargetLevelLabel->Show();
	}

	if( mpTargetNameLabel )
	{
		mpTargetNameLabel->SetTextColor( color );
		mpTargetNameLabel->SetText( name );
	}
}

void cTargetWindow::UpdateSize()
{
	if( mpTargetBuff && mpNodeSkin )
	{
		cUISize size;
		size.mWidth = mpNodeSkin->mSkinInfo->mWidth;
		size.mHeight = mpNodeSkin->mSkinInfo->mHeight;

		if( mpTargetBuff->IsHaveBuff() )
			size.mHeight += mpTargetBuff->GetAbsoluteRect().GetHeight();

		SetRelativeSize( size );
	}
}

void cTargetWindow::OnRButtonUp( const cUIPos& pos )
{
	cBaseObject* pTarget = HERO->GetTargetObject();
	if( pTarget && pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
	{
		cRButtonMenuWindow* win = GAMEUI->GetRButtonMenuWindow();
		if( win )
			win->PopUp( pos.mX, pos.mY, pTarget->GetObjectID(), pTarget->GetName(), ePopupPos_TargetWindow );
	}
}

void cTargetWindow::OnMouseHovered( const cUIPos& pos )
{
	cUIRect winRect;
	winRect = mpTargetJobLabel->GetAbsoluteRect();

	if ( winRect.ContainPoint( pos ) )
	{
		assert( HERO );
		cBaseObject* pTarget = HERO->GetTargetObject();
		if( pTarget == 0 )
		{
			return;
		}
		switch( pTarget->GetObjectType() )
		{
		case eOBJECTTYPE_HERO:
		case eOBJECTTYPE_PLAYER:
			{
				ePLAYER_JOB job = ((cPlayer*)pTarget)->GetJob();
				LPCTSTR     jobName = GAMERESOURCEMAN->GetJobName( job );
				if( jobName )
				{
					cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
					if( pTip )
					{	
						UIMAN->ShowCommunityTip( pTip, pos, jobName );
						mIsShowToolTip = true;
					}
				}
			}
			break;

		case eOBJECTTYPE_MONSTER:
			{
				eMONSTER_KIND_TYPE monType;
				monType = (eMONSTER_KIND_TYPE)((cMonster*)pTarget)->GetKind();
				LPCTSTR     jobName = NULL;

				if( monType == eMONSTER_KIND_NORMAL )
					jobName = UIMAN->GetUIText( 445 );
				else if( monType == eMONSTER_KIND_VETERAN )
					jobName = UIMAN->GetUIText( 446 );
				else if( monType == eMONSTER_KIND_ELITE )
					jobName = UIMAN->GetUIText( 447 );
				else if( monType == eMONSTER_KIND_BOSS )
					jobName = UIMAN->GetUIText( 448 );
				else
				{
					assert( 0 && "Unknown monster kind" );
					return;
				}

				if( jobName )
				{
					cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
					if( pTip )
					{	
						UIMAN->ShowCommunityTip( pTip, pos, jobName );
						mIsShowToolTip = true;
					}
				}
			}
			break;
		}
	}


	winRect = mpTargetNpcJobLabel->GetAbsoluteRect();
	if ( winRect.ContainPoint( pos ) )
	{
		assert( HERO );
		cBaseObject* pTarget = HERO->GetTargetObject();
		if( pTarget == 0 )
		{
			return;
		}
		if( pTarget->GetObjectType() == eOBJECTTYPE_NPC )
		{	 				
			LPCTSTR	 jobName = ((cNpc*)pTarget)->GetJobName();
			if( jobName )
			{
				cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
				if( pTip )
				{	
					UIMAN->ShowCommunityTip( pTip, pos, jobName );
					mIsShowToolTip = true;
				}
			}
		}
	}

}
void cTargetWindow::OnMouseLeft( const cUIPos& pos )
{
	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip && mIsShowToolTip )
	{
		pTip->Hide();
	}
}

void cTargetWindow::OnMouseMove( const cUIPos& pos )
{
	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip && mIsShowToolTip )
	{
		pTip->Hide();
	}
	
	cUIWindow::OnMouseMove( pos );
}

void cTargetWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	/// 윈도우 포인터 안에 포인터가 있어야먄 드래그한다 
	if( ContainPoint( pos ) )
	{
		mDragStart.mX = pos.mX;
		mDragStart.mY = pos.mY;

		mDragging = true;
	}

	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );
}

///////////////////////////////////////////////////////////////
////// 타겟 버프

cTargetBuff::cTargetBuff()
{
	
}

cTargetBuff::~cTargetBuff()
{

}

bool cTargetBuff::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_WINDOW ) == false )
	{
		assert( 0 && "not window type" );
		return false;
	}

	cBuffWindowSkin* p = (cBuffWindowSkin*)pskin;
	if( !p )
		return false;

	/// 사이즈는 새로운 상대좌표 * 새로운 해상도 
	unsigned int x = pskin->mSkinInfo->mX;
	unsigned int y = pskin->mSkinInfo->mY;
	unsigned int w = pskin->mSkinInfo->mWidth;
	unsigned int h = pskin->mSkinInfo->mHeight;

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	SetAlpha( p->mAlpha );
	SetHoldView( p->mHoldView );

	mRow = p->mRow;
	mColumn = p->mColumn;
	mRowSpace = p->mRowSpace;
	mColumnSpace = p->mColumnSpace;
	mIconSize = p->mIconSize;

	mTimerEnabled = true;
	mTimeOut = 100;

	mpBuffSkin = p;
	return true;
}

void cTargetBuff::UpdateSkin()
{
	cPartyBuff::UpdateSkin();

	if( mpBuffSkin )
	{
		cUIRect rc;
		rc.mLeft = mRelativeRect.mLeft;
		rc.mTop = mpBuffSkin->mSkinInfo->mY;
		SetRelativePos( cUIPos( rc.mLeft, rc.mTop ) );
	}

	mUpdateList = true;
}

void cTargetBuff::UpdateRect()
{
	cUIWindow::UpdateRect();

	mUpdatePos = true;
}

void cTargetBuff::UpdateWindowSize()
{
	/// window 재계산
	cUIRect rc;
	rc.mLeft = GetRelativeRect().mLeft;
	rc.mTop = GetRelativeRect().mTop;

	int buffCount = mBuffList.GetSize();
	int debuffCount = mDeBuffList.GetSize();

	if( buffCount > 0 )
		rc.mRight = rc.mLeft + (mIconSize+mColumnSpace) * buffCount - mColumnSpace;
	else
		rc.mRight = 0;

	if( debuffCount > 0 )
	{
		rc.mBottom = rc.mTop + (mIconSize+mRowSpace) * 2 - mRowSpace;

		/// 디버프가 더 만으면 디버프갯수로 재계싼
		if( buffCount < debuffCount )
			rc.mRight = rc.mLeft + (mIconSize+mColumnSpace) * debuffCount - mColumnSpace;
	}
	else
		rc.mBottom = rc.mTop + mIconSize;

	SetRelativeRect( rc );

	cTargetWindow* parent = (cTargetWindow*)mpParent;
	if( parent )
		parent->UpdateSize();
}
