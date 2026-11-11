#include "StdAfx.h"
#include "UIWindow.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UINodeProperty.h"
#include "UIRootNode.h"
#include "UIImage.h"
#include "Label.h"
#include "ComboBox.h"
#include "Button.h"
#include "EditBox.h"
#include "ListBox.h"
#include "DoubleItemIconListBox.h"
#include "ScrollBar.h"
#include "BarButton.h"
#include "SpinBox.h"
#include "Gauge.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "TabWindow.h"
#include "PushButton.h"
#include "NumberEditBox.h"
#include "TabButton.h"
#include "RadioButton.h"
#include "MultiEditBox.h"
#include "OptionGauge.h"
#include "Icon.h"
#include "FriendWindow.h"
#include "LootingWindow.h"
#include "ListFolderBox.h"
#include "ItemMixWindow.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealSearchSheet.h"
#include "MailSheet.h"
#include "BoardListWindow.h"
#include "BoardQuestWindow.h"
#include "QuestWindow.h"

#include "InvenSheetWindow.h"
#include "InvenEquipWindow.h"
#include "WareSheetWindow.h"
#include "OptionSheet.h"

#include "TarotOpenWindow.h"
#include "TarotCheckWindow.h"
#include "TarotSelectWindow.h"
#include "TarotResultWindow.h"

#include "BuffWindow.h"
#include "PartyBuff.h"
#include "TargetWindow.h"

#include "DataSheetWindow.h"
#include "TarotSheetWindow.h"
#include "MonsterSheetWindow.h"
#include "StorySheetWindow.h"

#include "SoundSystem.h"
#include "BitArray.h"

#include "SkillStepListWindow.h"
#include "SkillStepListBox.h"

#include "TwitterWindow.h"

cUIWindow::cUIWindow( eUINodeType type )
: cUIContainer( type )
, mDragging( false )
, mTimerEnabled( false )
, mTimeOut( 0 )
, mAccumTime( 0 )
, mLastAccumTime( 0 )
, mDragStart(0, 0)
, mMsgHandler( 0 )
, mAlphaBitArray( 0 )
, mModalType( eMSGBOX_NONE )
, mpCaller(0)
, mpRegistMsgBox(0)
, mOpenSoundIdx( ULONG_MAX )
, mCloseSoundIdx( ULONG_MAX )
, mEventType( eMBEVENT_NONE )
{
	mCaptionRect.Set( 0, 0, 0, 0);

	mProcessEnable = true;
}

cUIWindow::~cUIWindow()
{
	mpRegistMsgBox = 0;
}

bool cUIWindow::HandleEvent( const cUIEvent& event )
{
	if( mVisible == false || mEnabled == false )
		return false;

	if( cUIContainer::HandleEvent( event ) == false )
		return false;

	return true;
}

bool cUIWindow::Reset()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return false;
	}

	unsigned int x = pnodeSkin->mSkinInfo->mX;
	unsigned int y = pnodeSkin->mSkinInfo->mY;
	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		x = ( GetScreenWidth() * pnodeSkin->mSkinInfo->mX ) / mDefaultWidth;
		y = ( GetScreenHeight() * pnodeSkin->mSkinInfo->mY ) / mDefaultHeight;

		/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
		/// 붙어있도록 한다 
		if( (pnodeSkin->mSkinInfo->mX + pnodeSkin->mSkinInfo->mWidth) >= (int)mDefaultWidth )
		{
			x = GetScreenWidth() - w;
		}

		if( (pnodeSkin->mSkinInfo->mY + pnodeSkin->mSkinInfo->mHeight) >= (int)mDefaultHeight )
		{
			y = GetScreenHeight() - h;
		}
	}

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

// stage 변경 시점에만온다
void cUIWindow::Open()
{
	if( mVisible )
		return;

	UIMAN->GotoFrontNode( this );
	SetVisible( true );
}

