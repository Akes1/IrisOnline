#include "stdafx.h"
#include "ItemMixManager.h"
#include "Protocol.h"
#include "MakeSkill_Common.h"
#include "Application.h"

#include "GameUIManager.h"
#include "ItemMixWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
//
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "MakeSkillScript.h"
#include "ItemManager.h"

cItemMixManager* cItemMixManager::mpSingleton = 0;

cItemMixManager::cItemMixManager()
: mNetLock( false )
, mLockTry( eLOCK_ITEMMIXNONE )
, mTempIndex(0)
, mTempRecipeIdx(0)
{
	mpSingleton = this;
}

cItemMixManager::~cItemMixManager()
{
	DeleteKeepSkill1();
	DeleteKeepSkill2();
}

void cItemMixManager::Close()
{
	/// UI 클리어
	cItemMixWindow* win = GAMEUI->GetItemMixWindow();
	if( win )
		win->Clear();

	DeleteKeepSkill1();
	DeleteKeepSkill2();

	mNetLock = false;
	mLockTry = eLOCK_ITEMMIXNONE;
	mTempIndex = 0;
	mTempRecipeIdx = 0;
}

bool cItemMixManager::AddKeepSkill1( unsigned long recipeIdx, unsigned long restTime )
{
	sMakeSkillScript* pMakeSkill = (sMakeSkillScript*)MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( !pMakeSkill )
	{
		assert(0);
		return false;
	}

	cStringT th,tm,ts;
	short coolTime = (short)((pMakeSkill->mNextMakeTime) / 1000);

	/// 재사용시간 구해서 출력
	unsigned int h = (((unsigned int)coolTime ) / 60) / 60;
	unsigned int m = (((unsigned int)coolTime ) / 60) % 60;
	unsigned int s = ((unsigned int)coolTime % 60) % 60;

	if( h != 0 )
		th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
	if( m != 0 )
		tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
	if( s != 0 )
		ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );

	sKeepRecipe* info = new sKeepRecipe;
	
	info->mStartTime	= THEAPP->GetWorldAccumTime();
	info->mRestTime		= restTime;
	info->mNextMakeTime = pMakeSkill->mNextMakeTime;
	info->mStrNextMakeTime += th;
	info->mStrNextMakeTime += tm;
	info->mStrNextMakeTime += ts;	

	if( mKeepSkillMap1.Insert( recipeIdx, info ) == false )
	{
		assert(0);
		return false;
	}
	return true;
}

bool cItemMixManager::AddKeepSkill2( unsigned long recipeIdx, unsigned long restTime )
{
	sMakeSkillScript* pMakeSkill = (sMakeSkillScript*)MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( !pMakeSkill )
	{
		assert(0);
		return false;
	}

	cStringT th,tm,ts;
	short coolTime = (short)((pMakeSkill->mNextMakeTime) / 1000);

	/// 재사용시간 구해서 출력
	unsigned int h = (((unsigned int)coolTime ) / 60) / 60;
	unsigned int m = (((unsigned int)coolTime ) / 60) % 60;
	unsigned int s = ((unsigned int)coolTime % 60) % 60;

	if( h != 0 )
		th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
	if( m != 0 )
		tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
	if( s != 0 )
		ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );

	sKeepRecipe* info = new sKeepRecipe;

	info->mStartTime	= THEAPP->GetWorldAccumTime();
	info->mRestTime		= restTime;
	info->mNextMakeTime = pMakeSkill->mNextMakeTime;
	info->mStrNextMakeTime += th;
	info->mStrNextMakeTime += tm;
	info->mStrNextMakeTime += ts;	

	if( mKeepSkillMap2.Insert( recipeIdx, info ) == false )
	{
		assert(0);
		return false;
	}
	return true;
}

void cItemMixManager::DeleteKeepSkill1()
{
	cKeepRecipeMap::cIterator i = mKeepSkillMap1.Begin();
	cKeepRecipeMap::cIterator end = mKeepSkillMap1.End();
	for( ; i != end; ++i )
	{
		sKeepRecipe* info = (sKeepRecipe*)(i->mSecond);
		SAFE_DELETE( info );
	}
	mKeepSkillMap1.Clear();
}

void cItemMixManager::DeleteKeepSkill2()
{
	cKeepRecipeMap::cIterator i = mKeepSkillMap2.Begin();
	cKeepRecipeMap::cIterator end = mKeepSkillMap2.End();
	for( ; i != end; ++i )
	{
		sKeepRecipe* info = (sKeepRecipe*)(i->mSecond);
		SAFE_DELETE( info );
	}
	mKeepSkillMap2.Clear();
}

