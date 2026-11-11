/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.12
* 내    용 : std::char_traits를 감싼 문자 특성 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4267 )
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <stl/char_traits.h>

using namespace std;

/// 단항 빼기 연산자가 부호 없는 형식에 적용되었습니다.
#pragma warning(disable: 4146)
/// 형변환하면서 데이터가 손실될 수 있습니다.
#pragma warning(disable: 4244)
/// 함수가 deprecated로 선언되었습니다.
//#pragma warning(disable: 4996)

/// 문자 특성
template<class T>
class tCharTraits
{
};

template<>
class tCharTraits<char> : public char_traits<char>
{
public:
	/// 문자열 s0과 문자열 s1을 비교
	static int Compare( const char* s0, const char* s1 )
	{
		while( *s0 == *s1 )
		{
			if( *s0 == 0 )
			{
				return 0;
			}
			s0++;
			s1++;
		}
		return *s0 - *s1;
	}

	/// 문자열 s0과 문자열 s1을 길이 n만큼 비교
	static int Compare( const char* s0, const char* s1, unsigned int n )
	{
		return ::memcmp( s0, s1, n );
	}

	/// 문자열의 길이를 리턴
	static unsigned int GetLength( const char* s )
	{
		return ::strlen( s );
	}

	/// 숫자로 변환
	static long ToLong( const char*nptr, char**endptr, int base )
	{
		/// 원본
		//const char*s = nptr;
		//unsigned long acc;
		//int c;
		//unsigned long cutoff;
		//int neg = 0, any, cutlim;

		///*
		//* Skip white space and pick up leading +/- sign if any.
		//* If base is 0, allow 0x for hex and 0 for octal, else
		//* assume decimal; if base is already 16, allow 0x.
		//*/
		//do{
		//	c = *s++;
		//} while(::isspace(c));

		//if(c == '-')
		//{
		//	neg = 1;
		//	c = *s++;
		//}
		//else if(c == '+')
		//	c = *s++;
		//if((base == 0 || base == 16) &&
		//	c == '0' && (*s == 'x' || *s == 'X'))
		//{
		//	c = s[1];
		//	s += 2;
		//	base = 16;
		//}
		//if(base == 0)
		//	base = c == '0' ? 8 : 10;

		///*
		//* Compute the cutoff value between legal numbers and illegal
		//* numbers.  That is the largest legal value, divided by the
		//* base.  An input number that is greater than this value, if
		//* followed by a legal input character, is too big.  One that
		//* is equal to this value may be valid or not; the limit
		//* between valid and invalid numbers is then based on the last
		//* digit.  For instance, if the range for longs is
		//* [-2147483648..2147483647] and the input base is 10,
		//* cutoff will be set to 214748364 and cutlim to either
		//* 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
		//* a value > 214748364, or equal but the next digit is > 7 (or 8),
		//* the number is too big, and we will return a range error.
		//*
		//* Set any if any `digits' consumed; make it negative to indicate
		//* overflow.
		//*/
		//cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
		//cutlim = cutoff % (unsigned long)base;
		//cutoff /= (unsigned long)base;

		//for(acc = 0, any = 0;; c = *s++)
		//{
		//	if( ::isdigit(c) )
		//		c -= '0';
		//	else if( ::isalpha(c) )
		//		c -= ::isupper(c) ? 'A' - 10 : 'a' - 10;
		//	else
		//		break;
		//	if(c >= base)
		//		break;
		//	if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
		//		any = -1;
		//	else
		//	{
		//		any = 1;
		//		acc *= base;
		//		acc += c;
		//	}
		//}
		//if(any < 0)
		//{
		//	acc = neg ? LONG_MIN : LONG_MAX;
		//}
		//else if(neg)
		//	acc = -acc;
		//if(endptr != 0)
		//	*endptr = any ? (char*)s - 1 : (char*)nptr;
		//return acc;

		const char*s = nptr;
		unsigned long acc;
		int c;
		unsigned long cutoff;
		int neg = 0, any, cutlim;

		/*
		* Skip white space and pick up leading +/- sign if any.
		* If base is 0, allow 0x for hex and 0 for octal, else
		* assume decimal; if base is already 16, allow 0x.
		*/
		do{
			c = *s++;
		} while(::isspace(c));

		if(c == '-')
		{
			neg = 1;
			c = *s++;
		}
		else if(c == '+')
			c = *s++;
		if((base == 0 || base == 16) &&
			c == '0' && (*s == 'x' || *s == 'X'))
		{
			c = s[1];
			s += 2;
			base = 16;
		}
		if(base == 0)
			base = c == '0' ? 8 : 10;

		/*
		* Compute the cutoff value between legal numbers and illegal
		* numbers.  That is the largest legal value, divided by the
		* base.  An input number that is greater than this value, if
		* followed by a legal input character, is too big.  One that
		* is equal to this value may be valid or not; the limit
		* between valid and invalid numbers is then based on the last
		* digit.  For instance, if the range for longs is
		* [-2147483648..2147483647] and the input base is 10,
		* cutoff will be set to 214748364 and cutlim to either
		* 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
		* a value > 214748364, or equal but the next digit is > 7 (or 8),
		* the number is too big, and we will return a range error.
		*
		* Set any if any `digits' consumed; make it negative to indicate
		* overflow.
		*/
		if( neg == 1 )
		{
			cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
			cutlim = cutoff % (unsigned long)base;
			cutoff /= (unsigned long)base;

			for(acc = 0, any = 0;; c = *s++)
			{
				if( ::isdigit(c) )
					c -= '0';
				else if( ::isalpha(c) )
					c -= ::isupper(c) ? 'A' - 10 : 'a' - 10;
				else
					break;
				if(c >= base)
					break;
				if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
				{
					assert(NULL);	/// 수치 초과
					any = -1;
				}
				else
				{
					any = 1;
					acc *= base;
					acc += c;
				}
			}
			if(any < 0)
			{
				acc = neg ? LONG_MIN : LONG_MAX;
			}
			else if(neg)
				acc = -acc;
			if(endptr != 0)
				*endptr = any ? (char*)s - 1 : (char*)nptr;
			return acc;
		}
		else
		{
			/// unsigned 최대값을 ULONG_MAX(41억...) 값으로 놓고 읽어온다.
			cutoff = ULONG_MAX;
			cutlim = cutoff % (unsigned long)base;
			cutoff /= (unsigned long)base;

			for(acc = 0, any = 0;; c = *s++)
			{
				if( ::isdigit(c) )
					c -= '0';
				else if( ::isalpha(c) )
					c -= ::isupper(c) ? 'A' - 10 : 'a' - 10;
				else
					break;
				if(c >= base)
					break;
				if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
				{
					assert(NULL);	/// 수치 초과
					any = -1;
				}
				else
				{
					any = 1;
					acc *= base;
					acc += c;
				}
			}
			if(any < 0)
			{
				acc = ULONG_MAX;
			}
			if(endptr != 0)
				*endptr = any ? (char*)s - 1 : (char*)nptr;
			return acc;
		}
	}

