/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.17
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cMonsterSceneNode;

/// 몬스터 정보
class cMonsterInfo
{
public:
	cMonsterInfo( unsigned int nameIndex, unsigned int modelIndex );

public:
	/// 이름 인덱스
	unsigned int mNameIndex;

	/// 모델 인덱스
	unsigned int mModelIndex;
};

inline
cMonsterInfo::cMonsterInfo( unsigned int nameIndex, unsigned int modelIndex )
: mNameIndex( nameIndex )
, mModelIndex( modelIndex )
{
}

/// 몬스터 그룹
class cMonsterGroup
{
public:
	cMonsterGroup( unsigned int index );
	~cMonsterGroup();

	/// 저장
	bool Save( cFileSaver& saver );

	/// 리스트에서 노드를 제거
	void RemoveNode( cMonsterSceneNode* node );

public:
	/// 인덱스
	unsigned int mIndex;

	/// 최초 리젠 여부
	bool mFirstRegen;

	bool mChannelCheck;

	/// 다음 리젠될 그룹들의 목록
	typedef tHashMap<unsigned int, unsigned int> cNextRegenGroupMap;
	cNextRegenGroupMap mNextRegenGroupMap;

	unsigned int mMapGroupIndex;

	/// 리젠 퍼센트
	unsigned int mRegenPercent;

	/// 리젠 범위
	unsigned int mRegenRange;

	/// 리젠 대기 시간
	unsigned int mRegenWaitTime;

	/// 몬스터 리스트
	typedef tList<cMonsterSceneNode*> cNodeList;
	cNodeList mNodeList;
};

inline
cMonsterGroup::cMonsterGroup( unsigned int index )
: mIndex( index )
, mFirstRegen( true )
, mChannelCheck( false )
, mMapGroupIndex( 0 )
, mRegenPercent( 50 )
, mRegenRange( 100 )
, mRegenWaitTime( 10000 )
{
}