// stage 변경 시점에만온다
void cUIWindow::Close()
{
	if( mVisible == false )
		return;

	if( mpRegistMsgBox )
	{
		UIMAN->DestroyMsgBox( mpRegistMsgBox );
	}

	SetVisible( false );

	if( mModalType != eMSGBOX_NONE )
		UIMAN->DestroyMsgBox( this );

	ReleaseCapture();
	ReleaseHover();

	if( mDragging == true )
		mDragging = false;
}

void cUIWindow::ShowModal( eMSGBox_Event eventType, eMSGBox_Type modalType )
{
	if( mVisible )
		return;

	if( mOpenSoundIdx != ULONG_MAX )
		SOUNDSYS->Play2DSound( mOpenSoundIdx );

	mModalType = modalType;
	mEventType = eventType;

	UIMAN->GotoFrontNode( this );
	SetVisible( true );
	UIMAN->SetModal( this );

	OnShow();
}

void cUIWindow::Show( bool onsound )
{
	if( mVisible )
		return;

	UIMAN->GotoFrontNode( this );

	if( onsound && mOpenSoundIdx != ULONG_MAX )
		SOUNDSYS->Play2DSound( mOpenSoundIdx );

	SetVisible( true );

	OnShow();
}

void cUIWindow::Hide( bool onsound )
{
	if( mVisible == false )
		return;

	if( mModalType != eMSGBOX_NONE )
		UIMAN->DestroyMsgBox( this );

	mModalType = eMSGBOX_NONE;

	if( onsound && mCloseSoundIdx != ULONG_MAX )
		SOUNDSYS->Play2DSound( mCloseSoundIdx );

	SetVisible( false );

	OnHide();
}

void cUIWindow::HideESC( bool onsound )
{
	Hide( onsound );
}

void cUIWindow::OnShow()
{
	CheckEventNode();
}	

void cUIWindow::OnHide()
{
	// 자신앞으로 등록된 메세지 박스 닫기
	if( mpRegistMsgBox )
	{
		UIMAN->DestroyMsgBox( mpRegistMsgBox );
	}

	ReleaseCapture();
	ReleaseHover();

	if( mDragging == true )
		mDragging = false;
}

void cUIWindow::RegistMsgBox( cUIWindow* n )
{
	if( n )
	{
		mpRegistMsgBox = n;
	}
	else
		assert(0);
}

void cUIWindow::DeleteCallerMsgBox()
{
	if( mpCaller )
	{
		mpCaller->mpRegistMsgBox = 0;
	}
}

void cUIWindow::NetLock( int lockTry )
{
	OnNetLock( lockTry );
}

void cUIWindow::NetUnLock( int lockTry )
{
	OnNetUnLock( lockTry );
}

/// 자동 visible 토글 
void cUIWindow::Toggle()
{
	if( IsVisible() == true )
	{
		Hide();
	}
	else
	{
		Show();
	}
}

