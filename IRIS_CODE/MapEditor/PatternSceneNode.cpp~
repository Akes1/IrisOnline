#include "StdAfx.h"
#include "../mapeditor/PatternSceneNode.h"
#include "Engine/NaviMesh.h"

cPatternSceneNode::cPatternSceneNode( eSceneNodeType type )
: cStaticSceneNode( type )
, mAppendZ(0)
{

}

cPatternSceneNode::~cPatternSceneNode()
{
}

bool cPatternSceneNode::Init( cPatternSceneNodeParam& param )
{
	mAppendZ = param.mAppandZ;

	float height = 0.0f;
	NAVIMESH->CalcHeight( &height, param.mTranslate.x, param.mTranslate.y );
	param.mTranslate.z = height + mAppendZ;

	bool check = cStaticSceneNode::Init( param );

	if( check == false )
		return false;


	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	GetNiNode()->AttachProperty( mWireProp );
	GetNiNode()->UpdateProperties();
	
	return true;
}

void cPatternSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// 반복 동작을 가진 경우 엔진 노드를 처리
	/*if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}*/

	cSceneNode::Process( deltaTime, accumTime );

	mCheckFrustum = ( mAlpha < 0.000001f ) ? false : true;
}

void cPatternSceneNode::SetMaterialColor( NiColor color )
{
	cMaterialDataList::cIterator i = mMatDataList.Begin();
	cMaterialDataList::cIterator iend = mMatDataList.End();
	for( ; i != iend; ++i )
	{
		cMaterialData* matData = (cMaterialData*)(*i);
		NiMaterialProperty* mat = matData->mProp;
		if( mat )
			mat->SetDiffuseColor( color );
	}
}

void cPatternSceneNode::SetWireframe( bool wireFrame )
{
	mWireProp->SetWireframe( wireFrame );
}
