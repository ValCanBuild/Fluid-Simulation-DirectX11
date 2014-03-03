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

// Default simulation parameters
#define DIMENSION 80
#define TIME_STEP 0.125f
#define IMPULSE_RADIUS 3.0f
#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
#define JACOBI_ITERATIONS 15
#define VEL_DISSIPATION 0.999f
#define DENSITY_DISSIPATION 0.999f
#define TEMPERATURE_DISSIPATION 0.99f
#define SMOKE_BUOYANCY 1.0f
#define SMOKE_WEIGHT 0.05f
#define AMBIENT_TEMPERATURE 0.0f
#define IMPULSE_TEMPERATURE 1.5f
#define IMPULSE_DENSITY 1.0f

FluidSimulation::FluidSimulation() : pBoundingFrustum(nullptr) {
	mFluidCalculator = unique_ptr<Fluid3DCalculator>(new Fluid3DCalculator(Vector3(DIMENSION)));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(DIMENSION)));
}

FluidSimulation::FluidSimulation(unique_ptr<Fluid3DCalculator> fluidCalculator, shared_ptr<VolumeRenderer> volumeRenderer) :
	mFluidCalculator(move(fluidCalculator)), mVolumeRenderer(volumeRenderer), pBoundingFrustum(nullptr)
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

bool FluidSimulation::Update(float dt) {
	mFluidCalculator->Process();
	mVolumeRenderer->Update();
	return true;
}

void FluidSimulation::DisplayInfoOnBar(const TwBar * const pBar) const {

}

std::shared_ptr<VolumeRenderer> FluidSimulation::GetVolumeRenderer() const {
	return mVolumeRenderer;
}
