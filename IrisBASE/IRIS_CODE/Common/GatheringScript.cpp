#include "StdAfx.h"
#include "GatheringScript.h"

#include "BaseObject_Common.h"
#include "Gathering_Common.h"
#include "Stage_Common.h"

#include "Token.h"
#include "Parser.h"
#include "Lexer.h"
#include "FileLoader.h"
#include "Tokenizer.h"

#ifndef _CLIENT
#include "Drop_Common.h"
#endif

cGatheringScript* cGatheringScript::mpGatheringScript = NULL;



cGatheringScript::cGatheringScript(void)
{

	/// 싱글톤
	if( mpGatheringScript )
	{
		assert(NULL);
	}
	else
	{
		mpGatheringScript = this;
	}
}



bool cGatheringScript::Init()
{
	if( !LoadGatheringList() )
	{
		assert(NULL);
		return false;
	}

	return true;
}


bool cGatheringScript::StageInit( unsigned short mapNumber, unsigned char levelMode )
{
	if( mapNumber == MAP_NONE )
	{
		cPointHashMap* pGatheringRegenMap = new cPointHashMap;

		for( unsigned short mapNum = MAP_MIN ; mapNum <= MAP_MAX ; ++mapNum )
		{
			/// 맵에 위치한 채집 로드
			if( !LoadGatheringRegen( mapNum, levelMode, pGatheringRegenMap ) )
			{
				assert(NULL);
				return false;
			}
		}

		mGatheringRegenMap.Insert( levelMode, pGatheringRegenMap );
	}
	else
	{
		cPointHashMap* pGatheringRegenMap = new cPointHashMap;

		if( !LoadGatheringRegen( mapNumber, levelMode, pGatheringRegenMap ) )
		{
			assert(NULL);
			return false;
		}

		mGatheringRegenMap.Insert( levelMode, pGatheringRegenMap );
	}

	return true;
}


void cGatheringScript::Release()
{
	sGatheringList* pInfo = NULL;
	cPointHashMap::cIterator iList = mGatheringMap.Begin();
	cPointHashMap::cIterator eList = mGatheringMap.End();

	cPointHashMap::cIterator q;
	cPointHashMap::cIterator qEnd;

	for( ; iList != eList; ++iList )
	{
		pInfo = (sGatheringList*)((*iList).mSecond);
		if( pInfo )
		{

#ifndef _CLIENT

			sDropQTypeGather* pDropQ;
			cAryVoid* pAry;

			/// 퀘스트 아이템 목록 제거
			cPointHashMap* pDropMap = pInfo->mpQuestItemDropMap;
			if( pDropMap != NULL )
			{
				q = pDropMap->Begin();
				qEnd = pDropMap->End();
				for( ; q != qEnd; ++q )
				{
					pAry = (cAryVoid*)((*q).mSecond);
					if( pAry == NULL )
						continue;

					for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
					{
						pDropQ = (sDropQTypeGather*)(*pAry)[j];
						delete pDropQ;
					}					
					pAry->Clear();
					SAFE_DELETE( pAry );
				}
				pDropMap->Clear();
				SAFE_DELETE( pDropMap );
			}
#endif

			SAFE_DELETE( pInfo );
		}
	}
	mGatheringMap.Clear();

	cPointHashMap::cIterator iMode = mGatheringRegenMap.Begin();
	cPointHashMap::cIterator eMode = mGatheringRegenMap.End();

	for( ; iMode != eMode ; ++iMode )
	{
		cPointHashMap* pRegenMap = (cPointHashMap*)(*iMode).mSecond;
		if( pRegenMap == NULL )
			continue;

		sGatheringRegenHeader* pHeader = NULL;
		cPointHashMap::cIterator iRegen = pRegenMap->Begin();
		cPointHashMap::cIterator eRegen = pRegenMap->End();
		for( ; iRegen != eRegen; ++iRegen )
		{
			pHeader = (sGatheringRegenHeader*)((*iRegen).mSecond);
			if( pHeader != NULL )
			{
				cAryVoid* pAry = &pHeader->mRegenDetail;
				if( pAry != NULL )
				{
					sGatheringRegen* pDetail = NULL;
					for( unsigned int i = 0 ; i < pAry->GetSize() ; ++i )
					{
						pDetail = (sGatheringRegen*)((*pAry)[i]);
						SAFE_DELETE( pDetail );
					}
					pAry->Clear();
				}
				pAry = NULL;
			}
			SAFE_DELETE( pHeader );
		}
		pRegenMap->Clear();
		SAFE_DELETE( pRegenMap );
	}
	mGatheringRegenMap.Clear();
}



