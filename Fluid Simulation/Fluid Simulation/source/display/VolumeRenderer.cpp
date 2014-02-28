/********************************************************************
VolumeRenderer.cpp: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#include "VolumeRenderer.h"
#include <CommonStates.h>
#include "../system/ServiceProvider.h"
#include "D3DShaders/ShaderParams.h"
#include "../utilities/Camera.h"
#include "D3DShaders/VolumeRenderShader.h"

using namespace std;
using namespace DirectX;

static std::unique_ptr<VolumeRenderShader>	sharedVolumeRenderShader;

VolumeRenderer::VolumeRenderer(ID3D11DeviceContext *pContext, Vector3 &volumeSize) :
	PrimitiveGameObject(GeometricPrimitive::CreateCube(pContext, 1.0f, true)),
	mVolumeSize(volumeSize), 
	pD3dGraphicsObj(nullptr) 
{
	 
}

VolumeRenderer::~VolumeRenderer() {
	pD3dGraphicsObj = nullptr;
}

bool VolumeRenderer::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	if (sharedVolumeRenderShader == nullptr) {
		sharedVolumeRenderShader = unique_ptr<VolumeRenderShader>(new VolumeRenderShader(d3dGraphicsObj));
		bool result = sharedVolumeRenderShader->Initialize(d3dGraphicsObj->GetDevice(), hwnd);
		if (!result) {
			return false;
		}
	}

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
	sharedVolumeRenderShader->SetPixelBufferValues(*transform, camPos, mVolumeSize, pSourceTexSRV);

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	primitive->Draw(sharedVolumeRenderShader.get(), sharedVolumeRenderShader->GetInputLayout(), false, false, [=] 
		{
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