sKeepRecipe* cItemMixManager::GetKeepMixSkill1( unsigned long recipeIdx )
{
	return (sKeepRecipe*)mKeepSkillMap1.GetAt( recipeIdx );
}

sKeepRecipe* cItemMixManager::GetKeepMixSkill2( unsigned long recipeIdx )
{
	return (sKeepRecipe*)mKeepSkillMap2.GetAt( recipeIdx );
}

unsigned long cItemMixManager::GetKeepSkillText( unsigned char index )
{
	unsigned long textIdx = 0;

	switch( index )
	{
	case eMAKESKILL_COOK:		textIdx = 130; break;
	case eMAKESKILL_PAINTCARD:	textIdx = 131; break;
	case eMAKESKILL_AlCHEMY:	textIdx = 132; break;
	default: 
		assert(0);			
		return 0;
	}
	return textIdx;
}

unsigned long cItemMixManager::GetTypeText( int index, unsigned long type )
{
	unsigned long textIdx = 0;
	switch( index )
	{
	case eMAKESKILL_COOK:
		{
			switch( type )
			{
			case 1:	textIdx = 2001001; break;	//포만감
			case 2: textIdx = 2001002; break;	//만족감
			case 3: textIdx = 2001003; break;	//감칠맛
			case 4: textIdx = 2001004; break;	//회복
			case 5: textIdx = 2001005; break;	//기타
			default:
				assert(0);
				break;
			}
		}
		break;
	case eMAKESKILL_PAINTCARD:
		{
			switch( type )
			{
			case 1: textIdx = 2001011; break;	//아이템 장착
			case 2: textIdx = 2001012; break;	//캐릭터 착용
			case 3: textIdx = 2001013; break;	//재료
			case 4: textIdx = 2001014; break;	//기타
			case 5: textIdx = 2001015; break;	// 변신 카드그리기
			default:
				assert(0);
				break;
			}
		}
		break;
	case eMAKESKILL_AlCHEMY:
		{
			switch( type )
			{
			case 1: textIdx = 2001021; break;	//변환
			case 2: textIdx = 2001022; break;	//물약
			case 3: textIdx = 2001023; break;	//촉진제
			case 4: textIdx = 2001024; break;	//강화제
			case 5: textIdx = 2001025; break;	//영약
			default:
				assert(0);
				break;
			}
		}
		break;
	}
	return textIdx;
}


bool cItemMixManager::RegistSkill( unsigned short slot, char type, char subType, unsigned long makeSkillIdx )
{
	cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
	if( !inven )
	{
		assert(0);
		return false;
	}

	if( makeSkillIdx == 0 )
	{
		assert(0);
		return false;
	}

	/// HERO의 전문스킬 슬롯 검사
	sHeroInfo* info = HERO->GetHeroInfo();
	
	// 동일 전문스킬이 있는지 검사
	if( info->MakeSkill1.mIndex == makeSkillIdx )
	{
		// 이미 습득한 직업 증서
		if( info->MakeSkill1.mStep + 1 > subType )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2833 ),
								GAMERESOURCEMAN->GetGameText( 301 ),
								0, inven );
			return false;
		}
		else if( info->MakeSkill1.mStep + 1 < subType )
		{
			// 해당 등급의 다음 단계여야 한다
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 2803 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, inven );
			return false;
		}
	}
	else if( info->MakeSkill2.mIndex == makeSkillIdx )
	{
		// 이미 습득한 직업 증서
		if( info->MakeSkill2.mStep + 1 > subType )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 2833 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, inven );
			return false;
		}
		else if( info->MakeSkill2.mStep + 1 < subType )
		{
			// 해당 등급의 다음 단계여야 한다
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 2803 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, inven );
			return false;
		}
	}
	else
	{
		// 빈슬롯 검사 
		if( (info->MakeSkill1.mIndex > 0 && info->MakeSkill2.mIndex > 0) ||
			(info->MakeSkill1.mIndex > 0 && HERO->GetMakeSkillEnd() == 1) ) 
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2802 ),
								GAMERESOURCEMAN->GetGameText( 301 ),
								0, inven );
			return false;
		}

		// 1단계 검사
		if( subType != ITEM_MIX_SKILL_L1 )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2828 ),
								GAMERESOURCEMAN->GetGameText( 301 ),
								0, inven );
			return false;
		}
	}

	/// 조합기술 아이템
	MSG_REQ_MAKESKILL_ADD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_ADD_REQ;
	msg.mSlotNum = slot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

