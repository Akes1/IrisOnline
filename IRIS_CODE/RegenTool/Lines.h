/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.17
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cCamera;
class cSceneNode;

/// 원
/// 선택된 노드를 표시하기위해 사용한다.
class cCircleLines
{
public:
	cCircleLines( const NiColorA& color );

	void Render( cSceneNode* node, cCamera* camera );

private:
	NiMeshPtr mLines;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;
};
