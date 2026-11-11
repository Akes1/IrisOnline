#include "StdAfx.h"
#include "EditBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
#include "PlaneObject.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "Application.h"

cEditBox::cEditBox( eUINodeType type )
: cUIIMENode( type )
, mSelectionImage( 0 )
, mEditX( 0 )
, mEditY( 0 )
, mLastTick( 0 )
, mEditTabIndex(0)
, mShowLen(0)
, mShowCaretFlag( false )
, mCaretTimeFlag( false )
, mCopyFlag( true )
, mEditingPos(0, 0)
, mSelectionStartPos( 0, 0 )
, mTextAlign( eALIGN_LEFT )
{
	mOriginalTexRect.Set( 0, 0, 0, 0 );
	
	mTextColor = mDefaultColor;

	mProcessEnable = true;
}

cEditBox::~cEditBox()
{
	SAFE_DELETE( mSelectionImage );
}

bool cEditBox::OnCreate( cUINodeProperty* )
{
	if( cUIIMENode::OnCreate() == 0 )
		return false;

	/// 노 입력 에디트가 아니면 등록 
	if( mEditKind != eEDIT_NOINPUT )
	{
		/// 부모에 에디트 탭을 등록한다
		mEditTabIndex = mpParent->AddEditBox( this );
	}

	return true;
}

/// 주의사항 : 
bool cEditBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_EDITBOX ) == false )
	{
		assert( 0 && "not eidtbox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cEditBoxSkin* p = (cEditBoxSkin*)pskin;

	////// 비활성화 관련인데!!!!!!!!! 수정하기 070424 
	/// 이미지 사용안하는 경우 제외 
	if( mpImage )
	{
		mOriginalTexRect = mpImage->GetTextureRect();
	}
	
	/// 에디트 글자 출력 시작 위치 받아오기
	mEditingPos = p->mEditPos;

	/// 에디트 타입 
	mEditKind = (eEditKind)p->mEditKind;

	/// 에디트 내의 정렬 
	mTextAlign = p->mTextAlign;

	/// 에디트에 수용할수 있는 최대길이 지정
	SetMaxEditLength( p->mMaxEditLength );

	/// 한줄에 들어갈수 있는 폭 적용 ( 뒤 간격 포함 )
	SetOneLineWidth( GetAbsoluteRect().GetWidth() - mEditingPos.mX * 2 );

	/// 스크립트에서 미리 설정한 텍스트가 있는경우 사용함.
	if( p->mText.Cstr() )
	{
		Sstrncpy( mText, MAX_TEXT_LENGTH, p->mText.Cstr(), p->mText.GetLength() );

		/// 길이 지정
		mTextLength = p->mText.GetLength();
		/// 캐럿을 문자열 끝으로 옮김 
		CaretMoveEnd();
	}
	
	/// 드래그 선택 시 백 이미지 설정
	if( mEditKind != eEDIT_NOINPUT && p->mpSelectionTexture )
	{
		if( p->mpSelectionTexture )
		{
			unsigned short tx = (unsigned short)p->mSelectionSkin->mTexX;
			unsigned short ty = (unsigned short)p->mSelectionSkin->mTexY;
			unsigned short tw = (unsigned short)p->mSelectionSkin->mTexWidth;
			unsigned short th = (unsigned short)p->mSelectionSkin->mTexHeight;

			mSelectionImage = new cPlaneObject;

			if( mSelectionImage->Create( p->mpSelectionTexture, (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop, tw, th,
				tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create select image");
				return false;
			}
		}
	}

	mChangeText = true;

	/// 텍스트 좌표 세팅 
	UpdateText();
	return true;
}

/// 주의사항 : 
bool cEditBox::HandleEvent( const cUIEvent& event )
{
	if( event.mCode == eKEY_DOWN || event.mCode == eKEY_UP )
	{
		return mpParent->HandleEvent( event );
	}

	return cUINode::HandleEvent( event );
}

/// 주의사항 : 
void cEditBox::UpdateRect()
{
	cUINode::UpdateRect();

	/// 텍스트 좌표 계산 
	UpdateText();
}

/// 주의사항 : 
void cEditBox::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	/// 스크롤 텍스트 및 에디트에 실제로 찍히는 텍스트 갱신
	if( mChangeText )
	{
		mChangeText = false;

		UpdateText();
		UpdateShowText();
		
		if( !mDraggingFlag && mEditKind != eEDIT_NOINPUT )
		{
			if( mShowCaretFlag && GetFocus() == this )
			{
				//
				HIMC hIMC = ::ImmGetContext( THEAPP->GetHWND() );

				/// 드래깅중이 아니고, 에디트가 가능하면 보여준다
				LPTSTR caret = _T("|");

				int textwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mShowText.Cstr(), mScrollCaretX );
				int newcaretpos = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, caret, 1 ) / 2;

				/// 실제 캐럭 찍는 위치 구하기
				int caretX = mEditX + textwidth - newcaretpos+1;

				// 조합윈도우 
				COMPOSITIONFORM comf;
				comf.dwStyle = CFS_RECT;
				comf.ptCurrentPos.x = caretX;
				comf.ptCurrentPos.y = mEditY;
				comf.rcArea.left = caretX;
				comf.rcArea.top = mEditY;
				comf.rcArea.right = GetAbsoluteRect().mLeft + 512;
				comf.rcArea.bottom = GetAbsoluteRect().mTop + FONTAGENT->GetTextHeight(cFontAgent::eFont_System);
				ImmSetCompositionWindow( hIMC, &comf );

				::ImmReleaseContext( THEAPP->GetHWND(), hIMC );
			}
		}

 		cUIEvent event;
 		event.mID = mID;
 		event.mType	= eUIEVENT_EDITBOX_CHAR_UPDATED;
 		event.mpCaller = this;
		mpParent->HandleEvent( event );
	}

	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;

	/// 캐럿 타임 계산
	UpdateCaretTime( accumTime );

	/// 패스워드면 '*'로 대체해서 출력 
	if( mEditKind == eEDIT_ACCOUNT_PASS )
	{
		ChangePassword();
	}
}

