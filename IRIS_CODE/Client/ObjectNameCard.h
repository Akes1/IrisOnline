/* ====================================================================
 * 파    일 :	ObjectNameCard.h
 * 목    적 :	객체의 이름을 표시하기 위한 클래스
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	
 * =================================================================== */
#pragma once

#include "DepthPlane.h"

class cPlaneObject;
class cBaseObject;

class cNameCardGauge;

/* 처리 사항
1. 찍는 위치 => x,z(foot dummy 위치) y(foot dummy_y + head dummy_y + offset value)
2. font object에 깊이 버퍼를 추가한 객체를 사용한다.

게이지 
5 4 ( 150 * 8 )
5 16( 150 * 8 )
*/

/// 표시 내용 옵션
enum eNameCardOption
{
	eRenderType_3D = 0,
	eRenderType_2D,
};

const unsigned int	GAUGE_WIDTH = 119;
const unsigned int	GAUGE_HEIGHT = 7;

const unsigned int	GAUGE_TEXWIDTH = 1;
const unsigned int	GAUGE_TEXHEIGHT = 7;

const unsigned int	NAMECARD_WIDTH = 300;//100;
const unsigned int	NAMECARD_HEIGHT = 68;//34;
const unsigned int	NAME_GAP = 4;

const unsigned int  PVP_MARK_A = 62;
const unsigned int  PVP_MARK_B = 63;

/// 사용 형태는 3D객체에 대한 이름만 찍는 경우 사용한다. (예 : 아이템)
class cNameCard : public cPlane
{
public:
	cNameCard();
	virtual ~cNameCard();

	struct sName
	{
		TCHAR			mString[256];
		int				mStrLen;
		unsigned long	mColor;
		long			mPixelWidth;
		long			mPixelHeight;

		sName()
		{
			_stprintf_s( mString, _T("%s"), _T("") );
			mStrLen = ::_tcslen(mString);
			mColor = 0xFFFFFFFF;//0xFF19F0A0;//0xFF111111;
//			mPixelWidth = NAMECARD_WIDTH;
//			mPixelHeight = 12;
			mPixelWidth = 0;
			mPixelHeight = 0;
		}
	};

	inline LPCTSTR GetName() { return mName.mString; }
	inline virtual unsigned long GetNameColor() { return mName.mColor; }

	virtual void SetName( LPTSTR name );
	virtual void SetName( LPTSTR name, unsigned long color );
	virtual void SetNameColor( unsigned long color );

	/// 갱신이 필요한 경우에 외부에서 호출한다.
	virtual void ChangeTextValue() {}
	virtual void ChangeGaugeValue() {}
	
	void Set2DPos( int hscreenX, int hscreenY, int fscreenX, int fscreenY );
	void Set2DFootPos( int fscreenX, int fscreenY );
	virtual void Draw();

protected:
	NiColorA					mBackColor;

	/// 객체 이름
	sName mName;

	int mHeadScreenX;
	int mHeadScreenY;
	int mFootScreenX;
	int mFootScreenY;
};


/// 사용 형태는 3D객체중 HP및MP에 대한 표현이 필요한 객체 (예 : 몬스터, NPC, 캐릭터 )
class cObjectNameCard : public cNameCard
{
public:
	virtual ~cObjectNameCard();

	inline virtual void SetEnableGauge( bool enable ) { mUsedGauge = enable; }

	virtual void ChangeGuildName() {}
	virtual void ChangeGuildMark() {}

	virtual void SetTodayWord( LPTSTR word, unsigned long color = 0xFFFFFFFF ){}
	virtual void PreviewGuildMark( NiTexture* tex ) {}

	void PreRender();

protected:
	cObjectNameCard( cBaseObject* pObj );

protected:
	/// 이름표의 주인 객체
	cBaseObject*		mpOriginObject;

	/// 게이지 정보
	bool mUsedGauge;
	cNameCardGauge*	mpHPGauge;
	cNameCardGauge*	mpMPGauge;
};


/// NPC 정보를 표시한다.
class cNpcNameCard : public cObjectNameCard
{
public:
	cNpcNameCard( cBaseObject* pObj );
	virtual ~cNpcNameCard();

	inline LPCTSTR GetJobName() { return mJobName.mString; }
	inline virtual unsigned long GetJobNameColor() { return mJobName.mColor; }

	virtual void ChangeTextValue();

	virtual void Draw();

protected:

	virtual void SetJobName( LPTSTR name );
	//virtual void SetJobName( LPTSTR name, unsigned long color );

	inline virtual void SetJobNameColor( unsigned long color ) { mJobName.mColor = color; }

protected:
	sName	mJobName;
};


/// 사용 형태는 3D객체중 몬스터에 대한 정보를 표시한다.(마나 게이지를 표시 하지 않는다)
class cMonsterNameCard : public cNpcNameCard
{
public:
	cMonsterNameCard( cBaseObject* pObj );
	virtual ~cMonsterNameCard();

	virtual void ChangeTextValue();
	virtual void ChangeGaugeValue();

	virtual void Draw();
};


/// 사용 형태는 3D객체중 Player와 같이 게임의 특성 정보를 표시하는 경우
class cPlayerNameCard : public cObjectNameCard
{
public:
	cPlayerNameCard( cBaseObject* pObj );
	virtual ~cPlayerNameCard();

	virtual void ChangeTextValue();
	virtual void ChangeGaugeValue();
	virtual void ChangeGuildName();
	virtual void ChangeGuildMark();

	virtual void SetDeleteWaitWord( LPTSTR word, unsigned long color = 0xFFFFFFFF );
	virtual void SetTitleName( LPTSTR titleName, unsigned long color = 0xFFFFFFFF );
	virtual void SetTodayWord( LPTSTR word, unsigned long color = 0xFFFFFFFF );
	virtual void SetGuildName( LPTSTR guildName );
	virtual void PreviewGuildMark( NiTexture* tex );

	virtual void Draw();

protected:

	bool mIsHeroNameCard;

	/// 개인호칭  
	sName	mTitleName;

	/// 길드이름
	sName	mGuildName;

	/// 오늘의 한마디
	sName	mTodayWord;
	sName	mPreviewTodayWord;	/// 미리보기용

	/// 삭제대기
	sName	mDeleteWaitWord;

	/// 길드 마크
	cPlaneObject*	mpGuildMark;
	bool			mEnableGuildMark;

	// 전장 팀 마크
	cPlaneObject*	mpPVPMark;
	bool			mEnablePVPMark;
};
