#include "stdafx.h"
#include "UINpcTelling.h"

#include "Parser.h"
#include "Page.h"
#include "QuestDefine.h"
#include "UIManager.h"
#include "Npc_Common.h"
#include "NpcScript.h"

cUINpcTelling::cUINpcTelling()
: mDefaultTextIndex(0)
, mSpreadCount(0)
, mTarotPrice(0)
, mThemeIndex(0)
, mFuncCount(0)
, mEventIndex(0)
{
	memset( mSpreadIndex, 0, sizeof(mSpreadIndex) );
	memset( mTarotIndex, 0, sizeof(mTarotIndex) );
	memset( mFunc, 0, sizeof(mFunc) );
	memset( mJobData, 0, sizeof(mJobData) );
}

cUINpcTelling::~cUINpcTelling()
{
	cQuestLinkPage::cIterator i = mQuestLinkPage.Begin();
	cQuestLinkPage::cIterator end = mQuestLinkPage.End();

	for( ; i != end; ++i )
	{
		sQuestLinkPage* link = (sQuestLinkPage*)(i->mSecond);
		SAFE_DELETE( link );
	}
	mQuestLinkPage.Clear();
}

bool cUINpcTelling::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_NPC_STORE:
			{
				mFunc[mFuncCount] = NPC_FUNC_STORE;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_NPC_WAREHOUSE:
			{
				mFunc[mFuncCount] = NPC_FUNC_WAREHOUSE;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_NPC_DISJOINT:
			{
				mFunc[mFuncCount] = NPC_FUNC_ITEMDISJOINT;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_NPC_GUILD:
			{
				mFunc[mFuncCount] = NPC_FUNC_GUILD;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_NPC_DEAL:
			{
				mFunc[mFuncCount] = NPC_FUNC_DEAL;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_NPC_FORTUNETAROT:
			{
				mFunc[mFuncCount] = NPC_FUNC_FORTUNETAROT;

				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}

				/// 타로 요금
				mTarotPrice = parser.ParseInt();

				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int count = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						mTarotIndex[count] = (unsigned long)token.ToInt();
						count++;
					}

					if( count > 22 )
					{
						assert(0);
						break;
					}
				}
			}
			break;
		case eTOKEN_NPC_TAROT:
			{
				mFunc[mFuncCount] = NPC_FUNC_TAROT;
				
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}

				/// 타로 요금
				mTarotPrice = parser.ParseInt();

				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int count = 0;
				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						mTarotIndex[count] = (unsigned long)token.ToInt();
						count++;
					}

					if( count > 22 )
					{
						assert(0);
						break;
					}
				}
			}
			break;
		case eTOKEN_NPC_THEME:
			{
				mFunc[mFuncCount] = NPC_FUNC_THEME;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert( 0 );
				}

				mThemeIndex = parser.ParseInt();
			}
			break;
		case eTOKEN_NPC_SKILLPREVIEW:
			{
				mFunc[mFuncCount] = NPC_FUNC_SKILLPREVIEW;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}

				mJobData[0] = parser.ParseInt();
				mJobData[1] = parser.ParseInt();
			}
			break;
		case eTOKEN_NPC_EVENT:
			{
				mFunc[mFuncCount] = NPC_FUNC_EVENT;
				if( mFuncCount < NPC_FUNCHAVE_MAX )
				{
					mFuncCount++;
				}
				else
				{
					assert(0);
				}
				mEventIndex = parser.ParseInt();
			}
			break;
		case eTOKEN_NPC_DEFAULT:
			{
				mDefaultTextIndex = parser.ParseInt();

				unsigned long temp = 0;
				if( LoadPageList( parser, temp ) == false )
					return false;
			}
			break;
		case eTOKEN_NPC_QUEST:
			{
				unsigned long questIndex = parser.ParseInt();

				sQuestLinkPage* linkPage = new sQuestLinkPage;

				/// 퀘스트 연결 대화 파싱
				if( LoadQuestLink( parser, linkPage ) == false )
				{
					assert( 0 && "failed to load page" );
					SAFE_DELETE( linkPage );
					return false;
				}

				/// 퀘스트 링크 맵에 담기
				if( mQuestLinkPage.Insert( questIndex, linkPage ) == false )
				{
					assert( 0 && "failed to insert quest link page" );
					SAFE_DELETE( linkPage );
					return false;
				}
			}
			break;
		case eTOKEN_NPC_FORTUNESPREAD:
			{
				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						mSpreadIndex[mSpreadCount] = (unsigned long)token.ToInt();
						mSpreadCount++;
					}

					if( mSpreadCount > 5 )
					{
						assert(0);
						break;
					}
				}
			}
			break;
		case eTOKEN_NPC_SPREAD:
			{
				/// 보유한 스프레드 인덱스
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				while( token != "}" )
				{
					lexer->GetNextToken( &token );

					if( token.mType == eTOKEN_INT )
					{
						mSpreadIndex[mSpreadCount] = (unsigned long)token.ToInt();
						mSpreadCount++;
					}

					if( mSpreadCount > 5 )
					{
						assert(0);
						break;
					}
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

bool cUINpcTelling::LoadQuestLink( cParser& parser, sQuestLinkPage* linkPage )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long rewardIndex = 0;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_LCURLY:
			continue;
		case eTOKEN_NPC_NEW:
			{	
				linkPage->newIndex = (unsigned long)parser.ParseInt();

				if( LoadPageList( parser, rewardIndex ) == false )
					return false;
			}
			break;
		case eTOKEN_NPC_PLAY:
			{
				linkPage->playIndex = (unsigned long)parser.ParseInt();

				if( LoadPageList( parser, rewardIndex ) == false )
					return false;
			}
			break;
		case eTOKEN_NPC_COMPLETE:
			{
				linkPage->completeIndex = (unsigned long)parser.ParseInt();
				if( LoadPageList( parser, rewardIndex ) == false )
					return false;

				linkPage->rewardIndex = rewardIndex;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cUINpcTelling::LoadPageList( cParser& parser, unsigned long& rewardIndex )
{	
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		if( token.mType == eTOKEN_NPC_PAGE )
		{
			unsigned long pageIndex = parser.ParseInt();

			/// 새 페이지 생성
			cPage* page = new cPage;

			if( LoadPage( parser, page ) == false )
			{
				assert( 0 && "failed to load page data" );
				SAFE_DELETE( page );
				return false;
			}

			/// 보상 페이지는 complete 페이지로부터 시작한다
			if( page->mAnswerType == eEVENT_COMPLETE )
				rewardIndex = pageIndex;

			/// 페이지 삽입
			if( UIMAN->AddPage( pageIndex, page ) == false )
			{
				assert( 0 && "failed to insert page data" );
				SAFE_DELETE( page );
				return false;
			}
		}
		else
		{
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cUINpcTelling::LoadPage( cParser& parser, cPage* page )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_NPC_TEXT:
			{
				sPageText* pageText = new sPageText;
				pageText->textIndex = (unsigned int)parser.ParseInt();
				pageText->numLine = (unsigned int)parser.ParseInt();

				/// 기본대사 배열에 추가
				page->mDefaultTextArray.PushBack( pageText );
			}
			break;
		case eTOKEN_NPC_ANSWER:
			{
				/// 분기대사와 다음 페이지 넘기기는 동시에 있을수 없다
				if( page->mNextPageIndex > 0 )
				{
					assert(0);
					return false;
				}

				sPageAnswer* pageAns = new sPageAnswer;
				pageAns->textIndex = (unsigned int)parser.ParseInt();
				pageAns->numLine = (unsigned int)parser.ParseInt();
				pageAns->nextPageIndex = (unsigned int)parser.ParseInt();

				/// 분기 대사 배열에 추가
				page->mAnswerArr.PushBack( pageAns );
			}
			break;
		case eTOKEN_NPC_SOUND:
			break;
		case eTOKEN_NPC_SCENE:
			{
				page->mSceneIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_NPC_NEXT:
			{
				/// 분기대사와 다음 페이지 넘기기는 동시에 있을수 없다
				if( page->mAnswerArr.GetSize() > 0 )
				{
					assert(0);
					return false;
				}

				page->mNextPageIndex = (unsigned long)parser.ParseInt();

			}
			break;
		case eTOKEN_NPC_EVENTYES:
			{
				page->mAnswerType = eEVENT_YES;
			}
			break;
		case eTOKEN_NPC_EVENTREWARD:
			{
				page->mAnswerType = eEVENT_REWARD;
			}
			break;
		case eTOKEN_NPC_EVENTCOMPLETE:
			{
				page->mAnswerType = eEVENT_COMPLETE;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

/// 기본대사 랜덤으로 획득
sPageText* cUINpcTelling::GetDefaultTextRandomly()
{
	cPage* page = GetPage( mDefaultTextIndex );
	if( !page )
	{
		assert(0);
		return 0;
	}

	unsigned int i = page->mDefaultTextArray.GetSize();
	i = rand() % i;

	return (sPageText*)page->mDefaultTextArray[i];
}

unsigned long cUINpcTelling::GetDefaultScene()
{
	cPage* page = GetPage( mDefaultTextIndex );
	if( !page )
	{
		assert(0);
		return 0;
	}

	return page->GetSceneIdx();
}

/// 퀘스트 관련 페이지 정보 획득
sQuestLinkPage* cUINpcTelling::GetQuestLinkPage( unsigned long questIdx )
{
	return (sQuestLinkPage*)mQuestLinkPage.GetAt( questIdx );
}

cPage* cUINpcTelling::GetPage( unsigned long pageIdx )
{
	return UIMAN->GetPage( pageIdx );
}
