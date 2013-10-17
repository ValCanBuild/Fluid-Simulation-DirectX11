/***************************************************************
GraphicsSystem.cpp: In charge of displaying all content on the
screen.

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
#include "GraphicsSystem.h"
#include "ServiceProvider.h"

////Begin Set up NVidia PerfKit API ////
#define NVPM_INITGUID
#define PATH_TO_NVPMAPI_CORE L"d:/NvidiaPerfkit/bin/win7_x86/NvPmApi.Core.dll"
#include "../utilities/NVPerfKit/NvPmApi.Manager.h"

// Simple singleton implementation for grabbing the NvPmApi
static NvPmApiManager S_NVPMManager;
extern NvPmApiManager *GetNvPmApiManager() {return &S_NVPMManager;}

const NvPmApi *GetNvPmApi() {return S_NVPMManager.Api();}
////End Set up NVidia PerfKit API ////

#if defined (D3D)
	#include "../display/D3DGraphicsObject.h"
#endif

#include "../display/Scenes/Wave2DScene.h"
#include "../display/Scenes/Fluid2DScene.h"

GraphicsSystem::GraphicsSystem() {
	mFps = mCpuUsage = 0;
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

	D3DGraphicsObject *d3dObject = dynamic_cast<D3DGraphicsObject*>(mGraphicsObj.get());

	bool result = mGraphicsObj->Initialize(screenWidth,screenHeight,VSYNC_ENABLED,hwnd,FULL_SCREEN,SCREEN_DEPTH,SCREEN_NEAR);

	if (!result) {
		MessageBox(hwnd, L"Could not initialize Graphics Object", L"Error", MB_OK);
		return false;
	}

	// Initialize NVidia Perfkit
	/*result = InitializeNVPerfKit(hwnd);
	if (!result) {
		return false;
	}*/

	// Initialize current scene
	mCurrentScene = unique_ptr<IScene>(new Wave2DScene());
	result = mCurrentScene->Initialize(mGraphicsObj.get(),hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the scene", L"Error", MB_OK);
		return false;
	}

	// Initialize font
	mSpriteBatch = unique_ptr<SpriteBatch>(new SpriteBatch(d3dObject->GetDeviceContext()));
	mSpriteFont = unique_ptr<SpriteFont>(new SpriteFont(d3dObject->GetDevice(), L"data/TBNA.spritefont"));
	return true;
}

bool GraphicsSystem::Frame(float delta) const {
	// sample counters
	//NVPMUINT unCount;
	//NVPMRESULT nvResult = GetNvPmApi()->Sample(mNVPMContext, NULL, &unCount);
	mCurrentScene->Update(delta);
	return Render();
}

bool GraphicsSystem::Render() const {
	// first obtain all the needed matrices
	bool result;

	// Start rendering
	mGraphicsObj->BeginRender(0.0f,0.0f,0.0f,1.0f);
	{
		result = mCurrentScene->Render();
		if (!result)
			return false;
		result = RenderOverlay();
		if (!result)
			return false;
	}
	// Finish rendering and display
	mGraphicsObj->EndRender();

	return true;
}

bool GraphicsSystem::RenderOverlay() const {
	// Render overlay
	mSpriteBatch->Begin();
	{
		// Display FPS
		wstring text = L"FPS: " + std::to_wstring(mFps);
		mSpriteFont->DrawString(mSpriteBatch.get(),text.c_str(),XMFLOAT2(10,10));

		// Display CPU uage
		text = L"CPU Usage: " + std::to_wstring(mCpuUsage) + L"%";
		mSpriteFont->DrawString(mSpriteBatch.get(),text.c_str(),XMFLOAT2(10,30));

		// Display Mouse Coords
		int x,y;
		ServiceProvider::Instance().GetInputSystem()->GetMousePos(x,y);
		text = L"Mouse X: " + std::to_wstring(x) + L" Y: " +  std::to_wstring(y);
		mSpriteFont->DrawString(mSpriteBatch.get(),text.c_str(),XMFLOAT2(10,50));
		// Display GPU info
		/*NVPMUINT64 value(0), cycle(0);
		GetNvPmApi()->GetCounterValueByName(mNVPMContext, "gpu_idle", 0, &value, &cycle);
		float finalValue = (float) value / (float) cycle * 100.0f;
		wstring gpuText = L"GPU Value: " + std::to_wstring(finalValue);
		mSpriteFont->DrawString(mSpriteBatch.get(),gpuText.c_str(),XMFLOAT2(10,30));*/
	}
	mSpriteBatch->End();

	return true;
}

bool GraphicsSystem::InitializeNVPerfKit(HWND hwnd) {
	D3DGraphicsObject *d3dObject = dynamic_cast<D3DGraphicsObject*>(mGraphicsObj.get());

	if (GetNvPmApiManager()->Construct(PATH_TO_NVPMAPI_CORE) != S_OK) {
		MessageBox(hwnd, L"Could not construct NVPerfkit manager", L"Error", MB_OK);
		return false; 
	}

	NVPMRESULT nvResult = GetNvPmApi()->Init();
	if (nvResult != NVPM_OK) {
		MessageBox(hwnd, L"Could not initialize NVPerfkit API", L"Error", MB_OK);
		return false;
	}

	mNVPMContext = NVPMContext(0);

	nvResult = GetNvPmApi()->CreateContextFromD3D11Device(d3dObject->GetDevice(),&mNVPMContext);
	if (nvResult != NVPM_OK) {
		MessageBox(hwnd, L"Could not initialize NVPerfkit context", L"Error", MB_OK);
		return false;
	}


	// NVidia perfkit has been successfuly initialized, add counters
	nvResult = GetNvPmApi()->AddCounterByName(mNVPMContext,"gpu_idle");

	return true;
}

bool GraphicsSystem::TakeScreenshot(LPCWSTR name) const {
	return mGraphicsObj->Screenshot(name);
}

void GraphicsSystem::SetMonitorData(int fps, int cpuUsage) {
	mFps = fps;
	mCpuUsage = cpuUsage;
}