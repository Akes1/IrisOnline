/* ====================================================================
 * 파    일 :	MsgBuf.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/08/17
 * 주의사항 :	
 * =================================================================== */
#ifndef __MSG_BUF_H__
#define __MSG_BUF_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

// 윈도우즈 헤더
#include <windows.h>

// 1Byte 정렬 설정
#pragma pack(1)

#ifndef _MSGBUF_
#define _MSGBUF_
struct MSGBUF
{
	char protocol;
};
#endif

// 1Byte 정렬 해제
#pragma pack( )

#endif // __MSG_BUF_H__