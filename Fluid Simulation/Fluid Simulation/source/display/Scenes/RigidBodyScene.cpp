/***************************************************************
RigidBodyScene.cpp: Describes a scene that simulates rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/

#include "RigidBodyScene.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"

RigidBodyScene::RigidBodyScene() : mAngle(0.0f) {
}

RigidBodyScene::~RigidBodyScene() {
	pD3dGraphicsObj = nullptr;
}

bool RigidBodyScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,-10);

	mBox = unique_ptr<GeometricPrimitive>(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true).release());
	mPlane = unique_ptr<GeometricPrimitive>(GeometricPrimitive::CreatePlane(pD3dGraphicsObj->GetDeviceContext(), 100.0f, 1, true).release());
	
	return true;
}

void RigidBodyScene::Update(float delta) {
	UpdateCamera(delta);
	mAngle += 0.01f;
}

bool RigidBodyScene::Render() {
	Matrix viewMatrix, projectionMatrix, worldMatrix;
	pD3dGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);
	pD3dGraphicsObj->GetWorldMatrix(worldMatrix);

	Matrix objectMatrix = Matrix::CreateRotationY(mAngle);
	objectMatrix *= worldMatrix;

	mPlane->Draw(worldMatrix,viewMatrix,projectionMatrix);

	mBox->Draw(objectMatrix,viewMatrix,projectionMatrix);

	return true;
}

void RigidBodyScene::UpdateCamera(float delta) {
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();

	// Move camera with WASD 
	float forwardAmount = 0.0f;
	float rightAmount = 0.0f;
	const float moveFactor = 2.0f;

	if (inputSystem->IsKeyDown('W')) {
		forwardAmount += delta;
	}
	else if (inputSystem->IsKeyDown('S')) {
		forwardAmount -= delta;
	}
	if (inputSystem->IsKeyDown('A')) {
		rightAmount += delta;
	}
	else if (inputSystem->IsKeyDown('D')) {
		rightAmount -= delta;
	}

	if (forwardAmount != 0.0f || rightAmount != 0.0f) {
		mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
	}

	// Rotate camera with mouse button
	if (inputSystem->IsMouseRightDown()) {
		int xDelta,yDelta;
		float mouseSensitivity = 0.003f;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		mCamera->AddYawPitchRoll(-xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
	}

	mCamera->Update();
}