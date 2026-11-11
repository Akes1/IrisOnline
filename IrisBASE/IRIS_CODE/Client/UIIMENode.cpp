#include "Stdafx.h"
#include "UIIMENode.h"
#include "Application.h"
#include "FontAgent.h"
#include "ChatManager.h"
#include "StageManager.h"

HWND cUIIMENode::mHWnd = NULL;
bool cUIIMENode::mShiftKey = false;
bool cUIIMENode::mControlKey = false;

cUIIMENode::cUIIMENode( eUINodeType type )
: cUINode( type )
, mTextLength( 0 )
, mTextCaretX( 0 )
, mScrollIndex( 0 )
, mScrollCaretX( 0 )
, mOneLineWidth( 0 )
, mStartDragCaretX( 0 )
, mEndDragCaretX( 0 )
, mMaxEditLength(0)
, mFixFlag( false )
, mCompFlag( false )
, mDraggingFlag( false )
, mChangeText( false )
, mImeFlag( false )
{
	mEditKind = eEDIT_NORMAL;

	::ZeroMemory( mText, sizeof(mText) );
}

cUIIMENode::~cUIIMENode()
{

}

void cUIIMENode::Clear()
{
	::ZeroMemory( mText, sizeof(mText) );

	mCompText.Clear();

	mTextLength = 0;
	mTextCaretX = 0;
	mScrollIndex = 0;
	mScrollCaretX = 0;
	mDraggingFlag = false;
	mCompFlag = false;

	mChangeText = true;
}

bool cUIIMENode::OnCreate( cUINodeProperty* )
{
	if( cUINode::OnCreate() == false )
		return false;

	/// 핸들 저장 
	if( mHWnd == 0 )
	{
		mHWnd = THEAPP->GetHWND();
	}
	return true;
}

/// 주의사항 : 1byte 문자 넣기
void cUIIMENode::OnChar( unsigned int wparam )
{
	if( mEditKind == eEDIT_NOINPUT )
		return;

	TCHAR str[2] = {0,};

	str[0] = (TCHAR)wparam;
	str[1] = 0;

	/// 문자열이 널이면 경고
	if( *str <= 0 )
	{
		assert( 0 && "str is 0" );
	}

	bool except = false;
	BYTE byte2 = LOBYTE( wparam );

	/// 특문제외인경우 조건
	if( mEditKind == eEDIT_EXCEPT_SPECIAL )
	{
		if( (byte2 >= 48 && byte2 <= 57) ||
			(byte2 >= 65 && byte2 <= 90) || 
			(byte2 >= 97 && byte2 <= 122 ) )
			except = false;
		else
			except = true;
	}
	
	/// 해당 문자열이 유효한지 검사후 전체 텍스트에 삽입
	if( !except && IsValid( *str ) )
	{
		AddChar( str );
	}
}

/// 주의사항 : 입력받은 문자열이 유효한지 검사
bool cUIIMENode::IsValid( TCHAR c )
{
	switch( mEditKind )
	{
	case eEDIT_NORMAL:
		{
			if( c >= 32 && c < 127 )
				return true;
		}
		break;
	case eEDIT_NUMBER:
		{
			/// 숫자여야지만 유효
			if (c >= 0x0030 && c <= 0x0039)
				return true;
		}
		break;
	case eEDIT_EXCEPT_SPECIAL:
		{
			/// 특문 제외
			// 숫자
			if (c >= 0x0030 && c <= 0x0039)
				return true;
			// 영문
			else if ( (c >= 0x0041 && c <= 0x005A) || (c >= 0x0061 && c <= 0x007A) )
				return true;
			// 한글
//			else if (c >= 0XAC00 && c <= 0XD7A3)
//				return true;
			// 일어-HIRAGANA
			else if (c >= 0X3040 && c <= 0X309F)
				return true;
			// 일어-KATAKANA
			else if (c >= 0X30A0 && c <= 0X30FF)
				return true;
			// CJK Unified Ideographs (한중일통합)
			else if (c >= 0X4E00 && c <= 0X9FCF)
				return true;
		}
		break;
	case eEDIT_ACCOUNT_ID:
	case eEDIT_ACCOUNT_PASS:
		{
			// 숫자
			if (c >= 0x0030 && c <= 0x0039)
				return true;
			// 영문
			else if ( (c >= 0x0041 && c <= 0x005A) || (c >= 0x0061 && c <= 0x007A) )
				return true;
		}
		break;
	}

	return false;
}

