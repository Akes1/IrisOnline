#include "stdafx.h"
#include "SkillPreviewSheet.h"
#include "SkillScript.h"
#include "ResourceManager.h"
#include "UIImage.h"
#include "SkillIcon.h"
#include "Button.h"
#include "GameUIManager.h"
#include "SkillPreviewWindow.h"
#include "TipWindow.h"
#include "GameResourceManager.h"
#include "SkillSheetWindow.h"

cSkillPreviewSheet::cSkillPreviewSheet( eUINodeType type )
:cUIWindow( type )
, mpTreeInfo(0)
, mBackTex(0)
, mJobIndex(0)
{

}

cSkillPreviewSheet::~cSkillPreviewSheet()
{
	mManagedIcon.Clear();
	mLevelupButtonMap.Clear();
}

void cSkillPreviewSheet::Clear()
{
	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator iend = mManagedIcon.End();
	for( ; i != iend; ++i )
	{
		cSkillIcon* icon = (cSkillIcon*)(*i).mSecond;
		if( icon == 0 ) 
			continue;

		RemoveChildComplete( icon );
	}
	mManagedIcon.Clear();

	//
	cLevelupButtonMap::cIterator i2 = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator iend2 = mLevelupButtonMap.End();
	for( ; i2 != iend2; ++i2 )
	{
		sLevelButtons* p = (sLevelButtons*)(*i2).mSecond;
		if( p == 0 ) 
			continue;

		RemoveChildComplete( p->upButton );
		RemoveChildComplete( p->downButton );

		SAFE_DELETE( p );
	}
	mLevelupButtonMap.Clear();

	mJobIndex = 0;
	mpTreeInfo = 0;
}

void cSkillPreviewSheet::OnShow()
{
	cUIWindow::OnShow();

	cSkillPreviewWindow* win = GAMEUI->GetSkillPreviewWindow();
	if( win )
	{
		// ui 갱신
		win->CheckInfo( mJobIndex );
		win->UpdateInfoUI( mJobIndex );
	}
	else
		assert(0);

	//
	ShowSkillUpButton();
}

void cSkillPreviewSheet::OnCommand( cUINode* caller, unsigned int id )
{
	if( !caller )
	{
		assert(0);
		return;
	}

	cSkillPreviewWindow* win = GAMEUI->GetSkillPreviewWindow();
	if( !win )
	{
		assert(0);
		return;
	}

	unsigned long skillIndex = id;
	sLevelButtons* buttonData = (sLevelButtons*)mLevelupButtonMap.GetAt( skillIndex );
	if( !buttonData )
	{
		assert(0);
		return;
	}
	
	sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
	if( info == 0 )
	{
		assert(0);
		return;
	}

	cLevelButton* button = (cLevelButton*)caller;

	sSkillData* curData = win->GetSkillData( skillIndex );
	if( !curData )
	{
		assert(0);
		return;
	}

	unsigned char curStep = curData->step;

	// 업버튼
	if( buttonData->upButton == button )
	{
		unsigned char nextStep = curStep + 1;

		/// 스킬 단계 값 체크
		if( nextStep > info->mStepCount )	
		{ 
			assert(0); 
			return;
		}

		sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[nextStep];

		// 선행스킬 관련 데이터 갱신
		if( stepInfo.mLearnSkillIdx1 > 0 )
		{
			sSkillData* skillData = win->GetSkillData( stepInfo.mLearnSkillIdx1 );
			if( skillData )
			{
				skillData->skillSet.Insert( skillIndex );
			}
			else
				assert(0);
		}

		if( stepInfo.mLearnSkillIdx2 > 0 )
		{
			sSkillData* skillData = win->GetSkillData( stepInfo.mLearnSkillIdx2 );
			if( skillData )
			{
				skillData->skillSet.Insert( skillIndex );
			}
			else
				assert(0);
		}

		// 찍은 스킬의 단계 늘리기
		win->UpdateSkillStep( skillIndex, nextStep );

		// 아이콘 갱신
		cSkillIcon* icon = GetSkillIcon( skillIndex );
		if( icon && icon->GetSkillIdx() == skillIndex )
		{
			icon->SetSkillInfo( skillIndex, nextStep, win->GetCurrentJobIndex(), eTIP_SKILLPREVIEW );
			icon->SetColor( NiColorA::WHITE );
		}
		else
			assert(0);
	}
	// 다운버튼
	else
	{
		if( curStep == UCHAR_MAX )
			assert(0);

		unsigned char nextStep = (unsigned char)curStep - 1;

		// 해당 스킬이 안 익힌상태로 돌아가면
		if( nextStep == UCHAR_MAX )
		{
			sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[0];

			/// 선행 스킬 정보로부터 삭제한다
			if( stepInfo.mLearnSkillIdx1 > 0 )
			{
				sSkillData* skillData = win->GetSkillData( stepInfo.mLearnSkillIdx1 );
				if( skillData )
				{
					skillData->skillSet.Erase( skillIndex );
				}
				else
					assert(0);
			}

			if( stepInfo.mLearnSkillIdx2 > 0 )
			{
				sSkillData* skillData = win->GetSkillData( stepInfo.mLearnSkillIdx2 );
				if( skillData )
				{
					skillData->skillSet.Erase( skillIndex );
				}
				else
					assert(0);
			}
		}
		
		// 찍은 스킬의 단계 줄이기
		win->UpdateSkillStep( skillIndex, nextStep );

		// 아이콘 갱신
		cSkillIcon* icon = GetSkillIcon( skillIndex );
		if( icon && icon->GetSkillIdx() == skillIndex )
		{
			icon->SetSkillInfo( skillIndex, nextStep, win->GetCurrentJobIndex(), eTIP_SKILLPREVIEW );
			
			if( icon->IsBaseSkill() || nextStep != UCHAR_MAX )
			{
				icon->SetColor( NiColorA::WHITE );
			}
			else
			{
				icon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
			}
		}
		else
			assert(0);
	}

	// 직업관련 포인트 및 레벨 정보 갱신
	win->CheckInfo( mJobIndex );
	win->UpdateInfoUI( mJobIndex );

	//
	ShowSkillUpButton();
}

