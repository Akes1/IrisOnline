#include "stdafx.h"
#include "IntroStage.h"
#include "IntroUIManager.h"
#include "StageManager.h"

#include "Application.h"
#include "renderSystem.h"
#include "GameResourceManager.h"

#include "UIRootNode.h"
#include "IntroWindow.h"

cIntroStage::cIntroStage()
: mpIntroUIManager(0)
{
	mViewState = eVIEW_NONE;
	mAlpha = 0.0f;
}

cIntroStage::~cIntroStage()
{
}

/// 주의사항 : 
void cIntroStage::Init( cStageManager* pStageManager )
{	
	mpStageManager = pStageManager;
}
 
/// 주의사항 : 
void cIntroStage::InitStage()
{
	if( mInit )	return;

	/// UI
	mpIntroUIManager = new cIntroUIManager;
	mpIntroUIManager->Init();

	mInit = true;
}

/// 주의사항 : 
void cIntroStage::Exit()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	if( mpIntroUIManager )
	{
		mpIntroUIManager->Exit();
		delete mpIntroUIManager;
		mpIntroUIManager = 0;
	}

	mpStageManager = NULL;
}

/// 주의사항 : 
void cIntroStage::Open()
{
	/// ui
	mpIntroUIManager->Open();
	
	mViewState = eVIEW_GRADE;
	mAlpha = 4.0f;

	if( GAMERESOURCEMAN->LoadInitData1() == false )
	{
		assert(0);
	}
}

/// 주의사항 : 
void cIntroStage::Close()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	/// UI
	if( mpIntroUIManager )
		mpIntroUIManager->Close();
}

/// 주의사항 : 
void cIntroStage::Process( unsigned long deltaTime, unsigned long /*accumTime*/ )
{
	unsigned int flag = GAMERESOURCEMAN->CheckInitData();
	if( flag == eData_Error )
	{
		assert(0);
		NiMessageBox( "Failed load gameData. Please reinstall client or patch.", "GameData Error" );
		::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
		::PostQuitMessage(0);
		return;
	}

	cIntroWindow* win = mpIntroUIManager->GetBackWindow();
	if( win == 0 )
	{
		assert(0);

		if( flag == eData_End )
		{
			mpStageManager->ChangeStage( eStage_Login );
			mViewState = eVIEW_END;
		}
		return;
	}

	switch( mViewState )
	{
	case eVIEW_GRADE:
		{
			if( mAlpha == 0.0f )
			{
				if( flag == eData_End )
				{
					mViewState = eVIEW_INTRO;
					mAlpha = 4.0f;
					win->SetGradeTexture(0);

					mpStageManager->ChangeStage( eStage_Login );

					UIMAN->GotoFrontNode( (cUINode*)win );
				}
				break;
			}

			float val = deltaTime*0.0008f;
			if( mAlpha <= val )
				mAlpha = 0.0f;
			else
				mAlpha -= val;

			float alpha = 1.0f;
			if( mAlpha < 1.0f )
				alpha = mAlpha;
	
			win->SetAlphaGrade( alpha );
			if( alpha <= 1.0f )
				win->SetAlpha( 1.0f );
		}
		break;
	case eVIEW_INTRO:
		{
			UIMAN->GotoFrontNode( (cUINode*)win );

			float val = deltaTime*0.0008f;
			if( mAlpha <= val )
				mAlpha = 0.0f;
			else
				mAlpha -= val;

			float alpha = 1.0f;
			if( mAlpha < 1.0f )
				alpha = mAlpha;

			win->SetAlpha( alpha );
			if( mAlpha == 0.0f )
			{
				mViewState = eVIEW_END;
				win->Hide();
				win->SetBackTexture(0);
			}
		}
		break;
	case eVIEW_END:
		break;
	}
}

void cIntroStage::ClearBackImage()
{
	cIntroWindow* back = mpIntroUIManager->GetBackWindow();
	if( back )
	{
		back->SetBackTexture( 0 );
		back->SetGradeTexture( 0 );
	}
}

bool cIntroStage::IsProcessEnd()
{
	return (mViewState == eVIEW_END);
}


/// 주의사항 : 
void cIntroStage::NetworkMsgParser( unsigned long /*connectionIdx*/, char* /*pMsg*/, unsigned long /*length*/ )
{
}
