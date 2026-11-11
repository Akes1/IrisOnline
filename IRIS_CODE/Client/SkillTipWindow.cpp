#include "StdAfx.h"
#include "SkillTipWindow.h"
#include "FontAgent.h"
#include "PlaneObject.h"
#include "Skill_Common.h"
#include "SkillScript.h"
#include "SkillPreviewWindow.h"
#include "GameUIManager.h"
#include "GameResourceManager.h"

#include "ObjectManager.h"
#include "Hero.h"
#include "StatusScript.h"
#include "SkillManager.h"
#include "ItemManager.h"
#include "TotemScript.h"
#include "InputSystem.h"

const unsigned int mSecondColWidth = 80;

cSkillTipWindow::cSkillTipWindow()
: mRightIndex(0)
, mRightMaxWidth(0)
, mRequireIndex(0)
, mSaveSkillIndex(0)
, mRightGapCount(0)
{
	mLimitMaxWidth = 300;

	mBottomIndex = 0;
	mBottomRightIndex = 0;

	SetExceptSnap( true );
}

cSkillTipWindow::~cSkillTipWindow()
{

}
void cSkillTipWindow::Clear()
{
	cTipWindow::Clear();

	for( unsigned int i=0; i<MAX_RIGHT_TIP; i++ )
		mRightTip[i].mGapCount = 0;

	mRightIndex = 0;
	mRightMaxWidth = 0;
	mRightGapCount = 0;

	/// 하단 표현을 위한 변수
	mBottomIndex = 0;
	mBottomRightIndex = 0;

	/// 조건 표현
	for( unsigned int i=0; i<MAX_RIGHT_TIP; i++ )
	{
		mRequireValue[i][0].mString.Clear();
		mRequireValue[i][1].mString.Clear();
		mRequireValue[i][2].mString.Clear();
		mRequireValue[i][3].mString.Clear();

		mRequireValue[i][0].mGapCount = 0;
		mRequireValue[i][1].mGapCount = 0;
		mRequireValue[i][2].mGapCount = 0;
		mRequireValue[i][3].mGapCount = 0;
	}
	mRequireIndex = 0;

	mSaveSkillIndex = 0;
	
	mUpdateSizebyTime = false;
}

void cSkillTipWindow::Open()
{
}

void cSkillTipWindow::Close()
{
	cUIWindow::Close();
}

void cSkillTipWindow::OnHide()
{
	cUIWindow::OnHide();

	mTimerEnabled = false;
	mSaveSkillIndex = 0;
}

void cSkillTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	if( mVisible == false )
		return;

	/// 배경
	if( mMiddleBack )
		mMiddleBack->Draw();

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력
	unsigned int bottomStartY = 0;
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;
		if( mBottomIndex != 0 &&row == mBottomIndex )
		{
			/// 하단 찍기
			unsigned int width = mMaxWidth + mRightMaxWidth + GAP_SIZE * 2 + 20;
			int x = px + width/2 - 6;

			fontKeeper->AddFontItem( cFontAgent::eFont_UI, _T("▽"), x, py, COLOR_WHITE );

			py += mToolTip[row].mGapCount * GAP_SIZE;
			bottomStartY = py;
		}

		/// 다음줄 출력 
		unsigned long color = mToolTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );

		py += mLineHeight;
	}

	///
	for( unsigned int row = 0; row < mRequireIndex; ++row )
	{
		unsigned int x = px;
		py += mRequireValue[row][0].mGapCount * GAP_SIZE;
		for( unsigned int i=0; i<4; i++ )
		{
			cStringT& str = mRequireValue[row][i].mString;
			if( str.IsEmpty() )
				continue;

			unsigned int color = mRequireValue[row][i].mColor;
			fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), x, py, color );

			x += FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
		}
		py += mLineHeight;
	}

	///
	px = mX + GAP_SIZE + mMaxWidth + mRightMaxWidth + 20;
	py = mY;
	for( unsigned int row = 0; row < mRightIndex; ++row )
	{
		cStringT& str = mRightTip[row].mString;
		if( str.IsEmpty() )
			continue;

		/// 오른쪽 정렬 계산
		unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
		unsigned int x = px - width;
		if( mSaveSkillIndex > 0 )
			x -= 4;

		py += mRightTip[row].mGapCount * GAP_SIZE;
		if( mBottomRightIndex != 0 && row == mBottomRightIndex )
		{
			py = bottomStartY;
		}

		unsigned int color = mRightTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), x, py, color );

		py += mLineHeight;
	}

	///
	fontKeeper->DrawAll();
}

void cSkillTipWindow::CalcRightMaxWidth( const cStringT& str )
{
	unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
	if( width > mRightMaxWidth )
		mRightMaxWidth = width;

	if( mRightMaxWidth > mSecondColWidth )
		mRightMaxWidth = mSecondColWidth;
}

bool cSkillTipWindow::SetSkill( eTip_Kind tipkind, const cUIPos& pos, unsigned long skillIndex, unsigned char skillStep, unsigned long jobIdx )
{
	/// 
	Clear();

	if( tipkind == eTIP_SKILL )
	{
		if( SetNormalTip( skillIndex, skillStep, jobIdx ) == false )
			return false;
	}
	else if( tipkind == eTIP_QUICK )
	{
		if( SetQuickTip( skillIndex, skillStep, jobIdx ) == false )
			return false;
	}
	else if( tipkind == eTIP_SKILLPREVIEW )
	{
		if( SetSkillPreviewTip( skillIndex, skillStep, jobIdx ) == false )
			return false;
	}

	SetBackImg( pos );	
	return true;
}

void cSkillTipWindow::SetBackImg( const cUIPos& pos )
{
	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 세팅
	mX = pos.mX + 25;
	mY = pos.mY + 25;
	unsigned int w = mMaxWidth + mRightMaxWidth + GAP_SIZE * 2 + 20; 
	unsigned int h = (mIndex+mRequireIndex)*mLineHeight + (mGapCount+1)*GAP_SIZE;
	if( mBottomIndex != 0 )
	{
		h += mToolTip[mBottomIndex].mGapCount * GAP_SIZE;
	}

	unsigned int count = 0;
	for( unsigned int i=0;i<mRequireIndex;i++ )
	{
		count += mRequireValue[i][0].mGapCount;
	}
	h += count * GAP_SIZE;

	/// 화면 안벗어나게 조정
	if( mX + w > GetScreenWidth() )
	{	
		mX -= ((mX + w) - GetScreenWidth());
	}

	if( mY + h > GetScreenHeight() )
	{
		mY -= ((mY + h) - GetScreenHeight());
	}

	/// 스킬재사용시간 출력시 여유분 추가
	if( mSaveSkillIndex > 0 )
	{
		if( w <= 280 )
			w = 280;
	}

	if( mMiddleBack )
		mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );

	SetRelativeRect( cUIRect(mX, mY, mX+w, mY+h) );

	UpdateEdge();
}

