/********************************************************************
Fluid2DSimulator.cpp: Encapsulates a 2D fluid simulation
being calculated on the GPU. Allows for interaction and choosing what
fluid property to render.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#include "Fluid2DSimulator.h"
#include "../D3DShaders/Fluid2DShaders.h"

#define READ 0
#define WRITE 1
#define WRITE2 2
#define WRITE3 3

// Simulation parameters
#define TIME_STEP 0.125f
#define IMPULSE_RADIUS 20.0f
#define JACOBI_ITERATIONS 50
#define CELL_SIZE 1.0f
#define GRADIENT_SCALE 1.0f
#define VEL_DISSIPATION 0.999f
#define DENSITY_DISSIPATION 0.999f
#define TEMPERATURE_DISSIPATION 0.99f
#define SMOKE_BUOYANCY 1.0f
#define SMOKE_WEIGHT 0.05f
#define AMBIENT_TEMPERATURE 0.0f
#define IMPULSE_TEMPERATURE 4.0f
#define IMPULSE_DENSITY	1.0f

using namespace Fluid2D;

Fluid2DSimulator::Fluid2DSimulator() : pD3dGraphicsObj(nullptr), 
	timeStep(TIME_STEP),
	macCormackEnabled(true),
	jacobiIterations(JACOBI_ITERATIONS),
	mVelocitySP(nullptr),
	mDensitySP(nullptr),
	mTemperatureSP(nullptr),
	mPressureSP(nullptr),
	mObstacleSP(nullptr) {

}

Fluid2DSimulator::~Fluid2DSimulator() {
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

	pD3dGraphicsObj = nullptr;
}

bool Fluid2DSimulator::Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd) {
	pD3dGraphicsObj = d3dGraphicsObj;

	mForwardAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_FORWARD));
	bool result = mForwardAdvectionShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mBackwardAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_BACKWARD));
	result = mBackwardAdvectionShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mMacCormarckAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader(AdvectionShader::ADVECTION_TYPE_MACCORMARCK));
	result = mMacCormarckAdvectionShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mImpulseShader = unique_ptr<ImpulseShader>(new ImpulseShader());
	result = mImpulseShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mJacobiShader = unique_ptr<JacobiShader>(new JacobiShader());
	result = mJacobiShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mDivergenceShader = unique_ptr<DivergenceShader>(new DivergenceShader());
	result = mDivergenceShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mSubtractGradientShader = unique_ptr<SubtractGradientShader>(new SubtractGradientShader());
	result = mSubtractGradientShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mBuoyancyShader = unique_ptr<BuoyancyShader>(new BuoyancyShader());
	result = mBuoyancyShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mFluidRenderShader = unique_ptr<Fluid2DRenderShader>(new Fluid2DRenderShader());
	result = mFluidRenderShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	} 

	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);

	// Create the velocity shader params
	CComPtr<ID3D11Texture2D> velocityText[4];
	mVelocitySP = new ShaderParams[4];
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &velocityText[i]);
		if (FAILED(hr)) {
			return false;
		}
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(velocityText[i], NULL, &mVelocitySP[i].mSRV);
		if(FAILED(hr)) {
			return false;
		}
		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(velocityText[i], NULL, &mVelocitySP[i].mUAV);
		if(FAILED(hr)) {
			return false;
		}
	}

	// Create the density shader params
	CComPtr<ID3D11Texture2D> densityText[4];
	mDensitySP = new ShaderParams[4];
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &densityText[i]);
		if (FAILED(hr)){
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(densityText[i], NULL, &mDensitySP[i].mSRV);
		if(FAILED(hr)) {
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(densityText[i], NULL, &mDensitySP[i].mUAV);
		if(FAILED(hr)) {
			return false;
		}
	}

	// Create the temperature shader params
	CComPtr<ID3D11Texture2D> temperatureText[4];
	mTemperatureSP = new ShaderParams[4];
	for (int i = 0; i < 4; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &temperatureText[i]);
		if (FAILED(hr)){
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(temperatureText[i], NULL, &mTemperatureSP[i].mSRV);
		if(FAILED(hr)) {
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(temperatureText[i], NULL, &mTemperatureSP[i].mUAV);
		if(FAILED(hr)) {
			return false;
		}
	}

	// Create the obstacles shader params
	CComPtr<ID3D11Texture2D> obstaclesText[2];
	mObstacleSP = new ShaderParams[2];
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &obstaclesText[i]);
		if (FAILED(hr)){
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(obstaclesText[i], NULL, &mObstacleSP[i].mSRV);
		if(FAILED(hr)) {
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(obstaclesText[i], NULL, &mObstacleSP[i].mUAV);
		if(FAILED(hr)) {
			return false;
		}
	}

	// Create divergence shader params
	CComPtr<ID3D11Texture2D> divergenceText;
	mDivergenceSP = unique_ptr<ShaderParams>(new ShaderParams());
	HRESULT hresult = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &divergenceText);
	// Create the SRV and UAV.
	hresult = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(divergenceText, NULL, &mDivergenceSP->mSRV);
	if(FAILED(hresult)) {
		return false;
	}

	hresult = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(divergenceText, NULL, &mDivergenceSP->mUAV);
	if(FAILED(hresult)) {
		return false;
	}

	// Create pressure shader params and render targets
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	mPressureSP = new ShaderParams[2];
	CComPtr<ID3D11Texture2D> pressureText[2];	
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&textureDesc, NULL, &pressureText[i]);
		if (FAILED(hr)) {
			return false;
		}
		// Create the SRV and UAV.
		hr = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(pressureText[i], NULL, &mPressureSP[i].mSRV);
		if(FAILED(hr)) {
			return false;
		}

		hr = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(pressureText[i], NULL, &mPressureSP[i].mUAV);
		if(FAILED(hr)) {
			return false;
		}
		// Create the render target
		hr = pD3dGraphicsObj->GetDevice()->CreateRenderTargetView(pressureText[i], &renderTargetViewDesc, &mPressureRenderTargets[i]);
		if (FAILED(hr)) {
			return false;
		}
	}

	// Create the constant buffers
	D3D11_BUFFER_DESC inputBufferDesc;
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBufferGeneral);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;
	// General buffer
	hresult = pD3dGraphicsObj->GetDevice()->CreateBuffer(&inputBufferDesc, NULL, &mInputBufferGeneral);
	if(FAILED(hresult)) {
		return false;
	}
	// Dissipation buffer
	inputBufferDesc.ByteWidth = sizeof(InputBufferDissipation);
	hresult = pD3dGraphicsObj->GetDevice()->CreateBuffer(&inputBufferDesc, NULL, &mInputBufferDissipation);
	if(FAILED(hresult)) {
		return false;
	}
	// Impulse buffer
	inputBufferDesc.ByteWidth = sizeof(InputBufferImpulse);
	hresult = pD3dGraphicsObj->GetDevice()->CreateBuffer(&inputBufferDesc, NULL, &mInputBufferImpulse);
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
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hresult = pD3dGraphicsObj->GetDevice()->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(result)) {
		return false;
	}

	// Samplers don't change in this effect
	pD3dGraphicsObj->GetDeviceContext()->CSSetSamplers(0,1,&(mSampleState.p));

	// Create the quad for 2d rendering
	mTexQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	result = mTexQuad->Initialize(pD3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}

	return true;
}

bool Fluid2DSimulator::Render(FluidPropertyType_t fluidPropertyType) {
	ID3D11ShaderResourceView* currTexture = nullptr;
	switch (fluidPropertyType) {
	case DENSITY:
		currTexture = mDensitySP[READ].mSRV;
		break;
	case TEMPERATURE:
		currTexture = mTemperatureSP[READ].mSRV;
		break;
	case VELOCITY:
		currTexture = mVelocitySP[READ].mSRV;
		break;
	default:
		return false;
	}

	// Render texture to screen
	D3DRenderer* quadRenderer = mTexQuad->GetRenderer();
	quadRenderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
	bool result = mFluidRenderShader->Render(pD3dGraphicsObj,quadRenderer->GetIndexCount(),mObstacleSP[READ].mSRV,currTexture);
	
	return result;
}

void Fluid2DSimulator::ProcessEffect() {
	// Set the general constant buffer
	SetGeneralBuffer();

	// Advect velocity against itself
	SetDissipationBuffer(VEL_DISSIPATION);
	Advect(mVelocitySP);

	//Advect temperature against velocity
	SetDissipationBuffer(TEMPERATURE_DISSIPATION);
	Advect(mTemperatureSP);

	// Advect density against velocity
	SetDissipationBuffer(DENSITY_DISSIPATION);
	Advect(mDensitySP);

	int resultBuffer = macCormackEnabled ? WRITE : WRITE2;
	swap(mVelocitySP[READ],mVelocitySP[resultBuffer]);
	swap(mTemperatureSP[READ],mTemperatureSP[resultBuffer]);
	swap(mDensitySP[READ],mDensitySP[resultBuffer]);

	//Determine how the temperature of the fluid changes the velocity
	mBuoyancyShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[READ],&mDensitySP[READ],&mVelocitySP[WRITE]);
	swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	RefreshConstantImpulse();

	// Calculate the divergence of the velocity
	mDivergenceShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mObstacleSP[READ],mDivergenceSP.get());

	CalculatePressureGradient();

	//Use the pressure texture that was last computed. This computes divergence free velocity
	mSubtractGradientShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mPressureSP[READ],&mObstacleSP[READ],&mVelocitySP[WRITE]);
	std::swap(mVelocitySP[READ],mVelocitySP[WRITE]);
}

void Fluid2D::Fluid2DSimulator::AddObstacle(Vector2 &pos, float radius) {
	SetImpulseBuffer(pos, Vector2(1,1), radius);
	mImpulseShader->Compute(pD3dGraphicsObj,&mObstacleSP[READ],&mObstacleSP[WRITE]);
	swap(mObstacleSP[READ],mObstacleSP[WRITE]);
}

void Fluid2D::Fluid2DSimulator::AddDensity(Vector2 &pos, Vector2& amount, float radius) {
	SetImpulseBuffer(pos, amount, radius);
	mImpulseShader->Compute(pD3dGraphicsObj,&mDensitySP[READ],&mDensitySP[WRITE]);
	swap(mDensitySP[READ],mDensitySP[WRITE]);
}

void Fluid2D::Fluid2DSimulator::AddVelocity(Vector2 &pos, Vector2& amount, float radius) {
	SetImpulseBuffer(pos, amount, radius);
	mImpulseShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[WRITE]);
	swap(mVelocitySP[READ],mVelocitySP[WRITE]);
}

void Fluid2D::Fluid2DSimulator::Advect(ShaderParams *target) {
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&target[READ],&mObstacleSP[READ],&target[WRITE2]);
	if (macCormackEnabled) {
		mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&target[WRITE2],&mObstacleSP[READ],&target[WRITE3]);
		ShaderParams advectArrayDens[3] = {target[WRITE2], target[WRITE3], target[READ]};
		mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayDens,&mObstacleSP[READ],&target[WRITE]);
	}
}

void Fluid2D::Fluid2DSimulator::RefreshConstantImpulse() {
	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	float halfWidth = width * 0.5f;

	//refresh the impulse of the density and temperature
	SetImpulseBuffer(Vector2(halfWidth,(float)height),Vector2(IMPULSE_TEMPERATURE,IMPULSE_TEMPERATURE), IMPULSE_RADIUS);
	mImpulseShader->Compute(pD3dGraphicsObj,&mTemperatureSP[READ],&mTemperatureSP[WRITE]);

	swap(mTemperatureSP[READ],mTemperatureSP[WRITE]);

	SetImpulseBuffer(Vector2(halfWidth,(float)height),Vector2(IMPULSE_DENSITY,IMPULSE_DENSITY), IMPULSE_RADIUS);
	mImpulseShader->Compute(pD3dGraphicsObj,&mDensitySP[READ],&mDensitySP[WRITE]);

	swap(mDensitySP[READ],mDensitySP[WRITE]);
}

void Fluid2D::Fluid2DSimulator::CalculatePressureGradient() {
	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	// clear pressure texture to prepare for Jacobi
	float clearCol[4] = {0.0f,0.0f,0.0f,0.0f};
	context->ClearRenderTargetView(mPressureRenderTargets[READ], clearCol);

	// perform Jacobi on pressure field
	int i;
	for (i = 0; i < jacobiIterations; ++i) {		
		mJacobiShader->Compute(pD3dGraphicsObj,
			&mPressureSP[READ],
			mDivergenceSP.get(),
			&mObstacleSP[READ],
			&mPressureSP[WRITE]);

		swap(mPressureSP[READ],mPressureSP[WRITE]);
	}
}

void Fluid2DSimulator::SetGeneralBuffer() {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferGeneral* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferGeneral, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid2DEffect: failed to map buffer in SetGeneralBuffer function"));
	}

	dataPtr = (InputBufferGeneral*)mappedResource.pData;
	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	dataPtr->fTimeStep = timeStep;
	dataPtr->fBuoyancy = SMOKE_BUOYANCY;
	dataPtr->fDensityWeight	= SMOKE_WEIGHT;
	dataPtr->fAmbientTemperature = AMBIENT_TEMPERATURE;
	dataPtr->fAlpha	= -CELL_SIZE*CELL_SIZE;
	dataPtr->fInverseBeta = 0.25f;
	dataPtr->fHalfInverseCellSize = 0.5f/CELL_SIZE;	
	dataPtr->fGradientScale	= GRADIENT_SCALE;
	dataPtr->vDimensions = Vector2((float)width,(float)height);
	dataPtr->padding0 = Vector2();

	context->Unmap(mInputBufferGeneral,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(0,1,&(mInputBufferGeneral.p));
}

void Fluid2DSimulator::SetDissipationBuffer(float dissipation) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferDissipation* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferDissipation, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid2DEffect: failed to map buffer in SetDissipationBuffer function"));
	}

	dataPtr = (InputBufferDissipation*)mappedResource.pData;
	dataPtr->fDissipation = dissipation;	
	dataPtr->padding1 = Vector3();

	context->Unmap(mInputBufferDissipation,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(1,1,&(mInputBufferDissipation.p));
}

void Fluid2DSimulator::SetImpulseBuffer(Vector2& point, Vector2& amount, float radius) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferImpulse* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferImpulse, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("Fluid2DEffect: failed to map buffer in SetImpulseBuffer function"));
	}

	dataPtr = (InputBufferImpulse*)mappedResource.pData;
	dataPtr->vPoint	= point;
	dataPtr->vFillColor	= amount;
	dataPtr->fRadius = radius;	
	dataPtr->padding2 = Vector3();

	context->Unmap(mInputBufferImpulse,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(2,1,&(mInputBufferImpulse.p));
}