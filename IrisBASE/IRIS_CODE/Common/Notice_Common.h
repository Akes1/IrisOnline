/* ==========================================================================
* 작 성 자 : 조영인
* 작 성 일 : 2009.09.15
* 내    용 : 서버에게 공지를 보냄
* 주의사항 : 플레이어의 Gamein 상태 체크를 서버 측에서 하지 않는다.
*===========================================================================*/
#pragma once

#pragma pack( push, 1 )
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"

/// 서버 공지
struct MSG_REQ_NOTICE_SERVER : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_REQ_NOTICE_SERVER) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

struct MSG_SYN_NOTICE_SERVER : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_SYN_NOTICE_SERVER) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

/// 채널 공지
struct MSG_REQ_NOTICE_CHANNEL : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_REQ_NOTICE_CHANNEL) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

struct MSG_SYN_NOTICE_CHANNEL : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_SYN_NOTICE_CHANNEL) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

/// 지역 공지
struct MSG_REQ_NOTICE_AREA : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_REQ_NOTICE_AREA) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

struct MSG_SYN_NOTICE_AREA : public MSGROOT
{
	wchar_t mMessage[MAX_CHAT_BUFFER_SIZE]; /// 메시지
	unsigned long GetMsgLength() { return (unsigned long)(sizeof(MSG_SYN_NOTICE_AREA) - sizeof(mMessage) + ::wcslen(mMessage)*sizeof(wchar_t)); }
};

/// 서버다운 카운트 서버 공지
struct MSG_REQ_NOTICE_DOWN_COUNT_SERVER : public MSGROOT
{
	unsigned short mCount;
};

struct MSG_SYN_NOTICE_DOWN_COUNT_SERVER : public MSGROOT
{
	unsigned short mCount;
};

/// 서버다운 카운트 채널 공지
struct MSG_REQ_NOTICE_DOWN_COUNT_CHANNEL : public MSGROOT
{
	unsigned short mCount;
};

struct MSG_SYN_NOTICE_DOWN_COUNT_CHANNEL : public MSGROOT
{
	unsigned short mCount;
};

#pragma pack( pop )