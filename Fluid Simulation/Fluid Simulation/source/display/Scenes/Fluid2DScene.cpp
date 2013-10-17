/***************************************************************
Fluid2DScene.cpp: Describes a scene that displays the 2D wave 
equation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
**************************************************************/
#include "Fluid2DScene.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../D3DShaders/Fluid2DShaders.h"
#include "../D3DFrameBuffer.h"
#include "../../objects/D2DTexQuad.h"
#include "../../system/ServiceProvider.h"

Fluid2DScene::Fluid2DScene() {
	textureShowing = 0;
}

Fluid2DScene::~Fluid2DScene() {
	for (int i = 0; i < 2; i++) {
		delete mVelocityFrameBuffer[i];
		mVelocityFrameBuffer[i] = nullptr;
	}
	delete [] mVelocityFrameBuffer;
	mVelocityFrameBuffer = nullptr;

	for (int i = 0; i < 2; i++) {
		delete mDensityFrameBuffer[i];
		mDensityFrameBuffer[i] = nullptr;
	}
	delete [] mDensityFrameBuffer;
	mDensityFrameBuffer = nullptr;

	for (int i = 0; i < 2; i++) {
		delete mTemperatureFrameBuffer[i];
		mTemperatureFrameBuffer[i] = nullptr;
	}
	delete [] mTemperatureFrameBuffer;
	mTemperatureFrameBuffer = nullptr;

	for (int i = 0; i < 2; i++) {
		delete mPressureFrameBuffer[i];
		mPressureFrameBuffer[i] = nullptr;
	}
	delete [] mPressureFrameBuffer;
	mPressureFrameBuffer = nullptr;

	delete mDivergenceFrameBuffer;
	mDivergenceFrameBuffer = nullptr;

	pD3dGraphicsObj = nullptr;
}

bool Fluid2DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,0);

	mAdvectionShader = unique_ptr<AdvectionShader>(new AdvectionShader());
	bool result = mAdvectionShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
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

	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	//width = 512;
	//height = 512;

	// Create the frame buffer array that will hold the states of all the fields
	mVelocityFrameBuffer = new IFrameBuffer*[2];
	mDensityFrameBuffer = new IFrameBuffer*[2];
	mPressureFrameBuffer = new IFrameBuffer*[2];
	mTemperatureFrameBuffer = new IFrameBuffer*[2];
	for (int i = 0; i < 2; i++) {
		mVelocityFrameBuffer[i] = new D3DFrameBuffer(DXGI_FORMAT_R32G32_FLOAT);
		mVelocityFrameBuffer[i]->Initialize(graphicsObject,width,height);
		mVelocityFrameBuffer[i]->BeginRender(0.0f,0.0f,0.0f,1.0f);
		mVelocityFrameBuffer[i]->EndRender();

		mDensityFrameBuffer[i] = new D3DFrameBuffer(DXGI_FORMAT_R32_FLOAT);
		mDensityFrameBuffer[i]->Initialize(graphicsObject,width,height);
		mDensityFrameBuffer[i]->BeginRender(0.0f,0.0f,0.0f,1.0f);
		mDensityFrameBuffer[i]->EndRender();

		mPressureFrameBuffer[i] = new D3DFrameBuffer(DXGI_FORMAT_R32_FLOAT);
		mPressureFrameBuffer[i]->Initialize(graphicsObject,width,height);
		mPressureFrameBuffer[i]->BeginRender(0.0f,0.0f,0.0f,1.0f);
		mPressureFrameBuffer[i]->EndRender();

		mTemperatureFrameBuffer[i] = new D3DFrameBuffer(DXGI_FORMAT_R32_FLOAT);
		mTemperatureFrameBuffer[i]->Initialize(graphicsObject,width,height);
		mTemperatureFrameBuffer[i]->BeginRender(0.0f,0.0f,0.0f,1.0f);
		mTemperatureFrameBuffer[i]->EndRender();
	}

	mDivergenceFrameBuffer = new D3DFrameBuffer(DXGI_FORMAT_R32_FLOAT);
	mDivergenceFrameBuffer->Initialize(graphicsObject,width,height);
	mDivergenceFrameBuffer->BeginRender(0.0f,0.0f,0.0f,1.0f);
	mDivergenceFrameBuffer->EndRender();

	// Create the quad
	mTexQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	result = mTexQuad->Initialize(graphicsObject,hwnd);
	if (!result) {
		return false;
	}

	return true;
}

