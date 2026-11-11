#include "StdAfx.h"
#include "HeroInfoWindow.h"

#include "Gauge.h"
#include "Label.h"
#include "RadioButton.h"
#include "GameUIManager.h"
#include "GameResourceManager.h"
#include "TitleManager.h"

#include "ObjectManager.h"

#include "LevelScript.h"
#include "ItemManager.h"
#include "UIImage.h"
#include "Player_Common.h"
#include "StatusScript.h"

cHeroInfoWindow::cHeroInfoWindow()
: mpHPGauge(0)
, mpMPGauge(0)
, mpEXPGauge(0)
, mpSXPGauge(0)
, mpName(0)
, mpRace(0)
, mpGender(0)
, mpClass(0)
, mpTitle(0)
, mpGuild(0)
, mpPosition(0)	/// 직위
, mpLevelFont(0)
, mpSkillLevel(0)	/// 스킬 레벨
, mpStr(0)		/// 힘
, mpDex(0)		/// 민첩
, mpCon(0)		/// 체력
, mpInt(0)		/// 지능
, mpWis(0)		/// 지혜
, mpPlusStr(0)	/// 힘 증가
, mpPlusDex(0)	/// 민첩 증가
, mpPlusCon(0)	/// 체력 증가
, mpPlusInt(0)	/// 지능 증가
, mpPlusWis(0)	/// 지혜 증가
, mpAtt(0)		/// 공격력
, mpDef(0)		/// 방어력
, mpHit(0)		/// 명중
, mpAvoid(0)	/// 회피
, mpMag(0)		/// 마법력
, mpRes(0)		/// 저항력
, mpAttackCri(0)
, mpMagicCri(0)
, mpHPRecover(0)
, mpMPRecover(0)
, mpFast(0)
, mpHeal(0)
, mpMagDef(0)
, mpMagHit(0)
, mpForceType(0)
, mpPhysicalButton(0)
, mpMagicButton(0)
, mLevelFontPos(0,0)
, mpLevelPosSkin(0)
, mpPhyProtect(0)
, mpMagProtect(0)
{
	::memset( mpInfoImgSkin, 0, sizeof( mpInfoImgSkin ) );
	::memset( mpInfoImg, 0, sizeof( mpInfoImg ) );
}

cHeroInfoWindow::~cHeroInfoWindow()
{

	for( int cnt = 0; cnt < eHERO_INFO_IMAGE_MAX; cnt++ )
	{
		SAFE_DELETE( mpInfoImg[cnt] );
	}
}

void cHeroInfoWindow::InitHeroInfo()
{
	if( HERO == 0 )
	{
		assert(0);
		return;
	}

	/// 이름세팅
	if( mpName )
		mpName->SetText( HERO->GetName() );

	/// 종족 세팅
	if( mpRace )
		mpRace->SetText( GAMERESOURCEMAN->GetRaceName( HERO->GetRace() ) );

	/// 성별 세팅
	if( mpGender )
	{
		if( HERO->GetGender() == 0 || HERO->GetGender() == 2 )
			mpGender->SetText( GAMERESOURCEMAN->GetGameText( 1005 ) );		// 남
		else 
			mpGender->SetText( GAMERESOURCEMAN->GetGameText( 1006 ) );		// 여
	}

	UpdateHeroInfo();
	UpdateExpSxp();
}

void cHeroInfoWindow::UpdateExpSxp()
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
		SetMAXEXP( pExp->mExp );
		SetEXP( exp );
	}
	else
	{
		assert(0);
		return;
	}

	/// 캐릭터 스킬 경험치
	sSxpTable*	pSxp = LEVELSCRIPT->GetSxpTable( HERO->GetSkillLevel() );
	if( pSxp )
	{
		unsigned int sxp = HERO->GetHeroInfo()->SkillExp;
		SetMAXSXP( pSxp->mSxp );
		SetSXP( sxp );
	}
	else
	{
		assert(0);
		return;
	}
}

