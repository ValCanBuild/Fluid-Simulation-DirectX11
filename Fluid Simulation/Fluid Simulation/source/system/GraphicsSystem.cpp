/***************************************************************
GraphicsSystem.cpp: In charge of displaying all content on the
screen.

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#include "GraphicsSystem.h"
#include "ServiceProvider.h"

#if defined (D3D)
	#include "../display/D3DGraphicsObject.h"
	#include "../utilities/Camera.h"
	#include "../utilities/D3DTexture.h"
	#include "../display/D3DRenderer.h"
	#include "../display/D3DShaders/OrthoTextureShader.h"
	#include "../display/D3DShaders/OrthoColorShader.h"
	#include "../display/D3DShaders/WaveShader.h"
	#include "../display/D3DFrameBuffer.h"
	#include "../objects/D2DTexQuad.h"
	#include "../objects/D2DColorQuad.h"
#endif

ID3D11ShaderResourceView *pCurrent;
ID3D11ShaderResourceView *pPrev;

GraphicsSystem::GraphicsSystem() {
}


GraphicsSystem::GraphicsSystem(const GraphicsSystem& other) {
}


GraphicsSystem::~GraphicsSystem() {
}


bool GraphicsSystem::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

	#if defined (D3D)
		mGraphicsObj = unique_ptr<IGraphicsObject>(new D3DGraphicsObject());
	#endif

	if (!mGraphicsObj)
		return false;

	bool result = mGraphicsObj->Initialize(screenWidth,screenHeight,VSYNC_ENABLED,hwnd,FULL_SCREEN,SCREEN_DEPTH,SCREEN_NEAR);

	if (!result) {
		MessageBox(hwnd, L"Could not initialize Graphics Object", L"Error", MB_OK);
		return false;
	}

	D3DGraphicsObject *d3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(mGraphicsObj.get());

	mCamera = unique_ptr<Camera>(new Camera());
	
	mCamera->SetPosition(0,0,0);

	mTextureShader = unique_ptr<OrthoTextureShader>(new OrthoTextureShader());
	result = mTextureShader->Initialize(d3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mColorShader = unique_ptr<OrthoColorShader>(new OrthoColorShader());
	result = mColorShader->Initialize(d3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mWaveShader = unique_ptr<WaveShader>(new WaveShader());
	result = mWaveShader->Initialize(d3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	mTexture = unique_ptr<D3DTexture>(new D3DTexture());
	result = mTexture->Initialize(d3dGraphicsObj->GetDevice(),L"data/cobbles.jpg");
	if (!result) {
		return false;
	}

	mFrameBuffer = unique_ptr<IFrameBuffer>(new D3DFrameBuffer());
	result = mFrameBuffer->Initialize(mGraphicsObj.get(),screenWidth,screenHeight);
	if (!result) {
		return false;
	}

	mFrameBuffer2 = unique_ptr<IFrameBuffer>(new D3DFrameBuffer());
	result = mFrameBuffer2->Initialize(mGraphicsObj.get(),screenWidth,screenHeight);
	if (!result) {
		return false;
	}

	mFrameBuffer3 = unique_ptr<IFrameBuffer>(new D3DFrameBuffer());
	result = mFrameBuffer3->Initialize(mGraphicsObj.get(),screenWidth,screenHeight);
	if (!result) {
		return false;
	}

	/*mFrameBuffer4 = unique_ptr<IFrameBuffer>(new D3DFrameBuffer());
	result = mFrameBuffer4->Initialize(mGraphicsObj.get(),screenWidth,screenHeight);
	if (!result) {
		return false;
	}*/

	mQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	result = mQuad->Initialize(mGraphicsObj.get(),hwnd);
	if (!result) {
		return false;
	}

	mQuad->SetTexture(mTexture->GetTexture());

	mColorQuad = unique_ptr<D2DColorQuad>(new D2DColorQuad());
	result = mColorQuad->Initialize(mGraphicsObj.get(),hwnd);
	if (!result) {
		return false;
	}

	mColorQuad->mTransform.scale = Vector3f(0.05f,0.05f,0.0f);


	pCurrent = nullptr;
	pPrev = nullptr;

	return true;
}