/// 습득하고자 하는 스킬 단계 인덱스 
bool cSkillTipWindow::SetNormalTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx )
{
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		return false;
	}
	unsigned int bottomgapcount = 0;
	unsigned int rightbottomgapcount = 0;

	/// 이름
	AddGap( 1 );
	SetName( pInfo, COLOR_YELLOW );

	AddGap( 2 );

	/// 현재 스텝 정보를 표현한다.
	if( skillStep != UCHAR_MAX )
	{
		/// 시전 유형
		SetShotType( pInfo, COLOR_WHITE );

		unsigned int curStep = (unsigned int)skillStep;
		if( pInfo->mStepCount <= curStep )
		{
			assert(0);
		}

		sPlayerSkillStepInfo curStepInfo = pInfo->mpSetpInfoArray[curStep];

		/// 사용 장비
		SetEquip( pInfo, COLOR_WHITE );

		/// 시전시간
		SetCastingTime( pInfo, &curStepInfo, COLOR_WHITE );

		/// 재사용시간
		SetCoolTime( &curStepInfo, COLOR_WHITE, skillIdx );

		/// 재사용 시간 공유
		SetCoolTimeShareList( pInfo, &curStepInfo, jobIdx, COLOR_WHITE );

		/// 소모 비용
		SetUsedValue( pInfo, &curStepInfo, COLOR_WHITE );

		///  범위 방식
		SetRangeType( pInfo, COLOR_WHITE );

		cStringT temp;
		if( curStepInfo.mApplyValue1 != 0 || curStepInfo.mApplyValue2 != 0 )
		{
			if( curStepInfo.mApplyValue1 != 0 && curStepInfo.mApplyValue2 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1, curStepInfo.mApplyValue2 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
			else if( curStepInfo.mApplyValue1 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
		}
		else
		{
			/// 일반 툴팁 출력
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ) );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}

		/// 효과
		bool isInfluence = false;
		unsigned int influenceTipIndex = mIndex;
		unsigned int gapcount = 0;
		if( pInfo->mType == eSKILLTYPE_TOTEM )
		{
			sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( curStepInfo.mInfulenceIdx );
			if( totem )
				isInfluence = SetInfluence( totem->mInfluenceIdx );
		}
		else
		{
			unsigned long infIdx = curStepInfo.mInfulenceIdx;

			if( pInfo->mShotType == eSHOTTYPE_RUSHRUN || pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
			{
				infIdx = 0;

				if( curStepInfo.mInfulenceIdx != 0 )
				{
					sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( curStepInfo.mInfulenceIdx );
					if( pInfo2 == 0 )
						return false;

					unsigned int curStep = (unsigned int)skillStep;
					if( pInfo2->mStepCount <= curStep )
					{
						assert(0);
						return false;
					}

					sPlayerSkillStepInfo curStepInfo2 = pInfo2->mpSetpInfoArray[curStep];
					infIdx = curStepInfo2.mInfulenceIdx;
				}
			}

			isInfluence = SetInfluence( infIdx );			
		}

		if( isInfluence == true )
		{		
			gapcount = mGapCount;
		}

		SetNoOverLapInfluenceList( pInfo, &curStepInfo, jobIdx, COLOR_WHITE );
		
		bool isActiveEffect = false;
		unsigned int activeTipIndex = mIndex;
		unsigned int activegapcount = 0;
		isActiveEffect = SetActiveEffect( pInfo, &curStepInfo, COLOR_WHITE );
		if( isActiveEffect )
		{			
			activegapcount = mGapCount;
		}

		/// 오른쪽 툴팁
		AddRightGap( 2 );
		SetRightAttribute( pInfo, COLOR_WHITE ); 

		AddRightGap( 1 );
		SetRightStepInfo( pInfo, curStep, COLOR_WHITE ); 
		SetRightTargetDist( pInfo, &curStepInfo, COLOR_WHITE, skillIdx ); 
		SetRightBound( pInfo, &curStepInfo, COLOR_WHITE );
		SetRightApplyCnt( pInfo, &curStepInfo, COLOR_WHITE );

		if( isInfluence == true )
		{
			long leftHeight = (mLineHeight * influenceTipIndex) + (gapcount * GAP_SIZE);
			long rightHeight = (mLineHeight * mRightIndex) + (mRightGapCount * GAP_SIZE);
			unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;
						
			AddRightGap( addGapCount );
			SetRightInfluence( pInfo, &curStepInfo, curStep, COLOR_WHITE );
		}		

		if( isActiveEffect == true )
		{
			long leftHeight = (mLineHeight * activeTipIndex) + (activegapcount * GAP_SIZE);
			long rightHeight = (mLineHeight * mRightIndex) + (mRightGapCount * GAP_SIZE);
			unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

			AddRightGap( addGapCount );
			SetRightActiveEffect( &curStepInfo, COLOR_WHITE );
		}
			
		mBottomIndex = mIndex;
		mBottomRightIndex = mRightIndex;

		bottomgapcount			= mGapCount;
		rightbottomgapcount	= mRightGapCount;

		/// 라인 띄기 
		if( pInfo->mStepCount > (unsigned int)(skillStep + 1) )
			AddGap(2);
		else
			return true;
	}

	// 배열 정보 이므로 실제 스텝단계보다 하나 작다
	unsigned int nextStep = 0;
	if( skillStep != UCHAR_MAX )
	{
		nextStep = skillStep + 1;
	}
	
	if( pInfo->mStepCount <= nextStep )
	{
		assert(0);
	}

	sPlayerSkillStepInfo trainStepInfo = pInfo->mpSetpInfoArray[nextStep];

	/// 시전 유형
	SetShotType( pInfo, COLOR_WHITE );

	/// 사용 장비
	SetEquip( pInfo, COLOR_WHITE );

	/// 시전시간
	SetCastingTime( pInfo, &trainStepInfo, COLOR_WHITE );

	/// 재사용시간
	SetCoolTime( &trainStepInfo, COLOR_WHITE, 0 );

	/// 재사용 시간 공유 리스트
	SetCoolTimeShareList( pInfo, &trainStepInfo, jobIdx, COLOR_WHITE );

	/// 소모 비용
	SetUsedValue( pInfo, &trainStepInfo, COLOR_WHITE );

	///  범위 방식
	SetRangeType( pInfo, COLOR_WHITE );

	cStringT temp;
	if( trainStepInfo.mApplyValue1 != 0 || trainStepInfo.mApplyValue2 != 0 )
	{
		if( trainStepInfo.mApplyValue1 != 0 && trainStepInfo.mApplyValue2 != 0 )
		{
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1, trainStepInfo.mApplyValue2 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( trainStepInfo.mApplyValue1 != 0 )
		{
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
	}
	else
	{
		/// 일반 툴팁 출력
		AddGap( 1 );
		temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1 );
		CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
	}

	/// 효과
	bool isInfluence = false;
	unsigned int influenceTipIndex = mIndex;
	unsigned int gapcount = 0;
	if( pInfo->mType == eSKILLTYPE_TOTEM )
	{
		sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( trainStepInfo.mInfulenceIdx );
		if( totem )
		{
			isInfluence = SetInfluence( totem->mInfluenceIdx );
		}
	}
	else
	{
		unsigned long infIdx = trainStepInfo.mInfulenceIdx;

		if( pInfo->mShotType == eSHOTTYPE_RUSHRUN || pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
		{
			infIdx = 0;

			if( trainStepInfo.mInfulenceIdx != 0 )
			{
				sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mInfulenceIdx );
				if( pInfo2 == 0 )
					return false;

				unsigned int trainStep = (unsigned int)nextStep;
				if( pInfo2->mStepCount <= trainStep )
				{
					assert(0);
					return false;
				}

				sPlayerSkillStepInfo trainStepInfo2 = pInfo2->mpSetpInfoArray[trainStep];
				infIdx = trainStepInfo2.mInfulenceIdx;
			}
		}

		isInfluence = SetInfluence( infIdx );
	}

	if( isInfluence == true )
	{
		gapcount = mGapCount;
	}

	SetNoOverLapInfluenceList( pInfo, &trainStepInfo, jobIdx, COLOR_WHITE );

	bool isActiveEffect = false;
	unsigned int activeTipIndex = mIndex;
	unsigned int activegapcount = 0;
	isActiveEffect = SetActiveEffect( pInfo, &trainStepInfo, COLOR_WHITE );
	if( isActiveEffect )
	{			
		activegapcount = mGapCount;
	}
	
	/// 오른쪽 툴팁
	AddRightGap( 2 );

	if( nextStep == 0 )		
	{
		/// 미습득 표현
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 101050 ), nextStep, pInfo->mStepCount );
		mRightTip[mRightIndex].mColor = COLOR_RED;		
	}
	else
	{
		/// 다음 단계 표현
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100010 ), nextStep + 1, pInfo->mStepCount );
		mRightTip[mRightIndex].mColor = COLOR_WHITE;
	}

	CalcRightMaxWidth( mRightTip[mRightIndex].mString );
	mRightIndex++;
	if( mRightIndex> MAX_RIGHT_TIP )
	{
		assert( 0 && "error tip index" );
	}

	SetRightTargetDist( pInfo, &trainStepInfo, COLOR_WHITE, skillIdx ); 
	SetRightBound( pInfo, &trainStepInfo, COLOR_WHITE ); 
	SetRightApplyCnt( pInfo, &trainStepInfo, COLOR_WHITE );

	if( isInfluence == true )
	{
		long leftHeight = (mLineHeight * (influenceTipIndex- mBottomIndex)) + ((gapcount - bottomgapcount )* GAP_SIZE);
		long rightHeight = (mLineHeight * (mRightIndex- mBottomRightIndex)) + ((mRightGapCount - rightbottomgapcount) * GAP_SIZE);
		unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

		AddRightGap( addGapCount );
		SetRightInfluence( pInfo, &trainStepInfo, nextStep, COLOR_WHITE );
	}		

	if( isActiveEffect == true )
	{		
		long leftHeight = (mLineHeight * (activeTipIndex- mBottomIndex)) + ((activegapcount - bottomgapcount )* GAP_SIZE);
		long rightHeight = (mLineHeight * (mRightIndex- mBottomRightIndex)) + ((mRightGapCount - rightbottomgapcount) * GAP_SIZE);
		unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

		AddRightGap( addGapCount );		
		SetRightActiveEffect( &trainStepInfo, COLOR_WHITE );
	}	

	/// 조건 설명 표현
	{
		/// 필요 레벨
		if( trainStepInfo.mPlayerLevel > HERO->GetLevel() )
		{
			mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101001 ), trainStepInfo.mPlayerLevel );
			mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
			mRequireValue[mRequireIndex][0].mGapCount = 1;

			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][3].mString.Clear();

			mRequireIndex++;
		}

		/// 필요 직업
		bool allowJob = false;
		tArray<unsigned long>* pArray = SKILLSCRIPT->GetPlayerSkillJob( trainStepInfo.mJobType );
		if( pArray )
		{
			if( pArray->GetSize() > 3 )
			{
				assert(0);
				return false;
			}

			mRequireValue[mRequireIndex][0].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100021 );
			mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
			mRequireValue[mRequireIndex][0].mGapCount = 1;

			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][3].mString.Clear();

			tArray<unsigned long>* playerJobParent = GAMERESOURCEMAN->GetParentJob( HERO->GetJob() );
			if( playerJobParent == 0 )
			{
				/// 상위 직업 없는 경우
				for( unsigned int i = 0 ; i < pArray->GetSize() ; ++i )
				{
					mRequireValue[mRequireIndex][i+1].mString = GAMERESOURCEMAN->GetJobName( (*pArray)[i] );

					if( (*pArray)[i] == (unsigned long)HERO->GetJob() )
					{
						/// 글자 색깔을 흰색으로
						mRequireValue[mRequireIndex][i+1].mColor = COLOR_WHITE;
						mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;

						allowJob = true;
						break;
					}
					else
					{
						/// 글자 색깔을 빨강으로
						mRequireValue[mRequireIndex][i+1].mColor = COLOR_RED;
					}

					if( i+1 < pArray->GetSize() )
						mRequireValue[mRequireIndex][i+1].mString += _T(", ");
				}
			}
			else
			{
				/// 모든 상위 직업 검사
				for( unsigned int i = 0 ; i < pArray->GetSize() ; ++i )
				{
					mRequireValue[mRequireIndex][i+1].mString = GAMERESOURCEMAN->GetJobName( (*pArray)[i] );
					mRequireValue[mRequireIndex][i+1].mColor = COLOR_RED;

					if( (*pArray)[i] == (unsigned long)HERO->GetJob() )
					{
						allowJob = true;
						break;
					}

					for( unsigned int j = 0 ; j < playerJobParent->GetSize() ; ++j )
					{
						if( (*pArray)[i] == (*playerJobParent)[j] )
						{
							mRequireValue[mRequireIndex][i+1].mColor = COLOR_WHITE;
							mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;

							allowJob = true;
							break;
						}
					}
					if( i+1 < pArray->GetSize() )
						mRequireValue[mRequireIndex][i+1].mString += _T(", ");
				}
			}
		}
		/// 
		if( allowJob == false )
		{
			mRequireIndex++;

			if( mRequireIndex > MAX_RIGHT_TIP )
				assert( 0 && "error tip index" );
		}

		/// 선행 스킬
		mRequireValue[mRequireIndex][0].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100022 );
		mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
		mRequireValue[mRequireIndex][1].mColor = COLOR_RED;
		mRequireValue[mRequireIndex][2].mColor = COLOR_RED;

		/// 선행스킬 1
		bool allowLearn1 = false;
		if( trainStepInfo.mLearnSkillIdx1 )
		{
			sPlayerSkillBaseInfo* pLearn = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mLearnSkillIdx1 );
			if( pLearn )
			{
				LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pLearn->mNameIdx );
				mRequireValue[mRequireIndex][1].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100023 ), name, trainStepInfo.mLearnSkillStep1 );
				sKeepSkill* p = SKILLMAN->GetKeepInfo( trainStepInfo.mLearnSkillIdx1 );
				if( p )
				{
					if( p->mStep + 1 >= trainStepInfo.mLearnSkillStep1 )
					{
						mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
						allowLearn1 = true;
					}
				}

				if( trainStepInfo.mLearnSkillIdx2 )
					mRequireValue[mRequireIndex][1].mString += _T(", ");
			}
			else
			{
				mRequireValue[mRequireIndex][1].mString = _T("Error");
				mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
			}

			//allowLearn1 = false;
		}
		else
		{
			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
			allowLearn1 = true;
		}

		/// 선행스킬 2
		bool allowLearn2 = false;
		if( trainStepInfo.mLearnSkillIdx2 )
		{
			sPlayerSkillBaseInfo* pLearn = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mLearnSkillIdx2 );
			if( pLearn )
			{
				LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pLearn->mNameIdx );
				mRequireValue[mRequireIndex][2].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100023 ), name, trainStepInfo.mLearnSkillStep2 );
				sKeepSkill* p = SKILLMAN->GetKeepInfo( trainStepInfo.mLearnSkillIdx2 );
				if( p )
				{
					if( p->mStep + 1 >= trainStepInfo.mLearnSkillStep2 )
					{
						mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
						allowLearn2 = true;
					}
				}
			}
			else
			{
				mRequireValue[mRequireIndex][2].mString = _T("Error");
				mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
			}

			//allowLearn2 = false;
		}
		else
		{
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
			allowLearn2 = true;
		}

		/// 
		if( allowLearn1 == true && allowLearn2 == true )
		{
			mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;

			cStringT tempStr;
			tempStr += mRequireValue[mRequireIndex][0].mString;
			tempStr += mRequireValue[mRequireIndex][1].mString;
			tempStr += mRequireValue[mRequireIndex][2].mString;

			CalcMaxWidth( tempStr );
		}

		if( trainStepInfo.mLearnSkillIdx1 || trainStepInfo.mLearnSkillIdx2 )
		{
			if( allowLearn1 == false || allowLearn2 == false )
			{
				cStringT tempStr;
				tempStr += mRequireValue[mRequireIndex][0].mString;
				tempStr += mRequireValue[mRequireIndex][1].mString;
				tempStr += mRequireValue[mRequireIndex][2].mString;
				CalcMaxWidth( tempStr );

				mRequireIndex++;
				if( mRequireIndex > MAX_RIGHT_TIP )
					assert( 0 && "error tip index" );
			}
		}

		/// 습득 SP 비용
		/// 마스터 여부 체크
		if ( pInfo->mStepCount != nextStep)
		{
			sPlayerSkillStepInfo stepInfo = pInfo->mpSetpInfoArray[nextStep];
			unsigned int requireSP = stepInfo.mRequireSP;

			mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101009 ), requireSP );
			mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
			mRequireValue[mRequireIndex][0].mGapCount = 1;

			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][3].mString.Clear();

			if( requireSP <= HERO->GetSkillPointRemain() )
			{
				mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;
			}

			mRequireIndex++;
		}

		/// 사용 SP량
		if( pInfo->mJobStep != 0 && pInfo->mJobStep != 9 )
		{
			unsigned long point = HERO->GetJobUsedSkillPoint( pInfo->mJobStep-1 );
			if( point < pInfo->mBeforeReqSP )
			{
				mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101008 ), pInfo->mBeforeReqSP );
				mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
				mRequireValue[mRequireIndex][0].mGapCount = 1;

				mRequireValue[mRequireIndex][1].mString.Clear();
				mRequireValue[mRequireIndex][2].mString.Clear();
				mRequireValue[mRequireIndex][3].mString.Clear();

				mRequireIndex++;
			}
		}

	}

	return true;
}

