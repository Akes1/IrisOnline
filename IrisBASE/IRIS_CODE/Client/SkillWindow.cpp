#include "StdAfx.h"
#include "SkillWindow.h"
#include "TabButton.h"
#include "Label.h"
#include "CheckBox.h"
#include "UIRootNode.h"
#include "Skillscript.h"
#include "GameResourceManager.h"
#include "SkillManager.h"
#include "SkillStepListWindow.h"
#include "DragWindow.h"
#include "QuickManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "skillicon.h"
#include "TipWindow.h"
#include "SkillSheetWindow.h"

cSkillWindow::cSkillWindow()
: mpSkillPoint(0)
, mpSkillUsePoint(0)
, mBackupPos(0, 0)
, mpSkillStepListWindow(0)
, mImageSize(0, 0)
, mpVisibleStepWinCheckBtn(0)
{
	mpCommuButton = 0;
	mpCommuSheetWindow = 0;

	mJobTabCount = 0;

	mIsBackup = false;
		
	mCommuBtnBaseRect.Set(0, 0, 0, 0);

	::ZeroMemory( mpSkillPointImg, sizeof(mpSkillPointImg) );
}

cSkillWindow::~cSkillWindow()
{
	if( mpSkillPointImg )
		::ZeroMemory( mpSkillPointImg, sizeof(mpSkillPointImg) );
}

void cSkillWindow::Clear()
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		p->Clear();
		RemoveChildComplete( p );
	}
	mTabSheetArray.Clear();

	for( unsigned int i=0; i<mTabButtonArray.GetSize(); i++ )
	{
		if( mTabButtonArray[i] == mpCommuButton )
			continue;

		cTabButton* p = (cTabButton*)mTabButtonArray[i];
		if( p == 0 ) continue;

		RemoveChildComplete( p );
	}
	mTabButtonArray.Clear();

	mJobTabCount = 0;

	UpdateCommuButtonPos();
	AddTab( mpCommuButton );
	AddSheet( mpCommuSheetWindow );

	if( mpSkillStepListWindow )
		mpSkillStepListWindow->Clear();

	
}

void cSkillWindow::Open()
{

}

void cSkillWindow::Close()
{
	cTabWindow::Close();

	Clear();

	UIMAN->HideTip();
	UIMAN->HideDrag( this );

	/// npc 상점창이 열려있으면 닫기
	if( HERO->IsOpenNpcTalk() == true )
		HERO->CloseNpcTalk( false );

	ApplyBackupPos();
}

void cSkillWindow::OnShow()
{
	cTabWindow::OnShow();

	int size = mTabButtonArray.GetSize();
	if( size > 0)
		SetCurrent( size-1 );

	ShowSkillUpButton();
}

void cSkillWindow::OnHide()
{
	cTabWindow::OnHide();

	if( mpSkillStepListWindow )
		mpSkillStepListWindow->Hide();

	UIMAN->HideTip();
	UIMAN->HideDrag( this );

	/// npc 상점창이 열려있으면 닫기
	if( HERO->IsOpenNpcTalk() == true )
		HERO->CloseNpcTalk();

	ApplyBackupPos();
}

void cSkillWindow::InitSkillData()
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		p->InitSkillData();
	}

	// 스킬 초기화시 스킬 단계별 리스트의 모든 원소는 제거 시킨다.
	if( mpSkillStepListWindow )
		mpSkillStepListWindow->Clear();

	UpdateSkillPoint();
	UpdateSkillUsePoint();
}

void cSkillWindow::UpdateSkin()
{
	cTabWindow::UpdateSkin();

	if( mpSkillStepListWindow )
	{
		const cUIRect& rect = mpSkillStepListWindow->GetRelativeRect();
		mpSkillStepListWindow->SetRelativePos( cUIPos(rect.mLeft, mRelativeRect.GetHeight()-rect.GetHeight()) );
	}
	
	int size = mTabButtonArray.GetSize();
	if( size > 0)
		SetCurrent( size-1 );

}


