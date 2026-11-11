#include "StdAfx.h"

#include "gameResourceManager.h"
#include "Skill_Common.h"
#include "SkillScript.h"
#include "SkillManager.h"
#include "SkillStepListWindow.h"
#include "SkillStepListBox.h"
#include "SkillExecuter.h"
#include "SkillIcon.h"
#include "SkillWindow.h"

#include "DragWindow.h"

#include "ObjectManager.h"
#include "Hero.h"


cSkillStepListWindow::cSkillStepListWindow( eUINodeType type )
: cUIWindow( type )
, mpSkillStepListBox(0)
, mCurrentJobStep(0)
{
	SetExceptSnap( true );

	mUpdateSkillInfo = false;
}

cSkillStepListWindow::~cSkillStepListWindow()
{
	Clear();
}

void cSkillStepListWindow::Clear()
{
	if( mpSkillStepListBox )
		mpSkillStepListBox->Clear();

	mUpdateSkillInfo = false;
}

bool cSkillStepListWindow::OnCreate( cUINodeProperty* pProperty )
{
	if( cUIWindow::OnCreate( pProperty ) == false )
		return false;

	mpSkillStepListBox = (cSkillStepListBox*)GetChild( eUIID_GAME_SKILL_STEPLISTBOX );

	if( !mpSkillStepListBox ) 
		return false;

	// 스킬 아이콘 N개 생성 Default : 10
	// 직업 갯수만 큼 리스트 영역을 분할하여 생성한다.	나중에 캐릭터의 현재 직업 갯수 상태를 얻어와서 셋팅하게 바꾼다.
	mpSkillStepListBox->CreateSplitSize( gSplitCount, gSplitSize ); 
	mpSkillStepListBox->CreateSkillICon();

	Hide();

	return true;
}

void cSkillStepListWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mUpdateSkillInfo == true )
	{
		UpdateSkill();
		mUpdateSkillInfo = false;
	}

	cUIWindow::OnProcess( deltaTime, accumTime );
}


bool cSkillStepListWindow::SetSkin( const cUINodeSkin* pskin )
{
	if( false == cUIWindow::SetSkin( pskin ) )
		return false;
	
	return true;
}

void cSkillStepListWindow::Open()
{

}

// 해당 직업
void cSkillStepListWindow::UpdateSkill()
{
	if( NULL == mpSkillStepListBox )
		return;

	// 해당 직업 영역의 스킬 부분만 클리어 한다.
	mpSkillStepListBox->ClearSplitRange(mCurrentJobStep);

	tPointerHashMap<unsigned long, void*>* haveSkillMap = SKILLMAN->GetKeepSkillMap();

	// 사용자의 보유 스킬 목록
	if( haveSkillMap != NULL )
	{
		cStringT strLvAndStep;	
		unsigned long skillidx				= 0;			
		sKeepSkill* pHaveSkill				= NULL;
		sPlayerSkillBaseInfo* pSkillInfo	= NULL;
		sPlayerSkillStepInfo* pSkillStep	= NULL;

		tPointerHashMap<unsigned long, void*>::cIterator iter		= haveSkillMap->Begin();
		tPointerHashMap<unsigned long, void*>::cIterator iterEnd	= haveSkillMap->End();

		// 보유 스킬 목록을 가져온다.
		while( iter != iterEnd )
		{
			// 보유한 스킬의 정보를 얻어 온다.
			skillidx = (*iter).mFirst;
			pHaveSkill = (sKeepSkill*)((*iter).mSecond);
			pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillidx ); 

			++iter;

			if( !pHaveSkill || !pSkillInfo )
			{
				assert(NULL);
				continue;				
			}			

			//==================================================================
			//  기본공격, 패시브, 오라 스킬 제외
			if( mCurrentJobStep != pSkillInfo->mJobStep )
				continue;
			if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
				continue;
			if( skillidx <= NORMAL_ATTACK_SKILL_MAX )
				continue;
			if( pSkillInfo->mShotType == eSHOTTYPE_AURA )
				continue;
			//==================================================================

			// 스킬 습득 단계만큼 컨테이너에 삽입
			for( unsigned char nStep = 0; nStep <= pHaveSkill->mStep; nStep++ )
			{
				sSkillInfoEx *pExSkillData	= new sSkillInfoEx;
				pExSkillData->mpSkillInfo	= pSkillInfo;
				pExSkillData->mSkillStep	= nStep;

				pSkillStep = &pSkillInfo->mpSetpInfoArray[nStep];
				
				// 스킬 이름 / 습득요구 레벨, 스킬단계
				LPCTSTR strSkillname = GAMERESOURCEMAN->GetSkillNameText( pSkillInfo->mNameIdx );
				int rowIdx = mpSkillStepListBox->AddRowAt( mCurrentJobStep, strSkillname, COLOR_WHITE, static_cast<void*>( pExSkillData ) );
				if( rowIdx != -1 )
				{
					strLvAndStep.Format( UIMAN->GetUIText( 107 ), pSkillStep->mPlayerLevel, nStep+1 );
					mpSkillStepListBox->AddColume( rowIdx, (LPCTSTR)strLvAndStep.Cstr(), COLOR_WHITE );
				}
				else
				{
					SAFE_DELETE(pExSkillData);
				}
				
			}
		}
	
		// 단계별 리스트 박스 정렬
		mpSkillStepListBox->Sort();

		// 스킬 아이콘 리스트 박스 Render Data 값 갱신
		mpSkillStepListBox->UpdateIConListInfo();
	}
}

// 현재 직업 인덱스 및 리스트 박스 분할 영역 설정
void cSkillStepListWindow::SetCurrentJobStep( unsigned char jobStep )
{
	if( mpSkillStepListBox )
	{
		mCurrentJobStep = jobStep;
		mpSkillStepListBox->SetCurrentSplitIndex( jobStep );
	}
}

// 캐릭터의 모든 스킬 개신
//void cSkillStepListWindow::UpdateAllSkill()
//{
//	for( unsigned char iJob = 0; iJob < mCntJobs; iJob++ )
//	{
//		mCurrentJobStep = iJob;
//		UpdateSkill();
//	}
//}

void cSkillStepListWindow::OnListBoxSelected( cUINode* pcaller, unsigned int id)
{
	OnIconDoubleClicked( pcaller, id );
}

void cSkillStepListWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
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

void cSkillStepListWindow::OnIconDoubleClicked(cUINode* pcaller, unsigned int id)
{
	cSkillIcon* p = (cSkillIcon*)pcaller;
	unsigned long skillIdx	= p->GetSkillIdx();
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
		{
			SKILLEXECUTER->ExecuteSkill( skillIdx, skillStep );						
		}
	}
}

// 리스트 박스의 스킬 아이콘 재 설정 요청
void cSkillStepListWindow::UpdateIConListInfo()
{
	if( mpSkillStepListBox )
		mpSkillStepListBox->UpdateIConListInfo();
}

