/*************************************************************
Bounds Class: Provides a bounds component for objects to use.

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "Bounds.h"

using namespace DirectX;

Bounds::Bounds(const GameObject * const gameObject, BoundsType_t boundsType, _In_ Vector3 &center, _In_opt_ Vector3 &extents, _In_opt_ float radius) :
Component(gameObject),
mBoundsType(boundsType) 
{
	switch (boundsType) {
	case BOUNDS_TYPE_SPHERE: {
		assert(radius > 0.0f);
		BoundingSphere *boundingSphere = new BoundingSphere(center,radius);
		mBoundingShape = boundingSphere;
		mExtents = Vector3(radius);
		break;}							 
	case BOUNDS_TYPE_BOX:{
		BoundingBox *boundingBox = new BoundingBox(center,extents);
		mBoundingShape = boundingBox;
		mExtents = extents;
		break;}
	case BOUNDS_TYPE_ORIENTED_BOX:{
		mBoundingShape = nullptr;
		mExtents = Vector3(0.0f);
		break;}
	}
}

Bounds::~Bounds() {
	if (mBoundingShape) {
		delete mBoundingShape;
		mBoundingShape = nullptr;
	}
}

void Bounds::UpdateCenter(Vector3 &center) const {
	switch (mBoundsType) {
	case BOUNDS_TYPE_SPHERE:{
		BoundingSphere *sphere = static_cast<BoundingSphere*>(mBoundingShape);
		sphere->Center = center;
		break;}			 
	case BOUNDS_TYPE_BOX:{
		BoundingBox *box = static_cast<BoundingBox*>(mBoundingShape);
		box->Center = center;
		break;}
	case BOUNDS_TYPE_ORIENTED_BOX:{
		break;}
	}
}

const Vector3 &Bounds::GetExtents() const {
	return mExtents;
}

const BoundingBox * const Bounds::GetBoundingBox() const {
	if (mBoundsType != BOUNDS_TYPE_BOX) {
		return nullptr;
	}
	return static_cast<BoundingBox*>(mBoundingShape);
}

const BoundingSphere * const Bounds::GetBoundingSphere() const {
	if (mBoundsType != BOUNDS_TYPE_SPHERE) {
		return nullptr;
	}
	return static_cast<BoundingSphere*>(mBoundingShape);
}