/***************************************************************
GraphicsObject.h: Abstract class that encapsulates a graphics
object to be used when displaying graphics on the screen.
Different APIs extend this and provide their own implementation

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#ifndef _IGRAPHICSOBJECT_H
#define _IGRAPHICSOBJECT_H

#include <windows.h>

#if defined (D3D)
	#include "SimpleMath.h"
	using namespace DirectX::SimpleMath;
#endif

class IGraphicsObject {
public:
	IGraphicsObject(){}
	virtual ~IGraphicsObject(){}

	virtual bool Initialize(int screenWidth, int screenHeight, bool vSync, HWND hwnd, bool fullScreen, float screenDepth, float screenNear) = 0;
	
	virtual void BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha) = 0;
	virtual void EndRender() = 0;

	virtual void GetVideoCardInfo(char *cardName, int& memory) const = 0;

	virtual void GetWorldMatrix(Matrix&) const = 0;
	virtual void GetOrthoMatrix(Matrix&) const = 0;

	virtual void GetScreenDimensions(int &width, int &height) const = 0;
	virtual void GetScreenDepthInfo(float &nearVal, float &farVal) const = 0;

	virtual void SetBackBufferRenderTarget() const = 0;
	
	virtual void SetZBufferState(bool state) const = 0;
	virtual void SetAlphaBlendState(bool state) const = 0;
	
	virtual void TurnWireframeOn() const = 0;
	virtual void TurnWireframeOff() const = 0;
	
	virtual bool Screenshot(LPCWSTR name) const = 0;	
};
#endif