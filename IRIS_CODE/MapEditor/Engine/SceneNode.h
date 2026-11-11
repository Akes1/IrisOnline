/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.08
* 내    용 : 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Ray.h"
#include "Sphere.h"
#include "MaterialData.h"

class cSceneCuller;
class cLightSceneNode;

/// 장면 노드 타입
enum eSceneNodeType
{
	SCENENODE_NULL		= 0,
	SCENENODE_RENDERING	= (1 <<  0),

	SCENENODE_AREA		= (1 <<  1),
	SCENENODE_LIGHT		= (1 <<  2),
	SCENENODE_SOUND		= (1 <<  3),
	SCENENODE_STATIC	= (1 <<  4) | SCENENODE_RENDERING,
	SCENENODE_DYNAMIC	= (1 <<  5) | SCENENODE_RENDERING,
	SCENENODE_EFFECT	= (1 <<  6) | SCENENODE_RENDERING,

	SCENENODE_ITEM		= (1 <<  7) | SCENENODE_STATIC,
	SCENENODE_PORTAL	= (1 <<  8) | SCENENODE_STATIC,
	SCENENODE_GATHERING	= (1 <<  9) | SCENENODE_STATIC,
	SCENENODE_PATTERN	= (1 << 10) | SCENENODE_STATIC,
	SCENENODE_DOOR		= (1 << 11) | SCENENODE_STATIC,

	SCENENODE_PLAYER	= (1 << 12) | SCENENODE_DYNAMIC,
	SCENENODE_NPC		= (1 << 13) | SCENENODE_DYNAMIC,
	SCENENODE_MONSTER	= (1 << 14) | SCENENODE_DYNAMIC,

	SCENENODE_HERO		= (1 << 15) | SCENENODE_PLAYER,
};

enum eSceneNodeTypeVer8
{
	SCENENODE_NULL_VER8	= 0,
	SCENENODE_AREA_VER8,
	SCENENODE_LIGHT_VER8,
	SCENENODE_SOUND_VER8,
	SCENENODE_STATIC_VER8,
	SCENENODE_DYNAMIC_VER8,
};

/// 알파 데이타
class cAlphaData
{
public:
	cAlphaData( bool blendEnabled, bool testEnabled, NiAlphaProperty::TestFunction testFunc, unsigned char testRef, NiAlphaProperty* prop );
	~cAlphaData();

public:
	/// 알파 블렌딩
	bool mBlendEnabled;

	/// 알파 테스팅
	bool mTestEnabled;
	NiAlphaProperty::TestFunction mTestFunc;
	unsigned char mTestRef;

	/// 알파 속성
	NiAlphaProperty* mProp;
};

class cVertexColorData
{
public:
	cVertexColorData( NiVertexColorProperty::SourceVertexMode sourceMode, NiVertexColorProperty::LightingMode lightMode, NiVertexColorProperty* prop );
	~cVertexColorData();

public:
	NiVertexColorProperty::SourceVertexMode mSourceMode;
	NiVertexColorProperty::LightingMode mLightMode;

	NiVertexColorProperty* mProp;
};

/// 장면 노드 인자
class cSceneNodeParam
{
public:
	cSceneNodeParam();

public:
	unsigned long mWoldAccumTime;

	/// 경로 이름
	mutable cString mPathName;

	/// 위치
	NiPoint3 mTranslate;

	/// 회전
	NiMatrix3 mRotate;

	/// 배율
	float mScale;

	/// 픽킹
	mutable NiPick::QueryType mQueryType;
	mutable NiPick::SortType mPickSort;
	mutable NiPick::IntersectType mPickIntersect;
	mutable NiPick::CoordinateType mPickCoordinate;
	mutable bool mPickFrontOnly;
	mutable bool mPickReturnTexture;
	mutable bool mPickReturnNormal;
	mutable bool mPickReturnColor;
};