void cSkillPreviewSheet::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mpTreeInfo && mJobIndex > 0 )
	{
		SetSkillTree( mJobIndex, mpTreeInfo );

		ShowSkillUpButton();
	}
}

void cSkillPreviewSheet::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}

	/// 상대영역을 절대영역으로 바꾼다 
	cUIRect captionRc = GetAbsoluteRect();
	captionRc.mLeft = GetAbsoluteRect().mLeft + mCaptionRect.mLeft;
	captionRc.mRight = captionRc.mLeft + mCaptionRect.GetWidth();
	captionRc.mTop = GetAbsoluteRect().mTop + mCaptionRect.mTop;
	captionRc.mBottom = captionRc.mTop + mCaptionRect.GetHeight();

	/// 캡션영역 안에 포인터가 있어야먄 드래그한다 
	if( mpParent == (cUIContainer*)GetRoot() && captionRc.ContainPoint( pos ) )
	{
		mDragStart.mX = pos.mX;
		mDragStart.mY = pos.mY;

		mDragging = true;
	}
}

void cSkillPreviewSheet::SetSkillTree( unsigned long jobIndex, cSkillTree* tree )
{
	typedef tArray<unsigned long> cSkillAry;

	if( !tree )
	{
		assert(0);
		return;
	}

	cSkillPreviewWindow* win = ((cSkillPreviewWindow*)mpParent);
	if( !win )
	{
		assert(0);
		return;
	}

	// 초기화
	Clear();

	mpTreeInfo = tree;
	mJobIndex = jobIndex;

	// UI 갱신
	cString pathName;
	pathName.Format( "./Data/2DData/%s", tree->mImageName.Cstr() );
	mBackTex = RESOURCEMAN->LoadTexture( pathName, false );
	if( mpImage == 0 )
		mpImage = new cUIImage( mBackTex );
	else
		mpImage->SetTexture( mBackTex );

	mpImage->SetTextureRect( 0, 0, mBackTex->GetWidth(), mBackTex->GetHeight() );
	mpImage->SetScreenRect( mAbsoluteRect );

	// 스킬 아이콘 생성 및 세팅
	for( unsigned int i = 0; i < tree->mIconCount; ++i )
	{
		unsigned long skillIndex = tree->mIconList[i].skillIdx;

		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
		if( info )
		{
			cIconParam* param = UIMAN->GetIconParam( info->mIconIdx );
			if( param )
			{
				cSkillIcon* pIcon = new cSkillIcon;
				if( pIcon->CreateBySkinName( "SkillIcon", this, i + 1 ) == false )
				{
					assert(0);
					continue;
				}
				pIcon->SetRelativePos( cUIPos(tree->mIconList[i].uiPosX, tree->mIconList[i].uiPosY) );
				pIcon->ChangeImage( param );

				// 이미익힌 스킬인지 검사
				unsigned char step = UCHAR_MAX;
				sSkillData* skillData = win->GetSkillData( skillIndex );
				if( !skillData )
				{
					/// 기본제공 스킬 테스트
					cSkillAry* pArray = SKILLSCRIPT->GetFreeSkill( info->mJobStep );
					if( pArray )
					{
						cSkillAry::cIterator i = pArray->Begin();
						cSkillAry::cIterator end = pArray->End();
						for( ; i != end; ++i )
						{
							unsigned long defaultSkillIdx = (unsigned long)(*i);
							if( defaultSkillIdx == skillIndex )
							{
								// 기본제공이면 하나 익힌걸로 한다
								step = 0;
								break;
							}
						}
					}
				}
				else
				{
					step = skillData->step;
				}

				pIcon->SetSkillInfo( skillIndex, step, jobIndex, eTIP_SKILLPREVIEW );

				if( pIcon->IsBaseSkill() || step != UCHAR_MAX )
				{
					pIcon->SetColor( NiColorA::WHITE );
				}
				else
				{
					pIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}

				//
				win->InsertKeepSkill( skillIndex, step );

				// 아이콘 세팅
				if( mManagedIcon.Insert( skillIndex, pIcon ) == false )
				{
					assert(0);
				}

				/// 기본 제공 스킬이면서 단계가 1인 스킬은 버튼 생성을 제외한다.
				if( info->mStepCount == 1 && info->mpSetpInfoArray[0].mPlayerLevel == 0 )
					continue;

				cLevelButton* upButton = new cLevelButton;
				if( upButton->CreateBySkinName( "SkillLevelUpButton", this, skillIndex ) == false )
				{
					assert(0);
					continue;
				}

				cLevelButton* downButton = new cLevelButton;
				if( downButton->CreateBySkinName( "SkillLevelDownButton", this, skillIndex ) == false )
				{
					assert(0);
					continue;
				}

				sLevelButtons* p = new sLevelButtons;
				if( p )
				{
					p->upButton = upButton;
					p->downButton = downButton;
				}

				cUIRect iconRect = pIcon->GetRelativeRect();
				cUIRect btnRect  = upButton->GetRelativeRect();

				int left = iconRect.mRight - btnRect.GetWidth() / 2;
				upButton->SetRelativePos( cUIPos( left, iconRect.mTop - btnRect.GetHeight() /2 ) );
				downButton->SetRelativePos( cUIPos( left, upButton->GetRelativeRect().mBottom ) );

				upButton->Hide();
				downButton->Hide();

				if( mLevelupButtonMap.Insert( skillIndex, p ) == false )
				{
					assert(0);
				}
			}
		}
		else
		{
			assert(0);
			return;
		}
	}
}

