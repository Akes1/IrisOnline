#include "StdAfx.h"
#include "npcscript.h"

#include "NPC_Common.h"
#include "FileLoader.h"
#include "tokenizer.h"
#include "Stage_Common.h"
#include "BaseObject_Common.h" 

#include "Token.h"
#include "Parser.h"

#ifdef _CLIENT
#include "GameResourceManager.h"
#endif



cNPCScript* cNPCScript::mpNPCScript = NULL;



cNPCScript::cNPCScript()
{
	/// 싱글톤
	if( mpNPCScript )
	{
		assert(NULL);
	}
	else
	{
		mpNPCScript = this;
	}

	mpNPCListMap = new cPointHashMap( 999 );
	mpNPCRegenMap = new cPointHashMap( 999 );
	mpModelNumberMap = new cPointHashMap( 10 );
	mpNpcFuncMap = new cNpcFuncMap( 100 );
}



cNPCScript::~cNPCScript()
{
}

bool cNPCScript::Init()
{

	/// NPC 고유 리스트 로드
	if( !LoadNPCList() )
	{
		assert(NULL);
		return false;
	}

#ifndef _CLIENT

	if( LoadNPCStore() == false )
	{
		assert(NULL);
		return false;
	}

#endif

	return true;
}

bool cNPCScript::StageInit( unsigned short mapNumber, unsigned char levelMode )
{
	if( mapNumber == MAP_NONE )
	{
		cPointHashMap* pNPCRegenMap = new cPointHashMap;

		for( unsigned short mapNum = MAP_MIN ; mapNum <= MAP_MAX ; ++mapNum )
		{
			/// 맵에 위치한 NPC 로드
			if( !LoadNPCRegen( mapNum, levelMode, pNPCRegenMap ) )
			{
				assert(NULL);
				return false;
			}
		}

		mpNPCRegenMap->Insert( levelMode, pNPCRegenMap );
	}
	else
	{
		cPointHashMap* pNPCRegenMap = new cPointHashMap;

		/// 맵에 위치한 NPC 로드
		if( !LoadNPCRegen( mapNumber, levelMode, pNPCRegenMap ) )
		{
			assert(NULL);
			return false;
		}

		mpNPCRegenMap->Insert( levelMode, pNPCRegenMap );
	}

	return true;
}

void cNPCScript::Release()
{
	if( mpNpcFuncMap )
	{
		cNpcFuncMap::cIterator i = mpNpcFuncMap->Begin();
		cNpcFuncMap::cIterator end = mpNpcFuncMap->End();
		for( ; i != end; ++i )
		{
			sNpcFuncData* data = (sNpcFuncData*)(i->mSecond);
			SAFE_DELETE( data );
		}
		mpNpcFuncMap->Clear();
	}

	SAFE_DELETE( mpNpcFuncMap );
	ReleaseModelNumber();
	ReleaseNPCRegen();
	ReleaseNPCList();
}



bool cNPCScript::LoadNPCList()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/NPCList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load NPCList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// NPC 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long npcClassIdx = str.ToInt(); 

		/// NPC 이름
		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		TCHAR name[32];
#ifdef _CLIENT
		_stprintf_s( name, _T( "%s" ), GAMERESOURCEMAN->GetNpcName( str.ToInt() ) ); 
#else
		_stprintf_s( name, _T( "%d" ), str.ToInt() );
#endif
		/// npc type
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		short type = (short)str.ToInt();

		/// NPC 직업 인덱스
		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		TCHAR jobName[32];
#ifdef _CLIENT
		_stprintf_s( jobName, _T( "%s" ), GAMERESOURCEMAN->GetNpcName( str.ToInt() ) ); 
#else
		_stprintf_s( jobName, _T( "%d" ), str.ToInt() );
