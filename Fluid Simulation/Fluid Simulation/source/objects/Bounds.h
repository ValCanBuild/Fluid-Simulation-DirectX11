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
	BOUNDS_TYPE_BOX,
	BOUNDS_TYPE_ORIENTED_BOX
};

class Bounds : public Component{
public:
	Bounds(const GameObject * const gameObject, BoundsType_t boundsType, _In_ Vector3 &center, _In_opt_ Vector3 &extents = Vector3(0,0,0), _In_opt_ float radius = 0.0f);
	~Bounds();

	void UpdateCenter(Vector3 &center) const;

	const Vector3 &GetExtents() const;	// The extents of the box. Always half the size

	// Get bounds as bounding box if applicable
	const DirectX::BoundingBox * const GetBoundingBox() const;

	// Get bounds as bounding sphere if applicable
	const DirectX::BoundingSphere * const GetBoundingSphere() const;
private:
	void *			mBoundingShape;
	BoundsType_t	mBoundsType;
	Vector3			mExtents;
};

#endif