void cUIIMENode::SetFocus()
{
	cUINode::SetFocus();

	////
	HIMC hIMC = ::ImmGetContext( mHWnd );

	DWORD dwConversion, dwSentence;
	ImmGetConversionStatus( hIMC, &dwConversion, &dwSentence);

	/// 계정용이면 영문으로 변환
	if( mEditKind == eEDIT_ACCOUNT_ID || mEditKind == eEDIT_ACCOUNT_PASS )
	{
		if( dwConversion == IME_CMODE_HANGEUL )
			::ImmSetConversionStatus( hIMC, IME_CMODE_ALPHANUMERIC, 0 );
	}
//	else
//	{
		/// 한글모드이면 IME 버퍼 클리어
//		if( dwConversion == IME_CMODE_HANGEUL )
//			::ImmSetCompositionString( hIMC, SCS_SETSTR, NULL, 0, NULL, 0 );
//	}

	::ImmNotifyIME( hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );


	::ImmReleaseContext( mHWnd, hIMC );
}

void cUIIMENode::OnImeNotify( unsigned int /*wparam*/ )
{
	HIMC hIMC = ::ImmGetContext( mHWnd );

	DWORD dwConversion, dwSentence;
	ImmGetConversionStatus( hIMC, &dwConversion, &dwSentence);

	/// 계정용이면 영문으로 변환
	if( mEditKind == eEDIT_ACCOUNT_ID || mEditKind == eEDIT_ACCOUNT_PASS )
	{
		if( dwConversion == IME_CMODE_HANGEUL )
			::ImmSetConversionStatus( hIMC, IME_CMODE_ALPHANUMERIC, 0 );
	}
	::ImmReleaseContext( mHWnd, hIMC );
}

/// 주의사항 : 
void cUIIMENode::OnImeComposition( unsigned int, unsigned int lparam )
{
	/// 숫자용 에디트이거나, 문자를 못받게 하면 리턴 
	if( mEditKind == eEDIT_NOINPUT || mEditKind == eEDIT_NUMBER || 
		mEditKind == eEDIT_ACCOUNT_ID || mEditKind == eEDIT_ACCOUNT_PASS )
		return;

	/// 드래그 상태이면 ime 입력 안받음 
	if( mPressed == true )
	{
		mImeFlag = true;
		return;
	}

	/// 조합값이 완성되서 들어온경우 리턴 ( 드래그상태에서 입력되었던 내용들 )
	if( mImeFlag == true )
	{
		mImeFlag = false;
		return;
	}	

	HIMC hIMC = ::ImmGetContext( mHWnd );

	TCHAR str[512] = {0,};

	/// 조합중인 문자열을 얻어온다
	if( lparam & GCS_COMPSTR )
	{
		/// 길이를 얻어온다
		int strLen = ::ImmGetCompositionString( hIMC, GCS_COMPSTR, NULL, 0 );
	
		/// 얻어온 길이만큼의 문자를 얻는다
		::ImmGetCompositionString( hIMC, GCS_COMPSTR, str, strLen );

		/// 문자열 끝 
		int tlen = ::_tcslen( str );
	
		str[tlen] = 0;
		AddCompWord( str, tlen );
	}
	/// 완성된 문자열을 얻어온다
	if( lparam & GCS_RESULTSTR )
	{
		/// 길이를 얻어온다
		int strLen = ::ImmGetCompositionString( hIMC, GCS_RESULTSTR, NULL, 0 );

		::ImmGetCompositionString( hIMC, GCS_RESULTSTR, str, strLen );

		int tlen = ::_tcslen( str );

		str[tlen] = 0;
		AddString( str, tlen );
	}

	::ImmReleaseContext( mHWnd, hIMC );
}

void cUIIMENode::OnImeEndComposition( unsigned int /*wparam*/, unsigned int /*lparam*/ )
{
	mCompText.Clear();
	mCompFlag = false;
	mChangeText = true;
}