///
bool cUIWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( pproperty == 0 )
		return true;

	typedef tPointerList<void*> cPropList;
	const cPropList& propList = pproperty->GetChildList();
	cPropList::cConstIterator i = propList.Begin();
	cPropList::cConstIterator end = propList.End();

	for( ; i != end; ++i )
	{
		cUINodeProperty* childProp = (cUINodeProperty*)(*i);
		cUINode* child = 0;

		switch( childProp->GetType() )
		{
		case eUINODE_WINDOW:
		case eUINODE_TABSHEET:
			child = CreateChildWindow( childProp->GetType(), childProp->mSkinName.Cstr() );
			break;
		case eUINODE_LISTBOX:
			child = CreateChildListBox( childProp->GetType(), childProp->mSkinName.Cstr() );
			break;
		case eUINODE_LABEL:
			child = new cLabel;
			break;
		case eUINODE_COMBOBOX:
			child = new cComboBox;
			break;
		case eUINODE_BUTTON:
			child = new cButton;
			break;
		case eUINODE_EDITBOX:
			child = new cEditBox;
			break;
		case eUINODE_SCROLLBAR:
			child = new cScrollBar;
			break;
		case eUINODE_BARBUTTON:
			child = new cBarButton;
			break;
		case eUINODE_SPINBOX:
			child = new cSpinBox;
			break;
		case eUINODE_GAUGE:
			child = new cGauge;
			break;
		case eUINODE_GAUGETB:
			child = new cGaugeTB;
			break;
		case eUINODE_DOUBLEGAUGE:
			child = new cDoubleGauge;
			break;
		case eUINODE_TEXTBOX:
			child = new cTextBox;
			break;
		case eUINODE_CHECKBOX:
			child = new cCheckBox;
			break;
		case eUINODE_TABWINDOW:
			child = new cTabWindow;
			break;
		case eUINODE_PUSHBUTTON:
			child = new cPushButton;
			break;
		case eUINODE_NUMBEREDIT:
			child = new cNumberEditBox;
			break;
		case eUINODE_TABBUTTON:
			child = new cTabButton;
			break;
		case eUINODE_RADIOBUTTON:
			child = new cRadioButton;
			break;
		case eUINODE_MULTIEDITBOX:
			child = new cMultiEditBox;
			break;
		case eUINODE_OPTIONGAUGE:
			child = new cOptionGauge;
			break;
		case eUINODE_ICON:
			child = new cIcon;
			break;
		case eUINODE_LISTFOLDERBOX:
			{
				const cString& skinName = childProp->mSkinName.Cstr();
				if( skinName.Compare( "Quest_Folder" ) == 0 )
				{
					child = new cQuestListFolder;
				}
				else if( skinName.Compare( "ItemMix_SkillList" ) == 0 )
				{
					child = new cItemMixListFolder;
				}
				else
				{
					child = new cListFolderBox;
				}
			}
			break;
		default:
			assert( 0 && "type is incorrect!" );
			return false;
		}

		/// 자식을 생성
		if( child->CreateByProperty( childProp, this ) == false )
		{
			delete child;
			return false;
		}
	}

	return true;
}

/// 주의사항 : 
bool cUIWindow::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_WINDOW ) == false )
	{
		assert( 0 && "not window type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cUIWindowSkin* p = (cUIWindowSkin*)pskin;

	/// 캡션영역을 지정 
	mCaptionRect = p->mCaptionRect;
	if( mCaptionRect.GetWidth() < 0 || mCaptionRect.GetHeight() < 0 )
	{
		assert( 0 && "caption rect is 0" );
		return false;
	}

	mOpenSoundIdx = p->mOpenSoundIdx;
	mCloseSoundIdx = p->mCloseSoundIdx;

	/// 알파값을 저장 
	SetAlpha( p->mAlpha );

	/// view 세팅 가능 여부를 설정
	SetHoldView( p->mHoldView );

	///
	mAlphaBitArray = p->mAlphaBitArray;
	return true;
}

/// 주의사항 : 
void cUIWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mTimerEnabled == false )
		return;

	/// 타이머
	mAccumTime = accumTime;

	if( mAccumTime - mLastAccumTime >= mTimeOut )
	{
		OnTimer();
		mLastAccumTime = mAccumTime;
	}
}

void cUIWindow::OnPreRender()
{
	if( mVisible == false )
		return;

	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnPreRender();
		}
	}
}

/// 주의사항 : 
void cUIWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}
	pKeeper->DrawAll();
}

void cUIWindow::OnOk()
{
	Hide();
}

void cUIWindow::OnCancel()
{
	Hide();
}


void cUIWindow::SetTimer( bool enabled, unsigned long timeout )
{
	mTimerEnabled = enabled;
	mTimeOut = timeout;
	mLastAccumTime = mAccumTime;
}

void cUIWindow::OnTimer()
{
}

