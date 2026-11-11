#pragma once

class cGatheringSceneNode;

class cGatheringInfo
{
public:
	cGatheringInfo( unsigned int nameIndex, cString& filename );

public:

	/// 이름 인덱스
	unsigned int mNameIndex;

	/// 모델
	cString mFileName;;
};

inline
cGatheringInfo::cGatheringInfo( unsigned int nameIndex, cString& filename )
: mNameIndex( nameIndex )
, mFileName( filename )
{
}


class cGatheringGroup
{
public:
	cGatheringGroup( unsigned int index, unsigned int regenWaitTime = 10000, unsigned int regenRandTime = 10000 );
	virtual ~cGatheringGroup();

	/// 저장
	bool Save( cFileSaver& saver );

	/// 리스트에서 노드를 제거
	void RemoveNode( cGatheringSceneNode* node );

public:
	/// 인덱스
	unsigned int mIndex;

	/// 리젠되는 그룹 인덱스
	unsigned int mRegenGroupIndex;

	/// 리젠 대기 시간
	unsigned int mRegenWaitTime;

	/// 리젠 추가 랜덤 시간
	unsigned long mRegenRandTime;

	/// 몬스터 리스트
	typedef tList<cGatheringSceneNode*> cNodeList;
	cNodeList mNodeList;
};

inline
cGatheringGroup::cGatheringGroup( unsigned int index, unsigned int regenWaitTime, unsigned int regenRandTime )
: mIndex( index )
, mRegenGroupIndex( index )
, mRegenWaitTime( regenWaitTime )
, mRegenRandTime( regenRandTime )
{
}
