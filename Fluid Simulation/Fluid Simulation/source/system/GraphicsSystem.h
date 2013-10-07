/***************************************************************
GraphicsSystem.h: In charge of displaying all content on the
screen.

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#ifndef _GRAPHICSYSTEM_H_
#define _GRAPHICSYSTEM_H_

#include <windows.h>
#include <memory>

#include "../display/IGraphicsObject.h"

class Camera;
class D3DRenderer;
class OrthoTextureShader;
class OrthoColorShader;
class D3DTexture;
class IFrameBuffer;
class D2DTexQuad;
class D2DColorQuad;
class WaveShader;

using namespace std;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsSystem
////////////////////////////////////////////////////////////////////////////////
class GraphicsSystem
{
public:
	GraphicsSystem();
	GraphicsSystem(const GraphicsSystem&);
	~GraphicsSystem();

	bool Initialize(int, int, HWND);
	bool Frame();

	bool TakeScreenshot(LPCWSTR name) const;

private:
	bool Render();

private:
	unique_ptr<IGraphicsObject> mGraphicsObj;
	unique_ptr<Camera> mCamera;
	unique_ptr<D3DRenderer> mWindowRenderer;
	unique_ptr<D3DRenderer> mInteractionRenderer;
	unique_ptr<OrthoTextureShader> mTextureShader;
	unique_ptr<OrthoColorShader> mColorShader;
	unique_ptr<D3DTexture> mTexture;
	unique_ptr<IFrameBuffer> mFrameBuffer;
	unique_ptr<IFrameBuffer> mFrameBuffer2;
	unique_ptr<IFrameBuffer> mFrameBuffer3;
	//unique_ptr<IFrameBuffer> mFrameBuffer4;
	unique_ptr<D2DTexQuad> mQuad;
	unique_ptr<D2DColorQuad> mColorQuad;
	unique_ptr<WaveShader> mWaveShader;

	IFrameBuffer** mFrameArray;
};

#endif