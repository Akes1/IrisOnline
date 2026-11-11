#include "StdAfx.h"
#include "HeroStateWindow.h"

#include "FontAgent.h"

#include "Label.h"
#include "Gauge.h"
#include "ArcGauge.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "LevelScript.h"
#include "SkillExecuter.h"

#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "BaseIcon.h"

cHeroStateWindow::cHeroStateWindow()
: mpName(0)
, mpHPGauge(0)
, mpMPGauge(0)
, mpSXPGauge(0)
, mpEXPGauge(0)
, mpFace(0)
, mpLevelImage(0)
, mpLevelSkin(0)
, mpLeaderImage(0)
, mpNewMailLabel(0)
, mShowLeader( false )
, mShowNewMail( false )
, mLeaderPos(0, 0)
, mpHeroStateWinSkin(0)
, mpJobImage(0)
, mpJobSkin(0)
, mIsShowJobTip( false )
{
}

cHeroStateWindow::~cHeroStateWindow()
{
	SAFE_DELETE( mpJobImage );
	SAFE_DELETE( mpLeaderImage );
	SAFE_DELETE( mpLevelImage );
}

void cHeroStateWindow::Open()
{
	cUIWindow::Open();

	///직업 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	sJobInfo* pInfo = GAMERESOURCEMAN->GetJobInfo( HERO->GetJob() );
	if( pInfo )
	{
		cIconParam* pParam = UIMAN->GetIconParam( pInfo->jobTargetIconIndex );
		if( pParam )
		{
			mpJobImage->SetTexture( pParam->mpTexture );
			int tx = pParam->mTexPos.mX;
			int ty = pParam->mTexPos.mY;
			int  w = mpJobSkin->mSkinInfo->mWidth;
			int  h = mpJobSkin->mSkinInfo->mHeight;

			mpJobImage->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ));

		}
		else
			assert( 0 && "Fail to get icon param" );
	}
	else
		assert( 0 && "Fail to job info" );
}

void cHeroStateWindow::Close()
{
	cUIWindow::Close();

	mShowLeader   = false;
	mIsShowJobTip = false;
}

void cHeroStateWindow::InitHeroInfo()
{
	if( !mpName || !HERO )
	{
		assert(0);
		return;
	}

	/// 이름세팅
	LPTSTR str = HERO->GetName();
	if( mpName )
		mpName->SetText(str);
	
	/// 얼굴 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	cString skinName;
	unsigned int imgIndex = GetImageIndex( HERO->GetRace(), HERO->GetGender() );
	switch( imgIndex )
	{
	case 0:		skinName = "Face_Human_M";		break;
	case 1:		skinName = "Face_Beast_M";		break;
	case 2:		skinName = "Face_Elf_M";		break;
	case 3:		skinName = "Face_Human_W";		break;
	case 4:		skinName = "Face_Beast_W";		break;
	case 5:		skinName = "Face_Elf_W";		break;
	}

	/// 
	mpFaceSkin = pSkin->GetNodeSkin( skinName.Cstr() );
	if( mpFaceSkin && mpFace )
	{
		unsigned int tx = mpFaceSkin->mSkinInfo->mTexX;
		unsigned int ty = mpFaceSkin->mSkinInfo->mTexY;
		//unsigned int w = mpFaceSkin->mSkinInfo->mWidth;
		//unsigned int h = mpFaceSkin->mSkinInfo->mHeight;
		mpFace->ChangeImage( tx, ty );
	}

	/// 윈도우 초기화
	UpdateHeroInfo();
	UpdateExpSxp();

	mShowNewMail = false;

}

void cHeroStateWindow::UpdateExpSxp()
{
	if( HERO == 0 )
	{
		assert(0);
		return;
	}

	/// 캐릭터 경험치
	sExpTable*	pExp = LEVELSCRIPT->GetExpTable( HERO->GetLevel() );
	if( pExp )
	{
		unsigned int exp = HERO->GetHeroInfo()->Exp;

		SetMaxEXPGauge( (float)pExp->mExp );
		SetEXPGauge( static_cast<float>(exp) );
	}
	else
	{
		assert(0);
		return;
	}

	///
	sSxpTable*	pSxp = LEVELSCRIPT->GetSxpTable( HERO->GetSkillLevel() );
	if( pSxp )
	{
		unsigned int sxp = HERO->GetHeroInfo()->SkillExp;

		SetMAXSXP( (float)pSxp->mSxp );
		SetSXP( static_cast<float>(sxp) );
	}
	else
	{
		assert(0);
		return;
	}

}

void cHeroStateWindow::UpdateHeroInfo()
{
	if( HERO == 0 )
	{
		assert(0);
		return;
	}

	/// 레벨 세팅
	SetLevel( HERO->GetLevel() );

	/// hp, mp갱신
	UpdateHeroHPMP();
}

