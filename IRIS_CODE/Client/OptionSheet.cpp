#include "stdafx.h"
#include "OptionSheet.h"

#include "OptionManager.h"
#include "CheckBox.h"
#include "Button.h"
#include "RadioButton.h"
#include "OptionGauge.h"
#include "ComboBox.h"
#include "RenderSystem.h"
#include "StageManager.h"
#include "ObjectManager.h"
#include "bandicap.h"
#include "GameUIManager.h"
#include "TwitterWindow.h"

cOptionSheet1::cOptionSheet1( eUINodeType type )
: cUIWindow( type )
,mpShowHeroNameCard(0)
,mpShowPlayerNameCard(0)
,mpShowNPCNameCard(0)
,mpShowMonsterNameCard(0)
,mpShowGauge(0)
,mpShowBubble(0)
,mpShowHelmet(0)
,mpShowTip(0)
,mpRejectionTrade(0)
,mpRejectionDuel(0)
,mpRejectionWhisper(0)
,mpRejectionParty(0)
,mpRejectionFriend(0)
,mpRejectionPvP(0)
,mOptionChange(false)
,mHelmetStateChange(false)
{
	for( unsigned int i=0; i<2; i++ )
	{
		mShowHeroTotal[i] = 0;
		mShowPlayerTotal[i] = 0;
	}

	
}

cOptionSheet1::~cOptionSheet1()
{
}

bool cOptionSheet1::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpShowHeroNameCard = (cCheckBox*)GetChild(eUIID_OPTION_NAMECHECK);
	mShowHeroTotal[0] = (cRadioButton*)GetChild(eUIID_OPTION_NAMEFULLCHECK);
	mShowHeroTotal[1] = (cRadioButton*)GetChild(eUIID_OPTION_NAMEONLYCHECK);

	mpShowPlayerNameCard = (cCheckBox*)GetChild(eUIID_OPTION_OTHERNAMECHECK);
	mShowPlayerTotal[0] = (cRadioButton*)GetChild(eUIID_OPTION_OTHERNAMEFULLCHECK);
	mShowPlayerTotal[1] = (cRadioButton*)GetChild(eUIID_OPTION_OTHERNAMEONLYCHECK);

	mpShowNPCNameCard = (cCheckBox*)GetChild(eUIID_OPTION_NPCNAMECHECK);
	mpShowMonsterNameCard = (cCheckBox*)GetChild(eUIID_OPTION_MONSTERNAMECHECK);
	mpShowGauge = (cCheckBox*)GetChild(eUIID_OPTION_GAUGECHECK);
	mpShowBubble = (cCheckBox*)GetChild(eUIID_OPTION_BALLOONCHECK);
	mpShowHelmet = (cCheckBox*)GetChild(eUIID_OPTION_HELMETCHECK);
	mpShowTip = (cCheckBox*)GetChild(eUIID_OPTION_TIPCHECK);

	mpRejectionTrade = (cCheckBox*)GetChild(eUIID_OPTION_TRADECHECK);
	mpRejectionDuel = (cCheckBox*)GetChild(eUIID_OPTION_DUALCHECK);
	mpRejectionWhisper = (cCheckBox*)GetChild(eUIID_OPTION_WHISPERCHECK);
	mpRejectionParty = (cCheckBox*)GetChild(eUIID_OPTION_PARTYCHECK);
	mpRejectionFriend = (cCheckBox*)GetChild(eUIID_OPTION_FRIENDCHECK);
	mpRejectionPvP = (cCheckBox*)GetChild(eUIID_OPTION_PVPCHECK);

	return true;
}

void cOptionSheet1::OnShow()
{
	cUIWindow::OnShow();

	UpdateOptionData();
}

