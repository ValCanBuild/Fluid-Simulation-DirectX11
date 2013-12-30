/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include <iostream>
#include "CollisionManager.h"
#include "../objects/BaseD3DBody.h"
#include "Physics.h"
#include "btBulletCollisionCommon.h"

#define NUM_ITERATIONS 7	// number of iterations to use when applying collision impulse forces
#define ALLOWED_PENETRATION 0.1f	// allowed penetration before a collision occurs

using namespace DirectX;
using namespace std;

CollisionManager::CollisionManager(vector<BaseD3DBody*> *sceneObjects ) {

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btVector3	worldAabbMin(-1000,-1000,-1000);
	btVector3	worldAabbMax(1000,1000,1000);
	
	btAxisSweep3*	broadphase = new btAxisSweep3(worldAabbMin,worldAabbMax);

	mCollisionWorld = shared_ptr<btCollisionWorld>(new btCollisionWorld(dispatcher,broadphase,collisionConfiguration));

	// Add all objects that have a collider to the collision world
	for (BaseD3DBody * object : *sceneObjects) {
		if (object->collider) {
			mCollisionWorld->addCollisionObject(object->collider->GetCollisionObject().get());
		}
	}
}

CollisionManager::~CollisionManager() {
	
}

void CollisionManager::AddObject(BaseD3DBody* sceneObject) {
	if (sceneObject->collider) {
		mCollisionWorld->addCollisionObject(sceneObject->collider->GetCollisionObject().get());
	}
}

void CollisionManager::FixedUpdate(float dt) {

	mCollisionWorld->performDiscreteCollisionDetection();

	btDispatcher* dispatcher = mCollisionWorld->getDispatcher();
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
	
		int numContacts = contactManifold->getNumContacts();

		if (numContacts == 0) {
			continue;
		}

		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		shared_ptr<RigidBody3D> rigidBody1 = (static_cast<Collider*>(obA->getUserPointer()))->GetAttachedRigidBody();
		shared_ptr<RigidBody3D> rigidBody2 = (static_cast<Collider*>(obB->getUserPointer()))->GetAttachedRigidBody();

		float totalAppliedImpulse = 0.0f;
		ContactResult contactResult;
		contactResult.firstRigidBody = rigidBody1;
		contactResult.secondRigidBody = rigidBody2;
		shared_ptr<Transform> transform1 = rigidBody1->GetGameObject()->transform;
		shared_ptr<Transform> transform2 = rigidBody2->GetGameObject()->transform;
		btVector3 firstColliderLocalPoint(0.0f, 0.0f, 0.0f);
		btVector3 secondColliderLocalPoint(0.0f, 0.0f, 0.0f);

		btScalar penDist = 0.0f;
		for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
			for (int j = 0; j < numContacts; j++) {
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
			
				btVector3 ptA = pt.getPositionWorldOnA();
				btVector3 ptB = pt.getPositionWorldOnB();

				penDist = pt.getDistance();

				firstColliderLocalPoint = obA->getWorldTransform().invXform(ptA);
				secondColliderLocalPoint = obB->getWorldTransform().invXform(ptB);
				contactResult.normal = BtVector3ToVector3(pt.m_normalWorldOnB);

				contactResult.firstColliderCollisionPoint = BtVector3ToVector3(firstColliderLocalPoint);
				contactResult.secondColliderCollisionPoint = BtVector3ToVector3(secondColliderLocalPoint);

				Vector3 v1,v2; // store velocity of body about the contact point
				if (rigidBody1) {
					v1 = rigidBody1->GetLinearVelocity() + (rigidBody1->GetAngularVelocity().Cross(contactResult.firstColliderCollisionPoint));

				}
				if (rigidBody2) {
					v2 = rigidBody2->GetLinearVelocity() + (rigidBody2->GetAngularVelocity().Cross(contactResult.secondColliderCollisionPoint));
				}
				contactResult.relativeVelocity = v1 - v2;

				totalAppliedImpulse += HandleContactResult(contactResult, dt, penDist);
			}
		}

		if (totalAppliedImpulse > 0.0f) {
			cout << "Collision! Applying impulse " << totalAppliedImpulse << endl;
		}
		//you can un-comment out this line, and then all points are removed
		//contactManifold->clearManifold();	
	}
}

float CollisionManager::HandleContactResult(ContactResult &contact, float dt, float penetration) const {
	shared_ptr<RigidBody3D> rigidBody1 = contact.firstRigidBody; // First collider definetly has a rigid body
	shared_ptr<RigidBody3D> rigidBody2 = contact.secondRigidBody; // Second collider may or may not have a rigid body
	
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

	float invDt = dt > 0.0f ? 1.0f / dt : 0.0f;
	float biasFactorValue = 0.1f;
	float bias = biasFactorValue * invDt * max(0.0f, -penetration - ALLOWED_PENETRATION);
	bias -= vrn;
	bias = max(0.0f,bias);
	impulse *= bias;

	Vector3 impulseAlongNormal = contact.normal * impulse;

	Vector3 linearForceBody1 = impulseAlongNormal * mass1;
	Vector3 angularForceBody1 = contact.firstColliderCollisionPoint.Cross(impulseAlongNormal) * rigidBody1->GetInertiaTensor();
	// Add linear force from impact
	rigidBody1->AddForce(linearForceBody1);		
	// Add angular force from impact
	rigidBody1->AddTorque(angularForceBody1);


	if (rigidBody2 != nullptr) {
		Vector3 linearForceBody2 = -impulseAlongNormal * mass2;
		Vector3 angularForceBody2 = -contact.secondColliderCollisionPoint.Cross(impulseAlongNormal) * rigidBody2->GetInertiaTensor();

		// Add linear force from impact
		rigidBody2->AddForce(linearForceBody2);		
		// Add angular force from impact
		rigidBody2->AddTorque(angularForceBody2);
	}

	return impulse;
}