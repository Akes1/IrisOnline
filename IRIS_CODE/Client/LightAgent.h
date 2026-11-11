/* ====================================================================
 * 파    일 :	LightAgent.h
 * 목    적 :	객체 및 환경의 Light를 관리 및 처리
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.05
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.05     이법석	    테스트 버젼
 * 
 * 주의사항 :	Dynamic Light는 현재 추가 분석및 변형 해야 한다.
 * =================================================================== */
#pragma once

class cLightAgent
{
public:
	static cLightAgent*	mpLightAgent;

	cLightAgent();
	~cLightAgent();

	/// 초기화
	bool Init();

	void Close();

	///
	void AttachSkyLight( NiNode* pNode );

	/// 하늘 주변광
	void SetSkyAmbientLight( const NiColor& ambient, float dimmer );
	void SetSkyAmbient( const NiColor& ambient );
	void SetSkyDimmer( float dimmer );

	/// 지형 주변광
	void SetTerrainAmbientLight( const NiColor& ambient, float dimmer );
	void SetTerrainAmbient( const NiColor& ambient );
	void SetTerrainDimmer( float dimmer );

	/// 장면 주변광
	void SetSceneAmbient( unsigned int i, const NiColor& ambient );
	void SetSceneDimmer( unsigned int i, float dimmer );

	/// 장면 방향광
	NiDirectionalLight* GetSceneDirLight( unsigned int i ) const;

	/// Dimmer 정보
	//void	SetDimmer( unsigned int lightType, float dimmer );
	//float	GetDimmer( unsigned int lightType );

	/// Color 정보
	//void			SetNiColor( unsigned int lightType, NiColor color );
	//const NiColor&	GetNiColor( unsigned int lightType );

	/// 주변광을 리턴
	NiAmbientLight* GetTerrainAmbientLight() const;
	NiAmbientLight* GetSceneAmbientLight( unsigned int i ) const;

	NiAmbientLight* GetSelectAmbientLight() const;

	/// 월드의 전체 기본 Light정보
	//NiDirectionalLight* GetWorldLight() { return mpCharacterDirLight[eLIGHT_MAIN]; }

//protected:
	/// 기본 Light클래스를 사용하지 않고 변형이 용이하게 제작 해본다.
	/// 테스트
	//template <class T>
	//T* GetLight( NiNode* pNode, T* dummy = 0 );

protected:
	NiAmbientLightPtr mSkyAmbientLight;
	NiAmbientLightPtr mTerrainAmbientLight;

	NiAmbientLightPtr mSceneAmbientLight[2];
	NiDirectionalLightPtr mSceneDirLight[2];

	NiAmbientLightPtr mSelectAmbientLight;
};

inline
NiAmbientLight* cLightAgent::GetTerrainAmbientLight() const
{
	return mTerrainAmbientLight;
}

inline
NiAmbientLight* cLightAgent::GetSceneAmbientLight( unsigned int i ) const
{
	assert( i == 0 || i == 1 );

	return mSceneAmbientLight[i];
}

inline
NiDirectionalLight* cLightAgent::GetSceneDirLight( unsigned int i ) const
{
	assert( i == 0 || i == 1 );

	return mSceneDirLight[i];
}

inline
NiAmbientLight* cLightAgent::GetSelectAmbientLight() const
{
	return mSelectAmbientLight;
}

#define LIGHTAGENT cLightAgent::mpLightAgent
