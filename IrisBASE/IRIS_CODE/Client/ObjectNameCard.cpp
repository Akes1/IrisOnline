#include "StdAfx.h"
#include "objectnamecard.h"

#include "FontAgent.h"
#include "NameCardGauge.h"

#include "ResourceManager.h"

#include "UIManager.h"
#include "PVPManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Monster.h"
#include "Npc.h"
#include "TitleManager.h"
#include "PartyManager.h"
#include "PartyUnionManager.h"
#include "Application.h"
#include "OptionManager.h"
#include "GameResourceManager.h"

#include "StageManager.h"
#include "PlaneObject.h"
#include "GuildManager.h"

cNameCard::cNameCard()
{
	mHeadScreenX = 0;
	mHeadScreenY = 0;
	mFootScreenX = 0;
	mFootScreenY = 0;
	
	mBackColor = NiColorA::WHITE;
}

cNameCard::~cNameCard()
{
}

void cNameCard::Set2DPos( int hscreenX, int hscreenY, int fscreenX, int fscreenY )
{
	mHeadScreenX = hscreenX;
	mHeadScreenY = hscreenY;
	mFootScreenX = fscreenX;
	mFootScreenY = fscreenY;
}

void cNameCard::Set2DFootPos( int fscreenX, int fscreenY )
{
	mFootScreenX = fscreenX;
	mFootScreenY = fscreenY;
}

///
void cNameCard::SetName( LPTSTR name )
{
	::_stprintf_s( mName.mString, _T("%s"), name );
	mName.mStrLen = ::_tcslen(name);
	mName.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
						mName.mString, mName.mStrLen );

	mName.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
}

///
void cNameCard::SetName( LPTSTR name, unsigned long color )
{
	SetName( name );
	SetNameColor( color );
}

void cNameCard::SetNameColor( unsigned long color )
{
	mName.mColor = color;

	float red	= (float)((color & 0xFF0000) >> 16) / 255.0f;
	float green	= (float)((color & 0xFF00) >> 8) / 255.0f;
	float blue	= (float)((color & 0xFF)) / 255.0f;

	mBackColor = NiColorA(red, green, blue, 0.0f);
}

void cNameCard::Draw()
{
	POINT pos;

	pos.x = (LONG)mHeadScreenX - (mName.mPixelWidth/2);
	pos.y = (LONG)mHeadScreenY - FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );
}


//////////////////////////////////////////////////////////////////////////
// Object NameCard
cObjectNameCard::cObjectNameCard( cBaseObject* pObj )
: mpOriginObject( pObj ), mpHPGauge(0), mpMPGauge(0)
{
	mUsedGauge = true;
	SetName( mpOriginObject->GetName() );
}

cObjectNameCard::~cObjectNameCard()
{
	mpOriginObject = 0;

	SAFE_DELETE(mpMPGauge);
	SAFE_DELETE(mpHPGauge);
}

void cObjectNameCard::PreRender()
{
	if( mpHPGauge )
		mpHPGauge->PreDraw();
	if( mpMPGauge )
		mpMPGauge->PreDraw();
}

//////////////////////////////////////////////////////////////////////////
// NpcNameCard
cNpcNameCard::cNpcNameCard( cBaseObject* pObj )
: cObjectNameCard( pObj )
{
}

cNpcNameCard::~cNpcNameCard()
{
}

///
void cNpcNameCard::SetJobName( LPTSTR name )
{
	int len = _tcslen( name );
	if( len > 0 )
	{
		::_stprintf_s( mJobName.mString, _T("<%s>"), name );
		mJobName.mStrLen = ::_tcslen(mJobName.mString);
		mJobName.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
			mJobName.mString, mJobName.mStrLen );
		mJobName.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	}
}

///
//void cNpcNameCard::SetJobName( LPTSTR name, unsigned long color )
//{
//	SetJobName( name );
//	SetJobNameColor( color );
//}

void cNpcNameCard::ChangeTextValue()
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	cNpc* p = (cNpc*)mpOriginObject;
	SetName( p->GetName() );

	/// hero정보에 따른 변형
	SetJobName( p->GetJobName() );
}


void cNpcNameCard::Draw()
{
	POINT pos;

	pos.x = (LONG)mHeadScreenX - (mName.mPixelWidth/2);
	pos.y = (LONG)mHeadScreenY - FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );

	pos.x = (LONG)mHeadScreenX - (mJobName.mPixelWidth/2);
	pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mJobName.mString, mJobName.mStrLen, &pos, mJobName.mColor, true );
}


