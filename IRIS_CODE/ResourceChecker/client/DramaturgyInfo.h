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

enum eDRAMA_TYPE
{
	eDRAMA_SKILL = 0,		/// 스킬
	eDRAMA_INFULUENCE,		/// 효과(지속)
};

enum eDRAMAOBJ_KIND
{
	eDRAMAOBJ_EFFECT = 0,	/// 이펙트 객체(nif)
	eDRAMAOBJ_SOUND,		/// 사운드 객체
	eDRAMAOBJ_TRAILTEX,		/// 궤적용 텍스쳐
};

enum eDRAMAACTION
{
	eDRAMA_ACTION = 0,
	eDRAMA_ACTIVE,
	eDRAMA_DAMAGE,

	eDRAMA_BULLET,

	eDRAMA_TRAIL,
	eDRAMA_SHAKE,
	eDRAMA_FADEOUT,
};

enum eDRAMASTATE
{
	eDRAMASTATE_NONE = 0,
	eDRAMASTATE_CASTING,	/// 캐스팅
	eDRAMASTATE_ACTIVITY,	/// 발동
	eDRAMASTATE_APPLY,		/// 적용 (이 상태엔 발동 + 적용 처리를 한다.)

	eDRAMASTATE_MAX,

	eDRAMASTATE_CANCEL_WAIT,
};

struct sDramaObj
{
	eDRAMAOBJ_KIND	kind;
	cString			file;

	/// effect인 경우 사용
	NiMatrix3		rot;
	float			scale;

	/// sound인 경우 사용
	unsigned long	soundListIndex;

	sDramaObj()
	{
		rot = NiMatrix3::IDENTITY;
		scale = 1.0f;
	}
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
	unsigned long	lifeTime;

	bool			bLoop;
	bool			bFollow;

	unsigned short	objIdx;

	unsigned int	linkPos;
	NiPoint3		var;

	unsigned char	weaponType;

	sDramaAction()
	{
		var = NiPoint3::ZERO;
		weaponType = 0;
	}
};

struct sDamageAction : public sDramaActionBase
{
	float			percent;
};

struct sCrashInfo
{
	unsigned short	crashEffectIdx;
	unsigned int	crashLinkPos;
	NiPoint3		crashVar;
};

struct sBulletAction : public sDramaActionBase
{
	unsigned short	bulletID;

	float			speed;

	unsigned short	bulletEffectIdx;
	unsigned int	linkPos;
	unsigned int	targetLinkPos;
	NiPoint3		bulletVar;

	bool			existCrash;

	unsigned int	crashCount;
	sCrashInfo		crashInfo[5];

	sBulletAction() : existCrash(false)
	{
		crashCount = 0;
	}
};

struct sTrailAction : public sDramaActionBase
{
	unsigned long	lifeTime;

	unsigned short	texIdx;

	unsigned int	linkStart;
	unsigned int	linkEnd;

	float			lengthPer;
	float			speedFactor;

	sTrailAction() 
	{
		speedFactor = 1.0f;
		lengthPer = 100;
	}
};


/// 연출 상태 정보
class  cDramaState
{
public:
	cDramaState()
	{
		mAnimationIdx = UINT_MAX;

		mActionCount = 0;
		mpBegin = mpEnd = 0;
	}
	~cDramaState()
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
/*
	void PushFront( sDramaActionBase* p )
	{
		p->pNext = 0;
		if( mpBegin == 0 )
		{
			mpBegin = mpEnd = p;
		}
		else
		{
			p->pNext = mpBegin;
			mpBegin = p;
		}
		mActionCount++;
	}

	void PushBack( sDramaActionBase* p )
	{
		p->pNext = 0;
		if( mpBegin == 0 )
		{
			mpBegin = mpEnd = p;
		}
		else
		{
			mpEnd->pNext = p;
			mpEnd = p;
		}
		mActionCount++;
	}
*/
	sDramaActionBase*		GetStart() { return mpBegin; }

	unsigned int		mAnimationIdx;

protected:
	unsigned int		mActionCount;
	sDramaActionBase*	mpBegin;
	sDramaActionBase*	mpEnd;
};


/// skill 및 효과와 x:1매칭
class cDramaturgyInfo
{
public:
	cDramaturgyInfo( const char* pName );
	~cDramaturgyInfo();

	/// dramaobj 생성 필요!!
	void LoadDramaObj();
	void LoadThreadDramaObj();

	bool LoadFile();

	sDramaActionBase*	GetStartAction( eDRAMASTATE state );
	unsigned int		GetAnimation( eDRAMASTATE state );
	sDramaObj*			GetDramaObj( unsigned short idx );
protected:
	/// list 상세 로딩
	bool LoadHeader( cParser& parser );
	bool LoadContents( eDRAMASTATE state, cParser& parser );

	unsigned int	GetLinkIndex( cString str );

protected:
	cString			mFileName;

	/// 연출 공통 정보
	eDRAMA_TYPE		mType;

	/// 사용 객체 정보
	typedef tPointerHashMap<unsigned short,void*> cDramaObjMap;
	cDramaObjMap	mDramaObjMap;

	/// 발사체에 대한 참조 정보
	typedef cDramaObjMap cBulletMap;
	cBulletMap		mBulletMap;

	/// 캐스팅, 발동, 적용 상세 정보
	cDramaState* mpDramaState[eDRAMASTATE_MAX];
};

