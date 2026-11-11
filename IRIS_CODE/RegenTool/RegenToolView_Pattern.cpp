#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "PatternDialog.h"
#include "MonsterDialog.h"
#include "NaviMesh.h"
#include "../mapeditor/PatternSceneNode.h"
#include "SceneManager.h"
#include "FreeCamera.h"

int cRegenToolView::CreatePattern( unsigned long patternIdx )
{
	cPatternMap::cIterator i = mPatternMap.Find( patternIdx );

	if( i != mPatternMap.End() )
		return -1;

	cPattern* pPattern = new cPattern( patternIdx );
	if( mPatternMap.Insert( patternIdx, pPattern ) == false )
		return 0;

	mPatternDialog->UpdatePatternList( &mPatternMap );

	SelectPattern( patternIdx );
	return 1;
}

int cRegenToolView::DeletePattern( unsigned long patternIdx )
{
	cPatternMap::cIterator i = mPatternMap.Find( patternIdx );
	if( i != mPatternMap.End() )
	{
		cPattern* pattern = (cPattern*)(i->mSecond);
		pattern->Clear();

		if( pattern->mPatternIdx == patternIdx )
			mSelPattern = 0;

		mPatternMap.Erase( patternIdx );
		delete pattern;
		pattern = 0;

		mPatternDialog->UpdatePatternList( &mPatternMap );
		mPatternDialog->UpdatePosList( 0 );
	}

	SelectNode(0);
	return 1;
}

void cRegenToolView::SelectPattern( unsigned int patternIdx )
{
	SelectPatternPos( 0 );

	cPatternMap::cIterator i = mPatternMap.Find( patternIdx );
	if( i != mPatternMap.End() )
	{
		cPattern* pattern = (cPattern*)(i->mSecond);

		mSelPattern = pattern;

		// 새 패턴
		if( mSelPattern )
		{
			mPatternDialog->SelectPattern( mSelPattern );

			mSelPattern->SetVisible( true );
			mSelPattern->UpdateAllPathLine();

			if( mSelPattern->mSceneNodeList.IsEmpty() == false )
				SelectPatternPos( *(mSelPattern->mSceneNodeList.Begin()) );
		}
	}

	Update();
}

cPatternSceneNode* cRegenToolView::AddPatternPos( unsigned long patternIdx, NiPoint3 pos, unsigned long range, bool relaxPoint, unsigned long waitTime )
{
	cPattern* pattern = mPatternMap.GetAt( patternIdx );
	if( pattern == 0 )
		return 0;

	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, (float)pos.x, (float)pos.y );

	/// scene data 생성
	cPatternSceneNodeParam	param;
	param.mPathName = "./Data/Effect/PatternSphere.nif";
	param.mTranslate = NiPoint3( (float)pos.x, (float)pos.y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(0.0f) );

	int size = pattern->mSceneNodeList.GetSize();

	cPatternSceneNode* n = 0;

	/// 위에 추가
	if( size > 0 && mPatternDialog->IsCheckedUpper() )
	{
		/// 선택된 열 위에 삽입
		cPatternSceneNode* node = mPatternDialog->GetSelectedPos();
		if( !node )
			return 0;

		n = SCENEMAN->CreatePattern( param );
		if( n == 0 )
			return 0;

		n->SetMaterialColor( pattern->mColor );

		pattern->InsertNode( node, n );
	}
	/// 기본 추가
	else
	{
		n = SCENEMAN->CreatePattern( param );
		if( n == 0 )
			return 0;

		n->SetMaterialColor( pattern->mColor );

		pattern->AddNode( n );
	}

	///
	sPatternPos* newpos = new sPatternPos;
	newpos->mX = (unsigned long)pos.x;
	newpos->mY = (unsigned long)pos.y;
	newpos->mRange = range;
	newpos->mRelax = relaxPoint;
	newpos->mWaitTime = waitTime;

	n->SetUserData( newpos );
	n->SetUserData0( patternIdx );

	n->SetWireframe( relaxPoint );

	pattern->UpdateAllPathLine();

	mPatternDialog->UpdatePosList( &pattern->mSceneNodeList );

	return n;
}

int cRegenToolView::DeletePatternPos( cPatternSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return 0;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::ModifyPatternPos( NiPoint3 pos )
{
	cPatternSceneNode* node = mPatternDialog->GetSelectedPos();
	if( node )
	{
		sPatternPos* p = (sPatternPos*)node->GetUserData();
		if( p )
		{
			p->mX = (unsigned long)pos.x;
			p->mY = (unsigned long)pos.y;

			mNaviMesh->CalcHeight( &pos.z, pos.x, pos.y );
			node->SetTranslate( pos );
			node->Update();

			if( mSelPattern )
			{
				mSelPattern->UpdateAllPathLine();
				mPatternDialog->UpdatePosList( &mSelPattern->mSceneNodeList );
			}

			SelectNode( node );
		}	
	}
}

void cRegenToolView::UpdatePatternCombo()
{ 
	mMonsterDialog->UpdatePatternCombo( &mPatternMap ); 
}

void cRegenToolView::SelectPatternPos( cPatternSceneNode* node )
{
	if( node )
	{
		NiPoint3 at = node->GetCenter();
		NiPoint3 dir = node->GetWorldRotate() * -NiPoint3::UNIT_Y;
		dir.z = 0.0f;
		dir.Unitize();
		NiPoint3 pos = at + dir * 1200.0f;
		pos.z += 700.0f;
		mCamera->SetTranslate( pos );
		mCamera->LookAt( at );
	}

	SelectNode( node );
}