/// 퀵슬롯 팁
bool cSkillTipWindow::SetQuickTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx  )
{
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		return false;
	}

	/// 이름
	AddGap( 1 );
	SetName( pInfo, COLOR_YELLOW );

	AddGap( 2 );
	if( skillStep != UCHAR_MAX )
	{
		/// 현재 스텝 정보를 표현한다.
		unsigned int curStep = (unsigned int)skillStep;

		if( pInfo->mStepCount <= curStep )
		{
			assert(0);
		}

		sPlayerSkillStepInfo curStepInfo = pInfo->mpSetpInfoArray[curStep];

		/// 사용 장비
		SetEquip( pInfo, COLOR_WHITE );

		/// 시전시간
		SetCastingTime( pInfo, &curStepInfo, COLOR_WHITE );

		/// 재사용시간
		SetCoolTime( &curStepInfo, COLOR_WHITE, skillIdx );

		/// 재사용 시간 공유
		SetCoolTimeShareList( pInfo, &curStepInfo, jobIdx, COLOR_WHITE );

		/// 소모 비용
		SetUsedValue( pInfo, &curStepInfo, COLOR_WHITE );

		///  범위 방식
		SetRangeType( pInfo, COLOR_WHITE );

		cStringT temp;
		if( curStepInfo.mApplyValue1 != 0 || curStepInfo.mApplyValue2 != 0 )
		{
			if( curStepInfo.mApplyValue1 != 0 && curStepInfo.mApplyValue2 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1, curStepInfo.mApplyValue2 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
			else if( curStepInfo.mApplyValue1 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
		}
		else
		{
			/// 일반 툴팁 출력
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ) );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}		

		/// 오른쪽 툴팁
		AddRightGap( 2 );
		SetRightAttribute( pInfo, COLOR_WHITE ); 

		AddRightGap( 1 );
		SetRightStepInfo( pInfo, curStep, COLOR_WHITE ); 
		SetRightTargetDist( pInfo, &curStepInfo, COLOR_WHITE, skillIdx ); 
		SetRightBound( pInfo, &curStepInfo, COLOR_WHITE ); 
		SetRightApplyCnt( pInfo, &curStepInfo, COLOR_WHITE );		

		mBottomIndex = mIndex;
		mBottomRightIndex = mRightIndex;
	}
	return true;
}

