/*************************************************************
Collider Class: Provides a collider component for objects to use.

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _COLLIDER_H
#define _COLLIDER_H

#include "Component.h"
#include "Bounds.h"
#include "RigidBody3D.h"

class btCollisionShape;
class btCollisionObject;
class BaseD3DBody;

class Collider : public Component{
public:
	Collider(const BaseD3DBody * const d3dBody, btCollisionShape * collisionShape);
	~Collider();

	// Return the rigid body associated with this collider
	std::shared_ptr<RigidBody3D> GetAttachedRigidBody() const;

	// Return the collision object associated with this collider
	std::shared_ptr<btCollisionObject> GetCollisionObject() const;	

	void SetEnabled(bool enabled);

	bool IsEnabled();

	void Update();

private:
	bool mEnabled;
	std::shared_ptr<btCollisionObject> mCollisionObject;
};

#endif