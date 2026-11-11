#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "Protocol_Cheat.h"
#include "Notice_Common.h"
#include "UIPrintImageText.h"
#include "GameResourceManager.h"

#include <mbctype.h>

/// 공지관련
void cGMTool::OnShowNotice()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_NOTICE];

	SetDlgItemText( hWnd, IDC_RICHEDIT_TEXT, _T("") );
	SendMessage( GetDlgItem(hWnd, IDC_COMBO_NOTICETYPE), CB_SETCURSEL, 0, 0 );
	
	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_NOTICE].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_NOTICE].y = GetSystemMetrics(SM_CYCAPTION);

	SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_PATH, mCurrentPath );
	
	if( mIsNoticefileOpen == true )
	{
		mNoticeFileLoader.Close();
		mIsNoticefileOpen = false;
	}

	SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, 10, FALSE );

	mTimeInterval = 0;
	SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, mTimeInterval, false );
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, _T("") );
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_AUTO, _T(""));

	cStringT charCounter;
	charCounter.Format(_T("(0/%d)"), MSG_PRINT_LENGTH );
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_STATIC_NOTICE_CHAR_COUNTER, charCounter.Cstr() );
	SendDlgItemMessage(mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_LIST_NOTICE_FILELIST, LB_RESETCONTENT, NULL, NULL);

}