//////////////////////////////////////////////////////////////////////////
// MonsterNameCard
cMonsterNameCard::cMonsterNameCard( cBaseObject* pObj )
: cNpcNameCard( pObj )
{
	cString pathName;
	pathName.Format( "./language/%s/U_Basic_01.tga", cApplication::mLangaugeFolder.Cstr() );
	NiTexture* pTex = RESOURCEMAN->LoadTexture( pathName.Cstr(), false );
	if( pTex )
	{
		RECT rcFrame;
		rcFrame.left = 351;
		rcFrame.top = 377;
		rcFrame.right = GAUGE_WIDTH+2;
		rcFrame.bottom = GAUGE_HEIGHT+2;

		RECT rcFrameBack;
		rcFrameBack.left = 351;
		rcFrameBack.top = 367;
		rcFrameBack.right = GAUGE_WIDTH+2;
		rcFrameBack.bottom = GAUGE_HEIGHT+2;

		RECT rc;
		rc.left = 355;
		rc.top = 387;
		rc.right = GAUGE_WIDTH;
		rc.bottom = GAUGE_HEIGHT;

		mpHPGauge = new cNameCardGauge;
		if( mpHPGauge->Init( pTex,&rc, GAUGE_TEXWIDTH, GAUGE_TEXHEIGHT, pTex, &rcFrame, &rcFrameBack ) == false )
			SAFE_DELETE(mpHPGauge);
	}
}

cMonsterNameCard::~cMonsterNameCard()
{
}

void cMonsterNameCard::ChangeTextValue()
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	cMonster* p = (cMonster*)mpOriginObject;
	if( p )
	{
		SetJobName( p->GetJobName() );
		SetNameColor(0xFFFFFF00);
		SetJobNameColor(0xFFFFFF00);

		if( p->IsAutoDummy() == false )
		{
			SetName( p->GetName() );

			/// 변신상태이거나 몬스터 선공 인식 회피 효과가 있는지 체크
			if( HERO->GetTransMonsterClassIdx() > 0 || HERO->IsAvoidMonLook() == true )
			{
				// 변신중 or 선공인식 회피 효과가 있음
				if( p->IsPrecedeAttack() == true && p->IsChangeMonCheck() == true )
				{
					SetNameColor(0xFFDC00FF);
					SetJobNameColor(0xFFDC00FF);
				}
			}
			else
			{
				/// 변신 아님
				if( p->IsPrecedeAttack() == true )
				{
					SetNameColor(0xFFDC00FF);
					SetJobNameColor(0xFFDC00FF);
				}
			}
		}
		else
		{
			SetName( (LPTSTR)GAMERESOURCEMAN->GetGameText(14000) );
		}
	}
}

void cMonsterNameCard::ChangeGaugeValue()
{
	unsigned short width = 0;
	if( mpOriginObject->GetMaxHP() != 0 )
	{
		 width = (unsigned short)(GAUGE_WIDTH * mpOriginObject->GetHP() / mpOriginObject->GetMaxHP());
		 if( mpOriginObject->GetHP() > mpOriginObject->GetMaxHP() )
		 {
			 assert(0);
			 width = GAUGE_WIDTH;
		 }
	}

	if( mpHPGauge )
		mpHPGauge->SetGaugeWidth( width );
}

void cMonsterNameCard::Draw()
{
	POINT pos;
	pos.y = (LONG)mHeadScreenY;
	if( mUsedGauge && OPTIONMAN->IsShowGauge() )
	{
		pos.x = (LONG)mHeadScreenX - (GAUGE_WIDTH/2);
		pos.y -= GAUGE_HEIGHT;

		mpHPGauge->SetGaugePos( (short)pos.x, (short)pos.y );
		mpHPGauge->Draw();
	}

	if( OPTIONMAN->IsShowMonsterNameCard() )
	{
		pos.x = (LONG)mHeadScreenX - (mName.mPixelWidth/2);
		pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
		FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );

		pos.x = (LONG)mHeadScreenX - (mJobName.mPixelWidth/2);
		pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
		FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mJobName.mString, mJobName.mStrLen, &pos, mJobName.mColor, true );
	}
}

//////////////////////////////////////////////////////////////////////////
// PlayerNameCard

