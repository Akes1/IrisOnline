/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 리젠툴 뷰
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Npc.h"
#include "Monster.h"
#include "Portal.h"
#include "Gathering.h"
#include "Pattern.h"
#include "Door.h"

class cMapDialog;
class cNpcDialog;
class cMonsterDialog;
class cPortalDialog;
class cGatheringDialog;
class cPatternDialog;
class cDoorDialog;
class cCircleLines;

class cApplication;
class cCamera;
class cFreeCamera;
class cNaviMesh;
class cNaviField;
class cSceneNode;
class cNpcSceneNode;
class cMonsterSceneNode;
class cMonsterGroup;
class cPortalSceneNode;
class cGatheringSceneNode;
class cPathFinder;

struct sMapInfo
{
	unsigned int mapIdx;
	unsigned int mapfolerIdx;
	tList<unsigned int> degreeInfo;

	sMapInfo()
	{
		mapIdx = 0;
		mapfolerIdx = 0;
		degreeInfo.Clear();
	}
	~sMapInfo() 
	{
		degreeInfo.Clear();
	}
};


/// 편집 모드
//enum eEditMode
//{
//	eEDIT_NULL = -1,
//	eEDIT_NPC = 0,
//	eEDIT_PORTAL,
//	eEDIT_MONSTER,
//	eEDIT_MAX_MODES
//};

/// 리젠툴 뷰
class cRegenToolView : public CView
{
	static cRegenToolView* mSingleton;
	DECLARE_DYNCREATE( cRegenToolView )

public:
	cRegenToolView();
	virtual ~cRegenToolView();

	/// 지움
	void Clear();

	/// 초기화
	void Init();

	/// 갱신
	void Update();

	/// 화면 크기
	void Resize( unsigned int width, unsigned int height );

	/// 렌더링
	void RenderLines();

	/// 로딩
	bool LoadMapInfo();
	bool LoadMapNameList();
	bool LoadModelFileList();
	bool LoadNpcNameList();
	bool LoadNpcList();
	bool LoadMonsterNameList();
	bool LoadMonsterList();
	bool LoadMap( unsigned int mapIndex, unsigned int mapfolderIdx, unsigned int mapDegree );
	bool LoadNpcRegen( unsigned int mapIndex, unsigned int mapDegree );
	bool LoadMonsterRegen( unsigned int mapIndex, unsigned int mapDegree );
	bool LoadPortalRegen();
	bool LoadGatheringNameList();
	bool LoadGatheringList();
	bool LoadGatheringRegen( unsigned int mapIndex, unsigned int mapDegree );
	bool LoadPattern( unsigned int mapIndex, unsigned int mapDegree );
	bool LoadDoorRegen();

	/// 다이얼로그
	void SetMapDialog( cMapDialog* dlg );
	void SetNpcDialog( cNpcDialog* dlg );
	void SetMonsterDialog( cMonsterDialog* dlg );
	void SetPortalDialog( cPortalDialog* dlg );
	void SetGatheringDialog( cGatheringDialog* dlg );
	void SetPatternDialog( cPatternDialog* dlg );
	void SetDoorDialog( cDoorDialog* dlg );


	/// 편집 모드
	//void SetEditMode( eEditMode mode );
	//eEditMode GetEditMode() const;

	/// 카메라
	void SetCamFarDistance( float farDist );
	float GetCamFarDistance() const;
	void ZoomPersp();
	void ZoomOrtho();

	/// 선택 노드
	void SelectNode( cSceneNode* node, bool updateView = true );
	void DeleteSel();
	void SetSelTranslate( NiPoint3 trans, bool updateDialog = false );
	void SetSelRotate( float zangle, bool updateDialog = false );
	void SetSelAppendZ( float appendZ, bool updateDialog = false );

	void SetSelDoorInfo( float Twidth, float Bwidth, float LHeight, float RHeight );
	void SetSelDoorAppendY( float appendY );

	void SetSelPotalNameHeight( float nameHeight );

	cSceneNode* GetSelNode() const;
	NiPoint3 GetSelTranslate() const;
	float GetSelRotate() const;
	float GetSelAppendZ() const;


	/// 맵
	unsigned long GetCurrentMapIndex() { return mMapIndex; }

	const cString& GetMapName( unsigned int mapIdx );

	/// 모델 파일
	const cString& GetModelFile( unsigned int modelIndex ) const;

	/// Npc
	bool SaveNpcRegen();

	int CreateNpc( unsigned int index, float x, float y, float zdegree, bool updateDialog, bool updateView );
	int ChangeNpc( unsigned int index );
	int DeleteNpc( cNpcSceneNode* node );
	void SelectNpc( cNpcSceneNode* node );
	const cNpcInfo* GetNpcInfo( unsigned int index ) const;
	const cString& GetNpcNameByNameIndex( unsigned int nameIndex ) const;
	const cString& GetNpcNameByNpcIndex( unsigned int npcIndex ) const;

