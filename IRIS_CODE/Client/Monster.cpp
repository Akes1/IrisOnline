#include "stdafx.h"
#include "monster.h"

#include "SceneManager.h"
#include "MonsterSceneNode.h"

#include "GameResourceManager.h"

#include "Application.h"
#include "WorldManager.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "Skill_Common.h"
#include "SkillManager.h"
#include "MonsterScript.h"

#include "RangeCheck.h"
#include "ChatManager.h"
#include "DamagePrintManager.h"
#include "DramaturgyManager.h"

#include "EffectSceneNode.h"
#include "SoundSceneNode.h"
#include "mode.h"
#include "UIManager.h"
#include "ChatBubble.h"
#include "ResourceManager.h"
#include "RenderSystem.h"
#include "PlaneObject.h"

cMonster::cMonster( unsigned char type )
: cBaseObject( type )
, mMoveOverTime( 0 )
, mpMonsterInfo(0)
, mAutoDummy(0)
{
	mGotoX = 0.0f;
	mGotoY = 0.0f;

	mDesiredDir = -NiPoint3::UNIT_Y;

	mPathIndex = 0;
	mPathCount = 0;

	mHP = 0;
	mMP = 0;
	mMaxHP = 0;
	mMaxMP = 0;

	mState = eOBJECT_STATE_IDLE;

	mMonsterDieTime = 0;

	mLastGotoX = 0;
	mLastGotoY = 0;

	mMoveStopRange = 0.0f;

	mDelay = 0;
	mAngle = 0.0f;
	mAccumAngle = 0.0f;
	mAxis = NiPoint3::UNIT_Z;

	mDamageAfterAnim = M_ANITYPE_IDLE1;

	mMonsterIdx = 0;
	mMoveSpeed = 0;
	mFixedObjectSize = 0;

	mMoveSpeedType = eMOVESPEED_WALK;

	mCurrentModeIndex = 0;
	mpImageMode = 0;
	mpImagePlane = 0;

	mImageStartAccumTime = ULONG_MAX;
	mImageEndAccumTime = 0;

	mPvPTeamType = ePVPDM_TEAMTYPE_MAX;
}

cMonster::~cMonster()
{
	UIMAN->ClearMonModeImage( mpImagePlane );
	SAFE_DELETE( mpImagePlane );
}

bool cMonster::Create( sMonsterData* baseInfo, NiPoint3 pos, NiMatrix3 rotate )
{
	assert(baseInfo);

	/// 몬스터 스크립트 파일에서 정보를 읽어온다.
	sMonsterScript* pMonsterInfo;
	pMonsterInfo = MONSTERSCRIPT->GetMonsterListInfo( baseInfo->mMonsterClassIdx );
	if( pMonsterInfo == NULL )
	{
		MessageBox(NULL,_T("sMonsterScript ERROR"),_T("ERROR"),MB_OK);
		assert(NULL);
		return false;
	}

	mMonsterIdx = baseInfo->mMonsterIdx;
	
	//	mPos.X = sPos.x;
	//	mPos.Y = sPos.y;

	/// 몬스터 스크립트에서 읽어온 몬스터 정보 저장
	mpMonsterInfo = pMonsterInfo;

	mpModeAgent = MONSTERSCRIPT->GetMonsterModeAgent( baseInfo->mMonsterClassIdx );

	///// 몬스터 기본 설정값
	mMaxHP = baseInfo->mMaxHP;
	mMaxMP = baseInfo->mMaxMP;
	mHP = baseInfo->mHP;
	mMP = baseInfo->mMP;
	mMoveSpeed = baseInfo->mMoveSpeed;
	mAutoDummy = baseInfo->mAutoDummy;

	float scale = 1.0f;
	if( mpMonsterInfo->mMonsterScale > 0.0f )
	{
		scale = mpMonsterInfo->mMonsterScale;
	}
	else
	{
		assert(0);
	}

	mFixedObjectSize = mpMonsterInfo->mMonsterFixSize * scale; 
	mFixedObjectSizePer = baseInfo->mScale;


	const char* strFile = GAMERESOURCEMAN->GetModelFileName( pMonsterInfo->mKfmIdx );
	assert( strFile );

	cMonsterSceneNodeParam param;
	param.mpObject = this;
	param.mTranslate = pos;
	param.mRotate = rotate;
	param.mScale = scale * mFixedObjectSizePer * 0.01f;
	param.mHeadHeight = (float)mpMonsterInfo->mNameHeight;

	cString pathName;
	pathName.Format( "./Data/Monster/%s", strFile );
	param.mPathName = pathName;
	param.mMonClassIndex = GetMonsterClassIdx();

	cMonsterSceneNode* pSceneNode = SCENEMAN->CreateMonster( param );

	if( !pSceneNode )
	{
		assert(0);
		return false;
	}
	mpObjectSceneNode = pSceneNode;

	mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE1 );

	/// 수정요!
