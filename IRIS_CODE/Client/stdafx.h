// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define WINVER  0x0500
#define _WIN32_WINNT 0x0500 
#define DIRECTINPUT_VERSION         0x0800


// 아래 플래그는 기존에 사용하던 String관련 함수들도 그대로 두도록 한것임.
#define STRSAFE_NO_DEPRECATE

/// 추후에 없애자
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#pragma warning (push, 3)

#include <NiBuildConfiguration.h>

#include <NiMain.h>

#include <NiSystem.h>

#include <NiMeshLib.h>

/// Animation
#include <NiParticle.h>
#include <NiAnimation.h>

#include <NiUniversalTypes.h>
#include <NiVersion.h>

#include <NiDX9SystemDesc.h>
#include <NiD3DDefines.h>

#include <NiDirectXVersion.h>

#include <NiCollision.h>

#include <NiFilename.h>

/// Shader
#include <NiShaderFactory.h>
#include <NiShaderLibrary.h>
#include <NiShaderLibraryDesc.h>

#include <NiD3DShaderProgramFactory.h>

#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <ocidl.h>
#include <time.h>
#include <process.h>


#include <dinput.h>

#include <MMSystem.h>

#include <NiMilesAudio.h>


#pragma warning (pop)

///


#define STRSAFE_LIB
#include <STRSAFE.H>

/// 080403 Client Network Model (신규통신 모듈) - 최종수정.
#include "iocpclient.h"


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

/// 토큰
#include "Tokenizer.h"

#include "XPerformanceGuard.h"

/// defines
#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#define SAFE_NIDELETE(x) if(x) { NiDelete x; x = NULL; }
#define SAFE_EXIT_DELETE(x) if(x) { x->Exit(); delete x; x = NULL; }

inline long FloatToInt( float value ) { int i = (int)value; return ( value - i >= 0.999f ) ? ++i : i; }

const int SECOND = 1000;
const int MINUTE = 60 * SECOND;
const int HOUR = 60 * MINUTE;
const int DAY = 24 * HOUR;

#define S_PI    (3.14159265358979323846f)
#define S_ToRadian( degree ) ((degree) * (S_PI / 180.0f))

int ConvertToAscii( const wchar_t* uniStr, char* str, unsigned int bufferSize );
int ConvertToUnicode( const char* str, wchar_t* uniStr, unsigned int buffersize );
int ConvertUTF8ToWideChar( const char* str, wchar_t* uniStr, unsigned int buffersize );

#define MAXSTRINGLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
void Sstrncpy( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );
void Sstrncat( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );

#ifdef _DEVSYS
#undef assert
#define assert(exp) {if(!(exp)) __asm{int 0x03}}
#endif

void GameErrorLog( char* format, ... );
