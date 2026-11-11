#include "stdafx.h"
#include "StageManager.h"
#include "IntroStage.h"
#include "LoginStage.h"
#include "ServerStage.h"

#include "SelectStage.h"
#include "MakeStage.h"
#include "LoadStage.h"
#include "MapChange.h"
#include "GameStage.h"

#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"
#include "WorldManager.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "DramaturgyManager.h"
#include "ShortcutManager.h"
#include "UIManager.h"
#include "ItemManager.h"
#include "TarotManager.h"
#include "FilterManager.h"
#include "Application.h"
#include "CoolTimeManager.h"
#include "QuickManager.h"
#include "PartyManager.h"
#include "CameraManager.h"
#include "QuestManager.h"
#include "TriggerManager.h"
#include "CheatManager.h"
#include "PVPManager.h"
#include "TitleManager.h"
#include "LightAgent.h"
#include "GuildManager.h"
#include "TipManager.h"
#include "ItemMixManager.h"
#include "PartyUnionManager.h"
#include "TutorialManager.h"
#include "ChatManager.h"
#include "ThemeManager.h"
#include "WebManager.h"

#include "InputSystem.h"
#include "SoundSystem.h"

#include "uicursor.h"

#ifdef _GMTOOL
	#include "GMTool.h"
#endif


cStageManager* cStageManager::mpStageManager = NULL;

//unsigned long size[256][256];
//unsigned long check[256][256];

cStageManager::cStageManager()
{
//	ZeroMemory(check, sizeof(unsigned long)*256*256 );
//	ZeroMemory(size, sizeof(unsigned long)*256*256 );

	mpStageManager = this;

	for( int i = 0; i < eStage_Max; ++i )
		mpStage[i] = NULL;
	mCurStage = eStage_Intro;

    mUserIdx  = 0;
	mLoginIdx = 0;
	mChannelIdx = 0;
	mServerIdx = -1;
	mCurrentMapIdx = 0;
	mCurrentMapMode = 0;
	mIsAdultServer = false;

	///
	mpWebManager = NULL;
	mpThemeManager = NULL;
	mpUIManager = NULL;
	mpWorldManager = NULL;
	mpObjectManager = NULL;
	mpGameResourceManager = NULL;
	mpSkillManager = NULL;
	mpDramaturgyManager = NULL;
	mpShortcutManager = NULL;
	mpItemManager = NULL;
	mpTarotManager = NULL;
	mpFilterManager = NULL;
	mpCoolManager = NULL;
	mpQuickManager = NULL;
	mpPartyManager = NULL;
	mpQuestManager = NULL;
	mpTriggerManager = NULL;
	mpCheatManager = NULL;
	mpPvpManager = NULL;
	mpTitleManager = NULL;
	mpGuildManager = NULL;
	mpTipManager = NULL;
	mpItemMixManager = NULL;
	mpPartyUnionManager = NULL;
	mpTutorialManager = NULL;
	mIOCPCLient = NULL;
}

cStageManager::~cStageManager()
{
	//Exit();
/*
	cFileSaver saver;
	char buf[256] = {0,};

	SYSTEMTIME t;
	::GetLocalTime(&t);

	::sprintf_s(buf, "packet_check_%04d-%02d-%02d %02d-%02d-%02d.txt", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	if( saver.Open( buf ) == true )
	{
		/// window mode
		for( unsigned int i=0;i<256; ++i )
		{
			for( unsigned int k=0;k<256; ++k )
			{
				if( check[i][k] != 0 || size[i][k] != 0 )
				{
					::sprintf_s( buf, "Category[%d] Packet[%d] Count[%d] Size[%d]\r\n", i, k, check[i][k], size[i][k] );
					saver.Write( buf, strlen(buf) );
				}
			}
		}
	}
*/
}

