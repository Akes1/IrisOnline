/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 마우스 중개자
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 마우스 중개자
class cMouseAgent
{
	static cMouseAgent* mSingleton;

public:
	cMouseAgent();
	~cMouseAgent();

	/// 처리
	void Process();

	/// 마우스 메시지 처리 함수
	/// 윈도우 메시지 처리 함수에 의해 자동으로 호출된다.
	void SetLButtonDown( int x, int y );
	void SetLButtonUp( int x, int y );
	void SetRButtonDown( int x, int y );
	void SetRButtonUp( int x, int y );
	void SetMButtonDown( int x, int y );
	void SetMButtonUp( int x, int y );
	void SetPosition( int x, int y );
	void SetDeltaWheel( float dw );

	/// 마우스 좌표를 반환
	int GetX() const;
	int GetY() const;
	int GetDeltaX() const;
	int GetDeltaY() const;

	/// 왼쪽 마우스 버튼이 눌렸는지 여부를 반환
	bool IsLButtonDown() const;
	bool IsLButtonUp() const;

	/// 오른쪽 마우스 버튼이 눌렸는지 여부를 반환
	bool IsRButtonDown() const;
	bool IsRButtonUp() const;

	/// 가운데 마우스 버튼이 눌렸는지 여부를 반환
	bool IsMButtonDown() const;
	bool IsMButtonUp() const;

	/// 마우스 휠값
	float GetWheel() const;
	float GetDeltaWheel() const;

public:
	/// 단일체를 리턴
	static cMouseAgent* GetSingleton();

protected:
	/// 마우스 상태
	bool mLeft;
	bool mRight;
	bool mMiddle;

	/// 마우스 위치
	int mX;
	int mY;
	int mOldX;
	int mOldY;
	int mDeltaX;
	int mDeltaY;

	/// 마우스 휠 값 [-1.0, 1.0]
	float mWheel;
	float mOldWheel;
	float mDeltaWheel;
};

inline int cMouseAgent::GetX() const
{
	return mX;
}

inline int cMouseAgent::GetY() const
{
	return mY;
}

inline int cMouseAgent::GetDeltaX() const
{
	return mDeltaX;
}

inline int cMouseAgent::GetDeltaY() const
{
	return mDeltaY;
}

inline bool cMouseAgent::IsLButtonDown() const
{
	return mLeft;
}

inline bool cMouseAgent::IsLButtonUp() const
{
	return !mLeft;
}

inline bool cMouseAgent::IsRButtonDown() const
{
	return mRight;
}

inline bool cMouseAgent::IsRButtonUp() const
{
	return !mRight;
}

inline bool cMouseAgent::IsMButtonDown() const
{
	return mMiddle;
}

inline bool cMouseAgent::IsMButtonUp() const
{
	return !mMiddle;
}

inline float cMouseAgent::GetWheel() const
{
	return mWheel;
}

inline float cMouseAgent::GetDeltaWheel() const
{
	return mDeltaWheel;
}

inline cMouseAgent* cMouseAgent::GetSingleton()
{
	return mSingleton;
}

#define MOUSE cMouseAgent::GetSingleton()