void cGMTool::OnCommandNotice( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_NOTICE_CANCEL:
	case IDC_BUTTON_NOTICE_CANCEL2:
		{
			::ShowWindow( mSubDlg[GMTOOL_SUBDLG_NOTICE], SW_HIDE );
			if( mIsNoticefileOpen == true )
			{
				mNoticeFileLoader.Close();
				mIsNoticefileOpen = false;
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, _T("") );
				SendDlgItemMessage(mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_LIST_NOTICE_FILELIST, LB_RESETCONTENT, NULL, NULL);
			}
			
			if( mIsNoticeTimerRun == true )
			{ 
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND);
				mIsNoticeTimerRun = false;
			}
			
			if( mIsCounterTimerRun == true )
			{
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
				mIsCounterTimerRun = false;
			}

			::SetActiveWindow( mParentWnd );
		}
		break;
	case IDC_EDIT_NOTICE_NORMAL:
		{
			if( msg == EN_CHANGE )
			{
				/// 내용 받기
				TCHAR str[MSG_PRINT_LENGTH] = {0,};
				int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_NORMAL, str, MSG_PRINT_LENGTH );
				if( retcode == false )
				{
					cStringT charCounter;
					charCounter.Format(_T("(0/%d)"), MSG_PRINT_LENGTH );
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_STATIC_NOTICE_CHAR_COUNTER, charCounter.Cstr() );
					return;
				}

				cStringT temp = str;
				temp.Replace( _T("\r\n"), _T(" ") );
				temp.Replace( _T("\t"), _T(" ") );
				temp.Replace( _T("\n"), _T(" ") );
				temp.Replace( _T("\r"), _T(" ") );
				
				cStringT charCounter;
				charCounter.Format(_T("(%d/%d)"), temp.GetLength(), MSG_PRINT_LENGTH );
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_STATIC_NOTICE_CHAR_COUNTER, charCounter.Cstr() );
			}
		}
		break;
	case IDC_BUTTON_NOTICESEND:		/// 공지 전송
		if( NETWORK->IsConnected() == true )
		{
			HWND hComDlg = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_COMBO_NOTICETYPE );
			int noticeType = SendMessage( hComDlg, CB_GETCURSEL, 0, 0 );
			
			/// 내용 받기
			TCHAR str[MSG_PRINT_LENGTH] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_NORMAL, str, MSG_PRINT_LENGTH );
			if( retcode == false )
				return;

			/// 공지 보내기
			SendNoticeMsg( noticeType, str );
	
			/// 초기화
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_NORMAL, _T("") );
		}
		break;
	case IDC_BUTTON_NOTICE_FILESEARCH:
		{

			char findPath[ MAX_PATH ] = { 0, };
			sprintf_s( findPath, "%s\\*.txt", mCurrentPath );

			DlgDirListA( mSubDlg[GMTOOL_SUBDLG_NOTICE], findPath, IDC_LIST_NOTICE_FILELIST, 0, (DDL_ARCHIVE | DDL_DRIVES | DDL_DIRECTORY | DDL_READWRITE ) );
		}
		break;

	case IDC_BUTTON_NOTICE_AUTORUN:
		{
			if( mAutoNoticeList.IsEmpty() == true )
			{	
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8063 ) );
				return;
			}

			if( mIsNoticefileOpen == false )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8063 ) );
				return;
			}
			
			if( mIsNoticeTimerRun == true )
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );

			/// 리스트에 있는 첫번째 줄을 공지로 보내고 타이머 돌리기
			sAutoNotice* pAutoNotice = NULL;
			pAutoNotice = mAutoNoticeList.Front();
			if( pAutoNotice )
			{
				mTimeInterval = pAutoNotice->mTimeDelay;
				SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, pAutoNotice->mTimeDelay, FALSE );
				
				/// 공지 보내기
				int noticeType = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_COMBO_NOTICETYPE, CB_GETCURSEL, 0, 0 );
				SendNoticeMsg( noticeType, pAutoNotice->mNoticeMsg );
				SAFE_DELETE( pAutoNotice );
			}

			mAutoNoticeList.PopFront();

			ShowAutoNoticeMsg();

			SetTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND, 1000, NULL );
			mIsNoticeTimerRun = true;

		}
		break;
	case IDC_LIST_NOTICE_FILELIST:
		{
			switch( msg )
			{
				case LBN_DBLCLK:
					{
						BOOL dir;
						char selectedName[MAX_PATH] = { 0, };
						char path[MAX_PATH] = { 0, };

						dir = DlgDirSelectExA( mSubDlg[GMTOOL_SUBDLG_NOTICE], selectedName, MAX_PATH, IDC_LIST_NOTICE_FILELIST );
						
						///폴더 선택
						if( dir )
						{
							if(lstrcmpA(selectedName, "..\\") == 0)
							{
								char *p;
								p = strrchr( mCurrentPath, '\\' );
								*p = 0;
							}
							else if( selectedName[1] == ':' )
							{
								sprintf_s( mCurrentPath, "%c:\\", selectedName[0] );
							}
							else
							{
								sprintf_s( path, "%s\\%s", mCurrentPath, selectedName );
								path[::lstrlenA(path)-1]=0;
								::lstrcpyA(mCurrentPath, path );
							}
							sprintf_s( path, "%s\\*.txt", mCurrentPath );
							
							/// 비어있는 드라이브로 이동할 경우 DDL_DRIVES 옵션이 DDL_EXECLUSIVE를 호출하여 리스트가 생성되지 않으므로 2번 호출
							DlgDirListA( mSubDlg[GMTOOL_SUBDLG_NOTICE], path, IDC_LIST_NOTICE_FILELIST, 0, ( DDL_DRIVES ) );
							DlgDirListA( mSubDlg[GMTOOL_SUBDLG_NOTICE], path, IDC_LIST_NOTICE_FILELIST, 0, ( DDL_ARCHIVE | DDL_DRIVES | DDL_DIRECTORY | DDL_READWRITE ) );
							
							SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_PATH, mCurrentPath );
						}
						else
						{

							if( mIsNoticefileOpen == true )
							{
								if ( mNoticeFileLoader.Close() == -1 )
								{
									SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8065 ) );
									break;
								}
							}
							
							if( mIsNoticeTimerRun == true )
							{
								KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );
								mIsNoticeTimerRun = false;
							}

							SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, "" );

							/// 파일 입력시 .txt파일을 읽어오기
							sprintf_s( path, "%s\\%s", mCurrentPath, selectedName );
							
							if( ParsingNoticeFile( path ) == false )
								break;
						}
					}
				break;
			}
		}
		break;
	case IDC_BUTTON_SERVER_COUNT_SEND:
		{
			/// 서버 자동 종료 카운터 전송
			HWND hComDlg = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_COMBO_NOTICETYPE );
			int noticeType = SendMessage( hComDlg, CB_GETCURSEL, 0, 0 );
			if( noticeType != NM_NOTICE_NOTICE_SERVER_REQ && noticeType != NM_NOTICE_NOTICE_CHANNEL_REQ )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8066 ) );
				return;
			}

			int count = 0;
			BOOL ret  = FALSE;
			count = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_SERVER_COUNT, &ret, FALSE );
			if( ret == FALSE || count == 0 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8067 ) );
				return;
			}
			
			if( mIsCounterTimerRun == true )
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
						
			/// 서버 공지
			if( noticeType == 0 )
			{
				MSG_REQ_NOTICE_DOWN_COUNT_SERVER msg;
				msg.Category = NM_NOTICE;
				msg.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_REQ;
				msg.mCount   = (unsigned short)count;
				if( NETWORK->IsConnected() == true )
				{
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( msg ) );
				}
				
				if( count-1 > 0 )
				{
					SetTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT, 1000, NULL );
					mNumDownCounter.mCount		= (unsigned short)count-1;
					mNumDownCounter.mNoticeType = (short)noticeType;
				}
			}
			/// 채널 공지
			else if( noticeType == 1 )
			{
				MSG_REQ_NOTICE_DOWN_COUNT_CHANNEL msg;
				msg.Category = NM_NOTICE;
				msg.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_CHANNEL_REQ;
				msg.mCount   = (unsigned short)count;
				if( NETWORK->IsConnected() == true )
				{
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( msg ) );
				}

				if( count-1 > 0 )
				{
					SetTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT, 1000, NULL );
					mNumDownCounter.mCount		= (unsigned short)count-1;
					mNumDownCounter.mNoticeType = (short)noticeType;
				}
			}

			mIsCounterTimerRun = true;
		}
		break;

	case IDC_BUTTON_NOTICE_CANCEL_AUTORUN:
		{
			/// 서버 자동 공지 취소
			mIsNoticeTimerRun = false;
			mIsNoticefileOpen = false;
			mTimeInterval = 0;
			KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_AUTO, _T(""));
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8064 ) );
			SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, 0, FALSE );

			tAutoNoticeList::cConstIterator iBegin = mAutoNoticeList.Begin();
			tAutoNoticeList::cConstIterator iEnd   = mAutoNoticeList.End();
			for( ; iBegin != iEnd; ++iBegin )
			{
				sAutoNotice* pAutoNotice = NULL;
				pAutoNotice = *iBegin;
				SAFE_DELETE( pAutoNotice );
			}
			mAutoNoticeList.Clear();
		}
		break;
	case IDC_BUTTON_CANCEL_SERVER_COUNT:
		{
			/// 서버 자동 종료 카운터 취소
			if( mIsCounterTimerRun == true )
			{
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
				mIsCounterTimerRun = false;
			}
		}
		break;
	}
}