void cSkillPreviewSheet::InitSkillTree()
{
	typedef tArray<unsigned long> cSkillAry;

	cSkillPreviewWindow* win = ((cSkillPreviewWindow*)mpParent);
	if( !win )
	{
		assert(0);
		return;
	}

	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator e = mManagedIcon.End();
	for( ; i != e ; ++i )
	{
		unsigned long skillIdx = (*i).mFirst;
		cSkillIcon* pIcon = (cSkillIcon*)(*i).mSecond;
		if( pIcon )
		{
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
			if( info )
			{
				// 이미익힌 스킬인지 검사
				unsigned char step = UCHAR_MAX;
				
				/// 기본제공 스킬 테스트
				cSkillAry* pArray = SKILLSCRIPT->GetFreeSkill( info->mJobStep );
				if( pArray )
				{
					cSkillAry::cIterator i = pArray->Begin();
					cSkillAry::cIterator end = pArray->End();
					for( ; i != end; ++i )
					{
						unsigned long defaultSkillIdx = (unsigned long)(*i);
						if( defaultSkillIdx == skillIdx )
						{
							// 기본제공이면 하나 익힌걸로 한다
							step = 0;
							break;
						}
					}
				}

				pIcon->SetSkillInfo( skillIdx, (char)step, win->GetCurrentJobIndex(), eTIP_SKILLPREVIEW );

				if( pIcon->IsBaseSkill() || step != UCHAR_MAX )
				{
					pIcon->SetColor( NiColorA::WHITE );
				}
				else
				{
					pIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}

				// 익힌스킬로 취급
				if( step != UCHAR_MAX )
				{
					win->UpdateSkillStep( skillIdx, step );
				}
			}
			else
			{
				// 에러
				assert(0);
				pIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
			}
		}
	}

	// 직업관련 포인트 및 레벨 정보 갱신
	win->CheckInfo( mJobIndex );
	win->UpdateInfoUI( mJobIndex );

	ShowSkillUpButton();
}

