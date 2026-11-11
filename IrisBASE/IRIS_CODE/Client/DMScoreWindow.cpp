#include "StdAfx.h"
#include "DMScoreWindow.h"
#include "UIImageFont.h"
#include "Application.h"
#include "PVPManager.h"
#include "NumberEditBox.h"
#include "Label.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "Gauge.h"

cDMScoreWindow::cDMScoreWindow()
: mPlayTime(0)
, mStartTime(0)
, mTeamAFont(0)
, mTeamBFont(0)
, mTimeFont(0)
, mGoalPointFont(0)
, mTimePos(0, 0)
, mGoalPointPos(0, 0)
, mTeamAPointPos(0, 0)
, mTeamBPointPos(0, 0)
, mpFireSkin(0)
, mpWaterSkin(0)
, mpWindSkin(0)
, mpEarthSkin(0)
, mStoneGauge(0)
, mTeamAStoneMarkSkin(0)
, mTeamBStoneMarkSkin(0)
, mStoneImage(0)
{
	memset( mTeamAMark, 0, sizeof(mTeamAMark) );
	memset( mTeamBMark, 0, sizeof(mTeamBMark) );

	mTeamAPointSkinRect.Set(0, 0, 0, 0);
	mTeamBPointSkinRect.Set(0, 0, 0, 0);
}

cDMScoreWindow::~cDMScoreWindow()
{
	mTeamAFont = 0;
	mTeamBFont = 0;
	mTimeFont = 0;
	mGoalPointFont = 0;
	SAFE_DELETE( mStoneImage );
}

void cDMScoreWindow::Clear()
{
	mStartTime = 0;
	mPlayTime = 0;

	UpdatePoint( 0, 0 );

	for( int i = 0; i < 3; ++i )
	{
		if( mTeamAMark[i] )
			mTeamAMark[i]->Hide();
		if( mTeamBMark[i] )
			mTeamBMark[i]->Hide();
	}

	SAFE_DELETE( mStoneImage );

	if( mStoneGauge )
	{
		mStoneGauge->SetMaxValue(100);
		mStoneGauge->SetGaugeNoAni(0);
		mStoneGauge->Hide();
	}
}

bool cDMScoreWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	for( int i = 0; i < 3; ++i )
	{
		mTeamAMark[i] = (cLabel*)GetChild( eUIID_GAME_DMSCORE_TEAMAMARK0 + i );
	}
	
	for( int i = 0; i < 3; ++i )
	{
		mTeamBMark[i] = (cLabel*)GetChild( eUIID_GAME_DMSCORE_TEAMBMARK0 + i );
	}

	mTeamAFont = UIMAN->GetImageFont( eIAMGEFONT_TAROTSTR );
	mTeamBFont = UIMAN->GetImageFont( eIAMGEFONT_TAROTINV );
	mTimeFont = UIMAN->GetImageFont( eIAMGEFONT_PVPTIME );
	mGoalPointFont = UIMAN->GetImageFont( eIAMGEFONT_PVPDESTPOINT );

	mStoneGauge = (cGauge*)GetChild( eUIID_GAME_DMSCORE_STONE_GAUGE );
	if( !mStoneGauge )
	{
		assert(0);
		return false;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* timeSkin = pSkin->GetNodeSkin( "PVP_DMScore_Time" );
	if( timeSkin )
	{
		mTimePos.mX = timeSkin->mSkinInfo->mX;
		mTimePos.mY = timeSkin->mSkinInfo->mY;
	}

	cUINodeSkin* destSkin = pSkin->GetNodeSkin( "PVP_DMScore_DestPoint" );
	if( destSkin )
	{
		mGoalPointPos.mX = destSkin->mSkinInfo->mX;
		mGoalPointPos.mY = destSkin->mSkinInfo->mY;
	}

    cUINodeSkin* ourSkin = pSkin->GetNodeSkin( "PVP_DMScore_PointTeamA" );
	if( ourSkin )
	{
		mTeamAPointSkinRect.mLeft = ourSkin->mSkinInfo->mX;
		mTeamAPointSkinRect.mTop = ourSkin->mSkinInfo->mY;
		mTeamAPointSkinRect.mRight = mTeamAPointSkinRect.mLeft + ourSkin->mSkinInfo->mWidth;
		mTeamAPointSkinRect.mBottom = mTeamAPointSkinRect.mTop + ourSkin->mSkinInfo->mHeight;
	}

	cUINodeSkin* enumySkin = pSkin->GetNodeSkin( "PVP_DMScore_PointTeamB" );
	if( enumySkin )
	{
		mTeamBPointSkinRect.mLeft = enumySkin->mSkinInfo->mX;
		mTeamBPointSkinRect.mTop = enumySkin->mSkinInfo->mY;
		mTeamBPointSkinRect.mRight = mTeamBPointSkinRect.mLeft + enumySkin->mSkinInfo->mWidth;
		mTeamBPointSkinRect.mBottom = mTeamBPointSkinRect.mTop + enumySkin->mSkinInfo->mHeight;
	}

	mpFireSkin = pSkin->GetNodeSkin( "PVP_DMScore_MarkFire" );
	mpWaterSkin = pSkin->GetNodeSkin( "PVP_DMScore_MarkWater" );
	mpWindSkin = pSkin->GetNodeSkin( "PVP_DMScore_MarkWind" );
	mpEarthSkin = pSkin->GetNodeSkin( "PVP_DMScore_MarkEarth" );

	if( !mpFireSkin || !mpWaterSkin || !mpWindSkin || !mpEarthSkin )
	{
		assert(0);
		return false;
	}

	mTeamAStoneMarkSkin = pSkin->GetNodeSkin( "PVP_DMScore_TeamAStone" );
	mTeamBStoneMarkSkin = pSkin->GetNodeSkin( "PVP_DMScore_TeamBStone" ); 
	if( !mTeamAStoneMarkSkin || !mTeamBStoneMarkSkin )
	{
		assert(0);
		return false;
	}

	return true;
}

void cDMScoreWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mStoneImage && mTeamAStoneMarkSkin )
	{
		int x = GetAbsoluteRect().mLeft + mTeamAStoneMarkSkin->mSkinInfo->mX;
		int y = GetAbsoluteRect().mTop  + mTeamAStoneMarkSkin->mSkinInfo->mY;
		int width = x + mTeamAStoneMarkSkin->mSkinInfo->mWidth;
		int height = y + mTeamAStoneMarkSkin->mSkinInfo->mHeight;
		mStoneImage->SetScreenRect( cUIRect( x, y, width, height ) );	
	}
}

void cDMScoreWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	// 위치 갱신
	if( mGoalPointFont )
	{
		unsigned long goalPoint = PVPMAN->GetGoalPoint();
		mGoalPointStr.Format( "%d", goalPoint );

		int x = (int)((GetAbsoluteRect().GetWidth() - mGoalPointFont->GetTextWidth( goalPoint ) ) * 0.5f);
		mGoalPointPos.mX = x;
	}

	if( mTeamAFont && mTeamBFont )
	{
		unsigned long scoreA = PVPMAN->GetTeamAScore();
		unsigned long scoreB = PVPMAN->GetTeamBScore();

		mTeamAPointStr.Format( "%d", scoreA );
		mTeamBPointStr.Format( "%d", scoreB );

		//
		if( HERO->GetPVPTeam() == ePVPDM_TEAMTYPE_A )
		{
			mTeamAPointPos.mX = mTeamAPointSkinRect.mLeft + (int)((mTeamAPointSkinRect.GetWidth() - mTeamAFont->GetTextWidth( scoreA ) ) * 0.5f);
			mTeamAPointPos.mY = mTeamAPointSkinRect.mTop;

			mTeamBPointPos.mX = mTeamBPointSkinRect.mLeft + (int)((mTeamBPointSkinRect.GetWidth() - mTeamBFont->GetTextWidth( scoreB ) ) * 0.5f);
			mTeamBPointPos.mY = mTeamBPointSkinRect.mTop;
		}
		else
		{
			mTeamAPointPos.mX = mTeamBPointSkinRect.mLeft + (int)((mTeamBPointSkinRect.GetWidth() - mTeamAFont->GetTextWidth( scoreA ) ) * 0.5f);
			mTeamAPointPos.mY = mTeamBPointSkinRect.mTop;

			mTeamBPointPos.mX = mTeamAPointSkinRect.mLeft + (int)((mTeamAPointSkinRect.GetWidth() - mTeamAFont->GetTextWidth( scoreB ) ) * 0.5f);
			mTeamBPointPos.mY = mTeamAPointSkinRect.mTop;
		}
	}

	if( mStartTime == 0 )
		return;

	///
	long elapsedTime = accumTime - mStartTime;
	mPlayTime -= elapsedTime;
	mStartTime = accumTime;
    
	int m = 0;
	int s = 0;
	if( mPlayTime < 0 )
	{
		mPlayTime = 0;
	}
	else
	{
		int restTime = mPlayTime / 1000;
		m = restTime / 60;
		s = restTime % 60;
	}

	char timeStr[20]={0,};
	::sprintf_s( timeStr, "%02d:%02d", m, s );
	mTimeStr = timeStr;
}

void cDMScoreWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	int x = GetAbsoluteRect().mLeft;
	int y = GetAbsoluteRect().mTop;

	if( mTeamAFont )
		mTeamAFont->DrawText( mTeamAPointPos.mX + x, mTeamAPointPos.mY + y, mTeamAPointStr.Cstr() );

	if( mTimeFont )
		mTimeFont->DrawText( mTimePos.mX + x, mTimePos.mY + y, mTimeStr.Cstr() );
	
	if( mTeamBFont )
		mTeamBFont->DrawText( mTeamBPointPos.mX + x, mTeamBPointPos.mY + y, mTeamBPointStr.Cstr() );

	if( mGoalPointFont )
		mGoalPointFont->DrawText( mGoalPointPos.mX + x, mGoalPointPos.mY + y, mGoalPointStr.Cstr() );

	if( mStoneImage )
		mStoneImage->Draw();
}

void cDMScoreWindow::UpdatePoint( unsigned int pointA, unsigned int pointB )
{
	mTeamAPointStr.Format( "%d", pointA );
	mTeamBPointStr.Format( "%d", pointB );
}

/// 시간 세팅
void cDMScoreWindow::SetTime( long time )
{
	mPlayTime = time;
	
	int restTime = mPlayTime / 1000;
	int	m = restTime / 60;
	int	s = (restTime % 60) % 60;

	char timeStr[20]={0,};
	::sprintf_s( timeStr, "%02d:%02d", m, s );
	mTimeStr = timeStr;
}

void cDMScoreWindow::ActiveTime()
{
	mStartTime = THEAPP->GetWorldAccumTime();
}

void cDMScoreWindow::DeActiveTime()
{
	mStartTime = 0;
	SetTime( 0 );
}

/// true : 아군 (a팀), false, 적군 (b팀)
void cDMScoreWindow::SetTeamImage( bool fire, bool water, bool wind, bool earth )
{
	for( int i = 0; i < 3; ++i )
	{
		if( mTeamAMark[i] )
			mTeamAMark[i]->Hide();
		if( mTeamBMark[i] )
			mTeamBMark[i]->Hide();
	}

	int indexA = 0;
	int indexB = 0;

	/// fire
	if( fire == true )
	{
		if( mTeamAMark[indexA] )
		{
			mTeamAMark[indexA]->ChangeImage( mpFireSkin );
			mTeamAMark[indexA]->Show();
		}
		indexA++;
	}
	else
	{
		if( mTeamBMark[indexB] )
		{
			mTeamBMark[indexB]->ChangeImage( mpFireSkin );
			mTeamBMark[indexB]->Show();
		}
		indexB++;
	}

	/// water
	if( water == true )
	{
		if( mTeamAMark[indexA] )
		{
			mTeamAMark[indexA]->ChangeImage( mpWaterSkin );
			mTeamAMark[indexA]->Show();
		}
		indexA++;
	}
	else
	{
		if( mTeamBMark[indexB] )
		{
			mTeamBMark[indexB]->ChangeImage( mpWaterSkin );
			mTeamBMark[indexB]->Show();
		}
		indexB++;
	}

	/// wind
	if( wind == true )
	{
		if( mTeamAMark[indexA] )
		{
			mTeamAMark[indexA]->ChangeImage( mpWindSkin );
			mTeamAMark[indexA]->Show();
		}
		indexA++;
	}
	else
	{
		if( mTeamBMark[indexB] )
		{
			mTeamBMark[indexB]->ChangeImage( mpWindSkin );
			mTeamBMark[indexB]->Show();
		}
		indexB++;
	}

	/// earth
	if( earth == true )
	{
		if( mTeamAMark[indexA] )
		{
			mTeamAMark[indexA]->ChangeImage( mpEarthSkin );
			mTeamAMark[indexA]->Show();
		}
		indexA++;
	}
	else
	{
		if( mTeamBMark[indexB] )
		{
			mTeamBMark[indexB]->ChangeImage( mpEarthSkin );
			mTeamBMark[indexB]->Show();
		}
		indexB++;
	}
}