void cHeroStateWindow::UpdateHeroHPMP()
{
	if( HERO == 0 )
	{
		assert(0);
		return;
	}

	/// hp, mp갱신
	unsigned long maxHP = HERO->GetMaxHP();
	unsigned long curHP = HERO->GetHP();
	if( mpHPGauge && mpHPGauge->GetMaxValue() != maxHP )
		mpHPGauge->SetMaxValue( (float)maxHP );

	if( mpHPGauge && mpHPGauge->GetValue() != curHP )
		mpHPGauge->SetGaugeNoAni( (float)curHP );

	unsigned long maxMP = HERO->GetMaxMP();
	unsigned long curMP = HERO->GetMP();
	if( mpMPGauge && mpMPGauge->GetMaxValue() != maxMP )
		mpMPGauge->SetMaxValue( (float)maxMP );

	if( mpMPGauge && mpMPGauge->GetValue() != curMP )
		mpMPGauge->SetGaugeNoAni( (float)curMP );
}

char cHeroStateWindow::GetImageIndex( char race, char gender )
{
	/// 남자면 그대로 리턴
	/// 성별이 여자면,
	char result = race;
	if( gender == eGENDER_FEMALE )
	{
		if( race == eRACE_HUMAN ) result = 3;
		if( race == eRACE_BEAST ) result = 4;
		if( race == eRACE_ELF ) result = 5;
	}

	return result;
}

/// 주의사항 : 
bool cHeroStateWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpFace = (cLabel*)GetChild( eUIID_GAME_HERO_FACE );
	if( mpFace )
		mpFace->SetExceptEvent( true );

	mpName = (cLabel*)GetChild( eUIID_GAME_HERO_NAME_LABEL );
	mpHPGauge = (cDoubleGauge*)GetChild( eUIID_GAME_HERO_HP_GAUGE_BAR );
	mpMPGauge = (cDoubleGauge*)GetChild( eUIID_GAME_HERO_MP_GAUGE_BAR );
	mpSXPGauge = (cGauge*)GetChild( eUIID_GAME_HERO_SXP_GAUGE_BAR );
	mpEXPGauge = (cGauge*)GetChild( eUIID_GAME_HERO_EXP_GAUGE_BAR );
	mpNewMailLabel = (cLabel*)GetChild( EUIID_GAME_HERO_NEW_MAIL_LABEL ); 

	/// 얼굴 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	mpHeroStateWinSkin = pSkin->GetNodeSkin( "HeroStateWindow" );
	if( mpHeroStateWinSkin == NULL )
	{
		assert(0);
	}

	/// 디폴트 설정
	mpFaceSkin = pSkin->GetNodeSkin( "Face_Human_M" );
	if( mpFace )
	{
		unsigned int tx = mpFaceSkin->mSkinInfo->mTexX;
		unsigned int ty = mpFaceSkin->mSkinInfo->mTexY;
		mpFace->ChangeImage( tx, ty );
	}

	/// 레벨 배경 이미지
	mpLevelSkin = pSkin->GetNodeSkin( "HeroState_LevelImg" );
	if( mpLevelSkin )
	{
		mpLevelImage = new cUIImage( mpLevelSkin->mpTexture );

		unsigned int tx = mpLevelSkin->mSkinInfo->mTexX;
		unsigned int ty = mpLevelSkin->mSkinInfo->mTexY;
		unsigned int x = mpLevelSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLevelSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpLevelSkin->mSkinInfo->mWidth;
		unsigned int h = mpLevelSkin->mSkinInfo->mHeight;
		mpLevelImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLevelImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	/// 파티리더 이미지 
	cUINodeSkin* pLeaderSkin = pSkin->GetNodeSkin( "HeroState_Leader" );
	if( pLeaderSkin )
	{
		mpLeaderImage = new cUIImage( pLeaderSkin->mpTexture );

		unsigned int tx = pLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = pLeaderSkin->mSkinInfo->mTexY;
		unsigned int x = pLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = pLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = pLeaderSkin->mSkinInfo->mWidth;
		unsigned int h = pLeaderSkin->mSkinInfo->mHeight;

		mLeaderPos.mX = pLeaderSkin->mSkinInfo->mX;
		mLeaderPos.mY = pLeaderSkin->mSkinInfo->mY;

		mpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	/// 직업 이미지
	mpJobSkin = pSkin->GetNodeSkin( "HeroState_Job" );
	if( mpJobSkin )
	{
		mpJobImage = new cUIImage;
		unsigned int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpJobSkin->mSkinInfo->mWidth;
		unsigned int h = mpJobSkin->mSkinInfo->mHeight;
		mpJobImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	return true;
}

/// 주의사항 : 
bool cHeroStateWindow::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false )
		return mpParent->HandleEvent( event );

	if( event.mType == eUIEVENT_LBUTTON_DOWN || event.mType == eUIEVENT_LBUTTON_DBLCLK )
	{
		if( ContainPoint( event.mPos ) )
		{
			if( SKILLEXECUTER->IsSkillCharge() == true )
			{
				cBaseObject* curTarget = HERO->GetTargetObject();
				if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
				{
					/// 신규 타겟 등록후 발동
					HERO->SetTargetObject( eOBJECTTYPE_HERO, HERO->GetObjectID() );
				}
				else
				{
					/// 타겟 변경 없이 발동
					sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
					SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, HERO );
				}
			}
			else
			{
				HERO->SetTargetObject( eOBJECTTYPE_HERO, HERO->GetObjectID() );
			}
		}
	}

	return cUIWindow::HandleEvent( event );
}