void Fluid2DScene::Update(float delta) {
	mCamera->Update();
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	if (inputSystem->IsKeyDown(VK_SPACE)) {
		++textureShowing;
		if (textureShowing > 2)
			textureShowing = 0;
	}
}

bool Fluid2DScene::Render() {
	Matrix viewMatrix, orthoMatrix;
	pD3dGraphicsObj->GetOrthoMatrix(orthoMatrix);
	mCamera->GetViewMatrix(viewMatrix);

	bool result;

	int READ = 0;
	int WRITE = 1;

	// To use for flushing render targets out of the pixel shaders
	ID3D11ShaderResourceView *const pSRV[3] = {NULL,NULL,NULL};

	// Advect velocity against itself
	Advect((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
		   (ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
		   mVelocityFrameBuffer[WRITE], VEL_DISSIPATION);

	//Advect temperature against velocity
	Advect((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
		   (ID3D11ShaderResourceView*)mTemperatureFrameBuffer[READ]->GetTextureResource(),
		   mTemperatureFrameBuffer[WRITE], TEMPERATURE_DISSIPATION);

	// Advect density against velocity
	Advect((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
		   (ID3D11ShaderResourceView*)mDensityFrameBuffer[READ]->GetTextureResource(),
		   mDensityFrameBuffer[WRITE], DENSITY_DISSIPATION);

	pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);
	
	SwapBuffers(mVelocityFrameBuffer);
	SwapBuffers(mTemperatureFrameBuffer);
	SwapBuffers(mDensityFrameBuffer);	

	//Determine how the flow of the fluid changes the velocity
	ApplyBuoyancy((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
				  (ID3D11ShaderResourceView*)mTemperatureFrameBuffer[READ]->GetTextureResource(),
				  (ID3D11ShaderResourceView*)mDensityFrameBuffer[READ]->GetTextureResource(),
				  mVelocityFrameBuffer[WRITE]);

	pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 3, pSRV);

	SwapBuffers(mVelocityFrameBuffer);

	//refresh the impulse of the density and temperature
	ApplyImpulse(Vector2(400,600),Vector2(IMPULSE_DENSITY,IMPULSE_DENSITY), IMPULSE_RADIUS, (ID3D11ShaderResourceView*)mDensityFrameBuffer[READ]->GetTextureResource(), mDensityFrameBuffer[WRITE]);
	ApplyImpulse(Vector2(400,600),Vector2(IMPULSE_TEMPERATURE,IMPULSE_TEMPERATURE), IMPULSE_RADIUS, (ID3D11ShaderResourceView*)mTemperatureFrameBuffer[READ]->GetTextureResource(), mTemperatureFrameBuffer[WRITE]);
	

	SwapBuffers(mTemperatureFrameBuffer);
	SwapBuffers(mDensityFrameBuffer);	

	// Apply impulses to density and velocity
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	// mouse left button adds density
	if (inputSystem->IsMouseLeftDown()) {
		pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);
		int x,y;
		inputSystem->GetMousePos(x,y);
		int xDelta,yDelta;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		ApplyImpulse(Vector2((float)x,(float)y), Vector2(abs(xDelta*1.5f),abs(yDelta*1.5f)), INTERACTION_IMPULSE_RADIUS, (ID3D11ShaderResourceView*)mDensityFrameBuffer[READ]->GetTextureResource(), mDensityFrameBuffer[WRITE]);
		SwapBuffers(mDensityFrameBuffer);
	}
	// mouse right button adds velocity
	else if (inputSystem->IsMouseRightDown()) {
		pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);
		int x,y;
		inputSystem->GetMousePos(x,y);
		int xDelta,yDelta;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		ApplyImpulse(Vector2((float)x,(float)y), Vector2(xDelta*1.5f,yDelta*1.5f), INTERACTION_IMPULSE_RADIUS, (ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(), mVelocityFrameBuffer[WRITE]);
		SwapBuffers(mVelocityFrameBuffer);
	}	
	// mouse mid button adds temperature
	else if (inputSystem->IsMouseMidDown()) {
		pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);
		int x,y;
		inputSystem->GetMousePos(x,y);
		int xDelta,yDelta;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		ApplyImpulse(Vector2((float)x,(float)y), Vector2(abs(xDelta*1.5f),abs(yDelta*1.5f)), IMPULSE_RADIUS*0.4f, (ID3D11ShaderResourceView*)mTemperatureFrameBuffer[READ]->GetTextureResource(), mTemperatureFrameBuffer[WRITE]);
		SwapBuffers(mTemperatureFrameBuffer);
	}	

	// Calculate the divergence of the velocity
	ComputeDivergence((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(), 
					  mDivergenceFrameBuffer);

	// clear pressure frame buffer
	mPressureFrameBuffer[READ]->BeginRender(0,0,0,0);
	mPressureFrameBuffer[READ]->EndRender();

	// perform jacobi on pressure field
	int i;
	for (i = 0; i < JACOBI_ITERATIONS; ++i) {
		Jacobi((ID3D11ShaderResourceView*)mPressureFrameBuffer[READ]->GetTextureResource(), 
			   (ID3D11ShaderResourceView*)mDivergenceFrameBuffer->GetTextureResource(),
			   mPressureFrameBuffer[WRITE]);
		SwapBuffers(mPressureFrameBuffer);		
		pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);
	}

	//Use the pressure tex that was last rendered into. This computes divergence free velocity
	SubtractGradient((ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource(),
					 (ID3D11ShaderResourceView*)mPressureFrameBuffer[READ]->GetTextureResource(),
					 mVelocityFrameBuffer[WRITE]);

	SwapBuffers(mVelocityFrameBuffer);

	// choose the texture to see
	ID3D11ShaderResourceView* currTexture = nullptr;
	if (textureShowing == 0)
		currTexture = (ID3D11ShaderResourceView*)mDensityFrameBuffer[READ]->GetTextureResource();
	else if (textureShowing == 1)
		currTexture = (ID3D11ShaderResourceView*)mTemperatureFrameBuffer[READ]->GetTextureResource();
	else 
		currTexture = (ID3D11ShaderResourceView*)mVelocityFrameBuffer[READ]->GetTextureResource();

	// Render texture to screen
	mTexQuad->SetTexture(currTexture);
	result = mTexQuad->Render(&viewMatrix,&orthoMatrix);
	if (!result)
		return false;
	return true;
}

void Fluid2DScene::Advect(ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* advectionTarget, IFrameBuffer* renderTarget, float dissipation) {
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mAdvectionShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), TIME_STEP, dissipation, velocityField, advectionTarget);
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::ApplyBuoyancy(ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* temperatureField, ID3D11ShaderResourceView* density, IFrameBuffer* renderTarget) {
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mBuoyancyShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), TIME_STEP, SMOKE_BUOYANCY, SMOKE_WEIGHT, AMBIENT_TEMPERATURE, velocityField, temperatureField, density); 
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::ApplyImpulse(Vector2 mousePoint, Vector2 amount, float radius, ID3D11ShaderResourceView* originalState, IFrameBuffer* renderTarget) {
	// normalize mouse coords
	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);
	mousePoint.x = MapValue(mousePoint.x,0.0f,(float)width,0.0f,1.0f);
	mousePoint.y = MapValue(mousePoint.y,0.0f,(float)height,0.0f,1.0f);
	
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mImpulseShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), mousePoint, amount, radius, originalState);
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::Jacobi(ID3D11ShaderResourceView* pressure, ID3D11ShaderResourceView* divergence, IFrameBuffer* renderTarget) {
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mJacobiShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), -CELL_SIZE*CELL_SIZE, 0.25f, pressure, divergence);
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::ComputeDivergence(ID3D11ShaderResourceView* velocityField, IFrameBuffer* renderTarget) {
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mDivergenceShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), 0.5f/CELL_SIZE, velocityField);
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::SubtractGradient(ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* pressure, IFrameBuffer* renderTarget) {
	renderTarget->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);
		D3DRenderer *renderer = mTexQuad->GetRenderer();
		renderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		mSubtractGradientShader->Render(pD3dGraphicsObj, renderer->GetIndexCount(), GRADIENT_SCALE, velocityField, pressure);
		pD3dGraphicsObj->SetZBufferState(true);
	}
	renderTarget->EndRender();
}

void Fluid2DScene::SwapBuffers(IFrameBuffer** buffers) {
	IFrameBuffer* temp = buffers[0];
	buffers[0] = buffers[1];
	buffers[1] = temp;
}