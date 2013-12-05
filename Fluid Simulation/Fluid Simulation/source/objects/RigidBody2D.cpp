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

RigidBody2D::RigidBody2D(const GameObject * const gameObject) : Component(gameObject), mInCollision(false), mSpeed(0.0f), mRadius(0.0f), 
	mInertia(0.0f), mInertiaInverse(0.0f), mProjectedArea(0.0f), mAngularVelocity(0.0f), mVelocityBody(0.0f), mExtraLinearForces(0.0f), mExtraTorque(0.0f) {
	
}

RigidBody2D::~RigidBody2D() {

}

// Adds a force to the body in the global coordinate system. This only adds linear motion
void RigidBody2D::AddForce(Vector3 &force) {
	mExtraLinearForces += force;
}

// Adds torque to the rigid body's center of mass
void RigidBody2D::AddTorque(float torque) {
	mExtraTorque += torque;
}

// Aggregates forces acting on the particle
void RigidBody2D::CalculateLoads() {
	// Reset forces
	mForces = Vector3(0.0f);
	mMoment = Vector3(0.0f);

	// Aggregate forces
	if (mInCollision) {
		// Add impact forces
		mForces += mImpactForces;
	}
	else {
		shared_ptr<Transform> transform = GetGameObject()->transform;

		Vector3 Fb = Vector3(0.0f); // stores the sum of forces
		Vector3 Mb = Vector3(0.0f); // stores the sum of moments

		// Calculate forces and moments in body space:
		// Calculate the aerodynamic drag force:
			// Calculate local velocity:
			// The local velocity includes the velocity due to
			// linear motion of the craft,
			// plus the velocity at each element
			// due to the rotation of the craft.

		// rotational part
		Vector3 vTmp = mAngularVelocity.Cross(mDragCenter);
		Vector3 vLocalVelocity = mVelocityBody + vTmp;

		// Calculate local air speed
		float fLocalSpeed = vLocalVelocity.Length();
		
		// Find the direction in which drag will act.
		// Drag always acts in line with the relative
		// velocity but in the opposing direction
		float fTol = 0.0000001f;	// tolerance
		if(fLocalSpeed > fTol) {
			vLocalVelocity.Normalize();

			Vector3 vDragVector = -vLocalVelocity;

			// determine the resultant force on the element
			float tmp = 0.5f * Physics::fAirDensity * fLocalSpeed*fLocalSpeed * mProjectedArea;
			Vector3 vResultant = vDragVector * Physics::fLinearDragCoefficient * tmp;

			// Keep a running total of these resultant forces
			Fb += vResultant;

			// Calculate the moment about the CG
			// and keep a running total of these moments
			vTmp = mDragCenter.Cross(vResultant);
			Mb += vTmp;
		}

		// Calculate the Port & Starboard bow thruster forces:
		// Keep a running total of these resultant forces
		//Fb += mPThrust;

		// Calculate the moment about the CG of this element's force
		// and keep a running total of these moments (total moment)
		//vTmp = mPortBowThruster.Cross(mPThrust);
		//Mb += vTmp;

		// Keep a running total of these resultant forces (total force)
		//Fb += mSThrust;
		// Calculate the moment about the CG of this element's force
		// and keep a running total of these moments (total moment)
		//vTmp = mStarboardBowThruster.Cross(mSThrust);
		//Mb += vTmp;

		// Now add extra moment forces
		Mb.z += mExtraTorque;

		// Now add extra linear forces
		Fb += mExtraLinearForces;

		// Convert forces from model space to earth space
		mForces = VRotate2D(transform->rotation.z, Fb);

		mMoment += Mb;
		
		// Gravity
		/*mGravityForce.y = mMass * Physics::fGravity;
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

		mForces += vWind;*/
	}
}

// Integrates one time step using Euler integration
void RigidBody2D::UpdateBodyEuler(float dt) {
	// Calculate forces acting on body
	//CheckCollisionScreen(dt);

	CalculateLoads();

	shared_ptr<Transform> transform = GetGameObject()->transform;
	mPreviousPosition = transform->position;

	// Integrate linear equation of motion:
	Vector3 a = mForces / mMass;
	Vector3 dv = a * dt;
	mVelocity += dv;

	Vector3 ds = mVelocity * dt;
	transform->position += ds;

	// Integrate angular equation of motion
	// find angular acceleration by dividing the aggregate torque acting on the body by its mass moment of inertia
	float aa = mMoment.z / mInertia;	
	float dav = aa * dt;

	mAngularVelocity.z += dav;

	float dr = mAngularVelocity.z * dt;
	transform->rotation.z += dr;

	// Misc. calculations
	mSpeed = mVelocity.Length();
	mVelocityBody = VRotate2D(-transform->rotation.z, mVelocity);
	
	mImpactForces = Vector3(0.0f); // maybe move this somewhere else
	mExtraLinearForces = Vector3(0.0f);
	mExtraTorque = 0.0f;
	mInCollision = false;
}

void RigidBody2D::CheckCollisionScreen(float dt) {
	Vector3 n;	// unit normal vector pointing from the colliding object to the colliding particle
	Vector3 vr;	// relative velocity vector between particle and object
	float vrn;	// used to store the component of the relative velocity in direction of the collision normal vector
	float J;	// magnitude of impulse
	Vector3 Fi; // stores impact force as derived from J

	int screenWidth = Screen::width;
	int screenHeight = Screen::height;

	Vector3 extents = GetGameObject()->bounds->GetExtents();
	shared_ptr<Transform> transform = GetGameObject()->transform;
	// Check for collisions with ground
	int iGroundPlane = screenHeight - (extents.y*2);
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

Vector3 RigidBody2D::VRotate2D( float angle, Vector3 &u) {
	float x,y;
	x = u.x * cos(-angle) +
	u.y * sin(-angle);
	y = -u.x * sin(-angle) +
	u.y * cos(-angle);
	return Vector3( x, y, 0);
}

void RigidBody2D::SetIntertia(float inertia) {
	mInertia = inertia;
	mInertiaInverse = 1.0f/mInertia;
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