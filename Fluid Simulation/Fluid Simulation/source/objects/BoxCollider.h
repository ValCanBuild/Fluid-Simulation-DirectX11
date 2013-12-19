/*************************************************************
BoxCollider Class: Provides a oriented bounding box collider
component for game objects

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _BOXCOLLIDER_H
#define _BOXCOLLIDER_H

#include "Collider.h"

#include <DirectXCollision.h>

class BoxCollider : public Collider {
public:
	BoxCollider(const BaseD3DBody * const d3dBody);
	~BoxCollider();

	void Update(); // automatically update collider information if there has been a change in the parent object

	void UpdateExtents(Vector3 &extents);

	// get's the oriented bounding box as it was after the last update or reupdates if required
	std::shared_ptr<DirectX::BoundingOrientedBox> GetOrientedBoundingBox(bool forceUpdate = false); 
	const Vector3 &GetCenter() const;

private:
	std::shared_ptr<DirectX::BoundingOrientedBox> mBoundingOrientedBox;
	Vector3		mCenter;
	Vector3		mExtents;
	Quaternion	mRotation;
};

#endif