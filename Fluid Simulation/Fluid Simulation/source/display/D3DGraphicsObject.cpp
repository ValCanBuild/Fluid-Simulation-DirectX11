/***************************************************************
D3DGraphicsObject.cpp: A class that encapsulates a 
D3D graphics object implementation

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#include <wincodec.h>

#include "D3DGraphicsObject.h"
#include "ScreenGrab.h"

using namespace DirectX;

D3DGraphicsObject::D3DGraphicsObject(){

}

D3DGraphicsObject::~D3DGraphicsObject(){
	if(mSwapChain){
		mSwapChain->SetFullscreenState(false, NULL);
	}
#if defined (_DEBUG)
	CComPtr<ID3D11Debug> d3dDebug;
	mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
#endif
}

bool D3DGraphicsObject::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
			  float screenDepth, float screenNear) {
	// Store the vsync setting.
	mVsyncEnabled = vsync;

	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	mScreenDepth = screenDepth;
	mScreenNear = screenNear;

	// Create a DirectX graphics interface factory
	IDXGIFactory* factory;
	HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) {
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	IDXGIAdapter* adapter;
	result = factory->EnumAdapters(0, &adapter);
	if(FAILED(result)) {
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	IDXGIOutput* adapterOutput;
	result = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(result)) {
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	unsigned int numModes;
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result)) {
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList) {
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result)) {
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	unsigned int i;
	unsigned int numerator,denominator;
	for(i = 0; i < numModes; i++) {
		if(displayModeList[i].Width == (unsigned int)screenWidth) {
			if(displayModeList[i].Height == (unsigned int)screenHeight)	{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result)) {
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	mVideoCardMemoryMB = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	int error = wcstombs_s(&stringLength, mVideoCardDescription, 128, adapterDesc.Description, 128);
	if(error != 0) {
		return false;
	}

	// Release the display mode list.
	delete [] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if(mVsyncEnabled) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !fullscreen;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	UINT creationFlags = 0;
	#if defined (_DEBUG)
	creationFlags = D3D11_CREATE_DEVICE_DEBUG;
	#endif
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, &featureLevel, 1, 
		D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain, &mDevice, NULL, &mDeviceContext);
	if(FAILED(result)) {
		return false;
	}

	// Get the pointer to the back buffer.
	ID3D11Texture2D* backBufferPtr;
	result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(result)) {
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = mDevice->CreateRenderTargetView(backBufferPtr, NULL, &mRenderTargetView);
	if(FAILED(result)) {
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = mDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilBuffer);
	if(FAILED(result)) {
		return false;
	}

	// Create all depth stencil states
	if (!BuildDepthStencilStates()){
		return false;
	}

	// Set the depth stencil state.
	mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);

	// Initailze the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = mDevice->CreateDepthStencilView(mDepthStencilBuffer, &depthStencilViewDesc, &mDepthStencilView);
	if(FAILED(result)) {
		return false;
	}

	//Create blend states
	if (!BuildBlendStates()){
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	mDeviceContext->OMSetRenderTargets(1, &(mRenderTargetView.p), mDepthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = mDevice->CreateRasterizerState(&rasterDesc, &mRasterState);
	if(FAILED(result)) {
		return false;
	}

	// Create wireframe rasterizer
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	result = mDevice->CreateRasterizerState(&rasterDesc, &mRasterStateWireframe);
	if(FAILED(result)) {
		return false;
	}

	// Now set the rasterizer state.
	mDeviceContext->RSSetState(mRasterState);

	D3D11_VIEWPORT viewport;
	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	mDeviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	float fieldOfView = (float)PI / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	// Create an orthographic projection matrix for 2D rendering.
	mOrthoMatrix = Matrix::CreateOrthographic((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}

bool D3DGraphicsObject::BuildBlendStates() {
	//create the transparent BS
	D3D11_BLEND_DESC blendDesc = {0};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT result = mDevice->CreateBlendState(&blendDesc, &mTransparentBS);

	if (FAILED(result)){
		return false;
	}

	// Clear the blend state description.
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = mDevice->CreateBlendState(&blendDesc, &mAlphaEnabledBS);

	if(FAILED(result)){
		return false;
	}

	// Modify the description to create an alpha disabled blend state description.
	blendDesc.RenderTarget[0].BlendEnable = FALSE;

	// Create the second blend state using the description.
	result = mDevice->CreateBlendState(&blendDesc, &mAlphaDisabledBS);

	if(FAILED(result)){
		return false;
	}

	return true;
}

bool D3DGraphicsObject::BuildDepthStencilStates() {
	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the default depth stencil state.
	HRESULT result = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
	if(FAILED(result)) {
		return false;
	}

	// Create the depth disabled stencil state
	depthStencilDesc.DepthEnable = false;
	result = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthDisabledStencilState);
	if(FAILED(result)) {
		return false;
	}

	return true;
}

void D3DGraphicsObject::BeginRender(float red, float green, float blue, float alpha) {
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, color);
	
	// Clear the depth buffer.
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DGraphicsObject::EndRender() {
	// Present the back buffer to the screen since rendering is complete.
	mSwapChain->Present(mVsyncEnabled ? 1 : 0, 0);
}

void D3DGraphicsObject::SetBackBufferRenderTarget() const {
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView.p, mDepthStencilView);
}

bool D3DGraphicsObject::Screenshot(LPCWSTR name) const {
	HRESULT hr;
	ID3D11Resource *backbufferRes;
	mRenderTargetView->GetResource(&backbufferRes);
	hr = SaveWICTextureToFile(mDeviceContext, backbufferRes, GUID_ContainerFormatJpeg, name);
	if (FAILED(hr)){
		return false;
	}
	backbufferRes->Release();
	return true;
}

void D3DGraphicsObject::SetAlphaBlendState(bool state) const {
	float blendFactors[4];	

	// Setup the blend factor.
	blendFactors[0] = 0.0f;
	blendFactors[1] = 0.0f;
	blendFactors[2] = 0.0f;
	blendFactors[3] = 0.0f;
	
	// Turn on the alpha blending.
	if (state)
		mDeviceContext->OMSetBlendState(mAlphaEnabledBS, blendFactors, 0xffffffff);
	else
		mDeviceContext->OMSetBlendState(mAlphaDisabledBS, blendFactors, 0xffffffff);
}

void D3DGraphicsObject::SetZBufferState(bool state) const {
	if (state)
		mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);
	else
		mDeviceContext->OMSetDepthStencilState(mDepthDisabledStencilState, 1);
}

void D3DGraphicsObject::TurnWireframeOn() const {
	mDeviceContext->RSSetState(mRasterStateWireframe);
}

void D3DGraphicsObject::TurnWireframeOff() const {
	mDeviceContext->RSSetState(mRasterState);
}

///GETTERS///
ID3D11Device* D3DGraphicsObject::GetDevice()const {
	return mDevice;
}

ID3D11DeviceContext* D3DGraphicsObject::GetDeviceContext()const {
	return mDeviceContext;
}

ID3D11DepthStencilView* D3DGraphicsObject::GetDepthStencilView() const {
	return mDepthStencilView;
}

void D3DGraphicsObject::GetWorldMatrix(Matrix& worldMatrix)const {
	worldMatrix = mWorldMatrix;
}

void D3DGraphicsObject::GetOrthoMatrix(Matrix& orthoMatrix)const {
	orthoMatrix = mOrthoMatrix;
}

void D3DGraphicsObject::GetScreenDimensions(int &width, int &height) const{
	width = mScreenWidth;
	height = mScreenHeight;
}

void D3DGraphicsObject::GetScreenDepthInfo(float &nearVal, float &farVal) const{
	nearVal = mScreenNear;	
	farVal = mScreenDepth;	
}

void D3DGraphicsObject::GetVideoCardInfo(char* cardName, int& memory)const {
	strcpy_s(cardName, 128, mVideoCardDescription);
	memory = mVideoCardMemoryMB;
	return;
}