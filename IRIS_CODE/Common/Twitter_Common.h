#pragma once

#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"

#pragma pack(push,1)

struct MSG_SYN_TWITTER_MONSTER_DIE : public MSGROOT
{
	unsigned long	mMonsterIdx;
};

#pragma pack(pop)
