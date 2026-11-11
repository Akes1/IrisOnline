#include "StdAfx.h"
#include "bufftooltipwindow.h"

#include "PlaneObject.h"
#include "FontAgent.h"

#include "Skill_Common.h"
#include "GameResourceManager.h"
#include "SkillManager.h"
#include "SkillScript.h"

cBuffToolTipWindow::cBuffToolTipWindow()
{
	mTimeOut = 0;
	mShowTime = true;
	mUniqueIdx = 0;

	SetExceptSnap( true );

	mRightIndex = 0;

	for( unsigned int i = 0; i < MAX_RIGHT_BUFF_TIP; ++i )
	{
		mRightTip[i].mGapCount = 0;
		mRightTip[i].mColor = 0;
	}
}

cBuffToolTipWindow::~cBuffToolTipWindow()
{
}

void cBuffToolTipWindow::Open()
{
}

void cBuffToolTipWindow::Close()
{
	cUIWindow::Close();

	mTimerEnabled = false;
}

void cBuffToolTipWindow::Show( bool onsound )
{
	cTipWindow::Show( onsound );

	mTimerEnabled = true;
}

void cBuffToolTipWindow::Hide( bool onsound )
{
	cTipWindow::Hide( onsound );

	mTimerEnabled = false;
}

void cBuffToolTipWindow::OnTimer()
{
	if( mVisible == false )
		return;

	cInfluenceObject* p = SKILLMAN->GetInfluenceObject( mUniqueIdx );
	if( p == 0 )
		return;

	if( mShowTime == true )
	{
		if( p->GetIsRealTime() == false )
		{
			unsigned int restTime = (unsigned int)(p->GetRestTime() * 0.001f);

			unsigned int d = (restTime / 60) / 60 / 24;
			unsigned int h = (restTime / 60) / 60 % 24;
			unsigned int m = (restTime / 60) % 60;
			unsigned int s = (restTime % 60) % 60 + 1;

			cStringT td,th,tm,ts;
			if( d != 0 )
				td.Format( GAMERESOURCEMAN->GetSkillToolTipText(101206), d );
			if( h != 0 )
				th.Format( GAMERESOURCEMAN->GetSkillToolTipText(100017), h );
			if( m != 0 )
				tm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
			if( s != 0 )
				ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(100015), s );
			if( d == 0 && h == 0 && m == 0 && s == 0 )
				ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(100015), s );

			td += th; td += tm; td += ts;

			unsigned int timeTipIdx = mTimeLine[eTIMELINE_COOLTIME].timeTipIdx;
			mToolTip[timeTipIdx].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(100024), td.Cstr() );
			mToolTip[timeTipIdx].mColor = COLOR_WHITE;
			CalcMaxWidth( mToolTip[timeTipIdx].mString );
		}
	}

	if( mGaurdTipIdx[0] != -1 )
	{
		unsigned long hp = p->GetGuardHP();
		mToolTip[mGaurdTipIdx[0]].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(101203), hp );
		mToolTip[mGaurdTipIdx[0]].mColor = COLOR_WHITE;
		CalcMaxWidth( mToolTip[mGaurdTipIdx[0]].mString );
	}

	if( mGaurdTipIdx[1] != -1 )
	{
		unsigned long cnt = p->GetGuardCnt();
		mToolTip[mGaurdTipIdx[1]].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(101204), cnt );
		mToolTip[mGaurdTipIdx[1]].mColor = COLOR_WHITE;
		CalcMaxWidth( mToolTip[mGaurdTipIdx[1]].mString );
	}
}

void cBuffToolTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	if( mVisible == false )
		return;

	/// 배경
	mMiddleBack->Draw();

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력 
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;

		/// 다음줄 출력 
		unsigned int color = mToolTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );

		py += mLineHeight;
	}

	px = mAbsoluteRect.mRight - 5;
	py = mY;

	for( unsigned int i = 0; i < mRightIndex; ++i )
	{
		cStringT& str = mRightTip[i].mString;
		if( str.IsEmpty() )
			continue;

		unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
		unsigned int x = px - width;

		py += mRightTip[i].mGapCount * GAP_SIZE;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), x, py, mRightTip[i].mColor );

		py += mLineHeight;
	}
}

void cBuffToolTipWindow::Clear()
{
	cTipWindow::Clear();

	for( unsigned int i = 0; i < MAX_RIGHT_BUFF_TIP; ++i )
	{
		mRightTip[i].mString.Clear();
		mRightTip[i].mGapCount = 0;
		mRightTip[i].mColor = 0;
	}

	mRightIndex = 0;
}


