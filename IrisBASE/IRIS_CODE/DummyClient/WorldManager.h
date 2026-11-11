#pragma once

class cNaviField;
class cPathFinder;

///
struct sMapPathInfo
{
	cNaviField*		mNaviField;
	cPathFinder*	mPathFinder;
	float			mMapSize;

	sMapPathInfo()
	{
		mNaviField = 0;
		mPathFinder = 0;
		mMapSize = 0.0f;
	}
};

///
class cWorldManager
{
	static cWorldManager* mpSingleton;

public:
	cWorldManager();
	~cWorldManager();

	static cWorldManager* GetSingleton();


	/// ¸Ê´ç Á¤º¸

	bool Init();

	bool CalcHeight( unsigned short folderIdx, float* height, float x, float y );

	cPathFinder* GetPathFinder( unsigned long mapIdx, unsigned char mapMode );
	cPathFinder* GetPathFinder( unsigned long folderIdx );

protected:

	/// Load
	bool LoadMapData( unsigned int folderIdx );

private:
	typedef tPointerHashMap<unsigned long, void*> cMapInfoMap;
	cMapInfoMap mMapInfoMap;
};

inline
cWorldManager* cWorldManager::GetSingleton()
{
	return mpSingleton;
}

#define WORLDMAN	cWorldManager::GetSingleton()
