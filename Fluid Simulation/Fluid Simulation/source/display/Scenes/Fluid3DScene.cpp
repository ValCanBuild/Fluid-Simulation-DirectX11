/***************************************************************
Fluid3DScene.cpp: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/
#include "Fluid3DScene.h"


#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"
#include "../../objects/D2DTexQuad.h"
#include "../../utilities/D3DTexture.h"

#define DIM 80

#define READ 0
#define WRITE 1
#define WRITE2 2
#define WRITE3 3

// Simulation parameters
#define TIME_STEP 0.125f
#define IMPULSE_RADIUS 3.0f
#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
#define JACOBI_ITERATIONS 40
#define VEL_DISSIPATION 0.999f
#define DENSITY_DISSIPATION 0.999f
#define TEMPERATURE_DISSIPATION 0.99f
#define SMOKE_BUOYANCY 1.0f
#define SMOKE_WEIGHT 0.05f
#define AMBIENT_TEMPERATURE 0.0f
#define IMPULSE_TEMPERATURE 1.5f
#define IMPULSE_DENSITY 1.0f

Fluid3DScene::Fluid3DScene() : mPaused(false), mAngle(0.0f), mZoom(2.0f) {
	mVelocitySP    = nullptr;
	mDensitySP     = nullptr;
	mTemperatureSP = nullptr;
	mPressureSP    = nullptr;
}

Fluid3DScene::~Fluid3DScene() {
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

	pD3dGraphicsObj = nullptr;
}

bool Fluid3DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,5);

	mContainmentBox = unique_ptr<GeometricPrimitive>(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true).release());

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

	mFluidRenderShader = unique_ptr<Fluid3DRenderShader>(new Fluid3DRenderShader());
	result = mFluidRenderShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	} 

	// Create the velocity shader params
	CComPtr<ID3D11Texture3D> velocityText[4];
	mVelocitySP = new ShaderParams[4];
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
	textureDesc.Width = DIM;
	textureDesc.Height = DIM;
	textureDesc.Depth = DIM;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;	// 3 compononents for velocity in 3D + alpha
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

	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);

	// Create the fluid render result
	mFluidRenderResult = unique_ptr<ShaderParams>(new ShaderParams());
	CComPtr<ID3D11Texture2D> fluidRenderText;
	D3D11_TEXTURE2D_DESC fluidRenderTextureDesc;
	ZeroMemory(&fluidRenderTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	fluidRenderTextureDesc.Width = width;
	fluidRenderTextureDesc.Height = height;
	fluidRenderTextureDesc.ArraySize = 1;
	fluidRenderTextureDesc.MipLevels = 1;
	fluidRenderTextureDesc.SampleDesc.Count = 1;
	fluidRenderTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	fluidRenderTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	fluidRenderTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	fluidRenderTextureDesc.CPUAccessFlags = 0;
	fluidRenderTextureDesc.MiscFlags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC fluidRenderSrvDesc;
	fluidRenderSrvDesc.Format = fluidRenderTextureDesc.Format;
	fluidRenderSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	fluidRenderSrvDesc.Texture2D.MostDetailedMip = 0;
	fluidRenderSrvDesc.Texture2D.MipLevels = 1;

	D3D11_UNORDERED_ACCESS_VIEW_DESC fluidRenderUavDesc;
	fluidRenderUavDesc.Format = fluidRenderTextureDesc.Format;
	fluidRenderUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	fluidRenderUavDesc.Texture2D.MipSlice = 0;

	hresult = pD3dGraphicsObj->GetDevice()->CreateTexture2D(&fluidRenderTextureDesc, NULL, &fluidRenderText);
	if (FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render Texture Object", L"Error", MB_OK);
		return false;
	}
	hresult = pD3dGraphicsObj->GetDevice()->CreateShaderResourceView(fluidRenderText, &fluidRenderSrvDesc, &mFluidRenderResult->mSRV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render SRV", L"Error", MB_OK);
		return false;
	}
	hresult = pD3dGraphicsObj->GetDevice()->CreateUnorderedAccessView(fluidRenderText, &fluidRenderUavDesc, &mFluidRenderResult->mUAV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the fluid render UAV", L"Error", MB_OK);
		return false;
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
	samplerDesc.MaxAnisotropy = 16;
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

	/*mTexture = unique_ptr<D3DTexture>(new D3DTexture());
	result = mTexture->Initialize(pD3dGraphicsObj->GetDevice(),pD3dGraphicsObj->GetDeviceContext(),L"data/cobbles.jpg", hwnd);
	if (!result) {
		return false;
	}*/

	return true;
}

