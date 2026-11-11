#include "StdAfx.h"
#include "ItemMixWindow.h"
#include "Label.h"
#include "NumberEditBox.h"
#include "ComboBox.h"
#include "Button.h"
#include "CheckBox.h"
#include "UIImage.h"
#include "ListFolderBox.h"
#include "DoubleItemIconListBox.h"
#include "ItemIcon.h"
#include "Gauge.h"
#include "ItemMixManager.h"
#include "MakeSkillScript.h"
#include "ItemManager.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "ChatManager.h"
#include "Hero.h"
#include "DuelManager.h"

cItemMixWindow::cItemMixWindow()
: mpCoolTime(0)
, mpMixCount(0)
, mpCompleteCount(0)
, mpGaugeBar(0)
, mpSkillCombo(0)
, mpRecipeList(0)
, mpHighImage(0)
, mpItemList(0)
, mpRecipeCool(0)
, mpMixStart(0)
, mpMixGiveUp(0)
, mpMixStop(0)
, mAutoRemainCount(0)
, mAutoCount(0)
, mAutoSpeed(0)
//, mpGrade(0)
, mpPoint(0)
, mpGoalPoint(0)
, mpMax(0)
, mpPossibleRecipeCheck(0)
{
	memset( mpResultRate, 0, sizeof(mpResultRate) );
	memset( mpResultIcon, 0, sizeof(mpResultIcon) );
}

cItemMixWindow::~cItemMixWindow()
{
	SAFE_DELETE( mpHighImage );
}

void cItemMixWindow::Clear()
{
	InitCoolTime();

	if( mpMixCount )
		mpMixCount->SetValue( 1 );
	if( mpCompleteCount )
		mpCompleteCount->SetValue( 0 );

	if( mpGrade )
		mpGrade->Clear();
	if( mpPoint )
		mpPoint->SetValue( 0 );
	if( mpGoalPoint )
		mpGoalPoint->SetValue( 0 );

	if( mpGaugeBar )
		mpGaugeBar->ClearFront();
	if( mpSkillCombo )
	{
		mpSkillCombo->Clear();
		mpSkillCombo->SetEnabled( true );
	}
	if( mpRecipeList )
	{
		mpRecipeList->Clear();
		mpRecipeList->SetEnabled( true );
	}
	if( mpItemList )
		mpItemList->Clear();

	if( mpMixGiveUp )
		mpMixGiveUp->SetEnabled( false );
	if( mpMixStart )
		mpMixStart->SetEnabled( false );
	if( mpMixStop )
		mpMixStop->SetEnabled( false );
	if( mpMixCount )
		mpMixCount->SetEnabled( false );
	if( mpMax )
		mpMax->SetEnabled( false );

	for( int i = 0; i < 3; ++i )
	{
		mpResultRate[i]->Clear();
		mpResultIcon[i]->ChangeImage(0);
		mpResultIcon[i]->SetNumber(0);
		mpResultIcon[i]->SetItemIndex(0);
	}

	mAutoRemainCount = 0;
	mAutoCount = 0;
	mAutoSpeed = 0;
}

void cItemMixWindow::OnShow()
{
	cUIWindow::OnShow();

	GAMEUI->HideItemSetWindow( this );

	ITEMMAN->UpdateItem();
	// 보조직업 리스트 갱신
	UpdateMakeSkill();
}

void cItemMixWindow::Open()
{
}

void cItemMixWindow::Close()
{
	cUIWindow::Close();

	Clear();

	UIMAN->HideTip();
}

void cItemMixWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();

	UIMAN->HideTip();
}

void cItemMixWindow::HideESC( bool onsound )
{
	ITEMMIXMAN->ItemMixCancel();
}

bool cItemMixWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpCoolTime		= (cLabel*)GetChild( eUIID_GAME_ITEMMIX_COOLTIME );
	mpMixCount		= (cNumberEditBox*)GetChild( eUIID_GAME_ITEMMIX_NUMBEREDIT );
	mpMax			= (cButton*)GetChild( eUIID_GAME_ITEMMIX_MAX );
	mpCompleteCount = (cLabel*)GetChild( eUIID_GAME_ITEMMIX_COMPLETECOUNT );
	mpGrade			= (cLabel*)GetChild( eUIID_GAME_ITEMMIX_GRADE );
	mpPoint			= (cLabel*)GetChild( eUIID_GAME_ITEMMIX_SKILLNOW );
	mpGoalPoint		= (cLabel*)GetChild( eUIID_GAME_ITEMMIX_SKILLGOAL );

	mpGaugeBar		= (cGauge*)GetChild( eUIID_GAME_ITEMMIX_GAUGE );
	mpSkillCombo = (cComboBox*)GetChild( eUIID_GAME_ITEMMIX_MIXSKILLCOMBO );
	mpRecipeList  = (cItemMixListFolder*)GetChild( eUIID_GAME_ITEMMIX_MIXSKILLFOLDER );
	mpItemList	= (cDoubleItemIconListBox*)GetChild( eUIID_GAME_ITEMMIX_MIXLIST );
	mpMixStart		= (cButton*)GetChild( eUIID_GAME_ITEMMIX_START );
	mpMixGiveUp		= (cButton*)GetChild( eUIID_GAME_ITEMMIX_GIVEUP );
	mpMixStop		= (cButton*)GetChild( eUIID_GAME_ITEMMIX_CANCEL );
	mpPossibleRecipeCheck = (cCheckBox*)GetChild( eUIID_GAME_ITEMMIX_POSSIBLE_RECIPE );

	mpResultRate[0] = (cLabel*)GetChild( eUIID_GAME_ITEMMIX_RESULT1 );
	mpResultRate[1] = (cLabel*)GetChild( eUIID_GAME_ITEMMIX_RESULT2 );
	mpResultRate[2] = (cLabel*)GetChild( eUIID_GAME_ITEMMIX_RESULT3 );

	mpGrade->SetTextColor( mDefaultColor );
	mpGoalPoint->SetTextColor( mDefaultColor );
	mpPoint->SetTextColor( mDefaultColor );

	///
	mpItemList->CreateIcon( "ItemMix_Slot%02d", 6 );
	mpItemList->SetEnableSelect( false );

	/// 결과확률 등록 아이콘
	cString str;
	for( int i = 0; i < 3; ++i )
	{
		mpResultIcon[i] = new cItemIcon;

		str.Format( "ItemMixResult_Slot%02d", i );
		if( mpResultIcon[i]->CreateBySkinName( str, this, i+100 ) == false )
			return false;

		mpResultIcon[i]->SetUsedFlag( false, false );
	}
	
	/// 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "ItemMix_FolderCheck" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "ItemMix_FolderNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		unsigned int tx = pHighSkin->mSkinInfo->mTexX;
		unsigned int ty = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		unsigned int x = pHighSkin->mSkinInfo->mX;
		unsigned int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pHighNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pHighNoSkin->mSkinInfo->mTexY;

		mpRecipeList->SetHighNoCheckPos( tx, ty );
		mpRecipeList->SetHighCheckPos( tx2, ty2 );

		/// 하위열 출력위치 조정
		mpRecipeList->SetLowGap( w );
	}

	if( mpGaugeBar )
		mpGaugeBar->SetMaxValue( 100 );

	Clear();
	return true;
}

void cItemMixWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_ITEMMIX_CLOSE:
		ITEMMIXMAN->ItemMixCancel();
		break;
	case eUIID_GAME_ITEMMIX_GIVEUP:
		{
			if( ITEMMIXMAN->IsNetLock() )
				return;

			if( HERO->GetStopFlag() == eSTOP_ITEMMIX )
				return;

			/// 기술 포기
			unsigned char skillIndex = (unsigned char)mpSkillCombo->GetSelectData();

			cStringT str;
			unsigned long textIndex = ITEMMIXMAN->GetKeepSkillText( skillIndex );
			str.Format( GAMERESOURCEMAN->GetGameText( 2820 ), UIMAN->GetUIText( textIndex ) );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_ITEMMIX_DELETE, eSKIN_YESNO,
				(LPCTSTR)str.Cstr(),
				GAMERESOURCEMAN->GetGameText( 301 ),
				(void*)skillIndex );
		}
		break;
	case eUIID_GAME_ITEMMIX_START:
		if( mpMixCount && mpMixCount->GetValue() > 0 )
		{
			unsigned int row  = mpRecipeList->GetSelectRow();
			unsigned int column = mpRecipeList->GetSelectRowColumn();
			unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

			/// 선택된 스킬 정보
			unsigned char skillIndex = (unsigned char)mpSkillCombo->GetSelectData();
	
			/// 만들수 있는 최고치를 넘지 못한다
			unsigned int total = GetMaxValue( recipeIdx );
			int value = ( total < (int)mpMixCount->GetValue() ) ? total : mpMixCount->GetValue();

			if( value == 0 )
				return;
			
			if( value == UINT_MAX )
				mpMixCount->SetValue( 1 );
			else
				mpMixCount->SetValue( value );
		

			/// 
			if( ITEMMIXMAN->ItemMixStart( skillIndex, recipeIdx, value ) == true )
			{
				mAutoRemainCount = value;
				mAutoCount = 0;

				SetCompleteCount( mAutoCount );

				/// 
				sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
				if( script )
				{
					float speed = 100.0f / ((float)script->mMakeTime*0.001f);
					if( speed > 0.0f )
					{
						mAutoSpeed = speed;
						StartGauge( mAutoSpeed );
					}
				}
			}
		}
		else
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2809 ) );
		}	
		break;
	case eUIID_GAME_ITEMMIX_CANCEL:
		ITEMMIXMAN->ItemMixCancel( false );
		break;
	case eUIID_GAME_ITEMMIX_MAX:
		{
			/// max 버튼
			/// 선택된 퀘스트의 보상정보 세팅
			unsigned int row  = mpRecipeList->GetSelectRow();
			unsigned int column = mpRecipeList->GetSelectRowColumn();
			unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

			/// 재료 세팅
			unsigned int total = GetMaxValue( recipeIdx );
		
			if( mpMixCount )
			{
				if( total == UINT_MAX )
					mpMixCount->SetValue( 1 );
				else
					mpMixCount->SetValue( total );
			}
		}
		break;
	}
}

