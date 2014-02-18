/***************************************************************
RigidBody3D Class: Provides a rigid body component to for 3D
objects

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "RigidBody3D.h"
#include "../utilities/Physics.h"
#include "BaseD3DBody.h"

using namespace DirectX;
using namespace std;

RigidBody3D::RigidBody3D(const GameObject * const gameObject) : Component(gameObject), mSpeed(0.0f),
	mInertiaTensor(0.0f), mAngularVelocity(0.0f), mVelocityBody(0.0f),
	mExtraLinearForces(0.0f), mExtraTorque(0.0f), mMass(1.0f), mLinearDamping(0.01f), mAngularDamping(0.05f),
	 mImpulseForces(0.0f),
	mIsSleeping(false), mInContact(false), mIsImmovable(false)
{
}

RigidBody3D::~RigidBody3D() {

}

bool RigidBody3D::Initialize() {
	CalculateInertiaTensor();
	mPrevScale = GetGameObject()->transform->scale;
	return true;
}

// Adds a force to the body in the global coordinate system. This only adds linear motion along the center of mass
void RigidBody3D::AddForce(Vector3 &force, bool impulse) {
	if (!impulse)
		mExtraLinearForces += force;
	else
		mImpulseForces += force;
}

// Adds torque to the rigid body's center of mass
void RigidBody3D::AddTorque(Vector3 &torque) {
	mExtraTorque += torque;
}

void RigidBody3D::AddLinearVelocity(Vector3 &vel) {
	mVelocity += vel;
}

void RigidBody3D::AddAngularVelocity(Vector3 &angVel) {
	mAngularVelocity += angVel;
}

// Calculates the the moments and products of inertia of the body
void RigidBody3D::CalculateInertiaTensor() {
	if (mIsImmovable) {
		mMass = 10000000.0f;
	}
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
	mInertiaMatrix._44 = 1.0f;

	CalculateInverseInertiaTensor();	
}

void RigidBody3D::CalculateInverseInertiaTensor() {
	Matrix rotMatrix = Matrix::CreateFromQuaternion(GetGameObject()->transform->qRotation);

	mInertiaMatrix = rotMatrix.Invert() * mInertiaMatrix;
	mInertiaMatrix *= rotMatrix;

	mInverseInertiaMatrix = rotMatrix.Invert() * mInertiaMatrix.Invert();
	mInverseInertiaMatrix *= rotMatrix;
}

// Aggregates forces acting on the particle
void RigidBody3D::CalculateLoads() {
	if (mIsImmovable) {
		return;
	}

	// Reset forces
	mForces = Vector3(0.0f);
	mMoments = Vector3(0.0f);

	// Aggregate forces
	shared_ptr<Transform> transform = GetGameObject()->transform;
	
	// Linear forces
	mForces += mExtraLinearForces;

	// Angular forces
	mMoments += mExtraTorque;
}

void RigidBody3D::ApplyGravity(float dt) {
	if (mIsImmovable || mIsSleeping) {
		return;
	}

	Vector3 grav = Vector3(0.0f, mMass * Physics::fGravity, 0.0f);

	mVelocity += grav / mMass * dt;
}

// Integrates one time step using Euler integration
void RigidBody3D::UpdateBodyEuler(float dt) {
	// if immovable object, or sleeping - ignore update
	if (mIsImmovable || mIsSleeping) {
		return;
	}

	// Calculate forces acting on body
	CalculateLoads();

	shared_ptr<Transform> transform = GetGameObject()->transform;
	mPreviousPosition = transform->position;

	if (mPrevScale != transform->scale) {
		CalculateInertiaTensor();
		mPrevScale = transform->scale;
	}

	// Integrate linear equation of motion:
	Vector3 a = (mForces / mMass);
	Vector3 dv = a * dt;
	mVelocity += dv;
	mVelocity *= powf((1.0f - mLinearDamping),dt);

	// Add impulse forces
	mVelocity += mImpulseForces / mMass;

	Vector3 ds = mVelocity * dt;
	transform->position += ds;

	// Integrate angular equation of motion:
	Vector3 aa = Vector3::Transform(mMoments, mInverseInertiaMatrix);
	Vector3 da = aa * dt;
	mAngularVelocity += da;
	mAngularVelocity *= powf((1.0f - mAngularDamping),dt);

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
	mImpulseForces = Vector3(0.0f);

	// Determine if rigid body has to sleep
	float angularSpeed = mAngularVelocity.Length();
	if (mInContact && mSpeed <= Physics::fSleepVelocity && angularSpeed <= Physics::fSleepAngularVelocity) {
		mAngularVelocity = Vector3(0.0f);
		mVelocity = Vector3(0.0f);
		mIsSleeping = true;
	}

	mInContact = false;

	CalculateInverseInertiaTensor();
}

void RigidBody3D::SetMass(float mass) {
	if (mass > 0.0f) {
		mMass = mass;
		CalculateInertiaTensor(); // changing mass requires a recalculation of the inertia tensor
	}
}

void RigidBody3D::SetLinearVelocity(Vector3 &vel) {
	mVelocity = vel;
}

void RigidBody3D::SetAngularVelocity(Vector3 &angVel) {
	mAngularVelocity = angVel;
}

void RigidBody3D::SetLinearDamping(float damping) {
	if (damping >= 0.0f) {
		mLinearDamping = damping;
	}
}
	
void RigidBody3D::SetAngularDamping(float angDamping) {
	if (angDamping >= 0.0f) {
		mAngularDamping = angDamping;
	}
}

void RigidBody3D::WakeUp() {
	mIsSleeping = false;
}

void RigidBody3D::InContact() {
	mInContact = true;
}

void RigidBody3D::SetImmovable(bool isImmovable) {
	// if going to a movable state from an immovable one, reset mass
	if (mIsImmovable && !isImmovable) {
		mMass = 1.0f;
	}
	if (isImmovable) {
		mVelocity = Vector3(0.0f);
		mAngularVelocity = Vector3(0.0f);
	}
	mIsImmovable = isImmovable;
	CalculateInertiaTensor();
}

void RigidBody3D::GetVelocity(Vector3 &velocity) const {
	velocity = mVelocity;
}

void RigidBody3D::GetAngularVelocity(Vector3 &angVelocity) const {
	angVelocity = mAngularVelocity;
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

const Matrix &RigidBody3D::GetInertiaMatrix() const {
	return mInertiaMatrix;
}

const Matrix &RigidBody3D::GetInverseInertiaMatrix() const {
	return mInverseInertiaMatrix;
}

float RigidBody3D::GetMass() const {
	return mMass;
}

float RigidBody3D::GetSpeed() const {
	return mSpeed;
}

float RigidBody3D::GetLinearDamping() const {
	return mLinearDamping;
}

float RigidBody3D::GetAngularDamping() const {
	return mAngularDamping;
}

bool RigidBody3D::GetIsSleeping() const {
	return mIsSleeping;
}

bool RigidBody3D::GetIsInContact() const {
	return mInContact;
}

bool RigidBody3D::GetIsImmovable() const {
	return mIsImmovable;
}