/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.11
* 내    용 : 색 상자 그룹
* 주의사항 : 
*===========================================================================*/
#pragma once

class cColorBox;

/// 색 상자 그룹
class cColorBoxGroup
{
	friend class cColorBox;

public:
	cColorBoxGroup();
	~cColorBoxGroup();

	/// 체크된 상자을 설정
	void SetCheckedBox( int id );
	void SetCheckedBox( cColorBox* box );

	/// 체크된 상자을 리턴
	cColorBox* GetCheckedBox() const;
	int GetCheckedBoxIndex() const;

protected:
	/// 상자을 추가
	void Add( cColorBox* box );

	/// 상자을 제거
	bool Remove( unsigned int index );

protected:
	/// 상자 배열
	typedef tArray<cColorBox*> cBoxArray;
	cBoxArray mBoxArray;

	/// 체크된 상자
	cColorBox* mCheckedBox;
};

inline
cColorBox* cColorBoxGroup::GetCheckedBox() const
{
	return mCheckedBox;
}