unsigned int cItemMixWindow::GetMaxValue( unsigned long recipeIdx, bool bChatMsg )
{
	/// 현재 총무게 >= 무게제한 일 경우 0개.
	if( ITEMMAN->GetWeight() >= HERO->GetMaxWeight() )
	{
		if( bChatMsg )
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2824 ) );
		return 0;
	}

	typedef tArray<void*>* cMaterialArr;
	cMaterialArr materialArr;

	/// 재료 최대 개수를 넘어갈 수 없다.
	unsigned int total = UINT_MAX;

	sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( script )
	{
		materialArr = script->mpMaterial;
		if( materialArr )
		{
			/// 재료들의 무게의 합.
			int totalMaterialWeight = 0;

			for( unsigned int i = 0; i < materialArr->GetSize(); ++i )
			{
				sMaterialScript* data = (sMaterialScript*)(*materialArr)[i];
				if( data )
				{
					sItemCount* p = ITEMMAN->GetItemCount( data->mItemIdx );
					unsigned int invenCount = ( p ) ? p->bag : 0;

					cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( data->mItemIdx );
					if( pItemDefine )
						totalMaterialWeight += ( pItemDefine->GetWeight() * data->mItemCnt );

					/// 남은 재료만큼 만들수 있는 개수 조정
					unsigned int makeCount = (invenCount / data->mItemCnt);
					total = ( total < makeCount ) ? total : makeCount;
				}
				else
				{
					assert(0);
					return 0;
				}
			}

			// 재료 부족
			if( total == 0 || total == UINT_MAX )
			{
				if( bChatMsg )
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2827 ) );
				return 0;
			}
			
			for( unsigned int cnt = 0; cnt < MAKESKILL_MAKE_ITEM_CNT; ++cnt )		
			{
				if( script->mMakeItemIdx[cnt] == 0 )
					continue;

				/// 무게 체크
				cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( script->mMakeItemIdx[cnt] );
				if( pItemDefine )
				{
					int resultItemWeight = pItemDefine->GetWeight() * script->mMakeCnt[cnt];

					/// 현재 총무게 - 소모아이템들 무게 + 생성될 아이템 무게 > 총 무게제한 일 경우 0개.
					if( ITEMMAN->GetWeight() - totalMaterialWeight + resultItemWeight > HERO->GetMaxWeight() )
					{
						if( bChatMsg )
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2824 ) );
						return 0;
					}
					else if( resultItemWeight > totalMaterialWeight )
					{
						/// 생성될 아이템 무게 > 소모아이템들 무게 일 경우 [ 현재 여유무게 / 아이템 생성 시 늘어날 무게 ]
						int itemCount = (HERO->GetMaxWeight() - FloatToInt(ITEMMAN->GetWeight())) / (resultItemWeight - totalMaterialWeight);

						if( itemCount > 0 )
							total = ( total < (unsigned int)itemCount ) ? total : itemCount;
					}
				}
				else
				{
					assert( 0 && "Bad Item index" );
					return 0;
				}

				/// 보유가능 개수 체크
				if( pItemDefine->GetMaxHaveInven() > 0 )
				{
					sItemCount* p = ITEMMAN->GetItemCount( script->mMakeItemIdx[cnt] );
					unsigned int invenCount = ( p ) ? p->total : 0;

					int maxHaveInven = pItemDefine->GetMaxHaveInven() - invenCount - script->mMakeCnt[cnt];
					if( maxHaveInven > 0 )
					{
						total = (total < (unsigned int)maxHaveInven)? total : maxHaveInven;
					}
					else
					{
						if( bChatMsg )
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2825 ) );
						return 0;
					}
				}
	
				/// 빈슬롯 개수만큼 개수 조정
 				total = ITEMMAN->GetRemainCount( script->mMakeItemIdx[cnt], total );
				if( total < 1 )
				{
					if( bChatMsg )
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2826 ) );
					return 0;
				}
			}
			
			if( script->mNextMakeTime > 0 && total > 1)
				total = 1;

		}
	}
	return total;
}

void cItemMixWindow::OnComboSelected( cUINode* , unsigned int id )
{
	if( HERO->GetStopFlag() == eSTOP_ITEMMIX )
		return;

	if( !mpSkillCombo )
	{
		assert(0);
		return;
	}

	// 보조직업 콤보 갱신
	if( id == eUIID_GAME_ITEMMIX_MIXSKILLCOMBO )
	{
		unsigned char skillIndex = (unsigned char)mpSkillCombo->GetSelectData();

		sHeroInfo* info = HERO->GetHeroInfo();
		sMakeSkill* skill1 = &info->MakeSkill1;
		sMakeSkill* skill2 = &info->MakeSkill2;

		sMakeSkill* select = 0;
		if( skill1->mIndex == skillIndex )
		{
			select = skill1;
		}
		else if( skill2->mIndex == skillIndex )
		{
			select = skill2;
		}
		else
		{
			assert(0);
			return;
		}

		if( mpMixStart )
			mpMixStart->SetEnabled( false );

		if( mpMixStop )
			mpMixStop->SetEnabled( false );

		if( mpMixCount )
			mpMixCount->SetEnabled( false );

		if( mpMax )
			mpMax->SetEnabled( false );

		// 등급 & 숙련도 세팅
		unsigned long gradeIndex = 0;
		unsigned long maxpoint = 0;
		switch( select->mStep )
		{
		case ITEM_MIX_SKILL_L1:		gradeIndex = 2000871; maxpoint = MIXSKILL_EXP1; break;
		case ITEM_MIX_SKILL_L2:		gradeIndex = 2000872; maxpoint = MIXSKILL_EXP2; break;
		case ITEM_MIX_SKILL_L3:		gradeIndex = 2000873; maxpoint = MIXSKILL_EXP3; break;
		case ITEM_MIX_SKILL_L4:		gradeIndex = 2000874; maxpoint = MIXSKILL_EXP4; break;
		default:				assert(0);							return;
		}

		if( mpGrade )
			mpGrade->SetText( GAMERESOURCEMAN->GetItemToolTipText( gradeIndex ) );

		if( mpPoint )
			mpPoint->SetValue( select->mExp );

		if( mpGoalPoint )
			mpGoalPoint->SetValue( maxpoint );

		if( mpPossibleRecipeCheck && mpPossibleRecipeCheck->IsPress() == true )
		{
			UpdateRecipeList( skillIndex, true, true );
		}
		else
		{
			// 보조직업 관련 레시피 리스트로 갱신
			UpdateRecipeList( skillIndex );
		}		
	}
}

