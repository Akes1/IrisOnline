#include "stdafx.h"
#include "Monster.h"

#include "SceneManager.h"
#include "MonsterSceneNode.h"

cMonsterGroup::~cMonsterGroup()
{
	cNodeList::cIterator i = mNodeList.Begin();
	cNodeList::cIterator iend = mNodeList.End();

	for( ; i != iend; ++i )
	{
		cSceneNode* n = *i;

		/// 장면 관리자에서 제거
		SCENEMAN->DestroyNode( n );
	}
}

void cMonsterGroup::RemoveNode( cMonsterSceneNode* node )
{
	assert( node );

	mNodeList.Remove( node );
}

bool cMonsterGroup::Save( cFileSaver& saver )
{
	if( mNodeList.IsEmpty() )
		return false;

	/// 인덱스
	cString str;
	str.Format( "%d\r\n{\r\n", mIndex );
	saver.Write( str.Cstr(), str.GetSize() );

	/// 그룹 리젠 정보
	str.Format( "\t%d\t%d\t%d\t%d\t%d\t%d", mFirstRegen?1:0, mRegenPercent, mRegenRange, mRegenWaitTime, mMapGroupIndex, mChannelCheck );

	unsigned int count = 0;
	cNextRegenGroupMap::cIterator b = mNextRegenGroupMap.Begin();
	cNextRegenGroupMap::cIterator bend = mNextRegenGroupMap.End();

	cString nextGroupList = "";
	cString temp;
	for( ; b != bend; ++b )
	{
		unsigned int idx = b->mFirst;
		temp.Format( "\t%d", idx );
		nextGroupList += temp;
		count++;
	}

	temp.Format( "\t%d", count );

	str += temp;
	if( count != 0 )
		str += nextGroupList;

	str += "\r\n";
	saver.Write( str.Cstr(), str.GetSize() );

	/// 몬스터 리젠 리스트
	cNodeList::cIterator i = mNodeList.Begin();
	cNodeList::cIterator iend = mNodeList.End();

	for( ; i != iend; ++i )
	{
		cMonsterSceneNode* n = *i;
		unsigned int index = n->GetUserData0();
		const NiPoint3& t = n->GetWorldTranslate();
		const NiMatrix3& r = n->GetWorldRotate();
		float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
		r.ToEulerAnglesXYZ( xangle, yangle, zangle );

		str.Format( "\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", index, (unsigned int)t.x, (unsigned int)t.y, (unsigned int)D3DXToDegree(zangle), 
			n->GetUserData1(), n->GetUserData3(), n->GetUserData4(), n->GetUserData5() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	///
	saver.Write( "}\r\n", 3 );
	return true;
}
