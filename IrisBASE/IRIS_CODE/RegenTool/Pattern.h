#pragma once


const unsigned int MAX_PATH_COUNT = 2048;

class cPatternSceneNode;

struct sPatternPos 
{
	unsigned long	mX;
	unsigned long	mY;
	unsigned long	mRange;
	bool			mRelax;
	unsigned long	mWaitTime;

	cPatternSceneNode* mNode;

	sPatternPos()
	{
		mX = 0;
		mY = 0;
		mRange = 0;
		mRelax = 0;
		mWaitTime = 0;
		mNode = 0;
	}
};

/// 패턴
class cPattern
{
public:
	cPattern( unsigned long patternIdx );
	~cPattern();

	void			Clear();
	void			ClearSceneList();

	/// 저장
	bool			SavePattern( cFileSaver& saver );

	/// 패턴 정보기록
	void			SetPatternInfo( bool type )	{ mType = type; }

	/// 위치정보 추가
	void			AddNode( cPatternSceneNode* node );
	void			InsertNode( cPatternSceneNode* baseNode, cPatternSceneNode* node );
	void			RemoveNode( cPatternSceneNode* node );

	void			SetVisible( bool visible );
	void			UpdateAllPathLine();

	void			RenderPattern();

private:
	void			SetActiveLine( unsigned int count );

public:
	/// 인덱스
	unsigned long	mPatternIdx;

	/// 타입
	bool			mType;

	NiColor			mColor;

	/// 길찾기 정보
	NiPoint2 mPathArray[MAX_PATH_COUNT];
	unsigned int mPathCount;

	/// 경로
	NiMeshPtr	mPathLine;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// scene node 정보
	typedef tPointerList<cPatternSceneNode*> cSceneNodeList;
	cSceneNodeList	mSceneNodeList;
};