void cHeroInfoWindow::UpdateHeroInfo()
{
	assert( HERO );

	/// 캐릭터 레벨
	SetLevel( HERO->GetLevel() );

	/// 캐릭터 스킬 레벨
	SetSkillLevel( HERO->GetSkillLevel() );

	/// 캐릭터 호칭 셋팅
	cTitleDefine* define = TITLEMAN->GetTitleDefine( HERO->GetTitleIndex() );
	if( define )
	{
		cStringT name = define->mName;
		name.TrimLeft(_T("["));
		name.TrimRight(_T("]"));
		mpTitle->SetText( name.Cstr() );
	}

	/// 인벤토리도 세팅
	ITEMMAN->SetMaxWeightGauge( (float)HERO->GetMaxWeight() );

	/// 클래스
	SetClass( HERO->GetJob() );

	/// 세력 세팅
	SetForceType( HERO->GetForceType() );

	/// 길드
	SetGuild( HERO->GetGuildName() );

	///// 힘
	//SetStr( FloatToInt( HERO->GetStatus1()->mStr ), COLOR_WHITE );	
	///// 민첩
	//SetDex( FloatToInt( HERO->GetStatus1()->mDex ), COLOR_WHITE );	
	///// 체력
	//SetCon( FloatToInt( HERO->GetStatus1()->mCon ), COLOR_WHITE );	
	///// 지능
	//SetInt( FloatToInt( HERO->GetStatus1()->mInt ), COLOR_WHITE );	
	///// 지혜
	//SetWis( FloatToInt( HERO->GetStatus1()->mWis ), COLOR_WHITE );	

	/// 힘
	SetStr( FloatToInt( HERO->GetStatusBaseTotal( ePLAYER_STATUS_STR ) ), COLOR_WHITE );	
	/// 민첩
	SetDex( FloatToInt( HERO->GetStatusBaseTotal( ePLAYER_STATUS_DEX ) ), COLOR_WHITE );	
	/// 체력
	SetCon( FloatToInt( HERO->GetStatusBaseTotal( ePLAYER_STATUS_CON ) ), COLOR_WHITE );	
	/// 지능
	SetInt( FloatToInt( HERO->GetStatusBaseTotal( ePLAYER_STATUS_INT ) ), COLOR_WHITE );	
	/// 지혜
	SetWis( FloatToInt( HERO->GetStatusBaseTotal( ePLAYER_STATUS_WIS ) ), COLOR_WHITE );	

	/// 스테이터스 추가량 출력
	SetPlusStr( FloatToInt( HERO->GetStatusPlusTotal( ePLAYER_STATUS_STR ) ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_STR ) );
	SetPlusAgi( FloatToInt( HERO->GetStatusPlusTotal( ePLAYER_STATUS_DEX ) ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_DEX ) );
	SetPlusCon( FloatToInt( HERO->GetStatusPlusTotal( ePLAYER_STATUS_CON ) ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_CON ) );
	SetPlusInt( FloatToInt( HERO->GetStatusPlusTotal( ePLAYER_STATUS_INT ) ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_INT ) );
	SetPlusWis( FloatToInt( HERO->GetStatusPlusTotal( ePLAYER_STATUS_WIS ) ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_WIS ) );

	/*SetPlusStr( FloatToInt( HERO->GetStatus1Plus()->mStr ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_STR ) );
	SetPlusAgi( FloatToInt( HERO->GetStatus1Plus()->mDex ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_DEX ) );
	SetPlusCon( FloatToInt( HERO->GetStatus1Plus()->mCon ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_CON ) );
	SetPlusInt( FloatToInt( HERO->GetStatus1Plus()->mInt ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_INT ) );
	SetPlusWis( FloatToInt( HERO->GetStatus1Plus()->mWis ), HERO->GetBaseStatusInfoType( ePLAYER_STATUS_WIS ) );*/

	/// 무기에 따른 공격력 출력
	if( HERO->GetWeaponState() == eWEAPON_STATE_GUN ) 
	{
		/// 원거리 무기
		SetAtt( FloatToInt(HERO->GetStatus2()->mPhysicMinRangeAttack), 
				FloatToInt(HERO->GetStatus2()->mPhysicMaxRangeAttack), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_RATK ) );	
	}
	else
	{
		/// 근거리 무기
		SetAtt( FloatToInt(HERO->GetStatus2()->mPhysicMinAttack), 
				FloatToInt(HERO->GetStatus2()->mPhysicMaxAttack), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_ATK ) );	
	}

	/// 방어력
	SetDef( FloatToInt( HERO->GetStatus2()->mPhysicDefense ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_DEF ) );	
	
	/// 마법력
	SetMag( FloatToInt( HERO->GetStatus2()->mMagicMinAttack ), 
			FloatToInt( HERO->GetStatus2()->mMagicMaxAttack ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MATK ) );	

	/// 저항력
	SetRes( FloatToInt( HERO->GetStatus2()->mMagicAvoid ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MAVOID ) );	
	/// 명중
	SetHit( FloatToInt( HERO->GetStatus2()->mPhysicAttackRate ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_HIT ) );	
	/// 회피
	SetAvoid( FloatToInt( HERO->GetStatus2()->mPhysicAvoid ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_AVOID ) );	

	/// 물리 크리티컬
	SetAttackCritical( HERO->GetStatus2()->mPhysicCritical, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_CRI ));
	/// 마법 크리티컬
	SetMagicCritical( HERO->GetStatus2()->mMagicCritical, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MCRI ));
	/// HP 회복량
	SetHPrecovery( HERO->GetStatus2()->mRecovHP , HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_RECOVHP ));
	/// MP 회복량
	SetMPrecovery( HERO->GetStatus2()->mRecovMP, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_RECOVMP ));

	SetFast( FloatToInt(HERO->GetStatus2()->mMoveSpeed), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_SPD ) );
	SetHeal( FloatToInt( HERO->GetStatus2()->mHeal ), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_HEAL ) );
	SetMagDef( FloatToInt(HERO->GetStatus2()->mMagicDefense), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MDEF ) );
	SetMagHit( FloatToInt(HERO->GetStatus2()->mMagicAttackRate), HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MHIT ) );

	SetPhyProtect( HERO->GetStatus2()->mPhyCriDef, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_CRIDEF ) );
	SetMagProtect( HERO->GetStatus2()->mMagCriDef, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_MCRIDEF ) );

	/// HP,MP세팅
	UpdateHeroHPMP();
}

