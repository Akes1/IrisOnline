#include "stdafx.h"
#include "Resource.h"

#include "OptionSheet.h"
#include "OptionWindow.h"
#include "TwitterWindow.h"
#include "SimpleTipWindow.h"

#include "Label.h"
#include "Button.h"
#include "UIImage.h"
#include "EditBox.h"
#include "CheckBox.h"
#include "MultiEditBox.h"
#include "StyleListBox.h"

#include "CommonNetworkMsgStruct.h"

#include "UIManager.h"
#include "FontAgent.h"
#include "Application.h"
#include "ItemManager.h"
#include "RenderSystem.h"
#include "StageManager.h"
#include "QuestManager.h"
#include "ObjectManager.h"
#include "MonsterScript.h"
#include "GameUIManager.h"
#include "GameResourceManager.h"

#include "Hero.h"
#include "Item.h"

#include "tinyxml.h"
#include "httpdownloader.h"


// 공유 Data
sThreadShareData	gThreadShareData;

void			SetThreadStateAndResult( unsigned char nResult, unsigned char nState = eState_None );
bool			DownLoadTextureFromHttp( unsigned long nUserIdx, const char* imgUrl );
DWORD			StatusUpdate( LPCTSTR pSendMsg );
DWORD			GetHomeTimeLine();
DWORD			ReqRetweet( unsigned long idx );
DWORD			ReqFollow( unsigned long idx );
DWORD WINAPI	LoaderProgressRoutine(DWORD totalFileSize, DWORD totalBytesTransferred, void* data);


///=================================================================
//
//				트위터 제어 윈도우
//
///=================================================================
cTwitterWindow::cTwitterWindow()
: mpSendMessage(0)
, mpSendBtn(0)
, mpRegisterBtn(0)
, mpStateLabel(0)
, mUpdateTime(0)
, mAccountSaveType(0)
, mDBAccountSave(0)
, mpTwitListBox(0)
, mpCloseBtn(0)
, mTwitterThread(0)
, mIsLogin(0)
, mEventUpdate(0)
, mIsSendMsgFail(0)
, mFailShowTime(0)
, mUpdateReady(0)
{	
	gThreadShareData.mShutDown	= false;
	gThreadShareData.mResetRemainMinute = -1;
	InitializeCriticalSection( &gThreadShareData.mCs );	

	mTwitterEventQueue.Reserve( 100 );
}

cTwitterWindow::~cTwitterWindow()
{
	// Accept Thread가 종료될때까지 대기
	
	if ( mTwitterThread != NULL )
	{
		gThreadShareData.mShutDown = true;
		WaitForSingleObjectEx( mTwitterThread, INFINITE, TRUE );
		CloseHandle( mTwitterThread );
		mTwitterThread = NULL;
	}

	Clear();
	DeleteCriticalSection( &gThreadShareData.mCs );	

	// Dll UnLoad
	int nCnt = mDllArray.GetSize();
	for( int i = 0; i < nCnt; i++ )
	{
		HINSTANCE hIns = mDllArray[i];
		if( hIns )
		{
			FreeLibrary( hIns );
		}
	}

	mDllArray.Clear();

}

// 초기 트위터 관련 Data를 요청
void cTwitterWindow::Open()
{
	mIsSendMsgFail	= false;
	mFailShowTime	= 0;

	if( mDBAccountSave == true && mIsLogin == false )
	{
		int nLenID = ::strlen( STAGEMAN->GetTwitterUid() );
		int nLenPW = ::strlen( STAGEMAN->GetTwitterPwd() );

		if( nLenID > 0 && nLenPW > 0 )
		{
			cString id = STAGEMAN->GetTwitterUid();
			cString pw = STAGEMAN->GetTwitterPwd();

			CSBlock( &gThreadShareData.mCs )
			{
				gThreadShareData.mpTwitterContoller.setTwitterUsername( id );
				gThreadShareData.mpTwitterContoller.setTwitterPassword( pw );
			}
		}
		else
		{
			mDBAccountSave = false;
		}
	}	
}

void cTwitterWindow::Clear()
{
	mTwitterEventQueue.Clear();

	cCSLock lock( &gThreadShareData.mCs );

	gThreadShareData.mTwitterTextureMap.Clear();
	gThreadShareData.mTiwtTextIdxList.Clear();

	if( gThreadShareData.mImgBufferMap.IsEmpty() == false )
	{
		cImgBufferMap::cIterator i		= gThreadShareData.mImgBufferMap.Begin();
		cImgBufferMap::cIterator iEnd	= gThreadShareData.mImgBufferMap.End();

		for( ; i != iEnd; ++i )
		{
			sImgBufInfo* pImgBufInfo = (sImgBufInfo*)(i->mSecond);

			if( pImgBufInfo )
			{
				GlobalFree( pImgBufInfo->mBuf );
				SAFE_DELETE( pImgBufInfo );
			}
		}
		gThreadShareData.mImgBufferMap.Clear();		
	}
	
	if( gThreadShareData.mTwitDataMap.IsEmpty() == false )
	{
		unsigned int nRowCnt = gThreadShareData.mTwitDataMap.GetSize();
		for( unsigned int i = 0; i < nRowCnt; i++ )
		{
			sTwitDataInfo* pTwitDataInfo = (sTwitDataInfo*)gThreadShareData.mTwitDataMap[i];

			if( pTwitDataInfo )
			{
				SAFE_DELETE( pTwitDataInfo );
			}
		}
		gThreadShareData.mTwitDataMap.Clear();
	}	
}

void cTwitterWindow::LogOut()
{
	mIsLogin		= false; 
	mUpdateTime	= 0;

	Clear();

	if( mpStateLabel )
		mpStateLabel->Clear();

	if( mpTwitListBox )
	{
		mpTwitListBox->Clear();
	}
}

void cTwitterWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	// 쓰레드 동작중이라면
	if( gThreadShareData.mStateThread != eState_None )
		return;

	if( mIsLogin == true ) 
	{
		// 중복 메시지로 인한 에러 정보를 라벨에 SHOW_FAIL_MSG_TIME 만큼 보여준다.
		if( mIsSendMsgFail == true )
		{
			mFailShowTime += deltaTime;

			if( mFailShowTime > SHOW_FAIL_MSG_TIME )
			{
				mFailShowTime	= 0;
				mIsSendMsgFail	= false;

				if( mpStateLabel ) 
					mpStateLabel->Clear();
			}
		}

		// 다음 갱신 시점이 되면은 갱신 준비를 True 설정			
		if( mUpdateTime <= accumTime && mUpdateReady == false )		
		{
			mUpdateReady = true;
		
			/// 창 개방 상태이면 바로 갱신
			if( mVisible == true )
			{
				CreateTwitterThread( eCommand_GetTimeLineHome );
			}
			/// 창 비 개방 상태이면 보유하던 Data 제거. 창 열었을 때 갱신하여 설정
			else
			{
				mpTwitListBox->Clear();
				Clear();
			}

			/// LBS : thread생성 시점에 아래루틴으로 넘어가지 않도록 return
			return;
		}
	}

	// 쓰레드 결과 값이 있다면 처리
	if( gThreadShareData.mResultThread == eResult_None)
	{
		// 쓰레드 비동작 상태 그리고 결과 값이 없을 때 이벤트 큐에 쌓아두었던 이벤트 처리
		ProcessEvent();

		return;
	}

	switch( gThreadShareData.mResultThread )
	{
	case eResult_Success_Login:		/// 로그인 처리후 리스트 갱신
	case eResult_Success_GetList:	/// 리스트만 갱신
	case eResult_Success_SendMsg:	/// 메시지 업데이트
	case eResult_Success_EventMsg:
		{
			// ============ 로그인 처리 ================
			if( gThreadShareData.mResultThread == eResult_Success_Login )
			{
				if( mIsLogin == false )
				{
					mIsLogin = true;	
				}				

				cTwitterRegisterWindow* pTwitRegisterWin = GAMEUI->GetTwitterRegisterWindow();
				if( pTwitRegisterWin )
					pTwitRegisterWin->Hide();

				// 로그인 OK 후
				// 쓰레드 동작 이전 DB에 저장할지 여부(yes, no)에 따라 DB에 저장
				if( pTwitRegisterWin->IsDBSaveCheckPress() == true )	//실제 dB 저장 상태를 나타내는 것은 아님
				{
					SetRegistType( eRegister_LoginAndSave );

					MSG_REQ_TWITTER msg;
					::memset( &msg, 0, sizeof(msg) );

					msg.Category  = NM_USER;
					msg.Protocol  = NM_USER_TWITTER_REQ;
					strncpy( msg.uid, GetID(), MAX_TWITTER_UID );
					strncpy( msg.pwd, GetPW(), MAX_TWITTER_PWD );
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );					
				}
				else
				{
					SetRegistType( eRegister_OnlyLogin );

					MSG_REQ_TWITTER msg;
					::memset( &msg, 0, sizeof(msg) );

					msg.Category  = NM_USER;
					msg.Protocol  = NM_USER_TWITTER_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					pTwitRegisterWin->SetResetBtnEnable( false );

					cOptionWindow* pOptionWin = GAMEUI->GetOptionWindow();
					if( pOptionWin )
					{
						cOptionSheet3* pTwitSheet = (cOptionSheet3*)pOptionWin->GetSheetByID( eUIID_OPTION_SHEET3 );
						if( pTwitSheet )
						{
							pTwitSheet->SetResetBtnEnable( false );
						}
					}
				}				
			}			

			if( mIsLogin )
			{
				/// ================================= 리스트 갱신 =====================================
				if( mpStateLabel ) 
					mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText( 11745 ) );

				///================== Thread로부터 HTTP 수신 이미지 버퍼 NiTexture 생성 ========================
				LPDIRECT3DTEXTURE9 tex = 0;
				cRenderer* renderer = RENDERSYS->GetRenderer();

				cImgBufferMap::cIterator i		= gThreadShareData.mImgBufferMap.Begin();
				cImgBufferMap::cIterator iEnd	= gThreadShareData.mImgBufferMap.End();

				for( ; i != iEnd; ++i )
				{
					sImgBufInfo* pTwitDataInfo = (sImgBufInfo*)(i->mSecond);

					HRESULT ret = ::D3DXCreateTextureFromFileInMemoryEx( renderer->GetD3DDevice(), pTwitDataInfo->mBuf, pTwitDataInfo->mBufLen,
						0, 0, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0, &tex );

					if( FAILED(ret) || tex == 0 )
					{
						assert( 0 );
						continue;
					}

					NiTexturePtr nitex = renderer->CreateNiTextureFromD3DTexture( tex );

					if( nitex )
					{
						gThreadShareData.mTwitterTextureMap.Insert( i->mFirst, nitex );										
					}
					GlobalFree( pTwitDataInfo->mBuf );
					SAFE_DELETE(pTwitDataInfo);				
				}

				///================== Thread로부터 인코딩 된 Data 정보 리스트에 일괄 삽입 ========================
				if( gThreadShareData.mResultThread != eResult_Success_SendMsg &&
					gThreadShareData.mResultThread != eResult_Success_EventMsg )
				{
					mpTwitListBox->Clear();
				}

				int nRow = 0;
				unsigned int nRoWCnt = gThreadShareData.mTwitDataMap.GetSize();	

				for( unsigned int i = 0; i < nRoWCnt; i++ )
				{
					sTwitDataInfo* pTwitDataInfo = (sTwitDataInfo*)gThreadShareData.mTwitDataMap[i];
					if( pTwitDataInfo == NULL)
					{
						assert(0);
						continue;
					}

					NiTexture* pNiTex	= NULL;
					cUIImage* image		= NULL;

					cTwitterTextureMap::cIterator iTexture = gThreadShareData.mTwitterTextureMap.Find( pTwitDataInfo->mUserIdx );
					if( iTexture != gThreadShareData.mTwitterTextureMap.End() )
					{
						pNiTex = (NiTexture*)(iTexture->mSecond);
					}				

					if( pNiTex )
					{					
						image = new cUIImage( pNiTex );
						if( image )
						{
							image->SetTextureRect( 0, 0, pNiTex->GetWidth(), pNiTex->GetHeight() );
							image->SetScreenWH( PROFILE_ICON_SIZE, PROFILE_ICON_SIZE );
							image->SetScreenRect( cUIRect( 0, 0, 0 + PROFILE_ICON_SIZE, 0 + PROFILE_ICON_SIZE ) );
						}				
					}		

					// Send StatusUpdate Method 일때만 수행
					if( gThreadShareData.mResultThread == eResult_Success_SendMsg && nRoWCnt == 1 && i == 0 )
					{

						// 최근 자신이 작성한 10개 게시물 중에 동일 문장 존재시 처리
						if( mpTwitListBox )
						{	
							// 갱신전 리스트에서 같은 문장 존재 유무 확인
							if(  IsTwitterTextData( pTwitDataInfo->mTextIdx ) == false  )
							{
								nRow = mpTwitListBox->AddRowAtFirst( (LPCTSTR)(pTwitDataInfo->mText.Cstr()), COLOR_WHITE, image );
								if( nRow != -1)
								{
									mpTwitListBox->AddColume( nRow, (LPCTSTR)(pTwitDataInfo->mDate.Cstr()), COLOR_WHITE );
									mpTwitListBox->AddColume( nRow, (LPCTSTR)(pTwitDataInfo->mID.Cstr()),	COLOR_WHITE );				
								}

								/// 저장한 목록이 10개 이면 가장 마지막에 작성한 텍스트 Idx 제거하고, 최근 텍스트 Idx를 맨 앞에 삽입
								int nTexCount = gThreadShareData.mTiwtTextIdxList.GetSize();
								if( nTexCount >= DUPLICATION_COUNT_CHECK )
								{	
									for( int i = 0; i <= nTexCount - DUPLICATION_COUNT_CHECK; i++ )
									{
										gThreadShareData.mTiwtTextIdxList.PopBack();
									}								
								}

								gThreadShareData.mTiwtTextIdxList.PushFront( pTwitDataInfo->mTextIdx );

								if( mpStateLabel ) 
									mpStateLabel->Clear();
							}
							else
							{
								// 새로 작성한 문장이 있을시 최근 10개 게시물중 존재하여등록이 실패하면 그 이유를 알려준다.
								if( IsVisible() == true )
								{
									mIsSendMsgFail	= true;
									mFailShowTime	= 0;

									if( mpStateLabel ) 
										mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText( 11758 ) );
								}			
							}
						}	

						if( mpSendMessage )
							mpSendMessage->Clear();

					}
					else
					{
						
						nRow = mpTwitListBox->AddRow( (LPCTSTR)(pTwitDataInfo->mText.Cstr()), COLOR_WHITE, image );
						if( nRow != -1)
						{
							mpTwitListBox->AddColume( nRow, (LPCTSTR)(pTwitDataInfo->mDate.Cstr()), COLOR_WHITE );
							mpTwitListBox->AddColume( nRow, (LPCTSTR)(pTwitDataInfo->mID.Cstr()),	COLOR_WHITE );				
						}
					}					

					SAFE_DELETE(pTwitDataInfo);
				}

				if( mIsSendMsgFail == false && mpStateLabel ) 
					mpStateLabel->Clear();

				CSBlock( &gThreadShareData.mCs )
				{
					gThreadShareData.mImgBufferMap.Clear();
					gThreadShareData.mTwitDataMap.Clear();
				}

				/// 리스트 갱신 및 로그인 완료일때만 다음 갱신 타임 설정
				if( gThreadShareData.mResultThread == eResult_Success_GetList	||
					gThreadShareData.mResultThread == eResult_Success_Login		)
				{
					mUpdateReady = false;
					mUpdateTime = accumTime + HTTP_REQUEST_TIME;
				}

				if( gThreadShareData.mResultThread == eResult_Success_EventMsg )
				{
					NetUnLock();
					DestroyTwitterThread();

					// 이벤트 큐에 이벤트가 있다면 다시 처리 요청
					if( ProcessEvent() == 0 )
					{
						mEventUpdate = false;			
					}	
				}
				else
				{
					NetUnLock();	
					DestroyTwitterThread();		
				}						
			}			
		}
		break;

	case eResult_Success_Retweet:
		{	
		}
		break;
	case eResult_Error_NoLogin:
		{
			if( mpSendMessage )
				mpSendMessage->Clear();

			if( mpStateLabel )
				mpStateLabel->Clear();

			NetUnLock();
			DestroyTwitterThread();	
			
			if( mVisible == true )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11743 ),	// 알림 메세지
					GAMERESOURCEMAN->GetGameText( 2 )		// 알림
					);
			}
			
		}
		break;
	case eResult_Error_NoAccount:	// 계정 정보가 일치 않거나 없을때!!
		{
			if( mIsLogin == true )
			{
				if( mDBAccountSave == true )
				{				
					cString connectID = STAGEMAN->GetTwitterUid();
					cString connectPW = STAGEMAN->GetTwitterPwd();

					cCSLock lock(&gThreadShareData.mCs);
					gThreadShareData.mpTwitterContoller.setTwitterUsername( connectID );
					gThreadShareData.mpTwitterContoller.setTwitterPassword( connectPW );
				}
				else
				{
					cCSLock lock(&gThreadShareData.mCs);
					gThreadShareData.mpTwitterContoller.setTwitterUsername( mOldID );
					gThreadShareData.mpTwitterContoller.setTwitterPassword( mOldPW );
				}
			}
			else
			{
				LogOut();

				if( mDBAccountSave == true )
				{
					MSG_REQ_TWITTER msg;
					::memset( &msg, 0, sizeof(msg) );

					msg.Category  = NM_USER;
					msg.Protocol  = NM_USER_TWITTER_REQ;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}			

			cTwitterRegisterWindow* pRegisterWin = GAMEUI->GetTwitterRegisterWindow();
			if( pRegisterWin )
				pRegisterWin->Clear();

			if( mpStateLabel )
				mpStateLabel->Clear();

			NetUnLock();
			DestroyTwitterThread();	

			if( mVisible == true )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11737 ),	// 알림 메세지
					GAMERESOURCEMAN->GetGameText( 2 )		// 알림
					);
			}			
		}
		break;
	
	case eResult_Error_GetTimeLineHome:
		{
			LogOut();

			if( mDBAccountSave == true )	//실제 dB 저장 상태를 나타내는 것은 아님
			{
				MSG_REQ_TWITTER msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category  = NM_USER;
				msg.Protocol  = NM_USER_TWITTER_REQ;
				
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );					
			}

			if( mpStateLabel )
				mpStateLabel->Clear();

			if( mpSendMessage )
				mpSendMessage->Clear();

			NetUnLock();
			DestroyTwitterThread();	

			if( mVisible == true )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_TWITTER_ERROR_LIST_UPDATE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11751 ),	// 알림 메세지
					GAMERESOURCEMAN->GetGameText( 2 )		// 알림
					);			
			}		
		}
		break;
	
	case eResult_Error_UpdateStatus:
	case eResult_Error_EventUpdateStatus:	
	case eResult_Error_StatusUpdateLimit:
	case eResult_Error_LimitExceed:
		{
			if( mpSendMessage )
				mpSendMessage->Clear();

			if( mpStateLabel )
				mpStateLabel->Clear();

			unsigned int textIdx = 0;
			if( gThreadShareData.mResultThread == eResult_Error_UpdateStatus		|| 
				gThreadShareData.mResultThread == eResult_Error_EventUpdateStatus	) 
			{
				textIdx = 11750;
			}		
			else if( gThreadShareData.mResultThread == eResult_Error_LimitExceed ) 
			{
				if( mIsLogin == true )
					LogOut();

				textIdx = 11759;
			}
			else if( gThreadShareData.mResultThread == eResult_Error_StatusUpdateLimit ) 
			{
				textIdx = 11760;
			}

			if( mVisible == true )
			{
				if( gThreadShareData.mResultThread == eResult_Error_LimitExceed )
				{
					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( textIdx ), gThreadShareData.mResetRemainMinute );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						(LPTSTR)str.Cstr(),					// 알림 메세지
						GAMERESOURCEMAN->GetGameText( 2 )	// 알림
						);
				}
				else
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( textIdx ),// 알림 메세지
						GAMERESOURCEMAN->GetGameText( 2 )		// 알림
						);
				}
			}

			NetUnLock();
			DestroyTwitterThread();				
		}
		break;
	case eResult_Error_Retweet:
	case eResult_Error_Follow:
		{
			/*
			if( mpStateLabel )
				mpStateLabel->Clear();
			
			if( mVisible == true )
			{
				int textidx = 11759;
				if( gThreadShareData.mResultThread == eResult_Error_Retweet )
				{
					textidx = 11759;
				}
				else if( gThreadShareData.mResultThread == eResult_Error_Follow )
				{
					textidx = 11759;
				}

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( textidx ),	// 알림 메세지
					GAMERESOURCEMAN->GetGameText( 2 )		// 알림
					);
			}			
			
			NetUnLock();
			DestroyTwitterThread();		
			*/
		}
		break;
	case eResult_Error_Default:
		{
			if( mpStateLabel )
				mpStateLabel->Clear();

			NetUnLock();
			DestroyTwitterThread();	
		}
		break;
	}
}

bool cTwitterWindow::IsTwitterTextData( __int64 nFindTextIdx )
{
	cCSLock lock( &gThreadShareData.mCs );

	cTwitterTextIdxList::cIterator i	= gThreadShareData.mTiwtTextIdxList.Begin();
	cTwitterTextIdxList::cIterator iEnd = gThreadShareData.mTiwtTextIdxList.End();

	for( ; i != iEnd; ++i )
	{
		if( nFindTextIdx == *i )
		{
			return true;
		}
	}	

	return false;
}

void cTwitterWindow::AddEvent( sTwitterEvent& twitEvent )
{
	if( mIsLogin == false )
		return;

	mTwitterEventQueue.PushBack( twitEvent );
	mEventUpdate = true;
}

