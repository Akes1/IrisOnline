#pragma once
#pragma pack(push,1)

#include "CommonNetworkMsgStruct.h"


struct sVehicleList
{
	unsigned long	mVehicleIdx;		/// index

	float			mObjectFixSize;		/// size

	cString			mFileName;			/// kfm file
	cString			mTextureName;		/// nif or tga file

	unsigned long	mPlayerIdle1;		/// player ani index
	unsigned long	mPlayerIdle2;
	unsigned long	mPlayerRun;
	unsigned long	mPlayerJump;
};


#pragma pack(pop)
