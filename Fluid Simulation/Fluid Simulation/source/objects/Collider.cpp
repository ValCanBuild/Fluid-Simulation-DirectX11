/*************************************************************
Collider Class: Provides a collider component for objects to use.

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include "Collider.h"
#include "BaseD3DBody.h"
#include "BulletCollision\CollisionDispatch\btCollisionObject.h"
#include "BulletCollision\CollisionShapes\btCollisionShape.h"

Collider::Collider(const BaseD3DBody * const d3dBody, btCollisionShape * collisionShape) : Component(d3dBody), mEnabled(true) {
	mCollisionObject = std::shared_ptr<btCollisionObject>(new btCollisionObject());
	mCollisionObject->setCollisionShape(collisionShape);
	mCollisionObject->setUserPointer(this);	// user pointer of collision object will point to this collider
}

Collider::~Collider() {

}

void Collider::Update() {
	btTransform &collTransform = mCollisionObject->getWorldTransform();
	std::shared_ptr<Transform> transform = GetGameObject()->transform;
	collTransform.setOrigin(Vector3ToBtVector3(transform->position));
	collTransform.setRotation(QuaternionToBtQuaternion(transform->qRotation));
	mCollisionObject->getCollisionShape()->setLocalScaling(Vector3ToBtVector3(transform->scale));
}

std::shared_ptr<RigidBody3D> Collider::GetAttachedRigidBody() const{
	return static_cast<const BaseD3DBody* const>(GetGameObject())->rigidBody3D;
}

std::shared_ptr<btCollisionObject> Collider::GetCollisionObject() const{
	return mCollisionObject;
}

void Collider::SetEnabled(bool enabled) {
	mEnabled = enabled;
}

bool Collider::IsEnabled() {
	return mEnabled;
}