unsigned int cSkillWindow::AddJobTab( unsigned long jobIdx )
{
	cString str = "JobTabbutton";
	cTabButton* btn = new cTabButton;
	if( btn->CreateBySkinName( str, this, 1 ) == false )
	{
		assert(0);
		return 0;
	}

	///
	mCommuBtnBaseRect = btn->GetRelativeRect();
	btn->SetRelativePos( cUIPos( mCommuBtnBaseRect.mLeft + mCommuBtnBaseRect.GetWidth() * mJobTabCount, mCommuBtnBaseRect.mTop) );

	btn->SetText( GAMERESOURCEMAN->GetJobName( jobIdx ) );
	AddTab( btn );
	UIMAN->GetRootNode()->AddProcessChild( btn );

	mJobTabCount++;

	UpdateCommuButtonPos();

	///
	cSkillSheetWindow* sheet = new cSkillSheetWindow;
	if( sheet->CreateBySkinName( "skillSheet", this, (unsigned int)jobIdx ) == false )
	{
		assert(0);
		return 0;
	}

	cSkillTree* pTree = SKILLSCRIPT->GetSkillTree( jobIdx );
	assert(pTree);
	if( pTree )
	{
		sheet->InitSkillSheetInfo( jobIdx, pTree );

		// 스킬 단계별 표시 리스트 윈도우 생성 및 Data 초기화
		if( mpSkillStepListWindow )
			mpSkillStepListWindow->ResetSkillInfo();
	}

	AddSheet( sheet );
	UIMAN->GetRootNode()->AddProcessChild( sheet );

	SetCurrent( mJobTabCount );

	return mTabButtonArray.GetSize();
}

/// 주의사항 : 
bool cSkillWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cTabWindow::OnCreate( pproperty ) == false )
		return false;
	
	mpSkillPoint = (cLabel*)GetChild( eUIID_GAME_SKILL_POINT );
	mpSkillUsePoint = (cLabel*)GetChild( eUIID_GAME_SKILL_USEPOINT );
	mpSkillStepListWindow = (cSkillStepListWindow*)GetChild( eUIID_GAME_SKILL_STEPWINDOW );
	mpVisibleStepWinCheckBtn = (cCheckBox*)GetChild( eUIID_GAME_SKILL_CHECKBOX );
	mpSkillPointImg[eSKILL_USE_IMG] = (cUINode*)GetChild( eUIID_GAME_SKILL_USEPOINTIMG );
	mpSkillPointImg[eSKILL_REST_IMG] = (cUINode*)GetChild( eUIID_GAME_SKILL_POINTIMG );
	
	if( mpVisibleStepWinCheckBtn )
	{
		mpVisibleStepWinCheckBtn->SetPress( true );
	}

	if( mpSkillStepListWindow )
	{
		const cUIRect& rect = mpSkillStepListWindow->GetRelativeRect();
		mpSkillStepListWindow->SetRelativePos( cUIPos(rect.mLeft, mRelativeRect.GetHeight()-rect.GetHeight()) );
	}

	// 스킬 사용, 남은 SP 출력부분 이미지
	for( unsigned char i = 0; i < eSKILL_IMG_MAX; i++ )
	{
		if( mpSkillPointImg[ i ] )
		{
			mpSkillPointImg[ i ]->Hide();
		}
	}

	cString str = "JobTabbutton";
	mpCommuButton = new cTabButton;
	if( mpCommuButton->CreateBySkinName( str, this, 1 ) == false )
	{
		assert(0);
		return false;
	}
	mpCommuButton->SetText( UIMAN->GetUIText(24) );
	UpdateCommuButtonPos();
	AddTab(mpCommuButton);

	mpCommuSheetWindow = new cCommuSheetWindow;
	if( mpCommuSheetWindow->CreateBySkinName( "communitySheet", this, 2 ) == false )
	{
		assert(0);
		return false;
	}
	AddSheet( mpCommuSheetWindow );

	// 초기 이미지 사이즈 셋팅
	mImageSize.mWidth	= mRelativeRect.GetWidth();
	mImageSize.mHeight	= mRelativeRect.GetHeight();
	if( mpImage )
	{
		cUIRect imageRect;
		imageRect.mLeft		= mAbsoluteRect.mLeft;
		imageRect.mTop		= mAbsoluteRect.mTop;
		imageRect.mRight	= imageRect.mLeft + mImageSize.mWidth;
		imageRect.mBottom	= imageRect.mTop + mImageSize.mHeight;

		mpImage->SetScreenRect( imageRect );
	}
	return true;
}

