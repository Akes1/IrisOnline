#include "stdafx.h"

#include "ThemeEnterWindow.h"
#include "Label.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "Npc_Common.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "PartyManager.h"
#include "ItemManager.h"

cThemeEnterWindow::cThemeEnterWindow()
: mpEnter(0)
, mpClose(0)
, mpThemeName(0)
, mpThemeLevel(0)
, mpThemePeopleCnt(0)
//, mpThemeRefreshTime(0)
, mpThemeItem(0)
, mpThemeState(0)
, mpThemeUniqueIndex(0)
, mpThemeMode(0)
, mpMemberList(0)
, mThemeMapIndex(0)
{
	::memset( mpThemeEnterInfo, 0, sizeof( mpThemeEnterInfo ) );
}

cThemeEnterWindow::~cThemeEnterWindow()
{

}

void cThemeEnterWindow::Open()
{
}

void cThemeEnterWindow::Close()
{
	cUIWindow::Close();
	Clear();
}


bool cThemeEnterWindow::OnCreate( cUINodeProperty* pProperty )
{
	if( cUIWindow::OnCreate( pProperty ) == false )
		return false;
	
	mpEnter = (cButton*)GetChild( eUIID_GAME_THEME_ENTER_OK );
	mpClose = (cButton*)GetChild( eUIID_GAME_THEME_ENTER_CLOSE );
	mpThemeName =   (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_THEMENAME );
	mpThemeLevel = (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_LEVEL );
	mpThemePeopleCnt = (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_PEOPLE );
//	mpThemeRefreshTime = (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_REFRESH );
	mpThemeItem = (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_ITEM );
	mpThemeState =  (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_STATE );
	mpThemeUniqueIndex = (cLabel*)GetChild( eUIID_GAME_THEME_ENTER_THEMENUMBER );
	mpThemeMode = (cComboBox*)GetChild( eUIID_GAME_THEME_ENTER_MODE );
	mpMemberList =	(cListBox*)GetChild( eUIID_GAME_THEME_ENTER_MEMBERLIST );
	
	mpThemeUniqueIndex->Hide();
	return true;
	
}

void cThemeEnterWindow::Clear()
{
	mpThemeName->Clear();
	mpThemeState->Clear();
	mpThemeUniqueIndex->Clear();
	mpThemeUniqueIndex->Hide();
	mpMemberList->Clear();
	mpThemeMode->Clear();
	 	
	for( unsigned int cnt = 0; cnt < eTHEME_MODE_MAX; ++cnt )
	{
		if( mpThemeEnterInfo[cnt] )
		{
			cThemeEnterInfo::cIterator pos = mpThemeEnterInfo[cnt]->Begin();
			cThemeEnterInfo::cIterator end = mpThemeEnterInfo[cnt]->End();
			for( ; pos != end; ++pos )
			{
				sThemePartyInfo* pPartyInfo = (sThemePartyInfo*)(*pos).mSecond;
				SAFE_DELETE( pPartyInfo );
			}
		}
	}

	::memset( mpThemeEnterInfo, 0, sizeof( mpThemeEnterInfo ) );
	mThemeMapIndex = 0;
}

void cThemeEnterWindow::OnCommand( cUINode *node, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_THEME_ENTER_OK:
		{
			/// NPC대화창 닫기
			if( HERO->GetNpcFunc() == NPC_FUNC_THEME )
				HERO->CloseNpcTalk( true );

			/// 콤보 박스에 선택된 난이도로 들어가기
			unsigned char mode =  (unsigned char)mpThemeMode->GetSelectData();
			if( mode > eTHEME_MODE_NONE && mode < eTHEME_MODE_MAX )
			{
				MSG_REQ_THEME_READY msg;
				msg.Category = NM_THEME;
				msg.Protocol = NM_THEME_READY_REQ;
				msg.mThemeMapNum = mThemeMapIndex;
				msg.mThemeMode   = mode;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_THEME_READY ) );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_THEME_ENTER_WAIT, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 9020 ), 
					GAMERESOURCEMAN->GetGameText( 9000 )
					);
			}
			else
			{
				assert( 0 && "bad mode" );
			}
			Hide();
		}
		break;
	case eUIID_GAME_THEME_ENTER_CLOSE:
		/// NPC대화창 닫기
		if( HERO->GetNpcFunc() == NPC_FUNC_THEME )
			HERO->CloseNpcTalk( true );
		Hide();
		break;
	}
}

void cThemeEnterWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();


	cUINode* listNode = 0;
	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			if( ((cUINode*)(*i))->GetID() == eUIID_GAME_THEME_ENTER_MODE )
			{
				listNode = (cUINode*)(*i);
				continue;
			}

			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}
	pKeeper->DrawAll();

	if( listNode )
	{
		listNode->OnRender(pKeeper);
		pKeeper->DrawAll();
	}
}


void cThemeEnterWindow::HideESC( bool onsound )
{
	/// NPC대화창 닫기
	if( HERO->GetNpcFunc() == NPC_FUNC_THEME )
		HERO->CloseNpcTalk( true );

	cUIWindow::HideESC( onsound );
}

void cThemeEnterWindow::SetThemeEnterInfo( MSG_SYN_THEME_READYQUESTION *pSynMsg )
{

	Clear();

	/// 던젼 명 설정
	if( mpThemeName )
		mpThemeName->SetText( GAMERESOURCEMAN->GetMapName( pSynMsg->mThemeMapNum ) );

	/// 상태 미귀속.
	if( mpThemeState )
		mpThemeState->SetText( UIMAN->GetUIText( 26521 ) ); 

	/// 콤보 박스 설정
	tArray<unsigned char> modeArr;
	GAMERESOURCEMAN->GetMapMode( pSynMsg->mThemeMapNum, &modeArr );
	for( unsigned int cnt = 0; cnt < modeArr.GetSize(); ++cnt )
	{
		unsigned char mode = (unsigned char)modeArr[cnt];
		mpThemeMode->AddItem( UIMAN->GetUIText( 26500 + mode ), (void*)mode );
	}
	
	sThemePartyInfo* pPartyInfo = pSynMsg->mPartyInfo;	
	
	/// 데이터 수집
	for( unsigned int cnt=0 ; cnt < pSynMsg->mCount; cnt++, pPartyInfo++ )
	{
		int mode = pPartyInfo->mThemeMode;
		/// 콤보 박스 추가.
		if( mpThemeEnterInfo[mode] )
		{
			sThemePartyInfo* pInfo = new sThemePartyInfo;
			if( pInfo )
			{
				::memcpy( pInfo, pPartyInfo, sizeof( sThemePartyInfo ) );
				mpThemeEnterInfo[mode]->Insert( pInfo->mPlayerIdx, pInfo );
			}
			else
			{
				assert( 0 && "Fail to memmory allocation");
				return;
			}
		}
		else
		{
			mpThemeEnterInfo[mode] = new cThemeEnterInfo;
			if( mpThemeEnterInfo[mode] )
			{
				sThemePartyInfo* pInfo = new sThemePartyInfo;
				if( pInfo )
				{
					::memcpy( pInfo, pPartyInfo, sizeof( sThemePartyInfo ) );
					mpThemeEnterInfo[mode]->Insert( pInfo->mPlayerIdx, pInfo );
				}
				else
				{
					assert( 0 && "Fail to memmory allocation");
					return;
				}
			}
			else
			{
				assert( 0 && "Fail to memmory allocation");
				return;
			}
		}
	}

	mThemeMapIndex = pSynMsg->mThemeMapNum;

	Show( true );

	mpThemeMode->SetSelect( 0, true );

	unsigned char mode =  (unsigned char)mpThemeMode->GetSelectData();

	///  최소 요구 레벨
	if( mpThemeLevel )
	{
		unsigned char level = GAMERESOURCEMAN->GetMapMinLevel( pSynMsg->mThemeMapNum, mode );
		if( level > 0 )
		{
			cStringT str;
			str.Format( _T("%d"), level );
			mpThemeLevel->SetText(  (LPCTSTR)str.Cstr() );
		}
		else
		{
			mpThemeLevel->Clear();
		}
	}

	/// 최대 참가 인원
	if( mpThemePeopleCnt )
	{
		unsigned char playerCnt = GAMERESOURCEMAN->GetMapMaxPlayerCount( pSynMsg->mThemeMapNum, mode );
		if( playerCnt > 0 )
		{
			cStringT str;
			str.Format( _T("%d"), playerCnt );
			mpThemePeopleCnt->SetText( (LPCTSTR)str.Cstr() );
		}
		else
		{
			mpThemePeopleCnt->Clear();
		}
	}

	/// 입장 요구 아이템
	if( mpThemeItem )
	{		
		unsigned long itemIdx = GAMERESOURCEMAN->GetMapRequireItem( pSynMsg->mThemeMapNum, mode );
		if( itemIdx > 0 )
		{
			LPCTSTR pItemName = ITEMMAN->GetItemName( itemIdx );
			if( pItemName )
			{
				mpThemeItem->SetText( pItemName );
			}
			else
			{
				assert(0);
				mpThemeItem->Clear();
			}
		}
		else
		{
			mpThemeItem->Clear();
		}
	}		

}

