/**************************************************************
BaseD3DBody.cpp: A class to be used a base for any Direct3D
body

Author: Valentin Hinov
Date: 05/12/2013
**************************************************************/

#include "BaseD3DBody.h"
#include "btBulletCollisionCommon.h"
#include "../utilities/math/MathUtils.h"

using namespace DirectX;

BaseD3DBody::BaseD3DBody(std::unique_ptr<GeometricPrimitive> model, bool hasRigidBody, bool hasCollider) : mModel(std::move(model)), mPrevPosition(0.0f) {
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));

	if (hasRigidBody) {
		rigidBody3D = std::shared_ptr<RigidBody3D>(new RigidBody3D(this));
	}	
	
	if (hasCollider) {
		btBoxShape* boxShape = new btBoxShape(Vector3ToBtVector3(transform->scale*0.5f));
		collider = std::shared_ptr<Collider>(new Collider(this, boxShape));
	}
}

BaseD3DBody::~BaseD3DBody() {

}

bool BaseD3DBody::Initialize(D3DGraphicsObject * pGraphicsObj, HWND hwnd) {
	bool result = true;
	if (rigidBody3D != nullptr) {
		result = rigidBody3D->Initialize();
	}

	return result;
}

void BaseD3DBody::Render(const Matrix* viewMatrix, const Matrix* projMatrix) {
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);
	mModel->Draw(objectMatrix,*viewMatrix,*projMatrix);
}

void BaseD3DBody::Update(float dt) {
	// update bounding box if there has been a change in position
	
}

void BaseD3DBody::FixedUpdate(float fixedDeltaTime) {
	mPrevPosition = transform->position;
	if (rigidBody3D != nullptr) {
		rigidBody3D->UpdateBodyEuler(fixedDeltaTime);
	}
	bounds->Update();
	if (collider) {
		collider->Update();
	}
	/*if (transform->position.y - 0.5f*transform->scale.y < 0.0f) {
		transform->position.y = 0.5f*transform->scale.y;
	}*/	
}
