#include "stdafx.h"
#include "TarotResultWindow.h"
#include "TarotWindow.h"
#include "TarotManager.h"
#include "UIImageFont.h"
#include "Label.h"
#include "Button.h"
#include "Gauge.h"
#include "ItemDefine.h"

#include "GameUIManager.h"
#include "ObjectManager.h"
#include "Hero.h"

cTarotResultWindow::cTarotResultWindow()
: mpDescription(0)
, mpOpenTotal(0)
, mpResultGauge(0)
, mpRightBtn(0)
, mpUpBtn(0)
, mpResult(0)
, mpMain(0)
, mpAllRandom(0)
, mpAllStraight(0)
, mpAllInverse(0)
, mpOneStraight(0)
, mpOneInverse(0)
{
	for( unsigned int i=0; i<10; i++ )
		mpOpenValue[i] = 0;
}

cTarotResultWindow::~cTarotResultWindow()
{
}

///
bool cTarotResultWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMain = (cTarotWindow*)mpParent;

	for( unsigned int i = 0; i < 10; i++ )
		mpOpenValue[i] = (cLabel*)GetChild( eUIID_TAROTRESULT_OPENTXT1 + i );

	mpResultGauge = (cGauge*)GetChild( eUIID_TAROTRESULT_GAUGE );
	mpDescription = (cLabel*)GetChild( eUIID_GAME_TAROTRESULT_DESCRIPTION );
	mpOpenTotal = (cLabel*)GetChild( eUIID_GAME_TAROTRESULT_OPENTOTAL );
	mpRightBtn = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_RIGHT );
	mpUpBtn = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_UP );
	mpResult = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_RESULT );

	mpAllRandom   = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_RANDOM );;
	mpAllStraight = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_ALLSTR );;
	mpAllInverse  = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_ALLINV );;
	mpOneStraight = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_ONESTR );;
	mpOneInverse  = (cButton*)GetChild( eUIID_GAME_TAROTRESULT_ONEINV );;

	if( mpResult )
		mpResult->SetEnabled( false );

	cLabel* descBack = (cLabel*)GetChild( eUIID_TAROTRESULT_DESCBACKIMG );
	if( descBack )
		descBack->SetEnabled( false );
	return true;
}

///
void cTarotResultWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	UpdateButton();
}

///
void cTarotResultWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	///
	cUIWindow::OnRender( pKeeper );

	if( mpMain == 0 )
		return;

	cItemSpread* spread = mpMain->GetCurrentSpreadInfo();
	if( spread  )
	{
		for( unsigned int i=0; i<10; i++ )
		{
			unsigned int x = spread->mParam[i].mX + mpMain->GetAbsoluteRect().mLeft;
			unsigned int y = spread->mParam[i].mY + mpMain->GetAbsoluteRect().mTop;

			cString IndexStr;
			IndexStr.Format( "%d", i + 1 );
			cUIImageFont* pImageFont = UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );
			if( pImageFont )
			{
				pImageFont->DrawText( x + 8, y + 8, IndexStr.Cstr() );
			}
			else
			{
				assert(0);
			}

			unsigned int value = spread->mParam[i].mValue;
			if( value <= 0 )
				continue;
			if( mpMain == 0 )
				continue;

			/// calculate value render
			bool render = true;
			if( value > 0 )
			{
				cString ValueStr;
				ValueStr.Format( "%d", value );

				unsigned int openIndex = mpMain->GetSelectedCount();
				if( openIndex > 0 && i < openIndex )
				{
					sResultInfo* info = TAROTMAN->GetSelectCardInfo(i);
					if( info )
						render = info->Apply;
				}

				if( render == true )
				{
					/// 계산수식 렌더링 ( 0 : 노말, 1 : *, 2 : /, 3 : + )
					unsigned int Calc = spread->mParam[i].mCalc;
					if( Calc > 0 )
					{
						cString CalcStr;
						CalcStr.Format( "%d", Calc-1 );

						cUIImageFont* pCalcFont = UIMAN->GetImageFont( eIAMGEFONT_TAROTCALC );
						if( pCalcFont )
						{
							pCalcFont->DrawText( x + 2, y + 50, CalcStr.Cstr() );
						}
						else
						{
							assert(0);
						}
					}

					/// 타롯 방향에 따른 출력 글자 색 설정
					unsigned int ii = eIAMGEFONT_TAROTDEFAULT;
					unsigned int dir = spread->mParam[i].mDir;
					if( dir == eSPREAD_STR )
					{
						/// 정방향
						ii = eIAMGEFONT_TAROTSTR;
					}
					else if( dir == eSPREAD_INV )
					{
						/// 역방향
						ii = eIAMGEFONT_TAROTINV;
					}

					cUIImageFont* pValueFont = UIMAN->GetImageFont( ii );
					if( pValueFont )
					{
						pValueFont->DrawText( x + 30, y + 50, ValueStr.Cstr() );
					}
					else
					{
						assert(0);
					}

					render = false;
				}

			}
		}
	}
}

