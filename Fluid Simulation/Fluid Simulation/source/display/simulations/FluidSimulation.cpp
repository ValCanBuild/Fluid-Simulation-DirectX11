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
#include "../../utilities/Camera.h"

using namespace std;
using namespace DirectX;
using namespace Fluid3D;

FluidSimulation::FluidSimulation() : pBoundingFrustum(nullptr), mUpdatePaused(false) {
	FluidSettings fluidSettings;
	mFluidCalculator = unique_ptr<Fluid3DCalculator>(new Fluid3DCalculator(fluidSettings));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(fluidSettings.dimensions)));
}

FluidSimulation::FluidSimulation(unique_ptr<Fluid3DCalculator> fluidCalculator, shared_ptr<VolumeRenderer> volumeRenderer) :
	mFluidCalculator(move(fluidCalculator)), mVolumeRenderer(volumeRenderer), pBoundingFrustum(nullptr), mUpdatePaused(false)
{

}

FluidSimulation::~FluidSimulation() {
	pBoundingFrustum = nullptr;
}

bool FluidSimulation::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd, Camera *camera) {
	bool result = mVolumeRenderer->Initialize(d3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	result = mFluidCalculator->Initialize(d3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	mVolumeRenderer->SetSourceTexture(mFluidCalculator->GetVolumeTexture());
	mVolumeRenderer->SetCamera(camera);

	pBoundingFrustum = camera->GetBoundingFrustum();

	return true;
}

bool FluidSimulation::Render(const Matrix &viewMatrix, const Matrix &projectionMatrix) const {
	// Perform a frustum - bounding box containment test
	const BoundingBox *boundingBox = mVolumeRenderer->bounds->GetBoundingBox();
	ContainmentType cType = pBoundingFrustum->Contains(*boundingBox);
	if (cType != DISJOINT) {
		mVolumeRenderer->Render(viewMatrix, projectionMatrix);
		return true;
	}
	return false;
}

bool FluidSimulation::Update(float dt) const {
	mVolumeRenderer->Update();

	if (!mUpdatePaused) {
		mFluidCalculator->Process();
	}

	return !mUpdatePaused;
}

void FluidSimulation::DisplayInfoOnBar(TwBar * const pBar) {
	TwAddVarRW(pBar,"Active", TW_TYPE_BOOLCPP, &mUpdatePaused, nullptr);

	// Add fluid calculator settings
	TwAddVarCB(pBar,"Simulation", GetFluidSettingsTwType(), SetFluidSettings, GetFluidSettings, mFluidCalculator.get(), "");

	// Add volume renderer settings
	mVolumeRenderer->DisplayRenderInfoOnBar(pBar);
}

bool FluidSimulation::IntersectsRay(Ray &ray, float &distance) const {
	const BoundingBox *boundingBox = mVolumeRenderer->bounds->GetBoundingBox();
	return ray.Intersects(*boundingBox, distance);
}

std::shared_ptr<VolumeRenderer> FluidSimulation::GetVolumeRenderer() const {
	return mVolumeRenderer;
}

void TW_CALL FluidSimulation::GetFluidSettings(void *value, void *clientData) {
	Fluid3DCalculator* fluidCalculator = static_cast<Fluid3DCalculator *>(clientData);
	*static_cast<FluidSettings *>(value) = fluidCalculator->GetFluidSettings();
}

void TW_CALL FluidSimulation::SetFluidSettings(const void *value, void *clientData) {
	Fluid3DCalculator* fluidCalculator = static_cast<Fluid3DCalculator *>(clientData);
	FluidSettings fluidSettings = *static_cast<const FluidSettings *>(value);
	fluidCalculator->SetFluidSettings(fluidSettings);
}
