/*************************************************************
D3DFrameBuffer.cpp: Specific implementation of a Frame Buffer
to be used with the Direct3D API

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#include "D3DFrameBuffer.h"
#include "D3DGraphicsObject.h"

D3DFrameBuffer::D3DFrameBuffer() {
	pD3dGraphicsObject = nullptr;
}

D3DFrameBuffer::~D3DFrameBuffer() {
	pD3dGraphicsObject = nullptr;
}

bool D3DFrameBuffer::Initialize(IGraphicsObject* graphicsObject, int textureWidth, int textureHeight) {

	pD3dGraphicsObject = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	
	D3D11_TEXTURE2D_DESC textureDesc;

	ZeroMemory(&textureDesc,sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT result = pD3dGraphicsObject->GetDevice()->CreateTexture2D(&textureDesc,NULL, &(mRenderTargetTexture._Myptr));
	if (FAILED(result)) {
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = pD3dGraphicsObject->GetDevice()->CreateRenderTargetView(mRenderTargetTexture.get(), &renderTargetViewDesc, &(mRenderTargetView._Myptr));
	if (FAILED(result)) {
		return false;
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = pD3dGraphicsObject->GetDevice()->CreateShaderResourceView(mRenderTargetTexture.get(), &shaderResourceViewDesc, &mShaderResourceView._Myptr);
	if(FAILED(result)) {
		return false;
	}

	return true;
}

void D3DFrameBuffer::BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha) const {
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	context->OMSetRenderTargets(1, &(mRenderTargetView._Myptr), pD3dGraphicsObject->GetDepthStencilView());

	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = clearRed;
	color[1] = clearGreen;
	color[2] = clearBlue;
	color[3] = clearAlpha;

	// Clear the back buffer.
	context->ClearRenderTargetView(mRenderTargetView.get(), color);
    
	// Clear the depth buffer.
	context->ClearDepthStencilView(pD3dGraphicsObject->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DFrameBuffer::EndRender() const {
	pD3dGraphicsObject->SetBackBufferRenderTarget();
}

// Return a void pointer to the underlying texture resource - child class
const void* D3DFrameBuffer::GetTextureResource() const {
	return mShaderResourceView.get();
}