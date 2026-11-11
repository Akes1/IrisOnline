#pragma once

class cDummyClient;
class cObject;
class cPlayer;
class cMonster;
class cNpc;
class cPortal;
class cGather;
class cTotem;
class cDropItem;

struct sNpcData;
struct sMonsterData;

class cObjectManager
{
public:
	cObjectManager( cDummyClient* dummy );
	~cObjectManager();

	void Clear();

	void Process( unsigned long deltaTime, unsigned long accumTime );

	cDummyClient* GetHero() { return mHero; }

	cObject* GetObject( unsigned char type, unsigned long idx );

	cPlayer* GetPlayer( unsigned long idx );
	cMonster* GetMonster( unsigned long idx );
	cNpc* GetNpc( unsigned long idx );
	cPortal* GetPortal( unsigned long idx );
	cGather* GetGather( unsigned long idx );
	cTotem* GetTotem( unsigned long idx );

	///
	bool AddPlayer( MSGROOT* pMsg );
	bool AddMonster( sMonsterData* data );
	bool AddNpc( sNpcData* data );
	bool AddPortal( unsigned long idx, unsigned long posIdx, NiPoint3 pos, NiMatrix3 rot );
	bool AddGather( unsigned long idx );
	bool AddTotem( unsigned long idx );
	bool AddDropItem( MSGROOT* pMsg );

	///
	void DelPlayer( unsigned long idx );
	void DelMonster( unsigned long idx );
	void DelNpc( unsigned long idx );
	void DelPortal( unsigned long idx );
	void DelGather( unsigned long idx );
	void DelTotem( unsigned long idx );
	void DelDropItem( unsigned long idx );

	cPortal* GetMapChangePortal( unsigned short mapIdx, NiPoint3 pos );
	cMonster* GetNearMonster( unsigned char level, NiPoint3 pos );

	cDropItem* GetNearDropItem( unsigned long idx, NiPoint3 pos );

	unsigned short GetFolderIdx() { return mFolderIdx; }
	void SetFolderIdx(unsigned short folderIdx) { mFolderIdx = folderIdx; }

protected:

	unsigned short mFolderIdx;

	cDummyClient* mHero;

	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;

	cPointerHashMap mPlayerMap;
	cPointerHashMap mMonsterMap;
	cPointerHashMap mNpcMap;
	cPointerHashMap mPortalMap;
	cPointerHashMap mGatherMap;
	cPointerHashMap mTotemMap;
	cPointerHashMap mDropItemMap;


	unsigned long mPortalGen;

};