unsigned char cTwitterWindow::ProcessEvent()
{
	if( mEventUpdate					== false		||	// 추가된 0이벤트가 없다면
		gThreadShareData.mStateThread	!= eState_None	||	// 쓰레드 작동
		mTwitterEventQueue.IsEmpty()	== true			 )	// 큐에 쌓인 이벤트 여부
	{
		return 0;
	}

	if( mIsLogin == false )
		return 0 ;

	unsigned char nEventCnt = (unsigned char)mTwitterEventQueue.GetSize();

	sTwitterEvent &tEvent = mTwitterEventQueue.Front();

	switch( tEvent.mEvent )
	{
	case eTWITEVENT_LEVEL:
		{
			LPTSTR pName = HERO->GetName();
			if( pName == NULL )
			{	
				goto Err;
			}

			unsigned char level = (unsigned char)tEvent.mExData;

			mEventMsg[eTWITEVENT_LEVEL].Clear();
			mEventMsg[eTWITEVENT_LEVEL].Format( GAMERESOURCEMAN->GetGameText( 11753 ), pName, level );

			CreateTwitterThread( eCommand_EventUpdate, (LPVOID)mEventMsg[eTWITEVENT_LEVEL].Cstr() );
		}	

		break;

	case eTWITEVENT_ENHANCE:
		{
			LPCTSTR pName = HERO->GetName();
			if( pName == NULL )
			{	
				goto Err;
			}
			/// 강화아이템 검사
			cItem* pItem = ITEMMAN->GetItem( (unsigned char)tEvent.mExData );
			if( pItem == NULL )
			{
				goto Err;
			}

			cItemDefine* pDefine = pItem->GetDefine();
			if( pDefine == NULL )
			{
				goto Err;
			}
			
			mEventMsg[eTWITEVENT_ENHANCE].Clear();
			mEventMsg[eTWITEVENT_ENHANCE].Format( GAMERESOURCEMAN->GetGameText(11754), pName, (int)pItem->GetEnhanceStep(), pDefine->GetName() );

			CreateTwitterThread( eCommand_EventUpdate, (LPVOID)mEventMsg[eTWITEVENT_ENHANCE].Cstr() );
		}			
		break;

	case eTWITEVENT_QUEST:
		{
			unsigned long changeJobIdx = (unsigned long)tEvent.mExData;

			if( changeJobIdx > 0 )
			{
				LPTSTR pName = HERO->GetName();
				if( pName == NULL )
				{	
					goto Err;
				}

				LPCTSTR jobStr = GAMERESOURCEMAN->GetJobName( changeJobIdx );
				if( jobStr == NULL)
				{
					goto Err;
				}
				
				mEventMsg[eTWITEVENT_QUEST].Clear();
				mEventMsg[eTWITEVENT_QUEST].Format( GAMERESOURCEMAN->GetGameText(11755), pName, jobStr );

				CreateTwitterThread( eCommand_EventUpdate, (LPVOID)mEventMsg[eTWITEVENT_QUEST].Cstr() );
			}				
		}		
		break;

	case eTWITEVENT_MONSTER:
		{
			unsigned long monsterIdx = (unsigned long)tEvent.mExData;

			sMonsterScript* pMon = MONSTERSCRIPT->GetMonsterListInfo( monsterIdx );
			if( pMon == NULL )
			{
				goto Err;
			}

			int nLen = _tcslen( pMon->mJobName );
			mEventMsg[eTWITEVENT_MONSTER].Clear();
			
			if( nLen > 0 )
			{
				mEventMsg[eTWITEVENT_MONSTER].Format( GAMERESOURCEMAN->GetGameText(11756), pMon->mJobName, pMon->mName );
			}
			else
			{
				mEventMsg[eTWITEVENT_MONSTER].Format( GAMERESOURCEMAN->GetGameText(11757), pMon->mName );
			}

			CreateTwitterThread( eCommand_EventUpdate, (LPVOID)mEventMsg[eTWITEVENT_MONSTER].Cstr() );
		}
		break;
			
	}


Err:
	mTwitterEventQueue.PopFront();

	return nEventCnt;
}

NiTexture*	GetTwitterTexture( unsigned long nUseID )
{
	if( nUseID < 0 )
	{		
		return NULL;
	}

	cCSLock lock( &gThreadShareData.mCs );
	cTwitterTextureMap::cIterator i = gThreadShareData.mTwitterTextureMap.Find( nUseID );

	if( i != gThreadShareData.mTwitterTextureMap.End() )
	{
		return (NiTexture*)(i->mSecond);
	}

	return NULL;
}

void cTwitterWindow::DestroyTwitterThread()
{
	if( mTwitterThread )
	{
		CloseHandle( mTwitterThread );
		mTwitterThread  = NULL;
	}

	// 쓰레드 상태를 None, 결과 값도 None
	SetThreadStateAndResult( eResult_None );
}

bool cTwitterWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;
	
	mpSendMessage	= (cMultiEditBox*)GetChild( eUIID_GAME_TWITTER_TEXT );
	mpSendBtn		= (cButton*)GetChild( eUIID_GAME_TWITTER_UPDATE );
	mpRegisterBtn	= (cButton*)GetChild( eUIID_GAME_TWITTER_INSERT );
	mpCloseBtn		= (cButton*)GetChild( eUIID_GAME_TWITTER_CLOSE );
	mpTwitListBox	= (cTwitterListBox*)GetChild( eUIID_GAME_TWITTER_TWIT_LIST );
	mpStateLabel	= (cLabel*)GetChild( eUIID_GAME_TWITTER_STATE_LABEL );
	
	mUpdateTime	= 0;
	mIsLogin		= false;
	mDBAccountSave	= false;
	mpSendMessage->SetTextColor( COLOR_TWITTER_INSERTMSG );

	// DB에 저장되어 있다면 저장 
	char* pID = STAGEMAN->GetTwitterUid();
	char* pPW = STAGEMAN->GetTwitterPwd();

	if( pID && pPW )
	{
		int nLenID = ::strlen( STAGEMAN->GetTwitterUid() );
		int nLenPW = ::strlen( STAGEMAN->GetTwitterPwd() );

		if( nLenID > 0 && nLenPW > 0 )
		{
			cString id = STAGEMAN->GetTwitterUid();
			cString pw = STAGEMAN->GetTwitterPwd();

			CSBlock( &gThreadShareData.mCs )
			{
				gThreadShareData.mpTwitterContoller.setTwitterUsername( id );
				gThreadShareData.mpTwitterContoller.setTwitterPassword( pw );
			}

			mDBAccountSave	= true;	// DB 저장 상태
			mUpdateTime	= 0;
		}
	}

	// DLL 로드
	mDllArray.PushBack( LoadLibrary( L"dnsapi.dll") );
	mDllArray.PushBack( LoadLibrary( L"winrnr.dll") );
	mDllArray.PushBack( LoadLibrary( L"rasadhlp.dll") );
	mDllArray.PushBack( LoadLibrary( L"rasapi32.dll") );
	mDllArray.PushBack( LoadLibrary( L"rasman.dll") );
	mDllArray.PushBack( LoadLibrary( L"netapi32.dll") );
	mDllArray.PushBack( LoadLibrary( L"tapi32.dll") );
	mDllArray.PushBack( LoadLibrary( L"rtutils.dll") );
	mDllArray.PushBack( LoadLibrary( L"msv1_0.dll") );
	mDllArray.PushBack( LoadLibrary( L"cryptdll.dll") );
	mDllArray.PushBack( LoadLibrary( L"iphlpapi.dll") );
	mDllArray.PushBack( LoadLibrary( L"sensapi.dll") );
	mDllArray.PushBack( LoadLibrary( L"userenv.dll") );
	mDllArray.PushBack( LoadLibrary( L"urlmon.dll") );
	mDllArray.PushBack( LoadLibrary( L"msapsspc.dll") );
	mDllArray.PushBack( LoadLibrary( L"msvcrt40.dll") );
	mDllArray.PushBack( LoadLibrary( L"schannel.dll") );
	mDllArray.PushBack( LoadLibrary( L"digest.dll") );
	mDllArray.PushBack( LoadLibrary( L"msnsspc.dll") );
	mDllArray.PushBack( LoadLibrary( L"wsock32.dll") );

	return true;
}

/// 버튼에 따라 Thread 생성
void cTwitterWindow::OnCommand( cUINode* caller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TWITTER_UPDATE:
		{
			if( mpSendMessage && mpSendMessage->GetLength() > 0 )
			{				
				CreateTwitterThread( eCommand_SendUpdate, (LPVOID)mpSendMessage->GetText() );
			}
		}
		break;

	case eUIID_GAME_TWITTER_INSERT:
		{
			cTwitterRegisterWindow* pRegisterWin = GAMEUI->GetTwitterRegisterWindow();
			if( pRegisterWin )
			{
				if( pRegisterWin->IsVisible() )
				{
					pRegisterWin->Hide();
				}
				else
				{
					pRegisterWin->Show();
				}
			}
		}
		break;

	case eUIID_GAME_TWITTER_CLOSE:
		{			
			mpSendMessage->Clear();
			Hide();

			cTwitterRegisterWindow* pRegisterWin = GAMEUI->GetTwitterRegisterWindow();
			if( pRegisterWin )
			{
				if( pRegisterWin->IsVisible() )
					pRegisterWin->Hide();
			}
		}
		break;
	}
}


void cTwitterWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mIsLogin )
	{
		/// 갱신 준비가 되었을 때는 쓰레드 갱신 요청
		if( gThreadShareData.mStateThread == eState_None && mUpdateReady == true )
		{
			mUpdateReady = false;
			CreateTwitterThread( eCommand_GetTimeLineHome );
		}

		if( gThreadShareData.mStateThread == eState_Getting_TimeLineHomeMsg )
		{
			if( mpStateLabel ) 
				mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText( 11745 ) );
		}
		else if(	gThreadShareData.mStateThread == eState_Sending_UpdateMsg	|| 
					gThreadShareData.mStateThread == eState_Sending_EventMsg	)
		{
			if( mpStateLabel ) 
				mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText( 11746 ) );
		}

	}
	/// 처음 접속하여 창을 개방시 DB Save 상태이면 자동 로그인 시도
	else
	{
		if( gThreadShareData.mStateThread == eState_Sending_LoginMsg && mpStateLabel )
			mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText( 11745 ) );

		if( gThreadShareData.mStateThread == eState_None && mUpdateReady == false && mUpdateTime == 0)
		{	
			if( mDBAccountSave == true )
			{
				CreateTwitterThread( eCommand_AutoLogin );			
			}
			
		}
	}

	if( mpSendMessage )
		mpSendMessage->Clear();
}

void cTwitterWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpStateLabel ) 
		mpStateLabel->Clear();

	if( mpSendMessage )
		mpSendMessage->Clear();
}


void cTwitterWindow::OnNetLock( int lockTry )
{
	if( mpSendMessage )	
		mpSendMessage->NetLock( lockTry );

	if( mpSendBtn )	
		mpSendBtn->NetLock( lockTry );

	if( mpRegisterBtn )	
		mpRegisterBtn->NetLock( lockTry );

	if( mpTwitListBox )	
		mpTwitListBox->NetLock( lockTry );

	cTwitterRegisterWindow* pTwitRegisterWin = GAMEUI->GetTwitterRegisterWindow();
	if( pTwitRegisterWin )
	{		
		pTwitRegisterWin->NetLock( lockTry );
	}
}

void cTwitterWindow::OnNetUnLock( int lockTry )
{
	if( mpSendMessage )	
		mpSendMessage->NetUnLock( lockTry );

	if( mpSendBtn )	
		mpSendBtn->NetUnLock( lockTry );

	if( mpRegisterBtn )	
		mpRegisterBtn->NetUnLock( lockTry );

	if( mpTwitListBox )	
		mpTwitListBox->NetUnLock( lockTry );

	cTwitterRegisterWindow* pTwitRegisterWin = GAMEUI->GetTwitterRegisterWindow();
	if( pTwitRegisterWin )
	{		
		pTwitRegisterWin->NetUnLock( lockTry );
	}
}

LPCSTR cTwitterWindow::GetID()
{
	cCSLock lock(&gThreadShareData.mCs);
	return gThreadShareData.mpTwitterContoller.getTwitterUsername().Cstr();
}
LPCSTR cTwitterWindow::GetPW()
{
	cCSLock lock(&gThreadShareData.mCs);
	return gThreadShareData.mpTwitterContoller.getTwitterPassword().Cstr();
}

void cTwitterWindow::SetDBAccountSave( bool bSave )
{  
	mDBAccountSave = bSave; 

	if( bSave == false )
	{
		mOldID = "";
		mOldPW = "";
	}	
}


//==============================================
//		명령 타입에 따라 쓰레드 수행
//==============================================
DWORD cTwitterWindow::PerformTwitterThread( void* arg )
{
	sThreadInfo* pThreadinfo = (sThreadInfo*)arg;
	
	// 쓰레드 동작 수행전 쓰레드 상태를 None -> Run(글보내기, 리스트 갱신, 로그인) 으로 변경하고 수행
	
	switch( pThreadinfo->mCommandType )
	{
	case eCommand_SendUpdate:		// 트위터 새로운 트윗 올리기
		{
			return StatusUpdate( (LPCTSTR)(pThreadinfo->mExtraData) );
		}
		break;

	case eCommand_EventUpdate:		// 각종 이벤트 트윗 올리기
		{
			return StatusUpdate( (LPCTSTR)(pThreadinfo->mExtraData) );
		}
		break;

	case eCommand_GetTimeLineHome:	// eState_Getting_TimeLineHomeMsg = 리스트 갱신
		{
			return GetHomeTimeLine();
		}
		break;

	case eCommand_Login:			// eState_Sending_LoginMsg = 로그인 처리 + 리스트 갱신
	case eCommand_AutoLogin:			
		{			
			return GetHomeTimeLine();
		}		
		break;
	case eCommand_Retweet:			// 해당 게시물 RT 리트윗
		{
			return ReqRetweet( (unsigned long)(pThreadinfo->mExtraData) );
		}
		break;
	case eCommand_Follow:
		{
			return ReqFollow( (unsigned long)(pThreadinfo->mExtraData) );
		}
		break;
	}

	return 0;
}

