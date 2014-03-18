/*************************************************************
VolumeRenderShader.cpp: Implementation of the sky shader

Author: Valentin Hinov
Date: 18/03/2014
**************************************************************/

#include "SkyShader.h"
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

	shaderDescription.numLayoutElements = 3;

	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	shaderDescription.polygonLayout[0].SemanticName = "SV_Position";
	shaderDescription.polygonLayout[0].SemanticIndex = 0;
	shaderDescription.polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[0].InputSlot = 0;
	shaderDescription.polygonLayout[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[0].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[1].SemanticName = "NORMAL";
	shaderDescription.polygonLayout[1].SemanticIndex = 0;
	shaderDescription.polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[1].InputSlot = 0;
	shaderDescription.polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[1].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[2].SemanticName = "TEXCOORD";
	shaderDescription.polygonLayout[2].SemanticIndex = 0;
	shaderDescription.polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	shaderDescription.polygonLayout[2].InputSlot = 0;
	shaderDescription.polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[2].InstanceDataStepRate = 0;

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
