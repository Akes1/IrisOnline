#pragma once

#include "Lexer.h"

enum
{
	/// 옵션파일을 읽을때 사용할 토큰..
	eTOKEN_WINMODE = 100,
	eTOKEN_RESOLUTION,
	eTOKEN_RAMP,
	eTOKEN_ENVVOLUME,
	eTOKEN_EFFVOLUME,
	eTOKEN_VIDEOMODE,
	eTOKEN_LANGUAGE,
};


class cOptionLexer : public cLexer
{
public:
	cOptionLexer( const char* pbuffer, unsigned int size );
};


struct sGameOption
{
	union
	{
		struct
		{
			bool showGauge				: 1;	// 
			bool showBubble				: 1;	//
			bool showHeroNameCard		: 1;	//
			bool showHeroTotal			: 1;	//
			bool showPlayerNameCard		: 1;	//
			bool showPlayerTotal		: 1;	//
			bool showNPCNameCard		: 1;	//
			bool showMonsterNameCard	: 1;	//
			bool showHelmet				: 1;	//
			bool showTip				: 1;	//

			bool rejectionTrade			: 1;	//
			bool rejectionDuel			: 1;	//
			bool rejectionWhisper		: 1;	//
			bool rejectionParty			: 1;	//
			bool rejectionFriend		: 1;	//
			bool rejectionPvP			: 1;	//

			bool autoSendTwitterMsg		: 1;	//
		} option1;
		long optionData1;
	};
};

#define MIN_RAMP_POS     0
#define MAX_RAMP_POS     50
#define POS_FACTOR  10.0f


class cOptionManager
{
	static cOptionManager* mpSingleton;

public:
	cOptionManager();
	virtual ~cOptionManager();

	bool Init();

	void				SetOptionValue( long option1 );
	void				ReqOptionValue();
	sGameOption*		GetGameOption() { return &mGameOption; }

	/// 적용 함수
	void SetWindowedOption( bool windowed, unsigned int width, unsigned int height, unsigned char bpp );
	void SetGammaPer( float r );
	void SetVolume( float env, float eff );
	void SetVideoOption( int videoMode );

	void SetWindowedNoReset( bool windowed ) { mWindowed = windowed; }

	/// 참조 함수
	inline bool IsWindowed() { return mWindowed; }

	inline unsigned int GetWindowWidth() { return mWidth; }
	inline unsigned int GetWindowHeight() { return mHeight; }
	inline unsigned char GetWindowBpp() { return mBpp; }

	inline float GetGammaPer() { return mGamma; }

	inline float GetEnvVolume() { return mEnvVolume; }
	inline float GetEffVolume() { return mEffVolume; }

	inline int   GetVideoMode(){ return mVideoMode; }

	inline bool IsShowBubble() { return mGameOption.option1.showBubble; }
	inline bool IsShowNPCNameCard() { return mGameOption.option1.showNPCNameCard; }
	inline bool IsShowMonsterNameCard() { return mGameOption.option1.showMonsterNameCard; }
	inline bool IsShowGauge() { return mGameOption.option1.showGauge; }
	inline bool IsShowTip() { return mGameOption.option1.showTip; }

	inline bool IsShowHeroNameCard() { return mGameOption.option1.showHeroNameCard; }
	inline bool IsShowHeroTotal() { return mGameOption.option1.showHeroTotal; }
	inline bool IsShowPlayerNameCard() { return mGameOption.option1.showPlayerNameCard; }
	inline bool IsShowPlayerTotal() { return mGameOption.option1.showPlayerTotal; }
	bool IsShowHelmet();

	inline bool IsAutoSendTwitterMsg() { return mGameOption.option1.autoSendTwitterMsg; }



public:
	/// 단일체를 리턴
	static cOptionManager* GetSingleton();

protected:
	bool Save( const char* pathName );
	bool Load( const char* pathName );

protected:
	bool	mWindowed;
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned char mBpp;
	float mGamma;

	float mEnvVolume;
	float mEffVolume;

	sGameOption			mGameOption;

	int   mVideoMode; ///enum BCAP_PRESET

	unsigned int mLanguage;
};

inline void cOptionManager::SetOptionValue( long option1 )
{
	mGameOption.optionData1 = option1;
}

inline cOptionManager* cOptionManager::GetSingleton()
{
	return mpSingleton;
}

#define OPTIONMAN	cOptionManager::GetSingleton()