cPlayerNameCard::cPlayerNameCard( cBaseObject* pObj )
: cObjectNameCard(pObj)
, mpGuildMark(0)
, mpPVPMark(0)
, mEnableGuildMark( true )
, mEnablePVPMark( false )
{
	mIsHeroNameCard = pObj->GetObjectType() == eOBJECTTYPE_HERO;

	cString pathName;
	pathName.Format( "./language/%s/U_Basic_01.tga", cApplication::mLangaugeFolder.Cstr() );
	NiTexture* pTex = RESOURCEMAN->LoadTexture( pathName.Cstr(), false );
	if( pTex )
	{
		RECT rcFrame;
		rcFrame.left = 351;
		rcFrame.top = 377;
		rcFrame.right = GAUGE_WIDTH+2;
		rcFrame.bottom = GAUGE_HEIGHT+2;

		RECT rcFrameBack;
		rcFrameBack.left = 351;
		rcFrameBack.top = 367;
		rcFrameBack.right = GAUGE_WIDTH+2;
		rcFrameBack.bottom = GAUGE_HEIGHT+2;

		RECT rc;
		rc.left = 355;
		rc.top = 396;
		rc.right = GAUGE_WIDTH;
		rc.bottom = GAUGE_HEIGHT;

		mpHPGauge = new cNameCardGauge;
		if( mpHPGauge->Init( pTex,&rc, GAUGE_TEXWIDTH, GAUGE_TEXHEIGHT, pTex, &rcFrame, &rcFrameBack ) == false )
			SAFE_DELETE(mpHPGauge);
	}

	if( pTex )
	{
		RECT rcFrame;
		rcFrame.left = 351;
		rcFrame.top = 377;
		rcFrame.right = GAUGE_WIDTH+2;
		rcFrame.bottom = GAUGE_HEIGHT+2;

		RECT rcFrameBack;
		rcFrameBack.left = 351;
		rcFrameBack.top = 367;
		rcFrameBack.right = GAUGE_WIDTH+2;
		rcFrameBack.bottom = GAUGE_HEIGHT+2;

		RECT rc;
		rc.left = 355;
		rc.top = 405;
		rc.right = GAUGE_WIDTH;
		rc.bottom = GAUGE_HEIGHT;

		mpMPGauge = new cNameCardGauge;
		if( mpMPGauge->Init( pTex,&rc, GAUGE_TEXWIDTH, GAUGE_TEXHEIGHT, pTex, &rcFrame, &rcFrameBack ) == false )
			SAFE_DELETE(mpMPGauge);
	}

	//
	cIconParam* param = UIMAN->GetIconParam( PVP_MARK_A );
	if( param )
	{
		unsigned int tx = param->mTexPos.mX;
		unsigned int ty = param->mTexPos.mY;

		mpPVPMark = new cPlaneObject;
		if( mpPVPMark->Create( param->mpTexture, 0, 0, ICON_SIZE, ICON_SIZE, tx, ty, tx + ICON_SIZE, ty + ICON_SIZE ) == false )
		{
			SAFE_DELETE( mpPVPMark );
			assert(0);
		}
	}
}

cPlayerNameCard::~cPlayerNameCard()
{
	SAFE_DELETE( mpPVPMark );
	SAFE_DELETE( mpGuildMark );
}

void cPlayerNameCard::SetDeleteWaitWord( LPTSTR word, unsigned long color )
{
	::_stprintf_s( mDeleteWaitWord.mString, _T("%s"), word );
	mDeleteWaitWord.mStrLen = ::_tcslen(mDeleteWaitWord.mString);
	mDeleteWaitWord.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
								mDeleteWaitWord.mString, mDeleteWaitWord.mStrLen );
	mDeleteWaitWord.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	mDeleteWaitWord.mColor = color;
}

void cPlayerNameCard::SetTitleName( LPTSTR titleName, unsigned long color )
{
	::_stprintf_s( mTitleName.mString, _T("%s"), titleName );
	mTitleName.mStrLen = ::_tcslen(mTitleName.mString);
	mTitleName.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
							mTitleName.mString, mTitleName.mStrLen );
	mTitleName.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	mTitleName.mColor = color;
}