void cHeroInfoWindow::UpdateHeroHPMP()
{
	unsigned long maxHP = HERO->GetMaxHP();
	if( mpHPGauge )
	{
		if( mpHPGauge->GetMaxValue() != maxHP )
			mpHPGauge->SetMaxValue( (float)maxHP );
	}

	unsigned long curHP = HERO->GetHP();
	if( mpHPGauge )
	{
		if( mpHPGauge->GetValue() != curHP )
			mpHPGauge->SetGaugeNoAni( (float)curHP );
	}

	unsigned long maxMP = HERO->GetMaxMP();
	if( mpMPGauge )
	{
		if( mpMPGauge->GetMaxValue() != maxMP )
			mpMPGauge->SetMaxValue( (float)maxMP );
	}

	unsigned long curMP = HERO->GetMP();
	if( mpMPGauge )
	{
		if( mpMPGauge->GetValue() != curMP )
			mpMPGauge->SetGaugeNoAni( (float)curMP );
	}
}

// 주의사항 : 
bool cHeroInfoWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpHPGauge = (cDoubleGauge*)GetChild( eUIID_HERO_HP_GAUGE_BAR );
	mpMPGauge = (cDoubleGauge*)GetChild( eUIID_HERO_MP_GAUGE_BAR );
	mpEXPGauge = (cGauge*)GetChild( eUIID_HERO_EXP_GAUGE_BAR );
	mpSXPGauge = (cGauge*)GetChild( eUIID_HERO_SXP_GAUGE_BAR );

	mpLevelFont = UIMAN->GetImageFont( eIAMGEFONT_HEROLEVEL );
	mpSkillLevel = (cLabel*)GetChild( eUIID_HERO_SKILLLEVEL_LABEL );	 

	mpName = (cLabel*)GetChild( eUIID_HERO_NAME );	
	mpRace  = (cLabel*)GetChild( eUIID_HERO_RACE );	
	mpGender  = (cLabel*)GetChild( eUIID_HERO_GENDER );	
	mpClass  = (cLabel*)GetChild( eUIID_HERO_CLASS );	
	mpTitle = (cLabel*)GetChild( eUIID_HERO_TITLE );	
	mpGuild  = (cLabel*)GetChild( eUIID_HERO_GUILD );	
	mpPosition  = (cLabel*)GetChild( eUIID_HERO_POSITION );

	mpStr = (cLabel*)GetChild( eUIID_HERO_STR );	
	mpDex = (cLabel*)GetChild( eUIID_HERO_DEX );	
	mpCon = (cLabel*)GetChild( eUIID_HERO_CON );	
	mpInt = (cLabel*)GetChild( eUIID_HERO_INT );	
	mpWis = (cLabel*)GetChild( eUIID_HERO_WIS );	

	mpPlusStr = (cLabel*)GetChild( eUIID_HERO_PLUSSTR );	
	mpPlusDex = (cLabel*)GetChild( eUIID_HERO_PLUSDEX );	
	mpPlusCon = (cLabel*)GetChild( eUIID_HERO_PLUSCON );	
	mpPlusInt = (cLabel*)GetChild( eUIID_HERO_PLUSINT );	
	mpPlusWis = (cLabel*)GetChild( eUIID_HERO_PLUSWIS );	

	mpAtt = (cLabel*)GetChild( eUIID_HERO_ATT );	
	mpDef = (cLabel*)GetChild( eUIID_HERO_DEF );	
	mpHit = (cLabel*)GetChild( eUIID_HERO_HIT );	
	mpAvoid = (cLabel*)GetChild( eUIID_HERO_AVOID );	

	mpMag = (cLabel*)GetChild( eUIID_HERO_MAG );	
	mpRes = (cLabel*)GetChild( eUIID_HERO_RES );	

	mpAttackCri = (cLabel*)GetChild( eUIID_HERO_ATTACKCRI );	
	mpMagicCri = (cLabel*)GetChild( eUIID_HERO_MAGICCRI );	
	mpHPRecover = (cLabel*)GetChild( eUIID_HERO_HPRECOVER );	
	mpMPRecover = (cLabel*)GetChild( eUIID_HERO_MPRECOVER );	

	mpFast = (cLabel*)GetChild( eUIID_HERO_FAST );	
	mpHeal = (cLabel*)GetChild( eUIID_HERO_HEAL );	
	mpMagDef = (cLabel*)GetChild( eUIID_HERO_MAGDEF );	
	mpMagHit = (cLabel*)GetChild( eUIID_HERO_MAGHIT );	

	mpForceType = (cLabel*)GetChild( eUIID_HERO_FORCETYPE );

	mpPhysicalButton = (cRadioButton*)GetChild( eUIID_HERO_PHYSICAL_BUTTON );
	mpMagicButton    = (cRadioButton*)GetChild( eUIID_HERO_MAGIC_BUTTON);

	mpPhyProtect = (cLabel*)GetChild( eUIID_HERO_PHYSICSPROTECT );	
	mpMagProtect = (cLabel*)GetChild( eUIID_HERO_MAGICPROTECT );	

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	for( int cnt = 0; cnt < eHERO_INFO_IMAGE_MAX; cnt++ )
	{
		cString name;
		name.Format( "HeroInfoImage%02d", cnt );
		mpInfoImgSkin[cnt] = pSkin->GetNodeSkin( name );
		if( mpInfoImgSkin[cnt] )
		{
			mpInfoImg[cnt] = new cUIImage( mpInfoImgSkin[cnt]->mpTexture );

			unsigned int tx =  mpInfoImgSkin[cnt]->mSkinInfo->mTexX;
			unsigned int ty =  mpInfoImgSkin[cnt]->mSkinInfo->mTexY;
			unsigned int  w =  mpInfoImgSkin[cnt]->mSkinInfo->mWidth;
			unsigned int  h =  mpInfoImgSkin[cnt]->mSkinInfo->mHeight;
			unsigned int  x =  mpInfoImgSkin[cnt]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpInfoImgSkin[cnt]->mSkinInfo->mY + mAbsoluteRect.mTop;

			mpInfoImg[cnt]->SetTextureRect( tx, ty, tx + w, ty + h );
			mpInfoImg[cnt]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
		}
		else
		{
			assert(0);
			return false;
		}
	}

	mpLevelPosSkin = pSkin->GetNodeSkin( "Hero_Level_Img_Pos" );
	if( mpLevelPosSkin )
	{
		mLevelFontPos.mX = mpLevelPosSkin->mSkinInfo->mX;
		mLevelFontPos.mY = mpLevelPosSkin->mSkinInfo->mY;
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

void cHeroInfoWindow::Close()
{
	cUIWindow::Close();

	mpTitle->Clear();
	mpGuild->Clear();
	mpPosition->Clear();
}

/// 주의사항 : 
void cHeroInfoWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
		case eUIID_HERO_CLOSE_BUTTON :
			Hide();
			break;
		case eUIID_HERO_SOCIAL_BUTTON:
			GAMEUI->ToggleSocialWindow();
			break;
		case eUIID_HERO_TITLE_BUTTON:
			GAMEUI->ToggleTitleWindow();
			break;
	}

}

void cHeroInfoWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	if( mpPhysicalButton && mpPhysicalButton->IsPress() )
	{
		if( mpInfoImg[eHERO_INFO_PHYSICAL_IMAGE] )
			mpInfoImg[eHERO_INFO_PHYSICAL_IMAGE]->Draw();
	}
	else if( mpMagicButton && mpMagicButton->IsPress() )
	{
		if( mpInfoImg[eHERO_INFO_MAGIC_IMAGE] )
			mpInfoImg[eHERO_INFO_MAGIC_IMAGE]->Draw();
	}

	if( mpLevelFont )
		mpLevelFont->DrawText( mLevelFontPos.mX + mAbsoluteRect.mLeft, mLevelFontPos.mY + mAbsoluteRect.mTop, 	mHeroLevel.Cstr() );

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
	pKeeper->DrawAll();
}

void cHeroInfoWindow::OnRadioPressed( cUINode* , unsigned int id )
{
	if( id == eUIID_HERO_PHYSICAL_BUTTON )
	{
		/// 물리수치
		mpAtt->Show();	/// 공격력
		mpDef->Show();	/// 방어력
		mpHit->Show();			/// 명중
		mpAvoid->Show();		/// 회피
		mpAttackCri->Show();	/// 물리 크리티컬
		mpPhyProtect->Show();	/// 물리 방호

		/// 마법수치 
		mpMag->Hide();			/// 마법력
		mpMagDef->Hide();		/// 마법방어
		mpMagHit->Hide();		/// 마법명중
		mpRes->Hide();			/// 저항력
		mpMagicCri->Hide();		/// 마법 크리티컬
		mpHeal->Hide();			/// 힐회복량
		mpMagProtect->Hide();	/// 마법 방호

	}
	else if( id == eUIID_HERO_MAGIC_BUTTON )
	{
		/// 물리수치
		mpAtt->Hide();			/// 공격력
		mpDef->Hide();			/// 방어력
		mpHit->Hide();			/// 명중
		mpAvoid->Hide();		/// 회피
		mpAttackCri->Hide();	/// 물리 크리티컬
		mpPhyProtect->Hide();	/// 물리 방호

		/// 마법수치 
		mpMag->Show();			/// 마법력
		mpMagDef->Show();		/// 마법방어
		mpMagHit->Show();		/// 마법명중
		mpRes->Show();			/// 저항력
		mpMagicCri->Show();		/// 마법 크리티컬
		mpHeal->Show();			/// 힐회복량
		mpMagProtect->Show();	/// 마법 방호
	}

}

void cHeroInfoWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	for( int cnt = 0; cnt < eHERO_INFO_IMAGE_MAX; cnt++ )
	{
		if( mpInfoImgSkin[cnt] )
		{
			unsigned int  w =  mpInfoImgSkin[cnt]->mSkinInfo->mWidth;
			unsigned int  h =  mpInfoImgSkin[cnt]->mSkinInfo->mHeight;
			unsigned int  x =  mpInfoImgSkin[cnt]->mSkinInfo->mX + mAbsoluteRect.mLeft;
			unsigned int  y =  mpInfoImgSkin[cnt]->mSkinInfo->mY + mAbsoluteRect.mTop;

			mpInfoImg[cnt]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
		}
	}
}

void cHeroInfoWindow::OnShow()
{
	cUIWindow::OnShow();

	ePLAYER_JOB heroJob = HERO->GetJob();
	switch( heroJob )
	{
	/// 마법사 군
	case ePLAYER_MAGE       : /// 메이지
	case ePLAYER_MAGICIAN   : /// 매지션
	case ePLAYER_PRIEST     : /// 프리스트
	case ePLAYER_WIZARD     : /// 위자드
	case ePLAYER_HIGHPRIEST : /// 하이프리스트
	case ePLAYER_WARLOCK    : /// 워록
	case ePLAYER_SHAMAN     : /// 샤먼
	case ePLAYER_SAGE       : /// 세이지
	case ePLAYER_SORCERER   : /// 소서러
	case ePLAYER_MENTALIST  : /// 멘탈리스트
		if( mpMagicButton->IsPress() == false )
			mpMagicButton->SetPress( true, false );
		break;
	default:
		if( mpPhysicalButton->IsPress() == false )
			mpPhysicalButton->SetPress( true, false );
		break;
	}
}

/// 주의사항 : EXP 게이지와 EXP 라벨 세팅 
void cHeroInfoWindow::SetEXP( double exp )
{
	/// EXP 
	if( mpEXPGauge )
	{
		mpEXPGauge->SetGaugeNoAni( exp );

		cStringT str;
		str.Format( _T("%.2f"), mpEXPGauge->GetPercent() );
	}
}

/// 주의사항 : EXP 게이지와 EXP 라벨 세팅 
void cHeroInfoWindow::SetMAXEXP( double exp )
{
	if( mpEXPGauge )
		mpEXPGauge->SetMaxValue( exp );
}

/// 주의사항 : 스킬 경험치 세팅 
void cHeroInfoWindow::SetSXP( double sexp )
{
	/// sxp 세팅
	if( mpSXPGauge )
	{
		mpSXPGauge->SetGaugeNoAni( sexp );

		cStringT str;
		str.Format( _T("%.2f"), mpSXPGauge->GetPercent() );
	}
}

/// 주의사항 : 스킬 경험치 세팅 
void cHeroInfoWindow::SetMAXSXP( double sexp )
{
	if( mpSXPGauge )
		mpSXPGauge->SetMaxValue( sexp );
}

