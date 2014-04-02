/********************************************************************
VolumeRenderer.cpp: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#include "VolumeRenderer.h"
#include <AntTweakBar.h>
#include <CommonStates.h>
#include "../system/ServiceProvider.h"
#include "../display/D3DShaders/ShaderParams.h"
#include "../utilities/ICamera.h"
#include "../utilities/FluidCalculation/FluidSettings.h"

using namespace std;
using namespace DirectX;

static Color defaultSmokeColor = RGBA2Color(200,193,193,255);
static float defaultSmokeAbsorption = 60.0f;
static float defaultFireAbsorption = 40.0f;
static int   defaultNumSamples = 64;

TwType smokePropertiesTwType;
TwType firePropertiesTwType;

void DefinePropertiesTwType() {
	TwStructMember smokePropertiesStructMembers[] = {
		{ "Number of Samples", TW_TYPE_INT32, offsetof(SmokeProperties, iNumSamples), "min=16 max=512 step=1" },
		{ "Smoke Color", TW_TYPE_COLOR4F, offsetof(SmokeProperties, vSmokeColor), "" },
		{ "Smoke Absorption", TW_TYPE_FLOAT, offsetof(SmokeProperties, fSmokeAbsorption), "min=0.0 max=200.0 step=0.5" },
		{ "Fire Absorption", TW_TYPE_FLOAT, offsetof(SmokeProperties, fFireAbsorption), "min=0.0 max=200.0 step=0.5" }
	};

	smokePropertiesTwType = TwDefineStruct("Smoke Render Properties", smokePropertiesStructMembers, 3, sizeof(SmokeProperties), nullptr, nullptr);
	firePropertiesTwType = TwDefineStruct("Fire Render Properties", smokePropertiesStructMembers, 4, sizeof(SmokeProperties), nullptr, nullptr);
}

VolumeRenderer::VolumeRenderer(const Vector3 &volumeSize) :
	mVolumeSize(volumeSize), 
	pD3dGraphicsObj(nullptr) 
{
	mSmokeProperties = unique_ptr<SmokeProperties>(new SmokeProperties(defaultSmokeColor, defaultSmokeAbsorption, defaultFireAbsorption, defaultNumSamples));
}

VolumeRenderer::~VolumeRenderer() {
	pD3dGraphicsObj = nullptr;
}

bool VolumeRenderer::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd, const FluidType_t &fluidType) {
	pD3dGraphicsObj = d3dGraphicsObj;
	mFluidType = fluidType;

	primitive = GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, false);

	switch (mFluidType){
	case FIRE:
		mVolumeRenderShader = unique_ptr<SmokeRenderShader>(new FireRenderShader(d3dGraphicsObj));
		break;
	case SMOKE:
		mVolumeRenderShader = unique_ptr<SmokeRenderShader>(new SmokeRenderShader(d3dGraphicsObj));
		break;
	}

	bool result = mVolumeRenderShader->Initialize(d3dGraphicsObj->GetDevice(), hwnd);
	if (!result) {
		return false;
	}
	
	mVolumeRenderShader->SetSmokeProperties(*mSmokeProperties);
	mVolumeRenderShader->SetTransform(*transform);

	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();

	if (smokePropertiesTwType == TW_TYPE_UNDEF) {
		DefinePropertiesTwType();
	}

	return true;
}

void VolumeRenderer::Render(const ICamera &camera) {
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);

	Vector3 camPos;
	camera.GetPosition(camPos);

	Matrix wvpMatrix = objectMatrix*camera.GetViewProjectionMatrix();
	mVolumeRenderShader->SetVertexBufferValues(wvpMatrix, objectMatrix);
	//mVolumeRenderShader->SetTransform(*transform);

	if (mPrevCameraPos != camPos) {
		mVolumeRenderShader->SetCameraPosition(camPos);
		mPrevCameraPos = camPos;
	}

	auto context = pD3dGraphicsObj->GetDeviceContext();
	primitive->Draw(mVolumeRenderShader.get(), mVolumeRenderShader->GetInputLayout(), false, false, [=] 
		{
			ID3D11BlendState* blendState = pCommonStates->NonPremultiplied();
			ID3D11RasterizerState* rasterizeState = pCommonStates->CullClockwise();

			context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
			context->RSSetState(rasterizeState);
		}
	);

	ID3D11ShaderResourceView *const pSRVNULL[2] = {nullptr, nullptr};
	context->PSSetShaderResources(0, 2, pSRVNULL);
}

void VolumeRenderer::SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV) {
	mVolumeRenderShader->SetVolumeValuesTexture(sourceTexSRV);
}

void VolumeRenderer::SetReactionTexture(ID3D11ShaderResourceView *reactionTexSRV) {
	auto fireRenderShader = static_cast<FireRenderShader*>(mVolumeRenderShader.get());
	fireRenderShader->SetReactionValuesTexture(reactionTexSRV);
}

void VolumeRenderer::SetFireGradientTexture(ID3D11ShaderResourceView *gradientTexSRV) {
	auto fireRenderShader = static_cast<FireRenderShader*>(mVolumeRenderShader.get());
	fireRenderShader->SetFireGradientTexture(gradientTexSRV);
}

void VolumeRenderer::DisplayRenderInfoOnBar(TwBar * const pBar) {
	TwType typeToAdd = mFluidType == SMOKE ? smokePropertiesTwType : firePropertiesTwType;
	TwAddVarRW(pBar,"Rendering", typeToAdd, mSmokeProperties.get(), "");
	TwAddButton(pBar, "Apply Changes", SetSmokePropertiesCallback, this, "label='Apply Changes' group=Rendering");
}

void VolumeRenderer::RefreshSmokeProperties() {
	mVolumeRenderShader->SetSmokeProperties(*mSmokeProperties);
}

void __stdcall VolumeRenderer::SetSmokePropertiesCallback(void *clientData) {
	static_cast<VolumeRenderer *>(clientData)->RefreshSmokeProperties();
}

std::shared_ptr<SmokeProperties> VolumeRenderer::GetSmokeProperties() const {
	return mSmokeProperties;
}

void VolumeRenderer::SetNumRenderSamples(int numSamples) {
	if (numSamples != mSmokeProperties->iNumSamples) {
		mSmokeProperties->iNumSamples = numSamples;
		RefreshSmokeProperties();
	}
}
