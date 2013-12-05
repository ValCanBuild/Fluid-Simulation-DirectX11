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

	float		mProjectedArea; // Projected area of the body

	float		mWidth; // Bounding dimensions
	float		mLength;
	float		mHeight;

	Vector3		mPThrust,mSThrust; // bow thruster forces

	Vector3		mDragCenter;  // Location of center of drag in local coordinates
	Vector3		mThrustCenter;  // Location of center of propeller thrust in local coords.
	Vector3		mPortBowThruster; // Location of port bow thruster thrust in local coords.
	Vector3		mStarboardBowThruster; // Location of starboard bow thruster thrust in local coords.
private:
	/// BOOLEANS
	bool		mInCollision;

	/// SCALARS	
	float		mSpeed;	// Magnitude of velocity
	float		mInertia; // mass moment of inertia
	float		mInertiaInverse; // inverse of mass moment of intertia

	/// VECTORS
	Vector3		mVelocity;			// Linear velocity of body in global coordinates
	Vector3		mVelocityBody;		// Linear velocity in local coordinates
	Vector3		mAngularVelocity;	// Angular velocity in local coordinates
	Vector3		mForces;			// Total force acting on the body
	Vector3		mMoment;			// Total moment of body
	Vector3		mGravityForce;		// Gravity force vector
	Vector3		mPreviousPosition;	// Position at time t-dt
	Vector3		mImpactForces;		// Total impact forces acting on body

	Vector3		mExtraLinearForces; // Forces added outside of the rigid body private methods
	float		mExtraTorque;		// Torque added outside of the rigid body private methods
// Methods
public:
	void UpdateBodyEuler(float dt); // Integrates one time step using Euler integration
	void CollisionCheck(float dt, RigidBody2D *other, CollisionType2D_t collisionType);	// Collision  checks MUST be made before UpdateBody is called

	void AddForce(Vector3 &force);	// Adds a force to the body in the global coordinate system. This only adds linear motion
	void AddTorque(float torque); // Adds torque to the rigid body's center of mass

	void  SetIntertia(float inertia);
	void  GetVelocity(Vector3 &velocity) const;
	float GetMass() const;
	float GetSpeed() const;

private:
	void CalculateLoads();	// Aggregates forces acting on the particle
	void CheckCollisionScreen(float dt);

	void CheckCollisionBoundingCircle(float dt, RigidBody2D *other);
	void CheckCollisionBoundingBox(float dt, RigidBody2D *other);

	Vector3 VRotate2D( float angle, Vector3 &u);
};

#endif