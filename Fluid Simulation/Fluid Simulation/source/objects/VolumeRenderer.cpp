/********************************************************************
VolumeRenderer.cpp: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#include "VolumeRenderer.h"
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

static std::unique_ptr<VolumeRenderShader>	sharedVolumeRenderShader;

VolumeRenderer::VolumeRenderer(Vector3 &volumeSize) :
	mVolumeSize(volumeSize), 
	pD3dGraphicsObj(nullptr) 
{
	 
}

VolumeRenderer::~VolumeRenderer() {
	pD3dGraphicsObj = nullptr;
}

bool VolumeRenderer::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	primitive = GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, false);

	if (sharedVolumeRenderShader == nullptr) {
		sharedVolumeRenderShader = unique_ptr<VolumeRenderShader>(new VolumeRenderShader(d3dGraphicsObj));
		bool result = sharedVolumeRenderShader->Initialize(d3dGraphicsObj->GetDevice(), hwnd);
		if (!result) {
			return false;
		}
	}
	sharedVolumeRenderShader->SetSmokeProperties(defaultSmokeColor, defaultSmokeAbsorption, defaultNumSamples);

	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();

	return true;
}

void VolumeRenderer::Render(const Matrix &viewMatrix, const Matrix &projectionMatrix) {
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);

	Vector3 camPos;
	pCamera->GetPosition(camPos);

	Matrix wvpMatrix = objectMatrix*viewMatrix*projectionMatrix;
	sharedVolumeRenderShader->SetVertexBufferValues(wvpMatrix, objectMatrix);

	if (mPrevCameraPos != camPos) {
		sharedVolumeRenderShader->SetPixelBufferValues(*transform, camPos, mVolumeSize, pSourceTexSRV);
		mPrevCameraPos = camPos;
	}

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();
	primitive->Draw(sharedVolumeRenderShader.get(), sharedVolumeRenderShader->GetInputLayout(), false, false, [=] 
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
	pSourceTexSRV = sourceTexSRV;
}

void VolumeRenderer::SetCamera(Camera *camera) {
	pCamera = camera;
}