void cItemMixWindow::UpdateSelectedRecipe()
{
	/// 초기화
	if( mpMixCount ) 
		mpMixCount->SetValue( 1 );
	if( mpCompleteCount )
		mpCompleteCount->SetValue( 0 );

	if( mpMixStart )
		mpMixStart->SetEnabled( false );
	if( mpMixCount )
		mpMixCount->SetEnabled( false );
	if( mpMax )
		mpMax->SetEnabled( false );

	if( mpItemList )
		mpItemList->Clear();

	for( int i = 0; i < MAKESKILL_MAKE_ITEM_CNT; ++i )
	{
		/// 아이콘 초기화
		mpResultIcon[i]->ChangeImage(0);
		mpResultIcon[i]->SetNumber(0);
		mpResultIcon[i]->SetItemIndex(0);
		mpResultRate[i]->Clear();
	}

	InitCoolTime();

	// 조합아이템 리스트 갱신 ( 새로선택 )
	typedef tArray<void*>* cMaterialArr;
	cMaterialArr materialArr;

	unsigned int row  = mpRecipeList->GetSelectRow();
	unsigned int column = mpRecipeList->GetSelectRowColumn();
	unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

	/// 재료 세팅
	sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( !script )
		return;

	sHeroInfo* info = HERO->GetHeroInfo();
	sMakeSkill* skill1 = &info->MakeSkill1;
	sMakeSkill* skill2 = &info->MakeSkill2;

	/// 1. 재사용 시간 세팅
	unsigned char skillIndex = (unsigned char)mpSkillCombo->GetSelectData();
	if( skill1->mIndex == skillIndex )
	{
		mpRecipeCool = ITEMMIXMAN->GetKeepMixSkill1( recipeIdx );
	}
	else if( skill2->mIndex == skillIndex )
	{
		mpRecipeCool = ITEMMIXMAN->GetKeepMixSkill2( recipeIdx );
	}
	else
	{
		assert(0);
		return;
	}

	bool readyToMix = true;
	if( mpRecipeCool )
	{
		if( mpRecipeCool->mNextMakeTime > 0 && mpRecipeCool->mRestTime > 0 )
		{	
			readyToMix = false;
		}
	}
	else
	{
		assert(0);
		return;
	}		

	/// 2. 재료 세팅

	materialArr = script->mpMaterial;
	if( materialArr )
	{
		for( unsigned int i = 0; i < materialArr->GetSize(); ++i )
		{
			sMaterialScript* data = (sMaterialScript*)(*materialArr)[i];
			if( data )
			{
				cItemDefine* define = ITEMMAN->GetItemDefine( data->mItemIdx );
				if( define )
				{
					// 앞 : 인벤 갯수, 뒤 : 조합필요량
					sItemCount* p = ITEMMAN->GetItemCount( data->mItemIdx );
					int invenCount = ( p ) ? p->bag : 0;

					/// 부족한 아이템 붉은색으로 바꾸기
					unsigned long color = mDefaultColor;
					if( invenCount < data->mItemCnt ) 
					{
						color = COLOR_RED;
						readyToMix = false;
					}

					unsigned int row = mpItemList->AddRow( define->GetName(), color, (void*)data->mItemIdx );
					if( row >= 0 )
					{
						cStringT countStr;
						countStr.Format( _T("%d/%d"), invenCount, data->mItemCnt );
						mpItemList->AddColume( row, (LPCTSTR)countStr.Cstr(), color );
					}
				}
				else
				{
					assert(0);
				}
			}
			else
			{
				assert(0);
			}
		}
	}

	if( readyToMix == true && DUELMAN->IsDuelEnable() == false )
	{
		if( mpMixStart )
			mpMixStart->SetEnabled( true );

		if( mpMixCount )
			mpMixCount->SetEnabled( true );

		if( mpMax )
			mpMax->SetEnabled( true );
	}

	unsigned int totalRate = 0;
	for( int i = 0; i < MAKESKILL_MAKE_ITEM_CNT; ++i )
	{
		unsigned long itemIndex = script->mMakeItemIdx[i];
		if( itemIndex > 0 )
			totalRate = totalRate + script->mMakeRate[i];
	}

	for( int i = 0; i < MAKESKILL_MAKE_ITEM_CNT; ++i )
	{
		/// 아이콘 초기화
		mpResultIcon[i]->ChangeImage(0);
		mpResultIcon[i]->SetNumber(0);
		mpResultIcon[i]->SetItemIndex(0);
		mpResultRate[i]->Clear();

		unsigned long itemIndex = script->mMakeItemIdx[i];
		unsigned int count = script->mMakeCnt[i];
		unsigned short rate = script->mMakeRate[i];
		if( itemIndex > 0 )
		{
			cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
			if( define )
			{
				mpResultIcon[i]->SetItemIndex( itemIndex );
				mpResultIcon[i]->ChangeImage( define->GetIconParam() );
				mpResultIcon[i]->SetNumber( count );
			}

			float presentRate = (float)(rate * 100 / totalRate);

			cStringT str;
			str.Format( _T("%.f%%"), presentRate );
			mpResultRate[i]->SetText( (LPCTSTR)str.Cstr() );
		}
	}
}

