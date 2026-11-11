// EMERGENT GAME TECHNOLOGIES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Emergent Game Technologies and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2006 Emergent Game Technologies.
//      All Rights Reserved.
//
// Emergent Game Technologies, Chapel Hill, North Carolina 27517
// http://www.emergent.net

#pragma once

#include <math.h>

class NiPoint2
{
public:
	float x, y;

	NiPoint2 () { /**/ }
	NiPoint2 (float fX, float fY);

	inline float& operator[] (int i)
	{
		// warning:  safe for 4-byte aligned class member data
		const float* base = &x;
		return (float&) base[i];
	}

	inline const float& operator[] (int i) const
	{
		// warning:  safe for 4-byte aligned class member data
		const float* base = &x;
		return (float&) base[i];
	}

	inline bool operator== (const NiPoint2& pt) const;
	inline bool operator!= (const NiPoint2& pt) const;

	inline NiPoint2 operator+ (const NiPoint2& pt) const;
	inline NiPoint2 operator- (const NiPoint2& pt) const;
	inline float operator* (const NiPoint2& pt) const;
	inline NiPoint2 operator* (float fScalar) const;
	inline NiPoint2 operator/ (float fScalar) const;
	inline NiPoint2 operator- () const;
	inline friend NiPoint2 operator* (float fScalar, const NiPoint2& pt);
	static NiPoint2 ComponentProduct (const NiPoint2& p0, const NiPoint2& p1);

	inline NiPoint2& operator+= (const NiPoint2& pt);
	inline NiPoint2& operator-= (const NiPoint2& pt);
	inline NiPoint2& operator*= (float fScalar);
	inline NiPoint2& operator/= (float fScalar);

	inline float Length () const;
	inline float SqrLength () const;
	inline float Dot (const NiPoint2& pt) const;
	inline float Unitize ();

	static const NiPoint2 ZERO;
	static const NiPoint2 UNIT_X;
	static const NiPoint2 UNIT_Y;
};

//---------------------------------------------------------------------------
//  Inline include
#include "NiPoint2.inl"

//---------------------------------------------------------------------------