void Fluid3DScene::Update(float delta) {
	UpdateCamera(delta);
	//mAngle += delta;
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	int scrollValue;
	inputSystem->GetMouseScrollDelta(scrollValue);
	if (scrollValue > 0) {
		mZoom = mZoom / 1.1f;
	}
	else if (scrollValue < 0) {
		mZoom = mZoom * 1.1f;
	}
}

bool Fluid3DScene::Render() {
	Matrix viewMatrix, projectionMatrix, orthoMatrix;
	pD3dGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);
	pD3dGraphicsObj->GetOrthoMatrix(orthoMatrix);

	//Matrix worldMatrix;
	//pD3dGraphicsObj->GetWorldMatrix(worldMatrix);
	//Matrix objectMatrix = Matrix::CreateRotationY(mAngle);
	//objectMatrix *= worldMatrix;

	//mContainmentBox->Draw(objectMatrix,viewMatrix,projectionMatrix);

	// TODO Move to Update Loop
	if (!PerformComputation()) {
		return false;
	}

	// choose the texture to see
	mTexQuad->SetTexture(mFluidRenderResult->mSRV);
	bool result = mTexQuad->Render(&viewMatrix,&orthoMatrix);
	if (!result)
		return false;

	return true;
}

bool Fluid3DScene::PerformComputation() {
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
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[READ],&mVelocitySP[WRITE2]);
	mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mVelocitySP[WRITE2],&mVelocitySP[WRITE3]);
	ShaderParams advectArrayVel[3] = {mVelocitySP[WRITE2], mVelocitySP[WRITE3], mVelocitySP[READ]};
	mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayVel,&mVelocitySP[WRITE]);

	//Advect temperature against velocity
	result = SetDissipationBuffer(TEMPERATURE_DISSIPATION);
	if (!result) {
		return false;
	}
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[READ],&mTemperatureSP[WRITE2]);
	mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[WRITE2],&mTemperatureSP[WRITE3]);
	ShaderParams advectArrayTemp[3] = {mTemperatureSP[WRITE2], mTemperatureSP[WRITE3], mTemperatureSP[READ]};
	mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayTemp,&mTemperatureSP[WRITE]);


	// Advect density against velocity
	result = SetDissipationBuffer(DENSITY_DISSIPATION);
	if (!result) {
		return false;
	}
	mForwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mDensitySP[READ],&mDensitySP[WRITE2]);
	mBackwardAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mDensitySP[WRITE2],&mDensitySP[WRITE3]);
	ShaderParams advectArrayDens[3] = {mDensitySP[WRITE2], mDensitySP[WRITE3], mDensitySP[READ]};
	mMacCormarckAdvectionShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],advectArrayDens,&mDensitySP[WRITE]);

	swap(mVelocitySP[READ],mVelocitySP[WRITE]);
	swap(mTemperatureSP[READ],mTemperatureSP[WRITE]);
	swap(mDensitySP[READ],mDensitySP[WRITE]);

	//Determine how the flow of the fluid changes the velocity
	mBuoyancyShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mTemperatureSP[READ],&mDensitySP[READ],&mVelocitySP[WRITE]);

	swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	//refresh the impulse of the density and temperature
	result = SetImpulseBuffer(Vector4(DIM*0.5f,DIM,DIM*0.5f,0),Vector4(IMPULSE_TEMPERATURE,IMPULSE_TEMPERATURE,IMPULSE_TEMPERATURE,0), IMPULSE_RADIUS);
	if (!result) {
		return false;
	}
	mImpulseShader->Compute(pD3dGraphicsObj,&mTemperatureSP[READ],&mTemperatureSP[WRITE]);

	swap(mTemperatureSP[READ],mTemperatureSP[WRITE]);

	result = SetImpulseBuffer(Vector4(DIM*0.5f,DIM,DIM*0.5f,0),Vector4(IMPULSE_DENSITY,IMPULSE_DENSITY,IMPULSE_DENSITY,0), IMPULSE_RADIUS);
	if (!result) {
		return false;
	}
	mImpulseShader->Compute(pD3dGraphicsObj,&mDensitySP[READ],&mDensitySP[WRITE]);

	swap(mDensitySP[READ],mDensitySP[WRITE]);

	// Calculate the divergence of the velocity
	mDivergenceShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],mDivergenceSP.get());

	// clear pressure texture to prepare for jacobi
	float clearCol[4] = {0.0f,0.0f,0.0f,0.0f};
	context->ClearRenderTargetView(mPressureRenderTargets[READ], clearCol);

	// perform jacobi on pressure field
	int i;
	for (i = 0; i < JACOBI_ITERATIONS; ++i) {		
		mJacobiShader->Compute(pD3dGraphicsObj,
			&mPressureSP[READ],
			mDivergenceSP.get(),
			&mPressureSP[WRITE]);

		swap(mPressureSP[READ],mPressureSP[WRITE]);
	}

	//Use the pressure tex that was last computed. This computes divergence free velocity
	mSubtractGradientShader->Compute(pD3dGraphicsObj,&mVelocitySP[READ],&mPressureSP[READ],&mVelocitySP[WRITE]);

	std::swap(mVelocitySP[READ],mVelocitySP[WRITE]);

	// Render
	mFluidRenderShader->Compute(pD3dGraphicsObj,mDensitySP[READ].mSRV,mFluidRenderResult->mUAV);

	return true;
}