/// 주의사항 : 
void cUIWindow::OnMouseMove( const cUIPos& pos )
{
	if( mEnabled && mVisible )
	{
		if( mDragging )
		{
			Move( cUIPos( pos.mX - mDragStart.mX, pos.mY - mDragStart.mY ) );
			mDragStart = pos;
		}
	}	
}

/// 주의사항 : 
void cUIWindow::OnLButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	/// 윈도우도 앞으로 가게 한다.
	UIMAN->GotoFrontNode( this );

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
	if( mpParent == GetRoot() && captionRc.ContainPoint( pos ) )
	{
		mDragStart.mX = pos.mX;
		mDragStart.mY = pos.mY;

		mDragging = true;
	}
}

/// 주의사항 : 
void cUIWindow::OnLButtonUp( const cUIPos& )
{
	ReleaseCapture();
	mDragging = false;
}

///
void cUIWindow::OnCaptureLost()
{
	mDragging = false;
}

/// 주의사항 : 
void cUIWindow::SetAlpha( float a )
{
	cUINode::SetAlpha( a );

	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
		((cUINode*)(*i))->SetAlpha( a );
}

/// 주의사항:
cUIRect cUIWindow::GetCaptionRect() const
{
	return mCaptionRect;
}

void cUIWindow::SetCaptionRect( cUIRect& rect )
{
	/// 원래 윈도우 크기보다 클 경우 윈도우 크기로 맞춤
	if( rect.GetWidth() > mAbsoluteRect.GetWidth() )
	{
		assert(0);
		rect.mRight = rect.mLeft + mAbsoluteRect.GetWidth();
	}
	
	if( rect.GetHeight() > mAbsoluteRect.GetHeight() )
	{
		assert(0);
		rect.mBottom = rect.mTop + mAbsoluteRect.GetHeight();
	}

	mCaptionRect = rect;
}

