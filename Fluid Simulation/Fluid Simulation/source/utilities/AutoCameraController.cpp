/********************************************************************
AutoCameraController: Provides a controller that automatically 
moves a camera along a route of predefined points and predefined
rotations.

Author:	Valentin Hinov
Date: 24/4/2014
*********************************************************************/

#include "AutoCameraController.h"
#include "CameraImpl.h"
#include "math/Interpolate.h"

using std::vector;

static const float RouteRestoreThreshold = 5.0f;
static const float RouteRestoreSpeedFactor = 0.2f;

AutoCameraController::AutoCameraController(CameraImpl &camera) : 
	mCamera(camera), mSpeedFactor(0.05f), mLerpValue(0.0f), mNotOnPath(false), mRouteRestoreLerpValue(0.0f)
{

}

AutoCameraController::~AutoCameraController() {
	mMovementRoute.clear();
	mRotationRoute.clear();
}

void AutoCameraController::Update(float dt) {	
	if (mNotOnPath) {
		RestoreToPath(dt);
	}
	else {
		UpdateOnPath(dt);
	}
}

void AutoCameraController::RestoreToPath(float dt) {
	mRouteRestoreLerpValue += dt*RouteRestoreSpeedFactor;

	Vector3 newPos = Interpolate::Lerp(mRouteRestorePointStart, mMovementRoute[mRouteRestoreIndex], mRouteRestoreLerpValue);
	mCamera.SetPositionVec3(newPos);

	Quaternion newRot = Interpolate::Lerp(mRouteRestoreRotationStart, mRotationRoute[mRouteRestoreIndex], mRouteRestoreLerpValue);
	mCamera.SetRotationQuaternion(newRot);

	if (mRouteRestoreLerpValue > 1.0f) {
		mNotOnPath = false;
	}
}

void AutoCameraController::UpdateOnPath(float dt) {
	int numPos = mMovementRoute.size();
	int numRot = mRotationRoute.size();
	assert(numPos > 2 && numRot > 2 && numPos == numRot);

	mLerpValue += dt*mSpeedFactor;
	if (mLerpValue > 1.0f) {
		mLerpValue = 0.0f;
	}
	Vector3 newPos = Interpolate::NewCatmullRom(mMovementRoute.data(), numPos, 2, mLerpValue, true);
	mCamera.SetPositionVec3(newPos);

	Quaternion newRot = Interpolate::NewCatmullRom(mRotationRoute.data(), numRot, 2, mLerpValue, true);
	mCamera.SetRotationQuaternion(newRot);
}

void AutoCameraController::SetActive(bool state) {
	mNotOnPath = state;
	if (mNotOnPath) {
		mRouteRestoreLerpValue = 0.0f;
		// find the closest point on the route to the current camera position and restore from it
		float minDistance = FLT_MAX;
		mCamera.GetPosition(mRouteRestorePointStart);
		mCamera.GetRotationQuaternion(mRouteRestoreRotationStart);
		size_t numPoints = mMovementRoute.size();
		for (size_t i = 0; i < numPoints; ++i) {
			Vector3 &point = mMovementRoute[i];
			float dist = Vector3::DistanceSquared(mRouteRestorePointStart, point);
			if (dist < minDistance) {
				minDistance = dist;
				mRouteRestoreIndex = i;
			}
		}
		if (minDistance < RouteRestoreThreshold*RouteRestoreThreshold) {
			mNotOnPath = false;
		}
		else {
			mLerpValue = (float)mRouteRestoreIndex/(float)(numPoints - 1);
		}
	}
}

void AutoCameraController::AddRoutePoint(const Vector3& routePoint) {
	mMovementRoute.push_back(routePoint);
}

void AutoCameraController::AddRouteRotation(const Quaternion& routeRotation) {
	mRotationRoute.push_back(routeRotation);
}

void AutoCameraController::ModifySpeedFactor(float speed) {
	SetSpeedFactor(mSpeedFactor + speed);
}

void AutoCameraController::SetSpeedFactor(float speed) {
	mSpeedFactor = speed;
}