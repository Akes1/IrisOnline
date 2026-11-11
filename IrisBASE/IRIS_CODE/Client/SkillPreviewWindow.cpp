#include "stdafx.h"
#include "SkillPreviewWindow.h"
#include "Label.h"
#include "ComboBox.h"
#include "GameResourceManager.h"
#include "TabButton.h"
#include "SkillPreviewSheet.h"
#include "UIRootNode.h"
#include "SkillScript.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "SkillManager.h"

cSkillPreviewWindow::cSkillPreviewWindow()
: mpUsePoint(0)
, mpLimitLevel(0)
{
	memset( mpJobCombo, 0, sizeof(mpJobCombo) );
	mTabBaseRect.Set( 0, 0, 0, 0 );
}

cSkillPreviewWindow::~cSkillPreviewWindow()
{
}

void cSkillPreviewWindow::Close()
{
	Clear();
}

void cSkillPreviewWindow::OnHide()
{
	cTabWindow::OnHide();

	Clear();

	UIMAN->HideTip();
	HERO->CloseNpcTalk();
}

void cSkillPreviewWindow::Clear()
{
	// 콤보박스 클리어
	for( int i = 0; i < MAX_JOBCOMBO; ++i )
	{
		if( mpJobCombo[i] )
			mpJobCombo[i]->Clear();
	}

	//
	mJobIndexList.Clear();

	HideJobTab( 0 );

	// 데이터 삭제
	typedef tPointerHashMap<unsigned long, void*> cKeepSkillMap;

	cJobSkillMap::cIterator i = mJobSkillMap.Begin();
	cJobSkillMap::cIterator end = mJobSkillMap.End();
	for( ; i != end; ++i )
	{
		sPreviewData* data = (sPreviewData*)(i->mSecond);
		SAFE_DELETE( data );
	}
	mJobSkillMap.Clear();

	//
	cKeepSkillMap::cIterator di = mKeepSkillMap.Begin();
	cKeepSkillMap::cIterator dend = mKeepSkillMap.End();
	for( ; di != dend; ++di )
	{
		sSkillData* skillData = (sSkillData*)(di->mSecond);
		if( skillData )
		{
			skillData->skillSet.Clear();
		}
		SAFE_DELETE( skillData );
	}
	mKeepSkillMap.Clear();

	//
	SetLevel( 0 );
	SetUsePoint( 0 );
}

void cSkillPreviewWindow::Show( unsigned long jobindex1, unsigned long jobindex2 )
{
	if( !mpJobCombo[0] )
	{
		assert(0);
		return;
	}

	cTabWindow::Show( true );

	unsigned long first = jobindex1 / 1000;
	unsigned long second = jobindex2 / 1000;

	// 해당 기본직업과 관련있는 직업만 따로 추리기
	typedef tPointerHashMap<unsigned long, void*> cJopInfoMap;
	cJopInfoMap* jobInfoMap = GAMERESOURCEMAN->GetJobInfoMap();
	if( jobInfoMap && jobInfoMap->IsEmpty() == false )
	{
		cJopInfoMap::cIterator i = jobInfoMap->Begin();
		cJopInfoMap::cIterator end = jobInfoMap->End();
		for( ; i != end; ++i )
		{
			unsigned long index = (unsigned long)(i->mFirst);
			sJobInfo* info = (sJobInfo*)(i->mSecond);
			if( info )
			{
				unsigned long value = index / 1000;
				if( value == first || value == second )
				{
					mJobIndexList.PushBack( index );
				}
			}
			else
				assert(0);
		}

		///
		mJobIndexList.Sort();
	}

	// 다른 콤보는 기본으로 세팅
	for( int i = 1; i < MAX_JOBCOMBO; ++i )
	{
		if( mpJobCombo[i] )
		{
			mpJobCombo[i]->AddItem( UIMAN->GetUIText( 160 ), (void*)0 );
			mpJobCombo[i]->SetSelect( 0, false );
			mpJobCombo[i]->SetEnabled( false );
		}
	}

	// 기본직업 2개 세팅
	LPCTSTR jobname1 = GAMERESOURCEMAN->GetJobName( jobindex1 );
	LPCTSTR jobname2 = GAMERESOURCEMAN->GetJobName( jobindex2 );

	mpJobCombo[0]->AddItem( jobname1, (void*)jobindex1 );
	mpJobCombo[0]->AddItem( jobname2, (void*)jobindex2 );
	mpJobCombo[0]->SetSelect( 0 );

	// 스킬동기화 버튼 - hero의 직업과 관련된 직업군일때만 사용가능하게 한다
	if( mpSyncSkill )
	{
		mpSyncSkill->Hide();

		unsigned long heroJobIndex = HERO->GetJob();
		if( heroJobIndex == jobindex1 || heroJobIndex == jobindex2 )
		{
			mpSyncSkill->Show();
		}
		else
		{
			tArray<unsigned long>* parray = GAMERESOURCEMAN->GetParentJob( heroJobIndex );
			if( parray && parray->IsEmpty() == false )
			{
				unsigned long defaultJob = (unsigned long)(*parray)[0];
				if( defaultJob == jobindex1 || defaultJob == jobindex2 )
					mpSyncSkill->Show();
				else
					mpSyncSkill->Hide();
			}
		}
	}
}
 
