/***************************************************************
Fluid2DScene.cpp: Describes a scene that displays a 2D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
***************************************************************/
#include <AntTweakBar.h>
#include "Fluid2DScene.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../D3DShaders/Fluid2DShaders.h"
#include "../D3DFrameBuffer.h"
#include "../../objects/D2DTexQuad.h"
#include "../../system/ServiceProvider.h"

#define READ 0
#define WRITE 1
#define WRITE2 2
#define WRITE3 3

// Simulation parameters
#define TIME_STEP 0.125f
#define IMPULSE_RADIUS 20.0f
#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
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

Fluid2DScene::Fluid2DScene() {
	mTimeStep = TIME_STEP;
	textureShowing = 0;
	mMacCormackEnabled = true;
	mPaused = false;
	mJacobiIterations = JACOBI_ITERATIONS;
	mVelocitySP = nullptr;
	mDensitySP = nullptr;
	mTemperatureSP = nullptr;
	mPressureSP = nullptr;
}

Fluid2DScene::~Fluid2DScene() {
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

	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;
}

bool Fluid2DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,0);

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

	// Samplers don't change in this scene
	pD3dGraphicsObj->GetDeviceContext()->CSSetSamplers(0,1,&(mSampleState.p));

	// Create the quad for 2d rendering
	mTexQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	result = mTexQuad->Initialize(graphicsObject,hwnd);
	if (!result) {
		return false;
	}
	
	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("2D Fluid Simulation");
	// Position bar
	int barPos[2] = {580,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {218,150};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Jacobi Iterations", TW_TYPE_INT32, &mJacobiIterations, "min=1 max=100 step=1");
	TwAddVarRW(mTwBar,"Time Step", TW_TYPE_FLOAT, &mTimeStep, "min=0.0 max=1.0 step=0.001");
	TwAddVarRW(mTwBar,"MacCormarck Advection", TW_TYPE_BOOLCPP, &mMacCormackEnabled, nullptr);
	TwAddVarRW(mTwBar,"Simulation Paused", TW_TYPE_BOOLCPP, &mPaused, nullptr);
	return true;
}

void Fluid2DScene::Update(float delta) {
	mCamera->Update();
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	if (inputSystem->IsKeyClicked(VK_SPACE)) {
		++textureShowing;
		if (textureShowing > 2)
			textureShowing = 0;
	}
	// mouse mid button adds obstacles
	if (inputSystem->IsMouseMidDown()) {
		int x,y;
		inputSystem->GetMousePos(x,y);
		SetImpulseBuffer(Vector2((float)x,(float)y),Vector2(1,1), OBSTACLES_IMPULSE_RADIUS);
		mImpulseShader->Compute(pD3dGraphicsObj,&mObstacleSP[READ],&mObstacleSP[WRITE]);
		swap(mObstacleSP[READ],mObstacleSP[WRITE]);
	}	
}

bool Fluid2DScene::Render() {
	Matrix viewMatrix, orthoMatrix;
	pD3dGraphicsObj->GetOrthoMatrix(orthoMatrix);
	mCamera->GetViewMatrix(viewMatrix);

	if (!mPaused) {
		if (!PerformComputation()) {
			return false;
		}
	}

	// choose the texture to see
	ID3D11ShaderResourceView* currTexture = nullptr;
	if (textureShowing == 0)
		currTexture = mDensitySP[READ].mSRV;
	else if (textureShowing == 1)
		currTexture = mTemperatureSP[READ].mSRV;
	else 
		currTexture = mVelocitySP[READ].mSRV;

	// Render texture to screen
	//mTexQuad->SetTexture(currTexture);
	D3DRenderer* quadRenderer = mTexQuad->GetRenderer();
	quadRenderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
	bool result = mFluidRenderShader->Render(pD3dGraphicsObj,quadRenderer->GetIndexCount(),mObstacleSP[READ].mSRV,currTexture);
	if (!result)
		return false;

	return true;
}

