/* =====================================================================
* 파    일 :	DramaturgyInfo.h , DramaturgyInfo.cpp
* 목    적 :	연출정보 클래스
* 작 성 자 :	정원주
* 작 성 일 :    2006년 10월 16일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/
#pragma once

class cParser;

enum eDRAMAACTION
{
	eDRAMA_ACTION = 0,
	eDRAMA_ACTIVE,
	eDRAMA_DAMAGE,

	eDRAMA_BULLET,

	eDRAMA_TRAIL,
	eDRAMA_SHAKE,
};

struct sDramaActionBase
{
	eDRAMAACTION	actionType;	
	unsigned long	time;

	sDramaActionBase*	pNext;

	sDramaActionBase() : pNext(0) 
	{
	}
};

struct sDramaAction : public sDramaActionBase
{
	float			percent;
};

/// 연출 스크립트의 시간 정보
class  sDramaState
{
public:
	sDramaState()
	{
		mIsBullet = false;
		mActivityEndTime = 0;
		mBulletSpeed = 0.0f;

		mActionCount = 0;
		mpBegin = mpEnd = 0;
	}
	~sDramaState()
	{
		sDramaActionBase* pAction = mpBegin;
		sDramaActionBase* pNext;
		for( unsigned int i=0;i<mActionCount;++i )
		{
			pNext = pAction->pNext;
			delete pAction;

			pAction = pNext;
		}
		mActionCount = 0;
	}

	void Push( sDramaActionBase* p )
	{
		p->pNext = 0;
		if( mpBegin == 0 )
		{
			mpBegin = mpEnd = p;
		}
		else
		{
			/// 
			if( mpBegin->time > p->time )
			{
				p->pNext = mpBegin;
				mpBegin = p;
			}
			else
			{
				sDramaActionBase* cur = mpBegin;
				sDramaActionBase* next = cur->pNext;
				for( unsigned int i = 0;i<mActionCount;i++ )
				{
					if( cur->time <= p->time )
					{
						if( next == 0 )
						{
							cur->pNext = mpEnd = p;
							break;
						}

						if( next->time >= p->time )
						{
							cur->pNext = p;
							p->pNext = next;
							break;
						}
					}
					else
					{
						assert(0);
					}

					cur = cur->pNext;
					next = cur->pNext;
				}
			}
		}

		mActionCount++;
	}

protected:
	unsigned int		mActionCount;
	sDramaActionBase*	mpBegin;
	sDramaActionBase*	mpEnd;

public:
	unsigned long	mActivityEndTime;
	bool			mIsBullet;
	float			mBulletSpeed;

public:
	unsigned int	GetDamageCount()	{ return mActionCount; }

	sDramaActionBase* GetDamageBegin()	{ return mpBegin; }
	sDramaActionBase* GetDamageEnd()	{ return mpEnd; }
};

/// skill 및 효과와 x:1매칭
class cDramaturgyInfo
{
public:
	cDramaturgyInfo( const char* pName = 0 );
	~cDramaturgyInfo();

	bool			LoadFile();

	unsigned long	GetActivityEndTime();
	bool			IsBullet();
	float			GetBulletSpeed();

	/// 적용 정보
	sDramaState*	GetDramaState()	{ return mpDramaState; }
protected:
	/// list 상세 로딩
	bool			LoadContents( cParser& parser, bool stateSkip = true );

	/// 서버에서 필요없는 항목 통과
	bool			LoadHeader( cParser& parser );

protected:
	cString			mFileName;

	/// 사용 객체 정보
	typedef tPointerHashMap<unsigned short,void*> cDramaObjMap;
	cDramaObjMap	mDramaObjMap;

	/// 적용 정보
	sDramaState*	mpDramaState;
};