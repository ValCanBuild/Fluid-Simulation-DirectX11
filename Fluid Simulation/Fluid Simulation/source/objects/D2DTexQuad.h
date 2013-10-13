/***************************************************************
D2DTexQuad.h: A Direct3D renderable which represents a 2D
texturable ortho quad with
initial size of 1, which when rendered covers the whole screen.

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _D2DTEXQUAD_H
#define _D2DTEXQUAD_H

#include <memory>

#include "IRenderable.h"
#include "../display/D3DRenderer.h"

class OrthoTextureShader;

using namespace std;

class D2DTexQuad : public IRenderable {
public:
	D2DTexQuad();
	~D2DTexQuad();

	bool Initialize(IGraphicsObject* graphicsObject, HWND hwnd);
	bool Render(const Matrix* viewMatrix, const Matrix* projMatrix);

	void SetScale(float scaleX, float scaleY);
	void SetTexture(ID3D11ShaderResourceView* textureResource);

	D3DRenderer* GetRenderer();

private:
	D3DGraphicsObject *pD3dGraphicsObj;
	ID3D11ShaderResourceView* pTextureResource;

	unique_ptr<D3DRenderer> mRenderer;
	unique_ptr<OrthoTextureShader> mShader;	

	Matrix mTransformMatrix;
	
};

#endif