/// 주의사항 : 
void cEditBox::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 드래깅 영역 이미지 바 렌더링 
	if( mEditKind != eEDIT_NOINPUT && mDraggingFlag )
	{
		if( mSelectionImage )
			mSelectionImage->Draw();
	}

    /// 캐럿 그리기
	RenderCaret( pKeeper );
	
	/// 문자열 렌더링 
	pKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)mShowText.Cstr(), mEditX, mEditY, mTextColor, mOutLine );
}

/// 주의사항 : 
void cEditBox::OnKeyDown( eKeyCode code )
{
	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;

	switch( code )
	{
	case eKEY_CONTROL:
		{
			if( mCopyFlag == false )
				return;

			/// 패스워드이면 복사기능을 없앤다 
			if( mEditKind == eEDIT_ACCOUNT_PASS )
				return;

			mControlKey = true;
		}
		break;
	case eKEY_SHIFT:
		{
			mShiftKey = true;
		}
		break;
	case eKEY_TAB:
		{
			mpParent->ChangeFocus( mEditTabIndex );
		}
		break;
	case eKEY_ESCAPE:
		{
			ReleaseFocus();
			Clear();

			cUIEvent event;
			event.mType = eUIEVENT_EDITBOX_ENTER;
			event.mpCaller = this;
			event.mID = mID;
			event.mCode = eKEY_ESCAPE;
			mpParent->HandleEvent( event );
		}
		break;
	case eKEY_RETURN:
		{
			/// 이벤트 보내기 
			cUIEvent event;
			event.mType = eUIEVENT_EDITBOX_ENTER;
			event.mpCaller = this;
			event.mID = mID;
			event.mCode = eKEY_RETURN;
			mpParent->HandleEvent( event );
		}
		break;
	}
	
	cUIIMENode::OnKeyDown( code );
}

/// 주의사항 : 
void cEditBox::OnKeyUp( eKeyCode code )
{
	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;
    
	switch( code )
	{
	case eKEY_CONTROL:
		{
			/// 컨트롤 키 해제 
			if( mControlKey == true )
				mControlKey = false;
		}
		break;
	case eKEY_SHIFT:
		{
			/// 쉬프트 키 해제
			if( mShiftKey == true )
				mShiftKey = false;
		}
		break;
	}
}

