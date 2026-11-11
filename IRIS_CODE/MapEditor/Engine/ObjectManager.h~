/* ====================================================================
 * 파    일 :	ObjectManager.h
 * 목    적 :	게임 객체를 관리한다.
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

class cHero;

/// 오브젝트 관리자
class cObjectManager
{
	static cObjectManager* mSingleton;

public:
	cObjectManager();
	~cObjectManager();

	/// 초기화 (시작시 한번)
	bool Init();

	/// 종료화 (종료시 한번)
	void Exit();

	/// 초기부 (맵이동등 갱신시)
	void Open();

	/// 종료부 (맵이동등 갱신시)
	void Close();

	/// 처리
	void Process( unsigned long time );

	/// 마우스 픽킹 지점으로 주인공을 이동
	//void MoveHeroByMouse( int mouseX, int mouseY );

	void AddPlayer( unsigned long id, int kind, int x, int y );

	//void AddHero();
	//void AddHeroInfo( unsigned long id, int kind );

	//void MovePlayer( unsigned long id, int x, int y );

	//void RemovePlayer( unsigned long id );

	//inline cHero* GetHero() const;

public:
	/// 단일체를 리턴
	static cObjectManager* Get();

protected:
//	void CreateModel( int kind, cPlayer* pPlayer, NiPoint3 pos );

private:
	/// 주인공
	//cHero* mHero;
};

//inline cHero* cObjectManager::GetHero() const
//{
//	return mHero;
//}

inline cObjectManager* cObjectManager::Get()
{
	return mSingleton;
}
