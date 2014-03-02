/***************************************************************
Camera.h: Camera class to be used to obtain a view matrix

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _CAMERA_H
#define _CAMERA_H

#include <memory>
#include "../utilities/D3dIncludes.h"
#include "../utilities/math/MathUtils.h"

class Camera {
public:
	~Camera();

	static std::unique_ptr<Camera>	CreateCameraLH(float fieldOfView, float screenAspect, float screenNear, float screenFar);
	static std::unique_ptr<Camera>	CreateCameraRH(float fieldOfView, float screenAspect, float screenNear, float screenFar);

	void Update();

	void AddYawPitchRoll(float yaw, float pitch, float roll);
	void SetYawPitchRoll(float yaw, float pitch, float roll);

	void AddPosition(float x, float y, float z);
	void SetPosition(float x, float y, float z);

	void MoveFacing(float forwardAmount, float rightAmount);

	void GetPosition(Vector3& pos) const;
	void GetTarget(Vector3& target) const;

	void GetProjectionMatrix(Matrix& projMatrix) const;
	void GetViewMatrix(Matrix& viewMatrix) const;
	void GetRotationMatrix(Matrix& rotationMatrix) const;

	Ray  ScreenPointToRay(Vector2 position) const;

	// works only for Left handed coordinate systems
	DirectX::BoundingFrustum * GetBoundingFrustum();	

private:
	Camera(float fieldOfView, float screenAspect, float screenNear, float screenFar, bool rightHand);

private:
	Matrix mProjectionMatrix;
	Matrix mViewMatrix;

	DirectX::BoundingFrustum mBoundingFrustum;
	DirectX::BoundingFrustum mUntransformedFrustum;

	Matrix mRotationMatrix;

	float mYaw, mPitch, mRoll;
	Vector3 mPosition, mLookAt, mUp, mRight;
	const Vector3 mDefaultUp, mDefaultLookAt, mDefaultRight;
	Vector3 mTarget;

	bool mHasChanged;	//dirty flag to set when there has been a camera change
	bool mRightHanded;	// flag to specify the coordinate orientation of the camera coordinate system
};

#endif