void cPlayerNameCard::SetTodayWord( LPTSTR word, unsigned long color )
{
	::_stprintf_s( mTodayWord.mString, _T("%s"), word );
	mTodayWord.mStrLen = ::_tcslen( mTodayWord.mString );
	mTodayWord.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
							mTodayWord.mString, mTodayWord.mStrLen );
	mTodayWord.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
	mTodayWord.mColor = color;
}

void cPlayerNameCard::SetGuildName( LPTSTR guildName )
{
	if( ::_tcslen( guildName ) == 0 )
		::_stprintf_s( mGuildName.mString, _T("%s"), guildName );
	else
	{
		if( STAGEMAN->GetCurrentStage() == eStage_Game )
			::_stprintf_s( mGuildName.mString, _T("<%s>"), guildName );
		else
			::_stprintf_s( mGuildName.mString, _T("%s"), guildName );
	}

	mGuildName.mStrLen = ::_tcslen(mGuildName.mString);
	mGuildName.mPixelWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_DepthFont, 
							mGuildName.mString, mGuildName.mStrLen );
	mGuildName.mPixelHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
}

void cPlayerNameCard::ChangeTextValue()
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	cPlayer* p = (cPlayer*)mpOriginObject;
	if( p )
	{
		if( PARTYMAN->IsHeroParty( p->GetObjectID() ) || PARTYUNIONMAN->IsHeroParty( p->GetObjectID() ) )
			SetName( p->GetName(), 0xFF85D100 ); 
		else if( p->GetGuildIndex() > 0 && HERO->GetGuildIndex() == p->GetGuildIndex() && p->GetObjectID() != HERO->GetObjectID() )
			SetName( p->GetName(), 0xFFF2A500 ); 
		else
			SetName( p->GetName(), 0xFFFFFFFF ); //eCOLOR_WHITE


		sPlayerExrInfo* exrInfo = p->GetPlayerExrInfo();
		if( exrInfo )
		{
			/// 타이틀 세팅
			if( exrInfo->mTitleIndex > 0 )
			{
				cTitleDefine* define = TITLEMAN->GetTitleDefine( exrInfo->mTitleIndex );
				if( define )
					SetTitleName( (LPTSTR)define->mName, define->mColor );
			}
			else
			{
				SetTitleName( _T("") );
			}

			/// 길드명 세팅
			if( exrInfo->mGuildIndex > 0 )
				SetGuildName( exrInfo->mGuildName );
			else	
				SetGuildName( _T("") );

			/// 오늘의 한마디 세팅
			if( ::_tcslen( exrInfo->mTodayWord ) > 0 )
				SetTodayWord( exrInfo->mTodayWord, exrInfo->mTodayColor );
			else
				SetTodayWord( _T("") );

			/// 길드마크 검사후 세팅
			ChangeGuildMark();
		}

		mEnablePVPMark = false;
		if( PVPMAN->IsEnableBattle() )
		{
			mEnablePVPMark = true;

			// 세력 마크 표시
			if( exrInfo )
			{
				unsigned int id = PVP_MARK_A;
				switch( exrInfo->mPvPDMTeamType )
				{
				case ePVPDM_TEAMTYPE_A: id = PVP_MARK_A;	break;
				case ePVPDM_TEAMTYPE_B:	id = PVP_MARK_B;	break;
				default:				assert(0);			break;
				}
				
				cIconParam* param = UIMAN->GetIconParam( id );
				if( param )
				{
					unsigned int tx = param->mTexPos.mX;
					unsigned int ty = param->mTexPos.mY;

					if( mpPVPMark )
					{
						mpPVPMark->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
					}
					else
						assert(0);
				}
			}

			//
			if( PVPMAN->IsMyTeam( p ) == false )
			{
				int textIdx = 0;
				switch( p->GetForceType() )
				{
				case eFORCETYPE_FIRE:	textIdx = 120;	break;
				case eFORCETYPE_WATER:  textIdx = 121;	break;
				case eFORCETYPE_WIND:	textIdx = 122;	break;
				case eFORCETYPE_EARTH:	textIdx = 123;	break;
				case eFORCETYPE_NONE:	
					{
						/// 세력이 없는 GM 이 GM_Tool을 이용해 들어오는 경우만 들어옴.
						SetName( p->GetName(), 0xFFFFFFFF ); 
					}
					break;

				default: 
					{
						assert(0); 
						SetName( _T("Enemy"), 0xFFFF0000); //eCOLOR_RED
					}
					break;
				}

				if( textIdx != 0 )
					SetName( (LPTSTR)UIMAN->GetUIText( textIdx ), 0xFFFF0000 ); //eCOLOR_RED

				SetTitleName( _T("") );
				SetGuildName( _T("") );
				SetTodayWord( _T("") );
				
				mEnableGuildMark = false;
			}
		}
	}
}

void cPlayerNameCard::ChangeGaugeValue()
{
//	if( mpOriginObject->GetHP() > mpOriginObject->GetMaxHP() )
//		assert(0);
//	if( mpOriginObject->GetMP() > mpOriginObject->GetMaxMP() )
//		assert(0);

	/// hp 게이지 표시
	unsigned short width = 0;
	if( mpOriginObject->GetMaxHP() != 0 )
	{
		width = (unsigned short)(GAUGE_WIDTH * mpOriginObject->GetHP()/mpOriginObject->GetMaxHP());
		if( mpOriginObject->GetHP() > mpOriginObject->GetMaxHP() )
		{
			//unsigned long check1 = mpOriginObject->GetHP();
			//unsigned long check2 = mpOriginObject->GetMaxHP();

			assert(0);
			width = GAUGE_WIDTH;
		}
	}
	else
	{
		assert(0);
	}

	if( mpHPGauge )
		mpHPGauge->SetGaugeWidth( width );

	/// mp 게이지 표시
	width = 0;
	if( mpOriginObject->GetMaxMP() != 0 )
	{
		width = (unsigned short)(GAUGE_WIDTH * mpOriginObject->GetMP()/mpOriginObject->GetMaxMP());
		if( mpOriginObject->GetMP() > mpOriginObject->GetMaxMP() )
		{
			assert(0);
			width = GAUGE_WIDTH;
		}
	}
	else
	{
		assert(0);
	}

	if( mpMPGauge )
		mpMPGauge->SetGaugeWidth( width );
}

/// 길드명 변경
void cPlayerNameCard::ChangeGuildName()
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	cPlayer* p = (cPlayer*)mpOriginObject;
	if( p )
	{
		sPlayerExrInfo* exrInfo = p->GetPlayerExrInfo();
		if( exrInfo )
		{
			if( exrInfo->mGuildIndex > 0 )
				SetGuildName( exrInfo->mGuildName );
			else	
				SetGuildName( _T("") );
		}
	}
}

void cPlayerNameCard::ChangeGuildMark()
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	cPlayer* p = (cPlayer*)mpOriginObject;
	if( p )
	{
		/// pvp 상태에서 상대편인경우는 적용시키지 않는다
		if( PVPMAN->IsEnableBattle() )
		{
			if( PVPMAN->IsMyTeam( p ) == false )
				return;
		}

		sPlayerExrInfo* exrInfo = p->GetPlayerExrInfo();
		if( exrInfo )
		{
			SAFE_DELETE( mpGuildMark );

			mEnableGuildMark = false;

			if( exrInfo->mGuildIndex > 0 && exrInfo->mUseGuildMark == true )
			{
				NiTexture* tex = GUILDMAN->CheckMarkVersion( exrInfo->mGuildIndex, exrInfo->mGuildMarkDate );
				if( tex )
				{
					mpGuildMark = new cPlaneObject;
					if( mpGuildMark->Create( tex, 0, 0, MARK_WIDTH, MARK_HEIGHT, 0, 0, MARK_WIDTH, MARK_HEIGHT ) == false )
					{
						assert(0);
						return;
					}

					mEnableGuildMark = true;
				}
				else
				{
					GUILDMAN->GuildMarkRequest( exrInfo->mGuildIndex );
				}
			}
		}
	}
}

/// 길드마크 미리보기
void cPlayerNameCard::PreviewGuildMark( NiTexture* tex )
{
	if( mpOriginObject == 0 )
	{
		assert(0);
		return;
	}

	if( tex == 0 )
	{
		assert(0);
		return;
	}

	sPlayerExrInfo* exrInfo = ((cPlayer*)mpOriginObject)->GetPlayerExrInfo();
	if( exrInfo )
	{
		SAFE_DELETE( mpGuildMark );

		mEnableGuildMark = false;

		if( exrInfo->mGuildIndex > 0 )
		{
			mpGuildMark = new cPlaneObject;
			if( mpGuildMark->Create( tex, 0, 0, MARK_WIDTH, MARK_HEIGHT, 0, 0, MARK_WIDTH, MARK_HEIGHT ) == false )
			{
				assert(0);
				return;
			}

			mEnableGuildMark = true;
		}
	}
}

