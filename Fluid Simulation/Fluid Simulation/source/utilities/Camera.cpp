/***************************************************************
Camera.cpp: Camera class to be used to obtain a view matrix

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "Camera.h"

Camera::Camera() :
mDefaultUp(0,1,0), mDefaultLookAt(0,0,1), mDefaultRight(1,0,0) {
	mYaw = mPitch = mRoll = 0.0f;
	mPosition = Vector3(0,0,0);

	mHasChanged = true;// set to true so the camera gets updated the first time update loop is called
}

Camera::~Camera() {

}

void Camera::Update() {
	// don't compute anything if no attributes have changed
	if (mHasChanged) {
		mHasChanged = false;
	}	
	else {
		return;
	}

	Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(mYaw, mPitch, 0.0f);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	Vector3::TransformNormal(mDefaultLookAt,rotationMatrix,mLookAt);
	Vector3::TransformNormal(mDefaultUp,rotationMatrix,mUp);
	//D3DXVec3TransformNormal(&mLookAt, &mDefaultLookAt, &rotationMatrix);
	//D3DXVec3TransformNormal(&mUp, &mDefaultUp, &rotationMatrix);

	// Transform the right vector by the yaw matrix
	Matrix yawMatrix = Matrix::CreateRotationY(mYaw);
	Vector3::TransformNormal(mDefaultRight,yawMatrix,mRight);
	//D3DXMATRIX yawMatrix;
	//D3DXMatrixRotationY(&yawMatrix, mYaw);
	//D3DXVec3TransformNormal(&mRight, &mDefaultRight, &yawMatrix);	

	// update the target
	mTarget = mPosition + mLookAt;

	// Finally create the view matrix from the three updated vectors.
	mViewMatrix = Matrix::CreateLookAt(mPosition, mLookAt, mUp);
	//D3DXMatrixLookAtLH(&mViewMatrix, &mPosition, &mLookAt, &mUp);
}

void Camera::MoveFacing(float forwardAmount, float rightAmount) {
	mPosition += forwardAmount*mLookAt;
	mPosition += rightAmount*mRight;
	mHasChanged = true;
}

void Camera::AddYawPitchRoll(float yaw, float pitch, float roll) {
	mYaw += yaw;
	mPitch += pitch;
	mRoll += roll;
	mHasChanged = true;
}

void Camera::SetYawPitchRoll(float yaw, float pitch, float roll) {
	mYaw = yaw;
	mPitch = pitch;
	mRoll = roll;
	mHasChanged = true;
}

void Camera::AddPosition(float x, float y, float z) {
	mPosition += Vector3(x,y,z);
	mHasChanged = true;
}

void Camera::SetPosition(float x, float y, float z) {
	mPosition = Vector3(x,y,z);
	mHasChanged = true;
}

void Camera::GetPosition(Vector3& pos) const {
	pos = mPosition;
}

void Camera::GetTarget(Vector3& target) const {
	target = mTarget;
}

void Camera::GetViewMatrix(Matrix& viewMatrix) const {
	viewMatrix = mViewMatrix;
}