// CharModelDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "CharModelDlg.h"

#include "resource.h"

#include "ResourceManager.h"
#include "GameResourceManager.h"
#include ".\charmodeldlg.h"

#include "ModelView.h"
#include "Item_Common.h"

#include "Player_Common.h"
#include "PlayerSceneNode.h"
#include "CharacterForm.h"
#include "SceneManager.h"

// cCharModelDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cCharModelDlg, CDialog)

BEGIN_MESSAGE_MAP(cCharModelDlg, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio5)

	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnCbnSelchangeCombo3)
	ON_CBN_SELCHANGE(IDC_COMBO4, OnCbnSelchangeCombo4)
	ON_CBN_SELCHANGE(IDC_COMBO5, OnCbnSelchangeCombo5)
	ON_CBN_SELCHANGE(IDC_COMBO6, OnCbnSelchangeCombo6)
	ON_CBN_SELCHANGE(IDC_COMBO7, OnCbnSelchangeCombo7)
	ON_CBN_SELCHANGE(IDC_COMBO8, OnCbnSelchangeCombo8)
	ON_CBN_SELCHANGE(IDC_COMBO9, OnCbnSelchangeCombo9)
	ON_CBN_SELCHANGE(IDC_COMBO10, OnCbnSelchangeCombo10)
	ON_CBN_SELCHANGE(IDC_COMBO11, OnCbnSelchangeCombo11)

	ON_CBN_SELCHANGE(IDC_COMBO12, OnCbnSelchangeCombo12)
	ON_CBN_SELCHANGE(IDC_COMBO13, OnCbnSelchangeCombo13)
	ON_CBN_SELCHANGE(IDC_COMBO14, OnCbnSelchangeCombo14)
	ON_CBN_SELCHANGE(IDC_COMBO15, OnCbnSelchangeCombo15)
	ON_CBN_SELCHANGE(IDC_COMBO16, OnCbnSelchangeCombo16)
	ON_CBN_SELCHANGE(IDC_COMBO17, OnCbnSelchangeCombo17)
	ON_CBN_SELCHANGE(IDC_COMBO18, OnCbnSelchangeCombo18)
	ON_CBN_SELCHANGE(IDC_COMBO19, OnCbnSelchangeCombo19)


END_MESSAGE_MAP()

cCharModelDlg::cCharModelDlg(CWnd* /*pParent=NULL*/)
{
	mCurRace = 0;
	mCurGender = 0;

	mLinkWeapon[0] = (unsigned int)-1;
	mLinkWeapon[1] = (unsigned int)-1;

	mCurWeaponState = eWEAPON_STATE_NONE;

	mLinkCostume[0] = (unsigned int)-1;
	mLinkCostume[1] = (unsigned int)-1;
}

cCharModelDlg::~cCharModelDlg()
{
}

void cCharModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	for(unsigned int i=0;i<eRACE_MAX;i++)
		DDX_Control( pDX, IDC_RADIO1+i, mRaceRadio[i] );

	for(unsigned int i=0;i<eGENDER_MAX;i++)
		DDX_Control( pDX, IDC_RADIO4+i, mGenderRadio[i] );

	for( unsigned int i=0;i<COMBO_MAX;i++ )
		DDX_Control( pDX, IDC_COMBO1 + i, mComboArray[i] );

	mRaceRadio[eRACE_HUMAN].SetCheck( 1 );
	mGenderRadio[eGENDER_MALE].SetCheck( 1 );
}

