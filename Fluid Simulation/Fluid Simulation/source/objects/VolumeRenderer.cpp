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
#include "../utilities/Camera.h"
#include "../display/D3DShaders/VolumeRenderShader.h"

using namespace std;
using namespace DirectX;

static Color defaultSmokeColor = Color(0.74f, 0.0f, 0.99f, 1.0f);
static float defaultSmokeAbsorption = 60.0f;
static int   defaultNumSamples = 64;

TwType smokePropertiesTwType;

void DefineSmokePropertiesTwType() {
	TwStructMember smokePropertiesStructMembers[] = {
		{ "Smoke Color", TW_TYPE_COLOR4F, offsetof(SmokeProperties, vSmokeColor), "" },
		{ "Smoke Absorption", TW_TYPE_FLOAT, offsetof(SmokeProperties, fSmokeAbsorption), "min=0.0 max=200.0 step=0.5" },
		{ "Number of Samples", TW_TYPE_INT32, offsetof(SmokeProperties, iNumSamples), "min=16 max=512 step=1" }
	};

	smokePropertiesTwType = TwDefineStruct("Render Properties", smokePropertiesStructMembers, 3, sizeof(SmokeProperties), nullptr, nullptr);
}

VolumeRenderer::VolumeRenderer(Vector3 &volumeSize) :
	mVolumeSize(volumeSize), 
	pD3dGraphicsObj(nullptr) 
{
	 
}

VolumeRenderer::~VolumeRenderer() {
	pD3dGraphicsObj = nullptr;
	pCamera = nullptr;
}

bool VolumeRenderer::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	primitive = GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, false);

	mVolumeRenderShader = unique_ptr<VolumeRenderShader>(new VolumeRenderShader(d3dGraphicsObj));
	bool result = mVolumeRenderShader->Initialize(d3dGraphicsObj->GetDevice(), hwnd);
	if (!result) {
		return false;
	}
	
	mSmokeProperties = unique_ptr<SmokeProperties>(new SmokeProperties(defaultSmokeColor, defaultSmokeAbsorption, defaultNumSamples));

	mVolumeRenderShader->SetSmokeProperties(*mSmokeProperties);

	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();

	if (smokePropertiesTwType == TW_TYPE_UNDEF) {
		DefineSmokePropertiesTwType();
	}

	return true;
}

void VolumeRenderer::Render(const Matrix &viewMatrix, const Matrix &projectionMatrix) {
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);

	Vector3 camPos;
	pCamera->GetPosition(camPos);

	Matrix wvpMatrix = objectMatrix*viewMatrix*projectionMatrix;
	mVolumeRenderShader->SetVertexBufferValues(wvpMatrix, objectMatrix);

	if (mPrevCameraPos != camPos) {
		mVolumeRenderShader->SetPixelBufferValues(*transform, camPos, mVolumeSize);
		mPrevCameraPos = camPos;
	}

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();
	primitive->Draw(mVolumeRenderShader.get(), mVolumeRenderShader->GetInputLayout(), false, false, [=] 
		{
			// set custom sampler
			//sharedVolumeRenderShader->ApplySamplers();

			ID3D11BlendState* blendState = pCommonStates->NonPremultiplied();
			ID3D11RasterizerState* rasterizeState = pCommonStates->CullClockwise();
			
			context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
			context->RSSetState(rasterizeState);
		}
	);
}

void VolumeRenderer::SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV) {
	mVolumeRenderShader->SetVolumeValuesTexture(sourceTexSRV);
}

void VolumeRenderer::SetCamera(Camera *camera) {
	pCamera = camera;
}

void VolumeRenderer::DisplayRenderInfoOnBar(TwBar * const pBar) {
	TwAddVarRW(pBar,"Rendering", smokePropertiesTwType, mSmokeProperties.get(), "");
	TwAddButton(pBar, "Update", SetSmokePropertiesCallback, this, "label='Apply Changes' group=Rendering");
}

void VolumeRenderer::RefreshSmokeProperties() {
	mVolumeRenderShader->SetSmokeProperties(*mSmokeProperties);
}

void __stdcall VolumeRenderer::SetSmokePropertiesCallback(void *clientData) {
	static_cast<VolumeRenderer *>(clientData)->RefreshSmokeProperties();
}