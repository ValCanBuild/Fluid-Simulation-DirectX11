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

	const Vector3 &GetCenter() const;
	const Vector3 &GetExtents() const;
	const void	   GetLocalRotationVectors(Vector3 *localRot) const;

	const float   GetRadius() const;

private:
	void CalculateLocalRotation();

private:
	Vector3		&mCenter;   // Reference to body transform position vector
	Quaternion	&mRotation;	// Reference to body transform rotation quaternion
	Vector3		mExtents;	// Positive half-widths along each axis
	Vector3		mLocalRotation[3];			// Local x-y-z Axes
	
	float		mRadius;

	Quaternion	mPrevRotation;
};

#endif