void cEditBox::OnImeStartComposition( unsigned int wparam, unsigned int lparam )
{
	HIMC hIMC = ::ImmGetContext( THEAPP->GetHWND() );

	int startX = mEditX;

	/// 드래깅중이 아니고, 에디트가 가능하면 보여준다
	LPTSTR caret = _T("|");

	int textwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mShowText.Cstr(), mScrollCaretX );
	int newcaretpos = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, caret, 1 ) / 2;

	/// 실제 캐럭 찍는 위치 구하기
	int caretX = startX + textwidth - newcaretpos+1;

	// 조합윈도우 
	COMPOSITIONFORM comf;
	comf.dwStyle = CFS_RECT;
	comf.ptCurrentPos.x = caretX;
	comf.ptCurrentPos.y = mEditY;
	comf.rcArea.left = caretX;
	comf.rcArea.top = mEditY;
	comf.rcArea.right = GetAbsoluteRect().mLeft + 512;
	comf.rcArea.bottom = GetAbsoluteRect().mTop + FONTAGENT->GetTextHeight(cFontAgent::eFont_System);
	ImmSetCompositionWindow( hIMC, &comf );

	::ImmReleaseContext( THEAPP->GetHWND(), hIMC );
}

/// 주의사항 : 
void cEditBox::OnMouseMove( const cUIPos& pos )
{
	/// 에디트상에서 드래깅이 되면 백 이미지를 보여준다 
	if( mEditKind != eEDIT_NOINPUT && mEditKind != eEDIT_NUMBER && mPressed && mTextLength > 0 )
	{
		/// 백 이미지 설정
		UpdateBackImage( pos );
	}
}

/// 주의사항 : 
void cEditBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;

	/// 포커스를 가진다 
	if( GetFocus() != this )
	{
		SetFocus();
		SetCapture();
	}

	if( mCompFlag == true )
		return;

	/// 마우스로 처음 찍은 좌표를 저장 
	mSelectionStartPos = pos;

	/// 에디트에서 마우스 이벤트가 일어날시, 캐럿의 위치 변경 계산
	UpdateMouseMoveInEdit( pos, mScrollCaretX, mTextCaretX, true );

	/// 드래깅이 될 경우를위해 저장한다 
	mStartDragCaretX = mScrollCaretX;
	mEndDragCaretX = mScrollCaretX;

	/// 에디트가 눌림 
	mPressed = true;
	mDraggingFlag = false;
}

/// 주의사항 : 
void cEditBox::OnLButtonUp( const cUIPos& )
{
	ReleaseCapture();
	mPressed = false;
}

void cEditBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUINode::OnLButtonDoubleClick( pos );

	SetAllSelect();
}

/// 주의사항 : 
void cEditBox::OnCaptureLost( const cUIPos& )
{
	mDraggingFlag = false;
}

void cEditBox::OnMouseLeft( const cUIPos& )
{
	mPressed = false;
}

/// 주의사항 : 
void cEditBox::OnFocusLost()
{
	mDraggingFlag = false;

	mCompText.Clear();

	mCompFlag = false;
	mChangeText = true;
}

/// 주의사항 : 캐럿이 보이는 시간 간격 계산 
void cEditBox::UpdateCaretTime( unsigned long accumTime )
{
	/// 캐럿이 보일지 안보일지 결정
	if( mCaretTimeFlag )
	{
		mLastTick = accumTime;
		mCaretTimeFlag = false;
	}

	/// 캐럿이 깜빡이는 시간 조절 
	if( accumTime - mLastTick >= 600 )
	{
		mShowCaretFlag = !mShowCaretFlag;
		mLastTick = accumTime;
	}
}