bool Fluid3DScene::SetGeneralBuffer() {
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
	dataPtr->fTimeStep = TIME_STEP;
	dataPtr->fBuoyancy = SMOKE_BUOYANCY;
	dataPtr->fDensityWeight	= SMOKE_WEIGHT;
	dataPtr->fAmbientTemperature = AMBIENT_TEMPERATURE;
	dataPtr->vViewportDimensions[0] = width;
	dataPtr->vViewportDimensions[1] = height;
	dataPtr->vDimensions = Vector3(DIM,DIM,DIM);	//dim, dim da me nqma
	mCamera->GetPosition(dataPtr->vEyePos);
	dataPtr->fZoom = mZoom;
	mCamera->GetRotationMatrix(dataPtr->mRotationMatrix);
	dataPtr->padding10 = 0.0f;
	dataPtr->padding11 = Vector2();

	context->Unmap(mInputBufferGeneral,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(0,1,&(mInputBufferGeneral.p));
	return true;
}

bool Fluid3DScene::SetDissipationBuffer(float dissipation) {
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

bool Fluid3DScene::SetImpulseBuffer(Vector4& point, Vector4& amount, float radius) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferImpulse* dataPtr;

	ID3D11DeviceContext *context = pD3dGraphicsObj->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBufferImpulse, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBufferImpulse*)mappedResource.pData;
	dataPtr->vPoint	    = point;
	dataPtr->vFillColor	= amount;
	dataPtr->fRadius    = radius;	
	dataPtr->padding2   = Vector3();

	context->Unmap(mInputBufferImpulse,0);

	// Set the buffer inside the compute shader
	context->CSSetConstantBuffers(2,1,&(mInputBufferImpulse.p));

	return true;
}

void Fluid3DScene::UpdateCamera(float delta) {
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();

	// Move camera with WASD 
	float forwardAmount = 0.0f;
	float rightAmount = 0.0f;
	const float moveFactor = 2.0f;

	if (inputSystem->IsKeyDown('W')) {
		forwardAmount += delta;
	}
	else if (inputSystem->IsKeyDown('S')) {
		forwardAmount -= delta;
	}
	if (inputSystem->IsKeyDown('A')) {
		rightAmount += delta;
	}
	else if (inputSystem->IsKeyDown('D')) {
		rightAmount -= delta;
	}

	if (forwardAmount != 0.0f || rightAmount != 0.0f) {
		mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
	}

	// Rotate camera with mouse button
	if (inputSystem->IsMouseRightDown()) {
		int xDelta,yDelta;
		float mouseSensitivity = 0.003f;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		mCamera->AddYawPitchRoll(-xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
	}

	mCamera->Update();
}