	/// 몬스터
	bool SaveMonsterRegen();

	int CreateMonsterGroup( unsigned int index );
	int DeleteMonsterGroup( unsigned int index );
	void SelectMonsterGroup( cMonsterGroup* group );

	int CreateMonster( unsigned int index, unsigned int x, unsigned int y, int zdegree, unsigned int regenWaitTime, 
		unsigned int regenLifeTime, bool updateDialog, bool updateView, unsigned long regenFamilyNum, unsigned long patternIdx );
	int ChangeMonster( unsigned int index );
	int DeleteMonster( cMonsterSceneNode* node );
	void SelectMonster( cMonsterSceneNode* node );
	const cMonsterInfo* GetMonsterInfo( unsigned int index ) const;
	const cString& GetMonsterNameByNameIndex( unsigned int nameIndex ) const;
	const cString& GetMonsterNameByMonIndex( unsigned int monIndex ) const;

	void AddFamilyGroup( unsigned long familyIdx, tHashSet<unsigned long>* pSet )	{ mMonsterFamilyMap.Insert( familyIdx, pSet ); }

	/// 포탈
	bool SavePortalRegen();

	int CreatePortal( unsigned int index, const char* potalName, float x, float y, unsigned int range, unsigned int targetIndex, int zdegree, float nameHeight, bool isCreated, bool updateDialog, bool updateView );
	int DeletePortal( cPortalSceneNode* node );
	void SelectPortal( cPortalSceneNode* node );
	cPortalInfo* GetPortalInfo( unsigned int index ) const;

	/// 채집
	bool SaveGatheringRegen();

	int CreateGatheringGroup( unsigned int index );
	int DeleteGatheringGroup( unsigned int index );
	void SelectGatheringGroup( cGatheringGroup* group );

	int CreateGathering( unsigned int index, unsigned int x, unsigned int y, int zdegree, float appandZ, bool updateDialog, bool updateView );
	int ChangeGathering( unsigned int index );
	int DeleteGathering( cGatheringSceneNode* node );
	void SelectGathering( cGatheringSceneNode* node );

	const cGatheringInfo* GetGatheringInfo( unsigned int index ) const;
	const cString& GetGatheringNameByNameIndex( unsigned int nameIndex ) const;
	const cString& GetGatheringNameByGatherIndex( unsigned int gatherIndex ) const;

	/// 패턴
	int CreatePattern( unsigned long patternIdx );
	int DeletePattern( unsigned long patternIdx );
	void SelectPattern( unsigned int patternIdx );
	
	/// 현재 선택된 패턴그룹에 패턴 삽입
	cPatternSceneNode* AddPatternPos( unsigned long patternIdx, NiPoint3 pos, unsigned long range = 0, bool relaxPoint = 0, unsigned long waitTime = 0 );
//	void AddPatternPosByLoad( unsigned long patternIdx, NiPoint3 pos, unsigned long range = 0, bool relaxPoint = 0, unsigned long waitTime = 0 );
	void LoadPatternPos();
	int DeletePatternPos( cPatternSceneNode* node );
	void SelectPatternPos( cPatternSceneNode* node );
	void ModifyPatternPos( NiPoint3 pos );

	void UpdatePatternCombo();
	bool SavePattern();

	///
	int CreateDoor( unsigned int index, const char* fileName, NiPoint2 pos, float appendZ, int zdegree, float* rectWidth, float* rectHeight, float rectAppendY, bool updateDialog, bool updateView );
	int DeleteDoor( cDoorSceneNode* node );
	void SelectDoor( cDoorSceneNode* node );
	cDoorInfo* GetDoorInfo( unsigned int index ) const;

	bool SaveDoorRegen();

	cPathFinder* GetPathFinder() { return mPathFinder; }

protected:
	BOOL PreCreateWindow( CREATESTRUCT& cs );
	void OnDraw( CDC* dc );

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize( UINT type, int cx, int cy );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT flags, short zdelta, CPoint point );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	afx_msg void OnRButtonDown( UINT flags, CPoint point );
	afx_msg void OnRButtonUp( UINT flags, CPoint point );
	afx_msg void OnMButtonDown( UINT flags, CPoint point );
	afx_msg void OnMButtonUp( UINT flags, CPoint point );
	afx_msg void OnKeyDown(UINT c, UINT repeats, UINT flags);
	afx_msg void OnKeyUp(UINT c, UINT repeats, UINT flags);
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cRegenToolView* GetSingleton();

