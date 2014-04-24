/***************************************************************
Fluid3DScene.cpp: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/


#include "Fluid3DScene.h"
#include <algorithm>
#include <AntTweakBar.h>
#include <Effects.h>

#include "../D3DGraphicsObject.h"
#include "../../utilities/CameraImpl.h"
#include "../../utilities/AutoCameraController.h"
#include "../../system/ServiceProvider.h"
#include "../../objects/VolumeRenderer.h"
#include "../simulations/FluidSimulation.h"
#include "../../objects/SkyObject.h"
#include "../../objects/TerrainObject.h"
#include "../../objects/ModelGameObject.h"
#include "../../utilities/HeightmapParser.h"
#include "../../utilities/Screen.h"
#include "../../system/InputSystem.h"

using namespace Fluid3D;
using namespace DirectX;

const string barName = "3D Fluid Simulation";

Fluid3DScene::Fluid3DScene() : mPaused(false), pInputSystem(nullptr), mNumRenderedFluids(0), mNumFluidsUpdating(0), mAutoCamEnabled(false),
	pPickedRenderer(nullptr) {
	
}

Fluid3DScene::~Fluid3DScene() {
	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;

	pD3dGraphicsObj = nullptr;
	pInputSystem = nullptr;
	mVolumeRenderers.clear();
	mSimulations.clear();
	mModelObjects.clear();
}

bool Fluid3DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);

	InitCamera();
	InitGameObjects();	
	bool result = InitSimulations(hwnd);
	if (!result) {
		return false;
	}

	pInputSystem = ServiceProvider::Instance().GetService<InputSystem>();

	// Initialize this scene's tweak bar
	mTwBar = TwNewBar(barName.c_str());
	// Position bar
	int barWidth = 250;
	int barHeight = 250;
	int barX = Screen::width - barWidth - 1;
	int barY = 2;
	int barPos[2] = {barX, barY};
	TwSetParam(mTwBar, nullptr, "position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {barWidth, barHeight};
	TwSetParam(mTwBar, nullptr, "size", TW_PARAM_INT32, 2, barSize);
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
	/*for (int i = 0; i < 2; i++) {
		FluidSettings fluidSettingsSmoke(SMOKE);
		fluidSettingsSmoke.dimensions = Vector3(32);
		fluidSettingsSmoke.densityDissipation = 0.99f;

		auto smokeFluidSim = make_shared<FluidSimulation>(fluidSettingsSmoke);
		mSimulations.push_back(smokeFluidSim);
	}*/

	FluidSettings fluidSettingsSmoke(SMOKE);
	fluidSettingsSmoke.dimensions = Vector3(64,128,64);
	fluidSettingsSmoke.densityDissipation = 0.99f;

	auto volumeRendererSmoke = make_shared<VolumeRenderer>();
	volumeRendererSmoke->transform->scale = Vector3(4,8,4);
	volumeRendererSmoke->transform->position = Vector3(-0.15f,10.08f,6.43f);
	mVolumeRenderers.push_back(volumeRendererSmoke);

	auto smokeFluidSim = make_shared<FluidSimulation>(fluidSettingsSmoke);
	smokeFluidSim->AddVolumeRenderer(volumeRendererSmoke);
	mSimulations.push_back(smokeFluidSim);

	FluidSettings fluidSettingsFire(FIRE);
	fluidSettingsFire.dimensions = Vector3(34,60,34);
	fluidSettingsFire.constantInputPosition = Vector3(0.5f,0.05f,0.5f);
	auto fireFluidSim = make_shared<FluidSimulation>(fluidSettingsFire);
	mSimulations.push_back(fireFluidSim);

	// two fire simulations using one calculator
	for (int i = 0; i < 2; ++i) {
		auto volumeRendererFire = make_shared<VolumeRenderer>();
		volumeRendererFire->transform->scale = Vector3(1,2,1);
		float xPos = i == 0 ? 2.0f : -2.0f;
		volumeRendererFire->transform->position = Vector3(xPos,2.76f,0);

		auto smokeProperties = volumeRendererFire->GetRenderSettings();
		smokeProperties->vSmokeColor = RGBA2Color(40,40,40,255);
		
		fireFluidSim->AddVolumeRenderer(volumeRendererFire);
		mVolumeRenderers.push_back(volumeRendererFire);
	}

	for (auto simulation : mSimulations) {
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
	mCamera->SetPosition(0, 3.0f, -8.5);

	mAutoCameraController = unique_ptr<AutoCameraController>(new AutoCameraController(*mCamera));
	mAutoCameraController->AddRoutePoint(Vector3(0,3,-8.87f));
	mAutoCameraController->AddRoutePoint(Vector3(-12.0f, 4.84f, 12.15f));
	mAutoCameraController->AddRoutePoint(Vector3(10.32f, 5.48f, 13.33f));
	mAutoCameraController->AddRoutePoint(Vector3(19.0f, 13.0f, -15.64f));
	mAutoCameraController->AddRoutePoint(Vector3(-15.5f, 12.52f, -19.7f));
	mAutoCameraController->AddRoutePoint(Vector3(-12.0f, 3.6f, 2.2f));
	mAutoCameraController->AddRoutePoint(Vector3(10.62f, 2.8f, 0.0f));
	mAutoCameraController->AddRoutePoint(Vector3(0,3,-8.87f));	

	mAutoCameraController->AddRouteRotation(Quaternion(0,0,0,1));
	mAutoCameraController->AddRouteRotation(Quaternion(-0.03f,0.87f,0.02f,0.43f));
	mAutoCameraController->AddRouteRotation(Quaternion(0.013f,0.91f,0.03f,-0.40f));
	mAutoCameraController->AddRouteRotation(Quaternion(-0.12f,0.38f,-0.05f,-0.91f));
	mAutoCameraController->AddRouteRotation(Quaternion(-0.1f,-0.29f,-0.03f,-0.94f));
	mAutoCameraController->AddRouteRotation(Quaternion(0.03f,-0.68f,-0.03f,-0.72f));
	mAutoCameraController->AddRouteRotation(Quaternion(-0.19f,-0.49f,-0.11f,0.84f));
	mAutoCameraController->AddRouteRotation(Quaternion(0,0,0,1));

	mAutoCameraController->SetSpeedFactor(0.025f);
}

