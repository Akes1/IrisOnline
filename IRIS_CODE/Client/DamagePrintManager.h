#pragma once

enum eDAMAGEPRINT_TYPE
{
	eDAMAGEPRINT_NORMAL_OUR = 0,
	eDAMAGEPRINT_NORMAL_ENEMY,

	eDAMAGEPRINT_CRITICAL_OUR,
	eDAMAGEPRINT_CRITICAL_ENEMY,

	eDAMAGEPRINT_MP_NORMAL,
	eDAMAGEPRINT_MP_CRITICAL,

	eDAMAGEPRINT_HEAL_OUR,
	eDAMAGEPRINT_HEAL_ENEMY,

	eDAMAGEPRINT_MP_HEAL,

	eDAMAGEPRINT_MAX,

	/// 단독 처리
	eDAMAGEPRINT_MISS,
};

class cBaseObject;

class cDamagePrintManager
{
public:
	static cDamagePrintManager* mpDamagePrintManager;

	cDamagePrintManager();
	~cDamagePrintManager();

	bool PrintDamage( cBaseObject* pobj, unsigned int damage, eDAMAGEPRINT_TYPE type, bool ownerHero );

protected:
	/// 관리 node 검색 및 획득
	bool FindPrintTextureInfo( NiNode* pNode, NiMesh** pArray );

private:
	/// 생성자에서 초기화
	NiRect<float>	mTextureRect[eDAMAGEPRINT_MAX][10];
};

#define DAMAGEPRINTMAN	cDamagePrintManager::mpDamagePrintManager
