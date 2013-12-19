/***************************************************************
RigidBody3D Class: Provides a rigid body component to for 3D
objects

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "RigidBody3D.h"
#include "../utilities/Physics.h"
#include "GameObject.h"

using namespace DirectX;
using namespace std;

RigidBody3D::RigidBody3D(const GameObject * const gameObject) : Component(gameObject), mSpeed(0.0f),
	mInertiaTensor(0.0f), mProjectedArea(0.0f), mAngularVelocity(0.0f), mVelocityBody(0.0f),
	mExtraLinearForces(0.0f), mExtraTorque(0.0f), mMass(1.0f), mLinearDrag(0.0f), mAngularDrag(0.05f)
{
	
}

RigidBody3D::~RigidBody3D() {

}

bool RigidBody3D::Initialize() {
	CalculateMassProperties();

	return true;
}

// Adds a force to the body in the global coordinate system. This only adds linear motion
void RigidBody3D::AddForce(Vector3 &force) {
	mExtraLinearForces += force;
}

// Adds torque to the rigid body's center of mass
void RigidBody3D::AddTorque(Vector3 &torque) {
	mExtraTorque += torque;
}

// Calculates the the moments and products of inertia of the body
void RigidBody3D::CalculateMassProperties() {
	// This assumes we have a unit cube (of size 1) and the transform scale properties are used to size it up
	shared_ptr<Transform> transform = GetGameObject()->transform;
	float width = transform->scale.x;
	float height = transform->scale.y;
	float depth = transform->scale.z;

	float factor = 1.0f/12.0f * mMass;
	
	mInertiaTensor.x = factor*(height*height + depth*depth);
	mInertiaTensor.y = factor*(width*width + depth*depth);
	mInertiaTensor.z = factor*(width*width + height*height);

	mInertiaMatrix._11 = mInertiaTensor.x;
	mInertiaMatrix._22 = mInertiaTensor.y;
	mInertiaMatrix._33 = mInertiaTensor.z;

	mInverseInertiaMatrix = mInertiaMatrix.Invert();
}

// Aggregates forces acting on the particle
void RigidBody3D::CalculateLoads() {
	// Reset forces
	mForces = Vector3(0.0f);
	mMoments = Vector3(0.0f);

	// Aggregate forces
	shared_ptr<Transform> transform = GetGameObject()->transform;

	Vector3 Fb = Vector3(0.0f); // stores the sum of forces
	Vector3 Mb = Vector3(0.0f); // stores the sum of moments

	// Now add extra linear forces
	Fb += mExtraLinearForces;

	mForces = Vector3::Transform(Fb,transform->qRotation);
	
	// Gravity
	mForces.y += mMass * Physics::fGravity;

	// Linear forces
	mForces += Fb;

	// Angular forces
	Mb += mExtraTorque;

	mMoments += Mb;
}

// Integrates one time step using Euler integration
void RigidBody3D::UpdateBodyEuler(float dt) {
	// Calculate forces acting on body
	CalculateLoads();

	shared_ptr<Transform> transform = GetGameObject()->transform;
	mPreviousPosition = transform->position;

	// Integrate linear equation of motion:
	Vector3 a = mForces / mMass;
	Vector3 dv = a * dt;
	mVelocity += dv - (dv*mLinearDrag);

	Vector3 ds = mVelocity * dt;
	transform->position += ds;

	// Integrate angular equation of motion:
	Vector3 inertiaTimesVelocity = Vector3::Transform(mAngularVelocity,mInertiaMatrix);
	Vector3 temp = mAngularVelocity.Cross(inertiaTimesVelocity);
	Vector3 aa = Vector3::Transform(mMoments - temp, mInverseInertiaMatrix);
	aa *= dt;
	mAngularVelocity += aa - (aa*mAngularDrag);

	// Calculate the new rotation quaternion
	Quaternion tempQ = transform->qRotation * Quaternion(mAngularVelocity,0.0f);
	transform->qRotation += tempQ * 0.5f * dt;

	// and normalize it
	transform->qRotation.Normalize();

	// Calculate the velocity in body space
	Quaternion orientationConjugate;
	transform->qRotation.Conjugate(orientationConjugate);
	mVelocityBody = Vector3::Transform(mVelocity,orientationConjugate);
	
	// Misc. calculations
	mSpeed = mVelocity.Length();
	
	mExtraLinearForces = Vector3(0.0f);
	mExtraTorque = Vector3(0.0f);
}

void RigidBody3D::SetMass(float mass) {
	if (mass > 0.0f)
		mMass = mass;
}

void RigidBody3D::SetLinearDrag(float drag) {
	if (drag >= 0.0f) {
		mLinearDrag = drag;
	}
}
	
void RigidBody3D::SetAngularDrag(float angDrag) {
	if (angDrag >= 0.0f) {
		mAngularDrag = angDrag;
	}
}

void RigidBody3D::GetVelocity(Vector3 &velocity) const {
	velocity = mVelocity;
}

const Vector3 &RigidBody3D::GetLinearVelocity() const {
	return mVelocity;
}

const Vector3 &RigidBody3D::GetAngularVelocity() const {
	return mAngularVelocity;
}

// Get body velocity in local coordinates
const Vector3 &RigidBody3D::GetBodyVelocity() const {
	return mVelocityBody;
}

// Get diagonal inertia tensor of body
const Vector3 &RigidBody3D::GetInertiaTensor() const {
	return mInertiaTensor;
}

float RigidBody3D::GetMass() const {
	return mMass;
}

float RigidBody3D::GetSpeed() const {
	return mSpeed;
}

float RigidBody3D::GetLinearDrag() const {
	return mLinearDrag;
}

float RigidBody3D::GetAngularDrag() const {
	return mAngularDrag;
}