/// 주의사항 : 레벨 세팅
void cHeroInfoWindow::SetLevel( int level )
{
	if( level < 10 )
	{
		if( mpLevelPosSkin )
		{
			mLevelFontPos.mX = mpLevelPosSkin->mSkinInfo->mX;
			mLevelFontPos.mY = mpLevelPosSkin->mSkinInfo->mY;
		}
		else
		{
			assert(0);
			return;
		}

		mLevelFontPos.mX += (unsigned int)(mpLevelFont->GetTextWidth( level ) * 0.5);
	}
	else
	{
		if( mpLevelPosSkin )
		{
			mLevelFontPos.mX = mpLevelPosSkin->mSkinInfo->mX;
			mLevelFontPos.mY = mpLevelPosSkin->mSkinInfo->mY;
		}
		else
		{
			assert(0);
			return;
		}
	}
	
	mHeroLevel.Format("%d", level );
}

/// 주의사항 : 스킬 레벨 세팅
void cHeroInfoWindow::SetSkillLevel( int slevel )
{
	if( mpSkillLevel )
		mpSkillLevel->SetValue( slevel );
}

/// 클래스
void cHeroInfoWindow::SetClass( unsigned long job )
{
	if( mpClass )
		mpClass->SetText( GAMERESOURCEMAN->GetJobName( job ) );
}

void cHeroInfoWindow::SetTitle( LPCTSTR text )
{
	/// 타이틀 세팅
	if( mpTitle )
		mpTitle->SetText( text );
}

/// 길드
void cHeroInfoWindow::SetGuild( LPCTSTR text )
{
	int len = ::_tcslen( text );
	if( mpGuild )
	{
		if( len > 0 )
			mpGuild->SetText( text );
		else
			mpGuild->Clear();
	}
}

/// 직위
void cHeroInfoWindow::SetPosition( LPCTSTR text )
{
	int len = ::_tcslen( text );
	if( mpPosition )
	{
		if( len > 0 )
			mpPosition->SetText( text );
		else
			mpPosition->Clear();
	}
}

/// 힘
void cHeroInfoWindow::SetStr( int value, unsigned long color )	
{
	if( mpStr )
	{
		mpStr->SetTextColor( color );
		mpStr->SetValue( value );
	}
}
/// 민첩
void cHeroInfoWindow::SetDex( int value, unsigned long color )	
{
	if( mpDex )
	{
		mpDex->SetTextColor( color );
		mpDex->SetValue( value );
	}
}	
/// 체력
void cHeroInfoWindow::SetCon( int value, unsigned long color )		
{
	if( mpCon )
	{
		mpCon->SetTextColor( color );
		mpCon->SetValue( value );
	}
}
/// 지능
void cHeroInfoWindow::SetInt( int value, unsigned long color )		
{
	if( mpInt )
	{
		mpInt->SetTextColor( color );
		mpInt->SetValue( value );
	}
}
/// 지혜
void cHeroInfoWindow::SetWis( int value, unsigned long color )		
{
	if( mpWis )
	{
		mpWis->SetTextColor( color );
		mpWis->SetValue( value );
	}
}

