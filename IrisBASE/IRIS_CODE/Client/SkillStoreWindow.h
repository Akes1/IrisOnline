///* ==========================================================================
//* 파    일 : SkillStoreWindow.h
//* 목    적 : 
//* 작 성 자 : 이법석
//* 작 성 일 : 2007-10-31
//* 주의사항 :
//*===========================================================================*/
//#pragma once
//
//#include "uiwindow.h"
//#include "RadioButton.h"
//#include "UIImage.h"
//
//class cStyleListBox;
//class cLabel;
//class cSkillStoreWindow : public cUIWindow
//{
//public:
//	cSkillStoreWindow();
//	virtual ~cSkillStoreWindow();
//
//	virtual void Open();
//	virtual void Close();
//
//	virtual void UpdateSkin();
//
//	void Show( unsigned long npcIndex );
//	void SetMoney( unsigned int money );
//	void UpdateSkillPoint();
//
//	/// 현재 페이지 내용 갱신
//	inline void UpdatePage() { ChangePage( mCurPageIdx, true ); }
//
//	struct sSkillRowData
//	{
//		unsigned long	mSkillIdx;		/// skill Idx
//		cUIImage		mImage;			/// skill icon image
//
//		LPCTSTR			mName;			/// skill name
//		unsigned char	mTrainStep;		/// 습득 단계
//		unsigned long	mMoney;			/// 소모 금액
//		unsigned long	mSp;			/// 소모 sp
//		unsigned long	mMaxstep;		/// max step
//		bool			mMaster;
//	};
//
//protected:
//	virtual void OnMouseLeft( const cUIPos& pos );
//
//	bool OnCreate( cUINodeProperty* pproperty );
//	void OnRadioPressed( cUINode* pcaller, unsigned int id );
//	void OnProcess(unsigned long deltaTime, unsigned long accumTime);
//	void OnHide();
//
//	void OnCommand( cUINode* pcaller, unsigned int id );
//	void OnListBoxSelected( cUINode* caller, unsigned int id );
//	void OnListBoxRowHovered( cUINode* caller, unsigned int id, const cUIPos& pos, void* extraData );
//	void OnListBoxImageHorvered( cUINode* caller, unsigned int id, const cUIPos& pos, void* extraData );
//	void OnListBoxImageLeft( cUINode* caller, unsigned int id );
//
//	void InitPageData( unsigned int pageIdx, unsigned long jobIdx );
//	void ExitPageData();
//
//	void ChangePage( unsigned int pageIdx, bool useCurSel = false );
//
//	void CheckBuyListColor();
//
//	struct sPageData
//	{
//		cRadioButton			mRadio;
//		tPointerArray<void*>	mRowArray;
//		~sPageData()
//		{
//			if( mRowArray.IsEmpty() == false )
//			{
//				for( unsigned int i=0; i<mRowArray.GetSize(); i++ )
//				{
//					sSkillRowData* p = (sSkillRowData*)mRowArray[i];
//					delete p;
//				}
//				mRowArray.Clear();
//			}
//		}
//	};
//
//private:
//	cStyleListBox*		mpGoodList;
//
//	unsigned int			mCurPageIdx;
//	tPointerArray<void*>	mPageArray;
//
//	/// 소지금
//	cLabel*		mpMoney;
//
//	/// 스킬 포인트
//	cLabel*		mpSkillPoint;
//
//	bool		mCheckBuySkillList;
//};
//
//
//
