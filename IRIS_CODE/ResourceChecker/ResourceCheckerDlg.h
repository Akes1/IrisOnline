// ResourceCheckerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"


class cDramaturgyManager;
class cItemManager;
class cTriggerManager;
class cFilePackSystem;
class cQuestManager;
class cShortcutManager;
class cFilterManager;
class cGameResourceManager;
class cUIManager;
class cTarotManager;
class cTipManager;
class cTutorialManager;
class cMakeStage;
class cRenderSystem;
class cResourceManager;
class cSoundSystem;
class cTitleManager;
class cLoadStage;
class cWorldManager;
class cSceneManager;

static char* gCheckFolder[]    = { "data", "script", "sound", "language" };
enum eCheckFolder
{
	CHECK_FOLDER_DATA = 0,
	CHECK_FOLDER_SCRIPT,
	CHECK_FOLDER_SOUND,
	CHECK_FOLDER_LANGUAGE,

	CHECK_FOLDER_COUNT
};

enum eLoadIndex
{
	eLoadTexture	= 4,
	eLoadMapTexture = 5,
	eLoadNif		= 81,
	eLoadMapNif		= 83,
};

const char* const LoadInSource[83] = 
{
	///load texture
	"./data/2ddata/depthplane.tga",
	"./data/2ddata/taro_close.tga",
	"./data/2ddata/taro_open.tga",
	"./data/2ddata/taro_rollover.tga",
	/// loadmapteture
	"./data/2ddata/loadingmap_image00.dds",
	/// loadnif
	"./data/character/g_fool_001.nif",
	"./data/effect/camera_shake01.nif",
	"./data/effect/dropbox.nif",
	"./data/effect/dropbox02.nif",
	"./data/effect/eff_duel_01.nif",
	"./data/effect/eff_duel_02.nif",
	"./data/effect/eff_enhance_dagger1.nif",
	"./data/effect/eff_enhance_dagger2.nif",
	"./data/effect/eff_enhance_dagger3.nif",
	"./data/effect/eff_enhance_dagger4.nif",
	"./data/effect/eff_enhance_gun1.nif",
	"./data/effect/eff_enhance_gun2.nif",
	"./data/effect/eff_enhance_gun3.nif",
	"./data/effect/eff_enhance_gun4.nif",
	"./data/effect/eff_enhance_onehand1.nif",
	"./data/effect/eff_enhance_onehand2.nif",
	"./data/effect/eff_enhance_onehand3.nif",
	"./data/effect/eff_enhance_onehand4.nif",
	"./data/effect/eff_enhance_shield1.nif",
	"./data/effect/eff_enhance_shield2.nif",
	"./data/effect/eff_enhance_shield3.nif",
	"./data/effect/eff_enhance_shield4.nif",
	"./data/effect/eff_enhance_staff1.nif",
	"./data/effect/eff_enhance_staff2.nif",
	"./data/effect/eff_enhance_staff3.nif",
	"./data/effect/eff_enhance_staff4.nif",
	"./data/effect/eff_enhance_twohand1.nif",
	"./data/effect/eff_enhance_twohand2.nif",
	"./data/effect/eff_enhance_twohand3.nif",
	"./data/effect/eff_enhance_twohand4.nif",
	"./data/effect/eff_mon_regen_01.nif",
	"./data/effect/eff_npc_quest_01.nif",
	"./data/effect/eff_npc_quest_02.nif",
	"./data/effect/eff_npc_quest_03.nif",
	"./data/effect/eff_pc_change_01.nif",
	"./data/effect/eff_pc_change_02.nif",
	"./data/effect/eff_pc_collection_01.nif",
	"./data/effect/eff_pc_collection_02.nif",
	"./data/effect/eff_pc_enchant_02.nif",
	"./data/effect/eff_pc_enchant_03.nif",
	"./data/effect/eff_pc_get_01.nif",
	"./data/effect/eff_pc_get_02.nif",
	"./data/effect/eff_pc_get_03.nif",
	"./data/effect/eff_pc_get_04.nif",
	"./data/effect/eff_pc_itemget_01.nif",
	"./data/effect/eff_pc_itemget_02.nif",
	"./data/effect/eff_pc_itemget_03.nif",
	"./data/effect/eff_pc_jobchange_01.nif",
	"./data/effect/eff_pc_login_01.nif",
	"./data/effect/eff_pc_lvup_01.nif",
	"./data/effect/eff_pc_mix_01.nif",
	"./data/effect/eff_pc_mix_02.nif",
	"./data/effect/eff_pc_pvp_2.nif",
	"./data/effect/eff_pc_pvp_4.nif",
	"./data/effect/eff_pc_riding_02.nif",
	"./data/effect/eff_pc_slvup_01.nif",
	"./data/effect/eff_pc_target_01.nif",
	"./data/effect/eff_pc_target_02.nif",
	"./data/effect/eff_pc_target_03.nif",
	"./data/effect/eff_pc_target_11.nif",
	"./data/effect/eff_pc_target_12.nif",
	"./data/effect/eff_bagic_dam_01.nif",
	"./data/effect/eff_basic_miss_01.nif",
	"./data/effect/eff_bagic_damc_01.nif",
	"./data/effect/map_portal_c.nif",
	"./data/effect/eff_pc_rest.nif",
	"./data/effect/eff_pc_enchant_01.nif",
	"./Data/effect/eff_pc_get_05.nif",
	"./data/effect/eff_pc_pvp_1.nif",
	"./data/effect/eff_pc_pvp_2.nif",
	"./data/effect/eff_pc_pvp_3.nif",
	"./data/effect/eff_pc_pvp_4.nif",
	"./data/effect/eff_pc_itemdrop_01.nif",
	"./data/effect/eff_pc_itemdrop_02.nif",
	"./data/map/character_select_cardeffect.nif",
	"./data/map/character_select_effect.nif",
	/// loadmapnif
	"./data/map/character_select_bg.nif",
	"./data/map/character_select_card.nif",

};

