#pragma once

class Noncopyable
{
protected:
	Noncopyable() {}
	~Noncopyable() {}

private:
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator = (const Noncopyable&);
};

//#ifdef _MSC_VER
//#pragma warning(disable:4511)		// 복사 생성자를 생성하지 못했습니다.
//#pragma warning(disable:4512)		// 컴파일러에서 주어진 클래스에 대한 대입 연산자를 생성할 수 없습니다.
//#endif

/// 사정상 Noncopyable을 바로 상속받지 못하는 클래스에서, 
/// 복사 생성자 및 연산자를 막기 위한 매크로
#define DECLARE_NONCOPYABLE(NAME) private: \
	NAME(const NAME&) {} \
	NAME& operator = (const NAME&) { return *this; }