//==============================================
//		쓰레드 생성 지점
//==============================================
void cTwitterWindow::CreateTwitterThread( unsigned char nCommandType, void* pExData )
{
	static sThreadInfo threadInfo;

	threadInfo.mCommandType = nCommandType;
	threadInfo.mExtraData	= pExData;

	if( mTwitterThread  == NULL && gThreadShareData.mStateThread == eState_None )
	{
		// 쓰레드 생성 전에 요청 명령에 따라 라벨 상태값 변경 및 선행 검사 수행
		switch( nCommandType )
		{
			case eCommand_SendUpdate:
			case eCommand_EventUpdate:
				{
					/// 로그인 여부 검사!!
					if( mIsLogin == false )
					{
						if( mVisible == true )
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11743 ),	// 알림 메세지
								GAMERESOURCEMAN->GetGameText( 2 )		// 알림
								);
						}

						return;
					}

					if( mpStateLabel )
						mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText(11746) );						

					// UI 락킹
					NetLock();
				}
				break;
			case eCommand_GetTimeLineHome:			
				{
					/// 로그인 여부 검사!!
					if( mIsLogin == false )
					{
						if( mVisible == true )
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11743 ),	// 알림 메세지
								GAMERESOURCEMAN->GetGameText( 2 )		// 알림
								);
						}
						return;
					}			

					if( mpStateLabel )
						mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText(11745) );

					// UI 락킹
					NetLock();
					Clear();
				}								
				break;	
			case eCommand_Login:
				{			
					// 등록 윈도우에 있는 ID값과 비밀번호 UNICODE -> Multibyte 변환
					cTwitterRegisterWindow* pTwitterRegisterWin = GAMEUI->GetTwitterRegisterWindow();
					if( !pTwitterRegisterWin )
					{
						assert(0);
						return;
					}

					LPCTSTR pID = pTwitterRegisterWin->GetID();
					LPCTSTR pPW = pTwitterRegisterWin->GetPW();

					int nLenID = ::_tcslen( pID );
					int nLenPW = ::_tcslen( pPW );
					if( nLenID <= 0 || nLenPW <= 0 )
					{
						return;
					}

					char id[LEN_TWITTER_UID] = {0,};
					char pw[MAX_TWITTER_PWD] = {0,};

					nLenID = ConvertToAscii( pID, id, LEN_TWITTER_UID );
					nLenPW = ConvertToAscii( pPW, pw, MAX_TWITTER_PWD );

					cString connectId = id;
					cString connectPw = pw;
					if( mDBAccountSave )
					{
						cCSLock lock(&gThreadShareData.mCs);
						gThreadShareData.mpTwitterContoller.setTwitterUsername( connectId );
						gThreadShareData.mpTwitterContoller.setTwitterPassword( connectPw );
					}
					else
					{
						cCSLock lock(&gThreadShareData.mCs);
						mOldID = gThreadShareData.mpTwitterContoller.getTwitterUsername();
						mOldPW = gThreadShareData.mpTwitterContoller.getTwitterPassword();
						gThreadShareData.mpTwitterContoller.setTwitterUsername( connectId );
						gThreadShareData.mpTwitterContoller.setTwitterPassword( connectPw );
					}
					
					if( mpStateLabel )
						mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText(11747) );

					// UI 락킹
					NetLock();
					
				}
				break;

			case eCommand_AutoLogin:
				{
					if( mDBAccountSave == true && mIsLogin == false )
					{
						int nLenID = ::strlen( STAGEMAN->GetTwitterUid() );
						int nLenPW = ::strlen( STAGEMAN->GetTwitterPwd() );

						if( nLenID > 0 && nLenPW > 0 )
						{
							cString id = STAGEMAN->GetTwitterUid();
							cString pw = STAGEMAN->GetTwitterPwd();

							CSBlock( &gThreadShareData.mCs )
							{
								gThreadShareData.mpTwitterContoller.setTwitterUsername( id );
								gThreadShareData.mpTwitterContoller.setTwitterPassword( pw );
							}
						}

						if( mpStateLabel )
							mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText(11747) );

						// UI 락킹
						NetLock();
					}	
				}
				break;
			
			case eCommand_Retweet:
			case eCommand_Follow:
				{
					/// 로그인 여부 검사!!
					if( mIsLogin == false )
					{
						if( mVisible == true )
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11743 ),	// 알림 메세지
								GAMERESOURCEMAN->GetGameText( 2 )		// 알림
								);
						}
						
						return;
					}

					if( mpStateLabel )
						mpStateLabel->SetText( GAMERESOURCEMAN->GetGameText(11747) );

					// UI 락킹
					NetLock();
				}
				break;
				/// 그외 추가 기능 요구시 이곳에 추가
		}
	
		// 선행 조건 충족시 Thread 생성!!
		DWORD	threadId;		
		mTwitterThread = CreateThread( NULL, 0, PerformTwitterThread, (LPVOID)&threadInfo, 0, &threadId );

		if ( mTwitterThread == NULL )
			return ;

		/// LBS : thread 안에서 하던 처리를 밖으로 변경
		switch( nCommandType )
		{
		case eCommand_SendUpdate:		SetThreadStateAndResult( eResult_None, eState_Sending_UpdateMsg );			break;
		case eCommand_EventUpdate:		SetThreadStateAndResult( eResult_None, eState_Sending_EventMsg );			break;
		case eCommand_GetTimeLineHome:	SetThreadStateAndResult( eResult_None, eState_Getting_TimeLineHomeMsg );	break;
		case eCommand_Login:			
		case eCommand_AutoLogin:		SetThreadStateAndResult( eResult_None, eState_Sending_LoginMsg );			break;
		case eCommand_Retweet:			SetThreadStateAndResult( eResult_None, eState_Sending_RetweetMsg );			break;
		case eCommand_Follow:			SetThreadStateAndResult( eResult_None, eState_Sending_FollowMsg );			break;
		}
	}
}

void	SetThreadStateAndResult( unsigned char nResult, unsigned char nState )
{
	cCSLock lock( &gThreadShareData.mCs );
	gThreadShareData.mResultThread	= nResult;
	gThreadShareData.mStateThread	= nState;
}

bool DownLoadTextureFromHttp( unsigned long nUserIdx, const char* imgUrl )
{
	int len = strlen(imgUrl);
	if( len <= 0 || len <= (int)strlen("http://"))
	{
		assert(0);
		return false;
	}

	// url 주소 넘어오는 포맷					
	// 이미지를 등록한 경우와 등록하지 않은 경우 url이 틀리다. 추후 달라질 확률도 있다.
	// http://a1.twimg.com/profile_images/................."
	// http://s.twimg.com/a/고유ID/images/................."
	char* pTempImgUrl = const_cast<char*>(imgUrl);
	pTempImgUrl += strlen("http://");
	if( pTempImgUrl == NULL )
	{
		assert(0);
		return false;
	}

	char* pHomePageAddr	= strstr( pTempImgUrl, "/" );  
	if( pHomePageAddr == NULL )
	{
		assert(0);
		return false;		
	}
	cString imgUrlAddr = pHomePageAddr;	
	*pHomePageAddr = 0;

	Loader	HttpDownLoader;		// Http Url DownLoader

	if( HttpDownLoader.Open(  pTempImgUrl ) )
	{
		assert(0);
		return false;
	}	

	char* buffer    = NULL;
	DWORD bufferLen = 0;
	DWORD exitCode  = 0;
	DWORD retValue  = 0;
	
	retValue = HttpDownLoader.Get( const_cast<char*>(imgUrlAddr.Cstr()), &LoaderProgressRoutine, &exitCode, NULL, &buffer, &bufferLen );

	try
	{
		if( retValue == ERROR_INTERNET_TIMEOUT || retValue == ERROR_INTERNET_CONNECTION_RESET )
		{
			int retryConnetion = 0;

			do 
			{
				/// 재 접속
				if( HttpDownLoader.Open( const_cast<char*>( pTempImgUrl ) ) ) 
				{					
					return NULL;
				}

				retValue = HttpDownLoader.Get( pHomePageAddr, &LoaderProgressRoutine, &exitCode, NULL, &buffer, &bufferLen );

				if( ++retryConnetion > 20 )
				{
					throw(retValue);
				}

				if( retValue != 0 )
				{
					if( retValue != ERROR_INTERNET_CONNECTION_RESET && retValue != ERROR_INTERNET_TIMEOUT )
					{
						throw(retValue);
					}
				}

				if( THEAPP->IsActive() == false )
				{
					return false;
				}

				if( gThreadShareData.mShutDown == true )
				{
					return false;
				}

			} while( retValue != 0 );
		}
	} catch( ... ){

		HttpDownLoader.Close();
		GlobalFree( buffer );
		return false;
	}

	HttpDownLoader.Close();

	// 다운 받은 버퍼를 임시로 저장
	if ( buffer != NULL && bufferLen > 0 && retValue == 0)
	{
		sImgBufInfo* pImgBufInfo = new sImgBufInfo();

		pImgBufInfo->mBufLen= bufferLen;
		pImgBufInfo->mBuf	= buffer;

		cCSLock lock( &gThreadShareData.mCs );
		gThreadShareData.mImgBufferMap.Insert( nUserIdx, (void*)pImgBufInfo );


		return true;
	}

	return false;
}
DWORD WINAPI LoaderProgressRoutine(DWORD totalFileSize, DWORD totalBytesTransferred, void* data)
{
	if( totalFileSize == totalBytesTransferred )
		return 1;

	return 0;
}