/// 습득하고자 하는 스킬 단계 인덱스 
bool cSkillTipWindow::SetSkillPreviewTip( unsigned long skillIdx, unsigned char skillStep, unsigned long jobIdx )
{
	cSkillPreviewWindow* win = GAMEUI->GetSkillPreviewWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		return false;
	}

	unsigned int bottomgapcount = 0;
	unsigned int rightbottomgapcount = 0;
	/// 이름
	AddGap( 1 );
	SetName( pInfo, COLOR_YELLOW );

	AddGap( 2 );
	if( skillStep != UCHAR_MAX )	/// 현재 스텝 정보를 표현한다.
	{
		/// 시전 유형
		SetShotType( pInfo, COLOR_WHITE );

		unsigned int curStep = (unsigned int)skillStep;
		if( pInfo->mStepCount <= curStep )
		{
			assert(0);
		}

		sPlayerSkillStepInfo curStepInfo = pInfo->mpSetpInfoArray[curStep];

		/// 사용 장비
		SetEquip( pInfo, COLOR_WHITE );

		/// 시전시간
		SetCastingTime( pInfo, &curStepInfo, COLOR_WHITE );

		/// 재사용시간
		SetCoolTime( &curStepInfo, COLOR_WHITE, skillIdx );

		/// 재사용 시간 공유
		SetCoolTimeShareList( pInfo, &curStepInfo, jobIdx, COLOR_WHITE );

		/// 소모 비용
		SetUsedValue( pInfo, &curStepInfo, COLOR_WHITE );

		///  범위 방식
		SetRangeType( pInfo, COLOR_WHITE );

		cStringT temp;
		if( curStepInfo.mApplyValue1 != 0 || curStepInfo.mApplyValue2 != 0 )
		{
			if( curStepInfo.mApplyValue1 != 0 && curStepInfo.mApplyValue2 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1, curStepInfo.mApplyValue2 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
			else if( curStepInfo.mApplyValue1 != 0 )
			{
				AddGap( 1 );
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ), curStepInfo.mApplyValue1 );
				CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
			}
		}
		else
		{
			/// 일반 툴팁 출력
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( curStepInfo.mToolTipIdx ) );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}

		/// 효과
		bool isInfluence = false;
		unsigned int influenceTipIndex = mIndex;
		unsigned int gapcount = 0;
		if( pInfo->mType == eSKILLTYPE_TOTEM )
		{
			sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( curStepInfo.mInfulenceIdx );
			if( totem )
			{
				isInfluence = SetInfluence( totem->mInfluenceIdx );
			}
		}
		else
		{
			unsigned long infIdx = curStepInfo.mInfulenceIdx;

			if( pInfo->mShotType == eSHOTTYPE_RUSHRUN || pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
			{
				infIdx = 0;

				if( curStepInfo.mInfulenceIdx != 0 )
				{
					sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( curStepInfo.mInfulenceIdx );
					if( pInfo2 == 0 )
						return false;

					unsigned int curStep = (unsigned int)skillStep;
					if( pInfo2->mStepCount <= curStep )
					{
						assert(0);
						return false;
					}

					sPlayerSkillStepInfo curStepInfo2 = pInfo2->mpSetpInfoArray[curStep];
					infIdx = curStepInfo2.mInfulenceIdx;
				}
			}

			isInfluence = SetInfluence( infIdx );
		}		

		if( isInfluence == true )
		{
			gapcount = mGapCount;
		}
		
		SetNoOverLapInfluenceList( pInfo, &curStepInfo, jobIdx, COLOR_WHITE );

		bool isActiveEffect = false;
		unsigned int activeTipIndex = mIndex;
		unsigned int activegapcount = 0;
		isActiveEffect = SetActiveEffect( pInfo, &curStepInfo, COLOR_WHITE );
		if( isActiveEffect )
		{			
			activegapcount = mGapCount;
		}

		/// 오른쪽 툴팁
		AddRightGap( 2 );
		SetRightAttribute( pInfo, COLOR_WHITE ); 

		AddRightGap( 1 );
		SetRightStepInfo( pInfo, curStep, COLOR_WHITE ); 
		SetRightTargetDist( pInfo, &curStepInfo, COLOR_WHITE, skillIdx ); 
		SetRightBound( pInfo, &curStepInfo, COLOR_WHITE ); 
		SetRightApplyCnt( pInfo, &curStepInfo, COLOR_WHITE );
		
		if( isInfluence == true )
		{
			long leftHeight = (mLineHeight * influenceTipIndex) + (gapcount * GAP_SIZE);
			long rightHeight = (mLineHeight * mRightIndex) + (mRightGapCount * GAP_SIZE);
			unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

			AddRightGap( addGapCount );
			SetRightInfluence( pInfo, &curStepInfo, curStep, COLOR_WHITE );
		}		

		if( isActiveEffect == true )
		{
			long leftHeight = (mLineHeight * activeTipIndex) + (activegapcount * GAP_SIZE);
			long rightHeight = (mLineHeight * mRightIndex) + (mRightGapCount * GAP_SIZE);
			unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

			AddRightGap( addGapCount );		
			SetRightActiveEffect( &curStepInfo, COLOR_WHITE );
		}	

		mBottomIndex = mIndex;
		mBottomRightIndex = mRightIndex;

		bottomgapcount			= mGapCount;
		rightbottomgapcount	= mRightGapCount;

		/// 라인 띄기 
		if( pInfo->mStepCount > (unsigned int)(skillStep + 1) )
			AddGap(2);
		else
			return true;
	}

	unsigned int nextStep = 0;
	if( skillStep != UCHAR_MAX )
	{
		nextStep = skillStep + 1;
	}

	if( pInfo->mStepCount <= nextStep )
	{
		assert(0);
	}
	
	sPlayerSkillStepInfo trainStepInfo = pInfo->mpSetpInfoArray[nextStep];

	/// 시전 유형
	SetShotType( pInfo, COLOR_WHITE );

	/// 사용 장비
	SetEquip( pInfo, COLOR_WHITE );

	/// 시전시간
	SetCastingTime( pInfo, &trainStepInfo, COLOR_WHITE );

	/// 재사용시간
	SetCoolTime( &trainStepInfo, COLOR_WHITE, 0 );

	/// 재사용 시간 공유
	SetCoolTimeShareList( pInfo, &trainStepInfo, jobIdx, COLOR_WHITE );

	/// 소모 비용
	SetUsedValue( pInfo, &trainStepInfo, COLOR_WHITE );

	///  범위 방식
	SetRangeType( pInfo, COLOR_WHITE );

	cStringT temp;
	if( trainStepInfo.mApplyValue1 != 0 || trainStepInfo.mApplyValue2 != 0 )
	{
		if( trainStepInfo.mApplyValue1 != 0 && trainStepInfo.mApplyValue2 != 0 )
		{
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1, trainStepInfo.mApplyValue2 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( trainStepInfo.mApplyValue1 != 0 )
		{
			AddGap( 1 );
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
	}
	else
	{
		/// 일반 툴팁 출력
		AddGap( 1 );
		temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( trainStepInfo.mToolTipIdx ), trainStepInfo.mApplyValue1 );
		CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
	}

	/// 효과
	bool isInfluence = false;
	unsigned int influenceTipIndex = mIndex;
	unsigned int gapcount = 0;
	if( pInfo->mType == eSKILLTYPE_TOTEM )
	{
		sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( trainStepInfo.mInfulenceIdx );
		if( totem )
		{
			isInfluence = SetInfluence( totem->mInfluenceIdx );
		}
	}
	else
	{
		unsigned long infIdx = trainStepInfo.mInfulenceIdx;

		if( pInfo->mShotType == eSHOTTYPE_RUSHRUN || pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
		{
			infIdx = 0;

			if( trainStepInfo.mInfulenceIdx != 0 )
			{
				sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mInfulenceIdx );
				if( pInfo2 == 0 )
					return false;

				unsigned int trainStep = (unsigned int)nextStep;
				if( pInfo2->mStepCount <= trainStep )
				{
					assert(0);
					return false;
				}

				sPlayerSkillStepInfo trainStepInfo2 = pInfo2->mpSetpInfoArray[trainStep];
				infIdx = trainStepInfo2.mInfulenceIdx;
			}
		}

		isInfluence = SetInfluence( infIdx );
	}

	if( isInfluence == true )
	{
		gapcount = mGapCount;
	}

	SetNoOverLapInfluenceList( pInfo, &trainStepInfo, jobIdx, COLOR_WHITE );

	bool isActiveEffect = false;
	unsigned int activeTipIndex = mIndex;
	unsigned int activegapcount = 0;
	isActiveEffect = SetActiveEffect( pInfo, &trainStepInfo, COLOR_WHITE );
	if( isActiveEffect )
	{
		activegapcount = mGapCount;
	}

	/// 오른쪽 툴팁
	AddRightGap( 2 );

	if( nextStep == 0 )		
	{
		/// 미습득 표현
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 101050 ), nextStep, pInfo->mStepCount );
		mRightTip[mRightIndex].mColor = COLOR_RED;
	}
	else
	{
		/// 다음 단계 표현
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100010 ), nextStep + 1, pInfo->mStepCount );
		mRightTip[mRightIndex].mColor = COLOR_WHITE;
	}

	CalcRightMaxWidth( mRightTip[mRightIndex].mString );
	mRightIndex++;
	if( mRightIndex> MAX_RIGHT_TIP )
	{
		assert( 0 && "error tip index" );
	}

	SetRightTargetDist( pInfo, &trainStepInfo, COLOR_WHITE, skillIdx ); 
	SetRightBound( pInfo, &trainStepInfo, COLOR_WHITE ); 
	SetRightApplyCnt( pInfo, &trainStepInfo, COLOR_WHITE );

	if( isInfluence == true )
	{
		long leftHeight = (mLineHeight * (influenceTipIndex- mBottomIndex)) + ((gapcount - bottomgapcount )* GAP_SIZE);
		long rightHeight = (mLineHeight * (mRightIndex- mBottomRightIndex)) + ((mRightGapCount - rightbottomgapcount) * GAP_SIZE);
		unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

		AddRightGap( addGapCount );
		SetRightInfluence( pInfo, &trainStepInfo, nextStep, COLOR_WHITE );
	}		

	if( isActiveEffect == true )
	{		
		long leftHeight = (mLineHeight * (activeTipIndex- mBottomIndex)) + ((activegapcount - bottomgapcount )* GAP_SIZE);
		long rightHeight = (mLineHeight * (mRightIndex- mBottomRightIndex)) + ((mRightGapCount - rightbottomgapcount) * GAP_SIZE);
		unsigned int addGapCount = (leftHeight - rightHeight) / GAP_SIZE;

		AddRightGap( addGapCount );		
		SetRightActiveEffect( &trainStepInfo, COLOR_WHITE );
	}		

	/// 스킬 미리찍어보기 정보 참조
	/// 조건 설명 표현
	{
		/// 필요 레벨
		mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101001 ), trainStepInfo.mPlayerLevel );
		mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;
		mRequireValue[mRequireIndex][0].mGapCount = 1;

		mRequireValue[mRequireIndex][1].mString.Clear();
		mRequireValue[mRequireIndex][2].mString.Clear();
		mRequireValue[mRequireIndex][3].mString.Clear();

		mRequireIndex++;
		

		///  선행 스킬
		mRequireValue[mRequireIndex][0].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100022 );
		mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
		mRequireValue[mRequireIndex][1].mColor = COLOR_RED;
		mRequireValue[mRequireIndex][2].mColor = COLOR_RED;

		/// 선행스킬 1
		bool allowLearn1 = false;
		if( trainStepInfo.mLearnSkillIdx1 )
		{
			sPlayerSkillBaseInfo* pLearn = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mLearnSkillIdx1 );
			if( pLearn )
			{
				LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pLearn->mNameIdx );
				mRequireValue[mRequireIndex][1].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100023 ), name, trainStepInfo.mLearnSkillStep1 );

				short curstep = UCHAR_MAX;
				sSkillData* learnData = win->GetSkillData( trainStepInfo.mLearnSkillIdx1 );
				if( learnData )
					curstep = learnData->step;

				if( curstep != UCHAR_MAX )
				{
					if( curstep + 1 >= trainStepInfo.mLearnSkillStep1 )
					{
						mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
						allowLearn1 = true;
					}
				}

				if( trainStepInfo.mLearnSkillIdx2 )
					mRequireValue[mRequireIndex][1].mString += _T(", ");
			}
			else
			{
				mRequireValue[mRequireIndex][1].mString = _T("Error");
				mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
			}
		}
		else
		{
			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][1].mColor = COLOR_WHITE;
			allowLearn1 = true;
		}

		/// 선행스킬 2
		bool allowLearn2 = false;
		if( trainStepInfo.mLearnSkillIdx2 )
		{
			sPlayerSkillBaseInfo* pLearn = SKILLSCRIPT->GetPlayerSkillInfo( trainStepInfo.mLearnSkillIdx2 );
			if( pLearn )
			{
				LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pLearn->mNameIdx );
				mRequireValue[mRequireIndex][2].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100023 ), name, trainStepInfo.mLearnSkillStep2 );

				short curstep = UCHAR_MAX;
				sSkillData* learnData = win->GetSkillData( trainStepInfo.mLearnSkillIdx2 );
				if( learnData )
					curstep = learnData->step;

				if( curstep != UCHAR_MAX )
				{
					if( curstep + 1 >= trainStepInfo.mLearnSkillStep2 )
					{
						mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
						allowLearn2 = true;
					}
				}
			}
			else
			{
				mRequireValue[mRequireIndex][2].mString = _T("Error");
				mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
			}
		}
		else
		{
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][2].mColor = COLOR_WHITE;
			allowLearn2 = true;
		}

		/// 
		if( allowLearn1 == true && allowLearn2 == true )
		{
			mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;

			cStringT tempStr;
			tempStr += mRequireValue[mRequireIndex][0].mString;
			tempStr += mRequireValue[mRequireIndex][1].mString;
			tempStr += mRequireValue[mRequireIndex][2].mString;

			CalcMaxWidth( tempStr );
		}

		if( trainStepInfo.mLearnSkillIdx1 || trainStepInfo.mLearnSkillIdx2 )
		{
			if( allowLearn1 == false || allowLearn2 == false )
			{
				cStringT tempStr;
				tempStr += mRequireValue[mRequireIndex][0].mString;
				tempStr += mRequireValue[mRequireIndex][1].mString;
				tempStr += mRequireValue[mRequireIndex][2].mString;
				CalcMaxWidth( tempStr );

				mRequireIndex++;
				if( mRequireIndex > MAX_RIGHT_TIP )
					assert( 0 && "error tip index" );
			}
		}

		/// 습득 SP 비용
		/// 마스터 여부 체크
		if ( pInfo->mStepCount != nextStep)
		{
			sPlayerSkillStepInfo stepInfo = pInfo->mpSetpInfoArray[nextStep];
			unsigned int requireSP = stepInfo.mRequireSP;

			mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101009 ), requireSP );
			mRequireValue[mRequireIndex][0].mColor = COLOR_WHITE;
			mRequireValue[mRequireIndex][0].mGapCount = 1;

			mRequireValue[mRequireIndex][1].mString.Clear();
			mRequireValue[mRequireIndex][2].mString.Clear();
			mRequireValue[mRequireIndex][3].mString.Clear();

			mRequireIndex++;
		}

		/// 사용 SP량
		if( pInfo->mJobStep != 0 && pInfo->mJobStep != 9 )
		{
			// 이전 직업 총 사용 SP검사
			unsigned long currentJobIndex = win->GetCurrentJobIndex();

			unsigned long beforeJobIndex = 0;
			tArray<unsigned long>* parray = GAMERESOURCEMAN->GetParentJob( currentJobIndex );
			if( parray && parray->IsEmpty() == false )
			{
				int index = parray->GetSize() - 1;
				beforeJobIndex = (*parray)[index];
			}

			// 현재 찍어논 스킬로 검사해야한다
			sPreviewData* data = win->GetPreviewData( beforeJobIndex );
			if( data )
			{
				if( (unsigned long)data->jobPoint < pInfo->mBeforeReqSP )
				{
					mRequireValue[mRequireIndex][0].mString.Format(GAMERESOURCEMAN->GetSkillToolTipText( 101008 ), pInfo->mBeforeReqSP );
					mRequireValue[mRequireIndex][0].mColor = COLOR_RED;
					mRequireValue[mRequireIndex][0].mGapCount = 1;

					mRequireValue[mRequireIndex][1].mString.Clear();
					mRequireValue[mRequireIndex][2].mString.Clear();
					mRequireValue[mRequireIndex][3].mString.Clear();

					mRequireIndex++;
				}
			}
		}
	}

	return true;
}