bool cItemMixManager::RegistRecipe( unsigned short slot, unsigned long recipeIdx )
{
	cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
	if( !inven )
	{
		assert(0);
		return false;
	}

	/// 동일한 레시피 있는지 검사
	if( GetKeepMixSkill1( recipeIdx ) || GetKeepMixSkill2( recipeIdx ) )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 2804 ),
							GAMERESOURCEMAN->GetGameText( 301 ), 
							0, inven );
		return false;
	}

	sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( !script )
	{
		assert(0);
		return false;
	}

	cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( recipeIdx );
	if( pItemDefine == NULL )
	{
		assert(0);
		return false;
	}

	cItemLimit* pLimit = pItemDefine->GetLimit();
	if( pLimit == NULL )
	{
		assert(0);
		return false;
	}

	sHeroInfo* info = HERO->GetHeroInfo();

	// 해당 전문직업 익혔는지 검사
	if( info->MakeSkill1.mIndex == script->mMakeSkill )
	{
		// 요구 단계 비교
		if( info->MakeSkill1.mStep < script->mMakeStep )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2830 ),
								GAMERESOURCEMAN->GetGameText( 301 ), 
								0, inven );
			return false;
		}

		// 최소 숙련도 비교
		if( info->MakeSkill1.mExp < pLimit->GetMakeSkillExp() )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2829 ),
								GAMERESOURCEMAN->GetGameText( 301 ), 
								0, inven );
			return false;
		}
	}
	else if( info->MakeSkill2.mIndex == script->mMakeSkill )
	{
		// 요구 단계 비교
		if( info->MakeSkill2.mStep < script->mMakeStep )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2830 ),
								GAMERESOURCEMAN->GetGameText( 301 ), 
								0, inven );
			return false;
		}

		// 최소 숙련도 비교
		if( info->MakeSkill2.mExp < pLimit->GetMakeSkillExp() )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 2829 ),
								GAMERESOURCEMAN->GetGameText( 301 ), 
								0, inven );
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}

	/// 레시피 아이템
	MSG_REQ_MAKESKILL_RECIPE_ADD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_RECIPE_ADD_REQ;
	msg.mSlotNum = slot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

bool cItemMixManager::RegistRecipeGroup( unsigned short slot, unsigned long recipeGroupIdx )
{
	cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
	if( !inven )
	{
		assert(0);
		return false;
	}

	sHeroInfo* info = HERO->GetHeroInfo();
	sMakeSkill* skill1 = &info->MakeSkill1;
	sMakeSkill* skill2 = &info->MakeSkill2;

	sGroupInfo* groupInfo = MAKESKILLSCRIPT->GetGroupInfo( recipeGroupIdx );
	if( groupInfo == NULL )
	{
		assert(0);
		return false;
	}

	typedef tArray<void*> cRecipeArr;
	cRecipeArr* pRecipeAry = &groupInfo->mRecipeArr;

	bool isAddRecipe = false;
	bool isKeepMakeSkill = false;
	for( unsigned long i = 0 ; i < pRecipeAry->GetSize() ; ++i )
	{
		sRecipeGroupInfo* pGroupInfo = (sRecipeGroupInfo*)(*pRecipeAry)[i];
		if( pGroupInfo == NULL )
		{
			assert(NULL);
			return false;
		}

		sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( pGroupInfo->mRecipeIdx );
		if( script == NULL )
		{
			assert(NULL);
			return false;
		}

		cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( pGroupInfo->mRecipeIdx );
		if( pItemDefine == NULL )
		{
			assert(0);
			return false;
		}

		cItemLimit* pLimit = pItemDefine->GetLimit();
		if( pLimit == NULL )
		{
			assert(0);
			return false;
		}

		// 레시피랑 관련된 전문스킬 익혔는지 검사
		// 최소 요구 숙련도보다 낮으면 익힐수 없다
		if( skill1->mIndex == script->mMakeSkill )
		{
			// 요구 단계 비교
			if( skill1->mStep < script->mMakeStep )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 2830 ),
									GAMERESOURCEMAN->GetGameText( 301 ), 
									0, inven );
				return false;
			}

			// 최소 숙련도 비교
			if( skill1->mExp < pLimit->GetMakeSkillExp() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 2829 ),
									GAMERESOURCEMAN->GetGameText( 301 ), 
									0, inven );
				return false;
			}
		}
		else if( skill2->mIndex == script->mMakeSkill )
		{
			// 요구 단계 비교
			if( skill2->mStep < script->mMakeStep )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 2830 ),
									GAMERESOURCEMAN->GetGameText( 301 ), 
									0, inven );
				return false;
			}
			// 최소 숙련도 비교
			if( skill1->mExp < pLimit->GetMakeSkillExp() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 2829 ),
									GAMERESOURCEMAN->GetGameText( 301 ), 
									0, inven );
				return false;
			}
		}
		else
		{
			return false;
		}

		isKeepMakeSkill = true;

		/// 새로 배워야 할 레시피인지 검사
		if( !(GetKeepMixSkill1( pGroupInfo->mRecipeIdx ) || GetKeepMixSkill2( pGroupInfo->mRecipeIdx )) )
			isAddRecipe = true;
	}

	if( isKeepMakeSkill == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2819 ) );
		return false;
	}

	if( isAddRecipe == false )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 2804 ),
							GAMERESOURCEMAN->GetGameText( 301 ),
							0, inven );
		return false;
	}

	/// 레시피 아이템
	MSG_REQ_MAKESKILL_RECIPE_ADD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_RECIPE_ADD_REQ;
	msg.mSlotNum = slot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

