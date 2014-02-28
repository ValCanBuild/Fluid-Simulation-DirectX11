/*************************************************************
Bounds Class: Provides a bounds component for objects to use.

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "Bounds.h"
#include "GameObject.h"
#include <memory>

using namespace DirectX;
using namespace std;

Bounds::Bounds(const GameObject * const gameObject, BoundsType_t boundsType) :
Component(gameObject),
mBoundsType(boundsType) 
{
	shared_ptr<Transform> transform = GetGameObject()->transform;
	switch (boundsType) {
	case BOUNDS_TYPE_SPHERE: {
		float maxRadius = max(max(transform->scale.x,transform->scale.y),transform->scale.z);
		BoundingSphere *boundingSphere = new BoundingSphere(transform->position,maxRadius);
		mBoundingShape = boundingSphere;
		mExtents = Vector3(maxRadius);
		mCenter = transform->position;
		break;}							 
	case BOUNDS_TYPE_BOX:{
		BoundingBox *boundingBox = new BoundingBox(transform->position,transform->scale*0.5f);
		mBoundingShape = boundingBox;
		mExtents = transform->scale*0.5f;
		mCenter = transform->position;
		break;}
	}
}

Bounds::~Bounds() {
	if (mBoundingShape) {
		delete mBoundingShape;
		mBoundingShape = nullptr;
	}
}

void Bounds::UpdateCenter(Vector3 &center) {
	switch (mBoundsType) {
	case BOUNDS_TYPE_SPHERE:{
		BoundingSphere *sphere = static_cast<BoundingSphere*>(mBoundingShape);
		sphere->Center = center;
		mCenter = center;
		break;}			 
	case BOUNDS_TYPE_BOX:{
		BoundingBox *box = static_cast<BoundingBox*>(mBoundingShape);
		box->Center = center;
		mCenter = center;
		break;}
	}
}

void Bounds::UpdateExtents(Vector3 &extents) {
	if (mBoundsType == BOUNDS_TYPE_BOX) {
		BoundingBox *box = static_cast<BoundingBox*>(mBoundingShape);
		box->Extents = extents;
	}
}

void Bounds::Update() {
	shared_ptr<Transform> transform = GetGameObject()->transform;
	if (mCenter != transform->position) {
		UpdateCenter(transform->position);
	}
	if (mBoundsType == BOUNDS_TYPE_BOX && mExtents != transform->scale*0.5f) {
		UpdateExtents(transform->scale*0.5f);
	}
}

const Vector3 &Bounds::GetExtents() const {
	return mExtents;
}

const BoundingBox * const Bounds::GetBoundingBox() const {
	assert (mBoundsType == BOUNDS_TYPE_BOX);
	return static_cast<BoundingBox*>(mBoundingShape);
}

const BoundingSphere * const Bounds::GetBoundingSphere() const {
	assert (mBoundsType == BOUNDS_TYPE_SPHERE);
	return static_cast<BoundingSphere*>(mBoundingShape);
}