void cOptionSheet1::UpdateOptionData()
{
	mOptionChange = false;
	mHelmetStateChange = false;

	/// game option setting
	sGameOption* gameOpt = OPTIONMAN->GetGameOption();

	///
	if( mpShowHeroNameCard )
	{
		mpShowHeroNameCard->SetPress( gameOpt->option1.showHeroNameCard );

		if( mShowHeroTotal[0] && mShowHeroTotal[1] )
		{
			mShowHeroTotal[0]->SetEnabled( gameOpt->option1.showHeroNameCard );
			mShowHeroTotal[1]->SetEnabled( gameOpt->option1.showHeroNameCard );

			if( gameOpt->option1.showHeroNameCard == true )
			{
				mShowHeroTotal[0]->SetPress( gameOpt->option1.showHeroTotal, false );
				mShowHeroTotal[1]->SetPress( !gameOpt->option1.showHeroTotal, false );
			}
		}
	}

	///
	if( mpShowPlayerNameCard )
	{
		mpShowPlayerNameCard->SetPress( gameOpt->option1.showPlayerNameCard );

		if( mShowPlayerTotal[0] && mShowPlayerTotal[1] )
		{
			mShowPlayerTotal[0]->SetEnabled( gameOpt->option1.showPlayerNameCard );
			mShowPlayerTotal[1]->SetEnabled( gameOpt->option1.showPlayerNameCard );

			if( gameOpt->option1.showPlayerNameCard == true )
			{
				mShowPlayerTotal[0]->SetPress( gameOpt->option1.showPlayerTotal, false );
				mShowPlayerTotal[1]->SetPress( !gameOpt->option1.showPlayerTotal, false );
			}
		}
	}

	///
	if( mpShowNPCNameCard )
		mpShowNPCNameCard->SetPress( gameOpt->option1.showNPCNameCard );

	///
	if( mpShowMonsterNameCard )
		mpShowMonsterNameCard->SetPress( gameOpt->option1.showMonsterNameCard );

	///
	if( mpShowGauge )
		mpShowGauge->SetPress( gameOpt->option1.showGauge );

	///
	if( mpShowBubble )
		mpShowBubble->SetPress( gameOpt->option1.showBubble );

	if( mpShowHelmet )
		mpShowHelmet->SetPress( gameOpt->option1.showHelmet );

	if( mpShowTip )
		mpShowTip->SetPress( gameOpt->option1.showTip );

	///
	if( mpRejectionTrade )
		mpRejectionTrade->SetPress( gameOpt->option1.rejectionTrade );

	///
	if( mpRejectionDuel )
		mpRejectionDuel->SetPress( gameOpt->option1.rejectionDuel );

	///
	if( mpRejectionWhisper )
		mpRejectionWhisper->SetPress( gameOpt->option1.rejectionWhisper );

	///
	if( mpRejectionParty )
		mpRejectionParty->SetPress( gameOpt->option1.rejectionParty );

	///
	if( mpRejectionFriend )
		mpRejectionFriend->SetPress( gameOpt->option1.rejectionFriend );

	///
	if( mpRejectionPvP )
		mpRejectionPvP->SetPress( gameOpt->option1.rejectionPvP );
}

void cOptionSheet1::InitOptionData()
{
	UpdateOptionData();
}

void cOptionSheet1::ApplyOptionData()
{
	if( mOptionChange == false )
		return;

	/// game option setting
	sGameOption* gameOpt = OPTIONMAN->GetGameOption();

	gameOpt->option1.showGauge = mpShowGauge->IsPress();
	gameOpt->option1.showBubble = mpShowBubble->IsPress();
	gameOpt->option1.showHeroNameCard = mpShowHeroNameCard->IsPress();
	gameOpt->option1.showHeroTotal = mShowHeroTotal[0]->IsPress();
	gameOpt->option1.showPlayerNameCard = mpShowPlayerNameCard->IsPress();
	gameOpt->option1.showPlayerTotal = mShowPlayerTotal[0]->IsPress();
	gameOpt->option1.showNPCNameCard = mpShowNPCNameCard->IsPress();
	gameOpt->option1.showMonsterNameCard = mpShowMonsterNameCard->IsPress();
	gameOpt->option1.showHelmet = mpShowHelmet->IsPress();
	gameOpt->option1.showTip = mpShowTip->IsPress();

	gameOpt->option1.rejectionTrade = mpRejectionTrade->IsPress();
	gameOpt->option1.rejectionDuel = mpRejectionDuel->IsPress();
	gameOpt->option1.rejectionWhisper = mpRejectionWhisper->IsPress();
	gameOpt->option1.rejectionParty = mpRejectionParty->IsPress();
	gameOpt->option1.rejectionFriend = mpRejectionFriend->IsPress();
	gameOpt->option1.rejectionPvP = mpRejectionPvP->IsPress();

	OPTIONMAN->ReqOptionValue();
	mOptionChange = false;

	if( mHelmetStateChange == true )
	{
		if( STAGEMAN->GetCurrentStage() == eStage_Game )
			OBJECTMAN->UpdateHelmetState();
	}
}

void cOptionSheet1::CancelOptionData()
{
	mOptionChange = false;
}


