#include "StdAfx.h"
#include "TitleWindow.h"
#include "ListBox.h"
#include "TextBox.h"
#include "TitleManager.h"
#include "SkillScript.h"
#include "GameResourceManager.h"

cTitleWindow::cTitleWindow()
: mpTitleList(0)
, mpTitleQuickList(0)
, mpDescBox(0)
, mpEffectBox(0)
, mpSelect(0)
{

}

cTitleWindow::~cTitleWindow()
{
	Clear();
}

void cTitleWindow::Clear()
{
	if( mpTitleList )
		mpTitleList->Clear();

	if( mpTitleQuickList )
		mpTitleQuickList->Clear();

	if( mpDescBox )
		mpDescBox->Clear();

	if( mpEffectBox )
		mpEffectBox->Clear();
}

void cTitleWindow::ClearQuick()
{
	if( mpTitleQuickList )
		mpTitleQuickList->Clear();
}

bool cTitleWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpTitleList = (cListBox*)GetChild( eUIID_TITLE_LISTBOX );
	mpTitleQuickList = (cListBox*)GetChild( eUIID_TITLE_QUICK_LISTBOX );
	mpDescBox = (cTextBox*)GetChild( eUIID_TITLE_DESC_TEXTBOX );
	mpEffectBox = (cTextBox*)GetChild( eUIID_TITLE_EFFECT_TEXTBOX );
	mpSelect = GetChild(eUIID_TITLE_SELECT);

	mSelectedList = eUIID_TITLE_DESC_TEXTBOX;

	if( mpTitleList )
		mpTitleList->SetContinueEvent( true );

	if( mpTitleQuickList )
		mpTitleQuickList->SetContinueEvent( true );

	if( mpDescBox)
		mpDescBox->SetDrawTextBottom( false );

	if( mpEffectBox )
		mpEffectBox->SetDrawTextBottom( false );
	return true;
}

void cTitleWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_TITLE_CLOSE:
	case eUIID_TITLE_CANCEL:
		Hide();
		break;
	case eUIID_TITLE_SELECT:
		{
			if( mSelectedList == eUIID_TITLE_LISTBOX)
			{
				SelectTitle( eUIID_TITLE_LISTBOX );
			}
			else if( mSelectedList == eUIID_TITLE_QUICK_LISTBOX)
			{
				SelectTitle( eUIID_TITLE_QUICK_LISTBOX );
			}
		}
		break;
	default:
		assert(0);
		return;
	}
}

void cTitleWindow::OnListBoxSelected( cUINode* node, unsigned int id )
{
	SelectTitle( id );
}

void cTitleWindow::OnListBoxChanged( cUINode* node, unsigned int id )
{
	if( mpTitleList || mpTitleQuickList || mpDescBox || mpEffectBox )
	{
		mpDescBox->Clear();
		mpEffectBox->Clear();

		cListBox* pSelectedList = NULL;

		if( id == eUIID_TITLE_LISTBOX)
		{
			mSelectedList = eUIID_TITLE_LISTBOX;
			pSelectedList = mpTitleList;
		}
		else if( id == eUIID_TITLE_QUICK_LISTBOX )
		{
			mSelectedList = eUIID_TITLE_QUICK_LISTBOX;
			pSelectedList = mpTitleQuickList;
		}

		int row = pSelectedList->GetSelectIndex();
		unsigned long titleIdx = (unsigned long)pSelectedList->GetExtraData( row );

		/// 설명 추가
		cTitleDefine* define = TITLEMAN->GetTitleDefine( titleIdx );
		if( define )
		{
			mpDescBox->AddNewRow( define->mDescription, COLOR_WHITE );
		}

		/// 효과 추가
		sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( define->mInfluenceIndex );
		if( pInfo )
		{
			cStringT temp;

			/// 효과값이 있으면 효과출력
			if( pInfo->mValue1 != 0 || pInfo->mValue2 != 0 || pInfo->mValue3 != 0 )
			{
				if( pInfo->mValue1 && pInfo->mValue2 && pInfo->mValue3 )
				{
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2, pInfo->mValue3 );
					mpEffectBox->AddNewRow( (LPCTSTR)temp.Cstr(), COLOR_WHITE );
				}
				else if( pInfo->mValue1 && pInfo->mValue2 )
				{
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2 );
					mpEffectBox->AddNewRow( (LPCTSTR)temp.Cstr(), COLOR_WHITE );
				}
				else if( pInfo->mValue1 )
				{
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1 );
					mpEffectBox->AddNewRow( (LPCTSTR)temp.Cstr(), COLOR_WHITE );
				}
			}
		}
	}
}

