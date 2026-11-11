/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.11.27
* 내    용 : 체크 버튼 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

class cCheckButton;

/// 체크 버튼 그룹
class cCheckButtonGroup
{
	friend class cCheckButton;

public:
	cCheckButtonGroup();
	~cCheckButtonGroup();

	/// 체크된 버튼을 설정
	void SetCheckedButton( int id );
	void SetCheckedButton( cCheckButton* button );

	/// 체크된 버튼을 리턴
	cCheckButton* GetCheckedButton() const;

protected:
	/// 버튼을 추가
	void Add( cCheckButton* button );

	/// 버튼을 제거
	bool Remove( unsigned int index );

protected:
	/// 버튼 배열
	typedef tArray<cCheckButton*> cButtonArray;
	cButtonArray mButtonArray;

	/// 체크된 버튼
	cCheckButton* mCheckedButton;
};

inline
cCheckButton* cCheckButtonGroup::GetCheckedButton() const
{
	return mCheckedButton;
}
