/********************************************************************
VolumeRenderer.cpp: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#include "VolumeRenderer.h"
#include <CommonStates.h>
#include "../system/ServiceProvider.h"
#include "D3DShaders/VolumeRenderShader.h"
#include "D3DShaders/ShaderParams.h"
#include "../utilities/Camera.h"

using namespace std;
using namespace DirectX;

VolumeRenderer::VolumeRenderer(Vector3 &volumeSize, Vector3 &position) :
 mVolumeSize(volumeSize), pD3dGraphicsObj(nullptr) {
	 mTransform.position = position;
	 mTransform.scale = Vector3(1.0f);
}

VolumeRenderer::~VolumeRenderer() {
	pD3dGraphicsObj = nullptr;
}

bool VolumeRenderer::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	mVolumeRenderShader = unique_ptr<VolumeRenderShader>(new VolumeRenderShader(d3dGraphicsObj));
	bool result = mVolumeRenderShader->Initialize(d3dGraphicsObj->GetDevice(), hwnd);
	if (!result) {
		return false;
	}

	mVolumeBox = GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true);

	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();

	return true;
}

void VolumeRenderer::Render(ID3D11ShaderResourceView * sourceTexSRV, Camera *camera, const Matrix* viewMatrix, const Matrix* projMatrix) {
	Matrix objectMatrix;
	mTransform.GetTransformMatrixQuaternion(objectMatrix);

	Vector3 camPos;
	camera->GetPosition(camPos);

	Matrix wvpMatrix = objectMatrix*(*viewMatrix)*(*projMatrix);
	mVolumeRenderShader->SetVertexBufferValues(wvpMatrix, objectMatrix);
	mVolumeRenderShader->SetPixelBufferValues(mTransform, camPos, mVolumeSize, sourceTexSRV);

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	mVolumeBox->Draw(mVolumeRenderShader.get(), mVolumeRenderShader->GetInputLayout(), false, false, [=] 
		{
			ID3D11BlendState* blendState = pCommonStates->NonPremultiplied();
			ID3D11RasterizerState* rasterizeState = pCommonStates->CullClockwise();

			context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
			context->RSSetState(rasterizeState);
		}
	);
}

void VolumeRenderer::SetPosition(Vector3 &position) {
	mTransform.position = position;
}