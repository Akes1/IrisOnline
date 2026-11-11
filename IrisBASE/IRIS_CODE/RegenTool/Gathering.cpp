#include "stdafx.h"
#include ".\gathering.h"

#include "SceneManager.h"
#include "GatheringSceneNode.h"


cGatheringGroup::~cGatheringGroup()
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

void cGatheringGroup::RemoveNode( cGatheringSceneNode* node )
{
	assert( node );

	mNodeList.Remove( node );
}

bool cGatheringGroup::Save( cFileSaver& saver )
{
	if( mNodeList.IsEmpty() )
		return false;

	/// 인덱스
	cString str;
	str.Format( "%d\t%d\t%d\r\n{\r\n", mIndex, mRegenWaitTime, mRegenRandTime );
	saver.Write( str.Cstr(), str.GetSize() );

	/// 몬스터 리젠 리스트
	cNodeList::cIterator i = mNodeList.Begin();
	cNodeList::cIterator iend = mNodeList.End();

	for( ; i != iend; ++i )
	{
		cGatheringSceneNode* n = *i;
		unsigned int index = n->GetUserData0();
		const NiPoint3& t = n->GetWorldTranslate();
		const NiMatrix3& r = n->GetWorldRotate();
		float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
		r.ToEulerAnglesXYZ( xangle, yangle, zangle );

		str.Format( "\t%d\t%d\t%d\t%d\t%f\r\n", index, (unsigned int)t.x, (unsigned int)t.y, (unsigned int)D3DXToDegree(zangle), n->GetAppendZ() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	///
	saver.Write( "}\r\n", 3 );
	return true;
}
