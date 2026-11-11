/* ==========================================================================
* 파    일 : NumberEditBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-02
* 주의사항 : 숫자만을 쓸수 있는 확장 에디트 
			 스크롤기능, 복사, 붙여넣기 기능 제외
*===========================================================================*/
#pragma once
#include "EditBox.h"

//const unsigned int MAX_NUMBER_LEN = 10;

class cNumberEditBox : public cEditBox
{
public:
	cNumberEditBox( eUINodeType type = eUINODE_EDITBOX );
	virtual ~cNumberEditBox();

	/// 0226 웅주, 값을 세팅
	void SetValue	( unsigned long value );
	void SetMaxValue( unsigned long value );

	/// 0226 웅주, 값을 반환
	unsigned long GetValue	() const;
	unsigned long GetMaxValue	() const;

	void Clear();

	static LPCTSTR FormatNumber	( double value, bool zero = true );
	void SetUseZero( bool set );

private:
	virtual void SetText( LPCTSTR text, int align /* = 0  */);/// 이런식의 사용 금

	/// 이벤트 처리
	virtual void OnChar( unsigned int wparam );
	virtual void OnFocusLost();
	virtual void OnLButtonDown( const cUIPos&, bool, bool , bool );
	virtual bool OnCreate( cUINodeProperty* );
	virtual void OnKeyDown( eKeyCode code );

	/// 0226 웅주(수정), 숫자문자열을 숫자와 ',' 로 변형하여 mNumberText에 넣는다.
	LPCTSTR FormatNumber	( LPCTSTR )	const;
	LPCTSTR UnformatNumber	( LPCTSTR )	const;

	/// 0226 웅주, 쉼표를 넘기기 위해 오버라이딩
	void CaretMoveLeft();
	void CaretMoveRight();
	void KeyDownDelete();
	void KeyDownBackSpace();

private:
	unsigned long	mMaxValue;
	unsigned long   mValue;

	/// 0 값 세팅 사용할것인지 여부
	bool mUseZero;
};

inline void cNumberEditBox::SetUseZero( bool set )
{
	mUseZero = set;
}