///
void cTarotResultWindow::OnCommand( cUINode*, unsigned int id )
{
	switch(id)
	{
	case eUIID_GAME_TAROTRESULT_RESULT:
		{
			if( mpMain->GetSelectedCount() >= 10 )
			{
				eTarotType type = TAROTMAN->GetTarotType();
				if( type == eTAROT_USER )
				{
					/// 운세효과받기 및 재료소모, 요금, 포인트 획득 
					MSG_REQ_TAROT_SEEKER_ACCEPT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_SEEKER_ACCEPT_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_RESULT );
				}
				else if( type == eTAROT_SELF )
				{
					MSGROOT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_SELF_ACCEPT_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_RESULT );
				}
				else if( type == eTAROT_TUTORIAL )
				{
					MSG_REQ_TAROT_TUTORIAL_SEEKER_ACCEPT msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_ACCEPT_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					if( mpMain )
						mpMain->NetLock( ePAGE_RESULT );
				}
				else
					assert(0);
			}
		}
		break;
	case eUIID_GAME_TAROTRESULT_RANDOM:
	case eUIID_GAME_TAROTRESULT_ALLSTR:
	case eUIID_GAME_TAROTRESULT_ALLINV:
		{
			for( ;; )
			{
				if( mpMain->GetSelectedCount() >= 10 )
					break;
				
				if( mpMain->OpenResultCard() == true )
				{
					UpdateButton();
					SetTotalValue();

					if( mpMain->GetSelectedCount() >= 10 )
					{
						/// 결과버튼 활성화
						if( mpResult )
							mpResult->SetEnabled( true );

						/// 버튼 
						ShowButton( false );
					}
				}
			}
		}
		break;
	case eUIID_GAME_TAROTRESULT_ONESTR:
	case eUIID_GAME_TAROTRESULT_ONEINV:
	case eUIID_GAME_TAROTRESULT_RIGHT:
	case eUIID_GAME_TAROTRESULT_UP:
		if( mpMain->OpenResultCard() == true )
		{
			UpdateButton();
			SetTotalValue();

			if( mpMain->GetSelectedCount() >= 10 )
			{
				/// 결과버튼 활성화
				if( mpResult )
					mpResult->SetEnabled( true );

				/// 버튼 
				ShowButton( false );
			}
		}
		break;
	}
}

///
void cTarotResultWindow::OnShow()
{
	///
	cUIWindow::OnShow();

	ShowButton( true );
	SetTotalValue();
	UpdateButton();

	for( unsigned int i=0; i<10; i++ )
	{
		if( mpOpenValue[i] )
			mpOpenValue[i]->Clear();
	}

	SetMaxScore( MAX_TAROTVALUE );
}