void cCharModelDlg::OpenData()
{
	mLinkWeapon[0] = (unsigned int)-1;
	mLinkWeapon[1] = (unsigned int)-1;

	mLinkCostume[0] = (unsigned int)-1;
	mLinkCostume[1] = (unsigned int)-1;

	int idx = -1;

	for( unsigned int i=0;i<COMBO_MAX;i++ )
		mComboArray[i].ResetContent();

	for(unsigned int i=0;i<eRACE_MAX;i++)
		mRaceRadio[i].SetCheck(0);
	for(unsigned int i=0;i<eGENDER_MAX;i++)
		mGenderRadio[i].SetCheck(0);

	mCurRace = eRACE_HUMAN;
	mCurGender = eGENDER_MALE;

	mRaceRadio[mCurRace].SetCheck( 1 );
	mGenderRadio[mCurGender].SetCheck( 1 );

	CComboBox* box = 0;

	/// hair & hair color
	box = &mComboArray[COMBO_HAIR];
	unsigned int hairCount = GAMERESOURCEMAN->GetDefaultHairCount();
	for( unsigned int i=0; i<hairCount; i++ )
	{
		char buf[256] = {0,};
		sprintf_s( buf, "Hair%2d", i+1 );
		box->AddString( buf );
	}
	box->SetCurSel(0);

	box = &mComboArray[COMBO_HAIRCOLOR];
	unsigned int hairColorCount = GAMERESOURCEMAN->GetHairColorCount();
	for( unsigned int i=0; i<hairColorCount; i++ )
	{
		char buf[256] = {0,};
		sprintf_s( buf, "Hair Color%2d", i+1 );
		box->AddString( buf );
	}
	box->SetCurSel(0);

	/// face
	box = &mComboArray[COMBO_FACE];
	unsigned int faceCount = GAMERESOURCEMAN->GetDefaultFaceCount();
	for( unsigned int i=0; i<faceCount; i++ )
	{
		char buf[256] = {0,};
		sprintf_s( buf, "Face%2d", i+1 );
		box->AddString( buf );
	}
	box->SetCurSel(0);

	/// helmet
	box = &mComboArray[COMBO_HELMET];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(idx);

	/// jacket
	box = &mComboArray[COMBO_JACKET];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(idx);

	/// pants
	box = &mComboArray[COMBO_PANTS];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);

	/// gloves
	box = &mComboArray[COMBO_GLOVES];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);

	/// boots
	box = &mComboArray[COMBO_BOOTS];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);

	/// weapon1
	box = &mComboArray[COMBO_WEAPON_TYPE];
	box->AddString("NONE");
	box->AddString("SWORD");
	box->AddString("BLADE");
	box->AddString("DUAL");
	box->AddString("CUTTER");
	box->AddString("GUN");
	box->AddString("STAFF");
	box->SetCurSel(0);
	mCurWeaponState = eWEAPON_STATE_NONE;

	box = &mComboArray[COMBO_WEAPON_KIND];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);
	box->EnableWindow( FALSE );

	/// weapon2
	box = &mComboArray[COMBO_SHIELD];
	idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);

	///
	for( unsigned int i = COMBO_COS_JACKET; i<COMBO_MAX; ++i )
	{
		box = &mComboArray[i];
		idx = box->AddString("None");
		box->SetItemDataPtr(idx,0);
		box->SetCurSel(0);
	}

	mComboArray[COMBO_COS_JACKET].EnableWindow();
	mComboArray[COMBO_COS_PANTS].EnableWindow();
	mComboArray[COMBO_COS_GLOVES].EnableWindow();
	mComboArray[COMBO_COS_BOOTS].EnableWindow();

	/// Add item list
	cArrayHashMap* itemMap = GAMERESOURCEMAN->GetItemList();
	cArrayHashMap::cIterator i = itemMap->Begin();
	cArrayHashMap::cIterator iend = itemMap->End();
	for( ; i != iend; ++i )
	{
		sItemInfo* info = (sItemInfo*)(i->mSecond);
		if( info == 0 )
			continue;

		char buff[256] = {0,};

		switch( info->mType )
		{
		case ITEM_WEAPON:
			{
				if( info->mSubType == ITEM_WEAPON_SHIELD )
				{
					LPCTSTR str = GAMERESOURCEMAN->GetItemNameText( info->mNameIndex );
					::sprintf_s( buff, "%s (%d)", str, info->mIndex );
					idx = mComboArray[COMBO_SHIELD].AddString( buff );
					mComboArray[COMBO_SHIELD].SetItemDataPtr(idx,info);

				}
				else
				{
					int type = mComboArray[COMBO_WEAPON_TYPE].GetCurSel();
					if( info->mSubType == type )
					{
						LPCTSTR str = GAMERESOURCEMAN->GetItemNameText( info->mNameIndex );
						::sprintf_s( buff, "%s (%d)", str, info->mIndex );
						idx = mComboArray[COMBO_WEAPON_KIND].AddString( buff );
						mComboArray[COMBO_WEAPON_KIND].SetItemDataPtr(idx,info);
					}
				}
			}
			break;
		case ITEM_WEAR:
			{
				LPCTSTR str = GAMERESOURCEMAN->GetItemNameText( info->mNameIndex );

				if( info->mSubType == ITEM_WEAR_UPPER )
					box = &mComboArray[COMBO_JACKET];
				else if( info->mSubType == ITEM_WEAR_LOWER )
					box = &mComboArray[COMBO_PANTS];
				else if( info->mSubType == ITEM_WEAR_HANDS )
					box = &mComboArray[COMBO_GLOVES];
				else if( info->mSubType == ITEM_WEAR_FEET )
					box = &mComboArray[COMBO_BOOTS];
				else if( info->mSubType == ITEM_WEAR_HEAD )
					box = &mComboArray[COMBO_HELMET];
				else if( info->mSubType == ITEM_WEAR_COS_ONEPIECE )
					box = &mComboArray[COMBO_COS_ONEPIECE];
				else if( info->mSubType == ITEM_WEAR_COS_HEAD )
					box = &mComboArray[COMBO_COS_HELMET];
				else if( info->mSubType == ITEM_WEAR_COS_UPPER )
					box = &mComboArray[COMBO_COS_JACKET];
				else if( info->mSubType == ITEM_WEAR_COS_LOWER )
					box = &mComboArray[COMBO_COS_PANTS];
				else if( info->mSubType == ITEM_WEAR_COS_HANDS )
					box = &mComboArray[COMBO_COS_GLOVES];
				else if( info->mSubType == ITEM_WEAR_COS_FEET )
					box = &mComboArray[COMBO_COS_BOOTS];
				else
					continue;

				::sprintf_s( buff, "%s (%d)", str, info->mIndex );
				idx = box->AddString( buff );
				box->SetItemDataPtr( idx, info );
			}
			break;
		case ITEM_ACCESSORY:
			{
				LPCTSTR str = GAMERESOURCEMAN->GetItemNameText( info->mNameIndex );

				if( info->mSubType == ITEM_ACCESSORY_COS_FACE )
					box = &mComboArray[COMBO_COS_ACCHEAD];
				else if( info->mSubType == ITEM_ACCESSORY_COS_BACK )
					box = &mComboArray[COMBO_COS_ACCBACK];
				else
					continue;

				::sprintf_s( buff, "%s (%d)", str, info->mIndex );
				idx = box->AddString( buff );
				box->SetItemDataPtr( idx, info );
			}
			break;
		default:
			continue;
		}
	}
}

