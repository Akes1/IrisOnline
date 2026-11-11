 /* ====================================================================
 * 파    일 :	CullingProcess.h
 * 목    적 :	NiCullingProcess를 변형하여.. 알파블랜딩을 중심으로	
 *				두개의 저장공간에 나누어 저장.
 *				또한, 객체의 데이터형으로 cEntity를 사용하고, 최종적으로
 *				두개의 NiVisibleArray에 분산적으로 데이터를 적재한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.03
 * 주의사항 :	
 * =================================================================== */
#pragma once

class cSceneNode;

/// 이펙트 노드 전용 컬러
class cCullingProcess : public NiMeshCullingProcess //NiCullingProcess
{
public:
	cCullingProcess( NiVisibleArray* palphaArray, NiVisibleArray* alphaTestArray, NiVisibleArray* psolidArray );
	virtual ~cCullingProcess();

	/// 처리부
	void Cull( const NiCamera* pcamera, cSceneNode* pnode);

protected:
	virtual void AppendVirtual( NiRenderObject& geom );
	virtual void Process( NiAVObject* pobj );

protected:
	NiVisibleArray* mpAlphaArray;
	NiVisibleArray* mpAlphaTestArray;
};

class cModifierProcess : public NiMeshCullingProcess
{
public:
	cModifierProcess();
	virtual ~cModifierProcess();

	void Cull( const NiCamera* pcamera, cSceneNode* pnode );

protected:
	virtual void AppendVirtual( NiRenderObject& /*geom*/ ){}
	virtual void Process( NiAVObject* pobj );
	virtual void Process(const NiCamera* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet);
};

/*
class cCullingProcess : public NiCullingProcess
{
public:
cCullingProcess( NiVisibleArray* pAlphaSet, NiVisibleArray* pNoneAlphaSet );
virtual ~cCullingProcess();

/// 처리부
void Process( const NiCamera* pCamera, cSceneNode* pSceneNode );

/// 
inline NiVisibleArray*	GetAlphaVisibleSet() { return mpAlphaArray; }
inline bool				IsCullingAll() { return mCullingAll; }
inline void				SetAlphaCheckDistance( float dist ) { mAlphaCheckMaxDistance = dist; }
inline void				EnableAlphaCollisionCheck( bool enable ) { mAlphaCollisionCheck = enable; }

protected:
/// NiCullingProcess제공 가상 함수
virtual void AppendVirtual( NiGeometry& visible );
virtual void Process( NiAVObject* pObj );

/// 카메라 시야를 방해하는 객체인지를 판별한다.
bool InterruptTheView( NiPoint3 camPos, NiPoint3 camDir, NiAVObject* pObject, float& rDist );

protected:

/// Alpha처리가 되는 객체는 따로 구분한다.
NiVisibleArray* mpAlphaArray;

/// 처리되는 현재 객체의 알파 여부 플래그(ex.카메라 충돌)
bool mCurrentObjectAlphaBlendOption;

/// 처리되는 현재 객체를 모두 컬링해야 하는지 여부 플래그
bool mCullingAll;

/// 알파 처리 되어야 하는 객체에 대한 Picking
NiPick		mAlphaCheckPick;

/// 알파 체크하는 객체의 최대 거리
float		mAlphaCheckMaxDistance;

/// 직선과 물체간의 충돌처리로 알파여부를 판단 ( 배경객체에 사용 )
bool		mAlphaCollisionCheck;
};
*/