/// 주의사항 : 
void cUIIMENode::OnKeyDown( eKeyCode code )
{
	switch( code )
	{
	case eKEY_C:
		{
			/// 컨트롤키를 동시에 누르면,
			if( mControlKey )
			{
				KeyDownCopy();
			}
		}
		break;

	case eKEY_V:
		{
			if( mControlKey )
			{
				KeyDownPaste();
			}
		}
		break;
	case eKEY_TAB:
		{
			/// 게임스테이지가 아니면 리턴 
			if( STAGEMAN->GetCurrentStage() != eStage_Game )
				return;

			if( mShiftKey )
			{
				/// shfit + tab 키시 포커스 체인지 
				CHATMANAGER->ChangeFocus();
			}
		}
		break;
	case eKEY_LEFT:
		{
			CaretMoveLeft();
		}
		break;

	case eKEY_RIGHT:
		{
			CaretMoveRight();
		}
		break;

	case eKEY_HOME:
		{
			mDraggingFlag = false;
			CaretMoveHome();
		}
		break;

	case eKEY_END:
		{
			mDraggingFlag = false;
			CaretMoveEnd();
		}
		break;

	case eKEY_BACK:
		{
			/// 선택영역이 있으면,
			if( mDraggingFlag )
			{
				DeleteSell();
			}
			else
			{
				KeyDownBackSpace();
			}
		}
		break;

	case eKEY_DELETE:
		{
			/// 선택영역이 있으면,
			if( mDraggingFlag )
			{
				DeleteSell();
			}
			else
			{
				KeyDownDelete();
			}
		}
		break;
	default:
		break;
	}
}

////------------------------------------------------------------------------
/// 주의사항 : 1byte 문자 삽입 
/// UNICODE 버전이므로 한글이든, 영어든 모두 한글자의 길이는 1 이다.
void cUIIMENode::AddChar( LPTSTR word )
{
	/// 드래그 상태이면 ime 입력 안받음 
	if( mPressed == true )
		return;

	mCompFlag = false;
	mFixFlag = false; 

	/// 선택영역 해제 
	DeleteSell();

	/// 입력 제한 
	if( mTextLength >= mMaxEditLength )
		return;

	TCHAR rightStr[ MAX_TEXT_LENGTH ] = {0,};
	int rightLen = GetRightString( rightStr, MAX_TEXT_LENGTH );

	/// 삽입
	mText[ mTextCaretX ] = *( word );

	/// 캐럿인덱스, 길이 갱신
	mTextCaretX++;
	mTextLength++;

	assert( mTextCaretX < MAX_TEXT_LENGTH );

	/// 문자열 중간에 삽입이었다면, 오른쪽 문자열을 삽입한다
	if( rightLen != 0 ) 
	{
		Sstrncpy( mText + mTextCaretX, MAX_TEXT_LENGTH - mTextCaretX, rightStr, rightLen );
	}

	mText[ mTextLength ] = 0;

	/// 텍스트 width 구하기 
	unsigned int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, mTextCaretX - mScrollIndex );

	/// 제한폭보다 길면 스크롤 시킴 
 	if( (int)textWidth > mOneLineWidth )
	{
		mScrollIndex++;
	}
	else
	{
		mScrollCaretX++;
	}

	mChangeText = true;
}

/// 주의사항 : 
void cUIIMENode::AddCompWord( LPTSTR word, int len )
{	
	/// 선택영역 해제 
	DeleteSell();

	if( mTextLength >= mMaxEditLength )
		return;

	/// 조립중인 한글을 저장
	mCompText.Clear();
	mCompText.Append( word, len );

	/// 조립중인지 아닌지를 길이로 판단
	if( len > 0 )
	{	
		/// 첫 조립인경우만 처리 
		if( mCompFlag == false )
		{
			int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, mTextCaretX - mScrollIndex );
			int comWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPCTSTR)mCompText.Cstr(), mCompText.GetLength() );

			/// 제한폭보다 길면 스크롤 시킴 
			if( textWidth + comWidth > mOneLineWidth )
			{
				mScrollIndex++;
				mScrollCaretX--;
			}
		}

		/// 조합중임.
		mCompFlag = true;
	}
	else if( len <= 0 )
	{
		/// 조합이 끝남
		mCompFlag = false;
	}

	mChangeText = true;
}

