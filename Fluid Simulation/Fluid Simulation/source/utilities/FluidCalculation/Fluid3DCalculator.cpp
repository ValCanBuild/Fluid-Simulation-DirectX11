/********************************************************************
Fluid3DCalculator.h: Encapsulates a 3D fluid simulation
being calculated on the GPU.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#include "Fluid3DCalculator.h"
#include "Fluid3DShaders.h"
#include "Fluid3DBuffers.h"

#define READ 0
#define WRITE 1

#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
#define AMBIENT_TEMPERATURE 0.0f

namespace BufferDirtyFlags
{
	const int General = 0x01;
}

using namespace Fluid3D;

// Declare statics
map<Vector3, CommonFluidResources> Fluid3DCalculator::commonResourcesMap;
CComPtr<ID3D11SamplerState>	Fluid3DCalculator::sampleState;

// Static methods
void Fluid3DCalculator::AttachCommonResources(ID3D11DeviceContext* context) {
	context->CSSetSamplers(0,1,&(sampleState.p));
}

Fluid3DCalculator::Fluid3DCalculator(const FluidSettings &fluidSettings) : pD3dGraphicsObj(nullptr), 
	mFluidSettings(fluidSettings)
{

}

Fluid3DCalculator::~Fluid3DCalculator() {
	pD3dGraphicsObj = nullptr;
}

bool Fluid3DCalculator::Initialize(_In_ D3DGraphicsObject * d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;
	ID3D11Device *pDevice = pD3dGraphicsObj->GetDevice();
	bool result = InitShaders(hwnd);
	if (!result) {
		return false;
	}

	switch (mFluidSettings.GetFluidType()) {
	case SMOKE:
		mFluidResources = FluidResourcesPerObject::CreateResourcesSmoke(pDevice, mFluidSettings.dimensions, hwnd);
		break;
	case FIRE:
		mFluidResources = FluidResourcesPerObject::CreateResourcesFire(pDevice, mFluidSettings.dimensions, hwnd);
		break;
	}

	if (commonResourcesMap.count(mFluidSettings.dimensions) == 0) {
		mCommonResources = CommonFluidResources::CreateResources(pDevice, mFluidSettings.dimensions, hwnd);
		if (!result) {
			return false;
		}
		commonResourcesMap[mFluidSettings.dimensions] = mCommonResources;
	} 
	else {
		mCommonResources = commonResourcesMap[mFluidSettings.dimensions];
	}

	result = InitBuffersAndSamplers();
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the fluid buffers or samplers", L"Error", MB_OK);
		return false;
	}

	// Update buffers with values
	UpdateGeneralBuffer();

	// create obstacle shader for generating the static obstacle field
	ObstacleShader obstacleShader(mFluidSettings.dimensions);
	result = obstacleShader.Initialize(pD3dGraphicsObj->GetDevice(), hwnd);
	if (!result) {
		return false;
	}
	obstacleShader.Compute(pD3dGraphicsObj->GetDeviceContext(), &mFluidResources.obstacleSP);

	return true;
}

bool Fluid3DCalculator::InitShaders(HWND hwnd) {
	ID3D11Device *device = pD3dGraphicsObj->GetDevice();

	mAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_NORMAL, mFluidSettings.dimensions));
	bool result = mAdvectionShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mMacCormarckAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_MACCORMARCK, mFluidSettings.dimensions));
	result = mMacCormarckAdvectionShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mImpulseShader = unique_ptr<ImpulseShader>(new ImpulseShader(mFluidSettings.dimensions));
	result = mImpulseShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mVorticityShader = unique_ptr<VorticityShader>(new VorticityShader(mFluidSettings.dimensions));
	result = mVorticityShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mConfinementShader = unique_ptr<ConfinementShader>(new ConfinementShader(mFluidSettings.dimensions));
	result = mConfinementShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mJacobiShader = unique_ptr<JacobiShader>(new JacobiShader(mFluidSettings.dimensions));
	result = mJacobiShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mDivergenceShader = unique_ptr<DivergenceShader>(new DivergenceShader(mFluidSettings.dimensions));
	result = mDivergenceShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mSubtractGradientShader = unique_ptr<SubtractGradientShader>(new SubtractGradientShader(mFluidSettings.dimensions));
	result = mSubtractGradientShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mBuoyancyShader = unique_ptr<BuoyancyShader>(new BuoyancyShader(mFluidSettings.dimensions));
	result = mBuoyancyShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	// only initialize ExtinguishmentImpulseShader if fluid type is fire
	if (mFluidSettings.GetFluidType() == FIRE) {
		mExtinguishmentImpulseShader = unique_ptr<ExtinguishmentImpulseShader>(new ExtinguishmentImpulseShader(mFluidSettings.dimensions));
		result = mExtinguishmentImpulseShader->Initialize(device,hwnd);
		if (!result) {
			return false;
		}
	}

	return true;
}

bool Fluid3DCalculator::InitBuffersAndSamplers() {
	// Create the constant buffers
	bool result = BuildDynamicBuffer<InputBufferGeneral>(pD3dGraphicsObj->GetDevice(), &mInputBufferGeneral);
	if (!result) {
		return false;
	}
	result = BuildDynamicBuffer<InputBufferAdvection>(pD3dGraphicsObj->GetDevice(), &mInputBufferAdvection);
	if (!result) {
		return false;
	}
	result = BuildDynamicBuffer<InputBufferImpulse>(pD3dGraphicsObj->GetDevice(), &mInputBufferImpulse);
	if (!result) {
		return false;
	}

	// Create the sampler if not already created
	if (sampleState == nullptr) {
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
		HRESULT hresult = pD3dGraphicsObj->GetDevice()->CreateSamplerState(&samplerDesc, &sampleState);
		if(FAILED(hresult)) {
			return false;
		}
	}
	return true;
}

void Fluid3DCalculator::Process() {
	auto context = pD3dGraphicsObj->GetDeviceContext();

	context->CSSetSamplers(0,1,&(sampleState.p));

	// Set the obstacle texture - it is constant throughout the execution step
	context->CSSetShaderResources(4, 1, &(mFluidResources.obstacleSP.mSRV.p));

	// Set all the buffers to the context
	ID3D11Buffer *const pProcessConstantBuffers[3] = {mInputBufferGeneral, mInputBufferAdvection, mInputBufferImpulse};
	context->CSSetConstantBuffers(0, 3, pProcessConstantBuffers);

	//Advect temperature against velocity
	Advect(mFluidResources.temperatureSP, NORMAL, mFluidSettings.temperatureDissipation);

	// Advect density against velocity
	Advect(mFluidResources.densitySP, mFluidSettings.advectionType, mFluidSettings.densityDissipation);

	// Advect the reaction field against velocity
	if (mFluidSettings.GetFluidType() == FIRE) {
		Advect(mFluidResources.reactionSP, mFluidSettings.advectionType, 1.0f, mFluidSettings.reactionDecay);
	}

	// Advect velocity against itself
	Advect(mFluidResources.velocitySP, mFluidSettings.advectionType, mFluidSettings.velocityDissipation);

	//Determine how the temperature of the fluid changes the velocity
	mBuoyancyShader->Compute(context,&mFluidResources.velocitySP[READ], &mFluidResources.temperatureSP[READ], &mFluidResources.densitySP[READ], &mFluidResources.velocitySP[WRITE]);
	swap(mFluidResources.velocitySP[READ], mFluidResources.velocitySP[WRITE]);

	// Add a constant amount of density and temperature back into the system
	RefreshConstantImpulse();

	// Try to preserve swirling movement of the fluid by injecting vorticity back into the system
	ComputeVorticityConfinement();

	// Calculate the divergence of the velocity
	mDivergenceShader->Compute(context, &mFluidResources.velocitySP[READ], &mCommonResources.divergenceSP);

	CalculatePressureGradient();

	//Use the pressure texture that was last computed. This computes divergence free velocity
	mSubtractGradientShader->Compute(context, &mFluidResources.velocitySP[READ], &mCommonResources.pressureSP[READ], &mFluidResources.velocitySP[WRITE]);
	std::swap(mFluidResources.velocitySP[READ], mFluidResources.velocitySP[WRITE]);
}

void Fluid3DCalculator::Advect(std::array<ShaderParams, 2> &target, SystemAdvectionType_t advectionType, float dissipation, float decay) {
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();
	switch (advectionType) {
	case NORMAL:
		UpdateAdvectionBuffer(dissipation, 1.0f, decay);
		mAdvectionShader->Compute(context, &mFluidResources.velocitySP[READ], &target[READ], &target[WRITE]);
		break;
	case MACCORMARCK:
		UpdateAdvectionBuffer(1.0f, 1.0f, 0.0f);
		mAdvectionShader->Compute(context, &mFluidResources.velocitySP[READ], &target[READ], &mCommonResources.tempSP[0]);
		break;
	}

	if (advectionType == MACCORMARCK) {
		// advect backwards a step
		UpdateAdvectionBuffer(1.0f, -1.0f, 0.0f);
		mAdvectionShader->Compute(context, &mFluidResources.velocitySP[READ], &mCommonResources.tempSP[0], &mCommonResources.tempSP[1]);
		ShaderParams advectArrayDens[3] = {mCommonResources.tempSP[0], mCommonResources.tempSP[1], target[READ]};
		// proceed with MacCormack advection
		UpdateAdvectionBuffer(dissipation, 1.0f, decay);
		mMacCormarckAdvectionShader->Compute(context, &mFluidResources.velocitySP[READ], advectArrayDens, &target[WRITE]);
	}
	swap(target[READ],target[WRITE]);
}

void Fluid3DCalculator::RefreshConstantImpulse() {
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	Vector3 impulsePos = mFluidSettings.dimensions * mFluidSettings.constantInputPosition;
	float size = mFluidSettings.dimensions.x + mFluidSettings.dimensions.y + mFluidSettings.dimensions.z;
	float inputRadius = mFluidSettings.constantInputRadius * size;

	//refresh the impulse of the density and temperature
	switch (mFluidSettings.GetFluidType()) {
	case SMOKE:
		UpdateImpulseBuffer(impulsePos, mFluidSettings.constantDensityAmount, inputRadius);
		mImpulseShader->Compute(context, &mFluidResources.densitySP[READ], &mFluidResources.densitySP[WRITE]);
		swap(mFluidResources.densitySP[READ], mFluidResources.densitySP[WRITE]);
		break;
	case FIRE:
		UpdateImpulseBuffer(impulsePos, mFluidSettings.constantReactionAmount, inputRadius);
		mImpulseShader->Compute(context, &mFluidResources.reactionSP[READ], &mFluidResources.reactionSP[WRITE]);
		swap(mFluidResources.reactionSP[READ], mFluidResources.reactionSP[WRITE]);

		// Smoke forms as fire is extinguished
		UpdateImpulseBuffer(impulsePos, mFluidSettings.constantDensityAmount, inputRadius, mFluidSettings.reactionExtinguishment);
		mExtinguishmentImpulseShader->Compute(context, &mFluidResources.reactionSP[READ], &mFluidResources.densitySP[READ], &mFluidResources.densitySP[WRITE]);
		swap(mFluidResources.densitySP[READ], mFluidResources.densitySP[WRITE]);
		break;
	}
	

	UpdateImpulseBuffer(impulsePos, mFluidSettings.constantTemperature, inputRadius);
	mImpulseShader->Compute(context, &mFluidResources.temperatureSP[READ], &mFluidResources.temperatureSP[WRITE]);
	swap(mFluidResources.temperatureSP[READ], mFluidResources.temperatureSP[WRITE]);
}

void Fluid3DCalculator::ComputeVorticityConfinement() {
	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	mVorticityShader->Compute(context, &mFluidResources.velocitySP[READ], &mFluidResources.vorticitySP);
	mConfinementShader->Compute(context, &mFluidResources.velocitySP[READ], &mFluidResources.vorticitySP, &mFluidResources.velocitySP[WRITE]);
	swap(mFluidResources.velocitySP[READ], mFluidResources.velocitySP[WRITE]);
}

void Fluid3DCalculator::CalculatePressureGradient() {
	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	// clear pressure texture to prepare for Jacobi
	float clearCol[4] = {0.0f,0.0f,0.0f,0.0f};
	context->ClearUnorderedAccessViewFloat(mCommonResources.pressureSP[READ].mUAV, clearCol);
	ShaderParams *pDivergence = &mCommonResources.divergenceSP;
	// perform Jacobi on pressure field
	int i;
	for (i = 0; i < mFluidSettings.jacobiIterations; ++i) {		
		mJacobiShader->Compute(context,
			&mCommonResources.pressureSP[READ],
			pDivergence,
			&mCommonResources.pressureSP[WRITE]);

		swap(mCommonResources.pressureSP[READ], mCommonResources.pressureSP[WRITE]);
	}
}

void Fluid3DCalculator::UpdateGeneralBuffer() {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferGeneral* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferGeneral, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid3DEffect: failed to map buffer in UpdateGeneralBuffer function"));
	}

	dataPtr = (InputBufferGeneral*)mappedResource.pData;
	dataPtr->fTimeStep = mFluidSettings.timeStep;
	dataPtr->fDensityBuoyancy = mFluidSettings.densityBuoyancy;
	dataPtr->fDensityWeight	= mFluidSettings.densityWeight;
	dataPtr->fVorticityStrength = mFluidSettings.vorticityStrength;
	dataPtr->vBuoyancyDirection = mFluidSettings.buoyancyDirection;

	context->Unmap(mInputBufferGeneral,0);
}

void Fluid3DCalculator::UpdateAdvectionBuffer(float dissipation, float timeModifier, float decay) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferAdvection* dataPtr;
	
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferAdvection, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid3DEffect: failed to map buffer in UpdateDissipationBuffer function"));
	}

	dataPtr = (InputBufferAdvection*)mappedResource.pData;
	dataPtr->fDissipation = dissipation;
	dataPtr->fTimeStepModifier = timeModifier;
	dataPtr->fDecay = decay;

	context->Unmap(mInputBufferAdvection,0);
}

void Fluid3DCalculator::UpdateImpulseBuffer(const Vector3& point, float amount, float radius, float extinguishment) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferImpulse* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferImpulse, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid3DEffect: failed to map buffer in UpdateImpulseBuffer function"));
	}

	dataPtr = (InputBufferImpulse*)mappedResource.pData;
	dataPtr->vPoint				= point;
	dataPtr->fRadius			= radius;	
	dataPtr->fAmount			= amount;
	dataPtr->fExtinguishment	= extinguishment;

	context->Unmap(mInputBufferImpulse,0);
}

void Fluid3DCalculator::SetFluidSettings(const FluidSettings &fluidSettings) {
	// Update buffers if needed
	int dirtyFlags = GetUpdateDirtyFlags(fluidSettings);

	mFluidSettings = fluidSettings;

	if (dirtyFlags & BufferDirtyFlags::General) {
		UpdateGeneralBuffer();
	}
}

int Fluid3DCalculator::GetUpdateDirtyFlags(const FluidSettings &newSettings) const {
	int dirtyFlags = 0;

	if (newSettings.timeStep != mFluidSettings.timeStep || newSettings.densityBuoyancy != mFluidSettings.densityBuoyancy
		|| newSettings.densityWeight != mFluidSettings.densityWeight || newSettings.dimensions != mFluidSettings.dimensions
		|| newSettings.vorticityStrength != mFluidSettings.vorticityStrength
		|| newSettings.buoyancyDirection != mFluidSettings.buoyancyDirection)
	{
		dirtyFlags |= BufferDirtyFlags::General;
	}

	return dirtyFlags;
}

FluidSettings * const Fluid3D::Fluid3DCalculator::GetFluidSettingsPointer() const {
	return const_cast<FluidSettings*>(&mFluidSettings);
}

const FluidSettings &Fluid3DCalculator::GetFluidSettings() const {
	return mFluidSettings;
}

ID3D11ShaderResourceView * Fluid3DCalculator::GetVolumeTexture() const {
	return mFluidResources.densitySP[READ].mSRV;
}

ID3D11ShaderResourceView * Fluid3DCalculator::GetReactionTexture() const {
	return mFluidResources.reactionSP[READ].mSRV;
}