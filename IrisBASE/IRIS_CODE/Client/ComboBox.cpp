#include "StdAfx.h"
#include "ComboBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "ListBox.h"
#include "Button.h"
#include "EditBox.h"

#include "InputSystem.h"

cComboBox::cComboBox( eUINodeType type )
: cUIWindow( type )
, mpListBox(0)
, mpButton(0)
, mpEdit(0)
{
	mSize.mWidth = 0;
	mSize.mHeight = 0;
}

cComboBox::~cComboBox()
{
	mpEdit = 0;
	mpButton = 0;
	mpListBox = 0;
}

void cComboBox::Clear()
{
	if( mpEdit )
		mpEdit->Clear();

	if( mpListBox )
		mpListBox->Clear();

	if( mpButton )
		mpButton->Hide();
}

bool cComboBox::IsEmpty()
{
	return ( mpListBox && mpListBox->GetRowCount() > 0 ) ? false : true;
}

/// 주의사항 : 생성되자마자 해야할 일 처리
bool cComboBox::OnCreate( cUINodeProperty* )
{
	mpListBox = new cListBox;
	mpListBox->SetScrollSkin( mScrollSkin );
	if( mpListBox->CreateBySkinName( mListboxSkin, this, eCOMBOBOX_LISTBOX ) == false )
	{
		return false;
	}

	mpButton = new cButton;
	if( mpButton->CreateBySkinName( mButtonSkin, this, eCOMBOBOX_BUTTON ) == false )
	{
		return false;
	}

	mpEdit = new cEditBox;
	if( mpEdit->CreateBySkinName( mEditSkin, this, eCOMBOBOX_EDITBOX ) == false )
	{
		return false;
	}

	/// 기본위치 지정
	cUIRect editRc = mpEdit->GetRelativeRect();

	cUIPos buttonPos;
	buttonPos.mX = editRc.mRight;
	buttonPos.mY = mpButton->GetRelativeRect().mTop;
	mpButton->SetRelativePos( buttonPos );

	/// 콤보박스 사이즈 저장
	mSize.mWidth = editRc.GetWidth() + mpButton->GetRelativeRect().GetWidth() + editRc.mLeft;
	mSize.mHeight = editRc.GetHeight();
	SetRelativeSize( mSize );

	/// 이벤트 요청
	mpListBox->SetContinueEvent( true );

	/// 리스트 박스 초기값으로 안보임.
	mpListBox->Hide();

	///
	UIMAN->AddEventOtherClick( this );
	return true;
}


/// 주의사항 : 
bool cComboBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_COMBOBOX ) == false )
	{
		assert( 0 && "not combobox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	cComboBoxSkin* p = (cComboBoxSkin*)pskin;

	/// 자식 스킨 이름 적용
	mListboxSkin = p->mListboxSkin;
	mButtonSkin = p->mButtonSkin;
	mEditSkin = p->mEditSkin;
	mScrollSkin = p->mScrollSkin;

	cUISkin* pUISkin = UIMAN->GetSkin();
	if( !pskin )
	{
		assert(0);
		return false;
	}

	/// 스킨 네임 유효성 검사
	if( mListboxSkin && pUISkin->GetNodeSkin( mListboxSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mButtonSkin && pUISkin->GetNodeSkin( mButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mEditSkin && pUISkin->GetNodeSkin( mEditSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}

	return true;
}

/// 주의사항 : 
bool cComboBox::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return mpParent->HandleEvent( event );
	}

	switch( event.mType )
	{
	case eUIEVENT_COMMAND:
		CheckButtonEvent( event.mpCaller, event.mID );
		break;
	case eUIEVENT_LISTBOX_CHANGED:
		OnLButtonDown( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()), event.mControl, event.mAlt, event.mShift );
		break;
	case eUIEVENT_MOUSE_OTHERCLICK:
		OnMouseOtherClick( event.mPos );
		break;
	default:
		return cUINode::HandleEvent( event );
	}
	return true;
}

void cComboBox::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 기본위치 지정
	if( mpEdit && mpButton )
	{
		cUIRect editRc = mpEdit->GetRelativeRect();

		cUIPos buttonPos;
		buttonPos.mX = editRc.mRight;
		buttonPos.mY = mpButton->GetRelativeRect().mTop;
		mpButton->SetRelativePos( buttonPos );

		/// 콤보박스 사이즈 저장
		mSize.mWidth = editRc.GetWidth() + mpButton->GetRelativeRect().GetWidth() + editRc.mLeft;
		mSize.mHeight = editRc.GetHeight();
		SetRelativeSize( mSize );
	}

	UpdateListboxSize();
}

/// 주의사항 : 
void cComboBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );

	if( !mpListBox )
	{
		assert(0);
		return;
	}

	cUIRect listRc = mpListBox->GetAbsoluteRect();

	if( listRc.ContainPoint( pos ) == true )
	{
		/// 텍스트를 얻어와서
		unsigned int selectIdx = mpListBox->GetSelectIndex();
		mEditText = mpListBox->GetText( selectIdx );
		mpEdit->SetText( mEditText.Cstr() );

		/// event 발생
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_COMBO_SELECTED;
		mpParent->HandleEvent( event );

		mpListBox->Hide();

		/// 원래대로 변경
		SetRelativeSize( mSize );
	}
}


