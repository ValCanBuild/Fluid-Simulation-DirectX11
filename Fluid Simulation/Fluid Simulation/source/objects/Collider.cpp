/*************************************************************
Collider Class: Provides a collider component for objects to use.

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include "Collider.h"
#include "BaseD3DBody.h"

Collider::Collider(const BaseD3DBody * const d3dBody) : Component(d3dBody), mEnabled(true) {

}

Collider::~Collider() {

}

std::shared_ptr<RigidBody3D> Collider::GetAttachedRigidBody() const{
	return static_cast<const BaseD3DBody* const>(GetGameObject())->rigidBody3D;
}

void Collider::SetEnabled(bool enabled) {
	mEnabled = enabled;
}

bool Collider::IsEnabled() {
	return mEnabled;
}