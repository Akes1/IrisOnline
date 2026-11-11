// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#define _AFXDLL

// 아래 지정된 플랫폼에 우선하는 플랫폼을 대상으로 하는 경우 다음 정의를 수정하십시오.
// 다른 플랫폼에 사용되는 해당 값의 최신 정보는 MSDN을 참조하십시오.
#ifndef WINVER				// Windows XP 이상에서만 기능을 사용할 수 있습니다.
#define WINVER 0x0400		// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_WINNT		// Windows XP 이상에서만 기능을 사용할 수 있습니다.                   
#define _WIN32_WINNT 0x0400	// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 이상에서만 기능을 사용할 수 있습니다.
#define _WIN32_WINDOWS 0x0400 // Windows Me 이상에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

// 아래 플래그는 기존에 사용하던 String관련 함수들도 그대로 두도록 한것임.
#define STRSAFE_NO_DEPRECATE

/// 추후에 없애자
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

/// 비표준 확장을 사용했습니다 : 구조체/공용 구조체의 이름이 없습니다.
#pragma warning( disable: 4201 )

// 복사 생성자를 생성하지 못했습니다.
#pragma warning( disable: 4511 )

// 컴파일러에서 주어진 클래스에 대한 대입 연산자를 생성할 수 없습니다.
#pragma warning( disable: 4512 )

// 조건식이 상수입니다.
#pragma warning( disable: 4127 )

// 참조되지 않은 형식 매개 변수입니다.
#pragma warning( disable: 4100 )


// Windows 헤더 파일:
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <ocidl.h>
#include <time.h>


#include <MMSystem.h>

#pragma warning (push, 3)

#include "NiPoint2.h"
#include "NiTransform.h"
#include "NiTQueue.h"

#pragma warning (pop)



//#define STRSAFE_LIB
//#include <STRSAFE.H>

#undef assert
#define assert(exp) {if(!(exp)) __asm{int 0x03}}


/// STL
#include "Pool.h"
#include "BasicString.h"
#include "Algorithm.h"
#include "Allocator.h"
#include "PriorityQueue.h"
#include "Array.h"
#include "DefragmentArray.h"
#include "List.h"
#include "HashMap.h"
#include "HashSet.h"
#include "PointerArray.h"
#include "PointerList.h"
#include "PointerHashMap.h"
#include "PointerHashSet.h"


/// 파일
#include "Console.h"
#include "File.h"
#include "FileLoader.h"
#include "FileSaver.h"
#include "GameFile.h"
#include "Parser.h"
#include "Lexer.h"

/// 토큰
#include "Tokenizer.h"

#include "CommonNetworkMsgStruct.h"
#include "RangeCheck.h"


const int SECOND = 1000;
const int MINUTE = 60 * SECOND;
const int HOUR = 60 * MINUTE;
const int DAY = 24 * HOUR;

#define S_PI    (3.14159265358979323846f)
#define S_ToRadian( degree ) ((degree) * (S_PI / 180.0f))

/// defines
#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#define SAFE_EXIT_DELETE(x) if(x) { x->Exit(); delete x; x = NULL; }

int ConvertToAscii( const wchar_t* uniStr, char* str, unsigned int bufferSize );
int ConvertToUnicode( const char* str, wchar_t* uniStr, unsigned int buffersize );