bool cGatheringScript::LoadGatheringList()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/GatheringList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load GatheringList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sGatheringList* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sGatheringList;

		/// 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mGatheringIdx = str.ToInt();

		/// 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNameIdx = str.ToInt();

		/// 
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNameHeight = str.ToInt();

		/// 모델
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFileName = str;

		/// Icon image index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mIconIndex = str.ToInt();

		/// effect 파일명
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mEffectFileName = str;

		/// 종류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mType = (eGATHERING_TYPE)str.ToInt();

		/// 필요아이템
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNeedItemIdx = str.ToInt();

		/// 필요버프
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNeedBuffIdx = str.ToInt();

		/// 아이템 소모 여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// skip
		
		/// 캐릭터레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNeedLevel = (unsigned char)str.ToInt();

		/// 요구 제조스킬
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNeedMakeIdx = (unsigned char)str.ToInt();

		/// 요구 숙련도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// skip
		
		/// 요구 퀘스트
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mNeedQuestIdx = str.ToInt();	

		/// 효과 인덱스
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mInfluenceIdx = str.ToInt();	

		/// 채집 시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mGatheringTime = str.ToInt();

		/// 해쉬에 기록
		if( mGatheringMap.Insert( pList->mGatheringIdx, pList ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pList;
	return false;

}


sGatheringList* cGatheringScript::GetGatheringInfo( unsigned long gatheringIdx )
{
	return (sGatheringList*)mGatheringMap.GetAt( gatheringIdx );
}


bool cGatheringScript::LoadGatheringRegen( unsigned short mapnum, unsigned char levelMode,
										  tPointerHashMap<unsigned long, void*>* pGatheringRegenMap )
{

	if( pGatheringRegenMap == NULL )
		return false;

	cFileLoader loader;
	cString pathName;
	pathName.Format( "./script/resource/gatheringregen%d_%d.txt", mapnum, levelMode );

	if( loader.Open( pathName, true ) == false )
		return true;

	if( loader.GetSize() == 0 )
		return true;

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	sGatheringRegenHeader* pHeader = NULL;
	sGatheringRegen* pDetail = NULL;
	while( tok.IsEnd() == false )
	{

		/// 해더 정보 생성
		pHeader = new sGatheringRegenHeader;

		/// 리젠 번호 ( 리젠 스크립트 고유 번호 )
		tok.GetNext( &str );
		unsigned short groupNum = (unsigned short)str.ToInt();

		/// 고유번호가 맵넘버 자리까지 침범하는것을 방지
		if( MAPNUMBER_UNIT <= mapnum )
		{
			assert(NULL);
			goto ERR;
		}

		/// 리젠 번호 ( 리젠 스크립트 고유 번호 )
		unsigned long regenIdx = ( mapnum * MAPNUMBER_UNIT ) + groupNum;

		/// 리젠 대기시간
		tok.GetNext( &str );
		pHeader->mRegenWaitTime = str.ToInt();		

		/// 대기시간 이후 랜덤 리젠 시간
		tok.GetNext( &str );
		pHeader->mRandTime = str.ToInt();

		if( pGatheringRegenMap->Insert( regenIdx, pHeader ) == false )
		{
			assert(0);
			goto ERR;
		}

		tok.GetNext( &str );
		if( str != "{" )
			goto ERR;

		while( tok.IsEnd() == false )
		{
			tok.GetNext( &str );
			if( str == "}" )
				break;

			/// 개별 리젠 정보 기록
			pDetail = new sGatheringRegen;

			/// 채집IDX
			pDetail->mGatheringIdx = str.ToInt();	

			/// 맵번호
			pDetail->mMapNumber = mapnum;

			/// 좌표
			tok.GetNext( &str );
			pDetail->mPosX = str.ToFloat();
			tok.GetNext( &str );
			pDetail->mPosY = str.ToFloat();

			/// 회전값
			tok.GetNext( &str );
			pDetail->mDirection = S_ToRadian( str.ToFloat() );

			/// 높이 확장값
			tok.GetNext( &str );
			pDetail->mHeight = str.ToFloat();

			pHeader->mRegenDetail.PushBack( pDetail );
		}

		/// 채집IDX별 리젠위치 갯수 확인
		if( pHeader->mRegenDetail.GetSize() == 0 )
		{
			assert(NULL);
			goto ERR;
		}
	}

	return true;

ERR:
	delete pDetail;
	delete pHeader;
	return false;
}


const sGatheringRegenHeader* cGatheringScript::GetGatheringRegen( unsigned char levelMode, unsigned long regenIdx )
{
	cPointHashMap* pRegenMap = (cPointHashMap*)mGatheringRegenMap.GetAt( levelMode );
	if( pRegenMap == NULL )
		return NULL;

	return (const sGatheringRegenHeader*)pRegenMap->GetAt( regenIdx );
}