void Fluid3DScene::InitGameObjects() {
	// House
	DGSLEffectFactory fx(pD3dGraphicsObj->GetDevice());
	fx.SetDirectory(L"data/models/house");
	shared_ptr<ModelGameObject> house = unique_ptr<ModelGameObject>(new ModelGameObject(Model::CreateFromCMO(pD3dGraphicsObj->GetDevice(), L"data/models/house/English_thatched_house.cmo", fx, false)));
	auto houseTransform = house->transform;
	houseTransform->position = Vector3(0,0.15f,6);
	houseTransform->scale = Vector3(0.5f);
	houseTransform->qRotation = Quaternion::CreateFromAxisAngle(Vector3(1,0,0), -1.57f);
	houseTransform->qRotation *= Quaternion::CreateFromAxisAngle(Vector3(0,1,0), -1.57f);
	mModelObjects.push_back(house);

	// Campfire
	fx.SetDirectory(L"data/models/fountain");
	shared_ptr<Model> fountainModel(move(Model::CreateFromCMO(pD3dGraphicsObj->GetDevice(), L"data/models/fountain/Fountain.cmo", fx, false)));
	for (int i = 0; i < 2; ++i) {
		shared_ptr<ModelGameObject> fountain = unique_ptr<ModelGameObject>(new ModelGameObject(fountainModel));
		auto transform = fountain->transform;
		transform->scale = Vector3(0.003f);
		float xPos = i == 0 ? 2.0f : -2.0f;
		transform->position = Vector3(xPos, 0, 0);
		mModelObjects.push_back(fountain);
	}

	// Terrain
	HeightMap heightMap = HeightmapParser::GenerateFromTGA("data/heightmap1.tga");
	heightMap.SmoothHeights(0.75f);
	heightMap.SmoothHeights(0.75f);
	heightMap.ScaleHeights(0.3f);
	mTerrainObject = TerrainObject::CreateFromHeightMap(heightMap, pD3dGraphicsObj);
}

void Fluid3DScene::Update(float delta) {
	UpdateCamera(delta);
	HandleInput();

	mNumFluidsUpdating = 0;

	//UpdateFirePosition(delta);

	for (auto modelObject : mModelObjects) {
		modelObject->Update();
	}

	const ICamera &camera = *mCamera;
	if (!mPaused) {
		for (auto fluidSim : mSimulations) {
			if (fluidSim->Update(delta, camera)) {
				++mNumFluidsUpdating;
			}
		}
	}
}

bool Fluid3DScene::Render() {
	mNumRenderedFluids = 0;
	const ICamera &camera = *mCamera;
	ID3D11DeviceContext * const context = pD3dGraphicsObj->GetDeviceContext();
	mSkyObject->Render(camera);

	// draw the terrain
	mTerrainObject->Render(camera);
	
	for (auto modelObject : mModelObjects) {
		modelObject->Render(camera, context);
	}

	for (auto volumeRenderer : mVolumeRenderers) {
		bool isVisible = IsRendererVisibleByCamera(volumeRenderer);
		if (isVisible) {
			volumeRenderer->Render(camera);
		}
	}

	return true;
}

