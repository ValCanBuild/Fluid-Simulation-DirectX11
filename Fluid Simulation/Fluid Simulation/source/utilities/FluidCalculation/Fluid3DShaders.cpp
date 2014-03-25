/*************************************************************
Fluid3DShaders.cpp: Contains all the shader classes needed for
3D fluid simulation

Author: Valentin Hinov
Date: 09/11/2013
**************************************************************/
#include "Fluid3DShaders.h"

// Thread number defines based on values from cFluid3D.hlsl
#define NUM_THREADS_X 8
#define NUM_THREADS_Y 8
#define NUM_THREADS_Z 8

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
AdvectionShader::AdvectionShader(AdvectionShaderType_t advectionType, Vector3 dimensions) 
: BaseFluid3DShader(dimensions), mAdvectionType(advectionType) {
}

AdvectionShader::~AdvectionShader() {
}

void AdvectionShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult) {
	// Set the parameters inside the compute shader
	// MacCormarck expects 3 advection targets
	if (mAdvectionType == ADVECTION_TYPE_MACCORMARCK) {
		ID3D11ShaderResourceView *const pSRV[4] = {velocityField->mSRV, advectTarget[0].mSRV, advectTarget[1].mSRV, advectTarget[2].mSRV};
		context->CSSetShaderResources(0, 4, pSRV);
	}
	else {
		ID3D11ShaderResourceView *const pSRV[2] = {velocityField->mSRV, advectTarget->mSRV};
		context->CSSetShaderResources(0, 2, pSRV);
	}

	context->CSSetUnorderedAccessViews(0, 1, &(advectResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[4] = {nullptr, nullptr, nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 4, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription AdvectionShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	switch (mAdvectionType) {
		case ADVECTION_TYPE_NORMAL:
			shaderDescription.computeShaderDesc.shaderFunctionName = "AdvectComputeShader";
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

void ImpulseShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult) {
	// Set the parameters inside the compute shader	
	context->CSSetShaderResources(0, 1, &(impulseInitial->mSRV.p));
	context->CSSetUnorderedAccessViews(0, 1, &(impulseResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription ImpulseShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ImpulseComputeShader";

	return shaderDescription;
}
///////IMPULSE SHADER END////////


///////EXTINGUISHMENT IMPULSE SHADER BEGIN////////
ExtinguishmentImpulseShader::ExtinguishmentImpulseShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

ExtinguishmentImpulseShader::~ExtinguishmentImpulseShader() {
}

void ExtinguishmentImpulseShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* reactionField, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult) {
	// Set the parameters inside the compute shader	
	ID3D11ShaderResourceView *const pSRV[2] = {impulseInitial->mSRV, reactionField->mSRV};
	context->CSSetShaderResources(0, 2, pSRV);
	context->CSSetUnorderedAccessViews(0, 1, &(impulseResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription ExtinguishmentImpulseShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ExtinguishmentImpulseComputeShader";

	return shaderDescription;
}
///////EXTINGUISHMENT IMPULSE SHADER END////////


///////JACOBI SHADER BEGIN////////
JacobiShader::JacobiShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {
}

JacobiShader::~JacobiShader() {
}

void JacobiShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* pressureResult) {
	// Set the parameters inside the pixel shader
	ID3D11ShaderResourceView *const pSRV[2] = {divergence->mSRV, pressureField->mSRV};
	context->CSSetShaderResources(0, 2, pSRV);
	context->CSSetUnorderedAccessViews(0, 1, &(pressureResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
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

void DivergenceShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* divergenceResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0, 1, &(velocityField->mSRV.p));
	context->CSSetUnorderedAccessViews(0, 1, &(divergenceResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
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

void SubtractGradientShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* velocityResult) {
	// Set the parameters inside the pixel shader
	ID3D11ShaderResourceView *const pSRV[2] = {velocityField->mSRV, pressureField->mSRV};
	context->CSSetShaderResources(0, 2, pSRV);
	context->CSSetUnorderedAccessViews(0, 1, &(velocityResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
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

void BuoyancyShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* temperatureField, _In_ ShaderParams* density, _In_ ShaderParams* velocityResult) {
	// Set the parameters inside the pixel shader
	ID3D11ShaderResourceView *const pSRV[3] = {velocityField->mSRV, temperatureField->mSRV, density->mSRV};
	context->CSSetShaderResources(0, 3, pSRV);
	context->CSSetUnorderedAccessViews(0, 1, &(velocityResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[3] = {nullptr, nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 3, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription BuoyancyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "BuoyancyComputeShader";

	return shaderDescription;
}
///////BUOYANCY SHADER END////////

///////VORTICITY SHADER BEGIN////////
VorticityShader::VorticityShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {

}

VorticityShader::~VorticityShader() {

}

void VorticityShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* vorticityResult) {
	// Set the parameters inside the pixel shader
	context->CSSetShaderResources(0, 1, &(velocityField->mSRV.p));
	context->CSSetUnorderedAccessViews(0, 1, &(vorticityResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[1] = {nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 1, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription VorticityShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "VorticityComputeShader";

	return shaderDescription;
}
///////VORTICITY SHADER END////////

///////CONFINEMENT SHADER BEGIN////////
ConfinementShader::ConfinementShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {

}

ConfinementShader::~ConfinementShader() {

}

void ConfinementShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* vorticityField, _In_ ShaderParams* velocityResult) {
	// Set the parameters inside the pixel shader
	ID3D11ShaderResourceView *const pSRV[2] = {velocityField->mSRV, vorticityField->mSRV};
	context->CSSetShaderResources(0, 2, pSRV);
	context->CSSetUnorderedAccessViews(0, 1, &(velocityResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11ShaderResourceView *const pSRVNULL[2] = {nullptr, nullptr};
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};

	context->CSSetShaderResources(0, 2, pSRVNULL);
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription ConfinementShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ConfinementComputeShader";

	return shaderDescription;
}
///////CONFINEMENT SHADER END////////

///////OBSTACLE SHADER BEGIN////////
ObstacleShader::ObstacleShader(Vector3 dimensions) : BaseFluid3DShader(dimensions) {

}

ObstacleShader::~ObstacleShader() {

}

void ObstacleShader::Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* obstacleResult) {
	// Set the parameters inside the pixel shader
	context->CSSetUnorderedAccessViews(0, 1, &(obstacleResult->mUAV.p), nullptr);

	Dispatch(context);

	// To use for flushing shader parameters out of the shaders
	ID3D11UnorderedAccessView *const pUAVNULL[1] = {nullptr};
	context->CSSetUnorderedAccessViews(0, 1, pUAVNULL, nullptr);
}

ShaderDescription ObstacleShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cFluid3D.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "ObstaclesComputeShader";

	return shaderDescription;
}
///////OBSTACLE SHADER END////////