//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetWorldScale();

	mpObjectSceneNode->ShowNameGauge( true );
	mpObjectSceneNode->UpdateNameCardTextValue();
	mpObjectSceneNode->UpdateNameCardGaugeValue();

	mDesiredDir = rotate*mDesiredDir;
	mDesiredDir.Unitize();

	mGotoX = pos.x;
	mGotoY = pos.y;

	mDelay = 0;
	mAngle = 0.0f;
	mAccumAngle = 0.0f;
	mAxis = NiPoint3::UNIT_Z;

	mPvPTeamType = ePVPDM_TEAMTYPE_MAX;

	return true;
}

void cMonster::Update( unsigned long deltaTime, unsigned long accumTime )
{
	if( mAngle )
	{
		float angle = (float)deltaTime/(float)mDelay *mAngle;
		if( mAccumAngle + angle >= mAngle )
		{
			angle = mAngle - mAccumAngle;

			mAngle = 0;
			mAccumAngle = 0;
		}
		else
			mAccumAngle += angle;

		NiMatrix3 mat;
		if( mAxis.z < 0.0f )
			mat.FromEulerAnglesXYZ( 0.0f, 0.0f, -angle );
		else
			mat.FromEulerAnglesXYZ( 0.0f, 0.0f, angle );

		mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*mat );

		mDesiredDir = mat * mDesiredDir;
		mDesiredDir.z = 0.0f;
		mDesiredDir.Unitize();
	}

	/// HERO와의 거리체크
	/// 모드 이미지 출력용
	if( mState != eOBJECT_STATE_DIE )
	{
		if( mpImageMode )
		{
			// 일정시간 후에 발생 ( 1번만 발생 )
			if( accumTime >= mImageStartAccumTime )
			{
				NiPoint3 heroPos = HERO->GetPos();
				heroPos.z = 0.0f;

				NiPoint3 pos = GetPos();
				pos.z = 0.0f;

				float tempRange = OBJECTMANAGER->ObjectSizeRange( this, HERO, MODEIMAGE_VALID_DISTANCE );

				// HERO 와 일정거리 이하가 되면
				cRangeCheck rangeCheck( tempRange );
				if( rangeCheck.IsRange( pos, heroPos ) )
				{
					// 모드 이미지 표시
					cString path;
					path.Format("./language/%s/%s", cApplication::mLangaugeFolder.Cstr(), mpImageMode->mFileName.Cstr() );
					NiTexture* tex = RESOURCEMAN->LoadMapTexture( path, false );
					if( tex )
					{
						int tx = mpImageMode->mTexX;
						int ty = mpImageMode->mTexY;
						int x = (int)((RENDERSYS->GetScreenWidth() - MODEIMAGE_W) * 0.5);

						mpImagePlane = new cPlaneObject;
						if( mpImagePlane->Create( tex, x, MODEIMAGE_Y, MODEIMAGE_W, MODEIMAGE_H, tx, ty, tx + MODEIMAGE_W, ty + MODEIMAGE_H ) == false )
						{
							assert( 0 && "failed to create monster mode image");
						}

						UIMAN->SetMonModeImage( mpImagePlane );
					}
					else
						assert(0);

					// 삭제 시간 설정
					mImageEndAccumTime = mImageStartAccumTime + MODEIMAGE_TIME;
				}

				mImageStartAccumTime = ULONG_MAX;
			}

			// 일정시간 후 삭제
			if( accumTime >= mImageEndAccumTime )
			{
				UIMAN->ClearMonModeImage( mpImagePlane );
				SAFE_DELETE( mpImagePlane );

				mpImageMode = 0;
				mImageStartAccumTime = ULONG_MAX;
				mImageEndAccumTime = 0;
			}
		}
	}

	switch( mState )
	{
	case eOBJECT_STATE_IDLE:
		{
			mIdleTime += deltaTime;
			if( mIdleTime >= 5000 )
			{
				mIdleTime = 0;
				mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE2 );
			}		
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			if( mPathCount == 0 )
				return;

			/// 이동중이면
			NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

			/// 몬스터 이동 속도
			float speed = (float)mMoveSpeed;

			/// 방향 설정
			NiPoint2 dir1;
			dir1.x = mGotoX - pos.x;
			dir1.y = mGotoY - pos.y;
			dir1.Unitize();

			const unsigned long limitOverTime = 10;
			unsigned long plusTime;

			/// 목적지점 넘어서 저장된 시간을 이동시 +
			if( mMoveOverTime > limitOverTime )
				plusTime = limitOverTime;
			else
				plusTime = mMoveOverTime;

			mMoveOverTime = mMoveOverTime - plusTime;

			/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
			pos.x += dir1.x * speed * ((deltaTime + plusTime) *0.001f);
			pos.y += dir1.y * speed * ((deltaTime + plusTime) *0.001f);
			pos.z = 0.0f;

			/// 목적지 좌표와 예상 이동 위치를 비교
			NiPoint2 dir2;
			dir2.x = mGotoX - pos.x;
			dir2.y = mGotoY - pos.y;
			dir2.Unitize();

			if( dir2.Dot(dir1) <= 0.0f )
			{
				NiPoint2 overPoint( pos.x, pos.y );
				NiPoint2 nextPos( mGotoX, mGotoY );

				/// 길찾기 경로 목적 지점을 넘어선 경우 넘은 거리만큼 시간 보관
				float overLength = ( overPoint - nextPos ).Length();
				mMoveOverTime = mMoveOverTime + (unsigned long)((overLength/speed)*1000.0f);

				pos.x = mGotoX;
				pos.y = mGotoY;
				if( mPathIndex < mPathCount-1 )
				{
					NiPoint2& nextPos = mPathArray[++mPathIndex];
					Move( nextPos.x, nextPos.y );
				}
				else
				{
					mPathCount = 0;
					mPathIndex = 0;
					mMoveOverTime = 0;
					SetState( eOBJECT_STATE_IDLE );
				}
			}

			/// 몬스터가 목적지에서 근처(일정범위)까지 도달하면 정지하는 모듈
			if( mMoveStopRange != 0.0f )
			{
				/// 몬스터가 가려고하는 경로의 최종 목적지
				NiPoint3 targetPos;
				NiPoint3 monsterGuessPos( pos.x, pos.y, 0.0f );
				/// 높이값 계산
				if( WORLDMAN->CalcHeight( &monsterGuessPos.z, monsterGuessPos.x, monsterGuessPos.y ) == false )
					assert(NULL);
				
				cBaseObject* pBaseObject = GetTargetObject();
				if( pBaseObject == NULL || pBaseObject->GetState() == eOBJECT_STATE_DIE )
				{
					targetPos.x = mLastGotoX;
					targetPos.y = mLastGotoY;
					if( WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y ) == false )
						assert(NULL);
				}
				else
				{
					targetPos.x = pBaseObject->GetXPos();
					targetPos.y = pBaseObject->GetYPos();
					targetPos.z = pBaseObject->GetZPos();
				}

				float moveStopRangeFix = OBJECTMANAGER->ObjectSizeRange( this, pBaseObject, mMoveStopRange );

				/// 최종 목적지주변에 일정 범위에서 서야할 사이즈
				cRangeCheck rangecheck( moveStopRangeFix );

				/// 최종 목적지와 현재 위치의 거리를 비교
				if( rangecheck.IsRange( monsterGuessPos, targetPos ) )
				{
					/// 좌표 보정
					NiPoint3 p;

					NiPoint3 d = GetPos() - targetPos;
					d.Unitize();

					p.x = targetPos.x + ((moveStopRangeFix-1.0f)*d.x);
					p.y = targetPos.y + ((moveStopRangeFix-1.0f)*d.y);

					if( WORLDMAN->CalcHeight( &p.z, p.x, p.y ) )
					{
						SetPos(p);
						mLastGotoX =  p.x;
						mLastGotoY =  p.y;
					}
					else
					{
						assert(0);
					}

					mMoveStopRange = 0.0f;
					SetState( eOBJECT_STATE_IDLE );

					return;
				}
			}

			/// 높이값 계산
			if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
			{
				SetPos( pos );
			}
			else
			{
				assert( 0 );
				SetState( eOBJECT_STATE_IDLE );
			}

		}break;
	case eOBJECT_STATE_PUSHPULL:
		{
			/// 이동중이면
			NiPoint3 pos = mpObjectSceneNode->GetWorldTranslate();

			/// 몬스터 이동 속도
			float speed = SKILL_PUSHPULL_SPEED;

			/// 방향 설정
			NiPoint2 dir1;
			dir1.x = mGotoX - pos.x;
			dir1.y = mGotoY - pos.y;
			dir1.Unitize();

			/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
			pos.x += dir1.x * speed * ( deltaTime * 0.001f );
			pos.y += dir1.y * speed * ( deltaTime * 0.001f );
			pos.z = 0.0f;

			/// 목적지 좌표와 예상 이동 위치를 비교
			NiPoint2 dir2;
			dir2.x = mGotoX - pos.x;
			dir2.y = mGotoY - pos.y;
			dir2.Unitize();

			if( dir2.Dot(dir1) <= 0.0f )
			{
				pos.x = mGotoX;
				pos.y = mGotoY;

				mPathCount = 0;
				mPathIndex = 0;
				mMoveOverTime = 0;
				SetState( eOBJECT_STATE_IDLE );
			}

			/// 높이값 계산
			if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
			{
				SetPos( pos );
			}
			else
			{
				assert( 0 );
				SetState( eOBJECT_STATE_IDLE );
			}
		}
		break;
		/// 몬스터 상태 죽음
	case eOBJECT_STATE_DIE:
		{
			/// 몬스터가 죽은후 3초후에 몬스터 삭제리스트에 포함해 객체 제거함
			if( mMonsterDieTime+3000 < accumTime && mMonsterDieTime != 0 )
			{
				if( HERO->GetTargetObject() == this )
					HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );

				OBJECTMAN->AddMonsterDie( GetObjectID() );
			}
			/// 몬스터가 죽은후 2초후에 알파 블랜딩 시작
			else if( mMonsterDieTime+2000 < accumTime && mMonsterDieTime != 0 )
			{
				mpObjectSceneNode->SetAlphaBlended( 0.01f );
			}

			// 초기화
			if( mpImageMode )
			{
				UIMAN->ClearMonModeImage( mpImagePlane );
				SAFE_DELETE( mpImagePlane );
			}
			
			mCurrentModeIndex = 0;
			mpImageMode = 0;
			mImageStartAccumTime = ULONG_MAX;
			mImageEndAccumTime = 0;
		}
		break;
	default:
		break;
	}
	/// 애니메이션 연결에 대한 처리
	Interpret( accumTime );
}