void cItemMixWindow::UpdateRecipeList()
{
	if( mpPossibleRecipeCheck == NULL || mpRecipeList == NULL || mpSkillCombo == NULL )
	{
		assert(0);
		return;
	}

	unsigned char curIndex = 0;
	curIndex = (unsigned char)mpSkillCombo->GetSelectData();
	if( curIndex <= 0 )
		return;

	/// 리스트 초기화전 선택되어진 레시피를 저장
	unsigned int row		= mpRecipeList->GetSelectRow();
	unsigned int column		= mpRecipeList->GetSelectRowColumn();
	unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

	if( mpPossibleRecipeCheck->IsPress() == true  )
	{		
		UpdateRecipeList( curIndex, false, true );
	}
	else
	{
		UpdateRecipeList( curIndex, false, false );
	}	

	/// 이전에 선택되어진 레시피 있다면 설정
	sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( script )
	{
		int col = mpRecipeList->FindColumnByExtra( script->mType, recipeIdx );
		if( col >= 0 )
		{
			mpRecipeList->ChangeSelectColumn( col );
		}
		else
		{
			InitCoolTime();

			for( int i = 0; i < 3; ++i )
			{
				mpResultRate[i]->Clear();
				mpResultIcon[i]->ChangeImage(0);
				mpResultIcon[i]->SetNumber(0);
				mpResultIcon[i]->SetItemIndex(0);
			}

			if( mpItemList )
				mpItemList->Clear();
		}
	}		
}

void cItemMixWindow::OnListBoxChanged( cUINode* , unsigned int id )
{
	if( HERO->GetStopFlag() == eSTOP_ITEMMIX )
		return;

	if( id == eUIID_GAME_ITEMMIX_MIXSKILLFOLDER )
	{
		UpdateSelectedRecipe();		
	}
}

void cItemMixWindow::OnCheckBoxChanged( cUINode* , unsigned int id )
{

	if( mpPossibleRecipeCheck && mpSkillCombo )
	{
		if( mpMixStart )
			mpMixStart->SetEnabled( false );

		if( mpMixStop )
			mpMixStop->SetEnabled( false );

		if( mpMixCount )
			mpMixCount->SetEnabled( false );

		if( mpMax )
			mpMax->SetEnabled( false );

		unsigned char curIndex = (unsigned char)mpSkillCombo->GetSelectData();

		if( mpPossibleRecipeCheck->IsPress() == true )
		{
			UpdateRecipeList( curIndex, true, true );
		}
		else
		{
			if( mpSkillCombo )
			{
				UpdateRecipeList( curIndex, true, false );
			}
			else
			{
				assert(0);
				return;
			}
		}
	}
	else
	{
		assert(0);
		return;
	}
}

void cItemMixWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mpRecipeCool )
	{	
		cStringT th,tm,ts;		
		unsigned long elapsedTime = accumTime - mpRecipeCool->mStartTime;

		// 재 사용 대기 시간이 있는 아이템인 경우
		if( mpRecipeCool->mNextMakeTime > 0 )
		{
			//  현재 사용하여서 시간값이 갱신되는 경우
			if( mpRecipeCool->mRestTime > 0 )
			{
				if( mpRecipeCool->mRestTime > elapsedTime )
				{
					short coolTime = (short)((mpRecipeCool->mRestTime - elapsedTime) / 1000);

					/// 재사용시간 구해서 출력
					unsigned int h = (((unsigned int)coolTime ) / 60) / 60;
					unsigned int m = ((unsigned int)coolTime ) / 60 % 60;
					unsigned int s = ((unsigned int)coolTime % 60) % 60;

					if( h != 0 )
						th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
					if( m != 0 )
						tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
					if( s != 0 )
						ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );

					th += tm;
					th += ts;

					if( th.GetLength()  == 0 )
						th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), 0 );

					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 11250 ), th.Cstr(), mpRecipeCool->mStrNextMakeTime.Cstr() );

					if( mpCoolTime )
						mpCoolTime->SetText( (LPCTSTR)str.Cstr() );

					if( mpMixStart )
						mpMixStart->SetEnabled( false );
					if( mpMixCount )
						mpMixCount->SetEnabled( false );
					if( mpMax )
						mpMax->SetEnabled( false );
				}
				else
				{
					mpRecipeCool->mRestTime = 0;
					mpRecipeCool->mStartTime = 0;
					
					if( mpMax )
					{
						unsigned int row  = mpRecipeList->GetSelectRow();
						unsigned int column = mpRecipeList->GetSelectRowColumn();
						unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

						unsigned int total = GetMaxValue( recipeIdx, false );
						bool enable = false;
						if( total > 0 )
						{
							enable = true;							
						}
						else
						{
							enable = false;
						}

						if( mpMixStart )
							mpMixStart->SetEnabled( enable );
						if( mpMixCount )
							mpMixCount->SetEnabled( enable );
						if( mpMax )
							mpMax->SetEnabled( enable );
					}
					else
					{
						assert(0);
					}
				}

			}
			else
			{
				if( mpCoolTime )
					mpCoolTime->SetText( (LPCTSTR)mpRecipeCool->mStrNextMakeTime.Cstr()  );
			}			
		}
		else
		{
			ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), 0 );

			if( mpCoolTime )
				mpCoolTime->SetText( (LPCTSTR)ts.Cstr() );
		}		
	}
}