//==========================================================================
//			* 트위터 시스템 참고( 글올리기) *
//
//	트위터 시스템은 무분별한 도배 방지를 위해
//	똑같은 문장을 웹, API에서 다시 작성시 등록되지 않는다. 
//	API에서는 성공 메시지가 return 되지만 실제로 트위터에 추가되지 않는다.
//	※ 해당 문장에 대해서 최근 10개 게시물중에 똑같은 문장이 있다면 등록되지 않는다.
//
//==========================================================================
DWORD StatusUpdate( LPCTSTR pSendMsg )
{
	if( !pSendMsg )
	{
		assert(0);
		SetThreadStateAndResult( eResult_Error_Default );
		return 0;
	}

	int nLen = ::_tcslen( pSendMsg );
	if( nLen <= 0 )
	{
		SetThreadStateAndResult( eResult_Error_Default );
		return 0;
	}

	int nUTF8 = 0;	
	if ( (nUTF8 = WideCharToMultiByte(CP_UTF8, 0, pSendMsg, nLen, NULL, 0, NULL, NULL)) <= 0 )
	{
		assert(0);
		SetThreadStateAndResult( eResult_Error_Default );
		return 0;
	}

	/// UNI -> UTF-8 문자열 변환
	char* buff = new char[nUTF8+1] ;	
	nUTF8 = WideCharToMultiByte(CP_UTF8, 0, pSendMsg, nLen, (LPSTR)buff, nUTF8, NULL, NULL);

	if( nUTF8 )
	{
		buff[nUTF8] = 0;
		cString replyMsg;

		CSBlock( &gThreadShareData.mCs )
		{
			if( gThreadShareData.mpTwitterContoller.statusUpdate( buff ) )
			{
				gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );
			}
		}

		SAFE_DELETE_ARRAY(buff);

		if( replyMsg.GetLength() > 0 )
		{
			TiXmlDocument document;
			document.Parse( replyMsg.Cstr(), 0 );		

			TiXmlElement* hash = document.FirstChildElement( "hash" );
			if( hash != NULL )
			{
				TiXmlElement* error = hash->FirstChildElement("error");
				if( error != NULL )
				{
					const char* pErrorText = error->GetText();

					if( strcmp( pErrorText, "User is over daily status update limit.") == 0 )
					{
						SetThreadStateAndResult( eResult_Error_StatusUpdateLimit );
					}
					else if( strcmp( pErrorText, "Could not authenticate you.") == 0 )
					{
						SetThreadStateAndResult( eResult_Error_GetTimeLineHome );
					}
					else
					{
						if( gThreadShareData.mStateThread == eState_Sending_UpdateMsg )	
						{
							SetThreadStateAndResult( eResult_Error_UpdateStatus );
						}
						else if( gThreadShareData.mStateThread == eState_Sending_EventMsg )
						{
							SetThreadStateAndResult( eResult_Error_EventUpdateStatus );
						}
					}

					return 0;
				}
			}

			TiXmlElement* root = document.FirstChildElement( "status" );
			if( root != NULL )
			{	
				TiXmlElement* pTextIdx = root->FirstChildElement("id");
				if( pTextIdx == NULL ){ goto End; }

				TiXmlElement* pText = root->FirstChildElement("text");
				if( pText == NULL ){ goto End; }

				TiXmlElement* pCreatedDate = root->FirstChildElement("created_at");
				if( pCreatedDate == NULL ){ goto End; }

				TiXmlElement* pCreatedSource = root->FirstChildElement("source");
				if( pCreatedSource == NULL ){ goto End; }

				TiXmlElement* pUserInfo = root->FirstChildElement("user");
				if( pUserInfo == NULL ){ goto End; }

				TiXmlElement* pIdx = pUserInfo->FirstChildElement("id");
				if( pIdx == NULL ){ goto End; }

				TiXmlElement* pUserID = pUserInfo->FirstChildElement("screen_name");
				if( pUserID == NULL ){ goto End; }

				TiXmlElement* pImgUrl = pUserInfo->FirstChildElement("profile_image_url");
				if( pImgUrl == NULL ){	goto End; }

				//============================  Parsing Data 셋팅 ======================================
				unsigned long nUserIdx		= ::atol( pUserID->GetText() );		// 트위터 유저 고유 Idx
				__int64 nTexIdx				= ::_atoi64(pTextIdx->GetText());	// 1개 글에 대한 고유 번호

				// 텍스쳐 생성		
				if( ::strlen( pImgUrl->GetText() ) > 0 )
				{
					cCSLock lock( &gThreadShareData.mCs );
					cImgBufferMap::cIterator i = gThreadShareData.mImgBufferMap.Find( nUserIdx );

					if( i == gThreadShareData.mImgBufferMap.End() )
					{
						/// 전송된 유저 정보가 기존 정보에 없는 새로운 정보라면 해당 유저의 프로필 사진을 Http 다운 & 텍스쳐화!
						if( DownLoadTextureFromHttp( nUserIdx, pImgUrl->GetText() ) == false )
						{
							goto End;
						}
					}
				}

				//==========================================================
				//	트위터 날짜는 영국 GTK 시간을 사용
				//	날짜 String Format = Tue Oct 20 05:29:48 +0000 2009
				//==========================================================
				char* dateTime = const_cast<char*>(pCreatedDate->GetText());
				SYSTEMTIME stCurrentTime, stCreatedTime;
				GetSystemTime(&stCurrentTime);				

				stCreatedTime.wMilliseconds = 0;
				stCurrentTime.wMilliseconds = 0;

				dateTime[3] = 0;
				if (_strnicmp(&dateTime[0], "Sun", 3) == 0) { stCreatedTime.wDayOfWeek = 0; }
				if (_strnicmp(&dateTime[0], "Mon", 3) == 0) { stCreatedTime.wDayOfWeek = 1; }
				if (_strnicmp(&dateTime[0], "Tue", 3) == 0) { stCreatedTime.wDayOfWeek = 2; }
				if (_strnicmp(&dateTime[0], "Wed", 3) == 0) { stCreatedTime.wDayOfWeek = 3; }
				if (_strnicmp(&dateTime[0], "Thu", 3) == 0) { stCreatedTime.wDayOfWeek = 4; }
				if (_strnicmp(&dateTime[0], "Fri", 3) == 0) { stCreatedTime.wDayOfWeek = 5; }
				if (_strnicmp(&dateTime[0], "Sat", 3) == 0) { stCreatedTime.wDayOfWeek = 6; }

				dateTime[7] = 0;
				if (_strnicmp(&dateTime[4], "Jan", 3) == 0) { stCreatedTime.wMonth = 1; }
				if (_strnicmp(&dateTime[4], "Feb", 3) == 0) { stCreatedTime.wMonth = 2; }
				if (_strnicmp(&dateTime[4], "Mar", 3) == 0) { stCreatedTime.wMonth = 3; }
				if (_strnicmp(&dateTime[4], "Apr", 3) == 0) { stCreatedTime.wMonth = 4; }
				if (_strnicmp(&dateTime[4], "May", 3) == 0) { stCreatedTime.wMonth = 5; }
				if (_strnicmp(&dateTime[4], "Jun", 3) == 0) { stCreatedTime.wMonth = 6; }
				if (_strnicmp(&dateTime[4], "Jul", 3) == 0) { stCreatedTime.wMonth = 7; }
				if (_strnicmp(&dateTime[4], "Aug", 3) == 0) { stCreatedTime.wMonth = 8; }
				if (_strnicmp(&dateTime[4], "Sep", 3) == 0) { stCreatedTime.wMonth = 9; }
				if (_strnicmp(&dateTime[4], "Oct", 3) == 0) { stCreatedTime.wMonth = 10; }
				if (_strnicmp(&dateTime[4], "Nov", 3) == 0) { stCreatedTime.wMonth = 11; }
				if (_strnicmp(&dateTime[4], "Dec", 3) == 0) { stCreatedTime.wMonth = 12; }

				dateTime[10] = 0;
				stCreatedTime.wDay			= (unsigned short)atoi( &dateTime[8] );
				dateTime[13] = 0;
				stCreatedTime.wHour			= (unsigned short)atoi( &dateTime[11] );
				dateTime[16] = 0;
				stCreatedTime.wMinute		= (unsigned short)atoi( &dateTime[14] );
				dateTime[19] = 0;
				stCreatedTime.wSecond		= (unsigned short)atoi( &dateTime[17] );
				stCreatedTime.wYear			= (unsigned short)atoi( &dateTime[26] );			

				__int64 nCreateDate, nCurrentDate;

				::SystemTimeToFileTime(&stCreatedTime, (LPFILETIME)&nCreateDate);
				::SystemTimeToFileTime(&stCurrentTime, (LPFILETIME)&nCurrentDate);

				__int64 nDiffTime = _abs64(nCurrentDate - nCreateDate);

				//초단위로 변환
				unsigned long sec	= (unsigned long)((nDiffTime / 10000) / 1000); //100나노 -> 초
				unsigned long day	= (unsigned long)(((sec / 60) / 60) / 24);
				unsigned long hour	= (unsigned long)((sec  / 60) / 60);
				unsigned long min	= (unsigned long)((sec  / 60) );

				unsigned long textIdx = 0;
				unsigned long diffTime = 0;

				if( day != 0 )
				{
					textIdx = 11735;
					diffTime = day;
				}
				else if( hour != 0 )
				{
					textIdx = 11734;
					diffTime = hour;
				}				
				else if( min != 0 )
				{
					textIdx = 11733;
					diffTime = min;
				}
				else if( sec != 0 )
				{
					textIdx = 11732;
					diffTime = sec;
				}

				//===============================================================
				// 작성한곳에 대한 html <A> 태그가 있을시 제거 한다.
				//===============================================================
				char* pSource =  const_cast<char*>(pCreatedSource->GetText() );
				if( strncmp(&pSource[0], "<a",2 ) ==  0)
				{
					pSource = strstr( pSource, ">" );
					pSource += 1;

					if( pSource != NULL )
					{
						char* pSearchPos = strstr( pSource, "</a>");
						if( pSearchPos != NULL )
							*pSearchPos = 0;
					}
				}

				int  len = 0;
				// 수신 Data 인코딩 및 임시 자료구조에 삽입
				sTwitDataInfo *pTwitDataInfo = new sTwitDataInfo();
				pTwitDataInfo->mUserIdx = nUserIdx;
				pTwitDataInfo->mTextIdx = nTexIdx;

				pTwitDataInfo->mText.Clear();
				pTwitDataInfo->mText.Reserve( 255 );	// 내용
				len = ConvertUTF8ToWideChar( pText->GetText(), (WCHAR*)pTwitDataInfo->mText.Cstr(), 255 );
				if( len > 0) pTwitDataInfo->mText[len] = 0;

				cStringT mCretedSource;
				mCretedSource.Reserve( 255 );			// 작성 날짜
				len = ConvertUTF8ToWideChar( pSource, (WCHAR*)mCretedSource.Cstr(), 255 );
				if( len > 0 ) mCretedSource[len] = 0;			

				pTwitDataInfo->mDate.Clear();
				pTwitDataInfo->mDate.Reserve(255);

				pTwitDataInfo->mDate.Format( GAMERESOURCEMAN->GetGameText( textIdx ), (int)diffTime, (LPCTSTR)mCretedSource.Cstr() );

				pTwitDataInfo->mID.Clear();
				pTwitDataInfo->mID.Reserve( 255 );		// 작성자
				len = ConvertUTF8ToWideChar( pUserID->GetText(), (WCHAR*)pTwitDataInfo->mID.Cstr(), 255 );
				if( len > 0 ) pTwitDataInfo->mID[len] = 0;

				CSBlock( &gThreadShareData.mCs )
				{
					gThreadShareData.mTwitDataMap.PushBack( (void*)pTwitDataInfo );			
				}

				if( gThreadShareData.mStateThread == eState_Sending_UpdateMsg )	
				{
					SetThreadStateAndResult( eResult_Success_SendMsg );
				}
				else if( gThreadShareData.mStateThread == eState_Sending_EventMsg )
				{
					SetThreadStateAndResult( eResult_Success_EventMsg );
				}

				return 0;
			}
		}				
	}

End:

	assert(0);
	SAFE_DELETE_ARRAY(buff);

	if(  gThreadShareData.mStateThread == eState_Sending_UpdateMsg )
	{
		SetThreadStateAndResult( eResult_Error_UpdateStatus );
	}
	else if( gThreadShareData.mStateThread == eState_Sending_EventMsg )
	{
		SetThreadStateAndResult( eResult_Error_EventUpdateStatus );
	}

	return 0;
}

DWORD	ReqRetweet( unsigned long idx )
{
	if( idx <= 0 )
		return 0; 

	cString replyMsg, userIdx;

	userIdx.Format( "%d", idx );

	CSBlock( &gThreadShareData.mCs )
	{
		if( gThreadShareData.mpTwitterContoller.statusRetweet( userIdx ) )
		{		
			gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );		
		}
	}

	if( replyMsg.GetLength() > 0 )
	{
		// TinyXML Parsing
		TiXmlDocument document;
		document.Parse( replyMsg.Cstr(), 0 );		

		TiXmlElement* root = document.FirstChildElement( "status" );

		if( root != NULL )
		{	
			TiXmlElement* pRetweet = root->FirstChildElement("retweeted_status");
			if( pRetweet == NULL )
			{	
				assert(0);
				SetThreadStateAndResult( eResult_Error_Retweet );
			}

			SetThreadStateAndResult( eResult_Success_Retweet );
			return 0;
		}				
	}
	
	SetThreadStateAndResult( eResult_Error_Retweet );	

	return 0;
}