bool cMonster::Move( float x, float y )
{
	/// 추가: 부드러운 방향 전환을 넣도록 한다.

	NiPoint3 kWorldTranslate = NiPoint3(x, y, 0.0f);
	NiPoint3 kCurTranslate = mpObjectSceneNode->GetWorldTranslate();
	kCurTranslate.z = 0.0f;

	NiPoint3 kAt = kCurTranslate - kWorldTranslate;

	if( kAt == NiPoint3::ZERO )
		return false;

	if( kAt.SqrLength() < 0.001f )
	{
		return false;
	}

	kAt.Unitize();

	float fDot = mDesiredDir.Dot(-kAt);

	if( fDot < 1.0f - FLT_EPSILON )//0.999999f)
	{
		float fAngle = NiACos(fDot);
		if(mDesiredDir.Cross(-kAt).z > 0)
			fAngle = -fAngle;

		mDesiredDir = -kAt;

		NiMatrix3 kZRot;
		kZRot.MakeZRotation(fAngle);
		mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*kZRot );
	}

	// Animation Setting - Run
	SetState( eOBJECT_STATE_MOVE );

	/// 목적지 설정
	mGotoX = x;
	mGotoY = y;

	return true;
}



void cMonster::Interpret( unsigned long time )
{
	cActorManagerForPartition*	pActor = mpObjectSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpObjectSceneNode->GetTargetAnimation();

	/// 현재 애니메이션 종료 시점 획득
	float endTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( endTime == NI_INFINITY )
		return;
	if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		endTime = mpObjectSceneNode->GetTargetAnimationScaleEndTime();
	}

	float checkTime = (float)mpObjectSceneNode->GetScaleAccumTime() * 0.001f;
	if( endTime > 0 && endTime <= checkTime )
	{
		if( M_ANITYPE_IDLE1 == id )
		{
			return;
		}
		else if( M_ANITYPE_IDLE2 == id )
		{
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE1 );
		}
		else if( M_ANITYPE_ATTACK1 <= id && id <= M_ANITYPE_ATTACK3 )
		{
			SetState( eOBJECT_STATE_IDLE );
		}
		else if( M_ANITYPE_SKILL1 <= id && id <= M_ANITYPE_SKILL2 )
		{
			SetState( eOBJECT_STATE_IDLE );
		}
		else if( id == M_ANITYPE_DAMAGE1 || id == M_ANITYPE_DAMAGE2 )
		{
			/// 공격받는 애니메이션이 끝나면 기본 애니메이션으로 바꾼다.
			if( mDamageAfterAnim == M_ANITYPE_CASTING )
			{
				UpdateSkillAnimation( M_ANITYPE_CASTING );
			}
			else
				SetState( eOBJECT_STATE_IDLE );
		}
		else
		{
			NiSequenceData* seq = pActor->GetSequenceData( id );
//			NiControllerSequence* seq = pActor->GetSequence( id );
			if( seq )
			{
				if( seq->GetCycleType() != NiTimeController::LOOP )
					SetState( eOBJECT_STATE_IDLE );
			}
			else
			{
				assert(0);
				SetState( eOBJECT_STATE_IDLE );
			}
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		/// INVALID_TIME에 대한 보정
	}
}

