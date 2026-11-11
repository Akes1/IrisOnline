#include "stdafx.h"
#include "npc.h"

#include "GameResourceManager.h"
#include "SceneManager.h"
#include "NpcSceneNode.h"
#include "EffectSceneNode.h"

#include "NpcScript.h"

#include "Quest_Common.h"
#include "QuestManager.h"
#include "ObjectManager.h"
#include "RButtonMenuWindow.h"

cNpc::cNpc( unsigned char type )
: cBaseObject( type )
, mpNpcClassInfo(0)
, mpButtonWindow(0)
{
	mObjectId = 0;
	mIsSpeeching = false;

	mQuestEffectType = eNPCQUEST_EFFECT_NONE;
	mQuestEffect = 0;
	mEffectIdx = (unsigned long)-1;
}

cNpc::~cNpc()
{
	mpButtonWindow = 0;

	UIMAN->DeleteNpcTalkButton( mObjectId );

	mpNpcClassInfo = 0;

	if( mEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdx );
		if( node )
		{
			assert( node == mQuestEffect );
			node->Remove();
		}
		mQuestEffect = 0;
		mEffectIdx = (unsigned long)-1;
	}
}

bool cNpc::Create( unsigned long objId, unsigned long npcClassIdx, NiPoint3 sPos, NiMatrix3 sRot, float fScale )
{
	mpNpcClassInfo = NPCSCRIPT->GetNPCList( npcClassIdx );
	if( mpNpcClassInfo == 0 )
	{
		assert(0);
		return false;
	}

	mObjectId = objId;

	const char* strFile = GAMERESOURCEMAN->GetModelFileName( mpNpcClassInfo->mModelIndex );
	cNpcSceneNodeParam param;
	param.mpObject = this;
	param.mTranslate = sPos;
	param.mRotate = sRot;
	param.mScale = fScale;
	param.mNpcClassIndex = npcClassIdx;

	cString pathName;
	pathName.Format( "./Data/Npc/%s", strFile );
	param.mPathName = pathName;

	cNpcSceneNode* pSceneNode = SCENEMAN->CreateNPC( param );
	if( !pSceneNode )
	{
		assert(0);
		return false;
	}

	mpObjectSceneNode = pSceneNode;
	mpObjectSceneNode->SetTargetAnimation( NPC_ANI_IDLE1 );

//	NiPoint3 sFoot = mpObjectSceneNode->GetObjectFoot();
//	NiPoint3 sHead = mpObjectSceneNode->GetObjectHead();
//	mStatureValue = (sHead.z - sFoot.z) / mpObjectSceneNode->GetWorldScale();

	mpObjectSceneNode->ShowNameGauge( false );
	mpObjectSceneNode->UpdateNameCardTextValue();

	eNpcQuestEffect type = QUESTMAN->GetNpcStatus( mObjectId );
	SetQuestEffect( type );

	// 대화버튼 생성
	mpButtonWindow = UIMAN->AddNpcTalkButton( mObjectId );
	if( !mpButtonWindow )
		assert(0);
	return true;
}

void cNpc::Update( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpObjectSceneNode == 0 ) return;

	mIdleTime += deltaTime;
	if(	mIsSpeeching == false && mIdleTime >= 5000 )
	{
		mIdleTime = 0;

		int i = ::rand()%1000;

		if( i < 200 )
		{
			mpObjectSceneNode->SetTargetAnimation( NPC_ANI_IDLE2 );

		}
		else if( i< 400 )
		{
			if( mQuestEffectType != eNPCQUEST_EFFECT_NONE && mQuestEffectType != eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
			{
				mpObjectSceneNode->SetTargetAnimation( NPC_ANI_EVENT );
			}
		}
	}

	Interpret( accumTime );

	if( ((cNpcSceneNode*)mpObjectSceneNode)->IsFadeOutEnd() == true )
		OBJECTMAN->AddRemoveNpc( GetObjectID() );
}

void cNpc::Interpret( unsigned long time )
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
		if( id != NPC_ANI_IDLE1 )
		{
			mpObjectSceneNode->SetTargetAnimation( NPC_ANI_IDLE1 );
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		/// INVALID_TIME에 대한 보정
		return;
	}
}

