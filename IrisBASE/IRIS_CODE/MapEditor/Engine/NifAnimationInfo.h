/* ==========================================================================
* 작 성 자 : 이법석
* 작 성 일 : 2007.01.18
* 내    용 : Nif파일에 속한 애니메이션 정보
* 주의사항 : 
*===========================================================================*/
#pragma once

class cNifTimeManager 
{
public:
	enum 
	{
		TM_SCALE_FACTOR = 0,
		TM_START_TIME,
		TM_END_TIME,
		TM_TIME_MODE,
		TM_ENABLING
	};

	cNifTimeManager();

	float GetFrameTime( float fTime );
	float GetRewTime( float fTime );
	void SetFrameTime(float fTime);

	float GetScaleFactor() const;
	void SetScaleFactor(float fScale);

	float GetStartTime() const;
	void SetStartTime(float fStart);

	float GetEndTime() const;
	void SetEndTime(float fEnd);

	bool IsEnabled() const;
	void Enable();
	void Disable();

protected:
	float m_fAccumTime;
	float m_fScaleFactor;
	float m_fStartTime;
	float m_fEndTime;
	bool m_bTimingEnabled;

	float m_fLastTime;
};

//---------------------------------------------------------------------------
inline float cNifTimeManager::GetScaleFactor() const
{ 
	return m_fScaleFactor;
}
//---------------------------------------------------------------------------
inline void cNifTimeManager::SetScaleFactor(float fScale)
{ 
	m_fScaleFactor = fScale;
}
//---------------------------------------------------------------------------
inline float cNifTimeManager::GetStartTime() const
{ 
	return m_fStartTime;
}
//---------------------------------------------------------------------------
inline void cNifTimeManager::SetStartTime(float fStart)
{ 
	m_fStartTime = fStart;
}
//---------------------------------------------------------------------------
inline float cNifTimeManager::GetEndTime() const
{ 
	return m_fEndTime;
}
//---------------------------------------------------------------------------
inline void cNifTimeManager::SetEndTime(float fEnd)
{ 
	m_fEndTime = fEnd;
}
//---------------------------------------------------------------------------
inline bool cNifTimeManager::IsEnabled() const
{
	return m_bTimingEnabled;
}
//---------------------------------------------------------------------------
inline void cNifTimeManager::Enable()
{
	m_bTimingEnabled = true;
	m_fLastTime = -NI_INFINITY;
}
//---------------------------------------------------------------------------
inline void cNifTimeManager::Disable()
{
	m_bTimingEnabled = false;
}
//---------------------------------------------------------------------------

class cNifAnimationInfo
{
public:
	cNifAnimationInfo();
	~cNifAnimationInfo();

	enum
	{
		eEvent_None = 0,
		eEvent_Ing,
		eEvent_End,
	};

	/// 애니 정보에 대한 데이터 수집용 인터페이스
	void CollectData( NiNode* pRoot );

	/// 컨트롤러의 총 수
	inline unsigned int GetNumberOfControllers() { return mControllerCount; }
	/// 전체 컨트롤러의 키프레임 총 수
	inline unsigned int GetNumberOfKeyframes() { return mKeyframeCount; }
	/// 전체 컨트롤러들의 시작시간중 최소시간
	inline float GetMinTime() { return (mMinTime == FLT_MAX)? 0.0f:mMinTime; }
	/// 전체 컨트롤러들의 종료시간중 최대시간
	inline float GetMaxTime() { return mMaxTime; }

	/// 컨트롤러들의 속성 변경
	void SetAnimType( NiTimeController::AnimType type );
	void SetCycleType( NiTimeController::CycleType type );

	/// 애니메이션 활성화
	void Start( float time, bool begin = false );

	/// 애니메이션 비활성화
	void Stop();

	/// 루핑 정보에 대한 처리
	inline void SetLooping( bool loop ) { mLooping = loop; }
	inline bool IsLooping() { return mLooping; }
	inline bool IsEnable() { return mEnable; }

	/// 처리부
	unsigned char Update( unsigned long deltaTime, unsigned long accumTime );

	unsigned long GetFrameTime();

	void SetStopFlag( bool flag ) { mStopFlag = flag; }

protected:
	void Add(NiTimeController* pController);

	void AddInfo(NiTimeController* pController);
	void AddInterpInfo(NiInterpController* pController);

	void FindAndAdd(NiObjectNET* pkRoot);

protected:
	/// 전체 컨트롤들에 대한 정보
	float mMinTime;
	float mMaxTime;

	float mStartTime;

	unsigned int mKeyframeCount;
	unsigned int mControllerCount;

	/// 제어 컨트롤들의 리스트
	NiTPointerList<NiTimeController*> mControllerList;

	/// 제어 컨트롤들에 대한 반복 여부
	bool mLooping;

	/// 전체 컨트롤러의 활성화 여부
	bool mEnable;
	bool mStopFlag;
};