unsigned int cMonster::SetLinkdObject( unsigned int num, char* nif )
{
	/// 객체 링크를 활성화한다.
	return mpObjectSceneNode->LinkObject( num, nif );
}



void cMonster::UnLinkdObject( unsigned int num )
{
	/// 객체 링크를 비활성화한다.
	mpObjectSceneNode->UnLinkObject( num );
}



bool cMonster::SetDesiredDir( NiPoint3 CurrentPos, NiPoint3 TargetPos, unsigned long delay )
{
	if( GetState() == eOBJECT_STATE_DIE )
		return true;

	CurrentPos.z = 0;
	TargetPos.z = 0;

	/// 현재 캐릭터의 방향
	NiPoint3 charDir = mDesiredDir;

	/// 바라봐야할 방향
	NiPoint3 viewDir = TargetPos - CurrentPos;
	if( viewDir.SqrLength() < 0.001f )	
	{
//		assert(0);
		return false;
	}
	viewDir.Unitize();

	if( mDesiredDir == viewDir )
	{
		mDelay = 0;
		mAngle = 0;
		mAccumAngle = 0;

		return true;
	}

	/// 회전 Matrix 산출
	NiMatrix3 matA, matB, matRot;
	matA.SetCol( 0, charDir );
	matA.SetCol( 1, -NiPoint3::UNIT_Z );
	matA.SetCol( 2, charDir.UnitCross( -NiPoint3::UNIT_Z ) );

	matB.SetCol( 0, viewDir );
	matB.SetCol( 1, -NiPoint3::UNIT_Z );
	matB.SetCol( 2, viewDir.UnitCross( -NiPoint3::UNIT_Z ) );

	matRot = matB * matA.Transpose();

	if( delay == 0 )
	{
		mpObjectSceneNode->SetRotate( mpObjectSceneNode->GetRotate()*matRot );
		mDesiredDir = viewDir;

		mDelay = 0;
		mAngle = 0;
		mAccumAngle = 0;
	}
	else
	{
		mDelay = delay;
		mAccumAngle = 0;
		matRot.ExtractAngleAndAxis(mAngle, mAxis.x, mAxis.y, mAxis.z);

		if( mAngle > 360.0f )
			mAngle -= 360.0f;
		else if( mAngle < 0.0f )
			mAngle += 360.0f;
	}

	return true;
}



