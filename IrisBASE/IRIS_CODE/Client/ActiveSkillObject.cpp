#include "stdafx.h"
#include "ActiveSkillObject.h"

/*
#include "SkillManager.h"
#include "AppTimer.h"
#include "PlayerSceneNode.h"
#include "MonsterSceneNode.h"
#include "Monster.h"
#include "Hero.h"
#include "ObjectManager.h"

cActiveSkillObject::cActiveSkillObject()
{
}

cActiveSkillObject::~cActiveSkillObject()
{
}

void* cActiveSkillObject::operator new(size_t n)
{ 
	return 0;
//	return SKILLMAN->AllocActiveSkillObj();
} 

void cActiveSkillObject::operator delete(void* ptr)
{
//	( static_cast<cActiveSkillObject*>(ptr) )->SetSkillCode(NONUSESKILL);
//	SKILLMAN->FreeActiveSkillObj( static_cast<cActiveSkillObject*>(ptr) );
}

void cActiveSkillObject::SetActiveTime(float totallifetime, unsigned int ratio)
{
	mCurrenttime = TIMER->GetCurrentTime();
	mTotallifetime = totallifetime;
	mRatio = ratio;
	mbLife = true;
}

void cActiveSkillObject::Process()
{
/*
	eSkillShape shape = SKILLMAN->GetSkillShape(mUserKind,mSkillInfoIndex) ;
	switch( SKILLMAN->GetSkillShape(mUserKind,mSkillInfoIndex) )
	{
	case SKILLSHAPE_BUFF: break;
	default:
		{
			if(mTarget.type == eOBJECTTYPE_PLAYER)
			{
				int a = 0;
				a = 1;
			}

			float fasttime = TIMER->GetCurrentTime() - mCurrenttime;
			float endtime = mTotallifetime*mRatio*0.01f;

			if( mbLife && (TIMER->GetCurrentTime() - mCurrenttime) > (mTotallifetime*mRatio*0.01f) )
			{
				//eTARGET targetnum = eOBJECTTYPE_NONE;
				//switch(mUserKind)
				//{
				//case eATTACKER_PLAYER:
				//	{
				//		cPlayer* user;
				//		user = OBJECTMANAGER->GetPlayer(mUserNumber);
				//		user->GetTarget();
				//	}
				//	break;
				//case eATTACKER_MONSTER:
				//	{
				//		cMonster* user;
				//		user = OBJECTMANAGER->GetMonster(mUserNumber);
				//		user->GetTarget();
				//	}
				//	break;
				//}
			

				/// 스크립트 상의 타겟종류와 실제 타겟이 다르면 무시된다.
				///if( targetnum == SKILLMAN->GetSkillTarget(mUserKind,mSkillInfoIndex) )
				{

					switch(mTarget.type)
					{
					case eOBJECTTYPE_PLAYER:
						{
							cPlayer* pPlayer = OBJECTMANAGER->GetPlayer(mTarget.index);

							if(pPlayer)
							{
								if(pPlayer == HERO)
								{
									HERO->SetLinkdEffect( cPlayerSceneNode::eLINK_BODY, mLinkName.Cstr() );
								}
								else
								{
									pPlayer->SetLinkdEffect( cPlayerSceneNode::eLINK_BODY, mLinkName.Cstr() );
								}
							}
						}
						break;
					case eOBJECTTYPE_MONSTER:
						{

							cMonster* pMonster = OBJECTMANAGER->GetMonster(mTarget.index);

							if(pMonster)
							{
								pMonster->SetLinkdEffect( cMonsterSceneNode::eLINK_BODY, mLinkName.Cstr() );
							}

						}
						break;
					}

				}
				//else
				{
					//assert(0&&"Target is valid!");
				}
				mbLife = false;
			}

			fasttime = 0;
			endtime = 0;
		}
		break;
	}

	shape = SKILLSHAPE_ERROR;
//*/
//}
