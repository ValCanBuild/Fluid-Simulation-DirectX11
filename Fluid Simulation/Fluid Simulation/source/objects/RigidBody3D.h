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

	void AddForce(Vector3 &force);	// Adds a force to the body in the global coordinate system. This only adds linear motion
	void AddTorque(Vector3 &torque); // Adds torque to the rigid body's center of mass

	// SETTERS
	void  SetMass(float mass);
	void  SetLinearDrag(float drag);
	void  SetAngularDrag(float angDrag);

	// GETTERS
	void  GetVelocity(Vector3 &velocity) const;
	const Vector3 &GetLinearVelocity() const;
	const Vector3 &GetAngularVelocity() const;
	const Vector3 &GetBodyVelocity() const;	// Get body velocity in local coordinates
	const Vector3 &GetInertiaTensor() const;	// Get diagonal inertia tensor of body
	float GetMass() const;
	float GetSpeed() const;
	float GetLinearDrag() const;
	float GetAngularDrag() const;

// Private Methods
private:
	void CalculateLoads();	// Aggregates forces acting on the particle
	void CalculateMassProperties(); // Calculates the the moments and products of inertia of the body

private:
	/// SCALARS	
	float		mMass;	// Total Mass
	float		mLinearDrag; // Linear drag coefficient (0 means no damping) [0,1]
	float		mAngularDrag; // Angular drag coefficient ( 0 means no damping) [0,1]
	float		mSpeed;	// Magnitude of velocity
	Vector3		mInertiaTensor; // mass moment of inertia
	float		mProjectedArea; // Projected area of the body
	Matrix		mInertiaMatrix;	// Mass moment of inertia in body coordinates
	Matrix		mInverseInertiaMatrix;

	/// VECTORS
	Vector3		mVelocity;			// Linear velocity of body in global coordinates
	Vector3		mVelocityBody;		// Linear velocity in local coordinates
	Vector3		mAngularVelocity;	// Angular velocity in local coordinates
	Vector3		mForces;			// Total force acting on the body
	Vector3		mMoments;			// Total moment (torque) of body
	Vector3		mPreviousPosition;	// Position at time t-dt

	Vector3		mExtraLinearForces; // Forces added outside of the rigid body private methods
	Vector3		mExtraTorque;		// Torque added outside of the rigid body private methods
};

#endif