void cMonster::SetPathArray( NiPoint2* moveArray, unsigned int count )
{
	assert( moveArray );

	count = (count < MAX_PATH_COUNT) ? count : MAX_PATH_COUNT;

	for( unsigned int i = 0; i < count; ++i )
	{
		mPathArray[i].x = moveArray[i].x;
		mPathArray[i].y = moveArray[i].y;
	}

	mPathIndex = 0;
	mPathCount = count;

	if( mPathCount )
	{
		Move( mPathArray[0].x, mPathArray[0].y );
		SetLastGoto( mPathArray[count-1].x, mPathArray[count-1].y );
	}
}



void cMonster::SetState( unsigned char state, bool bShowAni ) 
{ 
	if( mState != eOBJECT_STATE_IDLE && state == eOBJECT_STATE_IDLE )
		mIdleTime = 0;

	if( mState ==eOBJECT_STATE_DIE )
		return;

	mState = (eOBJECTSTATE)state;

	if( mState != eOBJECT_STATE_ATTACK )
	{
		/// 기본 속도 보정
		if( mpObjectSceneNode )
			mpObjectSceneNode->UpdateAniScaleFactor( 1.0f );
	}

	switch( mState )
	{
	case eOBJECT_STATE_DIE:
		{
			/// 
			if( bShowAni )
				mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DIE1 );
			/// 죽은 시간 기록 - 죽은 시간으로 2초 후부터 알파, 3초후 소멸
			mMonsterDieTime = THEAPP->GetWorldAccumTime();
		}
		break;
	case eOBJECT_STATE_IDLE:
	case eOBJECT_STATE_STOP:
		{
			/// idle1 or idle2
			if( bShowAni )
				mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE1 );
		}
		break;
	case eOBJECT_STATE_MOVE:
		{
			switch( mMoveSpeedType )
			{
			case eMOVESPEED_WALK: mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_WORK ); break;
			case eMOVESPEED_RUN: mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_RUN ); break;
			}
		}
		break;
	case eOBJECT_STATE_PUSHPULL:
		{
			if( bShowAni )
				mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DAMAGE1 );
		}
		break;
	}
}