void cDMScoreWindow::SetStoneImage( unsigned char pvpTeamType )
{
	cUINodeSkin* stoneSkin = NULL;
	if( pvpTeamType == ePVPDM_TEAMTYPE_A )
		stoneSkin = mTeamAStoneMarkSkin;
	else if( pvpTeamType == ePVPDM_TEAMTYPE_B )
		stoneSkin = mTeamBStoneMarkSkin;
	else
	{
		assert(0);
		return;
	}

	SAFE_DELETE( mStoneImage );

	if( stoneSkin == NULL )
	{
		assert(0);
		return;
	}

	mStoneImage = new cUIImage( stoneSkin->mpTexture );
	unsigned int tx = stoneSkin->mSkinInfo->mTexX;
	unsigned int ty = stoneSkin->mSkinInfo->mTexY;
	unsigned int tw = stoneSkin->mSkinInfo->mTexWidth;
	unsigned int th = stoneSkin->mSkinInfo->mTexHeight;
	mStoneImage->SetTextureRect( tx, ty, tx + tw, ty + th );

	int x = GetAbsoluteRect().mLeft + stoneSkin->mSkinInfo->mX;
	int y = GetAbsoluteRect().mTop  + stoneSkin->mSkinInfo->mY;
	int width = x + stoneSkin->mSkinInfo->mWidth;
	int height = y + stoneSkin->mSkinInfo->mHeight;
	mStoneImage->SetScreenRect( cUIRect( x, y, width, height ) );	

	if( mStoneGauge )
		mStoneGauge->Show();
}

void cDMScoreWindow::SetStoneMaxHP( unsigned int maxHP )
{
	if( mStoneGauge )
	{
		mStoneGauge->SetMaxValue( maxHP );
	}
}

void cDMScoreWindow::SetStoneHP( unsigned int hp )
{
	if( mStoneGauge )
	{
		mStoneGauge->SetGaugeNoAni( hp );
	}
}

//////////////////////////////////////////////////////////////////////////
///

cDMRankingListWindow::cDMRankingListWindow()
: mpRankList(0)
{

}

cDMRankingListWindow::~cDMRankingListWindow()
{

}

bool cDMRankingListWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpRankList = (cListBox*)GetChild( eUIID_GAME_MIDDLERESULT_LIST );

	return true;
}

void cDMRankingListWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );

}

void cDMRankingListWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );
}


void cDMRankingListWindow::Clear()
{
	if( mpRankList )
		mpRankList->Clear();
}

void cDMRankingListWindow::AddPlayerRankInfo( sMyTeamInfo* info, bool leader )
{
	if( mpRankList == 0 || info == 0 )
	{
		assert(0);
		return;
	}

	/// 삽입
	unsigned long color = mDefaultColor;
	if( info->mPlayerIdx == HERO->GetObjectID() )
		color = COLOR_YELLOW;

	cStringT str;
	str.Format( _T("%d"), mpRankList->GetRowCount()+1 );
	int row = mpRankList->AddRow( (LPCTSTR)str.Cstr(), color );

	/// 아이디
	if( leader )
	{
		str.Format( _T("◆%s◆"), info->mName );
		mpRankList->AddColume( row, (LPCTSTR)str.Cstr(), color );
	}
	else
	{
		mpRankList->AddColume( row, info->mName, color );
	}

	///
	int textIdx = 0;
	switch( info->mForceType )
	{
	case eFORCETYPE_NONE:	textIdx = 119;	break;
	case eFORCETYPE_FIRE:	textIdx = 120;	break;
	case eFORCETYPE_WATER:  textIdx = 121;	break;
	case eFORCETYPE_WIND:	textIdx = 122;	break;
	case eFORCETYPE_EARTH:	textIdx = 123;	break;
	default: assert(0); return;
	}
	mpRankList->AddColume( row, UIMAN->GetUIText( textIdx ), color );

	/// 킬수 
	str.Format( _T("%d"), info->mKillCnt );
	mpRankList->AddColume( row, (LPCTSTR)str.Cstr(), color );

	/// DEATH
	str.Format( _T("%d"), info->mDeathCnt );
	mpRankList->AddColume( row, (LPCTSTR)str.Cstr(), color );
}

/////////////////////////////////////////////////////////////////////
////
cDMResultWindow::cDMResultWindow()
: mpReultList(0)
, mpPointList(0)
, mpMoney(0)
, mpEXP(0)
, mpSXP(0)
, mpWin(0)
, mpLose(0)
{
}

