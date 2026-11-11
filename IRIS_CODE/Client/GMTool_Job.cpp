#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "Protocol_Cheat.h"
#include "Cheat_Common.h"
#include "ChatManager.h"

void cGMTool::OnShowJob()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_JOB];
	HWND hJobCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_JOB], IDC_COMBO_JOB_LIST );

	SendMessage( hJobCombo, CB_RESETCONTENT, 0, 0 );

	/// HERO 현재 직업 이름 셋팅 및 전직 할 수 있는 직업 셋팅
	if( HERO && GAMERESOURCEMAN )
	{
		unsigned long heroJob = (unsigned long)HERO->GetJob();
		LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( heroJob );
		if( jobName )
		{
			SetDlgItemText( hWnd, IDC_EDIT_JOB_NAME, jobName );
		}

		// 해당 기본직업과 관련있는 직업만 따로 추리기
		typedef tArray<unsigned long> cJobIndexArray;
		typedef tPointerHashMap<unsigned long, void*> cJopTree;
		cJopTree* jobTree = GAMERESOURCEMAN->GetJobTree();
		if( jobTree && jobTree->IsEmpty() == false )
		{
			cJopTree::cConstIterator i = jobTree->Begin();
			cJopTree::cConstIterator end = jobTree->End();
			for( ; i != end; ++i )
			{
				unsigned long   childJob  = (unsigned long)(i->mFirst); /// 전직 직업
				cJobIndexArray* jobArr = (cJobIndexArray*)(i->mSecond); /// 부모 직업 배열
							
				// if( jobArr )
				if( jobArr && jobArr->GetSize() == 1 ) // 2차 전직은 막아둠. [12/16/2009 Jo_Client]
				{		
					/// 바로 전 전직 직업만 체크
					int jobCounter = jobArr->GetSize(); /// 전직 회수 세기
					unsigned long parentJob = (*jobArr)[jobCounter-1];

					if( parentJob == heroJob )
					{
						LPCTSTR changeJobName = GAMERESOURCEMAN->GetJobName( childJob );
						if( changeJobName )
						{
							int retIndex = SendMessage( hJobCombo, CB_ADDSTRING, 0, (LPARAM)changeJobName );
							if( retIndex != CB_ERR && retIndex != CB_ERRSPACE )
							{
								SendMessage( hJobCombo, CB_SETITEMDATA, retIndex, childJob );
							}
						}
					}
				}
// 				else
// 					assert(0);
			}
		}
	}

	int comboCount = SendMessage( hJobCombo, CB_GETCOUNT, 0, 0 );
	if( comboCount > 0 )
		EnableWindow( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_JOB], IDC_BUTTON_JOB_CHANGE ), TRUE );
	else
		EnableWindow( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_JOB], IDC_BUTTON_JOB_CHANGE ), FALSE );

	SendMessage( hJobCombo, CB_SETCURSEL, 0, 0 );


	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_JOB].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_JOB].y = GetSystemMetrics(SM_CYCAPTION);

}

/// 아이템 관련 버튼 이벤트
void cGMTool::OnCommandJob( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_JOB_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_JOB], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_JOB_CHANGE:
		{
			HWND jobCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_JOB], IDC_COMBO_JOB_LIST );
			int listIndex = SendMessage( jobCombo, CB_GETCURSEL, 0, 0 );
			int  jobIndex = SendMessage( jobCombo, CB_GETITEMDATA, listIndex, 0 );
			unsigned long heroJob = 0;

			if( HERO )
				heroJob = (unsigned long)HERO->GetJob();

			if( GAMERESOURCEMAN->IsCorrelateJob( jobIndex, heroJob ) == true )
			{
				/// 메시지 날리기
				MSG_REQ_CHEAT_GM_JOB_CHANGE msg;
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_GM_JOB_CHANGE_REQ;
				msg.mChangeJobIndex = jobIndex;
			
				if( NETWORK->IsConnected() )
				{
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_JOB_CHANGE ) );
				}
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8088 ) );
			}

			::ShowWindow( mSubDlg[GMTOOL_SUBDLG_JOB], SW_HIDE );
			::SetActiveWindow( mParentWnd );
		}
		break;
	default:
		return;
	}
}