DWORD	ReqFollow( unsigned long idx )
{
	if( idx <= 0 )
		return 0; 

	cString replyMsg, userIdx;

	userIdx.Format( "%d", idx );

	CSBlock( &gThreadShareData.mCs )
	{
		if( gThreadShareData.mpTwitterContoller.follow( userIdx ) )
		{		
			gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );		
		}
	}

	if( replyMsg.GetLength() > 0 )
	{
		// TinyXML Parsing
		TiXmlDocument document;
		document.Parse( replyMsg.Cstr(), 0 );		

		TiXmlElement* root = document.FirstChildElement( "user" );

		if( root != NULL )
		{	
			TiXmlElement* pStatus = root->FirstChildElement("status");
			if( pStatus == NULL )
			{	
				assert(0);
				SetThreadStateAndResult( eResult_Error_Follow );
			}

			SetThreadStateAndResult( eResult_Error_Follow );
			return 0;
		}				
	}

	SetThreadStateAndResult( eResult_Error_Follow );	

	return 0;

}


DWORD	GetHomeTimeLine()
{

	cString replyMsg;	

	CSBlock( &gThreadShareData.mCs )
	{
		if( gThreadShareData.mpTwitterContoller.timelineHomeGet() )
		{		
			gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );		
		}	
	}

	if( replyMsg.IsEmpty() )
	{			
		SetThreadStateAndResult( eResult_Error_Default );
		return 0;
	}

	// TinyXML Parsing
	TiXmlDocument document;
	document.Parse( replyMsg.Cstr(), 0, TIXML_ENCODING_UTF8 );

	//=========================================================
	//				계정 유효 여부 확인
	//=========================================================

	TiXmlElement* hash = document.FirstChildElement( "hash" );
	if( hash != NULL )
	{
		TiXmlElement* error = hash->FirstChildElement("error");
		if( error != NULL )
		{
			//== 계정 정보가 비정상일때
			if( strcmp( error->GetText(), "Could not authenticate you." ) == 0 )
			{				
				// 쓰레드 동작 상태에 따라 결과 값 셋팅
				if( gThreadShareData.mStateThread == eState_Sending_LoginMsg )	
				{
					SetThreadStateAndResult( eResult_Error_NoAccount );
				}
				else if( gThreadShareData.mStateThread == eState_Getting_TimeLineHomeMsg )
				{
					SetThreadStateAndResult( eResult_Error_GetTimeLineHome );
				}
			}
			//== 시간당 Api Request가 150개 이상일 때 
			else if( strcmp( error->GetText(), "Rate limit exceeded. Clients may not make more than 150 requests per hour.")  == 0 )
			{
				if( gThreadShareData.mpTwitterContoller.accountRateLimitGet() )
				{							
					gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );

					TiXmlDocument document;
					document.Parse( replyMsg.Cstr(), 0, TIXML_ENCODING_UTF8 );
					TiXmlElement* hash = document.FirstChildElement( "hash" );

					/// Api Request 150개로 초기화까지 남은 시간 계산
					if( hash )
					{
						TiXmlElement* resetTime	= hash->FirstChildElement( "reset-time-in-seconds");
						if( resetTime )
						{	
							FILETIME ft;
							SYSTEMTIME stCurrentTime, stResetTime;

							GetSystemTime(&stCurrentTime);					
							time_t t = (time_t)::atol( resetTime->GetText() );

							LONGLONG ll;				
							ll = Int32x32To64( t, 10000000) + 116444736000000000;
							ft.dwLowDateTime = (DWORD)ll;
							ft.dwHighDateTime = (DWORD)(ll >> 32);				
							FileTimeToSystemTime(&ft, &stResetTime);

							__int64 nResetDate, nCurrentDate, nDiffTime;

							::SystemTimeToFileTime(&stResetTime, (LPFILETIME)&nResetDate);
							::SystemTimeToFileTime(&stCurrentTime, (LPFILETIME)&nCurrentDate);
							nDiffTime = _abs64(nResetDate - nCurrentDate);

							//초단위로 변환
							unsigned long sec					= (unsigned long)((nDiffTime / 10000) / 1000); //100나노 -> 초
							gThreadShareData.mResetRemainMinute	= (unsigned long)((sec  / 60) );
							gThreadShareData.mResetRemainMinute++;
						}								
					}
				}

				SetThreadStateAndResult( eResult_Error_LimitExceed );
			}

			return 0;
		}
	}

	//=========================================================
	//				트위터 리스트 목록 파싱
	//=========================================================
	TiXmlElement* root = document.FirstChildElement( "statuses" );
	if( root == NULL )
	{
		assert(0);
		SetThreadStateAndResult( eResult_Error_Default );
		return 0;
	}

	TiXmlElement* pUserName			= NULL;
	TiXmlElement* pImgUrl			= NULL;
	TiXmlElement* pUserID			= NULL;
	TiXmlElement* pCreatedSource	= NULL;	
	TiXmlElement* pCreatedDate		= NULL;
	TiXmlElement* pUserElement		= NULL;	
	int len = 0;

	for(pUserElement	= root->FirstChildElement("status");
		pUserElement	!=NULL;
		pUserElement	= pUserElement->NextSiblingElement("status") )
	{

		if( gThreadShareData.mShutDown )
		{
			SetThreadStateAndResult( eResult_None );
			return 0;
		}
		//==================== User 정보 파싱!! =========================
		
		TiXmlElement* pUser =  pUserElement->FirstChildElement("user");
		if( pUser == NULL )
		{	
			assert(0); 
			continue;
		}		

		// 1개 글에 대한 고유 번호
		TiXmlElement* pTextIdx = pUserElement->FirstChildElement("id");
		if( pTextIdx == NULL )
		{	
			assert(0); 
			continue;
		}

		// 작성 내용
		TiXmlElement* pText = pUserElement->FirstChildElement("text");
		if( pText == NULL )
		{	
			assert(0); 
			continue;
		}

		// Retweet
		TiXmlElement* pRetweet =  pUserElement->FirstChildElement("retweeted_status");

		if( pRetweet != NULL )
		{
			TiXmlElement* pRetweetUser =  pRetweet->FirstChildElement("user");
			if( pRetweetUser )
			{	
				pCreatedDate	= pRetweet->FirstChildElement("created_at");
				pCreatedSource	= pRetweet->FirstChildElement("source");					// 작성 장소
				pImgUrl			= pRetweetUser->FirstChildElement("profile_image_url");		// 사용자 이미지 Url 주소
				pUserID			= pRetweetUser->FirstChildElement("id");					// 사용자 고유 Idx				
				pUserName		= pRetweetUser->FirstChildElement("screen_name");			// 사용자 Screen ID				

				if( pImgUrl == NULL || pUserID == NULL || pCreatedSource == NULL || pUserName == NULL || pCreatedDate == NULL )
				{
					assert(0); 
					continue;
				}		
			}
			else
			{
				assert(0); 
				continue;
			}
		}
		else
		{
			// 프로필 
			pCreatedDate	= pUserElement->FirstChildElement("created_at");
			pCreatedSource	= pUserElement->FirstChildElement("source");					// 작성 장소			
			pImgUrl			= pUser->FirstChildElement("profile_image_url");				// 사용자 이미지 Url 주소
			pUserID			= pUser->FirstChildElement("id");								// 사용자 고유 Idx			
			pUserName		= pUser->FirstChildElement("screen_name");						// 사용자 Screen ID			

			if( pImgUrl == NULL || pUserID == NULL || pCreatedSource == NULL || pUserName == NULL || pCreatedDate == NULL )
			{	
				assert(0); 
				continue;
			}		
		}

		//============================  Parsing Data 셋팅 ======================================
		unsigned long nUserIdx  = ::atol( pUserID->GetText() );		// 트위터 유저 고유 Idx
		__int64 nTexIdx			= ::_atoi64(pTextIdx->GetText());	// 1개 글에 대한 고유 번호

		// 텍스쳐 생성		
		if( ::strlen( pImgUrl->GetText() ) > 0 )
		{
			cCSLock lock( &gThreadShareData.mCs );
			cImgBufferMap::cIterator i = gThreadShareData.mImgBufferMap.Find( nUserIdx );

			if( i == gThreadShareData.mImgBufferMap.End() )
			{
				/// 전송된 유저 정보가 기존 정보에 없는 새로운 정보라면 해당 유저의 프로필 사진을 Http 다운 & 텍스쳐화!
				if( DownLoadTextureFromHttp( nUserIdx, pImgUrl->GetText() ) == false )
				{
					assert(0);
					continue;
				}
			}
		}

		//==========================================================
		//	트위터 날짜는 영국 GTK 시간을 사용
		//	날짜 String Format = Tue Oct 20 05:29:48 +0000 2009
		//==========================================================
		char* dateTime = const_cast<char*>(pCreatedDate->GetText());
		SYSTEMTIME stCurrentTime, stCreatedTime;
		GetSystemTime(&stCurrentTime);				

		stCreatedTime.wMilliseconds = 0;
		stCurrentTime.wMilliseconds = 0;

		dateTime[3] = 0;
		if (_strnicmp(&dateTime[0], "Sun", 3) == 0) { stCreatedTime.wDayOfWeek = 0; }
		if (_strnicmp(&dateTime[0], "Mon", 3) == 0) { stCreatedTime.wDayOfWeek = 1; }
		if (_strnicmp(&dateTime[0], "Tue", 3) == 0) { stCreatedTime.wDayOfWeek = 2; }
		if (_strnicmp(&dateTime[0], "Wed", 3) == 0) { stCreatedTime.wDayOfWeek = 3; }
		if (_strnicmp(&dateTime[0], "Thu", 3) == 0) { stCreatedTime.wDayOfWeek = 4; }
		if (_strnicmp(&dateTime[0], "Fri", 3) == 0) { stCreatedTime.wDayOfWeek = 5; }
		if (_strnicmp(&dateTime[0], "Sat", 3) == 0) { stCreatedTime.wDayOfWeek = 6; }

		dateTime[7] = 0;
		if (_strnicmp(&dateTime[4], "Jan", 3) == 0) { stCreatedTime.wMonth = 1; }
		if (_strnicmp(&dateTime[4], "Feb", 3) == 0) { stCreatedTime.wMonth = 2; }
		if (_strnicmp(&dateTime[4], "Mar", 3) == 0) { stCreatedTime.wMonth = 3; }
		if (_strnicmp(&dateTime[4], "Apr", 3) == 0) { stCreatedTime.wMonth = 4; }
		if (_strnicmp(&dateTime[4], "May", 3) == 0) { stCreatedTime.wMonth = 5; }
		if (_strnicmp(&dateTime[4], "Jun", 3) == 0) { stCreatedTime.wMonth = 6; }
		if (_strnicmp(&dateTime[4], "Jul", 3) == 0) { stCreatedTime.wMonth = 7; }
		if (_strnicmp(&dateTime[4], "Aug", 3) == 0) { stCreatedTime.wMonth = 8; }
		if (_strnicmp(&dateTime[4], "Sep", 3) == 0) { stCreatedTime.wMonth = 9; }
		if (_strnicmp(&dateTime[4], "Oct", 3) == 0) { stCreatedTime.wMonth = 10; }
		if (_strnicmp(&dateTime[4], "Nov", 3) == 0) { stCreatedTime.wMonth = 11; }
		if (_strnicmp(&dateTime[4], "Dec", 3) == 0) { stCreatedTime.wMonth = 12; }

		dateTime[10] = 0;
		stCreatedTime.wDay			= (unsigned short)atoi( &dateTime[8] );
		dateTime[13] = 0;
		stCreatedTime.wHour			= (unsigned short)atoi( &dateTime[11] );
		dateTime[16] = 0;
		stCreatedTime.wMinute		= (unsigned short)atoi( &dateTime[14] );
		dateTime[19] = 0;
		stCreatedTime.wSecond		= (unsigned short)atoi( &dateTime[17] );
		stCreatedTime.wYear			= (unsigned short)atoi( &dateTime[26] );			

		__int64 nCreateDate, nCurrentDate;

		::SystemTimeToFileTime(&stCreatedTime, (LPFILETIME)&nCreateDate);
		::SystemTimeToFileTime(&stCurrentTime, (LPFILETIME)&nCurrentDate);

		__int64 nDiffTime = _abs64(nCurrentDate - nCreateDate);

		//초단위로 변환
		unsigned long sec	= (unsigned long)((nDiffTime / 10000) / 1000); //100나노 -> 초
		unsigned long day	= (unsigned long)(((sec / 60) / 60) / 24);
		unsigned long hour	= (unsigned long)((sec  / 60) / 60);
		unsigned long min	= (unsigned long)((sec  / 60) );

		unsigned long textIdx = 0;
		unsigned long diffTime = 0;

		if( day != 0 )
		{
			textIdx = 11735;
			diffTime = day;
		}
		else if( hour != 0 )
		{
			textIdx = 11734;
			diffTime = hour;
		}				
		else if( min != 0 )
		{
			textIdx = 11733;
			diffTime = min;
		}
		else if( sec != 0 )
		{
			textIdx = 11732;
			diffTime = sec;
		}

		//===============================================================
		// 작성한곳에 대한 html <A> 태그가 있을시 제거 한다.
		//===============================================================
		char* pSource =  const_cast<char*>(pCreatedSource->GetText() );
		if( strncmp(&pSource[0], "<a",2 ) ==  0)
		{
			pSource = strstr( pSource, ">" );
			pSource += 1;

			if( pSource != NULL )
			{
				char* pSearchPos = strstr( pSource, "</a>");
				if( pSearchPos != NULL )
					*pSearchPos = 0;
			}
		}

		// 수신 Data 인코딩 및 임시 자료구조에 삽입
		sTwitDataInfo *pTwitDataInfo = new sTwitDataInfo();
		pTwitDataInfo->mUserIdx = nUserIdx;
		pTwitDataInfo->mTextIdx = nTexIdx;

		pTwitDataInfo->mText.Clear();
		pTwitDataInfo->mText.Reserve( 255 );	// 내용
		len = ConvertUTF8ToWideChar( pText->GetText(), (WCHAR*)pTwitDataInfo->mText.Cstr(), 255 );
		if( len > 0) pTwitDataInfo->mText[len] = 0;

		cStringT mCretedSource;
		mCretedSource.Reserve( 255 );			// 작성 날짜
		len = ConvertUTF8ToWideChar( pSource, (WCHAR*)mCretedSource.Cstr(), 255 );
		if( len > 0 ) mCretedSource[len] = 0;			

		pTwitDataInfo->mDate.Clear();
		pTwitDataInfo->mDate.Reserve(255);

		pTwitDataInfo->mDate.Format( GAMERESOURCEMAN->GetGameText( textIdx ), (int)diffTime, (LPCTSTR)mCretedSource.Cstr() );

		pTwitDataInfo->mID.Clear();
		pTwitDataInfo->mID.Reserve( 255 );		// 작성자
		len = ConvertUTF8ToWideChar( pUserName->GetText(), (WCHAR*)pTwitDataInfo->mID.Cstr(), 255 );
		if( len > 0 ) pTwitDataInfo->mID[len] = 0;

		CSBlock( &gThreadShareData.mCs )
		{
			gThreadShareData.mTwitDataMap.PushBack( (void*)pTwitDataInfo );			
		}			
	}

	///====================================================================================================
	//			
	//			※ 트위터는 자신이 작성했던 최근 10개 게시물중 동일 문장이 있는 경우 글이 입력되지 않는다.
	//			
	//			본인이 작성했던 10개 게시물에 대한 고유 Idx 추출
	///====================================================================================================
	if( gThreadShareData.mStateThread == eState_Sending_LoginMsg )		
	{	
		replyMsg.Clear();
		cString replyMsgCredential;
		unsigned long myIdx = 0;

		///============ 유저의 고유 IDX를 찾기 위해 ===============
		CSBlock( &gThreadShareData.mCs )
		{
			if( gThreadShareData.mpTwitterContoller.accountVerifyCredentialsGet() )
			{
				gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsgCredential );		
			}
		}

		if( replyMsgCredential.GetLength() > 0 )
		{
			TiXmlDocument document;
			document.Parse( replyMsgCredential.Cstr(), 0, TIXML_ENCODING_UTF8 );

			TiXmlElement* root = document.FirstChildElement("user");

			if( root != NULL )
			{
				TiXmlElement* pUserid = root->FirstChildElement("id");

				if( pUserid )
				{
					myIdx	= ::atol( pUserid->GetText() );	

					if( myIdx == 0 )
					{
						SetThreadStateAndResult( eResult_Error_NoAccount );
						return 0;
					}
				}
				else
				{
					SetThreadStateAndResult( eResult_Error_NoAccount );
					return 0;
				}
			}
			else
			{
				SetThreadStateAndResult( eResult_Error_NoAccount );
				return 0;
			}
		}
		///===========================================================

		///========= 유저가 작성한 최근 10개 게시물을 가져옴 =========
		CSBlock( &gThreadShareData.mCs )
		{			
			cString strIdx;
			strIdx.Format( "%d", myIdx );
			if( gThreadShareData.mpTwitterContoller.timelineUserGet( strIdx, true ) )
			{					
				gThreadShareData.mpTwitterContoller.getLastWebResponse( replyMsg );		
			}	
		}

		if( replyMsg.GetLength() > 0 )
		{
			/// 에러 체크 root 가 NULL 이면 Success!!
			TiXmlDocument document;
			document.Parse( replyMsg.Cstr(), 0, TIXML_ENCODING_UTF8 );
			
			TiXmlElement* root = document.FirstChildElement("hash");
			if( root != NULL )
			{
				TiXmlElement* error = root->FirstChildElement("error");
				const char* pErrorText = error->GetText();

				if( pErrorText != NULL )
				{
					if( strcmp( pErrorText, "Not found" ) == 0 )
					{
						SetThreadStateAndResult( eResult_Error_NoAccount );
					}
					else if( strcmp( pErrorText, "This method requires authentication.") == 0 )
					{
						SetThreadStateAndResult( eResult_Error_NoAccount );
					}
				}

				return 0;
			}
			
			root = document.FirstChildElement("statuses");
			if( root != NULL )
			{
				TiXmlElement* pUserElement = NULL;

				for(pUserElement	= root->FirstChildElement("status");
					pUserElement	!=NULL;
					pUserElement	= pUserElement->NextSiblingElement("status") )
				{
					if( gThreadShareData.mShutDown )
					{
						SetThreadStateAndResult( eResult_None );
						return 0;
					}		

					TiXmlElement* pTextIdx =  pUserElement->FirstChildElement("id");
					if( pTextIdx == NULL )
					{	
						assert(0); 
						continue;
					}

					__int64 nTexIdx	= ::_atoi64(pTextIdx->GetText());	// 1개 글에 대한 고유 번호
					gThreadShareData.mTiwtTextIdxList.PushBack( nTexIdx );
				}
			}
			else
			{
				SetThreadStateAndResult( eResult_Error_NoAccount );
				return 0;
			}
			
		}

		SetThreadStateAndResult( eResult_Success_Login );
	}
	else if( gThreadShareData.mStateThread == eState_Getting_TimeLineHomeMsg )
	{
		SetThreadStateAndResult( eResult_Success_GetList );
	}

	return 0;	

}