void cOptionSheet1::OnRadioPressed(cUINode* node, unsigned int id )
{
	switch( id )
	{
	case eUIID_OPTION_NAMEFULLCHECK:
	case eUIID_OPTION_NAMEONLYCHECK:
	case eUIID_OPTION_OTHERNAMEFULLCHECK:
	case eUIID_OPTION_OTHERNAMEONLYCHECK:
		mOptionChange = true;
		break;
	}
}

void cOptionSheet1::OnCheckBoxChanged(cUINode* node, unsigned int id)
{
	switch(id)
	{
	case eUIID_OPTION_NAMECHECK:
		{
			sGameOption* gameOpt = OPTIONMAN->GetGameOption();
			if( mShowHeroTotal[0] && mShowHeroTotal[1] )
			{
				if( node->IsPress() )
				{
					mShowHeroTotal[0]->SetPress( gameOpt->option1.showHeroTotal, false, true );
					mShowHeroTotal[1]->SetPress( !gameOpt->option1.showHeroTotal, false, true );
				}
				else
				{
					mShowHeroTotal[0]->SetPress( false, false, true );
					mShowHeroTotal[1]->SetPress( false, false, true );
				}

				mShowHeroTotal[0]->SetEnabled( node->IsPress() );
				mShowHeroTotal[1]->SetEnabled( node->IsPress() );
			}
			mOptionChange = true;
		}
		break;
	case eUIID_OPTION_OTHERNAMECHECK:
		{
			sGameOption* gameOpt = OPTIONMAN->GetGameOption();
			if( mShowPlayerTotal[0] && mShowPlayerTotal[1] )
			{
				if( node->IsPress() )
				{
					mShowPlayerTotal[0]->SetPress( gameOpt->option1.showPlayerTotal, false, true );
					mShowPlayerTotal[1]->SetPress( !gameOpt->option1.showPlayerTotal, false, true );
				}
				else
				{
					mShowPlayerTotal[0]->SetPress( false, false, true );
					mShowPlayerTotal[1]->SetPress( false, false, true );
				}

				mShowPlayerTotal[0]->SetEnabled( node->IsPress() );
				mShowPlayerTotal[1]->SetEnabled( node->IsPress() );
			}
			mOptionChange = true;
		}
		break;
	case eUIID_OPTION_HELMETCHECK:
		{
			mOptionChange = true;
			mHelmetStateChange = true;
		}
		break;
	case eUIID_OPTION_TIPCHECK:
	case eUIID_OPTION_NPCNAMECHECK:
	case eUIID_OPTION_MONSTERNAMECHECK:
	case eUIID_OPTION_GAUGECHECK:
	case eUIID_OPTION_BALLOONCHECK:
	case eUIID_OPTION_TRADECHECK:
	case eUIID_OPTION_DUALCHECK:
	case eUIID_OPTION_WHISPERCHECK:
	case eUIID_OPTION_PARTYCHECK:
	case eUIID_OPTION_FRIENDCHECK:
	case eUIID_OPTION_PVPCHECK:
		mOptionChange = true;
		break;
	}
}

//-----------------------------------------------------------------------
cOptionSheet2::cOptionSheet2( eUINodeType type )
: cUIWindow( type )
, mpWindowMode(0)
, mpWindowSize(0)
, mpLight(0)
, mpBackSound(0)
, mpEffSound(0)
, mGraphicChange(false)
, mSpinMoved(false)
, mVideoChanged(false)
{

	mDefaultRamp = 0.5f;

	mDefaultEnvVol = 0.5f;	///0.0f ~ 1.0f
	mDefaultEffVol = 0.5f;	///0.0f ~ 1.0f
}

cOptionSheet2::~cOptionSheet2()
{
}

bool cOptionSheet2::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpWindowMode = (cCheckBox*)GetChild(eUIID_OPTION_WINDOWCHECK);
	mpWindowSize = (cComboBox*)GetChild(eUIID_OPTION_WINDOWSIZE);
	mpLight = (cOptionGauge*)GetChild(eUIID_OPTION_LIGHT);

	mpBackSound = (cOptionGauge*)GetChild(eUIID_OPTION_BACKSOUND);
	mpEffSound = (cOptionGauge*)GetChild(eUIID_OPTION_EFFECTSOUND);

	mpVideoMode = (cComboBox*)GetChild(eUIID_OPTION_VIDEO);

	return true;
}