//// 추가량 
/// 힘
void cHeroInfoWindow::SetPlusStr( int value, eStateInfo_Type type )	
{
	if( mpPlusStr )
	{
		cStringT str;
		if( value > 0 )
			str.Format( _T("+%d"), value );
		else
			str.Format( _T("%d"), value );

		unsigned long color = GetColorForStateType(type);
		mpPlusStr->SetTextColor( color );
		mpPlusStr->SetText( (LPCTSTR)str.Cstr() );

		if( str.IsEmpty() == false && color == COLOR_WHITE )
		{
			assert(0);
		}

		if( value == 0 )	
			mpPlusStr->Hide();
		else
			mpPlusStr->Show();
	}
}
/// 민첩
void cHeroInfoWindow::SetPlusAgi( int value, eStateInfo_Type type )	
{
	if( mpPlusDex )
	{
		cStringT str;
		if( value > 0 )
			str.Format( _T("+%d"), value );
		else
			str.Format( _T("%d"), value );

		unsigned long color = GetColorForStateType(type);
		mpPlusDex->SetTextColor( color );
		mpPlusDex->SetText( (LPCTSTR)str.Cstr() );

		if( str.IsEmpty() == false && color == COLOR_WHITE )
		{
			assert(0);
		}

		if( value == 0 )
			mpPlusDex->Hide();
		else
			mpPlusDex->Show();
	}
}	
/// 체력
void cHeroInfoWindow::SetPlusCon( int value, eStateInfo_Type type )		
{
	if( mpPlusCon )
	{
		cStringT str;
		if( value > 0 )
			str.Format( _T("+%d"), value );
		else
			str.Format( _T("%d"), value );

		unsigned long color = GetColorForStateType(type);
		mpPlusCon->SetTextColor( GetColorForStateType(type) );
		mpPlusCon->SetText( (LPCTSTR)str.Cstr() );

		if( str.IsEmpty() == false && color == COLOR_WHITE )
		{
			assert(0);
		}

		if( value == 0 )	
			mpPlusCon->Hide();
		else
			mpPlusCon->Show();
	}
}
/// 지능
void cHeroInfoWindow::SetPlusInt( int value, eStateInfo_Type type )		
{
	if( mpPlusInt )
	{
		cStringT str;
		if( value > 0 )
			str.Format( _T("+%d"), value );
		else
			str.Format( _T("%d"), value );

		unsigned long color = GetColorForStateType(type);
		mpPlusInt->SetTextColor( GetColorForStateType(type) );
		mpPlusInt->SetText( (LPCTSTR)str.Cstr() );

		if( str.IsEmpty() == false && color == COLOR_WHITE )
		{
			assert(0);
		}

		if( value == 0 )	
			mpPlusInt->Hide();
		else
			mpPlusInt->Show();
	}
}
/// 지혜
void cHeroInfoWindow::SetPlusWis( int value, eStateInfo_Type type )		
{
	if( mpPlusWis )
	{
		cStringT str;
		if( value > 0 )
			str.Format( _T("+%d"), value );
		else
			str.Format( _T("%d"), value );

		mpPlusWis->SetTextColor( GetColorForStateType(type) );
		mpPlusWis->SetText( (LPCTSTR)str.Cstr() );

		if( value == 0 )
			mpPlusWis->Hide();
		else
			mpPlusWis->Show();
	}
}

