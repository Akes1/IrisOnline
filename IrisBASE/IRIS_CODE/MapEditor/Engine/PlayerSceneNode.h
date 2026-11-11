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

/// 플레이어 장면 노드 인자
class cPlayerSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cPlayerSceneNodeParam();
};

inline cPlayerSceneNodeParam::cPlayerSceneNodeParam()
{
}

/// 플레이어 장면 노드
class cPlayerSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;
	friend class cHero;

public:
	virtual ~cPlayerSceneNode();

	enum
	{
		/// 모델의 파트
		ePART_HAIR = 0,
		ePART_FACE,
		ePART_BODY1,
		ePART_BODY2,
		ePART_HAND,
		ePART_FOOT,
		ePART_MAX = ePART_FOOT + 1,		

		/// 링크를 위한 더미들
		eLINK_HEAD = ePART_MAX,
		eLINK_BODY,
		eLINK_RHAND,
		eLINK_LHAND,
		eLINK_FOOT,
		eLINK_LARM,

		eLINK_RWEAPON = ePART_MAX + 10,
		eLINK_LWEAPON,

		eLINK_MAX = eLINK_FOOT - eLINK_HEAD + 1,
	};

	/// 초기화
	virtual bool Init( const cPlayerSceneNodeParam& param );

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 파트 교체
	//virtual bool ChangeParts( unsigned int partIdx, const char* fileName );

	//virtual bool ChangePartsTexture( unsigned int partsIdx, const char* fileName );

	/// 더미에 객체 링크
	//virtual unsigned int LinkObject( unsigned int dummyIdx, const char* fileName );

	//virtual NiAVObject*		GetPartWeapon( unsigned int objectIdx, bool left );

	/// 객체의 중심좌표를 반환한다.
	virtual const NiPoint3& GetObjectCenter();

	/// 말풍선 표현.
	//virtual void ActiveTarotBubble( LPTSTR msg, unsigned long color );
	//virtual void DeActiveTarotBubble();

protected:
	cPlayerSceneNode( eSceneNodeType type = SCENENODE_PLAYER );

	/// 링크공간을 정의한다.
	virtual bool		InitLinkInfo();

	/// 분할 정보를 정의한다. (더미 포함)
	virtual bool		InitPartsName();

	/// 기본 애니메이션을 정의한다.
	virtual void		InitializeRandomIdle();

	///
	//virtual cObjectNameCard* CreateNameCard( cBaseObject* pObj );

	///
	//virtual void AddDepthPlaneObject() const;

protected:
	//cChatBubble*			mpTarotBubble;
};

///
//class cHeroSceneNode : public cPlayerSceneNode
//{
//	friend class cSceneManager;
//
//public:
//	virtual ~cHeroSceneNode();
//
//protected:
//	cHeroSceneNode( eType type = eHERO );
//
//	///
//	virtual void AddDepthPlaneObject() const;
//};