///=================================================================
//
//				아이디, 비밀번호 등록 윈도우
//
///=================================================================
cTwitterRegisterWindow::cTwitterRegisterWindow()
: mpID(0)
, mpPw(0)
, mpConnectAndSave(0)
, mpCancelBtn(0)
, mpResetBtn(0)
, mpDBSaveCheck(0)
{
	SetBackupVisible( false );
}

cTwitterRegisterWindow::~cTwitterRegisterWindow()
{
}

void cTwitterRegisterWindow::Open()
{

}

void cTwitterRegisterWindow::Clear()
{
	if( mpID )
		mpID->Clear();

	if( mpPw )
		mpPw->Clear();
}

bool cTwitterRegisterWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpID				= (cEditBox*)GetChild( eUIID_GAME_TWITTER_LOGIN_ID );
	mpPw				= (cEditBox*)GetChild( eUIID_GAME_TWITTER_LOGIN_PW );
	mpConnectAndSave	= (cButton*)GetChild( eUIID_GAME_TWITTER_LOGIN_CONNECT );
	mpCancelBtn			= (cButton*)GetChild( eUIID_GAME_TWITTER_LOGIN_CANCEL );
	mpResetBtn			= (cButton*)GetChild( eUIID_GAME_TWITTER_LOGIN_REFRESH );
	mpDBSaveCheck		= (cCheckBox*)GetChild( eUIID_GAME_TWITTER_LOGIN_IDSAVE );

	SetPressDBSaveCheck( true );
	SetResetBtnEnable( false );

	// DB에 저장되어 있다면 저장 
	char* pID = STAGEMAN->GetTwitterUid();
	char* pPW = STAGEMAN->GetTwitterUid();

	if( pID && pPW )
	{
		int nLenID = ::strlen( STAGEMAN->GetTwitterUid() );
		int nLenPW = ::strlen( STAGEMAN->GetTwitterUid() );

		if( nLenID > 0 && nLenPW > 0 )
		{
			SetResetBtnEnable( true );
		}
	}

	return true;
}

bool cTwitterRegisterWindow::IsDBSaveCheckPress()
{
	if( mpDBSaveCheck == NULL )
	{
		assert(0);
		return false;

	}
	
	return mpDBSaveCheck->IsPress();
}

void cTwitterRegisterWindow::SetPressDBSaveCheck( bool bPress )
{
	if( mpDBSaveCheck )
	{
		mpDBSaveCheck->SetPress( bPress );
	}
}

void cTwitterRegisterWindow::OnCommand( cUINode* caller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TWITTER_LOGIN_CONNECT:	///로그인 및 저장 여부
		{
			if( mpConnectAndSave && mpConnectAndSave->IsNetLock() )
			{
				return;
			}

			if( mpID->GetLength() <= 0 && mpPw->GetLength() <= 0 )
				return;

			if( mpID->GetLength() > LEN_TWITTER_UID && mVisible == true )		// 계정 길이가 제한을 넘을시
			{
				if( mVisible == true )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11748 ),	// 알림 메세지
						GAMERESOURCEMAN->GetGameText( 2 )		// 알림
						);
				}
				return;
			}

			if( mpPw->GetLength() > MAX_TWITTER_PWD && mVisible == true  )		// 비밀 번호 길이가 제한을 넘을시
			{
				if( mVisible == true )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11749 ),	// 알림 메세지
						GAMERESOURCEMAN->GetGameText( 2 )		// 알림
						);
				}
			
				return;
			}

			cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
			if( pTwitWin == NULL )
			{
				assert(0);
				return;
			}

			if( pTwitWin->IsLogin() == true )					// 로그인 중 계정 정보 저장 버튼 누를떄
			{
				char id[LEN_TWITTER_UID] = {0,};
				char pw[MAX_TWITTER_PWD] = {0,};
				ConvertToAscii( mpID->GetText(), id, LEN_TWITTER_UID );
				ConvertToAscii( mpPw->GetText(), pw, MAX_TWITTER_PWD );

				if( _stricmp( id, pTwitWin->GetID() ) == 0 &&	// 아이디, 비번 맞는 지 비교후 계정 정보만 저장
					_stricmp( pw, pTwitWin->GetPW() ) == 0 )		
				{
					Clear();

					if( pTwitWin->IsDBSaveAccount()  )	// 로그인 상태 + DB 저장 상태
					{
						if( mVisible == true)
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11742 ),	
								GAMERESOURCEMAN->GetGameText( 2 )		
								);		
						}										
					}
					else	// 로그인 상태 + DB 저장 상태X
					{
						if( mpDBSaveCheck->IsPress() )	//DB 저장 체크박스 체크
						{
							pTwitWin->SetRegistType( cTwitterWindow::eRegister_OnlySave );

							MSG_REQ_TWITTER msg;
							::memset( &msg, 0, sizeof(msg) );

							msg.Category  = NM_USER;
							msg.Protocol  = NM_USER_TWITTER_REQ;
							strncpy( msg.uid, pTwitWin->GetID(), MAX_TWITTER_UID );
							strncpy( msg.pwd, pTwitWin->GetPW(), MAX_TWITTER_PWD );

							NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );						
						}
						else			// DB 저장 체크 박스 체크 X
						{
							if( mVisible == true )
							{							
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 11742 ),	
									GAMERESOURCEMAN->GetGameText( 2 )		
									);
							}
						}
					}
					
					return;				
				}
			}

			pTwitWin->CreateTwitterThread( cTwitterWindow::eCommand_Login );
		}
		break;
	case eUIID_GAME_TWITTER_LOGIN_CANCEL:	///창 닫기
		{
			Hide();
		}
		break;
	case eUIID_GAME_TWITTER_LOGIN_REFRESH:	///계정 초기화
		{
			if( mpResetBtn && mpConnectAndSave->IsNetLock() )
			{
				return;
			}

			cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
			if( pTwitWin == NULL )
			{
				assert(0);
				return;
			}

			pTwitWin->SetRegistType( cTwitterWindow::eRegister_Refresh );

			MSG_REQ_TWITTER msg;
			::memset( &msg, 0, sizeof(msg) );

			msg.Category  = NM_USER;
			msg.Protocol  = NM_USER_TWITTER_REQ;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			NetLock();
		}
		break;
	}

}


