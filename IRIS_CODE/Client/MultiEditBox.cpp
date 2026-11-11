#include "StdAfx.h"
#include "MultiEditBox.h"
#include "EditBox.h"

#include "UIImage.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "UICursor.h"

cMultiEditBox::cMultiEditBox( eUINodeType type )
: cUIIMENode( type )
, mEditTabIndex(0)
, mCaretTimeFlag( false )
, mShowCaretFlag( false )
, mLastTick(0)
, mEditingPos(0, 0)
, mEditWidth(0)
, mEditX(0)
, mEditY(0)
{
	mTextAlign = eALIGN_LEFT; 

	::ZeroMemory( mShowText, sizeof(mShowText) );

	mProcessEnable = true;
}

cMultiEditBox::~cMultiEditBox()
{
}

bool cMultiEditBox::OnCreate( cUINodeProperty* )
{
	if( cUINode::OnCreate() == false )
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
bool cMultiEditBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_MULTIEDITBOX ) == false )
	{
		assert( 0 && "not eidtbox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cEditBoxSkin* p = (cEditBoxSkin*)pskin;
	
	/// 문자열 시작 위치 
	mEditingPos = p->mEditPos;

	/// 멀티 에디트 문자열 최대 width
	mEditWidth = p->mEditWidth;
	if( mEditWidth <= 0 )
	{
		/// 사이즈가 없으면 기본사이즈로 세팅 
		mEditWidth = pskin->mSkinInfo->mWidth;
	}

	/// 에디트 타입 
	mEditKind = (eEditKind)p->mEditKind;
	
	/// 에디트에 수용할수 있는 최대길이 지정
	SetMaxEditLength( p->mMaxEditLength );
			
	/// 한줄에 들어갈수 있는 폭 적용
	SetOneLineWidth( mEditWidth );

	/// 스크립트에서 미리 설정한 텍스트가 있는경우 사용함.
	if( mTextLength <= 0 )
	{
		Sstrncpy( mText, MAX_TEXT_LENGTH, p->mText.Cstr(), p->mText.GetLength() );

		/// 길이 지정
		mTextLength = p->mText.GetLength();

		/// 캐럿을 문자열 끝으로 옮김 
		CaretMoveEnd();
	}
	return true;
}

// 주의사항 : 
void cMultiEditBox::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	mEditX = mEditingPos.mX + GetAbsoluteRect().mLeft;
	mEditY = mEditingPos.mY + GetAbsoluteRect().mTop;
	
	/// 캐럿 타임 계산
	if( mEditKind != eEDIT_NOINPUT )
	{
		UpdateCaretTime( accumTime );
	}
	
	/// 텍스트 데이터 세팅 
	if( mChangeText )
	{
		SetTextData();

		mChangeText = false;
	}
}

/// 주의사항 : 
void cMultiEditBox::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();
	
	/// 에디트가 가능해야지만 캐럿 출력 
	if( mEditKind != eEDIT_NOINPUT )
	{
		/// 캐럿 계산 및 출력 
		RenderCaret( pKeeper );
	}

	/// 텍스트 계산 및 출력 
	RenderText( pKeeper );
}

/// 주의사항 : 
void cMultiEditBox::OnKeyDown( eKeyCode code )
{
	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;

	/// 탭키를 누르면 포커스를 옮긴다
	if( code == eKEY_TAB )
	{
		mpParent->ChangeFocus( mEditTabIndex );
	}
	else if( code == eKEY_ESCAPE )
	{
		ReleaseFocus();
	}

	cUIIMENode::OnKeyDown( code );
}

/// 주의사항 :
void cMultiEditBox::OnMouseMove( const cUIPos& )
{
}

/// 주의사항 : 
void cMultiEditBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind == eEDIT_NOINPUT )
		return;

	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	/// 포커스를 가진다 
	if( GetFocus() != this )
	{
		SetFocus();
//		SetCapture();
	}

	/// 마우스로 찍은 좌표에 해당하는 문자인덱스를 알아낸다.
	FindTextCaret( pos );
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cMultiEditBox::OnMouseLeft( const cUIPos& )
{
	CURSOR->SetCursor( eCURSOR_DEFAULT );
}

