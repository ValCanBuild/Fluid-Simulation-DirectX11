/*************************************************************
VolumeRenderShader.cpp: Implementation of the volume render
shader

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/

#include "VolumeRenderShader.h"
#include "../D3DGraphicsObject.h"
#include "../../objects/Transform.h"

VolumeRenderShader::VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject) : pD3dGraphicsObject(d3dGraphicsObject) {
}

VolumeRenderShader::~VolumeRenderShader() {
	pD3dGraphicsObject = nullptr;
}

ShaderDescription VolumeRenderShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vVolumeRender.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "VolumeRenderVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pVolumeRender.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "VolumeRenderPixelShader";

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

void VolumeRenderShader::SetVertexBufferValues(Matrix &wvpMatrix, Matrix &worldMatrix) const {
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
	context->VSSetConstantBuffers(0,1,&(mVertexInputBuffer.p));
}

void VolumeRenderShader::SetPixelBufferValues(Transform &transform, Vector3 &vEyePos, Vector3 &vDimensions, ID3D11ShaderResourceView* volumeValues) const {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PixelInputBuffer* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mPixelInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetPixelBufferValues function"));
	}

	dataPtr = (PixelInputBuffer*)mappedResource.pData;
	dataPtr->vDimensions = vDimensions;
	dataPtr->vScale = transform.scale;
	dataPtr->vTranslate = transform.position;
	dataPtr->vEyePos = vEyePos;
	dataPtr->padding0 = 0.0f;
	dataPtr->padding1 = 0.0f;
	dataPtr->padding2 = 0.0f;
	dataPtr->padding3 = 0.0f;

	context->Unmap(mPixelInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mPixelInputBuffer.p));

	context->PSSetShaderResources(0,1,&volumeValues);
}


bool VolumeRenderShader::SpecificInitialization(ID3D11Device* device) {
	// Create the vertex buffer
	D3D11_BUFFER_DESC vertexInputBufferDesc;
	vertexInputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexInputBufferDesc.ByteWidth = sizeof(VertexInputBuffer);
	vertexInputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexInputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexInputBufferDesc.MiscFlags = 0;
	vertexInputBufferDesc.StructureByteStride = 0;
	// General buffer
	HRESULT hresult = device->CreateBuffer(&vertexInputBufferDesc, NULL, &mVertexInputBuffer);
	if(FAILED(hresult)) {
		return false;
	}

	// Create the pixel buffer
	D3D11_BUFFER_DESC pixelInputBufferDesc;
	pixelInputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelInputBufferDesc.ByteWidth = sizeof(PixelInputBuffer);
	pixelInputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelInputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelInputBufferDesc.MiscFlags = 0;
	pixelInputBufferDesc.StructureByteStride = 0;
	// General buffer
	hresult = device->CreateBuffer(&pixelInputBufferDesc, NULL, &mPixelInputBuffer);
	if(FAILED(hresult)) {
		return false;
	}

	return true;
}