void cSkillTipWindow::AddRightGap( unsigned int count )
{
	mRightTip[mRightIndex].mGapCount = count;
	mRightGapCount += count;
}

void cSkillTipWindow::SetName( sPlayerSkillBaseInfo* pInfo, unsigned long color )
{
	mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameIdx );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetShotType( sPlayerSkillBaseInfo* pInfo, unsigned long color )
{
	unsigned long tipIndex = 0;
	switch( pInfo->mShotType )
	{
	case eSHOTTYPE_NONE: 		tipIndex = 101002;		break;
	case eSHOTTYPE_SPOT:		tipIndex = 101003;		break;
	case eSHOTTYPE_CASTING: 	tipIndex = 101004;		break;
	case eSHOTTYPE_CHANNELING: 	tipIndex = 101005;		break;
	case eSHOTTYPE_AURA: 		tipIndex = 101006;		break;
	case eSHOTTYPE_BLINK:		tipIndex = 101003;		break;
	case eSHOTTYPE_RUSHRUN:		tipIndex = 101010;		break;
	case eSHOTTYPE_RUSHBLINK:	tipIndex = 101011;		break;
	case eSHOTTYPE_BLINK_BACK:	tipIndex = 101012;		break;
	}

	mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText( tipIndex );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetEquip( sPlayerSkillBaseInfo* pInfo, unsigned long color )
{
	LPCTSTR temp = 0;
	switch( pInfo->mUseEquipment )
	{
		///제한 없음
	case 0: temp = GAMERESOURCEMAN->GetSkillToolTipText(100027); break;
		///한손검
	case 1: temp = GAMERESOURCEMAN->GetSkillToolTipText(100029); break;
		///양손검
	case 2: temp = GAMERESOURCEMAN->GetSkillToolTipText(100030); break;
		///이도류
	case 3: temp = GAMERESOURCEMAN->GetSkillToolTipText(100031); break;
		///단검
	case 4: temp = GAMERESOURCEMAN->GetSkillToolTipText(100032); break;
		///총
	case 5: temp = GAMERESOURCEMAN->GetSkillToolTipText(100033); break;
		///지팡이
	case 6: temp = GAMERESOURCEMAN->GetSkillToolTipText(100034); break;
		///방패 
	case 7: temp = GAMERESOURCEMAN->GetSkillToolTipText(100035); break;
		///한손검, 양손검
	case 8: temp = GAMERESOURCEMAN->GetSkillToolTipText(100036); break;
		///한손검, 이도류
	case 9: temp = GAMERESOURCEMAN->GetSkillToolTipText(100037); break;
		///양손검, 이도류
	case 10: temp = GAMERESOURCEMAN->GetSkillToolTipText(100038); break;
		///한손검, 양손검, 이도류
	case 11: temp = GAMERESOURCEMAN->GetSkillToolTipText(100039); break;
		///단검 또는 총
	case 12: temp = GAMERESOURCEMAN->GetSkillToolTipText(100040); break;
		///천 방어구
	case 13: temp = GAMERESOURCEMAN->GetSkillToolTipText(100041); break;
		///경갑 방어구
	case 14: temp = GAMERESOURCEMAN->GetSkillToolTipText(100042); break;
		///중갑 방어구
	case 15: temp = GAMERESOURCEMAN->GetSkillToolTipText(100043); break;
	}
	mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100000 ), temp );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetCastingTime( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color )
{
	float castingTime = (float)HERO->CalcCastingExt( pStepInfo->mCastingTime );
	if( pInfo->mShotType == eSHOTTYPE_CASTING )
		mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(100001), castingTime * 0.001f );
	else if( pInfo->mShotType == eSHOTTYPE_CHANNELING ) 
		mToolTip[mIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText(100002), pStepInfo->mCastingTime * 0.001f ); ///채널링은 유지시간이므로 스크립트에 있는값을 기준으로한다.
	else
		mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText(100026);

	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetCoolTime( sPlayerSkillStepInfo* pStepInfo, unsigned long color, unsigned long skillIdx )
{
	unsigned long coolTime = pStepInfo->mCoolTime / SECOND_THOUSAND;

	unsigned int h = (coolTime / 60) / 60;
	unsigned int m = (coolTime / 60) % 60;
	//unsigned int s = (coolTime % 60) % 60 + 1;
/*
	unsigned int h = coolTime / 3600;
	unsigned int m = coolTime / 60;*/
	float s = (float)( ( pStepInfo->mCoolTime % 60000 ) / 1000.0f );

	/// 재사용시간이 0 이면 찍지 않는다.
	if( h == 0 && m == 0 && s == 0 )
		return;

	cStringT th,tm,ts;
	if( h != 0 )
		th.Format( GAMERESOURCEMAN->GetSkillToolTipText(100017), h );
	if( m != 0 )
		tm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
	if( s != 0.0f )
		ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(101100), s );

	th += tm; th += ts;

	cStringT str1;
	str1.Format( GAMERESOURCEMAN->GetSkillToolTipText(100003), th.Cstr() );

	sKeepSkill* skill = SKILLMAN->GetKeepInfo( skillIdx );
	if( skill && skill->mRestTime > 0.0f)
	{
		mTimerEnabled = true;
		mUpdateSizebyTime = true;
		mSaveSkillIndex = skillIdx;
	}

	// 정보구성
	mTimeLine[ eTIMELINE_COOLTIME ].saveCoolTime = str1;
	mTimeLine[ eTIMELINE_COOLTIME ].timeTipIdx = mIndex;

	SetText( str1.Cstr(), color );
}

