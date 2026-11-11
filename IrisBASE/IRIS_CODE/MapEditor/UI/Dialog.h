/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.21
* 내    용 : 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

class cDialog : public CDialog
{
public:
	/// 버튼
	virtual void SetCheckedButton( int id ) = 0;
	virtual unsigned int GetCheckedButton() const = 0;
};
