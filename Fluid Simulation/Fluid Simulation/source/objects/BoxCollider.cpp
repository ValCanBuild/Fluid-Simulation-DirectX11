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

BoxCollider::BoxCollider(const BaseD3DBody * const d3dBody) : Collider(d3dBody), mRotation(GetGameObject()->transform->qRotation),
	mCenter(GetGameObject()->transform->position) {

	shared_ptr<Transform> transform = GetGameObject()->transform;
	mExtents = transform->scale*0.5f;

	mRadius = sqrtf(mExtents.x*mExtents.x + mExtents.y*mExtents.y + mExtents.z*mExtents.z) + 0.1f;

	CalculateLocalRotation();
}

BoxCollider::~BoxCollider() {
}

// automatically update collider information if there has been a change in the parent object
void BoxCollider::Update() {
	if (IsEnabled()) {
		// If object has rotated, update local rotation angles
		if (mPrevRotation != mRotation) {
			CalculateLocalRotation();
			mPrevRotation = mRotation;
		}
		shared_ptr<Transform> transform = GetGameObject()->transform;
		if (mExtents != transform->scale*0.5f) {
			mExtents = transform->scale*0.5f;
			mRadius = sqrtf(mExtents.x*mExtents.x + mExtents.y*mExtents.y + mExtents.z*mExtents.z) + 0.1f;
		}
	}
}

void BoxCollider::CalculateLocalRotation() {
	Matrix rotMatrix = Matrix::CreateFromQuaternion(mRotation);
	mLocalRotation[0] = Vector3(1.0f,0.0f,0.0f);
	mLocalRotation[1] = Vector3(0.0f,1.0f,0.0f);
	mLocalRotation[2] = Vector3(0.0f,0.0f,1.0f);

	Vector3::Transform(mLocalRotation,3,rotMatrix,mLocalRotation);
}


const Vector3 &BoxCollider::GetCenter() const {
	return mCenter;
}

const Vector3 &BoxCollider::GetExtents() const {
	return mExtents;
}

const float BoxCollider::GetRadius() const {
	return mRadius;
}

const void BoxCollider::GetLocalRotationVectors(Vector3 *localRot) const {
	localRot[0] = mLocalRotation[0];
	localRot[1] = mLocalRotation[1];
	localRot[2] = mLocalRotation[2];
}