#endif

		/// NPC 애니메이션 파일 번호(KFM)]
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned int modelIndex = str.ToInt(); 

		/// NPC 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long jobIconLarge = str.ToInt(); 

		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long jobIconSmall = str.ToInt(); 

		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long jobIconIndexTarget = str.ToInt(); 

		sNPCList* pList = new sNPCList;

		pList->mNpcClassIdx = npcClassIdx; 
		_stprintf_s( pList->mName, _T( "%s" ), name );
		_stprintf_s( pList->mJobName, _T( "%s" ), jobName );
		pList->mType = type;
		pList->mModelIndex = modelIndex; 
		pList->mJobIconIndexLarge = jobIconLarge;
		pList->mJobIconIndexSmall = jobIconSmall;
		pList->mJobIconIndexTarget = jobIconIndexTarget;

		/// 해쉬에 NPC 별로 기록
		if( mpNPCListMap->Insert( pList->mNpcClassIdx, pList ) == false )
		{
			assert(0);
			return false;
		}
	}

	return true;

}



sNPCList* cNPCScript::GetNPCList( unsigned long idx )
{
	sNPCList*	NPClist;
	NPClist = (sNPCList*)mpNPCListMap->GetAt( idx );

	if( !NPClist )
	{
		assert(NULL);
		return NULL;
	}

	return NPClist;
}



void cNPCScript::ReleaseNPCList()
{
	if( mpNPCListMap )
	{
		sNPCList* pInfo = NULL;
		cPointHashMap::cIterator i = mpNPCListMap->Begin();
		cPointHashMap::cIterator end = mpNPCListMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (sNPCList*)((*i).mSecond);
			if( pInfo )
			{
				delete pInfo;
				pInfo = NULL;
			}
		}
		mpNPCListMap->Clear();
		SAFE_DELETE( mpNPCListMap );
	}
}



bool cNPCScript::LoadNPCRegen( unsigned short mapNumber, unsigned char levelMode, 
							  tPointerHashMap<unsigned long, void*>* pNPCRegenMap )
{
	if( pNPCRegenMap == NULL )
		return false;

	cFileLoader loader;
	cString pathName;
	pathName.Format( "./Script/Resource/NPCRegen%d_%d.txt", mapNumber, levelMode );
	unsigned long insertidx = 1;

	if( loader.Open( pathName, true ) == false )
	{
		/// 파일을 읽지 못하면 정상리턴
		return true;
	}

	if( loader.GetSize() == 0 )
		return true;

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	cHashSet* pMondelNumberHashSet = (cHashSet*)mpModelNumberMap->GetAt( mapNumber );
	if( pMondelNumberHashSet == NULL )
	{
		pMondelNumberHashSet = new cHashSet;
		/// 맵에 해당하는 모델 정보 해쉬셋을 기록
		if( mpModelNumberMap->Insert( mapNumber, pMondelNumberHashSet ) == false )
		{
			assert(NULL);
			return false;
		}
	}

	while( tokenizer.IsEnd() == false )
	{
		/// NPC 번호(고유번호 아님-종류번호)
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long npcClassIdx = str.ToInt();

		/// NPC 생성위치 x
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float posX = str.ToFloat(); 

		/// NPC 생성위치 y
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float posY = str.ToFloat();

		/// NPC 방향
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float direction = str.ToFloat();

		sNPCRegen* pList = new sNPCRegen;

		pList->mNpcClassIdx = npcClassIdx;	
		pList->mMapNumber = mapNumber;
		pList->mPosX = posX; 	
		pList->mPosY = posY;
		pList->mDirection = S_ToRadian( direction );

		/// npc 고유번호가 맵넘버 자리까지 침범하는것을 방지
		if( MAPNUMBER_UNIT <= insertidx )
		{
			assert(NULL);
			return false;
		}

		/// NPC 고유 인덱스를 겹치지 않게 하기위해 맵넘버를 포함해 고유번호를 만들었다.
		if( pNPCRegenMap->Insert( ( mapNumber * MAPNUMBER_UNIT ) + insertidx, pList ) == false )
		{
			assert(0);
			return false;
		}
		++insertidx;

		/// 클라이언트에 loadstage시점에 먼저 데이타를 읽어와야 할 npc 목록 기록
		if( pMondelNumberHashSet->GetCount( pList->mNpcClassIdx ) == 0 )
		{
			pMondelNumberHashSet->Insert( pList->mNpcClassIdx );
		}
	}

	return true;
}