bool cSkillPreviewWindow::OnCreate( cUINodeProperty* property )
{
	if( cTabWindow::OnCreate( property ) == false )
		return false;

	mpUsePoint = (cLabel*)GetChild( eUIID_SKILLPREVIEW_USEPOINT );
	mpLimitLevel = (cLabel*)GetChild( eUIID_SKILLPREVIEW_LEVEL );
	mpSyncSkill = GetChild( eUIID_SKILLPREVIEW_SYNC );
	mpJobCombo[0] = (cComboBox*)GetChild( eUIID_SKILLPREVIEW_DEFAULTJOB );
	mpJobCombo[1] = (cComboBox*)GetChild( eUIID_SKILLPREVIEW_CHANGEJOB1 );

	for( int i = 0; i < MAX_JOBCOMBO; ++i )
	{
		cString str = "Preview_JobTabbutton";
		cTabButton* p = new cTabButton;
		if( p->CreateBySkinName( str, this, i+1 ) == false )
		{
			assert(0);
			return false;
		}

		// 위치 설정
		mTabBaseRect = p->GetRelativeRect();

		cUIPos pos;
		pos.mX = mTabBaseRect.mLeft + mTabBaseRect.GetWidth() * i;
		pos.mY = mTabBaseRect.mTop;
		p->SetRelativePos( pos );

		// 버튼 추가
		AddTab( p );
		UIMAN->GetRootNode()->AddProcessChild( p );

		// 시트 생성
		cSkillPreviewSheet* sheet = new cSkillPreviewSheet;
		if( sheet->CreateBySkinName( "Preview_SkillSheet", this, i+1 ) == false )
		{
			assert(0);
			return false;
		}

		// 시트 추가
		AddSheet( sheet );
		UIMAN->GetRootNode()->AddProcessChild( sheet );
	}
	return true;
}

void cSkillPreviewWindow::OnCommand( cUINode* node, unsigned int id )
{
	switch( id )
	{
	case eUIID_SKILLPREVIEW_CLOSE:
		Hide();
		break;
	case eUIID_SKILLPREVIEW_CLEAR:
		{
			// 전체 초기화
			typedef tPointerHashMap<unsigned long, void*> cKeepSkillMap;

			cJobSkillMap::cIterator i = mJobSkillMap.Begin();
			cJobSkillMap::cIterator end = mJobSkillMap.End();
			for( ; i != end; ++i )
			{
				sPreviewData* data = (sPreviewData*)(i->mSecond);
				if( data )
				{
					data->highLevel = 0;
					data->jobPoint = 0;
				}
			}

			cKeepSkillMap::cIterator di = mKeepSkillMap.Begin();
			cKeepSkillMap::cIterator dend = mKeepSkillMap.End();
			for( ; di != dend; ++di )
			{
				sSkillData* skillData = (sSkillData*)(di->mSecond);
				if( skillData )
				{
					skillData->step = UCHAR_MAX;
					skillData->skillSet.Clear();
				}
			}

			SetLevel( 0 );
			SetUsePoint( 0 );

			// 시트 초기화
			for( unsigned int i = 0; i < mTabSheetArray.GetSize(); i++ )
			{
				cSkillPreviewSheet* p = (cSkillPreviewSheet*)mTabSheetArray[i];
				if( p ) 
					p->InitSkillTree();
			}
		}
		break;
	case eUIID_SKILLPREVIEW_SYNC:
		// 현재 스킬과 동기화
		SyncHeroSkill();
		break;
	}
}