void cNpc::StartSpeech()
{
	mIsSpeeching = true;
}

void cNpc::EndSpeech()
{
	mIdleTime = 0;
	mIsSpeeching = false;

	mpObjectSceneNode->SetTargetAnimation( NPC_ANI_ENDSPEECH );
}

void cNpc::SetQuestEffect( eNpcQuestEffect type )
{
	if( mQuestEffectType == type )
		return;

	if( type == eNPCQUEST_EFFECT_NONE || type == eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
	{
		if( mEffectIdx != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdx );
			if( node )
			{
				assert( node == mQuestEffect );
				node->Remove();
			}
			mQuestEffect = 0;
			mEffectIdx = (unsigned long)-1;
		}
	}
	else
	{
		cString str;
		switch( type )
		{
		case eNPCQUEST_EFFECT_NEW:		/// 신규가능
			str = "./Data/Effect/Eff_NPC_Quest_01.nif";
			break;
		case eNPCQUEST_EFFECT_PLAYING:	/// 진행중
			str = "./Data/Effect/Eff_NPC_Quest_02.nif";
			break;		
		case eNPCQUEST_EFFECT_COMPLETE:/// 진행완료가능
		case eNPCQUEST_EFFECT_REWARD:	/// 보상가능
			str = "./Data/Effect/Eff_NPC_Quest_03.nif";
			break;
		case eNPCQUEST_EFFECT_NEXT_LEVEL_NEW: /// 고레벨 퀘스트 가능
			str = "./Data/Effect/Eff_NPC_Quest_08.nif";
			break;
		case eNPCQUEST_EFFECT_THEME_NEW:
			str = "./Data/Effect/Eff_NPC_Quest_04.nif";
			break;
		case eNPCQUEST_EFFECT_THEME_COMPLETE:
		case eNPCQUEST_EFFECT_THEME_REWARD:
			str = "./Data/Effect/Eff_NPC_Quest_05.nif";
			break;
		case eNPCQUEST_EFFECT_REPEAT_NEW:
			str = "./Data/Effect/Eff_NPC_Quest_06.nif";
			break;
		case eNPCQUEST_EFFECT_REPEAT_COMPLETE:
		case eNPCQUEST_EFFECT_REPEAT_REWARD:
			str = "./Data/Effect/Eff_NPC_Quest_07.nif";
			break;
		default:
			assert(0);
			return;
		}

		if( mEffectIdx != (unsigned long)-1 )
		{
			cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdx );
			if( node )
			{
				assert( node == mQuestEffect );
				node->Remove();
			}
			mQuestEffect = 0;
			mEffectIdx = (unsigned long)-1;
		}

		NiTransform trans;
		trans.MakeIdentity();

		float height = mpObjectSceneNode->GetObjectHead().z - mpObjectSceneNode->GetObjectFoot().z;
		trans.m_Translate = NiPoint3( 0.0f, 0.0f, height + 55.0f );

		mQuestEffect = mpObjectSceneNode->LinkEffect( eLINK_FOOT, str.Cstr(), &trans, true, true );
		if( mQuestEffect )
			mEffectIdx = mQuestEffect->GetIndexByManger();
	}
	mQuestEffectType = type;
}

void cNpc::FadeOut()
{
	if( mpObjectSceneNode )
	{
		((cNpcSceneNode*)mpObjectSceneNode)->FadeOut();
	}
}

void cNpc::UpdateTutorialBoard()
{
	if( mpObjectSceneNode )
	{
		((cNpcSceneNode*)mpObjectSceneNode)->UpdateTutorialBoard();
	}
}

void cNpc::ClearTutorialBoard()
{
	if( mpObjectSceneNode )
	{
		((cNpcSceneNode*)mpObjectSceneNode)->ClearTutorialBoard();
	}
}

void cNpc::ShowTalkWindow( unsigned long x, unsigned long y )
{
	if( mpButtonWindow )
		((cTalkButtonWindow*)mpButtonWindow)->Show( x, y );	
}

void cNpc::HideTalkWindow()
{
	if( mpButtonWindow && mpButtonWindow->IsVisible() )
	{
		((cTalkButtonWindow*)mpButtonWindow)->Hide();
	}
}
