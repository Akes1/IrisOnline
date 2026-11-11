#pragma once

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - n)))
#define REVERSE_ENDIAN(x) ((x) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | ((x) >> 24)

/* --------------------------------------------------------------------- */
//  sha1
/* --------------------------------------------------------------------- */
class sha1 {
public:

	sha1()
	{ 
		reset(); 
	}
	virtual ~sha1() {}

	explicit sha1(const char* src, size_t n) 
	{
		encode(src, n);
	}

	explicit sha1(const cString& src) 
	{
		encode(src);
	}

	
	sha1& encode(const char* src, size_t n) 
	{
		reset();
		update(src, n);
		finish();
		return *this;
	}

	sha1& encode(const cString& src) 
	{
		return encode(src.Cstr(), src.GetLength());
	}

	void reset()
	{
		memset(buffer_, 0, sizeof(buffer_)	);
		memset(code_,	0, sizeof(code_)	);

		count_[0] = 0;
		count_[1] = 0;

		w_[0] = 0x67452301;
		w_[1] = 0xefcdab89;
		w_[2] = 0x98badcfe;
		w_[3] = 0x10325476;
		w_[4] = 0xc3d2e1f0;
	}

	void update(const char* src, size_t n) 
	{
		if (n == 0) 
			return;

		unsigned char* block = (unsigned char*)src;

		unsigned int left = count_[0] & 0x3f;
		unsigned int fill = 64 - left;

		count_[0] += static_cast<unsigned int>(n);
		count_[0] &= 0xffffffff;

		if (count_[0] < n) 
			++count_[1];

		if (left > 0 && n >= fill) 
		{
			::memcpy(buffer_ + left, block, fill);
			calculate(buffer_, w_);

			n -= fill;
			block += fill;
			left = 0;
		}

		while (n >= 64) 
		{
			calculate(block, w_);
			n -= 64;
			block += 64;
		}

		if (n > 0) 
			memcpy(buffer_ + left, block, n);
	}

	void finish() 
	{
		unsigned int high	= REVERSE_ENDIAN((count_[0] >> 29) | (count_[1] << 3));
		unsigned int low	= REVERSE_ENDIAN(count_[0] << 3);
		unsigned char nbit[8];

		memcpy(nbit,	&high, 4);
		memcpy(&nbit[4], &low, 4);

		unsigned int last = count_[0] & 0x3f;
		unsigned int padn = (last < 56) ? (56 - last) : (120 - last);

		unsigned char padding[64];
		memset(padding, 0, sizeof(padding));
		padding[0] = 0x80;

		update((char*)padding, padn);
		update((char*)nbit, 8);

		for (int i = 0, j = 0; i < 5; ++i) 
		{
			code_[j++] = (unsigned char)((w_[i] >> 24) & 0x000000ff);
			code_[j++] = (unsigned char)((w_[i] >> 16) & 0x000000ff);
			code_[j++] = (unsigned char)((w_[i] >> 8) & 0x000000ff);
			code_[j++] = (unsigned char)(w_[i] & 0x000000ff);
		}

		memset(buffer_, 0, sizeof(buffer_));
	}

	int size() const { return sizeof(code_) - 1; }	
	const unsigned char* code() const { return code_; }

	std::string to_string() const 
	{
		cString str;		

		for (int i = 0; i < 5; ++i) 
		{
			char cBuff[10] = {0,};
			_itoa(w_[i], cBuff, 16);					
			int len = strlen(cBuff);

			char pTemp[9] = "00000000";
			strcpy_s( &pTemp[ 8 - len ], len, cBuff );
			
			str.Append( pTemp );			
		}
		return str.Cstr();
	}

private:

	unsigned char code_[21]; // message digest (32bit * 5 = 160bit)

	// working fields
	unsigned int	w_[5];
	unsigned int	count_[2];
	unsigned char	buffer_[64];

	static unsigned int f(int t, unsigned int b, unsigned int c, unsigned int d) 
	{
		if (t < 20) 
		{
			return (b & c) | (~b & d);
		}
		else if (t < 40) 
		{
			return b ^ c ^ d;
		}
		else if (t < 60) 
		{
			return (b & c) | (b & d) | (c & d);
		}
		else 
		{
			return b ^ c ^ d;
		}
	}

	static unsigned int k(int t) 
	{
		if (t < 20)
		{
			return 0x5a827999;
		}
		else if (t < 40) 
		{
			return 0x6ed9eba1;
		}
		else if (t < 60) 
		{
			return 0x8f1bbcdc;
		}
		else 
		{
			return 0xca62c1d6;
		}
	}

	static void calculate(const unsigned char* block, unsigned int* code) 
	{
		unsigned int x[16];
		memcpy(x, block, sizeof(x));

		unsigned int w[80];
		for (int i = 0; i < 16; ++i) w[i] = REVERSE_ENDIAN(x[i]);
		for (int i = 16; i < 80; ++i) w[i] = ROTATE_LEFT(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

		unsigned int a = code[0];
		unsigned int b = code[1];
		unsigned int c = code[2];
		unsigned int d = code[3];
		unsigned int e = code[4];

		for (int t = 0; t < 80; ++t) 
		{
			unsigned int tmp = ROTATE_LEFT(a,5) + f(t,b,c,d) + e + w[t] + k(t);
			e = d;
			d = c;
			c = ROTATE_LEFT(b, 30);
			b = a;
			a = tmp;
		}

		code[0] += a;
		code[1] += b;
		code[2] += c;
		code[3] += d;
		code[4] += e;

		// zeroize sensitive information.
		::memset(x, 0, sizeof(x));
	}
};