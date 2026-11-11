/* ====================================================================
* 파    일 :	TexturePalette.h
* 목    적 :	텍스쳐에 대한 공유를 위해서
* 작 성 자 :	이법석
* 작 성 일 :	2006.01
* 주의사항 :	
* =================================================================== */
#pragma once

class cStream;

/// 텍스처 팔레트
class cTexturePalette : public NiDefaultTexturePalette
{
public:
	cTexturePalette( unsigned int sizeHint );
	virtual ~cTexturePalette();

	virtual void SetTexture( const char* pathName, NiTexture* tex );
	virtual NiTexture* GetTexture( const char* pathName, NiTexture* pkSelf ) const;

	/// 제거용 인터페이스
	void RemoveTexture(char* strName);
	void RemoveAllTextures();

	virtual NiTexture* LoadTexture( const char* pathName );
};

NiSmartPointer(cTexturePalette);

