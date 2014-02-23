/*************************************************************
VolumeRenderShader.cpp: Implementation of the volume render
shader

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/

#include "VolumeRenderShader.h"
#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"

// Thread number defines based on values from cFluid2D.hlsl
#define NUM_THREADS_X 16.0f
#define NUM_THREADS_Y 8.0f

VolumeRenderShader::VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject) : pD3dGraphicsObject(d3dGraphicsObject) {
}

VolumeRenderShader::~VolumeRenderShader() {
	pD3dGraphicsObject = nullptr;
}

void VolumeRenderShader::Compute(_In_ ID3D11ShaderResourceView* targetToRender, _In_ ID3D11UnorderedAccessView* result) {
	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();
	// Samplers don't change in this scene
	context->CSSetSamplers(0,1,&(mSampleState.p));

	// Set the parameters inside the shader
	context->CSSetShaderResources(0,1,&targetToRender);
	context->CSSetUnorderedAccessViews(0,1,&result,nullptr);

	int width,height;
	pD3dGraphicsObject->GetScreenDimensions(width,height);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription VolumeRenderShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cVolumeRender.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "RenderComputeShader";

	return shaderDescription;
}

void VolumeRenderShader::SetDynamicBufferValues(Vector3 &position, const Camera *pCamera, float zoom, Vector3& dimensions) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("VolumeRenderShader: failed to map buffer in SetDynamicBufferValues function"));
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	int width,height;
	pD3dGraphicsObject->GetScreenDimensions(width,height);
	dataPtr->vViewportDimensions[0] = width;
	dataPtr->vViewportDimensions[1] = height;
	dataPtr->vDimensions = dimensions;
	pCamera->GetPosition(dataPtr->vEyePos);
	dataPtr->fZoom = zoom;
	dataPtr->vWorldPos = position;
	dataPtr->padding0 = 0.0f;
	dataPtr->padding1 = 0.0f;
	dataPtr->padding2 = Vector2(0.0f);

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(0,1,&(mInputBuffer.p));
}

bool VolumeRenderShader::SpecificInitialization(ID3D11Device* device) {
	// Create the buffer
	D3D11_BUFFER_DESC inputBufferDesc;
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;
	// General buffer
	HRESULT hresult = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
	if(FAILED(hresult)) {
		return false;
	}

	// Create the sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hresult = device->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(hresult)) {
		return false;
	}

	return true;
}