void cSkillTipWindow::SetCoolTimeShareList( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long jobIdx, unsigned long color )
{
	if( pInfo->mGroupCoolTimeIdx <= 0 )
		return;

	unsigned selectJobIdx = jobIdx;

	if( selectJobIdx <= 0 || selectJobIdx == ULONG_MAX )
	{
		typedef tArray<unsigned long> cJobIndexArray;

		cJobIndexArray* pJobClassArray = SKILLSCRIPT->GetPlayerSkillJob( pStepInfo->mJobType );
		if( pJobClassArray == NULL )
		{
			assert(0);
			return;
		}

		tArray<unsigned long>* parentJobArr = GAMERESOURCEMAN->GetParentJob( HERO->GetJob() );		

		for( unsigned int i = 0; i < pJobClassArray->GetSize(); i++ )
		{
			ePLAYER_JOB jobIdx =  (ePLAYER_JOB)(*pJobClassArray)[i];
			if( jobIdx == HERO->GetJob() )
			{
				selectJobIdx = jobIdx;
				break;
			}
			else
			{
				if( parentJobArr )
				{
					for( unsigned int j = 0; j < parentJobArr->GetSize(); j++)
					{
						ePLAYER_JOB jobIdx1 = (ePLAYER_JOB)(*parentJobArr)[i];
						if( jobIdx == jobIdx1 )
						{
							selectJobIdx = jobIdx;
							break;
						}
					}			
				}
			}
		}	
	}

	if( selectJobIdx > 0 && selectJobIdx != ULONG_MAX )
	{		
		int findCount = 0;
		cStringT skill;
		cStringT skillList;
	
		cSkillTree* pHeroSkillTree = SKILLSCRIPT->GetSkillTree( selectJobIdx );
		if( pHeroSkillTree )
		{						
			for( unsigned int i=0; i < pHeroSkillTree->mIconCount; ++i )
			{
				sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( pHeroSkillTree->mIconList[i].skillIdx );
				if( pSkillInfo )
				{
					if( pInfo->mSkillIdx == pSkillInfo->mSkillIdx )
						continue;

					if( pInfo->mGroupCoolTimeIdx == pSkillInfo->mGroupCoolTimeIdx )
					{
						LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pSkillInfo->mNameIdx );
						if( name == NULL )
						{
							assert(0);
							return;
						}

						skill.Format( GAMERESOURCEMAN->GetSkillToolTipText(101014), name );
						skillList += skill;
						findCount++;
					}				
				}
				else
					assert(0);
			}	

		}
		else
			assert(0);	

		if( findCount > 0 )
		{
			cStringT shareskillList;
			shareskillList.Format( GAMERESOURCEMAN->GetSkillToolTipText(101016), skillList.Cstr() );

			/// 효과 찍기
			mToolTip[mIndex].mString = shareskillList;
			mToolTip[mIndex].mColor = color;
			CalcMaxWidth( mToolTip[mIndex].mString );
			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}		
	}
}		

void cSkillTipWindow::OnTimer()
{
	cUIWindow::OnTimer();

	if( mVisible == false )
		return;

	if( mSaveSkillIndex == 0 )
		return;

	// 쿨타임
	if( mTimeLine[eTIMELINE_COOLTIME].timeTipIdx != 0 )
	{
		cStringT str1 = mTimeLine[eTIMELINE_COOLTIME].saveCoolTime;

		sKeepSkill* skill = SKILLMAN->GetKeepInfo( mSaveSkillIndex );
		if( skill && skill->mRestTime != skill->mTotalTime )
		{
			cStringT str2;

			/// 초를 출력시 반올림해서 출력
			unsigned int restTime = (unsigned int)(skill->mRestTime * 0.001f);
			unsigned int m = (restTime % 3600) / 60;
			unsigned int s = ((restTime % 600) % 60)%60 + 1;

			cStringT rm,rs;
			if( m != 0 )
				rm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
			rs.Format( GAMERESOURCEMAN->GetSkillToolTipText(100015), s );

			rm += rs;
			str2.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100004 ), rm.Cstr() );
			str1.Append( str2 );
		}

		unsigned int timeTipIdx = mTimeLine[eTIMELINE_COOLTIME].timeTipIdx;
		mToolTip[timeTipIdx].mString = str1;
		mToolTip[timeTipIdx].mColor = COLOR_WHITE;
		CalcMaxWidth( mToolTip[timeTipIdx].mString );
	}

	if( mUpdateSizebyTime == true )
	{
		SetBackImg( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
		mUpdateSizebyTime = false;
	}
}

void cSkillTipWindow::SetUsedValue( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color )
{
	if( pInfo->mUseItem == 0 )
	{
		/// 소모 아이템이 없는경우
		if( pStepInfo->mUseMP != 0 || pStepInfo->mUseHP != 0 || pStepInfo->mUseMPPer != 0 )
		{
			/// 소모 hp, mp가 있거나..
			cStringT str = GAMERESOURCEMAN->GetSkillToolTipText(100005);
			cStringT hp;
			if( pStepInfo->mUseHP != 0 )
			{
				hp.Format( GAMERESOURCEMAN->GetSkillToolTipText(100007), pStepInfo->mUseHP );
				str += hp;
			}
			cStringT mp;
			if( pStepInfo->mUseMP != 0 )
			{
				if( pStepInfo->mUseHP != 0 )
					str += _T(", ");

				unsigned long totalUseMP = HERO->CalcUseMPExt( pStepInfo->mUseMP );

				mp.Format( GAMERESOURCEMAN->GetSkillToolTipText(100006), totalUseMP );
				str += mp;
			}

			cStringT mpPer;
			if( pStepInfo->mUseMPPer != 0 )
			{
				if( pStepInfo->mUseHP != 0 || pStepInfo->mUseMP != 0 )
					str += _T(", ");

				unsigned long currentUseMP = HERO->GetOriMpMax();
				currentUseMP = (unsigned long)((currentUseMP * pStepInfo->mUseMPPer) / 100);
				unsigned long totalUseMP = HERO->CalcUseMPExt( currentUseMP );

				mpPer.Format( GAMERESOURCEMAN->GetSkillToolTipText(101207), pStepInfo->mUseMPPer, totalUseMP );
				str += mpPer;
			}

			mToolTip[mIndex].mString = str;
			mToolTip[mIndex].mColor = color;
			CalcMaxWidth( mToolTip[mIndex].mString );
			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}

		return;
	}

	/// 소모 아이템이 있는 경우
	cItemDefine* p = ITEMMAN->GetItemDefine( pInfo->mUseItem );
	if( pStepInfo->mUseMP != 0 || pStepInfo->mUseHP != 0 || pStepInfo->mUseMPPer != 0 )
	{
		cStringT str = GAMERESOURCEMAN->GetSkillToolTipText(100005);
		cStringT hp;
		if( pStepInfo->mUseHP != 0 )
		{
			hp.Format( GAMERESOURCEMAN->GetSkillToolTipText(100007), pStepInfo->mUseHP );
			str += hp;
		}
		cStringT mp;
		if( pStepInfo->mUseMP != 0 )
		{
			if( pStepInfo->mUseHP != 0 )
				str += _T(", ");

			unsigned long totalUseMP = HERO->CalcUseMPExt( pStepInfo->mUseMP );
			mp.Format( GAMERESOURCEMAN->GetSkillToolTipText(100006), totalUseMP );
			str += mp;
		}
		cStringT mpPer;
		if( pStepInfo->mUseMPPer != 0 )
		{
			if( pStepInfo->mUseHP != 0 || pStepInfo->mUseMP != 0 )
				str += _T(", ");

			unsigned long currentUseMP = HERO->GetOriMpMax();
			currentUseMP = (unsigned long)((currentUseMP * pStepInfo->mUseMPPer) / 100);
			unsigned long totalUseMP = HERO->CalcUseMPExt( currentUseMP );

			mpPer.Format( GAMERESOURCEMAN->GetSkillToolTipText(101207), pStepInfo->mUseMPPer, totalUseMP );
			str += mpPer;
		}

		mToolTip[mIndex].mString = str;
		mToolTip[mIndex].mColor = color;
		CalcMaxWidth( mToolTip[mIndex].mString );
		mIndex++;
		if( mIndex > MAX_TIP )
		{
			assert( 0 && "error tip index" );
		}

		if( p )
		{
			cStringT item;
			item.Format( GAMERESOURCEMAN->GetSkillToolTipText(100008), p->GetName(), pStepInfo->mUseItemCount );
			mToolTip[mIndex].mString = item;
			mToolTip[mIndex].mColor = color;
			CalcMaxWidth( mToolTip[mIndex].mString );
			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}
	}
	else
	{
		if( p )
		{
			cStringT str = GAMERESOURCEMAN->GetSkillToolTipText(100005);

			cStringT item;
			item.Format( GAMERESOURCEMAN->GetSkillToolTipText(100009), p->GetName(), pStepInfo->mUseItemCount );
			str += item;
			mToolTip[mIndex].mString = str;
			mToolTip[mIndex].mColor = color;
			CalcMaxWidth( mToolTip[mIndex].mString );
			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}
	}
}