void cHeroStateWindow::UpdateSkin()
{	
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}
	
	/// 사이즈는 새로운 상대좌표 * 새로운 해상도 
	int x = pnodeSkin->mSkinInfo->mX;
	int y = pnodeSkin->mSkinInfo->mY;

	/// 적용
	SetRelativePos( cUIPos( x, y ) );
}

void cHeroStateWindow::UpdateRect()
{
	
	cUIWindow::UpdateRect();

	if( mpLevelImage && mpLevelSkin )
	{
		short x = (short)(mpLevelSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft);
		short y = (short)(mpLevelSkin->mSkinInfo->mY + GetAbsoluteRect().mTop);
		mpLevelImage->SetPos( x, y );
	}

	if( mpLeaderImage )
	{
		short x = (short)(mLeaderPos.mX + GetAbsoluteRect().mLeft);
		short y = (short)(mLeaderPos.mY + GetAbsoluteRect().mTop);
		mpLeaderImage->SetPos( x, y );
	}

	if( mpJobSkin && mpJobImage )
	{
		unsigned int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpJobImage->SetPos( x, y );
	}
}

void cHeroStateWindow::OnMouseLeft( const cUIPos& pos )
{
	if( mIsShowJobTip )
	{
		UIMAN->HideTip();
		mIsShowJobTip = false;
	}
}

void cHeroStateWindow::OnMouseMove( const cUIPos& pos )
{
	cUIWindow::OnMouseMove( pos );

	cUIRect jobImgRect;
	jobImgRect.mLeft   = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
	jobImgRect.mTop    = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
	jobImgRect.mRight  = jobImgRect.mLeft + mpJobSkin->mSkinInfo->mWidth;
	jobImgRect.mBottom = jobImgRect.mTop + mpJobSkin->mSkinInfo->mHeight; 
	
	// 직업 이미지에 마우스 오버
	if ( jobImgRect.ContainPoint( pos ) == true )
	{
		if( mIsShowJobTip == false )
		{
			LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( HERO->GetJob() );
			if( jobName )
			{
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, jobName );
				mIsShowJobTip = true;
			}
		}
	}
	else
	{
		if( mIsShowJobTip == true )
		{
			UIMAN->HideTip();
			mIsShowJobTip = false;
		}
	}
}

void cHeroStateWindow::OnRender(cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	if( mpLevelSkin )
	{
		cUIRect rc;
		rc.mLeft = mpLevelSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		rc.mTop = mpLevelSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		rc.mRight = rc.mLeft + mpLevelSkin->mSkinInfo->mWidth;
		rc.mBottom = rc.mTop + mpLevelSkin->mSkinInfo->mHeight;

		unsigned int px = rc.mLeft + (int)((rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mLevel.Cstr(), mLevel.GetLength())) * 0.5f);
		unsigned int py = rc.mTop + (int)((rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI)) * 0.5f);
		pKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)mLevel.Cstr(), px, py, mDefaultColor );
	}

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 새 메일 이미지
	if( mpNewMailLabel )
	{
		if ( mShowNewMail )
			mpNewMailLabel->Show();
		else 
			mpNewMailLabel->Hide();
	}

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}

	/// 레벨 배경이미지
	if( mpLevelImage )
		mpLevelImage->Draw();

	/// 파티 리더 이미지
	if( mpLeaderImage && mShowLeader )
		mpLeaderImage->Draw();
	
	/// 직업 이미지
	if( mpJobImage )
		mpJobImage->Draw();

	pKeeper->DrawAll();
}

/// 주의사항 : 레벨세팅
void cHeroStateWindow::SetLevel( int level )
{
	mLevel.Format( _T("%d"), level );
}

/// 주의사항 : 현재 SXP 세팅 
void cHeroStateWindow::SetSXP( float value )
{
	if( mpSXPGauge )
		mpSXPGauge->SetGaugeNoAni( value );
}
/// 주의사항 : 최대 SXP 세팅 
void cHeroStateWindow::SetMAXSXP( float value )
{
	if( mpSXPGauge )
		mpSXPGauge->SetMaxValue( value );
}

void cHeroStateWindow::SetMaxEXPGauge( float value )
{
	if( mpEXPGauge )
		mpEXPGauge->SetMaxValue( value );
}

void cHeroStateWindow::SetEXPGauge( float value )
{
	if( mpEXPGauge )
		mpEXPGauge->SetGaugeNoAni( value );
}

void cHeroStateWindow::ShowLeaderImage( bool show )
{
	mShowLeader = show;
}

bool cHeroStateWindow::ShowNewMail( bool show )
{
	if( mShowNewMail == show )
		return false;

	mShowNewMail = show;

	return true;
}
