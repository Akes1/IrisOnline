/* ==========================================================================
* 파    일 : SpinCtrl.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cSpinCtrl : public cUIWindow
{
public:
	cSpinCtrl( eUINodeType type = eUINODE_SPINBOX );
	virtual ~cSpinCtrl();

	struct sSpinData
	{
		cStringT mLabel;

		/// 확장 데이터는 항상 참조 데이터이다.
		void*	mpExData;

		sSpinData() : mpExData(0) {}
	};

 	virtual void Clear();

private:
	/// 스핀 자료구조 
//	typedef tArray<sSpinData*> cSpinArray;
	typedef tArray<void*> cSpinArray;
	cSpinArray mSpinArray;
};