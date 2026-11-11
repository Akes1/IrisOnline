#include "stdafx.h"
#include "DramaturgyInfo.h"

#include "Token.h"
#include "Parser.h"
#include "DramaturgyLexer.h"

#include "./client/ResourceManager.h"
#include "./client/SoundSystem.h"

cDramaturgyInfo::cDramaturgyInfo( const char* pName )
{
	mFileName = pName;
	for(unsigned int ui=0;ui<eDRAMASTATE_MAX;++ui)
	{
		mpDramaState[ui] = 0;
	}

	mType = eDRAMA_SKILL;
}

cDramaturgyInfo::~cDramaturgyInfo()
{
	for(unsigned int ui=0;ui<eDRAMASTATE_MAX;++ui)
	{
		SAFE_DELETE(mpDramaState[ui]);
	}

	cDramaObjMap::cIterator i = mDramaObjMap.Begin();
	cDramaObjMap::cIterator iend = mDramaObjMap.End();
	for( ; i != iend; ++i )
	{
		delete (sDramaObj*)(i->mSecond);
	}
	mDramaObjMap.Clear();
}

unsigned int cDramaturgyInfo::GetLinkIndex( cString str )
{
	unsigned int linkPos = 0;
// 	if( str.Compare( "head" ) == 0 )
// 		linkPos = eLINK_HEAD;
// 	else if( str.Compare( "body" ) == 0 )
// 		linkPos = eLINK_BODY;
// 	else if( str.Compare( "foot" ) == 0 )
// 		linkPos = eLINK_FOOT;
// 	else if( str.Compare( "rhand" ) == 0 )
// 		linkPos = eLINK_RHAND;
// 	else if( str.Compare( "lhand" ) == 0 )
// 		linkPos = eLINK_LHAND;
// 	else if( str.Compare( "larm" ) == 0 )
// 		linkPos = eLINK_LARM;
// 	else if( str.Compare( "head2" ) == 0 )
// 		linkPos = eLINK_HEAD2;
// 	else if( str.Compare( "rweapon" ) == 0 )
// 		linkPos = eLINK_RWEAPON;
// 	else if( str.Compare( "lweapon" ) == 0 )
// 		linkPos = eLINK_LWEAPON;
// 	else
// 	{
// 		assert(0);
// 		linkPos = eLINK_BODY;
// 	}

	return linkPos;
}