void cCharModelDlg::OnBnClickedRadio1()
{
	if( mCurRace == eRACE_HUMAN )
		return;

	mCurRace = eRACE_HUMAN;
	mCurGender = eGENDER_MALE;

	for(unsigned int i=0;i<eGENDER_MAX;i++)
		mGenderRadio[i].SetCheck(0);
	mGenderRadio[mCurGender].SetCheck( 1 );

	UpdateCharacterData( mCurRace * eGENDER_MAX + mCurGender );

	CHARACTERFORM->UpdateDrama();
}


void cCharModelDlg::OnBnClickedRadio2()
{
	if( mCurRace == eRACE_BEAST )
		return;

	mCurRace = eRACE_BEAST;
	mCurGender = eGENDER_MALE;

	for(unsigned int i=0;i<eGENDER_MAX;i++)
		mGenderRadio[i].SetCheck(0);
	mGenderRadio[mCurGender].SetCheck( 1 );

	UpdateCharacterData( mCurRace * eGENDER_MAX + mCurGender );

	CHARACTERFORM->UpdateDrama();
}

void cCharModelDlg::OnBnClickedRadio3()
{
	if( mCurRace == eRACE_ELF )
		return;

	mCurRace = eRACE_ELF;
	mCurGender = eGENDER_MALE;

	for(unsigned int i=0;i<eGENDER_MAX;i++)
		mGenderRadio[i].SetCheck(0);
	mGenderRadio[mCurGender].SetCheck( 1 );

	UpdateCharacterData( mCurRace * eGENDER_MAX + mCurGender );

	CHARACTERFORM->UpdateDrama();
}