/// 장면 노드
class cSceneNode
{
	friend class cSceneManager;
	friend class cSceneTree;
	friend class cSceneTreeNode;
	friend class cObjectEditor;
	friend class cLightEditor;
	friend class cObjectDeletingInfo;

public:
	/// 처리
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );
	virtual void ProcessAlpha( unsigned long deltaTime );
	virtual bool OnVisible( cSceneCuller& culler );

	/// 갱신
	void Update();
	void UpdateAlpha();
	void UpdateTransform();

	virtual void SetAlphaBlended( float targetAlpha );
	bool IsAlphaBlended();

	virtual void SetViewNode( bool view );
	virtual bool IsViewNode() { return mViewNode; }

	/// 픽킹
	virtual bool Pick( const cRay& ray );

	/// 픽킹 결과
	/// 주의: Pick()를 호출한 직후에만 사용해야 한다.
	const NiPick::Results& GetPickResults();
	const NiPoint3& GetPickPos();
	float GetPickDistance();

	/// 조명
	void RemoveLight( cLightSceneNode* node );

	/// 타입
	eSceneNodeType GetType();
	bool IsKindof( eSceneNodeType type );

	bool IsEnableFrustumFlag() { return mCheckFrustum; }

	/// 엔진 오브젝트
	virtual inline NiNode* GetNiNode() { return mSceneNiNode; }
	virtual inline NiAVObject* GetNiObj() { return (NiAVObject*)mSceneNiNode; }

	/// 이동
	void Translate( const NiPoint3& move );
	virtual void SetTranslate( const NiPoint3& trans );
	const NiPoint3& GetTranslate();
	virtual const NiPoint3& GetWorldTranslate();

	/// 회전
	void SetRotate( const NiPoint3& axis, float angle );
	void SetRotate( const float xangle, float yangle, float zangle );
	virtual void SetRotate( const NiMatrix3& rot );
	const NiMatrix3& GetRotate();
	const NiMatrix3& GetWorldRotate();

	/// 크기
	void SetScale( float scale );
	float GetScale();
	float GetWorldScale();

	/// 경계 구
	const cSphere& GetBoundSphere();

	/// 중심점
	virtual const NiPoint3& GetCenter();

	/// 반지름
	virtual float GetRadius();

	/// 평면의 어느쪽
	unsigned int WhichSide( const NiPlane& plane );

	/// 파일 이름
	const cString& GetFileName();

	/// 사용자 데이타
	void SetUserData0( unsigned int data );
	void SetUserData1( unsigned int data );
	void SetUserData2( unsigned int data );
	void SetUserData3( unsigned int data );
	void SetUserData4( unsigned int data );
	void SetUserData5( unsigned int data );
	void SetUserData( void* data );
	unsigned int GetUserData0();
	unsigned int GetUserData1();
	unsigned int GetUserData2();
	unsigned int GetUserData3();
	unsigned int GetUserData4();
	unsigned int GetUserData5();
	void* GetUserData();

	/// 갱신 필요 여부
	void SetNeedUpdateTransform( bool update );
	void SetNeedUpdateEffects( bool update );

	/// 기하 오브젝트 수
	unsigned int GetNumGeoms();

#ifdef REGEN_TOOL
	/// 컬링
	void SetCulled( bool culled );
#endif

protected:
	cSceneNode( eSceneNodeType = SCENENODE_NULL );
	virtual ~cSceneNode();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 초기화
	virtual bool Init( const cSceneNodeParam& param );

	/// 재귀 함수
	void RecursiveNodeInfo( NiAVObject* obj, bool geom = true, bool property = true );

	virtual void CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty );

	void ClearCollectInfo();

	/// 기하 오브젝트
	NiMesh* GetGeom( NiAVObject* obj );

	/// 기하 오브젝트들을 가시 배열에 추가
	virtual void AddToVisibleArray( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray );

protected:
	/// 타입
	eSceneNodeType mType;

	/// 이름
	cString mFilePath;
	cString mFileName;

	/// 사용자 데이타
	unsigned int mUserData0;
	unsigned int mUserData1;
	unsigned int mUserData2;
	unsigned int mUserData3;
	unsigned int mUserData4;	/// 동족인식 그룹번호
	unsigned int mUserData5;	/// 패턴 index
	void*		 mUserData;
	
	/// 엔진 오브젝트
//	NiAVObjectPtr mNiObject;
	NiNodePtr mSceneNiNode;

	/// 재질 데이타 리스트
	typedef tPointerList<void*> cMaterialDataList;
	cMaterialDataList mMatDataList;

	/// 알파 데이타 리스트
	typedef tPointerList<void*> cAlphaDataList;
	cAlphaDataList mAlphaDataList;

	typedef tPointerList<void*> cVertexColorList;
	cVertexColorList mVertexColorDataList;


	/// 기하 오브젝트 리스트
	typedef tPair<NiRenderObject*, int> cRenderObjPair;
	typedef tList<cRenderObjPair> cRenderObjList;
	cRenderObjList mRenderObjList;

	/// 빌보드 노드 리스트
	typedef tList<NiBillboardNode*> cBillboardList;
	cBillboardList mBillboardList;

	/// 픽킹
	/// 장면 트리를 이용한 충돌 검사 과정에서 픽킹을 수행하고, 그 결과를 저장한다.
	bool mPickBound;
	NiPick mPick;
	NiPoint3 mPickPos;
	float mPickDistance;

	bool mViewNode;

	/// 갱신 필요 여부
	bool mNeedUpdateProperties;
	bool mNeedUpdateTransform;
	bool mNeedUpdateEffects;

	/// 경계 구
	cSphere mBoundSphere;

	/// 알파
	bool mEnableAlphaProcess;
	float mTargetAlpha;
	float mAlpha;
	float mAlphaSpeed;

	/// 조명 배열
	typedef tArray<cSceneNode*> cLightArray;
	cLightArray mLightArray;

	/// 장면 관리자에 의해 설정된 내부 인덱스
	unsigned int mIndexByManager;

	/// 컨테이너
	/// 장면 노드를 포함하는 장면 트리 노드
	cSceneTreeNode* mContainer;

	/// 컨테이너에 의해 설정된 내부 반복자
	tList<cSceneNode*>::cIterator mIteratorByContainer;

	bool mCheckFrustum;