void cGMTool::OnTimerNotice( int timerIndex )
{
	switch( timerIndex )
	{
	case eGMTOOL_TIMER_NOTICE_SEND:
		{

			--mTimeInterval;
			SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, mTimeInterval, FALSE );
			if( mTimeInterval < 1 )
			{
				/// 리스트에 있는 첫번째 줄을 공지로 보내고 타이머 돌리기
				sAutoNotice* pAutoNotice = NULL;
				pAutoNotice = mAutoNoticeList.Front();
				if( pAutoNotice )
				{
					mTimeInterval = pAutoNotice->mTimeDelay;
					SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, pAutoNotice->mTimeDelay, FALSE );

					/// 공지 보내기
					int noticeType = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_COMBO_NOTICETYPE, CB_GETCURSEL, 0, 0 );
					SendNoticeMsg( noticeType, pAutoNotice->mNoticeMsg );
					SAFE_DELETE( pAutoNotice );
				}
				mAutoNoticeList.PopFront();
				
				ShowAutoNoticeMsg();

				if( mAutoNoticeList.IsEmpty() == true )
				{
					mIsNoticeTimerRun = false;
					mIsNoticefileOpen = false;
					mTimeInterval = 0;
					KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_AUTO, _T(""));
					SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_FILEREAD_END, GAMERESOURCEMAN->GetGameText( 8064 ) );
					SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_TIME_INTERVAL, 0, FALSE );
					return;
				}
			}
		}
		break;

	case eGMTOOL_TIMER_DOWN_COUNT:
		{
			if( mNumDownCounter.mNoticeType == 0 )
			{
				MSG_REQ_NOTICE_DOWN_COUNT_SERVER msg;
				msg.Category = NM_NOTICE;
				msg.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_REQ;
				msg.mCount   = mNumDownCounter.mCount;
				if( NETWORK->IsConnected() == true )
				{
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( msg ) );
				}
			}
			else
			{
				MSG_REQ_NOTICE_DOWN_COUNT_CHANNEL msg;
				msg.Category = NM_NOTICE;
				msg.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_CHANNEL_REQ;
				msg.mCount   = mNumDownCounter.mCount;
				if( NETWORK->IsConnected() == true )
				{
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof( msg ) );
				}
			}

			SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_SERVER_COUNT, mNumDownCounter.mCount, NULL );

			if ( mNumDownCounter.mCount <= 1 )
			{
				KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
				mIsCounterTimerRun = false;
				return;
			}
			else
				mNumDownCounter.mCount--;

		}
		break;
	}

}

