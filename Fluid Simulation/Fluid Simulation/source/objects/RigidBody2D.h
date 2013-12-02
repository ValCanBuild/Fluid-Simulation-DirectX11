/*************************************************************
RigidBody2D Class: Provides a rigid body component to for 2D
objects

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "Component.h"

#include <memory>

#include "../utilities/math/MathUtils.h"
#include "Transform.h"
#include "Bounds.h"

enum CollisionType2D_t {
	COLLISION_TYPE2D_CIRCLE,
	COLLISION_TYPE2D_BOX
};

class RigidBody2D : public Component {
public:
	RigidBody2D(const GameObject * const gameObject);
	~RigidBody2D();

public:
	float		mMass;	// Total Mass
	float		mRadius; // Radius of object

private:
	/// BOOLEANS
	bool		mInCollision;

	/// SCALARS	
	float		mSpeed;	// Magnitude of velocity	

	/// VECTORS
	Vector3		mVelocity;	// Linear velocity of body
	Vector3		mForces; // Total force acting on the body
	Vector3		mGravityForce; // Gravity force vector
	Vector3		mPreviousPosition;	//position at time t-dt
	Vector3		mImpactForces; // Total impact forces acting on body

// Methods
public:
	void UpdateBodyEuler(float dt); // Integrates one time step using Euler integration
	void CollisionCheck(float dt, RigidBody2D *other, CollisionType2D_t collisionType);	// Collision  checks MUST be made before UpdateBody is called

	void  GetVelocity(Vector3 &velocity) const;
	float GetMass() const;
	float GetSpeed() const;

private:
	void CalculateLoads();	// Aggregates forces acting on the particle
	void CheckCollisionScreen(float dt);

	void CheckCollisionBoundingCircle(float dt, RigidBody2D *other);
	void CheckCollisionBoundingBox(float dt, RigidBody2D *other);
};

#endif