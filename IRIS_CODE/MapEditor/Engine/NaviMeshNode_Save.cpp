#include "stdafx.h"
#include "NaviMeshNode.h"

bool cNaviMeshBranchNode::Save( cFileSaver& saver )
{
	/// 경계 상자를 저장
	if( saver.Write( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	/// 자식 노드들을 저장
	if( mChild[0]->Save( saver ) == false )
		return false;
	if( mChild[1]->Save( saver ) == false )
		return false;
	if( mChild[2]->Save( saver ) == false )
		return false;
	if( mChild[3]->Save( saver ) == false )
		return false;
	return true;
}

bool cNaviMeshLeafNode::Save( cFileSaver& saver )
{
	/// 경계 상자를 저장
	if( saver.Write( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;
	return true;
}