void cSkillWindow::UpdateRect()
{	
	cUIWindow::UpdateRect();

	if( mpImage )
	{
		cUIRect imageRect;
		imageRect.mLeft		= mAbsoluteRect.mLeft;
		imageRect.mTop		= mAbsoluteRect.mTop;
		imageRect.mRight	= imageRect.mLeft + mImageSize.mWidth;
		imageRect.mBottom	= imageRect.mTop + mImageSize.mHeight;
		
		mpImage->SetScreenRect( imageRect );
	}
}

/// 주의사항 : 
void cSkillWindow::OnCommand( cUINode*, unsigned int id )
{
	/// 클로즈버튼누르면 안보임.
	if( id == eUIID_GAME_SKILL_CLOSE_BUTTON )
	{
		Hide();
	}
}

void cSkillWindow::OnIconDragged( cUINode* caller, unsigned int /*id*/, const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon == 0 )
		return;

	if( icon->GetIconType() == eICON_SKILL )
	{
		UIMAN->ShowDrag( this, DRAG_FROM_SKILLWINDOW, icon, pos, shift );
	}
	else if( icon->GetIconType() == eICON_COMMUNITY )
	{
		UIMAN->ShowDrag( this, DRAG_FROM_COMMUNITYWINDOW, icon, pos, shift );
	}
	else
	{
		assert(0);
	}
}

void cSkillWindow::OnCheckBoxChanged( cUINode* caller, unsigned int id )
{
	cCheckBox* checkbox = (cCheckBox*)caller;

	if( !checkbox ) return;

	switch( id )
	{
	case eUIID_GAME_SKILL_CHECKBOX:
		
		{
			if( checkbox->IsPress() )
			{
				if( mpSkillStepListWindow && mpCommuSheetWindow != GetCurrentSheet() )
				{
					SetRelativeSize( cUISize( mImageSize.mWidth+mpSkillStepListWindow->GetAbsoluteRect().GetWidth(), mRelativeRect.GetHeight()));
					mpSkillStepListWindow->Show();					
				}
			}
			else
			{
				SetRelativeSize( mImageSize );

				if( mpSkillStepListWindow )
					mpSkillStepListWindow->Hide();	
			}
			
		}

		break;
	}

	
}

void cSkillWindow::UpdateSkillStep( unsigned long idx, unsigned char step )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		cSkillIcon* icon = p->GetSkillIcon( idx );
		if( icon == 0 ) continue;

		assert( icon->GetSkillIdx() == idx );

		icon->SetSkillInfo( idx, step, p->GetJobIdx(), eTIP_SKILL );
		icon->SetUsedFlag( true );
		icon->SetColor( NiColorA::WHITE );
	}
}

/// 오라스킬 세팅용
void cSkillWindow::SetOnAura( unsigned long skillIdx, unsigned long AuraUniqueIdx, bool on )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		cSkillIcon* icon = p->GetSkillIcon( skillIdx );
		if( icon == 0 ) continue;

		/// 오라스킬 세팅
		icon->SetAuraUniqueIdx( AuraUniqueIdx );
		icon->SetAuraOn( on );
		QUICKMAN->SetSkillAniAura( skillIdx, icon->GetAniParam() );
	}
}

unsigned long cSkillWindow::GetAuraUniqueIdx( unsigned long idx )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		cSkillIcon* icon = p->GetSkillIcon( idx );
		if( icon == 0 ) continue;

		if( icon->IsAuraOn() == false )
			return 0;

		return icon->GetAuraUniqueIdx();
	}
	return 0;
}


