#include "stdafx.h"
#include "gamesrv.h"
#include "Parser.h"
#include "QuestManager.h"
#include "TriggerManager.h"
#include "Player_Common.h"
#include "Tokenizer.h"

/// 퀘스트 리스트 파일 로드
bool cQuestManager::Load( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
		return false;

	cString path;
	::GetFilePath( &path, pathname );

	cToken token;
	cQuestLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathname );

	if( parser.ExpectTokenString( "questlist" ) == false )
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
		case eTOKEN_STR:
			{
				cString pathName( path );
				pathName += token;

				if( LoadQuests( pathName ) == false )
				{
					assert( 0 && "failed to load quest script" );
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

/// 각 npc가 보유한 퀘슽 목록 로드
bool cQuestManager::LoadNpcQuest( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
		return false;

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathname.Cstr() );
	cString str;

	bool flag = true;

	if( tokenizer.GetNext( &str ) == false )
		return false;

	while( tokenizer.IsEnd() == false )
	{
		if( str.CompareNoCase( "npc" ) != 0 )
			return false;
			
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// npc 인덱스 받기
		unsigned long npcIndex = (unsigned long)str.ToInt();
		if( npcIndex == 0 )
			return false;
		
		sNpcQuestList* p = new sNpcQuestList;
		
		while( flag )
		{
			if( tokenizer.GetNext( &str ) == false )
				break;

			if( str.CompareNoCase( "npc" ) == 0 )
				break;

			unsigned long questIndex = (unsigned long)str.ToInt();
			if( questIndex == 0 )
				return false;

			/// 퀘스트인덱스 유효성 검사 
			cQuestDefine* define = GetQuestDefine( questIndex );
			if( !define )
				return false;

			if( p->mQuestSet.Insert( questIndex ) == false )
				return false;
		}

		if( mNpcQuestMap.Insert( npcIndex, p ) == false )
			return false;
	}
	return true;
}

/// 퀘스트 정보 스크립트 로딩
bool cQuestManager::LoadQuests( const cString& pathname )
{
	cFileLoader loader;
	if( loader.Open( pathname, true ) == false )
		return false;

	cToken token;
	cQuestLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathname );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_QUEST:
			{
				unsigned long i = (unsigned long)parser.ParseInt();

				/// 퀘스트 정보 생성
				cQuestDefine* p = new cQuestDefine;

				/// 트리거 정보 생성
				cTrigger* trigger = new cTrigger;

				/// 인덱스 저장
				p->mIndex = i;

				if( LoadQuestData( parser, p, trigger ) == false )
				{
					assert( 0 && "failed to load quest data" );
					SAFE_DELETE( p );
					SAFE_DELETE( trigger );
					return false;
				}

				/// 퀘스트 정보 저장
				if( mQuestDefine.Insert( i, p ) == false )
				{
					assert( 0 && "failed to insert quest define" );
					return false;
				}

				/// 트리거 정보 저장
				if( TRIGGERMAN->AddTrigger( i, trigger ) == false )
				{
					assert( 0 && "failed to insert trigger event" );
					return false;
				}

				/// 상황별 퀘스트 관리자
				if( p->mEventType != eQUEST_MAX )
				{
					mQuestType[p->mEventType].mTypeArr.PushBack( i );
				}
 
				/// 각 퀘스트에 대해 적합성 검사
				/// 반드시 있어야 할 정보등등..
				if( IsValid( p ) == false )
					return false;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}

	}

	return true;
}

