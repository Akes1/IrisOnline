#include "stdafx.h"
#include "CoolTimeManager.h"
#include "Icon.h"
#include "QuickManager.h"
#include "Application.h"

cCoolTimeManager* cCoolTimeManager::mSingleton = 0;

cCoolTimeManager::cCoolTimeManager()
: mGroupMap( 128 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cCoolTimeManager::~cCoolTimeManager()
{
	Clear();

	/// 그룹 데이터 삭제
	{
		cGroupMap::cIterator i = mGroupMap.Begin();
		cGroupMap::cIterator end = mGroupMap.End();

		for( ; i != end; ++i )
		{
			sGroupData* p = (sGroupData*)(i->mSecond);
			if( p )
				p->mIndexArr.Clear();

			SAFE_DELETE(p);
		}
		mGroupMap.Clear();
	}

	/// 아이템 쿨타임 정보 맵 삭제
	{
		cCoolTimeMap::cIterator b = mCoolTimeMap.Begin();
		cCoolTimeMap::cIterator end = mCoolTimeMap.End();

		for( ; b != end; ++b )
		{
			sCoolTime* data = (sCoolTime*)(b->mSecond);
			SAFE_DELETE( data );
		}
		mCoolTimeMap.Clear();
	}
	
	/// 쿨타임 프로세스 정보 삭제
	{
		cProcessMap::cIterator b = mProcessMap.Begin();
		cProcessMap::cIterator end = mProcessMap.End();

		for( ; b != end; ++b )
		{
			sCoolTime* data = (sCoolTime*)(b->mSecond);
			SAFE_DELETE( data );
		}
		mProcessMap.Clear();
	}
}

void cCoolTimeManager::Clear()
{
	/// 프로세스 맵 지우기
	cProcessMap::cIterator b = mProcessMap.Begin();
	cProcessMap::cIterator end = mProcessMap.End();

	for( ; b != end; ++b )
	{
		sCoolTime* data = (sCoolTime*)(b->mSecond);
		SAFE_DELETE( data );
	}
	mProcessMap.Clear();
}

/// 쿨타임 프로세스에 추가
bool cCoolTimeManager::AddProcessCool( unsigned long itemIndex, long restTime, long totalTime )
{
	sCoolTime* data = new sCoolTime;
	data->startTime = THEAPP->GetWorldAccumTime();
	data->restTime = restTime;
	data->totalTime = totalTime;

	if( mProcessMap.Insert( itemIndex, data ) == false )
	{
		assert( 0 && "failed to add cooltime" );
		SAFE_DELETE( data );
		return false;
	}
	return true;
}

/// 각 아이템의 쿨타임 정보를 저장한다
void cCoolTimeManager::LoadData( unsigned long itemIndex, unsigned int groupnum, long cooltime )
{
	sCoolTime* coolData = new sCoolTime;
	coolData->groupNum = groupnum;
	coolData->startTime = 0;
	coolData->restTime = cooltime;
	coolData->totalTime = cooltime;

	/// 데이터 저장
	if( mCoolTimeMap.Insert( itemIndex, coolData ) == false )
	{
		assert( 0 && "failed to add cooltime data" );
		return;
	}

	/// 그룹관련 데이터 저장
	if( groupnum > 0 )
	{
		sGroupData* data = (sGroupData*)mGroupMap.GetAt( groupnum );
		if( data )
		{
			data->mIndexArr.PushBack( itemIndex );
		}
		else
		{
			sGroupData* p = new sGroupData;
			p->mIndexArr.PushBack( itemIndex );

			if( mGroupMap.Insert( groupnum, p ) == false )
			{
				assert(0);
				return;
			}
		}
	}
}

/// 아이템 사용
void cCoolTimeManager::StartCoolTime( unsigned long itemIndex )
{
	/// 사용된 아이템이오면, 쿨타임에서 찾아서 프로세스에 넣는다.
	/// 그룹이면 그룹을 모두 올린다
	sCoolTime* coolData = GetCoolData( itemIndex );
	if( !coolData )
		return;
	
	/// 
	if( coolData->groupNum == 0 )
	{
		/// 쿨타임 프로세스에 추가
		AddProcessCool( itemIndex, coolData->restTime, coolData->totalTime );

		/// UI 에 세팅
		ITEMMAN->StartCoolTime( itemIndex, coolData->totalTime );

	}
	/// 그룹쿨타임 돌리기
	else
	{
		/// 그룹 리스트 뽑기
		sGroupData* group = (sGroupData*)mGroupMap.GetAt( coolData->groupNum );
		if( !group )
		{
			assert(0);
			return;
		}

		/// 그룹 쿨타임 세팅
		for( unsigned int i = 0; i < group->mIndexArr.GetSize(); ++i )
		{
			unsigned long groupItem = (unsigned long)group->mIndexArr[i];
			sCoolTime* groupData = GetCoolData( groupItem );

			if( !groupData )
				continue;

			/// 그룹 아이템이 쿨타임중이 아님.
			sCoolTime* processData = GetProcessData( groupItem );

			if( processData == 0 )
			{
				/// 자기 쿨타임 < 입력된 그룹 쿨타임 이면, 자기쿨타임으로 세팅
				if( groupData->totalTime <= coolData->totalTime )
				{
					AddProcessCool( groupItem, groupData->restTime, groupData->totalTime );
					ITEMMAN->StartCoolTime( groupItem, groupData->totalTime );
				}
				/// 자기 쿨타임 > 입력된 그룹 쿨타임 이면, 그룹 쿨타임으로 세팅
				else
				{
					AddProcessCool( groupItem, coolData->restTime, coolData->totalTime );
					ITEMMAN->StartCoolTime( groupItem, coolData->totalTime );
				}
			}
			/// 그룹 아이템이 쿨타임 중임.
			else
			{
				/// 자기 남은 쿨타임 > 입력된 그룹쿨타임 이면, 무시
				/// 자기 남은 쿨타임 < 입력된 그룹쿨타임 이면, 입력된 쿨타임으로 재 세팅
				long elasedTime = THEAPP->GetWorldAccumTime() - processData->startTime;
				long restTime = processData->totalTime - elasedTime;
				
				/// 입력된 쿨타임과 자기 원래 총 쿨타임도 비교한다
				long totalTime = ( groupData->totalTime < coolData->totalTime ) ? groupData->totalTime : coolData->totalTime;

				if( restTime < totalTime )
				{
					/// 다시세팅
					processData->startTime = THEAPP->GetWorldAccumTime();
					processData->totalTime = totalTime;

					ITEMMAN->EndCoolTime( groupItem );
					ITEMMAN->StartCoolTime( groupItem, processData->totalTime );
				}
				
			}
		}
	}
}

/// 아이템이 중간삽입 될때 세팅
void cCoolTimeManager::CopyCoolTime( unsigned long itemIndex )
{
	/// 실제로 존재하는 데이터인지 검사
	sCoolTime* coolData = GetCoolData( itemIndex );
	if( coolData )
	{
		/// 해당 아이템이 프로세스 중이라면 쿨타임 복사
		sCoolTime* processData = GetProcessData( itemIndex );
		if( processData )
			ITEMMAN->CopyCoolTime( itemIndex, processData->startTime, processData->restTime, processData->totalTime );
	}
}

///
void cCoolTimeManager::Process( unsigned long accumTime )
{
	/// 프로세스 할 쿨타임이 없으면 리턴
	if( mProcessMap.GetSize() == 0 )
		return;

	cProcessMap::cIterator i = mProcessMap.Begin();
	cProcessMap::cIterator end = mProcessMap.End();

	for( ; i != end; )
	{
		sCoolTime* data = (sCoolTime*)(i->mSecond);
		unsigned long key = (unsigned long)(i->mFirst);
		if( !data )
			continue;

		++i;

		long elasedTime = accumTime - data->startTime;
		if( elasedTime > data->restTime )
		{
			/// 쿨타임이 끝났으면 프로세스 맵으로부터 삭제
			mProcessMap.Erase( key );
			SAFE_DELETE( data );
		}
	}

}

/// 서버로부터 남은 쿨타임 정보 저장
void cCoolTimeManager::NetSetCool( sInventoryCooltime& cool )
{
	/// 남은 시간을 다시 세팅 
	AddProcessCool( cool.ItemIndex, cool.cooltime, cool.cooltime1 );
}

///
sCoolTime* cCoolTimeManager::GetCoolData( unsigned long itemIndex )
{
	return (sCoolTime*)mCoolTimeMap.GetAt( itemIndex );
}

/// 현재 프로세스중인 쿨타임 데이트 획득
sCoolTime* cCoolTimeManager::GetProcessData( unsigned long itemIndex )
{
	return (sCoolTime*)mProcessMap.GetAt( itemIndex );
}

