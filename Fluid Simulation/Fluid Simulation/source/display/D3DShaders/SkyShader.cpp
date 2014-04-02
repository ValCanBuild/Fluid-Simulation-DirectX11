/*************************************************************
VolumeRenderShader.cpp: Implementation of the sky shader

Author: Valentin Hinov
Date: 18/03/2014
**************************************************************/

#include "SkyShader.h"
#include <VertexTypes.h>
#include "../D3DGraphicsObject.h"

SkyShader::SkyShader(const D3DGraphicsObject * const d3dGraphicsObject) : pD3dGraphicsObject(d3dGraphicsObject), pSkyTexture(nullptr) {
}

SkyShader::~SkyShader() {
	pD3dGraphicsObject = nullptr;
	pSkyTexture = nullptr;
}

ShaderDescription SkyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vSkymap.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "SkymapVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pSkymap.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "SkymapPixelShader";

	shaderDescription.numLayoutElements = DirectX::VertexPositionNormalTexture::InputElementCount;
	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	for (int i = 0; i < shaderDescription.numLayoutElements; ++i) {
		shaderDescription.polygonLayout[i] = DirectX::VertexPositionNormalTexture::InputElements[i];
	}

	return shaderDescription;
}

void SkyShader::SetVertexBufferValues(Matrix &wvpMatrix, Matrix &worldMatrix) const {
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

void SkyShader::BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {
	deviceContext->VSSetConstantBuffers(0,1,&(mVertexInputBuffer.p));

	deviceContext->PSSetShaderResources(0,1,&pSkyTexture);
}

bool SkyShader::SpecificInitialization(ID3D11Device* device) {
	// Create the vertex buffer
	bool result = BuildDynamicBuffer<VertexInputBuffer>(device, &mVertexInputBuffer);
	if (!result) {
		return false;
	}

	return true;
}

void SkyShader::SetSkyCubeTexture(ID3D11ShaderResourceView *skyTexture) {
	pSkyTexture = skyTexture;
}