cDMResultWindow::~cDMResultWindow()
{

}

void cDMResultWindow::Clear()
{
	if( mpReultList )
		mpReultList->Clear();

	if( mpPointList )
		mpPointList->Clear();

	if( mpWin )
		mpWin->Hide();

	if( mpLose )
		mpLose->Hide();
}

void cDMResultWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );
}

bool cDMResultWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpReultList = (cListBox*)GetChild( eUIID_GAME_DMRESULT_LISTBOX );
	mpPointList = (cListBox*)GetChild( eUIID_GAME_DMRESULT_POINTLIST );
	mpMoney = (cLabel*)GetChild( eUIID_GAME_DMRESULT_MONEY );
	mpEXP = (cLabel*)GetChild( eUIID_GAME_DMRESULT_EXP );
	mpSXP = (cLabel*)GetChild( eUIID_GAME_DMRESULT_SXP );
	mpWin = GetChild( eUIID_GAME_DMRESULT_WIN );
	mpLose = GetChild( eUIID_GAME_DMRESULT_LOSE );
	return true;
}

void cDMResultWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );
}

void cDMResultWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_GAME_DMRESULT_CLOSE )
	{
		/// 맵이동요청
		MSGROOT msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		Hide();

		HERO->SetStopFlag( eSTOP_MAPCHANGE );

		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_MAPCHANGEWAIT, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
			GAMERESOURCEMAN->GetGameText( 2 ) // 알림
			);
	}
}

void cDMResultWindow::SetResult( bool win )
{
	if( win )
	{
		if( mpWin )
			mpWin->Show();
	}
	else
	{
		if( mpLose )
			mpLose->Show();
	}
}

void cDMResultWindow::InsertRank( sMyTeamInfo* data, int ranki )
{
	if( mpReultList == 0 || data == 0 )
	{
		assert(0);
		return;
	}

	/// 삽입
	cStringT str;

	unsigned long color = mDefaultColor;
	str.Format( _T("%d"), ranki+1 );
	int row = mpReultList->AddRow( (LPCTSTR)str.Cstr(), color );

	/// 아이디
	mpReultList->AddColume( row, data->mName, color );

	int textIdx = 0;
	switch( data->mForceType )
	{
	case eFORCETYPE_NONE:	textIdx = 119;	break;
	case eFORCETYPE_FIRE:	textIdx = 120;	break;
	case eFORCETYPE_WATER:  textIdx = 121;	break;
	case eFORCETYPE_WIND:	textIdx = 122;	break;
	case eFORCETYPE_EARTH:	textIdx = 123;	break;
	default: assert(0); return;
	}

	/// 
	mpReultList->AddColume( row, UIMAN->GetUIText( textIdx ), color );

	/// 킬수 
	str.Format( _T("%d"), data->mKillCnt );
	mpReultList->AddColume( row, (LPCTSTR)str.Cstr(), color );

	/// DEATH
	str.Format( _T("%d"), data->mDeathCnt );
	mpReultList->AddColume( row, (LPCTSTR)str.Cstr(), color );

	///
//	str.Format( _T("%d"), data->mScoreResult );
//	mpReultList->AddColume( row, (LPCTSTR)str.Cstr(), color );
}

void cDMResultWindow::SetReward( unsigned long money, unsigned long exp, unsigned long sxp )
{
	if( mpMoney )
	{
		LPCTSTR moneyStr = cNumberEditBox::FormatNumber( money );
		mpMoney->SetText( moneyStr );
	}

	if( mpEXP )
	{
		LPCTSTR expStr = cNumberEditBox::FormatNumber( exp );
		mpEXP->SetText( expStr );
	}

	if( mpSXP )
	{
		LPCTSTR sxpStr = cNumberEditBox::FormatNumber( sxp );
		mpSXP->SetText( sxpStr );
	}
}

void cDMResultWindow::SetRewardForce( unsigned long point, unsigned long friendly )
{
	if( mpPointList )
	{
		TCHAR pStr[20] = {0,};
		TCHAR fStr[20] = {0,};

		::_stprintf_s( pStr, _T("%d"), point );
		::_stprintf_s( fStr, _T("%d"), friendly );

		int row = mpPointList->AddRow( pStr, mDefaultColor );
		mpPointList->AddColume( row, fStr, mDefaultColor );
	}
}
