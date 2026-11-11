/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.08
* 내    용 : 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Ray.h"
#include "Sphere.h"

class NiCamera;
class NiVisibleArray;
class cLightSceneNode;

class NICOLLISION_ENTRY SimpleBoxPickObjectPolicy : 
	public NiPick::PickObjectPolicy
{
public:
	virtual bool FindIntersections(
		const NiPoint3& kOrigin, const NiPoint3& kDir, NiPick& kPick, 
		NiRenderObject* pkRenderObj);

	SimpleBoxPickObjectPolicy( bool onlyBound ) { mOnlyBound = onlyBound; }

protected:
	bool		mOnlyBound;

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
	/// 경로 이름
	cString mPathName;

	/// 위치
	NiPoint3 mTranslate;

	/// 회전
	NiMatrix3 mRotate;

	/// 배율
	float mScale;

	/// 픽킹
	NiPick::QueryType mQueryType;
	NiPick::SortType mPickSort;
	NiPick::IntersectType mPickIntersect;
	NiPick::CoordinateType mPickCoordinate;
	bool mPickFrontOnly;
	bool mPickReturnTexture;
	bool mPickReturnNormal;
	bool mPickReturnColor;
};

/// 장면 노드
class cSceneNode
{
	friend class cSceneManager;
	friend class cSceneTree;
	friend class cSceneTreeNode;

public:
	/// 장면 노드 타입
	enum eType
	{
		eNULL			= 0,
		eRENDERING		= (1 <<  0),

		eAREA			= (1 <<  1),
		eLIGHT			= (1 <<  2),
		eSOUND			= (1 <<  3),
		eSTATIC			= (1 <<  4) | eRENDERING,
		eMANAGEDSTATIC	= (1 <<  5) | eSTATIC,
		eDYNAMIC		= (1 <<  6) | eRENDERING,
		eEFFECT			= (1 <<  7) | eRENDERING,

		eDROPITEM		= (1 <<  8) | eMANAGEDSTATIC,
		eMAPPORTAL		= (1 <<  9) | eMANAGEDSTATIC,
		eGATHERING		= (1 << 10) | eMANAGEDSTATIC,
		eDOOR			= (1 << 11) | eMANAGEDSTATIC,

		ePLAYER			= (1 << 12) | eDYNAMIC,
		eNPC			= (1 << 13) | eDYNAMIC,
		eMONSTER		= (1 << 14) | eDYNAMIC,
		eVEHICLE		= (1 << 15) | eDYNAMIC,

		eHERO			= (1 << 16) | ePLAYER,
	};

	enum eTypeVer8
	{
		eNULL_VER8	= 0,
		eAREA_VER8,
		eLIGHT_VER8,
		eSOUND_VER8,
		eSTATIC_VER8,
		eDYNAMIC_VER8,
	};

public:
	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void ProcessAlpha( unsigned long deltaTime );
	virtual bool OnVisible();

	/// 픽킹
	virtual bool Pick( const cRay& ray );

	/// 픽킹 결과
	/// 주의: Pick()를 호출한 직후에만 사용해야 한다.
	const NiPick::Results& GetPickResults();
	const NiPoint3& GetPickPos();
	float GetPickDistance();

	/// 알파 블렌딩
	virtual void SetAlphaBlended( float targetAlpha );
	bool IsAlphaBlended();
	virtual void UpdateAlpha();

	/// 타입
	eType GetType();
	bool IsKindof( eType type );

	bool IsEnableFrustumFlag() { return mCheckFrustum; }

	/// 이동
	virtual void Translate( const NiPoint3& move );
	virtual void SetTranslate( const NiPoint3& trans );
	const NiPoint3& GetTranslate();

	virtual void SetWorldTranslate( const NiPoint3& trans );
	virtual const NiPoint3& GetWorldTranslate();

	/// 회전
	virtual void SetRotate( const NiPoint3& axis, float angle );
	virtual void SetRotate( float xangle, float yangle, float zangle );
	virtual void SetRotate( const NiMatrix3& rot );
	const NiMatrix3& GetRotate();
	const NiMatrix3& GetWorldRotate();

	/// 크기
	virtual void SetScale( float scale );
	float GetScale();
	float GetWorldScale();

	virtual void SetTargetScale( float /*scale*/ ) {}

	/// 경계 구
	const cSphere& GetBoundSphere();

	/// 중심점
	virtual const NiPoint3& GetCenter();

	/// 반지름
	virtual float GetRadius();

	/// 평면의 어느쪽
	virtual unsigned int WhichSide( const NiPlane& plane );

