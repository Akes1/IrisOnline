#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "Mapdialog.h"
#include "NpcDialog.h"
#include "MonsterDialog.h"
#include "PortalDialog.h"
#include "GatheringDialog.h"
#include "PatternDialog.h"
#include "DoorDialog.h"

#include "Terrain.h"
#include "NaviMesh.h"
#include "NaviField.h"
#include "SceneManager.h"
#include "MonsterSceneNode.h"
#include "GatheringSceneNode.h"
#include "PathFinder.h"

#include "Application.h"

#define NPC_REGEN		"Script/Resource/NpcRegen"
#define MONSTER_REGEN	"Script/Resource/MonsterRegen"
#define GATHERING_REGEN	"script/resource/gatheringregen"
#define PATTERN_REGEN	"Script/Resource/MonsterPattern"

const char* MAP_INFO_FILE_TXT = "Script/Resource/mapdata.txt";
//const char* MAP_NAME_LIST_TXT = "Script/Language/Map_Name.txt";
const char* MDL_FILE_LIST_TXT = "Script/Resource/ModelFileList.txt";
//const char* NPC_NAME_LIST_TXT = "Script/Language/Npc_Name.txt";
const char* NPC_LIST_TXT = "Script/Resource/NpcList.txt";
//const char* MONSTER_NAME_LIST_TXT = "Script/Language/Mon_Name.txt";
const char* MONSTER_LIST_TXT = "Script/Resource/MonsterList.txt";
//const char* GATHERING_NAME_LIST_TXT = "Script/Language/gatheringname.txt";
const char* GATHERING_LIST_TXT = "Script/Resource/gatheringlist.txt";
const char* PORTAL_LIST_TXT = "Script/Resource/MapChangePos.txt";
const char* DOOR_LIST_TXT = "Script/Resource/DoorPoslist.txt";

bool cRegenToolView::LoadMapInfo()
{
	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	cFileLoader loader;
	if( loader.Open( MAP_INFO_FILE_TXT, true ) == false )
	{
		MessageBox( "Failed to open map Info file.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", MAP_INFO_FILE_TXT );

	unsigned int mapIdx;
	unsigned int mapfolderIdx;
	unsigned int mapDegree;

	cString str;
	while( tok.IsEnd() == false )
	{
		if( tok.GetNext( &str ) == false )	{ return false; }
		mapIdx = str.ToInt();

		/// skip data
		if( tok.GetNext( &str ) == false )	{ return false; }
		mapfolderIdx = str.ToInt();

		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }

		if( tok.GetNext( &str ) == false )	{ return false; }
		mapDegree = str.ToInt();

		/// skip data
		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }
		if( tok.GetNext( &str ) == false )	{ return false; }

		cMapInfoMap::cIterator i = mMapInfoMap.Find( mapIdx );
		if( i != mMapInfoMap.End() )
		{
			sMapInfo* info = (sMapInfo*)i->mSecond;

			if( mapDegree != 0 )
				info->degreeInfo.PushBack( mapDegree );
		}
		else
		{
			sMapInfo* info = new sMapInfo;
			info->mapIdx = mapIdx;
			info->mapfolerIdx = mapfolderIdx;

			if( mapDegree != 0 )
				info->degreeInfo.PushBack( mapDegree );

			mMapInfoMap.Insert( mapIdx, info );
		}
	}

	return true;
}

bool cRegenToolView::LoadMapNameList()
{
	mNpcNameMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	cString path;
	path.Format("./language/%s/Map_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( loader.Open( path.Cstr(), true ) == false )
	{
		MessageBox( "Failed to open npc name list.", "Init" );
		return false;
	}

	/// 파싱
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, path.Cstr() );
	unsigned int index;
	cString str;

	while( lexer.IsEnd() == false )
	{
		/// Npc 인덱스
		index = parser.ParseInt();

		/// Npc 이름
		str = parser.ParseString();

		if( str.IsEmpty() )
			continue;

		///
		cMapNameMap::cIterator i = mMapNameMap.Find( index );

		if( i != mMapNameMap.End() )
			continue;

		mMapNameMap.Insert( index, str );
	}
	return true;
}

