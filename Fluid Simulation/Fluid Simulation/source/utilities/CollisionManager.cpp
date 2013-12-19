/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include "CollisionManager.h"
#include "../objects/BaseD3DBody.h"
#include "Physics.h"

using namespace DirectX;
using namespace std;

CollisionManager::CollisionManager(vector<BaseD3DBody*> *sceneObjects ) : pSceneObjects(sceneObjects) {
	mVertexListFirst = new Vector3[8];
	mVertexListSecond = new Vector3[8];
}

CollisionManager::~CollisionManager() {
	pSceneObjects = nullptr;
	if (mVertexListFirst) {
		delete [] mVertexListFirst;
		mVertexListFirst = nullptr;
	}
	if (mVertexListSecond) {
		delete [] mVertexListSecond;
		mVertexListSecond = nullptr;
	}
}

void CollisionManager::FixedUpdate(float dt) {
	// remove old contact results
	mContactResults.clear();

	for (BaseD3DBody * body : *pSceneObjects) {
		shared_ptr<BoxCollider> firstCollider = body->boxCollider;
		if (firstCollider == nullptr) {
			continue;
		}
		// only check for collision if collider has a rigid body and it is enabled is awake and collider is enabled
		if (firstCollider->IsEnabled() && firstCollider->GetAttachedRigidBody() != nullptr) {			
			// check against other enabled colliders - they don't need to have rigid bodies
			for (BaseD3DBody * otherBody : *pSceneObjects) {
				if (body == otherBody) {
					continue;
				}
				shared_ptr<BoxCollider> secondCollider = otherBody->boxCollider;
				if (secondCollider == nullptr) {
					continue;
				}
				if (secondCollider->IsEnabled()) {
					ContactResult contactResult;
					if (CheckForCollision(firstCollider.get(),secondCollider.get(),contactResult)) {
						mContactResults.push_back(contactResult);
					}
				}
			}
		}
	}

	// handle any contact results
	for (ContactResult contact : mContactResults) {
		shared_ptr<RigidBody3D> rigidBody1 = contact.firstCollider->GetAttachedRigidBody(); // First collider definetly has a rigid body
		shared_ptr<RigidBody3D> rigidBody2 = contact.secondCollider->GetAttachedRigidBody(); // Second collider may or may not have a rigid body
		
		// calculate impulse 
		float fCr = Physics::fRestitution;
		float vrn = contact.relativeVelocity.Dot(contact.normal);
		float mass1 = rigidBody1->GetMass();
		float mass2 = rigidBody2 != nullptr ? rigidBody2->GetMass() : 0.0f;
		float massFactorCombined = 1.0f/mass1;
		if (mass2 > 0.0f) {
			massFactorCombined += 1.0f/mass2;
		}

		float body1CollPointFactor = contact.normal.Dot(
									  ((contact.firstColliderCollisionPoint.Cross(contact.normal)) 
									  / 
									  rigidBody1->GetInertiaTensor()).Cross(contact.firstColliderCollisionPoint) );

		float body2CollPointFactor = 0.0f;
		if (rigidBody2 != nullptr) {
			body2CollPointFactor = contact.normal.Dot(
									((contact.secondColliderCollisionPoint.Cross(contact.normal)) 
									 / 
									 rigidBody2->GetInertiaTensor()).Cross(contact.secondColliderCollisionPoint) );
		}

		float impulse = (-(1.0f + fCr) * (vrn) /
						( (massFactorCombined) +
						body1CollPointFactor + body2CollPointFactor));

		Vector3 impulseAlongNormal = contact.normal * impulse;

		Vector3 linearForceBody1 = impulseAlongNormal * mass1;
		Vector3 angularForceBody1 = contact.firstColliderCollisionPoint.Cross(impulseAlongNormal) * rigidBody1->GetInertiaTensor();
		// Add linear force from impact
		rigidBody1->AddForce(linearForceBody1);		
		// Add angular force from impact
		rigidBody1->AddTorque(angularForceBody1);

		// Add force opposite gravity so it counteracts the next gravity tick
		rigidBody1->AddForce(Vector3(0.0f,-Physics::fGravity,0.0f)*rigidBody1->GetMass());

		if (rigidBody2 != nullptr) {
			Vector3 linearForceBody2 = impulseAlongNormal * mass2;
			Vector3 angularForceBody2 = contact.secondColliderCollisionPoint.Cross(impulseAlongNormal) * rigidBody2->GetInertiaTensor();

			// Add linear force from impact
			rigidBody2->AddForce(linearForceBody2);		
			// Add angular force from impact
			rigidBody2->AddTorque(angularForceBody2);

			// Add force opposite gravity so it counteracts the next gravity tick
			rigidBody2->AddForce(Vector3(0.0f,-Physics::fGravity,0.0f)*rigidBody1->GetMass());
		}
	}
}

bool CollisionManager::CheckForCollision(BoxCollider * pFirst, BoxCollider * pSecond, _Out_opt_ ContactResult &contactResult) {
	shared_ptr<BoundingOrientedBox> first = pFirst->GetOrientedBoundingBox(true);
	shared_ptr<BoundingOrientedBox> second = pSecond->GetOrientedBoundingBox(true);

	return false;
}