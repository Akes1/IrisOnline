/* ====================================================================
 * 파    일 :	packet.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/26
 * 주의사항 :	
 * =================================================================== */
#ifndef __PACKET_H__
#define __PACKET_H__

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

struct Packet
{
	u_long ver  :  4;      // header version   4bit.
	u_long hlen :  4;      // header length    4bit.
	u_long tos  :  8;      // type of service  8bit.
	u_long tlen : 16;      // total  length   16bit.
	u_long seq;            // sequence number 32bit.
};

#define PHVer       0x01    // packet header version.
#define PHLen       0x08    // packet header length.


struct RUDP : Packet
{
	u_long  addr;           // source address.
	u_short port;           // aource port.
};

#define RUDP_PHVer  0x02    // reliable udp header version
#define RUDP_PHLen  0x0e    // reliable udp header length


#define SM_ClassA   0xffffffff
#define SM_ClassB   0x00ffffff
#define SM_ClassC   0x0000ffff
#define SM_ClassD   0x000000ff

enum TYPE_OF_SERVICE
{
	TOS_CLIENT = 1,

	TOS_LOGIN,
	TOS_LOGIN_ACCEPT,
	TOS_LOGIN_2_GAME,
	TOS_LOGIN_LOG,

	TOS_GAME,
	TOS_GAME_ACCEPT,
	TOS_GAME_2_GAME,
	TOS_GAME_LOG,
	
	TOS_SERVERTOOL,
	TOS_TTL,
	TOS_RELAY,
	TOS_RUDP,
	TOS_HACKSHIELD,
	TOS_MAX,
};

// 1Byte 정렬 해제
#pragma pack( )

#endif // __PACKET_H__