bool cDramaturgyInfo::LoadFile()
{
	cString file = "./Script/Direct/";
	file += mFileName.Cstr();

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( file.Cstr() , true ) == false )
	{
		assert(0);
		cString msg;
		msg.Format("[%s]", file.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open", MB_OK | MB_ICONERROR );
		return false;
	}

	/// lexer & parser 선언
	cToken token;
	cDramaturgyLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
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
				if( LoadHeader(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_CASTING:
			{
				if( LoadContents( eDRAMASTATE_CASTING, parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_ACTIVITY:
			{
				if( LoadContents( eDRAMASTATE_ACTIVITY, parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_APPLY:
			{
				if( LoadContents( eDRAMASTATE_APPLY, parser) == false )
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

	return true;
}

bool cDramaturgyInfo::LoadHeader( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		cString msg;
		msg.Format("[%s]", parser.GetFileName() );
		MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	sDramaObj* pObj = 0;
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
		case eTOKEN_TYPE:
			{
				mType = (eDRAMA_TYPE)parser.ParseInt();
			}
			break;
		case eTOKEN_EFFECT:
			{
				idx = (unsigned short)parser.ParseInt();
				pObj = new sDramaObj;
				pObj->kind = eDRAMAOBJ_EFFECT;

				float angleX = D3DXToRadian(parser.ParseFloat());
				float angleY = D3DXToRadian(parser.ParseFloat());
				float angleZ = D3DXToRadian(parser.ParseFloat());
				pObj->rot.FromEulerAnglesXYZ( angleX, angleY, angleZ );

				pObj->scale = parser.ParseFloat();
				pObj->file = parser.ParseString();


				if( mDramaObjMap.Insert( idx, pObj ) == false )
				{
					assert(0);
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "fail to insert map", MB_OK | MB_ICONERROR );
				}
				else
				{
					// 맵 이미지 로딩 루틴 추가 [12/14/2009 Jo_Client]
					cString path;
					path.Format( "./Data/effect/%s", pObj->file.Cstr() );
					if( RESOURCEMAN->LoadNIF( path, false ) == false )
					{
						assert(0);
						cString msg;
						msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
						MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
					}
				}
			}
			break;
		case eTOKEN_SOUND:
			{
				idx = (unsigned short)parser.ParseInt();

				pObj = new sDramaObj;
				pObj->kind = eDRAMAOBJ_SOUND;

				pObj->soundListIndex = parser.ParseInt();

				if( mDramaObjMap.Insert( idx, pObj ) == false )
				{
					assert(0);
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "fail to insert map", MB_OK | MB_ICONERROR );
				}
				else
				{
					sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( pObj->soundListIndex );
					if( pInfo )
					{
						NiAudioSource* check = RESOURCEMAN->LoadTotalSound( pInfo->mFileName.Cstr() );
						check;
						assert(check);
					}
				}
			}
			break;
		case eTOKEN_TRAILTEX:
			{
				idx = (unsigned short)parser.ParseInt();
				pObj = new sDramaObj;
				pObj->kind = eDRAMAOBJ_TRAILTEX;

				pObj->file = parser.ParseString();

				if( mDramaObjMap.Insert( idx, pObj ) == false )
				{
					assert(0);
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "fail to insert map", MB_OK | MB_ICONERROR );
				}
 				else
 				{
 					// 맵 이미지 로딩 루틴 추가 [12/14/2009 Jo_Client]
 					cString path;
 					path.Format( "./Data/2ddata/%s", pObj->file.Cstr() );
 					RESOURCEMAN->LoadTexture( path );
 				}
			}
			break;
		default:
			{
				assert( 0 && "invalid token" );
				cString msg;
				msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
				MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
			}
			return false;
		}
	}

	return true;
}

bool cDramaturgyInfo::LoadContents( eDRAMASTATE state, cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		cString msg;
		msg.Format("[%s]", parser.GetFileName() );
		MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
		return false;
	}

	if( state >= eDRAMASTATE_MAX )
	{
		assert(0);
	}

	cDramaState* pDrama = new cDramaState;

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
			goto error;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_ANI:
			{
				pDrama->mAnimationIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_ACTION:
			{
				sDramaAction* pAct = new sDramaAction;
				pAct->actionType = eDRAMA_ACTION;

				pAct->weaponType = 0;
				pAct->time = parser.ParseInt();
				pAct->lifeTime = parser.ParseInt();

				pAct->objIdx = (unsigned short)parser.ParseInt();

				pAct->bLoop = (parser.ParseInt() == 1);
				pAct->bFollow = (parser.ParseInt() == 1);

				cString linkName = parser.ParseString();
				pAct->linkPos = GetLinkIndex( linkName );

				pAct->var.x = parser.ParseFloat();
				pAct->var.y = parser.ParseFloat();
				pAct->var.z = parser.ParseFloat();

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_ACTIONEX:
			{
				sDramaAction* pAct = new sDramaAction;
				pAct->actionType = eDRAMA_ACTION;

				pAct->weaponType = (unsigned char)parser.ParseInt();
				pAct->time = parser.ParseInt();
				pAct->lifeTime = parser.ParseInt();

				pAct->objIdx = (unsigned short)parser.ParseInt();

				pAct->bLoop = (parser.ParseInt() == 1);
				pAct->bFollow = (parser.ParseInt() == 1);

				cString linkName = parser.ParseString();
				pAct->linkPos = GetLinkIndex( linkName );

				pAct->var.x = parser.ParseFloat();
				pAct->var.y = parser.ParseFloat();
				pAct->var.z = parser.ParseFloat();

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_SOUNDPLAY:
			{
				sDramaAction* pAct = new sDramaAction;
				pAct->actionType = eDRAMA_ACTION;

				pAct->time = parser.ParseInt();

				pAct->objIdx = (unsigned short)parser.ParseInt();

				pAct->bLoop = (parser.ParseInt() == 1);

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_ACTIVE:
			{
				sDramaActionBase* pAct = new sDramaActionBase;
				pAct->actionType = eDRAMA_ACTIVE;
				pAct->time = parser.ParseInt();

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_BULLET:
			{
				/// 발사체 정보 저장
				sBulletAction* pAct = new sBulletAction;
				pAct->actionType = eDRAMA_BULLET;

				pAct->time = parser.ParseInt();

				pAct->bulletID = (unsigned short)parser.ParseInt();

				/// 추후에 고정값으로 변경
				pAct->speed = parser.ParseFloat();

				pAct->bulletEffectIdx = (unsigned short)parser.ParseInt();

				cString linkName = parser.ParseString();
				pAct->linkPos = GetLinkIndex( linkName );

				pAct->bulletVar.x = parser.ParseFloat();
				pAct->bulletVar.y = parser.ParseFloat();
				pAct->bulletVar.z = parser.ParseFloat();

				linkName = parser.ParseString();
				pAct->targetLinkPos = GetLinkIndex( linkName );

				/// 참조데이터 추가
				if( mBulletMap.Insert( pAct->bulletID, pAct ) == false )
				{
					assert(0);
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "fail to insert map", MB_OK | MB_ICONERROR );
				}

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_CRASH:
			{
				unsigned short id = (unsigned short)parser.ParseInt();
				unsigned short objectid = (unsigned short)parser.ParseInt();

				cString linkName = parser.ParseString();
				unsigned int linkpos = GetLinkIndex( linkName );

				NiPoint3 var;
				var.x = parser.ParseFloat();
				var.y = parser.ParseFloat();
				var.z = parser.ParseFloat();

				sBulletAction* pAct = (sBulletAction*)mBulletMap.GetAt( id );
				if( pAct )
				{
					/// 하나의 발사체에 충돌 이펙트는 최대 5개까지 등록 가능하다.
					pAct->existCrash = true;

					unsigned int count = pAct->crashCount;
					if( count >= 5 )
					{
						assert(0);
						break;
					}
					pAct->crashInfo[count].crashEffectIdx = objectid;
					pAct->crashInfo[count].crashLinkPos = linkpos;
					pAct->crashInfo[count].crashVar = var;

					pAct->crashCount++;
				}
			}
			break;
		case eTOKEN_DAMAGE:
			{
				sDamageAction* pAct = new sDamageAction;
				pAct->actionType = eDRAMA_DAMAGE;
				pAct->time = parser.ParseInt();

				pAct->percent = parser.ParseFloat() / 100.0f;

				/// 추가
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_TRAIL:
			{
				sTrailAction* pAct = new sTrailAction;
				pAct->actionType = eDRAMA_TRAIL;
				pAct->time = parser.ParseInt();

				pAct->lifeTime = parser.ParseInt();

				pAct->texIdx = (unsigned short)parser.ParseInt();

				cString linkName = parser.ParseString();
				pAct->linkStart = GetLinkIndex( linkName );

				linkName = parser.ParseString();
				pAct->linkEnd = GetLinkIndex( linkName );

				pAct->lengthPer = parser.ParseFloat() / 100.0f;
				pAct->speedFactor = parser.ParseFloat();

				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_SHAKE:
			{
				sDramaActionBase* pAct = new sDramaActionBase;
				pAct->actionType = eDRAMA_SHAKE;
				pAct->time = parser.ParseInt();
				pDrama->Push(pAct);
			}
			break;
		case eTOKEN_FADEOUT:
			{
				sDramaActionBase* pAct = new sDramaActionBase;
				pAct->actionType = eDRAMA_FADEOUT;
				pAct->time = parser.ParseInt();
				pDrama->Push(pAct);
			}
			break;
		default:
			assert( 0 && "invalid token" );
			goto error;
		}
	}

	if( mpDramaState[state] != 0 )
	{
		assert(0);
	}

	mpDramaState[state] = pDrama;

	return true;

error:
	SAFE_DELETE(pDrama);
	cString msg;
	msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}

sDramaActionBase*	cDramaturgyInfo::GetStartAction( eDRAMASTATE state )
{
	assert(state < eDRAMASTATE_MAX);
	if( mpDramaState[state] == 0 )
		return 0;
	return mpDramaState[state]->GetStart();
}

unsigned int cDramaturgyInfo::GetAnimation( eDRAMASTATE state )
{
	assert(state < eDRAMASTATE_MAX);
	if( mpDramaState[state] == 0 )
		return 0;
	return mpDramaState[state]->mAnimationIdx;
}

sDramaObj* cDramaturgyInfo::GetDramaObj( unsigned short idx )
{
	return (sDramaObj*)mDramaObjMap.GetAt( idx );
}

void cDramaturgyInfo::LoadDramaObj()
{
	sDramaObj* pobj = 0;

	cDramaObjMap::cIterator i = mDramaObjMap.Begin();
	cDramaObjMap::cIterator iend = mDramaObjMap.End();
	for( ; i != iend; ++i )
	{
		pobj = (sDramaObj*)(i->mSecond);
		if( pobj )
		{
			switch(pobj->kind)
			{
			case eDRAMAOBJ_EFFECT: 
				{
					cString pathName;
					pathName.Format( "./Data/Effect/%s", pobj->file.Cstr() );
					bool check = RESOURCEMAN->LoadNIF( pathName );
					if( check == false )
					{
						assert(0);
						MessageBoxA( NULL, pathName.Cstr(), "Loading Error", MB_OK | MB_ICONERROR );
					}
				}
				break;
			case eDRAMAOBJ_SOUND:
				{
 					sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( pobj->soundListIndex );
 					if( pInfo )
 					{
						NiAudioSource* check = RESOURCEMAN->LoadTotalSound( pInfo->mFileName.Cstr() );
						check;
						assert(check);
					}
				}
				break;
			case eDRAMAOBJ_TRAILTEX:
				{
					cString pathName;
					pathName.Format( "./Data/2DData/%s", pobj->file.Cstr() );
					RESOURCEMAN->LoadD3DTexture( pathName );
				}
				break;
			}
		}
	}
}

void cDramaturgyInfo::LoadThreadDramaObj()
{

}
