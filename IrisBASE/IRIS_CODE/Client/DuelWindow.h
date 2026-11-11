#pragma once
#include "UIWindow.h"
#include "Label.h"

class cDoubleGauge;
class cDuelBuff;
class cPlayer;
class cUITransformImage;
class cDuelFaceLabel;

class cDuelWindow : public cUIWindow
{
public:
	cDuelWindow();
	virtual~cDuelWindow();

	virtual void Open();
	virtual void Close();

	void AddBuff( bool setEnemy, unsigned long uniqueIdx, unsigned long influenceIdx );
	void DelBuff( bool setEnemy, unsigned long uniqueIdx );

	void AddDeBuff( bool setEnemy, unsigned long uniqueIdx, unsigned long influenceIdx );
	void DelDeBuff( bool setEnemy, unsigned long uniqueIdx );

	void SetDuelInfo( cPlayer* enemy );

	void UpdateHeroHP( unsigned long value, unsigned long max );
	void UpdateHeroMP( unsigned long value, unsigned long max );
	void UpdateEnemyHP( unsigned long value, unsigned long max );
	void UpdateEnemyMP( unsigned long value, unsigned long max );

	void ChangeCountImage( unsigned int i );
	void ShowFight();
	void ShowOutRangeText( bool show );

	/// °»½Å
	virtual void UpdateRect();

	virtual void Show( bool onsound=false );

	enum
	{
		eNone = 0,
		eShow,
		eHide,
	};
	void EndDuel() { mShowProcess = eHide; }

	virtual void NetLock( int lockTry = 0 );
	virtual void NetUnLock( int lockTry = 0 );

protected:
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateSkin();

	bool CreateFace();

	void OnShow();
	void OnHide();

	void UpdateRectEx();


protected:
	/// Hero
	cDoubleGauge*	mpHeroHP;
	cDoubleGauge*	mpHeroMP;

	cLabel*			mpHeroName;

	cDuelBuff*		mpHeroBuff;
	cDuelBuff*		mpHeroDeBuff;

	/// Enemy
	cDoubleGauge*	mpEnemyHP;
	cDoubleGauge*	mpEnemyMP;

	cLabel*			mpEnemyName;

	cDuelBuff*		mpEnemyBuff;
	cDuelBuff*		mpEnemyDeBuff;

	/// count
	cLabel*			mpCountLabel;
	cLabel*			mpFightLabel;

	/// hero face
	cLabel*	mpHeroFace;
	
	/// enemy face
	cLabel*	mpEnemyFace;

	cUIRect mFaceRect[6];

	unsigned char	mShowProcess;
	unsigned long	mOutRangeTime;
	unsigned int	mOutCount;
};
