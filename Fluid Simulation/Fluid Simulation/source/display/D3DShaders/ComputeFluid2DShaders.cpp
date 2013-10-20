/*************************************************************
Compute2DShaders.cpp: Contains all the shader classes needed for
2D fluid simulation

Author: Valentin Hinov
Date: 11/09/2013
**************************************************************/
#include "ComputeFluid2DShaders.h"
#include "../D3DGraphicsObject.h"

///////ADVECTION SHADER BEGIN////////
AdvectionShader::AdvectionShader() {
}

AdvectionShader::~AdvectionShader() {
}

bool AdvectionShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Set the parameters inside the compute shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(advectTarget->mSRV.p));
	
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	context->CSSetUnorderedAccessViews(0,1,&(advectResult->mUAV.p),nullptr);

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

	return true;
}

ShaderDescription AdvectionShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "AdvectComputeShader";

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

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

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

bool JacobiShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* pressureResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(divergence->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(pressureResult->mUAV.p),nullptr);

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

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

bool DivergenceShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* divergenceResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(divergenceResult->mUAV.p),nullptr);

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

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

bool SubtractGradientShader::Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* velocityResult) {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);

	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(velocityResult->mUAV.p),nullptr);

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

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

	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);

	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

	return true;
}

ShaderDescription BuoyancyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid2D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "BuoyancyComputeShader";

	return shaderDescription;
}
///////BUOYANCY SHADER END////////