void cItemMixWindow::NetGaugeEnd( bool success )
{
	if( mpGaugeBar )
	{
		mpGaugeBar->ClearFront();

		if( success == true )
		{
			/// 저장된 횟수만큼 다시 돌린다
			if( mAutoRemainCount > 0 )
			{	
				mAutoCount++;
				mAutoRemainCount--;

				SetCompleteCount( mAutoCount );
				mpMixCount->SetValue( mAutoRemainCount );
			}		

			/// 조합할게 더 남았는지 체크.
			if( mAutoRemainCount > 0 )
			{
				StartGauge( mAutoSpeed );
			}
			else
			{
				StopGauge();
				mpMixCount->SetValue( 1 );						
			}
		}
		else
		{
			StopGauge();
			mpMixCount->SetValue( 1 );				
		}				
	}
}

// 숙련도 갱신
void cItemMixWindow::SetMixExp( unsigned char skillIndex, unsigned long exp )
{
	// 현재 보고있는 보조직업이면 갱신
	unsigned char curIndex = (unsigned char)mpSkillCombo->GetSelectData();
	if( curIndex == skillIndex )
	{
		if( mpPoint )
			mpPoint->SetValue( exp );

		// 각 리스트의 컬러 갱신
		if( mpRecipeList )
		{
			int rowCnt = mpRecipeList->GetRowCount();
			for( int i = 0; i < rowCnt; ++i )
			{
				// 상위열 제외
				int colCnt = mpRecipeList->GetColumnCount( i );
				for( int j = 1; j < colCnt; ++j )
				{
					unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( i, j );
					
					sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
					if( script )
					{
						sFolderData* folderData = mpRecipeList->GetFolderData( i, j );
						if( !folderData )
						{
							assert(0);
							continue;
						}

						// 컬러 값 얻기
						unsigned long newcolor = COLOR_WHITE;

						// 최소숙련도 ~ 기본최소숙련도
						if( exp >= script->mMinExp && exp < script->mMinDefaultExp )
						{
							newcolor = COLOR_ORANGE;
						}
						// 기본최소숙련도 ~ 기본최대숙련도
						else if( exp >= script->mMinDefaultExp && exp < script->mMaxDefaultExp )
						{
							newcolor = COLOR_YELLOW;
						}
						// 기본최대숙련도 ~ 최대업가능숙련도
						else if( exp >= script->mMaxDefaultExp && exp < script->mMaxExp )
						{
							newcolor = COLOR_GREEN;
						}

						folderData->mTextColor = newcolor;
					}
				}
			}
		}
	}
}

// 소유 보조직업으로 갱신
void cItemMixWindow::UpdateMakeSkill()
{
	if( !mpSkillCombo || !mpMixGiveUp )
	{
		assert(0);
		return;
	}

	mpSkillCombo->Clear();

	//
	sHeroInfo* info = HERO->GetHeroInfo();
	sMakeSkill* skill1 = &info->MakeSkill1;
	sMakeSkill* skill2 = &info->MakeSkill2;

	if( skill1->mIndex == 0 && skill2->mIndex == 0 )
	{
		mpMixGiveUp->SetEnabled( false );
		return;
	}

	// 보조직업 1 넣기
	if( skill1->mIndex > 0 )
	{
		unsigned long textIndex = ITEMMIXMAN->GetKeepSkillText( skill1->mIndex );
		mpSkillCombo->AddItem( UIMAN->GetUIText( textIndex ), (void*)skill1->mIndex );
	}

	// 보조직업 2 넣기
	if( skill2->mIndex > 0 )
	{
		unsigned long textIndex = ITEMMIXMAN->GetKeepSkillText( skill2->mIndex );
		mpSkillCombo->AddItem( UIMAN->GetUIText( textIndex ), (void*)skill2->mIndex );
	}

	//
	mpMixGiveUp->SetEnabled( true );
	mpSkillCombo->SetSelect( 0 );
}

unsigned char cItemMixWindow::GetSelectSkill()
{
	return ( mpSkillCombo ) ? (unsigned char)mpSkillCombo->GetSelectData() : 0;
}

