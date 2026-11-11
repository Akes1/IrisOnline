#pragma once

#ifdef _CLIENT
	#include "HShield.h"
#else
	#include "AntiCpXSvr.h"
#endif

#include "CommonNetworkMsgStruct.h"

struct MSG_SYN_HACKSHIELD : public MSGROOT
{
	unsigned short nLength;
	unsigned char byBuffer[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_HACKSHIELD) - sizeof(byBuffer) + (nLength * sizeof(unsigned char));} 
};

struct MSG_REQ_HACKSHIELD : public MSGROOT
{
	unsigned short nLength;
	unsigned char byBuffer[ANTICPX_TRANS_BUFFER_MAX/* 송수신 패킷의 최대 크기 */];

	unsigned long GetMsgLength() { return sizeof(MSG_REQ_HACKSHIELD) - sizeof(byBuffer) + (nLength * sizeof(unsigned char));} 
};