bool cItemMixManager::ItemMixStart( unsigned char index, unsigned long recipeIdx, int count )
{
	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE || state == ePLAYER_STATE_SITDOWN ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	if( HERO->IsJumping() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}	
	
	if( IsNetLock() )
		return false;

	if( index == 0 || recipeIdx == 0 || count == 0 )
		return false;

	sKeepRecipe* recipeCool = 0;
	sHeroInfo* info = HERO->GetHeroInfo();
	if( info )
	{
		sMakeSkill* skill1 = &info->MakeSkill1;
		sMakeSkill* skill2 = &info->MakeSkill2;

		if( skill1->mIndex == index )
		{
			recipeCool = GetKeepMixSkill1( recipeIdx );
		}
		else if( skill2->mIndex == index )
		{
			recipeCool = GetKeepMixSkill2( recipeIdx );
		}
		else
		{
			assert(0);
		}
	}

	/// 조합 대기 시간 검사	
	if( recipeCool && recipeCool->mRestTime > 0 )
	{
		unsigned long elapsedTime = THEAPP->GetWorldAccumTime() - recipeCool->mStartTime;
		if( recipeCool->mRestTime > elapsedTime )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2814 ) );
			return false;
		}
	}

	/// 탈것 해제
	if( HERO->IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->AlightVehicle();
	}

	/// 따라가기 종료
	if( HERO->IsFollowing() )
		HERO->ReleaseFollowPlayer();

	MSG_REQ_MAKESKILL_ITEMMIX_START msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_ITEMMIX_START_REQ;
	msg.mMakeSkill  = index;
	msg.mRecipeIdx  = recipeIdx;
	msg.mMakeCnt	= (unsigned short)count;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	NetLock( eLOCK_ITEMMIXSTART );

	/// 상태
	HERO->SetStopFlag( eSTOP_ITEMMIX );

	mTempIndex = index;
	mTempRecipeIdx = recipeIdx;
	return true;
}

void cItemMixManager::ItemMixDelete( unsigned char index )
{
	if( IsNetLock() )
		return;

	if( index == 0 )
		return;

	MSG_REQ_MAKESKILL_DELETE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_MAKESKILL;
	msg.Protocol = NM_MAKESKILL_DEL_REQ;
	msg.mMakeSkill = index;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	NetLock( eLOCK_ITEMMIXDELETE );
}

void cItemMixManager::ItemMixOpen()
{
	if( IsNetLock() )
		return;

	cItemMixWindow* win = GAMEUI->GetItemMixWindow();
	if( !win )
	{
		assert(0);
		return;
	}

	if( win->IsVisible() )
		return;

	/// 보유한 보조기술이 없을시
	sHeroInfo* info = HERO->GetHeroInfo();
	if( info )
	{
		sMakeSkill* skill1 = &info->MakeSkill1;
		sMakeSkill* skill2 = &info->MakeSkill2;

		if( skill1->mIndex == 0 && skill2->mIndex == 0 )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 2821 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, win );

			return;
		}
	}

	/// 
	win->Show();
}

bool cItemMixManager::ItemMixCancel( bool hide )
{
	/// [조합중]일때는 서버에 요청
	if( HERO->GetStopFlag() == eSTOP_ITEMMIX )
	{
		if( IsNetLock() )
			return false;

		MSGROOT msg;
		::memset( &msg, 0, sizeof( msg ) );
		msg.Category = NM_MAKESKILL;
		msg.Protocol = NM_MAKESKILL_ITEMMIX_CANCEL_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->SetStopFlag( eSTOP_NONE );
	}

	cItemMixWindow* win = GAMEUI->GetItemMixWindow();
	if( win )
	{
		win->StopGauge();

		if( hide == true )
			win->Hide();
	}

	mNetLock = false;
	mTempIndex = 0;
	mTempRecipeIdx = 0;

	HERO->ClearItemMixEffect();

	return true;
}

