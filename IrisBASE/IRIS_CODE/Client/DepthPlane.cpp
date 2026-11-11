#include "stdafx.h"
#include "DepthPlane.h"

///
void cPlane::WorldPtToScreenPt( NiCamera* pCam, const NiPoint3& pt, float& posX, float& posY, bool InScreen )
{
	assert(pCam);
	const float* mat = pCam->GetWorldToCameraMatrix();
	NiRect<float> port = pCam->GetViewPort();

	// project a world space point to screen space
	float fW = pt.x * (*(mat + (4*3))) + pt.y * (*(mat + (4*3+1))) 
		+ pt.z * (*(mat + (4*3+2))) + (*(mat + (4*3+3)));

	if( fW > 1e-5f )
	{
		float fInvW = 1.0f / fW;

		posX = pt.x * (*mat) + pt.y * (*(mat + 1))
			+ pt.z * (*(mat + 2)) + (*(mat + 3));
		posY = pt.x * (*(mat + (4*1+0))) + pt.y * (*(mat + (4*1+1)))
			+ pt.z * (*(mat + (4*1+2))) + (*(mat + (4*1+3)));

		posX = posX * fInvW;
		posY = posY * fInvW;

		if( InScreen )
		{
			/// 스크린상에서 벗어나는 경우 보정
			if( posX < -1.0f ) posX = -1.0f;
			if( posY < -1.0f ) posY = -1.0f;

			if( posX > 1.0f ) posX = 1.0f;
			if( posY > 1.0f ) posY = 1.0f;
		}

		posX *= (port.m_right - port.m_left) * 0.5f;
		posY *= (port.m_top - port.m_bottom) * 0.5f;

		posX += (port.m_right + port.m_left) * 0.5f;
		posY += (port.m_top + port.m_bottom) * 0.5f;
	}
	else
	{
//		assert(0);
		return;
	}
}