// 싱크 맞추기
void cSkillPreviewSheet::SyncSkillTree()
{
	if( mJobIndex == 0 )
		return;

	typedef tArray<unsigned long> cSkillAry;

	cSkillPreviewWindow* win = GAMEUI->GetSkillPreviewWindow();
	if( !win )
	{
		assert(0);
		return;
	}

	cSkillIconList::cIterator i = mManagedIcon.Begin();
	cSkillIconList::cIterator e = mManagedIcon.End();
	for( ; i != e ; ++i )
	{
		unsigned long skillIdx = (*i).mFirst;
		cSkillIcon* pIcon = (cSkillIcon*)(*i).mSecond;
		if( pIcon )
		{
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
			if( info )
			{
				// 이미익힌 스킬인지 검사
				unsigned char step = UCHAR_MAX;

				sSkillData* data = win->GetSkillData( skillIdx );
				if( data )
				{
					step = data->step;
				}
				else
				{
					/// 기본제공 스킬 테스트
					//bool IsDefaultSkill = ( info->mJobStep == MAX_JOBSTEP ) ? true : false;
					cSkillAry* pArray = SKILLSCRIPT->GetFreeSkill( info->mJobStep );
					if( pArray )
					{
						cSkillAry::cIterator i = pArray->Begin();
						cSkillAry::cIterator end = pArray->End();
						for( ; i != end; ++i )
						{
							unsigned long defaultSkillIdx = (unsigned long)(*i);
							if( defaultSkillIdx == skillIdx )
							{
								// 기본제공이면 하나 익힌걸로 한다
								step = 0;
								break;
							}
						}
					}
				}

				pIcon->SetSkillInfo( skillIdx, (char)step, win->GetCurrentJobIndex(), eTIP_SKILLPREVIEW );

				if( pIcon->IsBaseSkill() || step != UCHAR_MAX )
				{
					pIcon->SetColor( NiColorA::WHITE );
				}
				else
				{
					pIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
				}

				// 익힌스킬로 취급
				if( step != UCHAR_MAX )
				{
					win->UpdateSkillStep( skillIdx, step );
				}
			}
			else
			{
				// 에러
				assert(0);
				pIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
			}
		}
	}

	// 직업관련 포인트 및 레벨 정보 갱신
	win->CheckInfo( mJobIndex );

	if( mVisible )
		win->UpdateInfoUI( mJobIndex );

	ShowSkillUpButton();
}