//
void cSkillPreviewWindow::OnComboSelected( cUINode* caller, unsigned int id )
{
	if( caller == 0 )
	{
		assert(0);
		return;
	}

	cComboBox* jobCombo = (cComboBox*)caller;
	unsigned long jobIndex = (unsigned long)jobCombo->GetSelectData();

	unsigned int jobStep = 0;
	switch( id )
	{
	case eUIID_SKILLPREVIEW_DEFAULTJOB:	jobStep = 0; break;
	case eUIID_SKILLPREVIEW_CHANGEJOB1:	jobStep = 1; break;
	default:							assert(0);	 return;
	}

	// 설정
	HideJobTab( jobStep );
	ShowJobTab( jobStep, jobIndex );

	// 콤보 갱신 - 선택 초기화 ( 1차 이상은 안보이게한다)
	if( jobStep + 1 > 1 )
		return;

	UpdateJobCombo( jobStep + 1 );
	mpJobCombo[jobStep + 1]->SetSelect( 0 );
}

void cSkillPreviewWindow::HideJobTab( unsigned int jobStep )
{
	if( mTabSheetArray.GetSize() < jobStep )
		return;

	// 탭버튼, 탭시트
	for( unsigned int i = jobStep; i < mTabSheetArray.GetSize(); i++ )
	{
		cSkillPreviewSheet* p = (cSkillPreviewSheet*)mTabSheetArray[i];
		if( p ) 
		{
			p->Hide();
			p->Clear();
		}
	}

	for( unsigned int i = jobStep; i < mTabButtonArray.GetSize(); i++ )
	{
		cTabButton* p = (cTabButton*)mTabButtonArray[i];
		if( p ) 
			p->Hide();
	}
}

void cSkillPreviewWindow::ShowJobTab( unsigned int jobStep, unsigned long jobIndex )
{
	cSkillTree* tree = SKILLSCRIPT->GetSkillTree( jobIndex );
	if( tree )
	{
		// 새탭 보이기
		cSkillPreviewSheet* sheet = (cSkillPreviewSheet*)mTabSheetArray[jobStep];
		cTabButton* button = (cTabButton*)mTabButtonArray[jobStep];
		if( sheet && button )
		{
			// 버튼 세팅
			LPCTSTR jobname = GAMERESOURCEMAN->GetJobName( jobIndex );
			button->SetText( jobname );

			// 스킬트리 셋팅
			sheet->SetSkillTree( jobIndex, tree );

			SetCurrent( jobStep );
			button->Show();
		}

	}
	else
	{
		if( jobStep > 0 )
			SetCurrent( jobStep - 1);
	}

}