	static double ToDouble( const char*s, char**sret )
	{
		long double r;    /* result */
		int e;            /* exponent */
		long double d;    /* scale */
		int sign;         /* +- 1.0 */
		int esign;
		int i;
		int flags=0;

		r = 0.0;
		sign = 1;
		e = 0;
		esign = 1;

		while((*s == ' ') || (*s == '\t'))
			s++;

		if(*s == '+')
			s++;
		else if(*s == '-')
		{
			sign = -1;
			s++;
		}

		while((*s >= '0') && (*s <= '9'))
		{
			flags |= 1;
			r *= 10.0;
			r += *s - '0';
			s++;
		}

		if(*s == '.')
		{
			d = 0.1L;
			s++;
			while((*s >= '0') && (*s <= '9'))
			{
				flags |= 2;
				r += d * (*s - '0');
				s++;
				d *= 0.1L;
			}
		}

		if(flags == 0)
		{
			if(sret)
				*sret = (char*)s;
			return 0;
		}

		if((*s == 'e') || (*s == 'E'))
		{
			s++;
			if(*s == '+')
				s++;
			else if(*s == '-')
			{
				s++;
				esign = -1;
			}
			if((*s < '0') || (*s > '9'))
			{
				if(sret)
					*sret = (char*)s;
				return r;
			}

			while((*s >= '0') && (*s <= '9'))
			{
				e *= 10;
				e += *s - '0';
				s++;
			}
		}

		if(esign < 0)
			for(i = 1; i <= e; i++)
				r *= 0.1L;
		else
			for(i = 1; i <= e; i++)
				r *= 10.0;

		if(sret)
			*sret = (char*)s;
		return r * sign;
	}

