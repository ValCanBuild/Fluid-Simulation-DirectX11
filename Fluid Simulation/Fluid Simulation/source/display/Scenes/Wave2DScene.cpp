/***************************************************************
Wave2DScene.cpp: Describes a scene that displays the 2D wave 
equation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
**************************************************************/

#include "Wave2DScene.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../D3DShaders/WaveShader.h"
#include "../D3DFrameBuffer.h"
#include "../../objects/D2DTexQuad.h"
#include "../../objects/D2DColorQuad.h"
#include "../../system/ServiceProvider.h"

Wave2DScene::Wave2DScene() {
	simStep = 1;
	prevStep = 0;
}

Wave2DScene::~Wave2DScene() {
	for (int i = 0; i < 3; i++) {
		delete mFrameArray[i];
		mFrameArray[i] = nullptr;
	}
	delete [] mFrameArray;
	mFrameArray = nullptr;
}

bool Wave2DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,0,0);

	mWaveShader = unique_ptr<WaveShader>(new WaveShader());
	bool result = mWaveShader->Initialize(pD3dGraphicsObj->GetDevice(),hwnd);
	if (!result) {
		return false;
	}

	int width,height;
	pD3dGraphicsObj->GetScreenDimensions(width,height);

	// Create the frame buffer array that will hold the states of the wave equation
	mFrameArray = new IFrameBuffer*[3];
	for (int i = 0; i < 3; i++) {
		mFrameArray[i] = new D3DFrameBuffer();
		mFrameArray[i]->Initialize(graphicsObject,width,height);
		mFrameArray[i]->BeginRender(0.0f,0.0f,0.0f,1.0f);
		mFrameArray[i]->EndRender();
	}

	// Create the quads
	mTexQuad = unique_ptr<D2DTexQuad>(new D2DTexQuad());
	result = mTexQuad->Initialize(graphicsObject,hwnd);
	if (!result) {
		return false;
	}

	mColorQuad = unique_ptr<D2DColorQuad>(new D2DColorQuad());
	result = mColorQuad->Initialize(graphicsObject,hwnd);
	if (!result) {
		return false;
	}

	mColorQuad->mTransform.scale = Vector3(0.03f,0.03f,1.0f);

	return true;
}

void Wave2DScene::Update(float delta) {
	mCamera->Update();
}

bool Wave2DScene::Render() {
	Matrix viewMatrix, orthoMatrix;
	pD3dGraphicsObj->GetOrthoMatrix(orthoMatrix);
	mCamera->GetViewMatrix(viewMatrix);

	bool result;

	int nextStep = simStep+1;
	if (nextStep > 2)
		nextStep = 0;

	mFrameArray[nextStep]->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		pD3dGraphicsObj->SetZBufferState(false);

		mTexQuad->GetRenderer()->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
		ID3D11ShaderResourceView* texArray[2] = {(ID3D11ShaderResourceView*)mFrameArray[simStep]->GetTextureResource(),
			(ID3D11ShaderResourceView*)mFrameArray[prevStep]->GetTextureResource()  };
		result = mWaveShader->Render(pD3dGraphicsObj, mTexQuad->GetRenderer()->GetIndexCount(),	texArray[0], texArray[1]);
		if (!result)
			return false;

		ID3D11ShaderResourceView *const pSRV[2] = {NULL,NULL};
		pD3dGraphicsObj->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV);

		// perform any extra input
		I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
		if (inputSystem->IsMouseLeftDown()) {
			int x,y;
			inputSystem->GetMousePos(x,y);
			int width,height;
			pD3dGraphicsObj->GetScreenDimensions(width,height);
			float xPos = MapValue((float)x,0.0f,(float)width,-1.0f,1.0f);
			float yPos = MapValue((float)y,0.0f,(float)height,1.0f,-1.0f);
			mColorQuad->mTransform.position = Vector3(xPos,yPos,0);
			result = mColorQuad->Render(&viewMatrix,&orthoMatrix);
			if (!result)
				return false;
		}
		pD3dGraphicsObj->SetZBufferState(true);
	}

	mFrameArray[nextStep]->EndRender();

	prevStep = simStep;
	simStep = nextStep;

	ID3D11ShaderResourceView* currTexture = (ID3D11ShaderResourceView*)mFrameArray[nextStep]->GetTextureResource();

	// Render texture to screen
	mTexQuad->SetTexture(currTexture);
	result = mTexQuad->Render(&viewMatrix,&orthoMatrix);
	if (!result)
		return false;

	return true;
}