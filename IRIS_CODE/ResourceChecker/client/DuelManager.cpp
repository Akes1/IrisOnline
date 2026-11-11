#include "StdAfx.h"
#include ".\duelmanager.h"

// #include "GameUIManager.h"
// #include "DuelWindow.h"
// 
// #include "ObjectManager.h"
// #include "hero.h"
 
#include "WorldManager.h"
 
#include "SceneManager.h"
#include "NpcSceneNode.h"

cDuelManager* cDuelManager::mpDuelManager = NULL;

cDuelManager::cDuelManager()
: mpReferee(0)
{
	mpDuelManager = this;

	mDuelState = eDuel_None;
	mEnemyIndex = 0;

	mCenterPos = NiPoint2::ZERO;
	mCountChekTime = 0;
}

cDuelManager::~cDuelManager()
{
}

void cDuelManager::Open()
{
	mDuelState = eDuel_None;
	mEnemyIndex = 0;

	mCenterPos = NiPoint2::ZERO;
	mpReferee = 0;
}

void cDuelManager::Process( unsigned long deltaTime )
{
// 	if( mDuelState == eDuel_None )
// 		return;
// 
// 	/// hero & enemy hp setting
// 	UpdateHPMP();
// 
// 	if( mDuelState == eDuel_Counting )
// 	{
// 		if( mCountChekTime <= deltaTime )
// 		{
// 			mCountChekTime = 0;
// 			mDuelState = eDuel_None;
// 
// 			cDuelWindow* win = GAMEUI->GetDuelWindow();
// 			if( win )
// 				win->ShowFight();
// 		}
// 		else
// 		{
// 			mCountChekTime -= deltaTime;
// 
// 			/// counting print
// 			int countIdx = mCountChekTime/1000;
// 
// 			cDuelWindow* win = GAMEUI->GetDuelWindow();
// 			if( win )
// 				win->ChangeCountImage( countIdx );
// 		}
// 	}
}


void cDuelManager::StartCount( unsigned long enemyIdx, NiPoint2 centerPos )
{
	mCenterPos = centerPos;
	mDuelState = eDuel_Counting;

	mCountChekTime = 5000;
	mEnemyIndex = enemyIdx;

// 	/// duel window set & show
// 	cDuelWindow* win = GAMEUI->GetDuelWindow();
// 	if( win )
// 	{
// 		cPlayer* enemy = OBJECTMAN->GetPlayer( mEnemyIndex );
// 		if( enemy )
// 		{
// 			win->SetDuelInfo( enemy );
// 		}
// 		else
// 		{
// 			assert(0);
// 		}
// 
// 		win->Show();
// 
// 		// duel ½ÃÀÛ
// 	}
// 
 	float height;
 	if( WORLDMAN->CalcHeight( &height, mCenterPos.x, mCenterPos.y ) == true )
 	{
 		if( mpReferee )
 		{
 			if( SCENEMAN->DestroyDynamic( (cSceneNode*)mpReferee ) == false )
 			{
 				assert(0);
 			}
 
 			mpReferee = 0;
 		}

		cSceneNodeParam param;
		param.mPathName = "./Data/Npc/Eff_Duel.kfm";
		param.mTranslate = NiPoint3( mCenterPos.x, mCenterPos.y, height);

		mpReferee = SCENEMAN->CreateReferee( param );
		if( mpReferee )
		{
			mpReferee->SetTargetAnimation( 1 );


			NiPoint3 pos = mpReferee->GetTranslate();
			pos.y += 20.0f;
			mpReferee->SetTranslate( pos );

			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = pos;
			SCENEMAN->CreateSelfEffect( "./Data/Effect/eff_duel_01.nif", false, trans, false );
		}
	}

// 	GAMEUI->SetDuelMode( true );
// }
// 
// 
// void cDuelManager::StartDuel()
// {
// 	mDuelState = eDuel_Fighting;
// 
// 	/// set fight texture
// 	cDuelWindow* win = GAMEUI->GetDuelWindow();
// 	if( win )
// 		win->ShowFight();
// 
// 	if( mpReferee )
// 	{
// 		mpReferee->SetTargetAnimation( 3 );
// 	}
// }
// 
// void cDuelManager::EndDuel()
// {
// 	mDuelState = eDuel_None;
// 	mEnemyIndex = 0;
// 
// 	HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
// 	GAMEUI->SetDuelMode( false );
// 
// 	/// duel window hide
// 	cDuelWindow* win = GAMEUI->GetDuelWindow();
// 	if( win )
// 		win->EndDuel();
// 
// 	if( mpReferee )
// 	{
// 		if( SCENEMAN->DestroyDynamic( mpReferee ) == false )
// 		{
// 			assert(0);
// 		}
// 
// 		mpReferee = 0;
// 	}
//	
}
 
// bool cDuelManager::IsDuelEnemy( cPlayer* player )
// {
// 	if( player == 0 )
// 		return false;
// 	if( mEnemyIndex == 0 )
// 		return false;
// 	if( mDuelState != eDuel_Fighting )
// 		return false;
// 
// 	if( player->GetObjectID() == mEnemyIndex )
// 		return true;
// 
// 	return false;
// }
// 
// void cDuelManager::SetOutofBound( bool out )
// {
// 	cDuelWindow* win = GAMEUI->GetDuelWindow();
// 	if( win )
// 		win->ShowOutRangeText( out );
// }
// 
// void cDuelManager::UpdateHPMP()
// {
// 	//unsigned long maxHP = HERO->GetMaxHP();
// 	//unsigned long hp = HERO->GetHP();
// 	/// setting window
// 	cDuelWindow* win = GAMEUI->GetDuelWindow();
// 	if( win )
// 	{
// 		win->UpdateHeroHP( HERO->GetHP(), HERO->GetMaxHP() );
// 		win->UpdateHeroMP( HERO->GetMP(), HERO->GetMaxMP() );
// 	}
// 
// 	cPlayer* enemy = OBJECTMAN->GetPlayer( mEnemyIndex );
// 	if( enemy )
// 	{
// 		/// setting window
// 		if( win )
// 		{
// 			win->UpdateEnemyHP( enemy->GetHP(), enemy->GetMaxHP() );
// 			win->UpdateEnemyMP( enemy->GetMP(), enemy->GetMaxMP() );
// 		}
// 	}
// }