bool cItemMixManager::IsKeepRecipe( unsigned long recipeIdx )
{
	if( recipeIdx > 0 )
	{
		if( GetKeepMixSkill1( recipeIdx ) != NULL )
			return true;

		if( GetKeepMixSkill2( recipeIdx ) != NULL )
			return true;
	}
	return false;
}

bool cItemMixManager::IsKeepMixSkill(  unsigned long makeSkillIdx, unsigned char subType )
{
	if( makeSkillIdx == 0 )
		return false;

	/// HERO의 전문스킬 슬롯 검사
	sHeroInfo* info = HERO->GetHeroInfo();

	// 동일 전문스킬이 있는지 검사
	if( info->MakeSkill1.mIndex == makeSkillIdx && info->MakeSkill1.mIndex != 0 )
	{
		// 이미 습득한 직업 증서
		if( info->MakeSkill1.mStep + 1 > subType )
			return true;
	}
	else if( info->MakeSkill2.mIndex == makeSkillIdx && info->MakeSkill2.mIndex != 0 )
	{
		// 이미 습득한 직업 증서
		if( info->MakeSkill2.mStep + 1 > subType )
			return true;
	}

	return false;
}

void cItemMixManager::NetworkMsgParserLoad( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_MAKESKILL_LIST1_RES:
		{
			/// 요리 레시피 목록
			MSG_RES_MAKESKILL_LIST* pmsg = (MSG_RES_MAKESKILL_LIST*)msg;
			
			sRecipeInfo* recipeInfo = pmsg->mRecipeInfo;
			for( int i = 0; i < pmsg->mCount; ++i, ++recipeInfo )
			{
				AddKeepSkill1( recipeInfo->mRecipeIdx, recipeInfo->mCoolTime );
			}
		}
		break;
	case NM_MAKESKILL_LIST2_RES:
		{
			/// 카드그리기 레시피 목록
			MSG_RES_MAKESKILL_LIST* pmsg = (MSG_RES_MAKESKILL_LIST*)msg;

			sRecipeInfo* recipeInfo = pmsg->mRecipeInfo;
			for( int i = 0; i < pmsg->mCount; ++i, ++recipeInfo )
			{
				AddKeepSkill2( recipeInfo->mRecipeIdx, recipeInfo->mCoolTime );
			}
		}
		break;
	}
}