void cMonster::HPChange( unsigned long objectHP, unsigned long objectMaxHP )
{
	assert(objectHP != 0);
	assert( objectHP <= objectMaxHP );

	mHP = objectHP;
	mMaxHP = objectMaxHP;
	mpObjectSceneNode->UpdateNameCardGaugeValue();
}

void cMonster::HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType )
{
	assert( GetState() != eOBJECT_STATE_DIE );
	if( GetState() != eOBJECT_STATE_DIE )
	{
		if( pAttacker )
		{
			if( mpModeAgent )
			{
				if( mpModeAgent->IsAntiRotation() == false )
					SetDesiredDir( GetPos(), pAttacker->GetPos() );
			}
			else
				SetDesiredDir( GetPos(), pAttacker->GetPos() );
		}

		SetState( eOBJECT_STATE_DIE );
		if( attackType == eDAMAGE_CRITICAL )
		{
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DIE2 );
			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE2 );
		}
		else
		{
			SKILLMAN->CreateApplyObject( pAttacker, this, 0, eAPPLYDRAMA_DIE1 );
		}

		/// 사망시 보유 효과 제거..
		ClearAllBuff( true );
		ClearAllDeBuff( true );
	}
	else
	{
		assert(0);
	}

	mHP = 0;
	mMaxHP = objectMaxHP;
	mpObjectSceneNode->UpdateNameCardGaugeValue();
}



void cMonster::MPChange( unsigned long objectMP, unsigned long objectMaxMP )
{
	if( GetState() == eOBJECT_STATE_DIE )
	{
		assert(objectMP == 0);
	}

	mMP = objectMP;
	mMaxMP = objectMaxMP;

	mpObjectSceneNode->UpdateNameCardGaugeValue();
}


void cMonster::SetMoveSpeed( unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType )
{ 
	mMoveSpeed = moveSpeed; 

	/// 기존값 유지
	if( eMOVESPEED_STOP != moveSpeedType )
		mMoveSpeedType = moveSpeedType; 
}


void cMonster::SetFixedObjectSizeScale( unsigned short scalePer, bool init )
{
	mFixedObjectSizePer = scalePer;

	if( mpMonsterInfo == NULL )
	{
		assert(0);
		return ;
	}

	float scale = 1.0f;
	if( mpMonsterInfo->mMonsterScale > 0.0f )
	{
		scale = mpMonsterInfo->mMonsterScale;
	}
	else
	{
		assert(0);
	}

	mFixedObjectSize = mpMonsterInfo->mMonsterFixSize * (float)scalePer * 0.01f;
	mpObjectSceneNode->SetTargetScale( scale * (float)scalePer * 0.01f, init );
}


void cMonster::MoveStop( float x, float y )
{ 
	mGotoX = x;
	mGotoY = y;

	mLastGotoX = x;
	mLastGotoY = y;

	mPathIndex = 0;
	mPathCount = 0;

	if( mState == eOBJECT_STATE_MOVE )
		SetState( eOBJECT_STATE_IDLE );

	FixPos( x, y );
}


void cMonster::MonsterAttack( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pTarget, float attackSpeedFactor )
{
	if( GetState() == eOBJECT_STATE_DIE )
		return;

	if( pTarget )
		SetDesiredDir( GetPos(), pTarget->GetPos() );

	SKILLMAN->CreateSkillObject( uIdx, skillIdx, 0, this, pTarget, attackSpeedFactor );

	SetState( eOBJECT_STATE_ATTACK );
}

void cMonster::MonsterAttack( unsigned long uIdx, unsigned long skillIdx, NiPoint3 fieldPos, float attackSpeedFactor )
{
	if( GetState() == eOBJECT_STATE_DIE )
		return;

	NiPoint3 pos = fieldPos;
	if( WORLDMAN->CalcHeight(&pos.z, pos.x, pos.y) == false )
	{
		assert(0);
		return;
	}

	SetDesiredDir( GetPos(), pos );

	SKILLMAN->CreateSkillObject( uIdx, skillIdx, 0, this, pos, attackSpeedFactor );

	SetState( eOBJECT_STATE_ATTACK );
}