	static int ToInt( const char* s )
	{
		return (int)ToLong( s, 0, 10 );
	}

	static float ToFloat( const char* s )
	{
		return (float)ToDouble( s, 0 );
	}

	/// 소문자를 대문자로 변환
	static char ToUpper( char c )
	{
		return ::toupper( c );
	}

	/// 문자열안의 소문자를 대문자로 변환
	static void ToUpper( char* s )
	{
		for( ; *s; ++s )
		{
			*s = ::toupper( *s );
		}
	}

	/// 문자열안의 대문자를 소문자로 변환
	static char ToLower( char c )
	{
		return ::tolower( c );
	}

	/// 문자열안의 대문자를 소문자로 변환
	static void ToLower( char* s )
	{
		for( ; *s; ++s )
		{
			*s = ::tolower( *s );
		}
	}

	/// 문자
	static unsigned int GetHashCode( const char* str )
	{
		unsigned long h = 0; 

		for( ; *str; ++str )
		{
			h = 5 * h + *str;
		}
		return unsigned int( h );
	}

	/// 공백 문자가 아니면 참을 리턴
	static bool IsNotSpace( char c )
	{
		return ::isspace(c) == false;
	};

	/// 가변 인자로부터 형식화 입력
	static int _vsnprintf( char* buffer, unsigned int count, const char* format, va_list argptr )
	{
		return ::_vsnprintf_s( buffer, count, _TRUNCATE, format, argptr );
	}
};


template<>
class tCharTraits<wchar_t> : public char_traits<wchar_t>
{
public:
	/// 문자열 s0과 문자열 s1을 비교
	static int Compare( const wchar_t* s0, const wchar_t* s1 )
	{
		while( *s0 == *s1 )
		{
			if( *s0 == 0 )
			{
				return 0;
			}
			s0++;
			s1++;
		}
		return *s0 - *s1;
	}

	/// 문자열 s0과 문자열 s1을 길이 n만큼 비교
	static int Compare( const wchar_t* s0, const wchar_t* s1, unsigned int n )
	{
		return ::memcmp( s0, s1, n*sizeof(wchar_t) );
	}

	/// 문자열의 길이를 리턴
	static unsigned int GetLength( const wchar_t* s )
	{
		return ::wcslen( s );
	}

	/// 숫자로 변환
	static long ToLong( const wchar_t* nptr, const wchar_t** endptr, int base )
	{
		const wchar_t* s = nptr;
		unsigned long acc;
		int c;
		unsigned long cutoff;
		int neg = 0, any, cutlim;

		/*
		* Skip white space and pick up leading +/- sign if any.
		* If base is 0, allow 0x for hex and 0 for octal, else
		* assume decimal; if base is already 16, allow 0x.
		*/
		do{
			c = *s++;
		} while( ::iswspace(c) );

		if(c == L'-')
		{
			neg = 1;
			c = *s++;
		}
		else if(c == L'+')
			c = *s++;
		if((base == 0 || base == 16) &&
			c == L'0' && (*s == L'x' || *s == L'X'))
		{
			c = s[1];
			s += 2;
			base = 16;
		}
		if(base == 0)
			base = c == '0' ? 8 : 10;

		/*
		* Compute the cutoff value between legal numbers and illegal
		* numbers.  That is the largest legal value, divided by the
		* base.  An input number that is greater than this value, if
		* followed by a legal input character, is too big.  One that
		* is equal to this value may be valid or not; the limit
		* between valid and invalid numbers is then based on the last
		* digit.  For instance, if the range for longs is
		* [-2147483648..2147483647] and the input base is 10,
		* cutoff will be set to 214748364 and cutlim to either
		* 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
		* a value > 214748364, or equal but the next digit is > 7 (or 8),
		* the number is too big, and we will return a range error.
		*
		* Set any if any `digits' consumed; make it negative to indicate
		* overflow.
		*/
		cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
		cutlim = cutoff % (unsigned long)base;
		cutoff /= (unsigned long)base;

		for(acc = 0, any = 0;; c = *s++)
		{
			if( ::iswdigit(c) )
				c -= L'0';
			else if( ::iswalpha(c) )
				c -= ::iswupper(c) ? L'A' - 10 : L'a' - 10;
			else
				break;
			if(c >= base)
				break;
			if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
				any = -1;
			else
			{
				any = 1;
				acc *= base;
				acc += c;
			}
		}
		if(any < 0)
		{
			acc = neg ? LONG_MIN : LONG_MAX;
		}
		else if(neg)
			acc = -acc;
		if(endptr != 0)
			*endptr = any ? (wchar_t*)s - 1 : (wchar_t*)nptr;
		return acc;
	}