void cItemMixManager::NetworkMsgParserGame( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	cStringT str;
	switch( tempMsg->Protocol )
	{
	case NM_MAKESKILL_ADD_RES:
		{
			/// 전문 직업 습득
			MSG_RES_MAKESKILL_ADD* pmsg = (MSG_RES_MAKESKILL_ADD*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_MAKESKILL_ADD_SUCCESS:
				{
					/// HERO에 저장
					sHeroInfo* info = HERO->GetHeroInfo();
					sMakeSkill* skill1 = &info->MakeSkill1;
					sMakeSkill* skill2 = &info->MakeSkill2;

					if( skill1->mIndex == pmsg->mMakeSkill )
					{
						HERO->SetMixSkillFirst( pmsg->mMakeSkill, pmsg->mMakeSkillStep );
					}
					else if( skill2->mIndex == pmsg->mMakeSkill )
					{
						HERO->SetMixSkillSecond( pmsg->mMakeSkill, pmsg->mMakeSkillStep );
					}
					else
					{
						if( skill1->mIndex == 0 )
						{
							HERO->SetMixSkillFirst( pmsg->mMakeSkill, pmsg->mMakeSkillStep );
						}
						else if( skill2->mIndex == 0 )
						{
							HERO->SetMixSkillSecond( pmsg->mMakeSkill, pmsg->mMakeSkillStep );
						}
					}

					/// UI 에 추가
					cItemMixWindow* win = GAMEUI->GetItemMixWindow();
					if( win )
					{
						win->UpdateMakeSkill();
						win->Show();
					}

					unsigned long textIndex = GetKeepSkillText( pmsg->mMakeSkill );
					str.Format( GAMERESOURCEMAN->GetGameText( 2801 ), UIMAN->GetUIText( textIndex ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

					///
					ITEMMAN->CheckItemLimitAll();

					ITEMMAN->CheckEquipAbility();
				}
				break;
			case ERROR_MAKESKILL_ADD_DBUSED:
			case ERROR_MAKESKILL_ADD_DBERROR:
			case ERROR_MAKESKILL_ADD_STATE:
			case ERROR_MAKESKILL_ADD_ITEM:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2806 ) );
				break;
			case ERROR_MAKESKILL_ADD_NOTEMPTY:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2802 ) );
				break;
			case ERROR_MAKESKILL_ADD_ALREADY:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2800 ) );
				break;
			case ERROR_MAKESKILL_ADD_STEP:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2803 ) );
				break;
            case ERROR_MAKESKILL_ADD_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
                break;
			}

			ITEMMAN->UnlockItemTry( ITEM_TRY_MIXSKILL );
		}
		break;
	case NM_MAKESKILL_RECIPE_ADD_RES:
		{
			/// 레시피 습득
			MSG_RES_MAKESKILL_RECIPE_ADD* pmsg = (MSG_RES_MAKESKILL_RECIPE_ADD*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_MAKESKILL_RECIPE_ADD_SUCCESS:
				{
					sHeroInfo* info = HERO->GetHeroInfo();

					sMakeSkill* skill1 = &info->MakeSkill1;
					sMakeSkill* skill2 = &info->MakeSkill2;

					for( unsigned short i = 0 ; i < pmsg->mCount ; ++i )
					{
						sRecipeGroupInfo* pGroupInfo = &pmsg->mGroupInfo[i];

						if( skill1->mIndex == pGroupInfo->mMakeSkill )
						{
							AddKeepSkill1( pGroupInfo->mRecipeIdx, 0 );
						}
						else if( skill2->mIndex == pGroupInfo->mMakeSkill )
						{
							AddKeepSkill2( pGroupInfo->mRecipeIdx, 0 );
						}
						else
						{
							assert(0);
							continue;
						}

						sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( pGroupInfo->mRecipeIdx );
						if( script )
						{
							unsigned long textIndex = GetTypeText( pGroupInfo->mMakeSkill, script->mType );
							str.Format( GAMERESOURCEMAN->GetGameText( 2805 ), GAMERESOURCEMAN->GetItemToolTipText( textIndex ), ITEMMAN->GetItemName( pGroupInfo->mRecipeIdx ) );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
						}
					}

					// 마지막으로 익힌 레시피로 세팅
					if( pmsg->mCount > 0 )
					{
						cItemMixWindow* win = GAMEUI->GetItemMixWindow();
						if( win )
						{
							unsigned char skillIndex = win->GetSelectSkill();		
							if( skillIndex )
							{
								win->UpdateRecipeList( skillIndex );
							}

							win->Show();
						}
					}
					else
					{
						assert(0);
					}
				}
				break;
			case ERROR_MAKESKILL_RECIPE_ADD_DBUSED:
			case ERROR_MAKESKILL_RECIPE_ADD_DBERROR:
			case ERROR_MAKESKILL_RECIPE_ADD_STATE:
			case ERROR_MAKESKILL_RECIPE_ADD_ITEM:
			case ERROR_MAKESKILL_RECIPE_ADD_SCRIPT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2818 ) );
				break;
			case ERROR_MAKESKILL_RECIPE_ADD_ALREADY:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2804 ) );
				break;
			case ERROR_MAKESKILL_RECIPE_ADD_NOTHAVE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2819 ) );
				break;
			case ERROR_MAKESKILL_RECIPE_ADD_STEP:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2830 ) );
				break;
			case ERROR_MAKESKILL_RECIPE_ADD_EXP:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2829 ) );
				break;
            case ERROR_MAKESKILL_RECIPE_ADD_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
				break;
			}

			ITEMMAN->UnlockItemTry( ITEM_TRY_MIXSKILL );
		}
		break;
	case NM_MAKESKILL_DEL_RES:
		{
			/// 전문스킬 삭제
			MSG_RES_MAKESKILL_DELETE* pmsg = (MSG_RES_MAKESKILL_DELETE*)msg;
		
			/// 성공
			if( pmsg->ErrorCode == ERROR_MAKESKILL_DEL_SUCCESS )
			{
				sHeroInfo* info = HERO->GetHeroInfo();
				if( info )
				{
					sMakeSkill* skill1 = &info->MakeSkill1;
					sMakeSkill* skill2 = &info->MakeSkill2;

					if( skill1->mIndex == pmsg->mMakeSkill )
					{
						HERO->SetMixSkillFirst( 0, 0 );
						DeleteKeepSkill1();
					}
					else if( skill2->mIndex == pmsg->mMakeSkill )
					{
						HERO->SetMixSkillSecond( 0, 0 );
						DeleteKeepSkill2();
					}
					else
					{
						assert(0);
					}
				}

				/// UI 갱신 
				cItemMixWindow* mixWin = GAMEUI->GetItemMixWindow();
				if( mixWin )
				{	
					mixWin->Clear();
					mixWin->UpdateMakeSkill();
				}
				
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2807 ) );

				// 
				ITEMMAN->CheckItemLimitAll();

				ITEMMAN->CheckEquipAbility();
			}
			else
			{
				/// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2808 ) );
			}

			NetUnLock( eLOCK_ITEMMIXDELETE );
		}
		break;
	case NM_MAKESKILL_ITEMMIX_START_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_MAKESKILL_START_SCRIPT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2813 ) );
				break;
			case ERROR_MAKESKILL_START_COOLTIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2814 ) );
				break;
			case ERROR_MAKESKILL_START_HAVESKILL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2813 ) );
				break;
			case ERROR_MAKESKILL_START_STATE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2813 ) );
				break;
			case ERROR_MAKESKILL_START_DBERROR:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2813 ) );
				break;
			case ERROR_MAKESKILL_START_COUNT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2809 ) );
				break;
			case ERROR_MAKESKILL_START_STEP:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2832 ) );
				break;
			case ERROR_MAKESKILL_START_EXP:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2831 ) );
				break;
			}

			if( pmsg->ErrorCode != ERROR_MAKESKILL_START_SUCCESS )
			{
				cItemMixWindow* win = GAMEUI->GetItemMixWindow();
				if( win )
					win->StopGauge();

				HERO->SetStopFlag( eSTOP_NONE );
			}

			NetUnLock( eLOCK_ITEMMIXSTART );
		}
		break;
	case NM_MAKESKILL_ITEMMIX_START_SYN:
		{
			MSG_SYN_MAKESKILL_ITEMMIX_START* pmsg = (MSG_SYN_MAKESKILL_ITEMMIX_START*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->mCharacterIdx );
			if( p != NULL )
			{
				p->SetStopFlag( eSTOP_ITEMMIX );
			}
		}
		break;
	case NM_MAKESKILL_ITEMMIX_MIDDLEEND_SYN:
		{
			MSG_SYN_MAKESKILL_ITEMMIX_MIDDLEEND* synMsg = (MSG_SYN_MAKESKILL_ITEMMIX_MIDDLEEND*)msg;
			cPlayer* p = OBJECTMAN->GetPlayer( synMsg->mCharacterIdx );
			if( p != NULL )
			{
				/// 이펙트 처리.
				NiTransform trans;
				trans.MakeIdentity();
				trans.m_Translate = NiPoint3( 0.0f, -40.0f, 0.0f );
				trans.m_fScale = 0.5f;
 				p->SetLinkdEffect( eLINK_BODY, "./Data/Effect/eff_pc_mix_02.nif", &trans, false, true );
 				p->SetLinkdSound( 2021 );

				///  조합 게이지 처리
				if( p == HERO )
				{
					cItemMixWindow* win = GAMEUI->GetItemMixWindow();
					if( win )
						win->NetGaugeEnd();
				}
			}
		}
		break;
	case NM_MAKESKILL_ITEMMIX_EXP_SYN:
		{
			/// 숙련도 
			MSG_SYN_MAKESKILL_ITEMMIX_EXP* synMsg = (MSG_SYN_MAKESKILL_ITEMMIX_EXP*)msg;

			HERO->UpdateMixSkillExp( synMsg->mMakeSkill, synMsg->mMakeSkillExp );

			// UI 갱신
			cItemMixWindow* win = GAMEUI->GetItemMixWindow();
			if( win )
				win->SetMixExp( synMsg->mMakeSkill, synMsg->mMakeSkillExp );

			ITEMMAN->CheckItemLimitAll();

			ITEMMAN->CheckEquipAbility();
		}
		break;
	case NM_MAKESKILL_ITEMMIX_END_SYN:
		{
			MSG_SYN_MAKESKILL_ITEMMIX_END* pmsg = (MSG_SYN_MAKESKILL_ITEMMIX_END*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->mCharacterIdx );
			if( p != NULL )
			{
				/// 이펙트 처리.
				if( pmsg->ErrorCode == ERROR_MAKESKILL_END_SUCCESS )
				{
					NiTransform trans;
					trans.MakeIdentity();
					trans.m_fScale = 0.5f;
					p->SetLinkdEffect( eLINK_LHAND, "./Data/Effect/eff_pc_mix_02.nif", &trans, false, false );
					p->SetLinkdSound( 2021 );
				}

				/// 조합 게이지 처리
				if( p == HERO )
				{
					HERO->ClearItemMixEffect();

					cItemMixWindow* win = GAMEUI->GetItemMixWindow();
					if( win == NULL )
					{
						assert(0);
						return;
					}

					if( pmsg->ErrorCode == ERROR_MAKESKILL_END_SUCCESS )
					{	
						win->NetGaugeEnd();

						// 재사용 시간 셋팅
						sKeepRecipe* recipeCool = 0;
						sHeroInfo* info = HERO->GetHeroInfo();
						if( info )
						{
							sMakeSkill* skill1 = &info->MakeSkill1;
							sMakeSkill* skill2 = &info->MakeSkill2;

							if( skill1->mIndex == mTempIndex )
							{
								recipeCool = GetKeepMixSkill1( mTempRecipeIdx );
							}
							else if( skill2->mIndex == mTempIndex )
							{
								recipeCool = GetKeepMixSkill2( mTempRecipeIdx );
							}
							else
								assert(0);

							sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( mTempRecipeIdx );
							if( recipeCool && script )
							{
								recipeCool->mStartTime = THEAPP->GetWorldAccumTime();
								recipeCool->mRestTime = script->mNextMakeTime;
							}
							else
								assert(0);
						}
					}
					else
					{
						win->NetGaugeEnd( false );

						switch( pmsg->ErrorCode )
						{
						case ERROR_MAKESKILL_END_ITEM:
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2817 ) );
							break;
						case ERROR_MAKESKILL_END_STATE:
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2816 ) );
							break;
						case ERROR_MAKESKILL_END_TIME:
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2814 ) );
							break;
						case ERROR_MAKESKILL_END_MAX_ITEM:
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2825 ) );
							break;
						}
					}

					mTempIndex = 0;
					mTempRecipeIdx = 0;
				}

				p->SetStopFlag( eSTOP_NONE );
			}
		}
		break;
	case NM_MAKESKILL_ITEMMIX_CANCEL_RES:
		{
			MSG_ERROR*	pmsg = (MSG_ERROR*)msg;

			if( pmsg->ErrorCode == ERROR_MAKESKILL_CANCEL_DBWORK )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2822 ) );
				ITEMMAN->LockItemTry( ITEM_TRY_ITEMMIXDBWORK );
			}
		}
		break;
	case NM_MAKESKILL_ITEMMIX_CANCEL_SYN:
		{
			MSG_SYN_MAKESKILL_ITEMMIX_CANCEL* pmsg = (MSG_SYN_MAKESKILL_ITEMMIX_CANCEL*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( pmsg->mCharacterIdx );
			if( p && p->GetStopFlag() == eSTOP_ITEMMIX )
			{
				p->SetStopFlag( eSTOP_NONE );
			}
		}
		break;
	case NM_MAKESKILL_ITEMMIX_CANCELFORCE_SYN:
		{
			// 서버로부터 강제 취소
			if( HERO->GetStopFlag() == eSTOP_ITEMMIX )
			{
				HERO->SetStopFlag( eSTOP_NONE );
				HERO->ClearItemMixEffect();

				cItemMixWindow* win = GAMEUI->GetItemMixWindow();
				if( win )
				{
					win->StopGauge();
					win->Hide();
				}

				mNetLock = false;
				mTempIndex = 0;
				mTempRecipeIdx = 0;
			}
		}
		break;
	}
}