void cOptionSheet2::OnShow()
{
	cUIWindow::OnShow();
	UpdateOptionData();
}

///
void cOptionSheet2::UpdateOptionData()
{
	/// graphic & sound option setting
	mGraphicChange = false;
	mSpinMoved	   = false;
	mVideoChanged  = false;

	if( mpWindowMode )
		mpWindowMode->SetPress( OPTIONMAN->IsWindowed() );

	if( mpLight )
	{
		mDefaultRamp = OPTIONMAN->GetGammaPer();
		mpLight->SetPercent( OPTIONMAN->GetGammaPer() * 100.0f );
	}
	if( mpBackSound )
	{
		mDefaultEnvVol = OPTIONMAN->GetEnvVolume();
		mpBackSound->SetPercent( OPTIONMAN->GetEnvVolume() * 100.0f );
	}
	if( mpEffSound )
	{
		mDefaultEffVol = OPTIONMAN->GetEffVolume();
		mpEffSound->SetPercent( OPTIONMAN->GetEffVolume() *100.0f );
	}

	/// 콤보박스 적용
	mpWindowSize->Clear();

	unsigned int width = RENDERSYS->GetMonitorWidth();
	unsigned int height = RENDERSYS->GetMonitorHeight();

	tArray<void*>* arr = RENDERSYS->GetResolutionArr();
	NiDX9AdapterDesc::ModeDesc* curMode = RENDERSYS->GetCurrentMode();
	if( arr )
	{
		TCHAR buff[256] = {0,};
		int select = -1;

		for( unsigned int i = 0; i<arr->GetSize(); i++ )
		{
			NiDX9AdapterDesc::ModeDesc* p = (NiDX9AdapterDesc::ModeDesc*)((*arr)[i]);
			if( p == 0 )
				continue;

			if( mpWindowMode && mpWindowMode->IsPress() )
			{
				if( p->m_uiWidth > width || p->m_uiHeight > height )
					continue;
			}

			::_stprintf_s( buff, _T("%dx%dx%d"), p->m_uiWidth, p->m_uiHeight, p->m_uiBPP );
			mpWindowSize->AddItem( buff, p );

			if( curMode == (NiDX9AdapterDesc::ModeDesc*)((*arr)[i]) )
				select = (int)i;
		}

		if( select != -1 )
			mpWindowSize->SetSelect( select );
	}

	if( mpVideoMode )
		mpVideoMode->Clear();
	else
	{
		assert(0);
		return;
	}

	/// 현재 비디오 모드 얻어오기
	int currentVideoMode = OPTIONMAN->GetVideoMode();
	int	selectIndex = -1;
	/// 비디오 모드 넣기.
	for( int idx = 0; idx < eVIDEO_MODE_MAX; ++idx )
	{
		unsigned int stringIndex = 26605;
		int			 presetIndex = BCAP_PRESET_GAME;
		
		switch( idx )
		{
		case eVIDEO_MODE_DEFAULT:
			stringIndex = 26600;
			presetIndex = BCAP_PRESET_320x240;
			break;
		case eVIDEO_MODE_640X480:
			stringIndex = 26601;
			presetIndex = BCAP_PRESET_640x480;
			break;
		case eVIDEO_MODE_800X600:
			stringIndex = 26602;
			presetIndex = BCAP_PRESET_800x600;
			break;
		case eVIDEO_MODE_1024X768:
			stringIndex = 26603;
			presetIndex = BCAP_PRESET_1024x768;
			break;
		case eVIDEO_MODE_1280X720:
			stringIndex = 26604;
			presetIndex = BCAP_PRESET_1280x720;
			break;
		case eVIDEO_MODE_GAME:
			stringIndex = 26605;
			presetIndex = BCAP_PRESET_GAME;
			break;
		default:
			continue;
		}

		mpVideoMode->AddItem( UIMAN->GetUIText( stringIndex ), (void*)presetIndex );

		///현재 설정되어있는 모드 얻어와서 선택 하기.
		if( presetIndex == currentVideoMode )
			selectIndex = idx;
	}

	if( selectIndex != -1 )
		mpVideoMode->SetSelect( selectIndex );

}

void cOptionSheet2::InitOptionData()
{
	if( mSpinMoved )
	{
		OPTIONMAN->SetGammaPer( mDefaultRamp );
		OPTIONMAN->SetVolume( mDefaultEnvVol, mDefaultEffVol );
	}

	UpdateOptionData();
}

