/***************************************************************
RigidBody2D Class: Provides a rigid body component to for 2D
objects

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "RigidBody2D.h"
#include "../utilities/Physics.h"
#include "GameObject.h"
#include "../utilities/Screen.h"

using namespace DirectX;
using namespace std;

RigidBody2D::RigidBody2D(const GameObject * const gameObject) : Component(gameObject), mInCollision(false), mSpeed(0.0f), mRadius(0.0f) {
	
}

RigidBody2D::~RigidBody2D() {

}

// Aggregates forces acting on the particle
void RigidBody2D::CalculateLoads() {
	// Reset forces
	mForces = Vector3(0.0f);

	// Aggregate forces
	if (mInCollision) {
		// Add impact forces
		mForces += mImpactForces;
	}
	else {
		// Gravity
		mGravityForce.y = mMass * Physics::fGravity;
		mForces += mGravityForce;

		// Still air drag
		Vector3 vDrag = -mVelocity;
		vDrag.Normalize();

		float fDrag = 0.5f * Physics::fAirDensity * mSpeed * mSpeed * (PI * mRadius * mRadius) * Physics::fDragCoefficient;

		vDrag *= fDrag;
		mForces += vDrag;

		// Wind
		Vector3 vWind;
		vWind.x = 0.5f * Physics::fAirDensity * Physics::fWindSpeed * Physics::fWindSpeed * (PI * mRadius * mRadius) * Physics::fDragCoefficient;

		mForces += vWind;
	}
}

// Integrates one time step using Euler integration
void RigidBody2D::UpdateBodyEuler(float dt) {
	// Calculate forces acting on body
	CheckCollisionScreen(dt);

	CalculateLoads();

	shared_ptr<Transform> transform = GetGameObject()->transform;
	mPreviousPosition = transform->position;

	// F = ma

	// Integrate equation of motion:
	Vector3 a = mForces / mMass;
	Vector3 dv = a * dt;
	mVelocity += dv;

	Vector3 ds = mVelocity * dt;
	transform->position += ds;

	// Misc. calculations
	mSpeed = mVelocity.Length();

	mInCollision = false;
	mImpactForces = Vector3(0.0f);
}

void RigidBody2D::CheckCollisionScreen(float dt) {
	Vector3 n;	// unit normal vector pointing from the colliding object to the colliding particle
	Vector3 vr;	// relative velocity vector between particle and object
	float vrn;	// used to store the component of the relative velocity in direction of the collision normal vector
	float J;	// magnitude of impulse
	Vector3 Fi; // stores impact force as derived from J

	int screenWidth = Screen::width;
	int screenHeight = Screen::height;

	int size = GetGameObject()->bounds->GetBoundingSphere()->Radius*2.0f;
	shared_ptr<Transform> transform = GetGameObject()->transform;
	// Check for collisions with ground
	int iGroundPlane = screenHeight - size;
	if (transform->position.y >= iGroundPlane) {
		n.x = 0;
		n.y = -1;
		vr = mVelocity;
		vrn = vr.Dot(n);
		// check to see if the particle is moving toward the ground
		if (vrn < 0.0f) {
			J = -(vrn) * (Physics::fRestitution + 1) * mMass;
			Fi = n;
			Fi *= J/dt;
			mImpactForces += Fi;

			// reposition particle in preparation for impact force to be applied next timestep
			transform->position.y = iGroundPlane;
			transform->position.x = ( (iGroundPlane - mPreviousPosition.y) / (transform->position.y - mPreviousPosition.y) * (transform->position.x - mPreviousPosition.x) ) + mPreviousPosition.x;
			
			mInCollision = true;
		}
	}
}

void RigidBody2D::CollisionCheck(float dt, RigidBody2D *other, CollisionType2D_t collisionType) {
	switch (collisionType) {
	case COLLISION_TYPE2D_CIRCLE:
		CheckCollisionBoundingCircle(dt,other);
		break;
	case COLLISION_TYPE2D_BOX:
		CheckCollisionBoundingBox(dt,other);
		break;
	}
}

void RigidBody2D::CheckCollisionBoundingCircle(float dt, RigidBody2D *other) {
	const BoundingSphere *thisSphere = GetGameObject()->bounds->GetBoundingSphere();
	const BoundingSphere *otherSphere = other->GetGameObject()->bounds->GetBoundingSphere();

	if (thisSphere == nullptr || otherSphere == nullptr) {
		return;
	}

	if (thisSphere->Intersects(*otherSphere)) {
		Vector3 n;	// unit normal vector pointing from the colliding object to the colliding particle
		Vector3 vr;	// relative velocity vector between particle and object
		float vrn;	// used to store the component of the relative velocity in direction of the collision normal vector
		float J;	// magnitude of impulse
		Vector3 Fi; // stores impact force as derived from J

		float r = thisSphere->Radius + otherSphere->Radius;
		XMFLOAT3 thisCenter = thisSphere->Center;
		XMFLOAT3 otherCenter = otherSphere->Center;
		Vector3 d = Vector3(thisCenter.x,thisCenter.y,thisCenter.z) - Vector3(otherCenter.x,otherCenter.y,otherCenter.z);
		float s = d.Length() - r;

		d.Normalize();
		n = d;
		Vector3 otherVelocity;
		other->GetVelocity(otherVelocity);
		vr = mVelocity - otherVelocity;
		vrn = vr.Dot(n);

		J = -(vrn) * (Physics::fRestitution + 1) / (1/mMass + 1/other->GetMass());
		Fi = n;
		Fi *= J/dt;
		mImpactForces += Fi;

		GetGameObject()->transform->position -= n*s;	// adjust position so particle does not penetrate obstacle

		mInCollision = true;
	}

}

void RigidBody2D::CheckCollisionBoundingBox(float dt, RigidBody2D *other) {
	
}

void RigidBody2D::GetVelocity(Vector3 &velocity) const {
	velocity = mVelocity;
}

float RigidBody2D::GetMass() const {
	return mMass;
}

float RigidBody2D::GetSpeed() const {
	return mSpeed;
}