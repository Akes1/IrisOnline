#include "StdAfx.h"
#include "StatusScript.h"
#include "FileLoader.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Player_Common.h"
#include "Lexer.h"
#include "GameFileLexer.h"

#ifdef _CLIENT
#include "GameResourceManager.h"
#endif


cStatusScript* cStatusScript::mpStatusScript = NULL;



cStatusScript::cStatusScript(void)
{
	/// 싱글톤
	if( !mpStatusScript )
	{
		mpStatusScript = this;
	}

}


bool cStatusScript::Init()
{

	/// 스텟 기본 해쉬맵 - 기본 종족군 6개
	mpStatusBaseMap = new cPointerHashMap( 6 );
	/// 스텟 증가수치 해쉬맵 - 직업군 30개 
	mpStatusMultiplMap = new cPointerHashMap( 50 );
	/// 스텟 확장 기본 해쉬맵 - 3개
	mpStatusExtBaseMap = new cPointerHashMap( 20 );
	/// 스텟 확장 증가수치 해쉬맵 - 51개 
	mpStatusExtMultiplMap = new cPointerHashMap( 20 );

	/// 스텟 기본정보 로드
	if( !LoadStateBase() )
	{
		assert(NULL);
		return false;
	}

	/// 스텟 증가수치 로드
	if( !LoadStateMultipl() )
	{
		assert(NULL);
		return false;
	}

	/// 스텟 확장 기본정보 로드
	if( !LoadStatusExtBase() )
	{
		assert(NULL);
		return false;
	}

	/// 스텟 확장 증가수치 로드
	if( !LoadStatusExtMultipl() )
	{
		assert(NULL);
		return false;
	}

	/// 데미지계산기 계산 수치 로드
	if( !LoadDamageCalcNumerical() )
	{
		assert(NULL);
		return false;
	}

#ifndef _CLIENT
	/// 직업에 관련한 연결 정보를 생성
	if( !LoadPlayerJob() )
	{
		assert(NULL);
		return false;
	}
#endif

	return true;

}



void cStatusScript::Release()
{

	RemovePlayerJobInfo();

	if( mpStatusExtMultiplMap )
	{
		float* pInfo = NULL;
		cPointerHashMap::cIterator i = mpStatusExtMultiplMap->Begin();
		cPointerHashMap::cIterator end = mpStatusExtMultiplMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (float*)(*i).mSecond;
			if( pInfo )
			{
				delete [] pInfo;
				pInfo = NULL;
			}
		}
		mpStatusExtMultiplMap->Clear();
		SAFE_DELETE( mpStatusExtMultiplMap );
	}


	if( mpStatusExtBaseMap )
	{
		cHashMap* pInfo = NULL;
		cPointerHashMap::cIterator i = mpStatusExtBaseMap->Begin();
		cPointerHashMap::cIterator end = mpStatusExtBaseMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (cHashMap*)(*i).mSecond;
			if( pInfo )
			{
				pInfo->Clear();
				delete pInfo;
				pInfo = NULL;
			}
		}
		mpStatusExtBaseMap->Clear();
		SAFE_DELETE( mpStatusExtBaseMap );
	}


	if( mpStatusMultiplMap )
	{
		sStatusMultiplScript* pInfo = NULL;
		cPointerHashMap::cIterator i = mpStatusMultiplMap->Begin();
		cPointerHashMap::cIterator end = mpStatusMultiplMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (sStatusMultiplScript*)(*i).mSecond;
			if( pInfo )
			{
				delete pInfo;
				pInfo = NULL;
			}
		}
		mpStatusMultiplMap->Clear();
		SAFE_DELETE( mpStatusMultiplMap );
	}


	if( mpStatusBaseMap )
	{
		sStatusBaseScript* pInfo = NULL;
		cPointerHashMap::cIterator i = mpStatusBaseMap->Begin();
		cPointerHashMap::cIterator end = mpStatusBaseMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (sStatusBaseScript*)(*i).mSecond;
			if( pInfo )
			{
				delete pInfo;
				pInfo = NULL;
			}
		}
		mpStatusBaseMap->Clear();
		SAFE_DELETE( mpStatusBaseMap );
	}

}



