/* ====================================================================
* 파    일 :	NameCardGauge.h
* 목    적 :	이름표에 표시될 게이지 표현
* 작 성 자 :	이법석
* 작 성 일 :	2007.03.22
* 주의사항 :	
* =================================================================== */
#pragma once

class cPlaneObject;

class cNameCardGauge
{
public:
	cNameCardGauge();
	~cNameCardGauge();

	bool Init( NiTexture* pGaugeFile, RECT* GaugeUV, unsigned short GaugeTexW, unsigned short GaugeTexH, NiTexture* pFrameFile = 0, RECT* FrameUV = 0, RECT* FrameBack = 0 );

	void SetGaugePos( short posx, short posy );

	void SetGaugeWidth( unsigned short width );

	void PreDraw();
	void Draw();

protected:

	NiRenderedTexturePtr	mpRenderedTexture;
	NiRenderTargetGroupPtr	mpRenderTargetGroup;

	cPlaneObject*			mpGaugeTarget;

	/// 내용
	cPlaneObject*			mpGaugeTexture;
	cPlaneObject*			mpFrameTexture;
	cPlaneObject*			mpFrameBackTexture;
};
