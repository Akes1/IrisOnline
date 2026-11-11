#pragma  once

/* --------------------------------------------------------------------- */
//  hmac
/* --------------------------------------------------------------------- */
template <class Engine>
inline Engine hmac(const char* key, size_t keylen, const char* data, size_t datalen) {

	unsigned char k_ipad[65];
	unsigned char k_opad[65];

	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));

	if (keylen > 64) {

		Engine ctx(key, keylen);
		memcpy(k_ipad, ctx.code(), ctx.size());
		memcpy(k_opad, ctx.code(), ctx.size());
	}
	else {

		memcpy(k_ipad, key, keylen);
		memcpy(k_opad, key, keylen);
	}

	for (int i = 0; i < 64; i++) 
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	Engine inner;
	inner.update(reinterpret_cast<const char*>(k_ipad), 64);
	inner.update(data, datalen);
	inner.finish();

	Engine outer;
	outer.update(reinterpret_cast<const char*>(k_opad), 64);
	outer.update(reinterpret_cast<const char*>(inner.code()), inner.size());
	outer.finish();

	return outer;
}

/* --------------------------------------------------------------------- */
//  hmac
/* --------------------------------------------------------------------- */
template <class Engine>
inline Engine hmac( const cString& key, const cString& data ) {
	return hmac<Engine>(key.Cstr(), key.GetLength(), data.Cstr(), data.GetLength());
}