/*
	mTextLineArray = 각 글자 인덱스마다 몇번째 라인에 속하는지 저장되어있음. 그러니 글자갯수만큼 존재 
	mTextIndexArray = 각 라인당 몇글자씩 포함하구 있는지 저장되어있음. 즉 라인 갯수만큼 존재 ( 누적됨 )
*/

/// 텍스트 데이터 세팅 ( 텍스트와 관련된 텍스트 정보들.. )
void cMultiEditBox::SetTextData()
{
	///
	mTextLineArray.Clear();
	mTextIndexArray.Clear();

	/// 스크롤 관련 화면출력 문자열 갱신
	/// 조합이 아니면 
	if( mCompFlag == false )
	{
		Sstrncpy( mShowText, MAX_TEXT_LENGTH, mText, mMaxEditLength );
	}
	else
	{
		/// 조합중이면 
		Sstrncpy( mShowText, MAX_TEXT_LENGTH, mText, mTextCaretX );
		//Sstrncat( mShowText, MAX_TEXT_LENGTH, mCompText, 1 );
		Sstrncat( mShowText, MAX_TEXT_LENGTH, (LPCTSTR)mCompText.Cstr(), mCompText.GetLength() );
		Sstrncat( mShowText, MAX_TEXT_LENGTH, mText + mTextCaretX, mMaxEditLength - mTextCaretX - 1);
	}

	/// 검사된 문자열 폭
	int cutWidth = 0;

	/// 잘린문자열의 길이 
	int cutlen = 0;

	/// 기본 문자열 길이 
	int showlen = ::_tcslen( mShowText ); 

	/// 라인 번호 
	int line = 0;

	/// 문자열
	LPTSTR str = (LPTSTR)mShowText;

	///
	unsigned int textlen = (int)::_tcslen(str);

	/// 텍스트 width 구하기 
	int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, str, textlen );

	/// 첫 조합이 시작되면 길이 더해줌
	if( mFixFlag == true )
	{
		//textWidth += FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mCompText, 1 );
		textWidth += FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mCompText.Cstr(), mCompText.GetLength() );
	}

	/// 텍스트가 훨씬 너비가 길면, 안넘는것으로 잘라내야한다.
	while( textWidth > mOneLineWidth )
	{
		/// 현재까지 검사한 문자폭과 제한 폭 비교
		if( cutWidth < mOneLineWidth )
		{
			/// UNICODE 니까 무조건 길이를 1씩 더한다
			cutlen += 1;

			/// 현재까지 검사된 문자들의 폭을 구한다 
			cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, str, cutlen );
		}
		else
		{
			/// 문자 폭 보정 ( 폭보다 튀어가나가는 글자는 다음번에 찍도록 미룬다 )
			if( cutWidth > mOneLineWidth )
			{
				/// UNICODE 니까 무조건 길이를 1씩 
				int minusWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, str + cutlen - 1, 1 );
				cutWidth -= minusWidth;
				cutlen -= 1;
			}

			/// 결과	
			/// 각 텍스트 해당 인덱스에 라인정보를 담는다 ( 라인번호 - 글자 : 매칭 시키기 )
			for( int i = cutlen*line; i < cutlen*(line+1); ++i )
			{
				mTextLineArray.PushBack( line );
			}

			/// 각 라인당 어떤 글자인덱스를 포함하는지 저장
			if( mTextIndexArray.GetSize() > 0 )
			{
				int index = mTextIndexArray[line-1];
				mTextIndexArray.PushBack( cutlen + index );
			}
			else
			{
				mTextIndexArray.PushBack( cutlen );
			}
		
			/// 
			textWidth -= cutWidth;
			str += cutlen;

			/// 잘라낸길이만큼 총길이를 줄여준다 
			showlen -= cutlen;
			cutWidth = 0;
			cutlen = 0;
			line++;
		}
	}

	/// 텍스트 폭이 한줄에 들어가는 폭이면, 그냥 삽입 
	if( showlen > 0 && textWidth <= mOneLineWidth )
	{
		/// 각 텍스트 해당 인덱스에 라인정보를 담는다 ( 라인번호 - 글자 : 매칭 시키기 )
		/// textlen + (cutlen*line) -> 누적 글자 수 담기 
		for( unsigned int i = cutlen*line; i < textlen + (cutlen*line); ++i )
		{
			mTextLineArray.PushBack( line );
		}

		/// 각 라인당 어떤 글자인덱스를 포함하는지 저장
		mTextIndexArray.PushBack( textlen );
	}
}

