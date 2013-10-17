/***************************************************************
Wave2DScene.h: Describes a scene that displays the 2D wave 
equation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
**************************************************************/
#ifndef _WAVE2DSCENE_H
#define _WAVE2DSCENE_H

#include <vector>
#include <memory>
#include "IScene.h"

class D2DColorQuad;
class D2DTexQuad;
class Camera;
class D3DRenderer;
class WaveShader;
class IFrameBuffer;
class D3DGraphicsObject;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

using namespace std;

class Wave2DScene : public IScene {
public:
	Wave2DScene();
	~Wave2DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	bool Render();

private:
	unique_ptr<Camera> mCamera;
	unique_ptr<D2DColorQuad> mColorQuad;
	unique_ptr<D2DTexQuad> mTexQuad;
	unique_ptr<WaveShader> mWaveShader;
	IFrameBuffer** mFrameArray;
	D3DGraphicsObject* pD3dGraphicsObj;

	ID3D11ShaderResourceView** mWaveTextures;
	ID3D11UnorderedAccessView** mWaveUAVs;

	int simStep,prevStep;
};

#endif