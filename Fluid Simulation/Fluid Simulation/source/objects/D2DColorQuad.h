/***************************************************************
D2DColorQuad.h: A Direct3D renderable which represents a 2D
colored ortho quad with
initial size of 1, which when rendered covers the whole screen.
Default color is white

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _D2DCOLORQUAD_H
#define _D2DCOLORQUAD_H

#include <memory>

#include "IRenderable.h"
#include "../display/D3DRenderer.h"
#include "Transform.h"

class OrthoColorShader;

using namespace std;

class D2DColorQuad : public IRenderable {
public:
	D2DColorQuad();
	~D2DColorQuad();

	bool Initialize(IGraphicsObject* graphicsObject, HWND hwnd);
	bool Render(const D3DXMATRIX* viewMatrix, const D3DXMATRIX* projMatrix);

	void SetColor(Vector4f colorV);

public:
	Transform mTransform;

private:
	D3DGraphicsObject *pD3dGraphicsObj;

	unique_ptr<D3DRenderer> mRenderer;
	unique_ptr<OrthoColorShader> mShader;
};

#endif