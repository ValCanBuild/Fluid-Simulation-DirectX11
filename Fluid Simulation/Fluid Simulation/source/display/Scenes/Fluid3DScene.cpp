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
#include "../../system/ServiceProvider.h"
#include "../../objects/VolumeRenderer.h"
#include "../simulations/FluidSimulation.h"
#include "../../objects/SkyObject.h"
#include "../../objects/TerrainObject.h"
#include "../../objects/ModelGameObject.h"
#include "../../utilities/HeightmapParser.h"

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

Fluid3DScene::Fluid3DScene() : mPaused(false), pInputSystem(nullptr), mNumRenderedFluids(0), mNumFluidsUpdating(0), 
	pPickedSimulation(nullptr) {
	
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
	FluidSettings fluidSettingsSmoke(SMOKE);
	fluidSettingsSmoke.dimensions = Vector3(64,128,64);
	fluidSettingsSmoke.densityDissipation = 0.99f;
	shared_ptr<FluidSimulation> fluidSimulationSmoke(new FluidSimulation(fluidSettingsSmoke));
	shared_ptr<VolumeRenderer> volumeRendererSmoke = fluidSimulationSmoke->GetVolumeRenderer();
	volumeRendererSmoke->transform->scale = Vector3(4,8,4);
	volumeRendererSmoke->transform->position = Vector3(-0.15f,10.08f,6.43f);
	mSimulations.push_back(fluidSimulationSmoke);

	FluidSettings fluidSettingsFire(FIRE);
	fluidSettingsFire.dimensions = Vector3(32,64,32);
	fluidSettingsFire.constantInputPosition = Vector3(0.5f,0.05f,0.5f);
	// two fire simulations
	for (int i = 0; i < 2; ++i) {
		shared_ptr<FluidSimulation> fluidSimulationFire(new FluidSimulation(fluidSettingsFire));
		shared_ptr<VolumeRenderer> volumeRendererFire = fluidSimulationFire->GetVolumeRenderer();
		volumeRendererFire->transform->scale = Vector3(1,2,1);
		float xPos = i == 0 ? 2.0f : -2.0f;
		volumeRendererFire->transform->position = Vector3(xPos,2.76f,0);

		auto smokeProperties = volumeRendererFire->GetSmokeProperties();
		smokeProperties->vSmokeColor = RGBA2Color(40,40,40,255);

		mSimulations.push_back(fluidSimulationFire);
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
	mCamera->SetPosition(0,3.0f,-4.5);
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
		for (auto simulation : mSimulations) {
			if (simulation->Update(delta, camera)) {
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

	for (auto simulation : mSimulations) {
		if (simulation->Render(camera)) {
			++mNumRenderedFluids;
		}
	}

	return true;
}

void Fluid3DScene::RenderOverlay(std::shared_ptr<DirectX::SpriteBatch> spriteBatch, std::shared_ptr<DirectX::SpriteFont> spriteFont) {
	wstring text = L"Fluids Rendered: " + std::to_wstring(mNumRenderedFluids);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,110));

	text = L"Fluids Updating: " + std::to_wstring(mNumFluidsUpdating);
	spriteFont->DrawString(spriteBatch.get(),text.c_str(),XMFLOAT2(10,135));
}

void Fluid3DScene::UpdateCamera(float delta) {
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