bool cRegenToolView::LoadModelFileList()
{
	mModelFileMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( MDL_FILE_LIST_TXT, true ) == false )
	{
		MessageBox( "Failed to open model file list.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", MDL_FILE_LIST_TXT );
	unsigned int index;
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 모델 인덱스
		tok.GetNext( &str );
		index = str.ToInt();

		/// 모델 파일
		str.Clear();
		tok.GetNext( &str );

		if( str.IsEmpty() )
			continue;

		///
		cModelFileMap::cIterator i = mModelFileMap.Find( index );

		if( i != mModelFileMap.End() )
			continue;

		mModelFileMap.Insert( index, str );
	}
	return true;
}

bool cRegenToolView::LoadNpcNameList()
{
	mNpcNameMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	cString path;
	path.Format("./language/%s/Npc_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( loader.Open( path.Cstr(), true ) == false )
	{
		MessageBox( "Failed to open npc name list.", "Init" );
		return false;
	}

	/// 파싱
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, path.Cstr() );
	unsigned int index;
	cString str;

	while( lexer.IsEnd() == false )
	{
		/// Npc 인덱스
		index = parser.ParseInt();

		/// Npc 이름
		str = parser.ParseString();

		if( str.IsEmpty() )
			continue;

		///
		cNpcNameMap::cIterator i = mNpcNameMap.Find( index );
	
		if( i != mNpcNameMap.End() )
			continue;

		mNpcNameMap.Insert( index, str );
	}
	return true;
}

bool cRegenToolView::LoadNpcList()
{
	mNpcInfoMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( NPC_LIST_TXT, true ) == false )
	{
		MessageBox( "Failed to open npc list.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", NPC_LIST_TXT );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int index = str.ToInt();

		/// 이름 인덱스
		tok.GetNext( &str );
		unsigned int nameIndex = str.ToInt();
		cNpcNameMap::cIterator ni = mNpcNameMap.Find( nameIndex );

		if( ni == mNpcNameMap.End() )
		{
			MessageBox( "Can't find npc name index.", "Npc List" );
			return false;
		}

		// 타입
		tok.GetNext( &str );
		unsigned int type = str.ToInt();
		type;
		
		/// 직업 인덱스
		tok.GetNext( &str );
		unsigned int jobIndex = str.ToInt();
		jobIndex;

		/// 모델 인덱스
		tok.GetNext( &str );
		unsigned int modelIndex = str.ToInt();
		cModelFileMap::cIterator mi = mModelFileMap.Find( modelIndex );
		
		
		/// 큰지도아이콘이미지인덱스
		tok.GetNext( &str );
		
		/// 미니맵아이콘인덱스
		tok.GetNext( &str );

		///icon_id
		tok.GetNext( &str );


		if( mi == mModelFileMap.End() )
		{
			MessageBox( "Can't find model index.", "Npc List" );
			return false;
		}

		/// 줄넘김
		tok.GotoNextLine();

		/// 정보 맵에 추가
		mNpcInfoMap.Insert( index, new cNpcInfo(nameIndex, modelIndex) );
	}

	assert( mNpcDialog );
	mNpcDialog->UpdateCombo( mNpcInfoMap );
	return true;
}

bool cRegenToolView::LoadMonsterNameList()
{
	mMonsterNameMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	cString path;
	path.Format("./language/%s/Mon_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( loader.Open( path.Cstr(), true ) == false )
	{
		MessageBox( "Failed to open monster name list.", "Init" );
		return false;
	}

	/// 파싱
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, path.Cstr() );
	unsigned int index;
	cString str;

	while( lexer.IsEnd() == false )
	{
		/// 몬스터 인덱스
		index = parser.ParseInt();

		/// 몬스터 이름
		str = parser.ParseString();

		if( str.IsEmpty() )
			continue;

		///
		cMonsterNameMap::cIterator i = mMonsterNameMap.Find( index );

		if( i != mMonsterNameMap.End() )
			continue;

		mMonsterNameMap.Insert( index, str );
	}
	return true;
}

