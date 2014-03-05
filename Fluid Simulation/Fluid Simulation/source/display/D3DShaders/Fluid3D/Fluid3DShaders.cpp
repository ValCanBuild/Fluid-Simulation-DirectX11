/*************************************************************
Fluid3DShaders.cpp: Contains all the shader classes needed for
3D fluid simulation

Author: Valentin Hinov
Date: 09/11/2013
**************************************************************/
#include "Fluid3DShaders.h"

// Thread number defines based on values from cFluid3D.hlsl
#define NUM_THREADS_X 16
#define NUM_THREADS_Y 4
#define NUM_THREADS_Z 4

using namespace Fluid3D;

BaseFluid3DShader::BaseFluid3DShader(Vector3 dimensions) {
	SetDimensions(dimensions);
}

BaseFluid3DShader::~BaseFluid3DShader() {

}

void BaseFluid3DShader::Dispatch(_In_ ID3D11DeviceContext* context) const {
	// Run compute shader
	SetComputeShader(context);
	context->Dispatch(mNumThreadGroupX,mNumThreadGroupY,mNumThreadGroupZ);
}

void BaseFluid3DShader::SetDimensions(const Vector3 &dimensions) {
	mNumThreadGroupX = (UINT)ceil(dimensions.x/NUM_THREADS_X);
	mNumThreadGroupY = (UINT)ceil(dimensions.y/NUM_THREADS_Y);
	mNumThreadGroupZ = (UINT)ceil(dimensions.z/NUM_THREADS_Z);
}

ShaderDescription BaseFluid3DShader::GetShaderDescription() {
	throw std::runtime_error(std::string("BaseFluid3DShader: GetShaderDescription called on Base class"));
}

///////ADVECTION SHADER BEGIN////////
AdvectionShader::AdvectionShader(AdvectionType_t advectionType, Vector3 dimensions) 
: BaseFluid3DShader(dimensions), mAdvectionType(advectionType) {
}

AdvectionShader::~AdvectionShader() {
}

bool AdvectionShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult) {
	// Set the parameters inside the compute shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));

	// MacCormarck expects 3 advection targets
	if (mAdvectionType == ADVECTION_TYPE_MACCORMARCK) {
		context->CSSetShaderResources(1,1,&(advectTarget[0].mSRV.p));
		context->CSSetShaderResources(2,1,&(advectTarget[1].mSRV.p));
		context->CSSetShaderResources(3,1,&(advectTarget[2].mSRV.p));
	}
	else {
		context->CSSetShaderResources(1,1,&(advectTarget->mSRV.p));
	}

	context->CSSetUnorderedAccessViews(0,1,&(advectResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[4] = {nullptr,nullptr,nullptr,nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 4, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription AdvectionShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
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
ImpulseShader::ImpulseShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

ImpulseShader::~ImpulseShader() {
}

bool ImpulseShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult) {
	// Set the parameters inside the compute shader	
	context->CSSetShaderResources(0,1,&(impulseInitial->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(impulseResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription ImpulseShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ImpulseComputeShader";

	return shaderDescription;
}
///////IMPULSE SHADER END////////


///////JACOBI SHADER BEGIN////////
JacobiShader::JacobiShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

JacobiShader::~JacobiShader() {
}

bool JacobiShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* pressureResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(divergence->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(pressureResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription JacobiShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "JacobiComputeShader";

	return shaderDescription;
}
///////JACOBI SHADER END////////


///////DIVERGENCE SHADER BEGIN////////
DivergenceShader::DivergenceShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

DivergenceShader::~DivergenceShader() {
}

bool DivergenceShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* divergenceResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(divergenceResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription DivergenceShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "DivergenceComputeShader";

	return shaderDescription;
}
///////DIVERGENCE SHADER END////////


///////SUBTRACT GRADIENT SHADER END////////
SubtractGradientShader::SubtractGradientShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

SubtractGradientShader::~SubtractGradientShader() {
}

bool SubtractGradientShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* velocityResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(pressureField->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(velocityResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription SubtractGradientShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "SubtractGradientComputeShader";

	return shaderDescription;
}
///////SUBTRACT GRADIENT SHADER END////////


///////BUOYANCY SHADER BEGIN////////
BuoyancyShader::BuoyancyShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

BuoyancyShader::~BuoyancyShader() {
}

bool BuoyancyShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* temperatureField, _In_ ShaderParams* density, _In_ ShaderParams* velocityResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0,1,&(velocityField->mSRV.p));
	context->CSSetShaderResources(1,1,&(temperatureField->mSRV.p));
	context->CSSetShaderResources(2,1,&(density->mSRV.p));
	context->CSSetUnorderedAccessViews(0,1,&(velocityResult->mUAV.p),nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[3] = {NULL,NULL,NULL};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {NULL};

	context->CSSetShaderResources(0, 3, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);

	return true;
}

ShaderDescription BuoyancyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "BuoyancyComputeShader";

	return shaderDescription;
}
///////BUOYANCY SHADER END////////