/// 주의사항 : 캐럿위치를 계산해서 렌더링
void cEditBox::RenderCaret( cUIFontItemKeeper* pKeeper )
{
	int startX = mEditX;

	/// 드래깅중이 아니고, 에디트가 가능하면 보여준다
	if( !mDraggingFlag && mEditKind != eEDIT_NOINPUT )
	{
		if( mShowCaretFlag && GetFocus() == this )
		{
			LPTSTR caret = _T("|");

			int textwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mShowText.Cstr(), mScrollCaretX );
			int newcaretpos = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, caret, 1 ) / 2;

			/// 실제 캐럭 찍는 위치 구하기
			int caretX = startX + textwidth - newcaretpos+1;

			/// 한글 조합이면,
			if( mCompFlag )
			{
				int compLen = (int)mCompText.GetLength();
				
				// 범위를 넘어가면
				if( mScrollCaretX + compLen > mShowLen )
				{
					compLen = compLen - (mScrollCaretX + compLen - mShowLen);
				}
		
				int nextwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mShowText.Cstr(), mScrollCaretX + compLen );

				/// 한글조합중이면 사각형 캐럿을 그린다 
				for( int i = 0; i < ( nextwidth - textwidth ); ++i )
				{
					caretX = startX + textwidth - newcaretpos;
					pKeeper->AddFontItem( cFontAgent::eFont_UI, caret, caretX, mEditY, COLOR_DARKGRAY, mOutLine );
					startX++;
				}
			}
			/// 조합이 아니면,
			else
			{
				pKeeper->AddFontItem( cFontAgent::eFont_UI, caret, caretX, mEditY, (unsigned long)mTextColor, mOutLine );
			}
		}
	}
}

/// 문자열 전체 선택 기능
void cEditBox::SetAllSelect()
{
	if( !mSelectionImage )
		return;

	if( !(mTextAlign == eALIGN_LEFT || mTextAlign == eALIGN_RIGHT) )
		return;

	if( mCompFlag == true )
		return;

	/// 실제 선택은 전체
	mStartDragCaretX = 0;
	mEndDragCaretX = mTextLength;

	/// 선택된 영역의 문자열 길이 알아냄.
	int len = mEndDragCaretX - mStartDragCaretX;
	if( len <= 0 )
		return;

	mDraggingFlag = true;

	/// 이미지 사이즈는 현재 보이는 길이만큼만!!
	int endX = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mShowLen );

	/// 양수방향 이미지 설정
	cUIRect rc;

	if( mTextAlign == eALIGN_LEFT )
	{
		rc.mLeft = GetAbsoluteRect().mLeft + mEditingPos.mX;
		rc.mRight = rc.mLeft + endX;
	}
	else
	{
		rc.mRight = GetAbsoluteRect().mRight;
		rc.mLeft = rc.mRight - endX;
	}

	/// 선택영역 이미지 그리기 
	rc.mTop = mEditY;
	rc.mBottom = rc.mTop + FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) + 1;
	mSelectionImage->SetScreenRect( rc.mLeft, rc.mTop, rc.GetWidth(), rc.GetHeight() );
}

/// 주의사항 : 드래깅시의 백 이미지 세팅 
void cEditBox::UpdateBackImage( const cUIPos& pos )
{
	if( !mSelectionImage )
		return;	

	cUIRect rc;
	int dummyvalue = 0;

	/// 드래그 하지 않았으면 리턴
	if( mSelectionStartPos.mX == pos.mX )
		return;

	/// 양수 방향
	if( mSelectionStartPos.mX < pos.mX )
	{
		/// 선택영역의 마지막 캐럿 위치를 계산  ( 마지막인자 : 양수방향 )
		UpdateMouseMoveInEdit( pos, mEndDragCaretX, dummyvalue, true );

		/// 선택된 영역의 문자열 길이 알아냄.
		int len = mEndDragCaretX - mStartDragCaretX;
		if( len <= 0 )
		{
			return;
		}

		mDraggingFlag = true;

		int startX = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mStartDragCaretX );
		int endX = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mEndDragCaretX );

		/// 양수방향 이미지 설정
		rc.mLeft = startX + GetAbsoluteRect().mLeft + mEditingPos.mX;
		rc.mRight = rc.mLeft + ( endX - startX );

		/// 선택영역 이미지 그리기 
		rc.mTop = mEditY;
		rc.mBottom = rc.mTop + FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) + 1;
		mSelectionImage->SetScreenRect( rc.mLeft, rc.mTop, rc.GetWidth(), rc.GetHeight() );
	}
	/// 음수 방향
	else if( mSelectionStartPos.mX > pos.mX )
	{
		/// 선택영역의 마지막 캐럿 위치를 계산  ( 마지막인자 : 음수방향 )
		UpdateMouseMoveInEdit( pos, mEndDragCaretX, dummyvalue, false );

		/// 선택된 영역의 문자열 길이 알아냄.
		int len = mStartDragCaretX - mEndDragCaretX;
		if( len <= 0 )
		{
			return;
		}

		mDraggingFlag = true;

		int startX = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mStartDragCaretX );
		int endX = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mEndDragCaretX );

		/// 음수방향 드래깅
		rc.mRight = startX + GetAbsoluteRect().mLeft + mEditingPos.mX;
		rc.mLeft = rc.mRight - ( startX - endX );

		/// 선택영역 이미지 그리기 
		rc.mTop = mEditY;
		rc.mBottom = rc.mTop + FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) + 2;
		mSelectionImage->SetScreenRect( rc.mLeft, rc.mTop, rc.GetWidth(), rc.GetHeight() );
	}
}

