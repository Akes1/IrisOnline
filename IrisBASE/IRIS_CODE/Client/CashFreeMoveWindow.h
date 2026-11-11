#include "UIWindow.h"
#include "BaseObject_Common.h"

class cComboBox;
class cUIImage;
struct sNpcData;
struct sBaseImageData;

class cCashFreeMoveWindow : public cUIWindow
{
public:
	cCashFreeMoveWindow();
	~cCashFreeMoveWindow();

	void Open() {}
	void Close();

	void Clear();

	void ClearNpc();	
	void ClearPotal();	
	void AddNpc( sNpcData* data );
	void AddPortal( unsigned long index );

	void Show( unsigned short slotIndex );

	unsigned short GetLinkSlot(){ return mInventorySlotIndex; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnShow();
	void OnHide();
	void UpdateRect();
	void UpdateSkin();

	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnComboSelected( cUINode* , unsigned int );
	void OnMouseMove( const cUIPos& pos );
	void OnMouseLeft(const cUIPos& pos );
	bool ContainPoint( const cUIPos& pos ) const;
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	void OnLButtonDoubleClick( const cUIPos& pos );
	
	void UpdateNamePos();

private:
	cComboBox* mpMapList;

	cUINodeSkin* mpMapSkin;
	cUIImage* mpMapImage;

	// 관리맵
	typedef tPointerHashMap<unsigned long, void*> cDataHashMap;
	
	// npc관리
	cDataHashMap	mNpcHashMap; 

	// portal
	cDataHashMap	mPortalHashMap;

	/// 화면 출력 hero 위치 
	float mOldX, mOldY;

	/// hero 데이터
	cUIImage*		mpHeroImage;
	NiPoint3		mHeroDir;
	cUISize			mHeroSize;

	/// 카메라 데이터
	cUIImage*		mpCameraImage;
	NiPoint3		mCameraDir;
	cUISize			mCameraSize;

	//이름 출력 
	cStringT mName;
	cStringT mJobName;
	cUIPos mNamePos;
	cUIPos mJobPos;

	/// 마우스 좌표
	cStringT		mPosStr;

	/// 업데이트 여부 
	bool mNeedUpdate;

	// 마우스 오버
	sBaseImageData*	 mpHoverData;

	// 자유 이동 주문서가 있는 인벤토리 슬롯
	unsigned short   mInventorySlotIndex;
};