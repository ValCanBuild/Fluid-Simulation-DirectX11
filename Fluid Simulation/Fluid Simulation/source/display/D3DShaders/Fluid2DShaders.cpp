/*************************************************************
Fluid2DShaders.cpp: Contains all the shader classes needed for
2D fluid simulation

Author: Valentin Hinov
Date: 11/09/2013
**************************************************************/
#include "Fluid2DShaders.h"
#include "../D3DGraphicsObject.h"

using namespace Fluid2D;

// Thread number defines based on values from cFluid2D.hlsl
#define NUM_THREADS_X 16.0f
#define NUM_THREADS_Y 8.0f

///////ADVECTION SHADER BEGIN////////
AdvectionShader::AdvectionShader(AdvectionShaderType_t advectionType) 
: mAdvectionType(advectionType) {
}

AdvectionShader::~AdvectionShader() {
}

bool AdvectionShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* obstacles, _In_ ShaderParams* advectResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Set the parameters inside the compute shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));

	// MacCormarck expects 3 advection targets
	if (mAdvectionType == ADVECTION_TYPE_MACCORMARCK) {
		context->CSSetShaderResources(1,1,&(advectTarget[0].mSRV.p));
		context->CSSetShaderResources(2,1,&(advectTarget[1].mSRV.p));
		context->CSSetShaderResources(3,1,&(advectTarget[2].mSRV.p));
		context->CSSetShaderResources(4,1,&(obstacles->mSRV.p));
	}
	else {
		context->CSSetShaderResources(1,1,&(advectTarget->mSRV.p));
		context->CSSetShaderResources(2,1,&(obstacles->mSRV.p));
	}

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	context->CSSetUnorderedAccessViews(0,1,&(advectResult->mUAV.p),nullptr);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[5] = {NULL,NULL,NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 5, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription AdvectionShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	switch (mAdvectionType) {
		case ADVECTION_TYPE_FORWARD:
			shaderDescription.computeShaderDesc.shaderFunctionName = "AdvectComputeShader";
			break;
		case ADVECTION_TYPE_BACKWARD:
			shaderDescription.computeShaderDesc.shaderFunctionName = "AdvectBackwardComputeShader";
			break;
		case ADVECTION_TYPE_MACCORMARCK:
			shaderDescription.computeShaderDesc.shaderFunctionName = "AdvectMacCormackComputeShader";
			break;
	}

	return shaderDescription;
}
///////ADVECTION SHADER END////////


///////IMPULSE SHADER BEGIN////////
ImpulseShader::ImpulseShader() {
}

ImpulseShader::~ImpulseShader() {
}

bool ImpulseShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Set the parameters inside the compute shader	
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	context->CSSetShaderResources(0,1,&(impulseInitial->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(impulseResult->mUAV.p),nullptr);

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

	return true;
}

ShaderDescription ImpulseShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ImpulseComputeShader";

	return shaderDescription;
}
///////IMPULSE SHADER END////////


///////JACOBI SHADER BEGIN////////
JacobiShader::JacobiShader() {
}

JacobiShader::~JacobiShader() {
}

bool JacobiShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* obstacles, _In_ ShaderParams* pressureResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(divergence->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetShaderResources(4,1,&(obstacles->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(pressureResult->mUAV.p),nullptr);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[5] = {NULL,NULL,NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 5, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription JacobiShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "JacobiComputeShader";

	return shaderDescription;
}
///////JACOBI SHADER END////////


///////DIVERGENCE SHADER BEGIN////////
DivergenceShader::DivergenceShader() {
}

DivergenceShader::~DivergenceShader() {
}

bool DivergenceShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* obstacles, _In_ ShaderParams* divergenceResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(4,1,&(obstacles->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(divergenceResult->mUAV.p),nullptr);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[5] = {NULL,NULL,NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 5, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription DivergenceShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "DivergenceComputeShader";

	return shaderDescription;
}
///////DIVERGENCE SHADER END////////


///////SUBTRACT GRADIENT SHADER END////////
SubtractGradientShader::SubtractGradientShader() {
}

SubtractGradientShader::~SubtractGradientShader() {
}

bool SubtractGradientShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* obstacles, _In_ ShaderParams* velocityResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetShaderResources(4,1,&(obstacles->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(velocityResult->mUAV.p),nullptr);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[5] = {NULL,NULL,NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 5, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription SubtractGradientShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "SubtractGradientComputeShader";

	return shaderDescription;
}
///////SUBTRACT GRADIENT SHADER END////////


///////BUOYANCY SHADER BEGIN////////
BuoyancyShader::BuoyancyShader() {
}

BuoyancyShader::~BuoyancyShader() {
}

bool BuoyancyShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* temperatureField, _In_ ShaderParams* density, _In_ ShaderParams* velocityResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(temperatureField->mSRV.p));
	context->CSSetShaderResources(2,1,&(density->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(velocityResult->mUAV.p),nullptr);

	UINT numThreadGroupX = (UINT)ceil(width/NUM_THREADS_X);
	UINT numThreadGroupY = (UINT)ceil(height/NUM_THREADS_Y);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadGroupX,numThreadGroupY,1);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[3] = {NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 3, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription BuoyancyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "BuoyancyComputeShader";

	return shaderDescription;
}
///////BUOYANCY SHADER END////////


///////FLUID2DRENDERSHADER BEGIN////////
Fluid2DRenderShader::Fluid2DRenderShader() {
}

Fluid2DRenderShader::~Fluid2DRenderShader() {
}

bool Fluid2DRenderShader::Render(_In_ D3DGraphicsObject* graphicsObject, int indexCount, _In_ ID3D11ShaderResourceView* obstacleTexture, _In_ ID3D11ShaderResourceView* targetToRender) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Set the parameters inside the shader
	context->PSSetShaderResources(0,1,&targetToRender);
	context->PSSetShaderResources(1,1,&obstacleTexture);

	// Set the samplers inside the pixel shader
	context->PSSetSamplers(0,1,&(mSampleState.p));

	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription Fluid2DRenderShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pFluid2DTexture.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "FluidPixelShader";

	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[2];

	shaderDescription.polygonLayout[0].SemanticName = "POSITION";
	shaderDescription.polygonLayout[0].SemanticIndex = 0;
	shaderDescription.polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[0].InputSlot = 0;
	shaderDescription.polygonLayout[0].AlignedByteOffset = 0;
	shaderDescription.polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[0].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[1].SemanticName = "TEXCOORD";
	shaderDescription.polygonLayout[1].SemanticIndex = 0;
	shaderDescription.polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	shaderDescription.polygonLayout[1].InputSlot = 0;
	shaderDescription.polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[1].InstanceDataStepRate = 0;

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool Fluid2DRenderShader::SpecificInitialization(ID3D11Device* device) {
	// Setup the sampler description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = device->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(result)) {
		return false;
	}
	return true;
}

///////FLUID2DRENDERSHADER SHADER END////////