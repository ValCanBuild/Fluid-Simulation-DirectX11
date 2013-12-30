/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _COLLISIONMANAGER_H
#define _COLLISIONMANAGER_H

#include <vector>
#include "../objects/Collider.h"
#include "GeometricPrimitive.h"

class BaseD3DBody;
class btCollisionWorld;

struct ContactResult {
	Vector3 normal; // Normal of the contact point
	std::shared_ptr<RigidBody3D> firstRigidBody;
	std::shared_ptr<RigidBody3D> secondRigidBody;
	Vector3 relativeVelocity;

	Vector3 firstColliderCollisionPoint;
	Vector3 secondColliderCollisionPoint;

	ContactResult() : normal(0.0f), firstRigidBody(nullptr), secondRigidBody(nullptr), relativeVelocity(0.0f),
	firstColliderCollisionPoint(0.0f), secondColliderCollisionPoint(0.0f) {

	}
};

class CollisionManager {
public:
	CollisionManager(std::vector<BaseD3DBody*> *sceneObjects);
	~CollisionManager();

	void AddObject(BaseD3DBody* sceneObject);
	void FixedUpdate(float dt);	// Update at physical timestep

private:
	float HandleContactResult(ContactResult &contact, float dt, float penetration) const;	// returns applied impulse

private:
	std::shared_ptr<btCollisionWorld>	mCollisionWorld;
	//std::vector<ContactResult>			mContactResults; // accumulates contact results each frame
	std::vector<BaseD3DBody> *			pExtraDraw;
};

#endif