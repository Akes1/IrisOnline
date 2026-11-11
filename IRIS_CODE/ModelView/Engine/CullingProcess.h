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