bool cStageManager::Init( HWND hWnd, unsigned int /*screenWidth*/, unsigned int /*screenHeight*/ )
{
	/// 스테이지 생성전에 네트워크 및 각종 관리자들을 생성
	/// 네트워크 생성 및 초기화

	/// 070330 네트워크 클래스
	mIOCPCLient = new cIOCPClient( hWnd, WM_SOCKEVENTMSG2 );
	if ( !mIOCPCLient || !mIOCPCLient->Initialize( ) )
		return false;

	mpDramaturgyManager = new cDramaturgyManager;
	if( !mpDramaturgyManager )
	{
		SAFE_DELETE(mpDramaturgyManager);
		assert( 0 && "failed to init dramaturgy manager" );
		return false;
	}

	/// 필터관리자 생성 및 초기화
	mpFilterManager = new cFilterManager;
	if( mpFilterManager == 0 || mpFilterManager->Init() == false )
	{
		return false;
	}

	/// 게임 리소스 관리자 생성 및 초기화
	mpGameResourceManager = new cGameResourceManager;
	if( !mpGameResourceManager || !mpGameResourceManager->Init() )
	{
		return false;
	}

	mpUIManager = new cUIManager;
	if( !mpUIManager || mpUIManager->Init() == false )
	{
		return false;
	}
	
	mpCoolManager = new cCoolTimeManager;
	if( mpCoolManager == 0 )
	{
		return false;
	}

	/// 아이템 관리자 생성 및 초기화
	mpItemManager = new cItemManager;
	if( mpItemManager == 0 )
	{
		return false;
	}

	/// 퀵슬롯 관리자 생성 및 초기화 
	mpQuickManager = new cQuickManager;
	if( mpQuickManager == 0 )
	{
		return false;
	}

	/// 타롯 관리자 생성 및 초기화 
	mpTarotManager = new cTarotManager;
	if( mpTarotManager == 0 || mpTarotManager->Init() == false )
	{
		return false;
	}

	/// 070201 웅주, 단축키 관리자 생성
	mpShortcutManager = new cShortcutManager;
	if( !mpShortcutManager )
	{
		return false;
	}

	/// 파티관리자 생성 및 초기화
	mpPartyManager = new cPartyManager;
	if( mpPartyManager == 0 )
	{
		return false;
	}

	mpTriggerManager = new cTriggerManager;
	if( mpTriggerManager == 0 /*|| mpTriggerManager->Init() == false*/ )
	{
		return false;
	}

	mpQuestManager = new cQuestManager;
	if( mpQuestManager == 0 /*|| mpQuestManager->Init() == false*/ )
	{
		return false;
	}

	/// 월드 관리자 생성 및 초기화
	mpWorldManager = new cWorldManager;
	if( !mpWorldManager )//|| !mpWorldManager->Init() )
	{
		return false;
	}

	/// 오브젝트 관리자 생성 및 초기화
	mpObjectManager = new cObjectManager;
	if( !mpObjectManager || !mpObjectManager->Init() )
	{
		return false;
	}

	/// 스킬 관리자를 생성 및 초기화
	mpSkillManager = new cSkillManager;
	if( !mpSkillManager || !mpSkillManager->Init() )
	{
		return false;
	}

	/// 치트 관리자를 생성
	mpCheatManager = new cCheatManager;

	/// PVP 관리자를 생성
	mpPvpManager = new cPVPManager;
	if( !mpPvpManager )
	{
		return false;
	}

	/// 호칭 관리자 생성
	mpTitleManager = new cTitleManager;
	if( !mpTitleManager )
	{
		return false;
	}

	/// 길드 관리자 생성
	mpGuildManager = new cGuildManager;
	if( !mpGuildManager || !mpGuildManager->Init() )
	{
		return false;
	}

	/// 팁 관리자 생성
	mpTipManager = new cTipManager;
	if( !mpTipManager || !mpTipManager->Init() )
	{
		return false;
	}

	/// 아이템 조합 관리자
	mpItemMixManager = new cItemMixManager;
	if( !mpItemMixManager )
	{
		return false;
	}

	/// 파티 연합 관리자
	mpPartyUnionManager = new cPartyUnionManager;
	if( !mpPartyUnionManager )
	{
		return false;
	}

	mpTutorialManager = new cTutorialManager;
	if( !mpTutorialManager || !mpTutorialManager->Init() )
	{
		return false;
	}

	mpThemeManager = new cThemeManager;
	if( !mpThemeManager )
	{
		return false;
	}

	mpWebManager = new cWebManager;
	if( !mpWebManager || !mpWebManager->Init( THEAPP->GetWindowInstance(), THEAPP->GetHWND() ) ) 
	{
		return false;
	}

	/// 각 스테이지 생성
	mCurStage = eStage_Intro;
	mpStage[eStage_Intro] = new cIntroStage;

	mpStage[eStage_Login] = new cLoginStage;

  	mpStage[eStage_Server] = new cServerStage;

	mpStage[eStage_Select] = new cSelectStage;

	mpStage[eStage_Make] = new cMakeStage;

	mpStage[eStage_Load] = new cLoadStage;

	mpStage[eStage_MapChange] = new cMapChangeStage;

	mpStage[eStage_Game] = new cGameStage;

	/// 배경음 로드
	SOUNDSYS->LoadBGMList();

	/// 각 stage 초기화
	for( int i = 0; i < eStage_Max; ++i )
	{
		mpStage[i]->Init( this );
	}

	/// stage open
	mpStage[mCurStage]->InitStage();

	mpStage[mCurStage]->Open();

	return true;
}

