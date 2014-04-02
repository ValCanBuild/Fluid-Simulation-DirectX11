/*************************************************************
MathUtils.h: Contains various math utility functions

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#ifndef _MATHUTILS_H
#define _MATHUTILS_H

#include <time.h>
#include <windows.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;


//*****************************************************************************
// Constants
//*****************************************************************************

const float INFINITY = 3.402823466e+38F;
const float PI       = 3.14159265358979323f;
const float MATH_EPS = 0.0001f;

// Converts ARGB 32-bit color format to ABGR 32-bit color format.
inline UINT ARGB2ABGR(UINT argb) {
	BYTE A = (argb >> 24) & 0xff;
	BYTE R = (argb >> 16) & 0xff;
	BYTE G = (argb >>  8) & 0xff;
	BYTE B = (argb >>  0) & 0xff;

	return (A << 24) | (B << 16) | (G << 8) | (R << 0);
}

inline Color RGBA2Color(UINT r, UINT g, UINT b, UINT a) {
	return Color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
}

inline float Deg2Rad(float deg) {
	return deg * PI / 180;
}

// Returns random float in [0, 1).
inline float RandF() {
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
inline float RandF(float a, float b) {
	return a + RandF()*(b-a);
} 

inline Color RandomColor(bool randAlpha = false) {
	return randAlpha ? Color(RandF(),RandF(),RandF(),RandF()) : Color(RandF(),RandF(),RandF());
}
 
template<typename T>
inline T Min(const T& a, const T& b) {
	return a < b ? a : b;
}

template<typename T>
inline T Max(const T& a, const T& b) {
	return a > b ? a : b;
}
 
template<typename T>
inline T Lerp(const T& a, const T& b, float t) {
	return a + (b-a)*t;
}

template<typename T>
inline T Clamp(const T& x, const T& low, const T& high) {
	return x < low ? low : (x > high ? high : x); 
}

template <typename T, typename U> 
inline U MapValue(T  x, T in_min, T in_max, U out_min, U out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif