/* ==========================================================================
* 파    일 : UIText.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-23
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Lexer.h"

class cParser;

/// UI 문자열
class cUIText
{
public:
	cUIText();
	~cUIText();

	/// 로딩
	bool Load( const cString& pathName );

	/// 텍스트를 리턴
	bool GetText( cString* ptext, unsigned int index );
	const cString& GetText( unsigned int id ) const;

private:
	typedef tHashMap<unsigned int, cString> cStringMap;
	cStringMap mStringMap;

	///
	cString mErrorText;
};