/// 주의사항 : 한 문자열에 특수문자, 공백 등을 처리하기 위함
void cUIIMENode::AddString( LPTSTR string, int len )
{
	LPTSTR str = string;
	for(int index = 0; index < len; ++index )
	{
		if( mEditKind == eEDIT_EXCEPT_SPECIAL )
		{
			/// 특문제외용 일경우 조건.
			if( IsValid(*str) == true )
			{
				AddChar( str );
				str++;
			}
/*
			if( (*str >= 48 && *str <= 57) ||
				(*str >= 65 && *str <= 90) || 
				(*str >= 97 && *str <= 122 ) ||
				(*str >= 12593 && *str <= 12642 ) ||
				(*str >= 44032 && *str <= 55203 ))
			{
				AddChar( str );
				str++;
			}
*/
		}
		else
		{
			/// 일반적인 경우.
			AddChar( str );
			str++;
		}
	}

	mCompText.Clear();
	mCompFlag = false;
	mChangeText = true;
}

/// 주의사항 : 캐럿을 중심으로 오른쪽 문자열을 얻는다
int cUIIMENode::GetRightString( LPTSTR dest, unsigned int maxDestLen )
{
	/// 문자열의 끝이면 리턴 
	if( mTextCaretX >= mTextLength )
		return 0;

	/// 현재 커서로부터 오른쪽 문자열 가져다가 dest에 복사
	Sstrncpy( dest, maxDestLen, mText + mTextCaretX, mTextLength - mTextCaretX );

	/// 잘라낸 오른쪽 문자열 길이 리턴 
	return (mTextLength - mTextCaretX);
}

/// 주의사항 : 캐럿을 문자열 처음으로 이동
void cUIIMENode::CaretMoveHome()
{
	mCompFlag = false;

	/// 이미 시작위치에 있으면 리턴
	if( mTextCaretX <= 0 )
		return;

	mTextCaretX = 0;
	mScrollCaretX = 0;
	mScrollIndex = 0;
	mChangeText = true;
}

/// 주의사항 : 캐럿을 문자열 끝으로 이동
void cUIIMENode::CaretMoveEnd()
{
	mCompFlag = false;

	/// 이미 끝 위치에 있으면 리턴
	if( mTextCaretX == mTextLength )
		return;

	mTextCaretX = mTextLength;
	
	/// 잘린 폭
	unsigned int cutWidth = 0;

	/// 잘린문자열의 길이 
	int cutlen = 0;

	/// 길이 검사 
	while( (int)cutWidth < mOneLineWidth )
	{
		/// 계속 검사할필요 없음.
		if( cutlen > mTextLength )
			break;

		cutlen++;

		cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mTextLength - cutlen, cutlen );
	}

	/// 문자 폭 보정 ( 폭보다 튀어가나가는 글자는 다음번에 찍도록 미룬다 )
	if( (int)cutWidth > mOneLineWidth )
	{
		cutlen--; 
	}

	/// 스크롤된 상태라면
	if( mTextLength - cutlen > 0 )
	{
		mScrollIndex = mTextLength - cutlen;
		mScrollCaretX = cutlen;
	}
	else
	{
		mScrollIndex = 0;
		mScrollCaretX = mTextLength;
	}

	mChangeText = true;
}

