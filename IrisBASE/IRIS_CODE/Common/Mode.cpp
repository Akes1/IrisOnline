#include "StdAfx.h"
#include ".\mode.h"

#include "Token.h"
#include "Parser.h"
#include "ModeLexer.h"

#include "BaseObject_Common.h"


//////////////////////////////////////////////////////////////////////////
cMode::cMode()
: mModeIndex( (unsigned long)(-1) )
{}

cMode::~cMode()
{
	mProcessModeInfoArr.Clear();

	///
	for( unsigned int i=0;i<mInitModeInfoArr.GetSize();i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)mInitModeInfoArr[i];
		SAFE_DELETE(p);
	}
	mInitModeInfoArr.Clear();

	///
	for( unsigned int i=0;i<mConditionModeInfoArr.GetSize();i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)mConditionModeInfoArr[i];
		SAFE_DELETE(p);
	}
	mConditionModeInfoArr.Clear();

	///
	for( unsigned int i=0;i<mCheckSkillModeInfoArr.GetSize();i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)mCheckSkillModeInfoArr[i];
		SAFE_DELETE(p);
	}
	mCheckSkillModeInfoArr.Clear();

	///
	for( unsigned int i=0;i<mCheckImmuneModeInfoArr.GetSize();i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)mCheckImmuneModeInfoArr[i];
		SAFE_DELETE(p);
	}
	mCheckImmuneModeInfoArr.Clear();
}

void cMode::PushInitData( sModeInfoBase* pBase, bool bProcess )
{
	mInitModeInfoArr.PushBack( pBase );

	if( bProcess == true )
		mProcessModeInfoArr.PushBack( pBase );
}

void cMode::PushConditionData( sModeInfoBase* pBase )
{
	mConditionModeInfoArr.PushBack( pBase );
}

void cMode::PushCheckSkill( sModeInfoBase* pBase )
{
	mCheckSkillModeInfoArr.PushBack( pBase );
}

void cMode::PushImmune( sModeInfoBase* pBase )
{
	mCheckImmuneModeInfoArr.PushBack( pBase );
}

//////////////////////////////////////////////////////////////////////////
cModeAgent::cModeAgent()
{

#ifdef _CLIENT
	mAntiRotation = false;
#endif

}

cModeAgent::~cModeAgent()
{
	{
		cModeMap::cIterator i = mModeMap.Begin();
		cModeMap::cIterator iend = mModeMap.End();
		for( ; i != iend; ++i )
		{
			delete (cMode*)(i->mSecond);
		}
		mModeMap.Clear();
	}

#ifdef _CLIENT
	{
		cModeObjMap::cIterator i = mModeObjMap.Begin();
		cModeObjMap::cIterator iend = mModeObjMap.End();
		for( ; i != iend; ++i )
		{
			delete (sModeObj*)(i->mSecond);
		}
		mModeObjMap.Clear();
	}
#else
	{
		for( unsigned long i = 0 ; i < mDeathMonsterAry.GetSize() ; ++i )
		{
			sModeDeathMonster* pDeath = (sModeDeathMonster*)mDeathMonsterAry[i];
			SAFE_DELETE( pDeath );
		}
		mDeathMonsterAry.Clear();
	}
#endif
}

