#include "stdafx.h"
#include "TarotManager.h"
#include "CardImage.h"

#include "ResourceManager.h"
#include "Tokenizer.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"

#include "ItemManager.h"

///
bool cTarotManager::LoadTarotParams( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load tarot params" );
		return false;
	}

	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	/// 타롯 텍스쳐 로딩
	if( parser.ExpectTokenString( "images" ) == false )
		return false;
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		/// "}" 가 나오면 끝
		if( token.mType == eTOKEN_RCURLY )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				unsigned int id = token.ToInt();
				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_STR );

				cString pathName;
				pathName.Format( "./Data/2DData/%s", token.Cstr() );
				NiTexture* p = RESOURCEMAN->LoadTexture( pathName, false );

				if( p == 0 )
				{
					assert( 0 && "failed to load texture" );
					return false;
				}

				/// 텍스쳐 맵에 추가
				mTarotTextureMap.Insert( id, p );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	/// 타롯 이미지 파람 리스트 파싱
	if( parser.ExpectTokenString( "tarotParams" ) == false )
		return false;
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_RCURLY )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				unsigned int imgIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned texIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texX = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texY = token.ToInt();

				/// 텍스쳐 찾기
				cTextureMap::cIterator i = mTarotTextureMap.Find( texIndex );
				if( i == mTarotTextureMap.End() )
				{
					assert( 0 && "failed to find tarot texture by id" );
					return false;
				}

				cCardParam* param = new cCardParam;
				param->mpTexture = (NiTexture*)i->mSecond;
				param->mTexPos.mX = texX;
				param->mTexPos.mY = texY;

				if( mTarotParamMap.Insert( imgIndex, param ) == false )
				{
					assert( 0 && "failed to insert tarot param, maybe already exist" );
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

bool cTarotManager::LoadNpcSpread( const cString& path )
{
	cFileLoader		loader;

	if( loader.Open( path, true ) == false )
	{
		assert( 0 && "failed to load LoadNpcSpread" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", path.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long idx = (unsigned long)str.ToInt();

		/// x 좌표
		if( tokenizer.GetNext( &str ) == false )
			return false;

		int xpos = str.ToInt();

		/// y 좌표
		if( tokenizer.GetNext( &str ) == false )
			return false;

		int ypos = str.ToInt();

		/// 운세 속성
		if( tokenizer.GetNext( &str ) == false )
			return false;

		short fortuneProp = (short)str.ToInt();

		/// 위치 속성
		if( tokenizer.GetNext( &str ) == false )
			return false;

		short posProp = (short)str.ToInt();

		/// 질문 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long questionIdx = (unsigned long)str.ToInt();

		///
		sNpcSpread* spread = (sNpcSpread*)mNpcSpreadMap.GetAt( idx );
		if( spread )
		{
			/// 검사
			if( spread->mCount >= MAX_NPCSPREAD_COUNT )
			{
				assert( 0 && "error npc spread axis" );
				return false;
			}

			int count = spread->mCount;
			spread->mParam[count].mX = xpos;
			spread->mParam[count].mY = ypos;
			spread->mParam[count].mPositionProp = posProp;
			spread->mParam[count].mQuestionIdx = questionIdx;
			spread->mCount++;
		}	
		else
		{
			/// 없으면 삽입
			spread = new sNpcSpread;
			spread->mParam[0].mX = xpos;
			spread->mParam[0].mY = ypos;
			spread->mParam[0].mPositionProp = posProp;
			spread->mParam[0].mQuestionIdx = questionIdx;

			spread->mFortuneProp = fortuneProp;
			spread->mCount = 1;

			if( mNpcSpreadMap.Insert( idx, spread ) == false )
			{
				assert( 0 && "faild to insert npc spread" );
				return false;
			}
		}
	}

	return true;
}

/// 해석 로드
bool cTarotManager::LoadComment( const cString& path )
{
	cFileLoader		loader;

	if( loader.Open( path, true ) == false )
	{
		assert( 0 && "failed to load LoadComment" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", path.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long idx = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 운세 속성
		short fortune = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 위치 속성
		short pos = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 방향 속성
		short dir = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 해석 1
		unsigned long comment1 = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 해석 2
		unsigned long comment2 = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 해석 3
		unsigned long comment3 = (unsigned long)str.ToInt();

		if( !(fortune >= 0 && fortune < 5) )
		{
			assert(0);
			return false;
		}

		if( !(pos >= 0 && pos < 3) )
		{
			assert(0);
			return false;
		}

		if( !(dir >= 0 && dir < 2) )
		{
			assert(0);
			return false;
		}

		sNpcTarotComment* p = (sNpcTarotComment*)mNpcCommentMap.GetAt( idx );
		if( p )
		{
			p->mComment[fortune][pos][dir][0] = comment1;
			p->mComment[fortune][pos][dir][1] = comment2;
			p->mComment[fortune][pos][dir][2] = comment3;
		}
		else
		{
			p = new sNpcTarotComment;
			p->mComment[fortune][pos][dir][0] = comment1;
			p->mComment[fortune][pos][dir][1] = comment2;
			p->mComment[fortune][pos][dir][2] = comment3;

			if( mNpcCommentMap.Insert( idx, p ) == false )
			{
				assert(0);
				return false;
			}
		}
	}
	return true;
}

///
bool cTarotManager::LoadTarotDeck( const cString& path )
{
	cFileLoader		loader;

	if( loader.Open( path, true ) == false )
	{
		assert( 0 && "failed to load LoadTarotDeck" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", path.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 카드 인덱스
		unsigned long cardIdx = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 이미지 인덱스
		unsigned long imgIdx = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 고유 번호
		short number = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 정방향 설명 인덱스
		unsigned long straightIdx = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 역방향 설명 인덱스
		unsigned long inverseIdx = (unsigned long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long commentIdx = (unsigned long)str.ToInt();

		/// 검사
		if( !(number >= 0 && number < MAX_TAROT_CARDS) )
		{
			assert(0);
			return false;
		}

		/// 카드 정보 삽입
		sNpcTarotCard* tarot = new sNpcTarotCard;
		tarot->mImageIndex  = imgIdx;
		tarot->mNumber		= number;
		tarot->mStraightIdx = straightIdx;
		tarot->mInverseIdx  = inverseIdx;
		tarot->mCommnetIndex = commentIdx;
		if( mNpcTarotMap.Insert( cardIdx, tarot ) == false )
		{
			assert(0);
			return false;
		}
	}
	return true;
}

///
cCardParam* cTarotManager::GetTarotParam( unsigned int itemIdx )
{
	///
	cItemDefine* pDefine = ITEMMAN->GetItemDefine( itemIdx );
	if( pDefine == 0 )
	{
		assert( 0 );
		return 0;
	}

	cItemTarot* pTarotDefine = pDefine->GetTarotDefine();
	if( pTarotDefine == 0 )
	{
		assert( 0 );
		return 0;
	}

	/// 이미지인덱스에 따른 속성 리턴 
	unsigned int imgIdx = pTarotDefine->GetImageIndex();
	return GetImageParam( imgIdx );
}

///
cCardParam* cTarotManager::GetImageParam( unsigned int imgIndex )
{
	/// 이미지인덱스에 따른 속성 리턴 
	cTarotParamMap::cIterator i = mTarotParamMap.Find( imgIndex );

	if( i == mTarotParamMap.End() )
		return 0;
	else
		return (cCardParam*)(i->mSecond);
}

sNpcSpread* cTarotManager::GetNpcSpread( unsigned long spreadIdx )
{
	return (sNpcSpread*)mNpcSpreadMap.GetAt( spreadIdx );
}

sNpcTarotCard* cTarotManager::GetNpcTarot( unsigned long cardIdx )
{
	return (sNpcTarotCard*)mNpcTarotMap.GetAt( cardIdx );
}

sNpcTarotComment* cTarotManager::GetNpcTarotComment( unsigned long commentIdx )
{
	return (sNpcTarotComment*)mNpcCommentMap.GetAt( commentIdx );
}