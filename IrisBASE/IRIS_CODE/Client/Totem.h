#pragma once

class cEffectSceneNode;

class cTotem
{
public:
	cTotem();
	virtual ~cTotem();

	bool Init( unsigned long totemClassIdx, NiPoint3 pos, float dir );

	//void Update( unsigned long deltaTime, unsigned long accumTime);

protected:

	void AddLoopEffect( cEffectSceneNode* pNode );

protected:
	tPointerList<void*>	mLoopEffect;
};
