/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _COLLISIONMANAGER_H
#define _COLLISIONMANAGER_H

#include <vector>
#include "../objects/BoxCollider.h"
#include "GeometricPrimitive.h"

class BaseD3DBody;

struct ContactResult {
	Vector3 normal; // Normal of the contact point
	Collider *firstCollider;
	Collider *secondCollider;
	Vector3 relativeVelocity;

	Vector3 firstColliderCollisionPoint;
	Vector3 secondColliderCollisionPoint;

	ContactResult() : normal(0.0f), firstCollider(nullptr), secondCollider(nullptr), relativeVelocity(0.0f),
	firstColliderCollisionPoint(0.0f), secondColliderCollisionPoint(0.0f) {

	}
};

class CollisionManager {
public:
	CollisionManager(std::vector<BaseD3DBody*> *sceneObjects);
	~CollisionManager();

	void FixedUpdate(float dt);	// Update at physical timestep

private:
	bool CheckForCollision(BoxCollider * pFirst, BoxCollider * pSecond, _Out_opt_ ContactResult &contactResult);

private:
	const std::vector<BaseD3DBody*> *	pSceneObjects;
	std::vector<ContactResult>			mContactResults; // accumulates contact results each frame
	std::vector<BaseD3DBody> *			pExtraDraw;
	Vector3 *							mVertexListFirst;
	Vector3 *							mVertexListSecond;
};

#endif