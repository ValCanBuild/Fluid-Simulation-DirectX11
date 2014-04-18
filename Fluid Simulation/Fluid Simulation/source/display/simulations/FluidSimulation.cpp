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
#include "../../utilities/D3DTexture.h"

using namespace std;
using namespace DirectX;
using namespace Fluid3D;

#define UPDATES_BEFORE_LOD 250

static D3DTexture fireTexture;

void InitFireTexture(D3DGraphicsObject * d3dGraphicsObj) {
	fireTexture.Initialize(d3dGraphicsObj->GetDevice(), d3dGraphicsObj->GetDeviceContext(), L"data/FireTransferFunction2.dds");
}

/*FluidSimulation::FluidSimulation() : 
	mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true), mFramesSinceLastProcess(0),
	mFluidUpdatesSinceStart(0)
{
	FluidSettings fluidSettings;
	fluidSettings.dimensions = Vector3(64.0f,128.0f,64.0f);
	mFluidCalculator = unique_ptr<Fluid3DCalculator>(new Fluid3DCalculator(fluidSettings));
	mVolumeRenderer = unique_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(fluidSettings.dimensions)));
}*/

FluidSimulation::FluidSimulation(const FluidSettings &fluidSettings) : mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true),
	mFramesSinceLastProcess(0), mFluidUpdatesSinceStart(0)
{
	mFluidCalculator = make_shared<Fluid3DCalculator>(fluidSettings);
	//mVolumeRenderer = shared_ptr<VolumeRenderer>(new VolumeRenderer(Vector3(fluidSettings.dimensions)));
}

/*FluidSimulation::FluidSimulation(shared_ptr<Fluid3DCalculator> fluidCalculator, shared_ptr<VolumeRenderer> volumeRenderer) :
	mFluidCalculator(fluidCalculator), mVolumeRenderer(volumeRenderer),
	mUpdateEnabled(true), mIsVisible(true), mRenderEnabled(true),
	mFramesSinceLastProcess(0), mFluidUpdatesSinceStart(0)
{

}*/

FluidSimulation::~FluidSimulation() {
}

void FluidSimulation::AddVolumeRenderer(std::shared_ptr<VolumeRenderer> volumeRenderer) {
	mVolumeRenderers.push_back(volumeRenderer);
}