	/// 이름
	const char* GetName();

	static NiMesh* GetGeom( NiAVObject* obj );
	static void SetAppCulled( NiAVObject* obj, bool cull );

	virtual void SetSelectLightFlag( bool attach );

	virtual void Play( unsigned long ) {}
	virtual void Rew( unsigned long ) {}
	virtual void Stop() {}

	virtual void SetViewNode( bool view );
	virtual bool IsViewNode() { return mViewNode; }

	inline unsigned int GetIndexByManger() { return mIndexByManager; }

	virtual inline NiNode* GetNiNode() { return mSceneNiNode; }
	virtual inline NiAVObject* GetNiObj() { return (NiAVObject*)mSceneNiNode; }

	/// 시점에 의한 충돌시 필요처리 정리
	virtual bool UpdateCameraPickInfo() { return true; }
	virtual void UpdatePickInfo( void* /*extra*/ ) {}

protected:
	cSceneNode( eType = eNULL );
	virtual ~cSceneNode();

	/// 초기화
	virtual bool Init( const cSceneNodeParam& param );

	/// 그래픽 참조 정보( 변경시 갱신요구 )

	/// 재귀 함수
	void RecursiveNodeInfo( NiAVObject* obj, bool geom = true, bool property = true );

	virtual void CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty );

	virtual void ClearCollectInfo();

	/// 기하 오브젝트들을 가시 배열에 추가
	virtual void AddToVisibleArray();

	/// 가시 레벨
	unsigned int CalcVisibleLevel();

protected:
	/// 타입
	eType mType;

	/// 픽킹
	/// 장면 트리를 이용한 충돌 검사 과정에서 픽킹을 수행하고, 그 결과를 저장한다.
	NiPick mPick;
	NiPoint3 mPickPos;
	float mPickDistance;

	/// 
	bool mViewNode;

	/// 갱신 필요 여부
	bool mNeedUpdateTransform;

	/// 경계 구
	cSphere mBoundSphere;

	/// 알파
	bool mEnableAlphaProcess;
	float mTargetAlpha;
	float mAlpha;
	float mAlphaSpeed;

	/// over & select node attach selectLight
	bool mAttachSelectLight;
	bool mUpdateSelectLIght;

	/// 장면 관리자에 의해 설정된 내부 인덱스
	unsigned int mIndexByManager;

	/// 컨테이너에 의해 설정된 내부 반복자
	bool mIteratorValid;
	tPointerList<void*>::cIterator mIteratorByContainer;

	/// 재질 속성 리스트
	typedef tPointerList<void*> cMatPropList;
	cMatPropList mMatPropList;

	/// 알파 데이타 리스트
	typedef tPointerList<void*> cAlphaDataList;
	cAlphaDataList mAlphaDataList;

	typedef tList<void*> cVertexColorList;
	cVertexColorList mVertexColorDataList;

	/// 기하 오브젝트 리스트
	typedef tPair<NiRenderObject*, int> cRenderObjPair;
	typedef tList<cRenderObjPair> cRenderObjList;
	cRenderObjList mRenderObjList;

	/// 빌보드 노드 리스트
	typedef tList<void*> cBillboardList;
	cBillboardList mBillboardList;

	/// 엔진 오브젝트
//	NiAVObjectPtr mNiObject;
	NiNodePtr mSceneNiNode;

	/// 컨테이너
	/// 장면 노드를 포함하는 장면 트리 노드
	cSceneTreeNode* mContainer;

	bool mCheckFrustum;

protected:
	/// 컬링용
	static NiCamera* mCamera;
	static NiVisibleArray* mSolidArray;
	static NiVisibleArray* mAlphaArray;
	static NiVisibleArray* mAlphaTestArray;
};

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
cSceneNode::eType cSceneNode::GetType()
{
	return mType;
}

inline
bool cSceneNode::IsKindof( eType type )
{
	return (mType & type) == type;
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
const char* cSceneNode::GetName()
{
	return (const char*)GetNiObj()->GetName();
}

inline
void cSceneNode::SetSelectLightFlag( bool attach )
{
	if( mAttachSelectLight != attach )
	{
		mAttachSelectLight = attach;
		mUpdateSelectLIght = true;

		mNeedUpdateTransform = true;
	}
}

/// 장면 노드 비교 함수자
/// 픽킹 거리로 비교한다.
class cSceneNodeCompareForPicking
{
public:
	bool operator () ( const void* left, const void* right ) const
	{
		return ( ((cSceneNode*)left)->GetPickDistance() < ((cSceneNode*)right)->GetPickDistance() );
	};
};