sNPCRegen* cNPCScript::GetNPCRegen( unsigned char levelMode, unsigned long idx )
{
	if( mpNPCRegenMap == NULL )
		return NULL;

	cPointHashMap* pRegenMap = (cPointHashMap*)mpNPCRegenMap->GetAt( levelMode );
	if( pRegenMap == NULL )
		return NULL;

	sNPCRegen*	pNPCRegen;
	pNPCRegen = (sNPCRegen*)pRegenMap->GetAt( idx );

	/// 없는 경우 NPC를 삭제 한경우
	if( !pNPCRegen )
	{
		return NULL;
	}

	return pNPCRegen;

}

void cNPCScript::GetNPCRegen( unsigned char levelMode, unsigned long mapidx,  tList<void*>* pNPCList )
{
	if( pNPCList == NULL )
		return;

	if( mpNPCRegenMap == NULL )
		return;

	cPointHashMap* pRegenMap = (cPointHashMap*)mpNPCRegenMap->GetAt( levelMode );
	if( pRegenMap == NULL )
		return;

	cPointHashMap::cIterator pos = pRegenMap->Begin();
	cPointHashMap::cIterator end = pRegenMap->End();
	for( ; pos != end; ++pos )
	{
		sNPCRegen*	pNPCRegen = (sNPCRegen*)(*pos).mSecond;
		if( pNPCRegen )
		{
			if( pNPCRegen->mMapNumber == mapidx )
				pNPCList->PushBack( pNPCRegen );
		}
	}
}


void cNPCScript::ReleaseNPCRegen()
{
	if( mpNPCRegenMap )
	{
		cPointHashMap::cIterator iT = mpNPCRegenMap->Begin();
		cPointHashMap::cIterator eT = mpNPCRegenMap->End();
		for( ; iT != eT ; ++iT )
		{
			cPointHashMap* pRegenMap = (cPointHashMap*)(*iT).mSecond;
			if( pRegenMap == NULL )
				continue;

			sNPCRegen* pInfo = NULL;
			cPointHashMap::cIterator i = pRegenMap->Begin();
			cPointHashMap::cIterator end = pRegenMap->End();
			for( ; i != end; ++i )
			{
				pInfo = (sNPCRegen*)((*i).mSecond);
				if( pInfo )
				{
					delete pInfo;
					pInfo = NULL;
				}
			}
			pRegenMap->Clear();
			SAFE_DELETE( pRegenMap );
		}
		mpNPCRegenMap->Clear();
		SAFE_DELETE( mpNPCRegenMap );
	}
}



void cNPCScript::ReleaseModelNumber()
{
	if( mpModelNumberMap )
	{
		cHashSet* pInfo = NULL;
		cPointHashMap::cIterator i = mpModelNumberMap->Begin();
		cPointHashMap::cIterator end = mpModelNumberMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (cHashSet*)(*i).mSecond;
			if( pInfo )
			{
				SAFE_DELETE(pInfo);
			}
		}
		mpModelNumberMap->Clear();
		SAFE_DELETE( mpModelNumberMap );
	}
}


