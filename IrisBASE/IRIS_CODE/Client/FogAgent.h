/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.23
* 내    용 : 안개 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 안개 관리자
class cFogAgent
{
public:
	static cFogAgent*	mpFogAgent;

	cFogAgent();
	~cFogAgent();

	/// 안개 속성을 부착
	//void AttachFogProperty( NiAVObject* pObj );

	/// 안개를 설정
	//void SetFog( const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );
	NiFogProperty* GetFogProperty() const;

protected:
	/// 안개 속성
	NiFogPropertyPtr mpFogProp;
};

inline NiFogProperty* cFogAgent::GetFogProperty() const
{
	return mpFogProp;
}

#define FOGAGENT cFogAgent::mpFogAgent
