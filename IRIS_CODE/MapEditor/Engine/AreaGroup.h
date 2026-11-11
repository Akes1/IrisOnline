/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.11.19
* 내    용 : 영역 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

#ifdef MAP_EDITOR
#include "UIImage.h"

class cSphere;
class cAreaSceneNode;

/// 영역 그룹
class cAreaGroup
{
public:
	cAreaGroup( NiAudioSource* snd, NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th );
	
	/// 처리
	void Process( unsigned long deltaTime, const cSphere& heroSphere );

	/// 렌더링
	void OnRender();

	/// 영역
	void AddArea( cAreaSceneNode* area );

	///
	void FadeOut();

private:
	/// 영역 노드 리스트
	typedef tList<cAreaSceneNode*> cAreaList;
	cAreaList mAreaList;

	///
	float mTempTime;
	float mTargetAlpha;
	float mAlpha;
	float mTargetGain;
	float mGain;

	///
	bool mHeroIn;

	/// 간판 이미지
	cUIImage mImage;

	/// 사운드
	NiAudioSource* mSound;

	///
	static cAreaGroup* mCurrentGroup;
};
#endif /// MAP_EDITOR
