#include "stdafx.h"
#include "Pattern.h"
#include "NaviMesh.h"
#include "RegenToolView.h"
#include "PathFinder.h"
#include "../mapeditor/PatternSceneNode.h"
#include "SceneManager.h"

cPattern::cPattern( unsigned long patternIdx )
{
	mPatternIdx = patternIdx;
	mType = false;
	mPathCount = 0;

	mColor = NiColor( (float(rand()%256))/255.0f,(float(rand()%256))/255.0f,(float(rand()%256))/255.0f );

	/// 경로 만들기
	mPathLine = NiNew NiMesh;
	mPathLine->SetPrimitiveType( NiPrimitiveType::PRIMITIVE_LINES );

	mPathLine->AddStream( NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, MAX_PATH_COUNT,
						  NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ, 
						  NiDataStream::USAGE_VERTEX );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::BLACK );
	mMatProp->SetDiffuseColor( NiColor::BLACK );
	mMatProp->SetSpecularColor( NiColor::BLACK );
	mMatProp->SetEmittance( NiColor( 1.0f, 0.0f, 0.0f ) );

	mVertColorProp = NiNew NiVertexColorProperty;
	mVertColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
	mVertColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E_A_D );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mPathLine->AttachProperty( mMatProp );
	mPathLine->AttachProperty( mVertColorProp );
	mPathLine->AttachProperty( mWireProp );

	mPathLine->UpdateProperties();
	mPathLine->Update( 0.0f );
}

cPattern::~cPattern()
{
	Clear();
}

void cPattern::Clear()
{
	///
	if( mSceneNodeList.GetSize() > 0 )
	{
		cSceneNodeList::cIterator i = mSceneNodeList.Begin();
		cSceneNodeList::cIterator e = mSceneNodeList.End();

		for( ; i != e; ++i )
		{
			cPatternSceneNode* n = (cPatternSceneNode*)(*i);

			sPatternPos* pos = (sPatternPos*)n->GetUserData();
			if( pos )
			{
				delete pos;
				pos = 0;
			}

			SCENEMAN->DestroyNode( n );
		}

		mSceneNodeList.Clear();
	}
}

void cPattern::ClearSceneList()
{
	if( mSceneNodeList.GetSize() > 0 )
		mSceneNodeList.Clear();
}

bool cPattern::SavePattern( cFileSaver& saver )
{
	if( mSceneNodeList.IsEmpty() )
		return false;

	/// 인덱스
	cString str;
	str.Format( "%d\t%d\r\n{\r\n", mPatternIdx, mType );
	saver.Write( str.Cstr(), str.GetSize() );

	/// 좌표 정보
	cSceneNodeList::cIterator i = mSceneNodeList.Begin();
	cSceneNodeList::cIterator e = mSceneNodeList.End();

	for( ; i != e; ++i )
	{
		cPatternSceneNode* n = (cPatternSceneNode*)(*i);
		sPatternPos* pos = (sPatternPos*)n->GetUserData();
		if( pos == 0 )
		{
			assert(0 && "save error");
			break;
		}

		if( pos->mRelax == true )
			str.Format( "%d\t%d\t%d\t%d\t%d\r\n", pos->mX, pos->mY, pos->mRange, pos->mRelax, pos->mWaitTime );
		else
			str.Format( "%d\t%d\t%d\t%d\t%d\r\n", pos->mX, pos->mY, pos->mRange, pos->mRelax, 0 );

		saver.Write( str.Cstr(), str.GetSize() );
	}

	saver.Write( "}\r\n", 3 );
	return true;
}

void cPattern::AddNode( cPatternSceneNode* node )
{
	mSceneNodeList.PushBack( node );
}

void cPattern::InsertNode( cPatternSceneNode* baseNode, cPatternSceneNode* node )
{
	cSceneNodeList::cIterator i = mSceneNodeList.Begin();
	cSceneNodeList::cIterator end = mSceneNodeList.End();
	for( ; i != end; ++i )
	{
		cPatternSceneNode* p = (cPatternSceneNode*)(*i);
		if( p == baseNode )
			break;
	}

	mSceneNodeList.Insert( i, node );
}