void cSkillWindow::ClearCoolTime( unsigned long exceptSkillIdx )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) 
			continue;

		p->ClearSkillCoolTime( exceptSkillIdx );
	}

	QUICKMAN->ClearCoolTime( exceptSkillIdx );

	if( mpSkillStepListWindow )
		mpSkillStepListWindow->UpdateIConListInfo();
}

void cSkillWindow::UpdateSkillPoint()
{
	if( mpSkillPoint )
		mpSkillPoint->SetValue( HERO->GetSkillPointRemain() );
}

void cSkillWindow::UpdateSkillUsePoint()
{
	if( mpSkillUsePoint )
	{
		if( mCurrentSheet == 0 )
			return;
		if( mCurrentSheet == mpCommuSheetWindow )
		{
			mpSkillUsePoint->Clear();
			return;
		}

		unsigned long totalUsSP = 0;
		unsigned long jobIdx = ((cSkillSheetWindow*)mCurrentSheet)->GetJobIdx();
		unsigned int jobStep = GAMERESOURCEMAN->GetJobStep( jobIdx );

		/// 직업 변환시 사용 스킬포인트 재계산..
		tPointerHashMap<unsigned long, void*>* keepSkillMap = SKILLMAN->GetKeepSkillMap();
		if( keepSkillMap )
		{
			tPointerHashMap<unsigned long, void*>::cIterator iter = keepSkillMap->Begin();
			while( iter != keepSkillMap->End() )
			{
				unsigned long skillIdx = (*iter).mFirst;
				sKeepSkill* pKeepSkill = (sKeepSkill*)((*iter).mSecond);
				if( pKeepSkill == 0 )
				{
					assert(0);
					return;
				}
				++iter;

				sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
				if( pSkillInfo == 0 )
					continue;
				if( pSkillInfo->mJobStep != jobStep )
					continue;
				if( pSkillInfo->mStepCount <= pKeepSkill->mStep )
				{
					assert(0);
					continue;
				}

				for( unsigned int i=0;i<=pKeepSkill->mStep;i++ )
				{
					sPlayerSkillStepInfo* pSkillStep = &pSkillInfo->mpSetpInfoArray[i];
					if( pSkillStep->mPlayerLevel != 0 )
						totalUsSP += pSkillStep->mRequireSP;
				}
			}
		}
		mpSkillUsePoint->SetValue( totalUsSP );
	}
}

void cSkillWindow::UpdateCommuButtonPos()
{
	if( mpCommuButton == 0 )
		return;

	mpCommuButton->SetRelativePos( cUIPos( mCommuBtnBaseRect.mLeft + mCommuBtnBaseRect.GetWidth() * mJobTabCount, mCommuBtnBaseRect.mTop) );
}

void cSkillWindow::RequestCommuSkill( unsigned long skillIndex )
{
	if( mpCommuSheetWindow )
		mpCommuSheetWindow->RequestCommuSkill( skillIndex );
}

void cSkillWindow::RequestLifeCommand( unsigned long comIdx, LPCTSTR value1, LPCSTR value2 )
{
	if( mpCommuSheetWindow )
		mpCommuSheetWindow->RequestLifeCommand( comIdx, value1, value2 );
}


void cSkillWindow::NetUnLockCommu()
{
	if( mpCommuSheetWindow )
		mpCommuSheetWindow->NetUnLock();
}