void cTwitterRegisterWindow::SetResetBtnEnable( bool bEnable )
{
	if( mpResetBtn )
	{
		mpResetBtn->SetEnabled( bEnable );
	}
	else
	{
		assert(0);
	}
}


void cTwitterRegisterWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

LPCTSTR cTwitterRegisterWindow::GetID()
{
	if( mpID )
	{
		return mpID->GetText();
	}
	else
	{
		assert(0);
	}
	return NULL;
}

LPCTSTR cTwitterRegisterWindow::GetPW()
{
	if( mpPw )
	{
		return mpPw->GetText();
	}
	else
	{
		assert(0);
	}
	return NULL;
}

void cTwitterRegisterWindow::OnNetLock( int lockTry )
{
	if( mpID )
		mpID->NetLock( lockTry );

	if( mpPw )				
		mpPw->NetLock( lockTry );

	if( mpConnectAndSave )
		mpConnectAndSave->NetLock( lockTry );

	if( mpCancelBtn )
		mpCancelBtn->NetLock( lockTry );

	if( mpResetBtn )
		mpResetBtn->NetLock( lockTry );

	if( mpDBSaveCheck )		
		mpDBSaveCheck->NetLock( lockTry );	
}

void cTwitterRegisterWindow::OnNetUnLock( int lockTry )
{
	if( mpID )
		mpID->NetUnLock( lockTry );

	if( mpPw )				
		mpPw->NetUnLock( lockTry );

	if( mpConnectAndSave )
		mpConnectAndSave->NetUnLock( lockTry );

	if( mpCancelBtn )
		mpCancelBtn->NetUnLock( lockTry );

	if( mpResetBtn )
		mpResetBtn->NetUnLock( lockTry );

	if( mpDBSaveCheck )		
		mpDBSaveCheck->NetUnLock( lockTry );

}


//==================================================================================
// UI 리스트 박스

cTwitterListBox::cTwitterListBox( eUINodeType type )
: cImageListBox( type )
{
}

cTwitterListBox::~cTwitterListBox()
{
}

void cTwitterListBox::OnMouseMove( const cUIPos& pos )
{
	cListBox::OnMouseMove( pos );
	ShowTwitterTextTip( pos );	
}

void cTwitterListBox::OnMouseWheel( const cUIPos& pos, int wheel )
{
	cListBox::OnMouseWheel( pos, wheel );
	ShowTwitterTextTip( pos );
}

void cTwitterListBox::OnMouseLeft( const cUIPos& pos )
{
	cListBox::OnMouseLeft( pos );

	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( !pTip ) return;

	pTip->Hide();
}

void cTwitterListBox::ShowTwitterTextTip( const cUIPos& pos )
{
	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( !pTip ) return;

	if( mHoveredRowIdx != -1 )
	{		
		sRowData* rData = (sRowData*)mRows[ mTopRowIdx + mHoveredRowIdx ];
		if( rData )
		{
			sListImageData* data = (sListImageData*)rData->mColumes[0];
			if( data )
			{
				long strwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );

				if( strwidth > PRINT_TEXT_WIDTH * 2 )
				{	
					UIMAN->ShowCommunityTip( pTip, pos, data->mText.Cstr() );
				}
				else
				{
					UIMAN->HideTip();
				}
			}
			else
				assert(0);		
		}
		else
			assert(0);
	}
	else
	{
		UIMAN->HideTip();
	}
}

int cTwitterListBox::AddRowAtFirst( LPCTSTR str, unsigned long color, cUIImage* image, void* pExtraData )
{	

	sListImageData* rowData = new sListImageData;
	rowData->mText		= str;
	rowData->mTextColor	= color;
	rowData->mpImage	= image;

	/// 라인 정보 생성
	sRowData* data = new sRowData;

	/// 정보 갱신
	data->mColumes.PushBack( rowData );
	data->mExtraData = pExtraData;

	unsigned int rowCnt = GetRowCount();

	if( rowCnt == MAX_TWITTER_HAVE_LIST )
	{
		sRowData* endRowData = 0;
		
		endRowData = (sRowData*)mRows[ MAX_TWITTER_HAVE_LIST - 1 ];
		if( endRowData )
		{
			for( unsigned int b = 0; b < endRowData->mColumes.GetSize(); ++b )
			{
				sListImageData* data = (sListImageData*)(endRowData->mColumes[b]);
				if( data )
				{
					SAFE_DELETE( data->mpImage );
				}
				SAFE_DELETE( data );
			}
			endRowData->mColumes.Clear();

			endRowData->mExtraData = 0;
			SAFE_DELETE(endRowData);
		}
		else
		{
			assert(0);
			return -1;
		}

		for( int i = rowCnt - 1; i > 0; i-- )
		{
			mRows[ i ]	= mRows[ i - 1];
		}
	}
	else
	{
		if( rowCnt - 1 >= 0 )
		{
			mRows.PushBack( mRows[ rowCnt - 1 ] );

			for( int i = rowCnt - 1; i > 0; i-- )
			{
				mRows[ i ]	= mRows[ i - 1];
			}			
		}		
	}	

	mRows[ 0 ] = data;
		
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();

	return 0;
}

/// 리스트 박스의 아이템을 그려줌.
void cTwitterListBox::RenderData( cUIFontItemKeeper* pKeeper )
{
	/// 전체 리스트를 얻어옴 
	if( GetRowCount() <= 0 )
		return;

	/// 스킨 받아오기 
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
		return;

	int textposX				= 0;
	int textPosY				= GetAbsoluteRect().mTop;
	unsigned int nTotalCount	= GetRowCount();

	for( unsigned char iRow = 0; iRow < MAX_TWITTER_RENDER_LIST; iRow++ )
	{
		unsigned int row = mTopRowIdx + iRow;

		if( row >= nTotalCount )
			break;

		sRowData* rData = (sRowData*)mRows[ row ];
		if( rData == NULL )
		{
			assert(0);			
			continue;
		}		

		/// 작성자 이름, 작성 시간, 작성 내용
		unsigned int colCount = rData->mColumes.GetSize(); 
		for( unsigned int col = 0; col < colCount; ++col )
		{
			sListImageData* data = (sListImageData*)rData->mColumes[col];
			if( !data ) continue;

		
			sListCtrlParam *pParam = pListSkin->GetCtrlParam( col );

			if( !pParam )
			{
				assert(0);
				continue;
			}

			// 이미지 및 트위터 작성 내용 셋팅
			if( col == 0 && data->mpImage )
			{	
				// ============= 프로필 이미지 =============
				sListCtrlParam* pImgParam = pListSkin->mpImgParam;
				unsigned int imageX = pImgParam->mColumnX;
				unsigned int imageY = GetAbsoluteRect().mTop + 7 + ((mRowHeight + 2) * iRow); 
				data->mpImage->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
				data->mpImage->Draw();
				

				// ============= 작성 내용 =============
				textPosY = GetAbsoluteRect().mTop + 7 + ((mRowHeight + 2) * iRow );

				/// 전체 텍스트 width 구하기 
				LPTSTR str = (LPTSTR)data->mText.Cstr();
				int nTotaltextWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, str, data->mText.GetLength() );

				// 리스트 폭보다 텍스트 길이가 길다면 상, 하단 나눠서 출력
				if( nTotaltextWidth > PRINT_TEXT_WIDTH )	
				{	
					int ntexcount = 0;
					unsigned int strWidth = 0;
					while( strWidth < PRINT_TEXT_WIDTH )
					{
						++ntexcount;
						strWidth  = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, str, ntexcount );							
					}

					// 상단 글자 출력
					cStringT cTextStr;
					cTextStr.Reserve( data->mText.GetLength() + 1 );

					cTextStr.Insert( 0, str, 0, ntexcount );	
					textposX = UpdateTextPos( pParam->mAlign, strWidth, pParam->mColumnX, pParam->mColumnW );
					pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)cTextStr.Cstr(), textposX, textPosY, COLOR_WHITE );
					
					// 하단 글자 출력
					cTextStr.Clear();
					LPTSTR downText = NULL;
					str += ntexcount;
					strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, str, data->mText.GetLength() - ntexcount );
										
					if( strWidth > PRINT_TEXT_WIDTH )
					{
						strWidth = 0;
						ntexcount = 0;

						unsigned int nDotLen = _tcslen( GAMERESOURCEMAN->GetGameText( 11730) );
						while( strWidth < (PRINT_TEXT_WIDTH - nDotLen) )
						{
							++ntexcount;
							strWidth  = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, str, ntexcount );							
						}
						
						cTextStr.Insert( 0, str, 0, ntexcount );
						cTextStr.Append( GAMERESOURCEMAN->GetGameText( 11730) );
						downText = (LPTSTR)cTextStr.Cstr();

					}
					else									
					{	
						downText = (LPTSTR)&data->mText[ ntexcount ];
						strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, downText, data->mText.GetLength() - ntexcount );					
					}
					
					textposX = UpdateTextPos( pParam->mAlign, strWidth, pParam->mColumnX, pParam->mColumnW );
					textPosY += FONTAGENT->GetTextHeight(cFontAgent::eFont_System)  + 3;
					pKeeper->AddFontItem( cFontAgent::eFont_System, downText, textposX, textPosY, COLOR_WHITE );
				}
				//	한줄이면 Row Height 중간에 출력!!
				else		
				{
					textPosY = GetAbsoluteRect().mTop + 17 + ((mRowHeight + 2) * iRow );
					unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
					textposX = UpdateTextPos( pParam->mAlign, strWidth, pParam->mColumnX, pParam->mColumnW );
					pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), textposX, textPosY, COLOR_WHITE );
				}

			}
			// 작성자와 작성 시간 표시
			else
			{
				unsigned long color = COLOR_WHITE;
				unsigned short font = cFontAgent::eFont_UI;

				// 작성 시간
				if( col == 1 )
				{
					color = COLOR_TWITTER_DATE;
				}
				// 작성자
				else if( col == 2)
				{
					color = COLOR_TWITTER_ID;
					font  = cFontAgent::eFont_UI_Bold;
				}

				textPosY = GetAbsoluteRect().mTop + 42 + ((mRowHeight + 2) * iRow );

				unsigned int strWidth = FONTAGENT->GetTextExtent( font, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
				textposX = UpdateTextPos( pParam->mAlign, strWidth, pParam->mColumnX, pParam->mColumnW );
				pKeeper->AddFontItem( font, (LPTSTR)data->mText.Cstr(), textposX, textPosY, color );
			}				
		}
	}
}
