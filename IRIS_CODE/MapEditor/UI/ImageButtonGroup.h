/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.11
* 내    용 : 이미지 버튼 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

class cImageButton;

/// 이미지 버튼 그룹
class cImageButtonGroup
{
	friend class cImageButton;

public:
	cImageButtonGroup();
	~cImageButtonGroup();

	/// 체크된 버튼을 설정
	void SetCheckedButton( int id );
	void SetCheckedButton( cImageButton* button );

	/// 체크된 버튼을 리턴
	cImageButton* GetCheckedButton() const;
	int GetCheckedButtonIndex() const;

protected:
	/// 버튼을 추가
	void Add( cImageButton* button );

	/// 버튼을 제거
	bool Remove( unsigned int index );

protected:
	/// 버튼 배열
	typedef tArray<cImageButton*> cButtonArray;
	cButtonArray mButtonArray;

	/// 체크된 버튼
	cImageButton* mCheckedButton;
};

inline
cImageButton* cImageButtonGroup::GetCheckedButton() const
{
	return mCheckedButton;
}
