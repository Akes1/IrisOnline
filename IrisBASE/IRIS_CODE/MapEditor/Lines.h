/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.17
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cCamera;
class cSceneNode;
class cTerrain;
class cNaviMesh;
class cNaviField;

/// 원
/// 선택된 노드를 표시하기위해 사용한다.
class cCircleLines
{
public:
	cCircleLines( const NiColorA& color0, const NiColorA& color1 );

	void Process( const NiPoint3& center, float radius, cTerrain* terrain );
	void Process( const NiPoint3& center, float radius, cNaviMesh* naviMesh );
	void Process( const NiPoint3& center, float radius, cNaviField* naviField );
	void Process( cSceneNode* node, cCamera* camera );
	
	void Render();

protected:
	void SetActiveCount( unsigned int count );

public:
	NiMeshPtr mLines;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;
};