bool cStatusScript::LoadStateBase()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/statusbase.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load statusbase.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString string;

	while( tokenizer.IsEnd() == false )
	{
		/// 직업군 코드
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		ePLAYER_JOB jobIndex = static_cast<ePLAYER_JOB>(string.ToInt());

		/// 힘
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		unsigned char str = static_cast<unsigned char>(string.ToInt());	

		/// 민첩
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		unsigned char dex = static_cast<unsigned char>(string.ToInt());	

		/// 체력
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		unsigned char con = static_cast<unsigned char>(string.ToInt());	

		/// 지능
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		unsigned char intelligence = static_cast<unsigned char>(string.ToInt());	

		/// 지혜
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		unsigned char wis = static_cast<unsigned char>(string.ToInt());	

		sStatusBaseScript* pList = new sStatusBaseScript;

		pList->mJobIndex = jobIndex;	
		pList->mBaseStatus[ePLAYER_STATUS_STR] = str;	
		pList->mBaseStatus[ePLAYER_STATUS_DEX] = dex;	
		pList->mBaseStatus[ePLAYER_STATUS_CON] = con;	
		pList->mBaseStatus[ePLAYER_STATUS_INT] = intelligence;	
		pList->mBaseStatus[ePLAYER_STATUS_WIS] = wis;	

		/// 해쉬에 몬스터 별로 기록
		if( mpStatusBaseMap->Insert( pList->mJobIndex, pList ) == false )
		{
			assert(NULL);
			return false;
		}
	}

	return true;

}



sStatusBaseScript* cStatusScript::GetStatusBaseInfo( unsigned long idx )
{
	sStatusBaseScript*	statusBase;
	statusBase = (sStatusBaseScript*)mpStatusBaseMap->GetAt( idx );

	if( !statusBase )
	{
		return NULL;
	}

	return statusBase;
}



bool cStatusScript::LoadStateMultipl()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/statusmultipl.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load statusmultipl.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString string;

	while( tokenizer.IsEnd() == false )
	{
		/// 직업군 코드
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		ePLAYER_JOB jobIndex = static_cast<ePLAYER_JOB>(string.ToInt());	

		/// 힘
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		float str = string.ToFloat();	

		/// 민첩
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		float dex = string.ToFloat();	

		/// 체력
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		float con = string.ToFloat();	

		/// 지능
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		float intelligence = string.ToFloat();	

		/// 지혜
		if( tokenizer.GetNext( &string ) == false )	{ return false; }
		float wis = string.ToFloat();	

		sStatusMultiplScript * pList = new sStatusMultiplScript;

		pList->mJobIndex = jobIndex;	
		pList->mBaseStatus[ePLAYER_STATUS_STR] = str;	
		pList->mBaseStatus[ePLAYER_STATUS_DEX] = dex;	
		pList->mBaseStatus[ePLAYER_STATUS_CON] = con;	
		pList->mBaseStatus[ePLAYER_STATUS_INT] = intelligence;	
		pList->mBaseStatus[ePLAYER_STATUS_WIS] = wis;

		/// 해쉬에 기록
		if( mpStatusMultiplMap->Insert( pList->mJobIndex, pList ) == false )
		{
			assert(NULL);
			return false;
		}

	}

	return true;

}



sStatusMultiplScript* cStatusScript::GetStatusMultiplInfo(unsigned long idx)
{

	sStatusMultiplScript*	statusMultipl;
	statusMultipl = (sStatusMultiplScript*)mpStatusMultiplMap->GetAt( idx );

	if( !statusMultipl )
	{
		return NULL;
	}

	return statusMultipl;

}



bool cStatusScript::LoadStatusExtBase()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/statusextbase.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load statusextbase.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	cHashMap* pStatusExt = NULL;
	while( tokenizer.IsEnd() == false )
	{
		/// 확장 스텟 구분
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_STATUS_EXT statusExtBase = (ePLAYER_STATUS_EXT)str.ToInt();

		/// 직업 구분
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_JOB job = (ePLAYER_JOB)str.ToInt();

		/// 기본값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float defaultValue = str.ToFloat();

		pStatusExt = (cHashMap*)mpStatusExtBaseMap->GetAt( statusExtBase );
		if( pStatusExt == NULL )
		{
			pStatusExt = new cHashMap;
			if( mpStatusExtBaseMap->Insert( statusExtBase, pStatusExt ) == NULL )
			{
				assert(NULL);
				return false;
			}
		}

		if( pStatusExt->Insert( job, defaultValue ) == NULL )
		{
			assert(NULL);
			return false;
		}
	}

	return true;

}

