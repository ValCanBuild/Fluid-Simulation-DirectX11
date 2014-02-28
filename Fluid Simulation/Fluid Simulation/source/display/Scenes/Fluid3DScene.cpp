/***************************************************************
Fluid3DScene.cpp: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/

#include <AntTweakBar.h>

#include "Fluid3DScene.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"
#include "../VolumeRenderer.h"
#include "../simulators/Fluid3DSimulator.h"

using namespace Fluid3D;

#define DIM 80

Fluid3DScene::Fluid3DScene() : mPaused(false), pInputSystem(nullptr) {
	
}

Fluid3DScene::~Fluid3DScene() {
	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;

	pD3dGraphicsObj = nullptr;
	pInputSystem = nullptr;
}

bool Fluid3DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,-10);

	mPlane = GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true);
	mFluid3DEffect = unique_ptr<Fluid3DSimulator>(new Fluid3DSimulator(Vector3(DIM)));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(DIM), Vector3(0.0f)));

	bool result = mFluid3DEffect->Initialize(pD3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}
	
	result = mVolumeRenderer->Initialize(pD3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	pInputSystem = ServiceProvider::Instance().GetInputSystem();


	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("3D Fluid Simulation");
	// Position bar
	int barPos[2] = {580,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {218,150};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Jacobi Iterations", TW_TYPE_INT32, &mFluid3DEffect->jacobiIterations, "min=1 max=100 step=1");
	TwAddVarRW(mTwBar,"Time Step", TW_TYPE_FLOAT, &mFluid3DEffect->timeStep, "min=0.0 max=1.0 step=0.001");
	TwAddVarRW(mTwBar,"MacCormarck Advection", TW_TYPE_BOOLCPP, &mFluid3DEffect->macCormackEnabled, nullptr);
	TwAddVarRW(mTwBar,"Simulation Paused", TW_TYPE_BOOLCPP, &mPaused, nullptr);

	return result;
}

void Fluid3DScene::Update(float delta) {
	UpdateCamera(delta);
	HandleInput();

	if (!mPaused) {
		mFluid3DEffect->ProcessEffect();
	}
}

bool Fluid3DScene::Render() {
	Matrix viewMatrix, projectionMatrix;
	pD3dGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);
	
	mVolumeRenderer->Render(mFluid3DEffect->GetVolumeTexture(), mCamera.get(), &viewMatrix, &projectionMatrix);

	return true;
}

void Fluid3DScene::UpdateCamera(float delta) {

	// Move camera with WASD 
	float forwardAmount = 0.0f;
	float rightAmount = 0.0f;
	const float moveFactor = 2.0f;

	if (pInputSystem->IsKeyDown('W')) {
		forwardAmount += delta;
	}
	else if (pInputSystem->IsKeyDown('S')) {
		forwardAmount -= delta;
	}
	if (pInputSystem->IsKeyDown('A')) {
		rightAmount += delta;
	}
	else if (pInputSystem->IsKeyDown('D')) {
		rightAmount -= delta;
	}

	if (forwardAmount != 0.0f || rightAmount != 0.0f) {
		mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
	}

	// Rotate camera with mouse button
	if (pInputSystem->IsMouseRightDown()) {
		int xDelta,yDelta;
		float mouseSensitivity = 0.003f;
		pInputSystem->GetMouseDelta(xDelta,yDelta);
		mCamera->AddYawPitchRoll(-xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
	}

	mCamera->Update();
}

void Fluid3DScene::HandleInput() {

}