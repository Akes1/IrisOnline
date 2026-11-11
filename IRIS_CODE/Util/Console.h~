/* ==========================================================================
* 파    일 : Console.h
* 목    적 : 콘솔 창에 메시지를 출력 (주로 디버깅 용도로 사용된다)
* 주의사항 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.01
*
* 추가내용 :
*      날짜     작업자  내용
*===========================================================================*/
#pragma once

// 에러 파일, 라인
#define ERROR_FILE	__FILE__
#define ERROR_LINE	__LINE__

/// 콘솔
class cConsole
{
public:
	/// 분리선 출력
	static void PrintLine();

	/// 에러 메시지 출력
	static void Print( const char* pfmt, ... );
	static void Error( const char* pfile, int line, const char* pfmt, ... );
};
