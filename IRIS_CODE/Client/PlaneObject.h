/* ====================================================================
 * 파    일 :	PlaneObject.h
 * 목    적 :	2D 랜더링 기본 객체
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	항상 같은 Property로 찍게 되는 객체들은 따로 관리하세요
 * =================================================================== */
#pragma once

class NiTexture;
class NiTexturingProperty;
class NiAlphaProperty;
class NiZBufferProperty;

class cPlaneObject
{
public:
	cPlaneObject();
	virtual ~cPlaneObject();

	/// 생성부
	virtual bool Create( const char* pathName, 
				int screenX, int screenY, unsigned int screenW, unsigned int screenH,
				unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom );

	/// 생성부
	virtual bool Create( NiTexture* pTexture, 
				int screenX, int screenY, unsigned int screenW, unsigned int screenH,
				unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom );

	/// 출력부
	virtual void Process( unsigned long accumTime ) {}
	virtual void Draw();

	inline int GetScreenX() { return mScreenX; }
	inline int GetScreenY() { return mScreenY; }
	inline unsigned int GetScreenW() { return mScreenW; }
	inline unsigned int GetScreenH() { return mScreenH; }

	/// 스크린 좌표 셋팅
	void SetScreenXY( int screenX, int screenY );
	void SetScreenX( int screenX );
	void SetScreenY( int screenY );

	/// 스크린 상에서 크기
	void SetScreenWH( unsigned int screenW, unsigned int screenH );
	void SetScreenWidth( unsigned int screenW );
	void SetScreenHeight( unsigned int screenH );

	void SetScreenRect( int screenX, int screenY, unsigned int screenW, unsigned int screenH );

	/// 텍스쳐 UV 셋팅
	virtual void SetTextureRect( unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom );

	/// 정점 컬러 정보 셋팅
	void SetColorA( float r, float g, float b, float a );
	void SetColorA( NiColorA color );

	/// 회전 및 이동
	/// 절대좌표.
	void UpdateImage();
	void SetTranslate( int x, int y );
	void SetRotate( float angle );

	inline NiAlphaProperty* GetAlphaProp() { return mpAlphaProp; }
	inline NiTexturingProperty* GetTexturingProp() { return mpTextureProp; }

	void SetRenderTargetWH( unsigned int width, unsigned int height );

protected:
	///	정점 정보와 관리 정보를 갖는 기본 객체
	NiMeshScreenElementsPtr		mpScreenElement;

	/// 텍스쳐 정보의 변경을 용이하게 하기 위한 참조 및 관리 변수
	NiTexturingProperty*	mpTextureProp;

	/// 알파 블랜딩을 위한 참조 및 관리 변수
	NiAlphaProperty*		mpAlphaProp;

	/// 사용 텍스쳐의 전체 크기
	unsigned int	mTextureWidth;
	unsigned int	mTextureHeight;

	/// 텍스쳐상에서의 위치
	unsigned int	mTexLeft;
	unsigned int	mTexTop;
	unsigned int	mTexRight;
	unsigned int	mTexBottom;

	/// 화면상에서의 위치 및 크기
	int				mScreenX;
	int				mScreenY;
	unsigned int	mScreenW;
	unsigned int	mScreenH;

	unsigned int	mRenderTargetWidth;
	unsigned int	mRenderTargetHeight;

	float mScaleX;
	float mScaleY;

	/// 회전 각도
	float mAngle;
	NiPoint2 mVertex[4];
	NiPoint2 mCenter;
};