bool FluidSimulation::Initialize(_In_ D3DGraphicsObject * d3dGraphicsObj, HWND hwnd) {
	bool result;

	result = mFluidCalculator->Initialize(d3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	const FluidSettings &settings = mFluidCalculator->GetFluidSettings();
	for (auto volumeRenderer : mVolumeRenderers) {
		result = volumeRenderer->Initialize(d3dGraphicsObj, hwnd, settings.GetFluidType());
		if (!result) {
			return false;
		}

		volumeRenderer->SetSourceTexture(mFluidCalculator->GetVolumeTexture());
		if (settings.GetFluidType() == FIRE) {
			volumeRenderer->SetReactionTexture(mFluidCalculator->GetReactionTexture());
			if (fireTexture.GetTexture() == nullptr) {
				InitFireTexture(d3dGraphicsObj);
			}
			volumeRenderer->SetFireGradientTexture(fireTexture.GetTexture());
		}

		volumeRenderer->Update();
	}

	//mLodController.SetObjectBoundingBox(mVolumeRenderer->bounds->GetBoundingBox());

	return true;
}

bool FluidSimulation::Render(const ICamera &camera) {
	
	for (auto volumeRenderer : mVolumeRenderers) {
		bool isVisible = IsRendererVisibleByCamera(volumeRenderer, camera);
		if (isVisible && mRenderEnabled) {
			//mVolumeRenderer->SetNumRenderSamples(mLodData.numSamples);
			volumeRenderer->Render(camera);
		}
	}
	return true;
}

bool FluidSimulation::Update(float dt, const ICamera &camera) {
	//mLodController.CalculateOverallLOD(camera);
	//mVolumeRenderer->Update();

	bool canUpdate = true;

	// do not do frame skipping until simulation has developed a bit
	/*if (mFluidUpdatesSinceStart < UPDATES_BEFORE_LOD) {
		mFluidUpdatesSinceStart++;
	}
	else {
		if (canUpdate) {
			canUpdate = mLodController.framesToSkip <= 0 || mFramesSinceLastProcess > mLodController.framesToSkip;
		}
	}*/

	if (canUpdate) {
		mFluidCalculator->Process();
		mFramesSinceLastProcess = 0;
	} 
	else {
		++mFramesSinceLastProcess;
	}
	return canUpdate;
}

void FluidSimulation::FluidInteraction(const Ray &ray) {
	/*float distance = 0.0f;
	if (IntersectsRay(ray, distance)) {
		Vector3 localPos = GetLocalIntersectPosition(ray, distance);
		// the value of local pos is anywhere between (-0.5,-0.5,-0.5) to (0.5, 0.5, 0.5)
		// for fluid interaction this needs to be in the range of (0,0,0) to (1,1,1)
		ExtraForce velocityForce;
		velocityForce.position = localPos + Vector3(0.5f);
		velocityForce.radius = 20.0f;
		velocityForce.amount = ray.direction*50.0f;
		mFluidCalculator->AddForce(velocityForce);
	}*/
}

Vector3 FluidSimulation::GetLocalIntersectPosition(const Ray &ray, float distance) const {
	/*Vector3 worldIntersectPos = ray.position + ray.direction * distance;
	Matrix matrix;
	mVolumeRenderer->transform->GetTransformMatrixQuaternion(matrix);
	matrix = matrix.Invert();
	return Vector3::Transform(worldIntersectPos, matrix);*/
	return Vector3(0,0,0);
}

bool FluidSimulation::IntersectsRay(const Ray &ray, float &distance) const {
	//const BoundingBox *boundingBox = mVolumeRenderer->bounds->GetBoundingBox();
	//return ray.Intersects(*boundingBox, distance);
	return false;
}

bool FluidSimulation::IsRendererVisibleByCamera(std::shared_ptr<VolumeRenderer> renderer, const ICamera &camera) const {
	DirectX::BoundingFrustum boundingFrustum = camera.GetBoundingFrustum();

	// Perform a frustum - bounding box containment test
	const BoundingBox *boundingBox = renderer->bounds->GetBoundingBox();
	ContainmentType cType = boundingFrustum.Contains(*boundingBox);
	return cType != DISJOINT;
}

/*std::shared_ptr<VolumeRenderer> FluidSimulation::GetVolumeRenderer() const {
	return mVolumeRenderer;
}*/


//////////////////////////////////////////////////////////////////////////
// ANTTWEAK BAR METHODS
//////////////////////////////////////////////////////////////////////////
void FluidSimulation::DisplayInfoOnBar(TwBar * const pBar) {
	TwAddVarRW(pBar,"Update", TW_TYPE_BOOLCPP, &mUpdateEnabled, nullptr);
	TwAddVarRW(pBar,"Render", TW_TYPE_BOOLCPP, &mRenderEnabled, nullptr);

	FluidSettings *settings = mFluidCalculator->GetFluidSettingsPointer();

	// Add fluid calculator settings
	TwAddVarCB(pBar,"Simulation", settings->GetFluidSettingsTwType(), SetFluidSettings, GetFluidSettings, mFluidCalculator.get(), "");
	TwAddVarRW(pBar,"Input Position", TW_TYPE_DIR3F, &settings->constantInputPosition, "group=Simulation");

	// Add volume renderer settings
	//mVolumeRenderer->DisplayRenderInfoOnBar(pBar);

	// Add LOD settings
	//TwAddVarRW(pBar,"LOD", LODController::GetLODDataTwType(), &mLodController, "");
}

void TW_CALL FluidSimulation::GetFluidSettings(void *value, void *clientData) {
	*static_cast<FluidSettings *>(value) = static_cast<const Fluid3DCalculator *>(clientData)->GetFluidSettings();
}

void TW_CALL FluidSimulation::SetFluidSettings(const void *value, void *clientData) {
	Fluid3DCalculator* fluidCalculator = static_cast<Fluid3DCalculator *>(clientData);
	FluidSettings fluidSettings = *static_cast<const FluidSettings *>(value);
	fluidCalculator->SetFluidSettings(fluidSettings);
}