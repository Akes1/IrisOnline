#include "stdafx.h"
#include "Object.h"

#include "DummyClient.h"

#include "Monster_Common.h"

#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "WorldManager.h"

cMonster::cMonster( cObjectManager* manager )
: cObject( eOBJECTTYPE_MONSTER, manager )
{
	mGotoX = 0.0f;
	mGotoY = 0.0f;

	mLastGotoX = 0;
	mLastGotoY = 0;

	mObjectId = 0;
	mMoveSpeed = 0;

	mMoveOverTime = 0;
	mMoveStopRange = 0.0f;

	mDesiredDir = -NiPoint3::UNIT_Y;

	mMoveSpeedType = eMOVESPEED_WALK;

	mPathIndex = 0;
	mPathCount = 0;

	mpMonsterInfo = 0;

	mHP = 0;
	mMP = 0;
	mMaxHP = 0;
	mMaxMP = 0;

	mFixedObjectSizePer = 0;
	mFixedObjectSize = 0.0f;
}

cMonster::~cMonster()
{

}

bool cMonster::Init( sMonsterData* baseInfo, NiPoint3 pos, NiMatrix3 rotate )
{
	mObjectId = baseInfo->mMonsterIdx;

	mpMonsterInfo = GAMERESOURCEMAN->GetMonsterListInfo( baseInfo->mMonsterClassIdx );
	if( mpMonsterInfo == NULL )
		return false;

	mTrans.m_Translate = pos;
	mTrans.m_Rotate = rotate;

	mMaxHP = baseInfo->mMaxHP;
	mMaxMP = baseInfo->mMaxMP;
	mHP = baseInfo->mHP;
	mMP = baseInfo->mMP;
	mMoveSpeed = baseInfo->mMoveSpeed;

	mDesiredDir = rotate*mDesiredDir;
	mDesiredDir.Unitize();

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



	return true;
}

void cMonster::Process( unsigned long deltaTime, unsigned long accumTime )
{
	deltaTime;
	accumTime;

	switch( mState )
	{
	case eOBJECT_STATE_MOVE:
		{
			if( mPathCount == 0 )
				return;

			/// 이동중이면
			NiPoint3 pos = mTrans.m_Translate;

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
				if( WORLDMAN->CalcHeight( mManager->GetFolderIdx(), &monsterGuessPos.z, monsterGuessPos.x, monsterGuessPos.y ) == false )
					assert(NULL);


				float moveStopRangeFix = 0.0f;
				cDummyClient* hero = mManager->GetHero();
				if( hero->GetObjectID() == mTarget.index )
				{
					if( hero->IsDie() )
					{
						targetPos.x = mLastGotoX;
						targetPos.y = mLastGotoY;
						if( WORLDMAN->CalcHeight( mManager->GetFolderIdx(), &targetPos.z, targetPos.x, targetPos.y ) == false )
							assert(NULL);
					}
					else
					{
						targetPos = hero->GetPos();
					}

					moveStopRangeFix = GetFixedObjectSize() + hero->GetFixedObjectSize() + mMoveStopRange;
				}
				else
				{
					cObject* pBaseObject = GetTargetObject();
					if( pBaseObject == NULL || pBaseObject->GetState() == eOBJECT_STATE_DIE )
					{
						targetPos.x = mLastGotoX;
						targetPos.y = mLastGotoY;
						if( WORLDMAN->CalcHeight( mManager->GetFolderIdx(), &targetPos.z, targetPos.x, targetPos.y ) == false )
							assert(NULL);
					}
					else
					{
						targetPos = pBaseObject->GetPos();
					}

					moveStopRangeFix = GetFixedObjectSize() + pBaseObject->GetFixedObjectSize() + mMoveStopRange;
				}


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

					if( WORLDMAN->CalcHeight( mManager->GetFolderIdx(), &p.z, p.x, p.y ) )
					{
						mTrans.m_Translate = p;
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
			if( WORLDMAN->CalcHeight( mManager->GetFolderIdx(), &pos.z, pos.x, pos.y ) )
			{
				mTrans.m_Translate = pos;
			}
			else
			{
				assert( 0 );
				SetState( eOBJECT_STATE_IDLE );
			}

		}
		break;
	default:
		break;
	}
}

bool cMonster::Move( float x, float y )
{
	/// 추가: 부드러운 방향 전환을 넣도록 한다.

	NiPoint3 kWorldTranslate = NiPoint3(x, y, 0.0f);
	NiPoint3 kCurTranslate = mTrans.m_Translate;
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

		mTrans.m_Rotate = mTrans.m_Rotate*kZRot;
	}

	// Animation Setting - Run
	SetState( eOBJECT_STATE_MOVE );

	/// 목적지 설정
	mGotoX = x;
	mGotoY = y;

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

		mLastGotoX = mPathArray[count-1].x; 
		mLastGotoY = mPathArray[count-1].y;
	}
}

void cMonster::HPChange( unsigned long objectHP, unsigned long objectMaxHP )
{
	assert(objectHP != 0);
	assert( objectHP <= objectMaxHP );

	mHP = objectHP;
	mMaxHP = objectMaxHP;
}

void cMonster::HPChangeDie( unsigned long objectMaxHP )
{
	mHP = 0;
	mMaxHP = objectMaxHP;

	SetState( eOBJECT_STATE_DIE );
}

void cMonster::SetMoveSpeed( unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType )
{ 
	mMoveSpeed = moveSpeed; 

	/// 기존값 유지
	if( eMOVESPEED_STOP != moveSpeedType )
		mMoveSpeedType = moveSpeedType; 
}

void cMonster::MoveStop( float x, float y )
{ 
	mGotoX = x;
	mGotoY = y;

	mLastGotoX = x;
	mLastGotoY = y;

	mPathIndex = 0;
	mPathCount = 0;

	float height = 0.0f;

	if( WORLDMAN->CalcHeight(mManager->GetFolderIdx(), &height, x, y) == false )
		assert(0);

	mTrans.m_Translate = NiPoint3(x,y,height);

	SetState( eOBJECT_STATE_IDLE );
}

void cMonster::SetState( unsigned char state ) 
{ 
	if( mState ==eOBJECT_STATE_DIE )
		return;

	mState = (eOBJECTSTATE)state;
}

float cMonster::GetFixedObjectSize()
{
	return mFixedObjectSize;
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
}
