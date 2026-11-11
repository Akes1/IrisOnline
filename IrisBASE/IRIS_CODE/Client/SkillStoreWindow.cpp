#include "StdAfx.h"
//#include "skillstorewindow.h"
//
//#include "icon.h"
//#include "StyleListBox.h"
//#include "label.h"
//
//#include "SkillScript.h"
//#include "SkillManager.h"
//#include "GameResourceManager.h"
//#include "uimanager.h"
//#include "GameUIManager.h"
//#include "NumberEditBox.h"
//#include "SkillTipWindow.h"
//#include "uimsgboxevent.h"
//#include "Hero.h"
//#include "ObjectManager.h"
//#include "NumberEditBox.h"
//
//cSkillStoreWindow::cSkillStoreWindow()
//: mpGoodList(0)
//, mpMoney(0)
//, mpSkillPoint(0)
//{
//	mCurPageIdx = 0;
//
//	mCheckBuySkillList = false;
//}
//
//cSkillStoreWindow::~cSkillStoreWindow()
//{
//	ExitPageData();
//}
//
//void cSkillStoreWindow::Open()
//{
//}
//
//void cSkillStoreWindow::Close()
//{
//	cUIWindow::Close();
//
//	UIMAN->HideTip();
//	UIMAN->HideNpcBackNoMsg();
//	SetMoney( 0 );
//
//	ExitPageData();
//
//	mCheckBuySkillList = false;
//}
//
//void cSkillStoreWindow::Show( unsigned long npcIndex )
//{
//	tArray<unsigned long>* parray = SKILLSCRIPT->GetStoreJobList( npcIndex );
//	if( parray == 0 )
//		return;
//
//	unsigned int page = 0;
//	ePLAYER_JOB job = HERO->GetJob();
//	for( unsigned int i=0; i<parray->GetSize(); i++ )
//	{
//		/// npc가 가지고 있는 종족에 포함되는지 검사
//		unsigned long jobIdx = (*parray)[i];
//		if( GAMERESOURCEMAN->IsCorrelateJob( job, jobIdx ) == true)
//		{
//			InitPageData( page, jobIdx );
//			page++;
//		}
//	}
//
//	ChangePage( 0 );
//
//	///
//	SetMoney( HERO->GetMoney() );
//
//	cUIWindow::Show();
//}
//
//void cSkillStoreWindow::OnHide()
//{
//	cUIWindow::OnHide();
//
//	UIMAN->HideTip();
//	UIMAN->HideNpcBack();
//	SetMoney( 0 );
//
//	ExitPageData();
//
//	mCheckBuySkillList = false;
//}
//
//
//bool cSkillStoreWindow::OnCreate( cUINodeProperty* pproperty )
//{	
//	if( cUIWindow::OnCreate( pproperty ) == false )
//		return false;
//
//	mpGoodList = (cStyleListBox*)GetChild( eUIID_GAME_SKILLSTORE_LIST );
//	mpMoney = (cLabel*)GetChild( eUIID_GAME_SKILLSTORE_MONEY );
//	mpSkillPoint = (cLabel*)GetChild( eUIID_GAME_SKILLSTORE_POINT );
//
//	SetMoney( 0 );
//	return true;
//}
//
//void cSkillStoreWindow::UpdateSkin()
//{
//	cUIWindow::UpdateSkin();
//
//	for( unsigned int i=0; i<mPageArray.GetSize(); i++ )
//	{
//		sPageData* p = (sPageData*)mPageArray[i];
//
//		cUIRect rt =  p->mRadio.GetRelativeRect();
//		p->mRadio.SetRelativePos( cUIPos(rt.mLeft + rt.GetWidth()*i, rt.mTop) );
//	}
//}
//
//void cSkillStoreWindow::OnProcess(unsigned long deltaTime, unsigned long accumTime)
//{
//	if( mCheckBuySkillList )
//	{
//		mCheckBuySkillList = false;
//
//		CheckBuyListColor();
//	}
//
//	cUIWindow::OnProcess( deltaTime, accumTime );
//}
//
//void cSkillStoreWindow::OnCommand( cUINode* /*pcaller*/, unsigned int id )
//{
//	/// 클로즈버튼누르면 안보임.
//	switch(id)
//	{
//	case eUIID_GAME_SKILLSTORE_CLOSEBTN:
//		{
//			/// 상점창을 닫으면서 별도의 처리도 필요
//			Hide();
//		}
//		break;
//	case eUIID_GAME_SKILLSTORE_TRAINBTN:
//		{
//			/// 구입
//			OnListBoxSelected( 0, 0 );
//		}
//		break;
//	}
//}
//
//void cSkillStoreWindow::OnListBoxRowHovered( cUINode* caller, unsigned int id, const cUIPos& pos, void* extraData )
//{
//	sSkillRowData* pdata = (sSkillRowData* )extraData;
//	if( pdata == 0 )
//		return;
//
//	if( SKILLMAN->CheckBuySkill( pdata->mSkillIdx,  pdata->mTrainStep ) == false )
//		mpGoodList->SetHoverTextColor( eCOLOR_RED );
//	else
//		mpGoodList->SetHoverTextColor( eCOLOR_GREEN );
//}
//
//void cSkillStoreWindow::OnListBoxImageHorvered( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& pos, void* extraData )
//{
//	sSkillRowData* pdata = (sSkillRowData* )extraData;
//	if( pdata == 0 )
//		return;
//
//	/// 툴팁 보여주기
//	UIMAN->ShowSkillTip( eTIP_SKILL, GAMEUI->GetSkillTip(), pos, pdata->mSkillIdx, pdata->mTrainStep-1, true );
//}
//		
//void cSkillStoreWindow::OnListBoxImageLeft( cUINode* /*caller*/, unsigned int /*id*/ )
//{
//	UIMAN->HideTip();
//}
//
//void cSkillStoreWindow::OnMouseLeft( const cUIPos& pos )
//{
//	cUIWindow::OnMouseLeft( pos );
//
//	UIMAN->HideTip();
//}
//
//void cSkillStoreWindow::OnRadioPressed( cUINode* pcaller, unsigned int id )
//{
//	pcaller;
//
//	if( id == mCurPageIdx+1 )
//		return;
//
//	if( id == 0 || mPageArray.GetSize() < id )
//	{
//		assert(0);
//		return;
//	}
//	ChangePage( (unsigned int)id-1 );
//}
//
//void cSkillStoreWindow::OnListBoxSelected( cUINode*, unsigned int )
//{
//	if( IsNetLock() == true )
//		return;
//
//	unsigned int idx = mpGoodList->GetSelectIndex();
//	sSkillRowData* pdata = (sSkillRowData*)mpGoodList->GetExtraData( idx );
//	if( pdata == 0 )
//		return;
//
//	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( pdata->mSkillIdx );
//	if( pInfo )
//	{
//		/// 마스터하였으면 알림메세지 띄우기
//		if( pdata->mMaster == true )
//		{
//			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
//				eMBEVENT_NONE, eSKIN_OK,
//				GAMERESOURCEMAN->GetGameText( 6002 ),
//				GAMERESOURCEMAN->GetGameText( 301 )
//				);
//		}
//		else
//		{
//			if( SKILLMAN->CheckBuySkill( pdata->mSkillIdx, pdata->mTrainStep ) == false )
//			{
//				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
//					eMBEVENT_NONE, eSKIN_OK,
//					GAMERESOURCEMAN->GetGameText( 6102 ),
//					GAMERESOURCEMAN->GetGameText( 301 )
//					);
//				return;
//			}
//
//			cStringT form;
//			form = _T("%s ");
//			form += GAMERESOURCEMAN->GetGameText( 6000 );
//
//			LPCTSTR money = cNumberEditBox::FormatNumber( pdata->mMoney );
//
//			cStringT str;
//			str.Format( form.Cstr(), pdata->mName, pdata->mTrainStep+1, pdata->mSp, money );
//
//			cStringT top;
//			top = GAMERESOURCEMAN->GetGameText( 2 );
//
//			/// 구매 확인 메세지 
//			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
//				eMBEVENT_SKILL_BUY, eSKIN_YESNO,
//				str.Cstr(),
//				top.Cstr(),
//				pdata );
//		}
//
//		UIMAN->HideTip();
//	}
//}
//
//void cSkillStoreWindow::InitPageData( unsigned int pageIdx, unsigned long jobIdx )
//{
//	cSkillTree* pTree = SKILLSCRIPT->GetSkillTree(jobIdx);
//	if( pTree == 0 )
//	{
//		assert(0);
//		return;
//	}
//
//	sPageData*	pPage = new sPageData;
//	if( pPage->mRadio.CreateBySkinName( "JobButton", this, pageIdx+1 ) == true )
//	{
//		cUIRect rt =  pPage->mRadio.GetRelativeRect();
//		pPage->mRadio.SetRelativePos( cUIPos(rt.mLeft + rt.GetWidth()*pageIdx, rt.mTop) );
//
//		LPCTSTR name = GAMERESOURCEMAN->GetJobName( jobIdx );
//		pPage->mRadio.SetText( name );
//	}
//	else
//	{
//		assert(0);
//	}
//
//	for( unsigned int i=0; i<pTree->mIconCount; i++ )
//	{
//		assert( i < 50 );
//		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( pTree->mIconList[i].skillIdx );
//		if( info )
//		{
//			/// 기본 제공 스킬이면서 단계가 1인 스킬은 제외한다.
//			if( info->mStepCount == 1 && 
//				info->mpSetpInfoArray[0].mPlayerLevel == 0 )
//				continue;
//
//			sSkillRowData* pdata = new sSkillRowData;
//			pdata->mSkillIdx = pTree->mIconList[i].skillIdx;
//			pdata->mName = GAMERESOURCEMAN->GetSkillNameText( info->mNameIdx );
//			pdata->mMaxstep = info->mStepCount;
//
//			cIconParam* pParam = UIMAN->GetIconParam( info->mIconIdx );
//			if( pParam )
//			{
//				pdata->mImage.SetTexture( pParam->mpTexture );
//
//				unsigned int tx = pParam->mTexPos.mX;
//				unsigned int ty = pParam->mTexPos.mY;
//
//				/// 이부분 수정요!!
//				unsigned int tw = 28;
//				unsigned int th = 28;
//				pdata->mImage.SetTextureRect( tx, ty, tx + tw, ty + th );
//				pdata->mImage.SetScreenRect( cUIRect(0,0,tw,th) );
//			}
//			else
//			{
//				assert(0);
//			}
//
//			pPage->mRowArray.PushBack( pdata );
//		}
//	}
//
//	/// page 등록
//	mPageArray.PushBack( pPage );
//
//	if( pageIdx == 0 )
//		pPage->mRadio.SetPress( true );
//}
//
//void cSkillStoreWindow::ExitPageData()
//{
//	if( mPageArray.IsEmpty() == false )
//	{
//		for( unsigned int i=0; i<mPageArray.GetSize(); i++ )
//		{
//			sPageData* p = (sPageData*)mPageArray[i];
//			RemoveChild( &p->mRadio );
//			delete p;
//		}
//		mPageArray.Clear();
//	}
//
//	/// 라디오 그룹 제거
//	mHeadRadioArr.Clear();
//}
//
//void cSkillStoreWindow::ChangePage( unsigned int pageIdx, bool useCurSel )
//{
//	if( mPageArray.GetSize() <= pageIdx )
//	{
//		assert(0);
//		return;
//	}
//
//	unsigned int selRow = 0;
//	unsigned int topRow = 0;
//	if( mpGoodList )
//	{
//		selRow = mpGoodList->GetSelectIndex();
//		topRow = mpGoodList->GetCurrentTopIndex();
//		mpGoodList->Clear();
//	}
//
//	mCurPageIdx = pageIdx;
//
//	SetMoney( HERO->GetMoney() );
//
//	sPageData* pPage = (sPageData*)mPageArray[mCurPageIdx];
//	for( unsigned int i=0; i<pPage->mRowArray.GetSize() ; i++ )
//	{
//		sSkillRowData* pdata = (sSkillRowData*)pPage->mRowArray[i];
//		unsigned char trainStep = 0;
//
//		sKeepSkill* pSkill = SKILLMAN->GetKeepInfo( pdata->mSkillIdx );
//		if( pSkill )
//			trainStep = pSkill->mStep+1;
//
//
//		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( pdata->mSkillIdx );
//		if( info == 0 )
//		{
//			assert(0);
//			continue;
//		}
//
//		unsigned long color = (unsigned long)eCOLOR_DEFAULT;
//		if( trainStep == pdata->mMaxstep )
//		{
//			pdata->mMaster = true;
//			pdata->mTrainStep = trainStep;
//
//			/// master한 경우
//			color = (unsigned long)eCOLOR_GRAY;
//
//			/// row add
//			if( mpGoodList )
//			{
//				int row = mpGoodList->AddRow( &pdata->mImage, pdata->mName, color, (void*)pdata );
//				if( row == -1 )
//				{
//					assert(0);
//					continue;
//				}
//
//				cStringT str = _T("-");
//				/// 요구 레벨
//				mpGoodList->AddColume( row, str.Cstr(), color );
//				/// 요구 sp
//				mpGoodList->AddColume( row, str.Cstr(), color );
//				/// 요구 금액
//				mpGoodList->AddColume( row, str.Cstr(), color );
//
//				/// 상태
//				str = _T("M");
//				mpGoodList->AddColume( row, str.Cstr(), color );
//
//			}
//		}
//		else
//		{
//			pdata->mMaster = false;
//
//			if( info->mStepCount <= trainStep )
//			{
//				assert(0);
//			}
//
//			sPlayerSkillStepInfo stepInfo = info->mpSetpInfoArray[trainStep];
//
//			pdata->mMoney = stepInfo.mUseMoney;
//			pdata->mSp = stepInfo.mRequireSP;
//			pdata->mTrainStep = trainStep;
//
//			/// 
//			if( SKILLMAN->CheckBuySkill( pdata->mSkillIdx, trainStep ) == false )
//				color = (unsigned long)eCOLOR_RED;
//
//			if( mpGoodList )
//			{
//				/// row add
//				int row = mpGoodList->AddRow( &pdata->mImage, pdata->mName, color, (void*)pdata );
//				if( row == -1 )
//				{
//					assert(0);
//					continue;
//				}
//
//				cStringT str;
//				/// 요구 레벨
//				str.Format( _T("%d"), stepInfo.mPlayerLevel );
//				mpGoodList->AddColume( row, str.Cstr(), color );
//				/// 요구 sp
//				str.Format( _T("%d"), stepInfo.mRequireSP );
//				mpGoodList->AddColume( row, str.Cstr(), color );
//				/// 요구 금액
//				str.Format( _T("%d"), stepInfo.mUseMoney );
//				mpGoodList->AddColume( row, str.Cstr(), color );
//				/// 상태
//				str.Format( _T("%d → %d"), trainStep, trainStep+1 );
//				mpGoodList->AddColume( row, str.Cstr(), color );
//			}
//		}
//	}
//
//	if( useCurSel && mpGoodList )
//	{
//		mpGoodList->ChangeTopIndex( topRow, false );
//		mpGoodList->ChangeSelectRow( selRow, false );
//	}
//}
//
//void cSkillStoreWindow::CheckBuyListColor()
//{
//	if( mPageArray.GetSize() <= mCurPageIdx )
//		return;
//
//	if( mpGoodList )
//	{
//		if( mpGoodList->GetRowCount() <= 0 )
//			return;
//
//		for( unsigned int row = 0; row<mpGoodList->GetRowCount(); row++ )
//		{
//			sSkillRowData* pdata = (sSkillRowData*)mpGoodList->GetExtraData( row );
//
//			if( pdata->mMaster == false )
//			{
//				unsigned long color = (unsigned long)eCOLOR_WHITE;
//				if( SKILLMAN->CheckBuySkill( pdata->mSkillIdx, pdata->mTrainStep) == false )
//					color = (unsigned long)eCOLOR_RED;
//
//				mpGoodList->ChangeTextColor( row, color );
//			}
//		}
//	}
//}
//
//void cSkillStoreWindow::SetMoney(unsigned int money )
//{
//	if( mpMoney )
//	{
//		LPCTSTR str = cNumberEditBox::FormatNumber( money );
//		mpMoney->SetText( str );
//	}
//
//	mCheckBuySkillList = true;
//}
//
//void cSkillStoreWindow::UpdateSkillPoint()
//{
//	if( mpSkillPoint )
//		mpSkillPoint->SetValue( HERO->GetSkillPointRemain() );
//
//	mCheckBuySkillList = true;
//}