/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.11.27
* 내    용 : 탭 버튼 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

class cTabButton;

/// 탭 버튼 그룹
class cTabButtonGroup
{
	friend class cTabButton;

public:
	cTabButtonGroup();
	~cTabButtonGroup();

	/// 체크된 버튼을 설정
	void SetCheckedButton( int id );
	void SetCheckedButton( cTabButton* button );

	/// 체크된 버튼을 리턴
	cTabButton* GetCheckedButton() const;

protected:
	/// 버튼을 추가
	void Add( cTabButton* button );

	/// 버튼을 제거
	bool Remove( unsigned int index );

protected:
	/// 버튼 배열
	typedef tArray<cTabButton*> cButtonArray;
	cButtonArray mButtonArray;

	/// 체크된 버튼
	cTabButton* mCheckedButton;
};

inline cTabButton* cTabButtonGroup::GetCheckedButton() const
{
	return mCheckedButton;
}