void cMonster::ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker )
{
	cBaseObject::ApplyDamageDrama( damage, damageType, pAttacker );

	if( GetState() == eOBJECT_STATE_DIE )
		return;

	unsigned int ani = mpObjectSceneNode->GetTargetAnimation();

	mDamageAfterAnim = M_ANITYPE_IDLE1;

	switch( ani )
	{
	case M_ANITYPE_WORK:
	case M_ANITYPE_RUN:
	case M_ANITYPE_SKILL1:
	case M_ANITYPE_SKILL2:
	case M_ANITYPE_ATTACK3:
	case M_ANITYPE_DAMAGE1:
//	case M_ANITYPE_DAMAGE2:
	case M_ANITYPE_CASTING:
	case M_ANITYPE_DIE1:
	case M_ANITYPE_DIE2:
		return;
	}

	if( ani >= M_ANITYPE_MODEANI )
		return;

	if( M_ANITYPE_ATTACK1 <= ani && ani <= M_ANITYPE_ATTACK3 )
	{
		if( GetTargetObject() != pAttacker )
			return;

		unsigned long time = mpObjectSceneNode->GetTargetAnimationEndTime() - 200;
		if( THEAPP->GetWorldAccumTime() < time )
			return;
	}

	if( ani == M_ANITYPE_CASTING )
		mDamageAfterAnim = M_ANITYPE_CASTING;

	/// damage동작 적용
	if( damageType < eDAMAGEPRINT_HEAL_OUR )
	{
		mIdleTime = 0;

		if( GetState() != eOBJECT_STATE_ATTACK )
		{
			if( mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE1 || 
				mpObjectSceneNode->GetTargetAnimation() == M_ANITYPE_IDLE2 )
			{
				if( pAttacker )
				{
					if( mpModeAgent )
					{
						if( mpModeAgent->IsAntiRotation() == false )
							SetDesiredDir( GetPos(), pAttacker->GetPos() );
					}
					else
						SetDesiredDir( GetPos(), pAttacker->GetPos() );
				}
			}
		}

		if( damageType == eDAMAGEPRINT_CRITICAL_ENEMY || damageType == eDAMAGEPRINT_CRITICAL_OUR || damageType == eDAMAGEPRINT_MP_CRITICAL )
		{
			UpdateAnimationSpeed(1.0f);
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DAMAGE2 );
		}
		else
		{
			UpdateAnimationSpeed(1.0f);
			mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_DAMAGE1 );
		}
	}
}

void cMonster::PlayIdle2Ani()
{
	if( GetState() != eOBJECT_STATE_IDLE )
		return;

	mpObjectSceneNode->SetTargetAnimation( M_ANITYPE_IDLE2 );
}

void cMonster::ActiveChatBubble( sTextItem* text )
{
	mpObjectSceneNode->ActiveChatBubble( text );
}

bool cMonster::IsPrecedeAttack() 
{
	return ( mpMonsterInfo ) ? mpMonsterInfo->mIsPrecedeAttack : false;
}

bool cMonster::IsChangeMonCheck()
{
	return ( mpMonsterInfo ) ? mpMonsterInfo->mIsChgMonCheck : false;
}

void cMonster::UpdateNameCardTextValue()
{
	if( mpObjectSceneNode )
		mpObjectSceneNode->UpdateNameCardTextValue();
}

