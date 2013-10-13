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
#endif

#include "../display/Scenes/Wave2DScene.h"
#include "../display/Scenes/Fluid2DScene.h"

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
	
	mCurrentScene = unique_ptr<IScene>(new Fluid2DScene());
	result = mCurrentScene->Initialize(mGraphicsObj.get(),hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the scene", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool GraphicsSystem::Frame(float delta) {
	mCurrentScene->Update(delta);
	return Render();
}

bool GraphicsSystem::Render() {
	// first obtain all the needed matrices
	bool result;

	// Start rendering
	mGraphicsObj->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		result = mCurrentScene->Render();
		if (!result)
			return false;
	}
	// Finish rendering and display
	mGraphicsObj->EndRender();

	return true;
}

bool GraphicsSystem::TakeScreenshot(LPCWSTR name) const{
	return mGraphicsObj->Screenshot(name);
}