bool cRegenToolView::LoadMonsterList()
{
	mMonsterInfoMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( MONSTER_LIST_TXT, true ) == false )
	{
		MessageBox( "Failed to open monster list.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", MONSTER_LIST_TXT );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int index = str.ToInt();

		/// 이름 인덱스
		tok.GetNext( &str );
		unsigned int nameIndex = str.ToInt();
		cMonsterNameMap::cIterator ni = mMonsterNameMap.Find( nameIndex );

		if( ni == mMonsterNameMap.End() )
		{
			MessageBox( "Can't find monster name index.", "Monster List" );
			return false;
		}

		/// 직업 인덱스
		tok.GetNext( &str );
		
		/// 모델 인덱스
		tok.GetNext( &str );
		unsigned int modelIndex = str.ToInt();
		cModelFileMap::cIterator mi = mModelFileMap.Find( modelIndex );

		if( mi == mModelFileMap.End() )
		{
			MessageBox( "Can't find model index.", "Monster List" );
			return false;
		}

		/// 줄넘김
		tok.GotoNextLine();

		/// 정보 맵에 추가
		mMonsterInfoMap.Insert( index, new cMonsterInfo(nameIndex, modelIndex) );
	}

	assert( mMonsterDialog );
	mMonsterDialog->UpdateCombo( mMonsterInfoMap );
	return true;
}

bool cRegenToolView::LoadGatheringNameList()
{
	mGatheringNameMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	cString path;
	path.Format("./language/%s/gatheringname_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( loader.Open( path.Cstr(), true ) == false )
	{
		MessageBox( "Failed to open gathering name list.", "Init" );
		return false;
	}

	/// 파싱
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, path.Cstr() );
	unsigned int index;
	cString str;

	while( lexer.IsEnd() == false )
	{
		/// 채집 객체 인덱스
		index = parser.ParseInt();

		/// 채집 객체 이름
		str = parser.ParseString();

		if( str.IsEmpty() )
			continue;

		///
		cGatheringNameMap::cIterator i = mGatheringNameMap.Find( index );

		if( i != mGatheringNameMap.End() )
			continue;

		mGatheringNameMap.Insert( index, str );
	}

	return true;
}

bool cRegenToolView::LoadGatheringList()
{
	mGatheringInfoMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( GATHERING_LIST_TXT, true ) == false )
	{
		MessageBox( "Failed to open gathering list.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", GATHERING_LIST_TXT );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int index = str.ToInt();

		/// 이름 인덱스
		tok.GetNext( &str );
		unsigned int nameIndex = str.ToInt();
		cMonsterNameMap::cIterator ni = mGatheringNameMap.Find( nameIndex );

		if( ni == mGatheringNameMap.End() )
		{
			MessageBox( "Can't find gathering name index.", "Gathering List" );
			return false;
		}

		/// 
		tok.GetNext( &str );

		/// 모델 인덱스
		tok.GetNext( &str );
		cString fileName = str;

		/// 줄넘김
		tok.GotoNextLine();

		/// 정보 맵에 추가
		mGatheringInfoMap.Insert( index, new cGatheringInfo(nameIndex, fileName) );
	}

	assert( mGatheringDialog );
	mGatheringDialog->UpdateCombo( mGatheringInfoMap );
	return true;
}

bool cRegenToolView::LoadMap( unsigned int mapIndex, unsigned int mapfolderIdx, unsigned int mapDegree )
{
	///
	Clear();
	mNpcDialog->EnableWindow( 0 );
	mMonsterDialog->EnableWindow( 0 );
	mPortalDialog->EnableWindow( 0 );
	mGatheringDialog->EnableWindow( 0 );
	mPatternDialog->EnableWindow( 0 );
	mDoorDialog->EnableWindow( 0 );

	/// 현재 경로를 설정
//	cString path;
//	path.Format( "%sMap/Map%02d/", theApp.GetBaseDir().Cstr(), mapIndex );
//	SetCurrentDirectory( path.Cstr() );

	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 지형을 로딩
	cString pathName;
	pathName.Format( "./data/map/%03d.terrain", mapfolderIdx );

	if( TERRAIN->Load( pathName ) == false )
		return false;

	/// 네비메시를 로딩
	pathName.Format( "./data/map/%03d.navimesh", mapfolderIdx );

	if( mNaviMesh->Load( pathName ) == false )
		return false;

	/// 네비필드를 로딩
	pathName.Format( "./data/map/%03d.navifield", mapfolderIdx );

	if( mNaviField->Load( pathName ) == false )
		return false;

	mPathFinder->Init( mNaviField );

	/// 장면을 로딩
	pathName.Format( "./data/map/%03d.scene", mapfolderIdx );

	if( SCENEMAN->Load( pathName ) == false )
		return false;

	/// 안개를 적용
	TERRAIN->SetFog( true, NiColor::WHITE, 0.1f );
	SCENEMAN->SetFog( true, NiColor::WHITE, 0.1f );

	/// 줌인
	ZoomPersp();

	///
	mNpcDialog->EnableWindow( 1 );
	mMonsterDialog->EnableWindow( 1 );
	mPortalDialog->EnableWindow( 1 );
	mGatheringDialog->EnableWindow( 1 );
	mPatternDialog->EnableWindow( 1 );
	mDoorDialog->EnableWindow( 1 );

	///
	{
		/// 현재 경로를 설정
//		cString path;
//		path.Format( "%sData/Monster/", theApp.GetBaseDir().Cstr() );
//		SetCurrentDirectory( path.Cstr() );

		/// 인덱스 검사
		cMonsterInfoMap::cIterator ii = mMonsterInfoMap.Find( 1 );

		if( ii == mMonsterInfoMap.End() )
			return true;

		/// 모델 파일 검사
		cMonsterInfo* info = ii->mSecond;
		cModelFileMap::cIterator mi = mModelFileMap.Find( info->mModelIndex );

		SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

		/// 장면 노드를 생성
		cMonsterSceneNodeParam param;

		cString str = mi->mSecond;
		cString pathName;
		pathName.Format( "./Data/Monster/%s", str.Cstr() );
		param.mPathName = pathName;

		cMonsterSceneNode* n = SCENEMAN->CreateMonster( param );
		if( n == 0 )
			return true;

		n->SetTargetAnimation( 1 );
		mDummyNode = n;
	}

	/// 리젠 정보를 로딩
	mMapIndex = mapIndex;
	mMapDegree = mapDegree;

	LoadNpcRegen( mapIndex, mapDegree );
	LoadMonsterRegen( mapIndex, mapDegree );
	LoadPortalRegen();
	LoadGatheringRegen( mapIndex, mapDegree );
	LoadPattern( mapIndex, mapDegree );
	LoadDoorRegen();

	return true;
}

bool cRegenToolView::LoadNpcRegen( unsigned int mapIndex, unsigned int mapDegree )
{
	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cString pathName;
	pathName.Format( NPC_REGEN "%d_%d.txt", mapIndex, mapDegree );
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
//		MessageBox( "Failed to open npc regen.", "Npc Regen" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int npcIndex = str.ToInt();

		/// x 좌표
		tok.GetNext( &str );
		float x = str.ToFloat();

		/// y 좌표
		tok.GetNext( &str );
		float y = str.ToFloat();

		/// 회전
		tok.GetNext( &str );
		float zdegree = str.ToFloat();

		/// Npc를 생성
		CreateNpc( npcIndex, x, y, zdegree, false, false );
	}

	assert( mNpcDialog );
	mNpcDialog->UpdateList( mNpcNodeMap );
	return true;
}