void cMonster::ChangeMonsterMode( long modeIdx )
{
	mCurrentModeIndex = 0;
	mpImageMode	= 0;

	// 기존 이미지 삭제
	if( mpImagePlane )
	{
		UIMAN->ClearMonModeImage( mpImagePlane );
		SAFE_DELETE( mpImagePlane );

		mImageStartAccumTime = ULONG_MAX;
		mImageEndAccumTime = 0;
	}
	
	/// 기존 모드 정보 제거
	if( mModeLoopEffectList.IsEmpty() == false )
	{
		cModeLoopList::cIterator i = mModeLoopEffectList.Begin();
		cModeLoopList::cIterator end = mModeLoopEffectList.End();
		for( ; i != end; ++i )
		{
			unsigned long idx = (unsigned long)*i;
			cEffectSceneNode* n = SCENEMAN->GetEffectSceneNode( idx );
			if( n )
				n->SetLoopFlag( false );
		}
		mModeLoopEffectList.Clear();
	}

	if( mModeLoopSoundList.IsEmpty() == false )
	{
		cModeLoopList::cIterator i = mModeLoopSoundList.Begin();
		cModeLoopList::cIterator end = mModeLoopSoundList.End();
		for( ; i != end; ++i )
		{
			unsigned long idx = (unsigned long)*i;
			cSoundSceneNode* n = SCENEMAN->GetSoundSceneNode( idx );
			if( n )
				n->FadeOut();
		}
		mModeLoopSoundList.Clear();
	}

	if( modeIdx < 0 )
		return;
	if( GetState() == eOBJECT_STATE_DIE )
		return;

	/// 신규 모드 Init정보 처리
	cModeAgent* pAgent = MONSTERSCRIPT->GetMonsterModeAgent( GetMonsterClassIdx() );
	if( pAgent == 0 )
		return;

	cMode* pMode = pAgent->GetMode( modeIdx );
	if( pMode == 0 )
		return;

	// 현재 모드 정보
	mCurrentModeIndex = modeIdx;

	cModeInfoArr* arr = pMode->GetInitModeInfoArr();
	for( unsigned int i=0; i<arr->GetSize(); i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)((*arr)[i]);
		switch( p->type )
		{
		case eMODEINFO_SPEECH:
			{
				// 채팅
				sModeSpeech* modeData = (sModeSpeech*)p;

				sTextItem* speech = GAMERESOURCEMAN->GetMonsterTalk( modeData->mSpeechIdx, this, HERO );
				if( speech )
				{
					cStringT str;
					str += GetName();
					str += _T(":");

					sTextItem* check = speech;
					while( check )
					{
						str += check->mText;
						check = check->mpNext;
					}

					switch( modeData->mSpeechType )
					{
					case eM_TALKVIEW_BALLOON:	/// 말풍선
						{
							ActiveChatBubble( speech );	
						}
						break;
					case eM_TALKVIEW_NORMAL:	/// 말풍선 + (일반)채팅창
						{
							ActiveChatBubble( speech );

							CHATMANAGER->AddChatMsg( str.Cstr(), eCHAT_NORMAL );
						}
						break;
					case eM_TALKVIEW_SHOUT:		/// 말풍선 + (외치기)채팅창
						{
							ActiveChatBubble( speech );

							CHATMANAGER->AddChatMsg( str.Cstr(), eCHAT_SHOUT );
						}
						break;
					}
				}
				else
					assert(0);
			}
			break;
		case eMODEINFO_MODESTATE:
			{
				/// Animation
				sModeState* modeData = (sModeState*)p;

				if( mpObjectSceneNode )
					mpObjectSceneNode->SetTargetAnimation( modeData->mAnimationIdx );
			}
			break;
		case eMODEINFO_APPLYRESOURCE:
			{
				sModeResource* modeData = (sModeResource*)p;

				sModeObj* pobj = pAgent->GetModeObject( modeData->mModeObjIdx );
				if( pobj == 0 )
					break;

				switch( pobj->kind )
				{
				case eMODEOBJ_EFFECT:
					{
						NiTransform trans;
						trans.m_Translate = modeData->mVar;
						trans.m_fScale = pobj->scale;
						trans.m_Rotate = pobj->rot;

						if( modeData->mFollow == false )
						{
							NiMatrix3 mat = GetModelRot();
							trans.m_Rotate = pobj->rot*mat;
						}

						char pathName[256] = {0,};
						::sprintf_s( pathName, "./Data/Effect/%s", pobj->file.Cstr() );

						cEffectSceneNode* pNode = SetLinkdEffect( modeData->mLinkPos, pathName, &trans, modeData->mLoop, modeData->mFollow );
						if( pNode && modeData->mLoop )
							mModeLoopEffectList.PushBack( pNode->GetIndexByManger() );
					}
					break;
				case eMODEOBJ_SOUND:
					{
						/// 사운드 적용
						cSoundSceneNode* pNode = SetLinkdSound( pobj->soundListIndex, modeData->mLoop );
						if( pNode && modeData->mLoop )
							mModeLoopSoundList.PushBack( pNode->GetIndexByManger() );
					}
					break;
				default: 
					{
						assert(0);
					}
					break;
				}
			}
			break;
		case eMODEINFO_IMAGE:
			{
				// 모드 이미지 출력용
				sModeImage* modeData = (sModeImage*)p;
				mpImageMode = modeData;

				//
				mImageStartAccumTime = mpImageMode->mTime + THEAPP->GetWorldAccumTime();
			}
			break;
		default:
			break;
		}
	}
}

cEffectSceneNode* cMonster::SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop, bool bFollow ) 
{
	if( ::strlen(nif) == 0 )
		return 0;

	if( num >= eLINK_RWEAPON )
		return 0;

	if( mpMonsterInfo && trans )
	{
		trans->m_fScale *= mpMonsterInfo->mEffectScale;
	}

	return mpObjectSceneNode->LinkEffect( num, nif, trans, bLoop, bFollow ); 
}

void cMonster::UpdateTutorialBoard()
{
	if( mpObjectSceneNode )
	{
		((cMonsterSceneNode*)mpObjectSceneNode)->UpdateTutorialBoard();
	}
}

void cMonster::ClearTutorialBoard()
{
	if( mpObjectSceneNode )
	{
		((cMonsterSceneNode*)mpObjectSceneNode)->ClearTutorialBoard();
	}
}

void cMonster::SetSkillPushPull( float posX, float posY )
{
	mGotoX = posX;
	mGotoY = posY;

	mPathIndex = 0;
	mPathCount = 0;
	SetMoveStopRange( 0 );

	SetState( eOBJECT_STATE_PUSHPULL );
}