/// 특별한 모양의 시트를 스크립트에서도 생성할 수 있도록 함.
/// 자식으로 특별한 윈도우를 가지는 경우에 여기에 특별한 윈도우 타입으로 생성할수 있게 한다.
cUIWindow* cUIWindow::CreateChildWindow( eUINodeType type, const cString& name )
{
	if( name.Compare( "InvenSheet_Window" ) == 0 )
	{
		cInvenSheetWindow* p = new cInvenSheetWindow;
		return p;
	}	
	else if( name.Compare( "InvenEquip_Window" ) == 0 )
	{
		cInvenEquipWindow* p = new cInvenEquipWindow;
		return p;
	}
	else if( name.Compare( "WareSheet_Window" ) == 0 )
	{
		cWareSheetWindow* p = new cWareSheetWindow;
		return p; 
	}
	else if( name.Compare( "Option_Sheet1" ) == 0 )
	{
		cOptionSheet1* p = new cOptionSheet1;
		return p; 
	}
	else if( name.Compare( "Option_Sheet2" ) == 0 )
	{
		cOptionSheet2* p = new cOptionSheet2;
		return p; 
	}
	else if( name.Compare( "Option_Sheet3" ) == 0 )
	{
		cOptionSheet3* p = new cOptionSheet3;
		return p; 
	}
	else if( name.Compare( "TarotOpenWindow" ) == 0 )
	{
		cTarotOpenWindow* p = new cTarotOpenWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "TarotCheckWindow" ) == 0 )
	{
		cTarotCheckWindow* p = new cTarotCheckWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "TarotSelectWindow" ) == 0 )
	{
		cTarotSelectWindow* p = new cTarotSelectWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "TarotResultWindow" ) == 0 )
	{
		cTarotResultWindow* p = new cTarotResultWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "NpcTarotCheckWindow" ) == 0 )
	{
		cNpcTarotCheckWindow* p = new cNpcTarotCheckWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "NpcTarotSelectWindow" ) == 0 )
	{
		cNpcTarotSelectWindow* p = new cNpcTarotSelectWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "NpcTarotResultWindow" ) == 0 )
	{
		cNpcTarotResultWindow* p = new cNpcTarotResultWindow;
		p->SetExceptEvent( true );
		return p;
	}
	else if( name.Compare( "PartyBuff" ) == 0 )
	{
		cPartyBuff* p = new cPartyBuff;
		return p;
	}
	else if( name.Compare( "TargetBuff" ) == 0 )
	{
		cTargetBuff* p = new cTargetBuff;
		return p;
	}
	else if( name.Compare( "DataSheet_Window" ) == 0 )
	{
		cDataSheetWindow* p = new cDataSheetWindow;
		return p;
	}
	else if( name.Compare( "TarotSheet_Window" ) == 0 )
	{
		cTarotSheetWindow* p = new cTarotSheetWindow;
		return p;
	}
	else if( name.Compare( "MonsterSheet_Window" ) == 0 )
	{
		cMonsterSheetWindow* p = new cMonsterSheetWindow;
		return p;
	}
	else if( name.Compare( "StorySheet_Window" ) == 0 )
	{
		cStorySheetWindow* p = new cStorySheetWindow;
		return p;
	}
	else if( name.Compare( "NpcDeal_SearchSheet" ) == 0 )
	{
		cNpcDealSearchSheet* p = new cNpcDealSearchSheet;
		return p;
	}
	else if( name.Compare( "NpcDeal_RegistSheet" ) == 0 )
	{
		cNpcDealRegistSheet* p = new cNpcDealRegistSheet;
		return p;
	}
	else if( name.Compare( "Mail_ListSheet" ) == 0 )
	{
		cMailListSheet* p = new cMailListSheet;
		return p;
	}
	else if( name.Compare( "Mail_ReadSheet" ) == 0 )
	{
		cMailReadSheet* p = new cMailReadSheet;
		return p;
	}
	else if( name.Compare( "Mail_SendSheet" ) == 0 )
	{
		cMailSendSheet* p = new cMailSendSheet;
		return p;
	}
	else if( name.Compare( "BoardList_Window" ) == 0 )
	{
		cBoardListWindow* p = new cBoardListWindow;
		return p;
	}
	else if( name.Compare( "BoardQuest_Window" ) == 0 )
	{
		cBoardQuestWindow* p = new cBoardQuestWindow;
		return p;
	}
	else if( name.Compare( "Skill_GradeList") == 0 )
	{
		cSkillStepListWindow* p = new cSkillStepListWindow;
		return p;
	}	

	return new cUIWindow(type);
}

cListBox* cUIWindow::CreateChildListBox(eUINodeType type, const cString& name )
{
	if( name.Compare( "Friend_ListBox" ) == 0 )
	{
		cFriendListbox* p = new cFriendListbox;
		return p;
	}
	else if( name.Compare( "Looting_List" ) == 0 )
	{
		cImageAndFontDataListBox* p = new cImageAndFontDataListBox;
		return p;	
	}
	else if( name.Compare( "Note_ListBox" ) == 0 )
	{
		cCheckBoxListBox* p = new cCheckBoxListBox;
		return p;
	}
	else if( name.Compare( "NpcStore_List" ) == 0 )
	{
		cDoubleItemIconListBox* p = new cDoubleItemIconListBox;
		return p;
	}
	else if( name.Compare( "ItemMix_List" ) == 0 )
	{
		cDoubleItemIconListBox* p = new cDoubleItemIconListBox;
		return p;
	}
	else if( name.Compare( "NpcDeal_SearchItemList" ) == 0 )
	{
		cImageAndFontDataListBox* p = new cImageAndFontDataListBox;
		return p;
	}
	else if( name.Compare( "NpcDeal_RegistItemList" ) == 0 )
	{
		cImageAndFontDataListBox* p = new cImageAndFontDataListBox;
		return p;
	}
	else if( name.Compare( "MailList_ListBox" ) == 0 )
	{
		cCheckBoxListBox* p = new cCheckBoxListBox;
		return p;
	}
	else if( name.Compare( "Skill_SkillGradeList") == 0 )
	{
		cSkillStepListBox* p = new cSkillStepListBox;
		return p;
	}
	else if( name.Compare( "Twitter_SearchItemList" ) == 0 )
	{
		cTwitterListBox* p = new cTwitterListBox;
		return p;
	}
	return new cListBox(type);
}

