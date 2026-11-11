/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.24
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cPortalSceneNode;

/// 포탈 정보
class cPortalInfo
{
public:
	cPortalInfo( const char* potalName, unsigned int mapIndex, float x, float y, unsigned int range, unsigned int targetIndex, int zdegree, float nameheight, bool IsCreated, cPortalSceneNode* node );

public:

	cString mPotalName;

	/// 소속맵 인덱스
	unsigned int mMapIndex;

	bool		mIsCreateView;

	/// 좌표
	float mX;
	float mY;

	/// 범위
	unsigned int mRange;

	/// 대상 포탈 인덱스
	unsigned int mTargetIndex;

	/// 회전
	int mZDegree;

	/// 이름 높이값
	float mNameHeight;

	/// 노드
	cPortalSceneNode* mNode;
};

inline
cPortalInfo::cPortalInfo( const char* potalName, unsigned int mapIndex, float x, float y, unsigned int range, unsigned int targetIndex, int zdegree, float nameheight, bool IsCreated, cPortalSceneNode* node )
: mMapIndex( mapIndex )
, mX( x )
, mY( y )
, mRange( range )
, mTargetIndex( targetIndex )
, mZDegree( zdegree )
, mNameHeight( nameheight )
, mIsCreateView(IsCreated )
, mNode( node )
{
	mPotalName = potalName;
}