/// 주의사항 : 에디트에서 마우스 이벤트가 일어날시, 캐럿의 위치 변경 계산
/// 인자 ( 마우스좌표, 보여주기위한 캐럿, 실제 텍스트상의 캐럿 )
void cEditBox::UpdateMouseMoveInEdit( const cUIPos& pos, int& viewcaret, int& editcaret, bool directionflag )
{
	/// 에디트 좌표부터 마우스 좌표까지의 거리 구하기
	int distance = pos.mX - ( GetAbsoluteRect().mLeft + mEditingPos.mX );

	/// 
	if( distance < 0 )
	{
		distance = 0;
	}

	int showlen = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), mShowLen );

	/// 현재 출력된 문자열보다 더 많이 선택할수 없다.
	if( distance > showlen )
	{
		distance = showlen;
	}

	if( distance > mOneLineWidth )
	{
		distance = mOneLineWidth;
	}

	/// 검사된 문자열 폭
	int cutWidth = 0;

	/// 잘린문자열의 길이 
	int cutlen = 0;

	/// distance 안에 몇글자가 들어가는지 검사
	while( (int)cutWidth < distance )
	{
		/// 계속 검사할필요 없음.
		if( cutlen >= mTextLength )
			break;

		cutlen++;

		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), cutlen );
	}

	/// 더 많이 나왔으면 보정
	if( cutWidth > distance )
	{
		cutlen--;
		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), cutlen );
	}

	/// 다음글자의 폭 구하기
	int nextWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mShowText.Cstr(), cutlen + 1);
	
	/// 앞뒤글자의 중간을 기준으로 찎는 방향에 따라 캐럿 위치가 달라지도록 한다
	int width = (nextWidth - cutWidth) / 2;

	/// 양수방향
	if( directionflag )
	{
		if( distance - cutWidth > width )
		{
			cutlen++;
		}
	}
	else
	{
		if( distance - cutWidth <= width )
		{
			cutlen--;
		}
		if( cutlen < 0 )
		{
			cutlen = 0;
		}
	}
	
	/// 재 세팅 
	viewcaret = cutlen;
	editcaret = viewcaret + mScrollIndex;
}

/// 주의사항 :.
void cEditBox::ChangePassword()
{
	unsigned int len = mShowText.GetLength();

	TCHAR temp[512] = {0,};
	::wmemset( temp, _T('*'), len );

	mShowText = temp;
}

/// 주의사항 :
void cEditBox::UpdateText()
{
	/// 에디트 박스의 좌표 받아오기
	cUIRect rc = GetAbsoluteRect();
	
	switch( mTextAlign )
	{
	case eALIGN_NONE:
	case eALIGN_LEFT:
		{
			mEditX = mEditingPos.mX + rc.mLeft;
			mEditY = mEditingPos.mY + rc.mTop;
		}
		break;
	case eALIGN_RIGHT:
		{
			unsigned int x = rc.mRight - FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText, mTextLength );

			mEditX = x - mEditingPos.mX;
			mEditY = mEditingPos.mY + rc.mTop;
		}
		break;
	case eALIGN_CENTER:
		{
			mEditX = rc.mLeft + ( rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText, mTextLength ) ) / 2;
			mEditY = rc.mTop + ( rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) ) / 2;
		}
		break;
	}
}