bool cQuestManager::LoadQuestData( cParser& parser, cQuestDefine* p, cTrigger* trigger )
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
		case eTOKEN_TYPE:
			{
				unsigned char type = (unsigned char)parser.ParseInt();
				p->mType = type;
			}
			break;
		case eTOKEN_TITLE:
			{
				p->mTitleIndex = (unsigned int)parser.ParseInt();
			}
			break;
		case eTOKEN_DESC:
			{
				p->mDestIndex = (unsigned long)parser.ParseInt();
				p->mDestLineNum = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_REPEAT:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_CHALLEANGE )
					p->mRepeatType = eQUEST_CHALLENGE;
				else if( token.mType == eTOKEN_REPETITION )
					p->mRepeatType = eQUEST_REPEAT;
				else
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_GROUP:
			{
				p->mGroup = (unsigned char)parser.ParseInt();
			}
			break;
		case eTOKEN_GIVENPC:
			{
				p->mGiveNpcIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKENPC:
			{
				p->mTakeNpcIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKEMAP:
			{
				p->mTakeMapIndex = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_SELECTCOUNT:
			{
				p->mSelectCount = (short)parser.ParseInt();
			}
			break;
		case eTOKEN_LEVEL:
			{
				p->mLevel = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_INFO:
			{
				if( parser.ExpectTokenString( "{" ) == false )
					return false;

				int cnt = 0;

				while( lexer->GetNextToken( &token ) )
				{
					if( token == "}" )
						break;

					if( token.mType == eTOKEN_INFOMAP )
					{
						parser.ParseInt();
						parser.ParseInt();
						parser.ParseInt();
						parser.ParseFloat();
						parser.ParseInt();

						cnt++;
						if( cnt > MAX_DUTY )
						{
							assert(0);
							return false;
						}
					}
					else
					{
						assert( 0 && "invalid token" );
						return false;
					}
				}
			}
			break;
		case eTOKEN_DUTY:
			{
				cQuestDuty* duty = new cQuestDuty;

				if( LoadDuty( parser, duty, p->mComplete ) == false ) 
				{
					assert( 0 && "failed to load duty" );
					SAFE_DELETE( duty );
					return false;
				}

				p->mQuestDuty = duty;		
			}
			break;
		case eTOKEN_LIMIT:
			{
				cQuestLimit* limit = new cQuestLimit;

				if( LoadLimit( parser, limit ) == false ) 
				{
					assert( 0 && "failed to load limit" );
					SAFE_DELETE( limit );
					return false;
				}

				p->mQuestLimit = limit;
			}
			break;
		case eTOKEN_END:
			{
				cQuestEnd* end = new cQuestEnd;

				if( LoadEnd( parser, end ) == false )
				{
					assert( 0 && "failed to load end" );
					SAFE_DELETE( end );
					return false;
				}
				
				p->mQuestEnd = end;
			}
			break;
		case eTOKEN_DEFAULT:
			{
				cQuestReward* reward = new cQuestReward;

				if( LoadDefault( parser, reward ) == false ) 
				{
					assert( 0 && "failed to load default reward" );
					SAFE_DELETE( reward );
					return false;
				}

				p->mDefaultReward = reward;
			}
			break;
		case eTOKEN_SELECT:
			{
				cQuestReward* select = new cQuestReward;

				if( LoadSelect( parser, select ) == false ) 
				{
					assert( 0 && "failed to load select reward" );
					SAFE_DELETE( select );
					return false;
				}

				p->mSelectReward = select;
			}
			break;
		case eTOKEN_FAIL:
			{
				cQuestFail* fail = new cQuestFail;

				if( LoadFail( parser, fail ) == false ) 
				{
					assert( 0 && "failed to load fail" );
					SAFE_DELETE( fail );
					return false;
				}

				p->mQuestFail = fail;
			}
			break;
		case eTOKEN_TRIGGER:
			{
				p->mEventType = eQUEST_MAX;

				if( LoadTrigger( parser, trigger, p->mEventType ) == false )
				{
					assert( 0 && "failed to load trigger" );
					SAFE_DELETE( trigger );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}

	return true;
}

bool cQuestManager::LoadTrigger( cParser& parser, cTrigger* trigger, unsigned char& type )
{
	cLexer* lexer = parser.GetLexer();

	cToken token;
	lexer->GetNextToken( &token );

	unsigned int haveItemCount = 0;

	switch( token.mType )
	{
	case eTOKEN_TE_LEVEL:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mLowLevel = (short)parser.ParseInt();
			trigger->mHighLevel = (short)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;
		}
		break;
	case eTOKEN_TE_RACE:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			unsigned int i = 0;
			while( token != ")" )
			{
				lexer->GetNextToken( &token );

				if( i > 2 )
				{
					assert(0);
					break;
				}

				if( token.mType == eTOKEN_HUMAN )
				{
					trigger->mRace[i] = eRACE_HUMAN;
					i++;
				}
				else if( token.mType == eTOKEN_ELF )
				{
					trigger->mRace[i] = eRACE_ELF;
					i++;
				}
				else if( token.mType == eTOKEN_BEAST )
				{
					trigger->mRace[i] = eRACE_BEAST;
					i++;
				}
			}
		}
		break;
	case eTOKEN_TE_GENDER:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mGender = (short)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;
		}
		break;
	case eTOKEN_TE_JOB:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			while( token != ")" )
			{
				lexer->GetNextToken( &token );
				
				if( token.mType == eTOKEN_INT )
				{
					unsigned long jobIdx = token.ToInt();
					if( trigger->mJobSet.Insert( jobIdx ) == false )
						return false;
				}
			}
		}
		break;
	case eTOKEN_TE_HAVEITEM:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			unsigned long itemIndex = (unsigned long)parser.ParseInt();
			unsigned int count = (unsigned int)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;

			if( haveItemCount < QUEST_ITEM_MAX )
			{
				trigger->mHaveItem[haveItemCount].itemIndex = itemIndex;
				trigger->mHaveItem[haveItemCount].count = count;
				haveItemCount++;
			}
			else
				assert(0);
		}
		break;
	case eTOKEN_TE_LINKQUEST:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			while( token != ")" )
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_INT )
				{
					unsigned long questIdx = token.ToInt();
					trigger->mLinkQuestArr.PushBack( questIdx );
				}
			}
		}
		break;
	case eTOKEN_TE_DAYEVENT:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			short& cnt = trigger->mDayCount;

			while( token != ")" )
			{
				lexer->GetNextToken( &token );

				switch( token.mType )
				{
				case eTOKEN_MON:	trigger->mDayType[cnt] = eDAY_MON;	break;
				case eTOKEN_TUE:	trigger->mDayType[cnt] = eDAY_TUE;	break;
				case eTOKEN_WED:	trigger->mDayType[cnt] = eDAY_WED;	break;
				case eTOKEN_THU:	trigger->mDayType[cnt] = eDAY_THU;	break;
				case eTOKEN_FRI:	trigger->mDayType[cnt] = eDAY_FRI;	break;
				case eTOKEN_SAT:	trigger->mDayType[cnt] = eDAY_SAT;	break;
				case eTOKEN_SUN:	trigger->mDayType[cnt] = eDAY_SUN;	break;
				default:			continue;
				}

				cnt++;

				if( cnt > eDAY_MAX )
				{
					assert(0);
					return false;
				}
			}

			type = eQUEST_TIMEEVENT;
		}
		break;
	case eTOKEN_TE_TIMEEVENT:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mStartHour = (short)parser.ParseInt();
			trigger->mEndHour	= (short)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;

			type = eQUEST_TIMEEVENT;
		}
		break;
	case eTOKEN_TE_TRANSMON:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mTransmonIndex = (unsigned long)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;

			type = eQUEST_TRANSMON;
		}
		break;
	case eTOKEN_TE_SETTITLE:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mTitleIndex = (unsigned long)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;

			type = eQUEST_SETTITLE;
		}
		break;
	case eTOKEN_TE_SJOBSKILL:
		{
			if( parser.ExpectTokenString( "(" ) == false )
				return false;

			trigger->mMakeSkillIndex = (unsigned char)parser.ParseInt();
			trigger->mMakeSkillExp = (unsigned long)parser.ParseInt();
			trigger->mMaxMakeSkillExp = (unsigned long)parser.ParseInt();

			if( parser.ExpectTokenString( ")" ) == false )
				return false;
		}
		break;
	default:
		assert( 0 && "invalid token" );
		break;
	}

	return true;
}