bool cRegenToolView::LoadMonsterRegen( unsigned int mapIndex, unsigned int mapDegree )
{
	mMonsterDialog->Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cString pathName;
	pathName.Format( MONSTER_REGEN "%d_%d.txt", mapIndex, mapDegree );
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
//		MessageBox( "Failed to open monster regen.", "Monster Regen" );
		return false;
	}
 
	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 몬스터 그룹 인덱스
		tok.GetNext( &str );
		unsigned int groupIndex = str.ToInt();

		/// {
		tok.GetNext( &str );
		if( str != '{' )
			return false;

		/// 몬스터 그룹을 생성
		cMonsterGroup* group = new cMonsterGroup( groupIndex );

		if( mMonsterGroupMap.Insert( groupIndex, group ) == false )
		{
			MessageBox( "Monster group index is duplicated.", "Monster Regen" );
			delete group;
			return false;
		}

		/// 최초 리젠 여부
		tok.GetNext( &str );
		group->mFirstRegen = (str.ToInt() != 0)? true : false;

		/// 리젠 퍼센트
		tok.GetNext( &str );
		group->mRegenPercent = str.ToInt();

		/// 리젠 범위
		tok.GetNext( &str );
		group->mRegenRange = str.ToInt();

		/// 리젠 대기 시간
		tok.GetNext( &str );
		group->mRegenWaitTime = str.ToInt();

		///
		tok.GetNext( &str );
		group->mMapGroupIndex = str.ToInt();

		///
		tok.GetNext( &str );
		group->mChannelCheck = (str.ToInt() != 0)? true : false;

		/// 리젠되는 그룹 인덱스
		tok.GetNext( &str );
		unsigned int regenCount = str.ToInt();

		if( regenCount != 0 )
		{
			for( unsigned int i=0;i<regenCount; ++i )
			{
				tok.GetNext( &str );
				unsigned int regenIdx = str.ToInt();

				if( regenIdx != 0 )
					group->mNextRegenGroupMap.Insert( regenIdx, regenIdx );
			}
		}

		while( tok.IsEnd() == false )
		{
			/// }
			tok.GetNext( &str );
			if( str == '}' )
				break;

			/// 몬스터 인덱스
			unsigned int monIndex = str.ToInt();

			/// 몬스터 리젠 좌표
			tok.GetNext( &str );
			unsigned int x = str.ToInt();

			tok.GetNext( &str );
			unsigned int y = str.ToInt();

			/// 몬스터 회전
			tok.GetNext( &str );
			int zdegree = str.ToInt();

			/// 몬스터 리젠 대기 시간
			tok.GetNext( &str );
			unsigned int regenWaitTime = str.ToInt();

			/// 몬스터 리젠 유지 시간
			tok.GetNext( &str );
			unsigned int regenLifeTime = str.ToInt();

			/// 몬스터 동족그룹 번호
			tok.GetNext( &str );
			unsigned int regenFamilyNum = str.ToInt();

			/// 패턴 idx
			tok.GetNext( &str );
			unsigned int regenPatternIdx = str.ToInt();

			/// 몬스터를 생성
			mSelMonsterGroup = group;
			CreateMonster( monIndex, x, y, zdegree, regenWaitTime, regenLifeTime, false, false, regenFamilyNum, regenPatternIdx );

			cHashSet* pSet = NULL;
			cMonsterFamilyMap::cIterator pI = mMonsterFamilyMap.Find( regenFamilyNum );
			cMonsterFamilyMap::cIterator pE = mMonsterFamilyMap.End();

			if( pI == pE )
			{
				pSet = new cHashSet;
				if( mMonsterFamilyMap.Insert( regenFamilyNum, pSet ) == false )
				{
					MessageBox( "Monster Pattern index is duplicated.", "Monster" );
					return false;
				}
			}
			else
			{
				pSet = (cHashSet*)(*pI).mSecond;
			}

			if( pSet != NULL )
				pSet->Insert( regenFamilyNum );
		}
	}

	///
	if( mMonsterGroupMap.IsEmpty() == false )
	{
		mMonsterDialog->UpdateGroupList( mMonsterGroupMap );
		mMonsterDialog->UpdateFamilyList( &mMonsterFamilyMap );
	}
	SelectMonsterGroup( 0 );
	return true;
}