float cStatusScript::GetStatusExtBaseInfo( unsigned long statusExt, unsigned long jobCode )
{
	cHashMap*	jobDefaultHashMap = NULL;
	jobDefaultHashMap = (cHashMap*)mpStatusExtBaseMap->GetAt( statusExt );

	if( jobDefaultHashMap == NULL )
	{
		return 0.0f;
	}

	cHashMap::cIterator find = jobDefaultHashMap->Find( jobCode );
	cHashMap::cIterator end = jobDefaultHashMap->End();

	if( find == end )
	{
		return SearchParentStatusExtbaseInfo( statusExt, jobCode );
	}

	return (*find).mSecond;
}

float cStatusScript::SearchParentStatusExtbaseInfo( unsigned long statusExt, unsigned long jobcode )
{
	/// 상위직업 검사
#ifdef _CLIENT
	tArray<unsigned long>* parentJob = GAMERESOURCEMAN->GetParentJob( jobcode );
	if( parentJob && parentJob->GetSize() > 0 )
	{
		for( unsigned int i=parentJob->GetSize()-1; i >=0; --i )
		{
			float statusValue = (float)GetStatusExtBaseInfo( statusExt, (*parentJob)[i] );
			if( FloatToInt(statusValue) != 0 )
				return statusValue;
		}
	}
#else
	tArray<unsigned long>* parentJob = GetParentJob( jobcode );
	if( parentJob && parentJob->GetSize() > 0 )
	{
		for( unsigned int i=parentJob->GetSize()-1; i >=0; --i )
		{
			float statusValue = (float)GetStatusExtBaseInfo( statusExt, (*parentJob)[i] );
			if( FloatToInt(statusValue) != 0 )
				return statusValue;
		}
	}
#endif

	return 0.0f;
}

bool cStatusScript::LoadStatusExtMultipl()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/statusextmultipl.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load statusextmultipl.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	float* statusExtMultiplScript = NULL;

	while( tokenizer.IsEnd() == false )
	{
		/// 구분
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		eEXT_MULTIPL statusExtBase = (eEXT_MULTIPL)(str.ToInt());

		/// 등록된 값이 있는지 체크
		statusExtMultiplScript = (float*)mpStatusExtMultiplMap->GetAt( statusExtBase );

		/// 등록된 값(배열포인터)이 없으면 신규 생성
		if( statusExtMultiplScript == NULL )
		{
			statusExtMultiplScript = new float[ePLAYER_STATUS_LEVEL_MAX];

			memset( statusExtMultiplScript, 0, sizeof(float[ePLAYER_STATUS_LEVEL_MAX]) );

			/// 해쉬에 기록
			if( mpStatusExtMultiplMap->Insert( statusExtBase, statusExtMultiplScript ) == false )
			{
				assert(NULL);
				return false;
			}
		}

		/// 항목[배열번호]
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		ePLAYER_STATUS type = static_cast<ePLAYER_STATUS>(str.ToInt());

		/// 비중
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float importance = str.ToFloat();

		if( ePLAYER_STATUS_LEVEL < type )
		{
			assert(NULL);
			return false;
		}

		statusExtMultiplScript[type]= importance;
	}

	return true;

}



float cStatusScript::GetStatusExtMultiplInfo( eEXT_MULTIPL statusExtBase, ePLAYER_STATUS type )
{

	float*	statusExtMultipl = (float*)mpStatusExtMultiplMap->GetAt( statusExtBase );
	if( statusExtMultipl == NULL )
	{
		return 0.0f;
	}

	if( ePLAYER_STATUS_LEVEL < type )
	{
		assert(NULL);
		return 0.0f;
	}

	return statusExtMultipl[type];

}



