/***************************************************************
Camera.h: Camera class to be used to obtain a view matrix

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _CAMERA_H
#define _CAMERA_H

#include "../utilities/D3dIncludes.h"

class Camera {
public:
	Camera();
	~Camera();

	void Update();

	void AddYawPitchRoll(float yaw, float pitch, float roll);
	void SetYawPitchRoll(float yaw, float pitch, float roll);

	void AddPosition(float x, float y, float z);
	void SetPosition(float x, float y, float z);

	void MoveFacing(float forwardAmount, float rightAmount);

	void GetPosition(Vector3f& pos) const;
	void GetTarget(Vector3f& target) const;

	void GetViewMatrix(D3DXMATRIX& viewMatrix) const;

private:
	D3DXMATRIX mViewMatrix;
	float mYaw,mPitch,mRoll;
	Vector3f mPosition, mLookAt, mUp, mRight;
	const Vector3f mDefaultUp, mDefaultLookAt, mDefaultRight;
	Vector3f mTarget;

	bool mHasChanged;	//dirty flag to set when there has been a camera change
};

#endif