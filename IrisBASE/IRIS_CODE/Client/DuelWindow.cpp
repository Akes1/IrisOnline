#include "StdAfx.h"
#include ".\duelwindow.h"

#include "Label.h"
#include "Gauge.h"

#include "DuelManager.h"
#include "ObjectManager.h"
#include "Hero.h"

#include "FontAgent.h"
#include "GameResourceManager.h"

#include "DuelBuff.h"
#include "SkillObject.h"

#include "GameUIManager.h"
#include "TargetWindow.h"

#include "ChatManager.h"
#include "PartyManager.h"
#include "PartyUnionManager.h"

cDuelWindow::cDuelWindow()
: mpHeroHP(0)
, mpHeroMP(0)
, mpHeroName(0)
, mpHeroBuff(0)
, mpHeroDeBuff(0)
, mpEnemyHP(0)
, mpEnemyMP(0)
, mpEnemyName(0)
, mpEnemyBuff(0)
, mpEnemyDeBuff(0)
, mpCountLabel(0)
, mpFightLabel(0)
, mpHeroFace(0)
, mpEnemyFace(0)
{
	mShowProcess = eNone;
	mOutRangeTime = 0;
	mOutCount = 0;
}

cDuelWindow::~cDuelWindow()
{
}

void cDuelWindow::SetDuelInfo( cPlayer* enemy )
{
	if( enemy == 0 )
	{
		assert(0);
		return;
	}

	mOutRangeTime = 0;

	/// setting name 
	if( mpHeroName )
		mpHeroName->SetText( HERO->GetName() );
	if( mpEnemyName )
		mpEnemyName->SetText( enemy->GetName() );

	/// setting gauge
	UpdateHeroHP( HERO->GetHP(), HERO->GetMaxHP() );
	UpdateHeroMP( HERO->GetMP(), HERO->GetMaxMP() );

	UpdateEnemyHP( enemy->GetHP(), enemy->GetMaxHP() );
	UpdateEnemyMP( enemy->GetMP(), enemy->GetMaxMP() );

	/// setting face image
	if( mpHeroFace )
	{
		char race = HERO->GetRace();
		char gender = HERO->GetGender();

		cUIRect r = mFaceRect[race * eGENDER_MAX + gender];

		mpHeroFace->ChangeImage( r.mLeft, r.mTop );
	}
	if( mpEnemyFace )
	{
		char race = enemy->GetRace();
		char gender = enemy->GetGender();

		cUIRect r = mFaceRect[race * eGENDER_MAX + gender];
		mpEnemyFace->ChangeImage( r.mLeft, r.mTop );
	}

	/// hero buff setting
	{
		typedef tPointerHashMap<unsigned long, void*> cBuffMap;
		cBuffMap* buffMap = HERO->GetBufMap();
		if( buffMap )
		{
			if( buffMap->GetSize() > 0 )
			{
				cBuffMap::cIterator i = buffMap->Begin();
				cBuffMap::cIterator end = buffMap->End();
				for( ; i != end; ++i )
				{
					cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
					if( p && mpHeroBuff )
						mpHeroBuff->AddIcon( p->GetManagedKey(), p->GetInfluenceIdx() );
				}
			}
		}
		cBuffMap* debuffMap = HERO->GetDeBufMap();
		if( debuffMap )
		{
			if( debuffMap->GetSize() > 0 )
			{
				cBuffMap::cIterator i = debuffMap->Begin();
				cBuffMap::cIterator end = debuffMap->End();
				for( ; i != end; ++i )
				{
					cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
					if( p && mpHeroDeBuff )
						mpHeroDeBuff->AddIcon( p->GetManagedKey(), p->GetInfluenceIdx() );
				}
			}
		}
	}

	/// enemy buff setting
	{
		typedef tPointerHashMap<unsigned long, void*> cBuffMap;
		cBuffMap* buffMap = enemy->GetBufMap();
		if( buffMap )
		{
			if( buffMap->GetSize() > 0 )
			{
				cBuffMap::cIterator i = buffMap->Begin();
				cBuffMap::cIterator end = buffMap->End();
				for( ; i != end; ++i )
				{
					cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
					if( p && mpEnemyBuff )
						mpEnemyBuff->AddIcon( p->GetManagedKey(), p->GetInfluenceIdx() );
				}
			}
		}
		cBuffMap* debuffMap = enemy->GetDeBufMap();
		if( debuffMap )
		{
			if( debuffMap->GetSize() > 0 )
			{
				cBuffMap::cIterator i = debuffMap->Begin();
				cBuffMap::cIterator end = debuffMap->End();
				for( ; i != end; ++i )
				{
					cInfluenceObject* p = (cInfluenceObject*)(i->mSecond);
					if( p && mpEnemyDeBuff )
						mpEnemyDeBuff->AddIcon( p->GetManagedKey(), p->GetInfluenceIdx() );
				}
			}
		}
	}
}