void cThemeEnterWindow::OnComboSelected( cUINode* caller, unsigned int id )
{
	///초기화
	mpThemeState->SetText( UIMAN->GetUIText( 26521 ) ); 
	mpMemberList->Clear();
	mpThemeUniqueIndex->Hide();

	/// 맴버에 추가.
	if( mpMemberList == NULL || mpThemeState == NULL )
	{
		assert(0);
		return;
	}

	unsigned char mode =  (unsigned char)mpThemeMode->GetSelectData();

	///  최소 요구 레벨
	if( mpThemeLevel )
	{
		unsigned char level = GAMERESOURCEMAN->GetMapMinLevel( mThemeMapIndex, mode );
		if( level > 0 )
		{
			cStringT str;
			str.Format( _T("%d"), level );
			mpThemeLevel->SetText(  (LPCTSTR)str.Cstr() );
		}
		else
		{
			mpThemeLevel->Clear();
		}
	}

	/// 최대 참가 인원
	if( mpThemePeopleCnt )
	{
		unsigned char playerCnt = GAMERESOURCEMAN->GetMapMaxPlayerCount( mThemeMapIndex, mode );
		if( playerCnt > 0 )
		{
			cStringT str;
			str.Format( _T("%d"), playerCnt );
			mpThemePeopleCnt->SetText( (LPCTSTR)str.Cstr() );
		}
		else
		{
			mpThemePeopleCnt->Clear();
		}
	}

	/// 입장 요구 아이템
	if( mpThemeItem )
	{		
		unsigned long itemIdx = GAMERESOURCEMAN->GetMapRequireItem( mThemeMapIndex, mode );
		if( itemIdx > 0 )
		{
			LPCTSTR pItemName = ITEMMAN->GetItemName( itemIdx );
			if( pItemName )
			{
				mpThemeItem->SetText( pItemName );
			}
			else
			{
				assert(0);
				mpThemeItem->Clear();
			}
		}
		else
		{
			mpThemeItem->Clear();
		}
	}		

	cThemeEnterInfo* pThemeInfo = mpThemeEnterInfo[ mode ];
	if( pThemeInfo )
	{
		/// HERO 테마 귀속 상태 ui 설정.
		sThemePartyInfo* pPartyInfo = (sThemePartyInfo*)pThemeInfo->GetAt( HERO->GetObjectID() );
		if( pPartyInfo )
		{
			/// 귀속 정보가 있을 경우에만 서버에서 데이터를 줌. 
			/// 상태:귀속
			mpThemeState->SetText( UIMAN->GetUIText( 26520 ) );
			/// 고유번호
			cStringT uniqueIndex;
			uniqueIndex.Format( _T("%d"), pPartyInfo->mThemeUniqueIdx );
			mpThemeUniqueIndex->SetText( uniqueIndex.Cstr() );
			mpThemeUniqueIndex->Show();
		
		}

		/// 파티원 테마 귀속 상태 ui 설정
		unsigned int partyMax = PARTYMAN->GetPartyCount();
		for( unsigned int index = 0; index < partyMax; ++index )
		{
			sPartyUserData* pPartyData;
			pPartyData = PARTYMAN->GetPartyInfo( index );
			if( pPartyData )
			{
				sThemePartyInfo* pPartyInfo = (sThemePartyInfo*)pThemeInfo->GetAt( pPartyData->userIndex );
				if( pPartyInfo )
				{
					/// 레벨
					cStringT str;
					str.Format( _T("%d"), pPartyData->level );

					int row = mpMemberList->AddRow( str.Cstr(), COLOR_WHITE, NULL );

					if( row > -1 )
					{
						/// 이름
						if( pPartyData->name )
							mpMemberList->AddColume( row, pPartyData->name, COLOR_WHITE );

						/// 직업
						LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( pPartyData->job );
						if( jobName )
							mpMemberList->AddColume( row, jobName, COLOR_WHITE );

						/// 상태
						if( pPartyInfo->mIsJoin )
							mpMemberList->AddColume( row, UIMAN->GetUIText( 26522 ), COLOR_WHITE );
						else
							mpMemberList->AddColume( row, UIMAN->GetUIText( 26523 ), COLOR_WHITE );

						if( pPartyInfo->mThemeUniqueIdx != 0)
						{
							/// 고유번호
							cStringT uniqueIndex;
							uniqueIndex.Format( _T("%d"), pPartyInfo->mThemeUniqueIdx );
							mpMemberList->AddColume( row, uniqueIndex.Cstr(), COLOR_WHITE );
						}
						else
						{
							mpMemberList->AddColume( row, UIMAN->GetUIText( 26521 ), COLOR_WHITE );
						}
					}
				}
				else
				{
					///파티원의 귀속 정보가 없는 경우 기본 셋팅
					/// 레벨
					cStringT str;
					str.Format( _T("%d"), pPartyData->level );

					int row = mpMemberList->AddRow( str.Cstr(), COLOR_WHITE, NULL );

					if( row > -1 )
					{
						/// 이름
						if( pPartyData->name )
							mpMemberList->AddColume( row, pPartyData->name, COLOR_WHITE );

						/// 직업
						LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( pPartyData->job );
						if( jobName )
							mpMemberList->AddColume( row, jobName, COLOR_WHITE );

						/// 미진입 상태
						mpMemberList->AddColume( row, UIMAN->GetUIText( 26523 ), COLOR_WHITE );

						/// 미귀속 상태
						mpMemberList->AddColume( row, UIMAN->GetUIText( 26521 ), COLOR_WHITE );
					}
				}
			}
			else
			{
				assert(0);
				continue;
			}
		}
	}
	else
	{
		/// 파티원들이 모두 귀속 정보가 없다면 기본 셋팅
		/// 파티원 테마 귀속 상태 ui 설정
		unsigned int partyMax = PARTYMAN->GetPartyCount();
		for( unsigned int index = 0; index < partyMax; ++index )
		{
			sPartyUserData* pPartyData;
			pPartyData = PARTYMAN->GetPartyInfo( index );
			if( pPartyData )
			{
				///파티원의 귀속 정보가 없는 경우 기본 셋팅
				/// 레벨
				cStringT str;
				str.Format( _T("%d"), pPartyData->level );

				int row = mpMemberList->AddRow( str.Cstr(), COLOR_WHITE, NULL );

				if( row > -1 )
				{
					/// 이름
					if( pPartyData->name )
						mpMemberList->AddColume( row, pPartyData->name, COLOR_WHITE );

					/// 직업
					LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( pPartyData->job );
					if( jobName )
						mpMemberList->AddColume( row, jobName, COLOR_WHITE );

					/// 미진입 상태
					mpMemberList->AddColume( row, UIMAN->GetUIText( 26523 ), COLOR_WHITE );

					/// 미귀속 상태
					mpMemberList->AddColume( row, UIMAN->GetUIText( 26521 ), COLOR_WHITE );
				}
			}
			else
			{
				assert(0);
				return;
			}
		}
	}
}
