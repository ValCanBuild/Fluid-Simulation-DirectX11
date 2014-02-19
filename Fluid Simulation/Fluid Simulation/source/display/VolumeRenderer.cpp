/********************************************************************
VolumeRenderer.cpp: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#include "VolumeRenderer.h"
#include "D3DShaders/VolumeRenderShader.h"
#include "D3DShaders/ShaderParams.h"
#include "../objects/D2DTexQuad.h"

using namespace std;

VolumeRenderer::VolumeRenderer(Vector3 &volumeSize, Vector3 &position) :
 mVolumeSize(volumeSize), mPosition(position), pD3dGraphicsObj(nullptr) {

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

	result = InitRenderResult(hwnd);
	if (!result) {
		return false;
	}

	result = InitializeRenderQuad(hwnd);
	if (!result) {
		return false;
	}

	return true;
}

bool VolumeRenderer::InitRenderResult(HWND hwnd) {
	int width, height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);

	mRenderResult = unique_ptr<ShaderParams>(new ShaderParams());
	CComPtr<ID3D11Texture2D> renderTexture;
	D3D11_TEXTURE2D_DESC renderTextureDesc;
	ZeroMemory(&renderTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	renderTextureDesc.Width = width;
	renderTextureDesc.Height = height;
	renderTextureDesc.ArraySize = 1;
	renderTextureDesc.MipLevels = 1;
	renderTextureDesc.SampleDesc.Count = 1;
	renderTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	renderTextureDesc.CPUAccessFlags = 0;
	renderTextureDesc.MiscFlags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC renderSrvDesc;
	renderSrvDesc.Format = renderTextureDesc.Format;
	renderSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderSrvDesc.Texture2D.MostDetailedMip = 0;
	renderSrvDesc.Texture2D.MipLevels = 1;

	D3D11_UNORDERED_ACCESS_VIEW_DESC renderUavDesc;
	renderUavDesc.Format = renderTextureDesc.Format;
	renderUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	renderUavDesc.Texture2D.MipSlice = 0;

	HRESULT hresult = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&renderTextureDesc, NULL, &renderTexture);
	if (FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render Texture Object", L"Error", MB_OK);
		return false;
	}
	hresult = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(renderTexture, &renderSrvDesc, &mRenderResult->mSRV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render SRV", L"Error", MB_OK);
		return false;
	}
	hresult = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(renderTexture, &renderUavDesc, &mRenderResult->mUAV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render UAV", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool VolumeRenderer::InitializeRenderQuad(HWND hwnd) {
	mTexQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	return mTexQuad->Initialize(pD3dGraphicsObj, hwnd);
}

bool VolumeRenderer::Render(ID3D11ShaderResourceView * sourceTexSRV, Camera *camera, float zoom, const Matrix* viewMatrix, const Matrix* projMatrix) {
	mVolumeRenderShader->SetDynamicBufferValues(camera, zoom, mVolumeSize);
	mVolumeRenderShader->Compute(sourceTexSRV,mRenderResult->mUAV);

	mTexQuad->SetTexture(mRenderResult->mSRV);
	return mTexQuad->Render(viewMatrix, projMatrix);
}

void VolumeRenderer::SetPosition(Vector3 &position) {
	mPosition = position;
}