bool cGMTool::LoadNoticeData()
{
	HWND typeCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_COMBO_NOTICETYPE );
	if( ::IsWindow( typeCombo ) == false )
	{
		assert(0);
		return false;
	}

	/// 정보 삽입
	SendMessage( typeCombo, CB_ADDSTRING, 0, (LPARAM)_T("Server"));
	SendMessage( typeCombo, CB_ADDSTRING, 0, (LPARAM)_T("Channel"));
	SendMessage( typeCombo, CB_ADDSTRING, 0, (LPARAM)_T("Map"));

	/// 공지 보낼 수 있는 길이 제한 걸기
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_NORMAL, EM_SETLIMITTEXT, (WPARAM)MSG_PRINT_LENGTH, 0 );
	
	/// 서버 카운트 다운 길이 제한걸기 999초 한계
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_SERVER_COUNT, EM_SETLIMITTEXT, (WPARAM)3, 0 );

	return true;
}

bool cGMTool::ParsingNoticeFile( char* filePath )
{
	if( mNoticeFileLoader.Open( filePath, true ) == false )
		return false;

	tAutoNoticeList::cConstIterator iBegin = mAutoNoticeList.Begin();
	tAutoNoticeList::cConstIterator iEnd   = mAutoNoticeList.End();
	for( ; iBegin != iEnd; ++iBegin )
	{
		sAutoNotice* pAutoNotice = NULL;
		pAutoNotice = *iBegin;
		SAFE_DELETE( pAutoNotice );
	}
	mAutoNoticeList.Clear();

	mIsNoticefileOpen = true;

	char noticeMsg [ MSG_PRINT_LENGTH*2 + 2 ] = { 0, };

	while( mNoticeFileLoader.IsEnd() != true )
	{
		memset( noticeMsg, 0, sizeof( noticeMsg ) );
		int readByte = mNoticeFileLoader.ReadLine( noticeMsg, sizeof(char)*MSG_PRINT_LENGTH*2 );
		if( readByte != 0 )
		{
			if( noticeMsg[0] == '\r' || noticeMsg[0] == '\n' )
				continue;
			
			if( _ismbslead( (unsigned char*)noticeMsg, (unsigned char*)&noticeMsg[readByte-1] ) == -1 )
			{
				char readBuf[2] = {0,};
				/// 읽어들인 부분의 마지막 글자가 짤려서 읽힌경우 1byte 더 읽음
				if( mNoticeFileLoader.ReadLine( readBuf, 2 ) != 0 )
					strcat_s( noticeMsg, sizeof(noticeMsg), readBuf );
			}

			try
			{
				/// 시간이 설정되어 있는지 검사
				if( noticeMsg[0] == '[' )
				{
					char* pos = NULL;
					pos = strchr( noticeMsg, ']' );
					if( pos != NULL )
					{
						char numBuffer[MSG_PRINT_LENGTH] = {0,};
						strncpy_s( numBuffer, sizeof(numBuffer), &noticeMsg[1], pos - noticeMsg );
						int timeDelay = atoi( numBuffer );

						/// 시간 값 축출 성공
						if( timeDelay != 0 )
						{
							sAutoNotice* pAutoNoticeMsg;
							pAutoNoticeMsg = new sAutoNotice;
							if( pAutoNoticeMsg )
							{
								pAutoNoticeMsg->mTimeDelay = timeDelay;
								if( ConvertToUnicode( ++pos, pAutoNoticeMsg->mNoticeMsg, sizeof( pAutoNoticeMsg->mNoticeMsg ) ) != 0 )
								{
									mAutoNoticeList.PushBack( pAutoNoticeMsg );
								}
								else
								{
									delete pAutoNoticeMsg;
								}
							}
						}
						else
							throw -1;
					}
					else
						throw -1;
					
				}
				else
					throw -1;
			}
			catch ( ... )
			{
				///시간 값 축출 실패시 기본적인 시간 딜레이를 1 초로 설정하고 내용을 출력한다.
				sAutoNotice* pAutoNoticeMsg;
				pAutoNoticeMsg = new sAutoNotice;
				if( pAutoNoticeMsg )
				{
					pAutoNoticeMsg->mTimeDelay = 1;
					if( ConvertToUnicode( noticeMsg, pAutoNoticeMsg->mNoticeMsg, sizeof( pAutoNoticeMsg->mNoticeMsg ) ) != 0 )
					{
						mAutoNoticeList.PushBack( pAutoNoticeMsg );
					}
					else
					{
						delete pAutoNoticeMsg;
					}
				}
			}
		}
	}

	//int fileSize = mNoticeFileLoader.GetSize();
	mNoticeFileLoader.Close();

	ShowAutoNoticeMsg();

	return true;
}