bool cNPCScript::LoadNPCStore()
{
	/// 파일 로딩
	cFileLoader loader;
	cString pathName;
	pathName.Format( "./Script/Resource/NPC_Func.txt" );

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cNpcLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_NPC_TELLING:
			{
				/// Npc 대화 인덱스
				unsigned int i = parser.ParseInt();

				/// 세부로딩
				sNpcFuncData* data = new sNpcFuncData;
				memset( data, 0, sizeof(sNpcFuncData) );

				LoadNpcStoreDetail( parser, data );

				/// Npc 대화 배열에 추가
				if( mpNpcFuncMap->Insert( i, data ) == false )
				{
					assert( 0 && "failed to insert mpNpcFuncMap, maybe already exist" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}


bool cNPCScript::LoadNpcStoreDetail( cParser& parser, sNpcFuncData* data )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long openCnt = 1;
	int& count = data->mFuncCount;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "{" )
			++openCnt;

		if( token == "}" )
			--openCnt;

		if( openCnt == 0 )
			break;

		switch( token.mType )
		{
		case eTOKEN_NPC_STORE:
			{
				data->mFunc[count] = NPC_FUNC_STORE;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
		case eTOKEN_NPC_WAREHOUSE:
			{
				data->mFunc[count] = NPC_FUNC_WAREHOUSE;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
		case eTOKEN_NPC_DISJOINT:
			{
				data->mFunc[count] = NPC_FUNC_ITEMDISJOINT;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
//		case eTOKEN_NPC_ENHANCE_RESET:
//			{
//				data->mFunc[count] = NPC_FUNC_ENHANCE_RESET;
//				assert( count < NPC_FUNCHAVE_MAX );

//				if( count < NPC_FUNCHAVE_MAX )
//					count++;
//			}
//			break;
//		case eTOKEN_NPC_ENHANCE_EXCHANGE:
//			{
//				data->mFunc[count] = NPC_FUNC_ENHANCE_EXCHANGE;
//				assert( count < NPC_FUNCHAVE_MAX );

//				if( count < NPC_FUNCHAVE_MAX )
//					count++;
//			}
//			break;

		case eTOKEN_NPC_EVENT:
			{
				data->mFunc[count] = NPC_FUNC_EVENT;
				assert( count < NPC_FUNCHAVE_MAX );

				data->mEventIndex = (unsigned long)parser.ParseInt();

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
		case eTOKEN_NPC_GUILD:
			{
				data->mFunc[count] = NPC_FUNC_GUILD;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
		case eTOKEN_NPC_FORTUNETAROT:
			{
				data->mFunc[count] = NPC_FUNC_FORTUNETAROT;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;

				/// 타로 요금정보
				data->mTarotPrice = (unsigned long)parser.ParseInt();

				// 카드 정보
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int cnt = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						data->mTarotIndex[cnt] = (unsigned long)token.ToInt();
						cnt++;
					}

					if( cnt > 22 )
					{
						assert(0);
						break;
					}
				}
			}
			break;
		case eTOKEN_NPC_TAROT:
			{
				// 유저형태 타롯
				data->mFunc[count] = NPC_FUNC_TAROT;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;

				/// 타로 요금정보
				data->mTarotPrice = (unsigned long)parser.ParseInt();

				// 카드 정보
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int cnt = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						data->mTarotIndex[cnt] = (unsigned long)token.ToInt();
						cnt++;
					}

					if( cnt > 22 )
					{
						assert(0);
						break;
					}
				}
			}
			break;
		case eTOKEN_NPC_FORTUNESPREAD:
			{
				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int cnt = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						data->mSpreadIndex[cnt] = (unsigned long)token.ToInt();
						cnt++;
					}

					if( cnt > 5 )
					{
						assert(0);
						break;
					}
				}

				data->mSpreadCount = cnt;
			}
			break;
		case eTOKEN_NPC_SPREAD:
			{
				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int cnt = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						data->mSpreadIndex[cnt] = (unsigned long)token.ToInt();
						cnt++;
					}

					if( cnt > 5 )
					{
						assert(0);
						break;
					}
				}

				data->mSpreadCount = cnt;
			}
			break;
		case eTOKEN_NPC_DEAL:
			{
				data->mFunc[count] = NPC_FUNC_DEAL;
				assert( count < NPC_FUNCHAVE_MAX );

				if( count < NPC_FUNCHAVE_MAX )
					count++;
			}
			break;
		case eTOKEN_NPC_THEME:
			{
				data->mFunc[count] = NPC_FUNC_THEME;
				assert( count < NPC_FUNCHAVE_MAX );

				if ( count < NPC_FUNCHAVE_MAX )
					count++;

				/// 테마 던젼 인덱스 정보
				data->mThemeIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_NPC_SKILLPREVIEW:
			{
				data->mFunc[count] = NPC_FUNC_SKILLPREVIEW;
				assert( count < NPC_FUNCHAVE_MAX );

				if ( count < NPC_FUNCHAVE_MAX )
					count++;

				// 직업 정보 인덱스 두개
				parser.ParseInt();
				parser.ParseInt();
			}
			break;
		}
	}

	return 0;
}