///
void cTarotResultWindow::OnHide()
{
	cUIWindow::OnHide();

	/// 초기화
	SetTotalValue();

	if( mpDescription )
		mpDescription->Clear();
	if( mpResult )
		mpResult->SetEnabled( false );
	if( mpResultGauge )
		mpResultGauge->SetMaxValue( 0 );
	if( mpResultGauge )
		mpResultGauge->SetGaugeNoAni( 0 );
}

///
void cTarotResultWindow::SetDescription( LPCTSTR desc )
{
	if( mpDescription )
	{
		mpDescription->SetText( desc );
	}
	else
	{
		assert(0);
	}
}

///
void cTarotResultWindow::SetTotalValue()
{
	unsigned long result = mpMain->GetCurrentResult();
	unsigned long total = mpMain->GetCardTotalPoint();

	if( mpOpenTotal )
		mpOpenTotal->SetValue( total );

	long idx = mpMain->GetSelectedCount() - 1;
	if( mpOpenValue[idx] && idx >= 0 )
		mpOpenValue[idx]->SetValue( result );

	if( mpResultGauge )
		mpResultGauge->SetGaugeNoAni( (float)total );
}

///
void cTarotResultWindow::ShowButton( bool show )
{
	if( mpRightBtn  && mpUpBtn )
	{
		if( show == true )
		{
			mpRightBtn->Show();
			mpUpBtn->Show();
			mpAllRandom->SetEnabled( true );
			mpAllStraight->SetEnabled( true );
			mpAllInverse->SetEnabled( true );
			mpOneStraight->SetEnabled( true );
			mpOneInverse->SetEnabled( true );

		}
		else
		{
			mpRightBtn->Hide();
			mpUpBtn->Hide();
			mpAllRandom->SetEnabled( false );
			mpAllStraight->SetEnabled( false );
			mpAllInverse->SetEnabled( false );
			mpOneStraight->SetEnabled( false );
			mpOneInverse->SetEnabled( false );
		}
	}
	else
	{
		assert(0);
	}
}

///
void cTarotResultWindow::UpdateButton()
{
	if( mpMain && mpUpBtn && mpRightBtn )
	{
		cItemSpread* spread = mpMain->GetCurrentSpreadInfo();
		unsigned int index = mpMain->GetSelectedCount();
		if( spread )
		{
			/// 열어야할 카드 위치에 따라 버튼 이동
			unsigned int x = spread->mParam[index].mX;
			unsigned int y = spread->mParam[index].mY;

			unsigned int cardHW = mpMain->GetCardHalfWidth();
			unsigned int cardHH = mpMain->GetCardHalfHeight();

			x = spread->mParam[index].mX + cardHW - mpUpBtn->GetRelativeRect().GetWidth()/2;
			y = spread->mParam[index].mY - mRelativeRect.mTop - mpUpBtn->GetRelativeRect().GetHeight() - 5;
			cUIPos upPos(x, y);
			mpUpBtn->SetRelativePos( upPos );

			x = spread->mParam[index].mX + cardHW*2 + 5;
			y = spread->mParam[index].mY - mRelativeRect.mTop + cardHH - mpRightBtn->GetRelativeRect().GetHeight()/2;

			cUIPos rightPos(x, y);
			mpRightBtn->SetRelativePos( rightPos );

			/// 버튼이 이동했는데도 캡쳐나 호버가 남아있으므로 삭제 
			UIMAN->ReleaseEventNode();	
		}
	}
	else
	{
		assert(0);
	}
}

/// 타로점괘 최대 수치 세팅
void cTarotResultWindow::SetMaxScore(unsigned int score )
{
	if( mpResultGauge )
	{
		mpResultGauge->SetMaxValue( (float)score );
	}
	else
	{
		assert(0);
	}
}


//-----------------------------------------------------------------------------------------
cNpcTarotResultWindow::cNpcTarotResultWindow()
: mpMain(0)
, mpDesc(0)
, mpQuestion(0)
, mpOpenCardInfo(0)
, mpComment(0)
, mpSpread(0)
, mpResult(0)
, mpRightBtn(0)
, mpUpBtn(0)
, mpStraightBtn(0)
, mpInverseBtn(0)
{
}