private:
	/// 어플리케이션
	cApplication* mApplication;

	/// 네비메시
	cNaviMesh* mNaviMesh;

	/// 네비필드
	cNaviField* mNaviField;

	/// 다이얼로그
	cMapDialog* mMapDialog;
	cNpcDialog* mNpcDialog;
	cMonsterDialog* mMonsterDialog;
	cPortalDialog* mPortalDialog;
	cGatheringDialog* mGatheringDialog;
	cPatternDialog* mPatternDialog;
	cDoorDialog* mDoorDialog;

	/// 편집 모드
	//eEditMode mEditMode;

	/// 카메라
	cFreeCamera* mCamera;

	/// 모델
	cString mErrorModelFile;

	typedef tMap<unsigned int, cString> cModelFileMap;
	cModelFileMap mModelFileMap;

	/// 맵
	unsigned int mMapIndex;
	unsigned int mMapDegree;

	typedef tMap<unsigned int, cString> cMapNameMap;
	cMapNameMap mMapNameMap;

	typedef tMap<unsigned int, sMapInfo*> cMapInfoMap;
	cMapInfoMap mMapInfoMap;

	/// Npc
	cString mErrorNpcName;

	typedef tMap<unsigned int, cString> cNpcNameMap;
	cNpcNameMap mNpcNameMap;

	typedef tMap<unsigned int, cNpcInfo*> cNpcInfoMap;
	cNpcInfoMap mNpcInfoMap;

	typedef tMap<unsigned int, cNpcSceneNode*> cNpcNodeMap;
	cNpcNodeMap mNpcNodeMap;

	unsigned long mNpcListIdx;


	/// 몬스터
	cMonsterGroup* mSelMonsterGroup;
	cString mErrorMonsterName;

	typedef tMap<unsigned int, cString> cMonsterNameMap;
	cMonsterNameMap mMonsterNameMap;

	typedef tMap<unsigned int, cMonsterInfo*> cMonsterInfoMap;
	cMonsterInfoMap mMonsterInfoMap;

	typedef tMap<unsigned int, cMonsterGroup*> cMonsterGroupMap;
	cMonsterGroupMap mMonsterGroupMap;

	typedef tHashSet<unsigned long> cHashSet;
	typedef tMap<unsigned int, void*> cMonsterFamilyMap;
	cMonsterFamilyMap mMonsterFamilyMap;

	typedef tList<cMonsterSceneNode*> cMonsterList;

	/// 채집 객체
	cGatheringGroup* mSelGatheringGroup;
	cString mErrorGatheringName;

	typedef tMap<unsigned int, cString> cGatheringNameMap;
	cGatheringNameMap mGatheringNameMap;

	typedef tMap<unsigned int, cGatheringInfo*> cGatheringInfoMap;
	cGatheringInfoMap mGatheringInfoMap;

	typedef tMap<unsigned int, cGatheringGroup*> cGatheringGroupMap;
	cGatheringGroupMap mGatheringGroupMap;

	typedef tList<cGatheringSceneNode*> cGatheringList;

	/// 포탈
	typedef tMap<unsigned int, cPortalInfo*> cPortalInfoMap;
	cPortalInfoMap mPortalInfoMap;

	/// 패턴
	typedef tPointerHashMap<unsigned int, cPattern*> cPatternMap;
	cPatternMap mPatternMap;

	cPattern* mSelPattern;

	/// 게이트
	typedef tMap<unsigned int, cDoorInfo*> cDoorInfoMap;
	cDoorInfoMap mDoorInfoMap;


	cMonsterSceneNode* mDummyNode;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 선택된 노드
	cSceneNode* mSelNode;
	cCircleLines* mSelLines;
	cCircleLines* mGroupLines;

	/// 길찾기
	cPathFinder* mPathFinder;
};

inline
void cRegenToolView::SetMapDialog( cMapDialog* dlg )
{
	assert( dlg );

	mMapDialog = dlg;
}

inline
void cRegenToolView::SetNpcDialog( cNpcDialog* dlg )
{
	assert( dlg );

	mNpcDialog = dlg;
}

inline
void cRegenToolView::SetMonsterDialog( cMonsterDialog* dlg )
{
	assert( dlg );

	mMonsterDialog = dlg;
}

inline
void cRegenToolView::SetPortalDialog( cPortalDialog* dlg )
{
	assert( dlg );

	mPortalDialog = dlg;
}

inline
void cRegenToolView::SetGatheringDialog( cGatheringDialog* dlg )
{
	assert( dlg );

	mGatheringDialog = dlg;
}

inline
void cRegenToolView::SetPatternDialog( cPatternDialog* dlg )
{
	assert( dlg );

	mPatternDialog = dlg;
}

inline
void cRegenToolView::SetDoorDialog( cDoorDialog* dlg )
{
	assert( dlg );

	mDoorDialog = dlg;
}

inline
cSceneNode* cRegenToolView::GetSelNode() const
{
	return mSelNode;
}

//inline
//eEditMode cRegenToolView::GetEditMode() const
//{
//	return mEditMode;
//}
//
//inline
//cFreeCamera* cRegenToolView::GetDefaultCamera() const
//{
//	return mCamera;
//}

inline
cRegenToolView* cRegenToolView::GetSingleton()
{
	return mSingleton;
}

#define VIEW cRegenToolView::GetSingleton()
