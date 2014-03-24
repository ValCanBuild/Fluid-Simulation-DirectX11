/********************************************************************
FluidSimulation.cpp: Encapsulates an object that handles both
simulating and rendering a 3D fluid

Author:	Valentin Hinov
Date: 3/3/2014
*********************************************************************/

#include "FluidSimulation.h"
#include <AntTweakBar.h>
#include "../../objects/VolumeRenderer.h"
#include "../../utilities/FluidCalculation/Fluid3DCalculator.h"
#include "../../utilities/ICamera.h"

using namespace std;
using namespace DirectX;
using namespace Fluid3D;

#define DEFAULT_FRAMES_TO_SKIP 0

FluidSimulation::FluidSimulation() : 
	mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true), mFramesSinceLastProcess(0), mFramesToSkip(DEFAULT_FRAMES_TO_SKIP),
	mTimeSinceProcessStart(0.0f)
{
	FluidSettings fluidSettings;
	fluidSettings.dimensions = Vector3(64.0f,128.0f,64.0f);
	//fluidSettings.constantInputPosition = Vector3(0.5f,0,0);
	mFluidCalculator = unique_ptr<Fluid3DCalculator>(new Fluid3DCalculator(fluidSettings));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(fluidSettings.dimensions)));
}

FluidSimulation::FluidSimulation(FluidSettings fluidSettings) : mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true),
	mFramesSinceLastProcess(0), mFramesToSkip(DEFAULT_FRAMES_TO_SKIP), mTimeSinceProcessStart(0.0f)
{
	mFluidCalculator = unique_ptr<Fluid3DCalculator>(new Fluid3DCalculator(fluidSettings));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(fluidSettings.dimensions)));
}

FluidSimulation::FluidSimulation(unique_ptr<Fluid3DCalculator> fluidCalculator, shared_ptr<VolumeRenderer> volumeRenderer) :
	mFluidCalculator(move(fluidCalculator)), mVolumeRenderer(volumeRenderer), 
	mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true), mFramesToSkip(DEFAULT_FRAMES_TO_SKIP),
	mFramesSinceLastProcess(0), mTimeSinceProcessStart(0.0f)
{

}

FluidSimulation::~FluidSimulation() {
}

bool FluidSimulation::Initialize(_In_ D3DGraphicsObject * d3dGraphicsObj, HWND hwnd) {
	bool result = mVolumeRenderer->Initialize(d3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	result = mFluidCalculator->Initialize(d3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	mVolumeRenderer->SetSourceTexture(mFluidCalculator->GetVolumeTexture());

	return true;
}

bool FluidSimulation::Render(const ICamera &camera) {
	mIsVisible = IsVisibleByCamera(camera);
	if (mIsVisible && mRenderEnabled) {
		mVolumeRenderer->Render(camera);
	}
	return mIsVisible;
}

bool FluidSimulation::Update(float dt) {
	mVolumeRenderer->Update();

	bool canUpdate = mUpdateEnabled;

	if (canUpdate) {
		canUpdate = mFramesToSkip <= 0 || mFramesSinceLastProcess > mFramesToSkip;
	}

	if (canUpdate) {
		mFluidCalculator->Process();
		mFramesSinceLastProcess = 0;
	} 
	else {
		++mFramesSinceLastProcess;
	}

	return canUpdate;
}

void FluidSimulation::DisplayInfoOnBar(TwBar * const pBar) {
	TwAddVarRW(pBar,"Update", TW_TYPE_BOOLCPP, &mUpdateEnabled, nullptr);
	TwAddVarRW(pBar,"Render", TW_TYPE_BOOLCPP, &mRenderEnabled, nullptr);

	FluidSettings *settings = mFluidCalculator->GetFluidSettingsPointer();

	// Add fluid calculator settings
	TwAddVarCB(pBar,"Simulation", GetFluidSettingsTwType(), SetFluidSettings, GetFluidSettings, mFluidCalculator.get(), "");
	TwAddVarRW(pBar,"Input Position", TW_TYPE_DIR3F, &settings->constantInputPosition, "group=Simulation");

	// Add volume renderer settings
	mVolumeRenderer->DisplayRenderInfoOnBar(pBar);

	// Add LOD settings
	TwAddVarRW(pBar,"Frames to skip", TW_TYPE_INT32, &mFramesToSkip, "group=LOD min=0 max=10");
}

bool FluidSimulation::IntersectsRay(const Ray &ray, float &distance) const {
	const BoundingBox *boundingBox = mVolumeRenderer->bounds->GetBoundingBox();
	return ray.Intersects(*boundingBox, distance);
}

std::shared_ptr<VolumeRenderer> FluidSimulation::GetVolumeRenderer() const {
	return mVolumeRenderer;
}

void TW_CALL FluidSimulation::GetFluidSettings(void *value, void *clientData) {
	*static_cast<FluidSettings *>(value) = static_cast<const Fluid3DCalculator *>(clientData)->GetFluidSettings();
}

void TW_CALL FluidSimulation::SetFluidSettings(const void *value, void *clientData) {
	Fluid3DCalculator* fluidCalculator = static_cast<Fluid3DCalculator *>(clientData);
	FluidSettings fluidSettings = *static_cast<const FluidSettings *>(value);
	fluidCalculator->SetFluidSettings(fluidSettings);
}

bool FluidSimulation::IsVisibleByCamera(const ICamera &camera) const {
	DirectX::BoundingFrustum boundingFrustum = camera.GetBoundingFrustum();

	// Perform a frustum - bounding box containment test
	const BoundingBox *boundingBox = mVolumeRenderer->bounds->GetBoundingBox();
	ContainmentType cType = boundingFrustum.Contains(*boundingBox);
	return cType != DISJOINT;
}