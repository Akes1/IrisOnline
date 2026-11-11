/* ====================================================================
* 파    일 :	PlayerSceneNode.h
* 목    적 :	플레이어 캐릭터에 대한 모델 정보를 정의한다.
* 작 성 자 :	이법석
* 작 성 일 :	2006.5
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		2006.06     이법석	    생성
* 
* 주의사항 :
* =================================================================== */
#pragma once

#include "DynamicSceneNode.h"

class cPlayer;
class cSignboard;

/// 플레이어 장면 노드 인자
class cPlayerSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cPlayerSceneNodeParam();

public:
	bool mDisplayObj;
};

inline cPlayerSceneNodeParam::cPlayerSceneNodeParam()
{
	mDisplayObj = false;
}

/// 플레이어 장면 노드
class cPlayerSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cPlayerSceneNode();

	/// 초기화
	virtual bool Init( const cPlayerSceneNodeParam& param );

	/// 파트 교체
	virtual bool ChangeParts( unsigned int partIdx, const char* fileName );
	virtual bool ChangeParts( unsigned int partsIdx, NiNode* pNode );

	virtual bool ChangePartsTexture( unsigned int partsIdx, const char* fileName );

	/// 더미에 객체 링크
	virtual unsigned int LinkObject( unsigned int dummyIdx, const char* fileName );

	virtual NiAVObject*		GetWeaponDummy( unsigned int objectIdx, bool left );

	/// 객체의 중심좌표를 반환한다.
	virtual const NiPoint3& GetObjectCenter();
	virtual const NiPoint3& GetObjectFoot();
	virtual const NiPoint3& GetObjectHead();

	virtual void UpdateLinkEffect();

	virtual void HideEar( bool hide );

protected:
	cPlayerSceneNode( eType type = ePLAYER );

	/// 링크공간을 정의한다.
	virtual bool		InitLinkInfo();

	/// 기본 애니메이션을 정의한다.
	virtual void		InitializeRandomIdle();

	virtual void ProcessTrail( unsigned long deltatime, unsigned long accumtime );

protected:

	cSignboard*				mpTarotBoard;
	cSignboard*				mpUserSellBoard;
};