void cTitleWindow::SelectTitle( unsigned int nListType )
{
	if( mSelectedList == NULL || mpTitleQuickList == NULL )
	{
		assert(0);
		return;
	}

	if( nListType == eUIID_TITLE_LISTBOX)
	{
		mSelectedList = eUIID_TITLE_LISTBOX;

		/// 설정할 호칭 선택
		int row = mpTitleList->GetSelectIndex();
		unsigned long selectTitleIdx = (unsigned long)mpTitleList->GetExtraData( row );
		TITLEMAN->TitleSet( selectTitleIdx );

		if( mpTitleQuickList != NULL )
		{
			bool bSearch = false;
			typedef tArray< unsigned long > cTitleArr;
			cTitleArr tmpTitleArray;

			for( unsigned int iRow = 0; iRow < mpTitleQuickList->GetRowCount(); iRow++ )
			{
				unsigned long titleIdx = (unsigned long)mpTitleQuickList->GetExtraData( iRow );			

				if( selectTitleIdx == titleIdx )
				{
					bSearch = true;
					continue;
				}

				tmpTitleArray.PushBack( titleIdx );
			}

			if( bSearch == false )
			{
				unsigned long nTitleIdx = 0;

				if( tmpTitleArray.GetSize() >= TITLE_QUICK_LIST_COUNT )
				{
					mpTitleQuickList->Clear();

					cTitleArr::cIterator i		= tmpTitleArray.Begin();
					cTitleArr::cIterator iEnd	= tmpTitleArray.End();
					++i;

					for( ; i != iEnd; ++i )
					{
						nTitleIdx = (unsigned long)*i;

						AddTitle( nTitleIdx, true );						
					}
				}

				AddTitle( selectTitleIdx, true );	

				// 퀵리스트 재 설정
				TITLEMAN->ClearQuickList();

				unsigned char nCnt = (unsigned char)mpTitleQuickList->GetRowCount();

				for( unsigned char i = 0; i < nCnt; i++ )
				{
					TITLEMAN->AddHaveQuickTitle( i, (long)mpTitleQuickList->GetExtraData( i ) );
				}
			}

			tmpTitleArray.Clear();
		}	
	}
	else if( nListType == eUIID_TITLE_QUICK_LISTBOX )
	{
		mSelectedList = eUIID_TITLE_QUICK_LISTBOX;

		int row = mpTitleQuickList->GetSelectIndex();
		unsigned long titleIdx = (unsigned long)mpTitleQuickList->GetExtraData( row );
		TITLEMAN->TitleSet( titleIdx );
	}
}

/// 호칭 추가
bool cTitleWindow::AddTitle( unsigned long titleIdx, bool bQuick )
{
	if( !mpTitleList || !mpTitleQuickList || !mpDescBox || !mpEffectBox )
	{
		assert(0);
		return false;
	}

	cTitleDefine* define = TITLEMAN->GetTitleDefine( titleIdx );
	if( !define )
	{
		assert(0);
		return false;
	}

	/// 리스트에 삽입
	cStringT str;
	str.Format( _T("%03d. %s"), titleIdx, define->mName );

	if( bQuick == true )
	{
		mpTitleQuickList->AddRow( (LPCTSTR)str.Cstr(), define->mColor, (void*)titleIdx );		
	}
	else
	{
		mpTitleList->AddRow( (LPCTSTR)str.Cstr(), define->mColor, (void*)titleIdx );
	}

	return true;
}

void cTitleWindow::GetQuickTitle( long* pListArr )
{
	if( mpTitleQuickList == NULL )
	{
		assert(0);
		return;
	}

	for( unsigned int i = 0; i < mpTitleQuickList->GetRowCount(); i++, pListArr++ )
	{
		*pListArr = (long)mpTitleQuickList->GetExtraData( i );
	}
}

void cTitleWindow::Sort()
{
	if( mpTitleList )
		mpTitleList->Sort();
}

void cTitleWindow::OnNetLock( int lockTry )
{
	if( mpSelect )
		mpSelect->NetLock( lockTry );
}

void cTitleWindow::OnNetUnLock( int lockTry )
{
	if( mpSelect )
		mpSelect->NetUnLock( lockTry );
}
