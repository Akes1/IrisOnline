#pragma once

class cRefereeSceneNode;
class cPlayer;

class cDuelManager
{
public:
	static cDuelManager* mpDuelManager;

public:
	cDuelManager();
	virtual ~cDuelManager();

	void Open();

	void Process( unsigned long deltaTime );

	void StartCount( unsigned long enemyIdx, NiPoint2 centerPos );
	void StartDuel();

	void EndDuel();

	bool IsDuelEnemy( cPlayer* player );
	inline bool IsDuelEnable() { return (mDuelState != eDuel_None); }
	inline unsigned long GetEnemyIdx() { return mEnemyIndex; }

	inline NiPoint2 GetDuelCenter() { return mCenterPos; }

	void SetOutofBound( bool out );

protected:
	void UpdateHPMP();

protected:

	enum 
	{
		eDuel_None = 0,
		eDuel_Counting,
		eDuel_Fighting,
	};

	unsigned char	mDuelState;
	unsigned long	mEnemyIndex;

	unsigned long	mCountChekTime;

	NiPoint2		mCenterPos;

	cRefereeSceneNode* mpReferee;
};

#define DUELMAN	cDuelManager::mpDuelManager