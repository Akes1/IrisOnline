#include "StdAfx.h"
#include "RadioButton.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "SoundSystem.h"

cRadioButton::cRadioButton( eUINodeType type )
: cButton( type )
, mGroupNum(0)
{
	mpPrev = NULL;
	mpNext = NULL;

	mDownSoundIdx = (unsigned long)-1;

	mProcessEnable = false;
}

cRadioButton::~cRadioButton()
{
}

void cRadioButton::Hide()
{
	cUINode::Hide();
}

bool cRadioButton::OnCreate( cUINodeProperty* pproperty )
{
	if( cUINode::OnCreate( pproperty ) == false )
		return false;

	mpParent->AddRadio( mGroupNum, this );

	return true;
}

/// 주의사항 : 
bool cRadioButton::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_RADIOBUTTON ) == false )
	{
		assert( 0 && "not radio button skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cRadioButtonSkin* p = (cRadioButtonSkin*)pskin;

	if( p->mText.IsEmpty() == false )
		mText = p->mText;

    mOriginalTexWidth = mpImage->GetWidth();
	mOriginalTexHeight = mpImage->GetHeight();
	mGroupNum = p->mGroupNum;

	/// 버튼의 이미지 변환좌표 저장 
	mUpPos = p->mUpPos;
	mDownPos = p->mDownPos;
	
	if( p->mUpOverPos.mX == 0 && p->mUpOverPos.mY == 0 )
		mUpOverPos = p->mUpPos;
	else
		mUpOverPos = p->mUpOverPos;
	
	/// 버튼 상태 텍스트 컬러
	mUpColor = p->mUpColor;
	mDownColor = p->mDownColor;

	/// 
	mDownSoundIdx = p->mDownSoundIdx;

	/// 텍스트 좌표 업데이트 
	UpdateText();
	
	/// 초기상태가 눌렸있다면 적용
	SetPress( mPressed );
	return true;
}

/// 주의사항 : 
void cRadioButton::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	if( mPressed && mText.IsEmpty() == false )
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY+1, mDownColor, mOutLine );
	else
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY, mUpColor, mOutLine );
}

/// 주의사항 : 
void cRadioButton::OnLButtonDown( const cUIPos&, bool, bool, bool )
{
	if( !mEnabled || mPressed )
		return;

	/// 부모윈도우 앞으로 내보내기 
	UIMAN->GotoFrontNode( this );
	SetPress( true );

	SOUNDSYS->Play2DSound( mDownSoundIdx );
}

/// 주의사항 : 어디선가 SetPress 이벤트가 오면,
void cRadioButton::SetPress( bool press, bool onsound, bool exceptEnable )
{
	if( !exceptEnable && !mEnabled ) 
		return;

	mPressed = press;

	/// 눌림을 적용 
	AdjustPress();

	if( mPressed == false )	
		return;

	if( onsound )
		SOUNDSYS->Play2DSound( mDownSoundIdx );

	/// 연결 그룹에 전달 
	UpdateGroup();

	/// 이벤트 발생
	cUIEvent event;
	event.mType = eUIEVENT_RADIO_PRESSED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );
}

/// 라디오 버튼 갱신을 위함
void cRadioButton::SetPressNoEvent( bool press, bool onsound )
{
	if( mEnabled == false )
		return;

	mPressed = press;

	/// 눌림을 적용 
	AdjustPress();

	if( mPressed == false )	
		return;

	if( onsound )
		SOUNDSYS->Play2DSound( mDownSoundIdx );

	/// 연결 그룹에 전달 
	UpdateGroup();
}

/// 주의사항 : 
void cRadioButton::SetEnabled( bool enabled )
{
	cUINode::SetEnabled( enabled );
}

/// 주의사항 :
void cRadioButton::AdjustPress()
{
	/// 이미지 갱신		
	if( mPressed == true )
	{
		/// 라디오 버튼이 눌렸으면 이미지를 갱신한다
		mpImage->SetTextureRect( (unsigned short)(mDownPos.mX), (unsigned short)(mDownPos.mY), 
								 (unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );
	}
	else if( mPressed == false )
	{
		/// 라디오 버튼이 안 눌린 이미지를 갱신한다 
		mpImage->SetTextureRect( (unsigned short)(mUpPos.mX), (unsigned short)(mUpPos.mY), 
								 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
	}
}

/// 주의사항 :
void cRadioButton::UpdateGroup()
{
	/// 연결 그룹에 전달 
	cRadioButton* tempNode = this;
	bool findNode = false;

	/// prev 라디오버튼 검사 
	while( tempNode->mpPrev )
	{
		tempNode = tempNode->mpPrev;
		if( tempNode->IsPress() == true )
		{ 
			tempNode->SetPress( false );
			findNode = true;
			return;
		}
	}

	/// true인것이 있었으면 업데이트 했으므로 
	/// next는 검사할 필요 없다
	if( findNode )
		return;

	tempNode = this;
	/// next 라디오버튼 검사 
	while( tempNode->mpNext )
	{
		tempNode = tempNode->mpNext;
		if( tempNode->IsPress() == true )
		{
			tempNode->SetPress( false );
			return;
		}
	}
}

/// 주의사항 : 그룹끼리 이중연결리스트로 묶음
void cRadioButton::Link( cUINode* node )
{
	if( node == 0 )
		return;

	cRadioButton* addNode = (cRadioButton*)node;
	cRadioButton* tempNode = this;

	/// next가 null 인 라디오를 찾아서 연결 
	while( tempNode->mpNext )
	{
		tempNode = tempNode->mpNext;
	}

	addNode->mpNext = tempNode->mpNext;
	tempNode->mpNext = addNode;
	addNode->mpPrev = tempNode;
}

/// 주의사항 :
void cRadioButton::UpdateText()
{
	if( mText )
	{
		/// 텍스트가 뿌려질 위치 계산 
		cUIRect rc = GetAbsoluteRect();
		/// 뿌려질 좌표 계산
		mTextPosX = rc.mLeft + (int)((rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mText.Cstr(), mText.GetLength()) ) * 0.5f);
		mTextPosY = rc.mTop + (int)((rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI) ) * 0.5f);
	}	
}