bool cQuestManager::LoadDuty( cParser& parser, cQuestDuty* duty, __int64& complete )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	int cnt = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_TALKNPC:
			{
				/// 이미 meet 조건이 있으면 더이상 넣지 못한다 
				for( int i = 0; i < cnt; ++i )
				{
					if( duty->mDuty[i].dutyType == eDUTY_MEET )
					{
						assert(0);
						continue;
					}
				}

				unsigned long npcIdx = (unsigned long)parser.ParseInt();

				duty->mDuty[cnt].dutyType = eDUTY_MEET;
				duty->mDuty[cnt].targetIdx = npcIdx;
				duty->mDuty[cnt].count = 0;
				cnt++;
			}
			break;
		case eTOKEN_HUNT:
			{	
				unsigned long idx = (unsigned long)parser.ParseInt();

				duty->mDuty[cnt].dutyType = eDUTY_HUNT;
				duty->mDuty[cnt].targetIdx = idx;
				duty->mDuty[cnt].count = (unsigned int)parser.ParseInt();
				cnt++;
			}
			break;
		case eTOKEN_PVP:
			{
				unsigned char type = (unsigned char)parser.ParseInt();
				unsigned short pvpType = (unsigned short)parser.ParseInt();
				unsigned short count = (unsigned short)parser.ParseInt();

				duty->mDuty[cnt].count = count;
				duty->mDuty[cnt].targetIdx = pvpType;

				switch( type )
				{
				case 1:	duty->mDuty[cnt].dutyType = eDUTY_PVPJOIN;		break;
				case 2:	duty->mDuty[cnt].dutyType = eDUTY_PVPWIN;		break;
				case 3:	duty->mDuty[cnt].dutyType = eDUTY_PVPLOSE;		break;
				default:
					assert(0);
					return false;
				}

				cnt++;
			}
			break;
		case eTOKEN_ACTIVESKILL:
			{
				unsigned long skillIndex = (unsigned long)parser.ParseInt();
				unsigned long monClassIndex = (unsigned long)parser.ParseInt();

				duty->mDuty[cnt].dutyType = eDUTY_ACTIVESKILL;
				duty->mDuty[cnt].targetIdx = monClassIndex;
				duty->mDuty[cnt].skillIndex = skillIndex;
				cnt++;
			}
			break;
		case eTOKEN_ENHANCE:
			{
				duty->mDuty[cnt].dutyType = eDUTY_ENHANCE;
				duty->mDuty[cnt].enhance = true;
				cnt++;
			}
			break;
		case eTOKEN_PUTCARD:
			{
				duty->mDuty[cnt].dutyType = eDUTY_PUTCARD;
				duty->mDuty[cnt].putcard = true;
				cnt++;
			}
			break;
		case eTOKEN_COLLECT:
			{
				unsigned long idx = (unsigned long)parser.ParseInt();

				duty->mDuty[cnt].dutyType = eDUTY_COLLECT;
				duty->mDuty[cnt].targetIdx = idx;
				duty->mDuty[cnt].count = (unsigned int)parser.ParseInt();
				cnt++;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}

	/// 8개 이상의 의무는 있을수 없다 ( 금액지불 제외 )
	if( cnt > 8 )
	{
		assert(0);
		return false;
	}

	/// 완료 비트 만들기 ( 조건은 하위비트부터 채워진다 )
	complete = 0;
	for( int i = cnt-1; i >= 0; i-- )
	{
		complete = complete << 8;

		switch( duty->mDuty[i].dutyType )
		{
		case eDUTY_HUNT:
		case eDUTY_COLLECT:
		case eDUTY_PVPJOIN:
		case eDUTY_PVPWIN:
		case eDUTY_PVPLOSE:
			{
				complete |= duty->mDuty[i].count;
			}
			break;
		case eDUTY_MEET:
		case eDUTY_ACTIVESKILL:
		case eDUTY_ENHANCE:
		case eDUTY_PUTCARD:
			{
				complete |= 0x01;
			}
			break;
		}
	}
	return true;
}