bool cDuelWindow::CreateFace()
{
	mpHeroFace = new cLabel;
	if( mpHeroFace->CreateBySkinName( "Duel_Face_Hero", this, eUIID_GAME_DUEL_HERO_FACE ) == false )
		return false;

	mpEnemyFace = new cLabel;
	if( mpEnemyFace->CreateBySkinName( "Duel_Face_Enemy", this, eUIID_GAME_DUEL_ENEMY_FACE ) == false )
		return false;

	/// 얼굴 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* skin = 0;
	skin = pSkin->GetNodeSkin( "Duel_Face_Hero" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[0] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	skin = pSkin->GetNodeSkin( "Duel_Face_HW" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[1] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	skin = pSkin->GetNodeSkin( "Duel_Face_SM" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[2] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	skin = pSkin->GetNodeSkin( "Duel_Face_SW" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[3] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	skin = pSkin->GetNodeSkin( "Duel_Face_JM" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[4] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	skin = pSkin->GetNodeSkin( "Duel_Face_JW" );
	if( skin )
	{
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int w = skin->mSkinInfo->mWidth;
		unsigned int h = skin->mSkinInfo->mHeight;
		mFaceRect[5] = cUIRect(tx, ty, tx + w, ty + h);
	}
	else
	{
		assert(0);
	}

	return true;
}


bool cDuelWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	///
	mpHeroHP = (cDoubleGauge*)GetChild( eUIID_GAME_DUEL_HERO_HP );
	if( mpHeroHP )
		mpHeroHP->SetExceptEvent( true );

	mpHeroMP = (cDoubleGauge*)GetChild( eUIID_GAME_DUEL_HERO_MP );
	if( mpHeroMP )
		mpHeroMP->SetExceptEvent( true );

	mpHeroName = (cLabel*)GetChild( eUIID_GAME_DUEL_HERO_NAME );

	mpEnemyHP = (cDoubleGauge*)GetChild( eUIID_GAME_DUEL_ENEMY_HP );
	if( mpEnemyHP )
	{
		mpEnemyHP->SetExceptEvent( true );
		mpEnemyHP->SetRightDecreasing( true );
	}

	mpEnemyMP = (cDoubleGauge*)GetChild( eUIID_GAME_DUEL_ENEMY_MP );
	if( mpEnemyMP )
	{
		mpEnemyMP->SetExceptEvent( true );
		mpEnemyMP->SetRightDecreasing( true );
	}

	mpEnemyName = (cLabel*)GetChild( eUIID_GAME_DUEL_ENEMY_NAME );
	mpCountLabel = (cLabel*)GetChild( eUIID_GAME_DUEL_COUNT );
	mpFightLabel = (cLabel*)GetChild( eUIID_GAME_DUEL_FIGHTIMAGE );

	mpHeroBuff = new cDuelBuff;
	if( mpHeroBuff->CreateBySkinName( "DuelBuffWindow1", this, eUIID_GAME_DUELBUFFWINDOW1 ) == false )
		return false;

	mpHeroDeBuff = new cDuelBuff;
	if( mpHeroDeBuff->CreateBySkinName( "DuelDeBuffWindow1", this, eUIID_GAME_DUELDEBUFFWINDOW1 ) == false )
		return false;

	mpEnemyBuff = new cDuelBuff;
	if( mpEnemyBuff->CreateBySkinName( "DuelBuffWindow2", this, eUIID_GAME_DUELBUFFWINDOW2 ) == false )
		return false;


	mpEnemyDeBuff = new cDuelBuff;
	if( mpEnemyDeBuff->CreateBySkinName( "DuelDeBuffWindow2", this, eUIID_GAME_DUELDEBUFFWINDOW2 ) == false )
		return false;

	mpEnemyBuff->SetLeftAlign( false );
	mpEnemyDeBuff->SetLeftAlign( false );

	///
	if( CreateFace() == false )
		return false;

	SetExceptEvent( true );

	cUIWindow::Hide( false );

	return true;
}

void cDuelWindow::ChangeCountImage( unsigned int i )
{
	if( i > 5 )
		return;

	cUINodeSkin* p = mpCountLabel->GetNodeSkin();
	int tx = p->mSkinInfo->mTexX - (p->mSkinInfo->mTexWidth * ( 4 - i ));
	mpCountLabel->ChangeImage( tx, p->mSkinInfo->mTexY );
}

void cDuelWindow::Show( bool onsound )
{
	// 결투상태 아니면 안보이도록 한다
	if( DUELMAN->IsDuelEnable() == false )
		return;

	cUIWindow::Show( onsound );

	mpFightLabel->Hide();
	mpCountLabel->Show();

	cUINodeSkin* p = mpCountLabel->GetNodeSkin();
	mpCountLabel->ChangeImage( p->mSkinInfo->mTexX, p->mSkinInfo->mTexY );

	mShowProcess = eShow;
	SetRelativePos( cUIPos(mRelativeRect.mLeft, -mAbsoluteRect.GetHeight()) );

	if( mpHeroBuff )
	{
		mpHeroBuff->SetUpdateList( true );
		mpHeroBuff->Show( false );
	}
	if( mpHeroDeBuff )
	{
		mpHeroDeBuff->SetUpdateList( true );
		mpHeroDeBuff->Show( false );
	}
	if( mpEnemyBuff )
	{
		mpEnemyBuff->SetUpdateList( true );
		mpEnemyBuff->Show( false );
	}
	if( mpEnemyDeBuff )
	{
		mpEnemyDeBuff->SetUpdateList( true );
		mpEnemyDeBuff->Show( false );
	}
}

void cDuelWindow::ShowFight()
{
	mpCountLabel->Hide();
	mpFightLabel->Show();
}

void cDuelWindow::Open()
{
}

void cDuelWindow::Close()
{
	cUIWindow::Close();

	mShowProcess = eNone;

	if( mpHeroBuff )
		mpHeroBuff->Clear();
	if( mpHeroDeBuff )
		mpHeroDeBuff->Clear();
	if( mpEnemyBuff )
		mpEnemyBuff->Clear();
	if( mpEnemyDeBuff )
		mpEnemyDeBuff->Clear();
}

void cDuelWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	//
	PARTYMAN->HideUI();
	PARTYUNIONMAN->HideUI();
}

void cDuelWindow::OnHide()
{
	cUIWindow::OnHide();
	mShowProcess = eNone;

	if( mpHeroBuff )
		mpHeroBuff->Clear();
	if( mpHeroDeBuff )
		mpHeroDeBuff->Clear();
	if( mpEnemyBuff )
		mpEnemyBuff->Clear();
	if( mpEnemyDeBuff )
		mpEnemyDeBuff->Clear();

	//
	unsigned long partyIndex = HERO->GetPartyIndex();
	unsigned long partyUnonIndex = HERO->GetPartyUnionIndex();
	if( partyIndex > 0 )
	{
		PARTYMAN->ShowUI();
	}
	else if( partyUnonIndex > 0 )
	{
		PARTYUNIONMAN->ShowUI();
	}
}

void cDuelWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	if( mOutRangeTime )
	{
		if( mOutRangeTime <= deltaTime )
			mOutRangeTime = 0;
		else
		{
			mOutRangeTime -=deltaTime;

			unsigned int count = mOutRangeTime/1000;
			if( mOutCount != count )
			{
				mOutCount = count;

				TCHAR buf[256] = {0,};
				LPCTSTR str2 = GAMERESOURCEMAN->GetGameText( 6215 );
				::_stprintf_s( buf, str2, mOutCount );
				CHATMANAGER->AddSystemMsg(eSYSTEM_NORMAL, buf);
			}
		}
	}

	if( mShowProcess == eShow )
	{
		cUIPos pos;
		pos.mX = mAbsoluteRect.mLeft;

		int value = deltaTime*100/1000; /// 1s->100pix
		if( value < 5 )
			value = 5;

		pos.mY = mAbsoluteRect.mTop + value;

		if( pos.mY >= 20 )
		{
			pos.mY = 20;
			mShowProcess = eNone;
		}
		SetRelativePos( pos );
	}
	else if( mShowProcess == eHide )
	{
		cUIPos pos;
		pos.mX = mRelativeRect.mLeft;

		int value = deltaTime*100/1000; /// 1s->100pix
		if( value < 5 )
			value = 5;

		pos.mY = mRelativeRect.mTop - value;
		if( pos.mY <= -mAbsoluteRect.GetHeight() )
		{
			pos.mY = 0;
			Hide( false );
		}
		SetRelativePos( pos );
	}
}

void cDuelWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	if( mOutRangeTime )
	{
		int posy = mAbsoluteRect.mBottom + 80;
		int posx = 0;

		LPCTSTR str1 = GAMERESOURCEMAN->GetGameText( 6214 );
		long fw = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, str1, ::_tcslen(str1) );
		posx = (GetScreenWidth()-fw)/2;

		pKeeper->AddFontItem( cFontAgent::eFont_System, str1, posx, posy, COLOR_RED );//, true, eCOLOR_WHITE );

		TCHAR buf[256] = {0,};
		LPCTSTR str2 = GAMERESOURCEMAN->GetGameText( 6215 );
		::_stprintf_s( buf, str2, mOutRangeTime/1000 );
		fw = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, buf, ::_tcslen(buf) );
		posx = (GetScreenWidth()-fw)/2;
		posy += FONTAGENT->GetTextHeight( cFontAgent::eFont_System );

		pKeeper->AddFontItem( cFontAgent::eFont_System, buf, posx, posy, COLOR_RED );//, true, eCOLOR_WHITE );
	}

	cUIWindow::OnRender( pKeeper );
}


void cDuelWindow::UpdateHeroHP( unsigned long value, unsigned long max )
{
	if( mpHeroHP )
	{
		mpHeroHP->SetMaxValue( (float)max );
		mpHeroHP->SetGaugeAni( (float)value );
	}
}

void cDuelWindow::UpdateHeroMP( unsigned long value, unsigned long max )
{
	if( mpHeroMP )
	{
		mpHeroMP->SetMaxValue( (float)max );
		mpHeroMP->SetGaugeAni( (float)value );
	}
}

void cDuelWindow::UpdateEnemyHP( unsigned long value, unsigned long max )
{
	if( mpEnemyHP )
	{
		mpEnemyHP->SetMaxValue( (float)max );
		mpEnemyHP->SetGaugeAni( (float)value );
	}
}

void cDuelWindow::UpdateEnemyMP( unsigned long value, unsigned long max )
{
	if( mpEnemyMP )
	{
		mpEnemyMP->SetMaxValue( (float)max );
		mpEnemyMP->SetGaugeAni( (float)value );
	}
}

void cDuelWindow::UpdateRect()
{
	UpdateRectEx();

	/// 자식 갱신
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);
		if( n )
			n->UpdateRect();
	}

	if( IsVisible() == true )
	{
		cTargetWindow* win = GAMEUI->GetTargetWindow();
		if( win )
		{
			cUIPos pos = mAbsoluteRect.GetCenter();
			win->SetRelativePos( cUIPos( pos.mX - win->GetAbsoluteRect().GetWidth()/2, mAbsoluteRect.mBottom + 5 ) );
		}
	}
}

void cDuelWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

}

void cDuelWindow::UpdateRectEx()
{
	/// 화면 중앙에 출력
	/// 상대 영역좌표가 부모를 넘지 않게 설정한다 (이상한 값이 못오게 함)
	//unsigned int rWidth = mRelativeRect.GetWidth();
	//unsigned int rHeight = mRelativeRect.GetHeight();

	const cUIRect parentAbsolute = mpParent->GetAbsoluteRect();
	mAbsoluteRect = mRelativeRect + parentAbsolute.mUpperLeft;

	/// 절대 영역좌표가 전체 화면 을 넘지 않도록 설정
	//unsigned int abWidth = mAbsoluteRect.GetWidth();
	//unsigned int abHeight = mAbsoluteRect.GetHeight();

	/// 컨테이너인 경우에는
	if( mMove && IsKindof( eUINODE_CONTAINER ) )
	{
		/// 백그라운드용 컨테이너면 그냥 리턴 
		cUIContainer* pCon = (cUIContainer*)this;
		if( pCon->IsOnlyBackground() == true )
			return;

		if( this->mpParent == (cUIContainer*)GetRoot() )
			mAbsoluteRect = mRelativeRect;

		mMove = false;
	}

	/// 이미지 영역을 갱신
	if( mpImage )
		mpImage->SetScreenRect( mAbsoluteRect );
}