/// 주의사항 :
void cMultiEditBox::RenderText( cUIFontItemKeeper* pKeeper )
{	
	TCHAR pText[MAX_TEXT_LENGTH] = {0,};
	unsigned int height = 0;
	unsigned int beforelen = 0;
	unsigned int curlen = 0;

	/// 라인별로 잘라서 뿌리기 ( 최대 라인갯수까지만 실행 )
	for( unsigned int i = 0; i < mTextIndexArray.GetSize(); ++i )
	{
		::ZeroMemory( pText, sizeof(pText) );

		//// i 번째 라인에 있는 글자 길이를 가져온다
		if( i > 0 )
		{
			beforelen = mTextIndexArray[i-1];
		}

		curlen = mTextIndexArray[i] - beforelen;

		/// 각라인이 포함하는길이 
		Sstrncpy( pText, MAX_TEXT_LENGTH, mShowText + beforelen, curlen );
        
		/// 출력할 위치값 설정
		height = ( i * FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) );
		int editY = mEditingPos.mY + GetAbsoluteRect().mTop + height;

		/// 중앙 정렬이면,
		if( mTextAlign == eALIGN_CENTER )
		{
			mEditX = GetAbsoluteRect().mLeft + ( GetAbsoluteRect().GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, pText, curlen ) ) / 2;
			mEditY = mEditingPos.mY + height + GetAbsoluteRect().mTop + ( GetAbsoluteRect().GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat) ) / 2;
		}

		pKeeper->AddFontItem( cFontAgent::eFont_Chat, pText, mEditX, editY, mDefaultColor, mOutLine );	
	}
}

/// 주의사항 : 캐럿위치를 계산해서 렌더링
void cMultiEditBox::RenderCaret( cUIFontItemKeeper* pKeeper )
{
	/// 캐럿 쇼 타임이 아니거나 멀티 에디트 박스에 포커스가 없으면 리턴 
	if( mShowCaretFlag == false || GetFocus() != this )
	{
		return;
	}

	LPTSTR caret = _T("|");
	int startX = mEditX;
	int startY = mEditY;
    unsigned int lineNum = 0;
	unsigned int startIndex = 0;		/// 각 라인의 시작 문자 인덱스 
	unsigned int height = 0;			/// 라인에 따른 y 위치 변화량 

	if( mTextCaretX > 0 )
	{
		/// 현재 캐럿이 있는 문자가 몇번째 라인에 있는지 받아온다 
		if( mCompFlag )
		{
			/// 조합중이면 예외처리 (한글인경우)
			lineNum = mTextLineArray[ mTextCaretX ];
		}
		else
		{
			lineNum = mTextLineArray[ mTextCaretX-1 ];
		}
	}

	if( lineNum > 0 )
	{
		/// 각 라인이 시작 문자의 인덱스가 뭔지 받아온다
		startIndex = mTextIndexArray[ lineNum-1 ];
	}

	height = lineNum * FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat);
	
	/// 캐럿 출력 
	int textwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mShowText + startIndex, mTextCaretX - startIndex );
	int nextwidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mShowText + startIndex, mTextCaretX - startIndex + 1);
	int newcaretpos = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, caret, 1 ) / 2;

	/// 실제 캐럭 찍는 위치 구하기
	int caretX = startX + textwidth - newcaretpos;

	/// 한글 조합중이면 사각형 캐럿으로 출력 
	if( mCompFlag )
	{
		/// 한글조합중이면 사각형 캐럿을 그린다 
		for( int i = 0; i < ( nextwidth - textwidth ); ++i )
		{
			caretX = startX + textwidth - newcaretpos;
			pKeeper->AddFontItem( cFontAgent::eFont_UI, caret, caretX, startY + height, COLOR_PINK, mOutLine );
			startX++;
		}
	}
	else
	{
		/// 한글이 아닌 모든경우는 그냥 출력 
		pKeeper->AddFontItem( cFontAgent::eFont_UI, caret, caretX, startY + height, (unsigned long)mDefaultColor, mOutLine );
	}
}