bool cStatusScript::LoadDamageCalcNumerical()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/DamageCalc.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load DamageCalc.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short num = static_cast<unsigned short>(str.ToInt());

		/// 값
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float value = str.ToFloat();

		/// 해쉬에 기록
		if( mDamageCalcNumericalMap.Insert( num, value ) == false )
		{
			assert(0);
			return false;
		}
	}
	return true;

}

float cStatusScript::GetDamageCalcNumericalInfo(unsigned long idx)
{
	cHashMap::cIterator i = mDamageCalcNumericalMap.Find( idx );
	if( i == mDamageCalcNumericalMap.End() )
	{
		assert( NULL );
		return 0.0f;
	}

	return (float)((*i).mSecond);
}

/// 주의 사항 : Player의 직업 연결을 생성한다.
bool cStatusScript::LoadPlayerJob()
{
#ifndef _CLIENT
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( _T("./Script/Resource/PlayerJobTree.txt"), true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cGameFileLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, _T("./Script/Resource/PlayerJobTree.txt") );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_JOBINFO:
			{
				if( LoadPlayerJobInfo(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_JOBTREE:
			{
				unsigned int depth = parser.ParseInt();

				if( LoadPlayerJobTree(parser, depth) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
#endif
	return true;
}



bool cStatusScript::LoadPlayerJobInfo( cParser& parser )
{
	parser;
#ifndef _CLIENT
	if( parser.ExpectTokenString( _T( "{" ) ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == _T( "}" ) )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				unsigned long jobIdx = (unsigned long)token.ToInt();
				unsigned char race = (unsigned char)parser.ParseInt();
				unsigned int jobNameIdx = parser.ParseInt();
				unsigned int jobIconIdx = parser.ParseInt();

				jobIdx;
				race;
				jobNameIdx;
				jobIconIdx;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
#endif

	return true;
}



bool cStatusScript::LoadPlayerJobTree( cParser& parser, unsigned int depth )
{
	parser;
	depth;
#ifndef _CLIENT
	if( parser.ExpectTokenString( _T( "{" ) ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == _T( "}" ) )
		{
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				cJobIndexArray	tempArray;

				/// 기본 직업군을 읽어들인다.
				unsigned long jobIdx = token.ToInt();
				tempArray.PushBack(jobIdx);

				/// 최종 차수까지의 직업군을 모두 담는다.
				for( unsigned int i=0; i<depth-1; ++i )
				{
					unsigned long jobIdx = (unsigned long)parser.ParseInt();
					tempArray.PushBack(jobIdx);
				}

				if( tempArray.GetSize() )
				{
					/// 정보를 담는다.
					for( int i=tempArray.GetSize()-1; i>0; --i )
					{
						/// 최종 전직차수의 직업 인덱스를 key로 사용
						unsigned long jobIdx = tempArray[i];

						/// 새로운 직업연결을 생성
						cJobIndexArray* newJobTree = new cJobIndexArray;

						/// 차수까지의 부모 직업들을 담는다.
						for( int k=0 ; k<i ; ++k )
						{
							unsigned long parentJob = tempArray[k];
							newJobTree->PushBack( parentJob );
						}

						/// 참조 맵을 구성한다.
						if( mJobTree.Insert( jobIdx, newJobTree) == false )
						{
							assert(0);
							SAFE_DELETE(newJobTree);
						}
					}
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
#endif

	return true;
}



void cStatusScript::RemovePlayerJobInfo()
{
	cPointerHashMap::cIterator i = mJobTree.Begin();
	cPointerHashMap::cIterator end = mJobTree.End();

	for( ; i != end; ++i )
	{
		cJobIndexArray* pArray = (cJobIndexArray*)((*i).mSecond);
		SAFE_DELETE(pArray);
	}

	mJobTree.Clear();
}



tArray<unsigned long>* cStatusScript::GetParentJob( unsigned long currentJob )
{
	cJobIndexArray* pArray = (cJobIndexArray*)mJobTree.GetAt( currentJob );
	if( pArray )
	{
		return pArray;
	}
	return 0;
}

unsigned int cStatusScript::GetJobStep( unsigned long jobIdx )
{
	tArray<unsigned long>* pArray = GetParentJob( jobIdx );
	if( pArray == 0 )
	{
		return 0;
	}
	return pArray->GetSize();
}