bool cBuffToolTipWindow::SetBuff( const cUIPos& pos, unsigned long uniqueIdx, unsigned long influenceIdx, bool showTime )
{
	Clear();

	mShowTime = showTime;
	mUniqueIdx =  uniqueIdx;

	if( SetBuffTip( uniqueIdx, influenceIdx ) == false )
		return false;

	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 최대 너비 제한 설정
	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;

	/// 세팅
	mX = pos.mX + 25;
	mY = pos.mY + 25;

	unsigned int w = mMaxWidth + GAP_SIZE * 2 + mRightWidth + 10;
	unsigned int h = mIndex * mLineHeight + (mGapCount+1)*GAP_SIZE;

	/// 화면 안벗어나게 조정
	if( mX + w > GetScreenWidth() )
	{	
		mX -= ((mX + w) - GetScreenWidth());
	}

	if( mY + h > GetScreenHeight() )
	{
		mY -= ((mY + h) - GetScreenHeight());
	}

	mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );
	SetRelativeRect( cUIRect(mX, mY, mX+w, mY+h) );

	UpdateEdge();
	return true;
}

bool cBuffToolTipWindow::SetBuffTip( unsigned long uniqueIdx, unsigned long influenceIdx )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( pInfo == 0 )
		return false;

	mGaurdTipIdx[0] = -1;
	mGaurdTipIdx[1] = -1;

	AddGap( 1 );

	/// name
	mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameidx );
	mToolTip[mIndex].mColor = COLOR_YELLOW;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}

	cInfluenceObject* p = SKILLMAN->GetInfluenceObject( uniqueIdx );

	/// 081014 PKH p NULL 체크
	if( p != NULL )
	{
		mShowTime = !(p->IsStatic());
		
		if( mShowTime )
		{
			AddGap(1);

			if( p->GetIsRealTime() == false )
			{
				/// 지속시간
				unsigned int restTime = (unsigned int)(p->GetRestTime() * 0.001f);

				unsigned int d = (restTime / 60) / 60 / 24;
				unsigned int h = (restTime / 60) / 60 % 24;
				unsigned int m = (restTime / 60) % 60;
				unsigned int s = (restTime % 60) % 60;

				cStringT td,th,tm,ts;
				if( d != 0 )
					td.Format( GAMERESOURCEMAN->GetSkillToolTipText(101206), d );
				if( h != 0 )
					th.Format( GAMERESOURCEMAN->GetSkillToolTipText(100017), h );
				if( m != 0 )
					tm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
				if( s != 0 )
					ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(100015), s );
				if( d == 0 && h == 0 && m == 0 && s == 0 )
					ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(100015), s );

				td += th; td += tm; td += ts;

				mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(100024), td.Cstr() );
				mToolTip[mIndex].mColor = COLOR_WHITE;
				CalcMaxWidth( mToolTip[mIndex].mString );

				mTimeLine[ eTIMELINE_COOLTIME ].saveCoolTime = mToolTip[mIndex].mString;
				mTimeLine[ eTIMELINE_COOLTIME ].timeTipIdx = mIndex;

				mIndex++;
				if( mIndex > MAX_TIP )
				{
					assert( 0 && "error tip index" );
				}
			}
			else
			{
				TCHAR temp1[128] = {0,};

				// 사용 기한
				struct tm date;

				time_t realTime = p->GetRealTime();
				localtime_s( &date, &realTime );

				date.tm_year = date.tm_year+1900;
				date.tm_mon  = date.tm_mon+1;

				// 101205	"사용 기한: %4d년 %d월 %d일 %d시 %d분"
				::_stprintf_s( temp1, GAMERESOURCEMAN->GetSkillToolTipText( 101205 ), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min );
				SetText( temp1, COLOR_WHITE );
			}
		}

		unsigned long hp = p->GetGuardHP();
		if( hp != 0 )
		{
			AddGap(1);

			mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(101203), hp );
			mToolTip[mIndex].mColor = COLOR_WHITE;
			CalcMaxWidth( mToolTip[mIndex].mString );

			mGaurdTipIdx[0] = mIndex;

			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}

		///
		unsigned long cnt = p->GetGuardCnt();
		if( cnt != 0 )
		{
			AddGap(1);
			mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(101204), cnt );
			mToolTip[mIndex].mColor = COLOR_WHITE;
			CalcMaxWidth( mToolTip[mIndex].mString );

			mGaurdTipIdx[1] = mIndex;

			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}
	}

	/// 설명
	cStringT temp;

	/// 효과값이 있으면 효과출력
	if( pInfo->mValue1 != 0 || pInfo->mValue2 != 0 || pInfo->mValue3 != 0 )
	{
		if( pInfo->mValue1 && pInfo->mValue2 && pInfo->mValue3 )
		{
			AddGap(1);
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2, pInfo->mValue3 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( pInfo->mValue1 && pInfo->mValue2 )
		{
			AddGap(1);
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( pInfo->mValue1 )
		{
			AddGap(1);
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
	}
	else
	{
		/// 일반 툴팁 출력
		AddGap( 1 );
		CheckText( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), COLOR_YELLOW );
	}


	/// 오른쪽 표현
	unsigned int typeIdx = 0;
	switch( pInfo->mTypeDetail )
	{
	case eINFLUENCETYPEDETAIL_AURA:			typeIdx = 100051; break;
	case eINFLUENCETYPEDETAIL_BUILD:		typeIdx = 100050; break;
	case eINFLUENCETYPEDETAIL_MAGIC:		typeIdx = 100052; break;

	case eINFLUENCETYPEDETAIL_TRAUMA:		typeIdx = 100054; break;
	case eINFLUENCETYPEDETAIL_MAGICDEBUFF:	typeIdx = 100055; break;
	case eINFLUENCETYPEDETAIL_POISON:		typeIdx = 100056; break;
	case eINFLUENCETYPEDETAIL_AFTEREFFECT:	typeIdx = 100057; break;
	case eINFLUENCETYPEDETAIL_LEADER:		typeIdx = 100058; break;
	case eINFLUENCETYPEDETAIL_NOPROPERTY:	typeIdx = 100059; break;

	case eINFLUENCETYPEDETAIL_POTION:		typeIdx = 100053; break;
	case eINFLUENCETYPEDETAIL_GOODS:		typeIdx = 100061; break;	

	case eINFLUENCETYPEDETAIL_COOKFULL:			typeIdx = 100071; break;
	case eINFLUENCETYPEDETAIL_COOKSATISFACTION:	typeIdx = 100072; break;
	case eINFLUENCETYPEDETAIL_COOKFLAVOR:		typeIdx = 100073; break;

	case eINFLUENCETYPEDETAIL_ALCHEMYPOTION:	typeIdx = 100081; break;
	case eINFLUENCETYPEDETAIL_ALCHEMYPROMOTION:	typeIdx = 100082; break;
	case eINFLUENCETYPEDETAIL_ALCHEMYENHANCE:	typeIdx = 100083; break;
	case eINFLUENCETYPEDETAIL_ALCHEMYDRUG:		typeIdx = 100084; break;

	case eINFLUENCETYPEDETAIL_CARD:			typeIdx = 100060; break;
	case eINFLUENCETYPEDETAIL_TAROT:		typeIdx = 100085; break;

	case eINFLUENCETYPEDETAIL_NAME:			typeIdx = 100901; break;
	case eINFLUENCETYPEDETAIL_VEHICLE:		typeIdx = 100902; break;

	case eINFLUENCETYPEDETAIL_ACTIVEWEAPON:		typeIdx = 100911; break;
	case eINFLUENCETYPEDETAIL_ACTIVEWEAR:		typeIdx = 100912; break;

	case eINFLUENCETYPEDETAIL_GATHERING:		typeIdx = 100921; break;
	case eINFLUENCETYPEDETAIL_DUEL:				typeIdx = 100922; break;
	case eINFLUENCETYPEDETAIL_QUEST:			typeIdx = 100923; break;
	case eINFLUENCETYPEDETAIL_PVP:				typeIdx = 100924; break;
		
	case eINFLUENCETYPEDETAIL_CASH:				typeIdx = 100931; break;
	case eINFLUENCETYPEDETAIL_PC:				typeIdx = 100941; break;
	case eINFLUENCETYPEDETAIL_EVENT:			typeIdx = 100951; break;
	case eINFLUENCETYPEDETAIL_GM:				typeIdx = 100961; break;
	default: assert(0); break;
	}
		
	mRightTip[mRightIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText( typeIdx );
	mRightTip[mRightIndex].mColor = COLOR_YELLOW;
	mRightTip[mRightIndex].mGapCount = 2;

	mRightWidth = 0;
	if( mRightTip[mRightIndex].mString.IsEmpty() == false )
		mRightWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mRightTip[mRightIndex].mString.Cstr(), mRightTip[mRightIndex].mString.GetLength() );

	mRightIndex++;

	// pc방, 이벤트, gm, 호칭 제외
	if( pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_PC || pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_EVENT || 
		pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_GM || pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_NAME )
		return true;

	// 효력값 출력
	cStringT groupOrder;
	groupOrder.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100025 ), pInfo->mGroupOrder );

	mRightTip[mRightIndex].mString = groupOrder;
	mRightTip[mRightIndex].mColor = COLOR_YELLOW;
	mRightTip[mRightIndex].mGapCount = 0;

	if( mRightTip[mRightIndex].mString.IsEmpty() == false )
	{
		unsigned int rightWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mRightTip[mRightIndex].mString.Cstr(), mRightTip[mRightIndex].mString.GetLength() );
		if( rightWidth > mRightWidth )
			mRightWidth = rightWidth;
	}

	mRightIndex++;
	return true;
}