// CResourceCheckerDlg 대화 상자
class CResourceCheckerDlg : public CDialog
{
// 생성입니다.
public:
	CResourceCheckerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	virtual ~CResourceCheckerDlg();
// 대화 상자 데이터입니다.
	enum { IDD = IDD_RESOURCECHECKER_DIALOG };
	
	void Clear();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	int Search(char* root, char* sub ); /// 폴더에 담겨져 있는 파일 저장하는 함수

public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonPath();

	CEdit mEditPath;
	CComboBox mComboLanguage;
	CButton mButtonStart;
	CButton mButtonCancel;
	CButton mButtonPath;
	CEdit mEditNotice;

private:

	cDramaturgyManager*   mpDramaturgyManager;
	cItemManager*		  mpItemManager;
	cTriggerManager*	  mpTriggerManager;
	cFilePackSystem*	  mpFileSystem;
	cQuestManager*		  mpQuestManager;
	cShortcutManager*     mpShortcutManager;
	cFilterManager*		  mpFilterManager;
	cGameResourceManager* mpGameResourceManager;
	cUIManager*			  mpUIManager;
	cTarotManager*		  mpTarotManager;
	cTipManager*		  mpTipManager;
	cTutorialManager*	  mpTutorialManager;
	cMakeStage*			  mpMakeStage;
	cRenderSystem*		  mpRenderSystem;
	cResourceManager*	  mpResourceManager;
	cSoundSystem*		  mpSoundSystem;	
	cTitleManager*		  mpTitleManager;
	cLoadStage*			  mpLoadStage;
	cWorldManager*		  mpWorldManager;
	cSceneManager*		  mpSceneManager;

	NiInitOptions		  mBryoOption;

	//tHashSet<cString> mFileList;
	tSet<cString> mFileList;

};
