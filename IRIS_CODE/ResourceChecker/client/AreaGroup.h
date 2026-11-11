/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.11.19
* 내    용 : 영역 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIImage.h"

class cSphere;
class cAreaSceneNode;

/// 영역 그룹
class cAreaGroup
{
	friend class cSceneManager;
public:
	cAreaGroup( NiAudioSource* snd, NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th, unsigned int nameIdx );
	~cAreaGroup();

	/// 처리
	void OnProcess( unsigned long deltaTime, unsigned long accumTime, const cSphere& heroSphere );

	/// 렌더링
	void OnRender();

	/// 영역
	void AddArea( cAreaSceneNode* area );

	///
	void FadeOut();

	bool IsHeroIn() { return mHeroIn; }

	void SetActive( bool active );

private:
	/// 영역 노드 리스트
	typedef tList<cAreaSceneNode*> cAreaList;
	cAreaList mAreaList;

	///
	unsigned long mTempTime;
	float mTargetAlpha;
	float mAlpha;

	///
	bool mHeroIn;

	/// 간판 이미지
	cUIImage mImage;
	unsigned int mImageWidth;

	unsigned int mNameIndex;

	/// sound
	NiAudioSource*		mSound;

	/// 현재 소속 지역..
	static cAreaGroup* mCurrentGroup;
};