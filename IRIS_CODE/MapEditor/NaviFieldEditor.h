/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.10
* 내    용 : 네비필드 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Lines.h"

class cSceneNode;
class cPathFinder;
class cNaviField;
class cNaviFieldLeafNode;
class cNaviFieldBuildDialog;
class cNaviFieldPaintDialog;
class cNaviFieldInfoDialog;

/// 네비필드 편집자
/// 맵상의 네비필드 리프 노드들을 편집하고
/// 네비필드 파일(*.navifield)을 저장, 로딩한다.
class cNaviFieldEditor
{
	static cNaviFieldEditor* mSingleton;
	friend class cMapEditorView;

public:
	cNaviFieldEditor();
	~cNaviFieldEditor();

	/// 지움
	void Clear();

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 네비필드를 새로 만듦
	void Init( unsigned int resolution, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );

	/// 네비필드를 로딩
	bool Load( const cString& pathName );

	/// 네비필드를 저장
	bool Save();
	bool SaveAs( const cString& pathName );

	/// 지형에 맞춤
	void SyncAllToNaviMesh();

	/// 색칠 다이얼로그를 설정
	void SetPaintDialog( cNaviFieldPaintDialog* dlg );

	/// 정보 다이얼로그를 설정
	void SetInfoDialog( cNaviFieldInfoDialog* dlg );

	/// 픽킹 위치 변경 여부를 설정
	void SetPickPosChanged( bool changed );

	/// 경로 이름을 리턴
	const cString& GetPathName() const;

	/// 해상도를 리턴
	unsigned int GetResolution() const;

	/// 정점당 미터수를 리턴
	float GetMetersPerVertex() const;

	/// 길찾기를 리턴
	cPathFinder* GetPathFinder() const;

protected:
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius );

private:
	/// 메시지를 처리
	void OnTimer_Paint();
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Paint( CPoint point );
	void OnLButtonUp();

public:
	/// 단일체를 리턴
	static cNaviFieldEditor* GetSingleton();

protected:
	/// 색칠 다이얼로그
	cNaviFieldPaintDialog* mPaintDialog;

	/// 정보 다이얼로그
	cNaviFieldInfoDialog* mInfoDialog;

	/// 네비필드
	cNaviField* mNaviField;

	/// 길찾기
	cPathFinder* mPathFinder;

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 드로잉 여부
	bool mDrawing;

	/// 픽킹 위치
	NiPoint3 mPickPos;
	NiPoint3 mPickOldPos;
	bool mPickPosChanged;

	/// 편집 원
	/// 선택 범위를 표시하는데 사용한다.
	cCircleLines mOuterLines;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 경로 이름
	cString mPathName;
};

inline
void cNaviFieldEditor::SetPaintDialog( cNaviFieldPaintDialog* dlg )
{
	assert( dlg && "null navifield paint dialog" );

	mPaintDialog = dlg;
}

inline
void cNaviFieldEditor::SetInfoDialog( cNaviFieldInfoDialog* dlg )
{
	assert( dlg && "null navifield info dialog" );

	mInfoDialog = dlg;
}

inline
void cNaviFieldEditor::SetPickPosChanged( bool changed )
{
	mPickPosChanged = changed;
}

inline
const cString& cNaviFieldEditor::GetPathName() const
{
	return mPathName;
}

inline
cNaviFieldEditor* cNaviFieldEditor::GetSingleton()
{
	return mSingleton;
}

inline
cPathFinder* cNaviFieldEditor::GetPathFinder() const
{
	return mPathFinder;
}

#define NAVIFIELDEDIT cNaviFieldEditor::GetSingleton()
#define PATHFINDER NAVIFIELDEDIT->GetPathFinder()