bool cRegenToolView::LoadPortalRegen()
{
	mPortalInfoMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( PORTAL_LIST_TXT, true ) == false )
	{
//		MessageBox( "Failed to open portal list.", "Init" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", PORTAL_LIST_TXT );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int index = str.ToInt();

		/// 
		tok.GetNext( &str );
		cString potalNif = str;

		/// 소속맵 인덱스
		tok.GetNext( &str );
		unsigned int mapIndex = str.ToInt();

		/// 좌표
		tok.GetNext( &str );
		float x = str.ToFloat();

		tok.GetNext( &str );
		float y = str.ToFloat();

		/// 범위
		tok.GetNext( &str );
		unsigned int range = str.ToInt();

		/// 대상 포탈 인덱스
		tok.GetNext( &str );
		unsigned int targetIndex = str.ToInt();

		/// 회전값
		tok.GetNext( &str );
		int zdegree = str.ToInt();

		/// 자동생성 유무
		tok.GetNext( &str );
		bool isCreated = str.ToInt() == 1;

		/// 포탈명 출력 값 ( y 위치 )
		tok.GetNext( &str );
		float nameheight = str.ToFloat();;

		/// 포탈을 생성
		if( mapIndex == mMapIndex )
		{
			CreatePortal( index, potalNif.Cstr(), (float)x, (float)y, range, targetIndex, zdegree, nameheight, isCreated, false, false );
		}
		else
		{
			mPortalInfoMap.Insert( index, new cPortalInfo( potalNif.Cstr(), mapIndex, x, y, range, targetIndex, zdegree, nameheight, isCreated, 0) );
		}
	}

	assert( mPortalDialog );
	mPortalDialog->UpdateList( mPortalInfoMap );
	return true;
}

bool cRegenToolView::LoadGatheringRegen( unsigned int mapIndex, unsigned int mapDegree )
{
	mGatheringDialog->Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cString pathName;
	pathName.Format( GATHERING_REGEN "%d_%d.txt", mapIndex, mapDegree );
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
//		MessageBox( "Failed to open gathering regen.", "Gathering Regen" );
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 채집 그룹 인덱스
		tok.GetNext( &str );
		unsigned int groupIndex = str.ToInt();
		tok.GetNext( &str );
		unsigned int regenWaitTime = str.ToInt();
		tok.GetNext( &str );
		unsigned int regenRandTime = str.ToInt();

		/// {
		tok.GetNext( &str );
		if( str != '{' )
			return false;

		/// 채집 그룹을 생성
		cGatheringGroup* group = new cGatheringGroup( groupIndex, regenWaitTime, regenRandTime );

		if( mGatheringGroupMap.Insert( groupIndex, group ) == false )
		{
			MessageBox( "Gatherint group index is duplicated.", "Gathering Regen" );
			delete group;
			return false;
		}

		while( tok.IsEnd() == false )
		{
			/// }
			tok.GetNext( &str );
			if( str == '}' )
				break;

			/// 채집 객체 인덱스
			unsigned int gatheringIndex = str.ToInt();

			/// 채집 객체 리젠 좌표
			tok.GetNext( &str );
			unsigned int x = str.ToInt();

			tok.GetNext( &str );
			unsigned int y = str.ToInt();

			/// 채집 객체 회전
			tok.GetNext( &str );
			int zdegree = str.ToInt();

			/// 채집 객체 추가 Z값
			tok.GetNext( &str );
			float appendZ = str.ToFloat();


			/// 채집객체를 생성
			mSelGatheringGroup = group;
			CreateGathering( gatheringIndex, x, y, zdegree, appendZ, false, false );
		}
	}

	///
	if( mGatheringGroupMap.IsEmpty() == false )
	{
		mGatheringDialog->UpdateGroupList( mGatheringGroupMap );
	}
	SelectGatheringGroup( 0 );
	return true;
}


