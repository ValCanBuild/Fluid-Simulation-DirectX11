/***************************************************************
Fluid3DScene.cpp: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/


#include "Fluid3DScene.h"
#include <algorithm>
#include <AntTweakBar.h>

#include "../D3DGraphicsObject.h"
#include "../../utilities/CameraImpl.h"
#include "../../system/ServiceProvider.h"
#include "../../objects/VolumeRenderer.h"
#include "../simulations/FluidSimulation.h"
#include "../../objects/SkyObject.h"

using namespace Fluid3D;
using namespace DirectX;

const string barName = "3D Fluid Simulation";

struct FluidSimulationDepthSort {
	Vector3 cameraPosition;

	FluidSimulationDepthSort(Vector3 &cameraPosition) : cameraPosition(cameraPosition) {}

	bool operator() (const std::shared_ptr<FluidSimulation>& first, const std::shared_ptr<FluidSimulation>& second) const {
		Vector3 firstPositon = first->GetVolumeRenderer()->transform->position;
		Vector3 secondPosition = second->GetVolumeRenderer()->transform->position;

		return Vector3::Distance(firstPositon, cameraPosition) > Vector3::Distance(secondPosition, cameraPosition);
	}
};

Fluid3DScene::Fluid3DScene() : mPaused(false), pInputSystem(nullptr), mNumRenderedFluids(0), mNumFluidsUpdating(0), pPickedSimulation(nullptr) {
	
}

Fluid3DScene::~Fluid3DScene() {
	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;

	pD3dGraphicsObj = nullptr;
	pInputSystem = nullptr;
	pPickedSimulation = nullptr;
	mPrimitiveObjects.clear();
	mSimulations.clear();
}

bool Fluid3DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);

	InitCamera();
	InitGameObjects();	
	bool result = InitSimulations(hwnd);
	if (!result) {
		return false;
	}

	pInputSystem = ServiceProvider::Instance().GetInputSystem();

	// Initialize this scene's tweak bar
	mTwBar = TwNewBar(barName.c_str());
	// Position bar
	int barPos[2] = {550,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {250,250};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);
	// hide bar initially
	string command = " '" + barName + "' iconified=true ";
	TwDefine(command.c_str());

	mSkyObject = unique_ptr<SkyObject>(new SkyObject());
	result = mSkyObject->Initialize(pD3dGraphicsObj, L"data/textures/skybox/ame_bluefreeze.dds", hwnd);
	if (!result) {
		return false;
	}

	return result;
}

bool Fluid3DScene::InitSimulations(HWND hwnd) {
	for (int i = 0; i < 1; i++) {
		shared_ptr<FluidSimulation> fluidSimulation(new FluidSimulation());
		mSimulations.push_back(fluidSimulation);
		shared_ptr<VolumeRenderer> volumeRenderer = fluidSimulation->GetVolumeRenderer();
		volumeRenderer->transform->position.y = 1.2f;
		volumeRenderer->transform->position.x = 0.0f + 1.0f*i;
		volumeRenderer->transform->scale = Vector3(1,2,1);
	}
	for (shared_ptr<FluidSimulation> simulation : mSimulations) {
		bool result = simulation->Initialize(pD3dGraphicsObj, hwnd);
		if (!result) {
			return false;
		}
	}

	return true;
}

void Fluid3DScene::InitCamera() {
	float nearVal, farVal;
	pD3dGraphicsObj->GetScreenDepthInfo(nearVal, farVal);
	int screenWidth, screenHeight;
	pD3dGraphicsObj->GetScreenDimensions(screenWidth, screenHeight);
	float fieldOfView = (float)PI / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	mCamera = CameraImpl::CreateCameraLH(fieldOfView, screenAspect, nearVal, farVal);
	mCamera->SetPosition(0,0.5f,-4);
}

void Fluid3DScene::InitGameObjects() {
	PrimitiveGameObject planeObject = PrimitiveGameObject(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, false));
	planeObject.transform->position = Vector3(0.0f,0.0f,0.0f);
	planeObject.transform->scale = Vector3(70.0f,0.1f,70.0f);

	mPrimitiveObjects.push_back(planeObject);
}

void Fluid3DScene::Update(float delta) {
	UpdateCamera(delta);
	HandleInput();

	mNumFluidsUpdating = 0;

	for (PrimitiveGameObject &object : mPrimitiveObjects) {
		object.Update();
	}

	if (!mPaused) {
		for (shared_ptr<FluidSimulation> simulation : mSimulations) {
			if (simulation->Update(delta)) {
				++mNumFluidsUpdating;
			}
		}
	}
}

bool Fluid3DScene::Render() {
	mNumRenderedFluids = 0;

	mSkyObject->Render(*mCamera);

	for (PrimitiveGameObject &object : mPrimitiveObjects) {
		object.Render(*mCamera);
	}

	for (shared_ptr<FluidSimulation> simulation : mSimulations) {
		if (simulation->Render(*mCamera)) {
			++mNumRenderedFluids;
		}
	}

	return true;
}

void Fluid3DScene::RenderOverlay(std::shared_ptr<DirectX::SpriteBatch> spriteBatch, std::shared_ptr<DirectX::SpriteFont> spriteFont) {
	wstring text = L"Fluids Rendered: " + std::to_wstring(mNumRenderedFluids);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,60));

	text = L"Fluids Updating: " + std::to_wstring(mNumFluidsUpdating);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,85));
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
		rightAmount -= delta;
	}
	else if (pInputSystem->IsKeyDown('D')) {
		rightAmount += delta;
	}

	if (forwardAmount != 0.0f || rightAmount != 0.0f) {
		mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
		// the camera has moved - sort transparent object render order
		SortTransparentObjects(); 
	}

	// Rotate camera with mouse button
	if (pInputSystem->IsMouseRightDown()) {
		int xDelta,yDelta;
		float mouseSensitivity = 0.003f;
		pInputSystem->GetMouseDelta(xDelta,yDelta);
		mCamera->AddYawPitchRoll(xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
	}

	mCamera->Update();
}

void Fluid3DScene::HandleInput() {
	if (pInputSystem->IsMouseLeftClicked()) {
		HandleMousePicking();
	}
}

void Fluid3DScene::HandleMousePicking() {
	int posX,posY;
	pInputSystem->GetMousePos(posX,posY);
	Ray ray = mCamera->ScreenPointToRay(Vector2((float)posX,(float)posY));

	float prevMinDistance = FLT_MAX;
	shared_ptr<FluidSimulation> picked(nullptr);
	for (shared_ptr<FluidSimulation> simulation : mSimulations) {
		float distance;
		if (simulation->IntersectsRay(ray, distance)) {
			if (distance < prevMinDistance) {
				picked = simulation;
			}
		}
	}

	if (picked == pPickedSimulation) {
		return;
	}
	else {
		if (pPickedSimulation != nullptr) {
			pPickedSimulation = nullptr;
			TwRemoveAllVars(mTwBar);
			string command = " '" + barName + "' iconified=true ";
			TwDefine(command.c_str());
		}
		if (picked != nullptr) {
			pPickedSimulation = picked;
			pPickedSimulation->DisplayInfoOnBar(mTwBar);
			string command = " '" + barName + "' iconified=false ";
			TwDefine(command.c_str());
		}
	}
}

void Fluid3DScene::SortTransparentObjects() {
	Vector3 camPos;
	mCamera->GetPosition(camPos);
	sort(mSimulations.begin(), mSimulations.end(), FluidSimulationDepthSort(camPos));
}
