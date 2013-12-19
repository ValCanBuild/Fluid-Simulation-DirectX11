/*************************************************************
Bounds Class: Provides a bounds component for objects to use.

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#ifndef _BOUNDS_H
#define _BOUNDS_H

#include "Component.h"
#include <DirectXCollision.h>
#include "../utilities/math/MathUtils.h"

enum BoundsType_t {
	BOUNDS_TYPE_SPHERE,
	BOUNDS_TYPE_BOX
};

class Bounds : public Component{
public:
	// Bounds information is automatically taken from game object unless provided
	Bounds(const GameObject * const gameObject, BoundsType_t boundsType);
	~Bounds();

	void UpdateCenter(Vector3 &center);
	void UpdateExtents(Vector3 &extents);

	const Vector3 &GetExtents() const;	// The extents of the box. Always half the size

	void Update(); // automatically update bounds information if there has been a change in the parent object

	// Get bounds as bounding box if applicable
	const DirectX::BoundingBox * const GetBoundingBox() const;

	// Get bounds as bounding sphere if applicable
	const DirectX::BoundingSphere * const GetBoundingSphere() const;
private:
	void *			mBoundingShape;
	BoundsType_t	mBoundsType;
	Vector3			mExtents;
	Vector3			mCenter;
};

#endif