// 제조스킬 리스트 갱신( 레시피목록 )
void cItemMixWindow::UpdateRecipeList( unsigned char skillIndex, bool bInit, bool bPossible )
{
	if( skillIndex == eMAKESKILL_NONE )
		return;

	/// 현재 UI를 보고 있을때만 갱신한다 ( 안볼땐 show하면서 자동갱신 )
	if( mVisible == false )
		return;

	if( bInit == true )
	{
		/// 초기화
		if( mpMixCount )
			mpMixCount->SetValue( 1 );
		if( mpCompleteCount )
			mpCompleteCount->SetValue( 0 );

		if( mpGaugeBar )
			mpGaugeBar->ClearFront();
		
		if( mpItemList )
			mpItemList->Clear();

		if( mpMixStart )
			mpMixStart->SetEnabled( false );
		if( mpMixStop )
			mpMixStop->SetEnabled( false );

		for( int i = 0; i < 3; ++i )
		{
			mpResultRate[i]->Clear();
			mpResultIcon[i]->ChangeImage(0);
			mpResultIcon[i]->SetNumber(0);
			mpResultIcon[i]->SetItemIndex(0);
		}
	}

	if( mpRecipeList )
		mpRecipeList->Clear();

	typedef tPointerHashMap<unsigned long, void*> cKeepRecipeMap;
	cKeepRecipeMap* keepRecipeMap = 0;

	sMakeSkill* select = 0;
	sHeroInfo* info = HERO->GetHeroInfo();
	if( info )
	{
		sMakeSkill* skill1 = &info->MakeSkill1;
		sMakeSkill* skill2 = &info->MakeSkill2;

		if( skill1->mIndex == skillIndex )
		{
			keepRecipeMap = ITEMMIXMAN->GetKeepSkillMap1();
			select = skill1;
		}
		else if( skill2->mIndex == skillIndex )
		{
			keepRecipeMap = ITEMMIXMAN->GetKeepSkillMap2();
			select = skill2;
		}
		else
		{
			assert(0);
			return;
		}
	}

	if( !keepRecipeMap )
		return;

	/// 세팅
	typedef tList<unsigned long> cTempList;
	cTempList tempList;

	cKeepRecipeMap::cIterator i = keepRecipeMap->Begin();
	cKeepRecipeMap::cIterator end = keepRecipeMap->End();
	for( ; i != end; ++i )
	{
		unsigned long key = (unsigned long)(i->mFirst);
		tempList.PushBack( key );
	}

	// 정렬
	if( tempList.GetSize() > 1 )
	{
		tempList.Sort();
	}

	typedef tArray<void*>* cMaterialArr;
	cMaterialArr materialArr;

	cTempList::cIterator ti = tempList.Begin();
	cTempList::cIterator tend = tempList.End();
	for( ; ti != tend; ++ti )
	{
		unsigned long key = (*ti);
		sKeepRecipe* data = (sKeepRecipe*)keepRecipeMap->GetAt( key );
		if( data )
		{
			sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( key );
			if( script )
			{
				materialArr = script->mpMaterial;

				bool bPossibleRecipe			= true;
				unsigned int nMinPossibleCnt	= INT_MAX; 

				for( unsigned int i = 0; i < materialArr->GetSize(); i++ )
				{
					sMaterialScript* data = (sMaterialScript*)(*materialArr)[i];
					if( data )
					{
						cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( data->mItemIdx );
						if( pItemDefine == NULL)
						{
							assert(0);
							return;
						}

						sItemCount* p = ITEMMAN->GetItemCount( data->mItemIdx );

						unsigned int invenCount = ( p ) ? p->bag : 0;							
						unsigned int makeCount	= (invenCount / data->mItemCnt);

						if( makeCount == 0 )
						{
							bPossibleRecipe = false;
							nMinPossibleCnt = 0;
							break;
						}

						nMinPossibleCnt = min( nMinPossibleCnt, makeCount );
					}
					else
					{
						assert(0);
						break;
					}					
				}

				// 텍스트
				unsigned long textIndex = ITEMMIXMAN->GetTypeText( skillIndex, script->mType );

				// 컬러 값 얻기
				unsigned long color = COLOR_WHITE;
				unsigned long exp = select->mExp;

				// 최소숙련도 ~ 기본최소숙련도
				if( exp >= script->mMinExp && exp < script->mMinDefaultExp )
				{
					color = COLOR_ORANGE;
				}
				// 기본최소숙련도 ~ 기본최대숙련도
				else if( exp >= script->mMinDefaultExp && exp < script->mMaxDefaultExp )
				{
					color = COLOR_YELLOW;
				}
				// 기본최대숙련도 ~ 최대업가능숙련도
				else if( exp >= script->mMaxDefaultExp && exp < script->mMaxExp )
				{
					color = COLOR_GREEN;
				}
								
				if(  bPossible == true )
				{
					if( bPossibleRecipe && nMinPossibleCnt > 0 && nMinPossibleCnt != INT_MAX )
					{
						cStringT str;
						str.Format( GAMERESOURCEMAN->GetGameText(2834), ITEMMAN->GetItemName( key ), nMinPossibleCnt );
						mpRecipeList->AddHighData( script->mType, GAMERESOURCEMAN->GetItemToolTipText( textIndex ), mDefaultColor, 0, mpHighImage );						
						mpRecipeList->AddLowData( script->mType, (LPTSTR)str.Cstr(), color, (void*)key );
					}
				}
				else
				{
					mpRecipeList->AddHighData( script->mType, GAMERESOURCEMAN->GetItemToolTipText( textIndex ), mDefaultColor, 0, mpHighImage );

					if( bPossibleRecipe && nMinPossibleCnt > 0 && nMinPossibleCnt != INT_MAX )
					{
						cStringT str;
						str.Format( GAMERESOURCEMAN->GetGameText(2834), ITEMMAN->GetItemName( key ), nMinPossibleCnt );
						mpRecipeList->AddLowData( script->mType, (LPTSTR)str.Cstr(), color, (void*)key );
					}
					else
					{
						mpRecipeList->AddLowData( script->mType, ITEMMAN->GetItemName( key ), color, (void*)key );
					}										
				}				
			}
		}
	}

	mpRecipeList->UpdateSelectImage();
}