/// 물리수치
/// 공격력
void cHeroInfoWindow::SetAtt( int min, int max, eStateInfo_Type type )		
{
	if( mpAtt )
	{
		cStringT str;
		str.Format( _T("%d~%d"), min, max );

		/// 세팅
		mpAtt->SetTextColor( GetColorForStateType(type) );
		mpAtt->SetText( (LPCTSTR)str.Cstr() );
	}
}
/// 방어력
void cHeroInfoWindow::SetDef( int value, eStateInfo_Type type )		
{
	if( mpDef )
	{
		mpDef->SetTextColor( GetColorForStateType(type) );
		mpDef->SetValue( value );
	}
}
/// 명중
void cHeroInfoWindow::SetHit( int value, eStateInfo_Type type )		
{
	if( mpHit )
	{
		cStringT str;
		str.Format( _T("%d%%"), value );

		mpHit->SetTextColor( GetColorForStateType(type) );
		mpHit->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// 회피
void cHeroInfoWindow::SetAvoid( int value, eStateInfo_Type type )		
{
	if( mpAvoid )
	{
		cStringT str;
		str.Format( _T("%d%%"), value );

		mpAvoid->SetTextColor( GetColorForStateType(type) );
		mpAvoid->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// 마법력
void cHeroInfoWindow::SetMag( int min, int max, eStateInfo_Type type )		
{
	if( mpMag )
	{
		cStringT str;
		str.Format( _T("%d~%d"), min, max );

		/// 세팅
		mpMag->SetTextColor( GetColorForStateType(type) );
		mpMag->SetText( (LPCTSTR)str.Cstr() );
	}
}
/// 저항력
void cHeroInfoWindow::SetRes( int value, eStateInfo_Type type )		
{
	if( mpRes )
	{
		cStringT str;
		str.Format( _T("%d%%"), value );

		mpRes->SetTextColor( GetColorForStateType(type) );
		mpRes->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// 물리 크리티컬
void cHeroInfoWindow::SetAttackCritical( float value, eStateInfo_Type type )
{
	if( mpAttackCri )
	{
		cStringT str;
		str.Format( _T("%.2f%%"), value );

		mpAttackCri->SetTextColor( GetColorForStateType(type) );
		mpAttackCri->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// 마법 크리티컬
void cHeroInfoWindow::SetMagicCritical( float value, eStateInfo_Type type )
{
	if( mpMagicCri )
	{
		cStringT str;
		str.Format( _T("%.2f%%"), value );

		mpMagicCri->SetTextColor( GetColorForStateType(type) );
		mpMagicCri->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// HP 회복량
void cHeroInfoWindow::SetHPrecovery( float value, eStateInfo_Type type )
{
	if( HERO->GetState() == ePLAYER_STATE_SITDOWN )
		value = value * STATUSSCRIPT->GetDamageCalcNumericalInfo( 10 );

	if( mpHPRecover )
	{
		cStringT str;
		str.Format( _T("%.2f"), value );

		/// 세팅
		mpHPRecover->SetTextColor( GetColorForStateType(type) );
		mpHPRecover->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// MP 회복량
void cHeroInfoWindow::SetMPrecovery( float value, eStateInfo_Type type )
{
	if( HERO->GetState() == ePLAYER_STATE_SITDOWN )
		value = value * STATUSSCRIPT->GetDamageCalcNumericalInfo( 10 );

	if( mpMPRecover )
	{
		cStringT str;
		str.Format( _T("%.2f"), value );

		/// 세팅
		mpMPRecover->SetTextColor( GetColorForStateType(type) );
		mpMPRecover->SetText( (LPCTSTR)str.Cstr() );
	}
}

void cHeroInfoWindow::SetFast( int value, eStateInfo_Type type )
{
	if( mpFast )
	{
		cStringT str;
		str.Format( _T("%d"), value );

		/// 세팅
		mpFast->SetTextColor( GetColorForStateType(type) );
		mpFast->SetText( (LPCTSTR)str.Cstr() );
	}
}

void cHeroInfoWindow::SetHeal( int value, eStateInfo_Type type )
{
	if( mpHeal )
	{
		cStringT str;
		str.Format( _T("%d"), value );

		/// 세팅
		mpHeal->SetTextColor( GetColorForStateType(type) );
		mpHeal->SetText( (LPCTSTR)str.Cstr() );
	}
}

void cHeroInfoWindow::SetMagDef( int value, eStateInfo_Type type )
{
	if( mpMagDef )
	{
		cStringT str;
		str.Format( _T("%d"), value );

		/// 세팅
		mpMagDef->SetTextColor( GetColorForStateType(type) );
		mpMagDef->SetText( (LPCTSTR)str.Cstr() );
	}
}

// 물리 방호
void cHeroInfoWindow::SetPhyProtect( float value, eStateInfo_Type type )
{
	if( mpPhyProtect )
	{
		cStringT str;
		str.Format( _T("%.2f%%"), value );

		mpPhyProtect->SetTextColor( GetColorForStateType(type) );
		mpPhyProtect->SetText( (LPCTSTR)str.Cstr() );		
	}

}

// 마법 방호
void cHeroInfoWindow::SetMagProtect( float value, eStateInfo_Type type )
{
	if( mpMagProtect )
	{
		cStringT str;
		str.Format( _T("%.2f%%"), value );

		mpMagProtect->SetTextColor( GetColorForStateType(type) );
		mpMagProtect->SetText( (LPCTSTR)str.Cstr() );		
	}
}

void cHeroInfoWindow::SetMagHit( int value, eStateInfo_Type type )
{
	if( mpMagHit )
	{
		cStringT str;
		str.Format( _T("%d%%"), value );

		/// 세팅
		mpMagHit->SetTextColor( GetColorForStateType(type) );
		mpMagHit->SetText( (LPCTSTR)str.Cstr() );
	}
}

/// 세력 타입
void cHeroInfoWindow::SetForceType( unsigned char type )
{
	int textIdx = 0;
	unsigned long toolTipIdx = 0;
	switch( type )
	{
	case eFORCETYPE_NONE:	textIdx = 119;	break;
	case eFORCETYPE_FIRE:	textIdx = 120;	toolTipIdx = 490; break;
	case eFORCETYPE_WATER:  textIdx = 121;	toolTipIdx = 491; break;
	case eFORCETYPE_WIND:	textIdx = 122;	toolTipIdx = 492; break;
	case eFORCETYPE_EARTH:	textIdx = 123;	toolTipIdx = 493; break;
	default: assert(0); return;
	}

	if( mpForceType )
	{
		mpForceType->SetText( UIMAN->GetUIText( textIdx ) );
		mpForceType->SetHoverTip( toolTipIdx );
	}
}

///
unsigned long cHeroInfoWindow::GetColorForStateType( eStateInfo_Type type )
{
	unsigned long color = mDefaultColor;

	switch( type )
	{
	case eSTATE_NORMAL:		color = mDefaultColor;		break;
	case eSTATE_PLUS:		color = COLOR_GREEN;		break;
	case eSTATE_MINUS:		color = COLOR_RED;			break;
	case eSTATE_OVERLAPPLUS:
	case eSTATE_OVERLAPMINUS:
		color = COLOR_YELLOW;
		break;
	}

	return color;
}
