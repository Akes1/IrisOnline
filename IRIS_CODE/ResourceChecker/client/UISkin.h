/* ==========================================================================
* 파    일 : UISkin.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.11
* 주의사항 : 
*===========================================================================*/
#pragma once

class cUINodeSkin;
class cParser;
class cBitArray;

/// 스킨
class cUISkin
{
public:
	cUISkin();
	~cUISkin();

	/// 지우기
	void Clear();

	/// 로딩
	bool Load( const cString& pathName );
	
	/// 재로딩( 스킨만 바꾸기 시 사용 )
	bool Reload( const cString& pathName );

	/// 텍스처
	NiTexture* GetTexture( unsigned int id );

	/// 알파비트 배열
	cBitArray* GetAlphaBitArray( unsigned int id );

	/// 노드 스킨
	cUINodeSkin* GetNodeSkin( const cString& name );

protected:
	bool LoadImages( const cString& path, cParser& mParser );
	bool LoadScripts( const cString& path, cParser& mParser );
	bool LoadScript( const cString& pathName );

	bool ReloadScripts( const cString& path, cParser& mParser );
	bool ReloadScript( const cString& pathName );

private:
	/// 텍스쳐 맵
	typedef tPointerHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mTextureMap;

	/// 알파 비트 배열 맵
	typedef tPointerHashMap<unsigned int, void*> cBitArrayMap;
	cBitArrayMap mAlphaBitArrayMap;

	/// 노드 스킨 맵
	typedef tPointerHashMap<cString, void*> cNodeSkinMap;
	cNodeSkinMap mNodeSkinMap;
};

inline NiTexture* cUISkin::GetTexture( unsigned int id )
{
	return (NiTexture*)mTextureMap.GetAt( id );
}

inline cBitArray* cUISkin::GetAlphaBitArray( unsigned int id )
{
	return (cBitArray*)mAlphaBitArrayMap.GetAt( id );
}

inline cUINodeSkin* cUISkin::GetNodeSkin( const cString& name )
{
	return (cUINodeSkin*)mNodeSkinMap.GetAt( name );
}