void cCharModelDlg::OnBnClickedRadio4()
{
	if( mCurGender == eGENDER_MALE )
		return;

	mCurGender = eGENDER_MALE;

	UpdateCharacterData( mCurRace * eGENDER_MAX + mCurGender );

	CHARACTERFORM->UpdateDrama();
}

void cCharModelDlg::OnBnClickedRadio5()
{
	if( mCurGender == eGENDER_FEMALE )
		return;

	mCurGender = eGENDER_FEMALE;

	UpdateCharacterData( mCurRace * eGENDER_MAX + mCurGender );

	CHARACTERFORM->UpdateDrama();
}

void cCharModelDlg::OnCbnSelchangeCombo1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_HAIR];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_HELMET];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	CComboBox* helmatBox = &mComboArray[COMBO_HELMET];
	helmatBox->SetCurSel(0);

	const char* filename = GAMERESOURCEMAN->GetDefaultHairFileName( curSel, mCurRace, mCurGender );
	VIEW->ChangeCharacterParts( ePART_HAIR, filename );

	box = &mComboArray[COMBO_HAIRCOLOR];
	int curColorSel = box->GetCurSel();
	if( curColorSel < 0 )
	{
		assert(0);
		return;
	}
	filename = GAMERESOURCEMAN->GetHairColorTextureFileName( curSel, curColorSel, mCurRace, mCurGender );
	VIEW->ChangeCharacterPartsTexture( ePART_HAIR, filename );
}

void cCharModelDlg::OnCbnSelchangeCombo2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* hairBox = &mComboArray[COMBO_HAIR];
	int curHairSel = hairBox->GetCurSel();
	if( curHairSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_HELMET];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	CComboBox* helmatBox = &mComboArray[COMBO_HELMET];
	helmatBox->SetCurSel(0);

	const char* filename = GAMERESOURCEMAN->GetDefaultHairFileName( curHairSel, mCurRace, mCurGender );
	VIEW->ChangeCharacterParts( ePART_HAIR, filename );

	CComboBox* colorBox = &mComboArray[COMBO_HAIRCOLOR];
	int curColorSel = colorBox->GetCurSel();
	if( curColorSel < 0 )
	{
		assert(0);
		return;
	}

	filename = GAMERESOURCEMAN->GetHairColorTextureFileName( curHairSel, curColorSel, mCurRace, mCurGender );
	VIEW->ChangeCharacterPartsTexture( ePART_HAIR, filename );
}

void cCharModelDlg::OnCbnSelchangeCombo3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_FACE];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	const char* filename = GAMERESOURCEMAN->GetFaceTextureFileName( curSel+1, mCurRace, mCurGender );
	VIEW->ChangeCharacterPartsTexture( ePART_FACE, filename );
}

