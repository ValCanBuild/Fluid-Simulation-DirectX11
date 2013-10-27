/***************************************************************
Fluid3DScene.cpp: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/
#include "Fluid3DScene.h"


#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"

#define READ 0
#define WRITE 1

Fluid3DScene::Fluid3DScene() : mPaused(false), mAngle(0.0f) {
}

Fluid3DScene::~Fluid3DScene() {
	pD3dGraphicsObj = nullptr;

}

bool Fluid3DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,-5);

	mContainmentBox = unique_ptr<GeometricPrimitive>(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true).release());

	return true;
}

void Fluid3DScene::Update(float delta) {
	UpdateCamera(delta);
	//mAngle += delta;
}

bool Fluid3DScene::Render() {
	Matrix viewMatrix, projectionMatrix;
	pD3dGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);
	Matrix worldMatrix;
	pD3dGraphicsObj->GetWorldMatrix(worldMatrix);
	Matrix objectMatrix = Matrix::CreateRotationY(mAngle);
	//objectMatrix *= worldMatrix;

	mContainmentBox->Draw(objectMatrix,viewMatrix,projectionMatrix);
	

	return true;
}

void Fluid3DScene::UpdateCamera(float delta) {
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