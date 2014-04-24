/***************************************************************
Camera.h: Camera implementation

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _CAMERAIMPL_H
#define _CAMERAIMPL_H

#include "ICamera.h"
#include <memory>

class CameraImpl : public ICamera {
public:
	~CameraImpl();

	static std::unique_ptr<CameraImpl>	CreateCameraLH(float fieldOfView, float screenAspect, float screenNear, float screenFar);
	static std::unique_ptr<CameraImpl>	CreateCameraRH(float fieldOfView, float screenAspect, float screenNear, float screenFar);

	void Update();

	void AddYawPitchRoll(float yaw, float pitch, float roll);
	void SetYawPitchRoll(float yaw, float pitch, float roll);
	void SetRotationQuaternion(const Quaternion &quaternion);

	void AddPosition(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void SetPositionVec3(const Vector3& pos);

	void MoveFacing(float forwardAmount, float rightAmount);

	void GetPosition(Vector3& pos) const;
	void GetTarget(Vector3& target) const;
	void GetRotationQuaternion(Quaternion &quaternion);

	const Matrix &GetProjectionMatrix() const;
	const Matrix &GetViewMatrix() const;
	const Matrix &GetViewProjectionMatrix() const;
	float GetFieldOfView() const;
	void GetProjectionMatrix(Matrix& projMatrix) const;
	void GetViewMatrix(Matrix& viewMatrix) const;
	void GetViewProjectionMatrix(Matrix& viewProjMatrix) const;
	void GetRotationMatrix(Matrix& rotationMatrix) const;

	Ray  ScreenPointToRay(Vector2 position) const;

	// works only for Left handed coordinate systems
	const DirectX::BoundingFrustum &GetBoundingFrustum() const;	

private:
	CameraImpl(float fieldOfView, float screenAspect, float screenNear, float screenFar, bool rightHand);

private:
	Matrix mProjectionMatrix;
	Matrix mViewMatrix;
	Matrix mViewProjectionMatrix;

	DirectX::BoundingFrustum mBoundingFrustum;
	DirectX::BoundingFrustum mUntransformedFrustum;

	Matrix mRotationMatrix;
	Quaternion mRotationQuaternion;

	float mFieldOfView;
	float mYaw, mPitch, mRoll;
	Vector3 mPosition, mLookAt, mUp, mRight;
	const Vector3 mDefaultUp, mDefaultLookAt, mDefaultRight;
	Vector3 mTarget;

	bool mHasChanged;	//dirty flag to set when there has been a camera change
	bool mRightHanded;	// flag to specify the coordinate orientation of the camera coordinate system
};

#endif