void cCharModelDlg::OnCbnSelchangeCombo4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_JACKET];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_JACKET];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	costume = &mComboArray[COMBO_COS_ONEPIECE];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		/// 기본 복장 적용
		const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_BODY1, mCurRace, mCurGender );
		VIEW->ChangeCharacterParts( ePART_BODY1, filename );
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_BODY1, filename );
		if( check == false )
		{
			/// 기본 복장 적용
			const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_BODY1, mCurRace, mCurGender );
			VIEW->ChangeCharacterParts( ePART_BODY1, filename );
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_PANTS];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_PANTS];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;
	costume = &mComboArray[COMBO_COS_ONEPIECE];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;


	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		/// 기본 복장 적용
		const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_BODY2, mCurRace, mCurGender );
		VIEW->ChangeCharacterParts( ePART_BODY2, filename );
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_BODY2, filename );
		if( check == false )
		{
			/// 기본 복장 적용
			const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_BODY2, mCurRace, mCurGender );
			VIEW->ChangeCharacterParts( ePART_BODY2, filename );
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_GLOVES];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_GLOVES];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;
	costume = &mComboArray[COMBO_COS_ONEPIECE];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		/// 기본 복장 적용
		const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_HAND, mCurRace, mCurGender );
		VIEW->ChangeCharacterParts( ePART_HAND, filename );
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_HAND, filename );
		if( check == false )
		{
			/// 기본 복장 적용
			const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_HAND, mCurRace, mCurGender );
			VIEW->ChangeCharacterParts( ePART_HAND, filename );
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_BOOTS];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_BOOTS];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;
	costume = &mComboArray[COMBO_COS_ONEPIECE];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		/// 기본 복장 적용
		const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_FOOT, mCurRace, mCurGender );
		VIEW->ChangeCharacterParts( ePART_FOOT, filename );
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_FOOT, filename );
		if( check == false )
		{
			/// 기본 복장 적용
			const char* filename = GAMERESOURCEMAN->GetDefaultWearFileName( ePART_FOOT, mCurRace, mCurGender );
			VIEW->ChangeCharacterParts( ePART_FOOT, filename );
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_HELMET];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	CComboBox* costume = &mComboArray[COMBO_COS_HELMET];
	if( costume->GetItemDataPtr( costume->GetCurSel() ) != 0 )
		return;

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		CComboBox* box = &mComboArray[COMBO_HAIR];
		int curHairSel = box->GetCurSel();
		if( curHairSel < 0 )
		{
			assert(0);
			return;
		}

		/// 기본 복장 적용
		const char* filename = GAMERESOURCEMAN->GetDefaultHairFileName( curHairSel, mCurRace, mCurGender );
		VIEW->ChangeCharacterParts( ePART_HAIR, filename );

		box = &mComboArray[COMBO_HAIRCOLOR];
		int curColorSel = box->GetCurSel();
		if( curColorSel < 0 )
		{
			assert(0);
			return;
		}
		filename = GAMERESOURCEMAN->GetHairColorTextureFileName( curHairSel, curColorSel, mCurRace, mCurGender );
		VIEW->ChangeCharacterPartsTexture( ePART_HAIR, filename );

		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_HAIR, filename, info->mHideEar == 1 );
		if( check == false )
		{
			CComboBox* box = &mComboArray[COMBO_HAIR];
			int curHairSel = box->GetCurSel();
			if( curHairSel < 0 )
			{
				assert(0);
				return;
			}

			/// 기본 복장 적용
			const char* filename = GAMERESOURCEMAN->GetDefaultHairFileName( curHairSel, mCurRace, mCurGender );
			VIEW->ChangeCharacterParts( ePART_HAIR, filename );

			box = &mComboArray[COMBO_HAIRCOLOR];
			int curColorSel = box->GetCurSel();
			if( curColorSel < 0 )
			{
				assert(0);
				return;
			}
			filename = GAMERESOURCEMAN->GetHairColorTextureFileName( curHairSel, curColorSel, mCurRace, mCurGender );
			VIEW->ChangeCharacterPartsTexture( ePART_HAIR, filename );

			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_WEAPON_TYPE];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	box = &mComboArray[COMBO_WEAPON_KIND];
	box->ResetContent();

	int idx = box->AddString("None");
	box->SetItemDataPtr(idx,0);
	box->SetCurSel(0);

	if( curSel == 0 )
	{
		box->EnableWindow( FALSE );
	}
	else
	{
		box->EnableWindow( TRUE );

		cArrayHashMap* itemMap = GAMERESOURCEMAN->GetItemList();
		cArrayHashMap::cIterator i = itemMap->Begin();
		cArrayHashMap::cIterator iend = itemMap->End();

		char buff[256] = {0,};
		for( ; i != iend; ++i )
		{
			sItemInfo* info = (sItemInfo*)(i->mSecond);
			if( info == 0 )
				continue;
			if( info->mType != ITEM_WEAPON )
				continue;
			if( info->mSubType != curSel-1 )
				continue;

			LPCTSTR str = GAMERESOURCEMAN->GetItemNameText( info->mNameIndex );
			::sprintf_s( buff, "%s (%d)", str, info->mIndex );
			idx = box->AddString( buff );
			box->SetItemDataPtr(idx,info);
		}
	}

	if( curSel > 1 )
	{
		mComboArray[COMBO_SHIELD].SetCurSel(0);
		mComboArray[COMBO_SHIELD].EnableWindow( FALSE );
	}
	else
		mComboArray[COMBO_SHIELD].EnableWindow( TRUE );


	mCurWeaponState = (unsigned char)curSel;
	UpdateWeaponData();
	CHARACTERFORM->UpdateDrama();
}