#ifdef REGEN_TOOL
	bool mCulled;
#endif
};

inline
void cSceneNode::SetNeedUpdateTransform( bool update )
{
	mNeedUpdateTransform = update;
}

inline
void cSceneNode::SetNeedUpdateEffects( bool update )
{
	mNeedUpdateEffects = update;
}

inline
void cSceneNode::SetAlphaBlended( float targetAlpha )
{
	mTargetAlpha = targetAlpha;

	if( mAlpha != mTargetAlpha )
		mEnableAlphaProcess = true;
}

inline
bool cSceneNode::IsAlphaBlended()
{
	if( mAlpha < 1.0f - FLT_EPSILON )
		return true;
	else
		return false;
}

inline
eSceneNodeType cSceneNode::GetType()
{
	return mType;
}

inline
bool cSceneNode::IsKindof( eSceneNodeType type )
{
	return (mType & type) == type;
}

inline
const NiPick::Results& cSceneNode::GetPickResults()
{
	return mPick.GetResults();
}

inline
const NiPoint3& cSceneNode::GetPickPos()
{
	return mPickPos;
}

inline
float cSceneNode::GetPickDistance()
{
	return mPickDistance;
}

inline
const NiPoint3& cSceneNode::GetTranslate()
{
	return GetNiObj()->GetTranslate();
}

inline
const NiMatrix3& cSceneNode::GetRotate()
{
	return GetNiObj()->GetRotate();
}

inline
float cSceneNode::GetScale()
{
	return GetNiObj()->GetScale();
}

inline
const NiPoint3& cSceneNode::GetWorldTranslate()
{
	return GetNiObj()->GetWorldTranslate();
}

inline
const NiMatrix3& cSceneNode::GetWorldRotate()
{
	return GetNiObj()->GetWorldRotate();
}

inline
float cSceneNode::GetWorldScale()
{
	return GetNiObj()->GetWorldScale();
}

inline
const cSphere& cSceneNode::GetBoundSphere()
{
	return mBoundSphere;
}

inline
unsigned int cSceneNode::WhichSide( const NiPlane& plane )
{
	float dist = plane.Distance( GetCenter() );
	float radius = GetRadius();

	if( dist <= -radius )
		return NiPlane::NEGATIVE_SIDE;
	else if( dist >= radius )
		return NiPlane::POSITIVE_SIDE;
	else
		return NiPlane::NO_SIDE;
}

inline
const cString& cSceneNode::GetFileName()
{
	return mFileName;
}

inline
void cSceneNode::SetUserData0( unsigned int data )
{
	mUserData0 = data;
}

inline
void cSceneNode::SetUserData1( unsigned int data )
{
	mUserData1 = data;
}

inline
void cSceneNode::SetUserData2( unsigned int data )
{
	mUserData2 = data;
}

inline 
void cSceneNode::SetUserData3( unsigned int data )
{
	mUserData3 = data;
}

inline 
void cSceneNode::SetUserData4( unsigned int data )
{
	mUserData4 = data;
}

inline 
void cSceneNode::SetUserData5( unsigned int data )
{
	mUserData5 = data;
}

inline
void cSceneNode::SetUserData( void* data )
{
	mUserData = data;
}

inline
unsigned int cSceneNode::GetUserData0()
{
	return mUserData0;
}

inline
unsigned int cSceneNode::GetUserData1()
{
	return mUserData1;
}

inline
unsigned int cSceneNode::GetUserData2()
{
	return mUserData2;
}

inline
unsigned int cSceneNode::GetUserData3()
{
	return mUserData3;
}

inline
unsigned int cSceneNode::GetUserData4()
{
	return mUserData4;
}

inline
unsigned int cSceneNode::GetUserData5()
{
	return mUserData5;
}

inline
void* cSceneNode::GetUserData()
{
	return mUserData;
}

inline
unsigned int cSceneNode::GetNumGeoms()
{
	return mRenderObjList.GetSize();
}

#ifdef REGEN_TOOL
inline
void cSceneNode::SetCulled( bool culled )
{
	mCulled = culled;
}
#endif

/// 장면 노드 비교 함수자
/// 픽킹 거리로 비교한다.
class cSceneNodeCompareForPicking
{
public:
	bool operator () ( cSceneNode* left, cSceneNode* right )
	{
		return left->GetPickDistance() < right->GetPickDistance();
	};
};
