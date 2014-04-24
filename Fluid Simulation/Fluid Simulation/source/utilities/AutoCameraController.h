/********************************************************************
AutoCameraController: Provides a controller that automatically 
moves a camera along a route of predefined points and predefined
rotations.

Author:	Valentin Hinov
Date: 24/4/2014
*********************************************************************/
#ifndef _AUTOCAMERACONTROLLER_H
#define _AUTOCAMERACONTROLLER_H

#include <vector>
#include "math/MathUtils.h"

class CameraImpl;

class AutoCameraController {
public:
	AutoCameraController(CameraImpl &camera);
	~AutoCameraController();

	// moves and rotates the camera based on the path values
	// does not call CameraImpl.Update()
	void Update(float dt);

	void SetActive(bool state);

	void AddRoutePoint(const Vector3& routePoint);
	void AddRouteRotation(const Quaternion& routeRotation);

	void SetSpeedFactor(float speed);
	void ModifySpeedFactor(float speed);

private:
	void RestoreToPath(float dt);
	void UpdateOnPath(float dt);

private:
	// this is the camera that this controller will handle
	CameraImpl &mCamera; 
	std::vector<Vector3> mMovementRoute;
	std::vector<Quaternion> mRotationRoute;
	float mSpeedFactor;
	float mLerpValue;

	bool mNotOnPath;
	Vector3 mRouteRestorePointStart;
	Quaternion mRouteRestoreRotationStart;
	UINT mRouteRestoreIndex;
	float mRouteRestoreLerpValue;
};


#endif 