void cItemMixManager::NetLock( int lockTry )
{
	switch( lockTry )
	{
	case eLOCK_ITEMMIXDELETE:
		{
			cItemMixWindow* win = GAMEUI->GetItemMixWindow();
			if( win )
				win->NetLock( eLOCK_ITEMMIXDELETE );
		}
		break;
	case eLOCK_ITEMMIXSTART:
		{
			cItemMixWindow* win = GAMEUI->GetItemMixWindow();
			if( win )
				win->NetLock( eLOCK_ITEMMIXSTART );
		}
		break;
	default:
		assert(0);
		return;
	}

	mNetLock = true;
	mLockTry = lockTry;
}

void cItemMixManager::NetUnLock( int lockTry )
{
	if( mNetLock == false )
		return;

	switch( lockTry )
	{
	case eLOCK_ITEMMIXDELETE:
		{
			if( mLockTry == lockTry )
			{
				cItemMixWindow* win = GAMEUI->GetItemMixWindow();
				if( win )
					win->NetUnLock( eLOCK_ITEMMIXDELETE );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case eLOCK_ITEMMIXSTART:
		{
			cItemMixWindow* win = GAMEUI->GetItemMixWindow();
			if( win )
				win->NetUnLock( eLOCK_ITEMMIXSTART );
		}
		break;
	default:
		assert(0);
		return;
	}

	mLockTry = eLOCK_ITEMMIXNONE;
	mNetLock = false;
}