void cStageManager::Exit()
{
	if( mpGameResourceManager )
		mpGameResourceManager->LoadEnd();

	/// 생성 역순으로...
	if( mpStage[mCurStage] )
        mpStage[mCurStage]->Close();

	for( int i = 0; i < eStage_Max; ++i )
	{
		SAFE_EXIT_DELETE( mpStage[i] );
	}

	SAFE_DELETE( mpWebManager );
	SAFE_DELETE( mpThemeManager );
	SAFE_DELETE( mpTutorialManager );
	SAFE_DELETE( mpPartyUnionManager );
	SAFE_DELETE( mpItemMixManager );
	SAFE_DELETE( mpTipManager );
	SAFE_DELETE( mpGuildManager );
	SAFE_DELETE( mpTitleManager );
	SAFE_DELETE( mpPvpManager );
	SAFE_DELETE( mpCheatManager );
	SAFE_DELETE( mpSkillManager );	
	SAFE_EXIT_DELETE( mpObjectManager );
	SAFE_EXIT_DELETE( mpWorldManager );
	SAFE_DELETE( mpQuestManager );
	SAFE_DELETE( mpTriggerManager );
	SAFE_DELETE( mpPartyManager );
	SAFE_DELETE( mpShortcutManager );
	SAFE_DELETE( mpTarotManager );
	SAFE_DELETE( mpQuickManager );
	SAFE_DELETE( mpItemManager );
	SAFE_DELETE( mpCoolManager );
	SAFE_DELETE( mpUIManager );
	SAFE_EXIT_DELETE( mpGameResourceManager );
	SAFE_DELETE( mpFilterManager );
	SAFE_DELETE( mpDramaturgyManager );
	SAFE_DELETE( mIOCPCLient );
}

static char buffer[256] = {0,};
void cStageManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	SOUNDSYS->ProcessStageBGM( accumTime );

	/// skill
	mpSkillManager->Process( deltaTime, accumTime );

	/// 관리 객체들의 변환
    mpObjectManager->Process( deltaTime, accumTime );

	if( mCurStage == eStage_Login )
		mpStage[eStage_Intro]->Process( deltaTime, accumTime );

	///
	mpStage[mCurStage]->Process( deltaTime, accumTime );

	/// 쿨타임 
	mpCoolManager->Process( accumTime );

	/// 퀘스트
	mpQuestManager->Process( accumTime );

	mpPvpManager->Process( deltaTime, accumTime );

	///
	mpPartyManager->Process( accumTime );

	/// 
	mpPartyUnionManager->Process( accumTime );

	/// 
	mpTutorialManager->Process( deltaTime, accumTime );

	/// ui
	mpUIManager->Process( deltaTime, accumTime );
}

void cStageManager::ProcessWorld( unsigned long deltaTime, unsigned long accumTime )
{
	/// 바닥과 스카이돔에 대한 처리 ( 별도 랜더링 되어질 객체를 구분한다. )
	if( mCurStage == eStage_Game )
		mpWorldManager->Process( deltaTime, accumTime );
}

void cStageManager::NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length )
{
/*
	MSGROOT* msg = (MSGROOT*)pMsg;
	if( msg )
	{
		check[msg->Category][msg->Protocol]++;
		size[msg->Category][msg->Protocol] += length;
	}
*/
	mpStage[mCurStage]->NetworkMsgParser( connectionindex, pMsg, length );
}

void cStageManager::MsgBoxEventParser( unsigned int event, unsigned int result, void* exData )
{
	mpStage[mCurStage]->MsgBoxEventParser( event, result, exData );
}

void cStageManager::ProcessKeyPressed( cKeyboardAgent* pKeyboard )
{
	assert(pKeyboard);
	if( SHORTCUTMAN->ProcessKeyPressed( pKeyboard ) == false )
	{
		mpStage[mCurStage]->ProcessKeyPressed( pKeyboard );
	}
}

void cStageManager::ProcessKeyUpDown( unsigned long time, cKeyboardAgent* pKeyboard )
{
	assert(pKeyboard);
	if( SHORTCUTMAN->ProcessKeyUpDown( pKeyboard ) == false )
	{
		mpStage[mCurStage]->ProcessKeyUpDown( time, pKeyboard );
	}
}

void cStageManager::ProcessMouseEvent( unsigned long time, cMouseAgent* pMouse )
{
	assert(pMouse);
	if( pMouse->LButtonDoubleClick() )
	{
		/// LBtnDoubleClick 이벤트 처리
		mpStage[mCurStage]->ProcessLButtonDClick( pMouse );
	}
	else
	{
		if( pMouse->LButtonDown() )
		{
			/// LBtnClick 이벤트 처리
			mpStage[mCurStage]->ProcessLButtonClick( pMouse );
		}
		else if( pMouse->RButtonDown() )
		{
			mpStage[mCurStage]->ProcessRButtonDown( pMouse );
		}
		else if( pMouse->RButtonUp() )
		{
			mpStage[mCurStage]->ProcessRButtonUp( pMouse );
		}
		else if( pMouse->IsDrag() )
		{
			/// mouse over에 대한 처리
			mpStage[mCurStage]->ProcessMouseover( time, pMouse );
		}
		else
		{
			mpStage[mCurStage]->ProcessMouseover( time, pMouse );
		}
	}
}

