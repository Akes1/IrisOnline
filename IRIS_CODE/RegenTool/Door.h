#pragma once

class cDoorSceneNode;

class cDoorInfo
{
public:
	cDoorInfo();
	~cDoorInfo();

public:

	cString mFileName;

	/// 소속맵 인덱스
	unsigned int mMapIndex;

	/// 좌표
	NiPoint3 mPos;

	float mAppendZ;

	/// 회전
	int mZDegree;

	/// 통과 제한 라인정보
	NiPoint3 mRectPos[4];
	float mRectAppendY;

	cDoorSceneNode* mNode;
};

inline
cDoorInfo::cDoorInfo()
{
	mMapIndex = 0;

	mPos = NiPoint3::ZERO;
	mAppendZ = 0.0f;

	for( unsigned int i=0; i<4; ++i )
		mRectPos[i] = NiPoint3::ZERO;

	mRectAppendY = 0.0f;

	mNode = 0;
}

inline 
cDoorInfo::~cDoorInfo()
{
	mNode = 0;
}