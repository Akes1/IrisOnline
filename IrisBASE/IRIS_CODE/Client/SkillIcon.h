#pragma once
#include "icon.h"

class cSkillIcon : public cIcon
{
public:
	cSkillIcon();
	virtual ~cSkillIcon();

	void SetSkillInfo( unsigned long idx, unsigned char step, unsigned long job, unsigned char tipKind );

	inline unsigned long	GetSkillIdx() { return mSkillIdx; }
	inline unsigned char	GetSkillStep() { return mSkillStep; }

	inline void SetAuraUniqueIdx( unsigned long idx ) { mAuraUniqueIdx = idx; }
	inline unsigned long	GetAuraUniqueIdx() { return mAuraUniqueIdx; }

	inline bool	IsBaseSkill() { return mBaseSkill; }
	inline bool IsActiveSkill() { return mIsActiveSkill; }

protected:
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	
protected:

	cUIRect				mFixedRect;

	/// 활성화 되어있냐
	bool				mBaseSkill;

	unsigned long		mJobIdx;
	unsigned long		mSkillIdx;
	unsigned char		mSkillStep;
	unsigned long		mAuraUniqueIdx;

	unsigned char		mTipKind;

	/// 발동용 스킬이냐
	bool				mIsActiveSkill;

	/// 발동타입
	int					mShotType;
};

class cCommunityIcon : public cIcon
{
public:
	cCommunityIcon();
	virtual ~cCommunityIcon();

	void InitInfo( unsigned long index, bool emote );

	inline bool IsEmoting() { return mEmoteIcon; }
	inline unsigned long GetCommunityIndex() { return mCommunityIndex; }

protected:
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );

protected:
	bool			mEmoteIcon;

	unsigned long	mCommunityIndex;
};

