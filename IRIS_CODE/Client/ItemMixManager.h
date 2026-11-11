/* ==========================================================================
* 파    일 : ItemMixManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-01-15
* 주의사항 :
*===========================================================================*/

/// 네트워크 락
enum
{
	eLOCK_ITEMMIXNONE,
	eLOCK_ITEMMIXDELETE,
	eLOCK_ITEMMIXSTART,
	eLOCK_ITEMMIXEND,
};

/// 보유 레시피 정보
struct sKeepRecipe
{
	unsigned long mRestTime;
	unsigned long mStartTime;
	unsigned long mNextMakeTime;
	cStringT	mStrNextMakeTime;
	
	sKeepRecipe()
	{
		mRestTime = 0;
		mStartTime = 0;		
		mNextMakeTime = 0;
	}
};

class cItemMixManager
{
public:
	cItemMixManager();
	~cItemMixManager();

	void Close();

	/// 스킬,레시피 등록 아이템 사용
	bool RegistSkill( unsigned short slot, char type, char subType, unsigned long makeSkillIdx );
	bool RegistRecipe( unsigned short slot, unsigned long recipeIdx );
	bool RegistRecipeGroup( unsigned short slot, unsigned long recipeGroupIdx );

	///
	bool ItemMixStart( unsigned char index, unsigned long recipeIdx, int count );
	void ItemMixDelete( unsigned char index );
	bool ItemMixCancel( bool hide = true );

	///
	void ItemMixOpen();

	///
	void NetworkMsgParserLoad( char* msg );
	void NetworkMsgParserGame( char* msg );

	/// 레시피인덱스
	sKeepRecipe* GetKeepMixSkill1( unsigned long recipeIdx );
	sKeepRecipe* GetKeepMixSkill2( unsigned long recipeIdx );
	unsigned long GetKeepSkillText( unsigned char index );
	unsigned long GetTypeText( int index, unsigned long type );
	bool IsNetLock() { return mNetLock; }

	bool IsKeepRecipe( unsigned long recipeIdx );
	bool IsKeepMixSkill( unsigned long makeSkillIdx, unsigned char subType );

	tPointerHashMap<unsigned long, void*>* GetKeepSkillMap1() { return &mKeepSkillMap1; }
	tPointerHashMap<unsigned long, void*>* GetKeepSkillMap2() { return &mKeepSkillMap2; }

public:
	static cItemMixManager*	mpSingleton;
	static cItemMixManager* GetSingleton();

private:
	bool AddKeepSkill1( unsigned long recipeIdx, unsigned long restTime );
	bool AddKeepSkill2( unsigned long recipeIdx, unsigned long restTime );
	void DeleteKeepSkill1();
	void DeleteKeepSkill2();
	
	/// 락걸기
	void NetLock( int lockTry );
	void NetUnLock( int lockTry );

private:
	typedef tPointerHashMap<unsigned long, void*> cKeepRecipeMap;
	cKeepRecipeMap	mKeepSkillMap1;	// 전문기술 1
	cKeepRecipeMap	mKeepSkillMap2;	// 전문기술 2

	/// 네트워크 관련 락
	bool	mNetLock;
	int		mLockTry;

	// 백업
	unsigned char mTempIndex;
	unsigned long mTempRecipeIdx;
};

inline cItemMixManager* cItemMixManager::GetSingleton() 
{
	return mpSingleton;
}

#define ITEMMIXMAN cItemMixManager::GetSingleton()