bool GraphicsSystem::Frame() {
	mCamera->Update();
	return Render();
}

bool GraphicsSystem::Render() {
	// first obtain all the needed matrices
	D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix, orthoMatrix;
	mCamera->GetViewMatrix(viewMatrix);
	mGraphicsObj->GetWorldMatrix(worldMatrix);
	mGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mGraphicsObj->GetOrthoMatrix(orthoMatrix);

	D3DGraphicsObject *d3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(mGraphicsObj.get());
	bool result;

	// Start rendering
	mGraphicsObj->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		// need 2 textures - one at time N and one at time N-1

		mFrameBuffer3->BeginRender(0.0f,0.0f,0.0f,1.0f);
		{
			mGraphicsObj->SetZBufferState(false);
			mQuad->SetTexture(pPrev);
			result = mQuad->Render(&viewMatrix,&orthoMatrix);
			if (!result)
				return false;
			mGraphicsObj->SetZBufferState(true);
		}
		mFrameBuffer3->EndRender();

		pPrev = (ID3D11ShaderResourceView*)mFrameBuffer3->GetTextureResource();

		// render texture N to frame Buffer 1 and texture N-1 to frame Buffer 2
		mFrameBuffer->BeginRender(0.0f,0.0f,0.0f,1.0f);
		{
			mGraphicsObj->SetZBufferState(false);

			mQuad->SetTexture(pCurrent);
			result = mQuad->Render(&viewMatrix,&orthoMatrix);
			if (!result)
				return false;
			// perform any extra interaction on frame buffer 1
			I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
			if (inputSystem->IsMouseLeftDown()) {
				int x,y;
				inputSystem->GetMousePos(x,y);
				int width,height;
				mGraphicsObj->GetScreenDimensions(width,height);
				float xPos = MapValue((float)x,0.0f,(float)width,-1.0f,1.0f);
				float yPos = MapValue((float)y,0.0f,(float)height,-1.0f,1.0f);
				mColorQuad->mTransform.position = Vector3f(xPos,-yPos,0);
				result = mColorQuad->Render(&viewMatrix,&orthoMatrix);
				if (!result)
					return false;
			}
			mGraphicsObj->SetZBufferState(true);
		}
		mFrameBuffer->EndRender();

		//pPrev = pCurrent;
		pCurrent = (ID3D11ShaderResourceView*)mFrameBuffer->GetTextureResource();

		// perform wave equation		
		mFrameBuffer2->BeginRender(0.0f,0.0f,0.0f,1.0f);
		{
			mGraphicsObj->SetZBufferState(false);
			mQuad->GetRenderer()->RenderBuffers(d3dGraphicsObj->GetDeviceContext());
			ID3D11ShaderResourceView* texArray[2] = {pCurrent, pPrev};
			result = mWaveShader->Render(d3dGraphicsObj, mQuad->GetRenderer()->GetIndexCount(), texArray);
			if (!result)
				return false;
			mGraphicsObj->SetZBufferState(true);
		}
		mFrameBuffer2->EndRender();

		pPrev = pCurrent;
		pCurrent = (ID3D11ShaderResourceView*)mFrameBuffer2->GetTextureResource();		

		//pPrev = pCurrent;
		//pCurrent = (ID3D11ShaderResourceView*)mFrameBuffer4->GetTextureResource();
		
		// Render texture to screen
		mGraphicsObj->SetZBufferState(false);
		mQuad->SetTexture(pCurrent);
		result = mQuad->Render(&viewMatrix,&orthoMatrix);
		if (!result)
			return false;
		mGraphicsObj->SetZBufferState(true);
	}
	// Finish rendering and display
	mGraphicsObj->EndRender();

	return true;
}

bool GraphicsSystem::TakeScreenshot(LPCWSTR name) const{
	return mGraphicsObj->Screenshot(name);
}