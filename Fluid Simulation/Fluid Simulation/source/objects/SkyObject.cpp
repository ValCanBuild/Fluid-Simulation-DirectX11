/********************************************************************
SkyObject: Represents a primitive object to be used a skysphere

Author:	Valentin Hinov
Date: 18/3/2014
*********************************************************************/

#include "SkyObject.h"
#include <CommonStates.h>
#include "../system/ServiceProvider.h"
#include "../utilities/ICamera.h"
#include "../utilities/D3DTexture.h"
#include "../display/D3DShaders/SkyShader.h"

using namespace DirectX;

SkyObject::SkyObject() : pD3dGraphicsObj(nullptr) {

}

SkyObject::~SkyObject() {
	pD3dGraphicsObj = nullptr;
}

bool SkyObject::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, _In_ WCHAR *texturePath,  HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	primitive = GeometricPrimitive::CreateSphere(pD3dGraphicsObj->GetDeviceContext(), 1.0f, 16U, false);
	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();

	mSkyTexture = unique_ptr<D3DTexture>(new D3DTexture());
	bool result = mSkyTexture->Initialize(pD3dGraphicsObj->GetDevice(), pD3dGraphicsObj->GetDeviceContext(), texturePath, hwnd);
	if (!result) {
		return false;
	}

	mSkyShader = unique_ptr<SkyShader>(new SkyShader(d3dGraphicsObj));
	result = mSkyShader->Initialize(pD3dGraphicsObj->GetDevice(), hwnd);
	if (!result) {
		return false;
	}
	mSkyShader->SetSkyCubeTexture(mSkyTexture->GetTexture());

	return true;
}

void SkyObject::Render(const ICamera &camera) {
	camera.GetPosition(transform->position);
	Matrix objectMatrix;
	transform->GetTransformMatrixQuaternion(objectMatrix);

	Matrix wvpMatrix = objectMatrix*camera.GetViewMatrix()*camera.GetProjectionMatrix();
	mSkyShader->SetVertexBufferValues(wvpMatrix, objectMatrix);

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();
	primitive->Draw(mSkyShader.get(), mSkyShader->GetInputLayout(), false, false, [=] 
	{
		ID3D11RasterizerState* rasterizeState = pCommonStates->CullNone();
		context->RSSetState(rasterizeState);
	}
	);
}