void cCharModelDlg::OnCbnSelchangeCombo9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_WEAPON_KIND];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	/// 방패 변동
	UpdateWeaponData();
}

void cCharModelDlg::OnCbnSelchangeCombo10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CComboBox* box = &mComboArray[COMBO_SHIELD];
	if( mComboArray[COMBO_WEAPON_TYPE].GetCurSel() > 1 )
	{
		box->SetCurSel(0);
		return;
	}

	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	/// 방패 변동
	UpdateWeaponData();
}

void cCharModelDlg::OnCbnSelchangeCombo12()
{
	/// 코스튬 상의
	CComboBox* box = &mComboArray[COMBO_COS_JACKET];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		OnCbnSelchangeCombo4();
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_BODY1, filename );
		if( check == false )
		{
			box->SetCurSel(0);
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo13()
{
	/// 코스튬 하의
	CComboBox* box = &mComboArray[COMBO_COS_PANTS];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		OnCbnSelchangeCombo5();
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_BODY2, filename );
		if( check == false )
		{
			box->SetCurSel(0);
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo14()
{
	/// 코스튬 손
	CComboBox* box = &mComboArray[COMBO_COS_GLOVES];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		OnCbnSelchangeCombo6();
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_HAND, filename );
		if( check == false )
		{
			box->SetCurSel(0);
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo15()
{
	/// 코스튬 발
	CComboBox* box = &mComboArray[COMBO_COS_BOOTS];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		OnCbnSelchangeCombo7();
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_FOOT, filename );
		if( check == false )
		{
			box->SetCurSel(0);
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo16()
{
	/// 코스튬 머리
	CComboBox* box = &mComboArray[COMBO_COS_HELMET];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		OnCbnSelchangeCombo11();
		return;
	}

	if( info->mType == ITEM_WEAR )
	{
		const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
		bool check = VIEW->ChangeCharacterParts( ePART_HAIR, filename );
		if( check == false )
		{
			box->SetCurSel(0);
			return;
		}
	}
}

void cCharModelDlg::OnCbnSelchangeCombo17()
{
	/// 코스튬 악세서리 얼굴
	CComboBox* box = &mComboArray[COMBO_COS_ACCHEAD];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
		return;

	VIEW->UnLinkCharacterCostume( mLinkCostume[0], (unsigned long)-1 );

	const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
	mLinkCostume[0] = VIEW->LinkCharacterCostume( eLINK_HEAD, filename );
}

void cCharModelDlg::OnCbnSelchangeCombo18()
{
	/// 코스튬 악세서리 등
	CComboBox* box = &mComboArray[COMBO_COS_ACCBACK];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
		return;

	VIEW->UnLinkCharacterCostume( (unsigned long)-1, mLinkCostume[1] );

	const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, false );
	mLinkCostume[1] = VIEW->LinkCharacterCostume( eLINK_BODY, filename );
}

void cCharModelDlg::OnCbnSelchangeCombo19()
{
	/// 코스튬 원피스
	CComboBox* box = &mComboArray[COMBO_COS_ONEPIECE];
	int curSel = box->GetCurSel();
	if( curSel < 0 )
	{
		assert(0);
		return;
	}

	sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
	if( info == 0 )
	{
		mComboArray[COMBO_COS_JACKET].EnableWindow();
		mComboArray[COMBO_COS_PANTS].EnableWindow();
		mComboArray[COMBO_COS_GLOVES].EnableWindow();
		mComboArray[COMBO_COS_BOOTS].EnableWindow();

		OnCbnSelchangeCombo12();
		OnCbnSelchangeCombo13();
		OnCbnSelchangeCombo14();
		OnCbnSelchangeCombo15();
	}
	else
	{
		mComboArray[COMBO_COS_JACKET].SetCurSel(0);
		mComboArray[COMBO_COS_PANTS].SetCurSel(0);
		mComboArray[COMBO_COS_GLOVES].SetCurSel(0);
		mComboArray[COMBO_COS_BOOTS].SetCurSel(0);

		mComboArray[COMBO_COS_JACKET].EnableWindow( FALSE );
		mComboArray[COMBO_COS_PANTS].EnableWindow( FALSE );
		mComboArray[COMBO_COS_GLOVES].EnableWindow( FALSE );
		mComboArray[COMBO_COS_BOOTS].EnableWindow( FALSE );

		sItemOnepieceInfo* onepieceInfo = (sItemOnepieceInfo*)GAMERESOURCEMAN->GetItemOnepieceInfo( info->mIndex );
		if( onepieceInfo == 0 )
		{
			assert(0);
			return;
		}

		if( onepieceInfo->mUpperIndex != 0 )
		{
			const char* filename = GAMERESOURCEMAN->GetItemFileName( onepieceInfo->mUpperIndex, mCurRace, mCurGender, false );
			VIEW->ChangeCharacterParts( ePART_BODY1, filename );
		}
		if( onepieceInfo->mLowerIndex != 0 )
		{
			const char* filename = GAMERESOURCEMAN->GetItemFileName( onepieceInfo->mLowerIndex, mCurRace, mCurGender, false );
			VIEW->ChangeCharacterParts( ePART_BODY2, filename );
		}
		if( onepieceInfo->mHandsIndex != 0 )
		{
			const char* filename = GAMERESOURCEMAN->GetItemFileName( onepieceInfo->mHandsIndex, mCurRace, mCurGender, false );
			VIEW->ChangeCharacterParts( ePART_HAND, filename );
		}
		if( onepieceInfo->mFootsIndex != 0 )
		{
			const char* filename = GAMERESOURCEMAN->GetItemFileName( onepieceInfo->mFootsIndex, mCurRace, mCurGender, false );
			VIEW->ChangeCharacterParts( ePART_FOOT, filename );
		}
	}

}

void cCharModelDlg::UpdateCharacterData( unsigned long modelIndex )
{
	VIEW->CreateCharacter( modelIndex, mCurRace, mCurGender );

	OnCbnSelchangeCombo1();
	OnCbnSelchangeCombo2();
	OnCbnSelchangeCombo3();
	OnCbnSelchangeCombo4();
	OnCbnSelchangeCombo5();
	OnCbnSelchangeCombo6();
	OnCbnSelchangeCombo7();

	mLinkWeapon[0] = (unsigned int)-1;
	mLinkWeapon[1] = (unsigned int)-1;
	UpdateWeaponData();

	UpdateCostumeData();
}

void cCharModelDlg::UpdateWeaponData()
{
	SCENEMAN->SetDramaProcess( true );

	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node )
		node->SetTargetAnimation( mCurWeaponState );

	/// 초기화
	VIEW->UnLinkCharacterWeapon( mLinkWeapon[0], mLinkWeapon[1] );
	mLinkWeapon[0] = (unsigned int)-1;
	mLinkWeapon[1] = (unsigned int)-1;

	float fScale = 1.00f;
	if( mCurRace == eRACE_HUMAN )
	{
		fScale = (mCurGender == eGENDER_MALE)? 1.03f:1.06f;
	}
	else if( mCurRace == eRACE_ELF )
	{
		fScale = (mCurGender == eGENDER_MALE)? 1.15f:1.09f;
	}
	else if( mCurRace == eRACE_BEAST )
	{
		fScale = (mCurGender == eGENDER_MALE)? 1.12f:1.00f;
	}

	/// 셋팅
	CComboBox* box = &mComboArray[COMBO_WEAPON_KIND];
	int curSel = box->GetCurSel();
	if( curSel > 0 )
	{
		sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
		if( info->mType == ITEM_WEAPON )
		{
			switch( info->mSubType )
			{
			case ITEM_WEAPON_SWORD:
				{
					const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, true );
					mLinkWeapon[0] = VIEW->LinkCharacterWeapon( eLINK_RHAND, filename, fScale );
				}
				break;
			case ITEM_WEAPON_DUAL:				// 이도류
			case ITEM_WEAPON_CUTTER:			// 단  검
				{
					const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, true );
					mLinkWeapon[0] = VIEW->LinkCharacterWeapon( eLINK_RHAND, filename, fScale );

					filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex2, mCurRace, mCurGender, true );
					mLinkWeapon[1] = VIEW->LinkCharacterWeapon( eLINK_LHAND, filename, fScale );

					mComboArray[COMBO_SHIELD].SetCurSel( 0 );
				}
				return;
			case ITEM_WEAPON_BLADE:				// 양손검
			case ITEM_WEAPON_GUN:				// 총,
			case ITEM_WEAPON_STAFF:				// 지팡이
				{
					const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, true );
					mLinkWeapon[0] = VIEW->LinkCharacterWeapon( eLINK_RHAND, filename, fScale );

					mComboArray[COMBO_SHIELD].SetCurSel( 0 );
				}
				return;
			default:
				{
					assert(0);
				}
				break;
			}
		}
	}

	box = &mComboArray[COMBO_SHIELD];
	curSel = box->GetCurSel();
	if( curSel > 0 )
	{
		sItemInfo* info = (sItemInfo*)box->GetItemDataPtr( curSel );
		if( info->mType == ITEM_WEAPON )
		{
			if( info->mSubType == ITEM_WEAPON_SHIELD )
			{
				const char* filename = GAMERESOURCEMAN->GetItemFileName( info->mModelIndex1, mCurRace, mCurGender, true );
				mLinkWeapon[1] = VIEW->LinkCharacterWeapon( eLINK_LARM, filename, fScale );
			}
		}
	}
}

void cCharModelDlg::UpdateCostumeData()
{
	if( mComboArray[COMBO_COS_ONEPIECE].GetCurSel() > 0 )
	{
		OnCbnSelchangeCombo19();
	}
	else
	{
		mComboArray[COMBO_COS_JACKET].EnableWindow();
		mComboArray[COMBO_COS_PANTS].EnableWindow();
		mComboArray[COMBO_COS_GLOVES].EnableWindow();
		mComboArray[COMBO_COS_BOOTS].EnableWindow();

		OnCbnSelchangeCombo12();
		OnCbnSelchangeCombo13();
		OnCbnSelchangeCombo14();
		OnCbnSelchangeCombo15();
	}

	OnCbnSelchangeCombo16();

	VIEW->UnLinkCharacterCostume( mLinkCostume[0], mLinkCostume[1] );
	mLinkCostume[0] = (unsigned int)-1;
	mLinkCostume[1] = (unsigned int)-1;

	OnCbnSelchangeCombo17();
	OnCbnSelchangeCombo18();
}