void cSkillWindow::SetCurrent( unsigned int index )
{
	if( !mVisible )
		return;

	cTabWindow::SetCurrent( index );

	UpdateSkillUsePoint();
	
	if( mpSkillStepListWindow  )
	{
		// 스킬 직업 관련 탭
		if( mpCommuSheetWindow != GetCurrentSheet() )
		{
			if( index <= 0 )
			{
				assert(0);
				return;
			}			

			mpSkillStepListWindow->SetCurrentJobStep( (unsigned char)(index - 1) );
			mpSkillStepListWindow->ResetSkillInfo();

			if( mpVisibleStepWinCheckBtn && mpVisibleStepWinCheckBtn->IsPress() )
			{				
				SetRelativeSize( cUISize( mImageSize.mWidth+mpSkillStepListWindow->GetAbsoluteRect().GetWidth(), mRelativeRect.GetHeight()));
				mpSkillStepListWindow->Show();
			}

			// 사용, 남은 SP 부분 이미지 Show
			for( unsigned char i = 0; i < eSKILL_IMG_MAX; i++ )
			{
				if( mpSkillPointImg[ i ] )
				{
					mpSkillPointImg[ i ]->Show();
				}
			}

			// 남은 SP
			if( mpSkillPoint )
				mpSkillPoint->Show();

			// 사용 SP
			if( mpSkillUsePoint )
				mpSkillUsePoint->Show();
		}
		// 커뮤니티 탭
		else
		{
			if( mpSkillStepListWindow->IsVisible() )
			{
				SetRelativeSize( mImageSize );			
				mpSkillStepListWindow->Hide();
			}

			// 사용, 남은 SP 부분 이미지 Hide
			for( unsigned char i = 0; i < eSKILL_IMG_MAX; i++ )
			{
				if( mpSkillPointImg[ i ] )
				{
					mpSkillPointImg[ i ]->Hide();
				}
			}

			// 남은 SP
			if( mpSkillPoint )
				mpSkillPoint->Hide();

			// 사용 SP
			if( mpSkillUsePoint )
				mpSkillUsePoint->Hide();
		}	
	}
}

void cSkillWindow::BackupWindowPos()
{
	cUIRect rt;
	mIsBackup = true;

	rt = GetAbsoluteRect();
	mBackupPos.mX = rt.mLeft;
	mBackupPos.mY = rt.mTop;
}

void cSkillWindow::ApplyBackupPos()
{
	if( mIsBackup == false )
		return;

	///
	SetRelativePos( mBackupPos );

	mIsBackup = false;
}

void cSkillWindow::ShowSkillUpButton()
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		p->ShowSkillUpButton();
	}
}

void cSkillWindow::SetPressStepListView( bool bView )
{
	if( mpVisibleStepWinCheckBtn )
	{
		mpVisibleStepWinCheckBtn->SetPress( bView );
	}
	else
	{
		assert(0);
	}
}

bool cSkillWindow::IsShowStepListWin()
{
	if( !mpVisibleStepWinCheckBtn )
	{
		assert(0);
		return false;		
	}
	
	return mpVisibleStepWinCheckBtn->IsPress();	
}

void cSkillWindow::OnNetLock( int lockTry )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p )
			p->SetEnabled( false );
	}

	if( mpVisibleStepWinCheckBtn )
		mpVisibleStepWinCheckBtn->NetLock( lockTry );
}

void cSkillWindow::OnNetUnLock( int lockTry )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p )
			p->SetEnabled( true );
	}

	if( mpVisibleStepWinCheckBtn )
		mpVisibleStepWinCheckBtn->NetUnLock( lockTry );
}

void cSkillWindow::StartCoolTime( unsigned long idx, unsigned long restTime, unsigned long totalTime )
{
	for( unsigned int i=0; i<mTabSheetArray.GetSize(); i++ )
	{
		if( mTabSheetArray[i] == mpCommuSheetWindow )
			continue;

		cSkillSheetWindow* p = (cSkillSheetWindow*)mTabSheetArray[i];
		if( p == 0 ) continue;

		cSkillIcon* icon = p->GetSkillIcon( idx );
		if( icon == 0 ) continue;

		if( restTime == 0 || totalTime == 0 )
			continue;

		icon->StartDelay( totalTime );

		if( totalTime < restTime )
			icon->SetElapsedTime( 0 );
		else
			icon->SetElapsedTime( totalTime - restTime );

		if( restTime == totalTime )
			QUICKMAN->StartSkillCoolTime( idx, totalTime );
	}

	if( mpSkillStepListWindow )
		mpSkillStepListWindow->UpdateIConListInfo();
}