void cPattern::RemoveNode( cPatternSceneNode* node )
{
	sPatternPos* pos = (sPatternPos*)node->GetUserData();
	if( pos )
	{
		delete pos;
		pos = 0;
	}

	mSceneNodeList.Remove( node );
}

void cPattern::RenderPattern()
{
	if( mSceneNodeList.GetSize() > 1 )
	{
		mMatProp->SetEmittance( NiColor( 1.0f, 0.0f, 0.0f ) );
		mPathLine->UpdateProperties();

		mPathLine->RenderImmediate( NiRenderer::GetRenderer() );
	}
}

void cPattern::UpdateAllPathLine()
{
	SetActiveLine( 0 );

	if( mSceneNodeList.GetSize() == 0 )
		return;

	cPathFinder* pathFinder = VIEW->GetPathFinder();
	if( !pathFinder )
	{
		assert(0);
		return;
	}

	NiPoint3 temp[MAX_PATH_COUNT];
	unsigned int cnt = 0;

	cSceneNodeList::cIterator b = mSceneNodeList.Begin();
	cSceneNodeList::cIterator end = mSceneNodeList.End();
	
	for( ; b != end; ++b, ++cnt )
	{
		cPatternSceneNode* n = (cPatternSceneNode*)(*b);
		sPatternPos* pos = (sPatternPos*)n->GetUserData();
		if( pos )
		{
			temp[cnt].x = (float)pos->mX;
			temp[cnt].y = (float)pos->mY;
			
			NAVIMESH->CalcHeight( &(temp[cnt].z), temp[cnt].x, temp[cnt].y );
			temp[cnt].z += 50.0f;
		}
	}

	/// 경로 설정
	NiDataStreamElementLock posLock( mPathLine, NiCommonSemantics::POSITION(), 0, 
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( posLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> posIter = posLock.begin<NiPoint3>();

	int count = 0;

	posIter[0] = temp[0];
	
	for( unsigned int c = 0; c < cnt; ++c )
	{
		if( c + 1 == cnt )
			break;

		if( pathFinder->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, NiPoint2(temp[c].x, temp[c].y), NiPoint2(temp[c+1].x, temp[c+1].y) ) )
		{
			NiPoint3 p;
			unsigned int i = 0;		
			unsigned int iend = mPathCount < MAX_PATH_COUNT - 1 ? mPathCount : MAX_PATH_COUNT - 1;

			for( ; i < iend; ++i )
			{
				p.x = mPathArray[i].x;
				p.y = mPathArray[i].y;

				NAVIMESH->CalcHeight( &(p.z), p.x, p.y );
				p.z += 50.0f;
				//posIter[2*i + (2*c) + 1] = posIter[2*i+ (2*c) + 2] = p;
				posIter[2*count+1] = posIter[2*count+2] = p;

				count++;
			}  
		}
	}

	posLock.Unlock();

	SetActiveLine( count * 2 );
	//SetActiveLine(2*cnt+(2*mPathCount)-1);
}

void cPattern::SetActiveLine( unsigned int count )
{
	if( mPathLine )
	{
		NiDataStreamRef* ref = mPathLine->FindStreamRef( NiCommonSemantics::POSITION() );
		NIASSERT( ref );

		if( ref )
			ref->SetActiveCount( 0, count );
	}
}

void cPattern::SetVisible( bool visible )
{
	cSceneNodeList::cIterator i = mSceneNodeList.Begin();
	cSceneNodeList::cIterator e = mSceneNodeList.End();

	for( ; i != e; ++i )
	{
		cPatternSceneNode* n = (cPatternSceneNode*)(*i);
		if( n )
		{
			n->SetViewNode( visible );
		}
	}

	if( visible == false )
	{
		SetActiveLine( 0 );
	}
}