/// 주의사항 : 캐럿이 보이는 시간 간격 계산 
void cMultiEditBox::UpdateCaretTime( unsigned long accumTime )
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

/// 주의사항 : 마우스로 찍었을 시에 그 좌표에 있는 텍스트로 캐럿을 옮긴다.
void cMultiEditBox::FindTextCaret( const cUIPos& pos )
{
	unsigned int len = (int)::_tcslen(mShowText);
	if( len <= 0 ) return;

	/// edit 상의 마우스 클릭 좌표 
	unsigned int mouseX = pos.mX - mEditX;
	unsigned int mouseY = pos.mY - mEditY;
	
	if( (int)mouseX < mEditingPos.mX )
		mouseX = mEditingPos.mX;

	if( (int)mouseY < mEditingPos.mY )
		mouseY = mEditingPos.mY;

	/// 라인 구하기
	unsigned int lineNum = mouseY / FONTAGENT->GetTextHeight(cFontAgent::eFont_Chat);

	/// y 축 선택 라인 보정
	if( lineNum >= mTextIndexArray.GetSize() )
		return;

	/// x 축 선택 문자 인덱스 구하기
	int startIndex = 0;
	if( lineNum > 0 )
	{
		/// 이전라인의 len 구하기 
		startIndex = mTextIndexArray[ lineNum - 1];
	}

	/// 검사된 문자열 폭
	unsigned int cutWidth = 0;

	/// 잘린문자열의 길이 
	unsigned int cutlen = 0;

	/// 몇글자가 들어가는지 검사
	while( cutWidth < mouseX )
	{
		/// 계속 검사할필요 없음.
		if( (int)cutlen >= mTextLength )
			break;

		cutlen++;

		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mShowText + startIndex, cutlen );
	}

	/// 더 많이 나왔으면 보정
	if( cutWidth > mouseX )
	{
		cutlen--;
		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mShowText + startIndex, cutlen );
	}

	/// 다음글자의 폭 구하기
	unsigned int nextWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mShowText + startIndex + cutlen, cutlen + 1);
		
	/// 앞뒤글자의 중간을 기준으로 찎는 방향에 따라 캐럿 위치가 달라지도록 한다
	unsigned int width = (nextWidth - cutWidth) / 2;

	if( mouseX - cutWidth > width )
	{
		cutlen++;
	}

	/// 문자열 끝 보정 ( 해당라인의 문자수보다 더 많이 선택되엇을 경우 )
	unsigned int limitlen = mTextIndexArray[ lineNum ];
	if( cutlen >= limitlen - startIndex )
	{
		cutlen = limitlen - startIndex;
	}

	/// 세팅
	if( lineNum > 0 )
	{
		mTextCaretX = cutlen + startIndex;
	}
	else
	{
		mTextCaretX = cutlen;
	}
}

///
void cMultiEditBox::SetText( LPCTSTR text, eText_Align align )
{
	::ZeroMemory( mText, sizeof( mText ) );

	Sstrncpy( mText, MAX_TEXT_LENGTH, text, _tcslen(text) );
	mTextLength = ::_tcslen(text);
	mTextAlign = align;
	mChangeText = true;
}