void cOptionSheet2::ApplyOptionData()
{
	if( mSpinMoved == true )
	{
		OPTIONMAN->SetGammaPer( (float)mpLight->GetPercent()*0.01f );
		OPTIONMAN->SetVolume( (float)mpBackSound->GetPercent()*0.01f, (float)mpEffSound->GetPercent()*0.01f );

		mSpinMoved = false;
	}

	if( mpWindowSize && mpWindowSize->IsEmpty() == true )
		return;

	if( mGraphicChange == true )
	{
		NiDX9AdapterDesc::ModeDesc* curMode = (NiDX9AdapterDesc::ModeDesc*)mpWindowSize->GetSelectData();
		if( curMode == 0 )
		{
			assert(0);
			return;
		}

		OPTIONMAN->SetWindowedOption( mpWindowMode->IsPress(), curMode->m_uiWidth, curMode->m_uiHeight, (unsigned char)curMode->m_uiBPP );

		mGraphicChange = false;
	}

	/// 값이 제대로 체크 되어 있는지 확인
	if( mVideoChanged )
	{
		int presetIndex = -1;
		if( mpVideoMode )

		{
			if( mpVideoMode->IsEmpty() == true )
				return;
			presetIndex = (int)mpVideoMode->GetSelectData();
			switch( presetIndex )
			{
			case BCAP_PRESET_320x240:
			case BCAP_PRESET_640x480:
			case BCAP_PRESET_800x600:
			case BCAP_PRESET_1024x768:
			case BCAP_PRESET_1280x720:
			case BCAP_PRESET_GAME:
				break;
			default:
				assert(0);
				return;
			}
		}
		else
		{
			assert(0);
			return;
		}
		OPTIONMAN->SetVideoOption( presetIndex );

		mVideoChanged = false;
	}
}

void cOptionSheet2::CancelOptionData()
{
	if( mSpinMoved )
	{
		OPTIONMAN->SetGammaPer( mDefaultRamp );
		OPTIONMAN->SetVolume( mDefaultEnvVol, mDefaultEffVol );
	}

	mGraphicChange = false;
	mSpinMoved     = false;
	mVideoChanged  = false;
}


void cOptionSheet2::OnComboSelected( cUINode* caller, unsigned int id )
{
	if( caller == mpWindowSize )
	{
		if( mpWindowSize && mpWindowSize->IsEmpty() == true )
			return;

		mGraphicChange = true;
	}
	else if( caller == mpVideoMode )
	{
		if( mpVideoMode && mpVideoMode->IsEmpty() == true )
			return;
		mVideoChanged = true;
	}
}

void cOptionSheet2::OnCheckBoxChanged(cUINode*, unsigned int id)
{
	switch( id )
	{
	case eUIID_OPTION_WINDOWCHECK:
		{
			UpdateResolutionComboData();
			if( mpWindowSize && mpWindowSize->IsEmpty() == true )
			{
				/// message
				return;
			}
			mGraphicChange = true;
		}
		break;
	}
}

void cOptionSheet2::OnOptiongaugeUpdate( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_OPTION_LIGHT:
		{
			/// 밝기 조절
			OPTIONMAN->SetGammaPer( (float)mpLight->GetPercent()*0.01f );
		}
		break;
	case eUIID_OPTION_BACKSOUND:
		{
			/// 배경음 조절
			OPTIONMAN->SetVolume( (float)mpBackSound->GetPercent()*0.01f, (float)mpEffSound->GetPercent()*0.01f );
		}
		break;
	case eUIID_OPTION_EFFECTSOUND:
		{
			/// 효과음 조절
			OPTIONMAN->SetVolume( (float)mpBackSound->GetPercent()*0.01f, (float)mpEffSound->GetPercent()*0.01f );
		}
		break;
	}

	mSpinMoved = true;
}