bool Fluid2DScene::PerformComputation() {
	// Set the general constant buffer
	bool result = SetGeneralBuffer();
	if (!result) {
		return false;
	}

	ID3D11DeviceContext* context = pD3dGraphicsObj->GetDeviceContext();

	// Advect velocity against itself
	result = SetDissipationBuffer(VEL_DISSIPATION);
	if (!result) {
		return false;
	}
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[READ],&mObstacleSP[READ],&mVelocitySP[WRITE2]);
	if (mMacCormackEnabled) {
		mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[WRITE2],&mObstacleSP[READ],&mVelocitySP[WRITE3]);
		ShaderParams advectArrayVel[3] = {mVelocitySP[WRITE2], mVelocitySP[WRITE3], mVelocitySP[READ]};
		mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayVel,&mObstacleSP[READ],&mVelocitySP[WRITE]);
	}

	//Advect temperature against velocity
	result = SetDissipationBuffer(TEMPERATURE_DISSIPATION);
	if (!result) {
		return false;
	}
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[READ],&mObstacleSP[READ],&mTemperatureSP[WRITE2]);
	if (mMacCormackEnabled) {
		mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[WRITE2],&mObstacleSP[READ],&mTemperatureSP[WRITE3]);
		ShaderParams advectArrayTemp[3] = {mTemperatureSP[WRITE2], mTemperatureSP[WRITE3], mTemperatureSP[READ]};
		mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayTemp,&mObstacleSP[READ],&mTemperatureSP[WRITE]);
	}

	// Advect density against velocity
	result = SetDissipationBuffer(DENSITY_DISSIPATION);
	if (!result) {
		return false;
	}
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mDensitySP[READ],&mObstacleSP[READ],&mDensitySP[WRITE2]);
	if (mMacCormackEnabled) {
		mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mDensitySP[WRITE2],&mObstacleSP[READ],&mDensitySP[WRITE3]);
		ShaderParams advectArrayDens[3] = {mDensitySP[WRITE2], mDensitySP[WRITE3], mDensitySP[READ]};
		mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayDens,&mObstacleSP[READ],&mDensitySP[WRITE]);
	}
	
	int resultBuffer = mMacCormackEnabled ? WRITE : WRITE2;

	swap(mVelocitySP[READ],mVelocitySP[resultBuffer]);
	swap(mTemperatureSP[READ],mTemperatureSP[resultBuffer]);
	swap(mDensitySP[READ],mDensitySP[resultBuffer]);

	//Determine how the flow of the fluid changes the velocity
	mBuoyancyShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[READ],&mDensitySP[READ],&mVelocitySP[WRITE]);

	swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	float halfWidth = width * 0.5f;

	//refresh the impulse of the density and temperature
	result = SetImpulseBuffer(Vector2(halfWidth,(float)height),Vector2(IMPULSE_TEMPERATURE,IMPULSE_TEMPERATURE), IMPULSE_RADIUS);
	if (!result) {
		return false;
	}
	mImpulseShader->Compute(pD3dGraphicsObj,&mTemperatureSP[READ],&mTemperatureSP[WRITE]);

	swap(mTemperatureSP[READ],mTemperatureSP[WRITE]);

	result = SetImpulseBuffer(Vector2(halfWidth,(float)height),Vector2(IMPULSE_DENSITY,IMPULSE_DENSITY), IMPULSE_RADIUS);
	if (!result) {
		return false;
	}
	mImpulseShader->Compute(pD3dGraphicsObj,&mDensitySP[READ],&mDensitySP[WRITE]);

	swap(mDensitySP[READ],mDensitySP[WRITE]);

	// Apply impulses to density velocity and temperature
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	int x,y;
	inputSystem->GetMousePos(x,y);
	int xDelta,yDelta;
	inputSystem->GetMouseDelta(xDelta,yDelta);
	// mouse left button adds density
	if (inputSystem->IsMouseLeftDown()) {
		result = SetImpulseBuffer(Vector2((float)x,(float)y),Vector2(abs(xDelta*1.5f),abs(yDelta*1.5f)), INTERACTION_IMPULSE_RADIUS);
		if (!result) {
			return false;
		}
		mImpulseShader->Compute(pD3dGraphicsObj,&mDensitySP[READ],&mDensitySP[WRITE]);
		swap(mDensitySP[READ],mDensitySP[WRITE]);
		result = SetImpulseBuffer(Vector2((float)x,(float)y),Vector2(xDelta*1.5f,yDelta*1.5f), INTERACTION_IMPULSE_RADIUS);
		if (!result) {
			return false;
		}
		mImpulseShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[WRITE]);
		swap(mVelocitySP[READ],mVelocitySP[WRITE]);
	}
	// mouse right button adds velocity
	else if (inputSystem->IsMouseRightDown()) {
		result = SetImpulseBuffer(Vector2((float)x,(float)y),Vector2(xDelta*1.5f,yDelta*1.5f), INTERACTION_IMPULSE_RADIUS);
		if (!result) {
			return false;
		}
		mImpulseShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[WRITE]);
		swap(mVelocitySP[READ],mVelocitySP[WRITE]);
	}	

	// Calculate the divergence of the velocity
	mDivergenceShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mObstacleSP[READ],mDivergenceSP.get());

	// clear pressure texture to prepare for jacobi
	float clearCol[4] = {0.0f,0.0f,0.0f,0.0f};
	context->ClearRenderTargetView(mPressureRenderTargets[READ], clearCol);

	// perform jacobi on pressure field
	int i;
	for (i = 0; i < mJacobiIterations; ++i) {		
		mJacobiShader->Compute(pD3dGraphicsObj,
								&mPressureSP[READ],
								mDivergenceSP.get(),
								&mObstacleSP[READ],
								&mPressureSP[WRITE]);

		swap(mPressureSP[READ],mPressureSP[WRITE]);
	}

	//Use the pressure tex that was last computed. This computes divergence free velocity
	mSubtractGradientShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mPressureSP[READ],&mObstacleSP[READ],&mVelocitySP[WRITE]);

	std::swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	return true;
}

bool Fluid2DScene::SetGeneralBuffer() {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferGeneral* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferGeneral, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBufferGeneral*)mappedResource.pData;
	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	dataPtr->fTimeStep = mTimeStep;
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
	return true;
}

bool Fluid2DScene::SetDissipationBuffer(float dissipation) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferDissipation* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferDissipation, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBufferDissipation*)mappedResource.pData;
	dataPtr->fDissipation = dissipation;	
	dataPtr->padding1 = Vector3();

	context->Unmap(mInputBufferDissipation,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(1,1,&(mInputBufferDissipation.p));

	return true;
}

bool Fluid2DScene::SetImpulseBuffer(Vector2& point, Vector2& amount, float radius) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferImpulse* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferImpulse, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBufferImpulse*)mappedResource.pData;
	dataPtr->vPoint	= point;
	dataPtr->vFillColor	= amount;
	dataPtr->fRadius = radius;	
	dataPtr->padding2 = Vector3();

	context->Unmap(mInputBufferImpulse,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(2,1,&(mInputBufferImpulse.p));

	return true;
}