/// 주의사항 : 캐럿을 한 자 왼쪽으로 이동
void cUIIMENode::CaretMoveLeft()
{
	mCompFlag = false;

	/// 드래그중인 영역이 있으면,
	if( mDraggingFlag )
	{
		/// 드래깅 영역 처음으로 이동 
		if( mStartDragCaretX < mEndDragCaretX )
		{
			mTextCaretX = mScrollIndex + mStartDragCaretX;
			mScrollCaretX = mStartDragCaretX;
		}
		else
		{
			mTextCaretX = mScrollIndex + mEndDragCaretX;
			mScrollCaretX = mEndDragCaretX;
		}
		mDraggingFlag = false;
		return;
	}

	/// 이미 시작위치에 있으면 리턴
	if( mTextCaretX <= 0 )
		return;

	/// 
	mTextCaretX--;

	/// 스크롤캐럿 위치가 showtext의 첫 위치가 아니면 그냥 감소
	if( mScrollCaretX - 1 >= 0 )
	{
		mScrollCaretX--;
	}
	else
	{
		/// 앞의 문장을 반만 떙겨보여주는 소스 
		/// 검사된 문자열 폭
		unsigned int cutWidth = 0;

		/// 잘린문자열의 길이 
		int cutlen = 0;

		/// 길이 검사 
		while( (int)cutWidth < mOneLineWidth )
		{
			cutlen++;

			cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, cutlen );
		}

		int div = cutlen / 2;
		int n = mScrollIndex / div;
		if( n > 0 )
		{
			mScrollIndex -= div;
		}
		else
		{
			/// 남은 내용을 모두 땡긴다
			int value = mScrollIndex % div;
			mScrollIndex -= value;
		}

		mScrollCaretX = mTextCaretX - mScrollIndex;
		mChangeText = true;
	}
}

/// 주의사항 : 캐럿을 한 자 오른쪽으로 이동
void cUIIMENode::CaretMoveRight()
{
	mCompFlag = false;

	/// 드래그중인 영역이 있으면,
	if( mDraggingFlag )
	{
		/// 드래깅 영역 처음으로 이동 
		if( mStartDragCaretX < mEndDragCaretX )
		{
			mTextCaretX = mScrollIndex + mEndDragCaretX;
			mScrollCaretX = mEndDragCaretX;
		}
		else
		{
			mTextCaretX = mScrollIndex + mStartDragCaretX;
			mScrollCaretX = mStartDragCaretX;
		}
		mDraggingFlag = false;
		return;
	}
	
	/// 이미 끝 위치에 있으면 리턴
	if( mTextCaretX == mTextLength )
		return;

	/// 오른쪾으로 한칸 증가 
	mTextCaretX++;

	unsigned int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, mTextCaretX - mScrollIndex );

	/// 스크롤캐럿 위치가 한라인의 끝이 아니면 증가
	if( (int)textWidth < mOneLineWidth )
	{
		mScrollCaretX++;
	}
	else
	{
		/// 검사된 문자열 폭
		unsigned int cutWidth = 0;

		/// 잘린문자열의 길이 
		int cutlen = 0;

		/// 길이 검사 
 		while( (int)cutWidth < mOneLineWidth )
		{
			/// 계속 검사할필요 없음.
			if( cutlen >= mTextLength )
				break;

			cutlen++;

			cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, cutlen );
		}

		/// 문자열 땡기기 
		int div = cutlen / 2;
		int n = (mTextLength - (mScrollIndex + cutlen) ) / div;
		if( n > 0 )
		{
			mScrollIndex += div;
		}
		else
		{
			/// 남은 내용을 모두 땡긴다
			int value = (mTextLength - (mScrollIndex + cutlen)) % div + 1;
			mScrollIndex += value;
		}

	
		mScrollCaretX = mTextCaretX - mScrollIndex;
		mChangeText = true;
	}
}

