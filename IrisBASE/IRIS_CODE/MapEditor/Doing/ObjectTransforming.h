/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 변환(이동, 회전, 배율) 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

/// 오브젝트 이동
class cObjectTranslating : public cDoing
{
	friend class cDoingManager;
	friend class cObjectGroup;

private:
	cObjectTranslating( const NiPoint3& undo, const NiPoint3& redo );

	void Undo();
	void Redo();

private:
	/// Undo 정보
	NiPoint3 mUndo;

	/// Redo 정보
	NiPoint3 mRedo;
};

/// 오브젝트 회전
class cObjectRotating : public cDoing
{
	friend class cDoingManager;
	friend class cObjectGroup;

private:
	cObjectRotating( const NiMatrix3& undo, const NiMatrix3& redo );

	void Undo();
	void Redo();

private:
	/// Undo 정보
	NiMatrix3 mUndo;

	/// Redo 정보
	NiMatrix3 mRedo;
};

/// 오브젝트 배율
class cObjectScaling : public cDoing
{
	friend class cDoingManager;
	friend class cObjectGroup;

private:
	cObjectScaling( float undo, float redo );

	void Undo();
	void Redo();

private:
	/// Undo 정보
	float mUndo;

	/// Redo 정보
	float mRedo;
};