bool cNPCScript::IsHaveNpcFunc( unsigned long classIdx, unsigned int func )
{
	sNpcFuncData* data = (sNpcFuncData*)mpNpcFuncMap->GetAt( classIdx );
	if( !data )
	{
		assert(0);
		return false;
	}

	for( int i = 0; i < data->mFuncCount; ++i )
	{
		if( data->mFunc[i] == func )
			return true;
	}

	return false;
}

sNpcFuncData* cNPCScript::GetNpcFuncData( unsigned long classIdx )
{
	return (sNpcFuncData*)mpNpcFuncMap->GetAt( classIdx );
}

cNpcLexer::cNpcLexer( const char* buffer, unsigned int size )
: cLexer( buffer, size )
{
	BindKeyword( "npc_telling", eTOKEN_NPC_TELLING );

	BindKeyword( "store", eTOKEN_NPC_STORE );
	BindKeyword( "warehouse", eTOKEN_NPC_WAREHOUSE );
	BindKeyword( "default", eTOKEN_NPC_DEFAULT );
	BindKeyword( "disjoint", eTOKEN_NPC_DISJOINT );
	BindKeyword( "guild", eTOKEN_NPC_GUILD );
	BindKeyword( "fortunetarot", eTOKEN_NPC_FORTUNETAROT );
	BindKeyword( "tarot", eTOKEN_NPC_TAROT );
	BindKeyword( "deal", eTOKEN_NPC_DEAL );
	BindKeyword( "fortunespread", eTOKEN_NPC_SPREAD );
	BindKeyword( "spread", eTOKEN_NPC_SPREAD );
	BindKeyword( "quest", eTOKEN_NPC_QUEST );
	BindKeyword( "theme", eTOKEN_NPC_THEME );
	BindKeyword( "skillpreview", eTOKEN_NPC_SKILLPREVIEW );
	BindKeyword( "eventreward", eTOKEN_NPC_EVENT );

	BindKeyword( "text", eTOKEN_NPC_TEXT );
	BindKeyword( "page", eTOKEN_NPC_PAGE );

	BindKeyword( "new", eTOKEN_NPC_NEW );
	BindKeyword( "play", eTOKEN_NPC_PLAY );
	BindKeyword( "complete", eTOKEN_NPC_COMPLETE );

	BindKeyword( "answer", eTOKEN_NPC_ANSWER );
	BindKeyword( "sound", eTOKEN_NPC_SOUND );
	BindKeyword( "scene", eTOKEN_NPC_SCENE );

	BindKeyword( "next", eTOKEN_NPC_NEXT );
	BindKeyword( "EVENT_YES", eTOKEN_NPC_EVENTYES );
	BindKeyword( "EVENT_REWARD", eTOKEN_NPC_EVENTREWARD );
	BindKeyword( "EVENT_COMPLETE", eTOKEN_NPC_EVENTCOMPLETE );

	BindKeyword( "event_yes", eTOKEN_NPC_EVENTYES );
	BindKeyword( "event_reward", eTOKEN_NPC_EVENTREWARD );
	BindKeyword( "event_complete", eTOKEN_NPC_EVENTCOMPLETE );
}
