/* ====================================================================
 * 파    일 :	loginSrvDefines.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/11/05
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOGIN_SERVER_DEFINES_H__
#define __LOGIN_SERVER_DEFINES_H__

#pragma once

//#pragma pack(1) // 1Byte 정렬 설정
//#pragma pack( ) // 1Byte 정렬 해제

struct TB_CAPTCHA
{
	long    idx;							// 고유번호
	wchar_t name[51];						// 이름
	long    size;							// 크기
	char    image[8000];					// 이미지
};

struct TB_SERVER
{
	long    idx;							// 고유번호
	wchar_t name[51];						// 이름
	wchar_t description[201];				// 설명
	BYTE    status;							// 상태 (0준비/1원활/2보통/3혼잡)
	BYTE    age;							// 적용나이
};

struct TB_CHANNEL
{
	long idx;								// 고유번호
	long serverNum;							// 서버번호
	long channelNum;						// 채널번호
	char inetnum[21];						// INetnum-IPv4
	BYTE status;							// 상태 (0준비/1원활/2보통/3혼잡)
};

#endif // __LOGIN_SERVER_DEFINES_H__
