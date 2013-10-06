/***************************************************************
D3DGraphicsObject.h: A class that encapsulates a 
D3D graphics object implementation

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DGRAPHICSOBJECT_H
#define _D3DGRAPHICSOBJECT_H

#define WIN32_LEAN_AND_MEAN

#include <memory>

#include "IGraphicsObject.h"
#include "../utilities/D3dIncludes.h"


using namespace std;

class D3DGraphicsObject : public IGraphicsObject {
public:
	D3DGraphicsObject();
	~D3DGraphicsObject();

	bool Initialize(int screenWidth, int screenHeight, bool vSync, HWND hwnd, bool fullScreen, float screenDepth, float screenNear);
	
	void BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha);
	void EndRender();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11DepthStencilView* GetDepthStencilView() const;

	void GetVideoCardInfo(char *cardName, int& memory) const;

	void GetProjectionMatrix(D3DXMATRIX&) const;
	void GetWorldMatrix(D3DXMATRIX&) const;
	void GetOrthoMatrix(D3DXMATRIX&) const;

	void GetScreenDimensions(int &width, int &height) const;
	void GetScreenDepthInfo(float &nearVal, float &farVal) const;
	
	void SetBackBufferRenderTarget() const;
	//void ResetViewport();
	void SetZBufferState(bool state) const;
	void SetAlphaBlendState(bool state) const;
	
	//void TurnWireframeOn();
	//void TurnWireframeOff();
 	
	//void TurnCullingOn();
	//void TurnCullingOff();
	
	bool Screenshot(LPCWSTR name) const;

private:
	//bool BuildRasterizers();
	bool BuildBlendStates();
	bool BuildDepthStencilStates();

private:
	int		mScreenWidth;
	int		mScreenHeight;
	float	mScreenNear;
	float	mScreenDepth;

	D3DXMATRIX mProjectionMatrix;
	D3DXMATRIX mWorldMatrix;
	D3DXMATRIX mOrthoMatrix;

	bool	mVsyncEnabled;
	int		mVideoCardMemoryMB;
	char	mVideoCardDescription[128];

	unique_ptr<ID3D11BlendState,COMDeleter>			mTransparentBS;
	unique_ptr<ID3D11BlendState,COMDeleter>			mAlphaEnabledBS;
	unique_ptr<ID3D11BlendState,COMDeleter>			mAlphaDisabledBS;

	unique_ptr<ID3D11DepthStencilState,COMDeleter>  mDepthStencilState;
	unique_ptr<ID3D11DepthStencilState,COMDeleter>  mDepthDisabledStencilState;
	
	unique_ptr<IDXGISwapChain,COMDeleter>			mSwapChain;
	unique_ptr<ID3D11Device,COMDeleter> 			mDevice;
	unique_ptr<ID3D11DeviceContext,COMDeleter> 		mDeviceContext;
	unique_ptr<ID3D11RenderTargetView,COMDeleter> 	mRenderTargetView;
	unique_ptr<ID3D11Texture2D,COMDeleter> 			mDepthStencilBuffer;
	unique_ptr<ID3D11DepthStencilView,COMDeleter>	mDepthStencilView;
	unique_ptr<ID3D11RasterizerState,COMDeleter>	mRasterState;

};
#endif