/// 주의사항 : 텍스트 스크롤과, 뿌려지는 텍스트 갱신 
void cEditBox::UpdateShowText()
{
	cStringT temp;

	/// 스크롤 관련 화면출력 문자열 갱신
	/// 조합이 아니면 
	if( mCompFlag == false )
	{
		temp.Append( mText + mScrollIndex, mMaxEditLength );
	}
	else
	{
		/// 조합중이면 
		temp.Append( mText + mScrollIndex, mTextCaretX - mScrollIndex );
		temp.Append( (LPCTSTR)mCompText.Cstr(), mCompText.GetLength() );
		temp.Append( mText + mTextCaretX, mTextLength - mTextCaretX );
	}

	// 초기화
	mShowLen = 0;
	mShowText.Clear();

	int compLen = mCompText.GetLength();
	if( mTextLength + compLen > 0 )
	{
		/// 텍스트 width 구하기 
		/// 조합중이면 조합 너비 만큼 더해준다 
		int showWidth = 0; 

		while( showWidth < mOneLineWidth )
		{
			if( mShowLen >= mTextLength + compLen - mScrollIndex )
				break;

			mShowLen++;

			showWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)temp.Cstr(), mShowLen );
		}

		/// 중간삽입할시, 문자열이 너무 길면 줄인다 
		if( showWidth > mOneLineWidth && mTextCaretX - mScrollIndex + compLen < mShowLen )
		{
			mShowLen--;
		}

		mShowText.Append( (LPCTSTR)temp.Cstr(), mShowLen );
	}
}

/// 주의사항 : 외부에서 에디트에 텍스트를 설정할때
void cEditBox::SetText( LPCTSTR text, int align )
{
	::ZeroMemory( mText, sizeof(mText) );

	Sstrncpy( mText, MAX_TEXT_LENGTH, text, _tcslen(text) );
	mTextLength = _tcslen(text);

	mTextAlign = align;
	CaretMoveEnd();
	UpdateText();
	UpdateShowText();
}

/////////////////////////////////////////////////////////////////////////////////////
cEditBoxSkin::cEditBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mTextAlign( eALIGN_LEFT )
, mMaxEditLength( 110 )
, mEditKind(0)
, mpSelectionTexture( 0 )
, mEditPos(0, 0)
, mSelectionSkin(0)
, mEditWidth(0)
{
	mSelectionSkin = new sSkinInfo;
}

cEditBoxSkin::~cEditBoxSkin()
{
	SAFE_DELETE( mSelectionSkin );
}

/// 주의사항 : 
bool cEditBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

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
			/// 드래깅 영역 이미지 설정 
		case eTOKEN_CHILD_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpSelectionTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_CHILD_SIZE:
			{
				mSelectionSkin->mWidth = mSelectionSkin->mTexWidth = (unsigned short)parser.ParseInt();
				mSelectionSkin->mHeight = mSelectionSkin->mTexHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_CHILD_TEXPOS:
			{
				mSelectionSkin->mTexX = (unsigned short)parser.ParseInt();
				mSelectionSkin->mTexY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_EDITKIND:
			{
				mEditKind = parser.ParseInt();
			}
			break;
		case eTOKEN_TEXT_ALIGN:
			{
				///
				lexer->GetNextToken( &token );

				/// 정렬 
				mTextAlign = eALIGN_LEFT;

				if( token.mType == eTOKEN_CENTER )
				{
					mTextAlign = eALIGN_CENTER;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mTextAlign = eALIGN_RIGHT;
				}
			}
			break;
		case eTOKEN_TEXT:
			{
				/// 에디트 박스에 미리 텍스트를 지정해 놓을경우를 대비함.
				int i = parser.ParseInt();
				if( UIMAN->GetUIText( &mText, i ) == false )
				{
					return false;
				}
			}
			break;
		case eTOKEN_EDITPOS:
			{
				mEditPos.mX = parser.ParseInt();
				mEditPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_EDITWIDTH:
			{
				mEditWidth = parser.ParseInt();
			}
			break;
		case eTOKEN_EDITMAXLEN:
			{
				mMaxEditLength = parser.ParseInt();
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