/// 주의사항 : 백스페이스를 눌렸을때의 처리
void cUIIMENode::KeyDownBackSpace()
{
	if( mPressed == true )
		return;

	mCompFlag = false;

	if( mTextCaretX <= 0 )
		return;

	/// 삭제시 캐럿중심으로 오른쪽 문자열이 있는경우 저장.
	TCHAR rightStr[ MAX_TEXT_LENGTH ] = {0,};
	int rightLen = GetRightString( rightStr, MAX_TEXT_LENGTH );
	
	if( mTextCaretX > 0 )
	{
		mTextCaretX--;
	}
	else
	{
		assert(0);
	}

	if( mTextLength > 0 )
	{
		mTextLength--;
	}
	else
	{
		assert(0);
	}

	/// 스크롤캐럿 위치가 showtext의 첫 위치가 아니면 그냥 감소
	if( mScrollCaretX - 1 >= 0 )
	{
		mScrollCaretX--;
	}
	else
	{
		/*
			이부분 다시짜기-_- 가나다라마바사아
		*/

		/// 앞의 문장을 반만 떙겨보여주는 소스 
		/// 검사된 문자열 폭
		unsigned int cutWidth = 0;

		/// 잘린문자열의 길이 
		int cutlen = 0;

		/// 길이 검사 
		while( (int)cutWidth < mOneLineWidth )
		{
			cutlen++;

			cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mText + mScrollIndex, cutlen );
		}

		int div = cutlen / 2;
		int n = mScrollIndex / div;
		if( n > 0 )
		{
			mScrollIndex -= div;
		}
		else
		{
			/// 남은 내용을 모두 땡긴다
			int value = mScrollIndex % div;
			mScrollIndex -= value;
		}

		mScrollCaretX = mTextCaretX - mScrollIndex;
		
	}

	/// 문자열 끝에 쓰레기 값이 없도록 청소
	ClearText( mTextCaretX );
	mText[ mTextCaretX ] = 0;

	/// 오른쪽 문자열이 존재하면 
	if( rightLen != 0 )
	{
		/// 문자열 끝에 쓰레기 값이 없도록 청소
		ClearText( mTextCaretX );

		Sstrncpy( mText + mTextCaretX, MAX_TEXT_LENGTH - mTextCaretX, rightStr, rightLen );
	}

	mChangeText = true;
}

/// 주의사항 : delete 키를 눌렀을때의 처리
void cUIIMENode::KeyDownDelete()
{
	if( mPressed == true )
		return;

	mCompFlag = false;

	/// 이미 끝 위치에 있으면 리턴
	if( mTextCaretX == mTextLength )
		return;

	/// 삭제시 캐럿중심으로 오른쪽 문자열이 있는경우 저장.
	TCHAR rightStr[ MAX_TEXT_LENGTH ] = {0,};
	int rightLen = GetRightString( rightStr, MAX_TEXT_LENGTH );

	// 만약 선택영역을 지울경우에는 다른 값을 넣도록 한다.
	if( mTextLength > 0 )
	{
		mTextLength--;
	}
	else
	{
		assert(0);
	}

	if( rightLen != 0 )
	{
		/// 문자열 끝에 쓰레기 값이 없도록 청소
		ClearText( mTextCaretX );

		Sstrncpy( mText + mTextCaretX, MAX_TEXT_LENGTH - mTextCaretX, rightStr + 1, rightLen - 1 );
	}

	/// 문자열 끝에 쓰레기 값이 없도록 청소
	ClearText( mTextLength );
	mText[ MAX_TEXT_LENGTH ] = 0;

	mChangeText = true;
}

/// 주의사항 : mText에서 쓰지 않는 인덱스를 지워버린다 
void cUIIMENode::ClearText( int index )
{
	/// copy size 검증 필요!!
	TCHAR str[ MAX_TEXT_LENGTH ] = {0,};
	Sstrncpy( str, MAX_TEXT_LENGTH, mText, index );

	::ZeroMemory( mText, sizeof(mText) );
	Sstrncpy( mText, MAX_TEXT_LENGTH, str, index );

	mChangeText = true;
}

/// 주의사항 : 
void cUIIMENode::KeyDownCopy()
{
	/// 원본
	LPTSTR str = mText;

	/// 복사 저장 문자열 
	TCHAR copyStr[ MAX_TEXT_LENGTH ] = {0,};

	/// 복사해둘 문자열 길이
	int copyLen = mEndDragCaretX - mStartDragCaretX;
	if( copyLen == 0 )
		return;
	
	if( copyLen > 0 )
	{
		Sstrncpy( copyStr, MAX_TEXT_LENGTH, str + mStartDragCaretX + mScrollIndex, copyLen );
	}
	else if( copyLen < 0 )
	{
		/// 양수변환
		copyLen = (-copyLen);
		Sstrncpy( copyStr, MAX_TEXT_LENGTH, str + mEndDragCaretX + mScrollIndex, copyLen );
	}
	
	/// 멀티바이트로 변환 
	char asciiStr[ MAX_TEXT_LENGTH ] = {0,};
	ConvertToAscii( copyStr, asciiStr, MAX_TEXT_LENGTH );

	int asciilen = ::strlen( asciiStr );

	/// 클립보드에 붙여넣기 
	HGLOBAL hMem;

	/// 전역 메모리 할당 
	hMem = ::GlobalAlloc( GHND, (asciilen + 1) );

	/// 메모리 사용 허가권 얻기 
	char* pBuff = (char*)::GlobalLock( hMem );

	/// 클립보드에 복사 
	memcpy( pBuff, asciiStr, asciilen );
	pBuff[ asciilen ] = 0;

	/// 메모리 허가 해제
	::GlobalUnlock( hMem );

	/// 클립보드 사용 가능하다면, 삽입
	if( OpenClipboard( mHWnd ) )
	{
		/// 기존의 데이터를 지운다
		::EmptyClipboard();

		/// 삽입
		::SetClipboardData( CF_TEXT, hMem );

		::CloseClipboard();
	}
}

