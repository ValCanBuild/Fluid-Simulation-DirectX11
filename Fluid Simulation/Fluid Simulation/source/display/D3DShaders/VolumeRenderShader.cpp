/*************************************************************
VolumeRenderShader.cpp: Implementation of the volume render
shader

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/

#include "VolumeRenderShader.h"
#include <VertexTypes.h>
#include "../D3DGraphicsObject.h"
#include "../../objects/Transform.h"

VolumeRenderShader::VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject) : pD3dGraphicsObject(d3dGraphicsObject), pVolumeValuesTexture(nullptr) {
}

VolumeRenderShader::~VolumeRenderShader() {
	pD3dGraphicsObject = nullptr;
	pVolumeValuesTexture = nullptr;
}

ShaderDescription VolumeRenderShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vVolumeRender.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "VolumeRenderVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pVolumeRender.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "VolumeRenderPixelShader";

	shaderDescription.numLayoutElements = DirectX::VertexPositionNormalTexture::InputElementCount;
	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	for (int i = 0; i < shaderDescription.numLayoutElements; ++i) {
		shaderDescription.polygonLayout[i] = DirectX::VertexPositionNormalTexture::InputElements[i];
	}

	return shaderDescription;
}

void VolumeRenderShader::SetVertexBufferValues(const Matrix &wvpMatrix, const Matrix &worldMatrix) const {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexInputBuffer* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mVertexInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetVertexBufferValues function"));
	}

	dataPtr = (VertexInputBuffer*)mappedResource.pData;
	dataPtr->wvpMatrix = wvpMatrix.Transpose();
	dataPtr->worldMatrix = worldMatrix.Transpose();

	context->Unmap(mVertexInputBuffer,0);

	// Set the buffer inside the vertex shader
}

void VolumeRenderShader::SetTransform(const Transform &transform) const {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PixelBufferPerObject* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mPixelBufferPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetTransform function"));
	}

	dataPtr = (PixelBufferPerObject*)mappedResource.pData;
	dataPtr->vScale = transform.scale;
	dataPtr->vTranslate = transform.position;

	context->Unmap(mPixelBufferPerObject,0);
}

void VolumeRenderShader::SetCameraPosition(const Vector3 &camPos) const {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PixelBufferPerFrame* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mPixelBufferPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetCameraPosition function"));
	}

	dataPtr = (PixelBufferPerFrame*)mappedResource.pData;
	dataPtr->vEyePos = camPos;

	context->Unmap(mPixelBufferPerFrame,0);
}

void VolumeRenderShader::SetSmokeProperties(const SmokeProperties &smokeProperties) const {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PixelSmokePropertiesBuffer* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mPixelSmokePropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetSmokeProperties function"));
	}

	dataPtr = (PixelSmokePropertiesBuffer*)mappedResource.pData;
	dataPtr->smokeProperties = smokeProperties;

	context->Unmap(mPixelSmokePropertiesBuffer,0);

	// Set the buffer inside the pixel shader
}

void VolumeRenderShader::BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {
	deviceContext->VSSetConstantBuffers(0,1,&(mVertexInputBuffer.p));
	ID3D11Buffer *const pPixelBuffers[3] = {mPixelBufferPerFrame, mPixelBufferPerObject, mPixelSmokePropertiesBuffer};
	deviceContext->PSSetConstantBuffers(0,3,pPixelBuffers);

	deviceContext->PSSetShaderResources(0,1,&pVolumeValuesTexture);
}

void VolumeRenderShader::ApplySamplers() {
	pD3dGraphicsObject->GetDeviceContext()->PSSetSamplers(0, 1, &(mSampleState.p));
}

bool VolumeRenderShader::SpecificInitialization(ID3D11Device* device) {
	// Create the vertex buffer
	bool result = BuildDynamicBuffer<VertexInputBuffer>(device, &mVertexInputBuffer);
	if (!result) {
		return false;
	}

	// Create the pixel per frame buffer
	result = BuildDynamicBuffer<PixelBufferPerFrame>(device, &mPixelBufferPerFrame);
	if (!result) {
		return false;
	}

	// Create the pixel per object buffer
	result = BuildDynamicBuffer<PixelBufferPerObject>(device, &mPixelBufferPerObject);
	if (!result) {
		return false;
	}

	// Create the pixel smoke properties buffer
	result = BuildDynamicBuffer<PixelSmokePropertiesBuffer>(device, &mPixelSmokePropertiesBuffer);
	if (!result) {
		return false;
	}

	// Setup the sampler description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT hresult = device->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(hresult)) {
		return false;
	}

	return true;
}

void VolumeRenderShader::SetVolumeValuesTexture(ID3D11ShaderResourceView *volumeValues) {
	pVolumeValuesTexture = volumeValues;
}