void cGMTool::SendNoticeMsg( int noticeType, WCHAR* str )
{
	/// 필터링
	cStringT temp = str;
	temp.Replace( _T("\r\n"), _T("\0") );
	temp.Replace( _T("\t"), _T(" ") );
	temp.Replace( _T("\n"), _T("\0") );
	temp.Replace( _T("\r"), _T("\0") );

	/// 서버공지
	if( noticeType == eGMTOOL_NOTICE_SERVER )
	{
		MSG_REQ_NOTICE_SERVER msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_SERVER_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, temp.Cstr(), MSG_PRINT_LENGTH );
		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );

	}
	/// 채널공지
	else if( noticeType == eGMTOOL_NOTICE_CHANNEL )
	{
		MSG_REQ_NOTICE_CHANNEL msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_CHANNEL_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, temp.Cstr(), MSG_PRINT_LENGTH );
		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );
	}
	/// 맵 공지
	else if( noticeType == eGMTOOL_NOTICE_MAP )
	{
		MSG_REQ_NOTICE_AREA msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_NOTICE;
		msg.Protocol = NM_NOTICE_NOTICE_AREA_REQ;
		::Sstrncpy( msg.mMessage, MAX_CHAT_BUFFER_SIZE, temp.Cstr(), MSG_PRINT_LENGTH );
		NETWORK->SendNetworkMsg( (char*)&msg, (unsigned short)msg.GetMsgLength() );
	}
	else
	{
		assert(0);
		return;
	}
}

void cGMTool::ShowAutoNoticeMsg()
{
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_AUTO, _T(""));

	if( mAutoNoticeList.IsEmpty() == true )
		return; 

	cStringT text;
	cStringT temp;

	tAutoNoticeList::cConstIterator iBegin = mAutoNoticeList.Begin();
	tAutoNoticeList::cConstIterator iEnd   = mAutoNoticeList.End();
	for( ; iBegin != iEnd; ++iBegin )
	{
		sAutoNotice* pAutoNotice = NULL;
		pAutoNotice = *iBegin;
	
		temp.Format( _T( "[%d]%s\r\n" ), pAutoNotice->mTimeDelay, pAutoNotice->mNoticeMsg );
		text += temp;
	}

	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_NOTICE], IDC_EDIT_NOTICE_AUTO, text.Cstr() );
}