/// 중첩 불가 스킬 목록
void cSkillTipWindow::SetNoOverLapInfluenceList( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long jobIdx, unsigned long color )	
{
	if( pStepInfo->mInfulenceIdx <= 0 )
		return;

	if( jobIdx <= 0 || jobIdx == ULONG_MAX )
		return;

	sInfluenceScript* pCurInfo = SKILLSCRIPT->GetInfluenceInfo( pStepInfo->mInfulenceIdx );
	if( pCurInfo == 0 )
		return;
	if( pCurInfo->mToolTipIdx == 0 )
		return;

	typedef tArray<unsigned long> cJobIndexArray;

	cJobIndexArray* pJobClassArray = SKILLSCRIPT->GetPlayerSkillJob( pStepInfo->mJobType );
	if( pJobClassArray == NULL )
	{
		assert(0);
		return;
	}
/*
	bool find = false;
	unsigned long selectjob = 0;
	tArray<unsigned long>* parentJobArr = GAMERESOURCEMAN->GetParentJob( HERO->GetJob() );		
		
	for( unsigned int i = 0; i < pJobClassArray->GetSize(); i++ )
	{
		ePLAYER_JOB jobIdx =  (ePLAYER_JOB)(*pJobClassArray)[i];
		if( jobIdx == HERO->GetJob() )
		{
			selectjob = jobIdx;
			find = true;
			break;
		}
		else
		{
			if( parentJobArr )
			{
				for( unsigned int j = 0; j < parentJobArr->GetSize(); j++)
				{
					ePLAYER_JOB jobIdx1 = (ePLAYER_JOB)(*parentJobArr)[i];
					if( jobIdx == jobIdx1 )
					{
						selectjob = jobIdx;
						find = true;
						break;
					}
				}			
			}
		}
	}	
*/
//	if( find == true && selectjob > 0)
	if( jobIdx > 0)
	{		
		cSkillTree* pHeroSkillTree = SKILLSCRIPT->GetSkillTree( jobIdx );

		int findCount = 0;
		cStringT skill;
		cStringT skillList;

		if( pHeroSkillTree )
		{	
			for( unsigned int i=0; i < pHeroSkillTree->mIconCount; ++i )
			{
				sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( pHeroSkillTree->mIconList[i].skillIdx );

				if( pInfo->mSkillIdx == pSkillInfo->mSkillIdx )
					continue;

				if( pSkillInfo )
				{
					sPlayerSkillStepInfo pSkillStep = pSkillInfo->mpSetpInfoArray[ 0 ];

					if( pSkillStep.mInfulenceIdx <= 0 )
						continue;

					/// 지면 스킬
					if( pSkillInfo->mType == eSKILLTYPE_TOTEM )
					{
						sTotemScript* pTotem = TOTEMSCRIPT->GetTotemInfo( pSkillStep.mInfulenceIdx );
						if( pTotem && pTotem->mInfluenceIdx > 0 )
						{
							sInfluenceScript* pTotemInfuence = SKILLSCRIPT->GetInfluenceInfo( pTotem->mInfluenceIdx );
							if( pTotemInfuence )
							{
								if( pTotemInfuence->mNotOverlapGroupIdx == pCurInfo->mNotOverlapGroupIdx )
								{
									LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pSkillInfo->mNameIdx );
									if( name )
									{
										skill.Format( GAMERESOURCEMAN->GetSkillToolTipText(101014), name );
										skillList += skill;
										findCount++;
									}
									else
									{
										assert(0);
									}
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
					else
					{
						unsigned long infIdx = pSkillStep.mInfulenceIdx;

						/// 연계 스킬
						if( pSkillInfo->mShotType == eSHOTTYPE_RUSHRUN || pSkillInfo->mShotType == eSHOTTYPE_RUSHBLINK )
						{
							infIdx = 0;

							if( pSkillStep.mInfulenceIdx != 0 )
							{
								sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( pSkillStep.mInfulenceIdx );
								if( pInfo2 == 0 )
								{
									assert(0);
									return;
								}

								sPlayerSkillStepInfo linkStepInfo = pInfo2->mpSetpInfoArray[0];

								infIdx = linkStepInfo.mInfulenceIdx;
							}
						}

						if( infIdx > 0 )
						{
							sInfluenceScript* pInfInfo = SKILLSCRIPT->GetInfluenceInfo( infIdx );
							if( pInfInfo )
							{	
								if( pInfInfo->mNotOverlapGroupIdx == pCurInfo->mNotOverlapGroupIdx )
								{
									LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( pSkillInfo->mNameIdx );
									if( name )
									{
										skill.Format( GAMERESOURCEMAN->GetSkillToolTipText(101014), name );
										skillList += skill;
										findCount++;
									}
									else
									{
										assert(0);
									}
								}								
							}
							else
								assert(0);
						}								
					}				
				}
				else
					assert(0);
			}	

			if( findCount > 0 )
			{
				AddGap(1);

				/// 효과 찍기
				mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText(101013);
				mToolTip[mIndex].mColor = COLOR_WHITE;
				CalcMaxWidth( mToolTip[mIndex].mString );
				mIndex++;
				if( mIndex > MAX_TIP )
				{
					assert( 0 && "error tip index" );
				}

				/// 효과 찍기
				mToolTip[mIndex].mString = skillList;
				mToolTip[mIndex].mColor	= COLOR_SKILL_NOT_OVERLAP;
				CalcMaxWidth( mToolTip[mIndex].mString );
				mIndex++;
				if( mIndex > MAX_TIP )
				{
					assert( 0 && "error tip index" );
				}
			}		
		}
		else
			assert(0);		
	}
}

/// 발동 효과
bool cSkillTipWindow::SetActiveEffect( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color)
{
	if( pStepInfo->mActiveIdx <= 0 )
		return false;

	sActInfluenceScript* pActInfluenceScript = SKILLSCRIPT->GetActInfluenceInfo( pStepInfo->mActiveIdx );

	if( pActInfluenceScript )
	{
		if( pActInfluenceScript->mInfIdx > 0)
		{
			sInfluenceScript* pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pActInfluenceScript->mInfIdx );

			if( pInfluenceScript  )
			{		
				AddGap(1);

				/// 효과 찍기
				mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText(101015);
				mToolTip[mIndex].mColor = COLOR_WHITE;
				CalcMaxWidth( mToolTip[mIndex].mString );
				mIndex++;
				if( mIndex > MAX_TIP )
				{
					assert( 0 && "error tip index" );
					return false;
				}
			
				if( pInfluenceScript && pInfluenceScript->mToolTipIdx > 0 )
				{
					cStringT temp;
					if( pInfluenceScript->mValue1 != 0 || pInfluenceScript->mValue2 != 0 || pInfluenceScript->mValue3 != 0 )
					{
						if( pInfluenceScript->mValue1 && pInfluenceScript->mValue2 && pInfluenceScript->mValue3 )
						{
							temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfluenceScript->mToolTipIdx ), pInfluenceScript->mValue1, pInfluenceScript->mValue2, pInfluenceScript->mValue3 );
							CheckText( (LPCTSTR)temp.Cstr(), COLOR_SKILL_ACTIVE_EFFECT );
						}
						else if( pInfluenceScript->mValue1 && pInfluenceScript->mValue2 )
						{
							temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfluenceScript->mToolTipIdx ), pInfluenceScript->mValue1, pInfluenceScript->mValue2 );
							CheckText( (LPCTSTR)temp.Cstr(), COLOR_SKILL_ACTIVE_EFFECT );
						}
						else if( pInfluenceScript->mValue1 )
						{
							temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfluenceScript->mToolTipIdx ), pInfluenceScript->mValue1 );
							CheckText( (LPCTSTR)temp.Cstr(), COLOR_SKILL_ACTIVE_EFFECT );
						}
					}
					else
					{
						/// 일반 툴팁 출력
						CheckText( GAMERESOURCEMAN->GetSkillToolTipText( pInfluenceScript->mToolTipIdx ), COLOR_SKILL_ACTIVE_EFFECT );
					}
				}
				else
				{
					assert(0);
					return false;
				}

				return true;
			}
		}
	}

	return false;
}

/// 범위 방식
void cSkillTipWindow::SetRangeType( sPlayerSkillBaseInfo* pInfo, unsigned long color)
{
	if( pInfo->mBoundType == eBOUNDTYPE_NONE)
		return;

	unsigned long textIdx = 0;
	switch( pInfo->mBoundType )
	{
	case eBOUNDTYPE_SELF_TARGET:
		textIdx = 101018;
		break;
	case eBOUNDTYPE_SELF_NOTARGET:
		textIdx = 101019;
		break;
	case eBOUNDTYPE_TARGET:
		textIdx = 101020;
		break;
	case eBOUNDTYPE_FIELD:
		textIdx = 101021;
		break;
	}

	cStringT rangeType;
	rangeType.Format(  GAMERESOURCEMAN->GetSkillToolTipText( 101017 ), GAMERESOURCEMAN->GetSkillToolTipText( textIdx ) );

	/// 효과 찍기
	mToolTip[mIndex].mString = rangeType;
	mToolTip[mIndex].mColor = COLOR_WHITE;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

bool cSkillTipWindow::SetInfluence( unsigned long influenceIdx )//, sPlayerSkillStepInfo* pStepInfo  )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( pInfo == 0 )
		return false;
	if( pInfo->mToolTipIdx == 0 )
		return false;

	AddGap( 1 );

	/// 효과 찍기
	mToolTip[mIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText(101007);
	mToolTip[mIndex].mColor = COLOR_WHITE;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;
	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}

	/// 효과값이 있으면 효과출력
	cStringT temp;
	if( pInfo->mValue1 != 0 || pInfo->mValue2 != 0 || pInfo->mValue3 != 0 )
	{
		if( pInfo->mValue1 && pInfo->mValue2 && pInfo->mValue3 )
		{
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2, pInfo->mValue3 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( pInfo->mValue1 && pInfo->mValue2 )
		{
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
		else if( pInfo->mValue1 )
		{
			temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1 );
			CheckText( (LPCTSTR)temp.Cstr(), COLOR_YELLOW );
		}
	}
	else
	{
		/// 일반 툴팁 출력
		CheckText( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), COLOR_YELLOW );
	}
	
	return true;
}

void cSkillTipWindow::SetRightAttribute( sPlayerSkillBaseInfo* pInfo, unsigned long color )
{
	if( pInfo->mAttributeType == eATTRIBUTETYPE_MAGIC )
		mRightTip[mRightIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100018 );
	else
		mRightTip[mRightIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100019 );
	mRightTip[mRightIndex].mColor = color;
	CalcRightMaxWidth( mRightTip[mRightIndex].mString );
	mRightIndex++;
	if( mRightIndex> MAX_RIGHT_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetRightStepInfo( sPlayerSkillBaseInfo* pInfo, unsigned int curStep, unsigned long color )
{
	if( pInfo->mStepCount == curStep+1 )
	{
		/// master
		mRightTip[mRightIndex].mString = GAMERESOURCEMAN->GetSkillToolTipText( 100011 );
		mRightTip[mRightIndex].mColor = color;
		CalcRightMaxWidth( mRightTip[mRightIndex].mString );
		mRightIndex++;
		if( mRightIndex> MAX_RIGHT_TIP )
		{
			assert( 0 && "error tip index" );
		}
	}
	else
	{
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100010 ), curStep+1, pInfo->mStepCount );
		mRightTip[mRightIndex].mColor = color;
		CalcRightMaxWidth( mRightTip[mRightIndex].mString );
		mRightIndex++;
		if( mRightIndex> MAX_RIGHT_TIP )
		{
			assert( 0 && "error tip index" );
		}
	}
}

void cSkillTipWindow::SetRightTargetDist( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color, unsigned long skillIdx )
{
	float targetDist = pStepInfo->mTargetDist;

	/// 일반공격 스킬이면 장착한 아이템의 사거리를 더한다.
	unsigned long weaponIdx = HERO->GetRightWeaponIdx();
	cItemDefine* define = ITEMMAN->GetItemDefine( weaponIdx );
	if( define )
	{
		cItemAbility* abil = define->GetAbililty();
		if( abil && skillIdx == 1)
			targetDist += abil->GetAttackRange();
	}

	/// 사거리 표현
	if( pInfo->mRangeType == eRANGETYPE_CLOSE )
	{
		if( pStepInfo->mTargetDist <= 0.0f )
			return;

		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100013 ), (float)(targetDist / 100) );
		mRightTip[mRightIndex].mColor = color;
		CalcRightMaxWidth( mRightTip[mRightIndex].mString );
		mRightIndex++;
		if( mRightIndex> MAX_RIGHT_TIP )
		{
			assert( 0 && "error tip index" );
		}
	}
	else if( pInfo->mRangeType == eRANGETYPE_LONG )
	{
		if( pStepInfo->mTargetDist <= 0.0f )
			return;

		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100012 ), (float)(targetDist / 100) );
		mRightTip[mRightIndex].mColor = color;
		CalcRightMaxWidth( mRightTip[mRightIndex].mString );
		mRightIndex++;
		if( mRightIndex> MAX_RIGHT_TIP )
		{
			assert( 0 && "error tip index" );
		}
	}
}

void cSkillTipWindow::SetRightInfluence( sPlayerSkillBaseInfo* pInfo,  sPlayerSkillStepInfo* pStepInfo, unsigned int step, unsigned long color )
{
	unsigned long infIdx = 0;
	long continuanceTime = 0;
	sInfluenceScript* pInfluenceScript = NULL;
	if( pInfo->mType == eSKILLTYPE_TOTEM )
	{
		if( pStepInfo->mInfulenceIdx > 0 )
		{
			sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( pStepInfo->mInfulenceIdx );
			if( totem )
			{
				infIdx				= totem->mInfluenceIdx;
				if( infIdx > 0 )
				{
					pInfluenceScript	= SKILLSCRIPT->GetInfluenceInfo( infIdx );
					if( pInfluenceScript == NULL )
					{
						assert(0);
						return;
					}

					continuanceTime		= totem->mContinuanceTime;				
				}				
			}
		}		
	}
	else
	{
		infIdx = pStepInfo->mInfulenceIdx;

		if( pInfo->mShotType == eSHOTTYPE_RUSHRUN || pInfo->mShotType == eSHOTTYPE_RUSHBLINK )
		{
			infIdx = 0;

			if( pStepInfo->mInfulenceIdx != 0 )
			{
				sPlayerSkillBaseInfo* pInfo2 = SKILLSCRIPT->GetPlayerSkillInfo( pStepInfo->mInfulenceIdx );
				if( pInfo2 == 0 )
					return;

				sPlayerSkillStepInfo curStepInfo2 = pInfo2->mpSetpInfoArray[step];
				infIdx				= curStepInfo2.mInfulenceIdx;

			}			
		}

		if( infIdx > 0 )
		{
			pInfluenceScript	= SKILLSCRIPT->GetInfluenceInfo( infIdx );
			if( pInfluenceScript == NULL )
			{
				assert(0);
				return;
			}

			continuanceTime		= pInfluenceScript->mContinuanceTime;
		}		
	}

	if( pInfluenceScript )
	{
		if( continuanceTime > 0 )
		{
			unsigned long coolTime = pInfluenceScript->mContinuanceTime / SECOND_THOUSAND;

			unsigned int h = (coolTime / 60) / 60;
			unsigned int m = (coolTime / 60) % 60;
			//			float s = (float)(((unsigned int)coolTime % 60) % 60);
			float s = (float)( ( pInfluenceScript->mContinuanceTime % 60000 ) / 1000.0f );

			/// 지속 시간 0 이면 찍지 않는다.
			if( h != 0 || m != 0 || s != 0 )
			{
				cStringT th,tm,ts;
				if( h != 0 )
					th.Format( GAMERESOURCEMAN->GetSkillToolTipText(100017), h );
				if( m != 0 )
					tm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
				if( s != 0.0f )
					ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(101100), s );

				th += tm; th += ts;

				mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100024 ), th.Cstr());
				mRightTip[mRightIndex].mColor = color;
				CalcRightMaxWidth( mRightTip[mRightIndex].mString );

				mRightIndex++;
				if( mRightIndex> MAX_RIGHT_TIP )
				{
					assert( 0 && "error tip index" );
				}
			}					
		}

		if( pInfluenceScript->mGroupOrder > 0 )
		{			
			mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100025 ), pInfluenceScript->mGroupOrder );		
			mRightTip[mRightIndex].mColor = color;
			CalcRightMaxWidth( mRightTip[mRightIndex].mString );

			mRightIndex++;
			if( mRightIndex> MAX_RIGHT_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}
	}
}

// 발동 효과 시간, 효력값 출력
void cSkillTipWindow::SetRightActiveEffect( sPlayerSkillStepInfo* pStepInfo, unsigned long color )
{
	if( pStepInfo->mActiveIdx <= 0 )
		return;
	
	sActInfluenceScript* pActInfluenceScript = SKILLSCRIPT->GetActInfluenceInfo( pStepInfo->mActiveIdx );

	if( pActInfluenceScript )
	{
		if( pActInfluenceScript->mInfIdx > 0)
		{
			sInfluenceScript* pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pActInfluenceScript->mInfIdx );

			if( pInfluenceScript  )
			{
				if( pInfluenceScript->mContinuanceTime > 0  )
				{
					unsigned long coolTime = pInfluenceScript->mContinuanceTime / SECOND_THOUSAND;

					unsigned int h = (coolTime / 60) / 60;
					unsigned int m = (coolTime / 60) % 60;
					//float s = (float)((coolTime % 60) % 60);
					float s = (float)( ( pInfluenceScript->mContinuanceTime % 60000 ) / 1000.0f );

					/// 지속 시간 0 이면 찍지 않는다.
					if( h != 0 || m != 0 || s != 0 )
					{
						cStringT th,tm,ts;
						if( h != 0 )
							th.Format( GAMERESOURCEMAN->GetSkillToolTipText(100017), h );
						if( m != 0 )
							tm.Format( GAMERESOURCEMAN->GetSkillToolTipText(100016), m );
						if( s != 0.0f )
							ts.Format( GAMERESOURCEMAN->GetSkillToolTipText(101100), s );

						th += tm; th += ts;

						mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100024 ), th.Cstr());
						mRightTip[mRightIndex].mColor = color;
						CalcRightMaxWidth( mRightTip[mRightIndex].mString );

						mRightIndex++;
						if( mRightIndex> MAX_RIGHT_TIP )
						{
							assert( 0 && "error tip index" );
						}		
					}
				}

				if( pInfluenceScript->mGroupOrder > 0 )
				{			
					mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100025 ), pInfluenceScript->mGroupOrder );		
					mRightTip[mRightIndex].mColor = color;
					CalcRightMaxWidth( mRightTip[mRightIndex].mString );

					mRightIndex++;
					if( mRightIndex> MAX_RIGHT_TIP )
					{
						assert( 0 && "error tip index" );
					}
				}		
			}
			else
			{
				assert(0);
			}
		}
	}
	else
	{
		assert(0);
	}
}

void cSkillTipWindow::SetRightBound( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color )
{
	unsigned short boundDist1 = 0;
	unsigned short boundDist2 = 0;

	//// 토템 지속 효과 인지 체크
	if( pInfo->mType == eSKILLTYPE_TOTEM )
	{
		sTotemScript* totem = TOTEMSCRIPT->GetTotemInfo( pStepInfo->mInfulenceIdx );
		if( totem )
		{
			boundDist1 = totem->mApplyRange1;
			boundDist2 = totem->mApplyRange2;
		}
	}
	else
	{
		boundDist1 = pStepInfo->mBoundDist1;
		boundDist2 = pStepInfo->mBoundDist2;
	}

	if( boundDist1 == 0 )
		return;
	else if( boundDist1 != 0 && boundDist2 != 0 )
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 101202 ), (float)(boundDist1 / 100), (float)(boundDist2 / 100) );
	else if( boundDist1 != 0 )
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100014 ), (float)(boundDist1 / 100) );
	else
		assert( 0 && "Bad Bound Distance Setting Value" );

	mRightTip[mRightIndex].mColor = color;
	CalcRightMaxWidth( mRightTip[mRightIndex].mString );
	mRightIndex++;
	if( mRightIndex> MAX_RIGHT_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

void cSkillTipWindow::SetRightApplyCnt( sPlayerSkillBaseInfo* pInfo, sPlayerSkillStepInfo* pStepInfo, unsigned long color )
{
	if( pInfo->mBoundType == eBOUNDTYPE_NONE )
		return;

	if( pStepInfo->mApplyCnt == 0 )
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 101201 ) ); /// 전체
	else
		mRightTip[mRightIndex].mString.Format( GAMERESOURCEMAN->GetSkillToolTipText( 101200 ), pStepInfo->mApplyCnt );

	mRightTip[mRightIndex].mColor = color;
	CalcRightMaxWidth( mRightTip[mRightIndex].mString );
	mRightIndex++;
	if( mRightIndex> MAX_RIGHT_TIP )
	{
		assert( 0 && "error tip index" );
	}
}
