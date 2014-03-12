/********************************************************************
Fluid3DCalculator.h: Encapsulates a 3D fluid simulation
being calculated on the GPU.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#include "Fluid3DCalculator.h"
#include "../../display/D3DShaders/Fluid3D/Fluid3DShaders.h"
#include "../../display/D3DShaders/Fluid3D/Fluid3DBuffers.h"

#define READ 0
#define WRITE 1
#define WRITE2 2
#define WRITE3 3

#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
#define AMBIENT_TEMPERATURE 0.0f

namespace BufferDirtyFlags
{
	const int General						= 0x01;
	const int DensityDissipation			= 0x02;
	const int TemperatureDissipation		= 0x04;
	const int VelocityDissipation			= 0x08;
}

using namespace Fluid3D;

Fluid3DCalculator::Fluid3DCalculator(FluidSettings fluidSettings) : pD3dGraphicsObj(nullptr), 
	mFluidSettings(fluidSettings),
	mVelocitySP(nullptr),
	mDensitySP(nullptr),
	mTemperatureSP(nullptr),
	mPressureSP(nullptr),
	mObstacleSP(nullptr),
	mVorticitySP(nullptr) {

}

Fluid3DCalculator::~Fluid3DCalculator() {
	if (mPressureSP) {
		delete [] mPressureSP;
		mPressureSP = nullptr;
	}
	if (mDensitySP) {
		delete [] mDensitySP;
		mDensitySP = nullptr;
	}
	if (mTemperatureSP) {
		delete [] mTemperatureSP;
		mTemperatureSP = nullptr;
	}
	if (mVelocitySP) {
		delete [] mVelocitySP;
		mVelocitySP = nullptr;
	}
	if (mObstacleSP) {
		delete [] mObstacleSP;
		mObstacleSP = nullptr;
	}
	if (mVorticitySP) {
		delete [] mVorticitySP;
		mVorticitySP = nullptr;
	}

	pD3dGraphicsObj = nullptr;
}

bool Fluid3DCalculator::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	bool result = InitShaders(hwnd);
	if (!result) {
		return false;
	}

	result = InitShaderParams(hwnd);
	if (!result) {
		return false;
	}

	result = InitBuffersAndSamplers();
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the fluid buffers or samplers", L"Error", MB_OK);
		return false;
	}

	// Update buffers with values
	UpdateGeneralBuffer();
	UpdateDissipationBuffer(VELOCITY);
	UpdateDissipationBuffer(DENSITY);
	UpdateDissipationBuffer(TEMPERATURE);

	return true;
}

bool Fluid3DCalculator::InitShaders(HWND hwnd) {
	ID3D11Device *device = pD3dGraphicsObj->GetDevice();

	mForwardAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_FORWARD, mFluidSettings.dimensions));
	bool result = mForwardAdvectionShader->Initialize(device,hwnd);
	if (!result) {
		return false;
	}

	mBackwardAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_BACKWARD, mFluidSettings.dimensions));
	result = mBackwardAdvectionShader->Initialize(device,hwnd);
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

	return true;
}

bool Fluid3DCalculator::InitShaderParams(HWND hwnd) {
	// Create the velocity shader params
	CComPtr<ID3D11Texture3D> velocityText[4];
	mVelocitySP = new ShaderParams[4];
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
	textureDesc.Width = (UINT) mFluidSettings.dimensions.x;
	textureDesc.Height = (UINT) mFluidSettings.dimensions.y;
	textureDesc.Depth = (UINT) mFluidSettings.dimensions.z;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;	// 3 components for velocity in 3D + alpha
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &velocityText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity Texture Object", L"Error", MB_OK);
			return false;
		}
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(velocityText[i], NULL, &mVelocitySP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity SRV", L"Error", MB_OK);
			return false;

		}
		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(velocityText[i], NULL, &mVelocitySP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity UAV", L"Error", MB_OK);
			return false;
		}
	}

	// Create the vorticity shader params
	CComPtr<ID3D11Texture3D> vorticityText[2];
	mVorticitySP = new ShaderParams[2];
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &vorticityText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the vorticity Texture Object", L"Error", MB_OK);
			return false;
		}
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(vorticityText[i], NULL, &mVorticitySP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the vorticity SRV", L"Error", MB_OK);
			return false;

		}
		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(vorticityText[i], NULL, &mVorticitySP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the vorticity UAV", L"Error", MB_OK);
			return false;
		}
	}

	// Create the density shader params
	CComPtr<ID3D11Texture3D> densityText[4];
	mDensitySP = new ShaderParams[4];
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &densityText[i]);
		if (FAILED(hr)){
			MessageBox(hwnd, L"Could not create the density Texture Object", L"Error", MB_OK);
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(densityText[i], NULL, &mDensitySP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the density SRV", L"Error", MB_OK);
			return false;
		}
		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(densityText[i], NULL, &mDensitySP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the density UAV", L"Error", MB_OK);
			return false;
		}
	}

	// Create the temperature shader params
	CComPtr<ID3D11Texture3D> temperatureText[4];
	mTemperatureSP = new ShaderParams[4];
	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &temperatureText[i]);
		if (FAILED(hr)){
			MessageBox(hwnd, L"Could not create the temperature Texture Object", L"Error", MB_OK);
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(temperatureText[i], NULL, &mTemperatureSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the temperature SRV", L"Error", MB_OK);
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(temperatureText[i], NULL, &mTemperatureSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the temperature UAV", L"Error", MB_OK);
			return false;
		}
	}

	// Create divergence shader params
	CComPtr<ID3D11Texture3D> divergenceText;
	mDivergenceSP = unique_ptr<ShaderParams>(new ShaderParams());
	HRESULT hresult = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &divergenceText);
	// Create the SRV and UAV.
	hresult = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(divergenceText, NULL, &mDivergenceSP->mSRV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the divergence SRV", L"Error", MB_OK);
		return false;
	}
	hresult = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(divergenceText, NULL, &mDivergenceSP->mUAV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the divergence UAV", L"Error", MB_OK);
		return false;
	}

	// Create pressure shader params and render targets
	mPressureSP = new ShaderParams[2];
	CComPtr<ID3D11Texture3D> pressureText[2];	
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture3D(&textureDesc, NULL, &pressureText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure Texture Object", L"Error", MB_OK);
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(pressureText[i], NULL, &mPressureSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure SRV", L"Error", MB_OK);
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(pressureText[i], NULL, &mPressureSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure UAV", L"Error", MB_OK);
			return false;
		}
		// Create the render target
		hr = pD3dGraphicsObj->GetDevice()->CreateRenderTargetView(pressureText[i], NULL, &mPressureRenderTargets[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure Render Target", L"Error", MB_OK);
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
	result = BuildDynamicBuffer<InputBufferDissipation>(pD3dGraphicsObj->GetDevice(), &mInputBufferDensityDissipation);
	if (!result) {
		return false;
	}
	result = BuildDynamicBuffer<InputBufferDissipation>(pD3dGraphicsObj->GetDevice(), &mInputBufferVelocityDissipation);
	if (!result) {
		return false;
	}
	result = BuildDynamicBuffer<InputBufferDissipation>(pD3dGraphicsObj->GetDevice(), &mInputBufferTemperatureDissipation);
	if (!result) {
		return false;
	}
	result = BuildDynamicBuffer<InputBufferImpulse>(pD3dGraphicsObj->GetDevice(), &mInputBufferImpulse);
	if (!result) {
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
	HRESULT hresult = pD3dGraphicsObj->GetDevice()->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(hresult)) {
		return false;
	}

	return true;
}

void Fluid3DCalculator::Process() {
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();
	context->CSSetSamplers(0,1,&(mSampleState.p));

	// Set all the buffers to the context
	ID3D11Buffer *const pProcessConstantBuffers[3] = {mInputBufferGeneral, mInputBufferVelocityDissipation, mInputBufferImpulse};
	context->CSSetConstantBuffers(0, 3, pProcessConstantBuffers);

	//Advect temperature against velocity
	context->CSSetConstantBuffers(1, 1, &(mInputBufferTemperatureDissipation.p));
	Advect(mTemperatureSP);

	// Advect density against velocity
	context->CSSetConstantBuffers(1, 1, &(mInputBufferDensityDissipation.p));
	Advect(mDensitySP);

	// Advect velocity against itself
	Advect(mVelocitySP);

	int resultBuffer = mFluidSettings.macCormackEnabled ? WRITE : WRITE2;
	swap(mVelocitySP[READ],mVelocitySP[resultBuffer]);
	swap(mTemperatureSP[READ],mTemperatureSP[resultBuffer]);
	swap(mDensitySP[READ],mDensitySP[resultBuffer]);

	//Determine how the temperature of the fluid changes the velocity
	mBuoyancyShader->Compute(context,&mVelocitySP[READ],&mTemperatureSP[READ],&mDensitySP[READ],&mVelocitySP[WRITE]);
	swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	// Add a constant amount of density and temperature back into the system
	RefreshConstantImpulse();

	// Try to preserve swirling movement of the fluid by injecting vorticity back into the system
	ComputeVorticityConfinement();

	// Calculate the divergence of the velocity
	mDivergenceShader->Compute(context,&mVelocitySP[READ],mDivergenceSP.get());

	CalculatePressureGradient();

	//Use the pressure texture that was last computed. This computes divergence free velocity
	mSubtractGradientShader->Compute(context,&mVelocitySP[READ],&mPressureSP[READ],&mVelocitySP[WRITE]);
	std::swap(mVelocitySP[READ],mVelocitySP[WRITE]);
}

void Fluid3DCalculator::Advect(ShaderParams *target) {
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	mForwardAdvectionShader->Compute(context,&mVelocitySP[READ],&target[READ],&target[WRITE2]);
	if (mFluidSettings.macCormackEnabled) {
		mBackwardAdvectionShader->Compute(context,&mVelocitySP[READ],&target[WRITE2],&target[WRITE3]);
		ShaderParams advectArrayDens[3] = {target[WRITE2], target[WRITE3], target[READ]};
		mMacCormarckAdvectionShader->Compute(context,&mVelocitySP[READ],advectArrayDens,&target[WRITE]);
	}
}

void Fluid3DCalculator::RefreshConstantImpulse() {
	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	Vector3 impulsePos = mFluidSettings.dimensions * mFluidSettings.constantInputPosition;

	//refresh the impulse of the density and temperature
	UpdateImpulseBuffer(impulsePos, mFluidSettings.constantDensityAmount, mFluidSettings.constantInputRadius);
	mImpulseShader->Compute(context, &mDensitySP[READ], &mDensitySP[WRITE]);
	swap(mDensitySP[READ], mDensitySP[WRITE]);

	UpdateImpulseBuffer(impulsePos, mFluidSettings.constantTemperature, mFluidSettings.constantInputRadius);
	mImpulseShader->Compute(context, &mTemperatureSP[READ], &mTemperatureSP[WRITE]);
	swap(mTemperatureSP[READ], mTemperatureSP[WRITE]);
}

void Fluid3DCalculator::ComputeVorticityConfinement() {
	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	mVorticityShader->Compute(context, &mVelocitySP[READ], &mVorticitySP[WRITE]);
	swap(mVorticitySP[READ], mVorticitySP[WRITE]);

	mConfinementShader->Compute(context, &mVelocitySP[READ], &mVorticitySP[READ], &mVelocitySP[WRITE]);

	swap(mVelocitySP[READ], mVelocitySP[WRITE]);
}

void Fluid3DCalculator::CalculatePressureGradient() {
	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	// clear pressure texture to prepare for Jacobi
	float clearCol[4] = {0.0f,0.0f,0.0f,0.0f};
	context->ClearRenderTargetView(mPressureRenderTargets[READ], clearCol);

	// perform Jacobi on pressure field
	int i;
	for (i = 0; i < mFluidSettings.jacobiIterations; ++i) {		
		mJacobiShader->Compute(context,
			&mPressureSP[READ],
			mDivergenceSP.get(),
			&mPressureSP[WRITE]);

		swap(mPressureSP[READ],mPressureSP[WRITE]);
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

	context->Unmap(mInputBufferGeneral,0);
}

void Fluid3DCalculator::UpdateDissipationBuffer(DissipationBufferType_t bufferType) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferDissipation* dataPtr;
	ID3D11Buffer* bufferToMap;
	float dissipationValue = 0.0f;

	switch (bufferType) {
	case DENSITY:
		bufferToMap = mInputBufferDensityDissipation;
		dissipationValue = mFluidSettings.densityDissipation;
		break;
	case VELOCITY:
		bufferToMap = mInputBufferVelocityDissipation;
		dissipationValue = mFluidSettings.velocityDissipation;
		break;
	case TEMPERATURE:
		bufferToMap = mInputBufferTemperatureDissipation;
		dissipationValue = mFluidSettings.temperatureDissipation;
		break;
	}

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(bufferToMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid3DEffect: failed to map buffer in UpdateDissipationBuffer function"));
	}

	dataPtr = (InputBufferDissipation*)mappedResource.pData;
	dataPtr->fDissipation = dissipationValue;

	context->Unmap(bufferToMap,0);
}

void Fluid3DCalculator::UpdateImpulseBuffer(Vector3& point, float amount, float radius) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferImpulse* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferImpulse, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid3DEffect: failed to map buffer in UpdateImpulseBuffer function"));
	}

	dataPtr = (InputBufferImpulse*)mappedResource.pData;
	dataPtr->vPoint	    = point;
	dataPtr->fRadius    = radius;	
	dataPtr->fAmount	= amount;

	context->Unmap(mInputBufferImpulse,0);
}

ID3D11ShaderResourceView * Fluid3DCalculator::GetVolumeTexture() const {
	return mDensitySP[READ].mSRV;
}

const FluidSettings &Fluid3DCalculator::GetFluidSettings() const {
	return mFluidSettings;
}

void Fluid3DCalculator::SetFluidSettings(const FluidSettings &fluidSettings) {
	// Update buffers if needed
	int dirtyFlags = GetUpdateDirtyFlags(fluidSettings);

	mFluidSettings = fluidSettings;

	if (dirtyFlags & BufferDirtyFlags::VelocityDissipation) {
		UpdateDissipationBuffer(VELOCITY);
	}
	if (dirtyFlags & BufferDirtyFlags::TemperatureDissipation) {
		UpdateDissipationBuffer(TEMPERATURE);
	}
	if (dirtyFlags & BufferDirtyFlags::DensityDissipation) {
		UpdateDissipationBuffer(DENSITY);
	}
	if (dirtyFlags & BufferDirtyFlags::General) {
		UpdateGeneralBuffer();
	}
}

int Fluid3DCalculator::GetUpdateDirtyFlags(const FluidSettings &newSettings) const {
	int dirtyFlags = 0;

	if (newSettings.velocityDissipation != mFluidSettings.velocityDissipation) {
		dirtyFlags |= BufferDirtyFlags::VelocityDissipation;
	}
	if (newSettings.temperatureDissipation != mFluidSettings.temperatureDissipation) {
		dirtyFlags |= BufferDirtyFlags::TemperatureDissipation;
	}
	if (newSettings.densityDissipation != mFluidSettings.densityDissipation) {
		dirtyFlags |= BufferDirtyFlags::DensityDissipation;
	}

	if (newSettings.timeStep != mFluidSettings.timeStep || newSettings.densityBuoyancy != mFluidSettings.densityBuoyancy
		|| newSettings.densityWeight != mFluidSettings.densityWeight || newSettings.dimensions != mFluidSettings.dimensions
		|| newSettings.vorticityStrength != mFluidSettings.vorticityStrength)
	{
		dirtyFlags |= BufferDirtyFlags::General;
	}

	return dirtyFlags;
}