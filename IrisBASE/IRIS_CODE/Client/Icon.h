/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.14
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "BaseIcon.h"
#include "UIImageFont.h"
#include "UIRoundGauge.h"

class cPlaneObject;
enum eTooltipType;
enum eAniType
{
	eANI_AURA,		/// 오라스킬의 테두리
	eANI_FLASH,		/// 쿨타임 종료후 반짝임
	eANI_COUNT,		/// 쿨타임 카운트 용
		
	eANI_MAX,
};

/// 애니메이션 인자
struct sAniParam
{
	short			mFrame;				/// 프레임수
	unsigned int	mTexX, mTexY;		/// 텍스쳐 좌표
	unsigned int	mWidth, mHeight;	/// 이미지 사이즈
	short			mCurFrame;			/// 현재 프레임
	bool			mAniOn;				/// 애니사용여부
	bool			mReturn;			/// 애니메이션 한바퀴 돌리기

	/// 타이머
	unsigned long	mTimeOut;			/// 애니메이션 갱신 시간
	unsigned long	mAccumTime;
	unsigned long	mLastAccumTime;

	sAniParam()
	{
		::memset( this, 0, sizeof(sAniParam) );
	}
	~sAniParam(){}
};

/// 아이콘
class cIcon : public cBaseIcon
{
public:
	cIcon( eUINodeType type = eUINODE_ICON );
	virtual ~cIcon();

	/// 드래그중이면 게이지 안보임
	void ShowGauge( bool show );
	void ChangeGauge( cUIRoundGauge& gauge );
	void UpdateGauge( unsigned long startTime, unsigned long elapsedTime, unsigned long maintainTime, bool active );
	cUIRoundGauge& GetGauge() { return mGauge; }

	///
	bool IsActive();
	void StartDelay( unsigned long delayTime );
	void StopDelay();
	void SetElapsedTime( unsigned long delta );

	/// 번호
	unsigned int GetNumber() { return mNumber; }
	void SetNumber( unsigned int num );
	void ClearNumber();

	/// 갯수 보기 여부 
	void ShowNumber( bool show );
	bool IsShowNumber() const;

	void SetTipType( unsigned int type );
	unsigned int GetTipType() { return mTipType; }
	
	/// 아이콘에 애니메이션 적용 여부 세팅
	void SetAniEnable( bool enable );

	/// 애니메이션 세팅
	void SetAuraOn( bool set );
	bool IsAuraOn();

	void SetAniParam( sAniParam* param );
	sAniParam* GetAniParam() { return mAniParam; }

	void ClearCooltime();

protected:
	virtual bool SetSkin( const cUINodeSkin* skin );
	virtual void UpdateRect();

	/// 이벤트
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	
	/// 각 타이머
	void OnTimer( unsigned int type );
	void SetAniOn( eAniType type, short frame = 0 );

protected:
	/// 원형 게이지
	cUIRoundGauge mGauge;

	/// 이미지 폰트
	cUIImageFont* mImageFont;

	/// 번호 (아이템 개수)
	unsigned int mNumber;
	cString mNumText;

	/// 드래그 여부에 따른 게이지 쇼 여부
	bool mShowGauge;

	/// 
	bool mShowNumber;

protected:
	/// 애니메이션을 사용하는 아이콘인지 설정
	bool		mAniEnable;

	/// 카운트용 이미지 ( 애니메이션 )
	cPlaneObject*	mpCountImage;

	/// 반짝이는 이미지 ( 애니메이션 )
	cPlaneObject*	mpFlashImage;

	/// 오라스킬용 테두리 이미지( 애니메이션 )
	cPlaneObject*	mpEdgeImage;

	/// 애니메이션 정보 저장
	sAniParam	mAniParam[ eANI_MAX ];

	/// 툴팁색상에 따른 이미지 보기 여부 (레어도 관련)
	cUIImage*		mpTipImage;
	eTooltipType	mTipType;		/// 팁 컬러 타입
};

inline void cIcon::SetNumber( unsigned int num )
{
	mNumber = num;
	mNumText.Format( "%d", num );
}

inline void cIcon::ClearNumber()
{
	mNumText.Format( "" );
}

inline void cIcon::ShowNumber( bool show )
{
	mShowNumber = show;
}

inline bool cIcon::IsShowNumber() const
{
	return mShowNumber;
}

inline void cIcon::SetAniEnable( bool enable )
{
	mAniEnable = enable;
}