cNpcTarotResultWindow::~cNpcTarotResultWindow()
{
}

bool cNpcTarotResultWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpDesc = (cLabel*)GetChild( eUIID_GAME_NPCTAROTRESULT_DESC );
	mpQuestion = (cLabel*)GetChild( eUIID_GAME_NPCTAROTRESULT_QUESTION );
	mpOpenCardInfo = (cLabel*)GetChild( eUIID_GAME_NPCTAROTRESULT_OPENCARDINFO );
	mpComment = (cLabel*)GetChild( eUIID_GAME_NPCTAROTRESULT_COMMENT );
	mpResult = (cButton*)GetChild( eUIID_GAME_NPCTAROTRESULT_RESULT );
	mpRightBtn = (cButton*)GetChild( eUIID_GAME_NPCTAROTRESULT_RIGHT );
	mpUpBtn = (cButton*)GetChild( eUIID_GAME_NPCTAROTRESULT_UP );
	mpStraightBtn = (cButton*)GetChild( eUIID_GAME_NPCTAROTRESULT_STRAIGHT );
	mpInverseBtn  = (cButton*)GetChild( eUIID_GAME_NPCTAROTRESULT_INVERSE );	

	cUINode* backImg = GetChild( eUIID_GAME_NPCTAROTRESULT_COMMENTBACK );
	if( backImg )
		backImg->SetEnabled( false );

	cUINode* descImg = GetChild( eUIID_GAME_NPCTAROTRESULT_DESCBACKIMG );
	if( descImg )
		descImg->SetEnabled( false );

	if( mpDesc )
		mpDesc->SetEnabled( false );
	if( mpQuestion )
		mpQuestion->SetEnabled( false );
	if( mpOpenCardInfo )
		mpOpenCardInfo->SetEnabled( false );
	if( mpComment )
		mpComment->SetEnabled( false );
	if( mpResult )
		mpResult->SetEnabled( false );

	mpMain = (cTarotWindow*)mpParent;
	if( !mpMain )
	{
		assert(0);
		return false;
	}

	return true;
}

void cNpcTarotResultWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	UpdateButton();
}

void cNpcTarotResultWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpSpread )
	{
		sNpcSpreadParam* spreadParam = mpSpread->mParam;
		for( unsigned int i = 0; i < mpSpread->mCount; ++i, ++spreadParam )
		{
			unsigned int x = spreadParam->mX + mpMain->GetAbsoluteRect().mLeft;
			unsigned int y = spreadParam->mY + mpMain->GetAbsoluteRect().mTop;

			cString IndexStr;
			IndexStr.Format( "%d", i + 1 );
			cUIImageFont* pImageFont = UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );
			if( pImageFont )
				pImageFont->DrawText( x + 8, y + 8, IndexStr.Cstr() );
			else
			{
				assert(0);
			}
		}

	}
}

void cNpcTarotResultWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_NPCTAROTRESULT_RESULT:
		{
			/// 운세 받기 
			HERO->CloseNpcTalk();
			TAROTMAN->ChangePage( ePAGE_NONE );

			/// 운세 받을시만 보이기
			GAMEUI->ShowTodayWordWindow();
		}
		break;
	case eUIID_GAME_NPCTAROTRESULT_RIGHT:
	case eUIID_GAME_NPCTAROTRESULT_UP:
	case eUIID_GAME_NPCTAROTRESULT_STRAIGHT:
	case eUIID_GAME_NPCTAROTRESULT_INVERSE:
		{
			if( mpMain->OpenNpcResultCard() == true )
			{
				UpdateButton();

				if( mpMain->GetSelectedCount() >= mpMain->GetMaxCount() )
				{
					if( mpResult )
						mpResult->SetEnabled( true );

					ShowButton( false );
				}
			}
		}
		break;
	case eUIID_GAME_NPCTAROTRESULT_REVIEWLEFT:
		{
			/// 해석 재확인 이전.
			unsigned int arrIndex = mpMain->GetShowReviewIndex();
			if( arrIndex > 0 )
			{
				mpMain->OpenNpcResultReview( arrIndex - 1 );
			}
		}
		break;
	case eUIID_GAME_NPCTAROTRESULT_REVIEWRIGHT:
		if( mpMain->GetSelectedCount() > 0 )
		{
			/// 해석 재확인 다음.
			unsigned int arrIndex = mpMain->GetShowReviewIndex();
			unsigned int max = mpMain->GetSelectedCount() - 1;
			if( arrIndex < max )
			{
				mpMain->OpenNpcResultReview( arrIndex + 1 );
			}
		}
		break;
	}
}

