#pragma once
#pragma pack(push,1)

#include "CommonNetworkMsgStruct.h"

struct MSG_SYN_OBSTRUCTION_LIST : public MSGROOT
{
	unsigned char		mCount;
	unsigned long		mObstructionIdx[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_OBSTRUCTION_LIST) - sizeof(mObstructionIdx) + (mCount * sizeof(unsigned long));} 
};

struct MSG_SYN_OBSTRUCTION_DELETE : public MSGROOT
{
	unsigned long		mObstructionIdx;
};

#pragma pack(pop)