bool cQuestManager::LoadLimit( cParser& parser, cQuestLimit* limit )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long buffCount = 0;
	unsigned long debuffCount = 0;
	unsigned long itemCount = 0;
	unsigned long giveitemCount = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_BUFF:
			{
				unsigned long buffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( buffIdx ) == true )
				{
					if( buffCount < QUEST_BUFF_MAX )
					{
						limit->mLimitBuff[buffCount] = buffIdx;
						buffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_DEBUFF:
			{
				unsigned long debuffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( debuffIdx ) == true )
				{
					if( debuffCount < QUEST_BUFF_MAX )
					{
						limit->mLimitDeBuff[debuffCount] = debuffIdx;
						debuffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_EQUIPITEM:
			{
				unsigned long idx = (unsigned long)parser.ParseInt();

				if( itemCount < QUEST_ITEM_MAX )
				{
					limit->mLimitEquipItem[itemCount] = idx;
					itemCount++;
				}
				else
					assert(0);
			}
			break;
		case eTOKEN_TIMETYPE:
			{
				limit->mTimeType = (unsigned char)parser.ParseInt();
			}
			break;
		case eTOKEN_TIME:
			{
				limit->mTime = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_MAPCHANGEPOS:
			{
				if( limit->mMonsterClassIndex > 0 )
				{
					assert(0);
					return false;
				}

				limit->mMapChangeIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_MONSTER:
			{
				/// 동시에 쓸수 없다
				if( limit->mMapChangeIndex > 0 )
				{
					assert(0);
					return false;
				}
				
				limit->mMonsterClassIndex = (unsigned long)parser.ParseInt();
				limit->mMonsterCount = (unsigned int)parser.ParseInt();
			}
			break;
		case eTOKEN_MONSTERTIME:
			{
				if( limit->mMapChangeIndex > 0 )
				{
					assert(0);
					return false;
				}

				limit->mMonsterTime = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_GIVEITEM:
			{
				unsigned long itemIndex = (unsigned long)parser.ParseInt();
				unsigned int count = (unsigned int)parser.ParseInt();

				if( giveitemCount < QUEST_ITEM_MAX )
				{
					limit->mGiveItem[giveitemCount].itemIndex = itemIndex;
					limit->mGiveItem[giveitemCount].count = count;
					giveitemCount++;
				}
				else
					assert(0);
			}
			break;
		case eTOKEN_TAKEMONEY:
			{
				limit->mTakeMoney = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKEFIRE:
			{
				limit->mTakeFire = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKEWATER:
			{
				limit->mTakeWater = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKEWIND:
			{
				limit->mTakeWind = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TAKEEARTH:
			{
				limit->mTakeEarth = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_TUTORIALMODE:
			{
				limit->mTutorialModeIndex = parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}
	return true;
}

bool cQuestManager::LoadEnd( cParser& parser, cQuestEnd* end )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long buffCount = 0;
	unsigned long debuffCount = 0;
	unsigned long takeitemCount = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_BUFF:
			{
				unsigned long buffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( buffIdx ) == true )
				{
					if( buffCount < QUEST_BUFF_MAX )
					{
						end->mEndBuff[buffCount] = buffIdx;
						buffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_DEBUFF:
			{
				unsigned long debuffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( debuffIdx ) == true )
				{
					if( debuffCount < QUEST_BUFF_MAX )
					{
						end->mEndDeBuff[debuffCount] = debuffIdx;
						debuffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_TAKEITEM:
			{
				unsigned long itemIndex = parser.ParseInt();
				unsigned int count = parser.ParseInt();

				if( takeitemCount < QUEST_ITEM_MAX )
				{
					end->mTakeItem[takeitemCount].itemIndex = itemIndex;
					end->mTakeItem[takeitemCount].count = count;
					takeitemCount++;
				}
				else
					assert(0);
			}
			break;
		case eTOKEN_TUTORIALMODE:
			{
				end->mTutorialModeIndex = parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}
	return true;
}

bool cQuestManager::LoadDefault( cParser& parser, cQuestReward* reward )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long buffCount = 0;
	unsigned long itemCount = 0;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_EXP:
			{
				reward->mExp = (unsigned int)parser.ParseInt();
			}
			break;
		case eTOKEN_SXP:
			{
				reward->mSxp = (unsigned int)parser.ParseInt();
			}
			break;
		case eTOKEN_MONEY:
			{
				reward->mMoney = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_SKILLPOINT:
			{
				reward->mSkillPoint = (unsigned int)parser.ParseInt();
			}
			break;
		case eTOKEN_CHANGEJOB:
			{
				reward->mChangeJobIndex = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_FORCETYPE:
			{
				reward->mForceType = (unsigned char)parser.ParseInt();
				if( reward->mForceType < eFORCETYPE_FIRE || reward->mForceType > eFORCETYPE_EARTH )
				{
					assert(0);
				}
			}
			break;
		case eTOKEN_MAPCHANGEPOS:
			{
				reward->mMapChangeIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_GIVETITLE:
			{
				reward->mGiveTitleIndex = (unsigned long)parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF:
			{
				unsigned long buffIdx = (unsigned long)parser.ParseInt();
				
				if( CheckAura( buffIdx ) == true )
				{
					if( buffCount < QUEST_BUFF_MAX )
					{
						reward->mRewardBuff[buffCount] = buffIdx;
						buffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_ITEM:
			{
				unsigned long itemIdx = (unsigned long)parser.ParseInt();
				unsigned int count = (unsigned int)parser.ParseInt();

				if( itemCount < QUEST_ITEM_MAX )
				{
					reward->mRewardItem[itemCount].itemIndex = itemIdx;
					reward->mRewardItem[itemCount].count = count;
					itemCount++;
				}
				else
					assert(0);
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}
	return true;
}

bool cQuestManager::LoadSelect( cParser& parser, cQuestReward* select )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long buffCount = 0;
	unsigned long itemCount = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_BUFF:
			{
				unsigned long buffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( buffIdx ) == true )
				{
					if( buffCount < QUEST_BUFF_MAX )
					{
						select->mRewardBuff[buffCount] = buffIdx;
						buffCount++;	
					}
					else
						assert(0);
				}
			}
			break;
		case eTOKEN_ITEM:
			{
				unsigned long itemIdx = (unsigned long)parser.ParseInt();
				unsigned int count = (unsigned int)parser.ParseInt();

				if( itemCount < QUEST_ITEM_MAX )
				{
					select->mRewardItem[itemCount].itemIndex = itemIdx;
					select->mRewardItem[itemCount].count = count;
					itemCount++;
				}
				else
					assert(0);
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}
	return true;
}

bool cQuestManager::LoadFail( cParser& parser, cQuestFail* fail )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long buffCount = 0;
	unsigned long debuffCount = 0;

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_BUFF:
			{
				unsigned long buffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( buffIdx ) == true )
				{
					if( buffCount < QUEST_BUFF_MAX )
					{
						fail->mFailBuff[buffCount] = buffIdx;
						buffCount++;	
					}
					else 
						assert(0);
				}
			}
			break;
		case eTOKEN_DEBUFF:
			{
				unsigned long debuffIdx = (unsigned long)parser.ParseInt();

				if( CheckAura( debuffIdx ) == true )
				{
					if( debuffCount < QUEST_BUFF_MAX )
					{
						fail->mFailDeBuff[debuffCount] = debuffIdx;
						debuffCount++;	
					}
					else 
						assert(0);
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			break;
		}
	}
	return true;
}

bool cQuestManager::IsValid( cQuestDefine* define )
{
	if( !define )
		return false;

	if( define->mDestLineNum == 0 )
		return false;

	if( define->mTakeNpcIndex == 0 )
		return false;

	if( define->mTakeMapIndex == ULONG_MAX )
		return false;

	if( define->mQuestDuty == 0 )
		return false;

	if( (define->mSelectCount > 0 && define->mSelectReward == 0) ||
		(define->mSelectCount == 0 && define->mSelectReward ) )
		return false;

	cQuestDuty* duty = define->mQuestDuty;
	if( duty )
	{
		for( int i = 0; i < MAX_DUTY; ++i )
		{
			if( duty->mDuty[i].dutyType == eDUTY_MEET )
			{
				/// npc 만나기 관련 의무가 있을때. 해당 npc가 완료 npc 가 아니면 에러
				if( define->mTakeNpcIndex != duty->mDuty[i].targetIdx )
					return false;
			}
		}
	}
	return true;
}

bool cQuestManager::CheckAura( unsigned long influenceIndex )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIndex );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	if( pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_AURA )
	{
		assert(0);
		return false;
	}
	return true;
}

cQuestLexer::cQuestLexer( const char* buffer, unsigned int size )
: cLexer( buffer, size )
{
	BindKeyword( "quest", eTOKEN_QUEST );
	BindKeyword( "title", eTOKEN_TITLE );
	BindKeyword( "type", eTOKEN_TYPE );
	BindKeyword( "description", eTOKEN_DESC );		/// 설명인덱스
	BindKeyword( "repeat", eTOKEN_REPEAT );			/// 반복여부
	BindKeyword( "group", eTOKEN_GROUP );			/// 그룹번호

	BindKeyword( "givenpc", eTOKEN_GIVENPC );	/// 퀘스트주는 NPC
	BindKeyword( "takenpc", eTOKEN_TAKENPC );	/// 보상 주는 NPC
	BindKeyword( "takemap", eTOKEN_TAKEMAP );	/// 보상 주는 NPC
	BindKeyword( "giveitem", eTOKEN_GIVEITEM );	/// 퀘스트 시작시 주는 아이템
	BindKeyword( "takeitem", eTOKEN_TAKEITEM );	/// 퀘스트 완료시 가져가는 아이템
	BindKeyword( "selectcount", eTOKEN_SELECTCOUNT );
	
	BindKeyword( "QUEST_CHALLENGE", eTOKEN_CHALLEANGE );
	BindKeyword( "QUEST_REPEAT", eTOKEN_REPETITION );

	BindKeyword( "QUEST_ALL", eTOKEN_ALL );
	BindKeyword( "QUEST_ORDER", eTOKEN_ORDER );

	BindKeyword( "info", eTOKEN_INFO );
	BindKeyword( "duty", eTOKEN_DUTY );
	BindKeyword( "limit", eTOKEN_LIMIT );
	BindKeyword( "end", eTOKEN_END );
	BindKeyword( "default", eTOKEN_DEFAULT );
	BindKeyword( "select", eTOKEN_SELECT );
	BindKeyword( "fail", eTOKEN_FAIL );

	BindKeyword( "map", eTOKEN_INFOMAP );
	BindKeyword( "talknpc", eTOKEN_TALKNPC );
	BindKeyword( "hunt", eTOKEN_HUNT );
	BindKeyword( "pvp", eTOKEN_PVP );
	BindKeyword( "activeskill", eTOKEN_ACTIVESKILL );
	BindKeyword( "collect", eTOKEN_COLLECT );
	BindKeyword( "enhance", eTOKEN_ENHANCE );
	BindKeyword( "putcard", eTOKEN_PUTCARD );

	BindKeyword( "level", eTOKEN_LEVEL );
	BindKeyword( "buff", eTOKEN_BUFF );
	BindKeyword( "debuff", eTOKEN_DEBUFF );
	BindKeyword( "equipitem", eTOKEN_EQUIPITEM );
	BindKeyword( "timetype", eTOKEN_TIMETYPE );
	BindKeyword( "time", eTOKEN_TIME );
	BindKeyword( "mapchangepos", eTOKEN_MAPCHANGEPOS );
	BindKeyword( "monster", eTOKEN_MONSTER );
	BindKeyword( "monstertime", eTOKEN_MONSTERTIME );
	BindKeyword( "takemoney", eTOKEN_TAKEMONEY );		/// 퀘스트 시작시 지불할 금액
	BindKeyword( "takefire", eTOKEN_TAKEFIRE );
	BindKeyword( "takewater", eTOKEN_TAKEWATER );
	BindKeyword( "takewind", eTOKEN_TAKEWIND );
	BindKeyword( "takeearth", eTOKEN_TAKEEARTH );
	BindKeyword( "tutorialmode", eTOKEN_TUTORIALMODE );

	BindKeyword( "exp", eTOKEN_EXP );
	BindKeyword( "sxp", eTOKEN_SXP );
	BindKeyword( "money", eTOKEN_MONEY );
	BindKeyword( "skillpoint", eTOKEN_SKILLPOINT );
	BindKeyword( "item", eTOKEN_ITEM );
	BindKeyword( "changejob", eTOKEN_CHANGEJOB );
	BindKeyword( "force", eTOKEN_FORCETYPE );
	BindKeyword( "givetitle", eTOKEN_GIVETITLE );

	BindKeyword( "trigger", eTOKEN_TRIGGER );	/// 퀘스트 발생 트리거
	BindKeyword( "TE_LEVEL", eTOKEN_TE_LEVEL );
	BindKeyword( "TE_RACE", eTOKEN_TE_RACE );
	BindKeyword( "TE_JOB", eTOKEN_TE_JOB );
	BindKeyword( "TE_HAVEITEM", eTOKEN_TE_HAVEITEM );
	BindKeyword( "TE_LINKQUEST", eTOKEN_TE_LINKQUEST );
	BindKeyword( "TE_DAYEVENT", eTOKEN_TE_DAYEVENT );
	BindKeyword( "TE_TIMEEVENT", eTOKEN_TE_TIMEEVENT );
	BindKeyword( "TE_TRANSMON", eTOKEN_TE_TRANSMON );
	BindKeyword( "TE_SETTITLE", eTOKEN_TE_SETTITLE );
	BindKeyword( "TE_GENDER", eTOKEN_TE_GENDER );
	BindKeyword( "TE_SJOBSKILL", eTOKEN_TE_SJOBSKILL );

	BindKeyword( "HUMAN", eTOKEN_HUMAN );
	BindKeyword( "ELF", eTOKEN_ELF );
	BindKeyword( "BEAST", eTOKEN_BEAST );

	BindKeyword( "true", eTOKEN_TRUE );
	BindKeyword( "false", eTOKEN_FALSE );

	BindKeyword( "NONE", eTOKEN_NONE );
	BindKeyword( "MON", eTOKEN_MON );
	BindKeyword( "TUE", eTOKEN_TUE );
	BindKeyword( "WED", eTOKEN_WED );
	BindKeyword( "THU", eTOKEN_THU );
	BindKeyword( "FRI", eTOKEN_FRI );
	BindKeyword( "SAT", eTOKEN_SAT );
	BindKeyword( "SUN", eTOKEN_SUN );

	BindKeyword( "npc", eTOKEN_NPC );
}

