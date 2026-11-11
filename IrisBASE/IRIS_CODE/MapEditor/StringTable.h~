/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.29
* 내    용 : 스트링 테이블
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 스트링 테이블
/// 엑셀의 csv파일로 저장할 수 있다.
/// csv 파일은 쉼표(,)로 칸을 구분하므로, 값 안에 쉼표를 포함하면 안된다.
class cStringTable
{
public:
	cStringTable();
	virtual ~cStringTable();

	void Clear();
	bool Save( const cString& pathName );

	void SetValue( unsigned int row, unsigned int col, const cString& value );
	void SetValue( unsigned int row, unsigned int col, const char* value );
	void SetValue( unsigned int row, unsigned int col, unsigned int value );
	void SetValue( unsigned int row, unsigned int col, float value );

protected:
	typedef tArray<cString*> cArray;
	typedef tArray<cArray*> cTable;

	cTable mTable;
	unsigned int mMaxCol;
};