void Fluid3DScene::RenderOverlay(std::shared_ptr<DirectX::SpriteBatch> spriteBatch, std::shared_ptr<DirectX::SpriteFont> spriteFont) {
	/*Vector3 camPos;
	mCamera->GetPosition(camPos);
	wstring text = L"Pos X: " + std::to_wstring(camPos.x) + L" Y: " + std::to_wstring(camPos.y) + L" Z: " + std::to_wstring(camPos.z);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,110));

	Quaternion rotQuat;
	mCamera->GetRotationQuaternion(rotQuat);
	text = L"Rot X: " + std::to_wstring(rotQuat.x) + L" Y: " + std::to_wstring(rotQuat.y)
		+ L" Z: " + std::to_wstring(rotQuat.z) + L" W: " + std::to_wstring(rotQuat.w);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,135));*/
}

void Fluid3DScene::UpdateCamera(float delta) {
	bool hasMoved = false;

	if (mAutoCamEnabled) {
		mAutoCameraController->Update(delta);
		hasMoved = true;
	}
	else {
		// Move camera with WASD 
		float forwardAmount = 0.0f;
		float rightAmount = 0.0f;
		const float moveFactor = 3.5f;

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

		hasMoved = (forwardAmount != 0.0f || rightAmount != 0.0f);
		if (hasMoved) {
			mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
		}

		// Rotate camera with mouse button
		if (pInputSystem->IsMouseRightDown()) {
			int xDelta,yDelta;
			float mouseSensitivity = 0.003f;
			pInputSystem->GetMouseDelta(xDelta,yDelta);
			mCamera->AddYawPitchRoll(xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
		}
	}
	if  (hasMoved) {
		// the camera has moved - sort transparent object render order
		SortTransparentObjects(); 
	}
	mCamera->Update();
}

void Fluid3DScene::HandleInput() {
	if (pInputSystem->IsMouseLeftDown() && pInputSystem->IsKeyDown(VK_SHIFT)) {
		HandleMousePicking(true);
	}
	else if (pInputSystem->IsMouseLeftClicked()) {
		HandleMousePicking(false);
	}

	if (pInputSystem->IsKeyClicked('B')) {
		mAutoCamEnabled = !mAutoCamEnabled;
		mAutoCameraController->SetActive(mAutoCamEnabled);
	}

	if (pInputSystem->IsKeyDown(VK_UP)) {
		mAutoCameraController->ModifySpeedFactor(0.001f);
	}
	else if (pInputSystem->IsKeyDown(VK_DOWN)) {
		mAutoCameraController->ModifySpeedFactor(-0.001f);
	}
}

void Fluid3DScene::HandleMousePicking(bool interaction) {
	int posX,posY;
	pInputSystem->GetMousePos(posX,posY);
	Ray ray = mCamera->ScreenPointToRay(Vector2((float)posX,(float)posY));

	float prevMinDistance = FLT_MAX;
	shared_ptr<FluidSimulation> pickedSim(nullptr);
	shared_ptr<VolumeRenderer> pickedRenderer(nullptr);
	for (shared_ptr<FluidSimulation> simulation : mSimulations) {
		float distance;
		shared_ptr<VolumeRenderer> renderer = simulation->IntersectsRay(ray, distance);
		if (renderer) {
			if (distance < prevMinDistance) {
				pickedSim = simulation;
				pickedRenderer = renderer;
			}
		}
	}

	if (interaction && pickedSim != nullptr) {
		pickedSim->FluidInteraction(ray);
	}
	else {
		if (pickedRenderer == pPickedRenderer) {
			return;
		}
		else {
			if (pPickedRenderer != nullptr) {
				pPickedRenderer = nullptr;
				TwRemoveAllVars(mTwBar);
				string command = " '" + barName + "' iconified=true ";
				TwDefine(command.c_str());
			}
			if (pickedRenderer != nullptr) {
				pPickedRenderer = pickedRenderer;
				pickedSim->DisplayInfoOnBar(mTwBar);
				pickedRenderer->DisplayRenderInfoOnBar(mTwBar);
				string command = " '" + barName + "' iconified=false ";
				TwDefine(command.c_str());
			}
		}
	}
}

bool Fluid3DScene::IsRendererVisibleByCamera(std::shared_ptr<VolumeRenderer> &renderer) const {
	DirectX::BoundingFrustum boundingFrustum = mCamera->GetBoundingFrustum();

	// Perform a frustum - bounding box containment test
	const BoundingBox *boundingBox = renderer->bounds->GetBoundingBox();
	ContainmentType cType = boundingFrustum.Contains(*boundingBox);
	return cType != DISJOINT;
}

void Fluid3DScene::SortTransparentObjects() {
	Vector3 camPos;
	mCamera->GetPosition(camPos);
	sort(mVolumeRenderers.begin(), mVolumeRenderers.end(),
		[&](const std::shared_ptr<VolumeRenderer>& first, const std::shared_ptr<VolumeRenderer>& second) {
			Vector3 firstPositon = first->transform->position;
			Vector3 secondPosition = second->transform->position;

			return Vector3::Distance(firstPositon, camPos) > Vector3::Distance(secondPosition, camPos);
	});
}