// 조합 결과 재료아이템 갯수 갱신 
void cItemMixWindow::UpdateItemCount()
{
	if( mpMixStart )
		mpMixStart->SetEnabled( false );
	if( mpMixCount )
		mpMixCount->SetEnabled( false );
	if( mpMax )
		mpMax->SetEnabled( false );

	typedef tArray<void*>* cMaterialArr;
	cMaterialArr materialArr;

	unsigned int row  = mpRecipeList->GetSelectRow();
	unsigned int column = mpRecipeList->GetSelectRowColumn();
	unsigned long recipeIdx = (unsigned long)mpRecipeList->GetColumnData( row, column );

	/// 재료 세팅
	bool readyToMix = true;
	sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( recipeIdx );
	if( script )
	{
		materialArr = script->mpMaterial;
		if( materialArr )
		{
			mpItemList->ClearText();

			for( unsigned int i = 0; i < materialArr->GetSize(); ++i )
			{
				sMaterialScript* data = (sMaterialScript*)(*materialArr)[i];
				if( data )
				{
					cItemDefine* define = ITEMMAN->GetItemDefine( data->mItemIdx );
					if( define )
					{
						// 앞 : 인벤 갯수, 뒤 : 조합필요량
						sItemCount* p = ITEMMAN->GetItemCount( data->mItemIdx );
						int invenCount = ( p ) ? p->bag : 0;

						/// 부족한 아이템 붉은색으로 바꾸기
						unsigned long color =  mDefaultColor;
						if( invenCount < data->mItemCnt )
						{
							color = COLOR_RED;
							readyToMix = false;
						}

						unsigned int row = mpItemList->AddRow( define->GetName(), color, (void*)data->mItemIdx );
						if( row >= 0 )
						{
							cStringT countStr;
							countStr.Format( _T("%d/%d"), invenCount, data->mItemCnt );
							mpItemList->AddColume( row, (LPCTSTR)countStr.Cstr(), color );
						}
					}
					else
					{
						assert(0);
					}
				}
				else
				{
					assert(0);
				}
			}

			/// 자동 조합중이 아니면 버튼 다시 활성화
 			if( readyToMix == true && mAutoRemainCount == 0 && DUELMAN->IsDuelEnable() == false )
			{
				if( mpMixStart )
 					mpMixStart->SetEnabled( true );

				if( mpMixCount )
					mpMixCount->SetEnabled( true );

				if( mpMax )
					mpMax->SetEnabled( true );
			}
		}
	}
}

void cItemMixWindow::InitCoolTime()
{
	mpRecipeCool = 0;

	if( mpCoolTime )
	{
		cStringT ts;
		ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), 0 );
		mpCoolTime->SetText( (LPCTSTR)ts.Cstr() );
	}
}

void cItemMixWindow::SetCompleteCount( int count )
{
	if( mpCompleteCount )
		mpCompleteCount->SetValue( count );
}

void cItemMixWindow::StartGauge( float speed )
{
	if( mpGaugeBar )
	{
		mpGaugeBar->SetGaugeSpeed( speed );
		mpGaugeBar->SetGaugeAni( 100 );
	}

	if( mpMixStop )
		mpMixStop->SetEnabled( true );
	if( mpRecipeList )
		mpRecipeList->SetEnabled( false );
	if( mpSkillCombo )
		mpSkillCombo->SetEnabled( false );
	if( mpMixStart )
		mpMixStart->SetEnabled( false );
	if( mpMixGiveUp )
		mpMixGiveUp->SetEnabled( false );
	if( mpMixCount )
		mpMixCount->SetEnabled( false );
	if( mpMax )
		mpMax->SetEnabled( false );
	if( mpPossibleRecipeCheck )
		mpPossibleRecipeCheck->SetEnabled( false );
}

void cItemMixWindow::StopGauge()
{
	mAutoRemainCount = 0;
	mAutoCount = 0;
	mAutoSpeed = 0;

	if( mpGaugeBar )
		mpGaugeBar->ClearFront();

	if( mpRecipeList )
		mpRecipeList->SetEnabled( true );
	if( mpSkillCombo )
		mpSkillCombo->SetEnabled( true );
	if( mpMixStart )
		mpMixStart->SetEnabled( true );
	if( mpMixGiveUp )
		mpMixGiveUp->SetEnabled( true );
	if( mpMixCount )
		mpMixCount->SetEnabled( true );
	if( mpMax )
		mpMax->SetEnabled( true );
	if( mpMixStop )
		mpMixStop->SetEnabled( false );
	if( mpPossibleRecipeCheck )
		mpPossibleRecipeCheck->SetEnabled( true );
}

void cItemMixWindow::OnNetLock( int lockTry )
{
	if( lockTry == eLOCK_ITEMMIXDELETE )
	{
		if( mpMixGiveUp )
			mpMixGiveUp->SetEnabled( false );
	}
}

void cItemMixWindow::OnNetUnLock( int lockTry )
{
	if( lockTry == eLOCK_ITEMMIXDELETE )
	{
		sHeroInfo* info = HERO->GetHeroInfo();

		// 보조스킬이 하나도없으면 활성화시키지 않는다
		bool enable = ( info->MakeSkill1.mIndex == 0 && info->MakeSkill2.mIndex == 0 )  ? false : true;
		if( mpMixGiveUp )
			mpMixGiveUp->SetEnabled( enable );
	}
}

cItemMixListFolder::cItemMixListFolder()
{
}

cItemMixListFolder::~cItemMixListFolder()
{
}

int cItemMixListFolder::FindColumnByExtra( unsigned long type, unsigned long recipe )
{
	int totalCount = 0;
	for( unsigned int ri = 0; ri < mRows.GetSize(); ++ri )
	{
		sRowData* p = (sRowData*)mRows[ri];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( !data )
			{
				assert(0);
				continue;
			}

			if( data->key != (long)type )
			{
				if( data->mCheck == true )
					totalCount += p->mColumes.GetSize();
				else
					totalCount += 1;
			}
			else
			{
				// 접혀있으면 리턴
				if( data->mCheck == false )
					return -1;

				/// 동일 데이터 검색
				for( unsigned int ci = 0; ci < p->mColumes.GetSize(); ++ci )
				{
					sFolderData* data = (sFolderData*)p->mColumes[ci];
					if( !data )
					{
						assert(0);
						continue;
					}

					if( ci > 0 && (unsigned long)(data->mExtraData) == recipe )
						return totalCount;

					totalCount++;
				}
				break;
			}
		}
	}
	return -1;
}
