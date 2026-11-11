/* ==========================================================================
* 파    일 : GuildManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-10-31
* 주의사항 :
*===========================================================================*/
#include "BaseStage.h"
#include "Guild_Common.h"

#define REGIST_GUILDMARK	"./GuildMark/mark.bmp"

const unsigned long GUILD_WAIT_TIME = 10000;		// 길드초대 지속시간

struct MSG_RES_NPC_GUILD_CREATE;

struct MARK_VERSION
{
	unsigned long	guildIndex;
	tm		versionData;
};

#define MARK_MAX_SIZE	MARK_SIZE + sizeof(MARK_VERSION)

class cGuildManager
{
public: 
	cGuildManager();
	~cGuildManager();

	bool Init();
	void Open();
	void Close();
	
	///
	bool GuildCreate();
	void GuildAdd( cStringT name );
	void GuildOut();
	void GuildCut( unsigned long characterIdx );
	void GuildDelete();
	void GuildGivePosition( unsigned long characterIdx, unsigned char position, unsigned char newPosition );
	void GuildNotice( LPCTSTR notice );
	void GuildMarkRequest( unsigned long guildIndex );

	void ResGuildCreate( MSG_RES_NPC_GUILD_CREATE* pmsg );

	/// 새 길드마크 등록 요청
	void GuildMarkUpdate();

	/// 길드 마크 텍스쳐 뽑기
	NiTexture* LoadGuildMark( unsigned long guildIndex );
	NiTexture* LoadPreviewGuildMark( const cString& pathName );
	void ClearPreviewGuildMark();

	/// 
	NiTexture* CheckMarkVersion( unsigned long guildIndex, tm registDate );

	///
	void NetworkMsgParser( char* msg );

	/// 직위의 텍스트 인덱스 가져오기
	LPCTSTR GetTextIndex( unsigned char position );

	void SetTempData( LPCTSTR name, unsigned long npcIdx );
	void ClearTempData();
	unsigned long GetMasterIndex() { return mGuildInfo.masterIndex; }
	wchar_t* GetGuildNotice() { return mGuildInfo.notice; }
	int GetTotalCount() { return mGuildUserMap.GetSize(); }

	void GuildInfoReset() { mGuildInfoInit = false; }

public:
	static cGuildManager* mpSingleton;	
	static cGuildManager* GetSingleton();

private:
	void Clear();
	
	/// 길드 정보 시트 갱신
	void UpdateInfoSheet();

	/// 길드원 가입/탈퇴 에 의한 변경
	void UpdateInfoSheetByUser();

	/// 길드 공지 출력
	void UpdateNotice();

	///
	void AddGuildUser( sGuildUserInfo* user );

	/// 길드마크폴더 생성
	bool CheckGuildMarkFolder();

	///
	void SaveGuildMark( unsigned long guildIndex, tm registDate, BYTE* mark );
	
private:
	/// 길드 정보
	sGuildInfo		mGuildInfo;

	/// 길드 유저리스트 (sGuildUserInfo)
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;
	cGuildUserMap	mGuildUserMap;

	/// 접속 인원 수
	unsigned long	mConnectCount;

	/// 길드이름 생성
	cStringT		mTempName;
	unsigned long	mTempNpcIndex;

	/// 추방유저 인덱스 임시 저장
	unsigned long	mTempCutPlayerIdx;

	/// 길드원 추가 유저명 저장
	cStringT		mTempAddName;

	bool			mGuildInfoInit;

	//
	NiTexture*		mpPreviewMark;

	/// 길드마크 버퍼
	BYTE			mMarkBuffer[MARK_SIZE];

	/// 길드마크 요청 정보( 길드인덱스 )
	typedef tHashSet<unsigned long> cRequestMarkSet;
	cRequestMarkSet		mRequestMarkSet;

	typedef tPointerHashMap<unsigned long, NiTexturePtr> cMarkTextureMap;
	cMarkTextureMap		mMarkTextureMap;
};

inline cGuildManager* cGuildManager::GetSingleton()
{
	return mpSingleton;
}

#define GUILDMAN cGuildManager::GetSingleton()

