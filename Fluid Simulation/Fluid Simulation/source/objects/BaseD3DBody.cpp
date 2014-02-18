/**************************************************************
BaseD3DBody.cpp: A class to be used a base for any Direct3D
body

Author: Valentin Hinov
Date: 05/12/2013
**************************************************************/

#include "BaseD3DBody.h"

using namespace DirectX;

const Color SLEEPING_COLOR = Color(0.0f,0.0f,0.0f);
const Color COLLISION_COLOR = Color(0.2f,0.9f,0.1f);
const Color IMMOVABLE_COLOR = Color(0.78f,0.13f,0.10f);

BaseD3DBody::BaseD3DBody(std::unique_ptr<GeometricPrimitive> model, bool hasRigidBody, bool hasCollider) : mModel(std::move(model)), mPrevPosition(0.0f) {
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));

	if (hasRigidBody) {
		rigidBody3D = std::shared_ptr<RigidBody3D>(new RigidBody3D(this));
	}	
	
	if (hasCollider) {
		boxCollider = std::shared_ptr<BoxCollider>(new BoxCollider(this));
	}

	mColor = Color(1.0f,1.0f,1.0f);
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

void BaseD3DBody::Render(const Matrix* viewMatrix, const Matrix* projMatrix, bool wireframe) {
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);
	Color colorToDraw = mColor;
	if (rigidBody3D) {
		if (!rigidBody3D->GetIsImmovable()) {
			if (rigidBody3D->GetIsSleeping()) {
				colorToDraw = SLEEPING_COLOR;
			}
			else if (rigidBody3D->GetIsInContact()) {
				colorToDraw = COLLISION_COLOR;
			}
		}
		else {
			colorToDraw = IMMOVABLE_COLOR;
		}
	}
	mModel->Draw(objectMatrix,*viewMatrix,*projMatrix,colorToDraw,nullptr,wireframe);
}

void BaseD3DBody::Update(float dt) {
	
}

void BaseD3DBody::FixedUpdate(float fixedDeltaTime) {
	mPrevPosition = transform->position;
	if (rigidBody3D != nullptr) {
		//rigidBody3D->ApplyGravity(fixedDeltaTime);
		rigidBody3D->UpdateBodyEuler(fixedDeltaTime);
	}
	bounds->Update();
	boxCollider->Update();
}

void BaseD3DBody::SetColor(Color &color) {
	mColor = color;
}