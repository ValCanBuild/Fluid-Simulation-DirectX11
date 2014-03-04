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

	// Add fluid calculator settings
	FluidSettings &fluidSettings = mFluidCalculator->fluidSettings;

	TwAddVarRW(pBar,"Active", TW_TYPE_BOOLCPP, &mUpdatePaused, nullptr);

	string groupName = "group=Simulation";

	TwAddVarRO(pBar,"Dimensions", TW_TYPE_DIR3F, &fluidSettings.dimensions, groupName.c_str());
	TwAddVarRW(pBar,"MacCormarck Advection", TW_TYPE_BOOLCPP, &fluidSettings.macCormackEnabled, groupName.c_str());
	TwAddVarRW(pBar,"Time Step", TW_TYPE_FLOAT, &fluidSettings.timeStep, ("min=0.0 max=1.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Jacobi Iterations", TW_TYPE_INT32, &fluidSettings.jacobiIterations, ("min=1 max=50 step=1 " + groupName).c_str());
	TwAddVarRW(pBar,"Velocity Dissipation", TW_TYPE_FLOAT, &fluidSettings.velocityDissipation, ("min=0.0 max=1.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Temperature Dissipation", TW_TYPE_FLOAT, &fluidSettings.temperatureDissipation, ("min=0.0 max=1.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Constant Temperature", TW_TYPE_FLOAT, &fluidSettings.constantTemperature, ("min=0.0 max=100.0 step=0.01 " + groupName).c_str());
	TwAddVarRW(pBar,"Density Dissipation", TW_TYPE_FLOAT, &fluidSettings.densityDissipation, ("min=0.0 max=1.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Constant Density", TW_TYPE_FLOAT, &fluidSettings.constantDensityAmount, ("min=0.0 max=100.0 step=0.01 " + groupName).c_str());
	TwAddVarRW(pBar,"Density Weight", TW_TYPE_FLOAT, &fluidSettings.densityWeight, ("min=0.05 max=10.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Density Buoyancy", TW_TYPE_FLOAT, &fluidSettings.densityBuoyancy, ("min=0.0 max=100.0 step=0.001 " + groupName).c_str());
	TwAddVarRW(pBar,"Input Radius", TW_TYPE_FLOAT, &fluidSettings.constantInputRadius, ("min=0.05 max=10.0 step=0.1 "  + groupName).c_str());
	TwAddVarRW(pBar,"Input Position", TW_TYPE_DIR3F, &fluidSettings.constantInputPosition, groupName.c_str());

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