/// 주의사항 : 
void cUIIMENode::KeyDownPaste()
{
	if( mPressed == true )
		return;

	/// 붙여넣기 할때 선택영역이 있으면 먼저 지운다 
	DeleteSell();

	/// 클립보드에 붙여넣기 
	HGLOBAL hMem;

	/// 클립보드에 TEXT 타입의 문자가 있으면, 가져온다
	if( ::IsClipboardFormatAvailable( CF_UNICODETEXT ) )
	{
		/// 열리지 않으면 리턴
		if( ::OpenClipboard( mHWnd ) == false )
			return;

		hMem = ::GetClipboardData( CF_UNICODETEXT );

		/// 메모리 락
		wchar_t* pBuff = (wchar_t*)::GlobalLock( hMem );
		int plen = ::wcslen(pBuff);
		if( plen == 0 )
		{
			::GlobalUnlock( hMem );
			::CloseClipboard();
			return;
		}

		/// 데이터 저장 
		if( plen - MAX_TEXT_LENGTH > 0 )
		{
			memset( pBuff + MAX_TEXT_LENGTH - 1, 0, plen - MAX_TEXT_LENGTH );
			plen = ::wcslen(pBuff);
		}

		LPTSTR s = pBuff;

		for(int index = 0; index < plen; ++index )
		{	
			// 탭 제거
			if( !(*s >= 0x0000 && *s <= 0x001f) )
			{
				AddChar( s );
			}
			s++;
		}		


		/// 해제
		::GlobalUnlock( hMem );

		/// 닫기 
		::CloseClipboard();

		mChangeText = true;
	}
}

/// 주의사항 : 영역이 선택된 상태에서 backspace 입력 혹은  delete 입력
void cUIIMENode::DeleteSell()
{
	if( mPressed == true )
		return;

	/// 이미 선택영역이 없으면 리턴 
	if( mDraggingFlag == false )
		return;

	/// 선택 영역 해제
	mDraggingFlag = false;

	int delstart = 0;
	int len = mEndDragCaretX - mStartDragCaretX;
	if( len == 0 )
		return;

	if( len < 0 )
	{
		len = (-len);
		delstart = len;
		mTextCaretX = mEndDragCaretX + mScrollIndex;
	}
	else if( len > 0 )
	{
		mTextCaretX = ( mStartDragCaretX > 0 ) ? mStartDragCaretX + mScrollIndex : mStartDragCaretX;
	}

	/// 삭제시 캐럿중심으로 오른쪽 문자열이 있는경우 저장.
	TCHAR rightStr[ MAX_TEXT_LENGTH ] = {0,};
	
	int rightLen = GetRightString( rightStr, MAX_TEXT_LENGTH );

	/// 살아 있는 텍스트 삭제 ( 선택 영역 부분만 확실히 삭제 )
	for( int i = mTextCaretX; i < mTextCaretX + delstart + len; ++i )
	{
		mText[i] = _T('\0');
	}	

	if( rightLen != 0 )
	{
		Sstrncpy( mText + mTextCaretX, MAX_TEXT_LENGTH - mTextCaretX, rightStr + len, rightLen - len );
	}

	if( mTextCaretX == 0 )
		mScrollIndex = 0;

	mScrollCaretX = mTextCaretX - mScrollIndex;

	if( mTextLength - len >= 0 )
	{
		mTextLength -= len;
	}
	else
	{
		assert(0);
	}

	mText[ mTextLength ] = 0;
	len = 0;

	mChangeText = true;
}