void cSkillPreviewWindow::UpdateJobCombo( int step )
{
	// 범위검사
	if( !(step > 0 && step < MAX_JOBCOMBO) )
		return;

	// 콤보 목록 재 세팅
	if( mJobIndexList.IsEmpty() == false )
	{
		mpJobCombo[step]->Clear();
		mpJobCombo[step]->AddItem( UIMAN->GetUIText( 160 ), (void*)0 );
		mpJobCombo[step]->SetSelect( 0, false );
		mpJobCombo[step]->SetEnabled( false );
		
		// 직업인덱스
		unsigned long selectJobIndex = (unsigned long)mpJobCombo[step-1]->GetSelectData();
		unsigned long selectDefaultJob = selectJobIndex;

		tArray<unsigned long>* parentJobArr = GAMERESOURCEMAN->GetParentJob( selectJobIndex );
		if( parentJobArr )
			selectDefaultJob = (*parentJobArr)[0];

		cJobIndexList::cIterator i = mJobIndexList.Begin();
		cJobIndexList::cIterator end = mJobIndexList.End();
		for( ; i != end; ++i )
		{
			unsigned long jobIndex = (unsigned long)(*i);
			unsigned long defaultJob = jobIndex;

			tArray<unsigned long>* parentJobArr = GAMERESOURCEMAN->GetParentJob( jobIndex );
			if( parentJobArr )
				defaultJob = (*parentJobArr)[0];

			// 기본직업이 동일한 직업 리스트만 삽입
			if( defaultJob != selectDefaultJob )
				continue;

			// 전직 단계 검사
			unsigned short jobStep = (short)(( jobIndex - defaultJob ) / 100);
			if( jobStep != step )
				continue;

			// 1차 전직이면 전부 삽입 그외 전직은 앞 단계와 연결 되는 내용만 삽입
			if( step != 1 )
			{
				short selectValue = (short)( selectJobIndex % 10 );
				short value = (short)( jobIndex % 10 );
				if( selectValue != value )
					continue;
			}

			// 동일 전직 차수만 삽입
			LPCTSTR jobname = GAMERESOURCEMAN->GetJobName( jobIndex );
			mpJobCombo[step]->AddItem( jobname, (void*)jobIndex );
			mpJobCombo[step]->SetEnabled( true );
		}
	}
}

void cSkillPreviewWindow::SyncHeroSkill()
{
	// 데이터 초기화
	typedef tPointerHashMap<unsigned long, void*> cKeepSkillMap;
	cKeepSkillMap::cIterator di = mKeepSkillMap.Begin();
	cKeepSkillMap::cIterator dend = mKeepSkillMap.End();
	for( ; di != dend; ++di )
	{
		//unsigned long skillIndex = (unsigned long)(di->mFirst);
		sSkillData* skillData = (sSkillData*)(di->mSecond);
		if( skillData )
		{
			skillData->step = UCHAR_MAX;
			skillData->skillSet.Clear();
		}
	}

	typedef tPointerHashMap<unsigned long, void*> cHeroKeepSkillMap;
	cHeroKeepSkillMap* keepSkillMap = SKILLMAN->GetKeepSkillMap();
	if( keepSkillMap )
	{
		cHeroKeepSkillMap::cIterator i = keepSkillMap->Begin();		
		cHeroKeepSkillMap::cIterator end = keepSkillMap->End();
		for( ; i != end; ++i )
		{
			unsigned long skillIndex = (unsigned long)(i->mFirst);
			sKeepSkill* pKeepSkill = (sKeepSkill*)(i->mSecond);

			if( pKeepSkill )
			{
				// 스텝정보 갱신
				sSkillData* data = (sSkillData*)mKeepSkillMap.GetAt( skillIndex );
				if( data )
				{
					sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
					if( info == 0 )
					{
						assert(0);
						continue;
					}

					sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[pKeepSkill->mStep];

					// 선행스킬 관련 데이터 갱신
					if( stepInfo.mLearnSkillIdx1 > 0 )
					{
						sSkillData* learnData = GetSkillData( stepInfo.mLearnSkillIdx1 );
						if( learnData )
						{
							learnData->skillSet.Insert( skillIndex );
						}
						else
							assert(0);
					}

					if( stepInfo.mLearnSkillIdx2 > 0 )
					{
						sSkillData* learnData = GetSkillData( stepInfo.mLearnSkillIdx2 );
						if( learnData )
						{
							learnData->skillSet.Insert( skillIndex );
						}
						else
							assert(0);
					}

					// 스탭 갱신
					data->step = pKeepSkill->mStep;
				}
			}
		}
	}

	for( unsigned int i = 0; i < mTabSheetArray.GetSize(); i++ )
	{
		cSkillPreviewSheet* p = (cSkillPreviewSheet*)mTabSheetArray[i];
		if( p ) 
			p->SyncSkillTree();
	}
}

