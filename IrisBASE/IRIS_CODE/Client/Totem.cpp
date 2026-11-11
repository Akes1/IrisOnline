#include "StdAfx.h"
#include ".\totem.h"

#include "TotemScript.h"
#include "SkillObject.h"

#include "SceneManager.h"
#include "EffectSceneNode.h"

cTotem::cTotem()
{
}

cTotem::~cTotem()
{
	if( mLoopEffect.IsEmpty() == false )
	{
		tPointerList<void*>::cIterator b = mLoopEffect.Begin();
		tPointerList<void*>::cIterator end = mLoopEffect.End();
		for( ; b != end; ++b )
		{
			sLoopEffect* p = (sLoopEffect*)(*b);
			if( p == 0 )
				continue;

			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( p->mIdxByManager );
			if( node )
			{
				assert( node == p->mEffectNode );
				//node->SetLoopFlag( false );
				node->Remove();
			}
			delete p;
		}
		mLoopEffect.Clear();
	}
}

bool cTotem::Init( unsigned long totemClassIdx, NiPoint3 pos, float dir )
{
	sTotemScript* pInfo = TOTEMSCRIPT->GetTotemInfo( totemClassIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	/// Effect °´Ã¼ »ý¼º
	char pathName[256] = {0,};
	::sprintf_s( pathName, "./Data/Effect/%s", pInfo->mFileName.Cstr() );

	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, dir );

	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = pos;
	trans.m_Rotate = rot;

	cEffectSceneNode* node = SCENEMAN->CreateSelfEffect( pathName, true, trans, true );
	if( node == 0 )
	{
		assert(0);
		return false;
	}

	AddLoopEffect( node );
	return true;
}

//void cTotem::Update( unsigned long deltaTime, unsigned long accumTime)
//{
//	deltaTime;
//	accumTime;
//
//}

void cTotem::AddLoopEffect( cEffectSceneNode* pNode )
{
	if( pNode == 0 )
	{
		assert( pNode );
		return;
	}

	sLoopEffect* p = new sLoopEffect;
	p->mIdxByManager = pNode->GetIndexByManger();
	p->mEffectNode = pNode;

	mLoopEffect.PushBack( p );
}