bool cModeAgent::LoadFile( unsigned long monsterIdx )
{
	cString file;
#ifdef _CLIENT
	file.Format("./script/direct/m%d_mode.txt", monsterIdx );
#else
	file.Format("./script/resource/m%d_mode.txt", monsterIdx );
#endif

	/// 몬스터당 genMonster의 고유idx를 달아두고 반복 시간 설정시 참고키로 쓴다.
	unsigned long orderNumMon = 0;
	/// 몬스터당 genTotem의 고유번호
	unsigned long orderNumTotem = 0;


	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( file.Cstr() , true ) == false )
	{
		return false;
	}

	/// lexer & parser 선언
	cToken token;
	cModeLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, file );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_HEADER:
			{
				if( LoadHeader( parser ) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_MODE:
			{
				cMode* mode = new cMode;
				if( mode == 0 )
				{
					assert(0);
					return false;
				}

				/// mode index
				unsigned long modeIdx = parser.ParseInt();
				mode->SetModeIndex( modeIdx );
				if( LoadMode( mode, parser, &orderNumMon, &orderNumTotem ) == false )
				{
					assert(0);
					delete mode;
					return false;
				}

				///
				if( mModeMap.Insert( modeIdx, mode ) == false )
				{
					assert(0);
					delete mode;
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

#ifdef _CLIENT
bool cModeAgent::LoadHeader( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	sModeObj* pObj = 0;
	unsigned short idx;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_EFFECT:
			{
				idx = (unsigned short)parser.ParseInt();
				pObj = new sModeObj;
				pObj->kind = eMODEOBJ_EFFECT;

				float angleX = D3DXToRadian(parser.ParseFloat());
				float angleY = D3DXToRadian(parser.ParseFloat());
				float angleZ = D3DXToRadian(parser.ParseFloat());
				pObj->rot.FromEulerAnglesXYZ( angleX, angleY, angleZ );	

				pObj->scale = parser.ParseFloat();
				pObj->file = parser.ParseString();

				if( mModeObjMap.Insert( idx, pObj ) == false )
				{
					assert(0);
					delete pObj;
				}
			}
			break;
		case eTOKEN_SOUND:
			{
				idx = (unsigned short)parser.ParseInt();	

				pObj = new sModeObj;
				pObj->kind = eMODEOBJ_SOUND;

				pObj->soundListIndex = parser.ParseInt();

				if( mModeObjMap.Insert( idx, pObj ) == false )
				{
					assert(0);
					delete pObj;
				}
			}
			break;
		case eTOKEN_DEATHDOOR:
			{
				parser.ParseInt();
			}
			break;
		case eTOKEN_DEATHMONSTER:
			{
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
			}
			break;
		case eTOKEN_PORTAL:
			{
				parser.ParseInt();
			}
			break;
		case eTOKEN_ANTIROTATION:
			{
				mAntiRotation = true;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}
#else
bool cModeAgent::LoadHeader( cParser& parser )
{
	unsigned long orderNumMon = 0;

	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_EFFECT:
			{
				parser.ParseInt();
				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();

				parser.ParseFloat();
				parser.ParseString();
			}
			break;
		case eTOKEN_SOUND:
			{
				parser.ParseInt();
				parser.ParseInt();
			}
			break;
		case eTOKEN_DEATHDOOR:
			{
				unsigned short idx = (unsigned short)parser.ParseInt();	

				mDeathDoorAry.PushBack( idx );
			}
			break;
		case eTOKEN_DEATHMONSTER:
			{
				sModeDeathMonster* pDeathMonster = new sModeDeathMonster;
				pDeathMonster->mGenMonsterIdx = parser.ParseInt();
				pDeathMonster->mOrderNum = ++orderNumMon;
				pDeathMonster->mCount = (unsigned char)parser.ParseInt();
				pDeathMonster->mGenRangeMin = parser.ParseInt();
				pDeathMonster->mGenRangeMax = parser.ParseInt();


				mDeathMonsterAry.PushBack( pDeathMonster );
			}
			break;
		case eTOKEN_ANTIROTATION:
			break;
		case eTOKEN_PORTAL:
			{
				unsigned long idx = (unsigned long)parser.ParseInt();	

				mTeleportPortalAry.PushBack( idx );					
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}
#endif

bool cModeAgent::LoadMode( cMode* mode, cParser& parser, unsigned long *orderNumMon, unsigned long *orderNumTotem )
{
	if( mode == 0 )
	{
		assert(0);
		return false;
	}

	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_APPLYBUFF:
			{
#ifdef _CLIENT

				parser.ParseInt();
				parser.ParseInt();
#else
				sModeBuff* pInfo = new sModeBuff;
				pInfo->type = eMODEINFO_APPLYBUFF;

				pInfo->mApplyBuffIdx = parser.ParseInt();
				pInfo->mModeDel = (parser.ParseInt() == 1);

				///
				mode->PushInitData( pInfo );
#endif
			}
			break;
		case eTOKEN_APPLYAURA:
			{
#ifdef _CLIENT

				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
#else
				sModeAura* pInfo = new sModeAura;
				pInfo->type = eMODEINFO_APPLYAURA;

				pInfo->mApplyBuffIdx = parser.ParseInt();
				pInfo->mEnemyApply = (parser.ParseInt() == 1);
				pInfo->mRange = (unsigned short)parser.ParseInt();
				pInfo->mModeDel = (parser.ParseInt() == 1);

				///
				mode->PushInitData( pInfo );
#endif
			}
			break;
		case eTOKEN_GENMONSTER:
			{
#ifdef _CLIENT
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
#else
				sModeGenMonster* pInfo = new sModeGenMonster;
				pInfo->type = eMODEINFO_GENMONSTER;

				pInfo->mGenMonsterIdx = parser.ParseInt();
				pInfo->mOrderNum = ++(*orderNumMon);
				pInfo->mCount = (unsigned char)parser.ParseInt();
				pInfo->mGenRangeMin = parser.ParseInt();
				pInfo->mGenRangeMax = parser.ParseInt();
				pInfo->mLoop = (parser.ParseInt() == 1);
				pInfo->mDelayTime = parser.ParseInt();
				pInfo->mGenPatternIdx = parser.ParseInt();

				///
				mode->PushInitData( pInfo, pInfo->mLoop );
#endif
			}
			break;
		case eTOKEN_GENTOTEM:
			{
#ifdef _CLIENT
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
#else
				sModeGenTotem* pInfo = new sModeGenTotem;
				pInfo->type = eMODEINFO_GENTOTEM;

				pInfo->mGenTotemIdx = parser.ParseInt();
				pInfo->mOrderNum = ++(*orderNumTotem);
				pInfo->mCount = (unsigned char)parser.ParseInt();
				pInfo->mGenRangeMin = parser.ParseInt();
				pInfo->mGenRangeMax = parser.ParseInt();
				pInfo->mLoop = (parser.ParseInt() == 1);
				pInfo->mDelayTime = parser.ParseInt();
				pInfo->mGenPatternIdx = parser.ParseInt();
				pInfo->mApplyType = parser.ParseInt();

				///
				mode->PushInitData( pInfo, pInfo->mLoop );
#endif
			}
			break;
		case eTOKEN_GENGATHERING:
			{
#ifdef _CLIENT
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
#else
				sModeGenGathering* pInfo = new sModeGenGathering;
				pInfo->type = eMODEINFO_GENGATHERING;

				pInfo->mGenGatheringIdx = parser.ParseInt();
				pInfo->mOrderNum = ++(*orderNumTotem);
				pInfo->mCount = (unsigned char)parser.ParseInt();
				pInfo->mGenRangeMin = parser.ParseInt();
				pInfo->mGenRangeMax = parser.ParseInt();
				pInfo->mLoop = (parser.ParseInt() == 1);
				pInfo->mDelayTime = parser.ParseInt();
				pInfo->mGenPatternIdx = parser.ParseInt();

				///
				mode->PushInitData( pInfo, pInfo->mLoop );
#endif
			}
			break;
		case eTOKEN_SPEECH:
			{
#ifdef _CLIENT
				sModeSpeech* pInfo = new sModeSpeech;
				pInfo->type = eMODEINFO_SPEECH;

				pInfo->mSpeechType = (unsigned char)parser.ParseInt();
				pInfo->mSpeechIdx = parser.ParseInt();

				///
				mode->PushInitData( pInfo );
#else
				parser.ParseInt();
				parser.ParseInt();
#endif
			}
			break;
		case eTOKEN_MODESTATE:
			{
				sModeState* pInfo = new sModeState;
				pInfo->type = eMODEINFO_MODESTATE;

				pInfo->mAnimationIdx = parser.ParseInt();
				pInfo->mMaintainTime = parser.ParseInt();

				///
				mode->PushInitData( pInfo );
			}
			break;
		case eTOKEN_MODEIMAGE:
			{
#ifdef _CLIENT
				sModeImage* pInfo = new sModeImage;
				pInfo->type = eMODEINFO_IMAGE;

				pInfo->mFileName = parser.ParseString();
				pInfo->mTexX	 = parser.ParseInt();
				pInfo->mTexY	 = parser.ParseInt();
				pInfo->mTime	 = parser.ParseInt();

				mode->PushInitData( pInfo );
#else
				parser.ParseString();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
#endif
			}
			break;
		case eTOKEN_APPLYRESOURCE:
			{
#ifdef _CLIENT
				sModeResource* pInfo = new sModeResource;
				pInfo->type = eMODEINFO_APPLYRESOURCE;

				pInfo->mModeObjIdx = (unsigned short)parser.ParseInt();
				pInfo->mLoop = (parser.ParseInt() == 1);
				pInfo->mFollow = (parser.ParseInt() == 1);

				cString linkName = parser.ParseString();
				pInfo->mLinkPos = (unsigned char)GetLinkIndex( linkName );

				pInfo->mVar.x = parser.ParseFloat();
				pInfo->mVar.y = parser.ParseFloat();
				pInfo->mVar.z = parser.ParseFloat();

				///
				mode->PushInitData( pInfo );
#else
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseInt();
				parser.ParseString();

				parser.ParseFloat();
				parser.ParseFloat();
				parser.ParseFloat();
#endif
			}
			break;
		case eTOKEN_AGGRO:
			{
#ifdef _CLIENT
#else
				sModeAggro* pInfo = new sModeAggro;
				pInfo->type = eMODEINFO_AGGRO;

				///
				mode->PushInitData( pInfo );
#endif
			}
			break;
		case eTOKEN_MODEINITSKILL:
			{
#ifdef _CLIENT
#else
				sModeInitSkill* pInfo = new sModeInitSkill;
				pInfo->type = eMODEINFO_MODEINITSKILL;

				///
				mode->PushInitData( pInfo );
#endif
			}
			break;
		case eTOKEN_MODECHANGE:
			{
#ifdef _CLIENT
				parser.ParseInt();

				for( unsigned int i=0; i<2; i++ )
				{
					parser.ParseInt();
					parser.ParseInt();
				}
#else
				sModeChange* pInfo = new sModeChange;
				pInfo->type = eMODEINFO_MODECHANGE;

				pInfo->mModeIdx = parser.ParseInt();

				for( unsigned int i=0; i<2; i++ )
				{
					pInfo->mCondition[i].conType = (unsigned char)parser.ParseInt();
					pInfo->mCondition[i].conValue = parser.ParseInt();
				}

				///
				mode->PushConditionData( pInfo );
#endif
			}
			break;
		case eTOKEN_USESKILL:
			{
#ifdef _CLIENT
				parser.ParseInt();

				parser.ParseInt();
				parser.ParseInt();

				for( unsigned int i=0; i<2; i++ )
				{
					parser.ParseInt();
					parser.ParseInt();
				}
#else
				sModeSkill* pInfo = new sModeSkill;
				pInfo->type = eMODEINFO_USESKILL;

				pInfo->mSkillIdx = parser.ParseInt();

				pInfo->mModeDelayTime = parser.ParseInt();
				pInfo->mActionPer = (unsigned char)parser.ParseInt();

				for( unsigned int i=0; i<2; i++ )
				{
					pInfo->mCondition[i].conType = (unsigned char)parser.ParseInt();
					pInfo->mCondition[i].conValue = parser.ParseInt();
				}

				///
				mode->PushCheckSkill( pInfo );
#endif
			}
			break;
		case eTOKEN_IMMUNE:
			{
#ifdef _CLIENT
				parser.ParseInt();

#else
				sModeImmune* pInfo = new sModeImmune;
				pInfo->type = eMODEINFO_IMMUNE;

				pInfo->mSkillApplyType = parser.ParseInt();

				///
				mode->PushImmune( pInfo );
#endif
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

unsigned int cModeAgent::GetLinkIndex( cString str )
{
	unsigned int linkPos = 0;
	if( str.Compare( "head" ) == 0 )
		linkPos = eLINK_HEAD;
	else if( str.Compare( "body" ) == 0 )
		linkPos = eLINK_BODY;
	else if( str.Compare( "foot" ) == 0 )
		linkPos = eLINK_FOOT;
	else if( str.Compare( "rhand" ) == 0 )
		linkPos = eLINK_RHAND;
	else if( str.Compare( "lhand" ) == 0 )
		linkPos = eLINK_LHAND;
	else if( str.Compare( "larm" ) == 0 )
		linkPos = eLINK_LARM;
	else if( str.Compare( "head2" ) == 0 )
		linkPos = eLINK_HEAD2;
	else if( str.Compare( "rweapon" ) == 0 )
		linkPos = eLINK_RWEAPON;
	else if( str.Compare( "lweapon" ) == 0 )
		linkPos = eLINK_LWEAPON;
	else
	{
		//		assert(0);
		linkPos = eLINK_BODY;
	}

	return linkPos;
}