sPreviewData* cSkillPreviewWindow::GetPreviewData( unsigned long jobIndex )
{
	if( mJobSkillMap.IsEmpty() == true )
		return 0;

	return (sPreviewData*)mJobSkillMap.GetAt( jobIndex );
}

sSkillData* cSkillPreviewWindow::GetSkillData( unsigned long skillIndex )
{
	return (sSkillData*)mKeepSkillMap.GetAt( skillIndex );
}

void cSkillPreviewWindow::InsertKeepSkill( unsigned long skillindex, unsigned char step )
{
	if( GetSkillData( skillindex ) == 0 )
	{
		sSkillData* p = new sSkillData;

		p->step = step;

		if( mKeepSkillMap.Insert( skillindex, p ) == false )
		{
			assert(0);
		}
	}
}

void cSkillPreviewWindow::UpdateSkillStep( unsigned long skillIndex, unsigned char step )
{
	sSkillData* skilldata = (sSkillData*)mKeepSkillMap.GetAt( skillIndex );
	if( skilldata )
	{
		skilldata->step = step;
	}
}

void cSkillPreviewWindow::SetLevel( char level )
{
	if( mpLimitLevel )
		mpLimitLevel->SetValue( level );
}

void cSkillPreviewWindow::SetUsePoint( long point )
{
	if( mpUsePoint )
		mpUsePoint->SetValue( point );
}

void cSkillPreviewWindow::CheckInfo( unsigned long jobIndex )
{
	unsigned long totalPoint = 0;
	char highLevel = 0;

	cSkillTree* tree = SKILLSCRIPT->GetSkillTree( jobIndex );
	if( tree )
	{
		for( unsigned int i = 0; i < tree->mIconCount; ++i )
		{
			unsigned long skillIndex = tree->mIconList[i].skillIdx;

			sSkillData* data = GetSkillData( skillIndex );
			if( !data )
				continue;

			if( data->step == UCHAR_MAX )
				continue;

			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
			if( info )
			{
				// 현재 단계까지 필요한 포인트 계산
				for( int step = 0; step < data->step + 1; ++step )
				{
					sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[step];
					totalPoint += stepInfo.mRequireSP;

					if( highLevel < stepInfo.mPlayerLevel )
						highLevel = stepInfo.mPlayerLevel;
				}
			}
		}
	}

	// 직업별 정보 갱신
	UpdatePreviewData( jobIndex, totalPoint, highLevel );
}

void cSkillPreviewWindow::UpdateInfoUI( unsigned long jobIndex )
{
	sPreviewData* data = GetPreviewData( jobIndex );
	if( data )
	{
		SetLevel( data->highLevel );
		SetUsePoint( data->jobPoint );
	}
}

bool cSkillPreviewWindow::UpdatePreviewData( unsigned long jobIndex, long jobPoint, char highLevel )
{
	sPreviewData* p = GetPreviewData( jobIndex );
	if( p == 0 )
	{
		p = new sPreviewData;
		if( mJobSkillMap.Insert( jobIndex, p ) == false )
		{
			SAFE_DELETE( p );
			assert(0);
			return false;
		}
	}

	p->jobPoint = jobPoint;
	p->highLevel = highLevel;
	return true;
}

unsigned long cSkillPreviewWindow::GetCurrentJobIndex()
{
	cSkillPreviewSheet* pSkillPrevSheet = (cSkillPreviewSheet*)GetCurrentSheet();
	if( pSkillPrevSheet && pSkillPrevSheet->GetParent() == this )
	{
		return pSkillPrevSheet->GetJobIndex();
	}
	else
	{
		assert(0);
		return 0;
	}
}