bool cUIWindow::ContainPoint( const cUIPos& pos ) const
{
	if( cUINode::ContainPoint( pos ) == false )
		return false;

	if( mpImage && mAlphaBitArray )
	{
		int x = pos.mX - mAbsoluteRect.mLeft + mpImage->GetTextureRect().mLeft;
		int y = pos.mY - mAbsoluteRect.mTop + mpImage->GetTextureRect().mTop;

		if( x < 0 || x >= (int)mAlphaBitArray->mWidth )
		{
			//assert( 0 );
			return false;
		}
		if( y < 0 || y >= (int)mAlphaBitArray->mHeight )
		{
			//assert( 0 );
			return false;
		}
		if( mpImage->GetTextureRect().ContainPoint( cUIPos(x,y) ) == false )
		{
			return false;
		}

		return (*mAlphaBitArray)[mAlphaBitArray->mWidth * y + x];
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
cUIWindowSkin::cUIWindowSkin( eUINodeType type )
: cUIContainerSkin( type )
, mAlpha( 1.0f )
, mHoldView( false )
, mAlphaBitArray( 0 )
{
	mCaptionRect.Set( 0, 0, 0, 0 );

	mOpenSoundIdx = ULONG_MAX;
	mCloseSoundIdx = ULONG_MAX;
}

cUIWindowSkin::~cUIWindowSkin()
{
}

/// 주의사항 : 
bool cUIWindowSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpTexture = UIMAN->GetSkin()->GetTexture( i );

				/// 해당 인덱스의 텍스쳐가 존재하지 않으면,
				if( !mpTexture )
				{
					assert( 0 );
					return false;
				}

				mAlphaBitArray = UIMAN->GetSkin()->GetAlphaBitArray( i );
			}
			break;
		case eTOKEN_OPENSOUND:
			{
				/// show 시 사운드
				mOpenSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_CLOSESOUND:
			{
				/// hide 시 사운드
				mCloseSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_HOLDVIEW:
			{
				/// mHoldView 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mHoldView = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mHoldView = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		case eTOKEN_CAPTIONXY:
			{
				mCaptionRect.mLeft = parser.ParseInt();
				mCaptionRect.mTop = parser.ParseInt();
			}
			break;
		case eTOKEN_CAPTIONWH:
			{
				mCaptionRect.mRight = mCaptionRect.mLeft + parser.ParseInt();
				mCaptionRect.mBottom = mCaptionRect.mTop + parser.ParseInt();
			}
			break;
		case eTOKEN_ALPHA:
			{
				mAlpha = parser.ParseFloat();
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}

cUINodeSkin* cUIWindowSkin::Create( const cString& name )
{
	if( name.Compare( "BuffWindow" ) == 0 || 
		name.Compare( "DeBuffWindow" ) == 0 || 
		name.Compare( "MallBuffWindow" ) == 0 || 
		name.Compare( "DuelBuffWindow1" ) == 0 || 
		name.Compare( "DuelDeBuffWindow1" ) == 0 || 
		name.Compare( "DuelBuffWindow2" ) == 0 || 
		name.Compare( "DuelDeBuffWindow2" ) == 0 || 
		name.Compare( "PartyBuff" ) == 0 || 
		name.Compare( "TargetBuff" ) == 0 )
	{
		return new cBuffWindowSkin;
	}

	return new cUIWindowSkin;
}