	static double ToDouble( const wchar_t* s, const wchar_t** sret )
	{
		long double r;    /* result */
		int e;            /* exponent */
		long double d;    /* scale */
		int sign;         /* +- 1.0 */
		int esign;
		int i;
		int flags=0;

		r = 0.0;
		sign = 1;
		e = 0;
		esign = 1;

		while((*s == L' ') || (*s == L'\t'))
			s++;

		if(*s == L'+')
			s++;
		else if(*s == L'-')
		{
			sign = -1;
			s++;
		}

		while((*s >= L'0') && (*s <= L'9'))
		{
			flags |= 1;
			r *= 10.0;
			r += *s - L'0';
			s++;
		}

		if(*s == L'.')
		{
			d = 0.1L;
			s++;
			while((*s >= L'0') && (*s <= L'9'))
			{
				flags |= 2;
				r += d * (*s - L'0');
				s++;
				d *= 0.1L;
			}
		}

		if(flags == 0)
		{
			if(sret)
				*sret = (wchar_t*)s;
			return 0;
		}

		if((*s == L'e') || (*s == L'E'))
		{
			s++;
			if(*s == L'+')
				s++;
			else if(*s == L'-')
			{
				s++;
				esign = -1;
			}
			if((*s < L'0') || (*s > L'9'))
			{
				if(sret)
					*sret = (wchar_t*)s;
				return r;
			}

			while((*s >= L'0') && (*s <= L'9'))
			{
				e *= 10;
				e += *s - L'0';
				s++;
			}
		}

		if(esign < 0)
			for(i = 1; i <= e; i++)
				r *= 0.1L;
		else
			for(i = 1; i <= e; i++)
				r *= 10.0;

		if(sret)
			*sret = (wchar_t*)s;
		return r * sign;
	}

	static int ToInt( const wchar_t* s )
	{
		return (int)ToLong( s, 0, 10 );
	}

	static float ToFloat( const wchar_t* s )
	{
		return (float)ToDouble( s, 0 );
	}

	/// 소문자를 대문자로 변환
	static wchar_t ToUpper( wchar_t c )
	{
		return ::towupper( c );
	}

	/// 문자열안의 소문자를 대문자로 변환
	static void ToUpper( wchar_t* s )
	{
		for( ; *s; ++s )
		{
			*s = ::towupper( *s );
		}
	}

	/// 문자열안의 대문자를 소문자로 변환
	static wchar_t ToLower( wchar_t c )
	{
		return ::towlower( c );
	}

	/// 문자열안의 대문자를 소문자로 변환
	static void ToLower( wchar_t* s )
	{
		for( ; *s; ++s )
		{
			*s = ::towlower( *s );
		}
	}

	/// 문자
	static unsigned int GetHashCode( const wchar_t* str )
	{
		unsigned long h = 0; 

		for( ; *str; ++str )
		{
			h = 5 * h + *str;
		}
		return unsigned int( h );
	}

	/// 공백 문자가 아니면 참을 리턴
	static bool IsNotSpace( wchar_t c )
	{
		return ::iswspace(c) == false;
	};

	/// 가변 인자로부터 형식화 입력
	static int _vsnprintf( wchar_t* buffer, unsigned int count, const wchar_t* format, va_list argptr )
	{
		return ::_vsnwprintf_s( buffer, count, _TRUNCATE, format, argptr );
	}
};

/// 문자열 전역 함수
typedef tCharTraits<char> cStr;
typedef tCharTraits<wchar_t> cStrW;
#pragma warning( pop )