void cStageManager::ChangeStage( eSTAGE stage )
{
	if( mCurStage == stage )	return;

	/// 스테이지의 배경 사운드 관리
	ManagedBGM( mCurStage, stage );

	mpStage[mCurStage]->Close();

	/// 기존 스테이지에서 사용한 정보를 제거한다.
	INPUTSYS->InputInfoClearAll();

	if( stage != eStage_Game )
	{
		LIGHTAGENT->Close();
		SOUNDSYS->Close();
	}

	if( mCurStage == eStage_Game || stage == eStage_Load )
	{
		WORLDMAN->ClearSkyDome();
		SCENEMAN->Clear();
		RESOURCEMAN->Close();
	}

	mpStage[stage]->InitStage();
	mpStage[stage]->Open();

	ManagedStageMap( mCurStage, stage );

	mCurStage = stage;

#ifdef _GMTOOL
  	if( mCurStage == eStage_Game )
  	{
 		GMTOOL->SetGMToolVisible( true );
  	}
  	else 
  		GMTOOL->SetGMToolVisible( false );
  
 	if( mCurStage == eStage_Select )
 	{
 		GMTOOL->Clear();
 	}
#endif

	CURSOR->SetCursor( eCURSOR_DEFAULT );
}

void cStageManager::SetTwitterUid(char* uid)
{
	strncpy( mTwitterUid, uid, MAX_TWITTER_UID );
}

void cStageManager::SetTwitterPwd(char* pwd)
{
	strncpy( mTwitterPwd, pwd, MAX_TWITTER_PWD );
}

void cStageManager::ManagedBGM( eSTAGE curStage, eSTAGE newStage )
{
	switch( newStage )
	{
	case eStage_Login:
		{
			if( curStage != eStage_Server && curStage != eStage_Select )
			{
				/// 배경 사운드 시작
				SOUNDSYS->PlayBGMSound( true );
			}
		}
		break;
	case eStage_Select:
		{
			if( curStage == eStage_Game )
			{
				/// 배경 사운드 시작
				SOUNDSYS->PlayBGMSound( true );
			}
		}
		break;
	case eStage_Load:
	case eStage_MapChange:
	case eStage_Game:
		{
			/// 모든 배경 사운드 종료
			SOUNDSYS->PlayBGMSound( false );
		}
		break;
	default:
		break;
	}
}

void cStageManager::ManagedStageMap( eSTAGE curStage, eSTAGE newStage )
{
	switch( newStage )
	{
	case eStage_Login:
		{
			CAMERAMAN->ChangeCamera( 1 );

			if( curStage != eStage_Select && curStage != eStage_Server )
			{
				SCENEMAN->Clear();
				cLoginStage* p = (cLoginStage*)mpStage[eStage_Login];
				if( p )
					p->CreateLoginMap();
			}
		}
		break;
	case eStage_Select:
		{
			CAMERAMAN->ChangeCamera( 1 );
			if( curStage == eStage_Game  )
			{
				SCENEMAN->Clear();
				cSelectStage* p = (cSelectStage*)mpStage[eStage_Select];
				if( p )
					p->CreateSelectMap( true );
			}
			else
			{
				cSelectStage* p = (cSelectStage*)mpStage[eStage_Select];
				if( p )
					p->CreateSelectMap( false );
			}
		}
		break;
	case eStage_Load:
		{
			CAMERAMAN->ChangeCamera( 0 );
			SCENEMAN->Clear();
			cSelectStage* p = (cSelectStage*)mpStage[eStage_Select];
			if( p )
				p->ClearSelectMap();
		}
		break;
	default:
		break;
	}
}

void cStageManager::AddScreenShotMsg( const charT* fileName )
{
	if( CHATMANAGER == 0 )
		return;

	if( mCurStage == eStage_Game && mpStage[eStage_Game] )
	{
		cStringT msg;
		msg.Format( GAMERESOURCEMAN->GetGameText( 10000 ), fileName );

		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
	}
}

void cStageManager::AddVideoCaptureMsg( const charT* fileName )
{
	if( CHATMANAGER == 0 )
		return;

	if( mCurStage == eStage_Game && mpStage[eStage_Game] )
	{
		cStringT msg;
		msg.Format( GAMERESOURCEMAN->GetGameText( 10001 ), fileName );

		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
	}
}


void cStageManager::SetChannelIdx( unsigned long idx )
{

#ifdef GMTOOL
	/// 채널 이동 체크
	if( mChannelIdx != idx )
	{
		GMTOOL->InitChannelChange();
	}
#endif

	mChannelIdx = idx;
}