void cDuelWindow::ShowOutRangeText( bool show )
{
	if( show == false )
		mOutRangeTime = 0;
	else
		mOutRangeTime = 10000;

	mOutCount = 0;
}

void cDuelWindow::AddBuff( bool setEnemy, unsigned long uniqueIdx, unsigned long influenceIdx )
{
	if( setEnemy )
	{
		if( mpEnemyBuff )
			mpEnemyBuff->AddIcon( uniqueIdx, influenceIdx );
	}
	else
	{
		if( mpHeroBuff )
			mpHeroBuff->AddIcon( uniqueIdx, influenceIdx );
	}
}

void cDuelWindow::DelBuff( bool setEnemy, unsigned long uniqueIdx )
{
	if( setEnemy )
	{
		if( mpEnemyBuff )
			mpEnemyBuff->DelIcon( uniqueIdx );
	}
	else
	{
		if( mpHeroBuff )
			mpHeroBuff->DelIcon( uniqueIdx );
	}
}

void cDuelWindow::AddDeBuff( bool setEnemy, unsigned long uniqueIdx, unsigned long influenceIdx )
{
	if( setEnemy )
	{
		if( mpEnemyDeBuff )
			mpEnemyDeBuff->AddIcon( uniqueIdx, influenceIdx );
	}
	else
	{
		if( mpHeroDeBuff )
			mpHeroDeBuff->AddIcon( uniqueIdx, influenceIdx );
	}
}

void cDuelWindow::DelDeBuff( bool setEnemy, unsigned long uniqueIdx )
{
	if( setEnemy )
	{
		if( mpEnemyDeBuff )
			mpEnemyDeBuff->DelIcon( uniqueIdx );
	}
	else
	{
		if( mpHeroDeBuff )
			mpHeroDeBuff->DelIcon( uniqueIdx );
	}
}

void cDuelWindow::OnLButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{

	if( !mpEnemyFace || !mpEnemyName || !mpHeroFace || !mpHeroName )
	{
		assert( 0 );
		return;
	}

	if( mpEnemyFace->ContainPoint( pos ) || mpEnemyName->ContainPoint( pos ) )
	{
		unsigned long idx = DUELMAN->GetEnemyIdx();
		cPlayer* p = OBJECTMAN->GetPlayer( idx );
		if( p )
			HERO->SetTargetObject( eOBJECTTYPE_PLAYER, idx );
	}
	else if( mpHeroFace->ContainPoint( pos ) || mpHeroName->ContainPoint( pos ) )
	{
		HERO->SetTargetObject( eOBJECTTYPE_HERO, HERO->GetObjectID() );
	}
}

void cDuelWindow::NetLock( int lockTry )
{
	cUINode::NetLock( lockTry );

	mpHeroBuff->NetLock( lockTry );
	mpHeroDeBuff->NetLock( lockTry );
}

void cDuelWindow::NetUnLock( int lockTry )
{
	if( mNetLock == true )
	{
		cUINode::NetUnLock( lockTry );

		mpHeroBuff->NetUnLock( lockTry );
		mpHeroDeBuff->NetUnLock( lockTry );	
	}
}
