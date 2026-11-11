/* ====================================================================
* 목    적 :	모드 그래픽 리소스에 대한 로딩 인터페이스 제공 및 보관 메모리 제공
* 작 성 자 :	이법석
* 작 성 일 :	2006.01
* 주의사항 :	쓰레드용 인터페이스는 별도의 용도이므로 임의 사용을 제한한다.
*				이곳에 로드되는 객체및 텍스쳐는 모두 유일하다.
* =================================================================== */
#pragma once

#include "TexturePalette.h"
#include "ModelInstance.h"

#ifndef _OBJ_LINK_INFO
#define _OBJ_LINK_INFO
enum eLINK
{
	eLINK_HEAD = 0,
	eLINK_BODY,
	eLINK_FOOT,
	eLINK_RHAND,
	eLINK_LHAND,
	eLINK_LARM,

	eLINK_HEAD2,

	eLINK_RWEAPON = 10,
	eLINK_LWEAPON,
};

enum ePART
{
	ePART_HAIR = 0,
	ePART_FACE,
	ePART_BODY1,
	ePART_BODY2,
	ePART_HAND,
	ePART_FOOT,

	ePART_MAX,

	/// 귀 분리객체
	ePART_EAR = 100,
};
#endif

class cCallbackStreamPACK;
class cMonsterSceneNode;

/// 리소스 관리자
class cResourceManager
{
	static cResourceManager* mpSingleton;

public:
	cResourceManager();
	~cResourceManager();

	enum
	{
		eCHARACTEREFFECT = 0,
		eOBJECTEFFECT,

		/// 파일에 지정된 정보를 그대로 사용한다.
		eFILEEFFECT,
	};

	/// 초기화
	bool Init();

	void Close();

	/// 종료
	void Exit();

	/// 기본 객체 로드 함수
	bool LoadNIF( const cString& pathName, NiAVObject** nifRoot = 0);
	bool LoadMapNIF( const cString& pathName );

	/// 쓰레드를 이용한 객체 로드(임의 사용을 제한한다)
	void LoadThreadNIF( const char* fileName );

	/// 로드 쓰레드 종료부
	void FinishThread();

	/// 텍스처
	void ChangeStageImageTexture( NiTexture* tex );

	NiTexture* LoadTexture( const cString& pathName, bool useMipMap = true );
	LPDIRECT3DTEXTURE9 LoadD3DTexture( const cString& pathName, bool useMipMap = true, DWORD filter = D3DX_FILTER_LINEAR, DWORD mipFilter = D3DX_FILTER_LINEAR );
	LPDIRECT3DTEXTURE9 GetD3DTexture( const cString& pathName );

	NiTexture* LoadMapTexture( const cString& pathName, bool useMipMap = true );

	/// 사운드
	NiAudioSource* LoadStageSound( const cString& fileName );
	NiAudioSource* LoadTotalSound( const cString& fileName );

	/// 쓰레드용 처리부
	void Process();

	/// 로드된 nif파일 객체를 저장공간에 추가한다.
	void AddObject( NiNode* n, const cString& pathName, bool mapObj );

	/// 객체 제거용 인터페이스
	void RemoveObject( const char* objectName );
	void RemoveAllMapTexture();
	void RemoveAllTexture();

	/// 객체 참조용 인터페이스
	NiNode*				GetObjectByName( const cString& pathName );

	NiAudioSource*		GetStageSoundByName( const cString& fileName );
	NiAudioSource*		GetTotalSoundByName( const cString& fileName );

	/// 객체 복사용 인터페이스
	NiNode*				CloneObjectByName( const cString& pathName );
	NiAudioSource*		CloneStageSoundByName( const cString& fileName );
	NiAudioSource*		CloneTotalSoundByName( const cString& fileName );

	NiNodePtr			DeepCopyObjectByName( const cString& pathName );

	/// kfm 파일 모델
	cModelInstance* LoadKFM( const cString& pathName );
	void AddModel( cModelInstance* model, const cString& pathName );
	void RemoveAllModels();

	/// 그림자 텍스처
	NiTexture* GetShadowTexture() const;
	NiTexture* GetFieldTargetTexture() const;

	cModelInstance* GetModelByName( const cString& pathName );

	///
	const char* GetManagedPartName( unsigned int partIdx );
	const char* GetManagedDummyName( unsigned int dummyIdx );

	bool CheckDummyKey( const char* name );

public:
	/// 단일체를 리턴
	static cResourceManager* GetSingleton();


protected:
	void CheckStreams( NiAVObject* obj );



protected:
	/// 쓰레드 로딩 클래스
	cCallbackStreamPACK* mpStreamPAK;

	/// 크리티컬 섹션
	static NiCriticalSection mObjectListCriticalSection;
	static NiCriticalSection mSoundListCriticalSection;

	/// 텍스처 팔레트
	cTexturePalettePtr mTexturePalette;
	cTexturePalettePtr mMapTexturePalette;

	/// 3D 오브젝트
	NiTStringPointerMap<NiNodePtr> mObjectMap;
	tArray<cString> mMapObjNameArray;

	/// 모델
	NiTStringPointerMap<cModelInstance*> mModelMap;

	/// 사운드
	typedef tPointerHashMap<cString, NiAudioSource*> cSoundMap;
	cSoundMap mStageSound; // stage change - clear
	cSoundMap mTotalSound; // game 

	/// 텍스처
	typedef tHashMap<cString, void*> cD3DTextureMap;
	cD3DTextureMap mD3DTextureMap;

	/// 그림자 텍스처
	NiTexture* mShadowTexture;
	NiTexture* mFieldTargetTexture;

	NiTexturePtr mStageImageTexture;

	/// 캐릭터 파트별 관리 Geom객체의 이름 (그래픽 제작시 결정된 이름을 등록한다.)
	NiTPointerMap<unsigned int, const char*> mPartsNameMap;

	/// 관리 Dummy객체의 이름 (그래픽 제작시 결정된 이름을 등록한다.)
	NiTPointerMap<unsigned int, const char*> mDummyNameMap;
};

inline
NiTexture* cResourceManager::GetShadowTexture() const
{
	return mShadowTexture;
}

inline
NiTexture* cResourceManager::GetFieldTargetTexture() const
{
	return mFieldTargetTexture;
}

inline
cResourceManager* cResourceManager::GetSingleton()
{
	return mpSingleton;
}

#define RESOURCEMAN	cResourceManager::GetSingleton()
