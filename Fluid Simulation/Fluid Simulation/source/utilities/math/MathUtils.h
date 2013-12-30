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
#include "LinearMath\btVector3.h"
#include "LinearMath\btQuaternion.h"

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

// Returns random float in [0, 1).
inline float RandF() {
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
inline float RandF(float a, float b) {
	return a + RandF()*(b-a);
} 

inline Vector3 AbsVector(Vector3 &vector) {
	return Vector3(abs(vector.x),abs(vector.y),abs(vector.z));
}

inline Color RandomColor(bool randAlpha = false) {
	return randAlpha ? Color(RandF(),RandF(),RandF(),RandF()) : Color(RandF(),RandF(),RandF());
}

inline Vector3 BtVector3ToVector3(btVector3 &btVector) {
	return Vector3(btVector[0],btVector[1],btVector[2]);
}

inline btVector3 Vector3ToBtVector3(Vector3 &vector3) {
	return btVector3(vector3.x,vector3.y,vector3.z);
}

inline btQuaternion QuaternionToBtQuaternion(Quaternion &quaternion) {
	return btQuaternion(quaternion.x,quaternion.y,quaternion.z,quaternion.w);
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
inline T MapValue(T  x, T in_min, T in_max, U out_min, U out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif