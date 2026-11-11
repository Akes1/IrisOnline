/* ====================================================================
* 파    일 :	ModelInstance.h
* 목    적 :	모델 로딩전에 .kfm 정보만을 미리 로당해 놓기 위해서 만든다.
* 작 성 자 :	이법석
* 작 성 일 :	2006.5
* 주의사항 :	게임의 시작전에 필요한 모델정보만 미리 로딩해 놓는다.
* =================================================================== */

#pragma once

class cModelInstance // : public NiRefObject
{
public:
	~cModelInstance();

	typedef tHashMap<unsigned int, cString> cStringMap;

	/// kfm파일을 읽어서 모델 인스턴스를 생성한다.
	static cModelInstance*	Create( const char* kfmFileName );

	/// 애니메이션 인덱스 정보를 담고있는 클래스 제공
	NiKFMToolPtr		GetKFMTool() { return mpKFMTool; }

	/// 애니 정보를 복사해서 제공한다.
	NiSequenceData*	CloneSequenceData( unsigned int eSequenceID );

	inline cStringMap* GetSequenceNameMap() { return &mSequenceNameMap; }

protected:
	cModelInstance();

	/// 로드된 kfm파일 정보를 기반으로 모델과 애니 및 확장정보를 얻는다.
	bool Init( NiKFMTool* kfmTool, NiStream& stream );

	/// 애니정보 .kf파일 로딩 및 추가
	bool AddSequenceData( unsigned int eSequenceID, NiStream& stream, bool isLoadFileKF );

	/// skined 객체에 대한 픽킹용 데이터를 생성한다...
	void CreatePickingData( NiAVObject* pobj );

protected:

	/// 모델의 모든 정보를 가지고 있는 게임브리오 클래스
	NiKFMToolPtr mpKFMTool;

	/// 복사본을 제공하기 위해서, 기본적으로 생성한 애니 정보 저장소
	NiTPointerMap<unsigned int, NiSequenceDataPtr> mSequenceDataMap;

	cStringMap mSequenceNameMap;
};

//NiSmartPointer(cModelInstance);