void cPlayerNameCard::Draw()
{
	POINT pos;
	pos.y = (LONG)mFootScreenY;
	if( mpHPGauge && mpMPGauge && mUsedGauge && OPTIONMAN->IsShowGauge() )
	{
		pos.x = (LONG)mFootScreenX - (GAUGE_WIDTH/2);
		pos.y -= GAUGE_HEIGHT;

		mpMPGauge->SetGaugePos( (short)pos.x, (short)pos.y );
		mpMPGauge->Draw();

		pos.x = (LONG)mFootScreenX - (GAUGE_WIDTH/2);
		pos.y -= GAUGE_HEIGHT+3+1;

		mpHPGauge->SetGaugePos( (short)pos.x, (short)pos.y );
		mpHPGauge->Draw();
	}

	bool showTotal = false;

	if( STAGEMAN->GetCurrentStage() == eStage_Select )
	{
		showTotal = true;
	}
	else
	{
		if( mIsHeroNameCard )
		{
			if( OPTIONMAN->IsShowHeroNameCard() == false )
				return;

			if( OPTIONMAN->IsShowHeroTotal() )
				showTotal = true;
		}
		else
		{

			if( OPTIONMAN->IsShowPlayerNameCard() == false )
				return;

			if( OPTIONMAN->IsShowPlayerTotal() )
				showTotal = true;
		}
	}

	pos.y = (LONG)mHeadScreenY;

	if( showTotal )
	{
		/// 호칭 & 이름
		pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);

		if( mTitleName.mStrLen > 0 )
		{
			// 호칭 & 이름
			pos.x = (LONG)mHeadScreenX - ((mName.mPixelWidth + mTitleName.mPixelWidth + NAME_GAP)/2);
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mTitleName.mString, mTitleName.mStrLen, &pos, mTitleName.mColor, true );

			pos.x += mTitleName.mPixelWidth + NAME_GAP;
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );
		}
		else
		{
			// 이름
			pos.x = (LONG)mHeadScreenX - (mName.mPixelWidth/2);
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );
		}

		/// 길드명
		if( mGuildName.mStrLen > 0 )
		{
			pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
			pos.x = (LONG)mHeadScreenX - ((mGuildName.mPixelWidth)/2);
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mGuildName.mString, mGuildName.mStrLen, &pos, mGuildName.mColor, true );
		}

		if( mEnableGuildMark && mpGuildMark )
		{
			long width = (mName.mPixelWidth + mTitleName.mPixelWidth > mGuildName.mPixelWidth) ? mName.mPixelWidth + mTitleName.mPixelWidth : mGuildName.mPixelWidth;
			pos.x = (LONG)mHeadScreenX - ((width + NAME_GAP)/2) - MARK_WIDTH;

			mpGuildMark->SetTranslate( (short)pos.x, (short)pos.y );
			mpGuildMark->Draw();
		}

		/// 오늘의 한마디
		if( mTodayWord.mStrLen > 0 )
		{
			pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
			pos.x = (LONG)mHeadScreenX - ((mTodayWord.mPixelWidth)/2);
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mTodayWord.mString, mTodayWord.mStrLen, &pos, mTodayWord.mColor, true );
		}

		if( mDeleteWaitWord.mStrLen > 0 )
		{
			pos.y -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
			pos.x = (LONG)mHeadScreenX - ((mDeleteWaitWord.mPixelWidth)/2);
			FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mDeleteWaitWord.mString, mDeleteWaitWord.mStrLen, &pos, mDeleteWaitWord.mColor, true );
		}

		if( mEnablePVPMark && mpPVPMark )
		{
			pos.y -= ICON_SIZE;
			pos.x = (LONG)mHeadScreenX - (ICON_SIZE/2);

			mpPVPMark->SetTranslate( (short)pos.x, (short)pos.y );
			mpPVPMark->Draw();
		}
	}
	else
	{
		/// 이름
		pos.x = (LONG)mHeadScreenX - (mName.mPixelWidth/2);
		pos.y  -= FONTAGENT->GetTextHeight(cFontAgent::eFont_DepthFont);
		FONTAGENT->DrawText( cFontAgent::eFont_DepthFont, mName.mString, mName.mStrLen, &pos, mName.mColor, true );
	}
}