void cNpcTarotResultWindow::OnShow()
{
	cUIWindow::OnShow();

	unsigned long spreadIdx = mpMain->GetSelectedSpreadIndex();
	mpSpread = TAROTMAN->GetNpcSpread( spreadIdx );
	assert(mpSpread);

	ShowButton( true );
	UpdateButton();
}

///
void cNpcTarotResultWindow::OnHide()
{
	cUIWindow::OnHide();

	mpSpread = 0;

	if( mpDesc )
		mpDesc->Clear();
	if( mpQuestion )
		mpQuestion->Clear();
	if( mpOpenCardInfo )
		mpOpenCardInfo->Clear();
	if( mpComment )
		mpComment->Clear();
	if( mpResult ) 
		mpResult->SetEnabled( false );
}

void cNpcTarotResultWindow::SetDesc( LPCTSTR desc )
{
	if( mpDesc )
		mpDesc->SetText( desc );
}

void cNpcTarotResultWindow::SetQuestion( LPCTSTR question )
{
	if( mpQuestion )
		mpQuestion->SetText( question );
}

void cNpcTarotResultWindow::SetCardInfo( LPCTSTR info )
{
	if( mpOpenCardInfo )
		mpOpenCardInfo->SetText( info );
}

void cNpcTarotResultWindow::SetComment( LPCTSTR comment )
{
	if( mpComment )
		mpComment->SetText( comment );
}

void cNpcTarotResultWindow::ShowButton( bool show )
{
	if( mpRightBtn && mpUpBtn )
	{
		if( show == true )
		{
			mpRightBtn->Show();
			mpUpBtn->Show();
			mpStraightBtn->SetEnabled( true );
			mpInverseBtn->SetEnabled( true );
		}
		else
		{
			mpRightBtn->Hide();
			mpUpBtn->Hide();
			mpStraightBtn->SetEnabled( false );
			mpInverseBtn->SetEnabled( false );
		}
	}
	else
	{
		assert(0);
	}
}

void cNpcTarotResultWindow::UpdateButton()
{
	if( mpSpread && mpRightBtn && mpUpBtn )
	{
		unsigned int i = mpMain->GetSelectedCount();
		unsigned int x = mpSpread->mParam[i].mX;
		unsigned int y = mpSpread->mParam[i].mY;

		unsigned int cardHW = mpMain->GetCardHalfWidth();
		unsigned int cardHH = mpMain->GetCardHalfHeight();

		x = mpSpread->mParam[i].mX + cardHW - mpUpBtn->GetRelativeRect().GetWidth()/2;
		y = mpSpread->mParam[i].mY - mRelativeRect.mTop - mpUpBtn->GetRelativeRect().GetHeight() - 5;
		cUIPos upPos(x, y);
		mpUpBtn->SetRelativePos( upPos );

		x = mpSpread->mParam[i].mX + cardHW*2 + 5;
		y = mpSpread->mParam[i].mY - mRelativeRect.mTop + cardHH - mpRightBtn->GetRelativeRect().GetHeight()/2;

		cUIPos rightPos(x, y);
		mpRightBtn->SetRelativePos( rightPos );

		/// 버튼이 이동했는데도 캡쳐나 호버가 남아있으므로 삭제 
		UIMAN->ReleaseEventNode();	
	}
}