///
LPCTSTR cRadioButton::GetText()
{
	return ( mText.IsEmpty() ) ? 0 : (LPCTSTR)mText.Cstr();
}


/// 주의사항 : 호버 노드 이벤트 받음 
void cRadioButton::OnMouseLeft( const cUIPos& pos )
{
	cUINode::OnMouseLeft( pos );

	// 소스 이미지 바꾸기
	if( !mPressed && mEnabled == true )
	{
		mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
			(unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
	}
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cRadioButton::OnMouseHovered( const cUIPos& pos )
{
	cUINode::OnMouseHovered( pos );

	// 소스 이미지 바꾸기
	if( !mPressed && mEnabled == true )
	{
		mpImage->SetTextureRect( (unsigned short)mUpOverPos.mX, (unsigned short)mUpOverPos.mY, 
			(unsigned short)(mUpOverPos.mX + mOriginalTexWidth), (unsigned short)(mUpOverPos.mY + mOriginalTexHeight) );
	}
}


//////////////////////////////////////////////////////////////////////////////
cRadioButtonSkin::cRadioButtonSkin( eUINodeType type )
: cUINodeSkin( type )
, mGroupNum(0)
, mUpPos(0, 0)
, mDownPos(0, 0)
, mUpOverPos(0, 0)
{
	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;

	mDownSoundIdx = (unsigned long)-1;
}

cRadioButtonSkin::~cRadioButtonSkin()
{
}

/// 주의사항 : 라디오 버튼에서 필요한 내용 따로 파싱
bool cRadioButtonSkin::Load( cParser& parser )
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
		case eTOKEN_DOWNSOUND:
			{
				mDownSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_UPCOLOR:
			{
				/// 다음 토큰은 "(" 이므로 넘어간다 
				if( parser.ExpectTokenString( "(" ) == false )
					return false;

				unsigned char r = (unsigned char)parser.ParseInt();
				unsigned char g = (unsigned char)parser.ParseInt();
				unsigned char b = (unsigned char)parser.ParseInt();

				if( parser.ExpectTokenString( ")" ) == false )
					return false;

				/// 변환 ( R,G,B 순서로 )
				unsigned long color = (unsigned long)( (DWORD)((BYTE)r << 16) | (WORD)((BYTE)g << 8) | (BYTE)b );

				/// down 컬러는 기본으로 up 컬러랑 동일하게 한다
				mUpColor = mDownColor = color | 0xff000000;
			}
			break;
		case eTOKEN_DOWNCOLOR:
			{
				/// 다음 토큰은 "(" 이므로 넘어간다 
				if( parser.ExpectTokenString( "(" ) == false )
					return false;

				unsigned char r = (unsigned char)parser.ParseInt();
				unsigned char g = (unsigned char)parser.ParseInt();
				unsigned char b = (unsigned char)parser.ParseInt();

				if( parser.ExpectTokenString( ")" ) == false )
					return false;

				/// 변환 ( R,G,B 순서로 )
				unsigned long color = (unsigned long)( (DWORD)((BYTE)r << 16) | (WORD)((BYTE)g << 8) | (BYTE)b );

				mDownColor = color | 0xff000000;
			}
			break;
		case eTOKEN_GROUPNUM:
			{
				mGroupNum = parser.ParseInt();
			}
			break;
		case eTOKEN_TEXT:
			{
				int i = parser.ParseInt();
				if( UIMAN->GetUIText( &mText, i ) == false )
				{
					return false;
				}
			}
			break;
		case eTOKEN_UPPOS:
			{
				mUpPos.mX = parser.ParseInt();
				mUpPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNPOS:
			{
				mDownPos.mX = parser.ParseInt();
				mDownPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_UPOVERPOS:
			{
				mUpOverPos.mX = parser.ParseInt();
				mUpOverPos.mY = parser.ParseInt();
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
