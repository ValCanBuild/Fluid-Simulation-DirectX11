/*************************************************************
RigidBody3D Class: Provides a rigid body component to for 3D
objects

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#ifndef _RIGIDBODY3D_H
#define _RIGIDBODY3D_H

#include "Component.h"

#include <memory>

#include "../utilities/math/MathUtils.h"
#include "Transform.h"
#include "Bounds.h"


class RigidBody3D : public Component {
public:
	RigidBody3D(const GameObject * const gameObject);
	~RigidBody3D();

// Public Methods
public:
	bool Initialize();

	void UpdateBodyEuler(float dt); // Integrates one time step using Euler integration
	void ApplyGravity(float dt);

	void AddForce(Vector3 &force, bool impulse = false);	// Adds a force to the body in the global coordinate system. This only adds linear motion along the center of mass
	void AddTorque(Vector3 &torque); // Adds torque to the rigid body's center of mass

	void AddLinearVelocity(Vector3 &vel);
	void AddAngularVelocity(Vector3 &angVel); 

	// SETTERS
	void SetLinearVelocity(Vector3 &vel);
	void SetAngularVelocity(Vector3 &angVel);
	void SetMass(float mass);
	void SetLinearDrag(float drag);
	void SetAngularDrag(float angDrag);
	void WakeUp();
	void SetImmovable(bool isImmovable);
	void InContact();

	// GETTERS
	void  GetVelocity(Vector3 &velocity) const;
	void  GetAngularVelocity(Vector3 &velocity) const;
	const Vector3 &GetLinearVelocity() const;
	const Vector3 &GetAngularVelocity() const;
	const Vector3 &GetBodyVelocity() const;	// Get body velocity in local coordinates
	const Vector3 &GetInertiaTensor() const;	// Get diagonal inertia tensor of body
	const Matrix  &GetInertiaMatrix() const;
	const Matrix  &GetInverseInertiaMatrix() const;
	float GetMass() const;
	float GetSpeed() const;
	float GetLinearDrag() const;
	float GetAngularDrag() const;

	bool  GetIsSleeping() const;
	bool  GetIsInContact() const;
	bool  GetIsImmovable() const;

// Public variables
public:
	Vector3		mVelocity;			// Linear velocity of body in global coordinates
	Vector3		mAngularVelocity;	// Angular velocity in local coordinates
// Private Methods
private:
	void CalculateLoads();	// Aggregates forces acting on the particle
	void CalculateInertiaTensor(); // Calculates the the moments and products of inertia of the body. Done once at startup and when mass changes
	void CalculateInverseInertiaTensor(); 
private:
	/// SCALARS	
	float		mMass;	// Total Mass
	float		mLinearDrag; // Linear drag coefficient (0 means no damping) [0,1]
	float		mAngularDrag; // Angular drag coefficient ( 0 means no damping) [0,1]
	float		mSpeed;	// Magnitude of velocity
	Vector3		mInertiaTensor; // mass moment of inertia
	Matrix		mInertiaMatrix;	// Mass moment of inertia in body coordinates
	Matrix		mInverseInertiaMatrix;

	/// VECTORS
	Vector3		mVelocityBody;

	Vector3		mForces;			// Total force acting on the body
	Vector3		mMoments;			// Total moment (torque) of body
	Vector3		mPreviousPosition;	// Position at time t-dt
	Vector3		mPrevScale;

	Vector3		mExtraLinearForces; // Forces added outside of the rigid body private methods
	Vector3		mExtraTorque;		// Torque added outside of the rigid body private methods

	Vector3		mImpulseForces;

	bool		mIsSleeping; // is body sleeping
	bool		mInContact; // is body in contact
	bool		mIsImmovable;
};

#endif