void cSkillPreviewSheet::ShowSkillUpButton()
{
	if( mLevelupButtonMap.IsEmpty() == true )
	{
		HideSkillUpButton();
		return;
	}

	cSkillPreviewWindow* win = GAMEUI->GetSkillPreviewWindow();
	if( !win )
	{
		assert(0);
		return;
	}

	cLevelupButtonMap::cIterator i = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator end = mLevelupButtonMap.End();
	for( ; i != end; ++i )
	{
		unsigned long  skillIdx = (unsigned long)(i->mFirst);
		sLevelButtons* p        = (sLevelButtons*)(i->mSecond);
		
		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pInfo == 0 )
		{
			assert(0);
			continue;
		}

		sSkillData* curData = win->GetSkillData( skillIdx );
		if( curData == 0 )
		{
			assert(0);
			continue;
		}

		unsigned char curStep = curData->step;
	
		// up 버튼 출력 여부
		if( p->upButton )
		{
			p->upButton->Hide();

			// 마스터 검사	
			if( curStep != UCHAR_MAX )
			{
				if( curStep + 1 == pInfo->mStepCount )
					continue;
			}

			// 이전 직업 총 사용 SP검사
			if( pInfo->mJobStep > 0 && pInfo->mJobStep < MAX_JOBSTEP )
			{
				unsigned long beforeJobIndex = mJobIndex;
				tArray<unsigned long>* parray = GAMERESOURCEMAN->GetParentJob( mJobIndex );
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
						continue;
				}
			}


			/// 미습득 스킬인 경우 현재 단계 0 으로 셋팅
			unsigned char nextStep = 0;
			if( curStep == UCHAR_MAX )
				curStep = 0;
			else
				nextStep = curStep + 1;

			sPlayerSkillStepInfo stepInfo = pInfo->mpSetpInfoArray[nextStep];

			// 선행 스킬 1
			if( stepInfo.mLearnSkillIdx1 != 0 )
			{
				sSkillData* learnData = win->GetSkillData( stepInfo.mLearnSkillIdx1 );
				if( !learnData )
				{
					assert(0);
					continue;
				}
				
				if( learnData->step == UCHAR_MAX )
					continue;

				if( learnData->step + 1 < stepInfo.mLearnSkillStep1 )
					continue;
			}

			// 선행 스킬 2
			if( stepInfo.mLearnSkillIdx2 != 0 )
			{
				sSkillData* learnData = win->GetSkillData( stepInfo.mLearnSkillIdx2 );
				if( !learnData )
				{
					assert(0);
					continue;
				}

				if( learnData->step == UCHAR_MAX )
					continue;

				if( learnData->step + 1 < stepInfo.mLearnSkillStep2 )
					continue;
			}

			p->upButton->Show();
		}
	}

	//
	cLevelupButtonMap::cIterator di = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator dend = mLevelupButtonMap.End();
	for( ; di != dend; ++di )
	{
		unsigned long  skillIdx = (unsigned long)(di->mFirst);
		sLevelButtons* p        = (sLevelButtons*)(di->mSecond);

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pInfo == 0 )
		{
			assert(0);
			continue;
		}

		sSkillData* curData = win->GetSkillData( skillIdx );
		if( curData == 0 )
		{
			assert(0);
			continue;
		}

		short curStep = curData->step;

		// down 버튼 출력 여부
		if( p->upButton && p->downButton )
		{
			p->downButton->Hide();

			if( curStep == UCHAR_MAX )
				continue;

			// 해당스킬을 선행스킬로 하는 스킬이 있으면 down 버튼 출력 불가 처리
			bool showDownButton = true;

			if( curData->skillSet.GetSize() != 0 )
			{
				///내가 물고 있는 스킬의 선행스킬들의 단계를 체크.
				tHashSet<unsigned long>::cConstIterator pos = curData->skillSet.Begin();
				tHashSet<unsigned long>::cConstIterator end = curData->skillSet.End();
				for( ; pos != end; ++pos )
				{
					unsigned long childSkillIdx = (*pos);
					sPlayerSkillBaseInfo* pChildInfo = SKILLSCRIPT->GetPlayerSkillInfo( childSkillIdx );
					if( pChildInfo == 0 )
					{
						assert(0);
						showDownButton = false;
						continue;
					}

					sSkillData* childData = win->GetSkillData( childSkillIdx );
					if( childData == 0 )
					{
						assert(0);
						showDownButton = false;
						continue;
					}

					/// 현재스킬을 선행스킬로 하는 스킬을 안 익혔다면 체크 할 필요없음.
					unsigned char childSkillStep = childData->step;
					if( childSkillStep == UCHAR_MAX )
						continue;
					
					sPlayerSkillStepInfo childStepInfo = pChildInfo->mpSetpInfoArray[childSkillStep];

					/// down 이미지 표시 해도 되는지 체크
					if( childStepInfo.mLearnSkillIdx1 == skillIdx )
					{
						if( childStepInfo.mLearnSkillStep1 >= (curStep+1) )
						{
							showDownButton = false;
							continue;
						}

					}
					else if( childStepInfo.mLearnSkillIdx2 == skillIdx )
					{
						if( childStepInfo.mLearnSkillStep2 >= (curStep+1) )
						{
							showDownButton = false;
							continue;
						}
					}
				}
			}
			
			if( showDownButton )
				p->downButton->Show();
		}
	}
}

void cSkillPreviewSheet::HideSkillUpButton()
{
	cLevelupButtonMap::cIterator i = mLevelupButtonMap.Begin();
	cLevelupButtonMap::cIterator end = mLevelupButtonMap.End();
	for( ; i != end ; ++i )
	{
		sLevelButtons* p = (sLevelButtons*)(i->mSecond);
		if( p ) 
		{
			if( p->upButton )
				p->upButton->Hide();
			if( p->downButton )
				p->downButton->Hide();

		}
	}
}
