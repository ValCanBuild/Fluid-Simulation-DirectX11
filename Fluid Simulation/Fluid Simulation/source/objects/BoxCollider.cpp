/*************************************************************
BoxCollider Class: Provides a oriented bounding box collider
component for game objects

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include "BoxCollider.h"
#include "BaseD3DBody.h"

using namespace DirectX;
using namespace std;

BoxCollider::BoxCollider(const BaseD3DBody * const d3dBody) : Collider(d3dBody) {
	shared_ptr<Transform> transform = GetGameObject()->transform;
	mCenter = transform->position;
	mExtents = transform->scale*0.5f;
	mRotation = transform->qRotation;
	mBoundingOrientedBox = shared_ptr<BoundingOrientedBox>(new BoundingOrientedBox(mCenter,mExtents,mRotation));
}

BoxCollider::~BoxCollider() {
}

void BoxCollider::UpdateExtents(Vector3 &extents) {
	mBoundingOrientedBox->Extents = extents;
}

// automatically update collider information if there has been a change in the parent object
void BoxCollider::Update() {
	if (IsEnabled()) {
		shared_ptr<Transform> transform = GetGameObject()->transform;
		if (mCenter != transform->position) {
			mCenter = transform->position;
			mBoundingOrientedBox->Center = mCenter;
		}
		if (mRotation != transform->qRotation) {
			mRotation = transform->qRotation;
			mBoundingOrientedBox->Orientation = mRotation;
		}
	}
}

const Vector3 &BoxCollider::GetCenter() const {
	return mCenter;
}

// get's the oriented bounding box as it was after the last update or reupdates if required
shared_ptr<BoundingOrientedBox> BoxCollider::GetOrientedBoundingBox(bool forceUpdate) {
	if (forceUpdate) {
		Update();
	}
	return mBoundingOrientedBox;
}