void cComboBox::OnHide()
{
	cUIWindow::OnHide();

	if( mpListBox )
		mpListBox->Hide();

	if( mSize.mHeight != 0 )
		SetRelativeSize( mSize );
}

void cComboBox::OnMouseOtherClick( const cUIPos& pos )
{
	if( !mpListBox )
	{
		assert(0);
		return;
	}

	/// 자기 리스트에 포함된 이벤트는 넘긴다.
	cUIRect listRc = mpListBox->GetAbsoluteRect();
	cUIRect buttonRc = mpButton->GetAbsoluteRect();

	if( listRc.ContainPoint( pos ) == false && 
		buttonRc.ContainPoint( pos ) == false )
	{
		/// 외부에서 마우스이벤트가 일어났음을 알려주므로, 처리한다.
		if( mpListBox->IsVisible() == true )
		{
			mpListBox->Hide();

			/// 원래대로 변경
			SetRelativeSize( mSize );
		}
	}
}

/// 주의사항 : 
void cComboBox::CheckButtonEvent( cUINode*, unsigned int id )
{
	if( !mpListBox )
	{
		assert(0);
		return;
	}

	if( id == eCOMBOBOX_BUTTON )
	{
		cUIRect listRc = mpListBox->GetAbsoluteRect();

		if( mpListBox->IsVisible() )
		{
			mpListBox->Hide();

			/// 원래대로 변경
			SetRelativeSize( mSize );
		}
		else
		{
			mpListBox->Show();

			/// 자식은 부모 밖에 있을때 이벤트를 받을수 없으므로
			/// 임의로 부모자체의 사이즈를 변경해준다.
			cUISize size = mSize;

			/// 넓이를 늘린다.
			if( size.mWidth < listRc.GetWidth() )
			{
				size.mWidth = listRc.GetWidth();
			}

			/// 높이를 늘린다.
			if( size.mHeight < listRc.GetHeight() )
			{
				size.mHeight = listRc.GetHeight() + mpEdit->GetAbsoluteRect().GetHeight();
			}

			SetRelativeSize( size );
		}
	}
}

/// 주의사항 : 
void cComboBox::AddItem( LPCTSTR text, void* extraData )
{
	if( mpListBox )
	{
		int ret = mpListBox->AddRow( text, (unsigned long)mDefaultColor, extraData );
		if( ret <= -1 )
		{
			assert(0);
		}

		UpdateListboxSize();
	}
	else
	{
		assert(0);
	}

	if( mpButton )
		mpButton->Show();
}

void cComboBox::UpdateListboxSize()
{
	if( mpListBox )
	{
		// 콤보박스의 사이즈 갱신
		unsigned long height = mpListBox->GetRowHeight() * mpListBox->GetRowCount();
		unsigned long maxheight = mpListBox->GetRowHeight() * mpListBox->GetMaxRowInPage();

		if( height > maxheight )
			height = maxheight;

		cUISize size;
		size.mWidth = mpListBox->GetAbsoluteRect().GetWidth();
		size.mHeight = height;

		mpListBox->SetRelativeSize( size );
	}
}

void* cComboBox::GetSelectData()
{
	return ( mpListBox ) ? mpListBox->GetExtraData( mpListBox->GetSelectIndex() ) : 0;
}

void cComboBox::SetSelect( int row, bool sendEvent )
{
	if( mpListBox && mpEdit )
	{
		mpListBox->ChangeSelectRow( row );

		if( sendEvent )
		{
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mType = eUIEVENT_COMBO_SELECTED;

			mpParent->HandleEvent( event );
		}
		
		/// 텍스트를 얻어와서
		unsigned int selectIdx = mpListBox->GetSelectIndex();
		mEditText = mpListBox->GetText( selectIdx );
		mpEdit->SetText( mEditText.Cstr() );
	}
}

unsigned int cComboBox::GetSelectIndex()
{
	if( mpListBox )
	{
		return mpListBox->GetSelectIndex();
	}
	else
	{
		assert(0);
		return 0;
	}
}

void cComboBox::SetEnabled( bool enabled )
{
	cUINode::SetEnabled( enabled );

	if( mpListBox )
		mpListBox->SetEnabled( enabled );

	if( mpButton )
		mpButton->SetEnabled( enabled );

	if( mpEdit )
		mpEdit->SetEnabled( enabled );
}

unsigned int cComboBox::GetSize()
{
	if( mpListBox )
		return mpListBox->GetRowCount();
	else
	{
		assert(0);
		return 0;
	}
}

void* cComboBox::GetExtraData( unsigned int index )
{
	return ( mpListBox ) ? mpListBox->GetExtraData( index ) : 0;
}


/////////////////////////////////////////////////////////////////////////////////
cComboBoxSkin::cComboBoxSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cComboBoxSkin::~cComboBoxSkin()
{

}

/// 주의사항 : 
bool cComboBoxSkin::Load( cParser& parser )
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
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_EDITSKIN:
			{
				mEditSkin = parser.ParseString();
			}
			break;
		case eTOKEN_LISTSKIN:
			{
				mListboxSkin = parser.ParseString();
			}
			break;
		case eTOKEN_BUTTONSKIN:
			{
				mButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_SCROLLSKIN:
			{
				mScrollSkin = parser.ParseString();
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