void cOptionSheet2::UpdateResolutionComboData()
{
	NiDX9AdapterDesc* adapter = RENDERSYS->GetCurrentAdapter();
	if( adapter == 0 )
		return;

	NiDX9AdapterDesc::ModeDesc* curMode = (NiDX9AdapterDesc::ModeDesc*)mpWindowSize->GetSelectData();

	mpWindowSize->Clear();

	tArray<void*> arr;
	arr.Clear();

	unsigned int modeCount = adapter->GetModeCount();

	for( unsigned int i=0; i<modeCount; i++ )
	{
		const NiDX9AdapterDesc::ModeDesc* pMode = adapter->GetMode( i );

		if( pMode == 0 )
			continue;

		/// 추가로 불필요한 모드는 제외시킨다.
		if( pMode->m_uiWidth <= 800 )
			continue;
		if( pMode->m_uiHeight <= 600 )
			continue;

		/// 배열에 담아 두자
		arr.PushBack( (void*)pMode );
	}

	if( arr.GetSize() > 1 )
		::Sort( arr.Begin(), arr.End(), cResolutionCompare() );


	/// 콤보 채우기
	TCHAR buff[256] = {0,};
	int select = -1;

	unsigned int width = RENDERSYS->GetMonitorWidth();
	unsigned int height = RENDERSYS->GetMonitorHeight();

	for( unsigned int i = 0; i<arr.GetSize(); i++ )
	{
		NiDX9AdapterDesc::ModeDesc* p = (NiDX9AdapterDesc::ModeDesc*)arr[i];
		if( p == 0 )
			continue;

		if( mpWindowMode && mpWindowMode->IsPress() )
		{
			if( p->m_uiWidth > width || p->m_uiHeight > height )
				continue;
		}

		::_stprintf_s( buff, _T("%dx%dx%d"), p->m_uiWidth, p->m_uiHeight, p->m_uiBPP );
		mpWindowSize->AddItem( (LPCTSTR)buff, p );

		if( curMode == (NiDX9AdapterDesc::ModeDesc*)arr[i] )
			select = (int)i;
	}

	if( select != -1 )
		mpWindowSize->SetSelect( select );
	else
		mpWindowSize->SetSelect( 0 );
}



//-----------------------------------------------------------------------
cOptionSheet3::cOptionSheet3( eUINodeType type )
: cUIWindow( type )
, mpAutoSendMsg(0)
, mpAccountRegist(0)
, mpAccountRefresh(0)
, mCheckStateChange(false)
{
}

cOptionSheet3::~cOptionSheet3()
{
}

bool cOptionSheet3::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpAutoSendMsg		= (cCheckBox*)GetChild(eUIID_OPTION_MESSAGE_AUTOSEND_CHECK);
	mpAccountRegist		= (cButton*)GetChild(eUIID_OPTION_ACCOUNT_INSERT);
	mpAccountRefresh	= (cButton*)GetChild(eUIID_OPTION_ACCOUNT_REFRESH);

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

void cOptionSheet3::OnCommand( cUINode* caller, unsigned int id )
{
	switch( id )
	{
	case eUIID_OPTION_ACCOUNT_INSERT:
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

	case eUIID_OPTION_ACCOUNT_REFRESH:
		{
			/// 계정이 등록되어 있을때 없으면 비활성화 되어서 여기 안옴
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
		}
		break;
	}
}

void cOptionSheet3::OnCheckBoxChanged(cUINode* caller, unsigned int id)
{
	switch( id )
	{
	case eUIID_OPTION_MESSAGE_AUTOSEND_CHECK:
		{
			mCheckStateChange = true;
		}
		break;
	}	
}

void cOptionSheet3::OnShow()
{
	cUIWindow::OnShow();
	UpdateOptionData();
}


bool cOptionSheet3::IsPressAutoSendCheck()
{
	if( mpAutoSendMsg == NULL )
	{
		assert(0);
		return false;
	}

	return mpAutoSendMsg->IsPress();

}

void cOptionSheet3::SetResetBtnEnable( bool bEnable )
{
	if( mpAccountRefresh )
	{
		mpAccountRefresh->SetEnabled( bEnable );
	}
	else
	{
		assert(0);
	}
}

void cOptionSheet3::UpdateOptionData()
{
	mCheckStateChange = false;

	/// game option setting
	sGameOption* gameOpt = OPTIONMAN->GetGameOption();

	if( mpAutoSendMsg )
	{
		mpAutoSendMsg->SetPress( gameOpt->option1.autoSendTwitterMsg );
	}
}

void cOptionSheet3::InitOptionData()
{
	UpdateOptionData();
}

void cOptionSheet3::ApplyOptionData()
{
	if( mCheckStateChange == false )
		return;

	/// game option setting
	sGameOption* gameOpt = OPTIONMAN->GetGameOption();

	gameOpt->option1.autoSendTwitterMsg = mpAutoSendMsg->IsPress();

	OPTIONMAN->ReqOptionValue();

	mCheckStateChange = false;

}

void cOptionSheet3::CancelOptionData()
{
	 mCheckStateChange = false;
}
