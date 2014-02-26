/***************************************************************
Camera.cpp: Camera class to be used to obtain a view matrix

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "Camera.h"
#include "../system/ServiceProvider.h"
#include "../display/IGraphicsObject.h"

Camera::Camera() :
mDefaultUp(0,1,0), mDefaultLookAt(0,0,1), mDefaultRight(1,0,0), mPosition(0,0,0) {
	mYaw = mPitch = mRoll = 0.0f;

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

	mRotationMatrix = Matrix::CreateFromYawPitchRoll(mYaw, mPitch, 0.0f);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	Vector3::TransformNormal(mDefaultLookAt,mRotationMatrix,mLookAt);
	Vector3::TransformNormal(mDefaultUp,mRotationMatrix,mUp);

	// Transform the right vector by the yaw matrix
	Matrix yawMatrix = Matrix::CreateRotationY(mYaw);
	Vector3::TransformNormal(mDefaultRight,yawMatrix,mRight);

	// update the target
	mTarget = mPosition + mLookAt;

	// Finally create the view matrix from the three updated vectors.
	mViewMatrix = Matrix::CreateLookAt(mPosition, mTarget, mUp);

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

Ray Camera::ScreenPointToRay(Vector2 position) const {
	const IGraphicsObject *graphicsObject = ServiceProvider::Instance().GetGraphicsSystem()->GetGraphicsObject();
	int screenWidth,screenHeight;
	graphicsObject->GetScreenDimensions(screenWidth,screenHeight);

	Matrix projectionMatrix;
	graphicsObject->GetProjectionMatrix(projectionMatrix);

	float vx = (+2.0f*position.x/screenWidth  - 1.0f)/projectionMatrix(0,0);
	float vy = (-2.0f*position.y/screenHeight + 1.0f)/projectionMatrix(1,1);

	Vector3 rayOrigin(0.0f);
	Vector3 rayDir(vx,vy,-1.0f);

	Matrix viewInverse;
	mViewMatrix.Invert(viewInverse);

	rayOrigin = Vector3::Transform(rayOrigin,viewInverse);
	rayDir = Vector3::TransformNormal(rayDir,viewInverse);
	rayDir.Normalize();

	return Ray(rayOrigin,rayDir);
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

void Camera::GetRotationMatrix(Matrix& rotationMatrix) const {
	rotationMatrix = mRotationMatrix;
}