bool cRegenToolView::LoadPattern( unsigned int mapIndex, unsigned int mapDegree )
{
	mPatternDialog->Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기

	cString pathName;
	pathName.Format( PATTERN_REGEN "%d_%d.txt", mapIndex, mapDegree );

	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;
	float z = 0;

	while( tok.IsEnd() == false )
	{
		/// 채집 그룹 인덱스
		tok.GetNext( &str );
		unsigned int patternIdx = str.ToInt();
		tok.GetNext( &str );
		bool type = str.ToInt() == 1 ? true : false;

		/// {
		tok.GetNext( &str );
		if( str != '{' )
			return false;

		cPattern* pPattern = new cPattern( patternIdx );
		pPattern->SetPatternInfo( type );

		if( mPatternMap.Insert( patternIdx, pPattern ) == false )
		{
			MessageBox( "Pattern index is duplicated.", "Pattern" );
			delete pPattern;
			return false;
		}

		tok.GetNext( &str );
		while( tok.IsEnd() == false )
		{
			int x = str.ToInt();

			tok.GetNext( &str );
			int y = str.ToInt();

			tok.GetNext( &str );
			int range = str.ToInt();

			/// 경유지설정
			tok.GetNext( &str );
			bool relaxPoint = str.ToInt() == 1 ? true : false;

			/// 대기시간
			tok.GetNext( &str );
			unsigned long waitTime = str.ToInt();

			AddPatternPos( patternIdx, NiPoint3((float)x, (float)y, (float)z), range, relaxPoint, waitTime );

			tok.GetNext( &str );
			if( str == "}" )
				break;
		}
	}

	///
	if( mPatternMap.IsEmpty() == false )
	{
		mPatternDialog->UpdatePatternList( &mPatternMap );
		mMonsterDialog->UpdatePatternCombo( &mPatternMap );
	}
	///SelectGatheringGroup( 0 );
	return true;
}

bool cRegenToolView::LoadDoorRegen()
{
	mDoorInfoMap.Clear();

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( DOOR_LIST_TXT, true ) == false )
	{
		return false;
	}

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", DOOR_LIST_TXT );
	cString str;

	while( tok.IsEnd() == false )
	{
		/// 인덱스
		tok.GetNext( &str );
		unsigned int index = str.ToInt();

		/// 소속맵 인덱스
		tok.GetNext( &str );
		unsigned int mapIndex = str.ToInt();

		/// 
		tok.GetNext( &str );
		cString doorNif = str;

		/// 좌표
		tok.GetNext( &str );
		float x = str.ToFloat();

		tok.GetNext( &str );
		float y = str.ToFloat();

		tok.GetNext( &str );
		float appendZ = str.ToFloat();

		/// 회전값
		tok.GetNext( &str );
		int zdegree = str.ToInt();

		tok.GetNext( &str );
		float rectAppendY = str.ToFloat();

		NiPoint3 rectPos[4];
		for( unsigned int i=0; i<4; ++i )
		{
			tok.GetNext( &str );
			rectPos[i].x = str.ToFloat();
			tok.GetNext( &str );
			rectPos[i].y = str.ToFloat();
			rectPos[i].z = 0.0f;
		}

		/// 포탈을 생성
		if( mapIndex == mMapIndex )
		{
			float width[2];
			float height[2];

			width[0] = (float)(unsigned int)(rectPos[0]-rectPos[1]).Length();
			width[1] = (float)(unsigned int)(rectPos[2]-rectPos[3]).Length();

			height[0] = (float)(unsigned int)(rectPos[0]-rectPos[3]).Length();
			height[1] = (float)(unsigned int)(rectPos[1]-rectPos[2]).Length();

			CreateDoor( index, doorNif.Cstr(), NiPoint2(x, y), appendZ, zdegree, width, height, rectAppendY, false, false );
		}
		else
		{
			cDoorInfo* info = new cDoorInfo();
			info->mFileName = doorNif.Cstr();
			info->mPos = NiPoint3(x,y,0.0f);
			info->mAppendZ = appendZ;
			info->mZDegree = zdegree;

			info->mMapIndex = mapIndex;
			for( unsigned int i=0; i<4; ++i )
				info->mRectPos[i] = rectPos[i];

			info->mRectAppendY = rectAppendY;

			mDoorInfoMap.Insert( index, info );
		}
	}

	assert( mDoorDialog );
	mDoorDialog->UpdateList( mDoorInfoMap );

	return true;
}
